#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "config.h"
#include "records.h"
#include "realtime.h"


/*!
 * Compare two records, returning <0, 0 or >0 if record a is less than,
 * equal to, or greater than record b.
 */
int compare_records(const void *a, const void *b) {
    record_t *rec_a = (record_t *) a;
    record_t *rec_b = (record_t *) b;

    return memcmp(rec_a->key, rec_b->key, KEY_SIZE);
}


/*!
 * Runs the performance test for a given number of records and a given
 * number of repetitions.  Returns the average total time taken.
 */
uint64_t sort_perf(int num_records, int num_repeats) {
    uint64_t total = 0;
    for (int rep = 0; rep < num_repeats; rep++) {
        // Generate the array of records to be sorted.
        record_t *records = generate_records(num_records);

        uint64_t start = rdtsc();
        qsort(records, num_records, sizeof(record_t), compare_records);
        uint64_t end = rdtsc();

        free(records);

        total += end - start;
    }

    uint64_t avg = total / num_repeats;
    return avg;
}


/*! Main entry point for the program. */
int main() {
    srandom(RANDOM_SEED);

    fprintf(stderr, "Sorting an array of records\n");

    printf("N\tTotal Time\tClocks per Element\n");

    for (int num_records = N_START; num_records <= N_END; num_records += N_STEP) {
        uint64_t avg_time = sort_perf(num_records, NUM_REPEATS);
        float avg_cpe = (float) avg_time / (float) num_records;
        printf("%d\t%" PRIu64 "\t%.2f\n", num_records, avg_time, avg_cpe);
    }

    return 0;
}

