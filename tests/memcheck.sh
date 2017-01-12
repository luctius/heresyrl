#!/bin/bash
N=$1
if [ -z ${N} ]; then echo "Usage $0 <id>"; exit 1; fi

rm memcheck_${N}.log || true
rm log_memcheck_${N} || true
rm save_memcheck_${N} || true

valgrind  --show-leak-kinds=all --leak-check=full --track-origins=yes  --log-file=memcheck_${N}.log  --tool=memcheck ../heresyrl --name="rnd_ai" --homeworld=hive --background=outcast --role=seeker --rnd_auto_play --pb_delay=0 --save_file=./save_memcheck_${N} --log_file=log_memcheck_${N} -d
