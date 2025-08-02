#!/usr/bin/env python3
"""
Python interface for the MOKP (Multi-Objective Knapsack Problem) DLL
This module provides a Python wrapper for the C DLL using ctypes.
"""

import ctypes
import os
import sys
from typing import List, Tuple, Optional

# Platform-specific library loading
if sys.platform.startswith('win'):
    LIB_NAME = 'mokp.dll'
elif sys.platform.startswith('linux'):
    LIB_NAME = 'libmokp.so'
elif sys.platform.startswith('darwin'):
    LIB_NAME = 'libmokp.dylib'
else:
    LIB_NAME = 'libmokp.so'


class MOKPError(Exception):
    """Exception raised for MOKP-related errors."""
    pass


class MOKPParameters(ctypes.Structure):
    """Parameters structure for MOKP algorithm."""
    _fields_ = [
        ("max_iterations", ctypes.c_int),
        ("population_size", ctypes.c_int),
        ("archive_size", ctypes.c_int),
        ("perturbation_rate", ctypes.c_double),
        ("kappa", ctypes.c_double),
        ("num_objectives", ctypes.c_int),
        ("num_items", ctypes.c_int),
        ("local_search_depth", ctypes.c_int),
    ]


class MOKPSolution(ctypes.Structure):
    """Solution structure for MOKP results."""
    _fields_ = [
        ("objectives", ctypes.c_double * 4),  # MOKP_MAX_OBJECTIVES
        ("items", ctypes.c_int * 250),        # MOKP_MAX_ITEMS
        ("num_items_selected", ctypes.c_int),
        ("capacities", ctypes.c_double * 4),  # MOKP_MAX_OBJECTIVES
    ]


class MOKP:
    """
    Python interface to the MOKP DLL.
    
    This class provides a high-level interface to the Multi-Objective Knapsack
    Problem optimization algorithm implemented in C.
    
    Example:
        mokp = MOKP()
        mokp.initialize()
        mokp.load_problem("250.2.txt")
        mokp.load_weights("Weights_2obj_FQ200.txt")
        mokp.run_optimization(100)
        solutions = mokp.get_pareto_solutions()
        mokp.cleanup()
    """
    
    # Error codes
    SUCCESS = 0
    ERROR_INVALID_PARAMETER = -1
    ERROR_FILE_NOT_FOUND = -2
    ERROR_MEMORY_ALLOCATION = -3
    ERROR_NOT_INITIALIZED = -4
    ERROR_INVALID_STATE = -5
    
    def __init__(self, lib_path: Optional[str] = None):
        """
        Initialize the MOKP interface.
        
        Args:
            lib_path: Path to the MOKP library. If None, searches in current directory.
        """
        self._lib = None
        self._initialized = False
        
        # Try to load the library
        if lib_path is None:
            lib_path = os.path.join(os.path.dirname(__file__), LIB_NAME)
        
        try:
            self._lib = ctypes.CDLL(lib_path)
        except OSError as e:
            raise MOKPError(f"Failed to load MOKP library from {lib_path}: {e}")
        
        # Define function signatures
        self._setup_function_signatures()
    
    def _setup_function_signatures(self):
        """Set up the function signatures for the C library."""
        
        # Core functions
        self._lib.mokp_initialize.argtypes = []
        self._lib.mokp_initialize.restype = ctypes.c_int
        
        self._lib.mokp_cleanup.argtypes = []
        self._lib.mokp_cleanup.restype = ctypes.c_int
        
        self._lib.mokp_load_problem.argtypes = [ctypes.c_char_p]
        self._lib.mokp_load_problem.restype = ctypes.c_int
        
        self._lib.mokp_load_weights.argtypes = [ctypes.c_char_p]
        self._lib.mokp_load_weights.restype = ctypes.c_int
        
        self._lib.mokp_set_parameters.argtypes = [ctypes.POINTER(MOKPParameters)]
        self._lib.mokp_set_parameters.restype = ctypes.c_int
        
        self._lib.mokp_get_parameters.argtypes = [ctypes.POINTER(MOKPParameters)]
        self._lib.mokp_get_parameters.restype = ctypes.c_int
        
        # Optimization functions
        self._lib.mokp_run_optimization.argtypes = [ctypes.c_int]
        self._lib.mokp_run_optimization.restype = ctypes.c_int
        
        self._lib.mokp_run_single_iteration.argtypes = []
        self._lib.mokp_run_single_iteration.restype = ctypes.c_int
        
        self._lib.mokp_get_pareto_size.argtypes = []
        self._lib.mokp_get_pareto_size.restype = ctypes.c_int
        
        self._lib.mokp_get_pareto_solutions.argtypes = [
            ctypes.POINTER(MOKPSolution), ctypes.c_int
        ]
        self._lib.mokp_get_pareto_solutions.restype = ctypes.c_int
        
        # Utility functions
        self._lib.mokp_evaluate_solution.argtypes = [
            ctypes.POINTER(ctypes.c_int), ctypes.POINTER(MOKPSolution)
        ]
        self._lib.mokp_evaluate_solution.restype = ctypes.c_int
        
        self._lib.mokp_is_solution_feasible.argtypes = [ctypes.POINTER(ctypes.c_int)]
        self._lib.mokp_is_solution_feasible.restype = ctypes.c_int
        
        self._lib.mokp_get_problem_info.argtypes = [
            ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)
        ]
        self._lib.mokp_get_problem_info.restype = ctypes.c_int
        
        self._lib.mokp_save_results.argtypes = [ctypes.c_char_p]
        self._lib.mokp_save_results.restype = ctypes.c_int
        
        # Error handling
        self._lib.mokp_get_last_error.argtypes = []
        self._lib.mokp_get_last_error.restype = ctypes.c_char_p
        
        self._lib.mokp_get_last_iteration_time.argtypes = []
        self._lib.mokp_get_last_iteration_time.restype = ctypes.c_double
    
    def _check_result(self, result: int, operation: str = "operation"):
        """Check the result of a C function call and raise exception if error."""
        if result != self.SUCCESS:
            error_msg = self._lib.mokp_get_last_error().decode('utf-8')
            raise MOKPError(f"Error in {operation}: {error_msg} (code: {result})")
    
    def initialize(self) -> None:
        """Initialize the MOKP library."""
        result = self._lib.mokp_initialize()
        self._check_result(result, "initialization")
        self._initialized = True
    
    def cleanup(self) -> None:
        """Clean up and free resources."""
        if self._initialized:
            result = self._lib.mokp_cleanup()
            self._check_result(result, "cleanup")
            self._initialized = False
    
    def __enter__(self):
        """Context manager entry."""
        self.initialize()
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit."""
        self.cleanup()
    
    def load_problem(self, filename: str) -> None:
        """
        Load a problem instance from file.
        
        Args:
            filename: Path to the problem file.
        """
        result = self._lib.mokp_load_problem(filename.encode('utf-8'))
        self._check_result(result, "loading problem")
    
    def load_weights(self, filename: str) -> None:
        """
        Load weight vectors from file.
        
        Args:
            filename: Path to the weights file.
        """
        result = self._lib.mokp_load_weights(filename.encode('utf-8'))
        self._check_result(result, "loading weights")
    
    def set_parameters(self, **kwargs) -> None:
        """
        Set algorithm parameters.
        
        Args:
            max_iterations: Maximum number of iterations
            population_size: Population size
            archive_size: Archive size
            perturbation_rate: Perturbation rate (0.0-1.0)
            kappa: Kappa parameter
            num_objectives: Number of objectives
            num_items: Number of items
            local_search_depth: Local search depth
        """
        params = MOKPParameters()
        
        # Get current parameters
        result = self._lib.mokp_get_parameters(ctypes.byref(params))
        self._check_result(result, "getting parameters")
        
        # Update with provided values
        for key, value in kwargs.items():
            if hasattr(params, key):
                setattr(params, key, value)
            else:
                raise ValueError(f"Unknown parameter: {key}")
        
        # Set updated parameters
        result = self._lib.mokp_set_parameters(ctypes.byref(params))
        self._check_result(result, "setting parameters")
    
    def get_parameters(self) -> dict:
        """
        Get current algorithm parameters.
        
        Returns:
            Dictionary of parameter name-value pairs.
        """
        params = MOKPParameters()
        result = self._lib.mokp_get_parameters(ctypes.byref(params))
        self._check_result(result, "getting parameters")
        
        return {
            "max_iterations": params.max_iterations,
            "population_size": params.population_size,
            "archive_size": params.archive_size,
            "perturbation_rate": params.perturbation_rate,
            "kappa": params.kappa,
            "num_objectives": params.num_objectives,
            "num_items": params.num_items,
            "local_search_depth": params.local_search_depth,
        }
    
    def run_optimization(self, max_iterations: int) -> None:
        """
        Run the complete optimization algorithm.
        
        Args:
            max_iterations: Maximum number of iterations to run.
        """
        result = self._lib.mokp_run_optimization(max_iterations)
        self._check_result(result, "optimization")
    
    def run_single_iteration(self) -> None:
        """Run a single iteration of the optimization algorithm."""
        result = self._lib.mokp_run_single_iteration()
        self._check_result(result, "single iteration")
    
    def get_pareto_size(self) -> int:
        """
        Get the number of solutions in the Pareto set.
        
        Returns:
            Number of Pareto solutions.
        """
        return self._lib.mokp_get_pareto_size()
    
    def get_pareto_solutions(self, max_solutions: Optional[int] = None) -> List[dict]:
        """
        Get the Pareto solutions.
        
        Args:
            max_solutions: Maximum number of solutions to retrieve.
        
        Returns:
            List of solution dictionaries.
        """
        pareto_size = self.get_pareto_size()
        if pareto_size == 0:
            return []
        
        if max_solutions is None:
            max_solutions = pareto_size
        else:
            max_solutions = min(max_solutions, pareto_size)
        
        # Allocate array for solutions
        solutions_array = (MOKPSolution * max_solutions)()
        
        # Get solutions from C library
        result = self._lib.mokp_get_pareto_solutions(solutions_array, max_solutions)
        if result < 0:
            self._check_result(result, "getting Pareto solutions")
        
        num_retrieved = result
        
        # Convert to Python format
        solutions = []
        for i in range(num_retrieved):
            sol = solutions_array[i]
            solutions.append({
                'objectives': list(sol.objectives),
                'items': list(sol.items),
                'num_items_selected': sol.num_items_selected,
                'capacities': list(sol.capacities),
            })
        
        return solutions
    
    def evaluate_solution(self, items: List[int]) -> dict:
        """
        Evaluate a given solution.
        
        Args:
            items: List of 0/1 values indicating which items are selected.
        
        Returns:
            Dictionary with objectives, capacities, and feasibility.
        """
        if len(items) > 250:  # MOKP_MAX_ITEMS
            raise ValueError("Too many items")
        
        # Convert to C array
        items_array = (ctypes.c_int * 250)()
        for i, item in enumerate(items):
            items_array[i] = item
        
        solution = MOKPSolution()
        result = self._lib.mokp_evaluate_solution(items_array, ctypes.byref(solution))
        self._check_result(result, "evaluating solution")
        
        return {
            'objectives': list(solution.objectives),
            'items': list(solution.items),
            'num_items_selected': solution.num_items_selected,
            'capacities': list(solution.capacities),
        }
    
    def is_solution_feasible(self, items: List[int]) -> bool:
        """
        Check if a solution is feasible.
        
        Args:
            items: List of 0/1 values indicating which items are selected.
        
        Returns:
            True if feasible, False otherwise.
        """
        if len(items) > 250:  # MOKP_MAX_ITEMS
            raise ValueError("Too many items")
        
        # Convert to C array
        items_array = (ctypes.c_int * 250)()
        for i, item in enumerate(items):
            items_array[i] = item
        
        result = self._lib.mokp_is_solution_feasible(items_array)
        if result < 0:
            self._check_result(result, "checking feasibility")
        
        return result == 1
    
    def get_problem_info(self) -> Tuple[int, int]:
        """
        Get problem information.
        
        Returns:
            Tuple of (num_objectives, num_items).
        """
        num_objectives = ctypes.c_int()
        num_items = ctypes.c_int()
        
        result = self._lib.mokp_get_problem_info(
            ctypes.byref(num_objectives), ctypes.byref(num_items)
        )
        self._check_result(result, "getting problem info")
        
        return num_objectives.value, num_items.value
    
    def save_results(self, filename: str) -> None:
        """
        Save Pareto solutions to file.
        
        Args:
            filename: Output file path.
        """
        result = self._lib.mokp_save_results(filename.encode('utf-8'))
        self._check_result(result, "saving results")
    
    def get_last_iteration_time(self) -> float:
        """
        Get the execution time of the last iteration.
        
        Returns:
            Time in seconds.
        """
        return self._lib.mokp_get_last_iteration_time()


def main():
    """Example usage of the MOKP Python interface."""
    
    print("=== MOKP Python Interface Example ===\n")
    
    try:
        # Initialize MOKP using context manager
        with MOKP() as mokp:
            print("1. Loading problem and weights...")
            mokp.load_problem("250.2.txt")
            mokp.load_weights("Weights_2obj_FQ200.txt")
            
            print("2. Getting problem information...")
            num_objectives, num_items = mokp.get_problem_info()
            print(f"   Problem: {num_objectives} objectives, {num_items} items")
            
            print("3. Setting algorithm parameters...")
            mokp.set_parameters(
                max_iterations=100,
                population_size=10,
                perturbation_rate=0.05
            )
            
            print("4. Running optimization...")
            mokp.run_optimization(20)
            
            print("5. Retrieving results...")
            pareto_size = mokp.get_pareto_size()
            print(f"   Found {pareto_size} Pareto solutions")
            
            if pareto_size > 0:
                solutions = mokp.get_pareto_solutions(max_solutions=5)
                print(f"   Retrieved {len(solutions)} solutions:")
                
                for i, sol in enumerate(solutions):
                    print(f"   Solution {i+1}:")
                    print(f"     Objectives: {sol['objectives'][:num_objectives]}")
                    print(f"     Items selected: {sol['num_items_selected']}")
                    print(f"     Capacities: {sol['capacities'][:num_objectives]}")
                
                print("6. Saving results...")
                mokp.save_results("python_results.txt")
                
                print("7. Testing solution evaluation...")
                # Create a test solution (first 10 items)
                test_items = [1] * 10 + [0] * (num_items - 10)
                evaluation = mokp.evaluate_solution(test_items)
                feasible = mokp.is_solution_feasible(test_items)
                
                print(f"   Test solution: {evaluation['objectives'][:num_objectives]}")
                print(f"   Feasible: {feasible}")
            
            print("\n✓ Example completed successfully!")
    
    except MOKPError as e:
        print(f"MOKP Error: {e}")
        return 1
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    return 0


if __name__ == "__main__":
    sys.exit(main())