#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <tuple>

typedef std::tuple<int, int, int> Vector3;

// Node representing elements of the sparse matrix
struct node {
    short z; // The z value
    short num_neighbours; // The # of neighbours (not always right, only when we need it)
    bool is_dead; // Is it a dead or an alive node?
    struct node *next, *prev; // Double linked list pointers
};
typedef struct node* z_list;

struct matrix_struct {
    short side;
    z_list* data;
};
typedef matrix_struct Matrix;

// Returns a initialized matrix
inline Matrix make_matrix(short side)
{
    Matrix m;
    m.side = side;
    m.data = (z_list*)calloc(side * side, sizeof(z_list));
    return m;
}

// Returns the head of the matric in the position x and y. Can be NULL.
inline z_list matrix_get(Matrix* m, short x, short y)
{
    return m->data[x + (y * m->side)];
}

// Returns the element with the x, y, z passed. NULL if it doesn't exist.
inline z_list matrix_get_ele(Matrix* m, short x, short y, short z)
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

// Insert a new element in the matrix (ordered)
inline void matrix_insert(Matrix* m, short x, short y, short z, bool is_dead, short num_nei)
{
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
        // We are the new head
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

// Inserts a dead node in front of the pointer passed in the arguments
inline void matrix_insert_dead_front(short z, z_list ptr)
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

// Remove the given pointer in the arguments from the matrix.
inline void matrix_remove_from_ptr(Matrix* m, z_list ptr, short x, short y)
{
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

// Remove from the matrix. We need to search for the z in the linked list
inline void matrix_remove(Matrix* m, short x, short y, short z)
{
    z_list ptr = matrix_get(m, x, y);

    while (ptr) {
        if (ptr->z == z) {
            matrix_remove_from_ptr(m, ptr, x, y);
            return;
        }
        ptr = ptr->next;
    }
}

// Print the live nodes in the matrix (the matrix only contains alive nodes at this point, so print all nodes)
void matrix_print_live(Matrix* m)
{
    short SIZE = m->side;
    for (short i = 0; i < SIZE; i++) {
        for (short j = 0; j < SIZE; j++) {
            z_list ptr = matrix_get(m, i, j);
            while (ptr != NULL) {
                printf("%hd %hd %hd\n", i, j, ptr->z);
                ptr = ptr->next;
            }
        }
    }
}

inline short pos_mod(short val, short mod)
{
    if (val >= mod)
        return val - mod;
    else if (val < 0)
        return val + mod;
    else
        return val;

    // The method above is faster!
    // return ((val % mod) + mod) % mod;
    // return (val % mod) + (mod * (val < 0));
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
    short SIZE;
    if (fscanf(fp, "%hd", &SIZE) == EOF) {
        printf("[ERROR] Unable to read the size.\n");
        return -1;
    }

    // Finished parsing metadata. Now only need to parse the actual positions
    Matrix m = make_matrix(SIZE);

    short x, y, z;
    while (fscanf(fp, "%hd %hd %hd", &x, &y, &z) != EOF) {
        matrix_insert(&m, x, y, z, false, -1);
    }
    // Finished parsing!
    fclose(fp);

    // Create SIZE*SIZE locks and initialize them
    omp_lock_t lock[SIZE];
    for (int i = 0; i < SIZE; i++) {
        omp_init_lock(&lock[i]);
    }

    end = omp_get_wtime();
    init_time = end - start;
    start = omp_get_wtime();

    //-----------------
    //--- MAIN LOOP ---
    //-----------------
    int gen;
#pragma omp parallel private(gen)
    {
        for (gen = 0; gen < generations; gen++) {
            z_list ptr, to_test;
            Vector3 t;
            int32_t i, j;
            short z, _z, _y, _x, y, x;
            bool looped;

#pragma omp for private(i, j, t, ptr, _z, _y, _x, x, y, z, to_test, looped) collapse(2)
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

                        // PROCESS AN ALIVE NODE
                        x = i;
                        y = j;
                        z = ptr->z;
                        ptr->num_neighbours = 0;

                        omp_set_lock(&lock[y]);
                        {
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
                        }
                        omp_unset_lock(&lock[y]);

                        _x = pos_mod(x + 1, SIZE);
                        omp_set_lock(&lock[y]);
                        {
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
                        }
                        omp_unset_lock(&lock[y]);

                        _x = pos_mod(x - 1, SIZE);
                        omp_set_lock(&lock[y]);
                        {
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
                        }
                        omp_unset_lock(&lock[y]);

                        _y = pos_mod(y + 1, SIZE);
                        omp_set_lock(&lock[_y]);
                        {
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
                        }
                        omp_unset_lock(&lock[_y]);

                        _y = pos_mod(y - 1, SIZE);
                        omp_set_lock(&lock[_y]);
                        {
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
                        omp_unset_lock(&lock[_y]);
                    }
                }
            }

#pragma omp for private(i, j, t, ptr, _z, _y, _x, x, y, z, to_test, looped) collapse(2)
            for (i = 0; i < SIZE; i++) {
                for (j = 0; j < SIZE; j++) {
                    ptr = matrix_get(&m, i, j);
                    // Iterate over every existing z for x and y
                    while (ptr != NULL) {
                        if (ptr->is_dead) {
                            if (ptr->num_neighbours == 2 || ptr->num_neighbours == 3) {
                                ptr->is_dead = false;
                            } else {
                                matrix_remove_from_ptr(&m, ptr, i, j);
                            }
                        } else {
                            if (ptr->num_neighbours < 2 || ptr->num_neighbours > 4) {
                                matrix_remove_from_ptr(&m, ptr, i, j);
                            }
                        }
                        ptr = ptr->next;
                    }
                }
            }
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