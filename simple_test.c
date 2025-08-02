#include <stdio.h>
#include <stdlib.h>
#include "knapsack_dll.h"

int main() {
    printf("Simple Knapsack DLL Test\n");
    printf("========================\n");
    
    /* Test basic functions that don't require complex memory management */
    printf("Library version: %s\n", knapsack_get_version());
    
    /* Test default parameters */
    knapsack_params_t params = knapsack_default_params();
    printf("Default parameters:\n");
    printf("  Population size: %d\n", params.population_size);
    printf("  Max iterations: %d\n", params.max_iterations);
    printf("  Perturbation rate: %.3f\n", params.perturbation_rate);
    printf("  Kappa: %.3f\n", params.kappa);
    printf("  Local search depth: %d\n", params.local_search_depth);
    
    /* Try to load a file (should fail gracefully) */
    printf("\nTesting file loading (expected to fail)...\n");
    knapsack_problem_t *problem = knapsack_load_problem("nonexistent.txt");
    if (!problem) {
        printf("File loading failed as expected: %s\n", knapsack_get_error());
    } else {
        printf("Unexpected success loading file\n");
        knapsack_free_problem(problem);
    }
    
    printf("\nBasic tests completed successfully!\n");
    return 0;
}