###### Parameters ######
RUN_FILE=./main
RES_DIR=results/pir-one-plaintext/
RUNS=10
########################

mkdir -p $RES_DIR

for ((repeat=1; repeat<=RUNS; repeat++)); do

    # Folklore with N=8192, single-thread
    log_d=13
    for ((log_n=4; log_n<=9; log_n++)); do
        $RUN_FILE simple -n $((2 ** log_n)) -d $log_d -e 0 -t 1 -w $RES_DIR
    done

    # Folklore with N=16384, single-thread
    log_d=14
    for ((log_n=8; log_n<=14; log_n++)); do
        $RUN_FILE simple -n $((2 ** log_n)) -d $log_d -e 0 -t 1 -w $RES_DIR
    done

    # Constant-weight with N=4096 and k=1, single-thread
    log_d=12
    hamming_weight=1
    for ((log_n=8; log_n<=17; log_n++)); do
        $RUN_FILE simple -n $((2 ** log_n)) -d $log_d -h $hamming_weight -e 1 -t 1 -w $RES_DIR
    done

    # Constant-weight with N=8192 and k=2, single-thread
    log_d=13
    hamming_weight=2
    for ((log_n=8; log_n<=16; log_n++)); do
        $RUN_FILE simple -n $((2 ** log_n)) -d $log_d -h $hamming_weight -e 1 -t 1 -w $RES_DIR
    done

    # Constant-weight with N=8192 and k=2, parallelized
    log_d=13
    hamming_weight=2
    for ((log_n=8; log_n<=18; log_n++)); do
        $RUN_FILE simple -n $((2 ** log_n)) -d $log_d -h $hamming_weight -e 1 -t 0 -w $RES_DIR
    done

done