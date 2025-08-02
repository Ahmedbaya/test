#!/usr/bin/env python3
"""
Example usage of the Multi-Objective Knapsack Python Interface

This script demonstrates how to use the knapsack_python module to solve
multi-objective knapsack problems.
"""

import os
import sys
import traceback
from knapsack_python import (
    KnapsackSolver, KnapsackError, load_and_solve, solve_problem_arrays
)

def create_example_problem():
    """Create a simple example problem for demonstration"""
    print("Creating example 2-objective knapsack problem...")
    
    # Problem parameters
    num_items = 5
    num_objectives = 2
    capacities = [10.0, 15.0]  # Two capacity constraints
    
    # Weights for each objective and item
    weights = [
        [2, 3, 4, 5, 1],  # Weights for objective 1
        [1, 2, 3, 4, 2]   # Weights for objective 2
    ]
    
    # Profits for each objective and item
    profits = [
        [3, 4, 5, 6, 2],  # Profits for objective 1
        [5, 6, 7, 8, 4]   # Profits for objective 2
    ]
    
    print(f"  Items: {num_items}")
    print(f"  Objectives: {num_objectives}")
    print(f"  Capacities: {capacities}")
    print(f"  Weights: {weights}")
    print(f"  Profits: {profits}")
    
    return num_items, num_objectives, capacities, weights, profits

def solve_example_problem():
    """Solve the example problem using arrays"""
    print("\n" + "="*60)
    print("SOLVING EXAMPLE PROBLEM FROM ARRAYS")
    print("="*60)
    
    try:
        # Create problem data
        num_items, num_objectives, capacities, weights, profits = create_example_problem()
        
        # Solve using convenience function
        result = solve_problem_arrays(
            num_items, num_objectives, capacities, weights, profits,
            max_iterations=50,
            population_size=10,
            perturbation_rate=0.1
        )
        
        print(f"\nSolution found!")
        print(f"Number of Pareto-optimal solutions: {len(result)}")
        
        # Display all solutions
        for i, solution in enumerate(result):
            print(f"\nSolution {i+1}:")
            print(f"  Selected items: {solution.selected_items}")
            print(f"  Total items selected: {solution.num_selected}")
            print(f"  Objective values: {solution.objectives}")
            print(f"  Capacity used: {solution.capacities_used}")
            
        return True
        
    except KnapsackError as e:
        print(f"Knapsack error: {e}")
        return False
    except Exception as e:
        print(f"Unexpected error: {e}")
        traceback.print_exc()
        return False

def solve_file_problem():
    """Solve problem from file if available"""
    print("\n" + "="*60)
    print("SOLVING PROBLEM FROM FILE")
    print("="*60)
    
    # Check for available problem files
    problem_files = ["250.2.txt", "500.2.txt", "750.2.txt", "test_problem.txt"]
    available_files = [f for f in problem_files if os.path.exists(f)]
    
    if not available_files:
        print("No problem files found. Available files checked:")
        for f in problem_files:
            print(f"  {f} - {'EXISTS' if os.path.exists(f) else 'NOT FOUND'}")
        return False
    
    filename = available_files[0]
    print(f"Using problem file: {filename}")
    
    try:
        # Solve using convenience function
        result = load_and_solve(
            filename,
            max_iterations=100,
            population_size=20,
            perturbation_rate=0.05
        )
        
        print(f"\nFile problem solved!")
        print(f"Number of Pareto-optimal solutions: {len(result)}")
        
        # Display first few solutions
        max_display = min(5, len(result))
        for i in range(max_display):
            solution = result[i]
            print(f"\nSolution {i+1}:")
            print(f"  Selected items: {len(solution.selected_items)} items")
            print(f"  Objective values: {solution.objectives}")
            print(f"  Capacity used: {solution.capacities_used}")
        
        if len(result) > max_display:
            print(f"\n... and {len(result) - max_display} more solutions")
            
        return True
        
    except KnapsackError as e:
        print(f"Knapsack error: {e}")
        return False
    except Exception as e:
        print(f"Unexpected error: {e}")
        traceback.print_exc()
        return False

def demonstrate_advanced_usage():
    """Demonstrate advanced usage of the solver"""
    print("\n" + "="*60)
    print("ADVANCED USAGE DEMONSTRATION")
    print("="*60)
    
    try:
        # Initialize solver
        solver = KnapsackSolver()
        print(f"Library version: {solver.get_version()}")
        
        # Create a larger problem
        print("\nCreating larger problem...")
        num_items = 10
        num_objectives = 3
        capacities = [20.0, 25.0, 30.0]
        
        # Generate random-ish weights and profits
        weights = []
        profits = []
        for obj in range(num_objectives):
            obj_weights = [(i + obj + 1) % 6 + 1 for i in range(num_items)]
            obj_profits = [(i * obj + 2) % 8 + 3 for i in range(num_items)]
            weights.append(obj_weights)
            profits.append(obj_profits)
        
        print(f"  Items: {num_items}")
        print(f"  Objectives: {num_objectives}")
        print(f"  Capacities: {capacities}")
        
        # Create problem
        problem = solver.create_problem(num_items, num_objectives, capacities, weights, profits)
        
        # Custom parameters
        params = solver.default_params()
        params.max_iterations = 75
        params.population_size = 15
        params.perturbation_rate = 0.08
        params.kappa = 0.03
        
        print(f"\nSolving with custom parameters:")
        print(f"  Max iterations: {params.max_iterations}")
        print(f"  Population size: {params.population_size}")
        print(f"  Perturbation rate: {params.perturbation_rate}")
        print(f"  Kappa: {params.kappa}")
        
        # Solve
        result = solver.solve(problem, params)
        
        print(f"\nAdvanced problem solved!")
        print(f"Number of solutions: {len(result)}")
        
        # Analyze Pareto front
        print(f"\nPareto front analysis:")
        objectives_matrix = [sol.objectives for sol in result]
        
        if objectives_matrix:
            for obj_idx in range(num_objectives):
                obj_values = [obj[obj_idx] for obj in objectives_matrix]
                print(f"  Objective {obj_idx + 1}: min={min(obj_values):.1f}, max={max(obj_values):.1f}")
        
        return True
        
    except KnapsackError as e:
        print(f"Knapsack error: {e}")
        return False
    except Exception as e:
        print(f"Unexpected error: {e}")
        traceback.print_exc()
        return False

def check_dll_availability():
    """Check if the DLL is available"""
    print("Checking DLL availability...")
    
    try:
        solver = KnapsackSolver()
        version = solver.get_version()
        print(f"✓ DLL loaded successfully")
        print(f"✓ Library version: {version}")
        return True
    except KnapsackError as e:
        print(f"✗ DLL loading failed: {e}")
        print("\nTroubleshooting:")
        print("1. Make sure you've built the DLL using 'make'")
        print("2. Check that the DLL file exists in the current directory")
        print("3. On Linux/macOS, ensure the library is in LD_LIBRARY_PATH")
        print("4. On Windows, ensure the DLL is in PATH or current directory")
        return False
    except Exception as e:
        print(f"✗ Unexpected error: {e}")
        return False

def main():
    """Main example function"""
    print("Multi-Objective Knapsack Python Interface Example")
    print("=" * 60)
    
    # Check if DLL is available
    if not check_dll_availability():
        print("\nCannot proceed without DLL. Please build it first using 'make'.")
        return 1
    
    success_count = 0
    total_tests = 3
    
    # Run different examples
    if solve_example_problem():
        success_count += 1
    
    if solve_file_problem():
        success_count += 1
    
    if demonstrate_advanced_usage():
        success_count += 1
    
    # Summary
    print("\n" + "="*60)
    print("SUMMARY")
    print("="*60)
    print(f"Tests passed: {success_count}/{total_tests}")
    
    if success_count == total_tests:
        print("✓ All examples completed successfully!")
        print("\nThe Multi-Objective Knapsack DLL is working correctly.")
        print("You can now use the knapsack_python module in your own projects.")
    else:
        print("⚠ Some examples failed. Check the error messages above.")
        print("The DLL might need debugging or the environment setup may be incomplete.")
    
    return 0 if success_count == total_tests else 1

if __name__ == "__main__":
    sys.exit(main())