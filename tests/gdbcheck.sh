#!/bin/bash
N=$1
if [ -z ${N} ]; then echo "Usage $0 <id>"; exit 1; fi

rm gdb_${N}.log || true
rm log_gdb_${N} || true
rm save_gdb_${N} || true

gdb -ex "run" --args ../heresyrl --name="rnd_ai" --homeworld=hive --background=outcast --role=seeker --rnd_auto_play --pb_delay=0 --save_file=./save_gdb_${N} --log_file=log_gdb_${N} -d
