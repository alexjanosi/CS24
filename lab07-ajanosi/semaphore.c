/*
 * General implementation of semaphores.
 *
 *--------------------------------------------------------------------
 * Adapted from code for CS24 by Jason Hickey.
 * Copyright (C) 2003-2010, Caltech.  All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#include "sthread.h"
#include "semaphore.h"
#include "glue.h"
#include "queue.h"

/*
 * The semaphore data structure contains an integer semaphore value
 * and a queue that contains the blocked threads
 */
struct _semaphore {

    // nonnegative variable
    int num;

    // blocked list
    Queue *list;

};

/************************************************************************
 * Top-level semaphore implementation.
 */

/*
 * Allocate a new semaphore.  The initial value of the semaphore is
 * specified by the argument.
 */
Semaphore *new_semaphore(int init) {

    // allocate memory
    Semaphore *semp = (Semaphore *)malloc(sizeof(Semaphore));
    Queue *list = (Queue *)malloc(sizeof(Queue));

    // make sure malloc worked
    if (!semp || !list) {
        fprintf(stderr, "Memory for semaphore failed!\n");
        exit(1);
    }

    // add attributes
    semp->num = init;
    semp->list = list;

    return semp;
}

/*
 * Decrement the semaphore.
 * This operation must be atomic, and it blocks iff the semaphore is zero.
 */
void semaphore_wait(Semaphore *semp) {
    
    // must perform atomically
    __sthread_lock();

    while (semp->num == 0) {
        // add to list
        queue_append(semp->list, sthread_current());
        // block current
        sthread_block();
        // relock since unblocking unlucks it
        __sthread_lock();
    }
    semp->num--;

    // we can unlock thread after safe decrementing
    __sthread_unlock();
}

/*
 * Increment the semaphore.
 * This operation must be atomic.
 */
void semaphore_signal(Semaphore *semp) {
    
    // we lock so this occurs atomically
    __sthread_lock();
    semp->num++;

    if (!queue_empty(semp->list)) {
        // unblock one of them
        sthread_unblock(queue_take(semp->list));
    }
    // we safely incremented, so we unlock
    __sthread_unlock();

}

