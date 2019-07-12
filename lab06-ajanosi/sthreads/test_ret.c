#include <stdio.h>
#include "sthread.h"


/*
 * Thread 1 prints "Thread 1"
 * Thread 2 prints "Thread 2"
 * Thread 3 prints "Thread 3"
 * Thread 4 prints "Thread 4"
 *
 * In this version of the code,
 * thread scheduling is explicit.
 */

/*! This thread-function prints "Thread 1" */
static void loop1(void *arg) {
    while(1) {
        for (int i = 0; i < 1; i++) {
            printf("Thread 1\n");
        }
        sthread_yield();
    }
}

/*! This thread-function prints "Thread 2" */
static void loop2(void *arg) {
    while(1) {
        for (int i = 0; i < 2; i++) {
            printf("Thread 2\n");
        }
        sthread_yield();
    }
}

/*! This thread-function prints "Thread 3" */
static void loop3(void *arg) {
    while(1) {
        for (int i = 0; i < 3; i++) {
            printf("Thread 3\n");
        }
        sthread_yield();
    }
}

/*! This thread-function prints "Thread 4" */
static void loop4(void *arg) {
    while(1) {
        for (int i = 0; i < 4; i++) {
            printf("Thread 4\n");
        }
        sthread_yield();
    }
}

/*
 * The main function starts the two loops in separate threads,
 * the start the thread scheduler.
 */
int main(int argc, char **argv) {
    sthread_create(loop1, NULL);
    sthread_create(loop2, NULL);
    sthread_create(loop3, NULL);
    sthread_create(loop4, NULL);
    sthread_start();
    return 0;
}
