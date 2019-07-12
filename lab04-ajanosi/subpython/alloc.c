/*! \file
 * Implementation of a simple memory allocator.  The allocator manages a small
 * pool of memory, provides memory chunks on request, and reintegrates freed
 * memory back into the pool.
 *
 * Adapted from Andre DeHon's CS24 2004, 2006 material.
 * Copyright (C) California Institute of Technology, 2004-2010.
 * All rights reserved.
 */

#include "alloc.h"

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "eval.h"

/*!
 * Specifies the size of the memory pool.  This is a static local variable;
 * the value is specified in the call to init_alloc().
 */
static int MEMORY_SIZE;


/*!
 * This is the starting address of the memory pool used in the implicit
 * allocator.  The pool is allocated within init_alloc().
 */
static unsigned char *mem;


/*!
 * The implicit allocator uses an external "free-pointer" to track where free
 * memory starts.  We can get away with this approach because our allocator
 * compacts memory towards the start of the pool during garbage collection.
 */
static unsigned char *freeptr;


/*!
 * This is the "reference table."  However, it is really just an array that
 * records where each Value starts in the pool.  References are just indexes
 * into this table.  An unused slot is indicated by storing NULL for the
 * Value pointer.  (Since it's an array of pointers, it's a pointer to a
 * pointer.)
 */
static struct Value **ref_table;

/*!
 * This is the number of references currently in the table.  Valid entries
 * are in the range 0 .. num_refs - 1.
 */
static int num_refs;

/*! This is the actual size of the ref_table. */
static int max_refs;

Reference make_reference();


/* marker
 * Input: a pointer and a reference
 * Output: nothing, just updates the Value types
 * This function updates the inputted values global variables by
 * looping through and changing the "marked" attribute
 */
void marker(const char *name, Reference ref) {
    // dereference to get Value *
    Value *curr_val = deref(ref);

    // base case to check for recursive
    if (!(curr_val) || curr_val->marked == 1) {
        return;
    }

    // list node has to mark all values within
    else if (curr_val->type == VAL_LIST_NODE) {
        curr_val->marked = 1;
        ListValue *listval = (ListValue *) curr_val;
        ListNode listnode = listval->list_node;

        // values and next part of list need to be marked
        marker(name, listnode.value);
        marker(name, listnode.next);
    }

    // dictionary node has to mark all values within
    else if (curr_val->type == VAL_DICT_NODE) {
        curr_val->marked = 1;
        DictValue *dictval = (DictValue *) curr_val;
        DictNode dictnode = dictval->dict_node;

        // key, values, and next part of dictionary need to be marked
        marker(name, dictnode.key);
        marker(name, dictnode.value);
        marker(name, dictnode.next);
    }

    // these values do not hold smaller values so just mark
    else {
        curr_val->marked = 1;
    }
}

/* sweeper_and_compactor
 * Input: Nothing
 * Output: Nothing
 * This function goes through the heap and gets rid of the unmarked garbage.
 * It also compacts the memory to the front of the heap.
 */
void sweeper_and_compactor() {
    unsigned char *curr = mem;
    unsigned char *nextfree = mem;

    // loop through the heap
    while (curr < freeptr) {
        Value *curr_value = (Value *) curr;
        Value *free_value = (Value *) nextfree;
        int data_size = curr_value->data_size;
        int value_size = sizeof(Value) + data_size;
        Reference ref = curr_value->ref;

        // marked so we keep it and compact
        if (curr_value->marked == 1) {
            // reset marked value
            curr_value->marked = 0;
            // move to nearest free spot to start of heap
            memmove(nextfree, curr, value_size);
            // update ref table
            ref_table[ref] = free_value;
            // move nextfree pointer ahead
            nextfree += value_size;

        }
        // unmarked so we get rid of it
        else {
            // set reference table to NULL
            ref_table[ref] = NULL;
        }
        curr += value_size;
    }
    // adjust freeptr
    freeptr = nextfree;
}

/*!
 * This function initializes both the allocator state, and the memory pool.  It
 * must be called before myalloc() or myfree() will work at all.
 *
 * Note that we allocate the entire memory pool using malloc().  This is so we
 * can create different memory-pool sizes for testing.  Obviously, in a real
 * allocator, this memory pool would either be a fixed memory region, or the
 * allocator would request a memory region from the operating system (see the
 * C standard function sbrk(), for example).
 */
void mm_init(int memory_size) {
    /*
     * Allocate the entire memory pool, from which our simple allocator will
     * serve allocation requests.
     */
    assert(memory_size > 0);
    MEMORY_SIZE = memory_size;
    mem = malloc(MEMORY_SIZE);

    if (mem == NULL) {
        fprintf(stderr,
                "init_malloc: could not get %d bytes from the system\n",
                MEMORY_SIZE);
        abort();
    }

    freeptr = mem;

    /* Start out with no references in our reference-table. */
    ref_table = NULL;
    num_refs = 0;
    max_refs = 0;


}


/*! Returns true if the specified address is within the memory pool. */
bool is_pool_address(void *addr) {
    return ((unsigned char *) addr >= mem &&
            (unsigned char *) addr < mem + MEMORY_SIZE);
}


/*! Returns true if the pool has the requested amount of space available. */
bool has_space_available(int requested) {
    return (freeptr + requested <= mem + MEMORY_SIZE);
}


/*!
 * Attempt to allocate a chunk of memory of "size" bytes.  Return 0 if
 * allocation fails.
 */
Value * mm_malloc(ValueType type, int data_size) {
    // Actually, this should always be > 0 since even empty strings need a
    // NUL-terminator.  But, we will stick with >= 0 for now.
    assert(data_size >= 0);

    if (type == VAL_INTEGER) {
        data_size = sizeof(IntegerValue) - sizeof(struct Value);
    } else if (type == VAL_FLOAT) {
        data_size = sizeof(FloatValue) - sizeof(struct Value);
    } else if (type == VAL_LIST_NODE) {
        data_size = sizeof(ListValue) - sizeof(struct Value);
    } else if (type == VAL_DICT_NODE) {
        data_size = sizeof(DictValue) - sizeof(struct Value);
    }

    int requested = sizeof(struct Value) + data_size;
    Value *new_value = NULL;

    // If we don't have space, this might work.
    if (!has_space_available(requested))
        collect_garbage();

    if (has_space_available(requested)) {

        /* Initialize the new Value in the bytes beginning at freeptr. */
        new_value = (Value *) freeptr;

        /* Assign a Reference to it; the Value will know its Reference. */
        make_reference(new_value);

        new_value->type = type;
        new_value->data_size = data_size;
        new_value->marked = 0;

        /* Set the data area to a pattern so that it's easier to debug. */
        memset(new_value + 1, 0xCC, data_size);

        /* Update the free pointer to point past the new Value. */
        freeptr += requested;
    } else {
        fprintf(stderr, "mm_malloc: cannot service request of size %d with"
                " %d bytes allocated\n", requested, (int) (freeptr - mem));
        exit(1);
    }

    return new_value;
}


/*! Allocates an available reference in the ref_table. */
Reference make_reference(Value *value) {
    int i;
    Reference ref;
    Value **new_table;

    assert(value != NULL);

    /* If we don't have a reference table yet, allocate one. */
    if (ref_table == NULL) {
        ref_table = malloc(sizeof(Value *) * INITIAL_SIZE);
        max_refs = INITIAL_SIZE;

        // Set all new reference entries to NULL, just to be safe/clean.
        for (i = 0; i < max_refs; i++) {
            ref_table[i] = NULL;
        }
    }

    /* Scan through the reference table to see if we have any unused slots
     * that we can use for this value.
     */
    for (i = 0; i < num_refs; i++) {
        if (ref_table[i] == NULL) {
            ref = (Reference) i;  // Probably unnecessary, but clearer.
            ref_table[i] = value;
            value->ref = ref;
            return ref;
        }
    }

    /* If we got here, we don't have any available slots.  Find out if
     * this is because we ran out of space in the reference table.
     */

    if (num_refs == max_refs) {
        /* Double the size of the reference table. */
        max_refs *= 2;
        new_table = realloc(ref_table, sizeof(Value *) * max_refs);
        if (new_table == NULL) {
            error("out of memory");
            exit(1);
        }
        ref_table = new_table;

        // Set all new reference entries to NULL, just to be safe/clean.
        for (i = num_refs; i < max_refs; i++) {
            ref_table[i] = NULL;
        }
    }

    /* This becomes the new reference. */
    ref = (Reference) num_refs;  // Probably unnecessary, but clearer.
    num_refs++;

    ref_table[ref] = value;
    value->ref = ref;
    return ref;
}


/*!
 * Dereferences a Reference into a Value-pointer so the value can be
 * accessed.
 *
 * A Reference of NULL_REF will cause this function to return NULL.
 */
Value * deref(Reference ref) {
    Value *pval = NULL;

    if (ref == NULL_REF)
        return NULL;

    // Make sure the reference is actually a valid index.
    assert(ref >= 0 && ref < num_refs);

    // Make sure the reference refers to a valid entry.  Unused entries
    // will be set to NULL.
    pval = ref_table[ref];
    assert(pval != NULL);

    // Make sure the reference's value is within the pool!
    assert(is_pool_address(pval));

    return pval;
}


/*! Get the amount of in-use memory. */
int memuse() {
    return freeptr - mem;
}


/*! Print all allocated objects and free regions in the pool. */
void memdump() {
    unsigned char *curr = mem;

    while (curr < freeptr) {
        Value *curr_value = (Value *) curr;
        int data_size = curr_value->data_size;
        int value_size = sizeof(Value) + data_size;
        Reference ref = curr_value->ref;

        fprintf(stdout, "Value 0x%08x; size %d; ref %d; marked %d; ",
            (int) (curr - mem), (int) sizeof(Value) + data_size, ref,
            curr_value->marked);

        switch (curr_value->type) {
            case VAL_NONE:
                fprintf(stdout, "type = VAL_NONE; value = None\n");
                break;

            case VAL_BOOL:
                fprintf(stdout, "type = VAL_BOOL; value = %s\n",
                            ref_is_true(ref) ? "True" : "False");
                break;

            case VAL_INTEGER:
                fprintf(stdout, "type = VAL_INTEGER: value = %d\n",
                    ((IntegerValue *) curr_value)->integer_value);
                break;

            case VAL_FLOAT:
                fprintf(stdout, "type = VAL_FLOAT; value = %f\n",
                    ((FloatValue *) curr_value)->float_value);
                break;

            case VAL_STRING:
                fprintf(stdout, "type = VAL_STRING; value = \"%s\"\n",
                    ((StringValue *) curr_value)->string_value);
                break;

            case VAL_LIST_NODE: {
                ListValue *lv = (ListValue *) curr_value;
                fprintf(stdout,
                    "type = VAL_LIST_NODE; value_ref = %d; next_ref = %d\n",
                    lv->list_node.value, lv->list_node.next);
                break;
            }

            case VAL_DICT_NODE: {
                DictValue *dv = (DictValue *) curr_value;
                fprintf(stdout,
                    "type = VAL_DICT_NODE; key_ref = %d; value_ref = %d; "
                    "next_ref = %d\n",
                    dv->dict_node.key, dv->dict_node.value, dv->dict_node.next);
                break;
            }

            default:
                fprintf(stdout,
                        "type = UNKNOWN; the memory pool is probably corrupt\n");
        }

        curr += value_size;
    }
    fprintf(stdout, "Free  0x%08x; size %lu\n", (int) (freeptr - mem),
        MEMORY_SIZE - (freeptr - mem));
}

/* collect_garbage
 * Input: nothing
 * Output: Int of number of bytes reclaimed
 * This function goes through the heap and decides what memory is not being used
 * and frees it. The number of bytes freed is returned.
 */
int collect_garbage(void) {
    unsigned char *old_freeptr = freeptr;
    int reclaimed;

    if (!quiet) {
        fprintf(stderr, "Collecting garbage.\n");
    }

    // marking phase
    foreach_global(marker);

    // sweeping and compacting phase
    sweeper_and_compactor();
    
    reclaimed = (int) (old_freeptr - freeptr);

    if (!quiet) {
        // Ths will report how many bytes we were able to free in this garbage
        // collection pass.
        fprintf(stderr, "Reclaimed %d bytes of garbage.\n", reclaimed);
    }

    return reclaimed;
}

/*!
 * Clean up the allocator state.
 * All this really has to do is free the user memory pool. This function mostly
 * ensures that the test program doesn't leak memory, so it's easy to check
 * if the allocator does.
 */
void mm_cleanup(void) {
    free(mem);
    mem = NULL;
}

