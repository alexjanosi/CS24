#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "config.h"
#include "records.h"
#include "realtime.h"


#define KEY_PREFIX_SIZE 8


/*!
 * A "record info" struct, which holds a part of the record's key, as well
 * as a pointer to the record itself.
 */
typedef struct recinfo_t {
    char key_prefix[KEY_PREFIX_SIZE];    //!< The prefix of the record's key
    record_t *record;                    //!< A pointer to the record itself
} recinfo_t;


/*!
 * Compare two records, returning <0, 0 or >0 if record a is less than,
 * equal to, or greater than record b.
 *
 * The records are referenced by "record info" structs.
 */
int compare_recinfos(const void *a, const void *b) {
    recinfo_t *rec_a = (recinfo_t *) a;
    recinfo_t *rec_b = (recinfo_t *) b;

    // memcpy is faster than checking bit by bit
    return memcmp(rec_a->key_prefix, rec_b->key_prefix, KEY_PREFIX_SIZE);
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

        // Make an array of record-info structs based on the records.
        recinfo_t *recinfos = malloc(num_records * sizeof(recinfo_t));
        if (recinfos == NULL) {
            fprintf(stderr, "Couldn't allocate record-info array.\n");
            exit(1);
        }
        for (int i = 0; i < num_records; i++) {
            memcpy(recinfos[i].key_prefix, records[i].key, KEY_PREFIX_SIZE);
            recinfos[i].record = records + i;
        }

        uint64_t start = rdtsc();
        qsort(recinfos, num_records, sizeof(recinfo_t), compare_recinfos);
        uint64_t end = rdtsc();

        free(recinfos);
        free(records);

        total += end - start;
    }

    uint64_t avg = total / num_repeats;
    return avg;
}


/*! Main entry point for the program. */
int main() {
    srandom(RANDOM_SEED);

    fprintf(stderr, "Sorting an array of record-info objects\n");

    printf("N\tTotal Time\tClocks per Element\n");

    for (int num_records = N_START; num_records <= N_END; num_records += N_STEP) {
        uint64_t avg_time = sort_perf(num_records, NUM_REPEATS);
        float avg_cpe = (float) avg_time / (float) num_records;
        printf("%d\t%" PRIu64 "\t%.2f\n", num_records, avg_time, avg_cpe);
    }

    return 0;
}

