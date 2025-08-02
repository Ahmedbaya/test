#define MOKP_DLL_EXPORTS
#include "mokp_dll.h"
#include "mokp_core.h"

// DLL state variables
static int dll_initialized = 0;
static int problem_loaded = 0;
static int optimization_completed = 0;
static char last_error_message[256] = "";

// Algorithm parameters
static int param_population_size = 10;
static int param_max_iterations = 100;
static double param_perturbation_rate = 0.05;

// Internal helper functions
static void set_error_message(const char* message) {
    strncpy(last_error_message, message, sizeof(last_error_message) - 1);
    last_error_message[sizeof(last_error_message) - 1] = '\0';
}

static int validate_dll_state(int require_loaded, int require_optimized) {
    if (!dll_initialized) {
        set_error_message("Optimizer not initialized. Call InitializeOptimizer() first.");
        return MOKP_ERROR_NOT_INITIALIZED;
    }
    if (require_loaded && !problem_loaded) {
        set_error_message("Problem not loaded. Call LoadProblem() first.");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    if (require_optimized && !optimization_completed) {
        set_error_message("Optimization not completed. Call RunOptimization() first.");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    return MOKP_SUCCESS;
}

// DLL Interface Implementation
MOKP_API int InitializeOptimizer(void) {
    if (dll_initialized) {
        Cleanup(); // Clean up previous state
    }
    
    // Initialize global variables
    dimension = 2; // Default to 2 objectives
    NBITEMS = 250; // Default number of items
    nf = 2;
    ni = 250;
    paretoIni = 28000;
    perturbation_rate = param_perturbation_rate;
    smallValue = 0.0000001f;
    max_value = 1000000.0f;
    kappa = 0.05;
    alpha = param_population_size;
    nombreLIGNE = 0;
    nextLn = 0;
    inv = 0;
    
    // Initialize random seed
    seed((unsigned int)time(NULL));
    
    // Reset state flags
    problem_loaded = 0;
    optimization_completed = 0;
    dll_initialized = 1;
    
    set_error_message("Success");
    return MOKP_SUCCESS;
}

MOKP_API int LoadProblem(const char* filename) {
    int result = validate_dll_state(0, 0);
    if (result != MOKP_SUCCESS) return result;
    
    if (!filename || strlen(filename) == 0) {
        set_error_message("Invalid filename parameter");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    // Check if file exists
    FILE* test_file = fopen(filename, "r");
    if (!test_file) {
        set_error_message("File not found or cannot be opened");
        return MOKP_ERROR_FILE_NOT_FOUND;
    }
    fclose(test_file);
    
    // Load the problem
    loadMOKP((char*)filename);
    
    // Update dimensions based on loaded problem
    dimension = nf;
    NBITEMS = ni;
    
    // Initialize bounds
    bounds = (range*) chk_malloc(nf * sizeof(range));
    if (!bounds) {
        set_error_message("Memory allocation failed for bounds");
        return MOKP_ERROR_MEMORY_ALLOCATION;
    }
    
    // Initialize populations
    P = create_pop(paretoIni, nf);
    if (!P) {
        set_error_message("Memory allocation failed for main population");
        return MOKP_ERROR_MEMORY_ALLOCATION;
    }
    
    problem_loaded = 1;
    optimization_completed = 0;
    set_error_message("Success");
    return MOKP_SUCCESS;
}

MOKP_API int SetParameters(int population_size, int max_iterations, double perturbation_rate) {
    int result = validate_dll_state(0, 0);
    if (result != MOKP_SUCCESS) return result;
    
    if (population_size <= 0 || max_iterations <= 0 || 
        perturbation_rate < 0.0 || perturbation_rate > 1.0) {
        set_error_message("Invalid parameter values");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    param_population_size = population_size;
    param_max_iterations = max_iterations;
    param_perturbation_rate = perturbation_rate;
    
    // Update global variables
    alpha = population_size;
    perturbation_rate = param_perturbation_rate;
    
    set_error_message("Success");
    return MOKP_SUCCESS;
}

MOKP_API int RunOptimization(void) {
    int result = validate_dll_state(1, 0);
    if (result != MOKP_SUCCESS) return result;
    
    // Load weights file based on dimension
    char weights_filename[100];
    if (dimension == 2) {
        strcpy(weights_filename, "Weights_2obj_FQ200.txt");
    } else if (dimension == 3) {
        strcpy(weights_filename, "Weights_3obj_FQ100.txt");
    } else if (dimension == 4) {
        strcpy(weights_filename, "Weights_4obj_FQ40.txt");
    } else {
        set_error_message("Unsupported number of objectives");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    // Check if weights file exists
    FILE* weights_file = fopen(weights_filename, "r");
    if (!weights_file) {
        set_error_message("Weights file not found");
        return MOKP_ERROR_FILE_NOT_FOUND;
    }
    fclose(weights_file);
    
    read_weights_file(weights_filename);
    
    // Run optimization iterations
    for (int it = 0; it < param_max_iterations; it++) {
        solutions = create_pop(alpha, nf);
        archive = create_pop(paretoIni, nf);
        
        if (!solutions || !archive) {
            set_error_message("Memory allocation failed during optimization");
            return MOKP_ERROR_MEMORY_ALLOCATION;
        }
        
        choose_weight();
        P_init_pop(solutions, P, alpha);
        extractPtoArchive(solutions, P);
        
        // Indicator-based local search
        calcul_weight(solutions, alpha);
        calcMaxbound(solutions, alpha);
        compute_all_fitness(solutions);
        
        Indicator_local_search1(solutions, archive, alpha);
        extractPtoArchive(archive, P);
        
        complete_free_pop(solutions);
        complete_free_pop(archive);
        solutions = NULL;
        archive = NULL;
    }
    
    optimization_completed = 1;
    set_error_message("Success");
    return MOKP_SUCCESS;
}

MOKP_API int GetResultCount(void) {
    int result = validate_dll_state(1, 1);
    if (result != MOKP_SUCCESS) return result;
    
    if (!P) return 0;
    return P->size;
}

MOKP_API int GetResult(int index, double* objectives, int* decision_vars) {
    int result = validate_dll_state(1, 1);
    if (result != MOKP_SUCCESS) return result;
    
    if (!P || index < 0 || index >= P->size) {
        set_error_message("Invalid solution index");
        return MOKP_ERROR_INVALID_INDEX;
    }
    
    if (!objectives || !decision_vars) {
        set_error_message("Invalid output parameter pointers");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    ind* solution = P->ind_array[index];
    
    // Copy objective values
    for (int i = 0; i < nf; i++) {
        objectives[i] = solution->f[i];
    }
    
    // Copy decision variables
    for (int i = 0; i < ni; i++) {
        decision_vars[i] = solution->Items[i];
    }
    
    set_error_message("Success");
    return MOKP_SUCCESS;
}

MOKP_API int GetResults(MOKPResults* results) {
    int result = validate_dll_state(1, 1);
    if (result != MOKP_SUCCESS) return result;
    
    if (!results) {
        set_error_message("Invalid results parameter");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    if (!P || P->size == 0) {
        results->solutions = NULL;
        results->count = 0;
        results->capacity = 0;
        return MOKP_SUCCESS;
    }
    
    // Allocate memory for results
    results->count = P->size;
    results->capacity = P->size;
    results->solutions = (MOKPSolution*)malloc(P->size * sizeof(MOKPSolution));
    
    if (!results->solutions) {
        set_error_message("Memory allocation failed for results");
        return MOKP_ERROR_MEMORY_ALLOCATION;
    }
    
    // Copy all solutions
    for (int i = 0; i < P->size; i++) {
        MOKPSolution* sol = &results->solutions[i];
        ind* source = P->ind_array[i];
        
        sol->num_objectives = nf;
        sol->num_items = ni;
        
        // Allocate and copy objectives
        sol->objectives = (double*)malloc(nf * sizeof(double));
        sol->decision_vars = (int*)malloc(ni * sizeof(int));
        
        if (!sol->objectives || !sol->decision_vars) {
            // Clean up on failure
            FreeResults(results);
            set_error_message("Memory allocation failed for solution data");
            return MOKP_ERROR_MEMORY_ALLOCATION;
        }
        
        for (int j = 0; j < nf; j++) {
            sol->objectives[j] = source->f[j];
        }
        
        for (int j = 0; j < ni; j++) {
            sol->decision_vars[j] = source->Items[j];
        }
    }
    
    set_error_message("Success");
    return MOKP_SUCCESS;
}

MOKP_API int GetProblemInfo(int* num_objectives, int* num_items) {
    int result = validate_dll_state(1, 0);
    if (result != MOKP_SUCCESS) return result;
    
    if (!num_objectives || !num_items) {
        set_error_message("Invalid output parameter pointers");
        return MOKP_ERROR_INVALID_PARAMETER;
    }
    
    *num_objectives = nf;
    *num_items = ni;
    
    set_error_message("Success");
    return MOKP_SUCCESS;
}

MOKP_API void Cleanup(void) {
    if (P) {
        complete_free_pop(P);
        P = NULL;
    }
    
    if (solutions) {
        complete_free_pop(solutions);
        solutions = NULL;
    }
    
    if (archive) {
        complete_free_pop(archive);
        archive = NULL;
    }
    
    if (bounds) {
        free(bounds);
        bounds = NULL;
    }
    
    dll_initialized = 0;
    problem_loaded = 0;
    optimization_completed = 0;
    set_error_message("Cleanup completed");
}

// Memory management helpers
MOKP_API MOKPResults* AllocateResults(int count) {
    MOKPResults* results = (MOKPResults*)malloc(sizeof(MOKPResults));
    if (!results) return NULL;
    
    results->count = 0;
    results->capacity = count;
    results->solutions = (MOKPSolution*)malloc(count * sizeof(MOKPSolution));
    
    if (!results->solutions) {
        free(results);
        return NULL;
    }
    
    return results;
}

MOKP_API void FreeResults(MOKPResults* results) {
    if (!results) return;
    
    if (results->solutions) {
        for (int i = 0; i < results->count; i++) {
            if (results->solutions[i].objectives) {
                free(results->solutions[i].objectives);
            }
            if (results->solutions[i].decision_vars) {
                free(results->solutions[i].decision_vars);
            }
        }
        free(results->solutions);
        results->solutions = NULL;
    }
    
    results->count = 0;
    results->capacity = 0;
}

MOKP_API void FreeResultsAndStructure(MOKPResults* results) {
    FreeResults(results);
    free(results);
}

MOKP_API const char* GetErrorMessage(int error_code) {
    switch (error_code) {
        case MOKP_SUCCESS:
            return "Success";
        case MOKP_ERROR_INVALID_PARAMETER:
            return "Invalid parameter";
        case MOKP_ERROR_FILE_NOT_FOUND:
            return "File not found";
        case MOKP_ERROR_MEMORY_ALLOCATION:
            return "Memory allocation failed";
        case MOKP_ERROR_NOT_INITIALIZED:
            return "Optimizer not initialized";
        case MOKP_ERROR_INVALID_INDEX:
            return "Invalid index";
        default:
            return last_error_message;
    }
}