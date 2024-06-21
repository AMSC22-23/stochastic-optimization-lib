### [<< Back to parent README](../../README.md)

# ABC - Artificial Bee colony   

The Artificial Bee Colony (ABC) algorithm is a metaheuristic optimization technique inspired by the foraging behavior of honey bees. It was introduced by Karaboga and Basturk and has proven effective for solving complex optimization problems, including constrained optimization problems (COPs).

The version developed in this project does refer to the Karaboga's ABC implementation for solving constrained optimization problem, which has been proposed [here](https://www.sciencedirect.com/science/article/pii/S1568494610003066).

The ABC algorithm simulates the intelligent foraging behavior of a honey bee swarm. The algorithm consists of three types of bees: employed bees, onlookers, and scouts, which work together to find the optimal solution.

## ABC key Features

- `Exploration and Exploitation`: The ABC algorithm balances exploration (global search) and exploitation (local search) through the cooperative behavior of the bees.
- `Adaptivity`: It dynamically adapts the search process based on the feedback from the environment, improving its ability to find optimal solutions.
- `Flexibility`: ABC can be applied to various types of optimization problems without significant modifications.

## Algorithm Description

It is worth to explain that, although different kind of bees are presented, they don't have to be considered effectively as if they are objects with different behaviours. In fact, in the algorithm, the employer phase, the scout phase and so on, just represent and characterize the different phases of the optimization single step. 

The ABC algorithm involves the following steps:

- `Initialization`: Generate an initial population of food sources (solutions) randomly.
- `Employed Bee phase`: Each employed bee evaluates the fitness of its food source and explores neighboring solutions, updating its position with a probability related to the algorithm parameter MR:

$$
x_{ij} = \begin{cases} 
x_{ij} + \beta_{ij}(x_{ij} - x_{kj}), & \text{if } R_j < MR \\
x_{ij}, & \text{otherwise}.
\end{cases}$$


- `Onlooker Bee Phase`: Onlooker bees select new food sources to be followed, based on their probability $p_i$, which is proportional to the fitness of the food source.

$$
p_i = \begin{cases}
  0.5 + \frac{fitness_i}{\sum\limits_{j=1}^{n} fitness_j} \times 0.5 & \text{if feasible} \\
  \left( 1 - \frac{violation_i}{\sum\limits_{j=1}^{n} (violation_j)} \right) \times 0.5 & \text{otherwise}
\end{cases}
$$

- `Scout Bee Phase`: If a food source cannot be improved further, it is abandoned, and a scout bee randomly searches for a new food source, meaning that the bee is reinitialized.
- `Termination`: The process repeats until a stopping criterion (maximum number of iterations or acceptable solution quality) is met.

## Constrained Optimization

The ABC algorithm handles constrained optimization problems by incorporating a penalty function approach. The feasibility-based rule ensures that:
- feasible solutions are preferred over infeasible ones
- among feasible solutions, the one with better objective function value is chosen
- among infeasible solutions, the one with the smallest total constraint violation is preferred.

## Code structure

Two main classes have been developed:

-  `Bee` Represents a single bee in the swarm and manages its initialization, evaluation, and update at each iteration. It implements the feasibility rule and the search behavior of employed, onlooker, and scout bees.
-  `ABC` wraps the colony of bees and the ABC algorithm, providing methods for initialization, execution, and output. The class supports both serial and parallel implementations

The `ABC` class provides methods for the **serial** and the **parallel** implementation for both initialization and optimization methods. The parallel implementation uses OpenMP for multithreading and MPI for multiprocessing.
 
In the parallel version of the ABC algorithm, the entire colony of bees is divided equally among the available processors. This parallel implementation is designed for shared memory architectures and has been developed according to what proposed [here](https://ieeexplore.ieee.org/document/5393726), in fact some modifications with respect to the classical algorithm are needed in order to parallelize it. 

In particular, each thread handles a subset of the total bee's colony and operate completely on it, neglecting what is done by the other threads. This rearrangement of the logic of the classical version ABC algorithm, allows not to have significant overhead due to synchronization in the parallel implementation.

In fact, just a final reduction at the end of the optimize process is needed. It allows to select the best bee among all the local colonies and will determine the global result.

The MPI parallelization has been performed exploiting the same algorithmic idea. The global colony is divided among different MPI processes, each one operating independetly from the other one, and a custom MPI reduction based on the feasibility rule has been implemented in order to find, at the end of the optimize process, the global optimal solution.



## Tests and Results
Optimization of a Problem - `optimize`

This test optimizes a test function defined in the `test_problem` definte with the given dimensions. It logs the best value, total constraint violation, and the number of feasible solutions over iterations, storing the data in `output/abc_optimize_threads_num.csv` file.

In the results below the G10 test problem in a 8D space is optimized. The swarm is composed by 20 particles and 6k iterations are performed.
<p align="center">
<img src= "https://github.com/AMSC22-23/stochastic-optimization-lib/assets/131521380/50532e2d-fb53-4396-9443-cb74cb50fb03)" height="500"> 
</p>


This plot highlights the policy to select the best among two particles that ABC utilizes:
1. A feasible solution is preferred over an infeasible solution
2. Among two feasible solutions, the one with better objective function value is preferred
3. Among two infeasible solutions, the one with smaller total constraint violation is chosen

As soon as the 0 violation region is reached, the particles won't leave it anymore, corresponding to a global contraint violation that starts being very high, but once has become 0 it won't increase anymore.
In addition, it is worth observing that the first 1k iterations are sufficient to reach already a good approximation of the real optimum, while the improvements obtained by the other 5k iterations are less relevant, although they allow to practically reach the correct solution.

Execution Time and Speedup - `time_numparticles`

This test optimizes several time a given test function varying only the number of particles. The optimization is done both serially and in parallel (omp) logging the execution time in order to compute the parallel speedup. 
The test stores in the `output/abc_time_numparticles_num_threads.csv` files all the logged execution time as function of the swarm size.

<p align="center">
<img src= "https://github.com/AMSC22-23/stochastic-optimization-lib/assets/131521380/5727a685-c26f-4730-b0e0-b57cb0c51fe5)" height="500">


This result shows a parallel speedup which increases with the dimension of the problem, reaching a maximum of around  $10\times$, running on an Intel Core i7-13700H machine (20 logical threads, 8 performance + 4 power efficient cores).
The serial execution time increases linearly with the number of particles, as expected for an algorithm with a computational complexity dependent on the number of particles.
The speedup initially increases sharply as the number of particles increases, indicating a high efficiency of parallelization for smaller colony sizes.
Beyond a certain point (around 500 particles), the speedup fluctuates but generally stays high, suggesting that the parallel implementation consistently performs well, though with some variations.

The data collected for many number of threads enable the possiblity to do a strong scaling study, done both for omp and mpi parallelization. Using the huge G10 test problem and 5000 iterations to have better results, the following results have been collected.

<p align="center">
<img src= "https://github.com/AMSC22-23/stochastic-optimization-lib/assets/131521380/fa90d5fd-4c3b-4baf-94ef-7fccf34bd89b)" height="500"> 
 
This plot refers to omp parallelization. It shows that, while the ABC algorithm benefits significantly from parallel execution, achieving substantial reductions in execution time, the efficiency of this scaling is influenced by problem size and overheads. In particular, we note an almost optimal behaviour with high dimension problems and a small number of threads (up to 4). A generale small deterioration is experienced between 4 and 8 threads, and finally from 8 to 16 threads it starts again to show almost optimal strong scalability.
Smaller problems exhibit less efficient scaling, emphasizing the need to match problem size with appropriate parallelization levels for optimal performance. Overall, for computations that lasts for less than 5 seconds it is possible to observe only small improvements from multithreading. This data has been collected running the tests in the MOX cluster, on up to 16 physical cores.

The following plot, instead, refers to mpi parallelization. It is evident that it shows a more consistent approach to ideal scaling. Unlike the OMP implementation, where smaller problem sizes showed significant deviations, MPI maintains closer adherence to the ideal scaling line across various problem sizes. This can be attributed to its distributed memory model and minimized communication overheads. 

<p align="center">
<img src= "https://github.com/AMSC22-23/stochastic-optimization-lib/assets/131521380/2d9048a9-63c9-41a7-aa44-ad4d054db835)" height="500"> 



Conclusion

The ABC algorithm is a robust optimization method suitable for a wide range of optimization problems, including constrained ones. Its parallel implementation ensures scalability and efficiency, making it a valuable tool for solving complex problems.


### [<< Back to parent README](../../README.md)
