#!/usr/bin/python

########################################################################
##
## This script builds the panda source tarball.
##
##
## The source tarball contains a hardwired version-number.  You specify
## the version number using the options --v1, --v2, --v3.
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
##
## Parse the command-line arguments.
##
########################################################################

VERSION1=1
VERSION2=0
VERSION3=0

def parseopts(args):
  global VERSION1,VERSION2,VERSION3
  longopts = ["v1=","v2=","v3="]
  try:
    opts, extras = getopt.getopt(args, "", longopts)
    for option,value in opts:
      if (option=="--v1"): VERSION1=int(value)
      if (option=="--v2"): VERSION2=int(value)
      if (option=="--v3"): VERSION3=int(value)
  except: usage(0)

parseopts(sys.argv[1:])

########################################################################
##
## Which files go into the source-archive?
##
########################################################################

ARCHIVE=["dtool","panda","direct","pandatool","pandaapp",
         "ppremake","SceneEditor","models","samples",
         "Config.pp.sample","Config.prc","LICENSE","README",
         "INSTALL-PP","INSTALL-MK","makepanda.bat","makepanda.py","maketarball.py",
         "InstallerNotes","ReleaseNotes","makepanda.sln","makepanda.vcproj"]

########################################################################
##
## The SPEC File
##
########################################################################

SPEC="""Summary: Panda 3D Engine
Name: panda3d
Version: VERSION1.VERSION2.VERSION3
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
makepanda.py --v1 VERSION1 --v2 VERSION2 --v3 VERSION3 --no-installer
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
echo "/usr/share/panda3d/lib" > $RPM_BUILD_ROOT/etc/ld.so.conf.d/panda3d
for x in $PANDA/bin/* ; do
  base=`basename $x`
  ln -sf /usr/share/panda3d/bin/$base $RPM_BUILD_ROOT/usr/bin
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
/etc/ld.so.conf.d/panda3d
/usr/bin
"""

SPEC=SPEC.replace("VERSION1",str(VERSION1))
SPEC=SPEC.replace("VERSION2",str(VERSION2))
SPEC=SPEC.replace("VERSION3",str(VERSION3))

########################################################################
##
## Build the tar-ball
##
########################################################################

TARDIR="panda3d-"+str(VERSION1)+"."+str(VERSION2)+"."+str(VERSION3)
oscmd("rm -rf "+TARDIR)
oscmd("mkdir -p "+TARDIR)
oscmd("mkdir -p "+TARDIR+"/thirdparty")
for x in ARCHIVE: oscmd("ln -sf ../"+x+" "+TARDIR+"/"+x)
oscmd("ln -sf ../../thirdparty/linux-libs-a "+TARDIR+"/thirdparty/linux-libs-a")
writefile(TARDIR+'/panda3d.spec',SPEC)
oscmd("tar --exclude CVS -chzf "+TARDIR+".tar.gz "+TARDIR)
oscmd("rm -rf "+TARDIR)


