# MOKP DLL Wrapper Documentation

## Multi-Objective Knapsack Problem (MOKP) DLL

This document describes the comprehensive DLL wrapper for the multi-objective knapsack optimization algorithm, designed to enable Python integration and cross-platform usage.

## Overview

The MOKP DLL provides a C-style interface to a sophisticated multi-objective knapsack optimization algorithm based on Indicator-Based Multi-Objective Local Search (IBMOLS). The wrapper maintains all the functionality of the original C code while providing a clean, safe interface for external applications.

## Features

- **Multi-objective optimization**: Supports 2, 3, or 4 objective functions
- **Pareto front generation**: Finds non-dominated solutions
- **Indicator-based local search**: Uses advanced optimization techniques
- **Memory safety**: Proper memory management across DLL boundaries
- **Cross-platform compatibility**: Works on Windows, Linux, and macOS
- **Python integration**: Easy-to-use Python wrapper using ctypes
- **Error handling**: Comprehensive error reporting and validation

## Architecture

### Core Components

1. **mokp_dll.h/c**: Main DLL interface and wrapper functions
2. **mokp_core.h/c**: Core algorithm implementation (adapted from original main.c)
3. **indicators.c**: Multi-objective indicator functions
4. **Common.h**: Data structure definitions
5. **IBMOLS.h**: Population and algorithm function declarations

### Key Data Structures

```c
// Individual solution
typedef struct {
    int nombr_nonpris;      // Number of non-selected items
    int nombr;              // Number of selected items
    int rank;               // Pareto rank
    int *Items;             // Item selection array (0/1)
    float fitnessbest;      // Best fitness value
    float fitness;          // Current fitness value
    int explored;           // Exploration flag
    double* f;              // Objective values
    double* capa;           // Capacity usage
    double* v;              // Weighted objective values
    int* d;                 // Item ordering
} ind;

// Population of solutions
typedef struct {
    int size;               // Current population size
    int maxsize;            // Maximum population size
    ind **ind_array;        // Array of individual pointers
} pop;

// Solution data for external use
typedef struct {
    double* objectives;     // Array of objective values
    int* decision_vars;     // Array of decision variables (0/1)
    int num_objectives;     // Number of objectives
    int num_items;          // Number of items
} MOKPSolution;

// Results container
typedef struct {
    MOKPSolution* solutions;
    int count;
    int capacity;
} MOKPResults;
```

## API Reference

### Error Codes

```c
#define MOKP_SUCCESS 0
#define MOKP_ERROR_INVALID_PARAMETER -1
#define MOKP_ERROR_FILE_NOT_FOUND -2
#define MOKP_ERROR_MEMORY_ALLOCATION -3
#define MOKP_ERROR_NOT_INITIALIZED -4
#define MOKP_ERROR_INVALID_INDEX -5
```

### Core Functions

#### InitializeOptimizer()
```c
int InitializeOptimizer(void);
```
Initializes the optimization system. Must be called first.

**Returns**: 0 on success, error code on failure

#### LoadProblem()
```c
int LoadProblem(const char* filename);
```
Loads a knapsack problem instance from file.

**Parameters**:
- `filename`: Path to problem file

**Returns**: 0 on success, error code on failure

#### SetParameters()
```c
int SetParameters(int population_size, int max_iterations, double perturbation_rate);
```
Configures algorithm parameters.

**Parameters**:
- `population_size`: Size of the population (default: 10)
- `max_iterations`: Maximum number of iterations (default: 100)
- `perturbation_rate`: Rate of perturbation (0.0-1.0, default: 0.05)

**Returns**: 0 on success, error code on failure

#### RunOptimization()
```c
int RunOptimization(void);
```
Executes the optimization algorithm.

**Returns**: 0 on success, error code on failure

#### GetResultCount()
```c
int GetResultCount(void);
```
Gets the number of solutions in the Pareto front.

**Returns**: Number of solutions, or negative value on error

#### GetResult()
```c
int GetResult(int index, double* objectives, int* decision_vars);
```
Retrieves a specific solution by index.

**Parameters**:
- `index`: Solution index (0-based)
- `objectives`: Output array for objective values (pre-allocated)
- `decision_vars`: Output array for decision variables (pre-allocated)

**Returns**: 0 on success, error code on failure

#### GetResults()
```c
int GetResults(MOKPResults* results);
```
Retrieves all Pareto front solutions.

**Parameters**:
- `results`: Output structure (allocated by function)

**Returns**: 0 on success, error code on failure

#### GetProblemInfo()
```c
int GetProblemInfo(int* num_objectives, int* num_items);
```
Gets problem dimensions.

**Parameters**:
- `num_objectives`: Output for number of objectives
- `num_items`: Output for number of items

**Returns**: 0 on success, error code on failure

#### Cleanup()
```c
void Cleanup(void);
```
Cleans up all allocated memory. Call when done.

### Memory Management

#### FreeResults()
```c
void FreeResults(MOKPResults* results);
```
Frees memory allocated by GetResults().

#### GetErrorMessage()
```c
const char* GetErrorMessage(int error_code);
```
Gets human-readable error message for error code.

## Problem File Format

The problem files follow this format:

```
<num_objectives> <num_items>
<capacity_1>
<item_name> <weight_1> <profit_1>
<item_name> <weight_1> <profit_1>
...
<capacity_2>
<item_name> <weight_2> <profit_2>
...
```

Example for 2-objective, 3-item problem:
```
2 3
100.0
item1 10 15
item2 20 25
item3 30 35
80.0
item1 15 12
item2 25 22
item3 35 32
```

## Weight Vector Files

The algorithm uses weight vector files for multi-objective optimization:
- `Weights_2obj_FQ200.txt`: For 2-objective problems
- `Weights_3obj_FQ100.txt`: For 3-objective problems  
- `Weights_4obj_FQ40.txt`: For 4-objective problems

## Compilation

### Using Make

```bash
# Build everything
make all

# Build shared library only
make dll

# Build static library only
make static

# Build test program
make test

# Clean build artifacts
make clean

# Run tests
make run-test
```

### Manual Compilation

#### Linux/macOS
```bash
# Compile object files
gcc -Wall -Wextra -O2 -fPIC -c mokp_dll.c -o mokp_dll.o
gcc -Wall -Wextra -O2 -fPIC -c mokp_core.c -o mokp_core.o
gcc -Wall -Wextra -O2 -fPIC -c indicators.c -o indicators.o

# Build shared library
gcc -shared -o libmokp.so mokp_dll.o mokp_core.o indicators.o -lm

# Build test program
gcc -Wall -Wextra -O2 -c test_mokp_dll.c -o test_mokp_dll.o
gcc -o test_mokp_dll test_mokp_dll.o mokp_dll.o mokp_core.o indicators.o -lm
```

#### Windows (MinGW)
```bash
# Build DLL
gcc -shared -o mokp.dll mokp_dll.o mokp_core.o indicators.o -lm -Wl,--out-implib,libmokp.a
```

## Python Integration

### Basic Usage

```python
from mokp_optimizer import MOKPOptimizer

# Create optimizer
optimizer = MOKPOptimizer("./libmokp.so")

# Initialize and load problem
optimizer.initialize()
optimizer.load_problem("250.2.txt")

# Set parameters and run
optimizer.set_parameters(population_size=20, max_iterations=50)
optimizer.run_optimization()

# Get results
solutions = optimizer.get_all_solutions()
for sol in solutions:
    print(f"Objectives: {sol['objectives']}")
    print(f"Selected items: {sol['selected_items']}")

# Cleanup
optimizer.cleanup()
```

### Advanced Usage

```python
# Get individual solutions
count = optimizer.get_result_count()
for i in range(count):
    objectives, decision_vars = optimizer.get_solution(i)
    selected_items = [j for j, val in enumerate(decision_vars) if val == 1]
    print(f"Solution {i}: objectives={objectives}, items={len(selected_items)}")

# Get problem information
num_objectives, num_items = optimizer.get_problem_info()
print(f"Problem: {num_objectives} objectives, {num_items} items")
```

## Performance Considerations

1. **Memory Usage**: The algorithm maintains populations in memory. For large problems, monitor memory usage.

2. **Convergence**: The algorithm uses local search with convergence detection. More iterations generally lead to better solutions.

3. **Population Size**: Larger populations explore more solutions but require more computation.

4. **File I/O**: Problem and weight files are read once during initialization.

## Algorithm Details

The implementation uses:

1. **Indicator-Based Multi-Objective Local Search (IBMOLS)**
2. **Additive Epsilon Indicator** for solution comparison
3. **Pareto dominance** for non-dominated solution identification
4. **Local search** with perturbation-based operators
5. **Weight vector decomposition** for objective space exploration

## Troubleshooting

### Common Issues

1. **Compilation Errors**:
   - Ensure all header files are present
   - Check compiler flags and library paths
   - Verify cross-platform compatibility settings

2. **Runtime Errors**:
   - Check file paths for problem and weight files
   - Verify memory allocation success
   - Ensure proper initialization sequence

3. **Python Integration**:
   - Verify DLL path and accessibility
   - Check ctypes function signatures
   - Ensure proper error handling

### Debug Information

Enable debug output by modifying compile flags:
```bash
gcc -DDEBUG -g -Wall -Wextra -O0 -fPIC -c mokp_dll.c
```

## Examples

### C Example
See `test_mokp_dll.c` for a complete C usage example.

### Python Example
See `python_example_simple.py` for a complete Python usage example.

## License and Credits

This DLL wrapper is based on the original multi-objective knapsack optimization algorithm. The wrapper provides additional functionality for cross-platform usage and Python integration while maintaining the core algorithmic behavior.

## Version History

- **v1.0**: Initial DLL wrapper implementation
  - Core algorithm porting
  - C-style API design
  - Memory management
  - Error handling
  - Python integration support
  - Cross-platform compatibility
  - Comprehensive testing