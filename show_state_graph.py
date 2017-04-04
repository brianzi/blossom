import pandas as pd
from collections import defaultdict

import graphviz as gv


def make_plot(df, outfile):
    g = gv.Graph(format='png')
    none = 99

    nodes = defaultdict(list)

    for i in df['vert']:
        if i not in nodes:
            nodes[i] = []

        if df['bl_par'][i] != none:
            nodes[df['bl_par'][i]].append(i)

    # curr_tree_idx = max(-1 if i == none else i for i in df['treeidx'])

    def make_blossom_subgraph(b, children):
        sg = gv.Graph(str(b))
        style = dict(label=str(b) + ", d=" + str(df['dual*'][b]))
        if df['treeidx'][b] == curr_tree_idx:
            style['style'] = 'bold'
            style['label'] += ', dit='+str(df['dist_it'][b])
        sg.node(str(b),  **style)
        for c in children:
            if nodes[c]:
                sg.subgraph(make_blossom_subgraph(c, nodes[c]))
            else:
                sg.node(str(c), label=str(c) + ", d=" + str(df['dual*'][c]))
            # else:
                # sg.node(str(c), color="green")
            sg.edge(str(b), str(c), style='dashed')

        return sg

    top_level = list(nodes.keys())

    for b, cs in nodes.items():
        for c in cs:
            top_level.remove(c)

    for b in top_level:
        cs = nodes[b]
        if cs:
            sg = make_blossom_subgraph(b, cs)
            g.subgraph(sg)
        else:
            style = dict(label=str(b) + ", d=" + str(df['dual*'][b]))
            if df['treeidx'][b] == curr_tree_idx:
                style['style'] = 'bold'
                style['label'] += ', dit='+str(df['dist_it'][b])
            g.node(str(b),  **style)

    for (n, m), weight in edges.items():
        try:
            weight = weight - df['dual*'][n] - df['dual*'][m]

            style = {}
            if df['match'][n] == m and df['match'][n] != none:
                style['color'] = 'red'

            if df['treepar'][n] == m or df['treepar'][m] == n:
                if df['treeidx'][n] == curr_tree_idx and df['treeidx'][m] == curr_tree_idx:
                    style['style'] = 'bold'
            # if weight < 0:
                # pass
            if df['bl_par'][n] == df['bl_par'][m] and df['bl_par'][n] != none:
                if df['treepar'][n] == m or df['treepar'][m] == n:
                    # style['style'] = 'dotted'
                    pass
                else:
                    continue
            elif df['bl_par'][n] != none or df['bl_par'][m] != none:
                continue

            fr, to = str(n), str(m)

            g.edge(fr, to, label=str(weight), **style)

        except KeyError:
            pass

    g.render(outfile, directory='graph_out')


infile = open("out.dat", "r")

buf = []
heading = []

cnt = 0

edges = {}

curr_tree_idx = -1

for l in infile.readlines():
    if l.startswith('  current_tree_index='):
        curr_tree_idx = int(l.replace('  current_tree_index=', '').strip())
    if l.startswith('\t'):
        if l.startswith('\tvert'):
            df = pd.DataFrame(list(map(lambda x: map(int, x), buf)), columns=heading)
            if len(df) > 0:
                make_plot(df, "p%04d" % (cnt))
            if cnt >= 200:
                break
            cnt += 1
            heading = l.strip().split('\t')
            buf = []
        else:
            buf.append(l.strip().split('\t'))
    if l.startswith('DUMPGRAPH:'):
        dg, n1, n2s = l.split(':')

        n1 = int(n1.strip())
        for e in list(edges.keys()):
            if n1 in e:
                del edges[e]

        for n2w in n2s.split(';')[:-1]:
            n2, w = n2w.split(',')

            n2 = int(n2.strip())
            w = int(w.strip())

            edges[min(n1, n2), max(n1, n2)] = w
