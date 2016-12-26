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

We store a list of vertices, and for each vertex we store a list of
neighboring vertices, together with the weight of the corresponding edges,
which is not modified after creation. This is the input to the algorithm,
so there is no way around this.

Blossoms are also created as vertices. For each vertex, we store a "blossom
parent", which is set to the blossom vertex the vertex is part of.  For
breaking a blossom, we thus also want to store a list of all the blossom
children; we only need to reset their blossom pointer to NULL.  Essentially,
we are thus storing the laminar covering as a doubly linked tree.

Instead, it might be possible to not store the children, just mark blossom
vertices as destroyed somehow, and unlink the children when they ever try to
access their parent and note its destruction?

Furthermore, for each vertex we store its partner in the matching, and its
parent in the tree.

The matching is a matching in M prime, that is, matching[v] must 
have the same blossom parent as v.

If a vertex has a blossom parent, it will never be part of the tree. At the
moment it becomes part of a blossom, its tree pointer nicely traces the
blossom towards its stem. We thus could use the tree pointer for restoring
blossoms. It would be the same idea as above: When making a blossom, revert
one branch in direction, so that the tree pointer forms a circuit tracing
the blossom. When restoring a blossom, start with the newly paired vertex
and re-pair by going around the cycle once. 


It might also be a good idea to maintain a monotonous function I(v) for each 
element in the tree, i.e. I(v) is decreasing decreasing towards the root. 
We furthermore require I(v) has the same parity as v, i.e. even for 
outer nodes, odd for inner nodes. When adding to the tree, just add 
a running counter to the two new added nodes.
Besides identifying nodes as even or odd,
this function should allow for easy finding of common ancestors:

``` 
    while w!=v: 
      if I(v) > I(w): 
        v = parent(v) 
      else: 
        w = parent(w)
      # v and w traverse the cycle, do something to them...  
```
When creating a blossom, set its index to the index of the stem, preserving the decreasing property. 

When breaking a blossom, maintaining the tree property is harder, as 
one vertex (the blossom) is expanded into a path.
I have several ideas: a) destroy the tree after breaking, 
rebuild from the same node.
This will build the same tree, but with new I(v).
b) When adding a blossom to the tree, increase the counter by the 
number of vertices in the blossom, instead of 1. Requires keeping 
track of blossom sizes.
c) "decrease towards root". Expand the blossom. 
Decrease I(v) on the whole path from the blossom to the root, 
until there is "room" in I(v) space for the new path.
Requires possibility of negative I(v).
        
When considering new outer edges, it needs to be determined whether a vertex
is in the tree already. If that is done by considering whether the tree
pointer is set, one needs to remove all tree pointers when collapsing the
tree, which takes time.  Instead, one could keep I(v) running between trees,
and check tree membership by comparing with `tree_index[root]`. Then we only
run into trouble when this index overruns...
However, with weights, collapsing the tree requires dual updates on the 
whole tree anyway, so that looping over the tree is required in any case,
and we maintain a list, collecting all elements in the tree.


Making blossoms
---------------

When making a new blossom B, we create a new (pseudo-)vertex in the
vertex list. We then need to insert Delta(B) in the adjacency list.
Finding delta(B) is a bit difficult. The easiest way is a marker on
adjacency edges: We run through the blossom once, toggling a one-bit marker
on all vertices adjacent to each vertex in the blossom.
After doing this to all edges, the marked edges are adjacent to the
new blossom.
Note that we only ever take real vertices into the adjacency list.

The blossom parent of each vertex in the blossom is set to the
new pseudo-vertex.

When constructing the blossom, we also need to create a structure that
enables us to restore the blossom later, and add it to the tree. The best
structure would be a double-linked circular list, and a pointer 
`tree_up` and `tree_down` from the pseudo-vertex to the (pseudo-)vertex in the blossom that is 
currently the source for the location in the tree to the outside. 

Then breaking the blossom simply starts from the treedown partner,
building tree and rematching while walking up in both directions
along the blossom, until the treeup partner is found.

If we only had a singly linked list, this should still be possible in
one round.

Actually, treeup can be found easily, by taking treeparent of
the upper vertex and following it to the second to last tier in the
laminar tree.

Treedown can be found in the same way at the instance of 
addition to the tree. At that moment, the break-blossom event has to be 
added to the queue anyway, and the treedown pointer could be stored
in the availablefrom field of the queue. Being in a blossom also 
identifies this event in the queue, the other events.

Events in the availability queue
--------------------------------

The queue contains a distance key, a vertex v and and avfrom field.
An element is technically considered in the queue if avfrom and avdist are
set.

Four actions are associated with each element in the following way. 

avfrom not in tree, paired: grow tree
avfrom not in tree, unpaired: collapse
avfrom in tree as plus: make blossom
avfrom in blossom: break blossom 

for the last case, not that the blossom v in this case is an inner node
in the tree, and thus cannot be available for any other case.

note that a vertex available for growing might be available again
immediately for making a blossom. However, the vertex it is available from
will vice versa also become available from v itself. 
We ambiguate this by saying that for making a blossom, 
v is the vertex with lower I(v) and avfrom is the vertex with higher I(v).

 
Outer edges
---------------

Vertices that are reachable from an outer node could be "added" to the tree,
if they are a) not in the tree, or b) an outer edge.
In case a) the tree is grown, in case b) a blossom is formed.


For that, they are held in a linked data structure.
If no weight calculation is performed, a stack (i.e. simple linked list) is
sufficient. If weight calculation is performed, the data structure must be
a heap that supports `decrease_key`. A Fibonacci heap is known to be best 
asymptotically; a simpler structure might be preferrable for small sizes. 

For now, I am using an unsorted linked list, with O(1) insert and decreasekey, but O(n) pop.


Dual updates
------------

Before collapsing the tree, the root is unpaired, and its dual variable is 0.

Then, a tree is build, and after collapsing, the dual variables on the tree
are updated so that all edges in the tree are tight.

When adding a node to the tree, we mark it with the dual-reduced distance
d(v,r) to the root. When finally an outer node o hits a cond unpaired node
n2, the dual of each element v in the tree is updated by +- (d(v, r) +
d(o,n2)).

The distance to the root needs already be calculated when adding to the
availability heap. 

Distance calculation: When the tree is grown along an edge e(o, i), (o in the
tree) the distance from the new inner and outer node to node is given by 

d(i, r) = d(o, r) + w(e) - dual(o) - dual(i)

The distance of the new outer node o2 = m(i) is the same.

One might shortcut this because previously collapsed trees 
tend to leave big chunks of zero-distance nodes, but this is the general idea of having several trees.



