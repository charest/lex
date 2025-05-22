#!/bin/bash

lines="10000 100000 1000000 10000000"
algs="hand fsm re2c"

echo "algorithm, lines, time" > bench.txt

for a in $algs; do
  for l in $lines; do
    python3 ../tools/gen_random.py --output fake_program.txt --lines $l
    out=`$PWD/lexit fake_program.txt $a`
    echo $out
    t=`echo "$out" | awk -F'Avg Elapsed: ' '{print $2}' | awk '{print $1}'`
    echo $a, $l, $t >> bench.txt
  done
done
