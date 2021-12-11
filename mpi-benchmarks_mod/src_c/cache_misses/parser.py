#!/usr/bin/python

import operator
import sys
from operator import add

file_name=sys.argv[1]
lines = []
with open(file_name) as f:
    lines = f.readlines()

print("#SIZE,L1,L2,ITERATION,L2TOT")
acc=0
for line in lines:
    if(line.find("L1")>0):
        iteration=int(line[9:line.find(", me")])
        size=int(line[line.find("size")+4:line.find("L1")-1])
        L1=int(line[line.find("L1 missess")+10:line.find("L2")-1])
        L2=int(line[line.find("L2 missess")+10:line.find("L2 tot")-2])
        L2tot=int(line[line.find("L2 total")+8:-1])
        if(size<=16777216):
            if(acc==3):
                print(size,",",L1,",",L2,",",iteration,",",L2tot)
                acc=-1
        else:
            if(acc==2):
                print(size,",",L1,",",L2,",",iteration,",",L2tot)
                acc=-1
        acc+=1
