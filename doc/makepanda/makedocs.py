########################################################################
##
## Win32 Usage: makepanda\makedocs.bat
## Linux Usage: makepanda/makedocs.py
##
########################################################################

import sys,os,re
sys.path = ["direct/src/directscripts"] + sys.path
import gendocs

########################################################################
##
## Some handy utility functions.
##
########################################################################

def MakeDirectory(path):
    if os.path.isdir(path): return 0
    os.mkdir(path)

########################################################################
##
## Read the version number from built/include/pandaVersion.h
##
########################################################################

VERSION="0.0.0"
try:
    f = file("built/include/pandaVersion.h","r")
    pattern = re.compile('^\s*[#]\s*define\s+PANDA_VERSION_STR\s+["]([0-9.]+)["]')
    for line in f:
        match = pattern.match(line,0)
        if (match):
            VERSION = match.group(1)
            break
    f.close()
except: sys.exit("Cannot read version number from built/include/pandaVersion.h")

print "Generating docs for "+VERSION

########################################################################
##
## Make sure panda has been built.
##
########################################################################

if (os.path.isfile("built/pandac/input/libpgraph.in")==0) or (os.path.isfile("built/pandac/input/libputil.in")==0):
    sys.exit("Cannot read the interrogate-output files in built/pandac/input")

########################################################################
##
## Generate the PHP version.
##
## The manual is in the form of a bunch of HTML files that can be
## included by a PHP script called "/apiref.php".
##
########################################################################

MakeDirectory("referphp")
gendocs.generate(VERSION, "built/pandac/input", "direct", "referphp", "", "", "/apiref.php?page=", "")

########################################################################
##
## Generate the HTML version.
##
## The manual is in the form of a bunch of standalone HTML files
## that contain links to each other.
##
########################################################################

HEADER = "<html><head></head><body>\n"
FOOTER = "</body></html>\n"

MakeDirectory("reference")
gendocs.generate(VERSION, "built/pandac/input", "direct", "reference", HEADER, FOOTER, "", ".html")

