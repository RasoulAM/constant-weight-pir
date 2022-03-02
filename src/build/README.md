# Reproducing results of paper

The following scripts can be used to the generate the data required to reproduce the results shown in the paper:

Experiment | File Name | Table | Execuatables
:----: | :----: | :----: | :----:
Plain Equality Operators | `equality-plain.sh` | Table 4 and 12 | benchmark_eq
Arithmetic Equality Operators | `equality-arithmetic.sh` | Table 5 and 12 | benchmark_eq
PIR with one plaintext payload | `pir-one-plaintext.sh` | Table 7 and 13 | main
PIR with large payloads | `pir-large-payload.sh` | Table 9 | main

These scripts must be run in the this directory (the `build` directory), next to the corresponding executables. The code reqiured to interpret the data and gernerate the tables is in `interpret-results.ipynb`.