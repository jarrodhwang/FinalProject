# CMPT 225 Final Project

## Database Indexing with B-Tree and B+ Tree Data Structures

This project is a C++17 Ubuntu terminal benchmark for database-style indexing workloads. It implements and compares two classic multiway search trees:

1. B-Tree
2. B+ Tree

The goal is to measure why B+ Trees are commonly preferred in database indexing systems, especially for range queries and page-oriented storage layouts.

## Project Goal

Implement and benchmark B-Tree and B+ Tree structures to evaluate their performance for:

- bulk insertion
- successful search
- unsuccessful search
- range query workloads

The code is written so the design differences are explicit and easy to explain in an academic report.

## Main Design Choices

### B-Tree

- Configurable order
- Internal nodes store both keys and values
- Supports:
  - `insert(key, value)`
  - `search(key)`
  - `rangeQuery(low, high)`

This implementation intentionally focuses on the workloads requested for the benchmark. Optional removal is not included so the code can stay centered on build/search/range-query behavior.

### B+ Tree

- Configurable order
- Internal nodes store separator keys only
- All values are stored in leaf nodes
- Leaf nodes are linked together using a `next` pointer
- Supports:
  - `insert(key, value)`
  - `search(key)`
  - `rangeQuery(low, high)`

The linked leaves are the key database-style feature because they make sequential range scans efficient.

## Why This Comparison Matters

B-Tree and B+ Tree structures are both balanced multiway search trees, but they behave differently in practice:

- B-Tree can finish a successful search early if the key is found in an internal node.
- B+ Tree pushes all records into leaves, which keeps internal nodes smaller and better suited for navigation.
- B+ Tree range queries are usually faster because leaf nodes are linked in order.

That final point is the main reason B+ Trees are so common in database indexing systems.

## Metrics Measured

For each structure, the benchmark reports:

- build time in milliseconds
- successful search time in milliseconds
- unsuccessful search time in milliseconds
- range query time in milliseconds
- tree height
- number of node splits
- disk-like accesses measured as node visits

Node visits are used as a simple page-access model. Every time the algorithm touches a tree node during traversal or split handling, the benchmark counts that as a disk-like access.

## Dataset Generation

The workload generator creates deterministic integer-key datasets for fair comparison between the two trees.

For each dataset size, it produces:

- `insertKeys`
- `successfulSearchQueries`
- `unsuccessfulSearchQueries`
- `rangeQueries`

Properties:

- inserted keys are unique
- successful search queries are a shuffled copy of inserted keys
- unsuccessful search queries use disjoint keys
- range queries are built from the sorted key set so both trees are tested on the same intervals

## Benchmark Configuration

Default experiments:

- dataset sizes: `1000`, `5000`, `10000`, `50000`
- repeats: `5`
- order: `8`

For each repeat:

1. Generate one dataset.
2. Build the B-Tree with that dataset.
3. Run successful searches.
4. Run unsuccessful searches.
5. Run range queries.
6. Repeat the same workload on the B+ Tree.
7. Average the results.

## Project Structure

```text
FinalProject/
|-- include/
|   |-- BPlusTree.h
|   |-- BTree.h
|   |-- BenchmarkRunner.h
|   |-- BenchmarkTypes.h
|   |-- DatasetGenerator.h
|   |-- IndexStructure.h
|   `-- ResultPrinter.h
|-- src/
|   |-- BPlusTree.cpp
|   |-- BTree.cpp
|   |-- BenchmarkRunner.cpp
|   |-- DatasetGenerator.cpp
|   |-- ResultPrinter.cpp
|   `-- main.cpp
|-- matlab/
|   |-- generate_figures.m
|   `-- generate_tables.m
|-- results/
|   |-- figures/
|   `-- benchmark_results.csv
|-- Makefile
`-- README.md
```

## Source File Roles

- `BTree.cpp` / `BTree.h`
  - classic B-Tree insertion, search, and recursive in-order range scan
- `BPlusTree.cpp` / `BPlusTree.h`
  - B+ Tree insertion, search, and linked-leaf range scan
- `DatasetGenerator.cpp`
  - builds fair, repeatable workloads
- `BenchmarkRunner.cpp`
  - executes experiments and validates correctness
- `ResultPrinter.cpp`
  - prints terminal tables, ASCII charts, and CSV output
- `main.cpp`
  - parses command-line options and runs the project
- `matlab/generate_figures.m`
  - creates report figures from the CSV
- `matlab/generate_tables.m`
  - creates summary CSV tables from the CSV

## Build Instructions

### Option 1: Use `make`

```bash
make
```

### Option 2: Direct `g++` command

Use this when `make` is not installed:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -Iinclude src/*.cpp -o final_project_indexing
```

## Run Instructions

### Default benchmark

```bash
./final_project_indexing
```

### Custom dataset sizes

```bash
./final_project_indexing --sizes 1000,5000,10000 --order 4 --repeats 3 --csv results/order4_results.csv
```

### Help

```bash
./final_project_indexing --help
```

## Command-Line Options

- `--sizes`
  - comma-separated dataset sizes
- `--order`
  - tree order used by both B-Tree and B+ Tree
- `--repeats`
  - number of repeated trials
- `--csv`
  - output CSV path
- `--seed`
  - base random seed

## Terminal Output

The program prints:

- formatted benchmark tables
- simple ASCII bar charts
- a CSV export path summary

Each table row shows:

- timing metrics
- height
- splits
- average node visits per operation type
- average number of rows returned by each range query

## CSV Output

Default output:

```text
results/benchmark_results.csv
```

Columns include:

- structure name
- dataset size
- order
- repeats
- range query count
- all timing metrics
- height
- splits
- node visits
- average range result count

This CSV is intended for direct use in the final report.

## MATLAB Scripts

The `matlab/` folder contains scripts for turning the CSV into report material.

### Create figures

In MATLAB:

```matlab
generate_figures
```

This creates PNG figures inside:

```text
results/figures/
```

### Create summary tables

In MATLAB:

```matlab
generate_tables
```

This writes summary CSV tables into:

```text
results/
```

## How To Explain The Code In Your Report

A strong report explanation is:

1. Introduce database indexing and why balanced multiway trees are used for block-based storage.
2. Explain B-Tree structure:
   - keys and values can appear in internal nodes
   - searches may stop before reaching a leaf
3. Explain B+ Tree structure:
   - internal nodes guide navigation only
   - all values live in leaves
   - linked leaves support efficient range scans
4. Describe the benchmark methodology:
   - same dataset for both trees
   - repeated experiments
   - timing plus structural metrics
5. Discuss the results:
   - B+ Tree should usually be strong for range queries
   - B-Tree may be competitive for point lookups

## Expected Findings

The benchmark is designed to make these ideas visible:

- Both trees stay balanced and scale well.
- Both trees require node splits as the dataset grows.
- B+ Tree range queries should usually require fewer structural jumps because leaves are linked in sorted order.
- B+ Tree is often the more database-friendly design because internal pages stay focused on navigation and leaf pages support sequential access.

## Notes On Height

The reported height is the number of node levels from root to leaf:

- empty tree: `0`
- tree with only a root leaf: `1`

## Verification

The benchmark runner validates correctness during execution:

- successful searches must return the expected value
- unsuccessful searches must fail
- range query results must exactly match the expected sorted key interval

This means the timing results are collected only after the implementation passes basic functional checks.

## Summary

This project provides a clean, modular comparison of B-Tree and B+ Tree indexing for database-style workloads. It is structured for both execution and explanation: the implementation is split across readable C++17 source files, the benchmark output is terminal-friendly, results are exported to CSV, and MATLAB scripts are included for report figures and tables.
