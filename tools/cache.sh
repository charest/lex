#!/bin/bash

algs="hand fsm re2c"

echo "algorithm, i1, il, l1, ll" > cache.txt

python3 ../tools/gen_random.py --output fake_program.txt --lines 1000000

for a in $algs; do
  out=$(valgrind --tool=cachegrind --cachegrind-out-file=cachegrind.${a}.out $PWD/lexit fake_program.txt $a 2>&1)
  line=`cg_annotate --auto=yes cachegrind.${a}.out | grep 'PROGRAM TOTALS' | sed 's/,//g'`
  line=`cg_annotate --auto=yes cachegrind.${a}.out | grep 'PROGRAM TOTALS' | sed 's/,//g'`
  IFS=' ' read -r -a parts <<< "$line"
  Ir=${parts[0]}
  I1mr=${parts[2]}
  ILmr=${parts[4]}
  Dr=${parts[6]}
  D1mr=${parts[8]}
  DLmr=${parts[10]}
  Dw=${parts[12]}
  D1mw=${parts[14]}
  DLmw=${parts[16]}
  
  total_accesses=$((Ir + Dr + Dw))
  l1_misses=$((I1mr + D1mr + D1mw))
  ll_misses=$((ILmr + DLmr + DLmw))


  # Compute hit rate using bc for floating point division
  l1_hit_rate=$(echo "scale=4; 1 - ($l1_misses / $total_accesses)" | bc -l)
  ll_hit_rate=$(echo "scale=4; 1 - ($ll_misses / $l1_misses)" | bc -l)

  dl=$(echo "$out" | grep "LLd miss rate" | awk '{print $5}')
  d1=$(echo "$out" | grep "D1  miss rate" | awk '{print $5}')
  il=$(echo "$out" | grep "LLi miss rate" | awk '{print $5}')
  i1=$(echo "$out" | grep "I1  miss rate" | awk '{print $5}')
  echo $a, $i1, $il, $d1, $dl >> cache.txt



done
