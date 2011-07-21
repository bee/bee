/*
** beeversion - compare bee package versionnumbers
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

#include "beeversion.h"
#include "parse.h"
#include "compare.h"


#define TEST_BITS 3
#define TYPE_BITS 2

#define USED_BITS (TEST_BITS+TYPE_BITS)

#define TEST_TYPE_MASK           (((1<<TYPE_BITS)-1)<<TEST_BITS)

#define TEST_MASK                ((1<<TEST_BITS)-1)

#define TEST_FULL_MASK           (TEST_TYPE_MASK|TEST_MASK)

#define TEST_WITH_2_ARGS         (1<<TEST_BITS)
#define TEST_WITH_1_OR_MORE_ARGS (2<<TEST_BITS)
#define TEST_WITH_2_OR_MORE_ARGS (3<<TEST_BITS)

#define T_LESS_THAN     0
#define T_LESS_EQUAL    1
#define T_GREATER_THAN  2
#define T_GREATER_EQUAL 3
#define T_EQUAL         4
#define T_NOT_EQUAL     5

#define T_IS_MAX     0
#define T_IS_MIN     1
#define T_IS_NOT_MAX 2
#define T_IS_NOT_MIN 3

#define T_MAX 0
#define T_MIN 1

#define OPT_FORMAT   128
#define OPT_KEYVALUE 129
#define OPT_VERSION  130
#define OPT_HELP     131
#define OPT_FILTER_PKGFULLNAME 132

#define MODE_TEST   1
#define MODE_PARSE  2

char *filter_pkgfullname;

int compare_beeversions(struct beeversion *, struct beeversion *);
char parse_extra(struct beeversion *);

void print_version(void) {
    printf("beeversion v%d.%d.%d - "
           "by Marius Tolzmann <tolzmann@molgen.mpg.de> 2010\n", 
           BEEVERSION_MAJOR, BEEVERSION_MINOR, BEEVERSION_PATCHLVL);
}

void print_full_usage(void) {

    printf("usage:\n\n");
    
    
    printf("   test: beeversion <packageA> -{lt|le|gt|ge|eq|ne} <packageB>\n");
    printf(" filter: beeversion [filter-options] -{min|max} <package1> [.. <packageN>]\n");
    printf("  parse: beeversion [parse-options] <package>\n\n");
    
    printf("         package := <pkgfullname>-<pkgfullversion>-<pkgrevision>\n");
    printf("                  | <pkgfullname>-<pkgfullversion>\n");
    printf("                  | <pkgfullversion>\n\n");
    
    printf("     pkgfullname := <pkgname>\n");
    printf("                  | <pkgname>_<pkgsubname>\n\n");
    
    printf("  pkgfullversion := <pkgversion>\n");
    printf("                  | <pkgversion>_<pkgextraversion>\n\n");
    
    printf("     pkgrevision := <pkgrevision>\n");
    printf("                  | <pkgrevision>.<arch>\n\n");
    
    printf("   filter-options:\n\n");

    printf("      --filter-pkgfullname=<pkgfullname>\n\n");
    
}

void cut_and_print(char *string, char delimiter, char opt_short)
{
    char *p, *s;
    
    p = s = string;
    
    printf("%s", string);
    
    while((p=strchr(p, delimiter))) {
        putchar(' ');
        
        while(s < p)
            putchar(*(s++));
        
        p++;
        
        s = (opt_short) ? p : string;
    }
    
    printf(" %s", s);
}


int parse_argument(char* text, struct beeversion *versionsnummer)
{	
    int p;
    
    if((p=parse_version(text, versionsnummer))) {
        fprintf(stderr, "syntax error at position %d in '%s'\n", p, text);
        return(0);
    }
    return(1);
}


static int compare_beepackages_gen(const void *a, const void *b) {
    return((int)compare_beepackages((struct beeversion *)a, (struct beeversion *)b));
}

void print_format(char* s, struct beeversion *v)
{
    char *p;
    size_t len;
    
    p=s;
    
    if(filter_pkgfullname) {
    
       len = strlen(v->pkgname);
       
       if(len > strlen(filter_pkgfullname))
           return;
       
       if(strncmp(v->pkgname, filter_pkgfullname, len))
           return;
       
       p = filter_pkgfullname+len;
       
       if((!*p && *(v->subname)) || (*p && *p++ != '_'))
           return;
       
       if(strcmp(p, v->subname))
           return;
    }
    
    for(p=s; *p; p++) {
        if(*p == '%') {
            switch(*(++p)) {
                case '%':
                    printf("%%");
                    break;
                case 'p':
                    printf("%s", v->pkgname);
                    break;
                case 's':
                    if(*(v->suffix))
                        printf(".%s", v->suffix);
                    break;
                case 'x':
                    printf("%s", v->subname);
                    break;
                case 'v':
                    printf("%s", v->version);
                    break;
                case 'e':
                    printf("%s", v->extraversion);
                    break;
                case 'r':
                    printf("%s", v->pkgrevision);
                    break;
                case 'a':
                    printf("%s", v->arch);
                    break;
                case 'P':
                    printf("%s", v->pkgname);
                    if(*(v->subname))
                        printf("_%s", v->subname);
                    break;
                case 'V':
                    printf("%s", v->version);
                    if(*(v->extraversion))
                        printf("_%s", v->extraversion);
                    break;
                case 'F':
                case 'A':
                    if(*(v->pkgname)) {
                        printf("%s", v->pkgname);
                        if(*(v->subname))
                            printf("_%s", v->subname);
                        printf("-");
                    }
                    
                    printf("%s", v->version);
                    if(*(v->extraversion))
                        printf("_%s", v->extraversion);
                        
                    if(*(v->pkgrevision)) {
                        printf("-%s", v->pkgrevision);
                        if(*p == 'A' && *(v->arch))
                            printf(".%s", v->arch);
                    }
                    break;
            }
            continue;
        } /* if '%' */
        
        if(*p == '@') {
            switch(*(++p)) {
                case 'v':
                    cut_and_print(v->version, '.', 0);
                    break;
            }
            continue;
        } /* if '@' */
        
        if(*p == '\\') {
            switch(*(++p)) {
                case 'n':
                    printf("\n");
                    break;
                case 't':
                    printf("\t");
                    break;
                    
            } 
            continue;
        } /* if '\' */
        
        printf("%c", *p);
        
    } /* for *p */
}


int do_test(int argc, char *argv[], char test) {
    int i;
    
    struct beeversion v[2];
    struct beeversion *a, *b, *va;
    
    int ret;
    char t;
    
    a = &v[0];
    b = &v[1];
    
    t = (test & TEST_MASK);
    
    if((test & TEST_TYPE_MASK) == TEST_WITH_2_ARGS) {
        if(argc != 2) {
            fprintf(stderr, "usage: beeversion <packageA> -[lt|le|gt|ge|eq|ne] <packageB>\n");
            return(255);
        }
        
        for(i=0; i<2; i++) {
            if(!parse_argument(argv[i], &v[i]))
               return(0);
        }
        
        ret = compare_beeversions(a, b);

        free(a->string);
        free(b->string);

        switch(t) {
            case T_LESS_THAN:
                return(ret < 0);
            case T_LESS_EQUAL:
                return(ret <= 0);
            case T_GREATER_THAN:
                return(ret > 0);
            case T_GREATER_EQUAL:
                return(ret >= 0);
            case T_EQUAL:
                return(ret == 0);
            case T_NOT_EQUAL:
                return(ret != 0);
        }
        fprintf(stderr, "YOU HIT A BUG #004\n");
    }
    
    /* min / max */
    if((test & TEST_TYPE_MASK) == TEST_WITH_2_OR_MORE_ARGS) {
        
        if(argc < 1) {
            fprintf(stderr, "usage: beeversion -[min|max] <package1> [<package2> .. <packageN>]\n");
            return(255);
        }
        
        if(!(va = calloc(sizeof(struct beeversion), argc))) {
            perror("va=calloc()");
            exit(255);
        }
        
        for(i=0;i<argc;i++) {
            if(!parse_argument(argv[i], va+i))
                return(0);
        }
        
        qsort(va, argc, sizeof(struct beeversion), compare_beepackages_gen);
        
        for(a=va,i=1;i<argc;i++) {
            b=va+i;
            
            /* a != b */
            if(compare_beepackage_names(a, b)) {
                print_format("%A\n", a);
                a = b;
            }
            
            if(t == T_MAX) 
               a = b;
        }
        print_format("%A\n", a);

        for(i=0;i<argc;i++) {
            free(va[i].string);
        }

        free(va);
        return(1);
    }
    
    fprintf(stderr, "YOU HIT A BUG #006\n");
    
    return(0);
}

int do_parse(int argc, char *argv[], char *format) {
    struct beeversion v;
    
    if(argc != 1) {
        fprintf(stderr, "usage: beeversion <package>\n"); 
        return(255);
    }
    
    if(!parse_argument(argv[0], &v))
        return(0);
    
    print_format(format, &v);

    free(v.string);

    return(1);
}

int main(int argc, char *argv[])
{
    int option_index = 0;
    int c = 0;
        
    char test_to_do   = 0;
    char *format      = NULL;
    int  test_index   = 0;
    int  build_format = 0;
    char mode         = 0;
    
    char *keyvalue;
    
    keyvalue = "PKGNAME=%p\n"
               "PKGEXTRANAME=%x\n"
               "PKGVERSION=( @v )\n"
               "PKGEXTRAVERSION=%e\n"
               "PKGREVISION=%r\n"
               "PKGARCH=%a\n"
               "PKGFULLNAME=%P\n"
               "PKGFULLVERSION=%V\n"
               "PKGFULLPKG=%F\n"
               "PKGALLPKG=%A\n"
               "PKGSUFFIX=%s\n";

    struct option long_options[] = {
        /* tests  with 2 args */
        {"lt",    no_argument, 0, TEST_WITH_2_ARGS|T_LESS_THAN},
        {"le",    no_argument, 0, TEST_WITH_2_ARGS|T_LESS_EQUAL},
        {"gt",    no_argument, 0, TEST_WITH_2_ARGS|T_GREATER_THAN},
        {"ge",    no_argument, 0, TEST_WITH_2_ARGS|T_GREATER_EQUAL},
        {"eq",    no_argument, 0, TEST_WITH_2_ARGS|T_EQUAL},
        {"ne",    no_argument, 0, TEST_WITH_2_ARGS|T_NOT_EQUAL},

        /* tests with optarg and 1 or more args */
        /*
        {"ismax",    required_argument, 0, TEST_WITH_1_OR_MORE_ARGS|T_IS_MAX},
        {"ismin",    required_argument, 0, TEST_WITH_1_OR_MORE_ARGS|T_IS_MIN},
        {"isnotmax", required_argument, 0, TEST_WITH_1_OR_MORE_ARGS|T_IS_NOT_MAX},
        {"isnotmin", required_argument, 0, TEST_WITH_1_OR_MORE_ARGS|T_IS_NOT_MIN},
        */
        /* filter with 2 or more args.. */
        {"max",   no_argument, 0, TEST_WITH_2_OR_MORE_ARGS|T_MAX},
        {"min",   no_argument, 0, TEST_WITH_2_OR_MORE_ARGS|T_MIN},

        /* normal parse mode */
        {"format",   required_argument, 0, OPT_FORMAT},
        /*
        {"keyvalue",       no_argument, 0, OPT_KEYVALUE},
        */
        
        /*  */
        {"pkgfullname",    no_argument,  0, 'P'},
        {"pkgfullversion", no_argument,  0, 'V'},
        {"pkgfullpkg",     no_argument,  0, 'F'},
        {"pkgallpkg",      no_argument,  0, 'A'},

        {"pkgname",         no_argument, 0, 'p'},
        {"pkgarch",         no_argument, 0, 'a'},
        {"pkgversion",      no_argument, 0, 'v'},
        {"pkgextraversion", no_argument, 0, 'e'},
        {"pkgrevision",     no_argument, 0, 'r'},
        {"pkgsuffix",       no_argument, 0, 's'},
        
        {"pkgextraname",    no_argument, 0, 'x'},
        {"pkgsubname",      no_argument, 0, 'x'},
        
        {"filter-pkgfullname", required_argument, 0, OPT_FILTER_PKGFULLNAME},
        
        {"version",     no_argument, 0, OPT_VERSION},
        {"help",        no_argument, 0, OPT_HELP},

        {0, 0, 0, 0}
    };
    
    while ((c = getopt_long_only(argc, argv, "PAVFpaversx", long_options, &option_index)) != -1) {
    
        if( (c & TEST_TYPE_MASK) && ! (c & ~TEST_FULL_MASK)) {
            if(mode && mode == MODE_PARSE) {
                fprintf(stderr, "skipping test-option --%s since already running in parse mode\n",
                          long_options[option_index].name);
                continue;
            }
            if(test_to_do) {
                fprintf(stderr, "skipping test-option --%s since --%s is already set\n",
                          long_options[option_index].name, long_options[test_index].name);
                continue;
            }
            mode       = MODE_TEST;
            test_index = option_index;
            test_to_do = c;
            continue;
        }
        
        if(c == OPT_FILTER_PKGFULLNAME) {
            filter_pkgfullname = optarg;
            continue;
        }
        
        if(mode && mode == MODE_TEST) {
            fprintf(stderr, "skipping parse-option --%s since already running in test mode\n",
                      long_options[option_index].name);
            continue;
        }
        
        mode = MODE_PARSE;
        
        /* define format */
        if((c >= 'A' && c <= 'z')) {
            if(format && ! build_format) {
                fprintf(stderr, "--%s ignored\n", long_options[option_index].name);
                continue;
            }
            
            if(!format) {
                format = calloc(sizeof(char), argc * 3 + 2);
                if(!format) {
                    perror("calloc(format)");
                    exit(255);
                }
            }
            
            if(build_format)
                format[build_format++] = ' ';
                
            format[build_format++] = '%';
            format[build_format++] = c;
            continue;
        }
        
        if(c == OPT_FORMAT) {
            if(format) {
                fprintf(stderr, "--%s ignored\n", long_options[option_index].name);
                continue;
            }
            format = optarg;
            continue;
        }
        
        if(c == OPT_KEYVALUE) {
            if(format) {
                fprintf(stderr, "--%s ignored\n", long_options[option_index].name);
                continue;
            }
            format = keyvalue;
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
        
        fprintf(stderr, "YOU HIT A BUG #003 opterr=%d\n", opterr);
    }  /* end while getopt_long_only */
    
    if(build_format)
        format[build_format++] = '\n';
    
    if(mode == MODE_TEST) 
        return(!do_test(argc-optind, argv+optind, test_to_do));
    
    if(!mode || mode == MODE_PARSE) {
        if(!format)
            format = keyvalue;
        
        filter_pkgfullname = NULL;
        
        return(!do_parse(argc-optind, argv+optind, format));
    }
        
    return(0);
}
