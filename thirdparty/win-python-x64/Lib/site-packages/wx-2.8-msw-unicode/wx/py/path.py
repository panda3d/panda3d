"""path.py is a utility containing some very simple path utilities for Py to use"""

__author__ = "David N. Mashburn <david.n.mashburn@gmail.com>"
# 07/01/2009

import os
import glob
import commands

def pwd():
    print os.getcwd()

def cd(path,usePrint=True):
    os.chdir(os.path.expandvars(os.path.expanduser(path)))
    if usePrint:
        pwd()

def ls(str='*',fullpath=False):
    g=glob.glob(os.path.expandvars(os.path.expanduser(str)))
    if fullpath:
        for i in g:
            print i
    else:
        for i in g:
            print os.path.split(i)[1]

# This prints the results of running a command in the GUI shell but be warned!
# This is a blocking call, and if you open any kind of interactive 
# command-line program like python or bash, the shell will permanantly
# freeze!
# If you want this kind of behavior to be available, please use ipython
# This is NOT a feature or goal of the Py project!
def sx(str=''):
    print commands.getoutput(str)

#cd('~',usePrint=False)
