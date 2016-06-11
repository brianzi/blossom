The plan
--------



We associate with each edge the following information:
  - slack: The slackness that we use to find the next instruction
  - x: Whether this edge is in the matching (=primal variable), is 0 or 1
  - node plus and node minus. The two nodes connected by this edge. Initially, two vertices.
  - parent edge. When the edge is in the alternating tree, this edge is the edge one step closer to the root. 
    If x(e)=1, x(parent(e))=0 and vice versa, by the defnition of the alternating tree.


We associate with each node the following information:
  - y: The dual variable related to a constraint due to this node
  - label: (+), (-) or 0. Related to the position of the node with respect to a tree.
  - pairing_edge: The edge this node is a pairing of, if any.
  - edge_list: a list of edges that emanate from this node.
  - If the node is a blossom, we also need:
    - subnodes_list: the list of subnodes in this blossom, in cyclic order, starting with the unpaired node
    - restore_list: for each edge in edge_list, we store the subnode that this edge is was connected to before forming the blossom.
  
Note that len(edge_list) = len(restore_list), we can align the two in one list.

An alternating tree is built with an unpaired edge at the root, and then alternatingly paired and unpaired (x=0, 1) edges as descendents, leaves must always be paired. 


We also keep
  - tree_list: List of all edges that are currently in the tree.


Note that the tree is considered a tree of edges, not a tree of nodes! (better structure in my opinion)

Blossom algorithm requires the following operations:

- grow: for all (+)/(-) vertices in the graph, increase/decrease y by dw. 
- add: add a paired and an unpaired edge to the tree. This is done providing an unpaired edge not in the tree, and a paired edge in the tree defining the position where to add. 
The unpaired edge must be incident on the (+) vertex of the paired edge.
From the unpaired edge, the other node is found, and from the node, the other paired edge is found.
- make blossom: create a new node ("blossom"). As input, give two paired edges in the tree whose (+) nodes should be joined. 
Find the cycle formed by the tree and this additional edge (backtrack the first node to the root, leaving breadcrumbs, then backtrack second until
trace found) , build a node from these. Build the edge list for this node and change the definition of all edges imanent to this node to join to the new node instead.
Save the original nodes for each edge so that reverting is possible. By changing the edges, the tree should have changed as well.
- expand blossom. Essentially the inverse of make_blossom. Use the restore list to restore the previous node structure. While doing so, the node must be paired, and 
that pairing is used to settle the pairing inside the blossom (which might be different from the incomplete pairing left behind when the blossom was formed)
- augment: Given an unpaired egde emanating from the tree, add it to the tree by letting x <- 1-x along the path from (including) the unpaired edge to the root of the tree. 
The tree is then "done" and a new tree is formed.

