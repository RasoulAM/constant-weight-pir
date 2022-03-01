###### Parameters ######
RUN_FILE=./benchmark_eq
RES_FILE=results/equality-arithmetic/
RUNS=10
########################

mkdir -p $RES_FILE

for ((repeat=1; repeat<=RUNS; repeat++)); do

    ####### Folklore Arithmetic #######
    echo "folklore arith"

    log_d=13
    for (( log_l=3; log_l<=4; log_l++ )); do
        $RUN_FILE fl-arith   -l $((2 ** log_l)) -d $log_d -v -w $RES_FILE
        $RUN_FILE fl-arith   -l $((2 ** log_l)) -d $log_d -p -v -w $RES_FILE
    done

    log_d=14
    for (( log_l=3; log_l<=9; log_l++ )); do
        $RUN_FILE fl-arith   -l $((2 ** log_l)) -d $log_d -v -w $RES_FILE
        $RUN_FILE fl-arith   -l $((2 ** log_l)) -d $log_d -p -v -w $RES_FILE
    done

    ####### Constant-weight Arithmetic #######

    echo "cw arith k=ell"

    # k = ell
    DIFF=0
    log_d=13
    for (( log_l=3; log_l<=3; log_l++ )); do
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    log_d=14
    for (( log_l=3; log_l<=9; log_l++ )); do
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    echo "cw arith k=ell/2"
    # k = ell/2
    DIFF=1

    log_d=13
    for (( log_l=3; log_l<=3; log_l++ )); do
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    log_d=14
    for (( log_l=3; log_l<=9; log_l++ )); do
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    echo "cw arith k=ell/4"
    # k = ell/4
    DIFF=2

    log_d=13
    for (( log_l=3; log_l<=4; log_l++ )); do
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    log_d=14
    for (( log_l=3; log_l<=9; log_l++ )); do
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    echo "cw arith k=ell/8"
    # k = ell/8
    DIFF=3

    log_d=12
    for (( log_l=3; log_l<=3; log_l++ )); do
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    log_d=13
    for (( log_l=3; log_l<=5; log_l++ )); do
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    log_d=14
    for (( log_l=3; log_l<=9; log_l++ )); do
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-arith -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

done