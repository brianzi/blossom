#include <stdio.h>
#include <string.h>


struct node {
    int y;
    int label;

    struct tree_edge* pair; //is this vertex paired? if so, this is the pairing

    int n_edges;
    struct tree_edge** edge_list; //edges emanating from this node

    int n_subnodes;
    struct node** node_list;
};


struct tree_edge {
    int slack;
    int x; // matching vector: is this edge in the pairing?
    struct node* vertex_plus;
    struct node* vertex_minus;
    struct tree_edge *parent;  // if this edge is in the tree: what is its parent?
};

int len_vertices;
struct node vertices[256];

int len_edges;
struct tree_edge edges[256];

int len_tree;
struct tree_edge tree[256];


int main() {

    return 0;
}


int grow(int delta_w) {
    int i;
    tree[0].vertex_plus += delta_w;
    for(i=1; i < len_tree; i++) {
        tree[i].vertex_plus->y += delta_w;
        tree[i].vertex_minus->y -= delta_w;
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

int add(struct tree_edge *parent, struct tree_edge *new_outer_edge, struct tree_edge *new_inner_edge) {
    struct tree_edge *new_te = &tree[len_tree++];

    new_te = new_inner_edge;



}

int shrink(int edge) {
}

int expand(int vertex) {
}
