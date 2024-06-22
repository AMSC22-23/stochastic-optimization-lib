# Stochastic Optimization Library
Small collection of stochastic algorithms for constrained and unconstrained function optimization. All the implemented algorithms provide a common interface in order to abstract the implementation details and facilitate the usage for the final user.


## Algorithms
Each of the implemented algorithms has its own ```README.md``` file in order to better explain its implementation, usage, implemented tests and their results. A brief description and the problem type that each algorithm can address is provided below.
- [**SASPSO - Self Adaptive Standard Particle Swarm Optimization**](include/SASPSO/README.md)
  - The SASPSO algorithm is a modified version of the Standard Particle Swarm Optimization 2011 (SPSO 2011) algorithm with **adaptive search parameters**.
  - Constrained optimization available with a constraint handling technique based on the **adaptive relaxation method** integrated with the **feasibility-based rule**.
  - Thread-level OpenMP parallelization available.
- [**ABC - Artificial Bee Colony**](include/ABC/README.md)
  - The ABC algorithm is inspired by the foraging behavior of honey bees. It simulates the intelligent food searching behavior of a honey bee swarm.
  - Constrained optimization available with a constraint handling technique based on the **feasibility-based rule**.
  - Thread-level OpenMP plus Process-level MPI parallelization available.

## Common interface
All the provided algorithms shares the same standard public interface. The user can use the same sequence of method calls to solve a given problem, indipendently from the choosen algorithm.

Each algorithm gives also the possibility to specify some additional parameters that are dependent to the optimization technique.
Custom parameters are preferred when solving specific and/or hard optimization problems in order to gain in performances.

The `Problem` class describes an optimization problem to be optimized. The user needs to instantiate a ```Problem``` object and set the space dimension in which it belongs, its fitness function, constraits, and bounds. It provides the following interface to build a problem:
- `Constructor` to provide: fitness function as `std::function`, lower bounds and upper bounds both as `Eigen::Matrix<double, dim, 1>` where `dim` is the seach space dimension.
- `add_equality_constraint` to add an equality constraint $f$ as `std::function` (or any cast compatible type) s.t. $f(\vec{x})=0$
- `add_inequality_constraint` to add an inequality constraint $f$ as `std::function` (or any cast compatible type) s.t. $f(\vec{x})\le0$

The `Optimizer` abstract class describes the interface for a specific optimizer implementation. The user needs to instantiate an object of the preferred algorithm passing the Problem as parameter.

The following code implements a possible basic usage of an optimizer. Firstly the problem to be optimized must be defined providing the fitness function `f`, the bounds and any constraint
```
auto f = [](const RealVector<2> &x)
{
  return x[0] * x[0] + x[1] * x[1];
};
RealVector<2> lb(-4, -4);
RealVector<2> ub(4, 4);
Problem<2> problem(f, lb, ub);
problem.add_inequality_constraint([](const RealVector<2> &x) {
  return - std::sin(4 * M_PI * x[0]);
});
```
Then the choosen solver must be created providing the problem, and then it can be initialized and solved.
```
std::unique_ptr<Optimizer<2>> opt = OptimizerFactory<2>::create(OptimizerFactory:SelfAdaptiveSPSO, problem, 100, 1000);
opt->initialize();
opt->optimize();
opt->print_results();
```
Here a SASPSO solver has been initialized for solving a 2 dimensional problem `problem` with 100 particles and 1000 iterations.

## Requirements
- CMake
- C++17
- OpenMP 3.1
- MPI 3.0
- Python3
- Eigen 3.3

NB: Eigen 3.3 can be loaded using [mk modules](https://github.com/pcafrica/mk) or downloading it from the official website.

## Compile and Run
1. Create the folders needed for building the project and saving the output `.csv` and `.png` files
   ```
   mkdir build
   mkdir output
   ```
2. Move into the build folder
   ```
   cd ./build
   ```
3. Execute cmake (it may require the flag with the explicit path to the Eigen library if it isn't in the default mk modules folder)
   ```
   cmake ..
   ```
4. Build the project
   ```
   cmake --build .
   ```
5. Launch the test executable. Suitable test name and algorithm name must be provided (a complete list of the available test is shown if no parameter is provided)
   ```
   ./test [test_name] [algorithm_name]
   ```
   The execution of a test may produce an output file `test-name_algorithm-name_num-threads.csv` that can be found under the `output` folder.
6. Plot the results in a graphical way using the scripts in the `/script` folder. `csv_plotter.csv` takes as argument only the filename of one .csv file stored in the the `output` folder and plots only data related to a single run of a single solver. While `csv_scaling.py` is used to plot data from many files and takes as argument a type of plot among `speedup` for the parallel speedup and `strongsingle` for the strong scalability of each single solver. Lastly the `csv_simulation.py` plots the evolution of the swarm on a Gomez-Levy 2D problem.
   It does requires the filename in which the simulation data for the GL problem has been stored, i.e. `simulation_solver-name_xx.csv` (it may takes a while depending on the number of iterations). 
   ```
   cd ../scripts
   python csv_plotter.py test-name_algorithm-name_threads-num.csv
   python csv_scaling.py [strongsingle | speedup]
   python csv_animation.py simulation_algorithm-name_1.csv
   ```
   All the plots are stored in the `/plots` folder.
   NB: we choose to have different scripts since each of them manages data in a different way: the first takes a single file, the second aggregates more files data, and the last manages files to produce an animation.

The complete documentation of the public interface of our project can be consulted [here](https://amsc22-23.github.io/stochastic-optimization-lib/)


## Authors
- Pierpaolo Marzo
- Domenico Santarsiero
- Alberto Guerrini
