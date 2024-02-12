/* SPDX-License-Identifier: LGPL-2.1-or-later */
#include <stdio.h>
#include <string.h>

#define OPTIONS_(X)                                                                                  \
	X(boolean, help,   'h', "help",   "print this help and exit")                                \
	X(boolean, le,     'l', "le",     "test little-endian primality",          true)             \
	X(boolean, be,     'b', "be",     "test big-endian primality",             true)             \
	X(number,  base,   'B', "base",   "print results in given base",   "BASE",   10, 2,      62) \
	X(string,  filter, 'f', "filter", "select result types to output", "TERMS")                  \
	X(number,  reps,   'r', "reps",   "set primality tests per item",  "COUNT", 100, 0, INT_MAX) \
	X(boolean, unidir, 'u', "unidir", "include unidirectional primes")

#define OPTIONS OPTIONS_
#include "nekropt.h"

int
main (int    argc,
      char **argv)
{
	struct args args = nekropt_init(argc, argv);
	do {
		if (args.p.e)
			fprintf(stderr, "%s\n", strerror(args.p.e));
		else if (args.p.n && !args.m_help)
			break;
		usage(argv[0], "[OPTION]... STRING...");
	} while (0);
	return nekropt_fini(&args);
}
