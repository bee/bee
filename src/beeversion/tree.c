
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tree.h"

struct tree *tree_allocate(void)
{
    struct tree *t;

    t = calloc(1, sizeof(*t));

    return t;
}

static struct tree_node *subtree_allocate(void)
{
    struct tree_node *t;

    t = calloc(1, sizeof(*t));

    t->height = 1;

    return t;
}

static void subtree_node_free_content(struct tree *tree, struct tree_node *node)
{

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

static void subtree_free(struct tree *tree, struct tree_node *this)
{
    if (!this)
        return;

    assert(tree);

    subtree_free(tree, this->left);
    subtree_free(tree, this->right);

    subtree_node_free_content(tree, this);
    free(this);
}

void tree_free(struct tree *tree)
{
     subtree_free(tree, tree->root);

     free(tree);
}

static void tree_update_node(struct tree_node *node)
{
    unsigned char l, r;

    l = TREE_HEIGHT(node->left);
    r = TREE_HEIGHT(node->right);

    node->height         = 1 + MAX(l,r);
    node->balance_factor = l - r;
}

static struct tree_node *subtree_rotate_left(struct tree_node *root)
{
    struct tree_node *pivot;

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

    tree_update_node(root);
    tree_update_node(pivot);

    if (!pivot->parent)
        return pivot;

    if (pivot->parent->left == root)
        pivot->parent->left  = pivot;
    else
        pivot->parent->right = pivot;

    while (root->parent) {
        root = root->parent;
        tree_update_node(root);
    }

    return pivot;
}

static struct tree_node *subtree_rotate_right(struct tree_node *root)
{
    struct tree_node *pivot;

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

    tree_update_node(root);
    tree_update_node(pivot);

    if (!pivot->parent)
        return pivot;

    if (pivot->parent->left == root)
        pivot->parent->left  = pivot;
    else
        pivot->parent->right = pivot;

    while (root->parent) {
        root = root->parent;
        tree_update_node(root);
    }

    return pivot;
}

static struct tree_node *tree_rotate_left(struct tree *tree, struct tree_node *node)
{
    struct tree_node *root;

    root = subtree_rotate_left(node);

    if (!root->parent)
        tree->root = root;

    return root;
}

static struct tree_node *tree_rotate_right(struct tree *tree, struct tree_node *node)
{
    struct tree_node *root;

    root = subtree_rotate_right(node);

    if (!root->parent)
        tree->root = root;

    return root;
}

static void node_print(struct tree *tree, struct tree_node *node, int depth)
{
    int i;

    assert(tree->print_key);

    for (i = 0 ; i < depth ; i++) {
        putchar('-');
    }

    tree->print_key(node->key);

#ifdef TREE_DEBUG
    printf(" [ h=%d bf=%d ]", node->height, node->balance_factor);
#endif

    putchar('\n');
}

static void tree_balance_node(struct tree *tree, struct tree_node *node)
{
    struct tree_node *child;

//    assert(node);

    while (node) {
        tree_update_node(node);

#ifdef TREE_DEBUG
        printf("balancing ");
	node_print(tree, node, 0);
#endif

        if (node->balance_factor == -2) {
            child = node->right;

            if (child->balance_factor == 1)
                tree_rotate_right(tree, child);

            tree_rotate_left(tree, node);

/*            if (child->balance_factor == -1) {
                tree_rotate_left(tree, node);
            } else if (child->balance_factor == 1) {
                tree_rotate_right(tree, child);
                tree_rotate_left(tree, node);
            }
*/
        } else if (node->balance_factor == 2) {
            child = node->left;

            if (child->balance_factor == -1)
                tree_rotate_left(tree, child);

            tree_rotate_right(tree, node);

/*
            if (child->balance_factor == 1) {
                tree_rotate_right(tree, node);
            } else if (child->balance_factor == -1) {
                tree_rotate_left(tree, child);
                tree_rotate_right(tree, node);
            }
*/
        }

        node = node->parent;
    }
}

static struct tree_node *tree_insert_node(struct tree *tree, struct tree_node *node)
{
    struct tree_node *current;
    int    cmp;

    assert(tree);
    assert(node);

    assert(tree->key_compare);

#ifdef TREE_DEBUG
    printf("inserting ");
    node_print(tree, node, 0);
#endif

    if (!tree->root)
        return (tree->root = node);

    current = tree->root;

    while (!node->parent) {
        cmp = tree->key_compare(node->key, current->key);

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

    tree_balance_node(tree, current);

    return node;
}

struct tree_node *tree_insert(struct tree *tree, void *data)
{
    struct tree_node *node;

    node = subtree_allocate();

    if (!node)
        return NULL;

    node->data = data;

    if (tree->generate_key) {
        node->key = tree->generate_key(data);
    } else {
        assert(!tree->free_key);
        node->key = node->data;
    }

    tree_insert_node(tree, node);

    return node;
}

static struct tree_node *tree_search_node_by_key(struct tree *tree, void *key)
{
    struct tree_node *node;
    int cmp;

    assert(tree);
    assert(key);
    assert(tree->key_compare);

    node = tree->root;

    while (node) {
        if (!(cmp = tree->key_compare(key, node->key)))
            return node;

        node = (cmp < 0) ? node->left : node->right;
    }

    return NULL;
}

/* search key in tree and return it's data*/
void *tree_search(struct tree *tree, void *key)
{
    struct tree_node *node;

    assert(tree);
    assert(key);
    assert(tree->key_compare);

    node = tree_search_node_by_key(tree, key);

    assert(node->data);

    return node->data;
}

static struct tree_node *subtree_successor(struct tree_node *node)
{
    node = node->right;

    if (!node)
        return NULL;

    while (node->left)
        node = node->left;

    return node;
}

static void subtree_node_copy_content(struct tree_node *from, struct tree_node *to)
{
#ifdef TREE_DEBUG
    printf("copying '%s'\n", (char *)from->key);
#endif
    to->key  = from->key;
    to->data = from->data;
}

static void subtree_delete_node(struct tree *tree, struct tree_node *node)
{
    struct tree_node *n = NULL;

    assert(tree);
    assert(node);

    subtree_node_free_content(tree, node);

    if (node->left && node->right) {
        n = subtree_successor(node);
        subtree_node_copy_content(n, node);
        subtree_node_free_content(tree, n);
        node = n;
        n = NULL;
    }

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

    tree_balance_node(tree, n);
}


void *tree_delete(struct tree *tree, void *key)
{
    struct tree_node *node;

    assert(tree);
    assert(key);
    assert(tree->key_compare);

    node = tree_search_node_by_key(tree, key);

    if (node)
        subtree_delete_node(tree, node);

    return NULL;
}

static void subtree_print(struct tree *tree, struct tree_node *node, int depth)
{
    if (!node)
        return;

    subtree_print(tree, node->left,  depth+1);
    node_print(tree, node, depth);
    subtree_print(tree, node->right, depth+1);
}

void tree_print(struct tree *tree)
{
    subtree_print(tree, tree->root, 0);
}
