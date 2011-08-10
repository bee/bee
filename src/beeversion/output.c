/*
** beeversion - compare bee package versionnumbers
** Copyright (C) 2010
**       Marius Tolzmann <tolzmann@molgen.mpg.de>
**       David Fessler <dfessler@uni-potsdam.de>
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
#include <string.h>
#include <assert.h>

#include "beeversion.h"

static void cut_and_print(char *string, char delimiter, char opt_short)
{
    char *p, *s;

    assert(string);

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

void print_format(char* s, struct beeversion *v, char *filter_pkgfullname)
{
    char *p;
    size_t len;

    assert(s);
    assert(v);
    assert(v->pkgname);
    assert(v->subname);
    assert(v->version);
    assert(v->extraversion);
    assert(v->pkgrevision);
    assert(v->arch);
    assert(v->suffix);

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
                    if(*(v->pkgname))
                        printf("%s", v->pkgname);
                    if(*(v->subname))
                        printf("_%s", v->subname);
                    if(*(v->version))
                        printf("-%s", v->version);
                    if(*(v->extraversion))
                        printf("_%s", v->extraversion);
                    if(*(v->pkgrevision))
                        printf("-%s", v->pkgrevision);
                    if(*p == 'A' && *(v->arch))
                        printf(".%s", v->arch);
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

