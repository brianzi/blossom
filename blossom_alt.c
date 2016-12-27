#include <stdlib.h>
#include <stdio.h>



//everything is integers, no structs, because the bitbang-level structure should be exposed.


#define MAX_VERTICES 100
#define MAX_DEGREE (20)
#define MAX_QUEUE (20)

#define NONE ((unsigned int)MAX_VERTICES-1)

//graph structure
unsigned int adjacence[MAX_VERTICES][MAX_DEGREE];
unsigned int weight[MAX_VERTICES][MAX_DEGREE];

//vertex dual variables
unsigned int dual[MAX_VERTICES];

//pairing
unsigned int matching[MAX_VERTICES];

//tree and laminar structure
unsigned int tree_parent[MAX_VERTICES];
unsigned int tree_index[MAX_VERTICES];
unsigned int blossom_parent[MAX_VERTICES];
unsigned int tree_list[MAX_VERTICES];

//used for tagging vertices when building blossoms
unsigned int delta_bit[MAX_VERTICES]; 

//availability double-linked list.
unsigned int available_next[MAX_VERTICES]; 
unsigned int available_from[MAX_VERTICES];
unsigned int available_dist[MAX_VERTICES];


//global state
unsigned int tree_root;
unsigned int tree_index_max;

unsigned int available_top;

unsigned int vertex_count;

unsigned int a, b, c; // pointer registers
unsigned int d; // dual value calculation register
unsigned int i; // register to iterate over adjacency list


void grow_tree() {
    //a is the available edge that should be paired
    //
    int d2;
    
    //add a to the tree (inner node)
    tree_index[a] = tree_index_max;
    tree_list[tree_index_max] = a;
    tree_index_max ++;
    tree_parent[a] = available_from[a];
    available_from[a] = NONE;

    //get b = matching[a]
    b = matching[a];
    while (blossom_parent[b] != NONE)
        b = blossom_parent[b];
    
    //add b to the tree (new outer node)
    tree_index[b] = tree_index_max;
    tree_list[tree_index_max] = b;
    tree_index_max ++;
    tree_parent[b] = a;


    //if new outer node is already available, 
    //reduce its availability to half 
    //(will make blossom later)
    //but we will use the not-halfed distance
    //for inserting other available nodes first.
    d2 = available_dist[a];
    if(available_from[b] != NONE) {
        available_dist[b] -=  (available_dist[b] - d2) >> 1;
    } else {
        available_dist[b] = d2;
    }

    //iterate over vertices a adjacent to b, to add to availability list
    //  if a not in available stack, 
    //  a not inner node (especially, 'to' itself) push to stack.
    for(i=0; i <= MAX_DEGREE && adjacence[b][i] != NONE; i++) {
        a = adjacence[b][i];

        //if ((tree_index[a] != NONE) && ((tree_index[a]&1) == 1)) {
        if (tree_index[a] != NONE) {
            //inner node, ignore
            continue;
        }

        //distance if other node is not in tree
        d = d2 + weight[b][i] - dual[a] - dual[b];

        if (available_from[a] == NONE) {
            //insert 
            available_next[a] = available_top;
            available_top = a; 
            //set value
            available_from[a] = b;
            available_dist[a] = d;
        } else if(available_dist[a] > d) {
            //decrease
            //set value
            available_dist[a] = d;
            available_from[a] = b;
        }
    }
}

void start_tree() {
    //start tree. The new root is in a
    
    //reset tree
    tree_index_max = 0;

    //add a to tree
    tree_list[tree_index_max] = a;
    tree_index[a] = tree_index_max++;

    //empty stack
    available_top = NONE;
    available_dist[a] = 0;
    
    //iterate over a in adjacence[m];
    for(i=0; i <= MAX_DEGREE && adjacence[a][i] != NONE; i++) {
        b = adjacence[a][i];
        d = available_dist[a] + weight[a][i] - dual[a] - dual[b];

        //is the partner in a blossom? then run up in blossom list
        while (blossom_parent[b] != NONE) {
            b = blossom_parent[b];
            d -= dual[b];
        }

        if (available_from[b] == NONE) {
                //insert into list
                available_next[b] = available_top;
                available_top = b; 

                //set value
                available_from[b] = a;
                available_dist[b] = d;
        } else if(available_dist[b] > d) {
            available_dist[b] = d;
            available_from[b] = a;
        }
    }
}

int blossom_of(int v) {
    //while blossom_parent[v] != null:
    //  v = blossom_parent[v]
    //return v
    //need to add dual calculation
}

void collapse_tree() {
    b = available_from[a];
    available_from[a] = NONE;

    d = available_dist[a];

    //rematch
    while (b != NONE) {
        matching[a] = b;
        matching[b] = a;

        a = tree_parent[b];
        if (a == NONE)
            break;
        b = tree_parent[a];
    }

    //clear stack
    a = available_top;
    while (a != NONE) {
        printf("start_tree clearing stack: %d\n", a);
        available_from[a] = NONE;
        a = available_next[a];
    }

    //dual update and clear tree

    //update and clear root
    a = 0;
    b = tree_list[a];
    dual[b] += d;
    tree_index[b] = NONE;
    tree_parent[b] = NONE;

    //update and clear rest of tree
    for (a=1; a<tree_index_max; a++) {

        //inner node
        b = tree_list[a];
        if (blossom_parent[b] != NONE) continue;
        dual[b] -= d-available_dist[b];

        //corresponding outer node. use av_dist from inner node, though.
        a = a+1;
        c = tree_list[a];
        dual[c] += d-available_dist[b]; 

        tree_parent[b] = NONE;
        tree_index[b] = NONE;
        tree_parent[c] = NONE;
        tree_index[c] = NONE;
    }
    tree_index_max = 0;
}

void make_blossom() {
    int c2, i2;

    //build a blossom from the plus vertex a to the plus
    
    b = available_from[a]; //the other outer node
    // this is how much we grow the duals on the tree
    d = available_dist[a];

    printf("growing blossom to dist: %d\n", d);

    
    
    c = b;


    //go through the blossom the first time:
    //collect adjacent nodes (not pseudo!)
    //update duals to make blossom tight
    //reverse tree structure on one branch
    while(1) {
        if(tree_index[a] >= tree_index[b]) {
            printf("make blossom round1: doing %d as a\n", a);
            for(i=0; i<= MAX_DEGREE && adjacence[a][i] != NONE; i++) {
                delta_bit[adjacence[a][i]] ^= 1;
            }

            if (tree_index[a] & 1) {
                dual[a] -= d - available_dist[a];
            }
            else {
                dual[a] += d - available_dist[tree_parent[a]];
            }
            blossom_parent[a] = vertex_count;

            if (tree_index[a] == tree_index[b]) {
                break;
            }

            c2 = tree_parent[a];
            tree_parent[a] = c;
            c = a;
            a = c2;

        }
        if(tree_index[a] < tree_index[b]) {
            printf("make blossom round1: doing %d as b\n", a);
            for(i=0; i<= MAX_DEGREE && adjacence[b][i] != NONE; i++) {
                delta_bit[adjacence[b][i]] ^= 1;
            }
            if (tree_index[b] & 1) {
                dual[b] -= d - available_dist[b];
            }
            else {
                dual[b] += d - available_dist[tree_parent[b]];
            }
            b = tree_parent[b];
            blossom_parent[b] = vertex_count;
        }
    }
    tree_parent[a] = c;
    //replace a in the tree_list by the blossom.
    tree_list[tree_index[a]] = vertex_count;
    tree_index[vertex_count] = tree_index[a];

    //match blossom and insert into tree
    matching[vertex_count] = matching[b];
    matching[matching[b]] = vertex_count;
    tree_parent[vertex_count] = matching[b];


    //b now is left at stem node, a iterates around a second time
    //to build adjacency list
    i2 = 0;
    do {
        for(i=0; i<= MAX_DEGREE && adjacence[a][i] != NONE; i++) {
            if (delta_bit[adjacence[a][i]] == 1) {
                delta_bit[adjacence[a][i]] = 0 ;
                adjacence[vertex_count][i2] = adjacence[a][i];
                weight[vertex_count][i2] = weight[a][i] - dual[a];
                i2 += 1;
            }
        }
        tree_index[a] = NONE;

        //TODO: this is just for clarity can be done when breaking
        //the blossom or whatever

        a = tree_parent[a];

    } while ( a != b);


    a = vertex_count;

    //blossom is plus, so we insert new outer nodes into av stack
    //TODO: merge with previous loop
    for(i=0; i <= MAX_DEGREE && adjacence[a][i] != NONE; i++) {
        b = adjacence[a][i];
        d = available_dist[a] + weight[a][i] - dual[a] - dual[b];

        //is the partner in a blossom? then run up in blossom stack
        while (blossom_parent[b] != NONE) {
            b = blossom_parent[b];
            d -= dual[b];
        }

        if (available_from[b] == NONE) {
                //insert into list
                available_next[b] = available_top;
                available_top = b; 

                //set value
                available_from[b] = a;
                available_dist[b] = d;
        } else if(available_dist[b] >= d) {
            available_dist[b] = d;
            available_from[b] = a;
        }
    }

    vertex_count += 1;
}

void break_blossom(int bl) {
    //TODO: trace blossom cycle, rematching, setting tree indices.
}

void init() {
    //set everything to NONE
    int i, j;
    for(i=0; i<MAX_VERTICES; i++) {
        for(j=0; j<MAX_DEGREE; j++) {
            adjacence[i][j] = NONE;
            weight[i][j] = NONE;
        }
        dual[i] = 0;
        matching[i] = NONE;
        tree_parent[i] = NONE;
        tree_index[i] = NONE;
        blossom_parent[i] = NONE;
        available_next[i] = NONE;
        available_from[i] = NONE;
        }

    tree_index_max = 0;
}

void read_file(char* filename) {
    //file format is node1 node2 weight
    FILE* file;
    file = fopen(filename, "r");

    int node1, node2, w;
    int i,j;

    vertex_count = 0;
    while(fscanf(file, "%d %d %d\n", &node1, &node2, &w) > 0) {
        printf("%d -> %d, weight %d\n", node1, node2, w);

        if(node1 > MAX_VERTICES || node2 > MAX_VERTICES) {
            printf("node index too large\n"); exit(1);
        }

        for(i=0; i < MAX_DEGREE && adjacence[node1][i] != NONE; i++);
        if(i >= MAX_DEGREE) {
            printf("node %d has too many neighbors\n", node1); exit(1);
        }
        adjacence[node1][i] = node2;
        weight[node1][i] = w;

        for(i=0; i < MAX_DEGREE && adjacence[node2][i] != NONE; i++);
        if(i >= MAX_DEGREE) {
            printf("node %d has too many neighbors\n", node2); exit(1);
        }
        adjacence[node2][i] = node1;
        weight[node2][i] = w;
    }
    for(i=0; i<MAX_VERTICES && adjacence[i][0] != NONE; i++);
    vertex_count = i;

}

void dump_graph() {
    int i, j;

    printf("vertex count: %d\n", vertex_count);

    for(i=0; i<MAX_VERTICES && adjacence[i][0] != NONE; i++) {
        printf("%d:\t", i);
        for(j=0; j<MAX_DEGREE && adjacence[i][j] != NONE; j++) {
            printf("%d, %d;\t", adjacence[i][j], weight[i][j]);
        }
        printf("\n");
    }
}

void dump_stack() {
    int max_recursion=20;
    int i;
    printf("available: ");
    for(i=available_top; i != NONE; i = available_next[i]) {
        if(max_recursion-- == 0) {
            printf("max recursion exceeded\n");
            exit(1);
        }
        printf(" %d (from %d)", i, available_from[i]);
    }
    printf("\n");
}


void dump_tree() {
    int i;

    printf("tree_index_max: %d\n", tree_index_max);

    printf("vert\tmatch\tdual\ttreepar\ttreeind\tbloss\tav_dist\tav_nxt\tav_fr\n");
    for(i=0; i<MAX_VERTICES && adjacence[i][0] != NONE; i++) {
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i,
            matching[i], dual[i],
            tree_parent[i], tree_index[i], 
            blossom_parent[i],
            available_dist[i], available_next[i], 
            available_from[i]);
    }

    printf("tree list: ");
    for(i=0; i < tree_index_max; i++) {
        printf("%d ", tree_list[i]);
    }
    printf("\n");
}


void dump_adjacency_matrix() {
    int i, j, i2;


    // generate the dense adjacency matrix
    int mat[MAX_VERTICES][MAX_VERTICES];

    for(i=0; i<MAX_VERTICES; i++)
        for(j=0; j<MAX_VERTICES; j++)
            mat[i][j] = 0xdead;
    

    for(i=0; i<MAX_VERTICES && adjacence[i][0] != NONE; i++) {
        for(j=0; j<MAX_DEGREE && adjacence[i][j] != NONE; j++) {
            i2 = adjacence[i][j];
            d = weight[i][j] - dual[i] - dual[i2];
            if (d < mat[i][i2]) {
                mat[i][i2] = d;
            }
        }
    }

    printf("    ");
    for(i=0; i<vertex_count; i++) {
        printf("%4d", i);
    }
    printf("\n");

    for(i=0; i<vertex_count; i++) {
        printf("%4d", i);
        for(j=0; j<vertex_count; j++) {

            if(mat[i][j] != 0xdead) {
                if(matching[i] == j) {
                    printf("%3d*", mat[i][j]);
                }
                else { 
                    printf("%4d", mat[i][j]);
                }
            }
            else {
                printf("   -");
            }
        }
        printf("\n");
    }
}


int top_loop() {
    int i, j;
    dump_graph();
    dump_adjacency_matrix();

    for(i=0; i <= MAX_VERTICES && adjacence[i][0] != NONE; i++) {
        //for each unpaired vertex
        if(matching[i] == NONE) {
            printf("\nstarting tree from %d\n", i);
            a = i; start_tree();
            dump_tree();
            dump_stack();
            
            while(available_top != NONE) {
                //find mininum in stack
                dump_adjacency_matrix();
                b = j = NONE;
                a = available_top;
                d = available_dist[a];
                while (a != NONE) {
                    if(available_dist[a] < d) {
                        d = available_dist[a];
                        printf(".");
                        b = j;
                    } 
                    j = a;
                    a = available_next[a];
                }
                //b is position before minimum, or NONE if first element.

                if(b != NONE) {
                    //unlink from list
                    a = available_next[b];
                    available_next[b] = available_next[a];
                } else {
                    a = available_top;
                    available_top = available_next[a];
                }
                printf("minimum in stack is %d at pos %d, after %d\n", d, a, b);

                if (tree_index[a] != NONE) {
                    printf("making blossom from %d\n", a);
                    make_blossom();
                    dump_tree();
                    dump_stack();
                    dump_graph();
                    dump_adjacency_matrix();
                }else if (matching[a] != NONE) {
                    //if matched, tree can grow
                    printf("growing to %d\n", a);
                    grow_tree();

                } else {
                    //if unmatched, we can collapse, done
                    printf("collapsing to %d\n", a);
                    collapse_tree();
                    dump_tree();
                    break;
                }
                dump_tree();
                dump_stack();
            }
        }
    }

    dump_tree();
    dump_adjacency_matrix();
}


int main(int argc, char** argv) {
    init();
    if(argc > 1) {
        read_file(argv[1]);
    }

    top_loop();
}
