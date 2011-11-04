
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tree.h"

void my_free_data(void *data)
{
    free(data);
}

struct tree *init_tree(void)
{
    struct tree *tree;

    tree = tree_allocate();

    assert(tree);

//    tree->free_data   = &my_free_data;

    return tree;
}


int main(int argc, char *argv[])
{
    struct tree *tree;

    int i = 1;

    tree = init_tree();

    while (i < argc) {
        if (argv[i][0] == '-')
            tree_delete(tree, &argv[i][1]);
        else
            tree_insert(tree, argv[i]);

        i++;

    }
    tree_print(tree);

    tree_free(tree);

    return 0;
}

