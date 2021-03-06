# ################################################################
#  
#    Active Particles on Curved Spaces (APCS)
#    
#    Author: Silke Henkes
#   
#    ICSMB, Department of Physics
#    University of Aberdeen
#    
#    (c) 2013, 2014
#    
#    This program cannot be used, copied, or modified without
#    explicit permission of the author.
#  
# ################################################################

#! /usr/bin/python

import sys, os, glob
import cPickle as pickle
import numpy as np
import scipy as sp
from scipy.io import savemat
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap
#from matplotlib import rc
import matplotlib
from mpl_toolkits.mplot3d import Axes3D

# setting global parameters
#matplotlib.rcParams['text.usetex'] = 'true'
matplotlib.rcParams['lines.linewidth'] = 2
matplotlib.rcParams['axes.linewidth'] = 2
matplotlib.rcParams['xtick.major.size'] = 8
matplotlib.rcParams['ytick.major.size'] = 8
matplotlib.rcParams['font.size']=20
matplotlib.rcParams['legend.fontsize']=14


cdict = {'red':   [(0.0,  0.75, 0.75),
				   (0.3,  1.0, 1.0),
                   (0.5,  0.4, 0.0),
                   (1.0,  0.0, 0.0)],

         'green': [(0.0,  0.0, 0.0),
				   (0.25,  0.0, 0.5),
                   (0.5, 1.0, 1.0),
                   (0.75,  0.5, 0.0),
                   (1.0,  0.0, 0.0)],

         'blue':  [(0.0,  0.0, 0.0),
                   (0.5,  0.0, 0.0),
                   (0.7, 1.0, 1.0),
                   (1.0,  0.25, 0.25)]}
                   

	

# This is the structured data file hierarchy. Replace as appropriate (do not go the Yaouen way and fully automatize ...)

#basedir='/media/drogon/Documents/Curved/Runs_nuslices_phi0.5/'
basedir='/media/drogon/home/silke/Documents/Curved/Runs_nuslices/'


JList=['1']
#vList=[ '0.01', '0.1','0.5']
vList=['0.5']
mList=['s','o']
#vList=[ '0.005', '0.01', '0.02', '0.05', '0.1', '0.2', '0.5']
#JList=['10']
#vList=['1']
nbin=180
rval=28.2094791
#RList=['8','12','16','20','40','60']
nuList=['0.001', '0.1', '0.2', '0.3', '0.5', '0.7', '1', '1.5', '2', '2.5','3','4','5']

testmap=LinearSegmentedColormap('test',cdict,N=len(vList))
testmap2=LinearSegmentedColormap('test',cdict,N=len(nuList))
nstep=20000000
nsave=10000
nsnap=int(nstep/nsave)
skip=int(nsnap/3)
dt=0.001


# Profiles
# Set column to plot
usecolumn=4

profList=[r'$\theta$',r'$\rho$',r'$\sqrt{\langle v^2 \rangle}/v_0$','energy','pressure',r'$\Sigma_{\theta \theta}$',r'$\Sigma_{\theta \phi}$',r'$\Sigma_{\phi \theta}$',r'$\Sigma_{\phi \phi}$',r'$\alpha$',r'$\alpha_v$']
profName=['theta','rho','vrms','energy','pressure','stt','stp','spt','spp','alpha','alpha_v']

	
for i in range(len(vList)):
    plt.figure(figsize=(10,7),linewidth=2.0)
    for j in range(len(nuList)):
        print vList[i],nuList[j]
        ax=plt.gca()
        outfile=basedir+'/profiles_v0' + vList[i] + '_nu' + nuList[j] + '.dat'
        outfile2=basedir + '/axis_v0'  + vList[i] + '_nu' + nuList[j] + '.dat'
        # header='theta rho vel energy pressure alpha alpha_v'
        profiles=sp.loadtxt(outfile, unpack=True)[:,:] 
        isdata=[index for index,value in enumerate(profiles[1,:]) if (value >0)]
        # Corrected
        ## Forgot the normalization of rho by the angle band width
        #if usecolumn==1:
            #normz=2*np.pi*rval*abs(np.cos(profiles[0,:]))
            #profiles[1,isdata]=profiles[1,isdata]/normz[isdata]
            #profiles[1,:]/=np.mean(profiles[1,:])
        if usecolumn==2:
            plt.plot(profiles[0,isdata],profiles[usecolumn,isdata]/float(vList[i]),color=testmap2(j), linestyle='solid',label=nuList[j])
        else:
            plt.plot(profiles[0,isdata],profiles[usecolumn,isdata],color=testmap2(j), linestyle='solid',label=nuList[j])
    #if usecolumn<=8:
        #plt.ylim(0,1.25*profiles[usecolumn,nbin/2])
    if usecolumn==9:
		plt.plot(2*profiles[0,isdata],0.45*2*profiles[0,isdata],'k--')
		plt.plot(2*profiles[0,isdata],0.0*2*profiles[0,isdata],'k--')
		plt.text(0.0,0.25,'slope 0.45')
        #if j==1:
            #plt.plot(2*profiles[0,isdata],1.25*2*profiles[0,isdata],'k--')
            #plt.text(0.5,0.75,'slope 1.25')
        #if j==0:
            #plt.plot(2*profiles[0,isdata],0.1*2*profiles[0,isdata],'k--')
            #plt.text(0.5,0.3,'slope 0.1')
            #plt.ylim(-0.4,0.4)
    plt.xlim(-np.pi/2,np.pi/2)
    #plt.xlim(-0.9,0.9)
    #plt.ylim(-0.35,0.35)
    plt.xlabel(profList[0]) 
    plt.ylabel(profList[usecolumn]) 
    plt.legend(loc=2,ncol=2)
    #plt.title('Velocity' + r'$v_0=$' + vList[i])
    
    #filename=picsfolder + '/profile_' + profName[usecolumn] + '_J' + JList[j] +'.pdf'
    #plt.savefig(filename)
    
    
    
## Order parameter
#plt.figure(figsize=(10,7),linewidth=2.0)
  
#corr=0.01 # 1/effective correlation time: number of independent samples (fudge factor, honestly)
#for i in range(len(vList)):
    #orderpar=np.zeros((len(nuList),))
    #dorder=np.zeros((len(nuList),))
    #nuval=np.zeros((len(nuList),))
    #for j in range(len(nuList)): 
        #print vList[i],nuList[j]
        #outfile2=basedir + '/axis_v0'  + vList[i] + '_nu' + nuList[j] + '.dat'
        #axis=sp.loadtxt(outfile2, unpack=True)[:,:] 
        #orderpar0=np.sqrt(axis[3,:]**2+axis[4,:]**2+axis[5,:]**2)
        #orderpar[j]=np.mean(orderpar0)
        #dorder[j]=np.std(orderpar0)/np.sqrt(corr*len(orderpar0)) 
        #nuval[j]=float(nuList[j])
    #plt.errorbar(nuval,orderpar,yerr=dorder,color=testmap(i), linestyle='solid',marker=mList[i],markersize=10,label=r'$v_0=$' + vList[i])
##hmm=np.linspace(-3,1,10)
##plt.plot(hmm,hmm/hmm,linestyle='--',color='k') 
    
#plt.xlabel(r'$\nu_r$') 
#plt.ylabel('p') 
##plt.ylim(0,1.1)
##plt.xlim(0,2.2)
#plt.legend()
##plt.title('Order parameter')    
    
    
    
## Defect statistics ...


## Defect statistics ...

#plt.figure(figsize=(10,7),linewidth=2.0)
#for i in range(len(vList)):
    #argh=[]
    #mdefects=np.empty((len(nuList),2))
    #for j in range(len(nuList)): 
        #print vList[i],nuList[j]
        ##ax=plt.gca()
        #outfile=basedir+'/defects_nu_' + nuList[j] + 'v0_'+ vList[i] +'_polar.dat'
        #ndefects=sp.loadtxt(outfile, unpack=True)[0:2,:] 
        #print ndefects
        #mdefects[j,0]=np.mean(ndefects[0,:])
        #mdefects[j,1]=np.mean(ndefects[1,:])
        #argh.append(2.0)
    #plt.semilogy(nuList,mdefects[:,1],color=testmap(i),marker=mList[i],markersize=10,linestyle='solid',label=r'$v_0=$' + vList[i])
    #plt.semilogy(nuList,mdefects[:,0],color=testmap(i),marker=mList[i],markersize=10,linestyle='--')
#plt.semilogy(nuList,argh,color='k',linestyle='-.')
#plt.xlabel(r'$\nu_r$')
#plt.ylabel('defect #')
##plt.title('Defects')
#plt.legend()

plt.show()	
		
		
		
		
		
		