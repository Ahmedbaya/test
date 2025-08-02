#include <stdio.h>
#include <stdlib.h>
#include "knapsack_dll.h"

int main() {
    printf("Multi-Objective Knapsack DLL Test\n");
    printf("==================================\n");
    
    /* Test version function */
    printf("Library version: %s\n", knapsack_get_version());
    
    /* Test default parameters */
    knapsack_params_t params = knapsack_default_params();
    printf("Default parameters:\n");
    printf("  Population size: %d\n", params.population_size);
    printf("  Max iterations: %d\n", params.max_iterations);
    printf("  Perturbation rate: %.3f\n", params.perturbation_rate);
    printf("  Kappa: %.3f\n", params.kappa);
    printf("  Local search depth: %d\n", params.local_search_depth);
    
    /* Create a simple test problem */
    printf("\nCreating test problem...\n");
    
    int num_items = 5;
    int num_objectives = 2;
    double capacities[] = {10.0, 15.0};
    
    /* Create weight and profit matrices */
    int *weights_data[2];
    int *profits_data[2];
    
    int weights_obj1[] = {2, 3, 4, 5, 1};
    int weights_obj2[] = {1, 2, 3, 4, 2};
    int profits_obj1[] = {3, 4, 5, 6, 2};
    int profits_obj2[] = {5, 6, 7, 8, 4};
    
    weights_data[0] = weights_obj1;
    weights_data[1] = weights_obj2;
    profits_data[0] = profits_obj1;
    profits_data[1] = profits_obj2;
    
    /* Create problem */
    knapsack_problem_t *problem = knapsack_create_problem(
        num_items, num_objectives, capacities, 
        (const int**)weights_data, (const int**)profits_data);
    
    if (!problem) {
        printf("Error creating problem: %s\n", knapsack_get_error());
        return 1;
    }
    
    printf("Problem created successfully!\n");
    printf("  Items: %d\n", num_items);
    printf("  Objectives: %d\n", num_objectives);
    printf("  Capacities: [%.1f, %.1f]\n", capacities[0], capacities[1]);
    
    /* Solve the problem */
    printf("\nSolving problem...\n");
    
    params.max_iterations = 20;
    params.population_size = 5;
    
    knapsack_result_t *result = NULL;
    int error_code = knapsack_solve(problem, &params, &result);
    
    if (error_code != KNAPSACK_SUCCESS) {
        printf("Error solving problem (code %d): %s\n", error_code, knapsack_get_error());
        knapsack_free_problem(problem);
        return 1;
    }
    
    printf("Problem solved successfully!\n");
    
    /* Display results */
    int solution_count = knapsack_get_solution_count(result);
    printf("Found %d Pareto-optimal solutions:\n", solution_count);
    
    for (int i = 0; i < solution_count; i++) {
        const knapsack_solution_t *solution = knapsack_get_solution(result, i);
        if (solution) {
            printf("  Solution %d:\n", i + 1);
            printf("    Items: [");
            for (int j = 0; j < num_items; j++) {
                printf("%d", solution->items[j]);
                if (j < num_items - 1) printf(", ");
            }
            printf("]\n");
            printf("    Objectives: [");
            for (int j = 0; j < num_objectives; j++) {
                printf("%.1f", solution->objectives[j]);
                if (j < num_objectives - 1) printf(", ");
            }
            printf("]\n");
            printf("    Capacity used: [");
            for (int j = 0; j < num_objectives; j++) {
                printf("%.1f", solution->capacities_used[j]);
                if (j < num_objectives - 1) printf(", ");
            }
            printf("]\n");
            
            /* Check feasibility */
            int feasible = knapsack_is_feasible(problem, solution);
            printf("    Feasible: %s\n", feasible == 1 ? "Yes" : (feasible == 0 ? "No" : "Error"));
        }
    }
    
    /* Test file loading if test problem exists */
    printf("\nTesting file loading...\n");
    if (result) {
        knapsack_free_result(result);
        result = NULL;
    }
    
    knapsack_problem_t *file_problem = knapsack_load_problem("250.2.txt");
    if (file_problem) {
        printf("Successfully loaded problem from file!\n");
        
        /* Solve file problem */
        error_code = knapsack_solve(file_problem, &params, &result);
        if (error_code == KNAPSACK_SUCCESS && result) {
            int file_solution_count = knapsack_get_solution_count(result);
            printf("File problem solved: %d solutions found\n", file_solution_count);
            knapsack_free_result(result);
        }
        
        knapsack_free_problem(file_problem);
    } else {
        printf("Could not load problem from file (this is normal if 250.2.txt doesn't exist)\n");
        printf("Error: %s\n", knapsack_get_error());
    }
    
    /* Cleanup */
    if (result) knapsack_free_result(result);
    knapsack_free_problem(problem);
    
    printf("\nTest completed successfully!\n");
    return 0;
}