#!/usr/bin/env python2

from random import random

dims = 3
n = 10240
print dims
print n
print 0
for i in range(0, n):
    print 50, # mass
    print 1, # radius
    for j in range(0, dims): # position
        print (int(random() * 2500))/10.,
    for j in range(0, dims): # acceleration
        print 0,
    for j in range(0, dims): # velocity
        print 0,
    print ''
print 40, 0.01
