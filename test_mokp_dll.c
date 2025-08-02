#include <stdio.h>
#include <stdlib.h>
#include "mokp_dll.h"

void print_solution(int index, double* objectives, int* decision_vars, int num_objectives, int num_items) {
    printf("Solution %d:\n", index);
    printf("  Objectives: ");
    for (int i = 0; i < num_objectives; i++) {
        printf("%.2f ", objectives[i]);
    }
    printf("\n");
    
    printf("  Selected items: ");
    int count = 0;
    for (int i = 0; i < num_items && count < 10; i++) {  // Show first 10 selected items
        if (decision_vars[i] == 1) {
            printf("%d ", i);
            count++;
        }
    }
    if (count == 10) printf("...");
    printf("\n\n");
}

int main() {
    printf("Testing MOKP DLL Wrapper\n");
    printf("========================\n\n");
    
    // Test 1: Initialize optimizer
    printf("1. Initializing optimizer...\n");
    int result = InitializeOptimizer();
    if (result != MOKP_SUCCESS) {
        printf("   Failed: %s\n", GetErrorMessage(result));
        return 1;
    }
    printf("   Success!\n\n");
    
    // Test 2: Load problem
    printf("2. Loading problem file...\n");
    result = LoadProblem("250.2.txt");
    if (result != MOKP_SUCCESS) {
        printf("   Failed: %s\n", GetErrorMessage(result));
        Cleanup();
        return 1;
    }
    printf("   Success!\n\n");
    
    // Test 3: Get problem info
    printf("3. Getting problem information...\n");
    int num_objectives, num_items;
    result = GetProblemInfo(&num_objectives, &num_items);
    if (result != MOKP_SUCCESS) {
        printf("   Failed: %s\n", GetErrorMessage(result));
        Cleanup();
        return 1;
    }
    printf("   Problem has %d objectives and %d items\n\n", num_objectives, num_items);
    
    // Test 4: Set parameters
    printf("4. Setting parameters...\n");
    result = SetParameters(10, 5, 0.05);  // Small values for quick test
    if (result != MOKP_SUCCESS) {
        printf("   Failed: %s\n", GetErrorMessage(result));
        Cleanup();
        return 1;
    }
    printf("   Parameters set: population=10, iterations=5, perturbation=0.05\n\n");
    
    // Test 5: Run optimization
    printf("5. Running optimization...\n");
    result = RunOptimization();
    if (result != MOKP_SUCCESS) {
        printf("   Failed: %s\n", GetErrorMessage(result));
        Cleanup();
        return 1;
    }
    printf("   Optimization completed!\n\n");
    
    // Test 6: Get results
    printf("6. Getting results...\n");
    int result_count = GetResultCount();
    if (result_count < 0) {
        printf("   Failed to get result count\n");
        Cleanup();
        return 1;
    }
    printf("   Found %d solutions in Pareto front\n\n", result_count);
    
    if (result_count > 0) {
        // Allocate memory for getting individual results
        double* objectives = (double*)malloc(num_objectives * sizeof(double));
        int* decision_vars = (int*)malloc(num_items * sizeof(int));
        
        // Show first few solutions
        int max_show = (result_count < 5) ? result_count : 5;
        printf("7. Showing first %d solutions:\n", max_show);
        
        for (int i = 0; i < max_show; i++) {
            result = GetResult(i, objectives, decision_vars);
            if (result == MOKP_SUCCESS) {
                print_solution(i, objectives, decision_vars, num_objectives, num_items);
            } else {
                printf("   Failed to get solution %d: %s\n", i, GetErrorMessage(result));
            }
        }
        
        free(objectives);
        free(decision_vars);
    }
    
    // Test 7: Get all results using MOKPResults structure
    printf("8. Testing MOKPResults structure...\n");
    MOKPResults results;
    result = GetResults(&results);
    if (result == MOKP_SUCCESS) {
        printf("   Successfully retrieved %d solutions using MOKPResults\n", results.count);
        FreeResults(&results);
    } else {
        printf("   Failed: %s\n", GetErrorMessage(result));
    }
    
    // Test 8: Cleanup
    printf("\n9. Cleaning up...\n");
    Cleanup();
    printf("   Cleanup completed!\n\n");
    
    printf("All tests completed successfully!\n");
    return 0;
}