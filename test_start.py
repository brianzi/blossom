import start
import pytest


@pytest.fixture()
def triangle_graph():
    g = start.Graph()
    v1 = g.add_vertex(0, 0)
    v2 = g.add_vertex(1, 0)
    v3 = g.add_vertex(0, 1)

    g.add_edge(v1, v2)
    g.add_edge(v1, v3)
    g.add_edge(v2, v3)

    return g


def test_print_empty():
    start.Graph()


def test_set_root(triangle_graph):
    g = triangle_graph

    i = g.set_root()
    assert i == 0


def test_add_three_vertices(triangle_graph):
    g = triangle_graph

    assert g.len_nodes == 3
    assert g.len_edges == 3

@pytest.mark.xfail()
def test_show(triangle_graph, capsys):
    g = triangle_graph
    g.show()
    out, err = capsys.readouterr()

    assert err == ""
    assert out == """Vertices:
0 (x=0, y=0):

   (0, 0 -- 1)
   (1, 0 -- 2)
1 (x=1, y=0):

   (0, 0 -- 1)
   (2, 1 -- 2)
2 (x=0, y=1):

   (1, 0 -- 2)
   (2, 1 -- 2)
Edges:
(0, 0 -- 1) pnt=-1 cyc=-1 id=0
(1, 0 -- 2) pnt=-1 cyc=-1 id=0
(2, 1 -- 2) pnt=-1 cyc=-1 id=0
[]
"""


def test_getters(triangle_graph):
    g = triangle_graph

    n = g.get_node(0)
    assert n["index"] == 0
    assert n["y"] == 0
    assert len(n["edge_list"]) == 2

    e = g.get_edge(n["edge_list"][0])

    assert {e["node_plus"], e["node_minus"]} == {0, 1}


def test_pair_edge(triangle_graph):
    g = triangle_graph
    n = g.get_node(0)
    e = g.get_edge(n["edge_list"][0])

    g.pair_edge(e['index'])

    n = g.get_node(e['node_minus'])
    assert n['pair'] == e['index']
    n = g.get_node(e['node_plus'])
    assert n['pair'] == e['index']


def test_simple_empty_graph_is_consistent(triangle_graph):
    triangle_graph.check_all()


def test_check_integrity_ladder():
    # make a ladder graph
    g = start.Graph()
    for i in range(20):
        g.add_vertex(0, 0)

    for i in range(10):
        for j in range(10):
            id = g.add_edge(i, j + 10)
            if i == j and i != 0:
                g.pair_edge(id, True)

    g.raw_set_root(0)

    g.check_all()


def test_bv_example_shrink():

    bv = start.Graph()

    bv.add_vertex(0, 5)
    bv.add_vertex(-1, 4)
    bv.add_vertex(1, 4)
    bv.add_vertex(-1, 3)
    bv.add_vertex(1, 3)
    bv.add_vertex(-2, 2)
    bv.add_vertex(0, 2)
    bv.add_vertex(-2, 1)
    bv.add_vertex(0, 1)
    bv.add_vertex(-2, 3)
    bv.add_vertex(-2.5, 4)
    bv.add_vertex(-2.5, 5)
    bv.add_vertex(0, 6)

    m = {}
    for u, v in {(0, 1), (0, 2), (1, 3), (2, 4), (3, 5), (3, 6),
                 (5, 7), (6, 8), (3, 9), (9, 10), (10, 11), (3, 4), (0, 12)}:
        idx = bv.add_edge(u, v)
        m[u, v] = idx

    for (u, v) in {(1, 3), (2, 4), (5, 7), (6, 8), (10, 11)}:
        bv.pair_edge(m[u, v])

    bv.raw_set_root(0)

    bv.grow(m[0, 1])
    bv.grow(m[3, 5])
    bv.grow(m[0, 2])

    bv.shrink(m[3, 4])

    bv.check_all()


def test_bv_example_shrink2():
    bv = start.Graph()

    bv.add_vertex(0, 5)
    bv.add_vertex(-1, 4)
    bv.add_vertex(1, 4)
    bv.add_vertex(-1, 3)
    bv.add_vertex(1, 3)
    bv.add_vertex(-2, 2)
    bv.add_vertex(0, 2)
    bv.add_vertex(-2, 1)
    bv.add_vertex(0, 1)
    bv.add_vertex(-2, 3)
    bv.add_vertex(-2.5, 4)
    bv.add_vertex(-2.5, 5)
    bv.add_vertex(0, 6)
    bv.add_vertex(0.5, 6)

    m = {}
    for u, v in {(0, 1), (0, 2), (1, 3), (2, 4), (3, 5), (4, 6), (5, 7),
                 (6, 8), (3, 9), (9, 10), (10, 11), (3, 4), (0, 12), (12, 13)}:
        idx = bv.add_edge(u, v)
        m[u, v] = idx

    bv.set_root()

    bv.augment(bv.get_outer_edges()[0])

    bv.set_root()

    bv.grow(6)

    bv.augment(12)

    bv.set_root()

    bv.grow(11)
    bv.grow(6)
    bv.shrink(1)

    bv.augment(4)

    bv.raw_set_root(5)
    bv.grow(13)

    bv.expand(14)

    bv.augment(3)

    bv.set_root()

    bv.grow(bv.get_outer_edges()[0])

    bv.augment(10)

    bv.check_all()


def test_bv_example_expand():

    bv = start.Graph()

    bv.add_vertex(0, 5)
    bv.add_vertex(-1, 4)
    bv.add_vertex(1, 4)
    bv.add_vertex(-1, 3)
    bv.add_vertex(1, 3)
    bv.add_vertex(-2, 2)
    bv.add_vertex(0, 2)
    bv.add_vertex(-2, 1)
    bv.add_vertex(0, 1)
    bv.add_vertex(-2, 3)
    bv.add_vertex(-2.5, 4)
    bv.add_vertex(-2.5, 5)
    bv.add_vertex(0, 6)
    bv.add_vertex(0.5, 6)

    m = {}
    for u, v in {(0, 1), (0, 2), (1, 3), (2, 4), (3, 5), (4, 6), (5, 7),
                 (6, 8), (3, 9), (9, 10), (10, 11), (3, 4), (0, 12), (12, 13)}:
        idx = bv.add_edge(u, v)
        m[u, v] = idx

    # for (u,v) in {(1,3), (2,4), (5,7), (10, 11), (0,12)}:
        # bv.pair_edge(m[u,v])

    bv.set_root()

    bv.augment(bv.get_outer_edges()[0])

    bv.set_root()

    bv.grow(6)

    bv.augment(12)

    bv.set_root()

    bv.grow(1)
    bv.grow(6)
    bv.shrink(11)
    bv.augment(8)

    bv.set_root()
    bv.grow(13)
    bv.augment(5)

    bv.set_root()

    bv.grow(4)
    bv.expand(14)

    bv.check_all()


def test_bv_example_pair_all():
    bv = start.Graph()

    bv.add_vertex(0, 5)
    bv.add_vertex(-1, 4)
    bv.add_vertex(1, 4)
    bv.add_vertex(-1, 3)
    bv.add_vertex(1, 3)
    bv.add_vertex(-2, 2)
    bv.add_vertex(0, 2)
    bv.add_vertex(-2, 1)
    bv.add_vertex(0, 1)
    bv.add_vertex(-2, 3)
    bv.add_vertex(-2.5, 4)
    bv.add_vertex(-2.5, 5)
    bv.add_vertex(0, 6)
    bv.add_vertex(0.5, 6)

    m = {}
    for u, v in {(0, 1), (0, 2), (1, 3), (2, 4), (3, 5), (4, 6), (5, 7),
                 (6, 8), (3, 9), (9, 10), (10, 11), (3, 4), (0, 12), (12, 13)}:
        idx = bv.add_edge(u, v)
        m[u, v] = idx

    bv.pair_all()

    bv.check_all()
