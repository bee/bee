/*
** beesep - split beefind output
**
** Copyright (C) 2012
**       Marius Tolzmann <tolzmann@molgen.mpg.de>
**       Tobias Dreyer <dreyer@molgen.mpg.de>
**       Matthias Ruester <ruester@molgen.mpg.de>
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
#include <string.h>
#include <assert.h>
#include <err.h>
#include <regex.h>

#define bee_fprint(fh, str)  bee_fnprint(fh, 0, str)

static int bee_fnprint(FILE *fh, size_t n, char *str)
{
    size_t m;

    if (ferror(fh))
        return 0;

    m =  strlen(str);

    assert(n <= m);

    if (!n)
        n = m;

    if (!n)
        return 1;

    m = fwrite(str, sizeof(*str), n, fh);

    if (m != n) {
        assert(ferror(fh));
        warn("fwrite");
        return 0;
    }

    return 1;
}

static void print_escaped(char *s, size_t n)
{
    char *c;

    assert(s);

    c = s;

    bee_fprint(stdout, "'");

    while ((c = strchr(s, '\'')) && c - s < n) {
        if (c-s)
            bee_fnprint(stdout, c - s, s);
        bee_fprint(stdout, "'\\''");
        n -= c - s + 1;
        s  = c + 1;
    }

    if (n)
        bee_fnprint(stdout, n, s);

    bee_fprint(stdout, "'\n");
}

static int bee_regcomp(regex_t *preg, char *regex, int cflags)
{
    int  regerr;
    char errbuf[BUFSIZ];

    regerr = regcomp(preg, regex, cflags);

    if (!regerr)
        return 1;

    regerror(regerr, preg, errbuf, BUFSIZ);
    warnx("bee_regcomp: %s\n", errbuf);
    return 0;
}

static short do_separation(char *str)
{
    int   res=0;
    int   r;

    int   start,
          end;

    char *key,
         *value;
    int   keylen,
          vallen;

    regex_t    regex_first;
    regex_t    regex_next;
    regmatch_t pmatch;

    /* compile regexes */

    r = bee_regcomp(&regex_first, "^[[:alnum:]]+=", REG_EXTENDED);
    if (!r)
        return 0;

    r = bee_regcomp(&regex_next, ":[[:alnum:]]+=", REG_EXTENDED);
    if (!r)
        goto out_first;


    /* match first key */

    r = regexec(&regex_first, str, 1, &pmatch, 0);
    if (r == REG_NOMATCH) {
        warnx("String '%s' does not start with a key\n", str);
        goto out;
    }

    /* init loop variables */

    end = pmatch.rm_eo;

    key    = str;
    value  = str+end;

    /* always continue search for next match within value */
    str = value;

    /* match all other keys */

    while (regexec(&regex_next, str, 1, &pmatch, 0) != REG_NOMATCH) {
        start   = pmatch.rm_so;
        end     = pmatch.rm_eo;

        /*
                +--+ keylen
                     +----+ vallen
                     +-----+ start
                     +-----------+ end
            ...:key1=value1:key2=value2:...
                ^    ^      ^
                |    |      |
                key  value  (nextkey)
                     str
        */

        keylen = value-key-1;
        vallen = start;

        /* print current key/value pair */

        bee_fnprint(stdout, keylen+1, key);
        print_escaped(value, vallen);

        /* reinit for next round */

        key   = str+start+1;
        value = str+end;
        str   = value;
    }

    /* print last key/value pair */

    keylen = value-key-1;

    bee_fnprint(stdout, keylen+1, key);
    print_escaped(value, strlen(value));

    /* we are done -> set error-state and clean up */

    res = !ferror(stdout);

out:
    regfree(&regex_next);

out_first:
    regfree(&regex_first);

    return res;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        warnx("argument missing\n");
        return 1;
    }

    if (!do_separation(argv[1]))
        return 1;

    return 0;
}
