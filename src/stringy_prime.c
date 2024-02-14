/* SPDX-License-Identifier: LGPL-2.1-or-later */
#include <stdio.h>
#include <string.h>

#include <gmp.h>

#define PROGNAME "stringy_prime"
#define SYNOPSIS "[OPTION]... STRING..."
#define PURPOSE  "Check ASCII strings for primality."

#define OPTIONS(X)                                      \
	X(boolean, help,          'h', "help",          \
	  "print this help text and exit")              \
	                                                \
	X(number,  base,          'B', "base",          \
	  "print results in specified base",            \
	  "BASE", 10, 2, 62)                            \
	                                                \
	X(boolean, big_endian,    'b', "big-endian",    \
	  "check primality in big-endian",              \
	  true)                                         \
	                                                \
	X(boolean, little_endian, 'l', "little-endian", \
	  "check primality in little-endian",           \
	  true)                                         \
	                                                \
	X(number,  reps,          'r', "reps",          \
	  "run at most COUNT primality tests",          \
	  "COUNT", 100, 0, INT_MAX)                     \
	                                                \
	X(boolean, always_print,  'a', "always-print",  \
	  "print all conversions, prime or not")        \
	                                                \
	X(boolean, palindromes,   'p', "palindromes",   \
	  "do not skip byte-level palindromes")         \
	                                                \
	X(boolean, single_endian, 's', "single-endian", \
	  "do not require both byte orders")

#define DETAILS  "By default, check in both byte orders."

#include "letopt.h"

int
main (int    argc,
      char **argv)
{
	struct letopt opt = letopt_init(argc, argv);

	if (letopt_nargs(&opt) < 1 || opt.m_help)
		letopt_helpful_exit(&opt);

	if (opt.has.little_endian != opt.has.big_endian) {
		opt.m_little_endian = opt.has.little_endian;
		opt.m_big_endian = opt.has.big_endian;
	}

	int base = (int)opt.m_base;
	int reps = (int)opt.m_reps;

	mpz_t le, be;
	if (opt.m_little_endian)
		mpz_init(le);
	if (opt.m_big_endian)
		mpz_init(be);

	for (int i = 0; i < letopt_nargs(&opt); i++) {
		char const *s = letopt_arg(&opt, i);
		if (!*s)
			// empty string
			continue;

		size_t n = strlen(&s[1]);
		if (!opt.m_palindromes) {
			size_t k = n, j = 0;
			for (; j < k && s[j] == s[k]; ++j, --k) {}
			if (j >= k)
				// byte-level palindrome
				continue;
		}
		n++;

		int lp = 3;
		if (opt.m_little_endian) {
			mpz_import(le, 1U, -1, n, -1, 0, s);
			lp = mpz_probab_prime_p(le, reps) & 3;
			if (!lp && !opt.m_always_print
			        && !opt.m_single_endian)
				// definitely not prime in little-endian
				continue;
		}

		int bp = 3;
		if (opt.m_big_endian) {
			mpz_import(be, 1U, -1, n,  1, 0, s);
			bp = mpz_probab_prime_p(be, reps) & 3;
			if (!bp &&  !opt.m_always_print
			        && (!opt.m_single_endian
			            || lp == 0 || lp == 3))
				// definitely not prime in big-endian
				continue;
		}

		fputs(s, stdout);
		putchar('\t');
		if (opt.m_little_endian)
			mpz_out_str(stdout, base, le);
		else
			putchar('-');
		putchar('\t');
		if (opt.m_big_endian)
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

	if (opt.m_little_endian)
		mpz_clear(le);
	if (opt.m_big_endian)
		mpz_clear(be);

	return letopt_fini(&opt);
}
