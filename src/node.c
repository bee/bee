/*
** bee-dep - dependency tool for bee
**
** Copyright (C) 2009-2012
**       Matthias Ruester <ruester@molgen.mpg.de>
**       Lucas Schwass <schwass@molgen.mpg.de>
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

#include "node.h"

struct node *node_new(char *name, char *type)
{
    struct node *n;

    if ((n = calloc(1, sizeof(struct node))) == NULL) {
        perror("bee-dep: node_new: calloc");
        exit(EXIT_FAILURE);
    }

    n->name = strdup(name);
    n->type = strdup(type);

    n->need       = tree_new();
    n->neededby   = tree_new();
    n->provide    = tree_new();
    n->providedby = tree_new();

    return n;
}

void node_set_type(struct node *n, char *type)
{
    if (n->type)
        free(n->type);

    n->type = strdup(type);
}

void node_free(struct node *n)
{
    tree_free(n->need);
    tree_free(n->neededby);
    tree_free(n->provide);
    tree_free(n->providedby);
    free(n->name);
    free(n->type);
    free(n);
}

int nodecmp(struct node *a, struct node *b)
{
    if (a == b)
        return 0;

    return strcmp(a->name, b->name);
}
