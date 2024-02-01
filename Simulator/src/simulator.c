/**
 * The code simulates a memory allocation system for multiple processors using a circular linked
 * list and calculates the average access time for each processor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "simulator.h"


#define MAX_CPU_CYCLES 1000000
#define NUM_MEMORY_MODULES 512
#define STANDARD_DEVIATION 5

double PI_VAL = 3.14159265358979323846;

struct Node* head = NULL;

/** Node defined for linked list. This linked list will store processor data*/
typedef struct Node {
    int pNum; // processor ID
    int mem_mod; 
    int numAccess;
    int mean;
    struct Node* next;
} Node;

/** Utility function to insert nodes in the circular linked list (queue)*/
void insertNode(int pNum) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->pNum = pNum;
    newNode->numAccess = 0;

    if (head == NULL) {
        newNode->next = newNode;
        head = newNode;
    } else {
        newNode->next = head;
        struct Node* temp = head;
        while (temp->next != head) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
}

/**Function to assign random value basis the type of distribution passed in the argument*/
int assignRandomValue(Node* listNode, char dist, int mem_size)
{
    if(dist=='u')
        return rand_uniform(mem_size);
    else 
        return rand_normal_wrap(listNode->mean, STANDARD_DEVIATION, mem_size);
}

/**
 * The function "allocateMemoryModules" assigns memory modules to nodes based on certain conditions and
 * returns the first node that was denied a memory module.
 */
Node* allocateMemoryModules(int memory_modules[], char dist, int mem_size) {

    Node* curr = head;
    Node* first_denied = NULL;
    do {
        if(memory_modules[curr->mem_mod] == 0) 
        {
            curr->numAccess += 1;
            memory_modules[curr->mem_mod] = curr->pNum;
            curr->mem_mod = assignRandomValue(curr,dist, mem_size);
            
        } else {
            if(first_denied==NULL)
                first_denied = curr;
        }
        curr = curr->next;
    } while(curr!=head);

    return first_denied;
}

/**
 * This function creates a circular queue by inserting nodes with values from 1 to procs.
 */
void createCircularQueue(int procs) {

    for (int i = 1; i <= procs; i++) {
        insertNode(i);
    }
}

/* Calculates the total time cummulative for each memory module and cpu cycle*/
int computeTotalTimeCummulative(int cpu_cycle) {
    Node* curr = head;
    float tot_tc_pi = 0;
    float tc_pi = 0;

    do {
        if (curr->numAccess != 0)
            tc_pi = (float)cpu_cycle / curr->numAccess;
        else 
            tc_pi = 0;

        tot_tc_pi += tc_pi;
        curr = curr->next;

    } while (curr != head);  

    return tot_tc_pi;
}

/**
 * Simulate memory access time for various configurations.
 *
 * This function performs a simulation to estimate the average memory access time
 * for a range of processor configurations and memory module access patterns.
 */
void simulate(double* avg_access_time, int avg_access_time_len, int procs, char dist) 
{
    double T_previous = 0;            // Previous average access time
    const double epsilon = 0.02;    // Termination condition threshold
    int cpu_cycle;

    createCircularQueue(procs);  // Initialize a circular queue of processors

    for(int mem = 1; mem <= NUM_MEMORY_MODULES; mem++) 
    {
        Node* tempNode = head;
        do {
            tempNode->mean = rand_uniform(mem);
            tempNode = tempNode->next;
        }while(tempNode!=head);
        
        T_previous=0;

        // Assign random memory modules to processors in the circular queue
        Node* temp = head;
        do {
            temp->mem_mod = assignRandomValue(temp, dist, mem);
        } while (temp != head);

        float tot_tc_pi;
        float T_current;
        int memory_modules[mem];  // An array to keep track of free memory modules, initialized to 0

        for(int itr = 0; itr < mem; itr++)
            memory_modules[itr] = 0;

        for(int cpu_cycle = 1; cpu_cycle <= MAX_CPU_CYCLES; cpu_cycle++) 
        {
            // Change the head of the linked list to the first process denied a memory module
            Node* denied_process = allocateMemoryModules(memory_modules, dist, mem);
            if (denied_process != NULL) {
                head = denied_process;
            }

            // Calculate T(S) for the current cycle
            tot_tc_pi = computeTotalTimeCummulative(cpu_cycle);

            T_current = (float)(tot_tc_pi / procs);

            // Check the change between T_previous and T_current against the termination condition
            if (fabs((T_previous - T_current) / T_current) < epsilon) {
                break;
            }
            T_previous = T_current;   

            // Free up memory modules after each cycle
            for(int itr = 0; itr < mem; itr++) 
            {
                memory_modules[itr] = 0;
            }
        }

        // Reset the numAccess for each processor in the circular queue
        Node* curr = head;
        do {
            curr->numAccess = 0;
            curr = curr->next;
        } while (curr != head);

        // Store the resulting average access time for this configuration
        avg_access_time[mem - 1] = T_current;

        // Print a message if the simulation ends due to reaching the maximum CPU cycles
        if (cpu_cycle == MAX_CPU_CYCLES) {
            printf("Simulation ended due to reaching the maximum CPU cycles.\n");
        }
    }
}

int rand_uniform(int max) {
    return rand() % max;
}

int rand_normal_wrap(int mean, int dev, int max) {
    static double U, V;
    static int phase = 0;
    double Z;
    if (phase == 0) {
        U = (rand() + 1.) / (RAND_MAX + 2.);
        V = rand() / (RAND_MAX + 1.);
        Z = sqrt(-2 * log(U)) * sin(2 * PI_VAL * V);
    } else {
        Z = sqrt(-2 * log(U)) * cos(2 * PI_VAL * V);
    }
    phase = 1 - phase;
    double res = dev * Z + mean;

    int res_int;
    if ((int)res % 2 == 0)
        res_int = (int)(res + 1);
    else
        res_int = (int)(res);

    int res_wrapped = res_int % max;
    if (res_wrapped < 0)
        res_wrapped += max;
    return res_wrapped;
}