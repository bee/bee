/*
** beegetopt - parse options
**
** Copyright (C) 2009-2012
**       Marius Tolzmann <tolzmann@molgen.mpg.de>
**       and other bee developers
**
** This file is part of bee.
**
** bee is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BEE_GETOPT_H
#define BEE_GETOPT_H 1

#define BEE_TYPE_STRING     1
#define BEE_TYPE_INTEGER    2
#define BEE_TYPE_FLOAT      3
#define BEE_TYPE_FLAG       4
#define BEE_TYPE_NO         5
#define BEE_TYPE_ENABLE     6
#define BEE_TYPE_WITH       7
#define BEE_TYPE_TOGGLE     8
#define BEE_TYPE_COUNT      9

#define BEE_FLAG_SKIPUNKNOWN   (1<<0)
#define BEE_FLAG_STOPONNOOPT   (1<<1)
#define BEE_FLAG_STOPONUNKNOWN (1<<2)
#define BEE_FLAG_KEEPOPTIONEND (1<<3)

#define BEE_OPT_LONG(name)     .long_opt      = (name)
#define BEE_OPT_SHORT(short)   .short_opt     = (short)
#define BEE_OPT_VALUE(v)       .value         = (v)
#define BEE_OPT_FLAG(f)        .flag          = (f)
#define BEE_OPT_TYPE(t)        .type          = (t)
#define BEE_OPT_OPTIONAL(args) .optional_args = (args)
#define BEE_OPT_REQUIRED(args) .required_args = (args)
#define _BEE_OPT_LEN(l)        ._long_len     = (l)

#define BEE_GETOPT_END        -1
#define BEE_GETOPT_ERROR      -2
#define BEE_GETOPT_NOVALUE    -3
#define BEE_GETOPT_NOOPT      -4
#define BEE_GETOPT_AMBIGUOUS  -5
#define BEE_GETOPT_OPTUNKNOWN -6
#define BEE_GETOPT_NOARG      -7
#define BEE_GETOPT_NOARGS     -8

#define BEE_OPTION_DEFAULTS \
            BEE_OPT_LONG(NULL), \
            BEE_OPT_SHORT(0), \
            BEE_OPT_VALUE(BEE_GETOPT_NOVALUE), \
            BEE_OPT_FLAG(NULL), \
            BEE_OPT_TYPE(BEE_TYPE_FLAG), \
            BEE_OPT_OPTIONAL(0), \
            BEE_OPT_REQUIRED(0), \
            _BEE_OPT_LEN(0)


#define BEE_INIT_OPTION_DEFAULTS(opt) \
            (opt)->long_opt  = NULL; \
            (opt)->short_opt = 0; \
            (opt)->value = BEE_GETOPT_NOVALUE; \
            (opt)->flag = NULL; \
            (opt)->type = BEE_TYPE_FLAG; \
            (opt)->optional_args = 0; \
            (opt)->required_args = 0; \
            (opt)->_long_len = 0


#define BEE_INIT_OPTION_END(opt) BEE_INIT_OPTION_DEFAULTS((opt))


#define BEE_OPTION_END { BEE_OPT_LONG(NULL), BEE_OPT_SHORT(0) }

#define BEE_OPTION(...) { BEE_OPTION_DEFAULTS, ## __VA_ARGS__ }

#define BEE_OPTION_NO_ARG(name, short, ...) \
            BEE_OPTION(BEE_OPT_LONG(name), \
                       BEE_OPT_SHORT(short), \
                       BEE_OPT_VALUE(short), \
                       ## __VA_ARGS__)

#define BEE_OPTION_REQUIRED_ARG(name, short, ...) \
            BEE_OPTION(BEE_OPT_LONG(name), \
                       BEE_OPT_SHORT(short), \
                       BEE_OPT_VALUE(short), \
                       BEE_OPT_TYPE(BEE_TYPE_STRING), \
                       BEE_OPT_REQUIRED(1), \
                       ## __VA_ARGS__ )

#define BEE_OPTION_REQUIRED_ARGS(name, short, n, ...) \
            BEE_OPTION(BEE_OPT_LONG(name), \
                       BEE_OPT_SHORT(short), \
                       BEE_OPT_VALUE(short), \
                       BEE_OPT_TYPE(BEE_TYPE_STRING), \
                       BEE_OPT_REQUIRED(n), \
                       ## __VA_ARGS__ )

#define BEE_OPTION_OPTIONAL_ARG(name, short, ...) \
            BEE_OPTION(BEE_OPT_LONG(name), \
                       BEE_OPT_SHORT(short), \
                       BEE_OPT_VALUE(short), \
                       BEE_OPT_TYPE(BEE_TYPE_STRING), \
                       BEE_OPT_OPTIONAL(1), \
                       ## __VA_ARGS__ )

#define BEE_OPTION_OPTIONAL_ARGS(name, short, n, ...) \
            BEE_OPTION(BEE_OPT_LONG(name), \
                       BEE_OPT_SHORT(short), \
                       BEE_OPT_VALUE(short), \
                       BEE_OPT_TYPE(BEE_TYPE_STRING), \
                       BEE_OPT_OPTIONAL(n), \
                       ## __VA_ARGS__ )

#define BEE_OPTION_ARGS(name, short, opt, req, ...) \
            BEE_OPTION(BEE_OPT_LONG(name), \
                       BEE_OPT_SHORT(short), \
                       BEE_OPT_VALUE(short), \
                       BEE_OPT_TYPE(BEE_TYPE_STRING), \
                       BEE_OPT_OPTIONAL(opt), \
                       BEE_OPT_REQUIRED(req), \
                       ## __VA_ARGS__ )

struct bee_option {
    char *long_opt;
    char  short_opt;

    int  value;
    int *flag;

    int  type;

    int  optional_args;
    int  required_args;

    int  _long_len;
};

struct bee_getopt_ctl {
    int    optind;

    char  *program;

    char  *optarg;

    int    optargc;
    char **optargv;

    int    argc;
    char **argv;

    struct bee_option *options;

    int   _argc;
    int   _optcnt;
    char *_unhandled_shortopts;

    int flags;
};

void bee_getopt_pop_current_argument(struct bee_getopt_ctl *optctl);

int bee_getopt_init(struct bee_getopt_ctl *ctl, int argc, char **argv, struct bee_option *optv);
int bee_getopt_long(struct bee_getopt_ctl *optctl, int *optindex);
int bee_getopt(struct bee_getopt_ctl *optctl, int *optindex);

void bee_getopt_print_quoted(char *s);

#endif
