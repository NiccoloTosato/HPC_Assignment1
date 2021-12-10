#!/usr/bin/python

import operator
import sys
from operator import add

file_name=sys.argv[1]
lines = []
with open(file_name) as f:
    lines = f.readlines()


timing=[]
d={}
for line in lines:
    if(line[0]!='#'):
        v=line.split(",")
        v=list(map(float,v))
        v[0]=int(v[0])
        mess_size=str(v[0])
        d_tmp={'mess_size':str(v[0]),'rep':v[1],'lat':v[2],'speed':v[3]}
        
        if mess_size in d:
            tmp=d[mess_size]
            if d_tmp['lat']>tmp['max_lat']:
                tmp['max_lat']=d_tmp['lat']

            if d_tmp['lat']<tmp['min_lat']:
                tmp['min_lat']=d_tmp['lat']

            if d_tmp['speed']>tmp['max_speed']:
                tmp['max_speed']=d_tmp['speed']

            if d_tmp['speed']<tmp['min_speed']:
                tmp['min_speed']=d_tmp['speed'] 

            tmp['speed']+=0.10*d_tmp['speed']
            tmp['lat']+=0.10*d_tmp['lat']   
        
        else:
            d[mess_size]={'mess_size':str(v[0]),'rep':d_tmp['rep'],'lat':d_tmp['lat']*0.10,'max_lat':d_tmp['lat'],'min_lat':d_tmp['lat'],'speed':d_tmp['speed']*0.10,'min_speed':d_tmp['speed'],'max_speed':d_tmp['speed']}
print("#bytes,repetitions,t,mbs,err_speed,err_lat")
for r in d:
    tmp=d[r]
    print(tmp['mess_size'],",",tmp['rep'],",",round(tmp['lat'],3),",",round(tmp['speed'],3),",",round((tmp['max_speed']-tmp['min_speed'])/2,3),",",round((tmp['max_lat']-tmp['min_lat'])/2,3),sep='')
