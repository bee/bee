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

#ifndef BEEDEP_TREE_H
#define BEEDEP_TREE_H

#include "node.h"

struct tree {
    struct tree_node *root;
};

struct tree_node {
    struct node *n;
    struct tree_node *left, *right, *parent;
    unsigned char max_depth;
    char balance_value;
};

extern struct tree *tree_new(void);
extern void tree_insert(struct tree *root, struct node *n);
extern struct tree_node *tree_search_tree_node(struct tree *t,
                                               struct node *n);
extern struct node *tree_search_node(struct tree *t, char *name);
extern void tree_node_free(struct tree *t, struct tree_node *n);
extern unsigned long tree_count(struct tree *t);
extern void tree_free_all_nodes(struct tree *t);
extern void tree_free(struct tree *t);
extern struct tree_node *tree_next(struct tree_node *t);
extern struct tree_node *tree_first(struct tree_node *t);

#endif
