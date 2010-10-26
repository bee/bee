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
#define BEECUT_MINOR    1
#define BEECUT_PATCHLVL 0

#define OPT_DELIMETER 'd'
#define OPT_SHORT     's'
#define OPT_VERSION   128
#define OPT_HELP      129

void print_version(void) 
{
    printf("beecut v%d.%d.%d - "
           "by Marius Tolzmann <tolzmann@molgen.mpg.de> 2010\n", 
             BEECUT_MAJOR, BEECUT_MINOR, BEECUT_PATCHLVL);
}

void print_full_usage(void) 
{
    printf("usage: beecut [--delimeter=<delimchar>] [--short] <string>\n\n");
    
    printf("examples:\n\n");
    
    printf("  beecut 2.4.23.0        will print '2.4.23.0 2 2.4 2.4.23 2.4.23.0'\n");
    printf("  beecut -s 2.4.23.0     will print '2.4.23.0 2 4 23 0'\n");
    printf("  beecut -s -d '-' a-b-c will print 'a-b-c a b c'\n\n");
}

void cut_and_print(char *string, char delimeter, char opt_short)
{
    char *p, *s;
    
    p = s = string;
    
    printf("%s", string);
    
    while((p=strchr(p, delimeter))) {
        putchar(' ');
        
        while(s < p)
            putchar(*(s++));
        
        p++;
        
        s = (opt_short) ? p : string;
    }
    
    printf(" %s", s);
}

int main(int argc, char *argv[])
{
    int  option_index = 0;
    int  c = 0;
    
    char delimeter = '.';
    
    char opt_short = 0;
    
    struct option long_options[] = {
        {"delimeter",   required_argument, 0, 'd'},
        
        {"short",       no_argument, 0, 's'},
        
        {"version",     no_argument, 0, OPT_VERSION},
        {"help",        no_argument, 0, OPT_HELP},
        
        {0, 0, 0, 0}
    };
    
    while ((c = getopt_long_only(argc, argv, "d:s", long_options, &option_index)) != -1) {
        
        if(c == OPT_DELIMETER) {
            delimeter = optarg[0];
            continue;
        }
        
        if(c == OPT_SHORT) {
            opt_short = 1;
            continue;
        }
        
        if(c == OPT_HELP) {
            printf("\n");
            print_version();
            printf("\n");
            print_full_usage();
            exit(0);
        }
        
        if(c == OPT_VERSION) {
            print_version();
            exit(0);
        }
        
        if(opterr)
           continue;
        
        fprintf(stderr, "YOU HIT A BUG #001 opterr=%d c='%c'\n", opterr, delimeter);
    }  /* end while getopt_long_only */
    
    if(argc-optind < 1) {
        print_full_usage();
        exit(1);
    }
    
    cut_and_print(argv[optind], delimeter, opt_short);
    
    printf("\n");
    
    return(0);
}
