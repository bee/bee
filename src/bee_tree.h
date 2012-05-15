
#ifndef _BEE_BEE_TREE_H
#define _BEE_BEE_TREE_H 1

struct bee_tree {
    struct bee_subtree *root;

    int flags;

    void   (*free_data)(void *data);

    void * (*generate_key)(const void *data);
    void   (*free_key)(void *key);

    int    (*compare_key)(void *a, void *b);
    int    (*compare_data)(void *a, void *b);

    void   (*print_key)(void *key);
    void   (*print)(void *key, void *data);
};

struct bee_subtree {
    struct bee_subtree *parent;
    struct bee_subtree *left;
    struct bee_subtree *right;

    unsigned char height;
    char balance_factor;

    void *key;
    void *data;
};

#define BEE_TREE_MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define BEE_TREE_HEIGHT(t) ((t) ? ((t)->height) : 0)

#define BEE_TREE_FLAG_UNIQUE      (1<<0)
#define BEE_TREE_FLAG_UNIQUE_DATA (1<<1)
#define BEE_TREE_FLAG_COMPARE_DATA_ON_EQUAL_KEY (1<<2)

struct bee_tree *bee_tree_allocate(void);
void bee_tree_free(struct bee_tree *tree);
struct bee_subtree *bee_tree_insert(struct bee_tree *tree, void *data);

void *bee_tree_search(struct bee_tree *tree, void *key);
void *bee_tree_delete(struct bee_tree *tree, void *key);

void bee_tree_print(struct bee_tree *tree);
void bee_tree_print_plain(struct bee_tree *tree);

int bee_tree_set_flags(struct bee_tree *tree, int flags);
int bee_tree_unset_flags(struct bee_tree *tree, int flags);

#endif
