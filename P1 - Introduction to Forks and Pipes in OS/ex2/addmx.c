#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#define LINE_SIZE 8192

int MEMSIZE = 0;

int* shared_mem(size_t size) {
  int protection = PROT_READ | PROT_WRITE;

  int visibility = MAP_SHARED | MAP_ANONYMOUS;

  return mmap(NULL, size, protection, visibility, -1, 0);
}

int **matrix_alloc(int rows, int cols)
{
    /* Allocate array of row pointers */
    int ** m = malloc(rows * sizeof(int*));
    if (!m) return NULL;

    /* Allocate block for data */
    m[0] = malloc(rows * cols * sizeof(int));
    if (!m[0]) {
        free(m);
        return NULL;
    }

    /* Assign row pointers */
    for(int r = 1; r < rows; r++) {
        m[r] = m[r-1]+cols;
    }

    return m; 
}
void matrix_free( int** m )
{
    if (m) free(m[0]);
    free(m);
}

void print_matrix(int *m,int row,int col){
    printf("%dx%d",row,col);
    printf("\n");
    for (int r=0; r<row; r++){
        for(int c =0; c<col; c++)
        {
            printf("%d     ",m[(col*r)+c]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: addmx \"input matrix filename\"  \"input matrix filename\" \n");
        return EXIT_FAILURE;
    }

    char * line1 = NULL;
    char * line2 = NULL;
    size_t len = LINE_SIZE;
    FILE * m1;
    FILE * m2;

    m1 = fopen(argv[1],"r");
    m2 = fopen(argv[2],"r");

    if (m1 == NULL){
        printf("Unable to open first file");
        exit(EXIT_FAILURE);
    }
    if (m2 == NULL){
        printf("Unable to open second file");
        exit(EXIT_FAILURE);
    }
        

    getline(&line1, &len, m1);
    getline(&line2, &len, m2);
    
    char *n = strtok(line1,"x");
    char *m = strtok(NULL,"");

    int row = atoi(n);
    int col = atoi(m);

    char *var1 = strtok(line2,"x");
    char *var2 = strtok(NULL,"");

    if(row != atoi(var1) || col != atoi(var2)){
        printf("Matrix sizes are diferent\n");
        exit(EXIT_FAILURE);
    }

    MEMSIZE = row*col*sizeof(int);

    int **matrix1 = matrix_alloc(row,col);
    int **matrix2 = matrix_alloc(row,col);
    int *matrixAnswer = malloc(MEMSIZE);

    char *token1;
    char *token2;
    
    for(int i = 0; i < row; i++) {
        getline(&line1, &len, m1);
        getline(&line2, &len, m2);

        matrix1[i][0] = atoi(strtok_r(line1," ",&token1));
        matrix2[i][0] = atoi(strtok_r(line2," ",&token2));

        for(int j = 1; j < col; j++) {
            matrix1[i][j] = atoi(strtok_r(NULL," ",&token1));
            matrix2[i][j] = atoi(strtok_r(NULL," ",&token2));
        }
        
    }

    int* sh_mem = shared_mem(MEMSIZE);
    memcpy(sh_mem,matrixAnswer,MEMSIZE);

    pid_t pid, wpid;
    int status = 0;

    for(int i =0 ; i < col; i++) {

        if ((pid = fork()) < 0) {
            perror("fork error");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
/* child */
            for(int c = 0; c < row; c++) {
                sh_mem[(col*c)+i] = matrix1[c][i] + matrix2[c][i];
            }

            exit(EXIT_SUCCESS);
        }

    }
    while ((wpid = wait(&status)) > 0);

    print_matrix(sh_mem,row,col);

    matrix_free(matrix1);
    matrix_free(matrix2);
    free(matrixAnswer);

    return EXIT_SUCCESS;
}