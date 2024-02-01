#include <stdlib.h>
#include <pthread.h>
#include "util.h" 

// Structure to define the range of rows that each thread will process.
typedef struct {
    size_t start; // Starting index of the row range.
    size_t end;   // Ending index of the row range (inclusive).
} rowRange;

// Structure to pass multiple arguments to the thread function.
typedef struct {
    Mat* A;
    Mat* B;
    Mat* C;
    rowRange* rowRange; // Pointer to the structure holding the start and end row indices.
} Args;

// The thread function to multiply parts of a matrix.
void* multiply(void* args_) {
    Args* args = (Args*)args_;
    Mat* A = args->A;
    Mat* B = args->B;
    Mat* C = args->C;
    rowRange range = *(args->rowRange);

    // Perform the matrix multiplication for the assigned row range.
    for (size_t row = range.start; row <= range.end; ++row) {
        for (size_t col = 0; col < B->n; ++col) {
            for (size_t k = 0; k < B->m; ++k) {
                size_t idxA = row * A->n + k;       // Index for A[row][k]
                size_t idxB = k * B->n + col;       // Index for B[k][col]
                size_t idxC = row * C->n + col;     // Index for C[row][col]
                // Multiply and accumulate the result in C.
                (C->ptr)[idxC] = (C->ptr)[idxC] + (A->ptr)[idxA] * (B->ptr)[idxB];
            }
        }
    }
    return NULL;
}

// Function to perform matrix multiplication using multiple threads.
void mat_multiply(Mat *A, Mat *B, Mat *C, unsigned int threads) {
    int width = A->m / threads; // Calculate the number of rows each thread should process.
    rowRange rowRanges[threads]; // Array to store the row ranges for each thread.
    rowRange rowRange; // Temporary variable to hold the current row range.
    int count = -1;

    // Divide the work among the threads by assigning row ranges.
    for (int i = 0; i < threads; ++i) {
        rowRange.start = count + 1;
        // If it's the last thread, ensure it processes all remaining rows.
        if (i == threads - 1) rowRange.end = A->m - 1;
        else rowRange.end = count + width;
        rowRanges[i] = rowRange; // Store the current row range.
        count = rowRange.end; 
    }

    Args* args[threads]; // Array to store the arguments for each thread.
    pthread_t thrList[threads]; // Array to store thread identifiers.

    // Create threads to perform matrix multiplication.
    for (int i = 0; i < threads; ++i) {
        args[i] = malloc(sizeof(Args)); // Allocate memory for thread arguments.
        args[i]->A = A; // Set matrix A for the current thread.
        args[i]->B = B; // Set matrix B for the current thread.
        args[i]->C = C; // Set matrix C for the current thread.
        args[i]->rowRange = &rowRanges[i]; // Set row range for the current thread.
        // Create a new thread to execute the multiply function with the given arguments.
        pthread_create(&thrList[i], NULL, &multiply, args[i]);
    }

    // Wait for all threads to complete their execution.
    for (int i = 0; i < threads; ++i) {
        pthread_join(thrList[i], NULL); // Block until thread i completes.
    }

    // Clean up and free the allocated memory for arguments.
    for (int i = 0; i < threads; ++i) {
        free(args[i]); 
    }
    return;
}
