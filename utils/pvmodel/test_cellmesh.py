import numpy as np
from cellmesh import *
from analyse_cells import *

from collections import OrderedDict

def test_bond_intersection():
    case = OrderedDict()
    x_c = np.array([20.,20.])
    a = np.array([25., 20.])
    b = np.array([16., 20.])
    case['flat'] = np.array([[20.,20.], [25.,20.], [16., 20.]])
    case['inner'] = np.array([[0.,0.], [-1.,-1.], [1.,1.]])
    case['half'] = np.array([[0.,0.], [-1/2., 0.], [5.,1.]])
    wl = 2.
    for ck, cv in case.items():
        print 
        print 'case ', ck
        x_c, a, b = cv

        inter = bond_intersection(x_c, wl, a, b)
        if inter == None:
            return None
        else:
            ma, mb, line = inter
            print ma, mb
            print line(ma), line(mb)


import time
#http://stackoverflow.com/questions/1622943/timeit-versus-timing-decorator
def timeit(f):
    def timed(*args, **kw):

        ts = time.time()
        result = f(*args, **kw)
        te = time.time()

        print 'func:%r args:[%r, %r] took: %2.4f sec' % \
          (f.__name__, args, kw, te-ts)
        return result
    return timed

tdataf = '/home/dan/cells/run/basic/vpotential_only/cell_0000000000.dat'
boundary_test_dataf = '/home/dan/cells/run/rprefA/rA_3.5/cell_0000004500.dat'
# simple object represents the args object I use to configure Senarios
class Block(object):
    def __init__(self, dd={}, **kwargs):
        for k, v in dd.items():
            setattr(self, k, v)
        for k, v in kwargs.items():
            setattr(self, k, v)

wld = 3.0
usedata = boundary_test_dataf
defaults = {'k':1.0, 'gamma':0., 'L':0., 'wl':wld, 'input':usedata, 'dir':'/home/dan/tmp/test_cells',
        's':False}
def time_stress():
    args = Block(defaults)
    stressrun = Stress_Senario(args)
    stressrun.save_parameters()
    nxt= timeit(stressrun.next)
    nxt()


if __name__=='__main__':
    #test_bond_intersection()
    time_stress()


