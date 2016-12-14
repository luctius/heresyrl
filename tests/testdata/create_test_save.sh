#!/bin/sh

if [ -z $1 ]
then
    echo "please specify a target save (ex. './save.2')"
    exit
fi

../../heresyrl --test_mode -l --save_file=./$1 -d || ../../build/heresyrl --test_mode -l --save_file=./$1 -d
