#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "graph.h"

void usage(void)
{
    printf("usage: dependencies [FILE...]\n"
           "possible options:\n"
           "    -h             \tprint this help\n"
           "    -q             \tbe quiet\n"
           "    -n             \tdo not really remove/insert/update\n"
           "    -u [old] [new] \tupdate 'old' with 'new' and print conflicts\n"
           "    -i [name]      \tinsert node in graph\n"
           "    -r [name]      \tremove node and print conflicts\n"
           "    -d [name]      \tprint which packages depended on this\n"
           "    -f [name]      \tprint from which this package depends\n"
           "    -a             \tprint all founded packages (only for -d or -f)\n");
    printf("    -p [name]      \tprint proviedes\n"
           "    -w [name]      \tprint conflicts with other packages\n"
           "    -e [name]      \tprint exclusiv provides form pacage\n"
           "    -c [name]      \tprint the count of packages provied the file\n"
           "    -o [name]      \tprint packages provide the file\n"
           "    -k [name]      \tprint packages need the file\n");
}

int main(int argc, char *argv[])
{
    struct tree *tree;
    char c, noop, quiet, all;
    char *name, *old, *new, *insert, *remove, *dep, *from_which, *pro, *provides, *exclusiv, *count, *need, *conflicts;
    struct stat st;

    if (argc == 1) {
        usage();
        return 1;
    }

    name = old = new = insert = remove = dep = from_which = pro = provides = exclusiv = conflicts = count = need = NULL;
    noop = quiet = 0;
    all = 1;

    while ((c = getopt(argc, argv, "u:i:r:d:f:p:e:w:c:o:k:hqna")) != -1) {
        switch (c) {
            case 'h':
                usage();
                return 1;

            case 'q':
                quiet = 1;
                break;

            case 'n':
                noop = 1;
                break;

            case 'a':
                all = 0;
                break;

            case 'u':
                old = optarg;
                break;

            case 'i':
                insert = optarg;
                break;

            case 'r':
                remove = optarg;
                break;

            case 'd':
                dep = optarg;
                break;

            case 'f':
                from_which = optarg;
                break;

            case 'p':
                provides = optarg;
                break;

            case 'e':
                exclusiv = optarg;
                break;

            case 'w':
                conflicts = optarg;
                break;

            case 'c':
                count = optarg;
                break;

            case 'o':
                pro = optarg;
                break;

            case 'k':
                need = optarg;
                break;

            case '?':
                if (optopt == 'u' || optopt == 'i'
                    || optopt == 'r' || optopt == 'd')
                    fprintf(stderr, "option -%c needs an argument\n", optopt);
                else
                    fprintf(stderr, "unknown option %c\n", optopt);

                usage();
                return 1;
        }
    }

    if (old) {
        if (optind != argc - 1) {
            fprintf(stderr, "error: need the new node for update\n");
            usage();
            return 1;
        }

        new = argv[optind];
    } else if (optind != argc) {
        fprintf(stderr, "error: too many arguments\n");
        usage();
        return 1;
    }

    tree = tree_new();

    if (stat(MAIN_FILE, &st) != -1)
        read_database(tree, MAIN_FILE);

    if (insert)
        tree_add_graph_nodes_from_file(tree, insert, quiet);

    if (!tree->root) {
       fprintf(stderr, "error: graph is empty\n");
       tree_delete(tree);
       return 1;
    }

    if (dep)
        print_which_packages_depended_on_this(tree, dep, all);

    if (from_which)
        print_from_which_this_package_depends(tree, from_which, all);

    if (provides)
        print_provides(tree, provides);

    if (remove)
        tree_remove_graph_node(tree, remove, quiet);

    if (old)
        tree_update_graph_node(tree, new, old, quiet);

    if (exclusiv)
        print_files_which_exclusively_provided_by_package(tree, exclusiv);

    if (conflicts)
        print_conflicts_with_other_packages(tree, conflicts);

    if (count)
        print_count_of_packets_provied_this_file(tree, count);

    if (pro)
        print_packages_provide_this_file(tree, pro);

    if (need)
        print_packages_needs_this_file(tree, need);

    if (!noop)
        tree_save(tree);

    tree_delete(tree);

    return 0;
}
