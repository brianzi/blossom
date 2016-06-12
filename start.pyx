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

    def add_vertex(self, x, y):
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
        self.pos_x[index] = x
        self.pos_y[index] = y

        return index

    def add_edge(self, index1, index2):
        cdef node *n1
        cdef node *n2
        cdef edge *e

        n1 = &self.nodes[index1]
        n2 = &self.nodes[index2]

        e = &self.edges[self.len_edges]
        e.index = self.len_edges
        self.len_edges += 1

        e.x = 0
        e.slack = 0

        e.node_plus = n1
        e.node_minus = n2

        e.parent = NULL
        e.breadcrumb = NULL

        n1.edge_list[n1.n_edges] = e
        n1.n_edges += 1

        n2.edge_list[n2.n_edges] = e
        n2.n_edges += 1

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

    def plot(self, ax=None):
        if ax is None:
            ax = plt.gca()

        cdef int i

        xs = []
        ys = []

        for i in range(self.len_nodes):
            xs.append(self.pos_x[self.nodes[i].index])
            ys.append(self.pos_y[self.nodes[i].index])

        plt.scatter(xs, ys)

        for i in range(self.len_edges):
            xs = [self.pos_x[self.edges[i].node_minus.index],
                    self.pos_x[self.edges[i].node_plus.index]]
            ys = [self.pos_y[self.edges[i].node_minus.index],
                    self.pos_y[self.edges[i].node_plus.index]]

            plt.plot(xs, ys, ".-k")

    def __dealloc__(self):
        free(self.nodes)
        free(self.edges)
        free(self.incidence)
        free(self.pos_x)
        free(self.pos_y)


