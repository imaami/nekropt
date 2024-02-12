/* SPDX-License-Identifier: LGPL-2.1-or-later */
#include <stdio.h>
#include <string.h>

#include <gmp.h>

#define OPTIONS OPTIONS_
#define OPTIONS_(X)                                                                                  \
	X(boolean, help,   'h', "help",   "print this help and exit")                                \
	X(boolean, le,     'l', "le",     "test little-endian primality",          true)             \
	X(boolean, be,     'b', "be",     "test big-endian primality",             true)             \
	X(number,  base,   'B', "base",   "print results in given base",   "BASE",   10, 2,      62) \
	X(string,  filter, 'f', "filter", "select result types to output", "TERMS")                  \
	X(number,  reps,   'r', "reps",   "set primality tests per item",  "COUNT", 100, 0, INT_MAX) \
	X(boolean, unidir, 'u', "unidir", "include unidirectional primes")

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

	if (args.seen[OPT_le] != args.seen[OPT_be]) {
		args.m_le = args.seen[OPT_le];
		args.m_be = args.seen[OPT_be];
	}

	int base = (int)args.m_base;
	int reps = (int)args.m_reps;

	mpz_t le, be;
	if (args.m_le)
		mpz_init(le);
	if (args.m_be)
		mpz_init(be);

	for (int i = 0; i < args.p.n; i++) {
		char const *s = args.p.q[i];
		if (!*s)
			// empty string
			continue;

		size_t n = strlen(&s[1]), k = n, j = 0;
		for (; j < k && s[j] == s[k]; ++j, --k) {}
		if (j >= k)
			// byte-level palindrome
			continue;
		n++;

		int lp = 3;
		if (args.m_le) {
			mpz_import(le, 1U, -1, n, -1, 0, s);
			lp = mpz_probab_prime_p(le, reps) & 3;
			if (!lp && !args.m_unidir)
				// definitely not prime in little-endian
				continue;
		}

		int bp = 3;
		if (args.m_be) {
			mpz_import(be, 1U, -1, n,  1, 0, s);
			bp = mpz_probab_prime_p(be, reps) & 3;
			if (!bp && (!args.m_unidir || !lp))
				// definitely not prime in big-endian
				continue;
		}

		fputs(s, stdout);
		putchar('\t');
		if (args.m_le)
			mpz_out_str(stdout, base, le);
		else
			putchar('-');
		putchar('\t');
		if (args.m_be)
			mpz_out_str(stdout, base, be);
		else
			putchar('-');
		puts(&"\tnn\0\tpn\0\tyn\0"
		      "\t_n\0\tnp\0\tpp\0"
		      "\typ\0\t_p\0\tny\0"
		      "\tpy\0\tyy\0\t_y\0"
		      "\tn_\0\tp_\0\ty_\0"
		      "\t__"[((bp << 2) | lp) << 2]);
	}

	if (args.m_le)
		mpz_clear(le);
	if (args.m_be)
		mpz_clear(be);

	return letopt_fini(&args);
}
