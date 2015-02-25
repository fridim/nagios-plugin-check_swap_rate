#include <getopt.h>                     // for optarg, getopt
#include <stdio.h>                      // for printf, fprintf, stderr, etc
#include <stdlib.h>                     // for exit, atoi
#include <time.h>                       // for time_t, time
#include <unistd.h>                     // for access, F_OK

typedef unsigned long long int ulli;
const char *fname = "/tmp/check_swap_rate.TIYUl6P61oMR9GSMhz.data";

#define VERSION "1.0"

/* default values */
#define CRITICAL_DEFAULT 60
#define WARNING_DEFAULT 30
#define INTERVAL 60

enum {
	STATE_OK,
	STATE_WARNING,
	STATE_CRITICAL,
	STATE_UNKNOWN,
};

void get_values(time_t * const s, ulli * const swpin, ulli * const swpout)
{
	FILE *f;
	char line[BUFSIZ];

	f = fopen("/proc/vmstat", "r");
	if (!f) {
		fprintf(stderr, "Can't open /proc/vmstat\n");
		exit(STATE_UNKNOWN);
	}

	int bin = 0, bout = 0;
	while (fgets(line, BUFSIZ - 1, f) != NULL) {
		if (!bin && sscanf(line, "pswpin %llu", swpin) == 1) {
			bin = 1;
		}

		if (!bout && sscanf(line, "pswpout %llu", swpout) == 1) {
			bout = 1;
		}

		if (bout && bin)
			break;
	}

	fclose(f);

	*s = time(NULL);
	if (!s) {
		fprintf(stderr, "Can't get Epoch time\n");
		exit(STATE_UNKNOWN);
	}
}

void write_values(const time_t s, const ulli swpin, const ulli swpout)
{
	FILE *f;

	f = fopen(fname, "w+");
	if (!f) {
		fprintf(stderr, "Can't open db file\n");
		exit(STATE_UNKNOWN);
	}

	fprintf(f, "%ld %llu %llu\n", s, swpin, swpout);
	fclose(f);
}

void get_last_values(time_t * const s, ulli * const swpin, ulli * const swpout)
{
	FILE *f;
	f = fopen(fname, "r");
	if (!f) {
		fprintf(stderr, "Can't opendata file\n");
		exit(STATE_UNKNOWN);
	}

	if (fscanf(f, "%ld %llu %llu", s, swpin, swpout) != 3) {
		fprintf(stderr, "ERROR while parsing old values\n");
		exit(STATE_UNKNOWN);
	}
	fclose(f);
}

void usage(char *argv[])
{
	printf("Usage: %s [-i INTERVAL] [-w W_THRESHOLD] [-c C_THRESHOLD] [-h] [-V]\n", argv[0]);
	printf("\n");
	printf("-i INTERVAL\tinterval in seconds (default %d)\n", INTERVAL);
	printf("-c C_THRESHOLD\tnumber of pages for critical threshold (default %d)\n", CRITICAL_DEFAULT);
	printf("-w W_THRESHOLD\tnumber of pages for warning threshold (default %d)\n", WARNING_DEFAULT);
	printf("-h \t\tthis help\n");
	printf("-V \t\tversion\n");
	printf("\n");
	printf("\nExamples: \n");
	printf("\t%s -i 2 -c 2 -w 1          # critical: 1 page/s  warning: 0.5 page/s\n", argv[0]);
	printf("\t%s -i 600 -c 1200 -w 300   # critical: 2 page/s  warning: 0.5 page/s\n", argv[0]);
}

void output(const char *const status, const float rin, const float rout, const float w, const float c)
{
	printf("%s: pages/s %.2f IN, %.2f OUT", status, rin, rout);
	printf(" | swapin=%f;%f;%f; swapout=%f;%f;%f;\n", rin, w, c, rout, w, c);
}

int main(int argc, char *argv[])
{
	int c;
	int warning = WARNING_DEFAULT, critical = CRITICAL_DEFAULT;
	int t = INTERVAL;

	while (c = getopt(argc, argv, "c:w:i:hV"), c != -1) {
		switch (c) {
		case 'c':
			critical = atoi(optarg);
			break;
		case 'w':
			warning = atoi(optarg);
			break;
		case 'V':
			printf("%s version %s\n", argv[0], VERSION);
			return STATE_OK;
			break;
		case 'i':
			t = atoi(optarg);
			if (t == 0) {
				fprintf(stderr, "INTERVAL can't be 0.\n");
				return STATE_UNKNOWN;
			}
			break;
		case 'h':
			usage(argv);
			return STATE_UNKNOWN;
		default:
			fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
			return STATE_UNKNOWN;
		}
	}

	ulli swpin = 0, swpout = 0, swpin_old = 0, swpout_old = 0;
	time_t s, s_old;

	get_values(&s, &swpin, &swpout);

	if (access(fname, F_OK) == -1) {
		printf("OK: first run.\n");
		write_values(s, swpin, swpout);
		return STATE_OK;
	}

	unsigned int elapsed_t = 0;
	float ratio_in = 0, ratio_out = 0;
	float c_ratio = critical / (float)t;
	float w_ratio = warning / (float)t;

	get_last_values(&s_old, &swpin_old, &swpout_old);

	elapsed_t = s - s_old;

	if (elapsed_t == 0) {
		printf("UNKNOWN: run twice in the same sec, can't compute rate.\n");
		return STATE_UNKNOWN;
	}

	write_values(s, swpin, swpout);

	ratio_in = (swpin - swpin_old) / (float)elapsed_t;
	ratio_out = (swpout - swpout_old) / (float)elapsed_t;

	if (ratio_in >= c_ratio || ratio_out >= c_ratio) {
		output("CRITICAL", ratio_in, ratio_out, w_ratio, c_ratio);
		return STATE_CRITICAL;
	} else if (ratio_in >= w_ratio || ratio_out >= w_ratio) {
		output("WARNING", ratio_in, ratio_out, w_ratio, c_ratio);
		return STATE_WARNING;
	} else {
		output("OK", ratio_in, ratio_out, w_ratio, c_ratio);
		return STATE_OK;
	}

	return STATE_UNKNOWN;
}
