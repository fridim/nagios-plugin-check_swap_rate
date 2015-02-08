#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

typedef unsigned long long int ulli;
const char *fname = "/tmp/check_swap_rate.TIYUl6P61oMR9GSMhz.data";

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

void get_values(time_t * s, ulli * swpin, ulli * swpout)
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

void write_values(time_t s, ulli swpin, ulli swpout)
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

void get_last_values(time_t * s, ulli * swpin, ulli * swpout)
{
	FILE *f;
	f = fopen(fname, "r");
	if (!f) {
		fprintf(stderr, "Can't opendata file\n");
		exit(STATE_UNKNOWN);
	}

	if (fscanf(f, "%ld %llu %llu", s, swpin, swpout) != 3) {
		fprintf(stderr, "ERROR while parsing old values");
		exit(STATE_UNKNOWN);
	}
	fclose(f);
}

void usage(char *argv[])
{
	printf
	    ("Usage: %s [-t INTERVAL] [-w W_THRESHOLD] [-c C_THRESHOLD] [-h]\n",
	     argv[0]);
	printf("\n");
	printf("-t INTERVAL\tinterval in seconds (default %d)\n", INTERVAL);
	printf
	    ("-c C_THRESHOLD\tnumber of pages for critical threshold (default %d)\n",
	     CRITICAL_DEFAULT);
	printf
	    ("-w W_THRESHOLD\tnumber of pages for warning threshold (default %d)\n",
	     WARNING_DEFAULT);
	printf("\n");
	printf("\nExamples: \n");
	printf
	    ("\t%s -t 2 -c 2 -w 1          # critical: 1 page/s  warning: 0.5 page/s\n",
	     argv[0]);
	printf
	    ("\t%s -t 600 -c 1200 -w 300   # critical: 2 page/s  warning: 0.5 page/s\n",
	     argv[0]);
}

int main(int argc, char *argv[])
{
	int c;
	int warning = WARNING_DEFAULT, critical = CRITICAL_DEFAULT;
	int t = INTERVAL;

	while (c = getopt(argc, argv, "c:w:t:h"), c != -1) {
		switch (c) {
		case 'c':
			if (sscanf(optarg, "%d", &critical) != 1) {
				fprintf(stderr,
					"ERROR while parsing critical value");
				return STATE_UNKNOWN;
			}
			break;
		case 'w':
			if (sscanf(optarg, "%d", &warning) != 1) {
				fprintf(stderr,
					"ERROR while parsing warning value");
				return STATE_UNKNOWN;
			}
			break;
		case 't':
			if (sscanf(optarg, "%d", &t) != 1) {
				fprintf(stderr,
					"ERROR while parsing interval value");
				return STATE_UNKNOWN;
			}
			break;
		case 'h':
			usage(argv);
			return STATE_UNKNOWN;
		default:
			fprintf(stderr, "Try '%s -h' for more information.",
				argv[0]);
			return STATE_UNKNOWN;
		}
	}

	ulli swpin = 0, swpout = 0, swpin_old = 0, swpout_old = 0;
	time_t s, s_old;

	get_values(&s, &swpin, &swpout);

	if (access(fname, F_OK) != -1) {
		unsigned int elapsed_t = 0;
		float ratio_in = 0, ratio_out = 0;
		float c_ratio = critical / (float)t;
		float w_ratio = warning / (float)t;

		get_last_values(&s_old, &swpin_old, &swpout_old);

		elapsed_t = s - s_old;

		if (elapsed_t == 0) {
			printf
			    ("UNKNOWN: run twice in the same sec, can't compute rate.\n");
			return STATE_UNKNOWN;
		}

		ratio_in = (swpin - swpin_old) / (float)elapsed_t;
		ratio_out = (swpout - swpout_old) / (float)elapsed_t;

		if (ratio_in >= c_ratio || ratio_out >= c_ratio) {
			printf
			    ("CRITICAL: %f pages/s swapin | %f pages/s swapout\n",
			     ratio_in, ratio_out);
			return STATE_CRITICAL;
		} else if (ratio_in >= w_ratio || ratio_out >= w_ratio) {
			printf
			    ("WARNING: %f pages/s swapin | %f pages/s swapout\n",
			     ratio_in, ratio_out);
			return STATE_WARNING;
		} else {
			printf("OK: %f pages/s swapin | %f pages/s swapout\n",
			       ratio_in, ratio_out);
			return STATE_OK;
		}
	} else {
		printf("OK: first run.\n");
	}

	write_values(s, swpin, swpout);

	return STATE_OK;
}
