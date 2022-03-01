###### Parameters ######
RUN_FILE=./main
RES_DIR=results/apps/
########################

mkdir -p $RES_DIR

for (( log_s=0; log_s<=10; log_s++ )); do

    $RUN_FILE -n 1000  -s $(( 2**log_s * 40960)) -d 13 -h 2 -e 1 -w $RES_DIR
    $RUN_FILE -n 10000 -s $(( 2**log_s * 40960)) -d 13 -h 2 -e 1 -w $RES_DIR

    $RUN_FILE -n 1000  -x $(( 2**32)) -s $(( 2**log_s * 40960)) -d 13 -h 3 -e 1 -w $RES_DIR
    $RUN_FILE -n 10000 -x $(( 2**32)) -s $(( 2**log_s * 40960)) -d 13 -h 3 -e 1 -w $RES_DIR

done

