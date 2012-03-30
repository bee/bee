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

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>

#include "graph.h"

#define IS_FILE(a)       ((a)[0] == '/')
#define IS_DIR(a)        (!strcmp((a)->type, DIR))
#define IS_WHITESPACE(a) ((a) == ' ' || (a) == '\t' || (a) == '\n' || (a) == '\r')

static void add_provide(struct node *a, struct node *b)
{
    tree_insert(a->provide, b);
    tree_insert(b->providedby, a);
}

static void add_need(struct node *a, struct node *b)
{
    tree_insert(a->need, b);
    tree_insert(b->neededby, a);
}

char *get_filename(char *s)
{
    while (*s != '/')
        s++;

    return s;
}

char *get_pkgname(char *s)
{
    char a[NODENAME_MAX];
    char *p, *i;

    p = s;
    i = a;

    while (*p != '/') {
        *i = *p;
        p++;
        i++;
    }

    *i = '\0';

    return strdup(a);
}

char is_virtual_file(char *s)
{
    if (*s == '/')
        return 0;

    while (*s && *s != '/')
        s++;

    return *s == '/';
}

int graph_insert_nodes(struct hash *hash, char *filename)
{
    FILE *file;
    register char *s, *p, *a;
    char type_flag, u;
    char line[LINE_MAX],
         prop[LINE_MAX],
         value[LINE_MAX],
         pkgname[NODENAME_MAX]  = {0},
         nodename[NODENAME_MAX] = {0};
    int l, line_cnt;
    struct node *n, *h, *v, *c;

    if ((file = fopen(filename, "r")) == NULL) {
        perror("bee-dep: graph_insert_nodes: fopen");
        return 1;
    }

    line_cnt = type_flag = u = 0;
    n        = NULL;

    while (fgets(line, LINE_MAX, file)) {
        line_cnt++;

        if (!line[0])
            continue;

        /* remove unnecessary characters */
        l = strlen(line);
        s = line;
        p = line + (l - 1) * sizeof(char);
        h = NULL;

        while (p - s && *p && IS_WHITESPACE(*p))
            *(p--) = 0;

        while (*s && (*s == ' ' || *s == '\t'))
            s++;

        l = p - s;

        if (l <= 0 || *s == '#')
            continue;

        /* read node name */
        if (*s == '[') {
            s++;

            if (*p != ']') {
                fprintf(stderr,
                        "bee-dep: %s: error at line %d: missing bracket\n",
                        filename, line_cnt);
                return 1;
            }

            *p = '\0';

            if (p - s == 0) {
                fprintf(stderr,
                        "bee-dep: %s: error at line %d: empty node name\n",
                        filename, line_cnt);
                return 1;
            }

            if (IS_FILE(s)) {
                if (!pkgname[0]) {
                    fprintf(stderr,
                            "bee-dep: %s: error at line %d: "
                            "dont know to which package"
                            "\"%s\" belongs to\n", filename, line_cnt, s);
                    return 1;
                }

                sprintf(nodename, "%s%s", pkgname, s);
            } else {
                sprintf(nodename, "%s", s);
            }

            c = node_new(nodename, UNKNOWN);
            n = hash_safe_insert(hash, c);
            if (c != n)
                node_free(c);

            type_flag = 0;
            continue;
        }

        /* read node properties */
        a = s;

        while (*a && *a != ' ' && *a != '\t' && *a != '=')
            a++;

        l = a - s;

        memset(prop, '\0', LINE_MAX);
        strncpy(prop, s, l * sizeof(char));

        while (*a && (*a == ' ' || *a == '\t' || *a == '='))
            a++;

        l = p - a + 1;

        if (!l) {
            fprintf(stderr,
                    "bee-dep: warning: %s: line %d: missing value "
                    "for property \"%s\"\n", filename, line_cnt, prop);
        }

        memset(value, '\0', LINE_MAX);
        strncpy(value, a, l * sizeof(char));

        if (strcasecmp(prop, TYPE) == 0) {
            if (type_flag) {
                fprintf(stderr,
                        "bee-dep: %s: error at line %d: "
                        "ambiguous type \"%s\"\n",
                        filename, line_cnt, value);
                return 1;
            }

            node_set_type(n, value);

            if (is_virtual_file(n->name))
                node_set_type(hash_search(hash, get_filename(n->name)),
                              value);

            if (strcasecmp(PACKAGE, value) == 0)
                sprintf(pkgname, "%s", n->name);

            type_flag = 1;
        } else if (strcasecmp(prop, PROVIDES) == 0) {
            c = node_new(value, UNKNOWN);
            h = hash_safe_insert(hash, c);

            if (c != h)
                node_free(c);

            if (IS_FILE(value)) {
                sprintf(nodename, "%s%s", pkgname, value);

                c = node_new(nodename, UNKNOWN);
                v = hash_safe_insert(hash, c);

                if (v != c)
                    node_free(c);

                add_provide(n, v);
                add_provide(v, h);
            } else {
                add_provide(n, h);
            }
        } else if (strcasecmp(prop, NEEDS) == 0) {
            c = node_new(value, UNKNOWN);
            h = hash_safe_insert(hash, c);

            if (c != h)
                node_free(c);

            add_need(n, h);
        }
    }

    if (fclose(file) == EOF) {
        perror("bee-dep: graph_insert_nodes: fclose");
        return 1;
    }

    if (line_cnt == 0) {
        fprintf(stderr, "bee-dep: error: file '%s' is empty\n", filename);
        return 1;
    }

    return 0;
}

static void search_dependencies(struct hash *hash, struct node *n,
                                struct tree *d)
{
    struct tree_node *t;
    char *pkgname;

    t = tree_first(n->neededby->root);

    while (t) {
        if (is_virtual_file(t->n->name)) {
            pkgname = get_pkgname(t->n->name);
            if (!tree_search_node(d, pkgname))
                tree_insert(d, hash_search(hash, pkgname));
            free(pkgname);
        }

        t = tree_next(t);
    }

    t = tree_first(n->provide->root);

    while (t) {
        search_dependencies(hash, t->n, d);
        t = tree_next(t);
    }
}

static int print_dependencies(struct hash *hash, struct node *n)
{
    struct tree *t;
    struct tree_node *e;

    t = tree_new();

    search_dependencies(hash, n, t);

    e = tree_first(t->root);

    while (e) {
        if (strcmp(n->name, e->n->name) != 0)
            puts(e->n->name);

        e = tree_next(e);
    }

    tree_free(t);

    return 0;
}

void add_all_neededby(struct hash *hash, struct node *n, struct tree *a)
{
    struct tree_node *t;
    char *pkgname;

    t = tree_first(n->neededby->root);

    while (t) {
        add_all_neededby(hash, t->n, a);
        pkgname = get_pkgname(t->n->name);
        if (is_virtual_file(t->n->name))
            tree_insert(a, hash_search(hash, pkgname));
        free(pkgname);
        t = tree_next(t);
    }
}

void check_remove(struct hash *hash, struct node *n, struct tree *r)
{
    struct tree_node *t;

    t = tree_first(n->provide->root);

    while (t) {
        check_remove(hash, t->n, r);
        t = tree_next(t);
    }

    if (tree_count(n->providedby) == 1)
        add_all_neededby(hash, n, r);
}

void print_broken(struct hash *hash, char *remove)
{
    struct node *n;
    struct tree *t;
    struct tree_node *e;

    if ((n = hash_search(hash, remove)) == NULL) {
        fprintf(stderr,
                "bee-dep: print_broken: cannot find \"%s\"\n",
                remove);
        return;
    }

    if (!IS_PKG(n)) {
        fprintf(stderr,
                "bee-dep: print_broken: \"%s\": no such package\n",
                remove);
        return;
    }

    t = tree_new();

    check_remove(hash, n, t);

    e = tree_first(t->root);

    if (!e)
        puts("none");

    while (e) {
        if (strcmp(remove, e->n->name) != 0)
            puts(e->n->name);

        e = tree_next(e);
    }

    tree_free(t);
}

void sort_dirs(char **dirs, int dir_cnt)
{
    int i, j;
    size_t c;
    char *h;

    for (i = 1; i < dir_cnt; i++) {
        h = dirs[i];
        j = i;
        c = strlen(h);

        while (j > 0 && strlen(dirs[j - 1]) < c) {
            dirs[j] = dirs[j - 1];
            j--;
        }

        dirs[j] = h;
    }
}

void search_removable(struct hash *hash, struct node *n,
                      struct tree *t, char *remove)
{
    struct tree_node *e;
    char *pkgname;

    e = tree_first(n->provide->root);

    while (e) {
        search_removable(hash, e->n, t, remove);
        e = tree_next(e);
    }

    if (IS_FILE(n->name) && tree_count(n->providedby) <= 1) {
        pkgname = get_pkgname(n->providedby->root->n->name);

        if (!strcmp(pkgname, remove))
            tree_insert(t, n);

        free(pkgname);
    }
}

int print_removable(struct hash *hash, char *remove)
{
    struct node *n;
    struct tree *t;
    struct tree_node *e;
    char **dirs, **files;
    int cnt, dir_cnt, file_cnt, i;

    if ((n = hash_search(hash, remove)) == NULL) {
        fprintf(stderr,
                "bee-dep: print_removable: cannot find \"%s\"\n",
                remove);
        return 1;
    }

    if (!IS_PKG(n)) {
        fprintf(stderr,
                "bee-dep: print_removable: \"%s\": no such package\n",
                remove);
        return 1;
    }

    t = tree_new();

    search_removable(hash, n, t, remove);

    cnt = tree_count(t);

    if ((dirs = calloc(cnt, sizeof(*dirs))) == NULL
        || (files = calloc(cnt, sizeof(*files))) == NULL) {
        perror("bee-dep: print_removable: calloc");
        return 1;
    }

    e = tree_first(t->root);
    dir_cnt = file_cnt = 0;

    while (e) {
        if (IS_DIR(e->n))
            dirs[dir_cnt++]   = e->n->name;
        else
            files[file_cnt++] = e->n->name;

        e = tree_next(e);
    }

    sort_dirs(dirs, dir_cnt);

    for (i = 0; i < file_cnt; i++)
        puts(files[i]);

    for (i = 0; i < dir_cnt; i++)
        puts(dirs[i]);

    free(dirs);
    free(files);
    tree_free(t);

    return 0;
}

int count_removable(struct hash *hash, char *remove)
{
    struct node *n;
    struct tree *t;
    int c;

    if ((n = hash_search(hash, remove)) == NULL) {
        fprintf(stderr,
                "bee-dep: print_removable: cannot find \"%s\"\n",
                remove);
        return -1;
    }

    if (!IS_PKG(n)) {
        fprintf(stderr,
                "bee-dep: print_removable: \"%s\": no such package\n",
                remove);
        return -1;
    }

    t = tree_new();

    search_removable(hash, n, t, remove);

    c = tree_count(t);

    tree_free(t);

    return c;
}

static int list_all_files(struct node *n, char print)
{
    struct tree_node *t;
    int count;

    t     = tree_first(n->provide->root);
    count = 0;

    while (t) {
        if (IS_FILE(t->n->name)) {
            count++;

            if (print)
                puts(t->n->name);
        }

        count += list_all_files(t->n, print);

        t = tree_next(t);
    }

    return count;
}

static int count_all_files(struct node *n)
{
    struct tree_node *t;
    int count;

    t     = tree_first(n->provide->root);
    count = 0;

    while (t) {
        if (IS_FILE(t->n->name))
            count++;

        count += list_all_files(t->n, 0);

        t = tree_next(t);
    }

    return count;
}

int list_files(struct hash *hash, char *pkgname)
{
    struct node *n;

    if ((n = hash_search(hash, pkgname)) == NULL) {
        fprintf(stderr,
                "bee-dep: list_files: cannot find \"%s\"\n",
                pkgname);
        return 1;
    }

    if (!IS_PKG(n)) {
        fprintf(stderr,
                "bee-dep: list_files: \"%s\": no such package\n",
                pkgname);
        return 1;
    }

    list_all_files(n, 1);

    return 0;
}

int count_files(struct hash *hash, char *pkgname)
{
    struct node *n;

    if ((n = hash_search(hash, pkgname)) == NULL) {
        fprintf(stderr,
                "bee-dep: count_files: cannot find \"%s\"\n",
                pkgname);
        return -1;
    }

    if (!IS_PKG(n)) {
        fprintf(stderr,
                "bee-dep: count_files: \"%s\": no such package\n",
                pkgname);
        return -1;
    }

    return count_all_files(n);
}

static void get_all_providers(struct node *n, struct tree *all)
{
    struct tree_node *t;

    t = tree_first(n->providedby->root);

    while (t) {
        if (IS_PKG(t->n) && !tree_search_node(all, t->n->name))
            tree_insert(all, node_new(t->n->name, ""));

        get_all_providers(t->n, all);

        t = tree_next(t);
    }
}

int print_providers(struct hash *hash, char *name)
{
    struct node *n;
    struct tree_node *t;
    struct tree *all;

    if ((n = hash_search(hash, name)) == NULL) {
        fprintf(stderr,
                "bee-dep: print_providers: cannot find \"%s\"\n",
                name);
        return 1;
    }

    if (IS_PKG(n)) {
        fprintf(stderr,
                "bee-dep: print_providers: \"%s\" is a package\n",
                name);
        return 1;
    }

    all = tree_new();

    get_all_providers(n, all);

    t = tree_first(all->root);

    while (t) {
        puts(t->n->name);
        t = tree_next(t);
    }

    tree_free_all_nodes(all);
    tree_free(all);

    return 0;
}

int count_providers(struct hash *hash, char *name)
{
    int count;
    struct node *n;
    struct tree *all;

    count = 0;

    if ((n = hash_search(hash, name)) == NULL) {
        fprintf(stderr,
                "bee-dep: count_providers: cannot find \"%s\"\n",
                name);
        return -1;
    }

    if (IS_PKG(n)) {
        fprintf(stderr,
                "bee-dep: count_providers: error: \"%s\" is a package\n",
                name);
        return -1;
    }

    all = tree_new();
    get_all_providers(n, all);

    count = tree_count(all);

    tree_free_all_nodes(all);
    tree_free(all);

    return count;
}

int get_virtual_files(struct node *n, char *name, struct tree *all)
{
    struct tree_node *t;

    t = tree_first(n->providedby->root);

    while (t) {
        if (!strcmp(name, get_filename(t->n->name))
            && !tree_search_node(all, t->n->name))
            tree_insert(all, t->n);

        t = tree_next(t);
    }

    return tree_count(all);
}

static void search_requirements(struct hash *hash, struct node *n,
                                struct tree *d)
{
    struct tree_node *t, *u;
    struct tree *providers;

    t = tree_first(n->need->root);

    while (t) {
        providers = tree_new();

        get_all_providers(t->n, providers);

        u = tree_first(providers->root);

        while (u) {
            if (!tree_search_node(d, u->n->name))
                tree_insert(d, hash_search(hash, u->n->name));

            u = tree_next(u);
        }

        tree_free_all_nodes(providers);
        tree_free(providers);

        t = tree_next(t);
    }

    t = tree_first(n->provide->root);

    while (t) {
        search_requirements(hash, t->n, d);
        t = tree_next(t);
    }
}

static int print_requirements(struct hash *hash, struct node *n)
{
    struct tree *t;
    struct tree_node *e;

    t = tree_new();

    search_requirements(hash, n, t);

    e = tree_first(t->root);

    while (e) {
        if (strcmp(n->name, e->n->name) != 0)
            puts(e->n->name);

        e = tree_next(e);
    }

    tree_free(t);

    return 0;
}

int print_needs(struct hash *hash, char *name)
{
    struct tree_node *t, *v;
    struct node *n;
    struct tree *all, *vf;
    char *pkgname;
    int count;
    char p;

    if ((n = hash_search(hash, name)) == NULL) {
        fprintf(stderr,
                "bee-dep: print_needs: cannot find \"%s\"\n",
                name);
        return 1;
    }

    if (IS_PKG(n))
        return print_requirements(hash, n);

    vf    = tree_new();
    count = 1;

    if (is_virtual_file(n->name))
        tree_insert(vf, n);
    else
        count = get_virtual_files(n, name, vf);

    if (!count) {
        fprintf(stderr, "bee-dep: could not get virtual file for \"%s\"\n",
                name);
        tree_free(vf);
        return 1;
    }

    v = tree_first(vf->root);

    while (v) {
        all = tree_new();

        t = tree_first(v->n->need->root);

        while (t) {
            get_all_providers(t->n, all);
            t = tree_next(t);
        }

        t = tree_first(all->root);
        p = (t && count > 1);

        if (p) {
            pkgname = get_pkgname(v->n->name);
            printf("%s:\n", pkgname);
            free(pkgname);
        }

        while (t) {
            puts(t->n->name);
            t = tree_next(t);
        }

        tree_free_all_nodes(all);
        tree_free(all);

        v = tree_next(v);

        if (v && p)
            puts("");
    }

    tree_free(vf);

    return 0;
}

int print_neededby(struct hash *hash, char *name)
{
    struct node *n;
    struct tree_node *t;
    struct tree *all;

    if ((n = hash_search(hash, name)) == NULL) {
        fprintf(stderr,
                "bee-dep: print_needs: cannot find \"%s\"\n",
                name);
        return 1;
    }

    if (IS_PKG(n))
        return print_dependencies(hash, n);

    all = tree_new();

    t = tree_first(n->neededby->root);

    while (t) {
        get_all_providers(t->n, all);
        t = tree_next(t);
    }

    t = tree_first(all->root);

    while (t) {
        puts(t->n->name);
        t = tree_next(t);
    }

    tree_free_all_nodes(all);
    tree_free(all);

    return 0;
}

void list_packages(struct hash *hash)
{
    int i;
    struct tree_node *t;

    for (i = 0; i < TBLSIZE; i++) {
        t = tree_first(hash->tbl[i]->root);

        while (t) {
            if (IS_PKG(t->n))
                puts(t->n->name);
            t = tree_next(t);
        }
    }
}

int count_packages(struct hash *hash)
{
    int i, c;
    struct tree_node *t;

    c = 0;

    for (i = 0; i < TBLSIZE; i++) {
        t = tree_first(hash->tbl[i]->root);

        while (t) {
            if (IS_PKG(t->n))
                c++;
            t = tree_next(t);
        }
    }

    return c;
}

int save_cache(struct hash *hash, char *path)
{
    int i;
    struct tree_node *s, *t;
    FILE *file;

    if ((file = fopen(path, "w")) == NULL) {
        perror("bee-dep: save_cache: fopen");
        return 1;
    }

    for (i = 0; i < TBLSIZE; i++) {
        if (hash->tbl[i]->root) {
            t = tree_first(hash->tbl[i]->root);

            while (t) {
                fprintf(file, "%s %s\n", t->n->name, t->n->type);
                t = tree_next(t);
            }
        }
    }

    fprintf(file, "#\n");

    for (i = 0; i < TBLSIZE; i++) {
        if (hash->tbl[i]->root) {
            t = tree_first(hash->tbl[i]->root);

            while (t) {
                if (t->n->need->root) {
                    s = tree_first(t->n->need->root);

                    while (s) {
                        fprintf(file, "%s %s n\n", t->n->name, s->n->name);
                        s = tree_next(s);
                    }
                }

                if (t->n->provide->root) {
                    s = tree_first(t->n->provide->root);

                    while (s) {
                        fprintf(file, "%s %s p\n", t->n->name, s->n->name);
                        s = tree_next(s);
                    }
                }

                t = tree_next(t);
            }
        }
    }

    if (fclose(file) == EOF) {
        perror("bee-dep: save_cache: fclose");
        return 1;
    }

    return 0;
}

struct hash *load_cache(char *filename)
{
    char line[LINE_MAX],
         a[NODENAME_MAX],
         b[NODENAME_MAX];
    char c;
    struct node *k, *l;
    int line_cnt;
    FILE *file;
    struct hash *hash;

    if ((file = fopen(filename, "r")) == NULL) {
        perror("bee-dep: load_cache: fopen");
        return NULL;
    }

    hash = hash_new();

    line_cnt = 0;

    while (fgets(line, LINE_MAX, file)) {
        line_cnt++;

        if (line[0] == '#')
            break;

        if (sscanf(line, "%s %s", a, b) == EOF) {
            fprintf(stderr, "beedep: load_cache: "
                            "cache file is broken (line %d)\n", line_cnt);
            hash_free(hash);
            return NULL;
        }

        hash_insert(hash, node_new(a, b));
    }

    while (fgets(line, LINE_MAX, file)) {
        line_cnt++;

        if (sscanf(line, "%s %s %c", a, b, &c) == EOF) {
            fprintf(stderr, "beedep: load_cache: "
                            "cache file is broken (line %d)\n", line_cnt);
            hash_free(hash);
            return NULL;
        }

        k = hash_search(hash, a);
        l = hash_search(hash, b);

        if (!k || !l) {
            fprintf(stderr, "beedep: load_cache: "
                            "cache file is broken (line %d)\n", line_cnt);
            hash_free(hash);
            return NULL;
        }

        if (c == 'n') {
            tree_insert(k->need, l);
            tree_insert(l->neededby, k);
        } else if (c == 'p') {
            tree_insert(k->provide, l);
            tree_insert(l->providedby, k);
        } else {
            fprintf(stderr, "beedep: load_cache: "
                            "cache file is broken (line %d)\n", line_cnt);
            hash_free(hash);
            return NULL;
        }
    }

    if (fclose(file) == EOF) {
        perror("bee-dep: load_cache: fclose");
        hash_free(hash);
        return NULL;
    }

    return hash;
}

void remove_all(struct hash *hash, struct node *n)
{
    struct tree *t;
    struct tree_node *h, *s;

    if (n->neededby->root || n->providedby->root)
        return;

    h = tree_first(n->provide->root);

    while (h) {
        s = tree_search_tree_node(h->n->providedby, n);
        tree_node_free(h->n->providedby, s);
        remove_all(hash, h->n);
        h = tree_next(h);
    }

    h = tree_first(n->need->root);

    while (h) {
        s = tree_search_tree_node(h->n->neededby, n);
        tree_node_free(h->n->neededby, s);
        remove_all(hash, h->n);
        h = tree_next(h);
    }

    t = hash->tbl[hash_index(n->name)];
    s = tree_search_tree_node(t, n);
    tree_node_free(t, s);

    node_free(n);
}

int remove_package(struct hash *hash, char *pkgname)
{
    struct node *n;

    if ((n = hash_search(hash, pkgname)) == NULL) {
        fprintf(stderr,
                "bee-dep: remove_package: cannot find \"%s\"\n",
                pkgname);
        return 1;
    }

    if (!IS_PKG(n)) {
        fprintf(stderr,
                "bee-dep: remove_package: \"%s\": no such package\n",
                pkgname);
        return 1;
    }

    remove_all(hash, n);

    return 0;
}

int print_conflicts(struct hash *hash)
{
    int i;
    struct tree_node *t, *s;
    char *pkgname;

    for (i = 0; i < TBLSIZE; i++) {
        t = tree_first(hash->tbl[i]->root);

        while (t) {
            if (!IS_FILE(t->n->name) || IS_DIR(t->n)
                || tree_count(t->n->providedby) < 2) {
                t = tree_next(t);
                continue;
            }

            printf("%s: ", t->n->name);

            s = tree_first(t->n->providedby->root);

            while (s) {
                if (IS_PKG(s->n)) {
                    printf("%s", s->n->name);
                } else if (is_virtual_file(s->n->name)) {
                    pkgname = get_pkgname(s->n->name);
                    printf("%s", pkgname);
                    free(pkgname);
                } else {
                    fprintf(stderr, "bee-dep: print_conflicts: "
                            "could not get pkgname for \"%s\"\n",
                            s->n->name);
                    return 1;
                }

                s = tree_next(s);

                if (!s)
                    puts("");
                else
                    printf(" ");
            }

            t = tree_next(t);
        }
    }

    return 0;
}

int print_not_cached(struct hash *hash, char *filename, char print)
{
    int c;
    FILE *file;
    char lookup[PATH_MAX];

    c = 0;

    if ((file = fopen(filename, "r")) == NULL) {
        perror("bee-dep: print_not_cached: fopen");
        return -1;
    }

    while (fgets(lookup, PATH_MAX, file) != NULL) {
        /* skip empty lines */
        if (lookup[0] == '\n')
            continue;

        /* remove trailing '\n' */
        lookup[strlen(lookup) - 1] = '\0';

        if (hash_search(hash, lookup))
            continue;

        if (print)
            puts(lookup);

        c++;
    }

    if (fclose(file) == EOF) {
        perror("bee-dep: print_not_cached: fclose");
        return -1;
    }

    return c;
}
