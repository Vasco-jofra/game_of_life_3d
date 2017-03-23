#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int x_plus[4] = { -1, 1, 0, 0 };
int y_plus[4] = { 0, 0, -1, 1 };

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
void STinsert(int N, z_list matrix[][n], int x, int y, int z)
{
    z_list ptr = matrix[x][y];
    z_list new_el;

    new_el = (z_list)malloc(sizeof(struct node));
    // new_el = (z_list) malloc(sizeof(z_list)); // @BUG: This was a bug! z_list is a pointer, so it was only allocating 8 bytes out of the 24 bytes we want. It was working out o luck because the allcoator probably allocates with some extra space to allign stuff probably.
    // printf("size of z_list: %ld\n", sizeof(z_list));
    // printf("size of node  : %ld\n", sizeof(struct node));

    new_el->z = z;
    new_el->next = NULL;
    new_el->prev = NULL;

    if (matrix[x][y] == NULL) {
        // Case where there are no nodes yet in the linked list
        matrix[x][y] = new_el;
    } else {
        // We already have elements. Insert it ordered.
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

int FindNeighbour(int SIZE, z_list matrix[][SIZE], int x, int y, int z)
{
    z_list ptr = matrix[x][y];

    while (ptr != NULL) {
        if (z == ptr->z)
            return 1;
        ptr = ptr->next;
    }

    return 0;
}

int onLimit(int x, int y, int max)
{
    if (x >= 0 && x <= max && y >= 0 && y <= max)
        return 1;

    return 0;
}

int CountNeighbours(int SIZE, z_list matrix[][SIZE], int x, int y, z_list ptr)
{
    int w = 0, cnt = 0;
    printf("POINTER:x %d y %d z %d SIZE:%d\n", x, y, ptr->z, SIZE);

    if (ptr->next != NULL && (ptr->next->z == ptr->z + 1))
        cnt++;

    if (ptr->prev != NULL && (ptr->prev->z == ptr->z - 1))
        cnt++;

    for (w; w < 4; w++) {
        if (onLimit(x + x_plus[w], y + y_plus[w], SIZE))
            cnt = cnt + FindNeighbour(SIZE, matrix, x + x_plus[w], y + y_plus[w], ptr->z);
    }

    printf("COUNTER: %d\n", cnt);

    return 0;
}

void TraverseAliveCells(int n, z_list matrix[][n])
{
    z_list ptr;
    int counter;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            ptr = matrix[i][j];
            while (ptr != NULL) {
                counter = CountNeighbours(n, matrix, i, j, ptr);
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

    TraverseAliveCells(SIZE, cell_matrix);

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