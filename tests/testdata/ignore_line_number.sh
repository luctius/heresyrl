#!/bin/sh -x
sed -E -i".bl" "s/line=([0-9]+)/line=000/g" $1
