#include <stdio.h>
#include <string.h>


struct node {
    int y;
    int label; //(+) = 1, (-) = -1, (0) = 0

    struct tree_edge* pair; //is this vertex paired? if so, this is the pairing

    int n_edges;
    struct tree_edge** edge_list; //edges emanating from this node

    int n_subnodes; // 1 if vertex
    struct node** node_list; //subnodes in this node if blossom, NULL if vertex
};


struct tree_edge {
    int slack;
    int x; 
    struct node* vertex_plus;
    struct node* vertex_minus;
    struct tree_edge *parent; // if this edge is in the tree: what is its (paired) parent?

    struct tree_edge *breadcrumbs;
};

int len_vertices;
struct node vertices[256];

int len_edges;
struct tree_edge edges[256];

int len_tree;
struct tree_edge *tree[256]; //only the paired edges are in this list

int len_restore;
struct tree_edge *restore[512];
int len_subnodes;
struct node *subnodes[512];

int main() {

    return 0;
}


int swap_nodes(struct tree_edge *e) {
    struct node *t;
    t = e->vertex_plus;
    e->vertex_plus = e->vertex_minus;
    e->vertex_minus = t;
}

int grow(int delta_w) {
    int i;
    tree[0]->vertex_plus += delta_w;
    for(i=1; i < len_tree; i++) {
        tree[i]->vertex_plus->y += delta_w;
        tree[i]->vertex_minus->y -= delta_w;
    }
}

int augment(struct tree_edge *new_edge, struct tree_edge *pair_edge) {
    struct tree_edge *e;
    new_edge->x = 1;

    for(e = pair_edge; e = e->parent->parent; e->vertex_minus != NULL) {
        e->x = 0;
        e->parent->x = 1;
        e->vertex_plus->label = 0;
        e->vertex_minus->label = 0;
    }
} 

int add(struct tree_edge *parent, struct tree_edge *new_unpaired_edge) {
    struct tree_edge *new_paired_edge;

    if(new_unpaired_edge->vertex_minus == parent->vertex_plus) {
        swap_nodes(new_unpaired_edge);
    }
    new_paired_edge = new_unpaired_edge->vertex_minus->pair;

    if(new_paired_edge->vertex_plus == new_unpaired_edge->vertex_minus) {
        swap_nodes(new_paired_edge);
    }

    new_unpaired_edge->parent = parent;
    new_paired_edge->parent = new_unpaired_edge;

    tree[len_tree++] = new_paired_edge;
}


int make_blossom(struct tree_edge *e1, struct tree_edge *e2) {

    struct tree_edge *lowest_common_ancestor;
    struct tree_edge *e;

    int no_nodes = 0;

    for(e=e1; e->parent != NULL; e=e->parent) {
        e->parent->breadcrumbs = e;
    }

    for(e=e2; e->parent->breadcrumbs == NULL; e=e->parent) {
        
    }









}

int expand(int vertex) {
}
