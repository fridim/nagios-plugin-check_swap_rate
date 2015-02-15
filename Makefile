P=check_swap_rate
OBJECTS=
CFLAGS=-g -Wall -Werror -O2 -std=c99
LDLIBS=

$(P): $(OBJECTS)

indent:
	indent -linux *.c -l110
