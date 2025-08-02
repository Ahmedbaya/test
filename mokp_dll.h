#ifndef MOKP_DLL_H
#define MOKP_DLL_H

#ifdef __cplusplus
extern "C" {
#endif

// DLL export declarations for Windows
#ifdef _WIN32
    #ifdef MOKP_DLL_EXPORTS
        #define MOKP_API __declspec(dllexport)
    #else
        #define MOKP_API __declspec(dllimport)
    #endif
#else
    #define MOKP_API
#endif

// Error codes
#define MOKP_SUCCESS 0
#define MOKP_ERROR_INVALID_PARAMETER -1
#define MOKP_ERROR_FILE_NOT_FOUND -2
#define MOKP_ERROR_MEMORY_ALLOCATION -3
#define MOKP_ERROR_NOT_INITIALIZED -4
#define MOKP_ERROR_INVALID_INDEX -5

// Structure for returning solution data
typedef struct {
    double* objectives;     // Array of objective values
    int* decision_vars;     // Array of decision variables (0/1 for each item)
    int num_objectives;     // Number of objectives
    int num_items;          // Number of items
} MOKPSolution;

// Structure for returning all results
typedef struct {
    MOKPSolution* solutions;
    int count;
    int capacity;
} MOKPResults;

// Main DLL interface functions
MOKP_API int InitializeOptimizer(void);
MOKP_API int LoadProblem(const char* filename);
MOKP_API int SetParameters(int population_size, int max_iterations, double perturbation_rate);
MOKP_API int RunOptimization(void);
MOKP_API int GetResultCount(void);
MOKP_API int GetResult(int index, double* objectives, int* decision_vars);
MOKP_API int GetResults(MOKPResults* results);
MOKP_API int GetProblemInfo(int* num_objectives, int* num_items);
MOKP_API void Cleanup(void);

// Memory management helpers for Python integration
MOKP_API MOKPResults* AllocateResults(int count);
MOKP_API void FreeResults(MOKPResults* results);
MOKP_API void FreeResultsAndStructure(MOKPResults* results);
MOKP_API const char* GetErrorMessage(int error_code);

#ifdef __cplusplus
}
#endif

#endif // MOKP_DLL_H