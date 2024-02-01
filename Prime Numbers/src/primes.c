#include <pthread.h>
#include <stdbool.h>
#include "util_common.h"
#include <math.h>
#include <stdio.h>
#include "primes.h"

// Function to determine if a number is prime or not
bool isPrime(unsigned int n){
    // 2 and 3 are prime numbers
    if (n == 2 || n == 3) return true;
    // Any even number other than 2 is not prime
    if (n % 2 == 0) return false;
    // Check for factors up to the square root of the number
    for (int i = 3; i <= floor(sqrt(n)); ++i){
        if(n % i == 0) return false; // n is divisible by some other number, hence not prime
    }
    return true; // n is prime
}

// Function to generate prime numbers sequentially up to 'max'
void primes_sequential(carr_d_t *primes_list, unsigned int max){
    for(unsigned int curNum = 2; curNum <= max; ++curNum){
        if(isPrime(curNum)) carr_d_push(primes_list, curNum); // if prime, add to the list
    }
    return;
}

// Struct for shared data between threads
typedef struct{
    pthread_mutex_t *pmtx; // mutex for locking
    carr_d_t *primes_list; // list to store prime numbers
    unsigned int *max;     // max range for prime search
    unsigned int *count;   // current count of number being checked
} sharedLib;

// Thread function to find primes in a range
void* getPrime(void *sLib){
    sharedLib *lib = (sharedLib *) sLib;
    while(true){
        // Lock the shared data to prevent race conditions
        pthread_mutex_lock(lib->pmtx);
        unsigned int curNum = *(lib->count);
        *(lib->count) = *(lib->count)+1;
        // Unlock after reading the data
        pthread_mutex_unlock(lib->pmtx);
        // Exit the loop if current number exceeds the max range
        if (curNum > *(lib->max)) break;
        // if the current number is prime, add to the list
        if (isPrime(curNum)){
            carr_d_push(lib->primes_list, curNum);
        }
    }
    pthread_exit(NULL); // exit the thread
}

// Function to generate prime numbers using multiple threads
void primes_parallel(carr_d_t *primes_list, unsigned int max, unsigned int thr){
    pthread_t threads[thr]; // Array to store thread identifiers
    pthread_mutex_t mutex;  // Mutex for locking shared data
    pthread_mutex_init(&mutex, NULL); // Initialize the mutex
    unsigned int count = 2; // Start checking primes from 2
    sharedLib sLib = {&mutex, primes_list, &max, &count}; // Create the shared library object

    // Create 'thr' number of threads
    for(unsigned int i = 0; i < thr; ++i){
        if(pthread_create(&threads[i], NULL, &getPrime, (void*)&sLib) != 0){
            fprintf(stderr, "pthread_create() error"); // Error creating the thread
            return;
        }
    }
    // Wait for all threads to finish
    for(unsigned int i = 0; i < thr; ++i){
        if(pthread_join(threads[i], NULL) != 0){
            fprintf(stderr, "pthread id: %u terminated error", i); // Error joining the thread
            return;
        }
    }
    pthread_mutex_destroy(&mutex); // Destroy the mutex after its use
    return;
}
