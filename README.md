The plan
--------



We associate with each edge the following information:
  - slack: The slackness that we use to find the next instruction
  - x: Whether this edge is in the matching (=primal variable), is 0 or 1
  - node plus and node minus. The two nodes connected by this edge. Initially, two vertices.
  - parent edge. When the edge is in the alternating tree, this edge is the edge one step closer to the root. 
    If x(e)=1, x(parent(e))=0 and vice versa, by the definition of the alternating tree.


We associate with each node the following information:
  - y: The dual variable related to a constraint due to this node
  - label: (+), (-) or 0. Related to the position of the node with respect to a tree.
  - pairing_edge: The edge this node is a pairing of, if any.
  - edge_list: a list of edges that emanate from this node.
  - If the node is a blossom, we also need:
    - subnodes_list: the list of subnodes in this blossom, in cyclic order, starting with the unpaired node, and all edges in the cycle.
    - restore_list: for each edge in edge_list, we store the subnode that this edge is was connected to before forming the blossom.
  
Note that len(edge_list) = len(restore_list), we can align the two in one list.

An alternating tree is built with an unpaired edge at the root, and then alternatingly paired and unpaired (x=0, 1) edges as descendents, leaves must always be paired. 


We also keep
  - tree_list: List of all edges that are currently in the tree.


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

