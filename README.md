# Nagios plugin check_swap_rate

[![Build Status](https://travis-ci.org/fridim/nagios-plugin-check_swap_rate.svg?branch=master)](https://travis-ci.org/fridim/nagios-plugin-check_swap_rate)

This check is inspired by this [article](http://word.bitly.com/post/74839060954/ten-things-to-monitor).

> It’s common to check for swap usage above a threshold, but even if you have a small quantity of memory swapped, it’s actually the rate it’s swapped in/out that can impact performance, not the quantity. This is a much more direct check for that state.

For now, it's only built/tested on Linux.

## Build

    make

## Usage

	$ ./check_swap_rate -h
	Usage: ./check_swap_rate [-t INTERVAL] [-w W_THRESHOLD] [-c C_THRESHOLD] [-h]

	-t INTERVAL     interval in seconds (default 60)
	-c C_THRESHOLD  number of pages for critical threshold (default 60)
	-w W_THRESHOLD  number of pages for warning threshold (default 30)


	Examples:
    ./check_swap_rate -t 2 -c 2 -w 1          # critical: 1 page/s  warning: 0.5 page/s
    ./check_swap_rate -t 600 -c 1200 -w 300   # critical: 2 page/s  warning: 0.5 page/s
