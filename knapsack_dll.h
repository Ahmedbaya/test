#ifndef KNAPSACK_DLL_H
#define KNAPSACK_DLL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Windows DLL Export/Import Macros */
#ifdef _WIN32
    #ifdef KNAPSACK_DLL_EXPORTS
        #define KNAPSACK_API __declspec(dllexport)
    #else
        #define KNAPSACK_API __declspec(dllimport)
    #endif
    #define KNAPSACK_CALL __stdcall
#else
    #define KNAPSACK_API
    #define KNAPSACK_CALL
#endif

/* Data Structure Definitions */
typedef struct {
    int num_items;
    int num_objectives;
    double *capacities;
    int **weights;
    int **profits;
} knapsack_problem_t;

typedef struct {
    int *items;        /* Binary array: 1 if item selected, 0 otherwise */
    double *objectives; /* Objective function values */
    double *capacities_used; /* Capacity used for each constraint */
    int num_items;
    int num_objectives;
} knapsack_solution_t;

typedef struct {
    knapsack_solution_t *solutions;
    int count;
    int capacity;
} knapsack_result_t;

/* Error Codes */
#define KNAPSACK_SUCCESS         0
#define KNAPSACK_ERROR_NULL_PTR  -1
#define KNAPSACK_ERROR_INVALID_PARAM -2
#define KNAPSACK_ERROR_MEMORY    -3
#define KNAPSACK_ERROR_FILE      -4

/* Algorithm Parameters */
typedef struct {
    int population_size;     /* Population size (default: 10) */
    int max_iterations;      /* Maximum iterations (default: 100) */
    double perturbation_rate; /* Perturbation rate (default: 0.05) */
    double kappa;            /* Epsilon indicator parameter (default: 0.05) */
    int local_search_depth;  /* Local search depth L (default: 5) */
    unsigned int seed;       /* Random seed (0 for time-based) */
} knapsack_params_t;

/* Core DLL Functions */

/**
 * Initialize knapsack problem from arrays
 * @param num_items Number of items
 * @param num_objectives Number of objectives (2-4 supported)
 * @param capacities Array of capacity constraints
 * @param weights 2D array [objective][item] of weights
 * @param profits 2D array [objective][item] of profits
 * @return Pointer to problem structure or NULL on error
 */
KNAPSACK_API knapsack_problem_t* KNAPSACK_CALL 
knapsack_create_problem(int num_items, int num_objectives, 
                       const double *capacities, const int **weights, const int **profits);

/**
 * Load knapsack problem from file
 * @param filename Path to problem file
 * @return Pointer to problem structure or NULL on error
 */
KNAPSACK_API knapsack_problem_t* KNAPSACK_CALL 
knapsack_load_problem(const char *filename);

/**
 * Free knapsack problem structure
 * @param problem Problem to free
 */
KNAPSACK_API void KNAPSACK_CALL 
knapsack_free_problem(knapsack_problem_t *problem);

/**
 * Create default algorithm parameters
 * @return Default parameters structure
 */
KNAPSACK_API knapsack_params_t KNAPSACK_CALL 
knapsack_default_params(void);

/**
 * Solve multi-objective knapsack problem
 * @param problem Problem to solve
 * @param params Algorithm parameters
 * @param result Pointer to store results
 * @return Error code (KNAPSACK_SUCCESS on success)
 */
KNAPSACK_API int KNAPSACK_CALL 
knapsack_solve(const knapsack_problem_t *problem, 
               const knapsack_params_t *params,
               knapsack_result_t **result);

/**
 * Free result structure
 * @param result Result to free
 */
KNAPSACK_API void KNAPSACK_CALL 
knapsack_free_result(knapsack_result_t *result);

/**
 * Get solution at specific index
 * @param result Results structure
 * @param index Solution index
 * @return Pointer to solution or NULL if invalid index
 */
KNAPSACK_API const knapsack_solution_t* KNAPSACK_CALL 
knapsack_get_solution(const knapsack_result_t *result, int index);

/**
 * Get number of solutions in result
 * @param result Results structure
 * @return Number of solutions
 */
KNAPSACK_API int KNAPSACK_CALL 
knapsack_get_solution_count(const knapsack_result_t *result);

/**
 * Check if solution is feasible
 * @param problem Problem structure
 * @param solution Solution to check
 * @return 1 if feasible, 0 if not feasible, -1 on error
 */
KNAPSACK_API int KNAPSACK_CALL 
knapsack_is_feasible(const knapsack_problem_t *problem, 
                     const knapsack_solution_t *solution);

/**
 * Get last error message
 * @return Pointer to error message string
 */
KNAPSACK_API const char* KNAPSACK_CALL 
knapsack_get_error(void);

/**
 * Get library version
 * @return Version string
 */
KNAPSACK_API const char* KNAPSACK_CALL 
knapsack_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* KNAPSACK_DLL_H */