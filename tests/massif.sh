#!/bin/bash
N=$1
if [ -z ${N} ]; then echo "Usage $0 <id>"; exit 1; fi

valgrind  --tool=massif ../heresyrl --name="rnd_ai" --homeworld=hive --background=outcast --role=seeker --rnd_auto_play --pb_delay=0 --save_file=./save_${N} --log_file=log_${N}
# --log-file=massif_${N}.log  
echo ms_print massif.$!
