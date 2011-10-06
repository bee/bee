/*
** beesort - sort bee packages
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
#include <string.h>
#include <limits.h>
#include <getopt.h>

#include "beeversion.h"
#include "compare.h"
#include "parse.h"
#include "output.h"
#include "tree.h"

#define NUM_OPTS       1
#define OPT_SHORT_UNIQ 'u'
#define OPT_LONG_UNIQ  "unique"

void my_free_data(void *data)
{
    struct beeversion *v = data;

    free(v->string);
    free(v);
}

int my_compare_key(void *a, void *b)
{
    return compare_beepackages(a, b);
}

void my_print_key(void *key)
{
    print_format("%A", key, NULL);
}

struct tree *init_tree(void)
{
    struct tree *tree;

    tree = tree_allocate();

    if(tree == NULL) {
        perror("cannot allocate memory ..");
        exit(EXIT_FAILURE);
    }

    tree->free_data   = &my_free_data;
    tree->compare_key = &my_compare_key;
    tree->print_key   = &my_print_key;

    return tree;
}


int main(int argc, char *argv[])
{
    char line[LINE_MAX], *s, *p, *optstring = NULL;
    FILE *file;
    struct tree *tree;
    struct beeversion *v;
    int l, opt, opt_uniq = 0, longindex;
    struct option long_options[] = {
        {OPT_LONG_UNIQ, 0, 0, OPT_SHORT_UNIQ},
        {0, 0, 0, 0}
    };

    optstring = calloc(NUM_OPTS + 1, sizeof(optstring));
    sprintf(optstring, "%c", OPT_SHORT_UNIQ);
    while((opt = getopt_long(argc, argv, optstring, long_options, &longindex)) != -1) {
        switch(opt) {
            case OPT_SHORT_UNIQ:
                opt_uniq = 1; break;
        }
    }
    free(optstring);

    tree = init_tree();

    if(argc > optind) {
        file = fopen(argv[optind], "r");
        if(file == NULL) {
            perror(argv[optind]);
            exit(EXIT_FAILURE);
        }
    } else {
        file = stdin;
    }

    while(fgets(line, LINE_MAX, file)) {
        l = strlen(line);
        s = line;
        p = line+l-1;

        while (p-s && *p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
            *(p--) = 0;

        while (*s && (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r'))
            s++;

        if(p-s < 0)
            continue;

        v = calloc(1, sizeof(*v));
        if(v == NULL) {
            perror("cannot allocate memory ..");
            exit(EXIT_FAILURE);
        }

        if(parse_version(s, v) != 0) {
            init_version(s, v);
            v->pkgname = v->string;
        }

        if(!(opt_uniq && tree_search(tree, v)))
            tree_insert(tree, v);
    }

    fclose(file);

    tree_print_plain(tree);

    tree_free(tree);

    return 0;
}

