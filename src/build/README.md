# Reproducing results of paper

The following scripts can be used to the generate the data required to reproduce the results shown in the paper:

Experiment | File Name | Table | Execuatables
:----: | :----: | :----: | :----:
Plain Equality Operators | `equality-plain.sh` | Table 4 and 12 | benchmark_eq
Arithmetic Equality Operators | `equality-arithmetic.sh` | Table 5 and 12 | benchmark_eq
PIR with one plaintext payload | `pir-one-plaintext.sh` | Table 7 and 13 | main
PIR with large payloads | `pir-large-payload.sh` | Table 9 | main

These scripts must be run in the this directory (the `build` directory), next to the corresponding executables. The code required to interpret the data and generate the tables is in `interpret-results.ipynb`. By running the cells of the specified
notebook, the results for the PIR experiments are shown in separate
tables.

In our paper, we run all experiments on an Intel Xeon E5-4640 @
2.40GHz server running Ubuntu 20.04 and we parallelize over 32 physical cores when specified.

Using the machine specified above, and assuming all the scripts are run in 
sequence, it can take up to two week to run, depending on the number of
repititions of the experiments. The PIR experiments are currently set to
run 10 times. This can be modified in the corresponding scripts to reduce
the waiting time.

