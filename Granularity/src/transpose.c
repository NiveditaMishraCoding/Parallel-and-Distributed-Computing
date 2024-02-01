
#include "util_common.h"
#include "transpose.h" 
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "limits.h"

// Helper function to get a pointer to a matrix element
static double *at(const Mat* mat, unsigned int i, unsigned int j) {
    return mat->ptr + (i * mat->n + j);
}

// Function to transpose a square matrix in-place sequentially
void mat_squaretransp_sequential(Mat *mat){
    assert(mat->m == mat->n); // Ensure the matrix is square
    for (unsigned int i = 0; i < mat->n - 1; i++) {
        for (unsigned int j = i + 1; j < mat->n; j++) {
            // Swap elements across the diagonal
            double temp = *at(mat, i, j);
            *at(mat, i, j) = *at(mat, j, i);
            *at(mat, j, i) = temp;
        }
    }
}

// Function to calculate the transposed index of a linear index in a matrix
static unsigned int transpose_index(unsigned int index, unsigned int n) {
    unsigned int i = index / n;
    unsigned int j = index % n;
    return j * n + i;
}

// Structure to hold arguments for the parallel transposition threads
typedef struct {
    pthread_mutex_t *pmtx;
    unsigned int *shrd_index;
    unsigned int *max_index;
    unsigned int *grain;
    Mat *mat;
} Args;

// Function to get the next index in row-major order
static unsigned int next_index(unsigned int index, unsigned int n) {
    if (index == UINT_MAX) return UINT_MAX; // Check for overflow
    unsigned int i = index / n;
    unsigned int j = index % n;
    // Move to the next column or wrap to the next row
    if (j + 1 < n) return index + 1;
    if (i + 1 < n - 1) return (i + 1) * n + i + 2;
    return UINT_MAX; // No more valid indices
}

// Thread function to transpose parts of the matrix in parallel
void *mat_squaretransp_thrd(void *args) {
    Args *a = (Args *) args;
    unsigned int index;
    do {
        // Lock the shared index to avoid race conditions
        pthread_mutex_lock(a->pmtx);
        index = *a->shrd_index;
        // Update the shared index for the next thread
        for (unsigned int i = 0; i < *a->grain; i++) *a->shrd_index = next_index(*a->shrd_index, a->mat->n);
        pthread_mutex_unlock(a->pmtx);
        
        double temp;
        unsigned int index_transposed;
        unsigned int count = 0;
        // Transpose the assigned elements
        while (count < *a->grain && index <= *a->max_index) {
            temp = a->mat->ptr[index];
            index_transposed = transpose_index(index, a->mat->n);
            a->mat->ptr[index] = a->mat->ptr[index_transposed];
            a->mat->ptr[index_transposed] = temp;
            count++;
            index = next_index(index, a->mat->n);
        }
    } while (index <= *a->max_index); // Continue until all elements are processed
    pthread_exit(NULL); // Exit the thread
}

// Function to initialize the parallel transposition of a matrix
void mat_squaretransp_parallel(Mat *mat, unsigned int grain, unsigned int thr){
    assert(mat->m == mat->n); // Ensure the matrix is square
    pthread_t threads[thr]; // Array to hold thread identifiers
    pthread_mutex_t mutex; // Mutex for synchronizing access to the shared index
    pthread_mutex_init(&mutex, NULL); // Initialize the mutex
    
    unsigned int shrd_index = 1; // Start with the first off-diagonal element
    unsigned int max_index = mat->n * mat->n - mat->n - 1; // Calculate the maximum valid index
    Args args = {&mutex, &shrd_index, &max_index, &grain, mat}; // Bundle arguments
    
    // Create threads to perform the transposition in parallel
    for (unsigned int i = 0; i < thr; i++) {
        if (pthread_create(&threads[i], NULL, &mat_squaretransp_thrd, (void *)&args)) {
            printf("Error creating thread\n"); // Error handling
            exit(-1);
        }
    }
    
    // Wait for all threads to finish
    for (unsigned int i = 0; i < thr; i++) pthread_join(threads[i], NULL);
    
    pthread_mutex_destroy(&mutex); // Clean up the mutex
    return;
}
