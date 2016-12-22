#include <stdlib.h>
#include <stdio.h>



//everything is integers, no structs, because the bitbang-level structure should be exposed.


#define MAX_VERTICES 100
#define MAX_DEGREE (20)
#define MAX_QUEUE (20)

#define NONE ((unsigned int)0x1234)

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

//availability stack via outer edge;
unsigned int available_next[MAX_VERTICES]; //next in stack
unsigned int available_from[MAX_VERTICES];


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
        available_from[i] = NONE;
        }
}

void read_file(char* filename) {
    //file format is node1 node2 weight
    FILE* file;
    file = fopen(filename, "r");

    int node1, node2, w;
    int i,j;

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
}

void dump_graph() {
    int i, j;

    printf("adjacence:\n");

    for(i=0; i<MAX_VERTICES && adjacence[i][0] != NONE; i++) {
        printf("%d:\t", i);
        for(j=0; j<MAX_DEGREE && adjacence[i][j] != NONE; j++) {
            printf("%d, %d;\t", adjacence[i][j], weight[i][j]);
        }
        printf("\n");
    }


}

int main(int argc, char** argv) {
    init();
    if(argc > 1) {
        read_file(argv[1]);
    }
    dump_graph();
}



int what_to_do_with_edge(int from, int to) {
    //if to not in tree: grow_tree
    //(i.e. if I(to) < I(root)
    //
    //if to in tree and inner, ignore
    //if to in tree and outer: make blossom
}

//the four primitives

void grow_tree(int from, int to) {
    //set tree_index[to]
    //set tree_parent[to] = from
    //get m = matching[to]
    //set tree_index[m]
    //set tree_parent[m] = to
    //iterate over a in adjacence[m]:
    //  if a not in available stack: push to stack.
}

int blossom_of(int v) {
    //while blossom_parent[v] != null:
    //  v = blossom_parent[v]
    //return v
    //need to add dual calculation
}

void collapse_tree(int from) {
    //a = from
    //b = parent[from]
    //while b is not null:
        //matching[a] = b
        //matching[b] = a
        //a = parent(b)
        //b = parent(a)
        //change dual[a/b]
    //empty available stack.
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
