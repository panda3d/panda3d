#!/usr/bin/python

########################################################################
##
##
## This script builds the panda source tarball and zip-file
##
##    usage: maketarball [version]
##
## The source tarball contains most of what is in CVS, but some of the
## control files (like the CVS directories themselves) are stripped out.
##
## The source tarball contains an rpmbuild 'spec' file so that you can
## easily build a binary RPM: rpmbuild -tb panda3d-version.tar.GZ
##
## The 'spec' file included in the tarball uses the 'makepanda' build
## system to compile panda.
##
########################################################################

import sys,os,time,stat,string,re,getopt,cPickle;

def oscmd(cmd):
  print cmd
  sys.stdout.flush()
  if (os.system(cmd)): sys.exit("Failed")

def writefile(dest,desiredcontents):
  print "Generating file: "+dest
  sys.stdout.flush()
  try:
    wfile = open(dest, 'wb');
    wfile.write(desiredcontents);
    wfile.close();
  except: sys.exit("Cannot write to "+dest);

########################################################################
#
# Locate the root of the panda tree
#
########################################################################

PANDASOURCE=os.path.dirname(os.path.abspath(sys.path[0]))

if ((os.path.exists(os.path.join(PANDASOURCE,"makepanda/makepanda.py"))==0) or
    (os.path.exists(os.path.join(PANDASOURCE,"makepanda/makepanda.sln"))==0) or
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

if (len(sys.argv)==2):
    VERSION = sys.argv[1]
    if (len(VERSION.split(".")) != 3): printUsage()
elif (len(sys.argv)==1):
    VERSION="0.0.0"
    try:
        f = file("dtool/PandaVersion.pp","r")
        pattern = re.compile('^[ \t]*[#][ \t]*define[ \t]+PANDA_VERSION[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)')
        for line in f:
            match = pattern.match(line,0);
            if (match):
                VERSION = match.group(1)+"."+match.group(2)+"."+match.group(3)
                break
        f.close()
    except: sys.exit("Cannot read version number from dtool/PandaVersion.pp")
else: printUsage()
    

########################################################################
##
## The SPEC File
##
########################################################################

SPEC="""Summary: Panda 3D Engine
Name: panda3d
Version: VERSION
Release: 1
Source0: %{name}-%{version}.tar.gz
License: Panda3D License
Group: Development/Libraries
BuildRoot: %{_builddir}/%{name}-%{version}/BUILDROOT
%description
The Panda3D engine.
%prep
%setup -q
%build
makepanda/makepanda.py --version VERSION --everything
%install
rm -rf $RPM_BUILD_ROOT
PANDA=$RPM_BUILD_ROOT/usr/share/panda3d
mkdir -p $PANDA
mkdir -p $RPM_BUILD_ROOT/etc/ld.so.conf.d
mkdir -p $RPM_BUILD_ROOT/usr/bin
cp --recursive built/bin     $PANDA/bin
cp --recursive built/lib     $PANDA/lib
cp --recursive built/etc     $PANDA/etc
cp --recursive built/include $PANDA/include
cp --recursive direct        $PANDA/direct
cp built/direct/__init__.py  $PANDA/direct/__init__.py
cp --recursive models        $PANDA/models
cp --recursive samples       $PANDA/samples
cp --recursive SceneEditor   $PANDA/SceneEditor
cp --recursive Config.prc    $PANDA/Config.prc
cp --recursive LICENSE       $PANDA/LICENSE
echo "/usr/share/panda3d/lib" > $RPM_BUILD_ROOT/etc/ld.so.conf.d/panda3d.conf
for x in $PANDA/bin/* ; do
  base=`basename $x`
  ln -sf /usr/share/panda3d/bin/$base $RPM_BUILD_ROOT/usr/bin
done
for x in $PANDA/direct/src/* ; do
  if [ `basename $x` != extensions ] ; then
    python -c "import compileall; compileall.compile_dir('$x')"
  fi
done
%post
/sbin/ldconfig
%postun
/sbin/ldconfig
%clean
rm -rf $RPM_BUILD_ROOT
%files
%defattr(-,root,root)
/usr/share/panda3d
/etc/ld.so.conf.d/panda3d.conf
/usr/bin
"""

SPEC=SPEC.replace("VERSION",str(VERSION))

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
oscmd("ln -sf ../models       "+TARDIR+"/models")
oscmd("ln -sf ../samples      "+TARDIR+"/samples")
oscmd("ln -sf ../doc          "+TARDIR+"/doc")
oscmd("ln -sf ../makepanda    "+TARDIR+"/makepanda")
oscmd("ln -sf ../../thirdparty/linux-libs-a "+TARDIR+"/thirdparty/linux-libs-a")
writefile(TARDIR+'/panda3d.spec',SPEC)
oscmd("tar --exclude CVS -chzf "+TARDIR+".tar.gz "+TARDIR)
oscmd("rm -rf "+TARDIR)

TARDIR="panda3d-"+VERSION.replace(".","-")
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
oscmd("ln -sf ../models       "+TARDIR+"/models")
oscmd("ln -sf ../samples      "+TARDIR+"/samples")
oscmd("ln -sf ../doc          "+TARDIR+"/doc")
oscmd("ln -sf ../makepanda    "+TARDIR+"/makepanda")
oscmd("ln -sf ../../thirdparty/win-libs-vc7 "+TARDIR+"/thirdparty/win-libs-vc7")
oscmd("ln -sf ../../thirdparty/win-python   "+TARDIR+"/thirdparty/win-python")
oscmd("ln -sf ../../thirdparty/win-util     "+TARDIR+"/thirdparty/win-util")
oscmd("ln -sf ../../thirdparty/win-nsis     "+TARDIR+"/thirdparty/win-nsis")
oscmd("zip -rq "+TARDIR+".zip "+TARDIR+" -x '*/CVS/*'")
oscmd("rm -rf "+TARDIR)


