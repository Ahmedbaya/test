#ifndef MOKP_DLL_H
#define MOKP_DLL_H

#ifdef __cplusplus
extern "C" {
#endif

// Platform-specific DLL export macros
#ifdef _WIN32
    #ifdef MOKP_DLL_EXPORTS
        #define MOKP_API __declspec(dllexport)
    #else
        #define MOKP_API __declspec(dllimport)
    #endif
    #define MOKP_CALL __stdcall
#else
    #define MOKP_API __attribute__((visibility("default")))
    #define MOKP_CALL
#endif

// Constants from the original implementation
#define MOKP_MAX_ITEMS 250
#define MOKP_MAX_OBJECTIVES 4
#define MOKP_DEFAULT_POPULATION_SIZE 10
#define MOKP_DEFAULT_ARCHIVE_SIZE 28000

// Error codes
#define MOKP_SUCCESS 0
#define MOKP_ERROR_INVALID_PARAMETER -1
#define MOKP_ERROR_FILE_NOT_FOUND -2
#define MOKP_ERROR_MEMORY_ALLOCATION -3
#define MOKP_ERROR_NOT_INITIALIZED -4
#define MOKP_ERROR_INVALID_STATE -5

// Structure for returning solution data to Python
typedef struct {
    double objectives[MOKP_MAX_OBJECTIVES];
    int items[MOKP_MAX_ITEMS];
    int num_items_selected;
    double capacities[MOKP_MAX_OBJECTIVES];
} MOKP_Solution;

// Structure for algorithm parameters
typedef struct {
    int max_iterations;
    int population_size;
    int archive_size;
    double perturbation_rate;
    double kappa;
    int num_objectives;
    int num_items;
    int local_search_depth;
} MOKP_Parameters;

// Core DLL interface functions
MOKP_API int MOKP_CALL mokp_initialize();
MOKP_API int MOKP_CALL mokp_cleanup();
MOKP_API int MOKP_CALL mokp_load_problem(const char* problem_file);
MOKP_API int MOKP_CALL mokp_load_weights(const char* weights_file);
MOKP_API int MOKP_CALL mokp_set_parameters(const MOKP_Parameters* params);
MOKP_API int MOKP_CALL mokp_get_parameters(MOKP_Parameters* params);

// Optimization functions
MOKP_API int MOKP_CALL mokp_run_optimization(int max_iterations);
MOKP_API int MOKP_CALL mokp_run_single_iteration();
MOKP_API int MOKP_CALL mokp_get_pareto_size();
MOKP_API int MOKP_CALL mokp_get_pareto_solutions(MOKP_Solution* solutions, int max_solutions);

// Population management functions
MOKP_API int MOKP_CALL mokp_initialize_population();
MOKP_API int MOKP_CALL mokp_get_population_size();
MOKP_API int MOKP_CALL mokp_get_population_solutions(MOKP_Solution* solutions, int max_solutions);

// Utility functions
MOKP_API int MOKP_CALL mokp_evaluate_solution(const int* items, MOKP_Solution* solution);
MOKP_API int MOKP_CALL mokp_is_solution_feasible(const int* items);
MOKP_API double MOKP_CALL mokp_get_last_iteration_time();
MOKP_API int MOKP_CALL mokp_get_problem_info(int* num_objectives, int* num_items);

// Advanced functions for fine-grained control
MOKP_API int MOKP_CALL mokp_indicator_local_search();
MOKP_API int MOKP_CALL mokp_extract_pareto_archive();
MOKP_API int MOKP_CALL mokp_update_weights();
MOKP_API int MOKP_CALL mokp_compute_fitness();

// Debug and monitoring functions
MOKP_API const char* MOKP_CALL mokp_get_last_error();
MOKP_API int MOKP_CALL mokp_get_algorithm_state();
MOKP_API int MOKP_CALL mokp_save_results(const char* output_file);

#ifdef __cplusplus
}
#endif

#endif // MOKP_DLL_H