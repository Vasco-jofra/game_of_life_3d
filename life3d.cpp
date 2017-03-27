#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <vector>

#define MAX_SIZE 10000
bool DEBUG = false;

// Define the hash for tuple<int, int, int> so we can use it in the hash_map
typedef std::tuple<int, int, int> Vector3;
namespace std {
template <>
struct hash<Vector3> {
    std::size_t operator()(const Vector3& k) const
    {
        return (std::get<0>(k) * (MAX_SIZE + 1) * (MAX_SIZE + 1)) + (std::get<1>(k) * (MAX_SIZE + 1)) + (std::get<2>(k));
    }
};
}

// Node representing elements of sparse matrix
struct node {
    int z;
    struct node *next, *prev;
};

typedef struct node* z_list;

std::vector<Vector3> to_insert;
std::vector<Vector3> to_remove;
std::unordered_map<Vector3, int> dead_to_check;

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
    if (DEBUG)
        printf("Inserting (%d, %d, %d)\n", x, y, z);

    z_list new_el = (z_list)malloc(sizeof(struct node));
    new_el->z = z;
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

void matrix_remove(Matrix* m, int x, int y, int z)
{
    z_list ptr = matrix_get(m, x, y);

    while(ptr) {
        if(ptr->z == z) {
            if (DEBUG)
                printf("Removing (%d, %d, %d)\n", x, y, z);

            // Actually removing here
            if(ptr->next) {
                ptr->next->prev = ptr->prev;
            }

            if(ptr->prev) {
                ptr->prev->next = ptr->next;
            } else {
                // We are the head being removed. (Assigning to NULL it's ok here.)
                m->data[x + (y * m->side)] = ptr->next;
            }
            free(ptr);
            ptr = NULL;
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
                printf("%d%s", ptr->z, ptr->next == NULL ? "" : ", ");
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
    if (val >= mod)   return val - mod;
    else if (val < 0) return val + mod;
    else              return val;
    // return ((val % mod) + mod) % mod;
}

bool matrix_ele_exists(Matrix* m, int x, int y, int z)
{
    z_list ptr = matrix_get(m, x, y);

    while (ptr != NULL) {
        if (ptr->z == z)
            return true;
        ptr = ptr->next;
    }

    return false;
}

void insert_or_update_in_dead_to_check(int x, int y, int z)
{
    // @ Sync: Synchronize here the addition and/or creation of the element!
    dead_to_check[std::make_tuple(x, y, z)]++;
    if(DEBUG)
        printf("    Dead cell (%d, %d, %d) now has a count of %d.\n", x, y, z, dead_to_check[std::make_tuple(x, y, z)]);
}

int count_neighbours(Matrix* m, int x, int y, z_list ptr)
{
    if(DEBUG)
        printf("Counting neighbors for (%d, %d, %d)\n", x, y, ptr->z);

    int cnt = 0;
    int z = ptr->z;
    int SIZE = m->side;

    int _z = z + 1;
    if (ptr->next) {
        // if we have a next we surely are not at the end
        if (ptr->next->z == _z)
            cnt++;
    } else {
        // check if we are wrapping arround, and if so, if the ele on the other side exists
        if (_z >= SIZE) {
            // We did wrap arround! Check again
            _z = _z - SIZE;
            if (matrix_ele_exists(m, x, y, _z))
                cnt++;
            else
                insert_or_update_in_dead_to_check(x, y, _z);
        } else {
            insert_or_update_in_dead_to_check(x, y, _z);
        }
    }

    _z = z - 1;
    if (ptr->prev) {
        // if we have a next we surely are not at the end
        if (ptr->prev->z == _z)
            cnt++;
    } else {
        // check if we are wrapping arround, and if so, if the ele on the other side exists
        if (_z < 0) {
            // We did wrap arround! Check again
            _z = (_z + SIZE);
            if (matrix_ele_exists(m, x, y, _z))
                cnt++;
            else
                insert_or_update_in_dead_to_check(x, y, _z);
        } else {
            insert_or_update_in_dead_to_check(x, y, _z);
        }
    }

    int _x = pos_mod(x + 1, SIZE);
    if (matrix_ele_exists(m, _x, y, z)) {
        cnt++;
    } else {
        insert_or_update_in_dead_to_check(_x, y, z);
    }

    _x = pos_mod(x - 1, SIZE);
    if (matrix_ele_exists(m, _x, y, z)) {
        cnt++;
    } else {
        insert_or_update_in_dead_to_check(_x, y, z);
    }

    int _y = pos_mod(y + 1, SIZE);
    if (matrix_ele_exists(m, x, _y, z)) {
        cnt++;
    } else {
        insert_or_update_in_dead_to_check(x, _y, z);
    }

    _y = pos_mod(y - 1, SIZE);
    if (matrix_ele_exists(m, x, _y, z)) {
        cnt++;
    } else {
        insert_or_update_in_dead_to_check(x, _y, z);
    }

    if (DEBUG)
        printf("Element (%d, %d, %d) has %d neighbors.\n", x, y, z, cnt);
    return cnt;
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
        matrix_insert(&m, x, y, z);
    }
    // Finished parsing!
    fclose(fp);

    //-----------------
    //--- MAIN LOOP ---
    //-----------------
    for (int gen = 0; gen < generations; gen++) {
        if (DEBUG)
            printf("------------------------\n");
        if (DEBUG)
            printf("Starting generation %d\n", gen);

        // @PARALLEL: Where we parallelize
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                z_list ptr = matrix_get(&m, i, j);
                // Iterate over every existing z for x and y
                while (ptr != NULL) {
                    int counter = count_neighbours(&m, i, j, ptr);
                    if (counter < 2 || counter > 4)
                        to_remove.push_back(std::make_tuple(i, j, ptr->z));
                    ptr = ptr->next;
                }
            }
        }

        // Check the dead ones that were neighbours now
        for (auto& it : dead_to_check) {
            if (it.second == 2 || it.second == 3) {
                to_insert.push_back(it.first);
            }
            if (DEBUG)
                printf("Dead cell (%d, %d, %d) has %d neighbors.\n", std::get<0>(it.first), std::get<1>(it.first), std::get<2>(it.first), it.second);
        }

        // By sorting we can remove the nodes closest to the head first. Could be further optimized (only go throught the list once)!
        // @PROFILE!
        std::sort(to_remove.begin(), to_remove.end());
        for (auto& t : to_remove) {
            matrix_remove(&m, std::get<0>(t), std::get<1>(t), std::get<2>(t));
        }

        // Do the inserting and removing now
        for (auto& t : to_insert) {
            matrix_insert(&m, std::get<0>(t), std::get<1>(t), std::get<2>(t));
        }

        // Clear all the structures for the next iteration
        to_insert.clear();
        to_remove.clear();
        dead_to_check.clear();
        if (DEBUG)
            printf("------------------------\n");
    }


    //-----------
    //--- END ---
    //-----------
    // Output the result
    matrix_print_live(&m);

    /*
    matrix_print(&m);
    printf("\n\n\n\n\n\n\n\n");
    Matrix m2 = make_matrix(5);
    matrix_insert(&m2, 0, 0, 2);
    matrix_print(&m2);
    matrix_insert(&m2, 0, 0, 1);
    matrix_print(&m2);
    matrix_insert(&m2, 0, 0, 0);
    matrix_print(&m2);
    */
}
