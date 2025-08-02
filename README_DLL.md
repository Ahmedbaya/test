# Multi-Objective Knapsack DLL Wrapper

This directory contains a complete DLL wrapper implementation for the existing C multi-objective knapsack optimization algorithm, enabling Python integration.

## Files Overview

### Core DLL Implementation
- **`knapsack_dll.h`** - DLL header with export declarations and API definitions
- **`knapsack_dll.c`** - DLL implementation with wrapper functions
- **`knapsack_core.h`** - Core algorithm functions header
- **`knapsack_core.c`** - Extracted core algorithm functions
- **`Common.h`** - Original data structures (from existing code)

### Python Interface
- **`knapsack_python.py`** - Python wrapper using ctypes
- **`example_usage.py`** - Comprehensive Python usage examples

### Build System
- **`Makefile`** - Cross-platform build system (Linux/macOS/Windows)

### Testing
- **`test_dll.c`** - C test program for DLL functionality
- **`simple_test.c`** - Basic DLL functionality test

## Quick Start

### 1. Build the DLL

```bash
# Build for current platform (Linux/macOS)
make

# Build for Windows (cross-compilation)
make windows

# Build for all platforms
make all_platforms

# Show help
make help
```

### 2. Test the Installation

```bash
# Run basic C tests
make test_static
./simple_test

# Test Python interface
python3 example_usage.py
```

### 3. Use in Python

```python
from knapsack_python import KnapsackSolver

# Initialize solver
solver = KnapsackSolver()
print(f"Library version: {solver.get_version()}")

# Load problem from file
problem = solver.load_problem("250.2.txt")

# Set parameters
params = solver.default_params()
params.max_iterations = 100
params.population_size = 20

# Solve
result = solver.solve(problem, params)

# Access solutions
for i, solution in enumerate(result):
    print(f"Solution {i+1}:")
    print(f"  Selected items: {solution.selected_items}")
    print(f"  Objectives: {solution.objectives}")
```

## API Reference

### C API (knapsack_dll.h)

#### Data Structures

```c
typedef struct {
    int num_items;
    int num_objectives;
    double *capacities;
    int **weights;
    int **profits;
} knapsack_problem_t;

typedef struct {
    int *items;        // Binary selection array
    double *objectives; // Objective function values
    double *capacities_used; // Capacity usage
    int num_items;
    int num_objectives;
} knapsack_solution_t;

typedef struct {
    knapsack_solution_t *solutions;
    int count;
    int capacity;
} knapsack_result_t;

typedef struct {
    int population_size;     // Default: 10
    int max_iterations;      // Default: 100
    double perturbation_rate; // Default: 0.05
    double kappa;            // Default: 0.05
    int local_search_depth;  // Default: 5
    unsigned int seed;       // 0 for time-based
} knapsack_params_t;
```

#### Core Functions

```c
// Problem management
knapsack_problem_t* knapsack_create_problem(int num_items, int num_objectives, 
                                           const double *capacities, 
                                           const int **weights, const int **profits);
knapsack_problem_t* knapsack_load_problem(const char *filename);
void knapsack_free_problem(knapsack_problem_t *problem);

// Algorithm execution
knapsack_params_t knapsack_default_params(void);
int knapsack_solve(const knapsack_problem_t *problem, 
                   const knapsack_params_t *params,
                   knapsack_result_t **result);

// Result access
int knapsack_get_solution_count(const knapsack_result_t *result);
const knapsack_solution_t* knapsack_get_solution(const knapsack_result_t *result, int index);
void knapsack_free_result(knapsack_result_t *result);

// Utility functions
int knapsack_is_feasible(const knapsack_problem_t *problem, 
                        const knapsack_solution_t *solution);
const char* knapsack_get_error(void);
const char* knapsack_get_version(void);
```

### Python API (knapsack_python.py)

#### Main Classes

```python
class KnapsackSolver:
    def __init__(self, dll_path=None)
    def get_version(self) -> str
    def load_problem(self, filename: str) -> KnapsackProblem
    def create_problem(self, num_items, num_objectives, capacities, weights, profits) -> KnapsackProblem
    def solve(self, problem: KnapsackProblem, params=None) -> KnapsackResult
    def default_params(self) -> KnapsackParams

class KnapsackResult:
    @property
    def count(self) -> int
    def get_solution(self, index: int) -> KnapsackSolution
    def get_all_solutions(self) -> List[KnapsackSolution]
    def __iter__(self)  # Iterate over solutions
    def __len__(self) -> int

class KnapsackSolution:
    @property
    def items(self) -> List[int]  # Binary selection array
    @property
    def objectives(self) -> List[float]
    @property
    def capacities_used(self) -> List[float]
    @property
    def selected_items(self) -> List[int]  # Indices of selected items
    @property
    def num_selected(self) -> int
```

#### Convenience Functions

```python
# Quick solve from file
result = load_and_solve("problem.txt", max_iterations=200, population_size=30)

# Quick solve from arrays
result = solve_problem_arrays(num_items, num_objectives, capacities, weights, profits,
                             max_iterations=100, population_size=20)
```

## Problem File Format

The algorithm uses the same format as the original C implementation:

```
<num_objectives> <num_items>
<capacity_1>
<item_1_label>:
<weight_obj1_item1>
<profit_obj1_item1>
<item_2_label>:
<weight_obj1_item2>
<profit_obj1_item2>
...
<capacity_2>
<item_1_label>:
<weight_obj2_item1>
<profit_obj2_item1>
...
```

Example (`test_problem.txt`):
```
2 5
10.0
1:
2
3
2:
3
4
3:
4
5
4:
5
6
5:
1
2
15.0
1:
1
5
2:
2
6
3:
3
7
4:
4
8
5:
2
4
```

## Algorithm Details

The wrapper implements the **IBMOLS (Indicator-Based Multi-Objective Local Search)** algorithm:

- **Multi-objective optimization** supporting 2-4 objectives
- **Pareto dominance** based selection
- **Epsilon indicator** for solution quality assessment
- **Local search** with perturbation for exploration
- **Population-based** approach with archive maintenance

### Key Features

1. **Cross-platform compatibility** - Windows, Linux, macOS
2. **Memory management** - Automatic cleanup and error handling
3. **Thread safety** - Local data copies for concurrent use
4. **Error reporting** - Detailed error messages and codes
5. **Flexible API** - Both C and Python interfaces
6. **Performance** - Optimized core algorithm from original implementation

## Build Requirements

### Linux/macOS
- GCC or Clang compiler
- Standard C library
- Math library (`-lm`)

### Windows
- MinGW-w64 or MSVC
- Windows SDK (for native compilation)

### Python
- Python 3.6+
- ctypes (standard library)
- numpy (optional, for array operations)

## Cross-Platform Notes

### Windows DLL
- Exports using `__declspec(dllexport)`
- Calling convention: `__stdcall`
- Import library generated: `libknapsack.a`

### Linux Shared Library
- Extension: `.so`
- Position Independent Code (`-fPIC`)
- Standard calling convention

### macOS Dynamic Library
- Extension: `.dylib`
- Compatible with both Intel and Apple Silicon

## Performance Considerations

- **Memory usage**: O(population_size × num_items × num_objectives)
- **Time complexity**: O(iterations × population_size × num_items)
- **Typical runtime**: Seconds to minutes depending on problem size
- **Scalability**: Tested up to 1000 items, 4 objectives

## Troubleshooting

### Compilation Issues
```bash
# Check compiler
gcc --version

# Clean and rebuild
make clean && make

# Verbose compilation
make CFLAGS="-v -g"
```

### Runtime Issues
```bash
# Check library path (Linux/macOS)
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.

# Test basic functionality
./simple_test

# Check Python interface
python3 -c "import knapsack_python; print('OK')"
```

### Common Problems

1. **"Library not found"** - Ensure DLL/SO is in current directory or system path
2. **"Permission denied"** - Check file permissions: `chmod +x libknapsack.so`
3. **"Segmentation fault"** - Usually memory management issue, check input validation
4. **"Import error"** - Ensure Python can find the module: `export PYTHONPATH=$PYTHONPATH:.`

## Contributing

The wrapper preserves the original algorithm's functionality while providing modern interfaces. To extend:

1. **Add new functions** to `knapsack_dll.h` and `knapsack_dll.c`
2. **Update Python wrapper** in `knapsack_python.py`
3. **Add tests** to verify functionality
4. **Update documentation** with new features

## License

This wrapper implementation follows the same license as the original algorithm. Please refer to the original repository for licensing information.

## Contact

For issues with the wrapper implementation, please check:
1. Original algorithm functionality in `main.c`
2. Wrapper-specific issues in DLL files
3. Python interface problems in `knapsack_python.py`

---

*This wrapper enables seamless integration of the multi-objective knapsack optimization algorithm into modern Python workflows while maintaining the performance and accuracy of the original C implementation.*