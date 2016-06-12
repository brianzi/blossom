import start


def test_print_empty():
    g = start.Graph(100)
    g.show()


def test_add_three_vertices():
    g = start.Graph(100)
    g.add_vertex(0, 0, [1, 2])
    g.add_vertex(1, 1, [0, 2])
    g.add_vertex(2, 2, [0, 1])

    assert g.len_nodes == 3
    assert g.len_edges == 3
