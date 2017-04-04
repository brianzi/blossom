import graphviz as gv

import sys


assert len(sys.argv) >= 2


g = gv.Graph()

with open(sys.argv[1], "r") as f:
    for s in f.readlines():
        a, b, c = s.strip().split(" ")
        g.edge(a, b, label=c)


g.view()
