#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "graph.h"

#define NEEDS    "needs"
#define PROVIDES "provides"
#define TYPE     "type"

#define PACKAGE "PACKAGE"
#define UNKNOWN "VOID"

#define NODENAME_MAX PATH_MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ABS(a) (((a) < 0) ? (-a) : (a))

struct queue_node {
    struct tree_node *root;
    struct queue_node *next;
    struct queue_node *prev;
};

struct queue {
    struct queue_node *first;
    struct queue_node *last;
};

/* protoypes */
struct tree *tree_new(void);

static struct graph_node *graph_node_new(char *name, char *type);

static struct tree_node *tree_node_new(struct graph_node *n);

static void tree_insert_graph_node(struct tree *t, struct graph_node *n);

static int graph_node_cmp(struct graph_node *a, struct graph_node *b);

static struct tree_node *tree_node_balance(struct tree_node *root);

static void tree_node_update_values(struct tree_node *root);

static struct tree_node *tree_node_rotate_left(struct tree_node *root);

static struct tree_node *tree_node_rotate_right(struct tree_node *root);

static struct graph_node *tree_search_graph_node(struct tree *t, char *name);

static void graph_node_add_need(struct graph_node *from, struct graph_node *to);

static void graph_node_add_provide(struct graph_node *from, struct graph_node *to);

static void tree_node_free_all(struct tree_node *root);

static void tree_node_delete_all(struct tree_node *n);

static struct graph_node *tree_node_free(struct tree_node *n);

static void graph_node_free(struct graph_node *n);

void tree_delete(struct tree *t);

void tree_add_graph_nodes_from_file(struct tree *t, char *filename, char quiet);

void tree_save(struct tree *t);

void tree_search_and_print_conflicts(struct tree *t);

void tree_print(struct tree *t);

struct tree_node *predecessor(struct tree_node *root);

struct queue *tree_to_tree_node_queue(struct tree *t);

struct tree_node *queue_pop(struct queue *q);

void tree_filter_graph_node_type(struct tree *t, struct tree *filter, char *type);

/* implementations */
static struct graph_node *graph_node_new(char *name, char *type)
{
    struct graph_node *n;

    assert(name);
    assert(type);

    if ((n = calloc(1, sizeof(struct graph_node))) == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    if ((n->name = strdup(name)) == NULL
        || (n->type = strdup(type)) == NULL) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    return n;
}

struct tree *tree_new(void)
{
    struct tree *t;

    if ((t = calloc(1, sizeof(struct tree))) == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    return t;
}

static struct tree_node *tree_node_new(struct graph_node *n)
{
    struct tree_node *tn;

    assert(n);

    if ((tn = calloc(1, sizeof(struct tree_node))) == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    tn->node = n;

    return tn;
}

static void tree_insert_graph_node(struct tree *t, struct graph_node *n)
{
    struct tree_node *current, *new;

    assert(n);
    assert(t);

    if (tree_search_graph_node(t, n->name))
        return;

    new = tree_node_new(n);

    if (!t->root) {
        t->root = new;
        return;
    }

    current = t->root;

    while (current) {
        if (graph_node_cmp(n, current->node) <= 0) {
            if (current->left) {
                current = current->left;
            } else {
                current->left = new;
                new->parent = current;
                break;
            }
        } else {
            if (current->right) {
                current = current->right;
            } else {
                current->right = new;
                new->parent = current;
                break;
            }
        }
    }

    tree_node_update_values(current);
    current = tree_node_balance(current);

    if (!current->parent)
        t->root = current;
}

static struct tree_node *tree_node_balance(struct tree_node *n)
{
    struct tree_node *tn;

    assert(n);

    if (ABS(n->balance_value) < 2) {
        if (n->parent)
            return tree_node_balance(n->parent);
        return n;
    }

    if (n->balance_value > 1) {
        /* left right case */
        if (n->left->balance_value < 0)
            n->left = tree_node_rotate_left(n->left);

        /* left left case */
        tn = tree_node_rotate_right(n);
    } else {
        /* right left case */
        if (n->right->balance_value > 0)
            n->right = tree_node_rotate_right(n->right);

        /* right right case */
        tn = tree_node_rotate_left(n);
    }

    if (tn->parent)
        return tree_node_balance(tn->parent);
    return tn;
}

static void tree_node_update_values(struct tree_node *root)
{
    unsigned char depth_left, depth_right;

    assert(root);

    do {
        depth_left  = root->left  ? root->left->max_depth  + 1 : 0;
        depth_right = root->right ? root->right->max_depth + 1 : 0;
        root->max_depth     = MAX(depth_left, depth_right);
        root->balance_value = depth_left - depth_right;
        root = root->parent;
    } while (root);
}

static struct tree_node *tree_node_rotate_left(struct tree_node *root)
{
    struct tree_node *pivot;

    assert(root);

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
    if (pivot->parent) {
        if (pivot->parent->left == root)
            pivot->parent->left  = pivot;
        else
            pivot->parent->right = pivot;
    }

    tree_node_update_values(root);

    return pivot;
}

static struct tree_node *tree_node_rotate_right(struct tree_node *root)
{
    struct tree_node *pivot;

    assert(root);

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
    pivot->right = root;
    root->parent = pivot;

    /* finish rotation by updating parents reference of pivot root if needed */
    if (pivot->parent) {
        if (pivot->parent->left == root)
            pivot->parent->left  = pivot;
        else
            pivot->parent->right = pivot;
    }

    tree_node_update_values(root);

    return pivot;
}

static struct graph_node *tree_search_graph_node(struct tree *t, char *name)
{
    struct tree_node *current;
    struct graph_node search;
    int cmp;

    current     = t->root;
    search.name = name;

    while (current) {
        if ((cmp = graph_node_cmp(&search, current->node)) == 0)
            break;

        if (cmp < 0)
            current = current->left;
        else
            current = current->right;
    }

    if (!current)
        return NULL;

    return current->node;
}

static struct tree_node *tree_search_tree_node(struct tree *t, char *name)
{
    struct tree_node *current;
    int cmp;

    current = t->root;

    while (current) {
        if ((cmp = strcmp(name, current->node->name)) == 0)
            break;

        if (cmp < 0)
            current = current->left;
        else
            current = current->right;
    }

    return current;
}

void graph_node_set_type(struct graph_node *n, char *value)
{
    assert(n);
    assert(value);

    if (n->type)
        free(n->type);

    if ((n->type = strdup(value)) == NULL) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }
}

void tree_add_graph_nodes_from_file(struct tree *t, char *filename, char quiet)
{
    FILE *file;
    char *s, *p, *a;
    char type_flag;
    char line[LINE_MAX],
         prop[LINE_MAX],
         value[LINE_MAX],
         pkgname[NODENAME_MAX]  = {0},
         nodename[NODENAME_MAX] = {0};
    int l;
    struct graph_node *n, *h, *m;

    assert(t);
    assert(filename);

    if ((file = fopen(filename, "r")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, LINE_MAX, file)) {
        if (!line[0])
            continue;

        /* remove_unnecessary_characters */
        l = strlen(line);
        s = line;
        p = line+l-1;
        h = NULL;

        while (p-s && *p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
            *(p--) = 0;

        while (*s && (*s == ' ' || *s == '\t'))
            s++;

        l = p - s;

        if (l <= 0 || *s == '#')
            continue;

        /* read node name */
        if (*s == '[') {
            s++;
            if (*p != ']') {
                fprintf(stderr, "missing bracket\n");
                exit(EXIT_FAILURE);
            }

            *p = '\0';

            if (p - s == 0) {
                fprintf(stderr, "empty node name\n");
                exit(EXIT_FAILURE);
            }

            if (!(n = tree_search_graph_node(t, s))) {
                n = graph_node_new(s, UNKNOWN);
                tree_insert_graph_node(t, n);
            }

            if (s[0] == '/') {
                if (!pkgname[0]) {
                    fprintf(stderr, "dont know to which package \"%s\" belongs to\n", s);
                    exit(EXIT_FAILURE);
                }
                sprintf(nodename, "%s%s", pkgname, s);

                if (!(h = tree_search_graph_node(t, nodename))) {
                    h = graph_node_new(nodename, UNKNOWN);
                    tree_insert_graph_node(t, h);
                }

                graph_node_add_provide(h, n);
            }

            type_flag = 0;
        } else {
            /* read node dependencies */
            a = s;
            while (*a && !(*a == ' ' || *a == '\t' || *a == '='))
                a++;

            l = a - s;
            memset(prop, '\0', LINE_MAX);
            strncpy(prop, s, l * sizeof(char));

            while (*a && (*a == ' ' || *a == '\t' || *a == '='))
                a++;

            l = p - a + 1;
            memset(value, '\0', LINE_MAX);
            strcpy(value, a);

            if (strcmp(prop, TYPE) == 0) {
                if (!type_flag) {
                    graph_node_set_type(n, value);

                    if (strcasecmp(PACKAGE, value) == 0)
                        sprintf(pkgname, "%s", n->name);

                    type_flag = 1;
                } else {
                    fprintf(stderr, "%s: ambiguous type \"%s\"\n", n->name, value);
                }
            } else if (strcmp(prop, PROVIDES) == 0) {

                if (strcmp(n->name, value) == 0)
                    continue;

                if (!(m = tree_search_graph_node(t, value))) {
                    m = graph_node_new(value, UNKNOWN);
                    tree_insert_graph_node(t, m);
                }

                if (m->name[0] == '/') {
                    if (n->name[0] != '/')
                        sprintf(pkgname, "%s", n->name);

                    if (!pkgname[0]) {
                        fprintf(stderr, "dont know to which package \"%s\" belongs to\n", s);
                        exit(EXIT_FAILURE);
                    }
                    sprintf(nodename, "%s%s", pkgname, m->name);

                    if (!(h = tree_search_graph_node(t, nodename))) {
                        h = graph_node_new(nodename, UNKNOWN);
                        tree_insert_graph_node(t, h);
                    }
                    graph_node_add_provide(h, m);
                }

                if (h)
                    graph_node_add_provide(n, h);
                else
                    graph_node_add_provide(n, m);
            } else if (strcmp(prop, NEEDS) == 0) {
                if (!(m = tree_search_graph_node(t, value))) {
                    m = graph_node_new(value, UNKNOWN);
                    tree_insert_graph_node(t, m);
                }
                graph_node_add_need(n, m);
            }
        }
    }

    if (fclose(file) == EOF) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }

    if (t->root && !quiet)
        tree_search_and_print_conflicts(t);
}

int graph_node_cmp(struct graph_node *a, struct graph_node *b)
{
    assert(a);
    assert(b);

    assert(a->name);
    assert(b->name);

    if (a == b)
        return 0;

    return strcmp(a->name, b->name);
}

void graph_node_add_need(struct graph_node *from, struct graph_node *to)
{
    tree_insert_graph_node(&(from->need), to);
    tree_insert_graph_node(&(to->neededby), from);
}

void graph_node_add_provide(struct graph_node *from, struct graph_node *to)
{
    tree_insert_graph_node(&(from->provide), to);
    tree_insert_graph_node(&(to->providedby), from);
}

struct graph_node *tree_node_free(struct tree_node *n)
{
    struct graph_node *r;

    assert(n);
    assert(!n->left);
    assert(!n->right);

    r = n->node;
    free(n);

    return r;
}

static void graph_node_free(struct graph_node *n)
{
    assert(n);

    tree_node_free_all(n->need.root);
    tree_node_free_all(n->neededby.root);
    tree_node_free_all(n->provide.root);
    tree_node_free_all(n->providedby.root);

    free(n->name);
    free(n->type);
    free(n);
}

static struct graph_node *tree_delete_tree_node(struct tree *t, char *name)
{
    struct tree_node *delete,
                     *replace,
                     *to_update;
    struct graph_node *s;

    assert(t);
    assert(t->root);
    assert(name);

    delete = tree_search_tree_node(t, name);

    if (!delete)
        return NULL;

    assert(delete->balance_value < 2);
    assert(delete->balance_value > -2);

    s = delete->node;

    if (delete->left) {
        if (delete->right) {
            replace = predecessor(delete);

            delete->node = replace->node;
            delete = replace;

            if (delete->left) {
                delete->node = delete->left->node;
                delete = delete->left;
            }

        } else {
            delete->node = delete->left->node;
            delete = delete->left;
        }
    } else {
        if (delete->right) {
            delete->node = delete->right->node;
            delete = delete->right;
        }
    }

    assert(!delete->left);
    assert(!delete->right);

    if ((to_update = delete->parent)) {
        if (to_update->left == delete)
            to_update->left = NULL;
        else
            to_update->right = NULL;
    } else {
        t->root = NULL;
    }

    tree_node_free(delete);

    if (to_update) {
        tree_node_update_values(to_update);
        replace = tree_node_balance(to_update);
        if (!replace->parent)
            t->root = replace;
    }

    return s;
}

static void tree_node_free_all(struct tree_node *n)
{
    if (!n)
        return;

    tree_node_free_all(n->right);
    tree_node_free_all(n->left);

    free(n);
}

static void tree_node_delete_all(struct tree_node *n)
{
    if (!n)
        return;

    tree_node_delete_all(n->right);
    tree_node_delete_all(n->left);

    graph_node_free(n->node);
    free(n);
}

void tree_delete(struct tree *t)
{
    assert(t);

    tree_node_delete_all(t->root);
    free(t);
}

struct queue *queue_new(void)
{
    struct queue *q;

    if ((q = calloc(1, sizeof(struct queue))) == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    return q;
}

struct queue_node *queue_node_new(struct tree_node *tn)
{
    struct queue_node *qn;

    assert(tn);

    if ((qn = calloc(1, sizeof(struct queue_node))) == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    qn->root = tn;

    return qn;
}

struct tree_node *queue_pop(struct queue *q)
{
    struct tree_node *tn;

    assert(q);

    if (!(q->last))
        return NULL;

    if (q->last == q->first) {
        tn       = q->last->root;
        free(q->last);
        q->first = NULL;
        q->last  = NULL;
        return tn;
    }

    tn      = q->last->root;
    q->last = q->last->prev;
    free(q->last->next);

    return tn;
}

void queue_push(struct queue *q, struct tree_node *tn)
{
    struct queue_node *qn;

    assert(q);
    assert(tn);

    qn = queue_node_new(tn);

    if (!q->first) {
        q->last  = qn;
        q->first = qn;
    } else {
        qn->next       = q->first;
        q->first->prev = qn;
        q->first       = qn;
    }
}

void queue_free(struct queue *q)
{
    assert(q);

    while(q->first != NULL && q->last != NULL)
        queue_pop(q);

    free(q);
}

struct queue *tree_to_tree_node_queue(struct tree *t)
{
    struct queue *q;
    struct queue_node *qn;

    q = queue_new();

    if (!t->root)
        return q;

    queue_push(q, t->root);
    qn = q->first;

    while (qn) {
        if (qn->root->left)
            queue_push(q, qn->root->left);
        if (qn->root->right)
            queue_push(q, qn->root->right);
        qn = qn->prev;
    }

    return q;
}

void tree_save(struct tree *t)
{
    FILE *file;
    struct queue *help, *prop;
    struct tree_node *h, *p;

    help = tree_to_tree_node_queue(t);

    if ((file = fopen(MAIN_FILE, "w")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    while ((h = queue_pop(help))) {
        fprintf(file, "[%s]\n", h->node->name);
        fprintf(file, "%s=%s\n", TYPE, h->node->type);

        prop = tree_to_tree_node_queue(&(h->node->provide));
        while ((p = queue_pop(prop)))
            fprintf(file, "%s=%s\n", PROVIDES, p->node->name);
        free(prop);

        prop = tree_to_tree_node_queue(&(h->node->need));
        while ((p = queue_pop(prop)))
            fprintf(file, "%s=%s\n", NEEDS, p->node->name);
        free(prop);
    }

    if (fclose(file) == EOF) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }

    free(help);
}

static void tree_nodes_print(struct tree_node *tn)
{
    if (!tn)
        return;

    tree_nodes_print(tn->left);

    printf("%s\n", tn->node->name);

    tree_nodes_print(tn->right);
}

void tree_print(struct tree *t)
{
    tree_nodes_print(t->root);
}

static void search_neededby_dependencies(struct tree *t, struct graph_node *gn, struct tree *e, char depth)
{
    struct queue *q;
    struct tree_node *tn;

    /* check if already in list */
    if (e->root && tree_search_graph_node(e, gn->name))
        return;

    if (depth == 0 || depth == 1) {
        depth += 2;
    } else {
        tree_insert_graph_node(e, gn);
        if (depth == (DIRECT + 2) && strcmp(gn->type, PACKAGE) == 0)
            return;
    }

    /* add need */
    q = tree_to_tree_node_queue(&(gn->need));
    while ((tn = queue_pop(q)))
        search_neededby_dependencies(t, tn->node, e, depth);
    free(q);

    /* add providedby */
    q = tree_to_tree_node_queue(&(gn->providedby));
    while ((tn = queue_pop(q)))
        search_neededby_dependencies(t, tn->node, e, depth);
    free(q);
}

static void search_need_dependencies(struct graph_node *gn, struct tree *e)
{
    struct queue *q;
    struct tree_node *tn;

    /* check if already in list */
    if (e->root && tree_search_graph_node(e, gn->name))
        return;

    tree_insert_graph_node(e, gn);
    if (strcmp(gn->type, PACKAGE) == 0)
        return;

    /* add neededby */
    q = tree_to_tree_node_queue(&(gn->neededby));
    while ((tn = queue_pop(q)))
        search_need_dependencies(tn->node, e);
    free(q);

    /* add providedby */
    q = tree_to_tree_node_queue(&(gn->providedby));
    while ((tn = queue_pop(q)))
        search_need_dependencies(tn->node, e);
    free(q);
}

static void search_providedby_dependencies(struct tree *t, struct graph_node *gn, struct tree *e)
{
    struct queue *q;
    struct tree_node *tn;

    /* check if already in list */
    if (e->root && tree_search_graph_node(e, gn->name))
        return;

    /* add neededby */
    q = tree_to_tree_node_queue(&(gn->neededby));
    while ((tn = queue_pop(q))) {
        /* check if already in list */
        if (e->root && tree_search_graph_node(e, gn->name))
            return;
        tree_insert_graph_node(e, tn->node);
        search_providedby_dependencies(t, tn->node, e);
    }
    free(q);

    /* provide */
    q = tree_to_tree_node_queue(&(gn->provide));
    while ((tn = queue_pop(q)))
        search_providedby_dependencies(t, tn->node, e);
    free(q);
}

void providedby_search_packages(struct tree *e, struct graph_node *n)
{
    struct tree_node *tn;
    struct queue *q;

    assert(e);

    if (!n)
        return;

    if (strcmp(n->type, PACKAGE) == 0 && !tree_search_graph_node(e, n->name))
        tree_insert_graph_node(e, n);

    if (n->providedby.root) {
        q = tree_to_tree_node_queue(&(n->providedby));
        while ((tn = queue_pop(q))) {
            providedby_search_packages(e, tn->node);
        }
        free(q);
    }
}

char tree_nodes_search_graph_node_conflicts(struct tree_node *n)
{
    struct tree *e;
    char conflicts = 0;

    if (n->left)
        if (tree_nodes_search_graph_node_conflicts(n->left))
            conflicts = 1;

    if (n->right)
        if (tree_nodes_search_graph_node_conflicts(n->right))
            conflicts = 1;

    e = tree_new();

    if (n->node->providedby.root)
        providedby_search_packages(e, n->node->providedby.root->node);

    if (e->root && e->root->max_depth) {
        fprintf(stderr, "%s is provided by:\n", n->node->name);
        tree_print(e);
        conflicts = 1;
    }

    tree_node_free_all(e->root);
    free(e);

    return conflicts;
}

void tree_search_and_print_conflicts(struct tree *t)
{
    assert(t);
    assert(t->root);

    fprintf(stderr, "conflicts:\n");
    if (!tree_nodes_search_graph_node_conflicts(t->root))
        fprintf(stderr, "none\n");
    fprintf(stderr, "conflicts end\n");
}

char graph_node_remove(struct tree *t, struct graph_node *g, char quiet)
{
    struct graph_node  *n;
    char conflicts = 0;

    assert(t);
    assert(g);

    if (g->neededby.root || g->providedby.root) {
        if (!quiet)
            fprintf(stderr, "%s\n", g->name);
        return 1;
    }

    while(g->provide.root) {
        n = g->provide.root->node;
        tree_delete_tree_node(&(g->provide), n->name);
        tree_delete_tree_node(&(n->providedby), g->name);
        if (graph_node_remove(t, n, quiet))
            conflicts = 1;
    }

    while(g->need.root) {
        n = g->need.root->node;
        tree_delete_tree_node(&(g->need), n->name);
        tree_delete_tree_node(&(n->neededby), g->name);
        if (graph_node_remove(t, n, quiet))
            conflicts = 1;
    }

    tree_delete_tree_node(t, g->name);
    graph_node_free(g);

    return conflicts;
}

void tree_remove_graph_node(struct tree *t, char *name, char quiet)
{
    struct graph_node *g;

    assert(t);
    assert(name);

    if (!(g = tree_search_graph_node(t, name))) {
        fprintf(stderr, "%s does not exist\n", name);
        return;
    }

    if (!quiet) {
        fprintf(stderr, "could not remove:\n");
    }

    if (!graph_node_remove(t, g, quiet) && !quiet)
        fprintf(stderr, "none\n");
}

struct tree_node *predecessor(struct tree_node *root)
{
    struct tree_node *current;

    assert(root);

    current = root->left;

    if (!current)
        return NULL;

    while (current->right)
        current = current->right;

    return current;
}

void tree_update_graph_node(struct tree *t, char *new, char *old, char quiet)
{
    if (!tree_search_graph_node(t, new))
        fprintf(stderr, "%s can not find new\n", new);

    tree_remove_graph_node(t, old, quiet);

    if (t->root && !quiet)
        tree_search_and_print_conflicts(t);
}

void tree_search_provides(struct tree_node *tn, struct tree *e)
{
    assert(e);

    if (!tn)
        return;

    /* check if already in list */
    if (e->root && tree_search_graph_node(e, tn->node->name))
        return;

    tree_insert_graph_node(e, tn->node);

    if (tn->node->provide.root)
        tree_search_provides(tn->node->provide.root, e);

    if (tn->left)
        tree_search_provides(tn->left, e);

    if (tn->right)
        tree_search_provides(tn->right, e);
}

void tree_search_providedbies(struct tree_node *tn, struct tree *e)
{
    assert(e);

    if (!tn)
        return;

    /* check if already in list */
    if (e->root && tree_search_graph_node(e, tn->node->name))
        return;

    tree_insert_graph_node(e, tn->node);

    if (tn->node->providedby.root)
        tree_search_providedbies(tn->node->providedby.root, e);

    if (tn->left)
        tree_search_providedbies(tn->left, e);

    if (tn->right)
        tree_search_providedbies(tn->right, e);
}

void print_provides(struct tree *t, char *name)
{
    struct tree *e;
    struct graph_node *g;

    assert(t);
    assert(name);

    if (!(g = tree_search_graph_node(t, name))) {
        fprintf(stderr, "\"%s\" not found\n", name);
        return;
    }

    e = tree_new();

    tree_search_provides(g->provide.root, e);

    tree_print(e);

    tree_node_free_all(e->root);
    free(e);
}

void print_from_which_this_package_depends(struct tree *t, char *name, char depth)
{
    struct graph_node *g;
    struct queue *q;
    struct tree_node *tn;
    struct tree *dependencies, *e;

    assert(t);
    assert(t->root);
    assert(name);
    assert(depth == 0 || depth == 1);

    if (!(g = tree_search_graph_node(t, name))) {
        fprintf(stderr, "\"%s\" not found\n", name);
        return;
    }

    dependencies = tree_new();
    e = tree_new();

    search_neededby_dependencies(t, g, dependencies, depth);

    q = tree_to_tree_node_queue(&(g->provide));
    while ((tn = queue_pop(q)))
        search_neededby_dependencies(t, tn->node, dependencies, depth);

    free(q);

    tree_filter_graph_node_type(dependencies, e, PACKAGE);

    tree_print(e);


    tree_node_free_all(e->root);
    free(e);
    tree_node_free_all(dependencies->root);
    free(dependencies);
}

void print_which_packages_depended_on_this(struct tree *t, char *name, char depth)
{
    struct graph_node *g;
    struct tree *dependencies, *e;
    struct queue *q;
    struct tree_node *tn;

    assert(t);
    assert(t->root);
    assert(name);
    assert(depth == 0 || depth == 1);

    if (!(g = tree_search_graph_node(t, name))) {
        fprintf(stderr, "\"%s\" not found\n", name);
        return;
    }

    dependencies = tree_new();
    e = tree_new();

    search_providedby_dependencies(t, g, dependencies);
    q = tree_to_tree_node_queue(dependencies);

    while ((tn = queue_pop(q)))
        providedby_search_packages(e, tn->node);

    tree_print(e);

    tree_node_free_all(dependencies->root);
    tree_node_free_all(e->root);
    free(dependencies);
    free(e);
    free(q);
}

void print_files_which_exclusively_provided_by_package(struct tree *t, char *name)
{
    struct tree *e, *f;
    struct graph_node *g;
    struct queue *q;
    struct tree_node *tn;

    assert(t);
    assert(name);

    if (!(g = tree_search_graph_node(t, name))) {
        fprintf(stderr, "\"%s\" not found\n", name);
        return;
    }

    if (strcmp(g->type, PACKAGE) != 0) {
        fprintf(stderr, "%s is no package\n", name);
        return;
    }

    e = tree_new();

    tree_search_provides(g->provide.root, e);

    if (!e->root) {
        fprintf(stderr, "%s has no provides\n", name);
        return;
    }

    q = tree_to_tree_node_queue(e);

    free(e);

    while ((tn = queue_pop(q))) {
        f = tree_new();
        tree_search_providedbies(tn->node->providedby.root, f);
        e = tree_new();
        tree_filter_graph_node_type(f, e, PACKAGE);
        if (!e->root->max_depth)
            printf("%s\n", tn->node->name);
        tree_node_free_all(e->root);
        tree_node_free_all(f->root);
        free(f);
        free(e);
    }

    free(q);
}

void tree_node_filter_type(struct tree_node *tn, struct tree *filter, char *type)
{
    assert(tn);
    assert(filter);
    assert(type);

    if (strcmp(tn->node->type, type) == 0)
        tree_insert_graph_node(filter, tn->node);

    if (tn->left)
        tree_node_filter_type(tn->left, filter, type);

    if (tn->right)
        tree_node_filter_type(tn->right, filter, type);
}

void tree_filter_graph_node_type(struct tree *t, struct tree *filter, char *type)
{
    assert(t);
    assert(filter);
    assert(type);

    if (!t->root) {
        fprintf(stderr, "no elements found\n");
        return;
    }

    tree_node_filter_type(t->root, filter, type);

    if (!filter->root)
        fprintf(stderr, "Type \"%s\" not found\n", type);
}

void print_conflicts_with_other_packages(struct tree *t, char *name)
{
    struct graph_node *g;
    struct tree *e, *f;
    struct tree_node *tn;
    struct queue *q;

    assert(t);
    assert(name);

    if (!(g = tree_search_graph_node(t, name))) {
        fprintf(stderr, "\"%s\" not found\n", name);
        return;
    }

    e = tree_new();
    tree_search_provides(g->provide.root, e);

    q = tree_to_tree_node_queue(e);

    free(e);

    while ((tn = queue_pop(q))) {
        f = tree_new();
        tree_search_providedbies(tn->node->providedby.root, f);
        e = tree_new();
        tree_filter_graph_node_type(f, e, PACKAGE);
        if (e->root->max_depth)
            printf("%s\n", tn->node->name);
        tree_node_free_all(f->root);
        tree_node_free_all(e->root);
        free(f);
        free(e);
    }

    free(q);
}

int tree_nodes_count_nodes(struct tree_node *tn)
{
    if (!tn)
        return 0;

    return (1 + tree_nodes_count_nodes(tn->left)
              + tree_nodes_count_nodes(tn->right));
}

int tree_count_nodes(struct tree *t)
{
    assert(t);

    return tree_nodes_count_nodes(t->root);
}

void print_count_of_packets_provied_this_file(struct tree *t, char *name)
{
    struct tree *e, *f;
    struct graph_node *g;
    int count;

    assert(t);
    assert(name);

    if (!(g = tree_search_graph_node(t, name))) {
        fprintf(stderr, "\"%s\" not found\n", name);
        return;
    }

    e = tree_new();
    f = tree_new();
    tree_search_providedbies(g->providedby.root, e);
    tree_filter_graph_node_type(e, f, PACKAGE);
    count = tree_count_nodes(f);

    printf("%d\n", count);

    tree_node_free_all(e->root);
    tree_node_free_all(f->root);
    free(e);
    free(f);
}

void print_packages_provide_this_file(struct tree* t, char *name)
{
    struct tree *e, *f;
    struct graph_node *g;

    assert(t);
    assert(name);

    if (!(g = tree_search_graph_node(t, name))) {
        fprintf(stderr, "\"%s\" not found\n", name);
        return;
    }

    e = tree_new();
    f = tree_new();
    tree_search_providedbies(g->providedby.root, e);
    tree_filter_graph_node_type(e, f, PACKAGE);

    tree_print(f);

    tree_node_free_all(e->root);
    tree_node_free_all(f->root);
    free(e);
    free(f);
}

void print_packages_needs_this_file(struct tree* t, char *name)
{
    struct tree *e, *f;
    struct graph_node *g;

    assert(t);
    assert(name);

    if (!(g = tree_search_graph_node(t, name))) {
        fprintf(stderr, "\"%s\" not found\n", name);
        return;
    }

    e = tree_new();
    search_need_dependencies(g, e);

    f = tree_new();
    tree_filter_graph_node_type(e, f, PACKAGE);

    tree_print(f);

    tree_node_free_all(e->root);
    tree_node_free_all(f->root);
    free(e);
    free(f);
}

void read_database(struct tree *t, char *filename)
{
    FILE *file;
    char *s, *p, *a;
    char type_flag;
    char line[LINE_MAX],
         prop[LINE_MAX],
         value[LINE_MAX],
         pkgname[NODENAME_MAX]  = {0};
    int l;
    struct graph_node *n, *h, *m;

    assert(t);
    assert(filename);

    if ((file = fopen(filename, "r")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, LINE_MAX, file)) {
        if (!line[0])
            continue;

        /* remove_unnecessary_characters */
        l = strlen(line);
        s = line;
        p = line+l-1;
        h = NULL;

        while (p-s && *p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
            *(p--) = 0;

        while (*s && (*s == ' ' || *s == '\t'))
            s++;

        l = p - s;

        if (l <= 0 || *s == '#')
            continue;

        /* read node name */
        if (*s == '[') {
            s++;
            if (*p != ']') {
                fprintf(stderr, "missing bracket\n");
                exit(EXIT_FAILURE);
            }

            *p = '\0';

            if (p - s == 0) {
                fprintf(stderr, "empty node name\n");
                exit(EXIT_FAILURE);
            }

            if (!(n = tree_search_graph_node(t, s))) {
                n = graph_node_new(s, UNKNOWN);
                tree_insert_graph_node(t, n);
            }

            type_flag = 0;
        } else {
            /* read node dependencies */
            a = s;
            while (*a && !(*a == ' ' || *a == '\t' || *a == '='))
                a++;

            l = a - s;
            memset(prop, '\0', LINE_MAX);
            strncpy(prop, s, l * sizeof(char));

            while (*a && (*a == ' ' || *a == '\t' || *a == '='))
                a++;

            l = p - a + 1;
            memset(value, '\0', LINE_MAX);
            strcpy(value, a);

            if (strcmp(prop, TYPE) == 0) {
                if (!type_flag) {
                    graph_node_set_type(n, value);

                    if (strcasecmp(PACKAGE, value) == 0)
                        sprintf(pkgname, "%s", n->name);

                    type_flag = 1;
                } else {
                    fprintf(stderr, "%s: ambiguous type \"%s\"\n", n->name, value);
                }
            } else if (strcmp(prop, PROVIDES) == 0) {
                if (!(m = tree_search_graph_node(t, value))) {
                    m = graph_node_new(value, UNKNOWN);
                    tree_insert_graph_node(t, m);
                }
                graph_node_add_provide(n, m);
            } else if (strcmp(prop, NEEDS) == 0) {
                if (!(m = tree_search_graph_node(t, value))) {
                    m = graph_node_new(value, UNKNOWN);
                    tree_insert_graph_node(t, m);
                }
                graph_node_add_need(n, m);
            }
        }
    }

    if (fclose(file) == EOF) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }
}
