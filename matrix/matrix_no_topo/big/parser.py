#!/usr/bin/python

import operator
import sys
from operator import add

file_name=sys.argv[1]
lines = []
with open(file_name) as f:
    lines = f.readlines()


print("#np,scatter,gather,parallel,total")

scatter=[]
gather=[]
user=[]
parallel=list(range(1,25,1))
np=list(range(1,25,1))
it=0
for line in lines:
    if line.find("Scatter")>=0:
        scatter.append(float(line[line.find("$")+1:-1]))
        continue
    if line.find("gather")>=0:
        gather.append(float(line[line.find("time")+4:-1]))
        continue
    if line.find("user")>=0:
        user.append(float(line[line.find("sed: ")+4:line.find("CPU")]))
        it=it+1
        continue
    if line.find("parallel")>=0:
        parallel[it]=(float(line[line.find("$")+1:-1]))
        continue


for i in range(len(np)):
    print(i,scatter[i],gather[i],parallel[i],user[i],sep=",")
