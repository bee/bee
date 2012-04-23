
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "bee_tree.h"

static void *bee_tree_generate_key_default(void *data)
{
    assert(data);

    return data;
}

static int bee_tree_compare_key_default(void *a, void *b)
{
    assert(a);
    assert(b);

    return strcmp(a, b);
}

static void bee_tree_print_key_default(void *key)
{
    assert(key);

    fputs(key, stdout);
}

struct bee_tree *bee_tree_allocate(void)
{
    struct bee_tree *t;

    if(!(t = calloc(1, sizeof(*t))))
        return NULL;

    t->generate_key = &bee_tree_generate_key_default;
    t->compare_key  = &bee_tree_compare_key_default;
    t->print_key    = &bee_tree_print_key_default;

    return t;
}

static struct bee_subtree *bee_subtree_allocate(void)
{
    struct bee_subtree *t;

    if(!(t = calloc(1, sizeof(*t))))
        return NULL;

    t->height = 1;

    return t;
}

static void bee_node_free_content(struct bee_tree *tree, struct bee_subtree *node)
{
    assert(tree);
    assert(node);

#ifdef TREE_DEBUG
    printf("freeing '%s'\n", (char *)node->key);
#endif

    if (tree->free_data && node->data)
        tree->free_data(node->data);

    if (tree->free_key && node->key)
        tree->free_key(node->key);

    node->data = NULL;
    node->key  = NULL;
}

static void bee_subtree_free(struct bee_tree *tree, struct bee_subtree *this)
{
    if (!this)
        return;

    assert(tree);

    bee_subtree_free(tree, this->left);
    bee_subtree_free(tree, this->right);

    bee_node_free_content(tree, this);
    free(this);
}

void bee_tree_free(struct bee_tree *tree)
{
     assert(tree);
     bee_subtree_free(tree, tree->root);
     free(tree);
}

static void bee_tree_update_node(struct bee_subtree *node)
{
    unsigned char l, r;

    assert(node);

    l = BEE_TREE_HEIGHT(node->left);
    r = BEE_TREE_HEIGHT(node->right);

    node->height         = 1 + BEE_TREE_MAX(l,r);
    node->balance_factor = l - r;
}

static struct bee_subtree *bee_subtree_rotate_left(struct bee_subtree *root)
{
    struct bee_subtree *pivot;

    assert(root);

#ifdef TREE_DEBUG
    printf("left rotating '%s'\n", (char *)root->key);
#endif

    /* if no rotation is possible return old root */
    if (!root->right)
        return root;

    /* initialize rotation: point pivot to the new root */
    pivot         = root->right;
    pivot->parent = root->parent;

    /* start rotation by moving pivot roots child to old root */
    root->right = pivot->left;
    if (root->right)
        root->right->parent = root;

    /* continue rotation by making old root a child of pivot root */
    pivot->left  = root;
    root->parent = pivot;

    /* finish rotation by updating parents reference of pivot root if needed */

    bee_tree_update_node(root);
    bee_tree_update_node(pivot);

    if (!pivot->parent)
        return pivot;

    if (pivot->parent->left == root)
        pivot->parent->left  = pivot;
    else
        pivot->parent->right = pivot;

    while (root->parent) {
        root = root->parent;
        bee_tree_update_node(root);
    }

    return pivot;
}

static struct bee_subtree *bee_subtree_rotate_right(struct bee_subtree *root)
{
    struct bee_subtree *pivot;

    assert(root);

#ifdef TREE_DEBUG
    printf("right rotating '%s'\n", (char *)root->key);
#endif

    /* if no rotation is possible return old root */
    if (!root->left)
        return root;

    /* initialize rotation: point pivot to the new root */
    pivot         = root->left;
    pivot->parent = root->parent;

    /* start rotation by moving pivot roots child to old root */
    root->left = pivot->right;
    if (root->left)
        root->left->parent = root;

    /* continue rotation by making old root a child of pivot root */
    pivot->right  = root;
    root->parent = pivot;

    /* finish rotation by updating parents reference of pivot root if needed */

    bee_tree_update_node(root);
    bee_tree_update_node(pivot);

    if (!pivot->parent)
        return pivot;

    if (pivot->parent->left == root)
        pivot->parent->left  = pivot;
    else
        pivot->parent->right = pivot;

    while (root->parent) {
        root = root->parent;
        bee_tree_update_node(root);
    }

    return pivot;
}

static struct bee_subtree *bee_tree_rotate_left(struct bee_tree *tree, struct bee_subtree *node)
{
    struct bee_subtree *root;

    assert(tree);
    assert(node);

    root = bee_subtree_rotate_left(node);

    if (!root->parent)
        tree->root = root;

    return root;
}

static struct bee_subtree *bee_tree_rotate_right(struct bee_tree *tree, struct bee_subtree *node)
{
    struct bee_subtree *root;

    assert(tree);
    assert(node);

    root = bee_subtree_rotate_right(node);

    if (!root->parent)
        tree->root = root;

    return root;
}

static void bee_node_print(struct bee_tree *tree, struct bee_subtree *node, int depth, int dir)
{
    int i;

    assert(tree);
    assert(node);

    assert(tree->print || tree->print_key);

    for (i = 0 ; i < depth ; i++) {
        putchar('-');
    }

    if(dir > 0)
        putchar('\\');

    if(dir < 0)
        putchar('/');

    if (tree->print)
        tree->print(node->key, node->data);
    else
        tree->print_key(node->key);

#ifdef TREE_DEBUG
    printf(" [ h=%d bf=%d ]", node->height, node->balance_factor);
#endif

    putchar('\n');
}

static void bee_tree_balance_node(struct bee_tree *tree, struct bee_subtree *node)
{
    struct bee_subtree *child;

    while (node) {
        bee_tree_update_node(node);

#ifdef TREE_DEBUG
        printf("balancing ");
	node_print(tree, node, 0, 0);
#endif

        if (node->balance_factor == -2) {
            child = node->right;

            if (child->balance_factor == 1)
                bee_tree_rotate_right(tree, child);

            bee_tree_rotate_left(tree, node);

        } else if (node->balance_factor == 2) {
            child = node->left;

            if (child->balance_factor == -1)
                bee_tree_rotate_left(tree, child);

            bee_tree_rotate_right(tree, node);

        }

        node = node->parent;
    }
}

static struct bee_subtree *bee_tree_insert_node(struct bee_tree *tree, struct bee_subtree *node)
{
    struct bee_subtree *current;
    int    cmp;

    assert(tree);
    assert(node);

    assert(tree->compare_key);

#ifdef TREE_DEBUG
    printf("inserting ");
    node_print(tree, node, 0, 0);
#endif

    if (!tree->root)
        return (tree->root = node);

    current = tree->root;

    while (!node->parent) {
        cmp = tree->compare_key(node->key, current->key);

        if ((cmp == 0) && (tree->flags & (BEE_TREE_FLAG_UNIQUE|BEE_TREE_FLAG_UNIQUE_DATA))) {
            /* do not insert dupes */
            if (!(tree->flags & BEE_TREE_FLAG_UNIQUE_DATA))
                return NULL;

            assert(tree->compare_data);

            cmp = tree->compare_data(node->data, current->data);

            if (cmp == 0)
                return NULL;
        }

        if (cmp < 0) {
            if (current->left) {
                current = current->left;
                continue;
            }

            current->left = node;
            node->parent  = current;
            break;
        }

        if (current->right) {
            current = current->right;
            continue;
        }

        current->right = node;
        node->parent   = current;
    }

    bee_tree_balance_node(tree, current);

    return node;
}

struct bee_subtree *bee_tree_insert(struct bee_tree *tree, void *data)
{
    struct bee_subtree *node;

    assert(tree);
    assert(data);

    assert(tree->generate_key);

    errno = 0;

    node = bee_subtree_allocate();
    if (!node)
        return NULL;

    node->data = data;
    node->key  = tree->generate_key(data);

    if (!node->key) {
        free(node);
        return NULL;
    }

    if(bee_tree_insert_node(tree, node))
        return node;

    if (tree->free_key)
        tree->free_key(node->key);

    free(node);

    errno=EEXIST;
    return NULL;
}

static struct bee_subtree *bee_tree_search_node_by_key(struct bee_tree *tree, void *key)
{
    struct bee_subtree *node;
    int cmp;

    assert(tree);
    assert(key);

    assert(tree->compare_key);

    node = tree->root;

    while (node) {
        if (!(cmp = tree->compare_key(key, node->key)))
            return node;

        node = (cmp < 0) ? node->left : node->right;
    }

    return NULL;
}

/* search key in tree and return it's data*/
void *bee_tree_search(struct bee_tree *tree, void *key)
{
    struct bee_subtree *node;

    assert(tree);
    assert(key);

    node = bee_tree_search_node_by_key(tree, key);

    if(node == NULL)
        return NULL;

    assert(node->data);

    return node->data;
}

static struct bee_subtree *bee_subtree_successor(struct bee_subtree *node)
{
    node = node->right;

    if (!node)
        return NULL;

    while (node->left)
        node = node->left;

    return node;
}

static void bee_node_copy_content(struct bee_tree *tree, struct bee_subtree *from, struct bee_subtree *to)
{
    assert(to);
    assert(from);

#ifdef TREE_DEBUG
    printf("copying '%s'\n", (char *)from->key);
#endif

    bee_node_free_content(tree, to);

    to->key  = from->key;
    to->data = from->data;
}

static void bee_subtree_delete_node(struct bee_tree *tree, struct bee_subtree *node)
{
    struct bee_subtree *n = NULL;

    assert(tree);
    assert(node);

    if (node->left && node->right) {
        n = bee_subtree_successor(node);
        bee_node_copy_content(tree, n, node);
        node = n;
        n = NULL;
        assert(!node->left || !node->right);
    }

    bee_node_free_content(tree, node);

    if (node->left)
        n = node->left;

    if (node->right)
        n = node->right;

    if (n)
        n->parent = node->parent;

    if (node->parent) {
        if (node->parent->left == node)
            node->parent->left  = n;
        else
            node->parent->right = n;
    } else {
        tree->root = n;
    }

    if (!n)
        n = node->parent;

    free(node);

    bee_tree_balance_node(tree, n);
}


void *bee_tree_delete(struct bee_tree *tree, void *key)
{
    struct bee_subtree *node;

    assert(tree);
    assert(key);

    node = bee_tree_search_node_by_key(tree, key);

    if (node)
        bee_subtree_delete_node(tree, node);

    return NULL;
}

static void bee_subtree_print(struct bee_tree *tree, struct bee_subtree *node, int depth, int dir)
{
    assert(tree);

    if (!node)
        return;

    bee_subtree_print(tree, node->left,  depth+1, -1);
    bee_node_print(tree, node, depth, dir);
    bee_subtree_print(tree, node->right, depth+1, 1);
}

static void bee_subtree_print_plain(struct bee_tree *tree, struct bee_subtree *node)
{
    assert(tree);

    if (!node)
        return;

    bee_subtree_print_plain(tree, node->left);
    bee_node_print(tree, node, 0, 0);
    bee_subtree_print_plain(tree, node->right);
}

int bee_tree_set_flags(struct bee_tree *tree, int flags)
{
    int oflags;

    assert(tree);
    assert(flags);

    oflags = tree->flags;
    tree->flags |= flags;

    return oflags;
}

int bee_tree_unset_flags(struct bee_tree *tree, int flags)
{
    int oflags;

    assert(tree);
    assert(flags);

    oflags = tree->flags;
    tree->flags &= ~flags;

    return oflags;
}

void bee_tree_print(struct bee_tree *tree)
{
    assert(tree);

    bee_subtree_print(tree, tree->root, 0, 0);
}

void bee_tree_print_plain(struct bee_tree *tree)
{
    assert(tree);

    bee_subtree_print_plain(tree, tree->root);
}
