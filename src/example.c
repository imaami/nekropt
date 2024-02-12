/* SPDX-License-Identifier: LGPL-2.1-or-later */
#include <stdio.h>
#include <string.h>

#include <gmp.h>

#define OPTIONS(X)                                        \
	X(boolean, help, 'h', "help",                     \
	  "print this help text and exit")                \
	                                                  \
	X(number,  base, 'B', "base",                     \
	  "print results in the specified base",          \
	  "BASE", 10, 2, 62)                              \
	                                                  \
	X(boolean, big_endian, 'b', "big-endian",         \
	  "test primality in big-endian byte order",      \
	  true)                                           \
	                                                  \
	X(boolean, little_endian, 'l', "little-endian",   \
	  "test primality in little-endian byte order",   \
	  true)                                           \
	                                                  \
	X(boolean, palindromic, 'p', "palindromic",       \
	  "do not skip byte-level palindromic strings")   \
	                                                  \
	X(number,  reps, 'r', "reps",                     \
	  "run (at most) COUNT primality tests per item", \
	  "COUNT", 100, 0, INT_MAX)                       \
	                                                  \
	X(boolean, single_endian, 's', "single-endian",   \
	  "do not require both byte orders to be prime")

#include "letopt.h"

int
main (int    argc,
      char **argv)
{
	struct letopt args = letopt_init(argc, argv);
	do {
		if (args.p.e)
			fprintf(stderr, "%s\n", strerror(args.p.e));
		else if (args.p.n && !args.m_help)
			break;
		usage(argv[0], "[OPTION]... STRING...");
		return letopt_fini(&args);
	} while (0);

	if (args.seen[OPT_little_endian] != args.seen[OPT_big_endian]) {
		args.m_little_endian = args.seen[OPT_little_endian];
		args.m_big_endian = args.seen[OPT_big_endian];
	}

	int base = (int)args.m_base;
	int reps = (int)args.m_reps;

	mpz_t le, be;
	if (args.m_little_endian)
		mpz_init(le);
	if (args.m_big_endian)
		mpz_init(be);

	for (int i = 0; i < args.p.n; i++) {
		char const *s = args.p.q[i];
		if (!*s)
			// empty string
			continue;

		size_t n = strlen(&s[1]), k = n, j = 0;
		for (; j < k && s[j] == s[k]; ++j, --k) {}
		if (j >= k && !args.m_palindromic)
			// byte-level palindrome
			continue;
		n++;

		int lp = 3;
		if (args.m_little_endian) {
			mpz_import(le, 1U, -1, n, -1, 0, s);
			lp = mpz_probab_prime_p(le, reps) & 3;
			if (!lp && !args.m_single_endian)
				// definitely not prime in little-endian
				continue;
		}

		int bp = 3;
		if (args.m_big_endian) {
			mpz_import(be, 1U, -1, n,  1, 0, s);
			bp = mpz_probab_prime_p(be, reps) & 3;
			if (!bp && (!args.m_single_endian || !lp))
				// definitely not prime in big-endian
				continue;
		}

		fputs(s, stdout);
		putchar('\t');
		if (args.m_little_endian)
			mpz_out_str(stdout, base, le);
		else
			putchar('-');
		putchar('\t');
		if (args.m_big_endian)
			mpz_out_str(stdout, base, be);
		else
			putchar('-');
		puts(&"\tnn\0\tpn\0\tyn\0"
		      "\t_n\0\tnp\0\tpp\0"
		      "\typ\0\t_p\0\tny\0"
		      "\tpy\0\tyy\0\t_y\0"
		      "\tn_\0\tp_\0\ty_\0"
		      "\t__"[((bp << 2U) | lp) << 2U]);
	}

	if (args.m_little_endian)
		mpz_clear(le);
	if (args.m_big_endian)
		mpz_clear(be);

	return letopt_fini(&args);
}
