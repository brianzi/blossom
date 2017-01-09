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

In this model, pointers always refer to vertices, the concept of "edges" 
is not directly mirrored in the data model. Each pointer can take a special value NONE.
In fact, we store a list of vertices, and for each vertex we store a list of
adjacent vertices, together with the weight of the corresponding edges,
which is the input to the problem. 
No vertex adjacency list is ever modified after creation. 
This list is the input to the algorithm, but it might be extended by blossom
pseudo vertices at runtime.

For each vertex, we store a "blossom
parent", which is set to the blossom vertex the vertex is part of, NONE otherwise.  

Furthermore, for each vertex we store a pointer to its partner in the matching, and its
parent in the tree.

The matching is a matching in M prime, that is, matching[v] is only valid 
if blossomparent[v] is NONE. (nb: for clarity, I set matching to NONE if the vertex is
in a blossom, I don't think this is strictly required)

We furthermore maintain a tree structure, so 
for each vertex we store a pointer treeparent[v].
It is NONE for the root vertex or if the vertex is not in the tree.
NOTE: this pointer is not guaranteed to be in m'. If b = blossomparent[treeparent[v]] 
is not NONE, then blossom parent is the actual parent. 
NOTE: It might be actually be smart to add logic that guarantees this.

Thus, if blossomparent[v] is not NONE, v will never be in a tree.
In this case, treeparent[v] has a secondary meaning: 
It contains a pointer to its blossom sibling, so that following treeparent
inside a blossom is a circular linked list along the blossom. The direction is arbitrary.

Furthermore, we maintain a monotonous function treeindex(v) = I[v] for each 
element in the tree, i.e. I(v) is decreasing towards the root. 
We furthermore require I(v) reflects the parity of v in the tree, i.e. even for 
outer nodes, odd for inner nodes. When adding new vertices to the tree, these properties
are maintained by just assigning values of a running counter to each new added vertex.
Besides identifying nodes as even or odd,
this function should allows for easy finding of common ancestors, and thus efficient finding of blossoms.

``` 
    while w!=v: 
      if I(v) > I(w): 
        v = parent(v) 
      else: 
        w = parent(w)
      # v and w traverse the cycle, do something to them...  
```
When contracting a blossom, we set its index to the index of the stem, preserving the decreasing property. 

When breaking a blossom, maintaining the tree property is harder, as 
one vertex (the blossom) is expanded into a path.
I have several ideas: a) destroy the tree after breaking, 
rebuild from the same root.
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
vertex list. We then need to insert Delta(B) in the adjacency list,
and also add the corresponding reduced weight. 
Note that we only ever take real vertices into the adjacency list,
running up the blossomparent when required.

We construct the blossom in two steps: In round one, we
use treeindex as described above to find all nodes in the 
blossom, link their treeparents up to a cyclic list and 
set their blossom parent; we also do the dual update here which 
makes all edges in the blossom tight.

In the next round, we follow the cyclic list to find the adjacency list of the blossom.
In order to find it, we need to keep a temporary pointer for each vertex,
which contains the position of each vertex in the adjacency list of the new blossom.
This allows to only insert each vertex once even if it is adjacent to several 
values.

The weights of the newly created vertices are reduced by the dual 
of the vertex in the blossom and set to the smallest weight if several edges exist.

Events in the availability queue
--------------------------------

We furthermore keep a priority queue, from which the lowest element is
popped at the top-level loop to determine the next action to the tree.

At the moment, I maintain the queue as an unsorted linked list (pop is O(n)).
The queue contains a key avdist, a vertex v and and avfrom field.
An element is technically considered in the queue if avfrom is not NONE, 
and avdist is the corresponding key.

At any point, a vertex v can only be in the queue if it is top-level, i.e. blossomparent[v] = NONE.

Four actions are associated with each element in the following way. 

v not in tree, paired: grow tree. avfrom is a plus node in the tree
v not in tree, unpaired: collapse. avfrom is a plus node in the tree
v in tree as plus: make blossom. avfrom is (another) plus node in the tree.
v in tree as minus: break blossom. avfrom is the node in the blossom which will be paired outside the blossom after collapse.

note that in case 3, the situation is symmetric; and we will have to ambiguate this situation.
This will be done is such a way (see below, outer node scan) that v will be 
the node that is added to the tree second 


Outer node scan
---------------

The actual main work at each step is the "outer node scan".
When the tree is not collapsed, an outer nodes is added created to the tree.
As a consequence, new vertices are available, and their distance to 
the root must be calculated and new events with the corresponding distances are added to the tree.
This is done in the following way:

for a in the adjacency list of the new outer node b:
  - get reduced weight from a to b.
  - run up in blossom list from a to toplevel to aprime, reducing the weight by blossom duals that a sits in.
  - if aprime is not in tree, insert or decreasekey aprime to dist[b] + reduces weight. 
    - (will lead to grow)
  - if aprime is in tree as plus, insert or decreasekey aprime to dist[b] + (reduced weight)/2. 
    - (will lead to makeblossom)
  - if aprime is in tree as minus and the treeparent of b and a blossom, insert or decreasekey aprime to dist[b] + dual[aprime].
    - avfrom is not set to b, but to the immidiate element of the blossom found while running up the blossom tree in step 1.
    - (will lead to break blossom)
  - if aprime is in tree as minus otherwise, ignore.


 
Dual updates
------------

The tree starts with a single root element as a plus node.

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


Idea: Lazy dual updates: Currently, after the collapse of each tree, 
the dual is updated on the whole tree. Each node requires d(v,r), 
which is calculated when the node is added to the availability queue, and
d(o, n2) (see above) which is calculated at collapse time. 
We could store the d(o, n2) which where found together with 
a "tree serial number", and tag each node with the tree serial number it
was last in. Then the dual update can be performed not at collapse time,
but when it is considered next (when it is added to the 
availability queue for the next time).

