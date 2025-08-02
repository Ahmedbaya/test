#define KNAPSACK_DLL_EXPORTS
#include "knapsack_dll.h"
#include "knapsack_core.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* Remove Windows-specific includes for cross-platform compatibility */
#ifdef _WIN32
    #include <windows.h>
#endif

/* Global constants and variables */
#define KNAPSACK_VERSION "1.0.0"
#define MAX_ERROR_MSG 256

static char last_error[MAX_ERROR_MSG] = "";

/* Local copies for thread safety */
static double *local_capacities = NULL;
static int **local_weights = NULL;
static int **local_profits = NULL;
static int local_nf = 0, local_ni = 0;

/* Forward declarations of internal functions */
static void set_error(const char *msg);
static int setup_global_data(const knapsack_problem_t *problem);
static void cleanup_global_data(void);
static knapsack_result_t* convert_population_to_result(pop *population);

#ifdef _WIN32
/* DLL Entry Point for Windows */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
#endif

/* Error handling */
static void set_error(const char *msg) {
    strncpy(last_error, msg, MAX_ERROR_MSG - 1);
    last_error[MAX_ERROR_MSG - 1] = '\0';
}

KNAPSACK_API const char* KNAPSACK_CALL knapsack_get_error(void) {
    return last_error;
}

KNAPSACK_API const char* KNAPSACK_CALL knapsack_get_version(void) {
    return KNAPSACK_VERSION;
}

/* Setup global data for algorithm */
static int setup_global_data(const knapsack_problem_t *problem) {
    if (!problem) return KNAPSACK_ERROR_NULL_PTR;
    
    cleanup_global_data();
    
    local_ni = problem->num_items;
    local_nf = problem->num_objectives;
    
    /* Allocate capacity array */
    local_capacities = (double*)chk_malloc(local_nf * sizeof(double));
    if (!local_capacities) {
        set_error("Failed to allocate capacity array");
        return KNAPSACK_ERROR_MEMORY;
    }
    
    /* Copy capacities */
    memcpy(local_capacities, problem->capacities, local_nf * sizeof(double));
    
    /* Allocate weight matrix */
    local_weights = (int**)chk_malloc(local_nf * sizeof(int*));
    if (!local_weights) {
        set_error("Failed to allocate weight matrix");
        return KNAPSACK_ERROR_MEMORY;
    }
    
    for (int f = 0; f < local_nf; f++) {
        local_weights[f] = (int*)chk_malloc(local_ni * sizeof(int));
        if (!local_weights[f]) {
            set_error("Failed to allocate weight row");
            return KNAPSACK_ERROR_MEMORY;
        }
        memcpy(local_weights[f], problem->weights[f], local_ni * sizeof(int));
    }
    
    /* Allocate profit matrix */
    local_profits = (int**)chk_malloc(local_nf * sizeof(int*));
    if (!local_profits) {
        set_error("Failed to allocate profit matrix");
        return KNAPSACK_ERROR_MEMORY;
    }
    
    for (int f = 0; f < local_nf; f++) {
        local_profits[f] = (int*)chk_malloc(local_ni * sizeof(int));
        if (!local_profits[f]) {
            set_error("Failed to allocate profit row");
            return KNAPSACK_ERROR_MEMORY;
        }
        memcpy(local_profits[f], problem->profits[f], local_ni * sizeof(int));
    }
    
    /* Set global variables for algorithm */
    nf = local_nf;
    ni = local_ni;
    capacities = local_capacities;
    weights = local_weights;
    profits = local_profits;
    
    return KNAPSACK_SUCCESS;
}

static void cleanup_global_data(void) {
    if (local_capacities) {
        free(local_capacities);
        local_capacities = NULL;
    }
    
    if (local_weights) {
        for (int f = 0; f < local_nf; f++) {
            if (local_weights[f]) {
                free(local_weights[f]);
            }
        }
        free(local_weights);
        local_weights = NULL;
    }
    
    if (local_profits) {
        for (int f = 0; f < local_nf; f++) {
            if (local_profits[f]) {
                free(local_profits[f]);
            }
        }
        free(local_profits);
        local_profits = NULL;
    }
    
    local_nf = local_ni = 0;
}

/* Create problem from arrays */
KNAPSACK_API knapsack_problem_t* KNAPSACK_CALL 
knapsack_create_problem(int num_items, int num_objectives, 
                       const double *capacities_array, const int **weights_array, const int **profits_array) {
    
    if (num_items <= 0 || num_objectives <= 0 || num_objectives > 4) {
        set_error("Invalid number of items or objectives");
        return NULL;
    }
    
    if (!capacities_array || !weights_array || !profits_array) {
        set_error("NULL pointer in input arrays");
        return NULL;
    }
    
    knapsack_problem_t *problem = (knapsack_problem_t*)chk_malloc(sizeof(knapsack_problem_t));
    if (!problem) {
        set_error("Failed to allocate problem structure");
        return NULL;
    }
    
    problem->num_items = num_items;
    problem->num_objectives = num_objectives;
    
    /* Allocate and copy capacities */
    problem->capacities = (double*)chk_malloc(num_objectives * sizeof(double));
    if (!problem->capacities) {
        free(problem);
        set_error("Failed to allocate capacities");
        return NULL;
    }
    memcpy(problem->capacities, capacities_array, num_objectives * sizeof(double));
    
    /* Allocate and copy weights */
    problem->weights = (int**)chk_malloc(num_objectives * sizeof(int*));
    if (!problem->weights) {
        free(problem->capacities);
        free(problem);
        set_error("Failed to allocate weights matrix");
        return NULL;
    }
    
    for (int f = 0; f < num_objectives; f++) {
        problem->weights[f] = (int*)chk_malloc(num_items * sizeof(int));
        if (!problem->weights[f]) {
            for (int i = 0; i < f; i++) free(problem->weights[i]);
            free(problem->weights);
            free(problem->capacities);
            free(problem);
            set_error("Failed to allocate weights row");
            return NULL;
        }
        memcpy(problem->weights[f], weights_array[f], num_items * sizeof(int));
    }
    
    /* Allocate and copy profits */
    problem->profits = (int**)chk_malloc(num_objectives * sizeof(int*));
    if (!problem->profits) {
        for (int f = 0; f < num_objectives; f++) free(problem->weights[f]);
        free(problem->weights);
        free(problem->capacities);
        free(problem);
        set_error("Failed to allocate profits matrix");
        return NULL;
    }
    
    for (int f = 0; f < num_objectives; f++) {
        problem->profits[f] = (int*)chk_malloc(num_items * sizeof(int));
        if (!problem->profits[f]) {
            for (int i = 0; i < f; i++) free(problem->profits[i]);
            free(problem->profits);
            for (int i = 0; i < num_objectives; i++) free(problem->weights[i]);
            free(problem->weights);
            free(problem->capacities);
            free(problem);
            set_error("Failed to allocate profits row");
            return NULL;
        }
        memcpy(problem->profits[f], profits_array[f], num_items * sizeof(int));
    }
    
    return problem;
}

/* Load problem from file */
KNAPSACK_API knapsack_problem_t* KNAPSACK_CALL 
knapsack_load_problem(const char *filename) {
    if (!filename) {
        set_error("NULL filename");
        return NULL;
    }
    
    FILE *source = fopen(filename, "r");
    if (!source) {
        set_error("Failed to open problem file");
        return NULL;
    }
    
    int num_objectives, num_items;
    if (fscanf(source, "%d %d", &num_objectives, &num_items) != 2) {
        fclose(source);
        set_error("Failed to read problem dimensions");
        return NULL;
    }
    
    if (num_objectives <= 0 || num_objectives > 4 || num_items <= 0) {
        fclose(source);
        set_error("Invalid problem dimensions");
        return NULL;
    }
    
    knapsack_problem_t *problem = (knapsack_problem_t*)chk_malloc(sizeof(knapsack_problem_t));
    if (!problem) {
        fclose(source);
        set_error("Failed to allocate problem structure");
        return NULL;
    }
    
    problem->num_items = num_items;
    problem->num_objectives = num_objectives;
    
    /* Allocate arrays */
    problem->capacities = (double*)chk_malloc(num_objectives * sizeof(double));
    problem->weights = (int**)chk_malloc(num_objectives * sizeof(int*));
    problem->profits = (int**)chk_malloc(num_objectives * sizeof(int*));
    
    if (!problem->capacities || !problem->weights || !problem->profits) {
        knapsack_free_problem(problem);
        fclose(source);
        set_error("Failed to allocate problem arrays");
        return NULL;
    }
    
    for (int f = 0; f < num_objectives; f++) {
        problem->weights[f] = (int*)chk_malloc(num_items * sizeof(int));
        problem->profits[f] = (int*)chk_malloc(num_items * sizeof(int));
        if (!problem->weights[f] || !problem->profits[f]) {
            knapsack_free_problem(problem);
            fclose(source);
            set_error("Failed to allocate item arrays");
            return NULL;
        }
    }
    
    /* Read data */
    for (int f = 0; f < num_objectives; f++) {
        if (fscanf(source, "%lf", &problem->capacities[f]) != 1) {
            knapsack_free_problem(problem);
            fclose(source);
            set_error("Failed to read capacity");
            return NULL;
        }
        
        for (int i = 0; i < num_items; i++) {
            char cl[20];
            if (fscanf(source, "%s", cl) != 1 ||
                fscanf(source, "%d", &problem->weights[f][i]) != 1 ||
                fscanf(source, "%d", &problem->profits[f][i]) != 1) {
                knapsack_free_problem(problem);
                fclose(source);
                set_error("Failed to read item data");
                return NULL;
            }
        }
    }
    
    fclose(source);
    return problem;
}

/* Free problem structure */
KNAPSACK_API void KNAPSACK_CALL 
knapsack_free_problem(knapsack_problem_t *problem) {
    if (!problem) return;
    
    if (problem->capacities) free(problem->capacities);
    
    if (problem->weights) {
        for (int f = 0; f < problem->num_objectives; f++) {
            if (problem->weights[f]) free(problem->weights[f]);
        }
        free(problem->weights);
    }
    
    if (problem->profits) {
        for (int f = 0; f < problem->num_objectives; f++) {
            if (problem->profits[f]) free(problem->profits[f]);
        }
        free(problem->profits);
    }
    
    free(problem);
}

/* Create default parameters */
KNAPSACK_API knapsack_params_t KNAPSACK_CALL 
knapsack_default_params(void) {
    knapsack_params_t params;
    params.population_size = 10;
    params.max_iterations = 100;
    params.perturbation_rate = 0.05;
    params.kappa = 0.05;
    params.local_search_depth = 5;
    params.seed = 0; /* Will use time-based seed */
    return params;
}

/* Convert population to result structure */
static knapsack_result_t* convert_population_to_result(pop *population) {
    if (!population || population->size <= 0) {
        set_error("Invalid population");
        return NULL;
    }
    
    knapsack_result_t *result = (knapsack_result_t*)chk_malloc(sizeof(knapsack_result_t));
    if (!result) {
        set_error("Failed to allocate result structure");
        return NULL;
    }
    
    result->count = population->size;
    result->capacity = population->size;
    result->solutions = (knapsack_solution_t*)chk_malloc(result->count * sizeof(knapsack_solution_t));
    
    if (!result->solutions) {
        free(result);
        set_error("Failed to allocate solutions array");
        return NULL;
    }
    
    /* Convert each individual to solution */
    for (int i = 0; i < population->size; i++) {
        ind *individual = population->ind_array[i];
        knapsack_solution_t *sol = &result->solutions[i];
        
        sol->num_items = local_ni;
        sol->num_objectives = local_nf;
        
        /* Allocate solution arrays */
        sol->items = (int*)chk_malloc(local_ni * sizeof(int));
        sol->objectives = (double*)chk_malloc(local_nf * sizeof(double));
        sol->capacities_used = (double*)chk_malloc(local_nf * sizeof(double));
        
        if (!sol->items || !sol->objectives || !sol->capacities_used) {
            set_error("Failed to allocate solution data");
            /* Free partially allocated solution */
            if (sol->items) free(sol->items);
            if (sol->objectives) free(sol->objectives);
            if (sol->capacities_used) free(sol->capacities_used);
            /* Free previous solutions */
            for (int j = 0; j < i; j++) {
                knapsack_solution_t *prev_sol = &result->solutions[j];
                free(prev_sol->items);
                free(prev_sol->objectives);
                free(prev_sol->capacities_used);
            }
            free(result->solutions);
            free(result);
            return NULL;
        }
        
        /* Initialize arrays to prevent accessing uninitialized memory */
        for (int j = 0; j < local_ni; j++) {
            sol->items[j] = 0;
        }
        for (int j = 0; j < local_nf; j++) {
            sol->objectives[j] = 0.0;
            sol->capacities_used[j] = 0.0;
        }
        
        /* Copy data from individual if valid */
        if (individual && individual->Items && individual->f && individual->capa) {
            /* Copy item selection */
            for (int j = 0; j < local_ni; j++) {
                sol->items[j] = individual->Items[j];
            }
            
            /* Copy objective values */
            for (int f = 0; f < local_nf; f++) {
                sol->objectives[f] = individual->f[f];
                sol->capacities_used[f] = individual->capa[f];
            }
        }
    }
    
    return result;
}

/* Main solve function */
KNAPSACK_API int KNAPSACK_CALL 
knapsack_solve(const knapsack_problem_t *problem, 
               const knapsack_params_t *params,
               knapsack_result_t **result) {
    
    if (!problem || !result) {
        set_error("NULL pointer in solve function");
        return KNAPSACK_ERROR_NULL_PTR;
    }
    
    *result = NULL;
    
    /* Setup global data */
    int err = setup_global_data(problem);
    if (err != KNAPSACK_SUCCESS) return err;
    
    /* Set algorithm parameters */
    knapsack_params_t default_params = knapsack_default_params();
    if (!params) params = &default_params;
    
    perturbation_rate = params->perturbation_rate;
    kappa = params->kappa;
    
    /* Initialize random seed */
    unsigned int seed = params->seed;
    if (seed == 0) seed = (unsigned int)time(NULL);
    srand(seed);
    
    /* Create populations */
    pop *solutions = create_pop(params->population_size, local_nf);
    pop *archive = create_pop(params->population_size * 10, local_nf); /* Large archive */
    
    if (!solutions || !archive) {
        cleanup_global_data();
        if (solutions) complete_free_pop(solutions);
        if (archive) complete_free_pop(archive);
        set_error("Failed to create populations");
        return KNAPSACK_ERROR_MEMORY;
    }
    
    /* Initialize population */
    random_init_pop(solutions, params->population_size);
    
    /* Run optimization iterations */
    for (int iter = 0; iter < params->max_iterations; iter++) {
        /* Extract to archive */
        extractPtoArchive(solutions, archive);
        
        /* Run simplified local search */
        simple_local_search(solutions, archive, 1);
    }
    
    /* Final extraction to archive */
    extractPtoArchive(solutions, archive);
    
    /* Convert result */
    *result = convert_population_to_result(archive);
    
    /* Cleanup */
    complete_free_pop(solutions);
    complete_free_pop(archive);
    cleanup_global_data();
    
    if (!*result) {
        return KNAPSACK_ERROR_MEMORY;
    }
    
    return KNAPSACK_SUCCESS;
}

/* Free result structure */
KNAPSACK_API void KNAPSACK_CALL 
knapsack_free_result(knapsack_result_t *result) {
    if (!result) return;
    
    if (result->solutions) {
        for (int i = 0; i < result->count; i++) {
            knapsack_solution_t *sol = &result->solutions[i];
            if (sol->items) free(sol->items);
            if (sol->objectives) free(sol->objectives);
            if (sol->capacities_used) free(sol->capacities_used);
        }
        free(result->solutions);
    }
    
    free(result);
}

/* Get solution at index */
KNAPSACK_API const knapsack_solution_t* KNAPSACK_CALL 
knapsack_get_solution(const knapsack_result_t *result, int index) {
    if (!result || index < 0 || index >= result->count) {
        set_error("Invalid result or index");
        return NULL;
    }
    return &result->solutions[index];
}

/* Get solution count */
KNAPSACK_API int KNAPSACK_CALL 
knapsack_get_solution_count(const knapsack_result_t *result) {
    if (!result) return 0;
    return result->count;
}

/* Check feasibility */
KNAPSACK_API int KNAPSACK_CALL 
knapsack_is_feasible(const knapsack_problem_t *problem, 
                     const knapsack_solution_t *solution) {
    if (!problem || !solution) {
        set_error("NULL pointer in feasibility check");
        return -1;
    }
    
    if (solution->num_items != problem->num_items || 
        solution->num_objectives != problem->num_objectives) {
        set_error("Dimension mismatch in feasibility check");
        return -1;
    }
    
    /* Check capacity constraints */
    for (int f = 0; f < problem->num_objectives; f++) {
        if (solution->capacities_used[f] > problem->capacities[f]) {
            return 0; /* Not feasible */
        }
    }
    
    return 1; /* Feasible */
}