/*
** beegetopt - parse options
**
** Copyright (C) 2009-2011
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

/*
 what we do support:

   --r=256  long  = "r" value = "256"
   --r 256  long  = "r" value = "256"

   -r=255   short = 'r' value = "=255"
   -r255    short = 'r' value = "255"
   -r 255   short = 'r' value = "255"

 what we do not support

    -help  long = "help"
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "bee_getopt.h"

int bee_getopt_init(struct bee_getopt_ctl *ctl, int argc, char **argv, struct bee_option *optv)
{
    int err = 0;

    ctl->optind  = 0;

    ctl->optarg  = NULL;

    ctl->argc    = argc;
    ctl->argv    = argv;

    ctl->program = NULL;

/*    for (i=0; i < argc; i++) {
       printf("XXX argv[%d] = %s\n", i, argv[i]);
    }
*/
    ctl->options = optv;

    for(ctl->_optcnt=0; optv[ctl->_optcnt].long_opt || optv[ctl->_optcnt].short_opt; ctl->_optcnt++) {
        if(optv[ctl->_optcnt].long_opt)
            optv[ctl->_optcnt]._long_len = strlen(optv[ctl->_optcnt].long_opt);
        if(optv[ctl->_optcnt].value < 0 && optv[ctl->_optcnt].value != BEE_GETOPT_NOVALUE) {
            fprintf(stderr, "*** bee_getopt_init() ERROR: negative value (%d) set for opt %d (long: %s, short: %c)\n", ctl->_optcnt, optv[ctl->_optcnt].value, optv[ctl->_optcnt].long_opt, optv[ctl->_optcnt].short_opt);
            err++;
        }
    }

    ctl->_argc  = argc;
    ctl->_unhandled_shortopts = NULL;

    ctl->flags = 0;

    return err?0:1;
}

static int find_long_option_by_name(struct bee_option *options, char *name, char **optarg)
{
    int i = 0;

    for(i=0; options[i].long_opt || options[i].short_opt; i++) {
        /* skip short-only-opts */
        if(!options[i].long_opt)
            continue;

        /* check if option starts with full long-name */
        if(strncmp(name, options[i].long_opt, options[i]._long_len) != 0)
            continue;

        /* if --long-name matches completly we are done */
        if(name[options[i]._long_len] == '\0')
            return i;

        /* skip option candidates without allowed argument */
        if (!options[i].optional_args && !options[i].required_args)
            continue;

        /* if --long-name= is not specified we don't have a full match at all */
        if(name[options[i]._long_len] != '=')
            continue;

        /* we matched --long-name= so set optarg and return success */
        if (optarg)
            *optarg = &(name[options[i]._long_len+1]);

        return i;
    }

    /* not match found */
    return BEE_GETOPT_OPTUNKNOWN;
}

static int find_short_option(struct bee_option *options, char **name, char **optarg)
{
   char short_opt;
   int i;
   int idx;

   short_opt = *name[0];

   assert(short_opt);

   if (optarg)
       *optarg = NULL;

   for (i=0, idx=-1; options[i].long_opt || options[i].short_opt; i++) {

        /* skip long-only-opts */
        if (!options[i].short_opt)
            continue;

        if (options[i].short_opt == short_opt)
            idx = i;
   }

    /* return error if option was not found */
    if (idx == -1) {
        /* reset unhandled options */
        *name = NULL;
        return BEE_GETOPT_OPTUNKNOWN;
    }

    /* skip this short option */
    (*name)++;

    if (!*name[0])
       *name = NULL;

    /* set optarg if option takes arguments and short_opt is not the last char */
    if (*name && (options[idx].required_args || options[idx].optional_args)) {
       *optarg = *name;
       *name   = NULL;
    }

    return idx;
}

static int find_long_option_by_subname(struct bee_option *options, char *name, char **optarg)
{
    int   i;
    int   l;
    char *s;
    int   idx;

    l = strlen(name);
    s = strchr(name, '=');

    if (l < 1)
       return -2;

    for (i=0, idx=-1; options[i].long_opt || options[i].short_opt; i++) {

        /* skip short-only-opts */
        if (!options[i].long_opt)
            continue;

        if (strncmp(name, options[i].long_opt, l) != 0) {
            /* skip if search string has no optarg */
            if(!s)
                continue;

            /* skip option candidates without allowed argument */
            if (!options[i].optional_args && !options[i].required_args)
                continue;

            /* skip if string without optarg does not match */
            if (strncmp(name, options[i].long_opt, s - name) != 0)
                continue;
        }

        if (idx >= 0) {
            if (options[i].value != options[idx].value)
                return BEE_GETOPT_AMBIGUOUS;
        } else {
            idx = i;
        }
    }

    /* return error if option was not found */
    if (idx == -1)
        return BEE_GETOPT_OPTUNKNOWN;

    /* set optarg if assigned */
    if (s && optarg)
        *optarg = s+1;

    return idx;
}

static int handle_option(struct bee_getopt_ctl *ctl, const int index)
{
    struct bee_option *o = &(ctl->options[index]);
    int argc;

    ctl->optargc = 0;

    /* skip the option */
    if (!ctl->_unhandled_shortopts)
        ctl->optind++;

    if (o->required_args) {
        /* we don't check required args for possible option matches */

        /* argument is required - optarg already set? */
        if (!ctl->optarg) {
            /* no optarg and no more arguments -> required argument missing */
            if (ctl->optind == ctl->_argc)
                return (o->required_args == 1)?BEE_GETOPT_NOARG:BEE_GETOPT_NOARGS;

            /* set optarg to this argument and skip it */
            ctl->optarg = ctl->argv[ctl->optind++];
        }

        /* if only one argument is required we can return here */
        if (o->required_args == 1)
            return o->value;

        /* more arguments are required .. check if they are available */

        argc = o->required_args-1;

        if (ctl->optind+argc > ctl->_argc) {
            /* on error: set optargc to number of args available */
            ctl->optargc = ctl->optind - ctl->_argc;
            return BEE_GETOPT_NOARGS;
        }

        /* all required args are available */
        /* first req argument is optarg */
        /* all other required arguments are in ctl->optargv[0 .. ctl->optargc-1] */
        ctl->optargc  = argc;
        ctl->optargv  = &(ctl->argv[ctl->optind]);

        /* skip all required args */
        ctl->optind  += argc;
    }

    if (o->optional_args) {
       /* optional arguments may not start with '-' */

       if (!o->required_args) {

           /* only optional argumentsBEE_FLAG_STOPONNOOPT - set optarg if not set */
           if (!ctl->optarg) {
               /* no optarg and no more arguments -> done without optional argument */
               if (ctl->optind == ctl->_argc)
                   return o->value;

               /* we are done if this argument looks like an option */
               if (ctl->argv[ctl->optind][0] == '-')
                   return o->value;

               /* set optarg to this argument and skip it */
               ctl->optarg = ctl->argv[ctl->optind++];
           }

           /* if only one optional argument is allowed we can return here */
           if (o->optional_args == 1)
               return o->value;
       }

       for (argc = 0; argc < o->optional_args-((o->required_args)?0:1) && ctl->optind < ctl->_argc; argc++) {
           /* stop on potential option */
           if (ctl->argv[ctl->optind][0] == '-')
              return o->value;

           ctl->optargc++;
           ctl->optind++;
       }
    }

    return o->value;
}

void bee_getopt_pop_current_argument(struct bee_getopt_ctl *optctl)
{
    int i;

    optctl->argv[optctl->argc] = optctl->argv[optctl->optind];

//    fprintf(stderr, "popping %d %s\n", optctl->optind, optctl->argv[optctl->optind]);

    for(i=optctl->optind; i < optctl->argc; i++) {
        optctl->argv[i] = optctl->argv[i+1];
    }

    optctl->argv[optctl->argc] = NULL;

    optctl->_argc--;
}

void bee_getopt_pop_all_arguments(struct bee_getopt_ctl *optctl)
{
    while(optctl->optind < optctl->_argc)
        bee_getopt_pop_current_argument(optctl);
}

static int _bee_getopt_long(struct bee_getopt_ctl *optctl, int *optindex)
{
    int this;

    short maybe_option = 0;
    short maybe_long   = 0;
    short maybe_short  = 0;
    char *name;
    int idx;

    if(optctl->optind == optctl->_argc)
        return BEE_GETOPT_END;

    assert(optctl->optind < optctl->_argc);

    this = optctl->optind;

    maybe_option = (optctl->argv[this][0] == '-');
    maybe_long   = maybe_option && (optctl->argv[this][1] == '-');
    maybe_short  = maybe_option && !maybe_long;
    name         = &(optctl->argv[this][maybe_option+maybe_long]);

    optctl->optarg = NULL;

    *optindex = -1;

    /* not an option: pop arguement */

    if(!maybe_option) {
        bee_getopt_pop_current_argument(optctl);
        return BEE_GETOPT_NOOPT;
    }

    /* match & skip '--' and pop all remaining arguments */

    if(maybe_long && (optctl->argv[this][2] == '\0')) {
        if(!(optctl->flags & BEE_FLAG_KEEPOPTIONEND))
            optctl->optind++;
        bee_getopt_pop_all_arguments(optctl);
        return BEE_GETOPT_END;
    }

    /* match long option */

    if (maybe_long) {
        idx = find_long_option_by_name(optctl->options, name, &optctl->optarg);

        if (idx < 0)
            idx = find_long_option_by_subname(optctl->options, name, &optctl->optarg);

        if (idx < 0)
            return idx;

        *optindex = idx;

        return handle_option(optctl, idx);
    }

    /* match short option */

    if (!optctl->_unhandled_shortopts)
        optctl->_unhandled_shortopts = name;

    idx = find_short_option(optctl->options, &optctl->_unhandled_shortopts, &optctl->optarg);

    if (idx < 0)
        return idx;

    *optindex = idx;

    return handle_option(optctl, idx);
}

int bee_getopt_long(struct bee_getopt_ctl *optctl, int *optindex)
{
    return _bee_getopt_long(optctl, optindex);
}

int bee_getopt(struct bee_getopt_ctl *optctl, int *optindex)
{
    int opt;

    while((opt = _bee_getopt_long(optctl, optindex)) != BEE_GETOPT_END) {
        switch(opt) {
            case BEE_GETOPT_NOVALUE:
                return BEE_GETOPT_NOVALUE;

            case BEE_GETOPT_NOOPT:
                if (optctl->flags & BEE_FLAG_STOPONNOOPT) {
                    bee_getopt_pop_all_arguments(optctl);
                    return BEE_GETOPT_END;
                }
                break;

            case BEE_GETOPT_AMBIGUOUS:
                if (optctl->program)
                    fprintf(stderr, "%s: ", optctl->program);
                fprintf(stderr, "option '%s' is ambiguous.\n", optctl->argv[optctl->optind]);
                return BEE_GETOPT_ERROR;

            case BEE_GETOPT_OPTUNKNOWN:
                if (optctl->flags & BEE_FLAG_STOPONUNKNOWN) {
                    bee_getopt_pop_all_arguments(optctl);
                    return BEE_GETOPT_END;
                }
                if (!(optctl->flags & BEE_FLAG_SKIPUNKNOWN)) {
                    if (optctl->program)
                        fprintf(stderr, "%s: ", optctl->program);
                    fprintf(stderr, "unrecognized option '%s'.\n", optctl->argv[optctl->optind]);
                    return BEE_GETOPT_ERROR;
                }
                bee_getopt_pop_current_argument(optctl);
                break;

            case BEE_GETOPT_NOARG:
            case BEE_GETOPT_NOARGS:
                if (optctl->program)
                    fprintf(stderr, "%s: ", optctl->program);
                if (optctl->options[*optindex].long_opt)
                    fprintf(stderr, "option '--%s' requires an argument.\n", optctl->options[*optindex].long_opt);
                else
                    fprintf(stderr, "option '-%c' requires an argument.\n", optctl->options[*optindex].short_opt);
                return BEE_GETOPT_ERROR;

            default:
                assert(opt >= 0);
                return opt;
        }
    }

    return opt;
}

void bee_getopt_print_quoted(char *s)
{
    putchar('\'');
    while (*s) {
       if (*s == '\'')
           printf("'\\''");
       else
           putchar(*s);
       s++;
    }
    putchar('\'');
}
