import numpy as np
import matplotlib.pyplot as plt


from stdlib cimport malloc, free



cdef:
    struct node:
        int index

        int y
        int l

        struct edge *pair

        int n_edges
        struct edge **edge_list

        int n_subnodes
        struct node **subnode_list

    struct edge:
        int index

        int slack
        int x

        struct node *node_plus
        struct node *node_minus

        struct edge *parent

        struct edge *breadcrumb



cdef Graph:

    cdef node *nodes
    cdef int len_nodes

    cdef edge *edges
    cdef int len_edges

    cdef edge **incidence
    cdef int len_incidence

    cdef int *pos_x
    cdef int *pos_y

    def __init__(self, size=256):

        self.nodes = malloc(size*sizeof(node))
        self.edges = malloc(size*sizeof(edge))
        self.incidence = malloc(size*sizeof(*edge))

        self.pos_x = malloc(size*sizeof(int))
        self.pos_y = malloc(size*sizeof(int))

        self.len_nodes = 0
        self.len_edges = 0
        self.len_incidence = 0


    def add_vertex(x, y, adj_node_list):
        cdef int index
        cdef node *n, *n2
        cdef edge *e

        index = self.len_nodes
        len_nodes += 1
        
        n = &self.nodes[index]

        n->index = index
        n->y = 0
        n->l = 0

        n->pair = NULL

        n->n_subnodes = 1
        n->subnode_list = NULL

        for n2_index in adj_node_list:
            n2 = &self.nodes[n2_index]
            
            e = &self.edges[self.len_edges]
            e->index = self.len_edges
            self.len_egdes += 1

            e->slack = 0
            e->x = 0

            e->parent = NULL
            e->breadcrumb = NULL

            e->node_plus = n
            e->node_minus = n2

        return index

    def print(self):
        cdef int i, j
        cdef node *n
        cdef edge *e

        print("Vertices:")

        for i in range(self.len_vertices):
            n = &self.nodes[i]
            print("%d: \n   "%n->index)
            for j in range(n->n_edges):
                print("   (%d, %d -- %d)"%
                    (n->edge_list[j]->index,
                     n->edge_list[j]->node_plus,
                     n->edge_list[j]->node_minus))


        print("Edges: ")
            for i in range(self.edges):
                e = &self.edges[i]
                print("(%d, %d -- %d)"%
                    (e->index,
                     e->node_plus,
                     e->node_minus))





    def __dealloc__(self):
        free(self.nodes)
        free(self.edges)
        free(self.incidence)
        free(self.pos_x)
        free(self.pos_y)


