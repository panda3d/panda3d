########################################################################
##
## Win32 Usage: makepanda\expandimports.bat
## Linux Usage: makepanda/expandimports.py
##
########################################################################

import sys,os,re
sys.path = ["direct/src/directscripts"] + sys.path
import gendocs

########################################################################
##
## Make sure panda has been built.
##
########################################################################

if (os.path.isfile("built/pandac/input/libpgraph.in")==0) or (os.path.isfile("built/pandac/input/libputil.in")==0):
    sys.exit("Cannot read the interrogate-output files in built/pandac/input")

gendocs.expandImports("built/pandac/input", "direct", "samples")
