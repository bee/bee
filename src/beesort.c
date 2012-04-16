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

#include "bee_version.h"
#include "bee_version_compare.h"
#include "bee_version_parse.h"
#include "bee_version_output.h"
#include "bee_tree.h"
#include "bee_getopt.h"

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

struct bee_tree *init_tree(void)
{
    struct bee_tree *tree;

    tree = bee_tree_allocate();

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
    char line[LINE_MAX], *s, *p;
    FILE *file;
    struct bee_tree *tree;
    struct beeversion *v;
    int l;

    int opt;
    int optindex;
    int optind;

    int opt_uniq  = 0;

    struct bee_getopt_ctl optctl;
    struct bee_option options[] = {
        BEE_OPTION_NO_ARG("unique",   'u'),
        BEE_OPTION_END
    };

    bee_getopt_init(&optctl, argc-1, &argv[1], options);

    optctl.program = "beesort";

    while((opt=bee_getopt(&optctl, &optindex)) != BEE_GETOPT_END) {

        if (opt == BEE_GETOPT_ERROR) {
            exit(1);
        }

        switch(opt) {
            case 'u':
                opt_uniq = 1;
                break;
        }
    }
    optind = optctl.optind;
    argc   = optctl.argc;
    argv   = optctl.argv;

    if(argc > optind) {
        file = fopen(argv[optind], "r");
        if(file == NULL) {
            perror(argv[optind]);
            exit(EXIT_FAILURE);
        }
    } else {
        file = stdin;
    }

    tree = init_tree();

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

        if(!(opt_uniq && bee_tree_search(tree, v)))
            bee_tree_insert(tree, v);
    }

    fclose(file);

    bee_tree_print_plain(tree);

    bee_tree_free(tree);

    return 0;
}
