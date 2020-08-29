#!/bin/bash

for c in 1 2 4 8 12 16 20 24
do
    sync; sudo echo 3 > /proc/sys/vm/drop_cachesl
     ./run.sh ${c} 90000
done
