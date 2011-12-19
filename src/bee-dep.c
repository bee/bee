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

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <sys/file.h>
#include <libgen.h>
#include <unistd.h>
#include <getopt.h>

#include "graph.h"

#define CACHENAME "index.db"

static void usage(void)
{
     printf("bee-dep v%s 2011\n"
            "  by Matthias Ruester and Lucas Schwass\n"
            "     Max Planck Institute for Molecular Genetics Berlin Dahlem\n\n"

            "Usage: bee dep <OPTION> [PKGNAME]\n\n"

            "Options:\n"
            "    --rebuild                      rebuild the cache\n"
            "    --update          <PKGNAME>    update the cache\n"
            "    --print-removable <PKGNAME>    print removable files "
                                               "from package\n"
            "    --remove          <PKGNAME>    remove package from cache\n",
            getenv("BEE_VERSION"));
}

int init_cache(struct hash *graph, char *bee_metadir, char *filename)
{
    struct dirent **package;
    int i, pkg_cnt;
    char path[PATH_MAX + 1];
    struct stat st;

    if ((pkg_cnt = scandir(bee_metadir, &package, 0, alphasort)) < 0) {
        perror("bee-dep: create_cache: scandir");
        return EXIT_FAILURE;
    }

    /* skip . and .. */
    free(package[0]);
    free(package[1]);

    for (i = 2; i < pkg_cnt; i++) {
        sprintf(path, "%s/%s", bee_metadir, package[i]->d_name);

        if (stat(path, &st) == -1) {
            perror("bee-dep: create_cache: stat");
            return EXIT_FAILURE;
        }

        if (S_ISDIR(st.st_mode)) {
            strcat(path, "/DEPENDENCIES");

            if (stat(path, &st) == -1) {
                fprintf(stderr,
                        "bee-dep: create_cache: missing "
                        "DEPENDENCIES file for package '%s'\n",
                        package[i]->d_name);
                return EXIT_FAILURE;
            }

            if (graph_insert_nodes(graph, path) == EXIT_FAILURE)
                return EXIT_FAILURE;
        }

        free(package[i]);
    }

    free(package);

    return save_cache(graph, filename);
}

void unlock(FILE *cache)
{
    if (flock(fileno(cache), LOCK_UN) == -1) {
        perror("bee-dep: unlock: flock");
        exit(EXIT_FAILURE);
    }

    if (fclose(cache) == EOF) {
        perror("bee-dep: unlock: fclose");
        exit(EXIT_FAILURE);
    }
}

void cleanup_and_exit(struct hash *h, FILE *f, int r)
{
    if (h)
        hash_free(h);

    if (f)
        unlock(f);

    exit(r);
}

void get_bee_variables(char **bee_cachedir, char **bee_metadir)
{
    if (!(*bee_cachedir = getenv("BEE_CACHEDIR"))) {
        fprintf(stderr, "BEE-ERROR: BEE_CACHEDIR not set\n");
        exit(EXIT_FAILURE);
    }

    if (!(*bee_metadir = getenv("BEE_METADIR"))) {
        fprintf(stderr, "BEE-ERROR: BEE_METADIR not set\n");
        exit(EXIT_FAILURE);
    }
}

static FILE *open_and_lock(char *filename, char *mode)
{
    FILE *f;

    if ((f = fopen(filename, mode)) == NULL) {
        perror("bee-dep: fopen");
        exit(EXIT_FAILURE);
    }

    if (flock(fileno(f), LOCK_EX) == -1) {
        perror("bee-dep: flock");
        exit(EXIT_FAILURE);
    }

    return f;
}

int main(int argc, char *argv[])
{
    int c, help, rebuild, update, remove, print, options;
    char found;
    char cachefile[PATH_MAX + 1], path[PATH_MAX + 1], tmp[PATH_MAX + 1];
    char *bee_metadir, *bee_cachedir, *dir, *pkgname;
    struct hash *graph;
    struct stat st;
    FILE *cache;
    struct node *h;

    struct option long_options[] = {
        {"help",            0, &help,    1},
        {"rebuild",         0, &rebuild, 1},
        {"update",          0, &update,  1},
        {"remove",          0, &remove,  1},
        {"print-removable", 0, &print,   1},
        {0, 0, 0, 0}
    };

    if (!getenv("BEE_VERSION")) {
        fprintf(stderr, "BEE-ERROR: please call beedep from bee\n");
        exit(EXIT_FAILURE);
    }

    get_bee_variables(&bee_cachedir, &bee_metadir);

    if (argc == 1) {
        usage();
        exit(EXIT_FAILURE);
    }

    help    = rebuild = update = remove = print = options = 0;
    pkgname = NULL;

    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (c) {
            case '?':
                usage();
                exit(EXIT_FAILURE);
                break;

            default:
                options++;
        }
    }

    if (help) {
        usage();
        exit(EXIT_SUCCESS);
    }

    if (argc != optind + !rebuild) {
        if (argc > optind + !rebuild)
            fprintf(stderr, "bee-dep: too many arguments\n");
        else
            fprintf(stderr, "bee-dep: pkgname needed\n");

        exit(EXIT_FAILURE);
    }

    if (!rebuild)
        pkgname = argv[optind];

    if (!options) {
        fprintf(stderr, "bee-dep: no option specified\n");
        exit(EXIT_FAILURE);
    }

    if (options > 1 && (options != 2 || !(remove && print))) {
        fprintf(stderr, "bee-dep: too many options specified\n");
        exit(EXIT_FAILURE);
    }

    if (sprintf(cachefile, "%s/%s", bee_cachedir, CACHENAME) < 0
        || sprintf(tmp, "%s/index.tmp", bee_cachedir) < 0) {
        perror("bee-dep: sprintf");
        exit(EXIT_FAILURE);
    }

    dir = strdup(cachefile);
    dir = dirname(dir);

    if (stat(dir, &st) == -1 && mkdir(dir, 0755) == -1) {
        perror("bee-dep: mkdir");
        exit(EXIT_FAILURE);
    }

    free(dir);

    graph = NULL;

    found = (stat(cachefile, &st) != -1 && S_ISREG(st.st_mode));

    graph = hash_new();

    if (rebuild) {
        if (init_cache(graph, bee_metadir, tmp) == EXIT_FAILURE)
            cleanup_and_exit(graph, NULL, EXIT_FAILURE);

        cache = open_and_lock(cachefile, "w");

        if (rename(tmp, cachefile) == -1) {
            perror("bee-dep: rename");
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }

        cleanup_and_exit(graph, cache, EXIT_SUCCESS);
    }

    if (found) {
        cache = open_and_lock(cachefile, "r");

        if (load_cache(graph, cache) == EXIT_FAILURE)
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
    } else {
        if (init_cache(graph, bee_metadir, tmp) == EXIT_FAILURE)
            cleanup_and_exit(graph, NULL, EXIT_FAILURE);

        cache = open_and_lock(cachefile, "w");

        if (rename(tmp, cachefile) == -1) {
            perror("bee-dep: rename");
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }
    }

    if (update) {
        found = !!hash_search(graph, pkgname);

        if (sprintf(path, "%s/%s/DEPENDENCIES",
                    bee_metadir, pkgname) < 0) {
            perror("bee-dep: sprintf");
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }

        if (stat(path, &st) != -1) {
            if (found) {
                fprintf(stderr, "bee-dep: package '%s' is "
                        "already in the cache\n", pkgname);
                cleanup_and_exit(graph, cache, EXIT_SUCCESS);
            }

            if (graph_insert_nodes(graph, path) == EXIT_FAILURE)
                cleanup_and_exit(graph, cache, EXIT_FAILURE);
        } else {
            if (!found) {
                fprintf(stderr,
                        "bee-dep: unknown package '%s'\n", pkgname);
                cleanup_and_exit(graph, cache, EXIT_FAILURE);
            }

            if ((h = hash_search(graph, pkgname)) == NULL || !IS_PKG(h)) {
                fprintf(stderr, "bee-dep: unknown package '%s'\n", pkgname);
                cleanup_and_exit(graph, cache, EXIT_FAILURE);
            }

            if (remove_package(graph, pkgname) == EXIT_FAILURE)
                cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }

        if (save_cache(graph, tmp) == EXIT_FAILURE)
            cleanup_and_exit(graph, cache, EXIT_FAILURE);

        if (rename(tmp, cachefile) == -1) {
            perror("bee-dep: rename");
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }

        cleanup_and_exit(graph, cache, EXIT_SUCCESS);
    }

    if ((h = hash_search(graph, pkgname)) == NULL || !IS_PKG(h)) {
        fprintf(stderr, "bee-dep: unknown package '%s'\n", pkgname);
        cleanup_and_exit(graph, cache, EXIT_FAILURE);
    }

    if (print) {
        if (print_removable(graph, pkgname) == EXIT_FAILURE)
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
    }

    if (remove) {
        if (remove_package(graph, pkgname) == EXIT_FAILURE)
            cleanup_and_exit(graph, cache, EXIT_FAILURE);

        if (save_cache(graph, tmp) == EXIT_FAILURE)
            cleanup_and_exit(graph, cache, EXIT_FAILURE);

        if (rename(tmp, cachefile) == -1) {
            perror("bee-dep: rename");
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }
    }

    cleanup_and_exit(graph, cache, EXIT_SUCCESS);

    return EXIT_FAILURE;
}
