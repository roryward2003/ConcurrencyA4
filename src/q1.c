#include <omp.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

void printSquareMatrix(int **mat, int n);
void printArray(int *arr, int n);

int main(int argc, char *argv[]) {
    
    // Parse parameters and seed the rng
    if (argc < 4) {
        printf("ERROR: 3 parameters expected\n");
        return -1;
    }
    int n = atoi(argv[1]);
    double p = atof(argv[2]);
    int s = atoi(argv[3]);
    srand(s);

    // Allocate the memory for my matrix
    int **mat = malloc(sizeof(*mat)*n);
    for(int i=0; i<n; i++) {
        mat[i] = malloc(sizeof(int)*n);
    }

    // Omp prep for my 4 core machine
    omp_set_dynamic(0);
    omp_set_num_threads(1);

    // Start timer
    // clock_t beforeInit = clock();

    // Matrix initialisation
#pragma omp parallel for
    for(int i=0; i<n; i++) {
        for(int j=0; j<n; j++)
            mat[i][j] = (double)(((rand() % 100) + 1) / 100.0) > p;
    }

    // Stop timer
    // clock_t afterInit = clock();
    // printf("Matrix init: %.0f ms\n", (double)(afterInit - beforeInit) * 1000 / CLOCKS_PER_SEC);

    // Print out the matrix
    printSquareMatrix(mat, n);

    // Count number of non zeroes (sequentially as specified)
    int numNonZeroes = 0;
    for(int i=0; i<n; i++)
        for(int j=0; j<n; j++)
            if(mat[i][j])
                numNonZeroes++;

    // Allocate memory for our csr arrays
    int *rowptr = malloc(sizeof(int)*n);
    int *cols = malloc(sizeof(int)*numNonZeroes);
    int *values = malloc(sizeof(int)*numNonZeroes);
    
    
    // Start timer
    // clock_t beforeCSR = clock();

    // Populate these array with the correct values
    #pragma omp parallel
    #pragma omp sections
    {
        #pragma omp section // Calculate the rowptr array
        { 
            rowptr[0] = 0;
            for(int i=0; i<n-1; i++) {
                rowptr[i+1] = rowptr[i];
                for(int j=0; j<n; j++) {
                    if(mat[i][j])
                    rowptr[i+1]++;
                }
            }
        }
        #pragma omp section // Calculate the cols array
        { 
            int k=0;
            for(int i=0; i<n; i++) {
                for(int j=0; j<n; j++) {
                    if(mat[i][j])
                    cols[k++] = j;
                }
            }
        }
        #pragma omp section // Calculate the values array
        { 
            int k=0;
            for(int i=0; i<n; i++) {
                for(int j=0; j<n; j++) {
                    if(mat[i][j])
                    values[k++] = mat[i][j];
                }
            }
        }
    }
    
    // Stop timer
    // clock_t afterCSR = clock();
    // printf("CSR conversion: %.0f ms\n", (double)(afterCSR - beforeCSR) * 1000 / CLOCKS_PER_SEC);

    // Print out the array contents
    printArray(rowptr, n);
    printArray(cols, numNonZeroes);
    printArray(values, numNonZeroes);

    return 0;
}

// Print out the values of a square int array of dimension n
void printSquareMatrix(int **mat, int n) {
    for(int i=0; i<n; i++) {
        printArray(mat[i], n);
    }
}

// Print out the values of an int array of dimension n
void printArray(int *arr, int n) {
    if(n==0) { return; }
    int i=0;
    printf("%d", arr[i]);
    for(i=1; i<n; i++) {
        printf(" %d", arr[i]);
    }
    printf("\n");
}