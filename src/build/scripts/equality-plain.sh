###### Parameters ######
RUN_FILE=./benchmark_eq
RES_FILE=results/equality-plain/
RUNS=1
########################

mkdir -p $RES_FILE

for ((repeat=1; repeat<=RUNS; repeat++)); do

    echo "folklore plain"
    ####### Folklore Plain #######
    log_d=13
    for (( log_l=3; log_l<=4; log_l++ )); do
        $RUN_FILE fl-plain -l $((2 ** log_l)) -d $log_d -v -w $RES_FILE
        $RUN_FILE fl-plain -l $((2 ** log_l)) -d $log_d -p -v -w $RES_FILE
    done

    log_d=14
    for (( log_l=3; log_l<=9; log_l++ )); do
        $RUN_FILE fl-plain -l $((2 ** log_l)) -d $log_d -v -w $RES_FILE
        $RUN_FILE fl-plain -l $((2 ** log_l)) -d $log_d -p -v -w $RES_FILE
    done

    ####### Constant-weight Plain #######

    echo "cw plain k=ell"

    # k = ell
    DIFF=0

    log_d=13
    for (( log_l=3; log_l<=4; log_l++ )); do
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    log_d=14
    for (( log_l=3; log_l<=9; log_l++ )); do
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    echo "cw plain k=ell/2"

    # k = ell/2
    DIFF=1

    log_d=13
    for (( log_l=3; log_l<=5; log_l++ )); do
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    log_d=14
    for (( log_l=3; log_l<=9; log_l++ )); do
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    echo "cw plain k=ell/4"
    # k = ell/4
    DIFF=2

    log_d=12
    for (( log_l=3; log_l<=3; log_l++ )); do
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    log_d=13
    for (( log_l=3; log_l<=6; log_l++ )); do
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    log_d=14
    for (( log_l=3; log_l<=9; log_l++ )); do
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    echo "cw plain k=ell/8"
    # k = ell/8
    DIFF=3

    log_d=12
    for (( log_l=3; log_l<=4; log_l++ )); do
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    log_d=13
    for (( log_l=3; log_l<=7; log_l++ )); do
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

    log_d=14
    for (( log_l=3; log_l<=9; log_l++ )); do
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -v -w $RES_FILE
        $RUN_FILE cw-plain -l $((2 ** log_l)) -d $log_d -k $((2 ** (log_l-DIFF))) -p -v -w $RES_FILE
    done

done