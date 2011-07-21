/*
** beeuniq - filter duplicate command line arguments
**
** Copyright (C) 2009-2011
**       Marius Tolzmann <tolzmann@molgen.mpg.de>
**       Tobias Dreyer <dreyer@molgen.mpg.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#define VERSION_MAJOR    0
#define VERSION_MINOR    1
#define VERSION_PATCHLVL 0

#define OPT_DELIMITER 'd'
#define OPT_VERSION   'v'
#define OPT_HELP      'h'

void print_version(void)
{
    printf("beeuniq v%d.%d.%d - "
           "by Marius Tolzmann <tolzmann@molgen.mpg.de> 2011\n",
             VERSION_MAJOR, VERSION_MINOR, VERSION_PATCHLVL);
}

void print_full_usage(void)
{
    printf("usage: beeuniq [options] <string>\n\n");
    printf("options:\n\n");
    printf("  -d | --delimiter <char>  specify the outputdelimiter character\n\n");
}

char in(char *search, char *strings[], int max)
{
   int i;

   for(i=0; i < max ; i++)
       if(!strcmp(search, strings[i]))
           return 1;

   return 0;
}

int bee_uniq(int count, char *strings[])
{
    int i = 0;
    int ret;

    for(i=1, ret=count; i < ret ; i++) {
        if(!in(strings[i], strings, i))
            continue;

        ret--;
        memmove(&strings[i], &strings[i+1], sizeof(*strings)*(ret-i));
        i--;
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int  option_index = 0;
    int  c = 0;
    int  max, i;

    char delimiter = ' ';

    struct option long_options[] = {
        {"delimiter",   required_argument, 0, OPT_DELIMITER},

        {"version",     no_argument, 0, OPT_VERSION},
        {"help",        no_argument, 0, OPT_HELP},

        {0, 0, 0, 0}
    };

    while ((c = getopt_long_only(argc, argv, "hv", long_options, &option_index)) != -1) {

        switch (c) {
            case OPT_DELIMITER:
                if (!optarg[0] || optarg[1]) {
                     fprintf(stderr, "invalid delimiter '%s'\n", optarg);
                     exit(EXIT_FAILURE);
                }
                delimiter = optarg[0];
                break;

            case OPT_HELP:
                print_version();
                printf("\n");
                print_full_usage();
                printf("\n");
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

    max  = bee_uniq(argc-optind, &argv[optind]);
    max += optind-1;

    for(i=optind; i <= max; i++) {
        fputs(argv[i], stdout);
        if(max-i)
            putchar(delimiter);
    }

    putchar('\n');

    return(EXIT_SUCCESS);
}
