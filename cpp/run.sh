#!/bin/sh
g++ -std=c++1z -Igsl -O2 -Wall $1
./a.out
#echo "TECHIO> terminal -i ./a.out"
#sleep 60
