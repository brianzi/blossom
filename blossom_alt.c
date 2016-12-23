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

//availability double-linked list.
unsigned int available_prev[MAX_VERTICES];
unsigned int available_next[MAX_VERTICES]; 
unsigned int available_from[MAX_VERTICES];
unsigned int available_dist[MAX_VERTICES];


//global state
unsigned int tree_root;
unsigned int tree_index_max;

unsigned int available_top;

unsigned int vertex_count;

unsigned int a, b; // pointer registers
unsigned int d; // dual value calculation register
unsigned int i; // register to iterate over adjacency list


void grow_tree() {
    //a is the available edge that should be paired
    
    //add a to the tree (inner node)
    tree_index[a] = tree_index_max;
    tree_list[tree_index_max] = a;
    tree_index_max ++;
    tree_parent[a] = available_from[a];

    //get b = matching[a]
    b = matching[a];
    
    //add b to the tree (new outer node)
    tree_index[b] = tree_index_max;
    tree_list[tree_index_max] = b;
    tree_index_max ++;
    tree_parent[b] = a;

    //iterate over vertices a adjacent to b, to add to availability list
    //  if a not in available stack, 
    //  a not inner node (especially, 'to' itself) push to stack.
    for(i=0; i <= MAX_DEGREE && adjacence[b][i] != NONE; i++) {
        a = adjacence[b][i];
        //TODO distance calculation
        //d = available_dist[b][i] + weight[b][i] - dual[a] - dual[b];
        if (available_from[a] == NONE) {
        //TODO if distance has decreased: update and bubble
        //if or available_from = NONE: insertion sort
            if (tree_index[a] == NONE || ((tree_index[a]&1) == 0)) {
                //insert into list
                available_next[a] = available_top;
                available_prev[a] = NONE;
                available_prev[available_top] = a;
                available_top = a; 

                //set value
                available_from[a] = b;
                //TODO available_dist[a] = d;
           }
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
    
    //iterate over a in adjacence[m];
    //  push to stack
    for(i=0; i<=MAX_DEGREE && adjacence[a][i] != NONE; i++) {
        b = adjacence[a][i];
        available_next[b] = available_top;
        available_prev[b] = NONE;
        available_prev[available_top] = b;
        available_top = b; 

        available_from[b] = a;
        //TODO: add distance calc
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
    //clear tree
    for (a=0; a<tree_index_max; a++) {
        tree_parent[a] = NONE;
        tree_index[a] = NONE;
        //TODO: add dual update
    }
    tree_index_max = 0;
}

void make_blossom(int a, int b) {
    //create new vertex bl
    //while tree_index[a] != tree_index[b]
    //  need to add new outer edges
    //  if ti[a] > ti[b]:
    //    blossom_parent[a] = bl;
    //    a = parent[a];
    //  if ti[a] < ti[b]:
    //    blossom_parent[b] = bl;   
    //    b = parent[b];
    //set tree_index[bl] = tree_index[a];
    //set tree_parent[bl] = tree_parent[a];
    
    //TODO: change tree pointers to blossom cycle
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
        dual[i] = NONE;
        matching[i] = NONE;
        tree_parent[i] = NONE;
        tree_index[i] = NONE;
        blossom_parent[i] = NONE;
        available_next[i] = NONE;
        available_prev[i] = NONE;
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

    printf("vert\tmatch\ttreepar\ttreeind\tav_dist\tav_nxt\tav_prv\tav_fr\n");
    for(i=0; i<MAX_VERTICES && adjacence[i][0] != NONE; i++) {
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i,
            matching[i], tree_parent[i], tree_index[i], 
            available_dist[i], available_next[i], available_prev[i],
            available_from[i]);
    }
}

void dump_adjacency_matrix() {
    int i, j, i2;


    // generate the dense adjacency matrix
    int mat[MAX_VERTICES][MAX_VERTICES];

    for(i=0; i<MAX_VERTICES; i++)
        for(j=0; j<MAX_VERTICES; j++)
            mat[i][j] = NONE;
    

    for(i=0; i<MAX_VERTICES && adjacence[i][0] != NONE; i++) {
        for(j=0; j<MAX_DEGREE && adjacence[i][j] != NONE; j++) {
            i2 = adjacence[i][j];
            mat[i][i2] = weight[i][j];
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

            if(mat[i][j] != NONE) {
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

int main(int argc, char** argv) {

    int i, j;

    init();
    if(argc > 1) {
        read_file(argv[1]);
    }

    dump_graph();
    dump_adjacency_matrix();

    for(i=0; i <= MAX_VERTICES && adjacence[i][0] != NONE; i++) {
        //for each unpaired vertex
        if(matching[i] == NONE) {
            printf("\nstarting tree from %d\n", i);
            a = i;
            start_tree();
            dump_tree();
            dump_stack();
            
            while(available_top != NONE) {
                j = available_top;
                available_top = available_next[j];
                available_prev[available_top] = NONE;

                if (matching[j] != NONE) {
                    //if matched, tree can grow
                    printf("growing to %d\n", j);
                    a = j;
                    grow_tree();
                    available_from[j] = NONE;
                } else {
                    //if unmatched, we can collapse, done
                    printf("collapsing to %d\n", j);
                    a = j;
                    collapse_tree();
                    available_from[j] = NONE;
                    break;
                }
                available_from[j] = NONE;
                dump_tree();
                dump_stack();
            }
        }
    }

    dump_tree();
    dump_adjacency_matrix();





}
