# Parallel and Distributed Computing
### Student: Mihailescu Eduard-Florin 332CB

## Overview
This code solves the first challenge of the Parallel and Distributed Computing course. The challenge is implementing a version of the MapReduce algorithm to crack the problem of counting the number of perfect powers in a given list of files.

The code is written in C++ and uses `pthreads` to parallelize the algorithm.

### Structure
- `main.cpp` - the project's main file, contains the main function and the code for the concrete implementation of the mapper and reducer functions, alongside the code for reading the files and auxiliary functions to help with the efficiency of the algorithm.
- `mapreduce.h` - the header file for the generic MapReduce library, contains the definition of the generic map and reduce functions
- `inparser.cpp` - the file containing the code for reading and parsing the input files by overloading the `>>` operator
- `constants.h` - the file containing the constants used in the project

---
### Compilation
The project can be compiled using the `make` command. The `make` command will compile the project and create an executable file named `tema1`. 

---
### How to run
The code is meant to be run with the following command:
```
./tema1 <num_threads> <num_files> <input_file>
```
where:
- `num_threads` is the number of threads that will be used for the computation
- `num_files` is the number of files that will be read from the input directory
- `input_file` is the path to the file containing how many files will be read and the paths to those files

---
## High-level description
At first, the program creates a queue of mapping tasks that the mapper threads will execute. After the queue is built, the program also creates nrReducers + nrMappers number of threads and gives each of them the information they need to execute the tasks using a custom-made structure.
Each thread is assigned an id to determine if the thread is a mapper or a reducer. If the thread is a mapper, it will execute the mapping tasks from the queue until it is empty. If the thread is a reducer, it will wait at the barrier until all the mappers have finished their jobs, and then it will execute the reduce task associated with its id. (i.e., reducing the vector of sets of perfect powers to a single set and printing its size to a file).

---
## Performance
To achieve better performance, the program uses a queue of mapping tasks. This way, the mapper threads can execute the tasks as soon as they are available instead of having a possible inadequate static allocation of files. This way, the program can perform better using the available resources more efficiently. 

The program also uses a precalculated set of perfect powers (this could be done as there aren't a whole lot of perfect numbers up to UINT_MAX) to speed up the checking of the perfect powers in the files (This drastically improves the performance of the program because, after the creation of the table which on Docker takes approx 0.06 sec, the check is done in amortized constant time). Also, this resource is shared between all the threads, so it is not recomputed for each thread and doesn't need synchronization mechanisms (read-only).

Another optimization was to make a custom parser for the files. Having observed that the input is made only of integers, reading the files in chunks of 4096 bytes and parsing each character with a simple formula (i.e., `number = number * 10 + (c - '0')`) instead of using the default `>>` operator, which is much slower, we can achieve much better performance.

---
