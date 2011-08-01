
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "beeversion.h"
#include "compare.h"
#include "parse.h"
#include "output.h"
#include "tree.h"

void my_free_data(void *data)
{
    struct beeversion *v = data;

    free(v->string);
    free(v);
}

int my_compare_key(void *a, void *b)
{
    return compare_beepackages(a,b);
}

void my_print_key(void *key)
{
    print_format("%A", key, NULL);
}

struct tree *init_tree(void)
{
    struct tree *tree;

    tree = tree_allocate();

    assert(tree);

    tree->free_data   = &my_free_data;
    tree->compare_key = &my_compare_key;
    tree->print_key   = &my_print_key;

    return tree;
}


int main(int argc, char *argv[])
{
    struct tree *tree;
    
    struct beeversion *v;

    int i = 1;

    tree = init_tree();

    while (i < argc) {
        v = calloc(1, sizeof(*v));
        assert(v);
    
        if (argv[i][0] == '-') {
            parse_version(&argv[i][1], v);
            tree_delete(tree, v);
        } else {
            parse_version(argv[i], v);
            tree_insert(tree, v);
        }
        i++;

    }
    tree_print(tree);

    tree_free(tree);

    return 0;
}

