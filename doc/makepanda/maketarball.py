#!/usr/bin/python

########################################################################
##
##
## This script builds the panda source tarballs and zip-files
##
##    usage: maketarball [version] [more options]
##
## The source tarball contains most of what is in CVS, but some of the
## control files (like the CVS directories themselves) are stripped out.
##
########################################################################

import sys,os,time,stat,string,re,getopt,cPickle

def oscmd(cmd):
  print cmd
  sys.stdout.flush()
  if (os.system(cmd)): sys.exit("Failed")

def writefile(dest,desiredcontents):
  print "Generating file: "+dest
  sys.stdout.flush()
  try:
    wfile = open(dest, 'wb')
    wfile.write(desiredcontents)
    wfile.close();
  except: sys.exit("Cannot write to "+dest)

########################################################################
#
# Locate the root of the panda tree
#
########################################################################

PANDASOURCE=os.path.dirname(os.path.abspath(sys.path[0]))

if ((os.path.exists(os.path.join(PANDASOURCE,"makepanda/makepanda.py"))==0) or
    (os.path.exists(os.path.join(PANDASOURCE,"dtool","src","dtoolbase","dtoolbase.h"))==0) or
    (os.path.exists(os.path.join(PANDASOURCE,"panda","src","pandabase","pandabase.h"))==0)):
    sys.exit("I am unable to locate the root of the panda source tree.")

os.chdir(PANDASOURCE)

########################################################################
##
## Read the default version number from dtool/PandaVersion.pp
##
## Parse the command-line arguments.
##
########################################################################

def printUsage():
    sys.exit("usage: maketarball [version]")

if (len(sys.argv)>=2):
    VERSION = sys.argv[1]
    if (len(VERSION.split(".")) != 3): printUsage()
elif (len(sys.argv)==1):
    VERSION="0.0.0"
    try:
        f = file("dtool/PandaVersion.pp","r")
        pattern = re.compile('^[ \t]*[#][ \t]*define[ \t]+PANDA_VERSION[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)')
        for line in f:
            match = pattern.match(line,0)
            if (match):
                VERSION = match.group(1)+"."+match.group(2)+"."+match.group(3)
                break
        f.close()
    except: sys.exit("Cannot read version number from dtool/PandaVersion.pp")
else: printUsage()


########################################################################
##
## Build the Zip-file and Tar-File
##
########################################################################

TARDIR="panda3d-"+VERSION
oscmd("rm -rf "+TARDIR)
oscmd("mkdir -p "+TARDIR)
oscmd("mkdir -p "+TARDIR+"/thirdparty")
oscmd("ln -sf ../dtool        "+TARDIR+"/dtool")
oscmd("ln -sf ../panda        "+TARDIR+"/panda")
oscmd("ln -sf ../direct       "+TARDIR+"/direct")
oscmd("ln -sf ../pandaapp     "+TARDIR+"/pandaapp")
oscmd("ln -sf ../pandatool    "+TARDIR+"/pandatool")
oscmd("ln -sf ../ppremake     "+TARDIR+"/ppremake")
oscmd("ln -sf ../SceneEditor  "+TARDIR+"/SceneEditor")
oscmd("ln -sf ../dmodels      "+TARDIR+"/dmodels")
oscmd("ln -sf ../models       "+TARDIR+"/models")
oscmd("ln -sf ../samples      "+TARDIR+"/samples")
oscmd("ln -sf ../doc          "+TARDIR+"/doc")
oscmd("ln -sf ../makepanda    "+TARDIR+"/makepanda")
oscmd("ln -sf ../../thirdparty/linux-libs-a "+TARDIR+"/thirdparty/linux-libs-a")
oscmd("ln -sf ../../thirdparty/Pmw          "+TARDIR+"/thirdparty/Pmw")
oscmd("tar --exclude CVS -chzf "+TARDIR+".tar.gz "+TARDIR)
oscmd("rm -rf "+TARDIR)


oscmd("mkdir -p "+TARDIR)
oscmd("mkdir -p "+TARDIR+"/thirdparty")
oscmd("ln -sf ../dtool        "+TARDIR+"/dtool")
oscmd("ln -sf ../panda        "+TARDIR+"/panda")
oscmd("ln -sf ../direct       "+TARDIR+"/direct")
oscmd("ln -sf ../pandaapp     "+TARDIR+"/pandaapp")
oscmd("ln -sf ../pandatool    "+TARDIR+"/pandatool")
oscmd("ln -sf ../ppremake     "+TARDIR+"/ppremake")
oscmd("ln -sf ../SceneEditor  "+TARDIR+"/SceneEditor")
oscmd("ln -sf ../dmodels      "+TARDIR+"/dmodels")
oscmd("ln -sf ../models       "+TARDIR+"/models")
oscmd("ln -sf ../samples      "+TARDIR+"/samples")
oscmd("ln -sf ../doc          "+TARDIR+"/doc")
oscmd("ln -sf ../makepanda    "+TARDIR+"/makepanda")
oscmd("ln -sf ../../thirdparty/win-libs-vc7 "+TARDIR+"/thirdparty/win-libs-vc7")
oscmd("ln -sf ../../thirdparty/win-python   "+TARDIR+"/thirdparty/win-python")
oscmd("ln -sf ../../thirdparty/win-util     "+TARDIR+"/thirdparty/win-util")
oscmd("ln -sf ../../thirdparty/win-nsis     "+TARDIR+"/thirdparty/win-nsis")
oscmd("ln -sf ../../thirdparty/win-extras   "+TARDIR+"/thirdparty/win-extras")
oscmd("ln -sf ../../thirdparty/Pmw          "+TARDIR+"/thirdparty/Pmw")
oscmd("zip -rq "+TARDIR+".zip "+TARDIR+" -x '*/CVS/*'")
oscmd("rm -rf "+TARDIR)


oscmd("mkdir -p "+TARDIR)
oscmd("mkdir -p "+TARDIR+"/thirdparty")
oscmd("ln -sf ../dtool        "+TARDIR+"/dtool")
oscmd("ln -sf ../panda        "+TARDIR+"/panda")
oscmd("ln -sf ../direct       "+TARDIR+"/direct")
oscmd("ln -sf ../pandaapp     "+TARDIR+"/pandaapp")
oscmd("ln -sf ../pandatool    "+TARDIR+"/pandatool")
oscmd("ln -sf ../ppremake     "+TARDIR+"/ppremake")
oscmd("ln -sf ../SceneEditor  "+TARDIR+"/SceneEditor")
oscmd("ln -sf ../dmodels      "+TARDIR+"/dmodels")
oscmd("ln -sf ../models       "+TARDIR+"/models")
oscmd("ln -sf ../doc          "+TARDIR+"/doc")
oscmd("ln -sf ../makepanda    "+TARDIR+"/makepanda")
oscmd("zip -rq "+TARDIR+"-sourceforge.zip "+TARDIR+" -x '*/CVS/*'")
oscmd("rm -rf "+TARDIR)


oscmd("mkdir -p "+TARDIR)
oscmd("ln -sf ../samples      "+TARDIR+"/samples")
oscmd("zip -rq "+TARDIR+"-samples.zip "+TARDIR+" -x '*/CVS/*'")
oscmd("rm -rf "+TARDIR)


oscmd("mkdir -p "+TARDIR)
oscmd("mkdir -p "+TARDIR+"/thirdparty")
oscmd("ln -sf ../../thirdparty/linux-libs-a "+TARDIR+"/thirdparty/linux-libs-a")
oscmd("ln -sf ../../thirdparty/Pmw          "+TARDIR+"/thirdparty/Pmw")
oscmd("zip -rq "+TARDIR+"-tools-linux.zip "+TARDIR+" -x '*/CVS/*'")
oscmd("rm -rf "+TARDIR)


oscmd("mkdir -p "+TARDIR)
oscmd("mkdir -p "+TARDIR+"/thirdparty")
oscmd("ln -sf ../../thirdparty/win-libs-vc7 "+TARDIR+"/thirdparty/win-libs-vc7")
oscmd("ln -sf ../../thirdparty/win-python   "+TARDIR+"/thirdparty/win-python")
oscmd("ln -sf ../../thirdparty/win-util     "+TARDIR+"/thirdparty/win-util")
oscmd("ln -sf ../../thirdparty/win-nsis     "+TARDIR+"/thirdparty/win-nsis")
oscmd("ln -sf ../../thirdparty/win-extras   "+TARDIR+"/thirdparty/win-extras")
oscmd("ln -sf ../../thirdparty/Pmw          "+TARDIR+"/thirdparty/Pmw")
oscmd("zip -rq "+TARDIR+"-tools-win32.zip "+TARDIR+" -x '*/CVS/*'")
oscmd("rm -rf "+TARDIR)


oscmd("mkdir -p "+TARDIR)
oscmd("ln -sf ../thirdparty "+TARDIR+"/thirdparty")
oscmd("zip -rq "+TARDIR+"-tools-all.zip "+TARDIR+" -x '*/CVS/*'")
oscmd("rm -rf "+TARDIR)
