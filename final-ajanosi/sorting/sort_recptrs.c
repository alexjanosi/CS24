#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "config.h"
#include "records.h"
#include "realtime.h"


typedef record_t * record_ptr_t;


/*!
 * Compare two records, returning <0, 0 or >0 if record a is less than,
 * equal to, or greater than record b.
 *
 * The array being sorted is an array of record-pointers, so this function
 * receives two record-pointer-pointers.  This requires two dereferences to
 * get to the record data itself.
 */
int compare_record_ptrs(const void *a, const void *b) {
    record_ptr_t *p_rec_a = (record_ptr_t *) a;
    record_ptr_t *p_rec_b = (record_ptr_t *) b;

    // Dereference each record-pointer-pointer, to get to the key data.
    return memcmp((*p_rec_a)->key, (*p_rec_b)->key, KEY_SIZE);
}


/*! 
 * Runs the performance test for a given number of records and a given
 * number of repetitions.  Returns the average total time taken.
 */
uint64_t sort_perf(int num_records, int num_repeats) {
    uint64_t total = 0;
    for (int rep = 0; rep < num_repeats; rep++) {
        // Generate the records themselves in a contiguous memory chunk.
        record_t *records = generate_records(num_records);

        // Make an array of record-pointers, so that we don't have to move the
        // records themselves around in memory.
        record_ptr_t *recptrs = malloc(num_records * sizeof(record_ptr_t));
        if (recptrs == NULL) {
            fprintf(stderr, "Couldn't allocate array of record-pointers.\n");
            exit(1);
        }
        for (int i = 0; i < num_records; i++) {
            recptrs[i] = records + i;
        }

        uint64_t start = rdtsc();
        qsort(recptrs, num_records, sizeof(record_ptr_t), compare_record_ptrs);
        uint64_t end = rdtsc();

        free(recptrs);
        free(records);

        total += end - start;
    }

    uint64_t avg = total / num_repeats;
    return avg;
}


/*! Main entry point for the program. */
int main() {
    srandom(RANDOM_SEED);

    fprintf(stderr, "Sorting an array of pointers to records\n");

    printf("N\tTotal Time\tClocks per Element\n");

    for (int num_records = N_START; num_records <= N_END; num_records += N_STEP) {
        uint64_t avg_time = sort_perf(num_records, NUM_REPEATS);
        float avg_cpe = (float) avg_time / (float) num_records;
        printf("%d\t%" PRIu64 "\t%.2f\n", num_records, avg_time, avg_cpe);
    }

    return 0;
}

