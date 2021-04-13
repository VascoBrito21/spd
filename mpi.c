#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>

int world_size, world_rank;
MPI_Status status;

//Autoria StackOverflow
//Modificado por Vasco Brito
long parse_order(char *path){
    long val = 0;
    while (*path) { // While there are more characters to process...
      if (isdigit(*path))
        val = strtol(path, &path, 10); // Read number
      else
        path++;
    }
    return val;
}

//Autoria Professor Pedro Guerreiro
int *ints_new (int n) {
    return (int *) malloc (n * sizeof(int));
}

//Autoria Professor Pedro Guerreiro
int **ints2_new(int rows, int cols)
{
    int **result = (int **) malloc(rows * sizeof(int *));
    int *p = ints_new(rows * cols);
    for (int i = 0; i < rows; i++, p += cols)
        result[i] = p;
    return result;
}

//Autoria Professor Pedro Guerreiro
int ints_get_some(int *a, int n, FILE *f) {
    int result = 0;
    int x;
    while (fscanf(f, "%d", &x) != EOF)
        a[result++] = x;
    return result;
}

//Autoria Professor Pedro Guerreiro
void ints2_get(int **m, int n, FILE *f) {
    ints_get_some(*m, n*n, f);
}

void sumLines(int **a, int n, int *sum) {
    for(int i = 0; i<n; i++){
        sum[i] = 0;
        for (int j = 0; j < n; j++){
            sum[i] += a[i][j];
        }
    }	    
}

void sumCols(int **a, int n, int *sum) { 
	for (int i = 0; i < n; i++){
        sum[i] = 0;
        for(int j=0; j<n; j++){
            sum[i] += a[j][i];
        }
    }
}

int sumDiagonal(int **a, int n) {
    int result = 0;
	for (int i = 0; i < n; i++)
		result += a[i][i];
    return result;
}

int sumDiagonalSec(int **a, int n) {
    int result = 0;
	for (int i = 0; i <n; i++)
		result += a[i][n-i-1];
    return result;
}

bool checkSum(int *vector, int n) {
    for(int i = 1; i<n ; i++){
        if(vector[i] != vector[i-1])
            return false;
    }
    return true;
}

void checkSquare(int **a, int n, int *vLines, int *vCols){
    
    if(checkSum(vLines,n) && checkSum(vCols,n) && vCols[0] == vLines[0] && sumDiagonalSec(a,n) == sumDiagonal(a,n))
        printf("Quadrado Magico\n");
    else if(checkSum(vLines,n) && checkSum(vCols,n) && vCols[0] == vLines[0] && sumDiagonal(a,n) != sumDiagonalSec(a,n))
        printf("Quadrado Magico Imperfeito\n");
    else
        printf("Nao e um Quadrado Magico\n");
}

void magic_square(char *filename) {
    FILE *f = fopen(filename, "r");
    int n = parse_order(filename);
    int size = n*n;
    int vLines[n];
    int vCols[n];
    int **m = ints2_new(n,n);

    if(world_rank == 0) {
        ints2_get(m,n,f);
        MPI_Send(&m[0][0], size, MPI_INT, 1, 99, MPI_COMM_WORLD);
        MPI_Send(&m[0][0], size, MPI_INT, 2, 99, MPI_COMM_WORLD);
        MPI_Send(&m[0][0], size, MPI_INT, 3, 99, MPI_COMM_WORLD);
    }

    else if(world_rank == 1) {
        MPI_Recv(&m[0][0], size, MPI_INT, 0, 99, MPI_COMM_WORLD, &status);
        sumLines(m,n,vLines);
        MPI_Send(vLines, n, MPI_INT, 3, 98, MPI_COMM_WORLD);
    }

    else if(world_rank == 2) {
        MPI_Recv(&m[0][0], size, MPI_INT, 0, 99, MPI_COMM_WORLD, &status);
        sumCols(m,n,vCols);
        MPI_Send(vCols, n, MPI_INT, 3, 97, MPI_COMM_WORLD);
    }

    else if(world_rank == 3) {
        MPI_Recv(&m[0][0], size, MPI_INT, 0, 99, MPI_COMM_WORLD, &status);
        MPI_Recv(vLines, n, MPI_INT, 1, 98, MPI_COMM_WORLD, &status);
        MPI_Recv(vCols, n, MPI_INT, 2, 97, MPI_COMM_WORLD, &status);
        checkSquare(m,n,vLines,vCols);
    }
}

int main(int argc, char** argv) {
    char *filename = argv[1];

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


    MPI_Barrier(MPI_COMM_WORLD);
    float start = MPI_Wtime();
    
    magic_square(filename);
   
    MPI_Barrier(MPI_COMM_WORLD);
    float end = MPI_Wtime();
    
    if (world_rank == 0) 
        printf("%f\n", end-start);

    // Finalize the MPI environment.
    MPI_Finalize();
}