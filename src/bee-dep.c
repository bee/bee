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

#define _GNU_SOURCE

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
#include <errno.h>
#include <assert.h>

#include "graph.h"

#define CACHENAME "index.db"

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define BEE_METADIR  env_bee_metadir()
#define BEE_CACHEDIR env_bee_cachedir()

static char *env_bee_metadir(void)
{
    static char *value = NULL;

    if(value)
        return value;

    value = getenv("BEE_METADIR");

    if(!value)
        value = "";

    return value;
}

static char *env_bee_cachedir(void)
{
    static char *value = NULL;

    if(value)
        return value;

    value = getenv("BEE_CACHEDIR");

    if(!value)
        value = "";

    return value;
}

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

int init_cache(struct hash *graph, char *filename)
{
    struct dirent **package;
    int i, pkg_cnt;
    char path[PATH_MAX + 1];
    struct stat st;

    /* TODO: need to handle all kinds of race conditions here 8) */

    if ((pkg_cnt = scandir(BEE_METADIR, &package, 0, alphasort)) < 0) {
        perror("bee-dep: create_cache: scandir");
        return 0;
    }

    /* skip . and .. */
    free(package[0]);
    free(package[1]);

    for (i = 2; i < pkg_cnt; i++) {
        sprintf(path, "%s/%s", BEE_METADIR, package[i]->d_name);

        if (stat(path, &st) == -1) {
            perror("bee-dep: create_cache: stat");
            return 0;
        }

        if (S_ISDIR(st.st_mode)) {
            strcat(path, "/DEPENDENCIES");

            if (stat(path, &st) == -1) {
                fprintf(stderr,
                        "bee-dep: create_cache: missing "
                        "DEPENDENCIES file for package '%s'\n",
                        package[i]->d_name);
                return 0;
            }

            if (graph_insert_nodes(graph, path) == EXIT_FAILURE)
                return 0;
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

/* create all directories in path with mode mode */
int mkdirp(char *path, mode_t mode)
{
    char *dir, *pdir, *end;
    int ret;

    assert(path);

    dir = end = path;

    while(*dir) {
        /* skip "/" */
        dir = end + strspn(end, "/");

        /* skip non-"/" */
        end = dir + strcspn(dir, "/");

        /* grab everything in path till current end */
        if(!(pdir = strndup(path, end - path)))
            return -1;

        /* create the directory ; ignore err if it already exists */
        ret = mkdir(pdir, mode);

        free(pdir);

        if(ret == -1 && errno != EEXIST)
            return -1;
  }

  return 0;
}

/*
 * checks if given filename is a regular file
 * returns:
 *
 *   1 : file exists and is a regular file
 *   0 : file is not a regular file or does not exist
 *       check errno for
 *          EEXIST : file exists but is not a regular file
 *          ENOENT : file does not exist
 *  -1 : error; check errno see(stat(2))
 */
int regular_file_exists(char *fname)
{
    struct stat st;
    int ret;

    ret = stat(fname, &st);

    if(likely(ret == 0)) {
        if(likely(S_ISREG(st.st_mode)))
            return 1;

        /* set errno for file exists but is not a regular file */
        errno = EEXIST;
        return 0;
    }

    if (likely(errno == ENOENT))
        return 0;

    return -1;
}

int main(int argc, char *argv[])
{
    int c, help, rebuild, update, remove, print, options;
    int ret;
    char *cachefile = NULL;
    char *depfile   = NULL;
    char found;
    char *pkgname;
    struct hash *graph;
    struct stat st;
    FILE *cache = NULL;
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

    if (mkdirp(BEE_CACHEDIR, 0755) == -1) {
        perror("bee-dep: mkdirp");
        exit(EXIT_FAILURE);
    }

    if(!(graph = hash_new())) {
        perror("bee-dep: hash_new");
        exit(EXIT_FAILURE);
    }

    if(asprintf(&cachefile, "%s/%s", BEE_CACHEDIR, CACHENAME) == -1) {
        perror("bee-dep: asprintf");
        exit(EXIT_FAILURE);
    }

    if (rebuild) {
        int ret = EXIT_SUCCESS;

        if (!init_cache(graph, cachefile))
            ret = EXIT_FAILURE;

        free(cachefile);
        hash_free(graph);
        return ret;
    }

    ret = regular_file_exists(cachefile);

    if (ret == -1 || (ret == 0 && errno != ENOENT)) {
        perror("bee-dep: regular_file_exists(cachefile)");
        free(cachefile);
        cleanup_and_exit(graph, cache, EXIT_FAILURE);
    } else if (ret) {
        cache = open_and_lock(cachefile, "r");

        if (!load_cache(graph, cache)) {
            free(cachefile);
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }
    } else {
        if (!init_cache(graph, cachefile)) {
            free(cachefile);
            hash_free(graph);
            return EXIT_FAILURE;
        }
    }

    if (update) {
        found = !!hash_search(graph, pkgname);

        if (asprintf(&depfile, "%s/%s/DEPENDENCIES",
                    BEE_METADIR, pkgname) == -1) {
            perror("bee-dep: asprintf");
            free(cachefile);
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }

        if (stat(depfile, &st) != -1) {
            if (found) {
                fprintf(stderr, "bee-dep: package '%s' is "
                        "already in the cache\n", pkgname);
                free(cachefile);
                free(depfile);
                cleanup_and_exit(graph, cache, EXIT_SUCCESS);
            }

            if (graph_insert_nodes(graph, depfile) == EXIT_FAILURE) {
                free(cachefile);
                free(depfile);
                cleanup_and_exit(graph, cache, EXIT_FAILURE);
            }
        } else {
            if (!found) {
                fprintf(stderr,
                        "bee-dep: unknown package '%s'\n", pkgname);
                free(cachefile);
                free(depfile);
                cleanup_and_exit(graph, cache, EXIT_FAILURE);
            }

            if ((h = hash_search(graph, pkgname)) == NULL || !IS_PKG(h)) {
                fprintf(stderr, "bee-dep: unknown package '%s'\n", pkgname);
                free(cachefile);
                free(depfile);
                cleanup_and_exit(graph, cache, EXIT_FAILURE);
            }

            if (remove_package(graph, pkgname) == EXIT_FAILURE)
                free(cachefile);
                free(depfile);
                cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }

        if (!save_cache(graph, cachefile)) {
            free(cachefile);
            free(depfile);
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }

        free(cachefile);
        free(depfile);
        cleanup_and_exit(graph, cache, EXIT_SUCCESS);
    }

    if ((h = hash_search(graph, pkgname)) == NULL || !IS_PKG(h)) {
        fprintf(stderr, "bee-dep: unknown package '%s'\n", pkgname);
        free(cachefile);
        cleanup_and_exit(graph, cache, EXIT_FAILURE);
    }

    if (print) {
        if (print_removable(graph, pkgname) == EXIT_FAILURE) {
            free(cachefile);
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }
    }

    if (remove) {
        if (remove_package(graph, pkgname) == EXIT_FAILURE) {
            free(cachefile);
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }

        if (!save_cache(graph, cachefile)) {
            free(cachefile);
            cleanup_and_exit(graph, cache, EXIT_FAILURE);
        }
    }

    free(cachefile);
    cleanup_and_exit(graph, cache, EXIT_SUCCESS);

    return EXIT_FAILURE;
}
