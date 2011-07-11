
#ifndef MX_TREE_H
#define MX_TREE_H 1

struct tree {
    struct tree_node *root;
    struct tree_node *current;

    void   (*free_data)(void *data);

    void * (*generate_key)(void *data);
    void   (*free_key)(void *data);

    int    (*compare_key)(void *a, void *b);
    void   (*print_key)(void *key);
};

struct tree_node {
    struct tree_node *parent;
    struct tree_node *left;
    struct tree_node *right;

    unsigned char height;
    char balance_factor;

    void *key;
    void *data;
};

#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define TREE_HEIGHT(t) ((t) ? ((t)->height) : 0)

struct tree *tree_allocate(void);
void tree_free(struct tree *tree);
struct tree_node *tree_insert(struct tree *tree, void *data);

void *tree_search(struct tree *tree, void *key);
void *tree_delete(struct tree *tree, void *key);

void tree_print(struct tree *tree);

#endif
