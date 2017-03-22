#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define conv(v,n) v + n

/********************/
/*** STRUCTURES ***/
/********************/

// Node representing elements of sparse matrix
struct node {
	int z;
	struct node *next, *prev;
};

typedef struct node *z_list;

/********************/
/*** FUNCTIONS ***/
/********************/

/**	Initialize hashmap	**/
void STinit(int max, z_list matrix[][max]){
	int i,j;
	
	for(i=0; i < max;i++)
			for(j=0; j < max;j++)
				matrix[i][j] = NULL;
}

/**  Add alive cell coordinates in hashmap  **/
	// Inserted by ordered to maintain sparse matrix
void STinsert(int n, z_list matrix[][n], int x, int y , int z){
	int i,j;
	
	z_list ptr;
	z_list new_el;
	
	ptr = matrix[x][y];
	new_el = (z_list) malloc(sizeof(z_list));
	new_el->z = z;
	new_el->next = NULL;
	new_el->prev = NULL;
	
	if(matrix[x][y] == NULL){
		matrix[x][y] = new_el;
		printf("AQUI111: %d %d %d\n", x , y, matrix[x][y]->z);
	}
		
	else{
		if(z < ptr->z){
			//matrix[x][y]->prev = new_el;
			matrix[x][y]->prev = new_el;
		    new_el->next = matrix[x][y];
            matrix[x][y]= new_el;
            return;
		}
		else{
			while(ptr->next != NULL){
				if(z < ptr->next->z){
					new_el->next = ptr->next;
					new_el->prev = ptr;
					ptr->next = new_el;
					printf("PUMBAS: %d %d %d\n", x , y, matrix[x][y]->z);
					return;
				}
				else
					ptr = ptr->next;
				}
				printf("INSERE NO ULTIMO\n");
				ptr->next = new_el;
				new_el->prev = ptr;
			}
		}
		printf("FIM\n");
	
	return;
}

int main(int argc, char *argv[]){
    FILE *fp;
	int iters, SIZE, N, i, j, x, y, z;
  	z_list ptr;
    
	 fp = fopen(argv[1],"r");
	 iters = atoi(argv[2]);
	 fscanf(fp,"%d", &N);
	 printf("%d iters: %d\n", N, iters);
	 SIZE = N;
	 N=N*2+1;
	 
	 z_list cell_matrix[N][N];
	 
	 STinit(N,cell_matrix);
		 
	 while(fscanf(fp,"%d %d %d", &x, &y, &z) != EOF){
		 printf("%d %d %d\n", conv(x,SIZE),conv(y,SIZE),conv(z,SIZE));
		 STinsert(N,cell_matrix,conv(x,SIZE),conv(y,SIZE),conv(z,SIZE));
	 }		 
	 fclose(fp);


    for(i=0;i < N;i++)
        for(j=0;j< N;j++){
            ptr = cell_matrix[i][j];
            while(ptr != NULL){
                printf("x y z : %d %d %d\n", i,j,ptr->z - SIZE);
                ptr = ptr->next;
            }
        }
	return 0;
}
