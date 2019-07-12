#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "records.h"


/*! Generates the data for a single record into the specified memory. */
void generate_record(record_t *rec) {
    for (int i = 0; i < KEY_SIZE; i++)
        rec->key[i] = 'A' + (char) (random() % KEY_VALUES);

    for (int i = 0; i < (VALUE_SIZE); i++)
        rec->value[i] = '0' + (char) (random() % 10);
}


/*!
 * Generates the data for the specified number of records into a newly
 * heap-allocated region of memory.
 */
record_t * generate_records(unsigned int num_records) {
    record_t *result;

    // Allocate the chunk of memory we need.

    size_t size = num_records * sizeof(record_t);
    result = (record_t *) malloc(size);
    if (result == 0) {
        fprintf(stderr, "Failed to allocate %zu bytes for %u records!\n",
            size, num_records);
        return NULL;
    }

    /* Generate data for each record in the collection. */
    for (int i = 0; i < num_records; i++)
        generate_record(result + i);

    return result;
}


