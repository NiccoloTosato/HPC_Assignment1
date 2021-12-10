#!/bin/bash

for filename in ./*.out; do
	cat $(basename "$filename") | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' > tmp.out
	cat tmp.out
	python parser.py  tmp.out > $(basename "$filename").csv
done

rm tmp.out
