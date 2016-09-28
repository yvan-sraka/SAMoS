# * *************************************************************
# *  
# *   Soft Active Mater on Surfaces (SAMoS)
# *   
# *   Author: Rastko Sknepnek
# *  
# *   Division of Physics
# *   School of Engineering, Physics and Mathematics
# *   University of Dundee
# *   
# *   (c) 2013, 2014
# * 
# *   School of Science and Engineering
# *   School of Life Sciences 
# *   University of Dundee
# * 
# *   (c) 2015, 2016
# * 
# *   Author: Silke Henkes
# * 
# *   Department of Physics 
# *   Institute for Complex Systems and Mathematical Biology
# *   University of Aberdeen  
# * 
# *   (c) 2014, 2015, 2016
# *  
# *   This program cannot be used, copied, or modified without
# *   explicit written permission of the authors.
# * 
# * ***************************************************************

# Utility code for building a single polymer on a plane

from datetime import *
from random import uniform, seed 
from math import *
import argparse
import numpy as np


parser = argparse.ArgumentParser()
parser.add_argument("-N", "--N", type=int, default=10, help="numper of particles")
parser.add_argument("-a", "--len", type=float, default=1.0, help="bonds length")
parser.add_argument("-o", "--output", type=str, default='poly.xyz', help="output file")
args = parser.parse_args()

print
print "\tSoft Active Matter on Surfaces (SAMoS)"
print "\tBuilding of single polymer on plane"
print 
print "\tRastko Sknepnek"
print "\tUniversity of Dundee"
print "\t(c) 2016"
print "\t----------------------------------------------"
print 
print "\tNumber of beads : ", args.N
print "\tOutput file : ", args.output
print 

start = datetime.now()

seed()

r = np.zeros((args.N,2))
out = open(args.output,'w')
out.write('%d\n' % args.N)
out.write('Generated by make_filament.py on %s\n' % str(datetime.now()))
out.write('C  %f  %f  %f\n' % (0.0,0.0,0.0))
for i in range(1,args.N):
    phi = uniform(-pi/2,pi/2)
    x = args.len*cos(phi)
    y = args.len*sin(phi)
    r[i] = np.array([r[i-1][0]+x,r[i-1][1]+y])
    out.write('C  %f  %f  %f\n' % (r[i][0],r[i][1],0.0))
out.close()


end = datetime.now()

total = end - start

print 
print "  *** Completed in ", total.total_seconds(), " seconds *** "
print


