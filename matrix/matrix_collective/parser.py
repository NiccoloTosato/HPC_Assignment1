#!/usr/bin/python

import operator
import sys
from operator import add

file_name=sys.argv[1]
lines = []
with open(file_name) as f:
    lines = f.readlines()


timing=[]
for line in lines:
