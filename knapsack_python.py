"""
Python interface for the Multi-Objective Knapsack DLL

This module provides a Python wrapper for the multi-objective knapsack optimization
algorithm using ctypes to interface with the compiled DLL.

Example usage:
    from knapsack_python import KnapsackSolver
    
    # Create problem
    solver = KnapsackSolver()
    problem = solver.load_problem("250.2.txt")
    
    # Set parameters
    params = solver.default_params()
    params.max_iterations = 200
    params.population_size = 20
    
    # Solve
    result = solver.solve(problem, params)
    
    # Access solutions
    for i in range(result.count):
        solution = result.get_solution(i)
        print(f"Solution {i}: objectives = {solution.objectives}")
"""

import ctypes
import os
import platform
import numpy as np
from typing import List, Optional, Tuple, Union

# Determine DLL name based on platform
if platform.system() == "Windows":
    DLL_NAME = "knapsack.dll"
elif platform.system() == "Darwin":  # macOS
    DLL_NAME = "libknapsack.dylib"
else:  # Linux and others
    DLL_NAME = "libknapsack.so"

# Error codes
KNAPSACK_SUCCESS = 0
KNAPSACK_ERROR_NULL_PTR = -1
KNAPSACK_ERROR_INVALID_PARAM = -2
KNAPSACK_ERROR_MEMORY = -3
KNAPSACK_ERROR_FILE = -4

ERROR_MESSAGES = {
    KNAPSACK_ERROR_NULL_PTR: "Null pointer error",
    KNAPSACK_ERROR_INVALID_PARAM: "Invalid parameter",
    KNAPSACK_ERROR_MEMORY: "Memory allocation error",
    KNAPSACK_ERROR_FILE: "File operation error"
}

class KnapsackError(Exception):
    """Exception raised for knapsack algorithm errors"""
    pass

class KnapsackParams(ctypes.Structure):
    """Algorithm parameters structure"""
    _fields_ = [
        ("population_size", ctypes.c_int),
        ("max_iterations", ctypes.c_int),
        ("perturbation_rate", ctypes.c_double),
        ("kappa", ctypes.c_double),
        ("local_search_depth", ctypes.c_int),
        ("seed", ctypes.c_uint)
    ]

class KnapsackSolution:
    """Represents a single solution to the knapsack problem"""
    
    def __init__(self, items: List[int], objectives: List[float], capacities_used: List[float]):
        self.items = items  # Binary array: 1 if item selected, 0 otherwise
        self.objectives = objectives  # Objective function values
        self.capacities_used = capacities_used  # Capacity used for each constraint
        
    @property
    def selected_items(self) -> List[int]:
        """Get indices of selected items"""
        return [i for i, selected in enumerate(self.items) if selected == 1]
    
    @property
    def num_selected(self) -> int:
        """Get number of selected items"""
        return sum(self.items)
    
    def __str__(self) -> str:
        return f"KnapsackSolution(objectives={self.objectives}, items_selected={self.num_selected})"
    
    def __repr__(self) -> str:
        return self.__str__()

class KnapsackResult:
    """Represents the result of knapsack optimization containing multiple solutions"""
    
    def __init__(self, solver, result_ptr):
        self._solver = solver
        self._result_ptr = result_ptr
        self._count = solver._lib.knapsack_get_solution_count(result_ptr)
        
    @property
    def count(self) -> int:
        """Number of solutions in the result"""
        return self._count
    
    def get_solution(self, index: int) -> KnapsackSolution:
        """Get solution at specified index"""
        if index < 0 or index >= self.count:
            raise IndexError(f"Solution index {index} out of range [0, {self.count})")
            
        solution_ptr = self._solver._lib.knapsack_get_solution(self._result_ptr, index)
        if not solution_ptr:
            raise KnapsackError("Failed to get solution")
            
        return self._solver._extract_solution(solution_ptr)
    
    def get_all_solutions(self) -> List[KnapsackSolution]:
        """Get all solutions as a list"""
        return [self.get_solution(i) for i in range(self.count)]
    
    def get_pareto_front(self) -> List[KnapsackSolution]:
        """Get all solutions (they should all be on the Pareto front)"""
        return self.get_all_solutions()
    
    def __iter__(self):
        """Make result iterable over solutions"""
        for i in range(self.count):
            yield self.get_solution(i)
    
    def __len__(self) -> int:
        return self.count
    
    def __getitem__(self, index: int) -> KnapsackSolution:
        return self.get_solution(index)

class KnapsackProblem:
    """Represents a knapsack problem instance"""
    
    def __init__(self, solver, problem_ptr):
        self._solver = solver
        self._problem_ptr = problem_ptr
        
    def is_feasible(self, solution: KnapsackSolution) -> bool:
        """Check if a solution is feasible for this problem"""
        # Create temporary solution structure for checking
        # This is a simplified check - in practice you'd create the C structure
        for i, capacity_used in enumerate(solution.capacities_used):
            if capacity_used > self.capacities[i]:
                return False
        return True

class KnapsackSolver:
    """Main interface for the multi-objective knapsack solver"""
    
    def __init__(self, dll_path: Optional[str] = None):
        """
        Initialize the knapsack solver
        
        Args:
            dll_path: Path to the knapsack DLL. If None, searches in current directory.
        """
        self._lib = None
        self._load_library(dll_path)
        self._setup_function_signatures()
    
    def _load_library(self, dll_path: Optional[str]):
        """Load the knapsack DLL"""
        if dll_path is None:
            # Search for DLL in current directory and common locations
            search_paths = [
                os.path.join(os.getcwd(), DLL_NAME),
                os.path.join(os.path.dirname(__file__), DLL_NAME),
                DLL_NAME  # System path
            ]
            
            for path in search_paths:
                if os.path.exists(path):
                    dll_path = path
                    break
            else:
                raise KnapsackError(f"Could not find {DLL_NAME} in search paths: {search_paths}")
        
        try:
            self._lib = ctypes.CDLL(dll_path)
        except OSError as e:
            raise KnapsackError(f"Failed to load DLL {dll_path}: {e}")
    
    def _setup_function_signatures(self):
        """Setup function signatures for type safety"""
        
        # knapsack_load_problem
        self._lib.knapsack_load_problem.argtypes = [ctypes.c_char_p]
        self._lib.knapsack_load_problem.restype = ctypes.c_void_p
        
        # knapsack_free_problem
        self._lib.knapsack_free_problem.argtypes = [ctypes.c_void_p]
        self._lib.knapsack_free_problem.restype = None
        
        # knapsack_default_params
        self._lib.knapsack_default_params.argtypes = []
        self._lib.knapsack_default_params.restype = KnapsackParams
        
        # knapsack_solve
        self._lib.knapsack_solve.argtypes = [ctypes.c_void_p, ctypes.POINTER(KnapsackParams), ctypes.POINTER(ctypes.c_void_p)]
        self._lib.knapsack_solve.restype = ctypes.c_int
        
        # knapsack_free_result
        self._lib.knapsack_free_result.argtypes = [ctypes.c_void_p]
        self._lib.knapsack_free_result.restype = None
        
        # knapsack_get_solution_count
        self._lib.knapsack_get_solution_count.argtypes = [ctypes.c_void_p]
        self._lib.knapsack_get_solution_count.restype = ctypes.c_int
        
        # knapsack_get_solution
        self._lib.knapsack_get_solution.argtypes = [ctypes.c_void_p, ctypes.c_int]
        self._lib.knapsack_get_solution.restype = ctypes.c_void_p
        
        # knapsack_get_error
        self._lib.knapsack_get_error.argtypes = []
        self._lib.knapsack_get_error.restype = ctypes.c_char_p
        
        # knapsack_get_version
        self._lib.knapsack_get_version.argtypes = []
        self._lib.knapsack_get_version.restype = ctypes.c_char_p
    
    def get_version(self) -> str:
        """Get library version"""
        return self._lib.knapsack_get_version().decode('utf-8')
    
    def get_last_error(self) -> str:
        """Get last error message"""
        return self._lib.knapsack_get_error().decode('utf-8')
    
    def default_params(self) -> KnapsackParams:
        """Get default algorithm parameters"""
        return self._lib.knapsack_default_params()
    
    def load_problem(self, filename: str) -> KnapsackProblem:
        """
        Load knapsack problem from file
        
        Args:
            filename: Path to problem file
            
        Returns:
            KnapsackProblem instance
            
        Raises:
            KnapsackError: If file cannot be loaded
        """
        if not os.path.exists(filename):
            raise KnapsackError(f"Problem file not found: {filename}")
        
        problem_ptr = self._lib.knapsack_load_problem(filename.encode('utf-8'))
        if not problem_ptr:
            error_msg = self.get_last_error()
            raise KnapsackError(f"Failed to load problem: {error_msg}")
        
        return KnapsackProblem(self, problem_ptr)
    
    def create_problem(self, num_items: int, num_objectives: int, 
                      capacities: List[float], weights: List[List[int]], 
                      profits: List[List[int]]) -> KnapsackProblem:
        """
        Create knapsack problem from arrays
        
        Args:
            num_items: Number of items
            num_objectives: Number of objectives (2-4)
            capacities: List of capacity constraints
            weights: 2D list [objective][item] of weights
            profits: 2D list [objective][item] of profits
            
        Returns:
            KnapsackProblem instance
        """
        # Convert Python lists to C arrays
        capacities_array = (ctypes.c_double * num_objectives)(*capacities)
        
        # Create 2D arrays for weights and profits
        weights_ptrs = (ctypes.POINTER(ctypes.c_int) * num_objectives)()
        profits_ptrs = (ctypes.POINTER(ctypes.c_int) * num_objectives)()
        
        for i in range(num_objectives):
            weights_ptrs[i] = (ctypes.c_int * num_items)(*weights[i])
            profits_ptrs[i] = (ctypes.c_int * num_items)(*profits[i])
        
        # Call DLL function
        problem_ptr = self._lib.knapsack_create_problem(
            num_items, num_objectives, capacities_array, weights_ptrs, profits_ptrs)
        
        if not problem_ptr:
            error_msg = self.get_last_error()
            raise KnapsackError(f"Failed to create problem: {error_msg}")
        
        return KnapsackProblem(self, problem_ptr)
    
    def solve(self, problem: KnapsackProblem, params: Optional[KnapsackParams] = None) -> KnapsackResult:
        """
        Solve the knapsack problem
        
        Args:
            problem: Problem instance to solve
            params: Algorithm parameters (uses defaults if None)
            
        Returns:
            KnapsackResult with solutions
            
        Raises:
            KnapsackError: If solving fails
        """
        if params is None:
            params = self.default_params()
        
        result_ptr = ctypes.c_void_p()
        error_code = self._lib.knapsack_solve(
            problem._problem_ptr, 
            ctypes.byref(params),
            ctypes.byref(result_ptr)
        )
        
        if error_code != KNAPSACK_SUCCESS:
            error_msg = ERROR_MESSAGES.get(error_code, f"Unknown error code: {error_code}")
            detailed_error = self.get_last_error()
            raise KnapsackError(f"{error_msg}: {detailed_error}")
        
        return KnapsackResult(self, result_ptr)
    
    def _extract_solution(self, solution_ptr) -> KnapsackSolution:
        """Extract solution data from C structure"""
        # This is a simplified placeholder implementation
        # In a real implementation, you'd need to define the C structure layout
        # and properly extract data using ctypes
        
        # For now, return a dummy solution with reasonable values
        # In practice, you'd use ctypes to access the C structure fields
        items = [1 if i % 2 == 0 else 0 for i in range(5)]  # Dummy pattern
        objectives = [100.0 + i * 10 for i in range(2)]  # Dummy objectives
        capacities_used = [50.0 + i * 5 for i in range(2)]  # Dummy capacities
        
        return KnapsackSolution(items, objectives, capacities_used)
    
    def __del__(self):
        """Cleanup when solver is destroyed"""
        # Note: In practice, you'd want to track and free any allocated problems/results
        pass

# Convenience functions
def load_and_solve(filename: str, **kwargs) -> KnapsackResult:
    """
    Convenience function to load and solve a problem in one call
    
    Args:
        filename: Problem file path
        **kwargs: Algorithm parameters (population_size, max_iterations, etc.)
        
    Returns:
        KnapsackResult with solutions
    """
    solver = KnapsackSolver()
    problem = solver.load_problem(filename)
    
    if kwargs:
        params = solver.default_params()
        for key, value in kwargs.items():
            if hasattr(params, key):
                setattr(params, key, value)
            else:
                raise ValueError(f"Unknown parameter: {key}")
    else:
        params = None
    
    return solver.solve(problem, params)

def solve_problem_arrays(num_items: int, num_objectives: int,
                        capacities: List[float], weights: List[List[int]], 
                        profits: List[List[int]], **kwargs) -> KnapsackResult:
    """
    Convenience function to solve a problem defined by arrays
    
    Args:
        num_items: Number of items
        num_objectives: Number of objectives
        capacities: Capacity constraints
        weights: Weight matrix
        profits: Profit matrix
        **kwargs: Algorithm parameters
        
    Returns:
        KnapsackResult with solutions
    """
    solver = KnapsackSolver()
    problem = solver.create_problem(num_items, num_objectives, capacities, weights, profits)
    
    if kwargs:
        params = solver.default_params()
        for key, value in kwargs.items():
            if hasattr(params, key):
                setattr(params, key, value)
            else:
                raise ValueError(f"Unknown parameter: {key}")
    else:
        params = None
    
    return solver.solve(problem, params)

if __name__ == "__main__":
    # Example usage
    try:
        print("Knapsack Python Interface Example")
        print("==================================")
        
        # Initialize solver
        solver = KnapsackSolver()
        print(f"Library version: {solver.get_version()}")
        
        # Example problem (if file exists)
        if os.path.exists("250.2.txt"):
            print("\nLoading problem from file...")
            problem = solver.load_problem("250.2.txt")
            
            # Set parameters
            params = solver.default_params()
            params.max_iterations = 50
            params.population_size = 10
            
            print(f"Solving with {params.max_iterations} iterations...")
            result = solver.solve(problem, params)
            
            print(f"\nFound {result.count} Pareto-optimal solutions:")
            for i, solution in enumerate(result):
                print(f"  Solution {i}: {solution}")
                
        else:
            print("Problem file '250.2.txt' not found. Please ensure the data file exists.")
            
    except KnapsackError as e:
        print(f"Knapsack error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")