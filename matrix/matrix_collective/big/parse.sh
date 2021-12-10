#!/bin/bash

for filename in ./*.out; do
	#cat $(basename "$filename") | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' > $(basename "$filename")
	python parser.py  $(basename "$filename") > $(basename "$filename").csv
	echo  $(basename "$filename" .out).csv
done

