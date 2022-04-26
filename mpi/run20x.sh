#!/bin/sh
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
do
  make run
  sleep 0.2

  make run2
  sleep 0.2

  make run3
  sleep 0.2

  make run4
  sleep 0.2
done
