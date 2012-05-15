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
#include <errno.h>

#include "bee_version.h"
#include "bee_version_compare.h"
#include "bee_version_parse.h"
#include "bee_version_output.h"
#include "bee_tree.h"
#include "bee_getopt.h"

void my_free_key(void *key)
{
    struct beeversion *v = key;

    free(v->string);
    free(v);
}

void my_free_data(void *data)
{
    free(data);
}

int my_compare_key(void *a, void *b)
{
    return compare_beepackages(a, b);
}

int my_compare_data(void *a, void *b)
{
    return strcmp(a, b);
}

void my_print(void *key, void *data)
{
    fputs(data, stdout);
}

void *my_generate_key(const void *data)
{
    const char *line = data;
    size_t l;
    char *s, *p;
    char *string;
    struct beeversion *v;

    string = strdup(line);

    if(!string) {
        perror("calloc(s)");
        return NULL;
    }

    s = string;
    l = strlen(s);
    p = s+l-1;

    while (p > s && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
        *(p--) = 0;

    while (*s && (*s == ' ' || *s == '\t'))
        s++;

    if(p < s) {
        free(string);
        errno = EINVAL;
        return NULL;
    }

    v = calloc(1, sizeof(*v));
    if(!v) {
        perror("calloc(beeversion)");
        free(s);
        return NULL;
    }

    if(parse_version(s, v) != 0) {
        free(v->string);
        init_version(s, v);
        v->pkgname = v->string;
    }

    free(string);

    return v;
}

struct bee_tree *init_tree(void)
{
    struct bee_tree *tree;

    tree = bee_tree_allocate();

    if(tree == NULL) {
        perror("cannot allocate memory ..");
        exit(EXIT_FAILURE);
    }

    tree->generate_key = &my_generate_key;
    tree->free_key     = &my_free_key;
    tree->free_data    = &my_free_data;
    tree->compare_key  = &my_compare_key;
    tree->compare_data = &my_compare_data;
    tree->print        = &my_print;

    return tree;
}


int main(int argc, char *argv[])
{
    char line[LINE_MAX];
    char *data;
    FILE *file;

    struct bee_tree *tree;
    struct bee_subtree *subtree;

    char *filename;

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
                opt_uniq++;
                break;
        }
    }

    optind = optctl.optind;
    argc   = optctl.argc;
    argv   = optctl.argv;

    if(argc > optind) {
        filename = argv[optind];
        file     = fopen(filename, "r");

        if(file == NULL) {
            perror(filename);
            exit(EXIT_FAILURE);
        }
    } else {
        file = stdin;
    }

    tree = init_tree();

    if (opt_uniq == 1)
        bee_tree_set_flags(tree, BEE_TREE_FLAG_UNIQUE_DATA);
    else if (opt_uniq > 1)
        bee_tree_set_flags(tree, BEE_TREE_FLAG_UNIQUE);

    bee_tree_set_flags(tree, BEE_TREE_FLAG_COMPARE_DATA_ON_EQUAL_KEY);

    while(fgets(line, LINE_MAX, file)) {
        data = strdup(line);
        if(!data) {
            perror("strdup(data)");
            bee_tree_free(tree);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        subtree = bee_tree_insert(tree, data);
        if(subtree)
            continue;

        if(errno == EINVAL)
            continue;

        if(errno == EEXIST)
            continue;

        bee_tree_free(tree);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);

    bee_tree_print_plain(tree);

    bee_tree_free(tree);

    return 0;
}
