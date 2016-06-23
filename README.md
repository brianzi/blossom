The plan
--------

We need a graph whose shape can be modified dynamically. 
We store a list of edges and a list of nodes.

We associate with each edge the following information:
  - slack: The slackness that we use to find the next instruction
  - node plus and node minus. The two nodes connected by this edge. 
Initially, two vertices. These change when blossoms are shrunk or expanded.
When the edge touches the tree, these vertices should be correct (if possible).
  - parent edge. When the edge is in the alternating tree, this edge is the edge one step closer to the root. 
    If x(e)=1, x(parent(e))=0 and vice versa, by the defnition of the alternating tree.
  - cycle edge. When the tree is in a blossom, should point along the blossom cycle. 
  - a tag bitfield, denoting the position of the edge with respect to the tree. see below.

We associate with each node the following information:
  - y: The dual variable related to a constraint due to this node
  - pair edge: The edge this node is a pairing of, if any. NULL if the edge is not paired yet. `<edge*> -1` if the node is a 'lonely node', i.e. the only unpaired 
node in a blossom.
  - edge list: a list of edges that emanate from this node.
  - If the node is a blossom, we also need:
    - restore list: for each edge in the edge list, we store the subnode that this edge is was connected to before forming the blossom. 
  
Note that len(edge list) = len(restore list), we could align the two in one list. (we don't do right now)

An alternating tree is built with an unpaired edge at the root, and then alternatingly paired and unpaired (x=0, 1) edges as descendents, leaves must always be paired. 

We also keep a list of all edges that share at least one node with the tree, called 
`seen_list`.

During the growth of the tree, this list is updated. At the same time,
we denote the status of the edge in a `tag` field.

There are five bits in the tag field:
  - tag[0]: (W) working tag (only used temporarily while forming a blossom)
  - tag[1]: (B) the edge is in a blossom which was created in the current tree
  - tag[2]: (O) the plus edge of the blossom is in the tree (is outer edge)
  - tag[3]: (I) the minus edge of the blossom is in the tree (is inner edge)
  - tag[4]: (D) the other edge is in the tree as well, with same sign (exactly one of O,I must be set) (double edge)

If the edge is in the tree, OI are set. If the edge is in a blossom, either OIB or ODB or IDB are set.
OI can also be set if the edge is not in the tree, but touching a plus and a minus node.
Such edges can be treated like in the tree for all purposes, so this is fine.

Note that the tree is considered a tree of edges, not a tree of nodes! 

The root of the tree is an edge with the special name `root`. It can be distinguished from
any other edge by having itself as parent.

Invariants: 
  - graph structure: each node refers to the nodes that in turn refer to the right edges. (Should also be connected!)
  - matching: the matching of should always be a (not complete) matching vector. A node indicating an edge as pairing should be indicated by both nodes.
  - tree: should be connected, should be an alternating tree.
  - tags should be always consistent with the position with respect to the tree

Blossom algorithm requires the following operations on the graph:

- add: add a paired and an unpaired edge to the tree. 

This is done by specifying an O edge, whose nodes are both paired (one of them in the tree).
This edge is added to the tree and also the second node pair is added to the tree.

All new edges must be examined and added to the seen list.

- make blossom: create a new node ("blossom"). 

The input is a DO edge

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

Also register all these nodes as outer nodes, 


- expand blossom. 

Essentially the inverse of `make_blossom`. Use the restore list to restore the previous node structure. While doing so, the node must be paired, and 
that pairing is used to settle the pairing inside the blossom (which might be different from the incomplete pairing left behind when the blossom was formed)

The input is the blossom node. Follow the restore list and revert all edges to their previous state.  

Then follow 

- augment: Given an unpaired egde emanating from the tree, add it to the tree by letting x <- 1-x along the path from (including) the unpaired edge to the root of the tree. 
The tree is then "done" and a new tree is formed.

