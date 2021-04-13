#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>   // for gettimeofday()
#include <limits.h>
#include <ctype.h>
#include <stdint.h>
#include "mpi.h"


// A function to generate odd sized magic squares

int world_size, rank;

int magicSquareLine(int *a, int size)
{ 
    int n = 0;
    int k = sqrt(size);
    int values[k];
    int index = 0;
    for(int i=0; i <= size;)
    {
      for (int j = i; j<k;j++)
      {
        n += a[j];
      }
      i+=k;
      values[index] = n;
      index++;
    }


    int final = values[0];
    for(int i=0; i<k; i++)
    {
        if(values[i] != final)
        {
          return -2;
        }
    }

  return final;
}


int magicSquareColumn(int *a, int size)
{ 
    int n = 0;
    int k = sqrt(size);
    int values[k];
    int index = 0;
    for(int i=0; i < k; i++)
    {
      for (int j = i; j<size;)
      {
        n += a[j];
        j+=k;
      }
      values[index] = n;
      n=0;
      index++;
    }


    int final = values[0];
    for(int i=0; i<k; i++)
    {
        if(values[i] != final)
        {
          return -1;
        }
    }

  return final;
}


int getDiagonal(int *a, int size)
{
    int n = 0, m=0;
    int k = sqrt(size);
    for(int i=0; i <= size ;)
    {
        n += a[i];
        i+=k+1;
    }

    for(int i=k-1; i<= size-k;)
    {
      m += a[i];
      i+=k-1;
    }

    int final;
    if(m != n)
      final = -5;

    else
      final = n;

  return final;
}

//Retirado do stackOverFlow com alterações realizadas por Rafael Gomes
int getNumber(char *file)
{
  char *p = file;
  int val = 0;

  while (*p) { // While there are more characters to process...
      if ( isdigit(*p) || ( (*p=='-'||*p=='+') && isdigit(*(p+1)) )) {
          // Found a number
          val = strtol(p, &p, 10); // Read number
          //printf("%ld\n", val); // and print it.
      } else {
          // Otherwise, move on to the next character.
          p++;
      }
  }

  return val;
}

int main(int argc, char **argv)
{ 
    double start,end;
     int myRank;
     MPI_Status status;
     MPI_Init(&argc, &argv);
      start = MPI_Wtime();
     MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
     if(myRank == 0)
     {
        char *filename = argv[1];
        FILE *f = fopen(filename,"r");
        int val = getNumber(filename);
        int size = val * val;
        int *d = (int*) malloc(val*val*sizeof(int));
        int x = 0;
        int index = 0;
        while(fscanf(f,"%d",&x) > 0)
        {
        d[index++] = x;
        }
        fclose(f);
        for (int i = 1; i < 3; i++)
        {
            MPI_Send( &size , 1 , MPI_INT , i, 5 , MPI_COMM_WORLD);
        }
        
        MPI_Send(d,size, MPI_INT, 1, 99, MPI_COMM_WORLD);
        MPI_Send(d,size, MPI_INT, 2, 98, MPI_COMM_WORLD);

        int column = magicSquareColumn(d,size);
        MPI_Send(&column,1,MPI_INT, 3, 94, MPI_COMM_WORLD);


    }
     else if(myRank == 1) //diagonal
     {      int size;
            MPI_Recv(&size, 1 , MPI_INT , 0, 5 , MPI_COMM_WORLD ,  &status);
            int *d = (int*) malloc(size*sizeof(int));
             MPI_Recv(d, size, MPI_INT, 0, 99, MPI_COMM_WORLD, &status);
             int di = getDiagonal(d,size);
             MPI_Send(&di, 1,MPI_INT,3,95,MPI_COMM_WORLD);
     }
     else if(myRank == 2)
     {  
            int size;
            MPI_Recv(&size, 1 , MPI_INT , 0, 5 , MPI_COMM_WORLD ,  &status);
            int *d = (int*) malloc(size*sizeof(int));
             MPI_Recv(d, size, MPI_INT, 0, 98, MPI_COMM_WORLD, &status);
            int l = magicSquareLine(d,size);
            MPI_Send(&l, 1,MPI_INT,3,97,MPI_COMM_WORLD);
     }

     else if(myRank == 3)
     {  
        int l,di,c;
        MPI_Recv(&c, 1, MPI_INT, 0, 94, MPI_COMM_WORLD, &status);
        MPI_Recv(&l, 1, MPI_INT, 2, 97, MPI_COMM_WORLD, &status);
        MPI_Recv(&di, 1, MPI_INT, 1, 95, MPI_COMM_WORLD, &status);

        if(l==di && c == l )
            printf("Quadrado Mágico\n");
        if(l!=di && c == l )
            printf("Quadrado Mágico Imperfeito\n");
        if(c != l )
            printf("Não é Quadrado Mágico\n");
             
        
        end = MPI_Wtime();
        double total = end-start;
        printf("Total Seconds: %f\n",total);     
             
             /*MPI_Recv(d, size, MPI_INT, 0, 97, MPI_COMM_WORLD, &status);
         int c = magicSquareColumn(d,size);
        MPI_Send(&c, 1,MPI_INT,0,96,MPI_COMM_WORLD);*/
     }
     MPI_Finalize();
}