/*============================================================================
 * Implementation of the FIFO page replacement policy.
 *
 * We don't mind if paging policies use malloc() and free(), just because it
 * keeps things simpler.  In real life, the pager would use the kernel memory
 * allocator to manage data structures, and it would endeavor to allocate and
 * release as little memory as possible to avoid making the kernel slow and
 * prone to memory fragmentation issues.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "vmpolicy.h"

// do this to simplify the struct below
typedef struct loaded_pages_t loaded_pages_t;


/*============================================================================
 * "Loaded Pages" Data Structure
 *
 * This data structure records all pages that are currently loaded in the
 * virtual memory, so that we can choose a page to evict very easily. The first
 * page in this queue will be chosen to be evicted.
 */

struct loaded_pages_t {
    
    // the page in this node
    page_t page;

    // the next node in the queue
    loaded_pages_t *next;

};


/*============================================================================
 * Policy Implementation
 */

// the head and tail of the queue
loaded_pages_t *head;
loaded_pages_t *tail;


/* Initialize the policy.  Return nonzero for success, 0 for failure. */
int policy_init(int max_resident) {
    fprintf(stderr, "Using FIFO eviction policy.\n\n");
    
    // empty queue to start
    head = NULL;
    tail = NULL;
    
    /* Return nonzero if initialization succeeded. */
    return 1;
}


/* Clean up the data used by the page replacement policy. */
void policy_cleanup(void) {

    loaded_pages_t *iter = head;
    // need to free each node
    while (iter != NULL) {
        loaded_pages_t *tofree = iter;
        iter = iter->next;
        free(tofree);
    }

    // set back to initial conditions
    head = NULL;
    tail = NULL;

}


/* This function is called when the virtual memory system maps a page into the
 * virtual address space.  Record that the page is now resident.
 */
void policy_page_mapped(page_t page) {
    
    // allocate new node
    loaded_pages_t *newnode = (loaded_pages_t *)malloc(sizeof(loaded_pages_t));

    // check for success
    if (newnode == NULL) {
        fprintf(stderr, "Malloc failed!\n");
        exit(1);
    }

    // update attributes
    newnode->page = page;
    newnode->next = NULL;

    // update queue
    if (!head && !tail) {

        head = newnode;
        tail = newnode;
    }
    else {

        tail->next = newnode;
        tail = newnode;
    }
}


/* This function is called when the virtual memory system has a timer tick. */
void policy_timer_tick(void) {
    /* Do nothing! */
}


/* 
 * Evict the page at the head of the queue
 */
page_t choose_and_evict_victim_page(void) {
    page_t victim;

    // pick the first page to evict
    victim = head->page;

    // update the queue
    loaded_pages_t *tofree = head;
    head = head->next;
    free(tofree);
    tofree = NULL;

#if VERBOSE
    fprintf(stderr, "Choosing victim page %u to evict.\n", victim);
#endif

    return victim;
}
