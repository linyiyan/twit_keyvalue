#!/bin/bash

start=$((8090))
num=$1
end=$((8090+$1-1))
for (( c=start; c<=end; c++ ))
do
   ./main $c $start $end&
done
