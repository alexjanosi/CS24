#include "accum.h"
#include "cmp_swap.h"


/* Adds "value" to *accum, returning the new total that is held in *accum as
 * a result of this particular addition (i.e. irrespective of other concurrent
 * additions that may be occurring).
 *
 * This function should be thread-safe, allowing calls from multiple threads
 * without generating spurious results.
 */
uint32_t add_to_accum(uint32_t *accum, uint32_t value) {
   
     // declare variables
    unsigned int success = 0;
    unsigned int old;

    // while we are unsuccessful
    while (!success) {
        // get value of pointer
        old = *accum;
        // use compare_and_swap to safely update
        success = compare_and_swap(accum, old, old + value);
    }
    // return new sum
    return *accum;
}

