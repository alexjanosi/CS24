/*
 * Overview:
 *
 * Block structure:
 * Block have a header and a footer that store the size of the block and if the
 * block is allocated or not (0 if not and anything else otherwise). Allocated
 * blocks have a payload that stores the allocated information say when malloc
 * is called. Free blocks have an empty payload that stores two pointers: a
 * pointer to the previous free block and the next free block. Allocated blocks
 * store the information after these pointers, but they are not used in the free
 * list since allocated. This is how we create the explicit list that is stored
 * in the heap. Free blocks will have 24 bytes (4 header + 4 footer
 * + 2 * 8 pointer) already and additional space in the payload, and allocated
 * blocks will just hold the information assigned to that block.
 *
 * Organization of the free list:
 * The free list is initially NULL, which is set as a global pointer. When
 * blocks are added, new blocks are put to the front of the doubly linked list.
 * Therefore, the orginzation does not follow the order of the heap, but it is
 * in order of how new the free block is. By creating a list like this, we have
 * quick traversal in find_fit to find the free block of a certain size.
 *
 * Allocator and the free list:
 * As stated, the free list starts as NULL when the heap is initialized in
 * mm_init. Most of the updating then happens in coalesce. When the first
 * block is added in extend_heap, the first if statement in coalesce occurs,
 * which adds this block as a free block. From there on out, functions like
 * malloc, free, and place that update the memory in the heap call coalesce.
 * Coalesce handles every case from just removing a free block to checking if
 * there are adjacent free blocks to join as one. The functions add_free_block
 * and remove_free_block handle updating the free list, which are called in
 * coalesce and put.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (7)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define SIZE_PTR(p)  ((size_t*)(((char*)(p)) - SIZE_T_SIZE))


/* Basic constants and macros */
/* Inspiration from the textbook */
#define WSIZE 4 /* Word and header/footer size (bytes) */
#define DSIZE 8 /* Double word size (bytes) */
#define CHUNKSIZE (1<<8) /* Extend heap by this amount (bytes) */

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr ptr, compute address of its header and footer */
#define HDRP(ptr) ((char *)(ptr) - WSIZE)
#define FTRP(ptr) ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)

/* Given block ptr ptr, compute address of next and previous blocks */
#define NEXT_BLKP(ptr) ((char *)(ptr) + GET_SIZE(((char *)(ptr) - WSIZE)))
#define PREV_BLKP(ptr) ((char *)(ptr) - GET_SIZE(((char *)(ptr) - DSIZE)))

// Find the previous and next free block
#define PVFR(ptr) (*(char **)(ptr))
#define NXFR(ptr) (*((char **)(ptr) + 1))

// pointer to first free block
char *firstfree;

/*
 * add_free_block
 * Input: pointer to beginning of payload, where the pointer of the previous
 * free block is
 * Output: Nothing
 * This function takes a pointer to a newly free block and adds it to the doubly
 * linked list
 */
static void add_free_block(char *ptr) {

    // no list
    if (firstfree == NULL) {
        firstfree = ptr;
        PVFR(firstfree) = NULL;
        NXFR(firstfree) = NULL;
    }

    // add to beginning of list
    else {
        PVFR(firstfree) = ptr;
        NXFR(ptr) = firstfree;
        PVFR(ptr) = NULL;
        firstfree = ptr;
    }
}

/*
 * remove_free_block
 * Input: pointer to beginning of payload, where the pointer of the previous
 * free block is
 * Output: Nothing
 * This function takes a pointer to a newly free block and removes it from
 * the doubly linked list
 */
static void remove_free_block(char *ptr) {

    // no list
    if (!firstfree) {
        printf("Freeing block that doesn't exist\n");
        exit(1);
    }
    else {
        // only block
        if (!NXFR(ptr) && !PVFR(ptr)) {
            firstfree = NULL;
        }
        // beginning of list
        else if (NXFR(ptr) && !PVFR(ptr)) {
            firstfree = NXFR(ptr);
            PVFR(NXFR(ptr)) = NULL;
        }
        // end of list
        else if (!NXFR(ptr) && PVFR(ptr)) {
            NXFR(PVFR(ptr)) = NULL;
        }
        // middle of list
        else {
            NXFR(PVFR(ptr)) = NXFR(ptr);
            PVFR(NXFR(ptr)) = PVFR(ptr);
        }
    }
}

/*
 * find_fit
 * Input: a size in bytes of type size_t
 * Output: Either a pointer to a free block or NULL
 * This function iterates through the free list and finds the first block that
 * has enough space given by the input. If no blocks match, return NULL
 */
static void *find_fit(size_t size)
{
    // traverse through free list to find fit
    void *ptr;

    for (ptr = firstfree; ptr; ptr = NXFR(ptr)) {
        if (size <= GET_SIZE(HDRP(ptr))) {
         return ptr;
        }
    }
    // no fit in search
    return NULL;
}

/*
 * Coalesce
 * Input: A pointer to a block
 * Output: A pointer to a new block
 * This function takes in a block pointer and sees if it can join free blocks
 * by checking if the previous and next blocks are allocated. This also updates
 * the free list by calling add_free_block or remove_free_block
 */
static void *coalesce(void *ptr) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t size = GET_SIZE(HDRP(ptr));
    size_t nextsize = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
    size_t prevsize = GET_SIZE(HDRP(PREV_BLKP(ptr)));

    // both prev and next are allocated
    if (prev_alloc && next_alloc) {
        add_free_block(ptr);
    }

    // next is not allocated
    else if (prev_alloc && !next_alloc) {
        remove_free_block(NEXT_BLKP(ptr));
        size += nextsize;
        PUT(HDRP(ptr), PACK(size, 0));
        PUT(FTRP(ptr), PACK(size,0));
        add_free_block(ptr);

    }

    // prev is not allocated
    else if (!prev_alloc && next_alloc) {
        remove_free_block(PREV_BLKP(ptr));
        size += prevsize;
        PUT(FTRP(ptr), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        add_free_block(PREV_BLKP(ptr));
        ptr = PREV_BLKP(ptr);
    }

    // both prev and next are not allocated
    else {
        remove_free_block(PREV_BLKP(ptr));
        remove_free_block(NEXT_BLKP(ptr));
        size += nextsize + prevsize;
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
        add_free_block(PREV_BLKP(ptr));
        ptr = PREV_BLKP(ptr);
    }
    return ptr;
}

/*
 * Place
 * Input: A pointer to a block and a size
 * Output: Nothing
 * This function takes in a pointer and a size and checks if the block has
 * enough space. It acts accordingly given the space in the block.
 */
static void place(void *ptr, size_t size)
{
    size_t csize = GET_SIZE(HDRP(ptr));

    // makes sure there is enough room
    // 24 = 4 (header) + 4 (footer) + 16 (pointers)
    if (csize - size > 24) {
        remove_free_block(ptr);
        PUT(HDRP(ptr), PACK(size, 1));
        PUT(FTRP(ptr), PACK(size, 1));
        ptr = NEXT_BLKP(ptr);
        PUT(HDRP(ptr), PACK(csize-size, 0));
        PUT(FTRP(ptr), PACK(csize-size, 0));
        coalesce(ptr);
    }
    else {
        remove_free_block(ptr);
        PUT(HDRP(ptr), PACK(csize, 1));
        PUT(FTRP(ptr), PACK(csize, 1));
    }
}

/*
 * Extend_heap
 * Input: An amount of bytes
 * Output: Nothing
 * This function takes in an amount of bytes and adds new blocks to the heap.
 * This makes sure that alligment is correct and makes the header and footer
 * for the blocks. It also calls coalesce which adds free blocks to the list
 */
static void *extend_heap(size_t words) {
    char *ptr;
    size_t size;

    // make sure even for allignment
    if (words % 2 != 0) {
      size = (words + 1) * WSIZE;
    }
    else {
      size = words * WSIZE;
    }

    // must be large enough for the header, footer, and pointers
    if (size < 24) {
        size = 24;
    }

    if ((long)(ptr = mem_sbrk(size)) == -1) {
        return NULL;
    }

    // Make free block header, footer, and updatew epilogue
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(ptr)), PACK(0, 1));

    // coalesce updated heap
    return coalesce(ptr);
}


/*
 * Mm_init
 * Input: Nothing
 * Output: Nothing
 * This is the first call which sets up the heap and creates a free block. This
 * function creates the alignment, prologue, and epilogue.
 */
int mm_init(void) {

    void *heap_listp;
    // Create the initial empty heap
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) {
        return -1;
    }

    firstfree = NULL;
    PUT(heap_listp, 0); // Alignment
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); // Prologue header
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); // Prologue footer
    PUT(heap_listp + (3*WSIZE), PACK(0, 1)); // Epilogue
    heap_listp += (2*WSIZE);

    // check the epilogue
    if (GET_SIZE(heap_listp + WSIZE) != 0 || GET_ALLOC(heap_listp + \
      WSIZE) != 1) {
        fprintf(stderr, "Epilogue header is wrong\n");
        exit(1);
    }

    // Extend the empty heap with a free block of CHUNKSIZE bytes
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
    }

    return 0;
}

/*
 * Malloc
 * Input: A size in bytes
 * Output: A pointer to where the data is stored in the heap_list
 * This function takes in a size of bytes, deals with alignment, and finds
 * space in the heap to store these bytes. The function searches through
 * free blocks in find_fit and then extends the heap if need in extend_heap
 */
void *malloc(size_t size) {

    size_t asize;
    size_t extendsize;
    char *ptr;

    // no memory to allocate
    if (size == 0) {
        return NULL;
    }

    // make sure bigger than content in block
    if (ALIGN(size) + 8 > 24) {
      asize = ALIGN(size) + 8;
    }
    else {
      asize = 24;
    }

    // search free list for spot with right size
    if ((ptr = find_fit(asize)) != NULL) {
        place(ptr, asize);
        return ptr;
    }

    // search failed so extend the heap with size
    if (asize > CHUNKSIZE) {
      extendsize = asize;
    }
    else {
      extendsize = CHUNKSIZE;
    }

    if ((ptr = extend_heap(extendsize / WSIZE)) == NULL) {
        return NULL;
    }
    place(ptr, asize);
    return ptr;
}

/*
 * Free
 * Input: a pointer
 * Output: Nothing
 * This function takes in a pointer and removes the memory that was stored at
 * that block. If NULL is inputted, nothing happens
 */
void free(void *ptr) {

    // no effect for NULL
    if (!ptr) {
        return;
    }
    else {
        // get size of block and set to zero
        size_t size = GET_SIZE(HDRP(ptr));
        PUT(HDRP(ptr), PACK(size, 0));
        PUT(FTRP(ptr), PACK(size, 0));
        // this updates the free list
        coalesce(ptr);
    }
}

/*
 * Realloc
 * Input: A pointer to where memory is stored and a size in bytes
 * Output: A pointer to a new place in memory
 * This function takes in existing memory and finds a new place to store it
 */
void *realloc(void *oldptr, size_t size) {
    size_t oldsize;
    void *newptr;

    // realloc to nothing so free
    if (size == 0) {
        free(oldptr);
        return NULL;
    }

    // no original data so malloc
    if (oldptr == NULL) {
        return malloc(size);
    }

    // if we cannot allocate, just leave it
    if((newptr = malloc(size)) == NULL) {
        return 0;
    }

    // copy data to new spot
    oldsize = *SIZE_PTR(oldptr);
    if(size < oldsize) oldsize = size;
    memcpy(newptr, oldptr, oldsize);

    // get rid of old block
    free(oldptr);

    return newptr;
}

/*
 * calloc - Allocate the block and set it to zero.
 */
void *calloc (size_t nmemb, size_t size) {

    void *newptr = malloc(nmemb * size);
    memset(newptr, 0, nmemb * size);

    return newptr;
}

/*
 * mm_checkheap:
 * Input: Integer line number
 * Output: Nothing, will exit if an error occurs
 * Function performs checks on the heap and free list
 */
void mm_checkheap(int lineno) {

    char *check;
    int countfreelist = 0;
    int freeheaplist = 0;

    // check the prologue header
    if (GET_SIZE(mem_heap_lo() + WSIZE) != DSIZE || GET_ALLOC(mem_heap_lo()\
     + WSIZE) != 1) {
        fprintf(stderr, "%d:Prologue header is wrong\n", lineno);
        exit(1);
    }

    // check the prologue footer
    if (GET_SIZE(mem_heap_lo() + 2 * WSIZE) != DSIZE || GET_ALLOC(mem_heap_lo()\
     + 2 * WSIZE) != 1) {
        fprintf(stderr, "%d:Prologue footer is wrong\n", lineno);
        exit(1);
    }

    // check the epilogue
    if (GET_SIZE(mem_heap_hi() - 3) != 0 || GET_ALLOC(mem_heap_hi() - 3) != 1) {
        fprintf(stderr, "%d:Epilogue is wrong\n", lineno);
        exit(1);
    }


    // checking the heap
    for (char *ptr = mem_heap_lo() + 4 * WSIZE; GET_SIZE(HDRP(ptr)) > 0;\
     ptr = NEXT_BLKP(ptr)) {
        // count number of free blocks
        if (!GET_ALLOC(HDRP(ptr))) {
            freeheaplist += 1;
            // there is a next block
            if (GET_SIZE(HDRP(NEXT_BLKP(ptr))) > 0) {
                // the next block is also free
                if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr)))) {
                    fprintf(stderr, "%d:Two free blocks: coalescing failed\n",\
                     lineno);
                    exit(1);
                }
            }

            if ((size_t)ptr % 8) {
                fprintf(stderr, "%d:Allignment off\n", lineno);
                exit(1);
            }

        }

        if (GET_SIZE(HDRP(ptr)) != GET_SIZE(FTRP(ptr)) || GET_ALLOC(HDRP(ptr)) \
        != GET_ALLOC(FTRP(ptr))) {
            fprintf(stderr, "%d:Headers and footers not consistent\n", lineno);
            exit(1);
        }
    }

    // checking the free list
    for (check = firstfree; check != NULL; check = NXFR(check)) {

        countfreelist += 1;

        // beginning of list so just check next, since prev is NULL
        if (check == firstfree) {

            // only block in list
            if (NXFR(check) == NULL) {
                continue;
            }

            // additional blocks after
            if (PVFR(NXFR(check)) != check ) {
                fprintf(stderr, "%d:Pointers not consistent1\n", lineno);
                exit(1);
            }

            // make sure pointers are between low and high
            if ((void *)NXFR(check) > mem_heap_hi() || (void *)NXFR(check)\
             < mem_heap_lo()) {
                fprintf(stderr, "%d:Pointers out of bounds1\n", lineno);
                exit(1);
            }
        }

        // end of list
        else if (NXFR(check) == NULL) {

            // check consistency
            if (NXFR(PVFR(check)) != check ) {
                fprintf(stderr, "%d:Pointers not consistent2\n", lineno);
                exit(1);
            }

            // make sure pointers are between low and high
            if ((void *)PVFR(check) > mem_heap_hi() || (void *)PVFR(check) \
            < mem_heap_lo()) {
                fprintf(stderr, "%d:Pointers out of bounds2\n", lineno);
                exit(1);
            }


        }
        // every other case since we do not need to check the very last part
        // since it is check by the second to last
        else {
            if (PVFR(NXFR(check)) != check || NXFR(PVFR(check)) != check) {
                fprintf(stderr, "%d:Pointers not consistent3\n", lineno);
                exit(1);
            }

            // make sure pointers are between low and high
            if ((void *)NXFR(check) > mem_heap_hi() || (void *)NXFR(check) \
            < mem_heap_lo()
                || (void *)PVFR(check) > mem_heap_hi() || (void *)PVFR(check) \
                < mem_heap_lo()) {
                fprintf(stderr, "%d:Pointers out of bounds3\n", lineno);
                exit(1);
            }
        }
    }


    // check count of freelist and free blocks in heap
    if (countfreelist != freeheaplist) {
        fprintf(stderr, "%d:Freelist is not same size as actual free blocks\n",\
         lineno);
        exit(1);
    }

    // All tests pass
}
