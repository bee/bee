/*
** beecut - split strings 
** Copyright (C) 2010 
**       Marius Tolzmann <tolzmann@molgen.mpg.de>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>

#define BEECUT_MAJOR    0
#define BEECUT_MINOR    4
#define BEECUT_PATCHLVL 0

#define OPT_DELIMETER 'd'
#define OPT_SHORT     's'
#define OPT_PREPEND   'p'
#define OPT_APPEND    'a'
#define OPT_NEWLINE   'n'
#define OPT_VERSION   128
#define OPT_HELP      129

void print_version(void) 
{
    printf("beecut v%d.%d.%d - "
           "by Marius Tolzmann <tolzmann@molgen.mpg.de> 2010-2011\n",
             BEECUT_MAJOR, BEECUT_MINOR, BEECUT_PATCHLVL);
}

void print_full_usage(void) 
{
    printf("usage: beecut [options] <string>\n\n");

    printf("options:\n\n");

    printf("  -d | --delimeter <char>  specify the delimeter character\n\n");

    printf("  -s | --short             output short elements\n");
    printf("  -n | --newline           output each element on a seperate line\n\n");

    printf("  -p | --prepend <string>  prepend <string> to each output element\n");
    printf("  -a | --append <string>   append  <string> to each output element\n\n");

    printf("examples:\n\n");

    printf("  beecut 2.4.23.0        will print '2.4.23.0 2 2.4 2.4.23 2.4.23.0'\n");
    printf("  beecut -s 2.4.23.0     will print '2.4.23.0 2 4 23 0'\n");
    printf("  beecut -s -d '-' a-b-c will print 'a-b-c a b c'\n\n");
}

void cut_and_print(char *string, char delimeter, char opt_short, char opt_newline, char *prefix, char *suffix)
{
    char *p, *s;
    char nl;

    nl = opt_newline ? '\n' : ' ';
    
    p = s = string;
    
    printf("%s%s", prefix, string);
    
    while((p=strchr(p, delimeter))) {
        if(s < p) /* only print space if we have something to print */
            printf("%s%c%s", suffix, nl, prefix);

        while(s < p)
            putchar(*(s++));
        
        p++;
        
        s = (opt_short) ? p : string;
    }
    
    printf("%s%c%s%s%s\n", suffix, nl, prefix, s, suffix);
}

int main(int argc, char *argv[])
{
    int  option_index = 0;
    int  c = 0;
    
    char delimeter = '.';
    
    char opt_short   = 0;
    char opt_newline = 0;
    
    char *opt_prepend = "";
    char *opt_append  = opt_prepend;

    struct option long_options[] = {
        {"delimeter",   required_argument, 0, OPT_DELIMETER},

        {"prepend",     required_argument, 0, OPT_PREPEND},
        {"append",      required_argument, 0, OPT_APPEND},

        {"short",       no_argument, 0, OPT_SHORT},
        {"newline",     no_argument, 0, OPT_NEWLINE},
        
        {"version",     no_argument, 0, OPT_VERSION},
        {"help",        no_argument, 0, OPT_HELP},
        
        {0, 0, 0, 0}
    };
    
    while ((c = getopt_long_only(argc, argv, "p:a:d:sn", long_options, &option_index)) != -1) {

        switch (c) {
            case OPT_DELIMETER:
                if (!optarg[0] || optarg[1]) {
                     fprintf(stderr, "invalid delimeter '%s'\n", optarg);
                     exit(EXIT_FAILURE);
                }
                delimeter = optarg[0];
                break;

            case OPT_PREPEND:
                opt_prepend = strdup(optarg);
                break;

            case OPT_APPEND:
                opt_append = strdup(optarg);
                break;

            case OPT_SHORT:
                opt_short = 1;
                break;

            case OPT_NEWLINE:
                opt_newline = 1;
                break;

            case OPT_HELP:
                printf("\n");
                print_version();
                printf("\n");
                print_full_usage();
                exit(EXIT_SUCCESS);

            case OPT_VERSION:
                print_version();
                exit(EXIT_SUCCESS);
        }
    }  /* end while getopt_long_only */
    
    if(argc-optind < 1) {
        print_full_usage();
        exit(EXIT_FAILURE);
    }
    
    while(optind < argc)
        cut_and_print(argv[optind++], delimeter, opt_short,
                      opt_newline, opt_prepend, opt_append);
    
    return(0);
}
