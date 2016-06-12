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


def test_add_three_vertices(triangle_graph):
    g = triangle_graph

    assert g.len_nodes == 3
    assert g.len_edges == 3


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
(0, 0 -- 1)
(1, 0 -- 2)
(2, 1 -- 2)
"""

def test_getters(triangle_graph):
    g = triangle_graph

    n = g.get_node(0)
    assert n["index"] == 0
    assert n["y"] == 0
    assert len(n["edge_list"]) == 2

    e = g.get_edge(n["edge_list"][0])

    assert {e["node_plus"], e["node_minus"]} == {0, 1}
