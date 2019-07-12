#include <stdio.h>
#include <stdlib.h>
#include "cmp_swap.h"


/* This function takes in the inputs to compare_and_swap
 * and makes sure the correct results are obtained
 */
void tester(uint32_t *target, uint32_t old, uint32_t new) {

	// value to know if we should switch
	int test = (*target == old);

	// print initial conditions
	printf("Target: %d\n", *target);
	printf("Old: %d\n", old);
	printf("New: %d\n", new);

	// runs the function on the inputs
	int result = compare_and_swap(target, old, new);

	// run tests
	if (test) {
		// test values swapped
		printf("The values were equal and new should be in target\n");
		printf("The value of target is now: %d\n", *target);

		if (*target == new) {
			printf("Swap successful\n");
		}
		else {
			printf("Swap unsuccessful\n");
		}

		// test return value
		printf("The return value should be one: %d\n", result);

		if (result == 1) {
			printf("Result was correct\n");
		}
		else {
			printf("Result was incorrect\n");
		}
	}
	else {
		// tests values did not swap
		printf("The values were not equal and new should not be in target\n");
		printf("The value of target is now: %d\n", *target);

		if (*target == new) {
			printf("Values are incorrect\n");
		}
		else {
			printf("Values are correct\n");
		}

		// test return value
		printf("The return value should be zero: %d\n", result);

		if (result == 0) {
			printf("Result was correct\n");
		}
		else {
			printf("Result was incorrect\n");
		}
	}

	printf("Test complete\n\n");
}

// main creates some tests and runs the function
int main() {
	unsigned int old1 = 8;
	unsigned int target1 = 8;
	unsigned int new1 = 12;

    // should not do anything and return zero
	tester(&target1, old1, new1);

    unsigned int old2 = 9;
	unsigned int target2 = 14;
	unsigned int new2 = 15;
    
    // switch should happen
    tester(&target2, old2, new2);

    return 0;
}

