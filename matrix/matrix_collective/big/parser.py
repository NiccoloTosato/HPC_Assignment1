#!/usr/bin/python

import operator
import sys
from operator import add

file_name=sys.argv[1]
lines = []
with open(file_name) as f:
    lines = f.readlines()


print("#np,prepare,scatter,parallel,reorder,total,gather,recv")

prepare=[]
scatter=[]
user=[]
reorder=[]
parallel=list(range(1,25,1))
gather=list(range(1,25,1))
recv=list(range(1,25,1))

np=[1,4,8,12,16,20,24]
it=0

for line in lines:
    if line.find("overhead")>=0:
        prepare.append(float(line[line.find("$")+1:-1]))
        continue
    if line.find("Master")>=0:
        scatter.append(float(line[line.find("$")+1:-1]))
        continue
    if line.find("user")>=0:
        user.append(float(line[line.find("sed: ")+4:line.find("CPU")]))
        it=it+1
        continue
    if line.find("Computation")>=0:
        parallel[it]=(float(line[line.find("$")+1:-1]))
        continue
    if line.find("Gather")>=0:
        gather[it]=(float(line[line.find("$")+1:-1]))
        continue
    if line.find("Slave")>=0:
        recv[it]=(float(line[line.find("$")+1:-1]))
        continue
    if line.find("Reorder")>=0:
        reorder.append(float(line[line.find("$")+1:-1]))
        continue



for i in range(len(np)):
    print(np[i],prepare[i],scatter[i],parallel[i],reorder[i],user[i],gather[i],recv[i],sep=",")
