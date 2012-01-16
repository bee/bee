/*
** beedep - dependency tool for bee
**
** Copyright (C) 2009-2011
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

#include "hash.h"

struct hash *hash_new(void)
{
    struct hash *h;
    unsigned long i;

    if ((h = calloc(1, sizeof(struct hash))) == NULL) {
        perror("bee-dep: hash_new: calloc");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < TBLSIZE; i++)
        h->tbl[i] = tree_new();

    return h;
}

unsigned long hash_index(char *key)
{
    register unsigned long index = 0;
    char c;

    while ((c = *key++))
        index = c + (index << 6) + (index << 16) - index;
     /* index = c + index * 65599 (prime number) */

    return index % TBLSIZE;
}

void hash_insert(struct hash *hash, struct node *n)
{
    unsigned long index = hash_index(n->name);

    tree_insert(hash->tbl[index], n);
}

struct node *hash_safe_insert(struct hash *hash, struct node *n)
{
    unsigned long index = hash_index(n->name);
    struct node *r;

    r = tree_search_node(hash->tbl[index], n->name);

    if (r)
        return r;

    tree_insert(hash->tbl[index], n);
    return n;
}

struct node *hash_search(struct hash *hash, char *key)
{
    return tree_search_node(hash->tbl[hash_index(key)], key);
}

void hash_free(struct hash *hash)
{
    int i;

    if (!hash)
        return;

    for (i = 0; i < TBLSIZE; i++) {
        tree_free_all_nodes(hash->tbl[i]);
        tree_free(hash->tbl[i]);
    }

    free(hash);
}
