#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include <math.h>
#include <time.h>

typedef size_t PID;
size_t MAX_ITER = UINT_MAX; // Max iterations to avoid infinite loops

// Copies the current load to a past load array for comparison
void copyLoadsList(unsigned loads[], unsigned pastLoads[], size_t proccessNumber) {
    for(size_t i=0; i<proccessNumber; i++)
        pastLoads[i] = loads[i];
}

// Checks if the maximum load difference is less than or equal to 1
bool isMaxDiff(unsigned loads[], size_t numProcs) {
    size_t maxVal = loads[0], minVal = loads[0];
    for(size_t i=0; i<numProcs; i++) {
        if(loads[i] > maxVal)
            maxVal = loads[i];
        else if(loads[i] < minVal)
            minVal = loads[i];
    }
    return maxVal - minVal <= 1;
}

// Compares current and past loads to determine if system is steady
bool checkLoad(unsigned loads[], unsigned pastLoads[], size_t numProcs) {
    size_t i = 0;
    while(i<numProcs) {
        if(loads[i] != pastLoads[i])
            break;
        i++;
    }
    return i == numProcs-1 || (isMaxDiff(loads, numProcs) && isMaxDiff(pastLoads, numProcs));
}

// Initializes execution cycles randomly within given min and max values
void initExecCyclesUniform(unsigned execCycles[], size_t numProcs, unsigned cycleMin, unsigned cycleMax) {
    assert(cycleMin <= cycleMax); // Ensures min is not greater than max

    for(PID pid = 0; pid < numProcs; pid++) {
        execCycles[pid] = rand() % (cycleMax - cycleMin + 1) + cycleMin;
    }
}

// Finds the lowest execution cycle value among processors
size_t findLowestExecCycle(unsigned execCycles[], size_t proccessNumber) {
    size_t lowest = execCycles[0];
    for(PID i=1; i<proccessNumber; i++) {
        if(execCycles[i] < lowest)
            lowest = execCycles[i];
    }
    return lowest;
}

// Checks if current and past execution cycles are overlapping
bool isExecCyclesOL(unsigned execCycles[], unsigned pastExecCycles[], size_t numProcs) {
    for(size_t i=0; i<numProcs; i++) {
        if(execCycles[i] == pastExecCycles[i])
            return true;
    }
    return false;
}

// Function to redistribute load from one processor to another
void give(unsigned loads[], PID fromProc, PID toProc, unsigned giveAmount) {
    if(giveAmount != 0) {
        assert(loads[fromProc] >= giveAmount); // Ensures load is sufficient to give

        loads[fromProc] -= giveAmount;
        loads[toProc] += giveAmount;
    }
}

// Balances load among a processor and its immediate neighbors
void balance(unsigned loads[], size_t numProcs, PID pid) {
    PID pidLeft = pid == 0 ? numProcs - 1 : pid - 1; // Handles circular arrangement
    PID pidRight = (pid + 1) % numProcs;

    unsigned loadSum = loads[pidLeft] + loads[pid] + loads[pidRight];
    double avgLoad = loadSum / 3.0;

    if(loads[pid] < avgLoad) return; // No action if load is less than average

    size_t startLoad = loadSum%3 == 2 ? ceil(avgLoad) : floor(avgLoad);
    unsigned giveAmountLeft = loads[pidLeft] < avgLoad ? startLoad - loads[pidLeft] : 0;
    unsigned giveAmountRight = loads[pidRight] < avgLoad ? floor(avgLoad) - loads[pidRight] : 0;

    if(loads[pid] < floor(avgLoad) - giveAmountLeft - giveAmountRight) return; // Ensures load doesn't go negative

    give(loads, pid, pidLeft, giveAmountLeft);
    give(loads, pid, pidRight, giveAmountRight);

    unsigned loadSumAfter = loads[pidLeft] + loads[pid] + loads[pidRight];
    assert(loadSum == loadSumAfter); // Verifies load sum remains constant
}

// Utility function to print processor loads
void printLoads(unsigned loads[], size_t numProcs) {
    printf("[");
    for(PID pid = 0; pid < numProcs; pid++) {
        printf("%u", loads[pid]);
        if(pid != numProcs - 1) printf(", ");
    }
    printf("]\n");
}

// Utility function to print execution intervals
void printIntervals(unsigned execCycles[], size_t numProcs) {
    printf("[");
    for(PID pid = 0; pid < numProcs; pid++) {
        printf("%u", execCycles[pid]);
        if(pid != numProcs - 1) printf(", ");
    }
    printf("]\n");
}

// Main loop to iterate over intervals and balance loads
void iterateOverIntervals(unsigned loads[], unsigned pastLoads[], unsigned execCycles[], unsigned pastExecCycles[], size_t numProcs, unsigned cycleMin, unsigned cycleMax) {
    size_t currExecCycle = findLowestExecCycle(execCycles, numProcs), stages = 0;
    while(currExecCycle < MAX_ITER && (!checkLoad(loads, pastLoads, numProcs) || isExecCyclesOL(execCycles, pastExecCycles, numProcs))) {
        for(PID proc=0; proc<numProcs; proc++) {
            if(execCycles[proc] == currExecCycle) {
                balance(loads, numProcs, proc);
                execCycles[proc] += rand() % (cycleMax - cycleMin + 1) + cycleMin;
            }
        }
        stages++;
        if(!checkLoad(loads, pastLoads, numProcs)){
            for(size_t i=0; i<numProcs; i++) pastExecCycles[i] = execCycles[i];
        }
        copyLoadsList(loads, pastLoads, numProcs);
        currExecCycle = findLowestExecCycle(execCycles, numProcs);
    }
    if(currExecCycle > MAX_ITER)
        printf("\nMax iterations were reached\n\n");
    else
        printf("\nNumber of stages to reach steady balanced state: %zu\n\n", stages);
}

int main() {
    srand(time(NULL)); // Seed the random number generator
    size_t numProcs[] = {3, 5, 10, 50, 100, 250}; // Different processor counts for simulation
    unsigned loadMin = 10, loadMax = 1000;
    unsigned cycleMin = 100, cycleMax = 1000;

    // Iterate over different cases
    for(size_t i=0; i<(int)(sizeof(numProcs)/sizeof(numProcs[0])); i++) {
        printf("Case %zu", i+1);
        printf("\nNumber of Processors: %zu", numProcs[i]);
        printf("\nMinimum Load Value: %u", loadMin);
        printf("\nMaximum Load Value: %u", loadMax);
        printf("\nMinimum Interval Value: %u", cycleMin);
        printf("\nMaximum Interval Value: %u\n\n", cycleMax);

        // Allocate and initialize loads and execution cycles
        unsigned* loads = calloc(numProcs[i], sizeof(unsigned));
        unsigned* pastLoads = calloc(numProcs[i], sizeof(unsigned));
        for(PID pid = 0; pid < numProcs[i]; pid++) {
            loads[pid] = rand() % (loadMax - loadMin + 1) + loadMin;
        }
        copyLoadsList(loads, pastLoads, numProcs[i]);
        printf("Initial Loads::\n");
        printLoads(loads, numProcs[i]);

        unsigned* execCycles = calloc(numProcs[i], sizeof(unsigned));
        unsigned* pastExecCycles = calloc(numProcs[i], sizeof(unsigned));
        initExecCyclesUniform(execCycles, numProcs[i], cycleMin, cycleMax);
        for(size_t i=0; i<numProcs[i]; i++) pastExecCycles[i] = execCycles[i];
        printf("\nInitial Cycles::\n");
        printIntervals(execCycles, numProcs[i]);

        iterateOverIntervals(loads, pastLoads, execCycles, pastExecCycles, numProcs[i], cycleMin, cycleMax);

        printf("Final Loads::\n");
        printLoads(loads, numProcs[i]);
        printf("\nFinal Cycles::\n");
        printIntervals(pastExecCycles, numProcs[i]);

        free(loads);
        free(pastLoads);
        free(execCycles);
        free(pastExecCycles);
    }
}
