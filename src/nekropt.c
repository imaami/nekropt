/* SPDX-License-Identifier: LGPL-2.1-or-later */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nekropt.h"

struct nekroparser
nekroparser_init (int    argc,
                  char **argv)
{
	struct nekroparser parser = {
		.v = argv,
		.c = argc
	};

	if (parser.c > 1 &&
	    !(parser.q = calloc((size_t)parser.c, sizeof *parser.q)))
		parser.e = errno;

	return parser;
}

bool
nekropt_get_number_arg (struct nekroparser *const parser,
                        int64_t *const            dest,
                        const int64_t             min,
                        const int64_t             max)
{
	if (!*parser->p) {
		parser->e = EINVAL;
		return false;
	}

	char *end = parser->p;
	int64_t n = _Generic(n
		, long: strtol
		, long long: strtoll
	)(parser->p, &end, 0);

	if (!*end && n >= min && n <= max) {
		*dest = n;
		return true;
	}

	parser->e = errno;
	if (!parser->e)
		parser->e = *end ? EINVAL : ERANGE;

	return false;
}

bool
nekropt_get_string_arg (struct nekroparser *const parser,
                        char const **const        dest)
{
	if (!*parser->p) {
		parser->e = EINVAL;
		return false;
	}

	*dest = parser->p;
	return true;
}

int
nekropt_get_long_opt_arg (struct nekroparser *const parser,
                          const size_t              opt_len)
{
	do {
		if (!parser->p[opt_len]) {
			if (++parser->i >= parser->c)
				break;
			parser->p = parser->v[parser->i];
		} else {
			if (parser->p[opt_len] != '=')
				return EAGAIN;
			parser->p += opt_len + 1U;
		}

		if (*parser->p)
			return 0;
	} while(0);

	parser->e = EINVAL;
	return EINVAL;
}
