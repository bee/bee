/*
 * beeversion - compare bee package versionnumbers
 * Copyright (C) 2010-2011
 *       Marius Tolzmann <tolzmann@molgen.mpg.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "beeversion.h"

int compare_version_strings(char *v1, char *v2) {
    char *a, *b;
    long long i,j;

    assert(v1);
    assert(v2);

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
            /* rewind string to first digit */
            /* e.g. to compare 12 vs 100 and not 2 vs 00 */
            while(a > v1 && isdigit(*(a-1))) {
                a--;
                b--;
            }
            i = atoll(a);
            j = atoll(b);

            if(i<j)
                return(-1);
            if(i>j)
                return(1);

            /* numbers are equal but strings are not?           */
            /* yes ->  leading zeros: atoll("01") == atoll("1") */
            return(0);
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

int compare_beepackage_names(struct beeversion *v1, struct beeversion *v2) {
    int ret;

    assert(v1);
    assert(v2);

    ret = strcmp(v1->pkgname, v2->pkgname);

    if(!ret)
        ret = strcmp(v1->subname, v2->subname);

    return(ret);

}

int compare_beeversions(struct beeversion *v1, struct beeversion *v2) {
    int ret;

    assert(v1);
    assert(v2);

    ret = compare_version_strings(v1->version, v2->version);
    if(ret) return(ret);

    if(v1->extraversion_typ < v2->extraversion_typ)
        return(-1);

    if(v1->extraversion_typ > v2->extraversion_typ)
        return(1);

    ret = compare_version_strings(v1->extraversion_nr, v2->extraversion_nr);
    if(ret) return(ret);

    ret = compare_version_strings(v1->pkgrevision, v2->pkgrevision);
    return ret;
}

int compare_beepackages(struct beeversion *v1, struct beeversion *v2) {
    int ret;

    assert(v1);
    assert(v2);

    ret = compare_beepackage_names(v1, v2);

    if(!ret)
        ret = compare_beeversions(v1, v2);

    return(ret);
}

