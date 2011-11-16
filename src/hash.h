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

#ifndef BEEDEP_HASH_H
#define BEEDEP_HASH_H

#include "beedep_tree.h"

#define TBLSIZE 2000003 /* prime number */

struct hash {
    struct tree *tbl[TBLSIZE];
    unsigned long cnt;
};

extern struct hash *hash_new(void);
extern unsigned long hash_index(char *key);
extern void hash_insert(struct hash *hash, struct node *n);
extern struct node *hash_search(struct hash *hash, char *key);
extern void hash_free(struct hash *hash);

#endif
