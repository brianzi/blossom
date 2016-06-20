import numpy as np
import matplotlib.pyplot as plt


from libc.stdlib cimport malloc, free



cdef:
    struct node:
        int index

        int y

        edge *pair

        int n_edges
        edge **edge_list

        edge *cycle_start

        int n_restore
        node **restore_list

    struct edge:
        int index

        int slack
        
        node *node_plus
        node *node_minus

        edge *parent

        edge *cycle
        int tag


cdef void swap_nodes(edge *e):
    cdef node *t
    t = e.node_plus
    e.node_plus = e.node_minus
    e.node_minus = t


cdef class Graph:


    cdef node *nodes
    cdef public int len_nodes

    cdef edge *edges
    cdef public int len_edges

    cdef edge **incidence
    cdef public int len_incidence

    cdef node **restore 
    cdef public int len_restore

    cdef float *pos_x
    cdef float *pos_y

    cdef int len_tree
    cdef edge **tree
    cdef edge root

    cdef int len_outer_edges
    cdef edge **outer_edges

    cdef public int size, max_neighbors

    def __init__(self, size=256, max_neighbors=10):

        self.size = size
        self.max_neighbors = max_neighbors

        self.nodes = <node*> malloc(size*sizeof(node))
        self.edges = <edge*> malloc(size*sizeof(edge))
        self.incidence = <edge**> malloc(size*max_neighbors*sizeof(edge*))
        self.tree = <edge**> malloc(size*sizeof(edge*))
        self.restore = <node**> malloc(size*sizeof(node*))
        self.outer_edges = <edge**> malloc(size*sizeof(edge*))

        self.pos_x = <float*> malloc(size*sizeof(int))
        self.pos_y = <float*> malloc(size*sizeof(int))

        self.len_nodes = 0
        self.len_edges = 0
        self.len_incidence = 0
        self.len_tree = 0
        self.len_restore = 0
        self.len_outer_edges = 0

        self.root.node_plus = NULL
        self.root.node_minus = NULL
        self.root.parent = &self.root
        self.root.index = -1

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

        n.pair = NULL

        n.n_edges = 0
        n.edge_list = &self.incidence[self.len_incidence]
        self.len_incidence += self.max_neighbors
        
        n.n_restore = 0
        n.restore_list = NULL
        self.pos_x[index] = x
        self.pos_y[index] = y

        return index

    def add_edge(self, index1, index2):
        cdef node *n1
        cdef node *n2
        cdef edge *e

        assert index1 < self.len_nodes
        assert index2 < self.len_nodes

        n1 = &self.nodes[index1]
        n2 = &self.nodes[index2]

        e = &self.edges[self.len_edges]
        e.index = self.len_edges
        self.len_edges += 1

        e.slack = 0

        e.tag = 0

        e.node_plus = n1
        e.node_minus = n2

        e.parent = NULL
        e.cycle = NULL

        n1.edge_list[n1.n_edges] = e
        n1.n_edges += 1

        n2.edge_list[n2.n_edges] = e
        n2.n_edges += 1

        return e.index

    def show(self):
        cdef int i, j
        cdef node *n
        cdef edge *e

        print("Vertices:")

        for i in range(self.len_nodes):
            n = &self.nodes[i]
            print("%d (x=%g, y=%g):\n"%(n.index, self.pos_x[n.index], self.pos_y[n.index]))
            if n.pair != NULL and n.pair != <edge*> -1:
                print("pair: %d"%n.pair.index)
            for j in range(n.n_edges):
                print("   (%d, %d -- %d)"%
                    (n.edge_list[j].index,
                     n.edge_list[j].node_plus.index,
                     n.edge_list[j].node_minus.index))


        print("Edges:")
        for i in range(self.len_edges):
            e = &self.edges[i]
            print("(%d, %d -- %d) pnt=%d cyc=%d"%
                (e.index,
                 e.node_plus.index,
                 e.node_minus.index,
                 -1 if e.parent == NULL else e.parent.index,
                 -1 if e.cycle == NULL else e.cycle.index))

        print([self.outer_edges[i].index for i in range(self.len_outer_edges)])



    def plot(self, ax=None):
        cdef edge *e
        if ax is None:
            ax = plt.gca()

        cdef int i

        xs = []
        ys = []
        cs = []

        for i in range(self.len_nodes):
            plt.text(self.pos_x[self.nodes[i].index],
                self.pos_y[self.nodes[i].index],
                "%d"%self.nodes[i].index)
            xs.append(self.pos_x[self.nodes[i].index])
            ys.append(self.pos_y[self.nodes[i].index])

            cs.append("blue" if self.nodes[i].restore_list == NULL else "green")

        plt.scatter(xs, ys, s=50, c=cs)


        if <node*>self.root.node_plus != NULL:
            x = self.pos_x[self.root.node_plus.index]
            y = self.pos_y[self.root.node_plus.index]
            plt.scatter((x), (y), s=150, c="red")

        for i in range(self.len_edges):
            e = &self.edges[i]
            xs = [self.pos_x[e.node_minus.index],
                    self.pos_x[e.node_plus.index]]
            ys = [self.pos_y[e.node_minus.index],
                    self.pos_y[e.node_plus.index]]


            x = e.node_plus.pair == e

            if e.cycle != NULL:
                color = "green"
                lw = 1
            else:
                if e.parent != NULL and x:
                    color = "purple"
                    lw = 3
                if e.parent == NULL and x:
                    color = "red"
                    lw = 3
                if e.parent != NULL and not x:
                    color = "blue"
                    lw = 1
                if e.parent == NULL and not x:
                    color = "black"
                    lw = 1

            plt.plot(xs, ys, "-", lw=lw, color=color)

    def check_all(self):
        self.check_graph_integrity()
        self.check_no_tags()
        self.check_tree_integrity()
        self.check_outer_edges()

    def check_graph_integrity(self):
        cdef edge *e
        cdef edge *e2

        for ei in range(self.len_edges):
            e = &self.edges[ei]
            if e.cycle == NULL:
                for ei2 in range(e.node_plus.n_edges):
                    if e.node_plus.edge_list[ei2] == e:
                        e2 = e.node_plus.edge_list[ei2]
                        break
                    if e.node_minus.edge_list[ei2] == e:
                        e2 = e.node_minus.edge_list[ei2]
                        break
                assert e2 == e
    
    def check_no_tags(self):
        cdef edge *edge
        for ei in range(self.len_edges):
            edge = &self.edges[ei]
            assert edge.tag == 0

        assert self.root.tag == 0

    def check_tree_integrity(self):
        cdef edge* e
        cdef edge* ep
        cdef int tree_size = 0

        assert self.root.parent == &self.root

        for i in range(self.len_edges):
            e = &self.edges[i]

            if e.parent != NULL:
                tree_size += 1

            if e.parent != NULL and e.cycle == NULL:
                ep = e
                while ep.parent != ep and ep.parent != NULL:
                    ep = ep.parent

                assert ep == &self.root


                ep = e.parent
                if e.node_plus.pair == e:
                    # alternating property
                    assert ep.node_plus.pair != ep
                    assert e.node_minus == ep.node_minus
                if e.node_plus.pair != e:
                    # alternating property
                    assert e.node_plus == ep.node_plus
                    if ep != &self.root:
                        assert ep.node_plus.pair == ep
                        assert ep.parent != NULL
                    # unpaired edges cannot be leaves
                    assert e.node_minus.pair != NULL
                    assert e.node_minus.pair.parent == e


                # follow up to see if root can be found
        assert tree_size == self.len_tree

    cdef void add_outer_edge(self, edge *e, node *node_in_tree):
        assert node_in_tree.pair != NULL
        assert node_in_tree.pair.parent != NULL
        if e.parent == NULL:
            self.outer_edges[self.len_outer_edges] = e
            self.len_outer_edges += 1
            if e.node_plus != node_in_tree:
                swap_nodes(e)

    def check_outer_edges(self):
        cdef edge *e
        cdef edge *te
        for i in range(self.len_outer_edges):
            e = self.outer_edges[i]

            assert e.node_plus.pair != NULL
            if e.node_plus.pair != <edge*> -1:
                assert e.node_plus.pair.parent != NULL

        for ti in range(self.len_tree):
            te = self.tree[ti]
            for ei in range(te.node_plus.n_edges):
                e = te.node_plus.edge_list[ei]
                if e.parent == NULL:
                    for oi in range(self.len_outer_edges):
                        if self.outer_edges[oi] == e:
                            break
                    assert self.outer_edges[oi] == e


                

    def augment(self, edge_index):
        cdef edge *unpaired_e
        cdef edge *edge_in_tree

        assert edge_index < self.len_edges 



        unpaired_e = &self.edges[edge_index]

        assert unpaired_e.node_plus.pair != unpaired_e
        assert unpaired_e.node_minus.pair != unpaired_e


        if unpaired_e.node_plus.pair == NULL:
            swap_nodes(unpaired_e)

        assert unpaired_e.node_plus.pair != NULL
        assert unpaired_e.node_plus.pair.parent != NULL

        edge_in_tree = unpaired_e.node_plus.pair

        self._pair_edge(unpaired_e)

        while edge_in_tree.parent != edge_in_tree:
            self._pair_edge(edge_in_tree.parent)
            edge_in_tree = edge_in_tree.parent.parent

        for i in range(self.len_tree):
            self.tree[i].parent = NULL
        
        self.len_tree = 0
        self.len_outer_edges = 0

        self.root.node_plus = NULL

    def grow(self, edge_index):
        cdef edge *unpaired_e
        cdef edge *edge_in_tree
        cdef edge *new_paired_edge

        assert edge_index < self.len_edges

        unpaired_e = &self.edges[edge_index]

        assert unpaired_e.node_minus.pair != NULL 
        assert unpaired_e.node_plus.pair != NULL 


        if unpaired_e.node_minus.pair.parent != NULL:
            swap_nodes(unpaired_e)


        new_paired_edge = unpaired_e.node_minus.pair
        #assert new_paired_edge.parent == NULL


        if new_paired_edge.node_plus == unpaired_e.node_minus:
            swap_nodes(new_paired_edge)


        unpaired_e.parent = unpaired_e.node_plus.pair
        new_paired_edge.parent = unpaired_e


        self.tree[self.len_tree] = unpaired_e
        self.len_tree += 1
        self.tree[self.len_tree] = new_paired_edge
        self.len_tree += 1

        for i in range(new_paired_edge.node_plus.n_edges):
            self.add_outer_edge(new_paired_edge.node_plus.edge_list[i], 
                    new_paired_edge.node_plus)




    def shrink(self, edge_index):
        cdef edge *unpaired_e
        cdef node *blossom
        cdef edge *blossom_pair_parent
        cdef node *lonely_node
        cdef edge *e


        assert edge_index < self.len_edges
        unpaired_e = &self.edges[edge_index]

        assert unpaired_e.node_plus.pair != NULL 
        assert unpaired_e.node_plus.pair.parent != NULL
        assert unpaired_e.node_minus.pair != NULL 
        assert unpaired_e.node_minus.pair.parent != NULL
        
        blossom = &self.nodes[self.len_nodes]
        blossom.index = self.len_nodes
        self.len_nodes += 1

        blossom.n_edges = 0
        blossom.restore_list = &self.restore[self.len_restore]
        blossom.edge_list = &self.incidence[self.len_incidence]

        self.pos_x[blossom.index] = 0
        self.pos_y[blossom.index] = 0


        blossom.cycle_start = unpaired_e

        ### step 1: find the cycle:


        #leave breadcrumbs
        e = unpaired_e.node_plus.pair
        e.cycle = unpaired_e
        while e.parent != e:
            e.parent.cycle = e
            e = e.parent


        
        e = unpaired_e.node_minus.pair
        if e != &self.root:
            while e.parent.cycle == NULL:
                e.cycle = e.parent
                e = e.parent
            e.cycle = e.parent.cycle
            unpaired_e.cycle = unpaired_e.node_minus.pair
        else:
            print("x")
            unpaired_e.cycle = self.root.cycle

            



        blossom.pair = e.parent
        blossom_pair_parent = blossom.pair.parent
        lonely_node = blossom.pair.node_plus


        #all edges in the blossom now point along the cycle with parent pointers

        ### step 2: collect the edges

        #run along the cycle once to mark the edges that should be added
        e = unpaired_e.cycle
        while e != unpaired_e:
            for i in range(e.node_plus.n_edges):
                e.node_plus.edge_list[i].tag ^= 1
            for i in range(e.node_minus.n_edges):
                e.node_minus.edge_list[i].tag ^= 1
            e = e.cycle.cycle
        for i in range(e.node_plus.n_edges):
            e.node_plus.edge_list[i].tag ^= 1

        #run a second time to add them to the edge and restore lists and change the node ends

        cnt = 1

        e = unpaired_e.cycle
        while e != unpaired_e:
            self.pos_x[blossom.index] += self.pos_x[e.node_plus.index]
            self.pos_y[blossom.index] += self.pos_y[e.node_plus.index]
            self.pos_x[blossom.index] += self.pos_x[e.node_minus.index]
            self.pos_y[blossom.index] += self.pos_y[e.node_minus.index]
            for i in range(e.node_plus.n_edges):
                if e.node_plus.edge_list[i].tag != 0:
                    self.incidence[self.len_incidence] = e.node_plus.edge_list[i]
                    self.len_incidence += 1
                    self.restore[self.len_restore] = e.node_plus
                    self.len_restore += 1
                    if e.node_plus.edge_list[i].node_plus == e.node_plus:
                        e.node_plus.edge_list[i].node_plus = blossom
                    else:
                        e.node_plus.edge_list[i].node_minus = blossom
                    blossom.n_edges += 1
                    e.node_plus.edge_list[i].tag = 0
                    self.add_outer_edge(e, e.node_plus)
                    if e.node_plus.edge_list[i].parent != NULL:
                        e.node_plus.edge_list[i].parent = blossom.pair

            for i in range(e.node_minus.n_edges):
                if e.node_minus.edge_list[i].tag != 0:
                    self.incidence[self.len_incidence] = e.node_minus.edge_list[i]
                    self.len_incidence += 1
                    self.restore[self.len_restore] = e.node_minus
                    self.len_restore += 1
                    if e.node_minus.edge_list[i].node_minus == e.node_minus:
                        e.node_minus.edge_list[i].node_minus = blossom
                    else:
                        e.node_minus.edge_list[i].node_plus = blossom
                    blossom.n_edges += 1
                    self.add_outer_edge(e, e.node_minus)
                    e.node_minus.edge_list[i].tag = 0


            cnt += 2
                
            e = e.cycle.cycle

        self.pos_x[blossom.index] += self.pos_x[e.node_plus.index]
        self.pos_y[blossom.index] += self.pos_y[e.node_plus.index]
        for i in range(e.node_plus.n_edges):
            if e.node_plus.edge_list[i].tag != 0:
                self.incidence[self.len_incidence] = e.node_plus.edge_list[i]
                self.len_incidence += 1
                self.restore[self.len_restore] = e.node_plus
                self.len_restore += 1
                if e.node_plus.edge_list[i].node_plus == e.node_plus:
                    e.node_plus.edge_list[i].node_plus = blossom
                else:
                    e.node_plus.edge_list[i].node_minus = blossom
                blossom.n_edges += 1
                e.node_plus.edge_list[i].tag = 0
                self.add_outer_edge(e, e.node_plus)
                if e.node_plus.edge_list[i].parent != NULL:
                    e.node_plus.edge_list[i].parent = blossom.pair


        blossom.pair.node_plus = blossom


        blossom.pair.parent = blossom_pair_parent
        lonely_node.pair = <edge*> -1



        self.pos_x[blossom.index] /= cnt
        self.pos_y[blossom.index] /= cnt

        

        #clean up cycle
        e = blossom.pair
        while e.parent != e:
            e.cycle = NULL
            e = e.parent
        e.cycle = NULL

    def expand(self, blossom_index):
        cdef node *blossom
        cdef edge *pair
        cdef edge *e
        cdef edge *start
        cdef edge *e2

        print(1)

        assert self.len_nodes > blossom_index

        blossom = &self.nodes[blossom_index]

        assert blossom.restore_list != NULL


        if blossom.pair.node_plus == blossom:
            swap_nodes(blossom.pair)
        print(2)

        # restore graph structure
        for i in range(blossom.n_edges):
            e = blossom.edge_list[i]
            if e.node_plus == blossom:
                e.node_plus = blossom.restore_list[i]
            else:
                e.node_minus = blossom.restore_list[i]
        print(3)

        # re-pair blossom if necessary
        print(blossom.pair.node_minus.pair.index)
        if blossom.pair.node_minus.pair != <edge*> -1:
            print(3.1)
            start = blossom.pair.node_minus.pair.cycle

            print(3.2)

            if start.node_plus == blossom.pair.node_minus:
                print(3.21)
                start = start.cycle
            if start.node_minus == blossom.pair.node_minus:
                print(3.22)
                start = start.cycle

            print(3.3)

            e = start

            while e.cycle != start:
                self._pair_edge(e)
                e = e.cycle.cycle
            print(3.4)

            blossom.pair.node_minus.pair = blossom.pair

        print(4)
        # regrow the tree

        
        e2 = blossom.pair.parent

        if (e2.node_minus.pair.cycle.node_minus == e2.node_minus or
                e2.node_minus.pair.cycle.node_plus == e2.node_minus):
            # tree grows against cycle


            # run up to the pairing node
            e = e2
            e2 = e2.node_minus.pair
            start = e2
            
            while (e2.node_plus.pair != blossom.pair and
                    e2.node_minus.pair != blossom.pair):
                e2 = e2.cycle


            e2 = e2.cycle

            blossom.pair.parent = e2

            if e2.node_plus.pair == blossom.pair:
                swap_nodes(e2)

            while e2 != start:
                if e2.node_minus.pair == e2:
                    if e2.cycle.node_plus == e2.node_minus:
                        swap_nodes(e2.cycle)
                else:
                    if e2.cycle.node_minus == e2.node_plus:
                        swap_nodes(e2.cycle)
                e2.parent = e2.cycle
                self.tree[self.len_tree] = e2
                self.len_tree += 1
                e2 = e2.cycle


            e2.parent = e
            self.tree[self.len_tree] = e2
            self.len_tree += 1


        else:
            # tree grows along cycle
            while e2.node_minus.pair != blossom.pair:
                e = e2
                e2 = e.node_minus.pair

                if e2.node_plus == e.node_minus:
                    swap_nodes(e2)

                e2.parent = e

                self.tree[self.len_tree] = e
                self.len_tree += 1
                e = e2
                e2 = e.cycle

                if e2.node_minus == e.node_plus:
                    swap_nodes(e2)

                e2.parent = e
                self.tree[self.len_tree] = e2
                self.len_tree += 1

            blossom.pair.parent = e2
        print(5)

        # delete cycle information
        start = blossom.cycle_start
        e = start.cycle
        while e != start:
            e2 = e.cycle
            e.cycle = NULL
            e = e2
        e.cycle = NULL


    def get_edge(self, edge_index):
        cdef edge *e
        e = &self.edges[edge_index]
        r = {
                "index": e.index,
                "slack": e.slack,
                "node_plus": e.node_plus.index,
                "node_minus": e.node_minus.index,
                }
        if e.parent != NULL:
            r["parent"] = e.parent.index
        else:
            r["parent"] = None

        return r

    def get_node(self, node_index):
        cdef node *n
        cdef int i
        n = &self.nodes[node_index]
        r = {
                "index": n.index,
                "y": n.y,
                }

        edge_list = []
        for i in range(n.n_edges):
            edge_list.append(n.edge_list[i].index)

        r["edge_list"] = edge_list

        if n.pair != NULL:
            r["pair"] = n.pair.index
        else:
            r["pair"] = None

        return r

    def get_root(self):
        if self.root.node_plus != NULL:
            return self.root.node_plus.index
        else :
            return -1 

    def raw_set_root(self, root_node_index):
        assert self.root.node_plus == NULL
        assert root_node_index < self.len_nodes
        self.root.node_plus = &self.nodes[root_node_index]
        self.nodes[root_node_index].pair = &self.root

        for i in range(self.root.node_plus.n_edges):
            self.add_outer_edge(self.root.node_plus.edge_list[i], self.root.node_plus)

    def set_root(self):
        cdef node *n

        assert self.root.node_plus == NULL

        for i in range(self.len_nodes):
            if self.nodes[i].pair == NULL:
                n = &self.nodes[i]
                break
        else:
            return -1

        self.root.node_plus = n
        n.pair = &self.root
        for i in range(n.n_edges):
            self.add_outer_edge(n.edge_list[i], n)

        return n.index


    def get_outer_edges(self):
        outer_edges = []
        for i in range(self.len_outer_edges):
            outer_edges.append(self.outer_edges[i].index)

        return outer_edges


    def raw_set_parent(self, parent_index, child_index):

        cdef edge *p
        cdef edge *e

        p = &self.edges[parent_index]
        e = &self.edges[child_index]

        e.parent = p

    cdef void _pair_edge(self, edge* edge):
        edge.node_plus.pair = edge
        edge.node_minus.pair = edge

    def pair_edge(self, edge_index, pair=True):
        cdef edge *e
        e = &self.edges[edge_index]
        self._pair_edge(e)

    def __dealloc__(self):
        free(self.nodes)
        free(self.edges)
        free(self.incidence)
        free(self.restore)
        free(self.tree)
        free(self.outer_edges)
        free(self.pos_x)
        free(self.pos_y)
    
