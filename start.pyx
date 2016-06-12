import numpy as np
import matplotlib.pyplot as plt


from libc.stdlib cimport malloc, free



cdef:
    struct node:
        int index

        int y
        int l

        edge *pair

        int n_edges
        edge **edge_list

        int n_subnodes
        node **subnode_list

    struct edge:
        int index

        int slack
        int x

        node *node_plus
        node *node_minus

        edge *parent

        edge *breadcrumb



cdef class Graph:

    cdef node *nodes
    cdef public int len_nodes

    cdef edge *edges
    cdef public int len_edges

    cdef edge **incidence
    cdef public int len_incidence

    cdef int *pos_x
    cdef int *pos_y

    cdef public int size, max_neighbors

    def __init__(self, size=256, max_neighbors=10):

        self.size = size
        self.max_neighbors = max_neighbors

        self.nodes = <node*> malloc(size*sizeof(node))
        self.edges = <edge*> malloc(size*sizeof(edge))
        self.incidence = <edge**> malloc(size*max_neighbors*sizeof(edge*))

        self.pos_x = <int*> malloc(size*sizeof(int))
        self.pos_y = <int*> malloc(size*sizeof(int))

        self.len_nodes = 0
        self.len_edges = 0
        self.len_incidence = 0

    def add_vertex(self, x, y, adj_node_list):
        cdef int index
        cdef node *n
        cdef node *n2
        cdef edge *e

        index = self.len_nodes
        self.len_nodes += 1
        
        n = &self.nodes[index]

        n.index = index
        n.y = 0
        n.l = 0

        n.pair = NULL

        n.n_subnodes = 1
        n.subnode_list = NULL

        n.n_edges = 0
        n.edge_list = &self.incidence[self.max_neighbors * index]
        for n2_index in adj_node_list:

            n2 = &self.nodes[n2_index]
            if n2_index < index:
                # edge should already be created, find it
                for i in range(n2.n_edges):
                    e = n2.edge_list[i]
                    if e.node_minus.index == index:
                        break
            else:
                # create new edge 
                e = &self.edges[self.len_edges]
                e.index = self.len_edges
                self.len_edges += 1

                e.slack = 0
                e.x = 0

                e.parent = NULL
                e.breadcrumb = NULL

                e.node_plus = n
                e.node_minus = n2
            
            n.edge_list[n.n_edges] = e
            n.n_edges += 1

        self.pos_x[index] = x
        self.pos_y[index] = y

        return index

    def show(self):
        cdef int i, j
        cdef node *n
        cdef edge *e

        print("Vertices:")

        for i in range(self.len_nodes):
            n = &self.nodes[i]
            print("%d (x=%d, y=%d):\n"%(n.index, self.pos_x[n.index], self.pos_y[n.index]))
            for j in range(n.n_edges):
                print("   (%d, %d -- %d)"%
                    (n.edge_list[j].index,
                     n.edge_list[j].node_plus.index,
                     n.edge_list[j].node_minus.index))


        print("Edges:")
        for i in range(self.len_edges):
            e = &self.edges[i]
            print("(%d, %d -- %d)"%
                (e.index,
                 e.node_plus.index,
                 e.node_minus.index))

    def plot(self):
        pass

    def __dealloc__(self):
        free(self.nodes)
        free(self.edges)
        free(self.incidence)
        free(self.pos_x)
        free(self.pos_y)


