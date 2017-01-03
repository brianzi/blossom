#include <stdlib.h>
#include <stdio.h>


#define MAX_VERTICES (100)
#define MAX_DEGREE (20)
#define MAX_TREES (MAX_VERTICES)

#define NONE ((unsigned int)MAX_VERTICES-1)

typedef unsigned int N; //node pointer. max=MAX_VERTICES
typedef int W; //weight
typedef unsigned int TREE_IDX; //index of a tree
typedef unsigned int TREE_M; //monotonous tag on tree. max=MAX_VERTICES
typedef unsigned int ADJ_IDX; //index to adjacency list. max=MAX_DEGREE

//graph structure
N adjacence[MAX_VERTICES][MAX_DEGREE];
W weight[MAX_VERTICES][MAX_DEGREE];

//dual variable
W dual[MAX_VERTICES];
W dist_in_tree[MAX_VERTICES];
W collapse_dist[MAX_TREES];

//matching partner
N matching[MAX_VERTICES];

//tree and laminar structure
N tree_parent[MAX_VERTICES];
TREE_IDX tree_index[MAX_VERTICES];
TREE_M tree_m[MAX_VERTICES];
N blossom_parent[MAX_VERTICES];


//used for tagging vertices when building blossoms
ADJ_IDX delta_idx[MAX_VERTICES]; 

//availability double-linked list.
N available_next[MAX_VERTICES]; 
N available_from[MAX_VERTICES];
W available_dist[MAX_VERTICES];
TREE_IDX available_index[MAX_VERTICES];
//unsigned int available_type[MAX_VERTICES];
// 0 grow tree, 1 collapse, 2 make blossom, 3 break blossom

//global state
TREE_IDX current_tree_index;
TREE_M current_tree_m;
N max_unpaired;

//stack counter
N available_top;

unsigned int vertex_count;

N a, b, c; // pointer registers
W d; // dual value calculation register
ADJ_IDX i; // register to iterate over adjacency list


void init() {
    //set everything to NONE
    int i, j;
    for(i=0; i<MAX_VERTICES; i++) {
        for(j=0; j<MAX_DEGREE; j++) {
            adjacence[i][j] = NONE;
            weight[i][j] = NONE;
        }
        dist_in_tree[i] = 0;
        
        tree_index[i] = NONE;
        tree_m[i] = 0;
        matching[i] = NONE;
        tree_parent[i] = NONE;
        tree_index[i] = NONE;
        blossom_parent[i] = NONE;
        available_next[i] = NONE;
        available_from[i] = NONE;
        available_index[i] = NONE;
        delta_idx[i] = NONE;
        }

    current_tree_index = 0;
}


void clear_tree_and_stack() {
    available_top = NONE;
    current_tree_m = 1;
}

void insert_or_decreasekey() {
    //insert_or_decreasekey a node
    //a: vertex
    //b: new av_from
    //d: new av_dist
    //a, b, d are not altered
    if (available_index[a] != current_tree_index) {
        //insert
        available_index[a] = current_tree_index;
        available_dist[a] = d;
        available_from[a] = b;
        
        available_next[a] = available_top;
        available_top = a;
    }
    else if (available_dist[a] >= d) {
        //decrease key
        available_dist[a] = d;
        available_from[a] = b;
    }
}

void deletemin() {
    //a <- lowest in the queue
    //b <- available_from[a];
    //d <- available_dist[a];
    d = available_dist[available_top];
    a = c = NONE;
    b = available_top;
    while (b != NONE) {
        if (available_dist[b] < d) {
            d = available_dist[b];
            a = c;
        }
        c = b;
        b = available_next[b];
    }

    //a is the node in the list before the minimum 
    //element, NONE if minimum element is the first one.
    
    if(a != NONE) {
        b = available_next[a];
        available_next[a] = available_next[b];
    } else {
        b = available_top;
        available_top = available_next[b];
    }

    a = b;
    b = available_from[a];
    d = available_dist[a];
}

void scan_outer_node() {
    //b: outer node
    //a <- x
    //c <- x
    //d <- x
    for(i = 0; i <= MAX_VERTICES && adjacence[b][i] != NONE; i++) {
        a = adjacence[b][i];

        d = weight[b][i] - dual[b];

        //run up in blossom
        c = NONE;
        while (blossom_parent[a] != NONE) {
            c = a;
            a = blossom_parent[a];
            d -= dual[a];
        }

        if(tree_index[a] != current_tree_index) {
            //a not in tree
            d -= dual[a];
            if (tree_index[a] != NONE) {
                //a was in tree before
                if ((tree_m[a] & 1) == 1) {
                    d += collapse_dist[tree_index[a]] - dist_in_tree[a];
                }
                else {
                    d -= collapse_dist[tree_index[a]] - dist_in_tree[a];
                }
            }
        } else if(a == tree_parent[b] && c != NONE) {
            //break blossom 
            d += dual[a];
            a = c;
        } else if((tree_index[a] & 1) == 0) {
            //TODO make blossom
            d -= dual[a];
            d >>= 1;
        } else {
            //ignore
            continue;
        }

        d += dist_in_tree[b];
        insert_or_decreasekey();
    }
}

void grow_tree() {
    //a: available node
    //b: av_from[a]
    //d: av_dist[a]
    //
    //a <- new_outer_node
    //b <- a
    
    //insert into tree
    tree_parent[a] = b;
    b = matching[a];
    tree_parent[b] = a;

    //deferred dual update
    if (tree_index[a] != NONE) {
        d = collapse_dist[tree_index[a]];
        if ((tree_m[a]&1) == 1) {
            dual[a] -= d - dist_in_tree[a];
            dual[b] += d - dist_in_tree[b];
        } else {
            dual[a] += d - dist_in_tree[a];
            dual[b] -= d - dist_in_tree[b];
        }
    }
    
    tree_index[a] = current_tree_index;
    tree_index[b] = current_tree_index;
    tree_m[a] = current_tree_m ++;
    tree_m[b] = current_tree_m ++;
    dist_in_tree[a] = available_dist[a];
    dist_in_tree[b] = available_dist[a];

    a = b; 
}

void collapse_tree() {
    //a: available node
    //b: av_from[a]
    //d: av_dist[a]
    tree_index[a] = current_tree_index;
    tree_m[a] = current_tree_m++;
    dist_in_tree[a] = d;
    
    while (a != NONE) {
        matching[a] = b;
        matching[b] = a;
        a = tree_parent[b];
        b = tree_parent[a];
    }


    collapse_dist[current_tree_index] = d;
    current_tree_index += 1;
    current_tree_m = 0;
}



void print_state() {
    int i;
    int dualstar;

    printf("\tvert\tmatch\tdual\tdual*\ttreepar\ttreeidx\ttree_m\tdist_it\tbl_par\tav_dist\tav_nxt\tav_fr\tav_idx\tdelta\n");
    for(i=0; i<MAX_VERTICES && adjacence[i][0] != NONE; i++) {

        if (tree_index[i] == NONE) {
            dualstar = 0;
        } else {
            dualstar = collapse_dist[tree_index[i]] - dist_in_tree[i];
            dualstar *= (tree_m[i]&1 == 1) ? (-1) : 1;
            dualstar += dual[i];
        }

        printf("\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i,
            matching[i], dual[i], dualstar,
            tree_parent[i], tree_index[i], tree_m[i], dist_in_tree[i],
            blossom_parent[i],
            available_dist[i], available_next[i], 
            available_from[i], available_index[i],
            delta_idx[i]);
    }

    if (current_tree_index > 0) {
        printf("tree collapse_dist: ");
        for(i = 0; i<current_tree_index; i++) {
            printf("%d: %d, ", i, collapse_dist[i]);
        }
        printf("\n");
    }

    printf("global: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d);
    printf("  current_tree_m=%d\n", current_tree_m);
    printf("  current_tree_index=%d\n", current_tree_index);
    printf("  available_top=%d\n", available_top);
    printf("  max_unpaired=%d\n", max_unpaired);

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

void print_stack() {
    int max_recursion=20;
    int i;
    printf("available queue: ");
    for(i=available_top; i != NONE; i = available_next[i]) {
        if(max_recursion-- == 0) {
            printf("max recursion exceeded\n");
            exit(1);
        }
        printf(" %d (from %d, dist %d)", i, available_from[i], available_dist[i]);
    }
    printf("\n");
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

void dump_adjacency_matrix() {
    int i, j, i2;
    int d;


    // generate the dense adjacency matrix
    int mat[MAX_VERTICES][MAX_VERTICES];

    for(i=0; i<MAX_VERTICES; i++)
        for(j=0; j<MAX_VERTICES; j++)
            mat[i][j] = 0xdead;
    

    for(i=0; i<MAX_VERTICES && adjacence[i][0] != NONE; i++) {
        for(j=0; j<MAX_DEGREE && adjacence[i][j] != NONE; j++) {
            i2 = adjacence[i][j];
            d = weight[i][j] - dual[i] - dual[i2];

            if(tree_index[i] != NONE) {
                if((tree_m[i] & 1) == 1)
                    d += collapse_dist[tree_index[i]] - dist_in_tree[i];
                else
                    d -= collapse_dist[tree_index[i]] - dist_in_tree[i];
            }

            if(tree_index[i2] != NONE) {
                if((tree_m[i2] & 1) == 1)
                    d += collapse_dist[tree_index[i2]] - dist_in_tree[i2];
                else
                    d -= collapse_dist[tree_index[i2]] - dist_in_tree[i2];
            }


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

void top_loop() {
    clear_tree_and_stack();
    max_unpaired = 0;

    while(adjacence[max_unpaired][0] != NONE) {


        printf(" **** starting tree at %d\n", max_unpaired);

        b = max_unpaired;
        tree_index[b] = current_tree_index;
        scan_outer_node();

        print_state();
        print_stack();

        printf("\n");
        
        while(available_top != NONE)  {
            deletemin();


            if(matching[a] == NONE) {
                printf(" **** collapsing to %d\n", a);
                collapse_tree();
                clear_tree_and_stack();
                while(matching[++max_unpaired] != NONE);
                break;
            }

            printf(" **** growing tree to to %d\n", a);
            grow_tree();
            scan_outer_node();

            print_state();
            print_stack();

        }
    }

    print_state();

}

int main(int argc, char** argv) {
    init();
    if(argc > 1) {
        read_file(argv[1]);
    }
    dump_graph();

    top_loop();

    dump_adjacency_matrix();

    /*printf("\n");*/

    /*clear_tree_and_stack();*/
    /*max_unpaired = 0;*/
    /*b = max_unpaired;*/
    /*tree_index[b] = current_tree_index;*/
    /*scan_outer_node();*/

    /*print_state();*/
    /*print_stack();*/

    /*deletemin();*/
    /*collapse_tree();*/
    /*clear_tree_and_stack();*/
    /*while(matching[++max_unpaired] != NONE);*/

    /*print_state();*/

    /*b = max_unpaired;*/
    /*tree_index[b] = current_tree_index;*/
    /*scan_outer_node();*/

    /*print_state();*/
    /*print_stack();*/
    

    /*deletemin();*/
    /*collapse_tree();*/
    /*clear_tree_and_stack();*/
    /*while(matching[++max_unpaired] != NONE);*/

    /*print_state();*/

    /*b = max_unpaired;*/
    /*tree_index[b] = current_tree_index;*/
    /*scan_outer_node();*/

    /*print_state();*/
    /*print_stack();*/

    /*deletemin();*/
    /*grow_tree();*/
    /*scan_outer_node();*/

    /*print_state();*/
    /*print_stack();*/

    /*deletemin();*/
    /*collapse_tree();*/
    /*clear_tree_and_stack();*/
    /*while(matching[++max_unpaired] != NONE);*/

    /*print_state();*/





}

