The plan
--------



Invariants: 
  - graph structure: each node refers to the nodes that in turn refer to the right edges. (Should also be connected!)
  - matching: the matching of should always be a (not complete) matching vector. A node indicating an edge as pairing should be indicated by both nodes.
  - tree: should be connected, should be an alternating tree.
  - outer edges list should hold the outer edges of the tree.


We associate with each edge the following information:
  - slack: The slackness that we use to find the next instruction
  - node plus and node minus. The two nodes connected by this edge. Initially, two vertices. These change when blossoms are shrunk or expanded.
  - parent edge. When the edge is in the alternating tree, this edge is the edge one step closer to the root. 
    If x(e)=1, x(parent(e))=0 and vice versa, by the defnition of the alternating tree.
  - cycle edge. When the tree is in a blossom, should point along the blossom cycle. 


We associate with each node the following information:
  - y: The dual variable related to a constraint due to this node
  - pair edge: The edge this node is a pairing of, if any. NULL if the edge is not paired yet. `<edge*> -1` if the node is a 'lonely node', i.e. the only unpaired 
node in a blossom.
  - edge list: a list of edges that emanate from this node.
  - If the node is a blossom, we also need:
    - restore list: for each edge in the edge list, we store the subnode that this edge is was connected to before forming the blossom. 
  
  
Note that len(edge list) = len(restore list), we could align the two in one list. (we don't do right now)

An alternating tree is built with an unpaired edge at the root, and then alternatingly paired and unpaired (x=0, 1) edges as descendents, leaves must always be paired. 

We also keep
  - treelist: List of all edges that are currently in the tree.
  - outer edges: list of all edges that are not in tree but incident to a (+) node in the tree
  - inner edges: all edges not in tree but incident to (-) node in tree


During the growth of the tree, these three lists change. We keep track of this by storing them in the list together with a counter label, which we also
add to the edge. If the counters do not agree anymore, the entry in the list is considered invalid.


Other approach: each list has a tag bitfield with the following information:
  - tag[0]: working tag
  - tag[1]: the edge is in a blossom
  - tag[2]: the edge is an inner edge from the minus node
  - tag[3]: the edge is an outer edge from the plus node
  - tag[4]: the edge is a double outer edge

If the edge is in the tree, 2 and 3 are set.

Note that the tree is considered a tree of edges, not a tree of nodes! (better structure in my opinion)

Blossom algorithm requires the following operations:

grow: for all (+)/(-) vertices in the graph, increase/decrease y by dw. Input: whole tree

- add: add a paired and an unpaired edge to the tree. 

This is done providing an unpaired edge not in the tree, and a paired edge in the tree defining the position where to add. 
The unpaired edge must be incident on the (+) vertex of the paired edge.
From the unpaired edge, the other node is found, and from the node, the other paired edge is found.

Later we will in this step also examine all outer edges of the newly added edge and add instructions to make them tight to the 
instruction list.

- make blossom: create a new node ("blossom"). 

As input, give two paired edges in the tree whose (+) nodes should be joined.

In a first step, we need to find the cycle formed by the tree and this additional edge.
We backtrack the first node to the root, leaving breadcrumbs (forward references from parent to child).
Then we start building the cycle, starting from e2, backtracking towards the root and adding 
the encountered nodes to the nodes list, until we hit the trace of breadcrumbs. We
then follow the trace in forward direction until we end at e2.  

Probably it is possible to save the structure of blossoms in the same structure for the tree information (parent edge)

[note: a better way of doing this is to save the depth of each edge in the tree:
First walk back from the deeper edge until reached the same level. If this is the other edge, done.
If not, go backwards each steps from both sides until meeting in one edge. While doing this, reverse the 
parent chain of the second edge immediately. Better algorithms would use path compression tricks]

While collecting the nodes, we also use the edge list of the nodes to tag all 
edges that are incident with one of the nodes modulo 2.

We then traverse the circle a second time to build the edge list and restore list of the new node 
(including only outer edges, using the tags generated in the previous step), and change all the edges
to be incident with new node.

Here is a problem: We don't know which of the two nodes stored in the edge is the one that must be changed. Is there anything we can do except an if clause?

- expand blossom. 

Essentially the inverse of make_blossom. Use the restore list to restore the previous node structure. While doing so, the node must be paired, and 
that pairing is used to settle the pairing inside the blossom (which might be different from the incomplete pairing left behind when the blossom was formed)

The input is the node and the edge matching the node. We follow the restore list and revert all edges to their previous state. At the same time,
we follow along in the subnode list. When we encounter the matching edge of the blossom, we start toggling the matchings of
edges of the cycle.

- augment: Given an unpaired egde emanating from the tree, add it to the tree by letting x <- 1-x along the path from (including) the unpaired edge to the root of the tree. 
The tree is then "done" and a new tree is formed.

