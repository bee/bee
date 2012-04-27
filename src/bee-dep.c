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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#include "graph.h"

#define CACHENAME "index.db"
#define TMPNAME   "index.tmp"
#define LOCKNAME  "index.lock"

#define REBUILD   1
#define UPDATE    2
#define REMOVE    3
#define LIST      4
#define CONFLICTS 5

static char *bee_version(void)
{
    static char *bee_v = NULL;

    if (!bee_v)
        bee_v = getenv("BEE_VERSION");

    return bee_v;
}

static char *bee_metadir(void)
{
    static char *bee_md = NULL;

    if (!bee_md)
        bee_md = getenv("BEE_METADIR");

    return bee_md;
}

static char *bee_cachedir(void)
{
    static char *bee_cd = NULL;

    if (!bee_cd)
        bee_cd = getenv("BEE_CACHEDIR");

    return bee_cd;
}

static void get_bee_variables(void)
{
    if (!bee_version()) {
        fprintf(stderr, "BEE-ERROR: please call bee-dep from bee\n");
        exit(EXIT_FAILURE);
    }

    if (!bee_metadir()) {
        fprintf(stderr, "BEE-ERROR: BEE_METADIR not set\n");
        exit(EXIT_FAILURE);
    }

    if (!bee_cachedir()) {
        fprintf(stderr, "BEE_ERROR: BEE_CACHEDIR not set\n");
        exit(EXIT_FAILURE);
    }
}

static char *cache_filename(void)
{
    static char cache[PATH_MAX + 1] = {0};

    if (!cache[0])
        sprintf(cache, "%s/%s", bee_cachedir(), CACHENAME);

    return cache;
}

static char *lock_filename(void)
{
    static char lock[PATH_MAX + 1] = {0};

    if (!lock[0])
        sprintf(lock, "%s/%s", bee_cachedir(), LOCKNAME);

    return lock;
}

static void usage_header(void)
{
    printf("bee-dep v%s 2011-2012\n"
           "  by Matthias Ruester and Lucas Schwass\n"
           "     Max Planck Institute for Molecular Genetics Berlin Dahlem\n\n",
           bee_version());
}

static void usage(void)
{
    usage_header();
    puts("Usage: bee dep <command> [<args>]\n\n"

         "Commands:\n"
         "    rebuild      rebuild the cache\n"
         "    update       update the cache for a specific package\n"
         "    remove       remove a package from the cache\n"
         "    list         list information\n"
         "    conflicts    show conflicting packages\n\n"

         "See 'bee dep <command> --help' for more information on a specific command.");
}

static void usage_rebuild(void)
{
    usage_header();
    puts("Usage: bee dep rebuild\n\n"

         "Rebuild the cache.\n");
}

static void usage_update(void)
{
    usage_header();
    puts("Usage: bee dep update <pkgname>\n\n"

         "Update the information about a package.\n");
}

static void usage_remove(void)
{
    usage_header();
    puts("Usage: bee dep remove [options] <pkgname>\n\n"

         "Remove a package from the cache.\n\n"

         "Options:\n"
         "    --print    print which files can be deleted from the hard drive\n");
}

static void usage_list(void)
{
    usage_header();
    puts("Usage: bee dep list [options]\n\n"

         "Get information from the cache.\n\n"

         "Options:\n"
         "    --packages                   list all packages\n"
         "    --files        <pkg>         list all files of a package\n"
         "    --depending-on <pkg|file>    list packages depending on a pkg or file\n"
         "    --required-by  <pkg|file>    list packages required by a pkg or file\n"
         "    --removable    <pkg>         show all removable files of a package\n"
         "    --provider-of  <file>        show the providers of a file\n"
         "    --not-cached   <file>        print files which are not in the cache;\n"
         "                                 check those files which are listed in <file>\n"
         "    --broken                     list all broken dependencies\n"
         "    --count                      do not print results; just count\n");
}

static void usage_conflicts(void)
{
    usage_header();
    puts("Usage: bee dep conflicts\n\n"

         "Print conflicting packages.\n");
}

/* create all directories in path with mode mode */
int mkdirp(char *path, mode_t mode)
{
    char *dir, *pdir, *end;
    int ret;

    assert(path);

    dir = end = path;

    while (*dir) {
        /* skip "/" */
        dir = end + strspn(end, "/");

        /* skip non-"/" */
        end = dir + strcspn(dir, "/");

        /* grab everything in path till current end */
        if (!(pdir = strndup(path, end - path)))
            return -1;

        /* create the directory ; ignore err if it already exists */
        ret = mkdir(pdir, mode);

        free(pdir);

        if (ret == -1 && errno != EEXIST)
            return -1;
    }

    return 0;
}

void ensure_directories(void)
{
    if (mkdirp(bee_cachedir(), 0755) == -1) {
        perror("bee-dep: mkdirp");
        exit(EXIT_FAILURE);
    }
}

static struct hash *init_cache(void)
{
    struct dirent **package;
    int i, pkg_cnt;
    char path[PATH_MAX + 1];
    struct stat st;
    struct hash *graph;

    if (stat(bee_metadir(), &st) == -1)
        return NULL;

    if ((pkg_cnt = scandir(bee_metadir(), &package, 0, alphasort)) < 0) {
        perror("bee-dep: init_cache: scandir");
        return NULL;
    }

    graph = hash_new();

    /* skip . and .. */
    free(package[0]);
    free(package[1]);

    for (i = 2; i < pkg_cnt; i++) {
        sprintf(path, "%s/%s", bee_metadir(), package[i]->d_name);

        if (stat(path, &st) == -1) {
            perror("bee-dep: init_cache: stat");
            hash_free(graph);
            return NULL;
        }

        if (S_ISDIR(st.st_mode)) {
            strcat(path, "/DEPENDENCIES");

            if (stat(path, &st) == -1) {
                fprintf(stderr,
                        "bee-dep: init_cache: missing "
                        "DEPENDENCIES file for package \"%s\"\n",
                        package[i]->d_name);
                hash_free(graph);
                return NULL;
            }

            if (graph_insert_nodes(graph, path)) {
                hash_free(graph);
                return NULL;
            }
        }

        free(package[i]);
    }

    free(package);

    if (save_cache(graph, cache_filename())) {
        hash_free(graph);
        return NULL;
    }

    return graph;
}

static int update_cache(struct hash *graph)
{
    struct dirent **package;
    int i, pkg_cnt;
    char path[PATH_MAX + 1];
    struct stat st;
    struct tree_node *t;

    if (stat(bee_metadir(), &st) == -1)
        return 0;

    if ((pkg_cnt = scandir(bee_metadir(), &package, 0, alphasort)) < 0) {
        perror("bee-dep: update_cache: scandir");
        return 1;
    }

    /* skip . and .. */
    free(package[0]);
    free(package[1]);

    /* add new (not known) packages */
    for (i = 2; i < pkg_cnt; i++) {
        if (hash_search(graph, package[i]->d_name))
            continue;

        printf("adding %s\n", package[i]->d_name);

        if (sprintf(path, "%s/%s", bee_metadir(), package[i]->d_name) < 0) {
            perror("bee-dep: update_cache: sprintf");
            return 1;
        }

        if (stat(path, &st) == -1) {
            perror("bee-dep: update_cache: stat");
            return 1;
        }

        if (S_ISDIR(st.st_mode)) {
            strcat(path, "/DEPENDENCIES");

            if (stat(path, &st) == -1) {
                fprintf(stderr,
                        "bee-dep: update_cache: missing "
                        "DEPENDENCIES file for package \"%s\"\n",
                        package[i]->d_name);
                return 1;
            }

            if (graph_insert_nodes(graph, path))
                return 1;
        }

        free(package[i]);
    }

    free(package);

    /* remove packages which not exist anymore */
    for (i = 0; i < TBLSIZE; i++) {
        t = tree_first(graph->tbl[i]->root);

        while (t) {
            if (IS_PKG(t->n)) {
                if (sprintf(path, "%s/%s", bee_metadir(), t->n->name) < 0) {
                    perror("bee-dep: update_cache: sprintf");
                    return 1;
                }

                if (stat(path, &st) == -1) {
                    printf("removing %s\n", t->n->name);

                    if (remove_package(graph, t->n->name))
                        return 1;
                }
            }

            t = tree_next(t);
        }
    }

    return 0;
}

struct hash *get_cache(void)
{
    struct hash *graph;
    struct stat st;

    if (stat(cache_filename(), &st) == -1)
        graph = init_cache();
    else
        graph = load_cache(cache_filename());

    if (!graph)
        exit(EXIT_FAILURE);

    return graph;
}

static int bee_dep_rebuild(int argc, char *argv[])
{
    int c, help;
    struct hash *graph;
    struct option long_options[] = {
        {"help", 0, &help, 1},
        {0, 0, 0, 0}
    };

    help = 0;

    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (c) {
            case '?':
                usage_update();
                return 1;
        }
    }

    if (help) {
        usage_rebuild();
        return 0;
    }

    if (argc > 1) {
        fprintf(stderr, "bee-dep: too many arguments\n");
        return 1;
    }

    graph = init_cache();
    hash_free(graph);

    return 0;
}

static int bee_dep_update(int argc, char *argv[])
{
    int c, help;
    char *pkg;
    char path[PATH_MAX + 1];
    struct hash *graph;
    struct stat st;
    struct option long_options[] = {
        {"help", 0, &help, 1},
        {0, 0, 0, 0}
    };

    help = 0;

    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (c) {
            case '?':
                usage_update();
                return 1;
        }
    }

    if (help) {
        usage_update();
        return 0;
    }

    if (argc == 1) {
        graph = get_cache();

        if (update_cache(graph) || save_cache(graph, cache_filename())) {
            hash_free(graph);
            return 1;
        }

        hash_free(graph);
        return 0;
    }

    if (argc < 2) {
        fprintf(stderr, "bee-dep: pkgname needed\n");
        return 1;
    }

    if (argc > 2) {
        fprintf(stderr, "bee-dep: too many arguments\n");
        return 1;
    }

    pkg = argv[1];

    if (sprintf(path, "%s/%s/DEPENDENCIES", bee_metadir(), pkg) < 0) {
        perror("bee-dep: sprintf");
        return 1;
    }

    graph = get_cache();

    if (stat(path, &st) != -1) {
        if (hash_search(graph, pkg)) {
            hash_free(graph);
            return 0;
        }

        if (graph_insert_nodes(graph, path)) {
            hash_free(graph);
            return 1;
        }
    } else {
        if (remove_package(graph, pkg)) {
            hash_free(graph);
            return 1;
        }
    }

    if (save_cache(graph, cache_filename())) {
        hash_free(graph);
        return 1;
    }

    hash_free(graph);
    return 0;
}

static int bee_dep_remove(int argc, char *argv[])
{
    int c, help, print;
    struct hash *graph;
    char *pkg;
    struct option long_options[] = {
        {"help",  0, &help,  1},
        {"print", 0, &print, 1},
        {0, 0, 0, 0}
    };

    help = print = 0;

    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (c) {
            case '?':
                usage_remove();
                return 1;
        }
    }

    if (help) {
        usage_remove();
        return 0;
    }

    if (optind == argc) {
        fprintf(stderr, "bee-dep: pkgname needed\n");
        return 1;
    }

    if (optind < argc - 1) {
        fprintf(stderr, "bee-dep: too many arguments\n");
        return 1;
    }

    pkg   = argv[optind];
    graph = get_cache();

    if (print && print_removable(graph, pkg)) {
        hash_free(graph);
        return 1;
    }

    if (remove_package(graph, pkg)
        || save_cache(graph, cache_filename())) {
        hash_free(graph);
        return 1;
    }

    hash_free(graph);
    return 0;
}

static int bee_dep_list(int argc, char *argv[])
{
    int c, i, opt_count, help, files, packages, count,
        depending_on, required_by, removable, provider_of,
        not_cached, broken;
    struct hash *graph;
    char *name;
    struct option long_options[] = {
        {"help",         0, &help,         1},
        {"files",        0, &files,        1},
        {"packages",     0, &packages,     1},
        {"count",        0, &count,        1},
        {"depending-on", 0, &depending_on, 1},
        {"required-by",  0, &required_by,  1},
        {"removable",    0, &removable,    1},
        {"provider-of",  0, &provider_of,  1},
        {"not-cached",   0, &not_cached,   1},
        {"broken",       0, &broken,       1},
        {0, 0, 0, 0}
    };

    opt_count = help         = files       = packages  = provider_of =
    count     = depending_on = required_by = removable = not_cached  =
    broken    = 0;

    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (c) {
            case '?':
                usage_list();
                return 1;

            default:
                opt_count++;
        }
    }

    if (help) {
        usage_list();
        return 0;
    }

    if (!opt_count && optind != argc) {
        fprintf(stderr, "bee-dep: too many arguments\n");
        return 1;
    }

    if (!opt_count)
        packages = 1;

    if (opt_count > 1 && !count) {
        fprintf(stderr, "bee-dep: too many options specified\n");
        return 1;
    }

    if (packages) {
        graph = get_cache();

        if (count)
            printf("%d\n", count_packages(graph));
        else
            list_packages(graph);

        hash_free(graph);
        return 0;
    }

    if (broken && optind < argc) {
        fprintf(stderr, "bee-dep: too many arguments\n");
        return 1;
    }

    if (optind == argc && !broken) {
        fprintf(stderr, "bee-dep: arguments needed\n");
        return 1;
    }

    if (count && (depending_on || required_by))
        fprintf(stderr, "bee-dep: ignoring option --count\n");

    graph = get_cache();

    for (i = optind; i < argc; i++) {
        name = argv[i];

        if (optind < argc - 1)
            printf("%s:\n", name);

        if (files) {
            if (count) {
                c = count_files(graph, name);

                if (c < 0) {
                    hash_free(graph);
                    return 1;
                }

                printf("%d\n", c);
            } else if (list_files(graph, name)) {
                 hash_free(graph);
                 return 1;
            }
        }

        if (removable) {
            if (count) {
                c = count_removable(graph, name);

                if (c < 0) {
                    hash_free(graph);
                    return 1;
                }

                printf("%d\n", c);
            } else if (print_removable(graph, name)) {
                hash_free(graph);
                return 1;
            }
        }

        if (depending_on && print_neededby(graph, name)) {
            hash_free(graph);
            return 1;
        }

        if (required_by && print_needs(graph, name)) {
            hash_free(graph);
            return 1;
        }

        if (provider_of) {
            if (count) {
                c = count_providers(graph, name);

                if (c < 0) {
                    hash_free(graph);
                    return 1;
                }

                printf("%d\n", c);
            } else if (print_providers(graph, name)) {
                hash_free(graph);
                return 1;
            }
        }

        if (not_cached) {
            c = print_not_cached(graph, name, !count);

            if (c < 0) {
                hash_free(graph);
                return 1;
            }

            if (count)
                printf("%d\n", c);
        }
    }

    if (broken) {
        c = print_broken(graph, !count);

        if (c < 0) {
            hash_free(graph);
            return 1;
        }

        if (count)
            printf("%d\n", c);
    }

    hash_free(graph);
    return 0;
}

static int bee_dep_conflicts(int argc, char *argv[])
{
    int c, opt_count, help;
    struct hash *graph;
    struct option long_options[] = {
        {"help", 0, &help, 1},
        {0, 0, 0, 0}
    };

    help = opt_count = 0;

    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (c) {
            case '?':
                usage_conflicts();
                return 1;

            default:
                opt_count++;
        }
    }

    if (help) {
        usage_conflicts();
        return 0;
    }

    if (optind < argc) {
        fprintf(stderr, "bee-dep: too many arguments\n");
        return 1;
    }

    graph = get_cache();

    if (print_conflicts(graph)) {
        hash_free(graph);
        return 1;
    }

    hash_free(graph);
    return 0;
}

static int get_command(char *command)
{
    if (!strcmp("rebuild", command))
        return REBUILD;

    if (!strcmp("update", command))
        return UPDATE;

    if (!strcmp("remove", command))
        return REMOVE;

    if (!strcmp("list", command))
        return LIST;

    if (!strcmp("conflicts", command))
        return CONFLICTS;

    return 0;
}

static FILE *lockfile(void)
{
    static FILE *file = NULL;

    if (!file) {
        ensure_directories();

        if ((file = fopen(lock_filename(), "w")) == NULL) {
            perror("bee-dep: lockfile");
            exit(EXIT_FAILURE);
        }

        fprintf(file, "locked by pid %d\n", getpid());
        fflush(file);
    }

    return file;
}

void lock(void)
{
    FILE *f;
    struct flock flo;

    f = lockfile();

    flo.l_start  = flo.l_len = 0;
    flo.l_whence = SEEK_SET;
    flo.l_type   = F_WRLCK;

    if (fcntl(fileno(f), F_SETLKW, &flo) == -1) {
        perror("bee-dep: lock");
        exit(EXIT_FAILURE);
    }
}

void unlock(void)
{
    FILE *f;
    struct flock flo;

    f = lockfile();

    if (!f)
        return;

    flo.l_start  = flo.l_len = 0;
    flo.l_whence = SEEK_SET;
    flo.l_type   = F_UNLCK;

    rewind(f);

    if (ftruncate(fileno(f), 0) == -1) {
        perror("bee-dep: unlock: ftruncate");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fileno(f), F_SETLK, &flo) == -1) {
        perror("bee-dep: unlock: fcntl");
        exit(EXIT_FAILURE);
    }

    if (fclose(f) == EOF) {
        perror("bee-dep: unlock: fclose");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    int command;

    get_bee_variables();

    if (argc < 2) {
        usage();
        return 1;
    }

    lock();
    if (atexit(unlock)) {
        perror("bee-dep: atexit");
        return 1;
    }

    command = get_command(argv[1]);

    argv++;
    argc--;

    switch (command) {
        case REBUILD:
            return bee_dep_rebuild(argc, argv);

        case UPDATE:
            return bee_dep_update(argc, argv);

        case REMOVE:
            return bee_dep_remove(argc, argv);

        case LIST:
            return bee_dep_list(argc, argv);

        case CONFLICTS:
            return bee_dep_conflicts(argc, argv);

        default:
            usage();
            return 1;
    }

    return 0;
}
