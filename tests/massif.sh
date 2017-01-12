#!/bin/bash
N=$1
if [ -z ${N} ]; then echo "Usage $0 <id>"; exit 1; fi

rm log_massif_${N} || true
rm save_massif_${N} || true

valgrind  --tool=massif ../heresyrl --name="rnd_ai" --homeworld=hive --background=outcast --role=seeker --rnd_auto_play --pb_delay=0 --save_file=./save_massif_${N} --log_file=log_massif_${N} -d &
# --log-file=massif_${N}.log  
PID=$!
wait

ms_print massif.out.${PID}
