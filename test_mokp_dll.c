#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mokp_dll.h"

void print_error_and_exit(const char* operation) {
    printf("Error in %s: %s\n", operation, mokp_get_last_error());
    mokp_cleanup();
    exit(1);
}

void print_solution(const MOKP_Solution* sol, int index) {
    printf("Solution %d:\n", index);
    printf("  Objectives: ");
    for (int i = 0; i < 4; i++) {
        printf("%.2f ", sol->objectives[i]);
    }
    printf("\n");
    printf("  Items selected: %d\n", sol->num_items_selected);
    printf("  Capacities used: ");
    for (int i = 0; i < 4; i++) {
        printf("%.2f ", sol->capacities[i]);
    }
    printf("\n\n");
}

int main() {
    printf("=== MOKP DLL Test Program ===\n\n");
    
    // Test 1: Initialize the library
    printf("Test 1: Initializing MOKP library...\n");
    int result = mokp_initialize();
    if (result != MOKP_SUCCESS) {
        print_error_and_exit("initialization");
    }
    printf("✓ Library initialized successfully\n\n");
    
    // Test 2: Get algorithm state
    printf("Test 2: Checking algorithm state...\n");
    int state = mokp_get_algorithm_state();
    printf("Algorithm state: %d\n", state);
    printf("✓ State check completed\n\n");
    
    // Test 3: Set parameters
    printf("Test 3: Setting algorithm parameters...\n");
    MOKP_Parameters params;
    params.max_iterations = 50;
    params.population_size = 10;
    params.archive_size = 1000;
    params.perturbation_rate = 0.05;
    params.kappa = 0.05;
    params.num_objectives = 2;
    params.num_items = 250;
    params.local_search_depth = 5;
    
    result = mokp_set_parameters(&params);
    if (result != MOKP_SUCCESS) {
        print_error_and_exit("parameter setting");
    }
    printf("✓ Parameters set successfully\n\n");
    
    // Test 4: Load problem file
    printf("Test 4: Loading problem file...\n");
    result = mokp_load_problem("250.2.txt");
    if (result != MOKP_SUCCESS) {
        print_error_and_exit("problem loading");
    }
    printf("✓ Problem file loaded successfully\n\n");
    
    // Test 5: Load weights file
    printf("Test 5: Loading weights file...\n");
    result = mokp_load_weights("Weights_2obj_FQ200.txt");
    if (result != MOKP_SUCCESS) {
        print_error_and_exit("weights loading");
    }
    printf("✓ Weights file loaded successfully\n\n");
    
    // Test 6: Get problem information
    printf("Test 6: Getting problem information...\n");
    int num_objectives, num_items;
    result = mokp_get_problem_info(&num_objectives, &num_items);
    if (result != MOKP_SUCCESS) {
        print_error_and_exit("getting problem info");
    }
    printf("Problem info: %d objectives, %d items\n", num_objectives, num_items);
    printf("✓ Problem info retrieved successfully\n\n");
    
    // Test 7: Initialize population
    printf("Test 7: Initializing population...\n");
    result = mokp_initialize_population();
    if (result != MOKP_SUCCESS) {
        print_error_and_exit("population initialization");
    }
    printf("✓ Population initialized successfully\n\n");
    
    // Test 8: Run optimization
    printf("Test 8: Running optimization (10 iterations)...\n");
    result = mokp_run_optimization(10);
    if (result != MOKP_SUCCESS) {
        print_error_and_exit("optimization");
    }
    printf("✓ Optimization completed successfully\n");
    printf("Last iteration time: %.4f seconds\n\n", mokp_get_last_iteration_time());
    
    // Test 9: Get Pareto solutions
    printf("Test 9: Retrieving Pareto solutions...\n");
    int pareto_size = mokp_get_pareto_size();
    printf("Number of Pareto solutions: %d\n", pareto_size);
    
    if (pareto_size > 0) {
        int max_to_show = (pareto_size < 5) ? pareto_size : 5;
        MOKP_Solution* solutions = malloc(max_to_show * sizeof(MOKP_Solution));
        
        if (solutions) {
            int retrieved = mokp_get_pareto_solutions(solutions, max_to_show);
            printf("Retrieved %d solutions (showing first %d):\n\n", retrieved, max_to_show);
            
            for (int i = 0; i < retrieved; i++) {
                print_solution(&solutions[i], i + 1);
            }
            
            free(solutions);
        }
    }
    printf("✓ Pareto solutions retrieved successfully\n\n");
    
    // Test 10: Test solution evaluation
    printf("Test 10: Testing solution evaluation...\n");
    int test_items[MOKP_MAX_ITEMS] = {0};
    // Set first 10 items as selected for testing
    for (int i = 0; i < 10 && i < num_items; i++) {
        test_items[i] = 1;
    }
    
    MOKP_Solution test_solution;
    result = mokp_evaluate_solution(test_items, &test_solution);
    if (result != MOKP_SUCCESS) {
        print_error_and_exit("solution evaluation");
    }
    
    printf("Test solution (first 10 items selected):\n");
    print_solution(&test_solution, 0);
    
    int feasible = mokp_is_solution_feasible(test_items);
    printf("Solution feasibility: %s\n", feasible ? "Feasible" : "Infeasible");
    printf("✓ Solution evaluation completed successfully\n\n");
    
    // Test 11: Save results
    printf("Test 11: Saving results to file...\n");
    result = mokp_save_results("test_results.txt");
    if (result != MOKP_SUCCESS) {
        print_error_and_exit("saving results");
    }
    printf("✓ Results saved successfully to test_results.txt\n\n");
    
    // Test 12: Cleanup
    printf("Test 12: Cleaning up...\n");
    result = mokp_cleanup();
    if (result != MOKP_SUCCESS) {
        printf("Error during cleanup: %s\n", mokp_get_last_error());
        return 1;
    }
    printf("✓ Cleanup completed successfully\n\n");
    
    printf("=== All tests completed successfully! ===\n");
    return 0;
}