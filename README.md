# MAX-CUT-Solvers
Three Algorithms to solve MaxCut: an optimal solver via Integer Linear Programming with Gurobi, an approximative greedy solver and an approximative random greedy solver.

## Preface
Solvers for the unweighted maximum directed cut problem as used in the paper *Greedy Maximization of Functions with Bounded Curvature under Partition Matroid Constraints* (AAAI 2019). Given a directed input graph G=(V,E), the problem consist of finding a subset U of nodes such that the number of edges from U to V-U is maximal. This problem is known to be NP-complete.
The solvers presented here are also able to solve the constrained version of the problem, where the cut set U is additionally constrained to be of a maximum size d.

This repository contains the following solving algorithms:
* **optimal solver:** Calculates the optimal solution of constrained unweighted maximum directed cut by modeling the problem as an Integer Linear Program (ILP) and solving the ILP with the LP solver Gurobi. The runtime of this algorithm is possibly exponential.
* **greedy solver:** The classical greedy algorithm, which always adds the node to U which has most edges to V-U at that point. This algorithm has an approximation factor of 1/2\*(1-e^(-2)) for undirected and of 1/α\*(1-e^(-α)) for directed graphs, where α is the curvature of the graph. The curvature can be upper-bounded by 1+min(d,Δ-)/Δ+, where Δ- is the maximum in-degree and Δ+ is the maximum out-degree of G. The runtime of this algorithm is O(|V|+|E|\*log |V|).
* **random greedy solver:** A random greedy algorithm which chooses one of the b nodes with most edges to V-U uniformly at random and adds it to U. Its runtime is O(d\*|V|+|E|\*log |V|).


## Installation
The solvers require the Stanford Network Analysis Platform (SNAP) library for C++ ("SNAP: A General-Purpose Network Analysis and Graph-Mining Library" by Jure Leskovec and Rok Sosič, https://snap.stanford.edu/). The optimal solver additionally requires Gurobi (http://www.gurobi.com).
After setting the SNAPPATH variable to the main directory of the snap library, a simple call to make should do the trick.

``> make``

## Usage
The algorithms take a graph in csv format as input and return a txt-file with the solution size. With the -:i option you specifiy an input file. With the -o: option you specify a directory for the output file. The output file name will always be the <input file name>-<ILP|LP|Greedy|Rand>-<b|P>-<b-value|p-value>-[<seed>].txt. All algorithms take either a fixed constraint with the -b: option or a constraint as a percentage of the total number of nodes with the -p: option. The default is the unconstrained version of the problem. Since the optimal solver sometimes runs infeasibly long, there is the -a:0 option to instead solve the relaxed version of the problem. By default, the ILP will be solved. For the randomized greedy algorithm there is the -s: option to specify a seed value for reconstruction and parallelization purposes. If no seed is specified or if -s:-1 is given, system time will be used to seed the PRNG.

### Options
```
-i:<path to input file>
        Input directed graph in csv-format, i.e. one directed edge per line.
-o:<directory of output file>
        Directory in which to save the output file. The name of the file is determined by the input parameters.
-b:<integer>
        Integer determining the maximum size of the cut set.
-p:<integer>
        Integer determining the maximum size of the cut set as a percentage of the total number of nodes in the graph.
-s:<integer>
        Seed value (int), system time is used if -1 is given. (only for random greedy)
-a:<0|1>
        Choosing between the relaxed and the integer linear program, 0=LP, 1=ILP. (only for optimal solver)
```

### Example
``> MaxCutRandomGreedy -i:inputgraph.csv -o:outputdir -p:50 -s:42``

Solves the input graph in 'inputgraph.csv' with the random greedy algorithm and outputs the solution into a file called 'inputgraph-P-50-Rand-42.txt' in the directory 'outputdir'. The seed for the pseudo-random number generator will be 42.
