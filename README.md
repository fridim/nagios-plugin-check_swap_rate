# Nagios plugin check_swap_rate

[![Build Status](https://travis-ci.org/fridim/nagios-plugin-check_swap_rate.svg?branch=master)](https://travis-ci.org/fridim/nagios-plugin-check_swap_rate)

This check is inspired by this [article](http://word.bitly.com/post/74839060954/ten-things-to-monitor).

> It’s common to check for swap usage above a threshold, but even if you have a small quantity of memory swapped, it’s actually the rate it’s swapped in/out that can impact performance, not the quantity. This is a much more direct check for that state.

A check should be instant. In order to avoid waiting several seconds for <code>vmstat</code> to compute the deritative of pswpin/pswpout, this check stores the current state in a file. The result is instant (no hang).

This check comes with performance data (used to generate a graph from a check).

## Build

    make

For now, it's only built/tested on Linux.

## Usage

	$ ./check_swap_rate -h
	Usage: ./check_swap_rate [-i INTERVAL] [-w W_THRESHOLD] [-c C_THRESHOLD] [-h] [-V]

	-i INTERVAL     interval in seconds (default 60)
	-c C_THRESHOLD  number of pages for critical threshold (default 60)
	-w W_THRESHOLD  number of pages for warning threshold (default 30)
	-h              this help
	-V              version


	Examples:
    ./check_swap_rate -i 2 -c 2 -w 1          # critical: 1 page/s  warning: 0.5 page/s
    ./check_swap_rate -i 600 -c 1200 -w 300   # critical: 2 page/s  warning: 0.5 page/s

Note: INTERVAL is only a mean to provide the rate, which in the end is a number of pages per sec.
