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

#ifndef BEEDEP_GRAPH_H
#define BEEDEP_GRAPH_H

#define NEEDS    "needs"
#define PROVIDES "provides"
#define TYPE     "type"

#define PACKAGE "PACKAGE"
#define DIR     "DIRECTORY"
#define UNKNOWN "VOID"

#define IS_FILE(a)       ((a)[0] == '/')
#define IS_DIR(a)        (!strcmp((a)->type, DIR))
#define IS_WHITESPACE(a) ((a) == ' ' || (a) == '\t' || (a) == '\n' || (a) == '\r')
#define IS_PKG(a)        (!strcmp((a)->type, PACKAGE))

#include "hash.h"

extern int graph_insert_nodes(struct hash *hash, char *filename);
extern void print_broken(struct hash *hash, char *remove);
extern int print_removable(struct hash *hash, char *remove);
extern int count_removable(struct hash *hash, char *remove);
extern int list_files(struct hash *hash, char *pkgname);
extern int count_files(struct hash *hash, char *pkgname);
extern int print_providers(struct hash *hash, char *name);
extern int count_providers(struct hash *hash, char *name);
extern int print_needs(struct hash *hash, char *name);
extern int print_neededby(struct hash *hash, char *name);
extern void list_packages(struct hash *hash);
extern int count_packages(struct hash *hash);
extern int remove_package(struct hash *hash, char *pkgname);
extern int print_conflicts(struct hash *hash);
extern int save_cache(struct hash *hash, char *path);
extern struct hash *load_cache(char *filename);
extern unsigned long count_providedby(struct hash *hash, char *count);

#endif
