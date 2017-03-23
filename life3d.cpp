#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int x_plus[4] = { -1, 1, 0, 0 };
int y_plus[4] = { 0, 0, -1, 1 };

// Node representing elements of sparse matrix
struct node {
    int z;
    struct node *next, *prev;
};

typedef struct node* z_list;

typedef struct matrix_str {
    int side;
    z_list* data;
} Matrix;

Matrix make_matrix(int side)
{
    Matrix m;
    m.side = side;
    m.data = (z_list*)calloc(side * side, sizeof(z_list));
    return m; 
}

z_list matrix_get(Matrix* m, int x, int y)
{
    return m->data[x + (y * m->side)];
}

void matrix_insert(Matrix* m, int x, int y, int z)
{
    z_list new_el = (z_list)malloc(sizeof(struct node));
    new_el->z = z;
    new_el->next = NULL;
    new_el->prev = NULL;

    z_list ptr = matrix_get(m, x, y);

    if (ptr == NULL) {
        // Case where there are no nodes yet in the linked list
        m->data[x + (y * m->side)] = new_el;
    } else {
        // We already have elements. Insert it ordered.
        while (ptr->next != NULL) {
            if (z < ptr->next->z) {
                new_el->next = ptr->next;
                new_el->prev = ptr;
                ptr->next = new_el;
                break;
            } else {
                ptr = ptr->next;
            }
        }
        ptr->next = new_el;
        new_el->prev = ptr;
    }
}

void matrix_print(Matrix* m)
{
    int SIZE = m->side;
    printf("MATRIX: \n");
    for (int i = 0; i < SIZE ; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d %d (", i, j);
            z_list ptr = matrix_get(m, i, j);
            while (ptr != NULL) {
                printf("%d%s", ptr->z, ptr->next == NULL ? "" : ", ");
                ptr = ptr->next;
            }
            printf(")\n");
        }
    }
}

int FindNeighbour(Matrix* m, int x, int y, int z)
{
  z_list ptr = matrix_get(m, x, y);

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

int CountNeighbours(Matrix* m, int x, int y, z_list ptr, int SIZE)
{
    int w = 0, cnt = 0;
    printf("POINTER:x %d y %d z %d\n", x, y, ptr->z);

    if (ptr->next != NULL && (ptr->next->z == ptr->z + 1))
        cnt++;

    if (ptr->prev != NULL && (ptr->prev->z == ptr->z - 1))
        cnt++;

    for (w; w < 4; w++) {
        if (onLimit(x + x_plus[w], y + y_plus[w], SIZE))
            cnt = cnt + FindNeighbour(m, x + x_plus[w], y + y_plus[w], ptr->z);
    }

    printf("COUNTER: %d\n", cnt);

    return 0;
}

void TraverseAliveCells(Matrix* m)
{
    z_list ptr;
    int counter;
    int SIZE = m->side;

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
           ptr = matrix_get(m, i, j);
            while (ptr != NULL) {
                counter = CountNeighbours(m, i, j, ptr,SIZE);
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
    Matrix m = make_matrix(SIZE+1);

    int x, y, z;
    while (fscanf(fp, "%d %d %d", &x, &y, &z) != EOF) {
        printf("GOT: %d %d %d\n", x, y, z);
        matrix_insert(&m, x, y, z);
    }
    fclose(fp);

    TraverseAliveCells(&m);

    // Print
    matrix_print(&m);
}
