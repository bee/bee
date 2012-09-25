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

#define errf(exit, format, ...) err(exit, "%s: " format, __func__, ## __VA_ARGS__)

static void bee_fnprint(FILE *fh, size_t n, char *str)
{
    size_t m;

    m =  strlen(str);

    assert(n <= m);

    if (!n)
        n = m;

    if (!n)
        return;

    while ((m = fwrite(str, sizeof(*str), n, fh)) != n)
        n -= m;
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

int do_separation(char *str)
{
    regex_t regex;
    regmatch_t pmatch;

    int r;

    int start;
    int end;
    int keylen;
    char *value;
    char *key;

    r = bee_regcomp(&regex, "^[[:alnum:]]+=", REG_EXTENDED);
    if (!r) {
        regfree(&regex);
        return 0;
    }

    if (regexec(&regex, str, 1, &pmatch, 0) == REG_NOMATCH) {
        fprintf(stderr, "beesep: do_separation: "
                        "separation failed for '%s': no key found\n", str);
        regfree(&regex);
        return 0;
    }

    regfree(&regex);

    end = pmatch.rm_eo;

    /* keep the key */
    key    = str;
    keylen = end;

    /* jump to end of key */
    str += end;

    /* assign the beginning of the value */
    value = str;

    r = bee_regcomp(&regex, ":[[:alnum:]]+=", REG_EXTENDED);
    if (!r) {
        regfree(&regex);
        return 0;
    }

    while (regexec(&regex, str, 1, &pmatch, 0) != REG_NOMATCH) {
        /*    s    e
           baz:foo=bar */
        start = pmatch.rm_so;
        end   = pmatch.rm_eo;

        /* jump to the beginning of the next key */
        str += start + 1;

        /* print previous key */
        bee_fnprint(stdout, keylen, key);

        /* print previous value */
        print_escaped(value, str - value - 1);

        /* memorize the found key */
        key    = str;
        keylen = end - start - 1;

        /* assign the new beginning of the value */
        value = str + (end - start) - 1;
    }

    /* print last key */
    bee_fnprint(stdout, keylen, key);

    /* print everything leftover as a value */
    print_escaped(value, strlen(value));

    regfree(&regex);

    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "beesep: argument missing\n");
        return 1;
    }

    if (!do_separation(argv[1]))
        return 1;

    return 0;
}
