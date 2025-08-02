#!/usr/bin/env python3
"""
Python wrapper for MOKP DLL using ctypes (without numpy dependency).
This demonstrates how to use the multi-objective knapsack optimization DLL from Python.
"""

import ctypes
from ctypes import Structure, POINTER, c_int, c_double, c_char_p

# Define the structures
class MOKPSolution(Structure):
    _fields_ = [
        ("objectives", POINTER(c_double)),
        ("decision_vars", POINTER(c_int)),
        ("num_objectives", c_int),
        ("num_items", c_int)
    ]

class MOKPResults(Structure):
    _fields_ = [
        ("solutions", POINTER(MOKPSolution)),
        ("count", c_int),
        ("capacity", c_int)
    ]

class MOKPOptimizer:
    """Python wrapper for the MOKP DLL."""
    
    def __init__(self, dll_path="./libmokp.so"):
        """Initialize the MOKP optimizer with the DLL path."""
        self.dll = ctypes.CDLL(dll_path)
        self._setup_function_signatures()
        self.initialized = False
        self.problem_loaded = False
        
    def _setup_function_signatures(self):
        """Set up function signatures for type safety."""
        # InitializeOptimizer
        self.dll.InitializeOptimizer.argtypes = []
        self.dll.InitializeOptimizer.restype = c_int
        
        # LoadProblem
        self.dll.LoadProblem.argtypes = [c_char_p]
        self.dll.LoadProblem.restype = c_int
        
        # SetParameters
        self.dll.SetParameters.argtypes = [c_int, c_int, c_double]
        self.dll.SetParameters.restype = c_int
        
        # RunOptimization
        self.dll.RunOptimization.argtypes = []
        self.dll.RunOptimization.restype = c_int
        
        # GetResultCount
        self.dll.GetResultCount.argtypes = []
        self.dll.GetResultCount.restype = c_int
        
        # GetResult
        self.dll.GetResult.argtypes = [c_int, POINTER(c_double), POINTER(c_int)]
        self.dll.GetResult.restype = c_int
        
        # GetProblemInfo
        self.dll.GetProblemInfo.argtypes = [POINTER(c_int), POINTER(c_int)]
        self.dll.GetProblemInfo.restype = c_int
        
        # GetResults
        self.dll.GetResults.argtypes = [POINTER(MOKPResults)]
        self.dll.GetResults.restype = c_int
        
        # FreeResults
        self.dll.FreeResults.argtypes = [POINTER(MOKPResults)]
        self.dll.FreeResults.restype = None
        
        # Cleanup
        self.dll.Cleanup.argtypes = []
        self.dll.Cleanup.restype = None
        
        # GetErrorMessage
        self.dll.GetErrorMessage.argtypes = [c_int]
        self.dll.GetErrorMessage.restype = c_char_p
        
    def initialize(self):
        """Initialize the optimizer."""
        result = self.dll.InitializeOptimizer()
        if result != 0:
            error_msg = self.dll.GetErrorMessage(result).decode('utf-8')
            raise Exception(f"Failed to initialize optimizer: {error_msg}")
        self.initialized = True
        return True
        
    def load_problem(self, filename):
        """Load a problem instance from file."""
        if not self.initialized:
            raise Exception("Optimizer not initialized. Call initialize() first.")
            
        result = self.dll.LoadProblem(filename.encode('utf-8'))
        if result != 0:
            error_msg = self.dll.GetErrorMessage(result).decode('utf-8')
            raise Exception(f"Failed to load problem: {error_msg}")
        self.problem_loaded = True
        return True
        
    def get_problem_info(self):
        """Get problem information (number of objectives and items)."""
        if not self.problem_loaded:
            raise Exception("Problem not loaded. Call load_problem() first.")
            
        num_objectives = c_int()
        num_items = c_int()
        result = self.dll.GetProblemInfo(ctypes.byref(num_objectives), ctypes.byref(num_items))
        
        if result != 0:
            error_msg = self.dll.GetErrorMessage(result).decode('utf-8')
            raise Exception(f"Failed to get problem info: {error_msg}")
            
        return num_objectives.value, num_items.value
        
    def set_parameters(self, population_size=10, max_iterations=100, perturbation_rate=0.05):
        """Set algorithm parameters."""
        if not self.initialized:
            raise Exception("Optimizer not initialized. Call initialize() first.")
            
        result = self.dll.SetParameters(population_size, max_iterations, perturbation_rate)
        if result != 0:
            error_msg = self.dll.GetErrorMessage(result).decode('utf-8')
            raise Exception(f"Failed to set parameters: {error_msg}")
        return True
        
    def run_optimization(self):
        """Run the optimization algorithm."""
        if not self.problem_loaded:
            raise Exception("Problem not loaded. Call load_problem() first.")
            
        result = self.dll.RunOptimization()
        if result != 0:
            error_msg = self.dll.GetErrorMessage(result).decode('utf-8')
            raise Exception(f"Optimization failed: {error_msg}")
        return True
        
    def get_result_count(self):
        """Get the number of solutions in the Pareto front."""
        count = self.dll.GetResultCount()
        if count < 0:
            raise Exception("Failed to get result count")
        return count
        
    def get_solution(self, index):
        """Get a specific solution by index."""
        num_objectives, num_items = self.get_problem_info()
        
        # Allocate arrays
        objectives = (c_double * num_objectives)()
        decision_vars = (c_int * num_items)()
        
        result = self.dll.GetResult(index, objectives, decision_vars)
        if result != 0:
            error_msg = self.dll.GetErrorMessage(result).decode('utf-8')
            raise Exception(f"Failed to get solution {index}: {error_msg}")
            
        # Convert to Python lists
        obj_list = [objectives[i] for i in range(num_objectives)]
        dec_list = [decision_vars[i] for i in range(num_items)]
        
        return obj_list, dec_list
        
    def get_all_solutions(self):
        """Get all solutions in the Pareto front."""
        count = self.get_result_count()
        solutions = []
        
        for i in range(count):
            objectives, decision_vars = self.get_solution(i)
            selected_items = [j for j, val in enumerate(decision_vars) if val == 1]
            solutions.append({
                'index': i,
                'objectives': objectives,
                'decision_variables': decision_vars,
                'selected_items': selected_items
            })
            
        return solutions
        
    def cleanup(self):
        """Clean up allocated memory."""
        self.dll.Cleanup()
        self.initialized = False
        self.problem_loaded = False

def main():
    """Example usage of the MOKP optimizer."""
    print("Python MOKP Optimizer Example")
    print("=============================\n")
    
    # Create optimizer instance
    optimizer = MOKPOptimizer()
    
    try:
        # Initialize
        print("1. Initializing optimizer...")
        optimizer.initialize()
        print("   ✓ Success!\n")
        
        # Load problem
        print("2. Loading problem...")
        optimizer.load_problem("250.2.txt")
        print("   ✓ Success!\n")
        
        # Get problem info
        print("3. Getting problem information...")
        num_objectives, num_items = optimizer.get_problem_info()
        print(f"   ✓ Problem has {num_objectives} objectives and {num_items} items\n")
        
        # Set parameters
        print("4. Setting parameters...")
        optimizer.set_parameters(population_size=15, max_iterations=10, perturbation_rate=0.1)
        print("   ✓ Parameters set: population=15, iterations=10, perturbation=0.1\n")
        
        # Run optimization
        print("5. Running optimization...")
        optimizer.run_optimization()
        print("   ✓ Optimization completed!\n")
        
        # Get results
        print("6. Getting results...")
        solutions = optimizer.get_all_solutions()
        print(f"   ✓ Found {len(solutions)} solutions in Pareto front\n")
        
        # Display solutions
        print("7. Solutions found:")
        for i, sol in enumerate(solutions):
            print(f"   Solution {i+1}:")
            print(f"     Objectives: {sol['objectives']}")
            print(f"     Selected items: {sol['selected_items'][:10]}{'...' if len(sol['selected_items']) > 10 else ''}")
            print(f"     Total selected: {len(sol['selected_items'])}")
            print()
            
        # Simple analysis
        if len(solutions) > 0:
            print("8. Analysis:")
            obj1_values = [sol['objectives'][0] for sol in solutions]
            obj2_values = [sol['objectives'][1] for sol in solutions]
            
            print(f"   Objective 1 range: [{min(obj1_values):.2f}, {max(obj1_values):.2f}]")
            print(f"   Objective 2 range: [{min(obj2_values):.2f}, {max(obj2_values):.2f}]")
            
            # Find extreme solutions
            max_obj1_idx = obj1_values.index(max(obj1_values))
            max_obj2_idx = obj2_values.index(max(obj2_values))
            
            print(f"   Best objective 1: Solution {max_obj1_idx+1} with value {obj1_values[max_obj1_idx]:.2f}")
            print(f"   Best objective 2: Solution {max_obj2_idx+1} with value {obj2_values[max_obj2_idx]:.2f}")
        
    except Exception as e:
        print(f"Error: {e}")
        
    finally:
        # Always cleanup
        print("\n9. Cleaning up...")
        optimizer.cleanup()
        print("   ✓ Cleanup completed!")

if __name__ == "__main__":
    main()