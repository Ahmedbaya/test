#define MOKP_DLL_EXPORTS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>

// Platform compatibility
#ifndef _WIN32
    // Define Windows-specific functions for Linux compatibility
    #define _strdup strdup
#endif

#include "mokp_dll.h"
#include "Common.h"
#include "IBMOLS.h"

// Global state variables (from original main.c)
static int g_initialized = 0;
static char g_last_error[256] = {0};
static double g_last_iteration_time = 0.0;
static MOKP_Parameters g_params = {
    .max_iterations = 100,
    .population_size = 10,
    .archive_size = 28000,
    .perturbation_rate = 0.05,
    .kappa = 0.05,
    .num_objectives = 2,
    .num_items = 250,
    .local_search_depth = 5
};

// External declarations from main.c
extern double capacities[];
extern int weights[][250];  // NBITEMS
extern int profits[][250]; // NBITEMS
extern int nf, ni, cardP;
extern pop *archive;
extern pop *P;
extern pop *solutions;
extern double perturbation_rate;
extern double vector_weight[];
extern double max_bound;
extern double OBJ_Weights[][10000];
extern int nombreLIGNE;
extern int nextLn;
extern int inv;
extern int alpha;
extern double kappa;

// Function declarations from main.c
extern void loadMOKP(char *s);
extern void read_weights_file(char *s);
extern pop* create_pop(int maxsize, int nf);
extern ind* create_ind(int nf);
extern void complete_free_pop(pop *pp);
extern void free_ind(ind *p_ind);
extern void P_init_pop(pop *SP, pop *Sarchive, int alpha);
extern void Indicator_local_search1(pop *SP, pop *Sarchive, int size);
extern int extractPtoArchive(pop *P, pop *archive);
extern void calcul_weight(pop *SP, int size);
extern void calcMaxbound(pop* SP, int size);
extern void compute_all_fitness(pop *SP);
extern void choose_weight();
extern void seed(unsigned int seed);

// Helper function to set error message
static void set_error(const char* error_msg) {
    strncpy(g_last_error, error_msg, sizeof(g_last_error) - 1);
    g_last_error[sizeof(g_last_error) - 1] = '\0';
}

// Helper function to validate initialization
static int check_initialized() {
    if (!g_initialized) {
        set_error("MOKP library not initialized");
        return 0;
    }
    return 1;
}

// Core DLL interface functions
MOKP_API int MOKP_CALL mokp_initialize() {
    if (g_initialized) {
        return MOKP_SUCCESS;
    }
    
    // Clear error state
    g_last_error[0] = '\0';
    
    // Initialize global variables to match main.c defaults
    nf = g_params.num_objectives;
    ni = g_params.num_items;
    alpha = g_params.population_size;
    perturbation_rate = g_params.perturbation_rate;
    kappa = g_params.kappa;
    
    // Initialize random seed
    seed((unsigned int)time(NULL));
    
    // Initialize global pointers to NULL
    archive = NULL;
    P = NULL;
    solutions = NULL;
    
    // Initialize counters
    nombreLIGNE = 0;
    nextLn = 0;
    inv = 0;
    
    g_initialized = 1;
    return MOKP_SUCCESS;
}

MOKP_API int MOKP_CALL mokp_cleanup() {
    if (!g_initialized) {
        return MOKP_SUCCESS;
    }
    
    // Free allocated populations
    if (archive) {
        complete_free_pop(archive);
        archive = NULL;
    }
    if (P) {
        complete_free_pop(P);
        P = NULL;
    }
    if (solutions) {
        complete_free_pop(solutions);
        solutions = NULL;
    }
    
    g_initialized = 0;
    g_last_error[0] = '\0';
    return MOKP_SUCCESS;
}

MOKP_API int MOKP_CALL mokp_load_problem(const char* problem_file) {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (!problem_file) {
        set_error("Invalid problem file path");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    // Use the existing loadMOKP function
    loadMOKP((char*)problem_file);
    
    // Update parameters with actual loaded values
    g_params.num_objectives = nf;
    g_params.num_items = ni;
    
    return MOKP_SUCCESS;
}

MOKP_API int MOKP_CALL mokp_load_weights(const char* weights_file) {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (!weights_file) {
        set_error("Invalid weights file path");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    // Use the existing read_weights_file function
    read_weights_file((char*)weights_file);
    return MOKP_SUCCESS;
}

MOKP_API int MOKP_CALL mokp_set_parameters(const MOKP_Parameters* params) {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (!params) {
        set_error("Invalid parameters pointer");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    // Validate parameters
    if (params->num_objectives < 1 || params->num_objectives > MOKP_MAX_OBJECTIVES) {
        set_error("Invalid number of objectives");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    if (params->num_items < 1 || params->num_items > MOKP_MAX_ITEMS) {
        set_error("Invalid number of items");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    if (params->population_size < 1) {
        set_error("Invalid population size");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    // Update global parameters
    g_params = *params;
    nf = params->num_objectives;
    ni = params->num_items;
    alpha = params->population_size;
    perturbation_rate = params->perturbation_rate;
    kappa = params->kappa;
    
    return MOKP_SUCCESS;
}

MOKP_API int MOKP_CALL mokp_get_parameters(MOKP_Parameters* params) {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (!params) {
        set_error("Invalid parameters pointer");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    *params = g_params;
    return MOKP_SUCCESS;
}

MOKP_API int MOKP_CALL mokp_initialize_population() {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    // Create main archive if not exists
    if (!P) {
        P = create_pop(g_params.archive_size, nf);
        if (!P) {
            set_error("Failed to create population");
            return MOKP_ERROR_MEMORY_ALLOCATION;
        }
        P->size = 0;
    }
    
    return MOKP_SUCCESS;
}

MOKP_API int MOKP_CALL mokp_run_single_iteration() {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    clock_t start = clock();
    
    // Create temporary populations
    solutions = create_pop(alpha, nf);
    archive = create_pop(g_params.archive_size, nf);
    
    if (!solutions || !archive) {
        set_error("Failed to create temporary populations");
        if (solutions) complete_free_pop(solutions);
        if (archive) complete_free_pop(archive);
        solutions = NULL;
        archive = NULL;
        return MOKP_ERROR_MEMORY_ALLOCATION;
    }
    
    // Choose weight vector
    choose_weight();
    
    // Initialize population from Pareto archive
    P_init_pop(solutions, P, alpha);
    
    // Extract Pareto archive
    extractPtoArchive(solutions, P);
    
    // Compute weights and fitness
    calcul_weight(solutions, alpha);
    calcMaxbound(solutions, alpha);
    compute_all_fitness(solutions);
    
    // Run indicator-based local search
    Indicator_local_search1(solutions, archive, alpha);
    
    // Update main Pareto archive
    extractPtoArchive(archive, P);
    
    // Cleanup temporary populations
    complete_free_pop(solutions);
    complete_free_pop(archive);
    solutions = NULL;
    archive = NULL;
    
    clock_t finish = clock();
    g_last_iteration_time = (double)(finish - start) / CLOCKS_PER_SEC;
    
    return MOKP_SUCCESS;
}

MOKP_API int MOKP_CALL mokp_run_optimization(int max_iterations) {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (max_iterations < 1) {
        set_error("Invalid number of iterations");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    // Initialize population if needed
    if (!P) {
        int result = mokp_initialize_population();
        if (result != MOKP_SUCCESS) {
            return result;
        }
    }
    
    // Run optimization loop
    for (int i = 0; i < max_iterations; i++) {
        int result = mokp_run_single_iteration();
        if (result != MOKP_SUCCESS) {
            return result;
        }
    }
    
    return MOKP_SUCCESS;
}

MOKP_API int MOKP_CALL mokp_get_pareto_size() {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (!P) {
        return 0;
    }
    
    return P->size;
}

MOKP_API int MOKP_CALL mokp_get_pareto_solutions(MOKP_Solution* solutions_out, int max_solutions) {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (!solutions_out || max_solutions < 1) {
        set_error("Invalid solutions array or size");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    if (!P || P->size == 0) {
        return 0;
    }
    
    int count = (P->size < max_solutions) ? P->size : max_solutions;
    
    for (int i = 0; i < count; i++) {
        ind* individual = P->ind_array[i];
        MOKP_Solution* sol = &solutions_out[i];
        
        // Copy objectives
        for (int j = 0; j < nf && j < MOKP_MAX_OBJECTIVES; j++) {
            sol->objectives[j] = individual->f[j];
            sol->capacities[j] = individual->capa[j];
        }
        
        // Copy items selection
        sol->num_items_selected = 0;
        for (int j = 0; j < ni && j < MOKP_MAX_ITEMS; j++) {
            sol->items[j] = individual->Items[j];
            if (individual->Items[j] == 1) {
                sol->num_items_selected++;
            }
        }
    }
    
    return count;
}

MOKP_API int MOKP_CALL mokp_get_population_size() {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    return alpha;
}

MOKP_API int MOKP_CALL mokp_evaluate_solution(const int* items, MOKP_Solution* solution) {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (!items || !solution) {
        set_error("Invalid items array or solution pointer");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    // Initialize solution
    memset(solution, 0, sizeof(MOKP_Solution));
    
    // Calculate objectives and capacities
    for (int i = 0; i < ni && i < MOKP_MAX_ITEMS; i++) {
        solution->items[i] = items[i];
        if (items[i] == 1) {
            solution->num_items_selected++;
            for (int j = 0; j < nf && j < MOKP_MAX_OBJECTIVES; j++) {
                solution->objectives[j] += profits[j][i];
                solution->capacities[j] += weights[j][i];
            }
        }
    }
    
    return MOKP_SUCCESS;
}

MOKP_API int MOKP_CALL mokp_is_solution_feasible(const int* items) {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (!items) {
        set_error("Invalid items array");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    // Check capacity constraints
    for (int j = 0; j < nf; j++) {
        double total_weight = 0.0;
        for (int i = 0; i < ni; i++) {
            if (items[i] == 1) {
                total_weight += weights[j][i];
            }
        }
        if (total_weight > capacities[j]) {
            return 0; // Not feasible
        }
    }
    
    return 1; // Feasible
}

MOKP_API double MOKP_CALL mokp_get_last_iteration_time() {
    return g_last_iteration_time;
}

MOKP_API int MOKP_CALL mokp_get_problem_info(int* num_objectives, int* num_items) {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (num_objectives) {
        *num_objectives = nf;
    }
    if (num_items) {
        *num_items = ni;
    }
    
    return MOKP_SUCCESS;
}

MOKP_API const char* MOKP_CALL mokp_get_last_error() {
    return g_last_error;
}

MOKP_API int MOKP_CALL mokp_get_algorithm_state() {
    if (!g_initialized) {
        return 0; // Not initialized
    }
    
    if (!P) {
        return 1; // Initialized but no population
    }
    
    return 2; // Ready to run
}

MOKP_API int MOKP_CALL mokp_save_results(const char* output_file) {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (!output_file) {
        set_error("Invalid output file path");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    if (!P || P->size == 0) {
        set_error("No solutions to save");
        return MOKP_ERROR_INVALID_STATE;
    }
    
    FILE* file = fopen(output_file, "w");
    if (!file) {
        set_error("Cannot open output file for writing");
        return MOKP_ERROR_FILE_NOT_FOUND;
    }
    
    for (int i = 0; i < P->size; i++) {
        for (int j = 0; j < nf; j++) {
            fprintf(file, "%f ", P->ind_array[i]->f[j]);
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    return MOKP_SUCCESS;
}

// Advanced functions - wrappers for specific algorithm components
MOKP_API int MOKP_CALL mokp_indicator_local_search() {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (!solutions || !archive) {
        set_error("Population not properly initialized for local search");
        return MOKP_ERROR_INVALID_STATE;
    }
    
    Indicator_local_search1(solutions, archive, alpha);
    return MOKP_SUCCESS;
}

MOKP_API int MOKP_CALL mokp_extract_pareto_archive() {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (!solutions || !P) {
        set_error("Population not properly initialized for Pareto extraction");
        return MOKP_ERROR_INVALID_STATE;
    }
    
    int convergence_rate = extractPtoArchive(solutions, P);
    return convergence_rate;
}

MOKP_API int MOKP_CALL mokp_update_weights() {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    choose_weight();
    return MOKP_SUCCESS;
}

MOKP_API int MOKP_CALL mokp_compute_fitness() {
    if (!check_initialized()) {
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    
    if (!solutions) {
        set_error("Population not initialized for fitness computation");
        return MOKP_ERROR_INVALID_STATE;
    }
    
    calcul_weight(solutions, alpha);
    calcMaxbound(solutions, alpha);
    compute_all_fitness(solutions);
    return MOKP_SUCCESS;
}