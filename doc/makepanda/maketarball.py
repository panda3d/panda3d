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
## The source tarball contains an rpmbuild 'spec' file so that you can
## easily build a binary RPM: rpmbuild -tb panda3d-version.tar.GZ
##
## The 'spec' file included in the tarball uses the 'makepanda' build
## system to compile panda.
##
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
    (os.path.exists(os.path.join(PANDASOURCE,"dtool","src","dtoolbase","dtoolbase.h"))==0) or
    (os.path.exists(os.path.join(PANDASOURCE,"panda","src","pandabase","pandabase.h"))==0)):
    sys.exit("I am unable to locate the root of the panda source tree.")

os.chdir(PANDASOURCE)

########################################################################
##
## Locate the Python SDK
##
########################################################################

if sys.platform == "win32":
    PythonSDK="python2.2"
    if 0: # Needs testing:
        if   (os.path.isdir("C:/Python22")): PythonSDK = "C:/Python22"
        elif (os.path.isdir("C:/Python23")): PythonSDK = "C:/Python23"
        elif (os.path.isdir("C:/Python24")): PythonSDK = "C:/Python24"
        elif (os.path.isdir("C:/Python25")): PythonSDK = "C:/Python25"
        else: sys.exit("Cannot find the python SDK")
else:
    if   (os.path.isdir("/usr/include/python2.5")): PythonSDK = "/usr/include/python2.5"
    elif (os.path.isdir("/usr/include/python2.4")): PythonSDK = "/usr/include/python2.4"
    elif (os.path.isdir("/usr/include/python2.3")): PythonSDK = "/usr/include/python2.3"
    elif (os.path.isdir("/usr/include/python2.2")): PythonSDK = "/usr/include/python2.2"
    else: sys.exit("Cannot find the python SDK")
    # this is so that the user can find out which version of python was used.

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
makepanda/makepanda.py --version VERSION --everything MOREARGUMENTS
%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/include
mkdir -p $RPM_BUILD_ROOT/usr/lib
mkdir -p $RPM_BUILD_ROOT/usr/share/panda3d
mkdir -p $RPM_BUILD_ROOT/usr/lib/PYTHONV/lib-dynload
mkdir -p $RPM_BUILD_ROOT/usr/lib/PYTHONV/site-packages
mkdir -p $RPM_BUILD_ROOT/etc/ld.so.conf.d
mkdir -p $RPM_BUILD_ROOT/usr/bin

sed -e 's@$THIS_PRC_DIR/[.][.]@/usr/share/panda3d@' < built/etc/Config.prc > $RPM_BUILD_ROOT/etc/Config.prc

cp built/etc/Confauto.prc    $RPM_BUILD_ROOT/etc/Confauto.prc
cp --recursive built/include $RPM_BUILD_ROOT/usr/include/panda3d
cp --recursive direct        $RPM_BUILD_ROOT/usr/share/panda3d/direct
cp --recursive built/pandac  $RPM_BUILD_ROOT/usr/share/panda3d/pandac
cp --recursive built/Pmw     $RPM_BUILD_ROOT/usr/share/panda3d/Pmw
cp built/direct/__init__.py  $RPM_BUILD_ROOT/usr/share/panda3d/direct/__init__.py
cp --recursive SceneEditor   $RPM_BUILD_ROOT/usr/share/panda3d/SceneEditor
cp --recursive built/models  $RPM_BUILD_ROOT/usr/share/panda3d/models
cp --recursive samples       $RPM_BUILD_ROOT/usr/share/panda3d/samples
cp --recursive built/lib     $RPM_BUILD_ROOT/usr/lib/panda3d
cp doc/LICENSE               $RPM_BUILD_ROOT/usr/lib/panda3d/LICENSE
cp doc/LICENSE               $RPM_BUILD_ROOT/usr/share/panda3d/LICENSE
cp doc/LICENSE               $RPM_BUILD_ROOT/usr/include/panda3d/LICENSE
cp doc/ReleaseNotes          $RPM_BUILD_ROOT/usr/share/panda3d/ReleaseNotes
echo "/usr/lib/panda3d" >    $RPM_BUILD_ROOT/etc/ld.so.conf.d/panda3d.conf
echo "/usr/share/panda3d" >  $RPM_BUILD_ROOT/usr/lib/PYTHONV/site-packages/panda3d.pth
cp built/bin/*               $RPM_BUILD_ROOT/usr/bin/

for x in built/lib/* ; do
  base=`basename $x`
  ln -sf /usr/lib/panda3d/$base $RPM_BUILD_ROOT/usr/lib/PYTHONV/lib-dynload/$base
done
for x in $RPM_BUILD_ROOT/usr/share/panda3d/direct/src/* ; do
  if [ `basename $x` != extensions ] ; then
    python -c "import compileall; compileall.compile_dir('$x')"
  fi
done
python -c "import compileall ; compileall.compile_dir('$RPM_BUILD_ROOT/usr/share/panda3d/Pmw');"
python -c "import compileall ; compileall.compile_dir('$RPM_BUILD_ROOT/usr/share/panda3d/SceneEditor');"

chmod -R 555 $RPM_BUILD_ROOT/usr/share/panda3d

%post
/sbin/ldconfig
rm -rf /usr/lib/PYTHONV/direct
rm -rf /usr/lib/PYTHONV/SceneEditor
%postun
/sbin/ldconfig
%clean
rm -rf $RPM_BUILD_ROOT
%files
%defattr(-,root,root)
/etc/Confauto.prc
/etc/Config.prc
/usr/share/panda3d
/etc/ld.so.conf.d/panda3d.conf
/usr/bin
/usr/lib
/usr/include/panda3d
"""

MORE=''
for x in sys.argv[2:]: MORE=MORE+x+' '
SPEC=SPEC.replace("VERSION",str(VERSION))
SPEC=SPEC.replace("MOREARGUMENTS",MORE)
SPEC=SPEC.replace("PYTHONV",os.path.basename(PythonSDK))

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
writefile(TARDIR+'/panda3d.spec',SPEC)
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
