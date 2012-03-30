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

#ifndef BEEDEP_NODE_H
#define BEEDEP_NODE_H

#include "beedep_tree.h"

#define NODENAME_MAX (2 * PATH_MAX)

struct node {
    char *name, *type;
    struct tree *need,
                *neededby,
                *provide,
                *providedby;
};

extern struct node *node_new(char *name, char *type);
extern void node_set_type(struct node *n, char *type);
extern void node_free(struct node *n);
extern int nodecmp(struct node *a, struct node *b);

#endif
