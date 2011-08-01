#ifndef DEPENDENCIES_GRAPH_H
#define DEPENDENCIES_GRAPH_H

#define MAIN_FILE "all.dep"

#define WITH_SELF    1
#define WITHOUT_SELF 0

#define INDIRECT 0
#define DIRECT   1

#define FROM_PACKAGE 0
#define TO_PACKAGE   1

struct tree {
    struct tree_node *root;
};

struct tree_node {
    struct graph_node *node;
    struct tree_node *left,
                     *right,
                     *parent;
    unsigned char max_depth;
    char balance_value;
};

struct graph_node {
    char *name,
         *type;
    struct tree need,
                neededby,
                provide,
                providedby;
};

extern struct tree *tree_new(void);
extern void tree_delete(struct tree *t);
extern void tree_add_graph_nodes_from_file(struct tree *t, char *filename, char quiet);
extern void tree_save(struct tree *t);
extern void tree_remove_graph_node(struct tree *t, char *name, char quiet);
extern void tree_update_graph_node(struct tree *t, char *new, char *old, char quiet);
extern void print_from_which_this_package_depends(struct tree *t, char *name, char depth);
extern void print_which_packages_depended_on_this(struct tree *t, char *name, char depth);
extern void print_provides(struct tree *t, char *name);
extern void print_files_which_exclusively_provided_by_package(struct tree *t, char *name);
extern void print_count_of_packets_provied_this_file(struct tree *t, char *name);
extern void print_packages_provide_this_file(struct tree* t, char *name);
extern void print_packages_needs_this_file(struct tree* t, char *name);
extern void read_database(struct tree *t, char *filename);
extern void print_conflicts_with_other_packages(struct tree *t, char *name);

#endif
