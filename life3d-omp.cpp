#include <algorithm>
#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

#define MAX_SIZE 10000
bool DEBUG = false;

const char* RED = "\033[31m";
const char* GREEN = "\033[32m";
const char* YELLOW = "\033[33m";
const char* BLUE = "\033[34m";
const char* NO_COLOR = "\033[0m";

// Define the hash for tuple<int, int, int> so we can use it in the hash_map
typedef std::tuple<int, int, int> Vector3;

// Node representing elements of sparse matrix
struct node {
    int z;
    int num_neighbours;
    bool is_dead;
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

void matrix_insert(Matrix* m, int x, int y, int z, bool is_dead, int num_nei)
{
    if (DEBUG)
        printf("%sInserting (%d, %d, %d) %s %s\n", GREEN, x, y, z, is_dead ? "dead" : "alive", NO_COLOR);

    z_list new_el = (z_list)malloc(sizeof(struct node));
    new_el->z = z;
    new_el->num_neighbours = num_nei;
    new_el->is_dead = is_dead;
    new_el->next = NULL;
    new_el->prev = NULL;

    z_list ptr = matrix_get(m, x, y);

    if (ptr == NULL) {
        // Case where there are no nodes yet in the linked list
        m->data[x + (y * m->side)] = new_el;
    } else if (z < ptr->z) {
        // We are the head
        new_el->next = ptr;
        ptr->prev = new_el;
        m->data[x + (y * m->side)] = new_el;
    } else {
        // Search the list for our spot
        while (ptr->next) {
            if (z < ptr->next->z) {
                new_el->next = ptr->next;
                new_el->prev = ptr;
                ptr->next->prev = new_el;
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

void matrix_insert_dead_front(int z, z_list ptr)
{
    z_list new_el = (z_list)malloc(sizeof(struct node));
    new_el->z = z;
    new_el->num_neighbours = 1;
    new_el->is_dead = true;

    new_el->next = ptr->next;
    new_el->prev = ptr;
    if (ptr->next)
        ptr->next->prev = new_el;
    ptr->next = new_el;
}

void matrix_remove_from_ptr(Matrix* m, z_list ptr, int x, int y)
{
    /*if (DEBUG)
        printf("%sRemoving (%d, %d, %d)%s\n", RED, x, y, z, NO_COLOR);*/

    if (ptr->next) {
        ptr->next->prev = ptr->prev;
    }

    if (ptr->prev) {
        ptr->prev->next = ptr->next;
    } else {
        // We are the head being removed. (Assigning to NULL it's ok here.)
        m->data[x + (y * m->side)] = ptr->next;
    }
    free(ptr);
    ptr = NULL;
}

void matrix_remove(Matrix* m, int x, int y, int z)
{
    z_list ptr = matrix_get(m, x, y);

    while (ptr) {
        if (ptr->z == z) {
            matrix_remove_from_ptr(m, ptr, x, y);
            return;
        }
        ptr = ptr->next;
    }
    if (DEBUG)
        printf("Tried to remove, but entry (%d, %d, %d) was not found.\n", x, y, z);
}

void matrix_print(Matrix* m)
{
    int SIZE = m->side;
    printf("MATRIX: \n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d %d (", i, j);

            z_list ptr = matrix_get(m, i, j);
            while (ptr != NULL) {
                printf("%s%d%s%s", ptr->is_dead ? RED : GREEN, ptr->z, ptr->next == NULL ? "" : ", ", NO_COLOR);
                ptr = ptr->next;
            }
            printf(")\n");
        }
    }
}

void matrix_print_backwards(Matrix* m)
{
    int SIZE = m->side;
    printf("MATRIX: \n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d %d (", i, j);
            fflush(stdout);

            z_list ptr = matrix_get(m, i, j);

            // Go to the end of the list
            while (ptr && ptr->next) {
                ptr = ptr->next;
            }
            while (ptr) {
                printf("%d%s", ptr->z, ptr->prev == NULL ? "" : ", ");
                fflush(stdout);
                ptr = ptr->prev;
            }
            printf(")\n");
        }
    }
}

void matrix_print_live(Matrix* m)
{
    int SIZE = m->side;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            z_list ptr = matrix_get(m, i, j);
            while (ptr != NULL) {
                printf("%d %d %d\n", i, j, ptr->z);
                ptr = ptr->next;
            }
        }
    }
}

inline int pos_mod(int val, int mod)
{
    if (val >= mod)
        return val - mod;
    else if (val < 0)
        return val + mod;
    else
        return val;
    // return ((val % mod) + mod) % mod;
}

bool matrix_ele_exists(Matrix* m, int x, int y, int z)
{
    z_list ptr = matrix_get(m, x, y);

    while (ptr != NULL) {
        if (ptr->z == z)
            return !ptr->is_dead;
        ptr = ptr->next;
    }

    return false;
}

z_list matrix_get_ele(Matrix* m, int x, int y, int z)
{
    z_list ptr = matrix_get(m, x, y);

    while (ptr != NULL) {
        if (ptr->z == z)
            return ptr;
        else if (ptr->z > z)
            return NULL;

        ptr = ptr->next;
    }

    return NULL;
}

size_t matrix_size(Matrix* m)
{
    int cnt = 0;
    int SIZE = m->side;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            z_list ptr = matrix_get(m, i, j);
            while (ptr != NULL) {
                cnt++;
                ptr = ptr->next;
            }
        }
    }
    return cnt;
}

int main(int argc, char* argv[])
{
    double start, end, init_time, process_time;
    start = omp_get_wtime();
    if (argc != 3) {
        printf("[ERROR] Incorrect usage!\n");
        printf("[Usage] ./life3d <input_file> <nr_generations>\n");
        return -1;
    }
    char* input_file = argv[1];
    int generations = atoi(argv[2]);
    if (DEBUG)
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
    Matrix m = make_matrix(SIZE); //@SEE: Why +1?

    int x, y, z;
    while (fscanf(fp, "%d %d %d", &x, &y, &z) != EOF) {
        if (DEBUG)
            printf("GOT: %d %d %d\n", x, y, z);
        matrix_insert(&m, x, y, z, false, -1);
    }
    // Finished parsing!
    fclose(fp);

    omp_lock_t writelock[SIZE];
    for (int i = 0; i < SIZE; i++) {
        omp_init_lock(&writelock[i]);
    }

    end = omp_get_wtime();
    init_time = end - start;
    start = omp_get_wtime();

    //-----------------
    //--- MAIN LOOP ---
    //-----------------
    int gen = 0;
#pragma omp parallel private(gen)
    {
        for (gen = 0; gen < generations; gen++) {
#pragma omp single
            {
                if (DEBUG)
                    printf("------------------------\n");
                if (DEBUG)
                    printf(" *** Starting generation %d ***\n", gen);
            }
            int i, j;
            z_list ptr, to_test;
            Vector3 t;
            int _z, _y, _x;
            int z, y, x;
            bool looped;
#pragma omp for private(i, j, t, ptr, _z, _y, _x, x, y, z, to_test, looped) schedule(dynamic, 100)
            for (i = 0; i < SIZE; i++) {
                for (j = 0; j < SIZE; j++) {
                    ptr = matrix_get(&m, i, j);
                    // Iterate over every existing z for x and y
                    while (ptr != NULL) {
                        // If its dead skip
                        if (ptr->is_dead) {
                            ptr = ptr->next;
                            continue;
                        }

                        x = i;
                        y = j;
                        z = ptr->z;
                        ptr->num_neighbours = 0;

                        _z = pos_mod(z + 1, SIZE);
                        looped = (_z != z + 1);
                        if (looped)
                            to_test = matrix_get_ele(&m, x, y, _z);
                        else
                            to_test = ptr->next;

                        if (to_test && to_test->z == _z) {
                            if (to_test->is_dead == true) {
                                to_test->num_neighbours++;
                            } else {
                                ptr->num_neighbours++;
                            }
                        } else {
                            if (!looped) {
                                matrix_insert_dead_front(_z, ptr);
                            } else {
                                matrix_insert(&m, x, y, _z, true, 1);
                            }
                        }

                        _z = pos_mod(z - 1, SIZE);
                        looped = (_z != z - 1);
                        if (looped)
                            to_test = matrix_get_ele(&m, x, y, _z);
                        else
                            to_test = ptr->prev;

                        if (to_test && to_test->z == _z) {
                            if (to_test->is_dead == true) {
                                to_test->num_neighbours++;
                            } else {
                                ptr->num_neighbours++;
                            }
                        } else {
                            if (!looped && to_test) {
                                matrix_insert_dead_front(_z, to_test);
                            } else {
                                matrix_insert(&m, x, y, _z, true, 1);
                            }
                        }

                        _x = pos_mod(x + 1, SIZE);
                        to_test = matrix_get_ele(&m, _x, y, z);
                        if (to_test) {
                            if (to_test->is_dead == true) {
                                to_test->num_neighbours++;
                            } else {
                                ptr->num_neighbours++;
                            }
                        } else {
                            matrix_insert(&m, _x, y, z, true, 1);
                        }

                        _x = pos_mod(x - 1, SIZE);
                        to_test = matrix_get_ele(&m, _x, y, z);
                        if (to_test) {
                            if (to_test->is_dead == true) {
                                to_test->num_neighbours++;
                            } else {
                                ptr->num_neighbours++;
                            }
                        } else {
                            matrix_insert(&m, _x, y, z, true, 1);
                        }

                        _y = pos_mod(y + 1, SIZE);
                        to_test = matrix_get_ele(&m, x, _y, z);
                        if (to_test) {
                            if (to_test->is_dead == true) {
                                to_test->num_neighbours++;
                            } else {
                                ptr->num_neighbours++;
                            }
                        } else {
                            matrix_insert(&m, x, _y, z, true, 1);
                        }

                        _y = pos_mod(y - 1, SIZE);
                        to_test = matrix_get_ele(&m, x, _y, z);
                        if (to_test) {
                            if (to_test->is_dead == true) {
                                to_test->num_neighbours++;
                            } else {
                                ptr->num_neighbours++;
                            }
                        } else {
                            matrix_insert(&m, x, _y, z, true, 1);
                        }

                        ptr = ptr->next;
                    }
                }
            }
            if (DEBUG) {
                printf("After inserts.\n");
                matrix_print(&m);
            }

#pragma omp for private(i, j, t, ptr, _z, _y, _x, x, y, z, to_test, looped) schedule(dynamic, 100)
            for (i = 0; i < SIZE; i++) {
                for (j = 0; j < SIZE; j++) {
                    ptr = matrix_get(&m, i, j);
                    // Iterate over every existing z for x and y
                    while (ptr != NULL) {
                        // printf("(%d, %d, %d) count: %d\n", i, j, ptr->z, ptr->num_neighbours);
                        // If its dead skip
                        if (ptr->is_dead) {
                            if (ptr->num_neighbours == 2 || ptr->num_neighbours == 3) {
                                ptr->is_dead = false;
                            } else {
                                if (DEBUG)
                                    printf("%sRemoving (%d, %d, %d)%s\n", RED, i, j, ptr->z, NO_COLOR);
                                matrix_remove_from_ptr(&m, ptr, i, j);
                            }
                        } else {
                            if (ptr->num_neighbours < 2 || ptr->num_neighbours > 4) {
                                if (DEBUG)
                                    printf("%sRemoving (%d, %d, %d)%s\n", RED, i, j, ptr->z, NO_COLOR);
                                matrix_remove_from_ptr(&m, ptr, i, j);
                            }
                        }
                        ptr = ptr->next;
                    }
                }
            }
            if (DEBUG) {
                printf("After removes.\n");
                matrix_print(&m);
            }

            /*
// Check the dead ones that were neighbours now
#pragma omp for private(i, x, y, z, t, counter) schedule(dynamic, 100)
            for (i = 0; i < dead_to_check.size(); i++) {
                x = std::get<0>(dead_to_check[i]);
                y = std::get<1>(dead_to_check[i]);
                z = std::get<2>(dead_to_check[i]);

                counter = count_neighbours_of_dead(&m, x, y, z);
                if (counter == 2 || counter == 3) {
#pragma omp critical(TO_INSERT)
                    {
                        to_insert.push_back(std::make_tuple(x, y, z));
                    }
                }
                // if (DEBUG)
                //     printf("Dead cell (%d, %d, %d) has %d neighbors.\n", std::get<0>(it.first), std::get<1>(it.first), std::get<2>(it.first), it.second);
            }
#pragma omp for private(i, x, y, z)
            for (i = 0; i < to_remove.size(); i++) {
                x = std::get<0>(to_remove[i]);
                y = std::get<1>(to_remove[i]);
                z = std::get<2>(to_remove[i]);

                omp_set_lock(&writelock[x]);
                matrix_remove(&m, x, y, z);
                omp_unset_lock(&writelock[x]);
            }

#pragma omp for private(i, x, y, z)
            for (i = 0; i < to_insert.size(); i++) {
                x = std::get<0>(to_insert[i]);
                y = std::get<1>(to_insert[i]);
                z = std::get<2>(to_insert[i]);

                omp_set_lock(&writelock[x]);

                if (!matrix_ele_exists(&m, x, y, z)) {
                    matrix_insert(&m, x, y, z);
                }

                omp_unset_lock(&writelock[x]);
            }

#pragma omp single
            {
                // Clear all the structures for the next iteration
                to_insert.clear();
                to_remove.clear();
                dead_to_check.clear();

                // matrix_print(&m);

                if (DEBUG)
                    printf("------------------------\n");
            }
*/
        }
    }
    end = omp_get_wtime();
    process_time = end - start;

    //-----------
    //--- END ---
    //-----------
    // Output the result
    matrix_print_live(&m);

    // Write the time log to a file
    FILE* out_fp = fopen("time.log", "w");
    char out_str[80];
    sprintf(out_str, "OMP %s: \ninit_time: %lf \nproc_time: %lf\n", input_file, init_time, process_time);
    fwrite(out_str, strlen(out_str), 1, out_fp);
}