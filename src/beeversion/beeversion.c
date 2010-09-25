/*
    beeversion - compare bee package versionnumbers
    Copyright (C) 2010  David Fessler <dfessler@uni-potsdam.de>
                        Marius Tolzmann <tolzmann@molgen.mpg.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>


#define EXTRA_UNKNOWN 200
#define EXTRA_ALPHA   1
#define EXTRA_BETA    2
#define EXTRA_RC      3
#define EXTRA_NONE    4
#define EXTRA_PATCH   5
#define EXTRA_ANY     6

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

#define MODE_TEST   1
#define MODE_PARSE  2

struct extra_version {
    char         *string;
    unsigned int  priority;
    size_t        length;
};

struct nr {
    char *string;
    char *pkgname;
    char *subname;
    char *version;
    char *extraversion;
    int   extraversion_typ;
    char *extraversion_nr;
    char *pkgrevision;
    char *arch;
};

char cmp(struct nr *, struct nr *);
char parse_extra(struct nr *);

/*
** IN: s: pointer to versionstring..
**     v: pointer to version structure
**
** OUT: filled structure on success..
**
** RETURN: 0  on success
**         >0 error at position x
**
*/
int parse_version(char *s,  struct nr *v)
{
    char *p;
    size_t len;

    if(! (v->string=strdup(s))) {
        perror("strdup");
        exit(254);
    }
    
    s   = v->string;
    len = strlen(s);
    
    v->pkgname          = s+len;
    v->subname          = s+len;
    v->version          = s+len;
    v->extraversion     = s+len;
    v->extraversion_nr  = s+len;
    v->pkgrevision      = s+len;
    v->arch             = s+len;
    v->extraversion_typ = EXTRA_UNKNOWN;
    
    /* p-v-r   v-r   v */

    if((p=strrchr(s, '-'))) {
        v->pkgrevision = p+1;
        *p=0;

        if(!*(v->pkgrevision)) {
            return(p-s+1);
        }

        if((p=strchr(v->pkgrevision, '.'))) {
            v->arch = p+1;
            *p=0;

            if(!*(v->arch) || !*(v->pkgrevision)) {
                return(p-s+1);
            }
        }

        if((p=strrchr(s, '-'))) {
            v->version = p+1;
            *p=0;

            v->pkgname = s;

            if(!*(v->pkgname) || *(v->pkgname) == '_') {
                return(1);
            }
        } else {
            v->version = s;
        }
    } else {
        v->version = s;
    }

    if(!*(v->version) || *(v->version) == '_') {
        return(p-s+1);
    }

    if((p=strchr(v->version, '_'))) {
        *p=0;
        v->extraversion=p+1;
        if(!*(v->extraversion)) {
            return(p-s+1);
        }
    }

    if(v->pkgname && (p=strchr(v->pkgname, '_'))) {
        *p=0;
        v->subname=p+1;
        if(!*(v->subname)) {
            return(p-s+1);
        }
    }
    
    parse_extra(v);
    return(0);
}

char parse_extra(struct nr *v)
{
    struct extra_version extra[] = {
        { "alpha", EXTRA_ALPHA, 5 },
        { "beta",  EXTRA_BETA,  4 },
        { "rc",    EXTRA_RC,    2 },
        { "patch", EXTRA_PATCH, 5 },
        { "p",     EXTRA_PATCH, 1 },
        { NULL,    EXTRA_ANY,   0 }
    };
    
    struct extra_version *ev;
    char                 *s;
   
    s  = v->extraversion;
    ev = extra;
    
    if(!*s) {
        v->extraversion_typ = EXTRA_NONE;
        v->extraversion_nr  = s;
#ifdef DEBUG
        printf(stderr, "parse_extra(%s) = %d, '%s'", 
            v->extraversion, v->extraversion_typ, v->extraversion_nr);
#endif
        return(1);
    }
    
    while(ev->string && strncmp(ev->string, s, ev->length))
        ev++;

    v->extraversion_typ = ev->priority;
    v->extraversion_nr  = s + ev->length;
    
#ifdef DEBUG
        printf(stderr, "parse_extra(%s) = %d, '%s'", 
            v->extraversion, v->extraversion_typ, v->extraversion_nr);
#endif
    return(1);
}

int parse_argument(char* text, int len, struct nr *versionsnummer)
{	
    int p;
    
    if((p=parse_version(text, versionsnummer))) {
        fprintf(stderr, "syntax error at position %d in '%s'\n", p, text);
        return(0);
    }
    return(1);
}

char version_cmp(char *v1, char *v2) {
    char *a, *b;
    long long i,j;
    a = v1;
    b = v2;

    while(*a && *b && *a == *b) {
        a++;
        b++;
    }
    
    /* strings are equal ; *a==*b==0*/
    if(*a == *b)
        return(0);
    
    if(isdigit(*a)) {
        if(isdigit(*b)) {
            i = atoll(a);
            j = atoll(b);
           
            if(i<j)
                return(-1);
            if(i>j)
                return(1);
                
            fprintf(stderr, "YOU HIT A BUG! #001\n");
            exit(254);
        }
        /* a > ('.',alpha, 0) */
        return(1);
    }
    
    if(isalpha(*a)) {
    
        /*  alpha < digit */
        if(isdigit(*b))
            return(-1);
        
        if(isalpha(*b)) {
            if(*a < *b)
                return(-1);
            return(1);
        }
        return(1);
    }
    
    if(! *b)
        return(1);
    
    return(-1);
}

char cmp(struct nr *v1, struct nr *v2) {
    char ret;

    ret = version_cmp(v1->version, v2->version);
    if(ret) return(ret);
    
    if(v1->extraversion_typ < v2->extraversion_typ)
        return(-1);

    if(v1->extraversion_typ > v2->extraversion_typ)
        return(1);
    
    ret = version_cmp(v1->extraversion_nr, v2->extraversion_nr);
    if(ret) return(ret);
    
    ret = version_cmp(v1->pkgrevision, v2->pkgrevision);
    return ret;
}

void print_format(char* s, struct nr* v)
{
    char *p;
    
    p=s;
    
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
                        if(*(v->arch))
                            printf(".%s", v->arch);
                    }
                    break;
            }
            continue;
        } /* if '%' */
        
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
    
    struct nr v[2];
    struct nr *a, *b, *tmp;
    
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
            if(!parse_argument(argv[i], 0, &v[i]))
               return(0);
        }
        
        ret = cmp(a, b);
        
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
        
        if(argc < 2) {
            fprintf(stderr, "usage: beeversion -[min|max] <package1> <package2> [.. <packageN>]\n");
            return(255);
        }
        
        if(!parse_argument(argv[0], 0, a))
            return(0);
        
        for(i=1; i<argc; i++) {
            if(!parse_argument(argv[i], 0, b))
                return(0);
            
            ret = cmp(a, b);
            
            if((t == T_MIN && ret > 0) || (t == T_MAX && ret < 0)) {
                tmp = b;
                b   = a;
                a   = tmp;
            }
        }
        
        print_format("%F\n", a);
        return(1);
    }
    
    fprintf(stderr, "YOU HIT A BUG #006\n");
    
    return(0);
}

int do_parse(int argc, char *argv[], char *format) {
    struct nr v;
    
    if(argc != 1) {
        fprintf(stderr, "usage: beeversion <package>\n"); 
        return(255);
    }
    
    if(!parse_argument(argv[0], 0, &v))
        return(0);
    
    print_format(format, &v);
    
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
    
    keyvalue = "PKGNAME=%p\nPKGSUBNAME=%s\nPKGVERSION=%v\nPKGEXTRAVERSION=%e\nPKGREVISION=%r\nPKGARCH=%a\nPKGFULLNAME=%P\nPKGFULLVERSION=%V\nPKGFULLPKG=%F\n";

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

        {"pkgname",         no_argument, 0, 'p'},
        {"pkgsubname",      no_argument, 0, 's'},
        {"pkgversion",      no_argument, 0, 'v'},
        {"pkgextraversion", no_argument, 0, 'e'},
        {"pkgrevision",     no_argument, 0, 'r'},
        {"pkgarch",         no_argument, 0, 'a'},

        {0, 0, 0, 0}
    };
    
    while ((c = getopt_long_only(argc, argv, "", long_options, &option_index)) != -1) {
    
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
                format = calloc(sizeof(char), argc * 3);
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
        
        if(opterr)
           continue;
        
        fprintf(stderr, "YOU HIT A BUG #003 opterr=%d\n", opterr);
    }  /* end while getopt_long_only */
    
    if(mode == MODE_TEST) 
        return(!do_test(argc-optind, argv+optind, test_to_do));
    
    if(!mode || mode == MODE_PARSE) {
        if(!format)
            format = keyvalue;
        
        do_parse(argc-optind, argv+optind, format);
    }
        
    return(0);
}
