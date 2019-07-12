#include <stdio.h>
#include "sthread.h"


/*
 * Thread 1 prints the argument + 1.
 * Thread 2 prints the argument - 1.
 *
 * In this version of the code,
 * thread scheduling is explicit.
 */

/*! This thread-function adds one */
static void loop1(void *arg) {
    while(1) {
        // cast back to int and update
        *(int *)arg += 1;
        printf("Value = %d\n", *(int *)arg);
        sthread_yield();
    }
}

/*! This thread-function subtracts one */
static void loop2(void *arg) {
    while(1) {
        // cast back to int and update
        *(int *)arg -= 1;
        printf("Value = %d\n", *(int *)arg);
        sthread_yield();
    }
}

/*
 * The main function starts the two loops in separate threads,
 * the start the thread scheduler.
 */
int main(int argc, char **argv) {
    // argument inputted to loops
    int argument = 0;

    sthread_create(loop1, (void *)&argument);
    sthread_create(loop2, (void *)&argument);
    sthread_start();
    return 0;
}
