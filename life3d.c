#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************/
/*** STRUCTURES ***/
/******************/
// Node representing elements of sparse matrix
struct node {
    int z;
    struct node *next, *prev;
};

typedef struct node* z_list;

/*****************/
/*** FUNCTIONS ***/
/*****************/
/**	Initialize cube	**/
void STinit(int max, z_list matrix[][max])
{
    for (int i = 0; i < max; i++) {
        for (int j = 0; j < max; j++) {
            matrix[i][j] = NULL;
        }
    }
}

/**  Add alive cell coordinates in the cube  **/
// Inserted by ordered to maintain sparse matrix
void STinsert(int n, z_list matrix[][n], int x, int y, int z)
{
    z_list ptr;
    z_list new_el;

    ptr = matrix[x][y];
    new_el = (z_list)malloc(sizeof(z_list));
    new_el->z = z;
    new_el->next = NULL;
    new_el->prev = NULL;

    if (matrix[x][y] == NULL) {
        matrix[x][y] = new_el;
    } else {
        if (z < ptr->z) {
			  matrix[x][y]->prev = new_el;
            new_el->next = matrix[x][y];
            matrix[x][y] = new_el;
            return;
        } else {
            while (ptr->next != NULL) {
                if (z < ptr->next->z) {
                    new_el->next = ptr->next;
                    new_el->prev = ptr;
                    ptr->next = new_el;
                    return;
                } else {
                    ptr = ptr->next;
                }
            }
            ptr->next = new_el;
            new_el->prev = ptr;
        }
    }
    return;
}

int onLimitZ(int N, int z) {
	
	if)
	return z+1 <= N || z-1 >=0;
	
}
	
int countNeighbours(int SIZE, z_list matrix[][SIZE], int x, int y, z_list ptr){
	printf("POINTER:x %d y %d z %d SIZE:%d\n",x,y, ptr->z,SIZE);
	
	if()
		printf("HELLO\n");
		
		
		
		return 0;
}

void TraverseAliveCells(int n, z_list matrix[][n]){
   z_list ptr;
	int counter;
	
   for (int i = 0; i < n; i++) {
       for (int j = 0; j < n; j++) {
           ptr = matrix[i][j];
           while (ptr != NULL) {
				  counter = countNeighbours(n,matrix,i,j,ptr);
               ptr = ptr->next;
           }
       }
   }
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        printf("[ERROR] Incorrect usage!\n");
        printf("[Usage] ./life3d <input_file> <nr_generations>\n");
        return -1;
    }
    char* input_file = argv[1];
    int generations = atoi(argv[2]);
    printf("Input file: '%s'\nGenerations: %d\n", input_file, generations);

    if (generations <= 0) {
        printf("[ERROR] Number of generations must be bigger that 0. Got: '%d'\n", generations);
        return -1;
    }

    FILE* fp = fopen(input_file, "r");
    if (fp == NULL) {
        printf("[ERROR] Unable to read the input file.\n");
        perror("[ERROR]");
        return -1;
    }
    int SIZE;
    if (fscanf(fp, "%d", &SIZE) == EOF) {
        printf("[ERROR] Unable to read the size.\n");
        return -1;
    }

    // Finished parsing metadata. Now only need to parse the actual positions
    z_list cell_matrix[SIZE][SIZE];
    STinit(SIZE, cell_matrix);

    int x, y, z;
    while (fscanf(fp, "%d %d %d", &x, &y, &z) != EOF) {
        printf("%d %d %d\n", x, y, z);
        STinsert(SIZE, cell_matrix, x, y, z);
    }
    fclose(fp);

TraverseAliveCells(SIZE,cell_matrix);

    // Print
    z_list ptr;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            ptr = cell_matrix[i][j];
            while (ptr != NULL) {
                printf("x y z : %d %d %d\n", i, j, ptr->z);
                ptr = ptr->next;
            }
        }
    }
    return 0;
}
