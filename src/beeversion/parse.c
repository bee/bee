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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <sys/utsname.h>

#include "beeversion.h"


char parse_extra(struct beeversion *v)
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

    assert(v);
    assert(v->extraversion);

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
/*
 * IN: string: pointer to versionstring..
 *          v: pointer to version structure..
 */
void bee_init_version(char *string, struct beeversion *v)
{
    char *s;
    size_t len;

    assert(string);
    assert(v);

    if(v->string)
        free(v->string);

    if(!(v->string=strdup(string))) {
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
    v->suffix           = s+len;
    v->extraversion_typ = EXTRA_UNKNOWN;
}

int bee_parse_version(struct beeversion *v)
{
    char   *p, *s;
    char   *version_or_revision;

    s = v->string;

    /* p-v-r   p-v   v */

    /* extract basename */
    if((p=strrchr(s,'/'))) {
        s = p+1;
    }

    /* extract suffix .bee* */
    if((p=strstr(s, ".bee"))) {
        v->suffix = p+1;
        *p=0;
    } else if((p=strstr(s, ".iee"))) {
        v->suffix = p+1;
        *p=0;
    }

    /* extract architecture if known.. */
    if((p=strrchr(s, '.')) && !strchr(++p, '-')) {
        struct utsname unm;
        char           *arch[] = { SUPPORTED_ARCHITECTURES, NULL };
        char           **a;

        if(uname(&unm)) {
             perror("uname");
             exit(1);
        }

        if(!strcmp(p, unm.machine)) {
            v->arch = p;
            *(p-1)  = 0;
        }

        for(a=arch; *(p-1) && *a; a++) {
            if(strcmp(p, *a))
                continue;

            v->arch = p;
            *(p-1)  = 0;
        }
    }

    /* do split pkg */

    if((p=strrchr(s, '-'))) {
        version_or_revision = p+1;
        *p=0;

        /* check for empty version_or_revision */
        if(!*version_or_revision)
            return(p-s+1);

        /* first part ist pname (will be checked later) */
        v->pkgname = s;

        /* version_or_revision must start with a digit */
        if(!isdigit(*version_or_revision)) {
            return(p-s+1);
        }

        /* if there is another dash
        **   revision is version_or_revision
        **   version  is p+1
        ** else
        **   revision is empty
        **   version  is version_or_revision
        */
        if((p=strrchr(s, '-'))) {
            int  r_isdigit = 1;
            char *r;

            /* check if revision matches ^[0-9]+$ */

            for(r=version_or_revision; *r; r++) {
                if(!isdigit(*r)) {
                    r_isdigit=0;
                    break;
                }
            }

            if(!r_isdigit)
                p = NULL;


            if(r_isdigit && !isdigit(*(p+1)))
                p = NULL;
        }

        if(p) {
            v->version     = p+1;
            *p=0;

            if(!*(v->version) || *(v->version) == '_')
                return(p-s+1);

            v->pkgrevision = version_or_revision;

        } else {
            v->version = version_or_revision;
        }
    } else {
        if(!isdigit(*s)) {
            return(1);
        }
        v->version = s;
    }

    /* check pname or version */
    if(!*s || *s == '_')
        return(1);

    if((p=strchr(v->version, '_'))) {
        *p=0;
        v->extraversion=p+1;
        if(!*(v->extraversion))
            return(p-s+1);
    }

    if(v->pkgname && (p=strchr(v->pkgname, '_'))) {
        *p=0;
        v->subname=p+1;
        if(!*(v->subname))
            return(p-s+1);
    }

    parse_extra(v);
    return(0);
}

/*
 * IN: string: pointer to versionstring..
 *          v: pointer to version structure
 *
 * OUT: filled structure on success..
 *
 * RETURN: 0  on success
 *         >0 error at position x
 *
 */
int parse_version(char *string, struct beeversion *v)
{
    bee_init_version(string, v);

    return bee_parse_version(v);
}
