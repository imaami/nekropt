/* SPDX-License-Identifier: LGPL-2.1-or-later */
#ifndef NEKROPT_H_
#define NEKROPT_H_

#if __STDC_VERSION__ < 202000L || (__GNUC__ < 13 && \
    (!defined(__clang_major__) || __clang_major__ < 18))
# include <stdbool.h>
#endif

#if defined OPTIONS
# include <errno.h>
# include <limits.h>
# include <stddef.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
#endif // OPTIONS

struct nekroparser {
	char       **v;
	int          c;
	int          e;
	char const **q;
	int          i;
	int          n;
	char        *p;
};

#if defined OPTIONS
#define NEKROPTIONS(X) eval(OPTIONS(X filter_args))
#define call(f, ...)   f(__VA_ARGS__)
#define eval(...)      __VA_ARGS__
#define first(x, ...)  x

#define boolean_arg(c,s,d,...) c, s, "", d
#define filter_args(T,tag,...) (T##_arg(__VA_ARGS__))
#define generic_arg(c,s,d,...) c, s, "" first(__VA_ARGS__,), d
#define number_arg generic_arg
#define string_arg generic_arg

#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wc++-compat"
# pragma clang diagnostic ignored "-Wc++98-compat"
# pragma clang diagnostic ignored "-Wgnu-empty-initializer"
# pragma clang diagnostic ignored "-Wpre-c2x-compat"
# pragma clang diagnostic ignored "-Wunknown-attributes"
#endif // __clang__

#define ws0x0()  " "
#define ws0x1()  "  "
#define ws0x2()  "    "
#define ws0x3()  "        "
#define ws0x4()  "                "
#define ws0x5()  "                                "
#define ws0x6()  "                                                                "
#define ws0x7() ws0x6() ws0x6()
#define ws0x8() ws0x7() ws0x7()
#define ws0x9() ws0x8() ws0x8()
#define ws0xa() ws0x9() ws0x9()
#define ws0xb() ws0xa() ws0xa()

#define bit_(b, n, v) _Generic(&(char[1U + ((n) & (1U << (size_t)b))]){0} \
	, char(*)[1U + (1U << (size_t)b)]: (const struct bit##b##_on){v}  \
	, char(*)[1U]: (const struct bit_off){})

#define bit(b, n)      bit_(b, n, {ws##b()})
#define bit_type(b, n) typeof(bit_(b, n, 0))

#define ws_type(n) struct {                             \
	bit_type(0x0,n) bit0x0; bit_type(0x1,n) bit0x1; \
	bit_type(0x2,n) bit0x2; bit_type(0x3,n) bit0x3; \
	bit_type(0x4,n) bit0x4; bit_type(0x5,n) bit0x5; \
	bit_type(0x6,n) bit0x6; bit_type(0x7,n) bit0x7; \
	bit_type(0x8,n) bit0x8; bit_type(0x9,n) bit0x9; \
	bit_type(0xa,n) bit0xa; bit_type(0xb,n) bit0xb; }

#define ws_init(n) {                                    \
	bit(0x0,n), bit(0x1,n), bit(0x2,n), bit(0x3,n), \
	bit(0x4,n), bit(0x5,n), bit(0x6,n), bit(0x7,n), \
	bit(0x8,n), bit(0x9,n), bit(0xa,n), bit(0xb,n), }

#define dog(x,y) x##y
#define cat(x,y) dog(x, y)
#define sym()    cat(m_, __COUNTER__)
#define c_str(x) char sym()[sizeof x]

#define line_align(x) (sizeof(max_align)-sizeof x)
#define space(x)      ws_type(line_align(x)) sym()
#define space_init(x) ws_init(line_align(x))

#define gen_opt_tag(T,tag,...) OPT_##tag,
enum opt_tag {
	OPT_NONE = 0U,
	OPTIONS(gen_opt_tag)
};
#undef gen_opt_tag

#define arg0(def_arg, ...) first(__VA_OPT__(__VA_ARGS__,)def_arg,)
#define arg1_(def, x, ...) arg0(def,__VA_ARGS__)
#define arg1(def_arg, ...) arg1_(def_arg,__VA_ARGS__)
#define arg2_(def, x, ...) arg1_(def,__VA_ARGS__)
#define arg2(def_arg, ...) arg2_(def_arg,__VA_ARGS__)
#define arg3_(def, x, ...) arg2_(def,__VA_ARGS__)
#define arg3(def_arg, ...) arg3_(def_arg,__VA_ARGS__)

#define gen_arg_list(T,tag,c,s,d,...) \
	T,tag,c,s,d T##_(__VA_ARGS__)
#define boolean_(...)   ,""           \
	, arg0(    false,__VA_ARGS__),,
#define string_(...)                  \
	, arg0(    "ARG",__VA_ARGS__) \
	, arg1(       "",__VA_ARGS__),,
#define number_(...)                  \
	, arg0(    "NUM",__VA_ARGS__) \
	, arg1(        0,__VA_ARGS__) \
	, arg2(INT64_MIN,__VA_ARGS__) \
	, arg3(INT64_MAX,__VA_ARGS__)

#define gen_opt_var(T,...) call(opt_var \
	, gen_arg_list(T,__VA_ARGS__),)

struct args {
	struct nekroparser p;

	#define opt_var(T, tag, c, s, d, a, ...) \
		T##_var(tag,__VA_ARGS__,)

	#define boolean_var(...)
	#define string_var(...)
	#define number_var(tag,...) int64_t m_##tag;
	OPTIONS(gen_opt_var)
	#undef number_var
	#undef string_var

	#define number_var(...)
	#define string_var(tag,...) char const *m_##tag;
	OPTIONS(gen_opt_var)
	#undef string_var
	#undef boolean_var

	#define string_var(...)
	#define boolean_var(tag,...) bool m_##tag;
	OPTIONS(gen_opt_var)
	#undef boolean_var
	#undef string_var
	#undef number_var
	#undef opt_var

	#define increment(...) + 1U
	bool seen[1U OPTIONS(increment)];
	#undef increment
};

#define parse_str(T, tag, chr, str, ...)                           \
	if (!__builtin_strncmp(args->p.p, str, sizeof str - 1U)) { \
		call(T##_str_opt,                                  \
		     gen_arg_list(T, tag, chr, str, __VA_ARGS__),) \
	}

#define boolean_str_opt(T, tag, chr, str, ...)  \
	if (!args->p.p[sizeof str - 1U]) {      \
		args->m_##tag = true;           \
		return OPT_##tag;               \
	}

#define number_str_opt(T, tag, chr, str, doc, a, def, min, max, ...) \
	int e = nekropt_get_long_opt_arg(&args->p, sizeof str - 1U); \
	if (!e) {                                                    \
		if (nekropt_get_number_arg(&args->p, &args->m_##tag, \
		                           min, max))                \
			return OPT_##tag;                            \
		break;                                               \
	}                                                            \
	if (e != EAGAIN)                                             \
		break;

#define string_str_opt(T, tag, chr, str, ...)                        \
	int e = nekropt_get_long_opt_arg(&args->p, sizeof str - 1U); \
	if (!e) {                                                    \
		args->m_##tag = args->p.p;                           \
		return OPT_##tag;                                    \
	}                                                            \
	if (e != EAGAIN)                                             \
		break;

__attribute__((always_inline))
static inline enum opt_tag
handle_long_opt (struct args *const args)
{
	extern int
	nekropt_get_long_opt_arg (struct nekroparser *parser,
	                          size_t              opt_len);
	extern bool
	nekropt_get_number_arg (struct nekroparser *parser,
	                        int64_t            *dest,
	                        int64_t             min,
	                        int64_t             max);

	do {
		OPTIONS(parse_str)
		args->p.e = EINVAL;
	} while (0);

	return OPT_NONE;
}

#undef string_str_opt
#undef number_str_opt
#undef boolean_str_opt
#undef parse_str

#define parse_chr(T, tag, chr, ...)                           \
	case chr:                                             \
		++args->p.p;                                  \
		call(T##_chr_opt,                             \
		     gen_arg_list(T, tag, chr, __VA_ARGS__),) \
		return OPT_##tag;

#define boolean_chr_opt(T, tag, ...)          \
	args->m_##tag = true;                 \
	if (*args->p.p) {                     \
		args->seen[OPT_##tag] = true; \
		goto next;                    \
	}

#define number_chr_opt(T, tag, chr, str, doc, arg, def, min, max, ...)   \
	if (!*args->p.p) {                                               \
		if (++args->p.i >= args->p.c)                            \
			goto invargs;                                    \
		args->p.p = args->p.v[args->p.i];                        \
	}                                                                \
	if (!nekropt_get_number_arg(&args->p, &args->m_##tag, min, max)) \
		break;

#define string_chr_opt(T, tag, ...)                            \
	if (!*args->p.p) {                                     \
		if (++args->p.i >= args->p.c)                  \
			goto invargs;                          \
		args->p.p = args->p.v[args->p.i];              \
	}                                                      \
	if (!nekropt_get_string_arg(&args->p, &args->m_##tag)) \
		break;

__attribute__((always_inline))
static inline enum opt_tag
handle_short_opt (struct args *const args)
{
	extern bool
	nekropt_get_number_arg (struct nekroparser *parser,
	                        int64_t            *dest,
	                        int64_t             min,
	                        int64_t             max);
	extern bool
	nekropt_get_string_arg (struct nekroparser  *parser,
	                        char const         **dest);

next:
	switch (*args->p.p) {
		OPTIONS(parse_chr)
	default:
	invargs:
		args->p.e = EINVAL;
	}
	return OPT_NONE;
}

#undef string_chr_opt
#undef number_chr_opt
#undef boolean_chr_opt
#undef parse_chr

static inline struct args
nekropt_init (int    argc,
              char **argv)
{
	extern struct nekroparser
	nekroparser_init (int    argc,
	                  char **argv);

	struct args args = {
		.p = nekroparser_init(argc, argv)

		#define opt_var(T, tag, chr, str, doc, arg, ...) \
			__VA_OPT__(, .m_##tag = first(__VA_ARGS__,))
		OPTIONS(gen_opt_var)
		#undef opt_var
	};
	struct nekroparser *parser = &args.p;
	int options_end  = parser->c;

	while (!parser->e && ++parser->i < parser->c) {
		parser->p = parser->v[parser->i];

		if (parser->i >= options_end || *parser->p != '-') {
			parser->q[parser->n++] = parser->p;
			continue;
		}

		unsigned tag;

		++parser->p;
		switch (*parser->p) {
		case '-':
			++parser->p;
			if (!*parser->p) {
				options_end = parser->i + 1;
				continue;
			}

			tag = handle_long_opt(&args);
			if (!tag)
				break;

			args.seen[tag] = true;
			continue;

		default:
			tag = handle_short_opt(&args);
			if (!tag)
				break;

			args.seen[tag] = true;
			continue;
		}

		if (!parser->e)
			parser->e = EINVAL;
	}

	return args;
}

#undef gen_opt_var
#undef number_
#undef string_
#undef boolean_
#undef gen_arg_list
#undef arg3
#undef arg3_
#undef arg2
#undef arg2_
#undef arg1
#undef arg1_
#undef arg0

static inline int
nekropt_fini (struct args *args)
{
	int e = args->p.e;
	free(args->p.q);
	__builtin_memset(args, 0, sizeof *args);
	return e;
}

#define line_vars(chr, str, arg, help)   \
	const c_str("  -");              \
	const c_str(", --" str " " arg); \
	const space(str " " arg);        \
	const c_str(help);

#define line_init(chr, str, arg, help) \
	{ ' ', ' ', '-', chr },        \
	", --" str " " arg " ",        \
	space_init(str " " arg),       \
	help "\n",

#define help_text(x) ((const union { \
	const struct {               \
		x(line_vars)         \
		const char nul[1];   \
	};                           \
	const char str[1U + sizeof ( \
		struct {             \
			x(line_vars) \
		}                    \
	)];                          \
}){{                                 \
	x(line_init) ""              \
}}.str)

[[gnu::optimize("Os")]]
static inline void
usage (char const *const progname,
       char const *const synopsis)
{
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	struct bit_off   { };
	#pragma GCC diagnostic pop

	struct bit0x0_on { const char a[1U <<  0U]; };
	struct bit0x1_on { const char a[1U <<  1U]; };
	struct bit0x2_on { const char a[1U <<  2U]; };
	struct bit0x3_on { const char a[1U <<  3U]; };
	struct bit0x4_on { const char a[1U <<  4U]; };
	struct bit0x5_on { const char a[1U <<  5U]; };
	struct bit0x6_on { const char a[1U <<  6U]; };
	struct bit0x7_on { const char a[1U <<  7U]; };
	struct bit0x8_on { const char a[1U <<  8U]; };
	struct bit0x9_on { const char a[1U <<  9U]; };
	struct bit0xa_on { const char a[1U << 10U]; };
	struct bit0xb_on { const char a[1U << 11U]; };

	#define mk_arr(c, str, arg, ...) c_str(str " " arg " ");
	typedef char max_align[sizeof(union {NEKROPTIONS(mk_arr)})];
	#undef mk_arr

	(void)printf("Usage: %s %s\n\nOptions:\n%s", progname, synopsis,
	             help_text(NEKROPTIONS));
}

#ifdef __clang__
# pragma clang diagnostic pop
#endif // __clang__

#undef help_text
#undef line_init
#undef line_vars
#undef space_init
#undef space
#undef line_align
#undef c_str
#undef sym
#undef cat
#undef dog
#undef ws_init
#undef ws_type
#undef bit_type
#undef bit
#undef bit_
#undef ws0xb
#undef ws0xa
#undef ws0x9
#undef ws0x8
#undef ws0x7
#undef ws0x6
#undef ws0x5
#undef ws0x4
#undef ws0x3
#undef ws0x2
#undef ws0x1
#undef ws0x0
#undef string_arg
#undef number_arg
#undef generic_arg
#undef filter_args
#undef boolean_arg
#undef first
#undef eval
#undef call
#undef NEKROPTIONS

#endif // OPTIONS

#endif // NEKROPT_H_
