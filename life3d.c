#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 64

int main(int argc, char** argv) {
    //printf("%d", atoi(NULL));
    if(argc != 3) {
        printf("[ERROR] Incorrect usage!\n");
        printf("[Usage] ./life3d <input_file> <nr_generations>\n");
        return -1;
    }
    char *input_file  = argv[1];
    int generations = atoi(argv[2]);

    if (generations <= 0) {
        printf("[ERROR] Number of generations must be bigger that 0. Got: '%d'\n", generations);
        return -1;
    }

    FILE *f = fopen(input_file, "r");
    if (f == NULL) {
        printf("[ERROR] Unable to read the input file.\n", generations);
        perror("[ERROR]");
        return -1;
    }

    char line[MAX_LINE_SIZE];
    int size;

    // Get size
    if (fgets(line, MAX_LINE_SIZE, f) == NULL) {
        printf("[ERROR] Unable to read size from the input file.\n", generations);
        return -1;
    }
    size = atoi(line);
    printf("size: %d\n", size);

    char *_x, *_y, *_z;
    int x, y, z;
    while(fgets(line, MAX_LINE_SIZE, f) != NULL) {
        char *aux = line;
        // printf("Got: %s", line);
        _x = strtok(aux , " ");
        _y = strtok(NULL, " ");
        _z = strtok(NULL, "\n");
        if(!_x || !_y || !_z) {
            printf("%p, %p, %p\n", _x, _y, _z);
            printf("[ERROR] Found an error parsing the input file.\n", generations);
            return -1;
        }
        x = atoi(_x);
        y = atoi(_y);
        z = atoi(_z);

        printf("(%d, %d, %d)\n", x, y, z);
    }

    fclose(f);

    printf("Got: %s, %d\n", input_file, generations);
    return 0;
}