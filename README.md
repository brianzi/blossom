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


Alternative Plan 
================

After some consideration, it might be worthwile to think about a more minimal
data storage model; computations are often cheap.

We store a list of vertices, and for each vertex we store a list of neighboring
vertices, together with the weight of the corresponding edges, which is not
modified after creation.

Blossoms are also created as vertices. For each vertex, we store a "blossom
parent", which is set to the blossom vertex the vertex is part of.  For
breaking a blossom, we thus also want to store a list of all the blossom
children; we only need to reset their blossom pointer to NULL.  Essentially, we
are thus storing the laminar covering as a doubly linked tree.

Instead, it might be possible to not store the children, just mark blossom
vertices as destroyed somehow, and unlink the children when they ever try to
access their parent and note its destruction?

Furthermore, for each vertex we store its partner in the matching, and its
parent in the tree.

The matching is a matching in M prime, that is, matching[v] must 
have the same blossom parent as v.

If a vertex has a blossom parent, it will never be part of the tree. At the
moment it becomes part of a blossom, its tree pointer nicely traces the blossom
towards its stem. We thus could use the tree pointer for restoring blossoms. It
would be the same idea as above: When making a blossom, revert one branch in
direction, so that the tree pointer forms a circuit tracing the blossom. When
restoring a blossom, start with the newly paired vertex and re-pair by going
around the cycle once. 

It might also be a good idea to store a running index I(v) for each 
element when added to the tree. This way, I(v) is decreasing towards the root.
This should allow for easy finding of common ancestors:

``` 
    while w!=v: 
      if I(v) > I(w): 
        v = parent(v) 
      else: 
        w = parent(w)
      # v and w traverse the cycle, do something to them...  
```

When creating a blossom, set its index to the index of the stem, preserving the
decreasing property.      
        
Also, since vertices are always added in plus-minus-pairs, the lowest bit
of I(v) determines whether a node is inner or outer.

When considering new outer edges, it needs to be determined whether a vertex is
in the tree already. If that is done by considering whether the tree pointer is
set, one needs to remove all tree pointers when collapsing the tree, which
takes time.  Instead, one could keep I(v) running between trees, and check tree
membership by comparing with `tree_index[root]`. Then we only run into trouble
when this index overruns...


Outer edge list
---------------
The algorithm requires to uphold a list of outer edges, i.e. edges connecting
a plus node in the tree with another node.
(The other node is either not in the tree or a plus node itself, call it 
"available")
Every node might be available in several ways, but once 
it is processed once, its availability will change from
not in tree -> in tree -> in blossom (not available any more).

