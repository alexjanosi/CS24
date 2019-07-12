#define RANDOM_SEED 12345


/* SMALLER TRENDS */

/*!
 * The number of times to repeat each performance test to try to reduce
 * noise in the results.
 */
#define NUM_REPEATS 20

/*! The starting collection size for performance testing. */
#define N_START 1

/*! The ending collection size (inclusive) for performance testing. */
#define N_END 10000

/*! The step size for performance testing. */
#define N_STEP 1


/* MEDIUM TRENDS

#define NUM_REPEATS 5

#define N_START 32
#define N_END 131072
#define N_STEP 32

*/


/* LONGER TRENDS

#define NUM_REPEATS 5

#define N_START 500
#define N_END 1000000
#define N_STEP 500

*/

