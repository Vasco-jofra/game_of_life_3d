#include <algorithm>
#include <assert.h>
#include <math.h>
#include <mpi.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <tuple>
#include <unistd.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define BLOCK_LOW(id, p, n) ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_SIZE(id, p, n) (BLOCK_HIGH(id, p, n) - BLOCK_LOW(id, p, n) + 1)
#define BLOCK_OWNER(index, p, n) (((p) * ((index) + 1) - 1) / (n))

#define TAG_Z_LENGTHS 16
#define TAG_INIT_MATRIX 17
#define TAG_SWAP_ROWS 18

// Node representing elements of the sparse matrix
struct node {
    short z; // The z value
    short num_neighbours; // The # of neighbours (not always right, only when we need it)
    bool is_dead; // Is it a dead or an alive node?
    bool operator<(const struct node& rhs) const
    {
        return z < rhs.z;
    }
};

void mpi_print(int id, int rank, const char* op, const char* fmt, ...)
{
    if (id == -5) {
        const char* COLORS[] = {
            "", "\033[33m" //YELLOW
            ,
            "\033[34m" //BLUE
            ,
            "\033[35m" //PURPLE
            ,
            "\033[32m" //GREEN
            ,
            "\033[36m" //CYAN
            ,
            "\033[31m" //RED
            ,
            "\033[30m" //GREY
        };
        const char* NO_COLOR = "\033[0m";

        printf("%s[%d: %s]%s ", COLORS[(id % 7) + 1], id, op, NO_COLOR);

        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        fflush(stdout);
    }
}

inline int highest_power_2(int n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

void print_node(struct node* n)
{
    printf("{z: %hd, num_nei: %hd, is_dead: %s}\n", n->z, n->num_neighbours, n->is_dead ? "true" : "false");
}

typedef struct da_struct {
    size_t initial_size;

    void* data;
    size_t used;
    size_t size;
    size_t step; // Size of an element
} dynamic_array;

void da_init(dynamic_array* da, size_t initial_size)
{
    da->initial_size = highest_power_2(initial_size);
    da->step = sizeof(struct node);
    da->used = 0;

    da->size = da->initial_size;
    da->data = realloc(NULL, da->initial_size * da->step);
    assert(da->data);
}

dynamic_array da_make(size_t initial_size)
{
    dynamic_array da;
    da_init(&da, MAX(4, initial_size));
    return da;
}

dynamic_array* da_make_ptr(size_t initial_size)
{
    dynamic_array* da = (dynamic_array*)malloc(sizeof(dynamic_array));
    da_init(da, MAX(4, initial_size));
    return da;
}

void da_resize(dynamic_array* da, size_t new_size)
{
    assert(da);

    da->size = highest_power_2(new_size);
    da->data = realloc(da->data, da->size * da->step);
    assert(da->data);

    // printf("Reallocating to size %d\n", da->size);
}

inline void da_contract(dynamic_array* da)
{
    if (da->size > da->initial_size && da->used <= da->size / 4) {
        da_resize(da, da->size / 2);
    }
}

void da_insert(dynamic_array* da, struct node* to_insert)
{
    assert(da);
    // printf("Inserting in da '%p'\n", da);

    if (da->used == da->size) {
        da_resize(da, da->size * 2);
    }

    struct node* dest = ((struct node*)da->data) + da->used;
    memcpy(dest, to_insert, da->step);
    da->used++;
}

void da_delete_at(dynamic_array* da, size_t i)
{
    assert(da);

    // printf("Deleting in da '%p' at: %lu\n", da, i);
    if (i >= da->used) { // i < 0  is always false, cause unsigned
        printf("Invalid delete: index smaller or larger than the array size!\n");
        return;
    }

    da->used--;

    struct node* dest = ((struct node*)da->data) + i;
    struct node* src = ((struct node*)da->data) + da->used;
    memcpy(dest, src, da->step);

    da_contract(da);
}

void da_free(dynamic_array* da)
{
    assert(da);

    if (da->data) {
        free(da->data);
        da->data = NULL;
        da->used = 0;
        da->size = 0;
    }
}

void da_print(dynamic_array* da)
{
    assert(da);

    printf("************************\n");
    printf("size: %lu\n", da->size);
    printf("used: %lu\n", da->used);
    printf("data:\n");

    size_t i;
    for (i = 0; i < da->used; i++) {
        struct node* ptr = ((struct node*)da->data) + i;
        printf("  ");
        print_node(ptr);
    }

    printf("************************\n");
}

int da_empty(dynamic_array* da)
{
    assert(da);

    return da->used;
}

void da_clear(dynamic_array* da)
{
    assert(da);

    da->used = 0;
    da_resize(da, da->initial_size);
}

int da_find_z(dynamic_array* da, short test_z)
{
    assert(da);

    // printf("\n\nStarting loop with da '%p'. Used = %lu; Data: %p\n", da, da->used, da->data);
    fflush(stdout);

    for (size_t i = 0; i < da->used; i++) {
        struct node* ptr = ((struct node*)da->data) + i;

        if (ptr->z == test_z) {
            return i;
        }
    }
    return -1;
}

// ############################################################
// ######################### MATRIX ###########################
// ############################################################
struct matrix_struct {
    short side;
    dynamic_array** data;
};
typedef matrix_struct Matrix;
void matrix_print(Matrix* m);

// Returns a initialized matrix
inline Matrix make_matrix(short side)
{
    Matrix m;
    m.side = side;
    m.data = (dynamic_array**)calloc(side * side, sizeof(dynamic_array*));
    return m;
}

// Returns the head of the matrix in the pos\ition x and y. Can be NULL.
inline dynamic_array* matrix_get(Matrix* m, short x, short y)
{
    return m->data[x + (y * m->side)];
}

// Returns the element with the x, y, z passed. NULL if it doesn't exist.
inline struct node* matrix_get_ele(Matrix* m, short x, short y, short z)
{
    dynamic_array* da = matrix_get(m, x, y);
    if (!da) {
        return NULL;
    }

    int pos = da_find_z(da, z);
    if (pos == -1) {
        return NULL;
    } else {
        return ((struct node*)da->data) + pos;
    }
}

// Insert a new element in the matrix (ordered)
inline void matrix_insert(Matrix* m, short x, short y, short z, bool is_dead, short num_nei)
{
    struct node new_el = { z, num_nei, is_dead };

    dynamic_array* da = matrix_get(m, x, y);
    if (da == NULL) {
        da = da_make_ptr(4);
        m->data[x + (y * m->side)] = da;
    }
    da_insert(da, &new_el);
}

// Remove from the matrix. We need to search for the z in the linked list
inline void matrix_remove(Matrix* m, short x, short y, short z)
{
    dynamic_array* da = matrix_get(m, x, y);
    if (!da) {
        return;
    }
    int pos = da_find_z(da, z);
    //printf("Trying to delete %hd %hd %hd\n", x, y, z);
    if (pos != -1) {
        da_delete_at(da, pos);
    }
}

void matrix_free(Matrix* m)
{
    for (int i = 0; i < m->side; i++) {
        for (int j = 0; j < m->side; j++) {
            dynamic_array* da = matrix_get(m, i, j);
            if (!da) {
                continue;
            }
            da_free(da);
            free(da);
            da = NULL;
        }
    }
    free(m->data);
}

// Print the live nodes in the matrix (the matrix only contains alive nodes at this point, so print all nodes)
void matrix_print_live(Matrix* m, int from = 0, int to = -1)
{
    short SIZE = m->side;
    if (to == -1) {
        to = SIZE - 1;
    }
    for (short x = from; x <= to; x++) {
        for (short y = 0; y < SIZE; y++) {
            dynamic_array* da = matrix_get(m, x, y);
            if (da != NULL) {
                struct node* ptr = ((struct node*)da->data);
                // Kinda sucks to sort here, but oh well
                std::sort(ptr, (ptr + da->used));
                for (size_t k = 0; k < da->used; k++) {
                    printf("%hd %hd %hd\n", x, y, ptr->z);
                    ptr++;
                }
            }
        }
    }
}

void get_z_lengths(Matrix* m, int x, int* z_lengths)
{
    for (int y = 0; y < m->side; y++) { // "Calculate" the length of each z_list
        dynamic_array* da = matrix_get(m, x, y);
        z_lengths[y] = (da == NULL) ? 0 : da->used;
    }
}

int get_row_length(Matrix* m, int x)
{
    int total = 0;
    for (int y = 0; y < m->side; y++) {
        dynamic_array* da = matrix_get(m, x, y);
        total += (da == NULL) ? 0 : da->used;
    }
    return total;
}

void matrix_print(Matrix* m)
{
    short SIZE = m->side;
    for (short i = 0; i < SIZE; i++) {
        for (short j = 0; j < SIZE; j++) {
            dynamic_array* da = matrix_get(m, i, j);
            if (da != NULL) {
                struct node* ptr = ((struct node*)da->data);
                printf("(%hd, %hd): [", i, j);
                for (size_t k = 0; k < da->used; k++) {
                    printf("%hd%c", ptr->z, k == da->used - 1 ? '\x07' : ',');
                    ptr++;
                }
                printf("] (size: %lu; used: %lu)\n", da->size, da->used);
            } else {
                printf("(%hd, %hd): []\n", i, j);
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

/**
    * m  -> the source matrix
    * x  -> row to send
    * to -> who are we sending to
*/
// @TODO: Try with Isend
// @NOTE: Don't bother with optimizing this. The ones that run on every iteration are the important ones.
void init_send_row(Matrix* m, int x, int to)
{
    int SIZE = m->side;
    int z_lengths[SIZE];
    get_z_lengths(m, x, z_lengths);

    MPI_Send(z_lengths, SIZE, MPI_INT, to, TAG_Z_LENGTHS, MPI_COMM_WORLD);

    // Send each z_list in the row
    for (int y = 0; y < SIZE; y++) {
        dynamic_array* da = matrix_get(m, x, y);
        if (da == NULL || da->used == 0) { // If we have nothing to send, don't send at all
            continue;
        }
        MPI_Send(da->data, (z_lengths[y] * da->step), MPI_BYTE, to, TAG_INIT_MATRIX, MPI_COMM_WORLD);
    }
}

// @NOTE: Don't bother with optimizing this. The ones that run on every iteration are the important ones.
void init_recv_row(Matrix* m, int x, int owner = 0)
{
    int SIZE = m->side;
    int z_lengths[SIZE];
    MPI_Recv(z_lengths, SIZE, MPI_INT, owner, TAG_Z_LENGTHS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    for (int y = 0; y < SIZE; y++) {
        // printf("    [RV: %d] (%d, %d) -> %d\n", id, x, y, z_lengths[y]);
        // fflush(stdout);
        dynamic_array* da = matrix_get(m, x, y);
        if (z_lengths[y] == 0) {
            if (da) {
                da->used = 0;
            }
            continue;
        }

        if (da == NULL) {
            da = da_make_ptr(z_lengths[y]);
            m->data[x + (y * SIZE)] = da;
        } else {
            da_resize(da, z_lengths[y]);
        }

        da->used = z_lengths[y];
        MPI_Recv(da->data, (da->used * da->step), MPI_BYTE, owner, TAG_INIT_MATRIX, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}


void swap_rows_less_sends_version(Matrix* m, int x_have, int x_want, int to, int tmp_id)
{
    //mpi_print(tmp_id, 0, "--%d-->[%d]\n", x_have, to);
    mpi_print(tmp_id, 0, "SWAP_ROWS", "%d--%d with %d.\n", x_have, x_want, to);
    MPI_Request request;
    MPI_Status status;
    int SIZE = m->side;

    // Recv asynchronously their lengths
    int their_z_lengths[SIZE];
    MPI_Irecv(their_z_lengths, SIZE, MPI_INT, to, TAG_Z_LENGTHS, MPI_COMM_WORLD, &request);

    // Compute and send our lengths
    int my_z_lengths[SIZE];
    get_z_lengths(m, x_have, my_z_lengths);
    MPI_Send(my_z_lengths, SIZE, MPI_INT, to, TAG_Z_LENGTHS, MPI_COMM_WORLD);

    // Wait for completion of the recv
    MPI_Wait(&request, &status);
    //mpi_print(tmp_id, 1, "FINISHED LENGTH SWAP", "%d--%d with %d.\n", x_have, x_want, to);

    MPI_Request request_read;
    MPI_Status status_read;

    // Receive (asynchronously)
    size_t total_size_to_recv = 0;
    for (int y = 0; y < SIZE; y++) {
        total_size_to_recv += (their_z_lengths[y] * sizeof(struct node));
    }

    char* recv_data = NULL;
    if (total_size_to_recv > 0) {
        recv_data = (char*)malloc(total_size_to_recv);
        MPI_Irecv(recv_data, total_size_to_recv, MPI_BYTE, to, TAG_SWAP_ROWS, MPI_COMM_WORLD, &request_read);
    }

    // Init data to send, and send it
    size_t total_size_to_send = 0;
    for (int y = 0; y < SIZE; y++) {
        total_size_to_send += (my_z_lengths[y] * sizeof(struct node));
    }
    char* send_data = NULL;
    int current_pos = 0;
    if (total_size_to_send > 0) {
        send_data = (char*)malloc(total_size_to_send);

        for (int y = 0; y < SIZE; y++) {
            dynamic_array* da = matrix_get(m, x_have, y);
            if (da == NULL || da->used == 0) { // If we have nothing to send, don't send at all
                continue;
            }

            memcpy((send_data + current_pos), da->data, (my_z_lengths[y] * da->step));
            current_pos += (my_z_lengths[y] * da->step);
        }

        MPI_Send(send_data, total_size_to_send, MPI_BYTE, to, TAG_SWAP_ROWS, MPI_COMM_WORLD);
    }

    // Wait until the recv finishes
    if (total_size_to_recv > 0) {
        MPI_Wait(&request_read, &status_read);
    }

    // Insert the received data in the arrays
    current_pos = 0;
    for (int y = 0; y < SIZE; y++) {
        dynamic_array* da = matrix_get(m, x_want, y);
        if (their_z_lengths[y] == 0) {
            if (da) {
                da->used = 0;
            }
            continue;
        }

        if (da == NULL) {
            da = da_make_ptr(their_z_lengths[y]);
            m->data[x_want + (y * SIZE)] = da;
        } else {
            da_resize(da, their_z_lengths[y]);
        }

        da->used = their_z_lengths[y];
        // mpi_print(tmp_id, 1, "WAITING FOR", "%d %d with %d\n", x_want, y, to);
        memcpy(da->data, (recv_data + current_pos), (da->used * da->step));
        current_pos += (da->used * da->step);
    }
}


void swap_rows(Matrix* m, int x_have, int x_want, int to, int tmp_id)
{
    MPI_Request request;
    MPI_Status status;
    int SIZE = m->side;

    // Recv asynchronously their lengths
    int their_z_lengths[SIZE];
    MPI_Irecv(their_z_lengths, SIZE, MPI_INT, to, TAG_Z_LENGTHS, MPI_COMM_WORLD, &request);

    // Compute and send our lengths
    int my_z_lengths[SIZE];
    get_z_lengths(m, x_have, my_z_lengths);
    MPI_Send(my_z_lengths, SIZE, MPI_INT, to, TAG_Z_LENGTHS, MPI_COMM_WORLD);

    // Wait for completion of the recv
    MPI_Wait(&request, &status);

    MPI_Request requests[SIZE];
    MPI_Status statuses[SIZE];
    int request_i = 0; // Ends up as the total amount of requests (sends with 0 size are skiped, that why)

    // Assynchronously receive all z_lists
    for (int y = 0; y < SIZE; y++) {
        dynamic_array* da = matrix_get(m, x_want, y);
        if (their_z_lengths[y] == 0) {
            if(da) {
                da->used = 0;
            }
            continue;
        }

        if (da == NULL) {
            da = da_make_ptr(their_z_lengths[y]);
            m->data[x_want + (y * SIZE)] = da;
        } else {
            da_resize(da, their_z_lengths[y]);
        }

        da->used = their_z_lengths[y];
        MPI_Irecv(da->data, (da->used * da->step), MPI_BYTE, to, TAG_SWAP_ROWS, MPI_COMM_WORLD, &requests[request_i]);
        request_i++;
    }

    // Send each z_list in the row
    for (int y = 0; y < SIZE; y++) {
        dynamic_array* da = matrix_get(m, x_have, y);
        if (da == NULL || da->used == 0) { // If we have nothing to send, don't send at all
            continue;
        }

        MPI_Send(da->data, my_z_lengths[y] * da->step, MPI_BYTE, to, TAG_SWAP_ROWS, MPI_COMM_WORLD);
    }

    // If we sent any array at all (aka the z's were not all empty)
    if (request_i != 0) {
        MPI_Waitall(request_i, requests, statuses);
    }
}

int main(int argc, char* argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);

    int id, p;
    double elapsed_time, scatter_time, run_time, gather_time;

    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    Matrix m;
    int generations;
    short SIZE;
    char* input_file;

    if (id == 0) {
        if (argc != 3) {
            printf("[ERROR] Incorrect usage!\n");
            printf("[Usage] %s <input_file> <nr_generations>\n", argv[0]);
            return -1;
        }
        input_file = argv[1];
        generations = atoi(argv[2]);

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

        if (fscanf(fp, "%hd", &SIZE) == EOF) {
            printf("[ERROR] Unable to read the size.\n");
            return -1;
        }

        // Finished parsing metadata. Now only need to parse the actual positions
        Matrix aux = make_matrix(SIZE);
        short x, y, z;
        while (fscanf(fp, "%hd %hd %hd", &x, &y, &z) != EOF) {
            matrix_insert(&aux, x, y, z, false, -1);
        }
        // Finished parsing!
        fclose(fp);

        // ==================================
        // === WE NOW START TO SEND STUFF ===
        // ==================================
        // Send generations and size
        int buf[2];
        buf[0] = (int)SIZE;
        buf[1] = generations;
        MPI_Bcast(buf, 2, MPI_INT, 0, MPI_COMM_WORLD);

        m = make_matrix(SIZE);

        // Send rows
        for (int x = 0; x < SIZE; x++) {
            int owner = BLOCK_OWNER(x, p, SIZE);

            if (owner != 0) {
                init_send_row(&aux, x, owner);
            } else {
                // If we are the process 0, just setup the matrix, don't send anything
                for (int y = 0; y < SIZE; y++) {
                    dynamic_array* da = matrix_get(&aux, x, y);
                    if (da == NULL) {
                        continue;
                    }

                    dynamic_array* new_da = da_make_ptr(da->used);
                    m.data[x + (y * m.side)] = new_da;

                    new_da->used = da->used;
                    memcpy(new_da->data, da->data, da->used * new_da->step);
                }
            }
        }

        matrix_free(&aux);

    } else {
        // Receive SIZE and generations
        int buf[2];
        MPI_Bcast(buf, 2, MPI_INT, 0, MPI_COMM_WORLD);
        SIZE = (short)buf[0];
        generations = buf[1];

        // Recv my rows
        m = make_matrix(SIZE);
        for (int x = BLOCK_LOW(id, p, SIZE); x <= BLOCK_HIGH(id, p, SIZE); x++) {
            init_recv_row(&m, x);
        }
    }

    int MY_HIGH = BLOCK_HIGH(id, p, SIZE);
    int MY_LOW = BLOCK_LOW(id, p, SIZE);

    // Get the frontier rows
    int MY_HIGH_FRONTIER = pos_mod(MY_HIGH + 1, SIZE);
    int MY_LOW_FRONTIER = pos_mod(MY_LOW - 1, SIZE);

    int MY_HIGH_FRONTIER_OWNER = BLOCK_OWNER(MY_HIGH_FRONTIER, p, SIZE);
    int MY_LOW_FRONTIER_OWNER = BLOCK_OWNER(MY_LOW_FRONTIER, p, SIZE);

    MPI_Barrier(MPI_COMM_WORLD);
    scatter_time = elapsed_time + MPI_Wtime();

    //-----------------
    //--- MAIN LOOP ---
    //-----------------
    for (int gen = 0; gen < generations; gen++) {
        dynamic_array* da;
        struct node *ptr, *to_test;
        int32_t j;
        short z, _z, _y, _x, y, x;

        // @SEE: (Maybe failes if processor numbers are odd (impar))
        // The ideia here is to swap with the lower block and then with to top one if we are an odd processor
        // or viceversa if we are even.
        if (id % 2 == 0) {
            swap_rows(&m, MY_HIGH, MY_HIGH_FRONTIER, MY_HIGH_FRONTIER_OWNER, id);
            swap_rows(&m, MY_LOW, MY_LOW_FRONTIER, MY_LOW_FRONTIER_OWNER, id);
        } else {
            swap_rows(&m, MY_LOW, MY_LOW_FRONTIER, MY_LOW_FRONTIER_OWNER, id);
            swap_rows(&m, MY_HIGH, MY_HIGH_FRONTIER, MY_HIGH_FRONTIER_OWNER, id);
        }

        for (int _a = 0, i = MY_LOW_FRONTIER; _a < BLOCK_SIZE(id, p, SIZE) + 2; _a++) {
            for (j = 0; j < SIZE; j++) {
                da = matrix_get(&m, i, j);
                if (!da) {
                    continue;
                }

                size_t limit = da->used;
                // Iterate over every existing z for x and y
                for (size_t k = 0; k < limit; k++) {
                    ptr = ((struct node*)da->data) + k;

                    // If its dead skip
                    if (ptr->is_dead) {
                        continue;
                    }

                    // PROCESS AN ALIVE NODE
                    x = i;
                    y = j;
                    z = ptr->z;
                    ptr->num_neighbours = 0;

                    _z = pos_mod(z + 1, SIZE);
                    to_test = matrix_get_ele(&m, x, y, _z);
                    if (to_test) {
                        if (to_test->is_dead == true) {
                            to_test->num_neighbours++;
                        } else {
                            ptr->num_neighbours++;
                        }
                    } else {
                        matrix_insert(&m, x, y, _z, true, 1);
                        ptr = ((struct node*)da->data) + k;
                    }

                    _z = pos_mod(z - 1, SIZE);
                    to_test = matrix_get_ele(&m, x, y, _z);
                    if (to_test) {
                        if (to_test->is_dead == true) {
                            to_test->num_neighbours++;
                        } else {
                            ptr->num_neighbours++;
                        }
                    } else {
                        matrix_insert(&m, x, y, _z, true, 1);
                        ptr = ((struct node*)da->data) + k;
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
                        ptr = ((struct node*)da->data) + k;
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
                        ptr = ((struct node*)da->data) + k;
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
                        ptr = ((struct node*)da->data) + k;
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
                        ptr = ((struct node*)da->data) + k;
                    }
                }
            }
            i = pos_mod(++i, SIZE);
        }

        for (int _a = 0, i = MY_LOW_FRONTIER; _a < BLOCK_SIZE(id, p, SIZE) + 2; _a++) {
            for (j = 0; j < SIZE; j++) {
                da = matrix_get(&m, i, j);
                if (!da) {
                    continue;
                }

                // Iterate over every existing z for x and y
                for (int k = (int)da->used - 1; k >= 0; k--) {
                    ptr = ((struct node*)da->data) + k;
                    if (ptr->is_dead) {
                        if (ptr->num_neighbours == 2 || ptr->num_neighbours == 3) {
                            ptr->is_dead = false;
                        } else {
                            matrix_remove(&m, i, j, ptr->z);
                            ptr = ((struct node*)da->data) + k;
                        }
                    } else {
                        if (ptr->num_neighbours < 2 || ptr->num_neighbours > 4) {
                            matrix_remove(&m, i, j, ptr->z);
                            ptr = ((struct node*)da->data) + k;
                        }
                    }
                }
            }
            i = pos_mod(++i, SIZE);
        }
    }

    run_time = elapsed_time + MPI_Wtime() - scatter_time;

    // Gather the results
    if (id == 0) {
        for (int x = 0; x < SIZE; x++) {
            int owner = BLOCK_OWNER(x, p, SIZE);
            if (owner != 0) {
                init_recv_row(&m, x, owner);
            }
        }
        matrix_print_live(&m);

        gather_time = elapsed_time + MPI_Wtime() - scatter_time - run_time;

        // Write the time log to a file
        FILE* out_fp = fopen("time.log", "w");
        char out_str[120];
        sprintf(out_str, "MPI %s: \nscatter_time: %lf \n    run_time: %lf\n gather_time: %lf\n   total_time: %lf", input_file, scatter_time, run_time, gather_time, (scatter_time + run_time + gather_time));
        fwrite(out_str, strlen(out_str), 1, out_fp);

        // printf("scatter_time: %lf\n", scatter_time);
        // printf("    run_time: %lf\n", run_time);
        // printf(" gather_time: %lf\n", gather_time);
        // printf("----------------------\n", gather_time);
        // printf("  total_time: %lf\n", (scatter_time + run_time + gather_time));
    } else {
        for (int x = MY_LOW; x <= MY_HIGH; x++) {
            init_send_row(&m, x, 0);
        }
    }

    // matrix_free(&m);
    MPI_Finalize();
    return 0;
}
