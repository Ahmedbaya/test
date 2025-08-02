# MOKP DLL Wrapper - Comprehensive Documentation

## Overview

This package provides a complete DLL wrapper for the Multi-Objective Knapsack Problem (MOKP) optimization algorithm, enabling seamless integration with Python and other languages through a C-compatible API.

## Features

- **Complete Algorithm Preservation**: Maintains exact functionality of the original C implementation
- **Cross-Platform Compatibility**: Supports Windows (DLL), Linux (shared library), and macOS
- **Python Integration**: High-level Python interface with comprehensive error handling
- **Memory Management**: Automatic memory allocation and cleanup
- **Parameter Control**: Full access to algorithm parameters and configuration
- **Multi-Format Output**: Support for various output formats and result extraction

## Architecture

### Core Components

1. **mokp_dll.h** - DLL interface header with function declarations
2. **mokp_dll.c** - DLL implementation wrapping the original C code  
3. **mokp_python.py** - Python interface using ctypes
4. **Makefile** - Build system for all platforms

### Data Structures

#### Individual (`ind`)
- `f[]` - Objective function values
- `capa[]` - Capacity usage per objective
- `Items[]` - Binary selection array (0/1 for each item)
- `fitness` - Individual fitness value
- `nombr` - Number of selected items

#### Population (`pop`)
- `ind_array[]` - Array of individuals
- `size` - Current population size
- `maxsize` - Maximum population capacity

#### Global Arrays
- `weights[dimension][NBITEMS]` - Item weights per objective
- `profits[dimension][NBITEMS]` - Item profits per objective
- `capacities[dimension]` - Knapsack capacities per objective

## Installation and Building

### Prerequisites

```bash
# Linux/macOS
sudo apt-get install build-essential gcc make

# Windows (MinGW)
# Install MinGW-w64 or use MSYS2
```

### Building the Library

#### Linux/macOS
```bash
# Build shared library
make

# Build static library
make static

# Build with debug symbols
make debug

# Build test program
make test

# Run tests
make run-test
```

#### Windows
```bash
# Build DLL using MinGW
make dll

# Or using Visual Studio (create project with provided source files)
```

### Installation
```bash
# Linux system-wide installation
sudo make install

# Manual installation - copy files to desired location
cp libmokp.so /usr/local/lib/
cp mokp_dll.h /usr/local/include/
```

## API Reference

### Core Functions

#### Initialization and Cleanup
```c
int mokp_initialize();
int mokp_cleanup();
```

#### Problem Loading
```c
int mokp_load_problem(const char* problem_file);
int mokp_load_weights(const char* weights_file);
```

#### Parameter Management
```c
int mokp_set_parameters(const MOKP_Parameters* params);
int mokp_get_parameters(MOKP_Parameters* params);
```

#### Optimization
```c
int mokp_run_optimization(int max_iterations);
int mokp_run_single_iteration();
```

#### Results Extraction
```c
int mokp_get_pareto_size();
int mokp_get_pareto_solutions(MOKP_Solution* solutions, int max_solutions);
```

#### Solution Evaluation
```c
int mokp_evaluate_solution(const int* items, MOKP_Solution* solution);
int mokp_is_solution_feasible(const int* items);
```

#### Utility Functions
```c
int mokp_get_problem_info(int* num_objectives, int* num_items);
const char* mokp_get_last_error();
double mokp_get_last_iteration_time();
int mokp_save_results(const char* output_file);
```

### Error Codes

```c
#define MOKP_SUCCESS 0
#define MOKP_ERROR_INVALID_PARAMETER -1
#define MOKP_ERROR_FILE_NOT_FOUND -2
#define MOKP_ERROR_MEMORY_ALLOCATION -3
#define MOKP_ERROR_NOT_INITIALIZED -4
#define MOKP_ERROR_INVALID_STATE -5
```

### Parameter Structure

```c
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
```

### Solution Structure

```c
typedef struct {
    double objectives[MOKP_MAX_OBJECTIVES];
    int items[MOKP_MAX_ITEMS];
    int num_items_selected;
    double capacities[MOKP_MAX_OBJECTIVES];
} MOKP_Solution;
```

## Usage Examples

### C/C++ Usage

```c
#include "mokp_dll.h"

int main() {
    // Initialize
    if (mokp_initialize() != MOKP_SUCCESS) {
        fprintf(stderr, "Initialization failed: %s\n", mokp_get_last_error());
        return 1;
    }
    
    // Load problem
    if (mokp_load_problem("250.2.txt") != MOKP_SUCCESS) {
        fprintf(stderr, "Load failed: %s\n", mokp_get_last_error());
        mokp_cleanup();
        return 1;
    }
    
    // Load weights
    mokp_load_weights("Weights_2obj_FQ200.txt");
    
    // Set parameters
    MOKP_Parameters params;
    mokp_get_parameters(&params);
    params.max_iterations = 100;
    params.population_size = 20;
    mokp_set_parameters(&params);
    
    // Run optimization
    mokp_run_optimization(100);
    
    // Get results
    int pareto_size = mokp_get_pareto_size();
    MOKP_Solution* solutions = malloc(pareto_size * sizeof(MOKP_Solution));
    int count = mokp_get_pareto_solutions(solutions, pareto_size);
    
    // Print results
    for (int i = 0; i < count; i++) {
        printf("Solution %d: %.2f %.2f\n", i, 
               solutions[i].objectives[0], 
               solutions[i].objectives[1]);
    }
    
    free(solutions);
    mokp_cleanup();
    return 0;
}
```

### Python Usage

```python
from mokp_python import MOKP

# Using context manager (recommended)
with MOKP() as mokp:
    mokp.load_problem("250.2.txt")
    mokp.load_weights("Weights_2obj_FQ200.txt")
    
    mokp.set_parameters(
        max_iterations=100,
        population_size=20,
        perturbation_rate=0.05
    )
    
    mokp.run_optimization(100)
    
    solutions = mokp.get_pareto_solutions()
    for i, sol in enumerate(solutions):
        print(f"Solution {i}: {sol['objectives']}")

# Manual management
mokp = MOKP()
mokp.initialize()
try:
    # ... use mokp
finally:
    mokp.cleanup()
```

## File Formats

### Problem File Format
```
<num_objectives> <num_items>
<capacity_1>
<item_type_1>
<weight_1_obj_1>
<profit_1_obj_1>
<item_type_2>
<weight_2_obj_1>
<profit_2_obj_1>
...
<capacity_2>
<item_type_1>
<weight_1_obj_2>
<profit_1_obj_2>
...
```

### Weights File Format
```
<weight_1_obj_1> <weight_1_obj_2> ... <weight_1_obj_n>
<weight_2_obj_1> <weight_2_obj_2> ... <weight_2_obj_n>
...
```

## Algorithm Parameters

### Core Parameters
- **population_size** (default: 10): Size of the working population
- **archive_size** (default: 28000): Maximum Pareto archive size
- **perturbation_rate** (default: 0.05): Rate of solution perturbation (0.0-1.0)
- **kappa** (default: 0.05): Indicator calculation parameter
- **local_search_depth** (default: 5): Depth of local search operations

### Problem Parameters
- **num_objectives**: Number of optimization objectives (2-4)
- **num_items**: Number of items in the knapsack (up to 250)

## Performance Characteristics

### Time Complexity
- **Single Iteration**: O(α × N × L) where α=population size, N=items, L=search depth
- **Full Algorithm**: O(iterations × α × N × L)

### Memory Usage
- **Base Memory**: ~100MB for default parameters
- **Per Solution**: ~2KB (250 items × 2-4 objectives)
- **Archive**: Up to 28,000 solutions (~56MB max)

### Scalability
- **Items**: Tested up to 250 items (configurable)
- **Objectives**: Supports 2-4 objectives
- **Solutions**: Can handle thousands of Pareto solutions

## Platform-Specific Notes

### Windows
- Use MinGW-w64 for compilation
- DLL exports use `__stdcall` convention
- Library name: `mokp.dll`

### Linux
- Requires `libm` (math library)
- Uses default visibility attributes
- Library name: `libmokp.so`

### macOS
- Similar to Linux with clang compiler
- Library name: `libmokp.dylib`

## Troubleshooting

### Common Issues

1. **Library not found**
   ```bash
   # Linux/macOS
   export LD_LIBRARY_PATH=/path/to/library:$LD_LIBRARY_PATH
   
   # Or copy to system directory
   sudo cp libmokp.so /usr/local/lib/
   sudo ldconfig
   ```

2. **Permission denied**
   ```bash
   chmod +x libmokp.so
   ```

3. **Python import errors**
   ```python
   # Specify library path explicitly
   mokp = MOKP(lib_path="/full/path/to/libmokp.so")
   ```

### Debug Mode
Build with debug symbols:
```bash
make debug
gdb ./test_mokp
```

### Memory Leaks
Use valgrind on Linux:
```bash
valgrind --leak-check=full ./test_mokp
```

## Testing

### Unit Tests
```bash
make test
./test_mokp
```

### Integration Tests
```bash
python3 mokp_python.py
```

### Performance Testing
```bash
time ./test_mokp  # Basic timing
```

## Contributing

### Code Style
- Follow existing C99 standard
- Use consistent indentation (4 spaces)
- Document all public functions
- Include error handling

### Adding Features
1. Extend the C API in `mokp_dll.h`
2. Implement in `mokp_dll.c`
3. Add Python bindings in `mokp_python.py`
4. Update tests and documentation

## License

This project preserves the license of the original MOKP implementation while adding the DLL wrapper functionality.

## References

- Original MOKP Algorithm: Indicator-Based Multi-Objective Local Search
- Problem Definition: Multi-Objective Knapsack Problem
- Optimization Technique: Local Search with Pareto Archive

## Support

For issues and questions:
1. Check the troubleshooting section
2. Review the test programs for usage examples
3. Examine the Python interface for high-level usage patterns