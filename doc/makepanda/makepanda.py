#!/usr/bin/python
########################################################################
#
# Caution: there are two separate, independent build systems:
# 'makepanda', and 'ppremake'.  Use one or the other, do not attempt
# to use both.  This file is part of the 'makepanda' system.
#
# To build panda using this script, type 'makepanda.py' on unix
# or 'makepanda.bat' on windows, and examine the help-text.
# Then run the script again with the appropriate options to compile
# panda3d.
#
########################################################################

import sys,os,time,stat,string,re,getopt,cPickle,fnmatch,threading,Queue
from glob import glob

########################################################################
##
## PARSING THE COMMAND LINE OPTIONS
##
## You might be tempted to change the defaults by editing them
## here.  Don't do it.  Instead, create a script that compiles
## panda with your preferred options.  Or, create
## a 'makepandaPreferences' file and put it into your python path.
##
########################################################################

if (sys.platform == "win32"): COMPILERS=["MSVC7"]
if (sys.platform == "linux2"): COMPILERS=["LINUXA"]
COMPILER=COMPILERS[0]
OPTIMIZE="3"
INSTALLER=0
GENMAN=0
VERSION=0
VERBOSE=1
COMPRESSOR="zlib"
PACKAGES=["PYTHON","ZLIB","PNG","JPEG","TIFF","VRPN","FMOD","NVIDIACG","HELIX","NSPR",
          "OPENSSL","FREETYPE","FFTW","MILES","MAYA5","MAYA6","MAYA65","MAX5","MAX6","MAX7",
          "BISON","FLEX","PANDATOOL","PANDAAPP"]
OMIT=PACKAGES[:]
WARNINGS=[]
DIRECTXSDK = None
MAYASDK = {}
MAXSDK = {}
MAXSDKCS = {}
PYTHONSDK=0
STARTTIME=time.time()
BUILTANYTHING=0
SLAVEFILE=0
DEPENDENCYQUEUE=[]
FILEDATECACHE = {}
CXXINCLUDECACHE = {}
ALLIN=[]
SLAVEBUILD=0
THREADCOUNT=0

##########################################################################################
#
# If there is a makepandaPreferences.py, import it
#
##########################################################################################

try:
    from makepandaPreferences import *
except ImportError:
    pass

########################################################################
##
## Utility Routines
##
########################################################################

def exit(msg):
    print msg
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(1)

def filedate(path):
    if FILEDATECACHE.has_key(path):
        return FILEDATECACHE[path]
    try: date = os.path.getmtime(path)
    except: date = 0
    FILEDATECACHE[path] = date
    return date

def updatefiledate(path):
    try: date = os.path.getmtime(path)
    except: date = 0
    FILEDATECACHE[path] = date

def chkolder(file,date,files):
    if type(files) == str:
        source = filedate(files)
        if (source==0):
            exit("Error: source file not readable: "+files)
        if (date < source):
            return 1
        return 0
    for sfile in files:
        if (chkolder(file,date,sfile)):
            return 1
    return 0

def older(files,others):
    for file in files:
        date = filedate(file)
        if (date == 0):
            return 1
        elif (chkolder(file, date, others)):
            return 1
    return 0

def xpaths(prefix,base,suffix):
    if type(base) == str:
        return prefix + base + suffix
    result = []
    for x in base:
        result.append(xpaths(prefix,x,suffix))
    return result

if sys.platform == "win32":
    import _winreg
    def GetRegistryKey(path, subkey):
        k1=0
        key=0
        try:
            key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, path, 0, _winreg.KEY_READ)
            k1, k2 = _winreg.QueryValueEx(key, subkey)
        except: pass
        if (key!=0): _winreg.CloseKey(key)
        return k1

def oscmd(cmd):
    print cmd
    sys.stdout.flush()
    if sys.platform == "win32":
        exe = cmd.split()[0]+".exe"
        if os.path.isfile(exe)==0:
            for i in os.environ["PATH"].split(";"):
                if os.path.isfile(os.path.join(i, exe)):
                    exe = os.path.join(i, exe)
                    break
            if os.path.isfile(exe)==0:
                exit("Cannot find "+exe+" on search path")
        res = os.spawnl(os.P_WAIT, exe, cmd)
    else:
        res = os.system(cmd)
    if res != 0:
	exit("")

def getbuilding(opts):
    building = 0
    for x in opts:
        if (x[:9]=="BUILDING_"): building = x[9:]
    return building

def getoptlevel(opts,defval):
    for x in opts:
        if (x[:3]=="OPT"):
            n = int(x[3:])
            if (n > defval): defval = n
    return defval

def PrettyTime(t):
    t = int(t)
    hours = t/3600
    t -= hours*3600
    minutes = t/60
    t -= minutes*60
    seconds = t
    if (hours): return str(hours)+" hours "+str(minutes)+" min"
    if (minutes): return str(minutes)+" min "+str(seconds)+" sec"
    return str(seconds)+" sec"

def MakeDirectory(path):
    if os.path.isdir(path): return 0
    os.mkdir(path)

def ReadFile(wfile):
    try:
        srchandle = open(wfile, "rb")
        data = srchandle.read()
        srchandle.close()
        return data
    except: exit("Cannot read "+wfile)

def WriteFile(wfile,data):
    try:
        dsthandle = open(wfile, "wb")
        dsthandle.write(data)
        dsthandle.close()
        updatefiledate(wfile)
    except: exit("Cannot write "+wfile)

def ConditionalWriteFile(dest,desiredcontents):
    try:
        rfile = open(dest, 'rb')
        contents = rfile.read(-1)
        rfile.close()
    except:
        contents=0
    if contents != desiredcontents:
        if VERBOSE:
            print "Regenerating file: "+dest
        sys.stdout.flush()
        WriteFile(dest,desiredcontents)

def SetDifference(add, sub):
    set = {}
    for x in add: set[x]=1
    for x in sub:
        if (set.has_key(x)):
            del set[x]
    return set.keys()

def PkgSelected(pkglist, pkg):
    if (pkglist.count(pkg)==0): return 0
    if (OMIT.count(pkg)): return 0
    return 1

def DependencyQueue(fn, args, targets, sources):
    DEPENDENCYQUEUE.append([fn,args,targets,sources])

########################################################################
##
## Help with packages.
##
## Output some brief information to help someone understand what the
## package options are.
##
########################################################################

def packageInfo():
    print """
  See panda3d/doc/INSTALL-PP for more detailed information.

  3D modeling an painting packages:
    MAX5      3D Studio Max version 5
    MAX6      3D Studio Max version 6
    MAX7      3D Studio Max version 7

    MAYA5     Maya version 5
    MAYA6     Maya version 6

  Audio playback:
    FMOD      f mod
              "http://www.fmod.org/"
              A music and sound effects library (including playback).
              (for .wav, .mp3 and other files)

    MILES     Miles Sound System from RAD Game Tools
              "http://www.radgametools.com/default.htm"
              A proprietary (non-opensource) audio library.
              (for .wav, .mp3, and other files).

  Compression/decompression:
    ZLIB      z lib
              "http://www.gzip.org/zlib"
              A commression/decomression library.
              (for .zip and similar files and data)

  Font manipulation:
    FREETYPE  free type
              "http://www.freetype.org/"
              A font manipulation library.
              (for .ttf files).


  Image support libraries:
    JPEG      Join Photographic Experts Group
              "http://www.ijg.org"
              An image library.
              (.jpg and .jpeg files)

    PNG       Portable Network Graphics
              "http://www.libpng.org"
              An image library.
              (.png files)

    TIFF      Tagged Image File Format
              "ftp://ftp.sgi.com/graphics/tiff"
              An image library.
              (.tiff files)

  Misc libraries:
    FFTW      Fast Fourier Transform (in the West)
              "http://www.fftw.org/"
              A library for computing DFT in one or more dimensions.

    NVIDIACG  nVidia cg
              "http://developer.nvidia.com/page/cg_main.html"
              (for .??? files)
              A library for gpu programming (shaders and such).

  Network communication:
    SSL       Open Secure Socket Layer
              "http://www.openssl.org/"
              A network encryption library.

    NSPR      Netscape Portable Runtime
              "http://www.mozilla.org/projects/nspr/"
              Used for network sockets and threading.

  User input:
    VRPN      Virtual Reality Peripheral Network
              "http://www.cs.unc.edu/Research/vrpn/"
              A controller/peripheral input library.
"""
    exit("")

########################################################################
##
## Command-line parser.
##
## You can type "makepanda --help" to see all the options.
##
########################################################################

def usage(problem):
    if (problem):
        print ""
        print problem
    print ""
    print "Makepanda generates a 'built' subdirectory containing a"
    print "compiled copy of Panda3D.  Command-line arguments are:"
    print ""
    print "  --help            (print the help message you're reading now)"
    print "  --package-info    (help info about the optional packages)"
    print "  --compiler X      (currently, compiler can only be MSVC7,LINUXA)"
    print "  --optimize X      (optimization level can be 1,2,3,4)"
    print "  --installer       (build an installer)"
    print "  --v1 X            (set the major version number)"
    print "  --v2 X            (set the minor version number)"
    print "  --v3 X            (set the sequence version number)"
    print "  --lzma            (use lzma compression when building installer)"
    print "  --slaves X        (use the distributed build system. see manual)"
    print "  --threads N       (use the multithreaded build system. see manual)"
    print ""
    for pkg in PACKAGES:
        p = pkg.lower()
        print "  --use-%-9s   --no-%-9s (enable/disable use of %s)"%(p, p, pkg)
    print ""
    print "  --nothing         (disable every third-party lib)"
    print "  --everything      (enable every third-party lib)"
    print ""
    print "  --quiet           (print less output)"
    print "  --verbose         (print more output and debugging info)"
    print ""
    print "The simplest way to compile panda is to just type:"
    print ""
    print "  makepanda --everything"
    print ""
    exit("")

def parseopts(args):
    global COMPILER,OPTIMIZE,OMIT,INSTALLER,GENMAN,SLAVEBUILD
    global VERSION,COMPRESSOR,DIRECTXSDK,VERBOSE,SLAVEFILE,THREADCOUNT
    longopts = [
        "help","package-info","compiler=","directx-sdk=","slavebuild=",
        "optimize=","everything","nothing","installer","quiet","verbose",
        "version=","lzma","no-python","slaves=","threads="]
    anything = 0
    for pkg in PACKAGES: longopts.append("no-"+pkg.lower())
    for pkg in PACKAGES: longopts.append("use-"+pkg.lower())
    try:
        opts, extras = getopt.getopt(args, "", longopts)
        for option,value in opts:
            if (option=="--help"): raise "usage"
            elif (option=="--package-info"): raise "package-info"
            elif (option=="--compiler"): COMPILER=value
            elif (option=="--directx-sdk"): DIRECTXSDK=value
            elif (option=="--optimize"): OPTIMIZE=value
            elif (option=="--quiet"): VERBOSE-=1
            elif (option=="--verbose"): VERBOSE+=1
            elif (option=="--installer"): INSTALLER=1
            elif (option=="--genman"): GENMAN=1
            elif (option=="--everything"): OMIT=[]
            elif (option=="--nothing"): OMIT=PACKAGES[:]
            elif (option=="--slaves"): SLAVEFILE=value
            elif (option=="--threads"): THREADCOUNT=int(value)
            elif (option=="--slavebuild"): SLAVEBUILD=value
            elif (option=="--version"):
                VERSION=value
                if (len(VERSION.split(".")) != 3): raise "usage"
            elif (option=="--lzma"): COMPRESSOR="lzma"
            else:
                for pkg in PACKAGES:
                    if (option=="--use-"+pkg.lower()):
                        if (OMIT.count(pkg)): OMIT.remove(pkg)
                        break
                for pkg in PACKAGES:
                    if (option=="--no-"+pkg.lower()):
                        if (OMIT.count(pkg)==0): OMIT.append(pkg)
                        break
            anything = 1
    except "package-info": packageInfo()
    except: usage(0)
    if (anything==0): usage(0)
    if   (OPTIMIZE=="1"): OPTIMIZE=1
    elif (OPTIMIZE=="2"): OPTIMIZE=2
    elif (OPTIMIZE=="3"): OPTIMIZE=3
    elif (OPTIMIZE=="4"): OPTIMIZE=4
    else: usage("Invalid setting for OPTIMIZE")
    if (COMPILERS.count(COMPILER)==0): usage("Invalid setting for COMPILER: "+COMPILER)

parseopts(sys.argv[1:])

########################################################################
#
# Locate the root of the panda tree
#
########################################################################

PANDASOURCE=os.path.dirname(os.path.abspath(sys.path[0]))

if ((os.path.exists(os.path.join(PANDASOURCE,"makepanda/makepanda.py"))==0) or
    (os.path.exists(os.path.join(PANDASOURCE,"dtool","src","dtoolbase","dtoolbase.h"))==0) or
    (os.path.exists(os.path.join(PANDASOURCE,"panda","src","pandabase","pandabase.h"))==0)):
    exit("I am unable to locate the root of the panda source tree.")

os.chdir(PANDASOURCE)

########################################################################
##
## If you have the "sdks" directory, supply all sdks
##
## This is a temporary hack, it may go away.
##
########################################################################

if (os.path.isdir("sdks")):
    DIRECTXSDK="sdks/directx"
    MAXSDKCS["MAX5"] = "sdks/maxsdk5"
    MAXSDKCS["MAX6"] = "sdks/maxsdk6"
    MAXSDKCS["MAX7"] = "sdks/maxsdk7"
    MAXSDK["MAX5"]   = "sdks/maxsdk5"
    MAXSDK["MAX6"]   = "sdks/maxsdk6"
    MAXSDK["MAX7"]   = "sdks/maxsdk7"
    MAYASDK["MAYA5"] = "sdks/maya5"
    MAYASDK["MAYA6"] = "sdks/maya6"
    MAYASDK["MAYA65"] = "sdks/maya65"

########################################################################
##
## Locate the DirectX SDK
##
########################################################################

if sys.platform == "win32" and DIRECTXSDK is None:
    dxdir = GetRegistryKey("SOFTWARE\\Microsoft\\DirectX SDK", "DX9SDK Samples Path")
    if (dxdir != 0): DIRECTXSDK = os.path.dirname(dxdir)
    else:
        dxdir = GetRegistryKey("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment","DXSDK_DIR")
        if (dxdir != 0): DIRECTXSDK=dxdir
        else:
            dxdir = GetRegistryKey("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment","DXSDKROOT")
            if dxdir != 0:
                if dxdir[-2:]=="/.":
                    DIRECTXSDK=dxdir[:-1]
                else:
                    DIRECTXSDK=dxdir
            else:
                exit("The registry does not appear to contain a pointer to the DirectX 9.0 SDK.")
    DIRECTXSDK=DIRECTXSDK.replace("\\", "/").rstrip("/")

########################################################################
##
## Locate the Maya 5.0 and Maya 6.0 SDK
##
########################################################################

MAYAVERSIONS=[("MAYA5",  "SOFTWARE\\Alias|Wavefront\\Maya\\5.0\\Setup\\InstallPath"),
              ("MAYA6",  "SOFTWARE\\Alias|Wavefront\\Maya\\6.0\\Setup\\InstallPath"),
              ("MAYA65", "SOFTWARE\\Alias|Wavefront\\Maya\\6.5\\Setup\\InstallPath")
]

for (ver,key) in MAYAVERSIONS:
    if (OMIT.count(ver)==0) and (MAYASDK.has_key(ver)==0):
        if (sys.platform == "win32"):
            MAYASDK[ver]=GetRegistryKey(key, "MAYA_INSTALL_LOCATION")
            if (MAYASDK[ver] == 0):
                WARNINGS.append("The registry does not appear to contain a pointer to the "+ver+" SDK.")
                WARNINGS.append("I have automatically added this command-line option: --no-"+ver.lower())
                OMIT.append(ver)
            else:
                MAYASDK[ver] = MAYASDK[ver].replace("\\", "/").rstrip("/")
        else:
            WARNINGS.append(ver+" not yet supported under linux")
            WARNINGS.append("I have automatically added this command-line option: --no-"+ver.lower())
            OMIT.append(ver)

########################################################################
##
## Locate the 3D Studio Max and Character Studio SDKs
##
########################################################################

MAXVERSIONS = [("MAX5", "SOFTWARE\\Autodesk\\3DSMAX\\5.0\\MAX-1:409", "uninstallpath", "Cstudio\\Sdk"),
               ("MAX6", "SOFTWARE\\Autodesk\\3DSMAX\\6.0",            "installdir",    "maxsdk\\cssdk\\include"),
               ("MAX7", "SOFTWARE\\Autodesk\\3DSMAX\\7.0",            "Installdir",    "maxsdk\\include\\CS")]

for version,key1,key2,subdir in MAXVERSIONS:
    if (OMIT.count(version)==0) and (MAXSDK.has_key(version)==0):
        if (sys.platform == "win32"):
            top = GetRegistryKey(key1,key2)
            if (top == 0):
                WARNINGS.append("The registry does not appear to contain a pointer to "+version)
                WARNINGS.append("I have automatically added this command-line option: --no-"+version.lower())
                OMIT.append(version)
            else:
                if (os.path.isdir(top + "\\" + subdir)==0):
                    WARNINGS.append("Your copy of "+version+" does not include the character studio SDK")
                    WARNINGS.append("I have automatically added this command-line option: --no-"+version.lower())
                    OMIT.append(version)
                else:
                    MAXSDK[version] = top + "maxsdk"
                    MAXSDKCS[version] = top + subdir
        else:
            WARNINGS.append(version+" not yet supported under linux")
            WARNINGS.append("I have automatically added this command-line option: --no-"+version.lower())
            OMIT.append(version)

########################################################################
##
## Locate the Python SDK
##
########################################################################

if (OMIT.count("PYTHON")==0):
    if (sys.platform == "win32"):
        PYTHONSDK="thirdparty/win-python"
    else:
        if   (os.path.isdir("/usr/include/python2.5")): PYTHONSDK = "/usr/include/python2.5"
        elif (os.path.isdir("/usr/include/python2.4")): PYTHONSDK = "/usr/include/python2.4"
        elif (os.path.isdir("/usr/include/python2.3")): PYTHONSDK = "/usr/include/python2.3"
        elif (os.path.isdir("/usr/include/python2.2")): PYTHONSDK = "/usr/include/python2.2"
        else: exit("Cannot find the python SDK")

########################################################################
##
## Locate Visual Studio 7.0 or 7.1 or the Visual Toolkit 2003
##
## The visual studio compiler doesn't work unless you set up a
## couple of environment variables to point at the compiler.
##
########################################################################


def AddToVisualStudioPath(path,add):
    if (os.environ.has_key(path)):
        os.environ[path] = add + ";" + os.environ[path]
    else:
        os.environ[path] = add

def LocateVisualStudio():

    # Try to use the Visual Toolkit 2003
    vcdir = GetRegistryKey("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment","VCToolkitInstallDir")
    if (vcdir != 0) or (os.environ.has_key("VCTOOLKITINSTALLDIR")):
        if (vcdir == 0): vcdir = os.environ["VCTOOLKITINSTALLDIR"]
        platsdk=GetRegistryKey("SOFTWARE\\Microsoft\\MicrosoftSDK\\InstalledSDKs\\8F9E5EF3-A9A5-491B-A889-C58EFFECE8B3",
                               "Install Dir")
        if (platsdk == 0): exit("Found VC Toolkit, but cannot locate MS Platform SDK")
        WARNINGS.append("Using visual toolkit: "+vcdir)
        WARNINGS.append("Using MS Platform SDK: "+platsdk)
        AddToVisualStudioPath("PATH", vcdir + "\\bin")
        AddToVisualStudioPath("INCLUDE", platsdk + "\\include")
        AddToVisualStudioPath("INCLUDE", vcdir + "\\include")
        AddToVisualStudioPath("INCLUDE", DIRECTXSDK + "\\include")
        AddToVisualStudioPath("LIB",     platsdk + "\\lib")
        AddToVisualStudioPath("LIB",     vcdir + "\\lib")
        AddToVisualStudioPath("LIB",     "thirdparty\\win-libs-vc7\\extras\\lib")
        AddToVisualStudioPath("INCLUDE", DIRECTXSDK + "\\lib")
        return

    # Try to use Visual Studio
    vcdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\7.1", "InstallDir")
    if (vcdir == 0):
        vcdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\7.0", "InstallDir")
    if (vcdir != 0) and (vcdir[-13:] == "\\Common7\\IDE\\"):
        vcdir = vcdir[:-12]
        WARNINGS.append("Using visual studio: "+vcdir)
        AddToVisualStudioPath("PATH",    vcdir + "vc7\\bin")
        AddToVisualStudioPath("PATH",    vcdir + "Common7\\IDE")
        AddToVisualStudioPath("PATH",    vcdir + "Common7\\Tools")
        AddToVisualStudioPath("PATH",    vcdir + "Common7\\Tools\\bin\\prerelease")
        AddToVisualStudioPath("PATH",    vcdir + "Common7\\Tools\\bin")
        AddToVisualStudioPath("INCLUDE", vcdir + "vc7\\ATLMFC\\INCLUDE")
        AddToVisualStudioPath("INCLUDE", vcdir + "vc7\\include")
        AddToVisualStudioPath("INCLUDE", vcdir + "vc7\\PlatformSDK\\include\\prerelease")
        AddToVisualStudioPath("INCLUDE", vcdir + "vc7\\PlatformSDK\\include")
        AddToVisualStudioPath("LIB",     vcdir + "vc7\\ATLMFC\\LIB")
        AddToVisualStudioPath("LIB",     vcdir + "vc7\\LIB")
        AddToVisualStudioPath("LIB",     vcdir + "vc7\\PlatformSDK\\lib\\prerelease")
        AddToVisualStudioPath("LIB",     vcdir + "vc7\\PlatformSDK\\lib")
        return

    # Give up
    exit("Cannot locate Microsoft Visual Studio 7.0, 7.1, or the Visual Toolkit 2003")


if (COMPILER == "MSVC7"):
    LocateVisualStudio()

##########################################################################################
#
# Disable Helix
#
##########################################################################################

if (OMIT.count("HELIX")==0):
    WARNINGS.append("HELIX is currently nonoperational")
    WARNINGS.append("I have automatically added this command-line option: --no-helix")
    OMIT.append("HELIX")

##########################################################################################
#
# See if there's a "MILES" subdirectory under 'thirdparty'
#
##########################################################################################

if (os.path.isdir(os.path.join("thirdparty", "win-libs-vc7", "miles"))==0):
    if (OMIT.count("MILES")==0):
        WARNINGS.append("You do not have a copy of MILES sound system")
        WARNINGS.append("I have automatically added this command-line option: --no-miles")
        OMIT.append("MILES")

##########################################################################################
#
# Verify that LD_LIBRARY_PATH contains the built/lib directory.
#
# If not, add it on a temporary basis, and issue a warning.
#
##########################################################################################

if (sys.platform != "win32"):
    BUILTLIB = os.path.abspath("built/lib")
    try:
        LDPATH = []
        f = file("/etc/ld.so.conf","r")
        for line in f: LDPATH.append(line.rstrip())
        f.close()
    except: LDPATH = []
    if (os.environ.has_key("LD_LIBRARY_PATH")):
        LDPATH = LDPATH + os.environ["LD_LIBRARY_PATH"].split(":")
    if (LDPATH.count(BUILTLIB)==0):
        WARNINGS.append("Caution: the built/lib directory is not in LD_LIBRARY_PATH")
        WARNINGS.append("or /etc/ld.so.conf.  You must add it before using panda.")
        if (os.environ.has_key("LD_LIBRARY_PATH")):
            os.environ["LD_LIBRARY_PATH"] = BUILTLIB + ":" + os.environ["LD_LIBRARY_PATH"]
        else:
            os.environ["LD_LIBRARY_PATH"] = BUILTLIB

#######################################################################
#
# Ensure that bison can find its skeleton file
#
#######################################################################

if (sys.platform == "win32"):
     os.environ["BISON_SIMPLE"] = "thirdparty/win-util/bison.simple"

########################################################################
##
## Choose a version number for the installer
##
########################################################################

if (VERSION == 0):
    try:
        f = file("dtool/PandaVersion.pp","r")
        pattern = re.compile('^[ \t]*[#][ \t]*define[ \t]+PANDA_VERSION[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)')
        for line in f:
            match = pattern.match(line,0)
            if (match):
                VERSION = match.group(1)+"."+match.group(2)+"."+match.group(3)
                break
        f.close()
    except: VERSION="0.0.0"

########################################################################
#
# Sanity check some command-line arguments.
#
########################################################################

if (INSTALLER) and (OMIT.count("PYTHON")):
    exit("Cannot build installer without python")

########################################################################
##
## Give a Status Report on Command-Line Options
##
########################################################################

def printStatus(header,warnings):
    global VERBOSE
    if VERBOSE >= -2:
        print ""
        print "-------------------------------------------------------------------"
        print header
        tkeep = ""
        tomit = ""
        for x in PACKAGES:
            if (OMIT.count(x)==0): tkeep = tkeep + x + " "
            else:                  tomit = tomit + x + " "
        print "Makepanda: Compiler:",COMPILER
        print "Makepanda: Optimize:",OPTIMIZE
        print "Makepanda: Keep Pkg:",tkeep
        print "Makepanda: Omit Pkg:",tomit
        print "Makepanda: DirectX SDK dir:",DIRECTXSDK
        print "Makepanda: Verbose vs. Quiet Level:",VERBOSE
        if (GENMAN): print "Makepanda: Generate API reference manual"
        else       : print "Makepanda: Don't generate API reference manual"
        if (sys.platform == "win32"):
            if INSTALLER:  print "Makepanda: Build installer, using",COMPRESSOR
            else        :  print "Makepanda: Don't build installer"
        print "Makepanda: Version ID: "+VERSION
        for x in warnings: print "Makepanda: "+x
        print "-------------------------------------------------------------------"
        print ""
        sys.stdout.flush()

if (SLAVEBUILD==0):
    printStatus("Makepanda Initial Status Report", WARNINGS)


########################################################################
##
## The CXX include-cache.
##
## Dictionary: for each CXX source file, a list of all the
## include-directives inside that file.
##
## Makepanda analyzes include-directives to determine the dependencies
## of C source files.  This requires us to read the C source files,
## a time-consuming process.  This means that doing a 'makepanda'
## takes quite a bit of time, even if there's nothing to compile.
##
## To accelerate this process, we store the list of include-directives
## in each source file in the "CXX include-cache".  This cache is
## preserved (using the 'cPickle' module) from execution to execution
## of makepanda.  The use of file dates in the cache makes it very
## unlikely for the cache to get out-of-sync with the source tree.
##
########################################################################

iCachePath="built/tmp/makepanda-icache"
try: icache = open(iCachePath,'rb')
except: icache = 0
if (icache!=0):
    CXXINCLUDECACHE = cPickle.load(icache)
    icache.close()

########################################################################
##
## CxxGetIncludes
##
## return a list of the include-directives in a given source file
##
########################################################################

global CxxIncludeRegex
CxxIncludeRegex = re.compile('^[ \t]*[#][ \t]*include[ \t]+"([^"]+)"[ \t\r\n]*$')

def CxxGetIncludes(path):
    date = filedate(path)
    if (CXXINCLUDECACHE.has_key(path)):
        cached = CXXINCLUDECACHE[path]
        if (cached[0]==date): return cached[1]
    try: sfile = open(path, 'rb')
    except: exit("Cannot open source file \""+path+"\" for reading.")
    include = []
    for line in sfile:
        match = CxxIncludeRegex.match(line,0)
        if (match):
            incname = match.group(1)
            include.append(incname)
    sfile.close()
    CXXINCLUDECACHE[path] = [date, include]
    return include

########################################################################
##
## CxxFindSource
##
## given a source file name and a directory list, searches the
## directory list for the given source file.  Returns the full
## pathname of the located file.
##
########################################################################

def CxxFindSource(name, ipath):
    for dir in ipath:
        if (dir == "."): full = name
        else: full = dir + "/" + name
        if filedate(full) > 0: return full
    return 0

########################################################################
##
## CxxFindHeader
##
## given a source file name and an include directive in that source
## file, locates the relevant header file.
##
########################################################################

def CxxFindHeader(srcfile, incfile, ipath):
    if (incfile[:1]=="."):
        last = srcfile.rfind("/")
        if (last < 0): exit("CxxFindHeader cannot handle this case #1")
        srcdir = srcfile[:last+1]
        while (incfile[:1]=="."):
            if (incfile[:2]=="./"):
                incfile = incfile[2:]
            elif (incfile[:3]=="../"):
                incfile = incfile[3:]
                last = srcdir[:-1].rfind("/")
                if (last < 0): exit("CxxFindHeader cannot handle this case #2")
                srcdir = srcdir[:last+1]
            else: exit("CxxFindHeader cannot handle this case #3")
        full = srcdir + incfile
        if filedate(full) > 0: return full
        return 0
    else: return CxxFindSource(incfile, ipath)

########################################################################
##
## CxxCalcDependencies(srcfile, ipath, ignore)
##
## Calculate the dependencies of a source file given a
## particular include-path.  Any file in the list of files to
## ignore is not considered.
##
########################################################################

global CxxIgnoreHeader
global CxxDependencyCache
CxxIgnoreHeader = {}
CxxDependencyCache = {}

def CxxCalcDependencies(srcfile, ipath, ignore):
    if (SLAVEBUILD!=0): return []
    if (CxxDependencyCache.has_key(srcfile)):
        return CxxDependencyCache[srcfile]
    if (ignore.count(srcfile)): return []
    dep = {}
    dep[srcfile] = 1
    includes = CxxGetIncludes(srcfile)
    for include in includes:
        if (CxxIgnoreHeader.has_key(include)==0):
            header = CxxFindHeader(srcfile, include, ipath)
            if (header!=0):
                if (ignore.count(header)==0):
                    hdeps = CxxCalcDependencies(header, ipath, [srcfile]+ignore)
                    for x in hdeps: dep[x] = 1
            else:
                print "CAUTION: header file "+include+" cannot be found in "+srcfile+" IPATH="+str(ipath)
    result = dep.keys()
    CxxDependencyCache[srcfile] = result
    return result

def CxxCalcDependenciesAll(srcfiles, ipath):
    dep = {}
    for srcfile in srcfiles:
        for x in CxxCalcDependencies(srcfile, ipath, []):
            dep[x] = 1
    return dep.keys()

def ReadCvsEntries(dir):
    try:
        if (os.path.isfile(dir+"/CVS-Entries")):
            srchandle = open(dir+"/CVS-Entries", "r")
        else:
            srchandle = open(dir+"/CVS/Entries", "r")
        files = []
        for line in srchandle:
            if (line[0]=="/"):
                s = line.split("/",2)
                if (len(s)==3): files.append(s[1])
        srchandle.close()
        files.sort()
        return files
    except: return 0

########################################################################
##
## Routines to copy files into the build tree
##
########################################################################

def CopyFile(dstfile,srcfile):
    if (dstfile[-1]=='/'):
        dstdir = dstfile
        fnl = srcfile.rfind("/")
        if (fnl < 0): fn = srcfile
        else: fn = srcfile[fnl+1:]
        dstfile = dstdir + fn
    if (older([dstfile],srcfile)):
        global VERBOSE
        if VERBOSE >= 1:
            print "Copying \"%s\" --> \"%s\""%(srcfile, dstfile)
        WriteFile(dstfile,ReadFile(srcfile))

def CopyAllFiles(dstdir, srcdir, suffix=""):
    suflen = len(suffix)
    files = os.listdir(srcdir)
    for x in files:
        if (os.path.isfile(srcdir+x)):
            if (suflen==0) or (x[-suflen:]==suffix):
                CopyFile(dstdir+x, srcdir+x)

def CopyAllHeaders(dir, skip=[]):
    if (SLAVEBUILD!=0): return
    # get a list of headers
    dirlist = os.listdir(dir)
    dirlist.sort()
    files = fnmatch.filter(dirlist,"*.h")+fnmatch.filter(dirlist,"*.I")+fnmatch.filter(dirlist,"*.T")
    # actually copy the headers.
    copied = []
    if (skip!="ALL"):
        # even if you skip all, the warning-messages
        # about "x is not in CVS" are still useful.
        for filename in files:
            if (skip.count(filename)==0):
                srcfile = dir + "/" + filename
                dstfile = "built/include/" + filename
                if (older([dstfile],srcfile)):
                    copied.append(filename)
                    WriteFile(dstfile,ReadFile(srcfile))
    # sanity check - do headers in directory match headers in CVS?
    cvsentries = ReadCvsEntries(dir)
    if (cvsentries != 0):
        cvsheaders = fnmatch.filter(cvsentries,"*.h")+fnmatch.filter(cvsentries,"*.I")+fnmatch.filter(cvsentries,"*.T")
        for x in SetDifference(files, cvsheaders):
            if ((skip=="ALL") or (skip.count(x)==0)):
                msg = "WARNING: header file %s is in your directory, but not in CVS"%(dir+"/"+x)
                print msg
                WARNINGS.append(msg)
        for x in SetDifference(cvsheaders, files):
            if ((skip=="ALL") or (skip.count(x)==0)):
                msg = "WARNING: header file %s is CVS, but not in your directory"%(dir+"/"+x)
                print msg
                WARNINGS.append(msg)

def CopyTree(dstdir,srcdir):
    if (os.path.isdir(dstdir)): return 0
    if (COMPILER=="MSVC7"): cmd = 'xcopy /I/Y/E/Q "' + srcdir + '" "' + dstdir + '"'
    if (COMPILER=="LINUXA"): cmd = 'cp --recursive --force ' + srcdir + ' ' + dstdir
    oscmd(cmd)
    updatefiledate(dstdir)

########################################################################
##
## EnqueueCxx
##
########################################################################

def CompileCxxMSVC7(wobj,fullsrc,ipath,opts):
    cmd = "cl /Fo" + wobj + " /nologo /c"
    if (OMIT.count("PYTHON")==0): cmd = cmd + " /Ibuilt/python/include"
    if (opts.count("DXSDK")): cmd = cmd + ' /I"' + DIRECTXSDK + '/include"'
    for ver in ["MAYA5","MAYA6","MAYA65"]:
      if (opts.count(ver)): cmd = cmd + ' /I"' + MAYASDK[ver] + '/include"'
    for max in ["MAX5","MAX6","MAX7"]:
        if (PkgSelected(opts,max)):
            cmd = cmd + ' /I"' + MAXSDK[max] + '/include" /I"' + MAXSDKCS[max] + '" /D' + max
    for pkg in PACKAGES:
        if (pkg[:4] != "MAYA") and PkgSelected(opts,pkg):
            cmd = cmd + " /Ithirdparty/win-libs-vc7/" + pkg.lower() + "/include"
    for x in ipath: cmd = cmd + " /I" + x
    if (opts.count('NOFLOATWARN')): cmd = cmd + ' /wd4244 /wd4305'
    if (opts.count("WITHINPANDA")): cmd = cmd + ' /DWITHIN_PANDA'
    if (opts.count("MSFORSCOPE")==0): cmd = cmd + ' /Zc:forScope'
    if (opts.count("PTMALLOC2")): cmd = cmd + ' /DUSE_MEMORY_PTMALLOC2'
    optlevel = getoptlevel(opts,OPTIMIZE)
    if (optlevel==1): cmd = cmd + " /MD /Zi /RTCs /GS"
    if (optlevel==2): cmd = cmd + " /MD /Zi "
    if (optlevel==3): cmd = cmd + " /MD /Zi /O2 /Ob2 /DFORCE_INLINING "
    if (optlevel==4): cmd = cmd + " /MD /Zi /Ox /Ob2 /DFORCE_INLINING /GL "
    cmd = cmd + " /Fd" + wobj[:-4] + ".pdb"
    building = getbuilding(opts)
    if (building): cmd = cmd + " /DBUILDING_" + building
    cmd = cmd + " /EHsc /Zm300 /DWIN32_VC /DWIN32 /W3 " + fullsrc
    oscmd(cmd)

def CompileCxxLINUXA(wobj,fullsrc,ipath,opts):
    if (fullsrc[-2:]==".c"): cmd = 'gcc -c -o ' + wobj
    else:                    cmd = 'g++ -ftemplate-depth-30 -c -o ' + wobj
    if (OMIT.count("PYTHON")==0): cmd = cmd + ' -I"' + PYTHONSDK + '"'
    if (PkgSelected(opts,"VRPN")):     cmd = cmd + ' -Ithirdparty/linux-libs-a/vrpn/include'
    if (PkgSelected(opts,"FFTW")):     cmd = cmd + ' -Ithirdparty/linux-libs-a/fftw/include'
    if (PkgSelected(opts,"FMOD")):     cmd = cmd + ' -Ithirdparty/linux-libs-a/fmod/include'
    if (PkgSelected(opts,"NVIDIACG")): cmd = cmd + ' -Ithirdparty/linux-libs-a/nvidiacg/include'
    if (PkgSelected(opts,"NSPR")):     cmd = cmd + ' -Ithirdparty/linux-libs-a/nspr/include'
    if (PkgSelected(opts,"FREETYPE")): cmd = cmd + ' -I/usr/include/freetype2'
    for x in ipath: cmd = cmd + ' -I' + x
    if (opts.count("WITHINPANDA")): cmd = cmd + ' -DWITHIN_PANDA'
    optlevel = getoptlevel(opts,OPTIMIZE)
    if (optlevel==1): cmd = cmd + " -g"
    if (optlevel==2): cmd = cmd + " -O1"
    if (optlevel==3): cmd = cmd + " -O2"
    if (optlevel==4): cmd = cmd + " -O2"
    building = getbuilding(opts)
    if (building): cmd = cmd + " -DBUILDING_" + building
    cmd = cmd + ' ' + fullsrc
    oscmd(cmd)

def EnqueueCxx(obj=0,src=0,ipath=[],opts=[],xdep=[]):
    if ((obj==0)|(src==0)): exit("syntax error in EnqueueCxx directive")
    if (COMPILER=="MSVC7"):
        wobj = "built/tmp/"+obj
        fn = CompileCxxMSVC7
    if (COMPILER=="LINUXA"):
        wobj = "built/tmp/" + obj[:-4] + ".o"
        fn = CompileCxxLINUXA
    if (SLAVEBUILD!=0) and (SLAVEBUILD!=wobj): return
    ipath = ["built/tmp"] + ipath + ["built/include"]
    fullsrc = CxxFindSource(src, ipath)
    if (fullsrc == 0): exit("Cannot find source file "+src)
    dep = CxxCalcDependencies(fullsrc, ipath, []) + xdep
    DependencyQueue(fn, [wobj,fullsrc,ipath,opts], [wobj], dep)

########################################################################
##
## CompileBison
##
########################################################################

def CompileBisonMSVC7(pre, dsth, dstc, wobj, ipath, opts, src):
#   CopyFile(".", "thirdparty/win-util/bison.simple")
    oscmd('thirdparty/win-util/bison -y -d -o built/tmp/y_tab.c -p '+pre+' '+src)
    CopyFile(dstc, "built/tmp/y_tab.c")
    CopyFile(dsth, "built/tmp/y_tab.h")
    CompileCxxMSVC7(wobj,dstc,ipath,opts)

def CompileBisonLINUXA(pre, dsth, dstc, wobj, ipath, opts, src):
    oscmd("bison -y -d -o built/tmp/y.tab.c -p "+pre+" "+src)
    CopyFile(dstc, "built/tmp/y.tab.c")
    CopyFile(dsth, "built/tmp/y.tab.h")
    CompileCxxLINUXA(wobj,dstc,ipath,opts)

def EnqueueBison(ipath=0,opts=0,pre=0,obj=0,dsth=0,src=0):
    if ((ipath==0)|(opts==0)|(pre==0)|(obj==0)|(dsth==0)|(src==0)):
        exit("syntax error in EnqueueBison directive")
    if (COMPILER=="MSVC7"):
        wobj="built/tmp/"+obj
        fn = CompileBisonMSVC7
    if (COMPILER=="LINUXA"):
        wobj="built/tmp/"+obj[:-4]+".o"
        fn = CompileBisonLINUXA
    if (SLAVEBUILD!=0) and (SLAVEBUILD!=wobj): return
    ipath = ["built/tmp"] + ipath + ["built/include"]
    fullsrc = CxxFindSource(src, ipath)
    if (OMIT.count("BISON")):
        dir = os.path.dirname(fullsrc)
        CopyFile("built/tmp/"+dstc, dir+"/"+dstc+".prebuilt")
        CopyFile("built/tmp/"+dsth, dir+"/"+dsth+".prebuilt")
        EnqueueCxx(ipath=ipath,opts=opts,obj=obj,src=dstc)
        return()
    dstc=obj[:-4]+".cxx"
    if (fullsrc == 0): exit("Cannot find source file "+src)
    dstc="built/tmp/"+dstc
    dsth="built/tmp/"+dsth
    DependencyQueue(fn, [pre,dsth,dstc,wobj,ipath,opts,fullsrc], [wobj, dsth], [fullsrc])

########################################################################
##
## CompileFlex
##
########################################################################

def CompileFlexMSVC7(pre,dst,src,wobj,ipath,opts,dashi):
    if (dashi): oscmd("thirdparty/win-util/flex -i -P" + pre + " -o"+dst+" "+src)
    else:       oscmd("thirdparty/win-util/flex    -P" + pre + " -o"+dst+" "+src)
    CompileCxxMSVC7(wobj,dst,ipath,opts)

def CompileFlexLINUXA(pre,dst,src,wobj,ipath,opts,dashi):
    if (dashi): oscmd("flex -i -P" + pre + " -o "+dst+" "+src)
    else:       oscmd("flex    -P" + pre + " -o "+dst+" "+src)
    CompileCxxLINUXA(wobj,dst,ipath,opts)

def EnqueueFlex(ipath=0,opts=0,pre=0,obj=0,src=0,dashi=0):
    if ((ipath==0)|(opts==0)|(pre==0)|(obj==0)|(src==0)):
        exit("syntax error in EnqueueFlex directive")
    if (COMPILER=="MSVC7"):
        wobj="built/tmp/"+obj[:-4]+".obj"
        dst="built/tmp/"+obj[:-4]+".cxx"
        fn=CompileFlexMSVC7
    if (COMPILER=="LINUX"):
        wobj="built/tmp/"+obj[:-4]+".o"
        dst="built/tmp/"+obj[:-4]+".cxx"
        fn=CompileFlexLINUXA
    if (SLAVEBUILD!=0) and (SLAVEBUILD!=wobj): return
    ipath = ["built/tmp"] + ipath + ["built/include"]
    fullsrc = CxxFindSource(src, ipath)
    if (OMIT.count("FLEX")):
        dir = os.path.dirname(fullsrc)
        CopyFile("built/tmp/"+dst, dir+"/"+dst+".prebuilt")
        EnqueueCxx(ipath=IPATH, opts=OPTS, obj=obj, src=dst)
        return()
    if (fullsrc == 0): exit("Cannot find source file "+src)
    DependencyQueue(fn, [pre,dst,fullsrc,wobj,ipath,opts,dashi], [wobj], [fullsrc])

########################################################################
##
## EnqueueIgate
##
########################################################################

def CompileIgateMSVC7(ipath,opts,outd,outc,wobj,src,module,library,files):
    if (OMIT.count("PYTHON")):
        WriteFile(outc,"")
    else:
        cmd = "built/bin/interrogate -srcdir "+src+" -I"+src
        cmd = cmd + ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -longlong __int64 -D_X86_ -DWIN32_VC -D_WIN32'
        cmd = cmd + ' -D"_declspec(param)=" -D_near -D_far -D__near -D__far -D__stdcall'
        optlevel=getoptlevel(opts,OPTIMIZE)
        if (optlevel==1): cmd = cmd + ' '
        if (optlevel==2): cmd = cmd + ' '
        if (optlevel==3): cmd = cmd + ' -DFORCE_INLINING'
        if (optlevel==4): cmd = cmd + ' -DFORCE_INLINING'
        cmd = cmd + ' -Sbuilt/include/parser-inc'
        cmd = cmd + ' -Ibuilt/python/include'
        for pkg in PACKAGES:
            if (PkgSelected(opts,pkg)):
                cmd = cmd + " -Ithirdparty/win-libs-vc7/" + pkg.lower() + "/include"
        cmd = cmd + ' -oc ' + outc + ' -od ' + outd
        cmd = cmd + ' -fnames -string -refcount -assert -python-native'
        for x in ipath: cmd = cmd + ' -I' + x
        building = getbuilding(opts)
        if (building): cmd = cmd + " -DBUILDING_"+building
        if (opts.count("WITHINPANDA")): cmd = cmd + " -DWITHIN_PANDA"
        cmd = cmd + ' -module ' + module + ' -library ' + library
        if ((COMPILER=="MSVC7") and opts.count("DXSDK")): cmd = cmd + ' -I"' + DIRECTXSDK + '/include"'
        for ver in ["MAYA5","MAYA6","MAYA65"]:
            if ((COMPILER=="MSVC7") and opts.count(ver)): cmd = cmd + ' -I"' + MAYASDK[ver] + '/include"'
        for x in files: cmd = cmd + ' ' + x
        oscmd(cmd)
    CompileCxxMSVC7(wobj,outc,ipath,opts)

def CompileIgateLINUXA(ipath,opts,outd,outc,wobj,src,module,library,files):
    if (OMIT.count("PYTHON")):
        WriteFile(outc,"")
    else:
        cmd = 'built/bin/interrogate -srcdir '+src
        cmd = cmd + ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -D__i386__ -D__const=const'
        optlevel = getoptlevel(opts,OPTIMIZE)
        if (optlevel==1): cmd = cmd + ' '
        if (optlevel==2): cmd = cmd + ' '
        if (optlevel==3): cmd = cmd + ' '
        if (optlevel==4): cmd = cmd + ' '
        cmd = cmd + ' -Sbuilt/include/parser-inc -S/usr/include'
        cmd = cmd + ' -Ibuilt/python/include'
        for pkg in PACKAGES:
            if (PkgSelected(opts,pkg)):
                cmd = cmd + " -Ithirdparty/linux-libs-a/" + pkg.lower() + "/include"
        cmd = cmd + ' -oc ' + outc + ' -od ' + outd
        cmd = cmd + ' -fnames -string -refcount -assert -python-native'
        for x in ipath: cmd = cmd + ' -I' + x
        building = getbuilding(opts)
        if (building): cmd = cmd + " -DBUILDING_"+building
        if (opts.count("WITHINPANDA")): cmd = cmd + " -DWITHIN_PANDA"
        cmd = cmd + ' -module ' + module + ' -library ' + library
        if (opts.count("DXSDK")): cmd = cmd + ' -I"' + DIRECTXSDK + '/include"'
        for ver in ["MAYA5","MAYA6","MAYA65"]:
            if (opts.count(ver)): cmd = cmd + ' -I"' + MAYASDK[ver] + '/include"'
        for x in files: cmd = cmd + ' ' + x
        oscmd(cmd)
    CompileCxxLINUXA(wobj,outc,ipath,opts)

def EnqueueIgate(ipath=0, opts=0, outd=0, obj=0, src=0, module=0, library=0, also=0, skip=0):
    if ((ipath==0)|(opts==0)|(outd==0)|(obj==0)|(src==0)|(module==0)|(library==0)|(also==0)|(skip==0)):
        exit("syntax error in EnqueueIgate directive")
    if (COMPILER=="MSVC7"):
        dep = "built/bin/interrogate.exe"
        wobj = "built/tmp/"+obj
        fn = CompileIgateMSVC7
    if (COMPILER=="LINUXA"):
        dep = "built/bin/interrogate"
        wobj = "built/tmp/"+outc[:-4]+".o"
        fn = CompileIgateLINUXA
    if (SLAVEBUILD!=0) and (SLAVEBUILD!=wobj): return
    ALLIN.append(outd)
    outd = 'built/pandac/input/'+outd
    dirlisting = os.listdir(src)
    files = fnmatch.filter(dirlisting,"*.h")
    if (skip=='ALL'): files=[]
    else:
        files.sort()
        for x in skip:
            if (files.count(x)!=0): files.remove(x)
    for x in also: files.append(x)
    ipath = ["built/tmp"] + ipath + ["built/include"]
    dep = [dep, "built/tmp/dtool_have_python.dat"]
    dep = dep + CxxCalcDependenciesAll(xpaths(src+"/",files,""), ipath)
    outc = "built/tmp/"+obj[:-4]+".cxx"
    DependencyQueue(fn, [ipath,opts,outd,outc,wobj,src,module,library,files], [wobj, outd], dep)

########################################################################
##
## EnqueueImod
##
########################################################################

def CompileImodMSVC7(outc, wobj, module, library, ipath, opts, files):
    if (OMIT.count("PYTHON")):
        WriteFile(outc,"")
    else:
        cmd = 'built/bin/interrogate_module '
        cmd = cmd + ' -oc ' + outc + ' -module ' + module + ' -library ' + library + ' -python-native '
        for x in files: cmd = cmd + ' ' + x
        oscmd(cmd)
    CompileCxxMSVC7(wobj,outc,ipath,opts)

def CompileImodLINUXA(outc, wobj, module, library, ipath, opts, files):
    if (OMIT.count("PYTHON")):
        WriteFile(outc,"")
    else:
        cmd = 'built/bin/interrogate_module '
        cmd = cmd + ' -oc ' + outc + ' -module ' + module + ' -library ' + library + ' -python-native '
        for x in files: cmd = cmd + ' ' + x
        oscmd(cmd)
    CompileCxxLINUXA(wobj,outc,ipath,opts)

def EnqueueImod(ipath=0, opts=0, obj=0, module=0, library=0, files=0):
    if ((ipath==0)|(opts==0)|(obj==0)|(module==0)|(library==0)|(files==0)):
        exit("syntax error in EnqueueImod directive")
    if (COMPILER=="MSVC7"):
        wobj = "built/tmp/"+obj[:-4]+".obj"
        fn = CompileImodMSVC7
    if (COMPILER=="LINUXA"):
        wobj = "built/tmp/"+obj[:-4]+".o"
        fn = CompileImodLINUXA
    if (SLAVEBUILD!=0) and (SLAVEBUILD!=wobj): return
    ipath = ["built/tmp"] + ipath + ["built/include"]
    outc = "built/tmp/"+obj[:-4]+".cxx"
    files = xpaths("built/pandac/input/",files,"")
    dep = files + ["built/tmp/dtool_have_python.dat"]
    DependencyQueue(fn, [outc, wobj, module, library, ipath, opts, files], [wobj], dep)

########################################################################
##
## EnqueueLib
##
########################################################################

def CompileLibMSVC7(wlib, wobj, opts):
    cmd = 'link /lib /nologo /OUT:' + wlib
    optlevel = getoptlevel(opts,OPTIMIZE)
    if (optlevel==4): cmd = cmd + " /LTCG "
    for x in wobj: cmd = cmd + ' ' + x
    oscmd(cmd)

def CompileLibLINUXA(wlib, wobj, opts):
    cmd = 'ar cru ' + wlib
    for x in wobj: cmd=cmd + ' ' + x
    oscmd(cmd)

def EnqueueLib(lib=0, obj=[], opts=[]):
    if (lib==0): exit("syntax error in EnqueueLib directive")

    if (COMPILER=="MSVC7"):
        if (lib[-4:]==".ilb"): wlib = "built/tmp/" + lib[:-4] + ".lib"
        else:                  wlib = "built/lib/" + lib[:-4] + ".lib"
        if (SLAVEBUILD!=0) and (SLAVEBUILD!=wlib): return
        wobj = xpaths("built/tmp/",obj,"")
        DependencyQueue(CompileLibMSVC7, [wlib, wobj, opts], [wlib], wobj)

    if (COMPILER=="LINUXA"):
        if (lib[-4:]==".ilb"): wlib = "built/tmp/" + lib[:-4] + ".a"
        else:                  wlib = "built/lib/" + lib[:-4] + ".a"
        if (SLAVEBUILD!=0) and (SLAVEBUILD!=wlib): return
        wobj = []
        for x in obj: wobj.append("built/tmp/" + x[:-4] + ".o")
        DependencyQueue(CompileLibLINUXA, [wlib, wobj, opts], [wlib], wobj)

########################################################################
##
## EnqueueLink
##
########################################################################

def CompileLinkMSVC7(wdll, wlib, wobj, opts, ldef):
    cmd = 'link /nologo /NODEFAULTLIB:LIBCI.LIB /NODEFAULTLIB:MSVCRTD.LIB /DEBUG '
    if (wdll[-4:]!=".exe"): cmd = cmd + " /DLL"
    optlevel = getoptlevel(opts,OPTIMIZE)
    if (optlevel==1): cmd = cmd + " /MAP /MAPINFO:LINES /MAPINFO:EXPORTS"
    if (optlevel==2): cmd = cmd + " /MAP:NUL "
    if (optlevel==3): cmd = cmd + " /MAP:NUL "
    if (optlevel==4): cmd = cmd + " /MAP:NUL /LTCG "
    cmd = cmd + " /FIXED:NO /OPT:REF /STACK:4194304 /INCREMENTAL:NO "
    if (ldef!=0): cmd = cmd + ' /DEF:"' + ldef + '"'
    cmd = cmd + ' /OUT:' + wdll
    if (wlib != 0): cmd = cmd + ' /IMPLIB:' + wlib
    if (OMIT.count("PYTHON")==0): cmd = cmd + ' /LIBPATH:built/python/libs '
    for x in wobj: cmd = cmd + ' ' + x
    if (wdll[-4:]==".exe"): cmd = cmd + ' panda/src/configfiles/pandaIcon.obj'
    if (opts.count("D3D8") or opts.count("D3D9") or opts.count("DXDRAW") or opts.count("DXSOUND") or opts.count("DXGUID")):
        cmd = cmd + ' /LIBPATH:"' + DIRECTXSDK + '/lib/x86"'
        cmd = cmd + ' /LIBPATH:"' + DIRECTXSDK + '/lib"'
    if (opts.count("D3D8")):        cmd = cmd + ' d3d8.lib d3dx8.lib dxerr8.lib'
    if (opts.count("D3D9")):        cmd = cmd + ' d3d9.lib d3dx9.lib dxerr9.lib'
    if (opts.count("DXDRAW")):      cmd = cmd + ' ddraw.lib'
    if (opts.count("DXSOUND")):     cmd = cmd + ' dsound.lib'
    if (opts.count("DXGUID")):      cmd = cmd + ' dxguid.lib'
    if (opts.count("WINSOCK")):     cmd = cmd + " wsock32.lib"
    if (opts.count("WINSOCK2")):    cmd = cmd + " wsock32.lib ws2_32.lib"
    if (opts.count("WINCOMCTL")):   cmd = cmd + ' comctl32.lib'
    if (opts.count("WINCOMDLG")):   cmd = cmd + ' comdlg32.lib'
    if (opts.count("WINUSER")):     cmd = cmd + " user32.lib"
    if (opts.count("WINMM")):       cmd = cmd + " winmm.lib"
    if (opts.count("WINIMM")):      cmd = cmd + " imm32.lib"
    if (opts.count("WINKERNEL")):   cmd = cmd + " kernel32.lib"
    if (opts.count("WINOLDNAMES")): cmd = cmd + " oldnames.lib"
    if (opts.count("WINGDI")):      cmd = cmd + " gdi32.lib"
    if (opts.count("ADVAPI")):      cmd = cmd + " advapi32.lib"
    if (opts.count("GLUT")):        cmd = cmd + " opengl32.lib glu32.lib"
    if (PkgSelected(opts,"ZLIB")):     cmd = cmd + ' thirdparty/win-libs-vc7/zlib/lib/libpandazlib1.lib'
    if (PkgSelected(opts,"PNG")):      cmd = cmd + ' thirdparty/win-libs-vc7/png/lib/libpandapng13.lib'
    if (PkgSelected(opts,"JPEG")):     cmd = cmd + ' thirdparty/win-libs-vc7/jpeg/lib/libpandajpeg.lib'
    if (PkgSelected(opts,"TIFF")):     cmd = cmd + ' thirdparty/win-libs-vc7/tiff/lib/libpandatiff.lib'
    if (PkgSelected(opts,"VRPN")):
        cmd = cmd + ' thirdparty/win-libs-vc7/vrpn/lib/vrpn.lib'
        cmd = cmd + ' thirdparty/win-libs-vc7/vrpn/lib/quat.lib'
    if (PkgSelected(opts,"FMOD")):
        cmd = cmd + ' thirdparty/win-libs-vc7/fmod/lib/fmod.lib'
    if (PkgSelected(opts,"MILES")):
        cmd = cmd + ' thirdparty/win-libs-vc7/miles/lib/mss32.lib'
    if (PkgSelected(opts,"NVIDIACG")):
        if (opts.count("CGGL")):
            cmd = cmd + ' thirdparty/win-libs-vc7/nvidiacg/lib/cgGL.lib'
        cmd = cmd + ' thirdparty/win-libs-vc7/nvidiacg/lib/cg.lib'
    if (PkgSelected(opts,"HELIX")):
        cmd = cmd + ' thirdparty/win-libs-vc7/helix/lib/runtlib.lib'
        cmd = cmd + ' thirdparty/win-libs-vc7/helix/lib/syslib.lib'
        cmd = cmd + ' thirdparty/win-libs-vc7/helix/lib/contlib.lib'
        cmd = cmd + ' thirdparty/win-libs-vc7/helix/lib/debuglib.lib'
        cmd = cmd + ' thirdparty/win-libs-vc7/helix/lib/utillib.lib'
        cmd = cmd + ' thirdparty/win-libs-vc7/helix/lib/stlport_vc7.lib'
    if (PkgSelected(opts,"NSPR")):
        cmd = cmd + ' thirdparty/win-libs-vc7/nspr/lib/nspr4.lib'
    if (PkgSelected(opts,"OPENSSL")):
        cmd = cmd + ' thirdparty/win-libs-vc7/openssl/lib/libpandassl.lib'
        cmd = cmd + ' thirdparty/win-libs-vc7/openssl/lib/libpandaeay.lib'
    if (PkgSelected(opts,"FREETYPE")):
        cmd = cmd + ' thirdparty/win-libs-vc7/freetype/lib/freetype.lib'
    if (PkgSelected(opts,"FFTW")):
        cmd = cmd + ' thirdparty/win-libs-vc7/fftw/lib/rfftw.lib'
        cmd = cmd + ' thirdparty/win-libs-vc7/fftw/lib/fftw.lib'
    for maya in ["MAYA5","MAYA6","MAYA65"]:
        if (PkgSelected(opts,maya)):
            cmd = cmd + ' "' + MAYASDK[maya] +  '/lib/Foundation.lib"'
            cmd = cmd + ' "' + MAYASDK[maya] +  '/lib/OpenMaya.lib"'
            cmd = cmd + ' "' + MAYASDK[maya] +  '/lib/OpenMayaAnim.lib"'
    for max in ["MAX5","MAX6","MAX7"]:
        if PkgSelected(opts,max):
            cmd = cmd + ' "' + MAXSDK[max] +  '/lib/core.lib"'
            cmd = cmd + ' "' + MAXSDK[max] +  '/lib/edmodel.lib"'
            cmd = cmd + ' "' + MAXSDK[max] +  '/lib/gfx.lib"'
            cmd = cmd + ' "' + MAXSDK[max] +  '/lib/geom.lib"'
            cmd = cmd + ' "' + MAXSDK[max] +  '/lib/mesh.lib"'
            cmd = cmd + ' "' + MAXSDK[max] +  '/lib/maxutil.lib"'
            cmd = cmd + ' "' + MAXSDK[max] +  '/lib/paramblk2.lib"'
    oscmd(cmd)

def CompileLinkLINUXA(wdll, wobj, opts, ldef):
    if (dll[-4:]==".exe"): cmd = 'g++ -o ' + wdll + ' -Lbuilt/lib -L/usr/X11R6/lib'
    else:                  cmd = 'g++ -shared -o ' + wdll + ' -Lbuilt/lib -L/usr/X11R6/lib'
    for x in obj:
        suffix = x[-4:]
        if   (suffix==".obj"): cmd = cmd + ' built/tmp/' + x[:-4] + '.o'
        elif (suffix==".dll"): cmd = cmd + ' -l' + x[3:-4]
        elif (suffix==".lib"): cmd = cmd + ' built/lib/' + x[:-4] + '.a'
        elif (suffix==".ilb"): cmd = cmd + ' built/tmp/' + x[:-4] + '.a'
    if (PkgSelected(opts,"FMOD")):     cmd = cmd + ' -Lthirdparty/linux-libs-a/fmod/lib -lfmod-3.74'
    if (PkgSelected(opts,"NVIDIACG")):
        cmd = cmd + ' -Lthirdparty/nvidiacg/lib '
        if (opts.count("CGGL")): cmd = cmd + " -lCgGL"
        cmd = cmd + " -lCg"
    if (PkgSelected(opts,"NSPR")):     cmd = cmd + ' -Lthirdparty/linux-libs-a/nspr/lib -lpandanspr4'
    if (PkgSelected(opts,"ZLIB")):     cmd = cmd + " -lz"
    if (PkgSelected(opts,"PNG")):      cmd = cmd + " -lpng"
    if (PkgSelected(opts,"JPEG")):     cmd = cmd + " -ljpeg"
    if (PkgSelected(opts,"TIFF")):     cmd = cmd + " -ltiff"
    if (PkgSelected(opts,"OPENSSL")):  cmd = cmd + " -lssl"
    if (PkgSelected(opts,"FREETYPE")): cmd = cmd + " -lfreetype"
    if (PkgSelected(opts,"VRPN")):     cmd = cmd + ' -Lthirdparty/linux-libs-a/vrpn/lib -lvrpn -lquat'
    if (PkgSelected(opts,"FFTW")):     cmd = cmd + ' -Lthirdparty/linux-libs-a/fftw/lib -lrfftw -lfftw'
    if (opts.count("GLUT")):           cmd = cmd + " -lGL -lGLU"
    oscmd(cmd)

def EnqueueLink(dll=0, obj=[], opts=[], xdep=[], ldef=0):
    if (dll==0): exit("syntax error in EnqueueLink directive")

    if (COMPILER=="MSVC7"):
        wobj = []
        for x in obj:
            suffix = x[-4:]
            if   (suffix==".obj"): wobj.append("built/tmp/"+x)
            elif (suffix==".dll"): wobj.append("built/lib/"+x[:-4]+".lib")
            elif (suffix==".lib"): wobj.append("built/lib/"+x)
            elif (suffix==".ilb"): wobj.append("built/tmp/"+x[:-4]+".lib")
            else: exit("unknown suffix in object list.")
        if (dll[-4:]==".exe"):
            wdll = "built/bin/"+dll
            if (SLAVEBUILD!=0) and (SLAVEBUILD!=wdll): return
            DependencyQueue(CompileLinkMSVC7, [wdll, 0,    wobj, opts, ldef], [wdll], wobj)
        elif (dll[-4:]==".dll"):
            wdll = "built/bin/"+dll
            wlib = "built/lib/"+dll[:-4]+".lib"
            if (SLAVEBUILD!=0) and (SLAVEBUILD!=wdll): return
            DependencyQueue(CompileLinkMSVC7, [wdll, wlib, wobj, opts, ldef], [wdll, wlib], wobj)
        else:
            wdll = "built/plugins/"+dll
            if (SLAVEBUILD!=0) and (SLAVEBUILD!=wdll): return
            DependencyQueue(CompileLinkMSVC7, [wdll, 0,    wobj, opts, ldef], [wdll], wobj)

    if (COMPILER=="LINUXA"):
        if (dll[-4:]==".exe"): wdll = "built/bin/"+dll[:-4]
        else: wdll = "built/lib/"+dll[:-4]+".so"
        wobj = []
        for x in obj:
            suffix = x[-4:]
            if   (suffix==".obj"): wobj.append("built/tmp/"+x[:-4]+".o")
            elif (suffix==".dll"): wobj.append("built/lib/"+x[:-4]+".so")
            elif (suffix==".lib"): wobj.append("built/lib/"+x[:-4]+".a")
            elif (suffix==".ilb"): wobj.append("built/tmp/"+x[:-4]+".a")
            else: exit("unknown suffix in object list.")
        if (SLAVEBUILD!=0) and (SLAVEBUILD!=wdll): return
        DependencyQueue(CompileLinkLINUXA, [wdll, wobj, opts, ldef], [wdll], wobj)


##########################################################################################
#
# EnqueueBam
#
##########################################################################################

def CompileBam(preconv, bam, egg):
    if (egg[-4:] == ".flt"):
        oscmd("built/bin/flt2egg -pr " + preconv + " -o built/tmp/tmp.egg" + " " + egg)
        oscmd("built/bin/egg2bam -o " + bam + " built/tmp/tmp.egg")
    else:
        oscmd("built/bin/egg2bam -pr " + preconv + " -o " + bam + " " + egg)

def EnqueueBam(preconv, bam, egg):
    if (sys.platform == "win32"):
        flt2egg = "built/bin/flt2egg.exe"
        egg2bam = "built/bin/egg2bam.exe"
    else:
        flt2egg = "built/bin/flt2egg"
        egg2bam = "built/bin/egg2bam"
    DependencyQueue(CompileBam, [preconv, bam, egg], [bam], [egg, flt2egg, egg2bam])

##########################################################################################
#
# If the 'make depend' process discovers an 'include'
# directive that includes one of the following files,
# the specified file is not added as a dependency,
# nor is it traversed.
#
##########################################################################################

CxxIgnoreHeader["Python.h"] = 1
CxxIgnoreHeader["Python/Python.h"] = 1
CxxIgnoreHeader["Cg/cg.h"] = 1
CxxIgnoreHeader["Cg/cgGL.h"] = 1
CxxIgnoreHeader["alloc.h"] = 1
CxxIgnoreHeader["ctype.h"] = 1
CxxIgnoreHeader["stdlib.h"] = 1
CxxIgnoreHeader["ipc_thread.h"] = 1
CxxIgnoreHeader["platform/symbian/symbian_print.h"] = 1
CxxIgnoreHeader["hxtypes.h"] = 1
CxxIgnoreHeader["hxcom.h"] = 1
CxxIgnoreHeader["hxiids.h"] = 1
CxxIgnoreHeader["hxpiids.h"] = 1
CxxIgnoreHeader["dsound.h"] = 1
CxxIgnoreHeader["hlxosstr.h"] = 1
CxxIgnoreHeader["ddraw.h"] = 1
CxxIgnoreHeader["mss.h"] = 1
CxxIgnoreHeader["MacSocket.h"] = 1
CxxIgnoreHeader["textureTransition.h"] = 1
CxxIgnoreHeader["transformTransition.h"] = 1
CxxIgnoreHeader["billboardTransition.h"] = 1
CxxIgnoreHeader["transformTransition.h"] = 1
CxxIgnoreHeader["transparencyTransition.h"] = 1
CxxIgnoreHeader["allTransitionsWrapper.h"] = 1
CxxIgnoreHeader["allTransitionsWrapper.h"] = 1
CxxIgnoreHeader["namedNode.h"] = 1
CxxIgnoreHeader["renderRelation.h"] = 1
CxxIgnoreHeader["renderTraverser.h"] = 1
CxxIgnoreHeader["get_rel_pos.h"] = 1

# Ignore Windows headers.
CxxIgnoreHeader["windows.h"] = 1
CxxIgnoreHeader["windef.h"] = 1
CxxIgnoreHeader["afxres.h"] = 1

# Ignore MAX headers
CxxIgnoreHeader["Max.h"] = 1
CxxIgnoreHeader["iparamb2.h"] = 1
CxxIgnoreHeader["iparamm2.h"] = 1
CxxIgnoreHeader["istdplug.h"] = 1
CxxIgnoreHeader["iskin.h"] = 1
CxxIgnoreHeader["stdmat.h"] = 1
CxxIgnoreHeader["phyexp.h"] = 1
CxxIgnoreHeader["bipexp.h"] = 1
CxxIgnoreHeader["modstack.h"] = 1
CxxIgnoreHeader["decomp.h"] = 1
CxxIgnoreHeader["shape.h"] = 1
CxxIgnoreHeader["simpobj.h"] = 1
CxxIgnoreHeader["surf_api.h"] = 1

# OpenSSL headers
CxxIgnoreHeader["openssl/evp.h"] = 1
CxxIgnoreHeader["openssl/rand.h"] = 1
CxxIgnoreHeader["openssl/md5.h"] = 1
CxxIgnoreHeader["openssl/err.h"] = 1
CxxIgnoreHeader["openssl/ssl.h"] = 1
CxxIgnoreHeader["openssl/pem.h"] = 1
CxxIgnoreHeader["openssl/rsa.h"] = 1
CxxIgnoreHeader["openssl/bio.h"] = 1
CxxIgnoreHeader["openssl/x509.h"] = 1

# STD headers
CxxIgnoreHeader["map"] = 1
CxxIgnoreHeader["vector"] = 1
CxxIgnoreHeader["set"] = 1
CxxIgnoreHeader["algorithm"] = 1

##########################################################################################
#
# Create the directory tree
#
##########################################################################################

MakeDirectory("built")
MakeDirectory("built/bin")
MakeDirectory("built/lib")
MakeDirectory("built/etc")
MakeDirectory("built/plugins")
MakeDirectory("built/include")
MakeDirectory("built/include/parser-inc")
MakeDirectory("built/include/parser-inc/openssl")
MakeDirectory("built/include/parser-inc/Cg")
MakeDirectory("built/include/openssl")
MakeDirectory("built/tmp")
MakeDirectory("built/models")
MakeDirectory("built/models/audio")
MakeDirectory("built/models/audio/sfx")
MakeDirectory("built/models/icons")
MakeDirectory("built/models/maps")
MakeDirectory("built/models/misc")
MakeDirectory("built/models/gui")

if (OMIT.count("PYTHON")==0):
    MakeDirectory("built/direct")
    MakeDirectory("built/pandac")
    MakeDirectory("built/pandac/input")

########################################################################
#
# If using the Master-Slave build system,
# receive command-line arguments from master to slave
#
########################################################################

if (SLAVEBUILD!=0):
    try:
        slavectrl=open("built/tmp/slave-control","rb")
        OMIT     = cPickle.load(slavectrl)
        OPTIMIZE = cPickle.load(slavectrl)
        slavectrl.close()
    except: exit("Cannot read from built/tmp/slave-control")

if (SLAVEFILE!=0):
    try:
        slavectrl=open("built/tmp/slave-control","wb")
        cPickle.dump(OMIT,     slavectrl, 1)
        cPickle.dump(OPTIMIZE, slavectrl, 1)
        slavectrl.close()
    except: exit("Cannot write to built/tmp/slave-control")

##########################################################################################
#
# Generate dtool_config.h and dtool_have_xxx.dat
#
##########################################################################################

DTOOLDEFAULTS=[
    #_Variable_________________________Windows___________________Unix__________
    ("HAVE_PYTHON",                    '1',                      '1'),
    ("PYTHON_FRAMEWORK",               'UNDEF',                  'UNDEF'),
    ("COMPILE_IN_DEFAULT_FONT",        '1',                      '1'),
    ("HAVE_MAYA",                      '1',                      '1'),
    ("MAYA_PRE_5_0",                   'UNDEF',                  'UNDEF'),
    ("HAVE_SOFTIMAGE",                 'UNDEF',                  'UNDEF'),
    ("SSL_097",                        'UNDEF',                  'UNDEF'),
    ("REPORT_OPENSSL_ERRORS",          '1',                      '1'),
    ("HAVE_GL",                        '1',                      '1'),
    ("HAVE_MESA",                      'UNDEF',                  'UNDEF'),
    ("MESA_MGL",                       'UNDEF',                  'UNDEF'),
    ("HAVE_SGIGL",                     'UNDEF',                  'UNDEF'),
    ("HAVE_GLX",                       'UNDEF',                  '1'),
    ("HAVE_WGL",                       '1',                      'UNDEF'),
    ("HAVE_DX",                        '1',                      'UNDEF'),
    ("HAVE_CHROMIUM",                  'UNDEF',                  'UNDEF'),
    ("HAVE_THREADS",                   'UNDEF',                  'UNDEF'),
    ("HAVE_AUDIO",                     '1',                      '1'),
    ("NOTIFY_DEBUG",                   'UNDEF',                  'UNDEF'),
    ("DO_PSTATS",                      'UNDEF',                  'UNDEF'),
    ("DO_COLLISION_RECORDING",         'UNDEF',                  'UNDEF'),
    ("TRACK_IN_INTERPRETER",           'UNDEF',                  'UNDEF'),
    ("DO_MEMORY_USAGE",                'UNDEF',                  'UNDEF'),
    ("DO_PIPELINING",                  'UNDEF',                  'UNDEF'),
    ("EXPORT_TEMPLATES",               'yes',                    'yes'),
    ("LINK_IN_GL",                     'UNDEF',                  'UNDEF'),
    ("LINK_IN_PHYSICS",                'UNDEF',                  'UNDEF'),
    ("DEFAULT_PATHSEP",                '";"',                    '":"'),
    ("DEFAULT_PRC_DIR",                '"<auto>etc"',            '"<auto>etc"'),
    ("PRC_DIR_ENVVARS",                '"PANDA_PRC_DIR"',        '"PANDA_PRC_DIR"'),
    ("PRC_PATH_ENVVARS",               '"PANDA_PRC_PATH"',       '"PANDA_PRC_PATH"'),
    ("PRC_PATTERNS",                   '"*.prc"',                '"*.prc"'),
    ("PRC_EXECUTABLE_PATTERNS",        '""',                     '""'),
    ("PRC_EXECUTABLE_ARGS_ENVVAR",     '"PANDA_PRC_XARGS"',      '"PANDA_PRC_XARGS"'),
    ("PRC_PUBLIC_KEYS_FILENAME",       '""',                     '""'),
    ("PRC_RESPECT_TRUST_LEVEL",        'UNDEF',                  'UNDEF'),
    ("PRC_SAVE_DESCRIPTIONS",          '1',                      '1'),
    ("WORDS_BIGENDIAN",                'UNDEF',                  'UNDEF'),
    ("HAVE_NAMESPACE",                 '1',                      '1'),
    ("HAVE_OPEN_MASK",                 'UNDEF',                  'UNDEF'),
    ("HAVE_WCHAR_T",                   '1',                      '1'),
    ("HAVE_WSTRING",                   '1',                      '1'),
    ("HAVE_TYPENAME",                  '1',                      '1'),
    ("SIMPLE_STRUCT_POINTERS",         '1',                      'UNDEF'),
    ("HAVE_DINKUM",                    'UNDEF',                  'UNDEF'),
    ("HAVE_STL_HASH",                  'UNDEF',                  'UNDEF'),
    ("HAVE_GETTIMEOFDAY",              'UNDEF',                  '1'),
    ("GETTIMEOFDAY_ONE_PARAM",         'UNDEF',                  'UNDEF'),
    ("HAVE_GETOPT",                    'UNDEF',                  '1'),
    ("HAVE_GETOPT_LONG_ONLY",          'UNDEF',                  '1'),
    ("HAVE_GETOPT_H",                  'UNDEF',                  '1'),
    ("IOCTL_TERMINAL_WIDTH",           'UNDEF',                  '1'),
    ("HAVE_STREAMSIZE",                '1',                      '1'),
    ("HAVE_IOS_TYPEDEFS",              '1',                      '1'),
    ("HAVE_IOS_BINARY",                '1',                      '1'),
    ("STATIC_INIT_GETENV",             '1',                      'UNDEF'),
    ("HAVE_PROC_SELF_EXE",             'UNDEF',                  '1'),
    ("HAVE_PROC_SELF_MAPS",            'UNDEF',                  '1'),
    ("HAVE_PROC_SELF_ENVIRON",         'UNDEF',                  '1'),
    ("HAVE_PROC_SELF_CMDLINE",         'UNDEF',                  '1'),
    ("HAVE_GLOBAL_ARGV",               '1',                      'UNDEF'),
    ("PROTOTYPE_GLOBAL_ARGV",          'UNDEF',                  'UNDEF'),
    ("GLOBAL_ARGV",                    '__argv',                 'UNDEF'),
    ("GLOBAL_ARGC",                    '__argc',                 'UNDEF'),
    ("HAVE_IO_H",                      '1',                      'UNDEF'),
    ("HAVE_IOSTREAM",                  '1',                      '1'),
    ("HAVE_MALLOC_H",                  '1',                      '1'),
    ("HAVE_SYS_MALLOC_H",              'UNDEF',                  'UNDEF'),
    ("HAVE_ALLOCA_H",                  'UNDEF',                  '1'),
    ("HAVE_LOCALE_H",                  'UNDEF',                  '1'),
    ("HAVE_MINMAX_H",                  '1',                      'UNDEF'),
    ("HAVE_SSTREAM",                   '1',                      '1'),
    ("HAVE_NEW",                       '1',                      '1'),
    ("HAVE_SYS_TYPES_H",               '1',                      '1'),
    ("HAVE_SYS_TIME_H",                'UNDEF',                  '1'),
    ("HAVE_UNISTD_H",                  'UNDEF',                  '1'),
    ("HAVE_UTIME_H",                   'UNDEF',                  '1'),
    ("HAVE_GLOB_H",                    'UNDEF',                  '1'),
    ("HAVE_DIRENT_H",                  'UNDEF',                  '1'),
    ("HAVE_SYS_SOUNDCARD_H",           'UNDEF',                  '1'),
    ("HAVE_RTTI",                      '1',                      '1'),
    ("GLOBAL_OPERATOR_NEW_EXCEPTIONS", 'UNDEF',                  '1'),
    ("OLD_STYLE_ALLOCATOR",            'UNDEF',                  'UNDEF'),
    ("GNU_STYLE_ALLOCATOR",            'UNDEF',                  '1'),
    ("VC6_STYLE_ALLOCATOR",            'UNDEF',                  'UNDEF'),
    ("MODERN_STYLE_ALLOCATOR",         'UNDEF',                  'UNDEF'),
    ("NO_STYLE_ALLOCATOR",             '1',                      'UNDEF'),
    ("HAVE_ZLIB",                      'UNDEF',                  'UNDEF'),
    ("HAVE_PNG",                       'UNDEF',                  'UNDEF'),
    ("HAVE_JPEG",                      'UNDEF',                  'UNDEF'),
    ("HAVE_TIFF",                      'UNDEF',                  'UNDEF'),
    ("HAVE_VRPN",                      'UNDEF',                  'UNDEF'),
    ("HAVE_FMOD",                      'UNDEF',                  'UNDEF'),
    ("HAVE_NVIDIACG",                  'UNDEF',                  'UNDEF'),
    ("HAVE_NSPR",                      'UNDEF',                  'UNDEF'),
    ("HAVE_FREETYPE",                  'UNDEF',                  'UNDEF'),
    ("HAVE_FFTW",                      'UNDEF',                  'UNDEF'),
    ("HAVE_OPENSSL",                   'UNDEF',                  'UNDEF'),
    ("HAVE_NET",                       'UNDEF',                  'UNDEF'),
    ("HAVE_CG",                        'UNDEF',                  'UNDEF'),
    ("HAVE_CGGL",                      'UNDEF',                  'UNDEF'),
    ]

def CalculateDtoolConfig():
    dtoolconfig={}
    if (sys.platform == "win32"):
        for key,win,unix in DTOOLDEFAULTS:
            dtoolconfig[key] = win
    else:
        for key,win,unix in DTOOLDEFAULTS:
            dtoolconfig[key] = unix
    
    for x in PACKAGES:
        if (OMIT.count(x)==0):
            if (dtoolconfig.has_key("HAVE_"+x)):
                dtoolconfig["HAVE_"+x] = '1'
    
    dtoolconfig["HAVE_NET"] = dtoolconfig["HAVE_NSPR"]
    
    if (OMIT.count("NVIDIACG")==0):
        dtoolconfig["HAVE_CG"] = '1'
        dtoolconfig["HAVE_CGGL"] = '1'
    
    if (OPTIMIZE <= 3):
        if (dtoolconfig["HAVE_NET"] != 'UNDEF'):
            dtoolconfig["DO_PSTATS"] = '1'
    
    if (OPTIMIZE <= 3):
        dtoolconfig["DO_COLLISION_RECORDING"] = '1'
    
    #if (OPTIMIZE <= 2):
    #    dtoolconfig["TRACK_IN_INTERPRETER"] = '1'
    
    if (OPTIMIZE <= 3):
        dtoolconfig["DO_MEMORY_USAGE"] = '1'
    
    #if (OPTIMIZE <= 1):
    #    dtoolconfig["DO_PIPELINING"] = '1'
    
    if (OPTIMIZE <= 3):
        dtoolconfig["NOTIFY_DEBUG"] = '1'
    
    conf = "/* dtool_config.h.  Generated automatically by makepanda.py */\n"
    for key,win,unix in DTOOLDEFAULTS:
        val = dtoolconfig[key]
        if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
        else:                conf = conf + "#define " + key + " " + val + "\n"
    return conf

if (SLAVEBUILD==0):
    for x in PACKAGES:
        if (OMIT.count(x)): ConditionalWriteFile('built/tmp/dtool_have_'+x.lower()+'.dat',"0\n")
        else:               ConditionalWriteFile('built/tmp/dtool_have_'+x.lower()+'.dat',"1\n")
    ConditionalWriteFile('built/include/dtool_config.h', CalculateDtoolConfig())

##########################################################################################
#
# Generate pandaVersion.h, pythonversion, null.cxx
#
##########################################################################################

PANDAVERSION_H="""
#define PANDA_MAJOR_VERSION VERSION1
#define PANDA_MINOR_VERSION VERSION2
#define PANDA_SEQUENCE_VERSION VERSION2
#undef  PANDA_OFFICIAL_VERSION
#define PANDA_VERSION NVERSION
#define PANDA_VERSION_STR "VERSION1.VERSION2.VERSION3"
#define PANDA_DISTRIBUTOR "makepanda"
"""

CHECKPANDAVERSION_CXX="""
# include "dtoolbase.h"
EXPCL_DTOOL int panda_version_VERSION1_VERSION2_VERSION3 = 0;
"""

CHECKPANDAVERSION_H="""
# include "dtoolbase.h"
extern EXPCL_DTOOL int panda_version_VERSION1_VERSION2_VERSION3;
# ifndef WIN32
/* For Windows, exporting the symbol from the DLL is sufficient; the
      DLL will not load unless all expected public symbols are defined.
      Other systems may not mind if the symbol is absent unless we
      explictly write code that references it. */
static int check_panda_version = panda_version_VERSION1_VERSION2_VERSION3;
# endif
"""

def CreatePandaVersionFiles():
    version1=int(VERSION.split(".")[0])
    version2=int(VERSION.split(".")[1])
    version3=int(VERSION.split(".")[2])
    nversion=version1*1000000+version2*1000+version3
    
    pandaversion_h = PANDAVERSION_H.replace("VERSION1",str(version1))
    pandaversion_h = pandaversion_h.replace("VERSION2",str(version2))
    pandaversion_h = pandaversion_h.replace("VERSION3",str(version3))
    pandaversion_h = pandaversion_h.replace("NVERSION",str(nversion))
    
    checkpandaversion_cxx = CHECKPANDAVERSION_CXX.replace("VERSION1",str(version1))
    checkpandaversion_cxx = checkpandaversion_cxx.replace("VERSION2",str(version2))
    checkpandaversion_cxx = checkpandaversion_cxx.replace("VERSION3",str(version3))
    checkpandaversion_cxx = checkpandaversion_cxx.replace("NVERSION",str(nversion))
    
    checkpandaversion_h = CHECKPANDAVERSION_H.replace("VERSION1",str(version1))
    checkpandaversion_h = checkpandaversion_h.replace("VERSION2",str(version2))
    checkpandaversion_h = checkpandaversion_h.replace("VERSION3",str(version3))
    checkpandaversion_h = checkpandaversion_h.replace("NVERSION",str(nversion))

    ConditionalWriteFile('built/include/pandaVersion.h',        pandaversion_h)
    ConditionalWriteFile('built/include/checkPandaVersion.cxx', checkpandaversion_cxx)
    ConditionalWriteFile('built/include/checkPandaVersion.h',   checkpandaversion_h)
    if (OMIT.count("PYTHON")==0):
        ConditionalWriteFile("built/tmp/pythonversion", os.path.basename(PYTHONSDK))
    ConditionalWriteFile("built/tmp/null.cxx","")


if (SLAVEBUILD==0): CreatePandaVersionFiles()

##########################################################################################
#
# Generate direct/__init__.py
#
##########################################################################################

DIRECTINIT="""
import os,sys
srcdir1 = os.path.join(__path__[0], 'src')
srcdir2 = os.path.join(__path__[0], '..', '..', 'direct', 'src')
if    (os.path.isdir(srcdir1)): __path__[0] = srcdir1
elif  (os.path.isdir(srcdir2)): __path__[0] = srcdir2
else: exit("Cannot find the 'direct' tree")
"""

if (SLAVEBUILD==0) and (OMIT.count("PYTHON")==0):
    ConditionalWriteFile('built/direct/__init__.py', DIRECTINIT)

##########################################################################################
#
# Generate the PRC files into the ETC directory.
#
##########################################################################################

if (SLAVEBUILD==0):
    confautoprc=ReadFile("makepanda/confauto.in")
    if (os.path.isfile("makepanda/myconfig.in")):
      configprc=ReadFile("makepanda/myconfig.in")
    else:
      configprc=ReadFile("makepanda/config.in")
    
    if (sys.platform != "win32"):
        confautoprc = confautoprc.replace("aux-display pandadx9","")
        confautoprc = confautoprc.replace("aux-display pandadx8","")
        confautoprc = confautoprc.replace("aux-display pandadx7","")
    
    ConditionalWriteFile("built/etc/Confauto.prc", confautoprc)
    ConditionalWriteFile("built/etc/Config.prc", configprc)

##########################################################################################
#
# Copy the precompiled binaries and DLLs into the build.
#
##########################################################################################

if (SLAVEBUILD==0):
    for pkg in (PACKAGES + ["extras"]):
        if (OMIT.count(pkg)==0):
            if (COMPILER == "MSVC7"):
                if (os.path.exists("thirdparty/win-libs-vc7/"+pkg.lower()+"/bin")):
                    CopyAllFiles("built/bin/","thirdparty/win-libs-vc7/"+pkg.lower()+"/bin/")
            if (COMPILER == "LINUXA"):
                if (os.path.exists("thirdparty/linux-libs-a/"+pkg.lower()+"/lib")):
                    CopyAllFiles("built/lib/","thirdparty/linux-libs-a/"+pkg.lower()+"/lib/")
    if (sys.platform == "win32"):
        CopyFile('built/bin/', 'thirdparty/win-python/python24.dll')
        if (OMIT.count("PYTHON")==0):
            CopyTree('built/python', 'thirdparty/win-python')

########################################################################
##
## Copy various stuff into the build.
##
########################################################################

if (SLAVEBUILD==0):
    CopyFile("built/", "doc/LICENSE")
    CopyFile("built/", "doc/ReleaseNotes")
    CopyAllFiles("built/plugins/",  "pandatool/src/scripts/", ".mel")
    CopyAllFiles("built/plugins/",  "pandatool/src/scripts/", ".ms")
    if (OMIT.count("PYTHON")==0):
        CopyTree('built/Pmw',         'thirdparty/Pmw')
        CopyTree('built/epydoc',      'thirdparty/epydoc')
        CopyTree('built/SceneEditor', 'SceneEditor')
    ConditionalWriteFile('built/include/ctl3d.h', '/* dummy file to make MAX happy */')

########################################################################
#
# Copy header files to the PREFIX/include/parser-inc directory.
#
########################################################################

if (SLAVEBUILD==0):
    CopyAllFiles('built/include/parser-inc/','dtool/src/parser-inc/')
    CopyAllFiles('built/include/parser-inc/openssl/','dtool/src/parser-inc/')
    CopyFile('built/include/parser-inc/Cg/','dtool/src/parser-inc/cg.h')
    CopyFile('built/include/parser-inc/Cg/','dtool/src/parser-inc/cgGL.h')

########################################################################
#
# This section contains a list of all the files that need to be compiled.
#
########################################################################

if (SLAVEBUILD==0):
    print "Generating dependencies..."
    sys.stdout.flush()

#
# DIRECTORY: dtool/src/dtoolbase/
#

CopyAllHeaders('dtool/src/dtoolbase')
IPATH=['dtool/src/dtoolbase']
OPTS=['BUILDING_DTOOL', 'NSPR', 'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='indent.cxx',    obj='dtoolbase_indent.obj')
if (sys.platform == "win32"):
    OPTS.append("PTMALLOC2")
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='ptmalloc2_smp.c', obj='dtoolbase_allocator.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dtoolbase.cxx', obj='dtoolbase_dtoolbase.obj')
else:
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='null.cxx', obj='dtoolbase_allocator.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dtoolbase.cxx', obj='dtoolbase_dtoolbase.obj')


#
# DIRECTORY: dtool/src/dtoolutil/
#

CopyAllHeaders('dtool/src/dtoolutil', skip=["pandaVersion.h", "checkPandaVersion.h"])
IPATH=['dtool/src/dtoolutil']
OPTS=['BUILDING_DTOOL', 'NSPR', 'OPT3']
CopyFile('built/include/','dtool/src/dtoolutil/vector_src.cxx')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='gnu_getopt.c',             obj='dtoolutil_gnu_getopt.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='gnu_getopt1.c',            obj='dtoolutil_gnu_getopt1.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='dtoolutil_composite.cxx',  obj='dtoolutil_composite.obj')

#
# DIRECTORY: dtool/metalibs/dtool/
#

CopyAllHeaders('dtool/metalibs/dtool')
IPATH=['dtool/metalibs/dtool']
OPTS=['BUILDING_DTOOL', 'NSPR', 'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='dtool.cxx', obj='dtool_dtool.obj')
EnqueueLink(opts=['ADVAPI', 'NSPR', 'OPT3'], dll='libdtool.dll', obj=[
             'dtool_dtool.obj',
             'dtoolutil_gnu_getopt.obj',
             'dtoolutil_gnu_getopt1.obj',
             'dtoolutil_composite.obj',
             'dtoolbase_dtoolbase.obj',
             'dtoolbase_allocator.obj',
             'dtoolbase_indent.obj',
])

#
# DIRECTORY: dtool/src/cppparser/
#

CopyAllHeaders('dtool/src/cppparser', skip="ALL")
IPATH=['dtool/src/cppparser']
OPTS=['NSPR','OPT3']
EnqueueBison(ipath=IPATH, opts=OPTS, pre='cppyy', src='cppBison.yxx', dsth='cppBison.h', obj='cppParser_cppBison.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='cppParser_composite.cxx', obj='cppParser_composite.obj')
EnqueueLib(lib='libcppParser.ilb', obj=[
             'cppParser_composite.obj',
             'cppParser_cppBison.obj',
])

#
# DIRECTORY: dtool/src/prc/
#

CopyAllHeaders('dtool/src/prc')
IPATH=['dtool/src/prc']
OPTS=['BUILDING_DTOOLCONFIG', 'OPENSSL', 'NSPR', 'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='prc_composite.cxx', obj='prc_composite.obj')

#
# DIRECTORY: dtool/src/dconfig/
#

CopyAllHeaders('dtool/src/dconfig')
IPATH=['dtool/src/dconfig']
OPTS=['BUILDING_DTOOLCONFIG', 'NSPR', 'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='dconfig_composite.cxx', obj='dconfig_composite.obj')

#
# DIRECTORY: dtool/src/interrogatedb/
#

CopyAllHeaders('dtool/src/interrogatedb')
IPATH=['dtool/src/interrogatedb']
OPTS=['BUILDING_DTOOLCONFIG', 'NSPR', 'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='interrogatedb_composite.cxx', obj='interrogatedb_composite.obj')

#
# DIRECTORY: dtool/metalibs/dtoolconfig/
#

CopyAllHeaders('dtool/metalibs/dtoolconfig')
IPATH=['dtool/metalibs/dtoolconfig']
OPTS=['BUILDING_DTOOLCONFIG', 'NSPR', 'OPT3']
SRCFILE="pydtool.cxx"
if (OMIT.count("PYTHON")): SRCFILE="null.cxx"
EnqueueCxx(ipath=IPATH, opts=OPTS, src='dtoolconfig.cxx', obj='dtoolconfig_dtoolconfig.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src=SRCFILE, obj='dtoolconfig_pydtool.obj', xdep=["built/tmp/dtool_have_python.dat"])
EnqueueLink(opts=['ADVAPI', 'NSPR', 'OPENSSL', 'OPT3'], dll='libdtoolconfig.dll', obj=[
             'dtoolconfig_dtoolconfig.obj',
             'dtoolconfig_pydtool.obj',
             'interrogatedb_composite.obj',
             'dconfig_composite.obj',
             'prc_composite.obj',
             'libdtool.dll',
])

#
# DIRECTORY: dtool/src/pystub/
#

CopyAllHeaders('dtool/src/pystub')
IPATH=['dtool/src/pystub']
OPTS=['BUILDING_DTOOLCONFIG', 'NSPR', 'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pystub.cxx', obj='pystub_pystub.obj')
EnqueueLink(opts=['ADVAPI', 'NSPR', 'OPT3'], dll='libpystub.dll', obj=[
             'pystub_pystub.obj',
             'libdtool.dll',
])

#
# DIRECTORY: dtool/src/interrogate/
#

CopyAllHeaders('dtool/src/interrogate')
IPATH=['dtool/src/interrogate', 'dtool/src/cppparser', 'dtool/src/interrogatedb']
OPTS=['NSPR', 'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='interrogate_composite.cxx', obj='interrogate_composite.obj')
EnqueueLink(opts=['ADVAPI', 'NSPR', 'OPENSSL', 'OPT3'], dll='interrogate.exe', obj=[
             'interrogate_composite.obj',
             'libcppParser.ilb',
             'libpystub.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

EnqueueCxx(ipath=IPATH, opts=OPTS, src='interrogate_module.cxx', obj='interrogate_module_interrogate_module.obj')
EnqueueLink(opts=['ADVAPI', 'NSPR', 'OPENSSL', 'OPT3'], dll='interrogate_module.exe', obj=[
             'interrogate_module_interrogate_module.obj',
             'libcppParser.ilb',
             'libpystub.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

EnqueueCxx(ipath=IPATH, opts=OPTS, src='parse_file.cxx', obj='parse_file_parse_file.obj')
EnqueueLink(opts=['ADVAPI', 'NSPR', 'OPENSSL', 'OPT3'], dll='parse_file.exe', obj=[
             'parse_file_parse_file.obj',
             'libcppParser.ilb',
             'libpystub.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

#
# DIRECTORY: dtool/src/prckeys/
#

if (OMIT.count("OPENSSL")==0):
    CopyAllHeaders('dtool/src/prckeys')
    IPATH=['dtool/src/prckeys']
    OPTS=['OPENSSL', 'NSPR']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='makePrcKey.cxx', obj='make-prc-key_makePrcKey.obj')
    EnqueueLink(opts=['ADVAPI', 'NSPR', 'OPENSSL'], dll='make-prc-key.exe', obj=[
                 'make-prc-key_makePrcKey.obj',
                 'libpystub.dll',
                 'libdtool.dll',
                 'libdtoolconfig.dll',
                 ])

#
# DIRECTORY: dtool/src/test_interrogate/
#

CopyAllHeaders('dtool/src/test_interrogate', skip="ALL")
IPATH=['dtool/src/test_interrogate']
OPTS=['NSPR']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='test_interrogate.cxx', obj='test_interrogate_test_interrogate.obj')
EnqueueLink(opts=['ADVAPI', 'NSPR', 'OPENSSL'], dll='test_interrogate.exe', obj=[
             'test_interrogate_test_interrogate.obj',
             'libpystub.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

#
# DIRECTORY: panda/src/pandabase/
#

CopyAllHeaders('panda/src/pandabase')
IPATH=['panda/src/pandabase']
OPTS=['BUILDING_PANDAEXPRESS', 'NSPR']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandabase.cxx', obj='pandabase_pandabase.obj')

#
# DIRECTORY: panda/src/express/
#

CopyAllHeaders('panda/src/express')
IPATH=['panda/src/express']
OPTS=['BUILDING_PANDAEXPRESS', 'OPENSSL', 'ZLIB', 'NSPR']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='express_composite1.cxx', obj='express_composite1.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='express_composite2.cxx', obj='express_composite2.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libexpress.in', obj='libexpress_igate.obj',
            src='panda/src/express',  module='pandaexpress', library='libexpress',
            skip=[], also=["express_composite1.cxx", "express_composite2.cxx"])

#
# DIRECTORY: panda/src/downloader/
#

CopyAllHeaders('panda/src/downloader')
IPATH=['panda/src/downloader']
OPTS=['BUILDING_PANDAEXPRESS', 'OPENSSL', 'ZLIB', 'NSPR']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='downloader_composite.cxx', obj='downloader_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdownloader.in', obj='libdownloader_igate.obj',
            src='panda/src/downloader',  module='pandaexpress', library='libdownloader',
            skip=[], also=["downloader_composite.cxx"])

#
# DIRECTORY: panda/metalibs/pandaexpress/
#

CopyAllHeaders('panda/metalibs/pandaexpress')
IPATH=['panda/metalibs/pandaexpress']
OPTS=['BUILDING_PANDAEXPRESS', 'ZLIB', 'NSPR']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandaexpress.cxx', obj='pandaexpress_pandaexpress.obj')
EnqueueImod(ipath=IPATH, opts=OPTS, obj='libpandaexpress_module.obj',
            module='pandaexpress', library='libpandaexpress',
            files=['libdownloader.in', 'libexpress.in'])
EnqueueLink(opts=['ADVAPI', 'WINSOCK2', 'NSPR', 'OPENSSL', 'ZLIB'], dll='libpandaexpress.dll', obj=[
             'pandaexpress_pandaexpress.obj',
             'libpandaexpress_module.obj',
             'downloader_composite.obj',
             'libdownloader_igate.obj',
             'express_composite1.obj',
             'express_composite2.obj',
             'libexpress_igate.obj',
             'pandabase_pandabase.obj',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

#
# DIRECTORY: panda/src/putil/
#

IPATH=['panda/src/putil']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/putil')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='putil_composite1.cxx', obj='putil_composite1.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='putil_composite2.cxx', obj='putil_composite2.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libputil.in', obj='libputil_igate.obj',
            src='panda/src/putil',  module='panda', library='libputil',
            skip=["test_bam.h"], also=["putil_composite1.cxx", "putil_composite2.cxx"])

#
# DIRECTORY: panda/src/audio/
#

IPATH=['panda/src/audio']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/audio')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='audio_composite.cxx', obj='audio_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libaudio.in', obj='libaudio_igate.obj',
            src='panda/src/audio',  module='panda', library='libaudio',
            skip="ALL", also=["audio.h"])

#
# DIRECTORY: panda/src/event/
#

IPATH=['panda/src/event']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/event')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='event_composite.cxx', obj='event_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libevent.in', obj='libevent_igate.obj',
            src='panda/src/event',  module='panda', library='libevent',
            skip=[], also=["event_composite.cxx"])

#
# DIRECTORY: panda/src/linmath/
#

IPATH=['panda/src/linmath']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/linmath')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='linmath_composite.cxx', obj='linmath_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='liblinmath.in', obj='liblinmath_igate.obj',
            src='panda/src/linmath',  module='panda', library='liblinmath',
            skip=['lmat_ops_src.h', 'cast_to_double.h', 'lmat_ops.h', 'cast_to_float.h'],
            also=["linmath_composite.cxx"])

#
# DIRECTORY: panda/src/mathutil/
#

IPATH=['panda/src/mathutil']
OPTS=['BUILDING_PANDA', 'FFTW', 'NSPR']
CopyAllHeaders('panda/src/mathutil')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='mathutil_composite.cxx', obj='mathutil_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libmathutil.in', obj='libmathutil_igate.obj',
            src='panda/src/mathutil',  module='panda', library='libmathutil',
            skip=['mathHelpers.h'], also=["mathutil_composite.cxx"])

#
# DIRECTORY: panda/src/gsgbase/
#

IPATH=['panda/src/gsgbase']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/gsgbase')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='gsgbase_composite.cxx', obj='gsgbase_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libgsgbase.in', obj='libgsgbase_igate.obj',
            src='panda/src/gsgbase',  module='panda', library='libgsgbase',
            skip=[], also=["gsgbase_composite.cxx"])

#
# DIRECTORY: panda/src/pnmimage/
#

IPATH=['panda/src/pnmimage']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/pnmimage')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pnmimage_composite.cxx', obj='pnmimage_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libpnmimage.in', obj='libpnmimage_igate.obj',
            src='panda/src/pnmimage',  module='panda', library='libpnmimage',
            skip=[], also=["pnmimage_composite.cxx"])

#
# DIRECTORY: panda/src/net/
#

if (OMIT.count("NSPR")==0):
    IPATH=['panda/src/net']
    OPTS=['BUILDING_PANDA', 'NSPR']
    CopyAllHeaders('panda/src/net')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='net_composite.cxx', obj='net_composite.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libnet.in', obj='libnet_igate.obj',
                src='panda/src/net',  module='panda', library='libnet',
                skip=["datagram_ui.h"], also=["net_composite.cxx"])

#
# DIRECTORY: panda/src/pstatclient/
#

IPATH=['panda/src/pstatclient']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/pstatclient')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pstatclient_composite.cxx', obj='pstatclient_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libpstatclient.in', obj='libpstatclient_igate.obj',
            src='panda/src/pstatclient',  module='panda', library='libpstatclient',
            skip=[], also=["pstatclient_composite.cxx"])

#
# DIRECTORY: panda/src/gobj/
#

IPATH=['panda/src/gobj']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/gobj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='gobj_composite1.cxx', obj='gobj_composite1.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='gobj_composite2.cxx', obj='gobj_composite2.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libgobj.in', obj='libgobj_igate.obj',
            src='panda/src/gobj',  module='panda', library='libgobj',
            skip=[], also=["gobj_composite1.cxx", "gobj_composite2.cxx"])

#
# DIRECTORY: panda/src/lerp/
#

IPATH=['panda/src/lerp']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/lerp')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='lerp_composite.cxx', obj='lerp_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='liblerp.in', obj='liblerp_igate.obj',
            src='panda/src/lerp',  module='panda', library='liblerp',
            skip=["lerp_headers.h","lerpchans.h"], also=["lerp_composite.cxx"])

#
# DIRECTORY: panda/src/pgraph/
#

IPATH=['panda/src/pgraph']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/pgraph')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='nodePath.cxx', obj='pgraph_nodePath.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pgraph_composite1.cxx', obj='pgraph_composite1.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pgraph_composite2.cxx', obj='pgraph_composite2.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pgraph_composite3.cxx', obj='pgraph_composite3.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pgraph_composite4.cxx', obj='pgraph_composite4.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libpgraph.in', obj='libpgraph_igate.obj',
            src='panda/src/pgraph',  module='panda', library='libpgraph',
            skip=["antialiasAttrib.h"],
            also=["nodePath.cxx",
                  "pgraph_composite1.cxx", "pgraph_composite2.cxx",
                  "pgraph_composite3.cxx", "pgraph_composite4.cxx"])

#
# DIRECTORY: panda/src/effects/
#

IPATH=['panda/src/effects']
OPTS=['BUILDING_PANDAFX', 'NSPR', 'NVIDIACG']
CopyAllHeaders('panda/src/effects')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='effects_composite.cxx', obj='effects_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libeffects.in', obj='libeffects_igate.obj',
            src='panda/src/effects',  module='pandafx', library='libeffects',
            skip=["cgShader.h", "cgShaderAttrib.h", "cgShaderContext.h"],
            also=["effects_composite.cxx"])

#
# DIRECTORY: panda/src/chan/
#

IPATH=['panda/src/chan']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/chan')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='chan_composite.cxx', obj='chan_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libchan.in', obj='libchan_igate.obj',
            src='panda/src/chan',  module='panda', library='libchan',
            skip=['movingPart.h', 'chan_headers.h', 'animChannelFixed.h'],
            also=["chan_composite.cxx"])

#
# DIRECTORY: panda/src/char/
#

IPATH=['panda/src/char']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/char')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='char_composite.cxx', obj='char_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libchar.in', obj='libchar_igate.obj',
            src='panda/src/char',  module='panda', library='libchar',
            skip=[], also=["char_composite.cxx"])

#
# DIRECTORY: panda/src/dgraph/
#

IPATH=['panda/src/dgraph']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/dgraph')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='dgraph_composite.cxx', obj='dgraph_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdgraph.in', obj='libdgraph_igate.obj',
            src='panda/src/dgraph',  module='panda', library='libdgraph',
            skip=[], also=["dgraph_composite.cxx"])

#
# DIRECTORY: panda/src/display/
#

IPATH=['panda/src/display']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/display')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='display_composite.cxx', obj='display_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdisplay.in', obj='libdisplay_igate.obj',
            src='panda/src/display',  module='panda', library='libdisplay',
            skip=['renderBuffer.h'], also=["display_composite.cxx"])

#
# DIRECTORY: panda/src/device/
#

IPATH=['panda/src/device']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/device')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='device_composite.cxx', obj='device_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdevice.in', obj='libdevice_igate.obj',
            src='panda/src/device',  module='panda', library='libdevice',
            skip=[], also=["device_composite.cxx"])

#
# DIRECTORY: panda/src/pnmtext/
#

if (OMIT.count("FREETYPE")==0):
    IPATH=['panda/src/pnmtext']
    OPTS=['BUILDING_PANDA', 'NSPR', 'FREETYPE']
    CopyAllHeaders('panda/src/pnmtext')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pnmtext_composite.cxx', obj='pnmtext_composite.obj')

#
# DIRECTORY: panda/src/text/
#

IPATH=['panda/src/text']
OPTS=['BUILDING_PANDA', 'ZLIB', 'NSPR', 'FREETYPE']
CopyAllHeaders('panda/src/text')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='text_composite.cxx', obj='text_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libtext.in', obj='libtext_igate.obj',
            src='panda/src/text',  module='panda', library='libtext',
            skip=[], also=["text_composite.cxx"])

#
# DIRECTORY: panda/src/grutil/
#

IPATH=['panda/src/grutil']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/grutil')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='multitexReducer.cxx', obj='grutil_multitexReducer.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='grutil_composite.cxx', obj='grutil_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libgrutil.in', obj='libgrutil_igate.obj',
            src='panda/src/grutil',  module='panda', library='libgrutil',
            skip=[], also=["multitexReducer.cxx","grutil_composite.cxx"])

#
# DIRECTORY: panda/src/tform/
#

IPATH=['panda/src/tform']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/tform')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='tform_composite.cxx', obj='tform_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libtform.in', obj='libtform_igate.obj',
            src='panda/src/tform',  module='panda', library='libtform',
            skip=[], also=["tform_composite.cxx"])

#
# DIRECTORY: panda/src/collide/
#

IPATH=['panda/src/collide']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/collide')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='collide_composite.cxx', obj='collide_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libcollide.in', obj='libcollide_igate.obj',
            src='panda/src/collide',  module='panda', library='libcollide',
            skip=["collide_headers.h"], also=["collide_composite.cxx"])

#
# DIRECTORY: panda/src/parametrics/
#

IPATH=['panda/src/parametrics']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/parametrics')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='parametrics_composite.cxx', obj='parametrics_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libparametrics.in', obj='libparametrics_igate.obj',
            src='panda/src/parametrics',  module='panda', library='libparametrics',
            skip=['nurbsPPCurve.h'], also=["parametrics_composite.cxx"])

#
# DIRECTORY: panda/src/pgui/
#

IPATH=['panda/src/pgui']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/pgui')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pgui_composite.cxx', obj='pgui_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libpgui.in', obj='libpgui_igate.obj',
            src='panda/src/pgui',  module='panda', library='libpgui',
            skip=[], also=["pgui_composite.cxx"])

#
# DIRECTORY: panda/src/pnmimagetypes/
#

IPATH=['panda/src/pnmimagetypes', 'panda/src/pnmimage']
OPTS=['BUILDING_PANDA', 'PNG', 'ZLIB', 'JPEG', 'ZLIB', 'NSPR', 'JPEG', 'TIFF']
CopyAllHeaders('panda/src/pnmimagetypes')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pnmFileTypePNG.cxx', obj='pnmimagetypes_pnmFileTypePNG.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pnmFileTypeTIFF.cxx', obj='pnmimagetypes_pnmFileTypeTIFF.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pnmimagetypes_composite.cxx', obj='pnmimagetypes_composite.obj')

#
# DIRECTORY: panda/src/recorder/
#

IPATH=['panda/src/recorder']
OPTS=['BUILDING_PANDA', 'NSPR']
CopyAllHeaders('panda/src/recorder')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='recorder_composite.cxx', obj='recorder_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='librecorder.in', obj='librecorder_igate.obj',
            src='panda/src/recorder',  module='panda', library='librecorder',
            skip=[], also=["recorder_composite.cxx"])

#
# DIRECTORY: panda/src/vrpn/
#

if (OMIT.count("VRPN")==0):
    IPATH=['panda/src/vrpn']
    OPTS=['BUILDING_PANDA', 'NSPR', 'VRPN']
    CopyAllHeaders('panda/src/vrpn')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='vrpn_composite.cxx', obj='pvrpn_composite.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libpvrpn.in', obj='libpvrpn_igate.obj',
                src='panda/src/vrpn',  module='panda', library='libpvrpn',
                skip=[], also=["vrpn_composite.cxx"])

#
# DIRECTORY: panda/src/helix/
#

if (OMIT.count("HELIX")==0):
  IPATH=['panda/src/helix']
  OPTS=['BUILDING_PANDA', 'NSPR', 'HELIX']
  CopyAllHeaders('panda/src/helix')
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='config_helix.cxx', obj='helix_config_helix.obj')
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='fivemmap.cxx', obj='helix_fivemmap.obj')
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='HelixClient.cxx', obj='helix_HelixClient.obj')
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='HxAdviseSink.cxx', obj='helix_HxAdviseSink.obj')
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='HxAuthenticationManager.cxx', obj='helix_HxAuthenticationManager.obj')
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='HxClientContext.cxx', obj='helix_HxClientContext.obj')
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='HxErrorSink.cxx', obj='helix_HxErrorSink.obj')
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='HxSiteSupplier.cxx', obj='helix_HxSiteSupplier.obj')
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='iids.cxx', obj='helix_iids.obj')
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='print.cxx', obj='helix_print.obj')
  EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libhelix.in', obj='libhelix_igate.obj',
              src='panda/src/helix',  module='panda', library='libhelix',
              skip="ALL", also=["HelixClient.cxx"])
  EnqueueLib(lib='libhelix.ilb', obj=[
             'helix_config_helix.obj',
             'helix_fivemmap.obj',
             'helix_HelixClient.obj',
             'helix_HxAdviseSink.obj',
             'helix_HxAuthenticationManager.obj',
             'helix_HxClientContext.obj',
             'helix_HxErrorSink.obj',
             'helix_HxSiteSupplier.obj',
             'helix_iids.obj',
             'helix_print.obj',
             'libhelix_igate.obj'])

#
# DIRECTORY: panda/metalibs/panda/
#

CopyAllHeaders('panda/src/glgsg')
CopyAllHeaders('panda/src/wgldisplay')
CopyAllHeaders('panda/src/physics')
CopyAllHeaders('panda/src/particlesystem')
IPATH=['panda/metalibs/panda']
OPTS=['BUILDING_PANDA', 'ZLIB', 'VRPN', 'JPEG', 'PNG', 'TIFF', 'NSPR', 'FREETYPE', 'HELIX', 'FFTW',
      'ADVAPI', 'WINSOCK2', 'WINUSER', 'WINMM']
INFILES=['librecorder.in', 'libpgraph.in', 'libgrutil.in', 'libchan.in', 'libpstatclient.in',
         'libchar.in', 'libcollide.in', 'libdevice.in', 'libdgraph.in', 'libdisplay.in', 'libevent.in',
         'libgobj.in', 'libgsgbase.in', 'liblinmath.in', 'libmathutil.in', 'libparametrics.in',
         'libpnmimage.in', 'libtext.in', 'libtform.in', 'liblerp.in', 'libputil.in', 'libaudio.in',
         'libpgui.in']
OBJFILES=['panda_panda.obj', 'libpanda_module.obj',
          'recorder_composite.obj', 'librecorder_igate.obj',
          'pgraph_nodePath.obj', 
          'pgraph_composite1.obj', 'pgraph_composite2.obj', 'pgraph_composite3.obj', 'pgraph_composite4.obj', 'libpgraph_igate.obj',
          'grutil_multitexReducer.obj', 'grutil_composite.obj', 'libgrutil_igate.obj',
          'chan_composite.obj', 'libchan_igate.obj',
          'pstatclient_composite.obj', 'libpstatclient_igate.obj',
          'char_composite.obj', 'libchar_igate.obj',
          'collide_composite.obj', 'libcollide_igate.obj',
          'device_composite.obj', 'libdevice_igate.obj',
          'dgraph_composite.obj', 'libdgraph_igate.obj',
          'display_composite.obj', 'libdisplay_igate.obj',
          'event_composite.obj', 'libevent_igate.obj',
          'gobj_composite1.obj', 'gobj_composite2.obj', 'libgobj_igate.obj',
          'gsgbase_composite.obj', 'libgsgbase_igate.obj',
          'linmath_composite.obj', 'liblinmath_igate.obj',
          'mathutil_composite.obj', 'libmathutil_igate.obj',
          'parametrics_composite.obj', 'libparametrics_igate.obj',
          'pnmimagetypes_composite.obj', 'pnmimagetypes_pnmFileTypePNG.obj', 'pnmimagetypes_pnmFileTypeTIFF.obj',
          'pnmimage_composite.obj', 'libpnmimage_igate.obj',
          'text_composite.obj', 'libtext_igate.obj',
          'tform_composite.obj', 'libtform_igate.obj',
          'lerp_composite.obj', 'liblerp_igate.obj',
          'putil_composite1.obj', 'putil_composite2.obj', 'libputil_igate.obj',
          'audio_composite.obj', 'libaudio_igate.obj',
          'pgui_composite.obj', 'libpgui_igate.obj',
          'pandabase_pandabase.obj', 'libpandaexpress.dll', 'libdtoolconfig.dll', 'libdtool.dll']
if OMIT.count("HELIX")==0:
    OBJFILES.append("libhelix.ilb")
    INFILES.append("libhelix.in")
if OMIT.count("VRPN")==0:
    OBJFILES.append("pvrpn_composite.obj")
    OBJFILES.append("libpvrpn_igate.obj")
    INFILES.append("libpvrpn.in")
if OMIT.count("NSPR")==0:
    OBJFILES.append("net_composite.obj")
    OBJFILES.append("libnet_igate.obj")
    INFILES.append("libnet.in")
if OMIT.count("FREETYPE")==0:
    OBJFILES.append("pnmtext_composite.obj")
#    OBJFILES.append("pnmtext_config_pnmtext.obj")
#    OBJFILES.append("pnmtext_freetypeFont.obj")
#    OBJFILES.append("pnmtext_pnmTextGlyph.obj")
#    OBJFILES.append("pnmtext_pnmTextMaker.obj")
CopyAllHeaders('panda/metalibs/panda')
EnqueueImod(ipath=IPATH, opts=OPTS, obj='libpanda_module.obj',
            module='panda', library='libpanda', files=INFILES)
EnqueueCxx(ipath=IPATH, opts=OPTS, src='panda.cxx', obj='panda_panda.obj')
EnqueueLink(opts=OPTS, dll='libpanda.dll', obj=OBJFILES, xdep=[
        'built/tmp/dtool_have_helix.dat',
        'built/tmp/dtool_have_vrpn.dat',
        'built/tmp/dtool_have_nspr.dat',
        'built/tmp/dtool_have_freetype.dat',
])

#
# DIRECTORY: panda/src/audiotraits/
#

if OMIT.count("FMOD") == 0:
  IPATH=['panda/src/audiotraits']
  OPTS=['BUILDING_FMOD_AUDIO', 'NSPR', 'FMOD']
  CopyAllHeaders('panda/src/audiotraits')
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='fmod_audio_composite.cxx', obj='fmod_audio_fmod_audio_composite.obj')
  EnqueueLink(opts=['ADVAPI', 'WINUSER', 'WINMM', 'FMOD', 'NSPR'], dll='libfmod_audio.dll', obj=[
               'fmod_audio_fmod_audio_composite.obj',
               'libpanda.dll',
               'libpandaexpress.dll',
               'libdtoolconfig.dll',
               'libdtool.dll',
  ])

if OMIT.count("MILES") == 0:
  IPATH=['panda/src/audiotraits']
  OPTS=['BUILDING_MILES_AUDIO', 'NSPR', 'MILES']
  CopyAllHeaders('panda/src/audiotraits')
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='miles_audio_composite.cxx', obj='miles_audio_miles_audio_composite.obj')
  EnqueueLink(opts=['ADVAPI', 'WINUSER', 'WINMM', 'MILES', 'NSPR'], dll='libmiles_audio.dll', obj=[
               'miles_audio_miles_audio_composite.obj',
               'libpanda.dll',
               'libpandaexpress.dll',
               'libdtoolconfig.dll',
               'libdtool.dll',
  ])

#
# DIRECTORY: panda/src/distort/
#

IPATH=['panda/src/distort']
OPTS=['BUILDING_PANDAFX', 'NSPR']
CopyAllHeaders('panda/src/distort')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='distort_composite.cxx', obj='distort_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdistort.in', obj='libdistort_igate.obj',
            src='panda/src/distort',  module='pandafx', library='libdistort',
            skip=[], also=["distort_composite.cxx"])

#
# DIRECTORY: panda/src/downloadertools/
#
# There's something funny about this OMIT.count('OPENSSL')... check it out.
#

if OMIT.count("OPENSSL")==0:
    IPATH=['panda/src/downloadertools']
    OPTS=['OPENSSL', 'ZLIB', 'NSPR']
    LIBS=['libpandaexpress.dll', 'libpanda.dll', 'libdtoolconfig.dll', 'libdtool.dll', 'libpystub.dll']
    CopyAllHeaders('panda/src/downloadertools')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='apply_patch.cxx', obj='apply_patch_apply_patch.obj')
    EnqueueLink(dll='apply_patch.exe', opts=['ADVAPI', 'NSPR'], obj=['apply_patch_apply_patch.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='build_patch.cxx', obj='build_patch_build_patch.obj')
    EnqueueLink(dll='build_patch.exe', opts=['ADVAPI', 'NSPR'], obj=['build_patch_build_patch.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='check_adler.cxx', obj='check_adler_check_adler.obj')
    EnqueueLink(dll='check_adler.exe', opts=['ADVAPI', 'NSPR', 'ZLIB'], obj=['check_adler_check_adler.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='check_crc.cxx', obj='check_crc_check_crc.obj')
    EnqueueLink(dll='check_crc.exe', opts=['ADVAPI', 'NSPR', 'ZLIB'], obj=['check_crc_check_crc.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='check_md5.cxx', obj='check_md5_check_md5.obj')
    EnqueueLink(dll='check_md5.exe', opts=['ADVAPI', 'NSPR', 'OPENSSL'], obj=['check_md5_check_md5.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='multify.cxx', obj='multify_multify.obj')
    EnqueueLink(dll='multify.exe', opts=['ADVAPI', 'NSPR'], obj=['multify_multify.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pcompress.cxx', obj='pcompress_pcompress.obj')
    EnqueueLink(dll='pcompress.exe', opts=['ADVAPI', 'NSPR', 'ZLIB'], obj=['pcompress_pcompress.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pdecompress.cxx', obj='pdecompress_pdecompress.obj')
    EnqueueLink(dll='pdecompress.exe', opts=['ADVAPI', 'NSPR', 'ZLIB'], obj=['pdecompress_pdecompress.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pdecrypt.cxx', obj='pdecrypt_pdecrypt.obj')
    EnqueueLink(dll='pdecrypt.exe', opts=['ADVAPI', 'NSPR', 'OPENSSL'], obj=['pdecrypt_pdecrypt.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pencrypt.cxx', obj='pencrypt_pencrypt.obj')
    EnqueueLink(dll='pencrypt.exe', opts=['ADVAPI', 'NSPR', 'OPENSSL'], obj=['pencrypt_pencrypt.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='show_ddb.cxx', obj='show_ddb_show_ddb.obj')
    EnqueueLink(dll='show_ddb.exe', opts=['ADVAPI', 'NSPR'], obj=['show_ddb_show_ddb.obj']+LIBS)

#
# DIRECTORY: panda/src/windisplay/
#

if (sys.platform == "win32"):
    IPATH=['panda/src/windisplay']
    OPTS=['BUILDING_PANDAWIN', 'NSPR']
    CopyAllHeaders('panda/src/windisplay')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='windisplay_composite.cxx', obj='windisplay_composite.obj')
    EnqueueLink(opts=['WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM', 'NSPR'],
                dll='libwindisplay.dll', obj=[
      'windisplay_composite.obj',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libdtoolconfig.dll',
      'libdtool.dll',
      ])

#
# DIRECTORY: panda/metalibs/pandadx7/
#
# 
# if (sys.platform == "win32"):
#     IPATH=['panda/src/dxgsg7']
#     OPTS=['BUILDING_PANDADX', 'DXSDK', 'NSPR']
#     CopyAllHeaders('panda/src/dxgsg7')
#     EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxGraphicsStateGuardian7.cxx', obj='dxgsg7_dxGraphicsStateGuardian7.obj')
#     EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxgsg7_composite.cxx', obj='dxgsg7_composite.obj')
# 
#     IPATH=['panda/metalibs/pandadx7']
#     OPTS=['BUILDING_PANDADX', 'DXSDK', 'NSPR']
#     CopyAllHeaders('panda/metalibs/pandadx7')
#     EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandadx7.cxx', obj='pandadx7_pandadx7.obj')
#     EnqueueLink(dll='libpandadx7.dll', opts=['ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DXDRAW', 'DXGUID', 'D3D8', 'NSPR'], obj=[
#       'pandadx7_pandadx7.obj',
#       'dxgsg7_dxGraphicsStateGuardian7.obj',
#       'dxgsg7_composite.obj',
#       'libpanda.dll',
#       'libpandaexpress.dll',
#       'libwindisplay.dll',
#       'libdtoolconfig.dll',
#       'libdtool.dll',
#       ])
# 

#
# DIRECTORY: panda/metalibs/pandadx8/
#

if (sys.platform == "win32"):
    IPATH=['panda/src/dxgsg8', 'panda/metalibs/pandadx8']
    OPTS=['BUILDING_PANDADX', 'DXSDK', 'NSPR']
    CopyAllHeaders('panda/src/dxgsg8')
    CopyAllHeaders('panda/metalibs/pandadx8')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxGraphicsStateGuardian8.cxx', obj='dxgsg8_dxGraphicsStateGuardian8.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxgsg8_composite.cxx', obj='dxgsg8_composite.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandadx8.cxx', obj='pandadx8_pandadx8.obj')
    EnqueueLink(dll='libpandadx8.dll',
      opts=['ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DXDRAW', 'DXGUID', 'D3D8', 'NSPR'], obj=[
      'pandadx8_pandadx8.obj',
      'dxgsg8_dxGraphicsStateGuardian8.obj',
      'dxgsg8_composite.obj',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libwindisplay.dll',
      'libdtoolconfig.dll',
      'libdtool.dll',
      ])

#
# DIRECTORY: panda/metalibs/pandadx9/
#
# 
# if (sys.platform == "win32"):
#     IPATH=['panda/src/dxgsg9']
#     OPTS=['BUILDING_PANDADX', 'DXSDK', 'NSPR']
#     CopyAllHeaders('panda/src/dxgsg9')
#     EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxGraphicsStateGuardian9.cxx', obj='dxgsg9_dxGraphicsStateGuardian9.obj')
#     EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxgsg9_composite.cxx', obj='dxgsg9_composite.obj')
# 
#     IPATH=['panda/metalibs/pandadx9']
#     OPTS=['BUILDING_PANDADX', 'DXSDK', 'NSPR']
#     CopyAllHeaders('panda/metalibs/pandadx9')
#     EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandadx9.cxx', obj='pandadx9_pandadx9.obj')
#     EnqueueLink(dll='libpandadx9.dll',
#       opts=['ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DXDRAW', 'DXGUID', 'D3D9', 'NSPR'], obj=[
#       'pandadx9_pandadx9.obj',
#       'dxgsg9_dxGraphicsStateGuardian9.obj',
#       'dxgsg9_composite.obj',
#       'libpanda.dll',
#       'libpandaexpress.dll',
#       'libwindisplay.dll',
#       'libdtoolconfig.dll',
#       'libdtool.dll',
#       ])
# 

#
# DIRECTORY: panda/src/egg/
#

IPATH=['panda/src/egg']
OPTS=['BUILDING_PANDAEGG', 'NSPR']
CopyAllHeaders('panda/src/egg')
EnqueueBison(ipath=IPATH, opts=OPTS, pre='eggyy', src='parser.yxx', dsth='parser.h', obj='egg_parser.obj')
EnqueueFlex(ipath=IPATH, opts=OPTS, pre='eggyy', src='lexer.lxx', obj='egg_lexer.obj', dashi=1)
EnqueueCxx(ipath=IPATH, opts=OPTS, src='egg_composite1.cxx', obj='egg_composite1.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='egg_composite2.cxx', obj='egg_composite2.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libegg.in', obj='libegg_igate.obj',
            src='panda/src/egg',  module='pandaegg', library='libegg',
            skip=["parser.h"], also=["egg_composite1.cxx","egg_composite2.cxx"])

#
# DIRECTORY: panda/src/egg2pg/
#

IPATH=['panda/src/egg2pg']
OPTS=['BUILDING_PANDAEGG', 'NSPR']
CopyAllHeaders('panda/src/egg2pg')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='egg2pg_composite.cxx', obj='egg2pg_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libegg2pg.in', obj='libegg2pg_igate.obj',
            src='panda/src/egg2pg',  module='pandaegg', library='libegg2pg',
            skip="ALL", also=['load_egg_file.h'])

#
# DIRECTORY: panda/src/framework/
#

IPATH=['panda/src/framework']
OPTS=['BUILDING_FRAMEWORK', 'NSPR']
CopyAllHeaders('panda/src/framework')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='framework_composite.cxx', obj='framework_composite.obj')
EnqueueLink(dll='libframework.dll', opts=['ADVAPI', 'NSPR'], obj=[
             'framework_composite.obj',
             'libpanda.dll',
             'libpandaexpress.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
             ])

#
# DIRECTORY: panda/metalibs/pandafx/
#

IPATH=['panda/metalibs/pandafx', 'panda/src/distort']
OPTS=['BUILDING_PANDAFX', 'NSPR', 'NVIDIACG']
CopyAllHeaders('panda/metalibs/pandafx')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandafx.cxx', obj='pandafx_pandafx.obj')
EnqueueImod(ipath=IPATH, opts=OPTS, obj='libpandafx_module.obj',
            module='pandafx', library='libpandafx',
            files=['libdistort.in', 'libeffects.in'])
EnqueueLink(dll='libpandafx.dll', opts=['ADVAPI', 'NSPR', 'NVIDIACG'], obj=[
             'pandafx_pandafx.obj',
             'libpandafx_module.obj',
             'distort_composite.obj',
             'libdistort_igate.obj',
             'effects_composite.obj',
             'libeffects_igate.obj',
             'libpanda.dll',
             'libpandaexpress.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

#
# DIRECTORY: panda/src/glstuff/
#

IPATH=['panda/src/glstuff']
OPTS=['NSPR', 'NVIDIACG', 'CGGL']
CopyAllHeaders('panda/src/glstuff')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='glpure.cxx', obj='glstuff_glpure.obj')
EnqueueLink(dll='libglstuff.dll', opts=['ADVAPI', 'GLUT', 'NSPR', 'NVIDIACG', 'CGGL'], obj=[
             'glstuff_glpure.obj',
             'libpanda.dll',
             'libpandafx.dll',
             'libpandaexpress.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

#
# DIRECTORY: panda/src/glgsg/
#

IPATH=['panda/src/glgsg', 'panda/src/glstuff', 'panda/src/gobj']
OPTS=['BUILDING_PANDAGL', 'NSPR', 'NVIDIACG']
CopyAllHeaders('panda/src/glgsg')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='config_glgsg.cxx', obj='glgsg_config_glgsg.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='glgsg.cxx', obj='glgsg_glgsg.obj')

#
# DIRECTORY: panda/metalibs/pandaegg/
#

IPATH=['panda/metalibs/pandaegg', 'panda/src/egg']
OPTS=['BUILDING_PANDAEGG', 'NSPR']
CopyAllHeaders('panda/metalibs/pandaegg')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandaegg.cxx', obj='pandaegg_pandaegg.obj')
EnqueueImod(ipath=IPATH, opts=OPTS, obj='libpandaegg_module.obj',
            module='pandaegg', library='libpandaegg',
            files=['libegg2pg.in', 'libegg.in'])
EnqueueLink(dll='libpandaegg.dll', opts=['ADVAPI', 'NSPR'], obj=[
             'pandaegg_pandaegg.obj',
             'libpandaegg_module.obj',
             'egg2pg_composite.obj',
             'libegg2pg_igate.obj',
             'egg_composite1.obj',
             'egg_composite2.obj',
             'egg_parser.obj',
             'egg_lexer.obj',
             'libegg_igate.obj',
             'libpanda.dll',
             'libpandaexpress.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

#
# DIRECTORY: panda/src/glxdisplay/
#

if (sys.platform != "win32"):
    IPATH=['panda/src/glxdisplay', 'panda/src/gobj']
    OPTS=['BUILDING_PANDAGLUT', 'NSPR', 'GLUT', 'NVIDIACG', 'CGGL']
    CopyAllHeaders('panda/src/glxdisplay')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='glxdisplay_composite.cxx', obj='glxdisplay_composite.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libglxdisplay.in', obj='libglxdisplay_igate.obj',
                src='panda/src/glxdisplay',  module='pandagl', library='libglxdisplay',
                skip="ALL", also=['glxGraphicsPipe.h'])

    IPATH=['panda/metalibs/pandagl']
    OPTS=['BUILDING_PANDAGL', 'NSPR', 'NVIDIACG', 'CGGL']
    CopyAllHeaders('panda/metalibs/pandagl')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandagl.cxx', obj='pandagl_pandagl.obj')
    EnqueueLink(opts=['GLUT', 'NVIDIACG', 'CGGL', 'NSPR'], dll='libpandagl.dll', obj=[
      'pandagl_pandagl.obj',
      'glgsg_config_glgsg.obj',
      'glgsg_glgsg.obj',
      'glxdisplay_composite.obj',
      'libglxdisplay_igate.obj',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libglstuff.dll',
      'libpandafx.dll',
      'libdtoolconfig.dll',
      'libdtool.dll',
      ])

#
# DIRECTORY: panda/src/wgldisplay/
#

if (sys.platform == "win32"):
    IPATH=['panda/src/wgldisplay', 'panda/src/glstuff', 'panda/src/gobj']
    OPTS=['BUILDING_PANDAGL', 'NSPR', 'NVIDIACG', 'CGGL']
    CopyAllHeaders('panda/src/wgldisplay')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='wgldisplay_composite.cxx', obj='wgldisplay_composite.obj')
    IPATH=['panda/metalibs/pandagl']
    OPTS=['BUILDING_PANDAGL', 'NSPR', 'NVIDIACG', 'CGGL']
    CopyAllHeaders('panda/metalibs/pandagl')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandagl.cxx', obj='pandagl_pandagl.obj')
    EnqueueLink(opts=['WINGDI', 'GLUT', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM', 'NSPR', 'NVIDIACG', 'CGGL'],
                dll='libpandagl.dll', obj=[
      'pandagl_pandagl.obj',
      'glgsg_config_glgsg.obj',
      'glgsg_glgsg.obj',
      'wgldisplay_composite.obj',
      'libwindisplay.dll',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libglstuff.dll',
      'libpandafx.dll',
      'libdtoolconfig.dll',
      'libdtool.dll',
      ])

#
# DIRECTORY: panda/src/physics/
#

IPATH=['panda/src/physics']
OPTS=['BUILDING_PANDAPHYSICS', 'NSPR']
CopyAllHeaders('panda/src/physics')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='physics_composite.cxx', obj='physics_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libphysics.in', obj='libphysics_igate.obj',
            src='panda/src/physics',  module='pandaphysics', library='libphysics',
            skip=["forces.h"], also=["physics_composite.cxx"])

#
# DIRECTORY: panda/src/particlesystem/
#

IPATH=['panda/src/particlesystem']
OPTS=['BUILDING_PANDAPHYSICS', 'NSPR']
CopyAllHeaders('panda/src/particlesystem')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='particlesystem_composite.cxx', obj='particlesystem_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libparticlesystem.in', obj='libparticlesystem_igate.obj',
            src='panda/src/particlesystem',  module='pandaphysics', library='libparticlesystem',
            skip=['orientedParticle.h', 'orientedParticleFactory.h', 'particlefactories.h', 'emitters.h', 'particles.h'],
            also=["particlesystem_composite.cxx"])

#
# DIRECTORY: panda/metalibs/pandaphysics/
#

IPATH=['panda/metalibs/pandaphysics']
CopyAllHeaders('panda/metalibs/pandaphysics')
OPTS=['BUILDING_PANDAPHYSICS', 'NSPR']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandaphysics.cxx', obj='pandaphysics_pandaphysics.obj')
EnqueueImod(ipath=IPATH, opts=OPTS, obj='libpandaphysics_module.obj',
            module='pandaphysics', library='libpandaphysics',
            files=['libphysics.in', 'libparticlesystem.in'])
EnqueueLink(dll='libpandaphysics.dll', opts=['ADVAPI', 'NSPR'], obj=[
             'pandaphysics_pandaphysics.obj',
             'libpandaphysics_module.obj',
             'physics_composite.obj',
             'libphysics_igate.obj',
             'particlesystem_composite.obj',
             'libparticlesystem_igate.obj',
             'libpanda.dll',
             'libpandaexpress.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

#
# DIRECTORY: panda/src/testbed/
#

IPATH=['panda/src/testbed']
OPTS=['NSPR']
CopyAllHeaders('panda/src/testbed')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pview.cxx', obj='pview_pview.obj')
EnqueueLink(dll='pview.exe', opts=['ADVAPI', 'NSPR'], obj=[
             'pview_pview.obj',
             'libframework.dll',
             'libpanda.dll',
             'libpandafx.dll',
             'libpandaexpress.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
             'libpystub.dll',
])

#
# DIRECTORY: direct/src/directbase/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/directbase']
    OPTS=['BUILDING_DIRECT', 'NSPR']
    CopyAllHeaders('direct/src/directbase')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='directbase.cxx', obj='directbase_directbase.obj')

    EnqueueCxx(ipath=IPATH, opts=['BUILDING_PPYTHON'], src='ppython.cxx', obj='ppython.obj')
    EnqueueLink(opts=['WINUSER'], dll='ppython.exe', obj=['ppython.obj'])
    EnqueueCxx(ipath=IPATH, opts=['BUILDING_GENPYCODE'], src='ppython.cxx', obj='genpycode.obj')
    EnqueueLink(opts=['WINUSER'], dll='genpycode.exe', obj=['genpycode.obj'])
    EnqueueCxx(ipath=IPATH, opts=['BUILDING_PACKPANDA'], src='ppython.cxx', obj='packpanda.obj')
    EnqueueLink(opts=['WINUSER'], dll='packpanda.exe', obj=['packpanda.obj'])


#
# DIRECTORY: direct/src/dcparser/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/dcparser']
    OPTS=['WITHINPANDA', 'BUILDING_DIRECT', 'NSPR']
    CopyAllHeaders('direct/src/dcparser')
    EnqueueBison(ipath=IPATH, opts=OPTS, pre='dcyy', src='dcParser.yxx', dsth='dcParser.h', obj='dcparser_dcParser.obj')
    EnqueueFlex(ipath=IPATH, opts=OPTS, pre='dcyy', src='dcLexer.lxx', obj='dcparser_dcLexer.obj', dashi=0)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dcparser_composite.cxx', obj='dcparser_composite.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdcparser.in', obj='libdcparser_igate.obj',
                src='direct/src/dcparser',  module='direct', library='libdcparser',
                skip=['dcmsgtypes.h'],
                also=["dcparser_composite.cxx"])

#
# DIRECTORY: direct/src/deadrec/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/deadrec']
    OPTS=['BUILDING_DIRECT', 'NSPR']
    CopyAllHeaders('direct/src/deadrec')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='deadrec_composite.cxx', obj='deadrec_composite.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdeadrec.in', obj='libdeadrec_igate.obj',
                src='direct/src/deadrec',  module='direct', library='libdeadrec',
                skip=[], also=["deadrec_composite.cxx"])

#
# DIRECTORY: direct/src/distributed/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/distributed', 'direct/src/dcparser']
    OPTS=['WITHINPANDA', 'BUILDING_DIRECT', 'OPENSSL', 'NSPR']
    CopyAllHeaders('direct/src/distributed')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='config_distributed.cxx', obj='distributed_config_distributed.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='cConnectionRepository.cxx', obj='distributed_cConnectionRepository.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='cDistributedSmoothNodeBase.cxx', obj='distributed_cDistributedSmoothNodeBase.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdistributed.in', obj='libdistributed_igate.obj',
                src='direct/src/distributed',  module='direct', library='libdistributed',
                skip=[], also=['config_distributed.cxx', 'cConnectionRepository.cxx', 'cDistributedSmoothNodeBase.cxx'])

#
# DIRECTORY: direct/src/interval/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/interval']
    OPTS=['BUILDING_DIRECT', 'NSPR']
    CopyAllHeaders('direct/src/interval')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='interval_composite.cxx', obj='interval_composite.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libinterval.in', obj='libinterval_igate.obj',
                src='direct/src/interval',  module='direct', library='libinterval',
                skip=[], also=["interval_composite.cxx"])

#
# DIRECTORY: direct/src/showbase/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/showbase']
    OPTS=['BUILDING_DIRECT', 'NSPR']
    CopyAllHeaders('direct/src/showbase')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='showBase.cxx', obj='showbase_showBase.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mersenne.cxx', obj='showbase_mersenne.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libshowbase.in', obj='libshowbase_igate.obj',
                src='direct/src/showbase', module='direct', library='libshowbase',
                skip=[], also=["mersenne.cxx","showBase.cxx"])

#
# DIRECTORY: direct/metalibs/direct/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/metalibs/direct']
    OPTS=['BUILDING_DIRECT', 'NSPR']
    CopyAllHeaders('direct/metalibs/direct')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='direct.cxx', obj='direct_direct.obj')
    EnqueueImod(ipath=IPATH, opts=OPTS, obj='libdirect_module.obj',
                module='direct', library='libdirect',
                files=['libdcparser.in', 'libshowbase.in', 'libdeadrec.in', 'libinterval.in', 'libdistributed.in'])
    EnqueueLink(dll='libdirect.dll', opts=['ADVAPI', 'NSPR', 'OPENSSL'], obj=[
                 'direct_direct.obj',
                 'libdirect_module.obj',
                 'directbase_directbase.obj',
                 'dcparser_composite.obj',
                 'dcparser_dcParser.obj',
                 'dcparser_dcLexer.obj',
                 'libdcparser_igate.obj',
                 'showbase_showBase.obj',
                 'showbase_mersenne.obj',
                 'libshowbase_igate.obj',
                 'deadrec_composite.obj',
                 'libdeadrec_igate.obj',
                 'interval_composite.obj',
                 'libinterval_igate.obj',
                 'distributed_config_distributed.obj',
                 'distributed_cConnectionRepository.obj',
                 'distributed_cDistributedSmoothNodeBase.obj',
                 'libdistributed_igate.obj',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
    ])

#
# DIRECTORY: direct/src/dcparse/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/dcparse', 'direct/src/dcparser']
    OPTS=['WITHINPANDA', 'NSPR']
    CopyAllHeaders('direct/src/dcparse')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dcparse.cxx', obj='dcparse_dcparse.obj')
    EnqueueLink(dll='dcparse.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'dcparse_dcparse.obj',
                 'libdirect.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: direct/src/heapq/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/heapq']
    OPTS=['NSPR']
    CopyAllHeaders('direct/src/heapq')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='heapq.cxx', obj='heapq_heapq.obj')
    EnqueueLink(dll='libheapq.dll', opts=['ADVAPI', 'NSPR'], obj=[
                 'heapq_heapq.obj',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
    ])

#
# DIRECTORY: pandatool/src/pandatoolbase/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/pandatoolbase']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/pandatoolbase')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandatoolbase_composite1.cxx', obj='pandatoolbase_composite1.obj')
    EnqueueLib(lib='libpandatoolbase.lib', obj=['pandatoolbase_composite1.obj'])

#
# DIRECTORY: pandatool/src/converter/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/converter']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/converter')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='somethingToEggConverter.cxx', obj='converter_somethingToEggConverter.obj')
    EnqueueLib(lib='libconverter.lib', obj=['converter_somethingToEggConverter.obj'])

#
# DIRECTORY: pandatool/src/progbase/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/progbase']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/progbase')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='progbase_composite1.cxx', obj='progbase_composite1.obj')
    EnqueueLib(lib='libprogbase.lib', obj=['progbase_composite1.obj'])

#
# DIRECTORY: pandatool/src/eggbase/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/eggbase']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/eggbase')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggbase_composite1.cxx', obj='eggbase_composite1.obj')
    EnqueueLib(lib='libeggbase.lib', obj=['eggbase_composite1.obj'])

#
# DIRECTORY: pandatool/src/bam/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/bam']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/bam')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='bamInfo.cxx', obj='bam-info_bamInfo.obj')
    EnqueueLink(dll='bam-info.exe', opts=['ADVAPI', 'NSPR', 'FFTW'], obj=[
                 'bam-info_bamInfo.obj',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='bamToEgg.cxx', obj='bam2egg_bamToEgg.obj')
    EnqueueLink(dll='bam2egg.exe', opts=['ADVAPI', 'NSPR', 'FFTW'], obj=[
                 'bam2egg_bamToEgg.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggToBam.cxx', obj='egg2bam_eggToBam.obj')
    EnqueueLink(dll='egg2bam.exe', opts=['ADVAPI', 'NSPR', 'FFTW'], obj=[
                 'egg2bam_eggToBam.obj',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libconverter.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    
#
# DIRECTORY: pandatool/src/cvscopy/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/cvscopy']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/cvscopy')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='cvscopy_composite1.cxx', obj='cvscopy_composite1.obj')
    EnqueueLib(lib='libcvscopy.lib', obj=['cvscopy_composite1.obj'])

#
# DIRECTORY: pandatool/src/dxf/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/dxf']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/dxf')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxf_composite1.cxx', obj='dxf_composite1.obj')
    EnqueueLib(lib='libdxf.lib', obj=['dxf_composite1.obj'])

#
# DIRECTORY: pandatool/src/dxfegg/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/dxfegg']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/dxfegg')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxfToEggConverter.cxx', obj='dxfegg_dxfToEggConverter.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxfToEggLayer.cxx', obj='dxfegg_dxfToEggLayer.obj')
    EnqueueLib(lib='libdxfegg.lib', obj=[
                 'dxfegg_dxfToEggConverter.obj',
                 'dxfegg_dxfToEggLayer.obj',
    ])

#
# DIRECTORY: pandatool/src/dxfprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/dxfprogs']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/dxfprogs')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxfPoints.cxx', obj='dxf-points_dxfPoints.obj')
    EnqueueLink(dll='dxf-points.exe', opts=['ADVAPI', 'NSPR', 'FFTW'], obj=[
                 'dxf-points_dxfPoints.obj',
                 'libprogbase.lib',
                 'libdxf.lib',
                 'libpandatoolbase.lib',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxfToEgg.cxx', obj='dxf2egg_dxfToEgg.obj')
    EnqueueLink(dll='dxf2egg.exe', opts=['ADVAPI', 'NSPR', 'FFTW'], obj=[
                 'dxf2egg_dxfToEgg.obj',
                 'libdxfegg.lib',
                 'libdxf.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggToDXF.cxx', obj='egg2dxf_eggToDXF.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggToDXFLayer.cxx', obj='egg2dxf_eggToDXFLayer.obj')
    EnqueueLink(dll='egg2dxf.exe', opts=['ADVAPI', 'NSPR', 'FFTW'], obj=[
                 'egg2dxf_eggToDXF.obj',
                 'egg2dxf_eggToDXFLayer.obj',
                 'libdxf.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandatool/src/palettizer/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/palettizer']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/palettizer')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='palettizer_composite1.cxx', obj='palettizer_composite1.obj')
    EnqueueLib(lib='libpalettizer.lib', obj=['palettizer_composite1.obj'])

#
# DIRECTORY: pandatool/src/egg-mkfont/
#

if (OMIT.count("FREETYPE")==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/egg-mkfont', 'pandatool/src/palettizer']
    OPTS=['NSPR', 'FREETYPE']
    CopyAllHeaders('pandatool/src/egg-mkfont')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggMakeFont.cxx', obj='egg-mkfont_eggMakeFont.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='rangeDescription.cxx', obj='egg-mkfont_rangeDescription.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='rangeIterator.cxx', obj='egg-mkfont_rangeIterator.obj')
    EnqueueLink(dll='egg-mkfont.exe', opts=['ADVAPI', 'NSPR', 'FREETYPE'], obj=[
                 'egg-mkfont_eggMakeFont.obj',
                 'egg-mkfont_rangeDescription.obj',
                 'egg-mkfont_rangeIterator.obj',
                 'libpalettizer.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandatool/src/eggcharbase/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/eggcharbase']
    OPTS=['ZLIB', 'NSPR']
    CopyAllHeaders('pandatool/src/eggcharbase')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggcharbase_composite1.cxx', obj='eggcharbase_composite1.obj')
    EnqueueLib(lib='libeggcharbase.lib', obj=['eggcharbase_composite1.obj'])

#
# DIRECTORY: pandatool/src/egg-optchar/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/egg-optchar']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/egg-optchar')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='config_egg_optchar.cxx', obj='egg-optchar_config_egg_optchar.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggOptchar.cxx', obj='egg-optchar_eggOptchar.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggOptcharUserData.cxx', obj='egg-optchar_eggOptcharUserData.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='vertexMembership.cxx', obj='egg-optchar_vertexMembership.obj')
    EnqueueLink(dll='egg-optchar.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'egg-optchar_config_egg_optchar.obj',
                 'egg-optchar_eggOptchar.obj',
                 'egg-optchar_eggOptcharUserData.obj',
                 'egg-optchar_vertexMembership.obj',
                 'libeggcharbase.lib',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandatool/src/egg-palettize/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/egg-palettize', 'pandatool/src/palettizer']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/egg-palettize')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggPalettize.cxx', obj='egg-palettize_eggPalettize.obj')
    EnqueueLink(dll='egg-palettize.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'egg-palettize_eggPalettize.obj',
                 'libpalettizer.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandatool/src/egg-qtess/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/egg-qtess']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/egg-qtess')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='egg-qtess_composite1.cxx', obj='egg-qtess_composite1.obj')
    EnqueueLink(dll='egg-qtess.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'egg-qtess_composite1.obj',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libconverter.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    
#
# DIRECTORY: pandatool/src/eggprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/eggprogs']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/eggprogs')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggCrop.cxx', obj='egg-crop_eggCrop.obj')
    EnqueueLink(dll='egg-crop.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'egg-crop_eggCrop.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggMakeTube.cxx', obj='egg-make-tube_eggMakeTube.obj')
    EnqueueLink(dll='egg-make-tube.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'egg-make-tube_eggMakeTube.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggTextureCards.cxx', obj='egg-texture-cards_eggTextureCards.obj')
    EnqueueLink(dll='egg-texture-cards.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'egg-texture-cards_eggTextureCards.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggTopstrip.cxx', obj='egg-topstrip_eggTopstrip.obj')
    EnqueueLink(dll='egg-topstrip.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'egg-topstrip_eggTopstrip.obj',
                 'libeggcharbase.lib',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggTrans.cxx', obj='egg-trans_eggTrans.obj')
    EnqueueLink(dll='egg-trans.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'egg-trans_eggTrans.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggToC.cxx', obj='egg2c_eggToC.obj')
    EnqueueLink(dll='egg2c.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'egg2c_eggToC.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandatool/src/flt/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/flt']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/flt')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltVectorRecord.cxx', obj='flt_fltVectorRecord.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='flt_composite1.cxx', obj='flt_composite1.obj')
    EnqueueLib(lib='libflt.lib', obj=['flt_fltVectorRecord.obj', 'flt_composite1.obj'])

#
# DIRECTORY: pandatool/src/fltegg/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/fltegg']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/fltegg')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltToEggConverter.cxx', obj='fltegg_fltToEggConverter.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltToEggLevelState.cxx', obj='fltegg_fltToEggLevelState.obj')
    EnqueueLib(lib='libfltegg.lib', obj=['fltegg_fltToEggConverter.obj', 'fltegg_fltToEggLevelState.obj'])

#
# DIRECTORY: pandatool/src/fltprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/fltprogs', 'pandatool/src/flt', 'pandatool/src/cvscopy']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/fltprogs')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggToFlt.cxx', obj='egg2flt_eggToFlt.obj')
    EnqueueLink(dll='egg2flt.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'egg2flt_eggToFlt.obj',
                 'libflt.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libconverter.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltInfo.cxx', obj='flt-info_fltInfo.obj')
    EnqueueLink(dll='flt-info.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'flt-info_fltInfo.obj',
                 'libprogbase.lib',
                 'libflt.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltTrans.cxx', obj='flt-trans_fltTrans.obj')
    EnqueueLink(dll='flt-trans.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'flt-trans_fltTrans.obj',
                 'libprogbase.lib',
                 'libflt.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltToEgg.cxx', obj='flt2egg_fltToEgg.obj')
    EnqueueLink(dll='flt2egg.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'flt2egg_fltToEgg.obj',
                 'libflt.lib',
                 'libfltegg.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libconverter.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltCopy.cxx', obj='fltcopy_fltCopy.obj')
    EnqueueLink(dll='fltcopy.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'fltcopy_fltCopy.obj',
                 'libcvscopy.lib',
                 'libflt.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandatool/src/imagebase/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/imagebase']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/imagebase')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='imagebase_composite1.cxx', obj='imagebase_composite1.obj')
    EnqueueLib(lib='libimagebase.lib', obj=['imagebase_composite1.obj'])

#
# DIRECTORY: pandatool/src/imageprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/imageprogs']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/imageprogs')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='imageInfo.cxx', obj='image-info_imageInfo.obj')
    EnqueueLink(dll='image-info.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'image-info_imageInfo.obj',
                 'libimagebase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='imageResize.cxx', obj='image-resize_imageResize.obj')
    EnqueueLink(dll='image-resize.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'image-resize_imageResize.obj',
                 'libimagebase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='imageTrans.cxx', obj='image-trans_imageTrans.obj')
    EnqueueLink(dll='image-trans.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'image-trans_imageTrans.obj',
                 'libimagebase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandatool/src/lwo/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/lwo']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/lwo')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='lwo_composite1.cxx', obj='lwo_composite1.obj')
    EnqueueLib(lib='liblwo.lib', obj=['lwo_composite1.obj'])

#
# DIRECTORY: pandatool/src/lwoegg/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/lwoegg']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/lwoegg')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='lwoegg_composite1.cxx', obj='lwoegg_composite1.obj')
    EnqueueLib(lib='liblwoegg.lib', obj=['lwoegg_composite1.obj'])

#
# DIRECTORY: pandatool/src/lwoprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/lwoprogs', 'pandatool/src/lwo']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/lwoprogs')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='lwoScan.cxx', obj='lwo-scan_lwoScan.obj')
    EnqueueLink(dll='lwo-scan.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'lwo-scan_lwoScan.obj',
                 'liblwo.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='lwoToEgg.cxx', obj='lwo2egg_lwoToEgg.obj')
    EnqueueLink(dll='lwo2egg.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'lwo2egg_lwoToEgg.obj',
                 'liblwo.lib',
                 'liblwoegg.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandatool/src/maya/
#

for VER in ["5","6","65"]:
  if (OMIT.count("MAYA"+VER)==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/maya']
    OPTS=['MAYA'+VER, 'NSPR']
    CopyAllHeaders('pandatool/src/maya')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='maya_composite1.cxx',    obj='maya'+VER+'_composite1.obj')
    EnqueueLib(lib='libmaya'+VER+'.lib', obj=[ 'maya'+VER+'_composite1.obj' ])
    
#
# DIRECTORY: pandatool/src/mayaegg/
#

for VER in ["5","6","65"]:
  if (OMIT.count("MAYA"+VER)==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/mayaegg', 'pandatool/src/maya']
    OPTS=['MAYA'+VER, 'NSPR']
    CopyAllHeaders('pandatool/src/mayaegg')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaEggLoader.cxx', obj='mayaegg'+VER+'_loader.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaegg_composite1.cxx',   obj='mayaegg'+VER+'_composite1.obj')
    EnqueueLib(lib='libmayaegg'+VER+'.lib', obj=[ 'mayaegg'+VER+'_composite1.obj' ])

#
# DIRECTORY: pandatool/src/maxegg/
#

for VER in ["6", "7"]:
  if (OMIT.count("MAX"+VER)==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/maxegg']
    OPTS=['MAX'+VER, 'NSPR', "WINCOMCTL", "WINCOMDLG", "WINUSER", "MSFORSCOPE"]
    CopyAllHeaders('pandatool/src/maxegg')
    CopyFile("built/tmp/maxEgg.obj", "pandatool/src/maxegg/maxEgg.obj")
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='maxEggLoader.cxx',obj='maxegg'+VER+'_loader.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='maxegg_composite1.cxx',obj='maxegg'+VER+'_composite1.obj')
    EnqueueLink(opts=OPTS, dll='maxegg'+VER+'.dlo', ldef="pandatool/src/maxegg/maxEgg.def", obj=[
                'maxegg'+VER+'_composite1.obj',
                'maxEgg.obj',
                'libeggbase.lib',
                'libprogbase.lib',
                'libpandatoolbase.lib',
                'libconverter.lib',
                'libpandaegg.dll',
                'libpanda.dll',
                'libpandaexpress.dll',
                'libdtoolconfig.dll',
                'libdtool.dll',
                'libpystub.dll'
               ])

#
# DIRECTORY: pandatool/src/maxprogs/
#

for VER in ["6", "7"]:
  if (OMIT.count("MAX"+VER)==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/maxprogs']
    OPTS=['MAX'+VER, 'NSPR', "WINCOMCTL", "WINCOMDLG", "WINUSER", "MSFORSCOPE"]
    CopyAllHeaders('pandatool/src/maxprogs')
    CopyFile("built/tmp/maxImportRes.obj", "pandatool/src/maxprogs/maxImportRes.obj")
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='maxEggImport.cxx',obj='maxprogs'+VER+'_maxeggimport.obj')
    EnqueueLink(opts=OPTS, dll='maxeggimport'+VER+'.dle', ldef="pandatool/src/maxprogs/maxEggImport.def", obj=[
                'maxegg'+VER+'_loader.obj',
                'maxprogs'+VER+'_maxeggimport.obj',
                'maxImportRes.obj',
                'libpandaegg.dll',
                'libpanda.dll',
                'libpandaexpress.dll',
                'libdtoolconfig.dll',
                'libdtool.dll',
                'libpystub.dll'
               ])

#
# DIRECTORY: pandatool/src/vrml/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/vrml']
    OPTS=['ZLIB', 'NSPR']
    CopyAllHeaders('pandatool/src/vrml')
    EnqueueBison(ipath=IPATH, opts=OPTS, pre='vrmlyy', src='vrmlParser.yxx', dsth='vrmlParser.h', obj='pvrml_vrmlParser.obj')
    EnqueueFlex(ipath=IPATH, opts=OPTS, pre='vrmlyy', src='vrmlLexer.lxx', obj='pvrml_vrmlLexer.obj', dashi=0)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='parse_vrml.cxx', obj='pvrml_parse_vrml.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='standard_nodes.cxx', obj='pvrml_standard_nodes.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='vrmlNode.cxx', obj='pvrml_vrmlNode.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='vrmlNodeType.cxx', obj='pvrml_vrmlNodeType.obj')
    EnqueueLib(lib='libpvrml.lib', obj=[
                 'pvrml_parse_vrml.obj',
                 'pvrml_standard_nodes.obj',
                 'pvrml_vrmlNode.obj',
                 'pvrml_vrmlNodeType.obj',
                 'pvrml_vrmlParser.obj',
                 'pvrml_vrmlLexer.obj',
    ])

#
# DIRECTORY: pandatool/src/vrmlegg/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/vrmlegg', 'pandatool/src/vrml']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/vrmlegg')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='indexedFaceSet.cxx', obj='vrmlegg_indexedFaceSet.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='vrmlAppearance.cxx', obj='vrmlegg_vrmlAppearance.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='vrmlToEggConverter.cxx', obj='vrmlegg_vrmlToEggConverter.obj')
    EnqueueLib(lib='libvrmlegg.lib', obj=[
                 'vrmlegg_indexedFaceSet.obj',
                 'vrmlegg_vrmlAppearance.obj',
                 'vrmlegg_vrmlToEggConverter.obj',
    ])

#
# DIRECTORY: pandatool/src/xfile/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/xfile']
    OPTS=['ZLIB', 'NSPR']
    CopyAllHeaders('pandatool/src/xfile')
    EnqueueBison(ipath=IPATH, opts=OPTS, pre='xyy', src='xParser.yxx', dsth='xParser.h', obj='xfile_xParser.obj')
    EnqueueFlex(ipath=IPATH, opts=OPTS, pre='xyy', src='xLexer.lxx', obj='xfile_xLexer.obj', dashi=1)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='xfile_composite1.cxx', obj='xfile_composite1.obj')
    EnqueueLib(lib='libxfile.lib', obj=[
                 'xfile_composite1.obj',
                 'xfile_xParser.obj',
                 'xfile_xLexer.obj',
    ])

#
# DIRECTORY: pandatool/src/xfileegg/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/xfileegg', 'pandatool/src/xfile']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/xfileegg')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='xfileegg_composite1.cxx', obj='xfileegg_composite1.obj')
    EnqueueLib(lib='libxfileegg.lib', obj=[
                 'xfileegg_composite1.obj',
    ])

#
# DIRECTORY: pandatool/src/ptloader/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/ptloader', 'pandatool/src/flt', 'pandatool/src/lwo', 'pandatool/src/xfile', 'pandatool/src/xfileegg']
    OPTS=['BUILDING_PTLOADER', 'NSPR']
    CopyAllHeaders('pandatool/src/ptloader')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='config_ptloader.cxx', obj='ptloader_config_ptloader.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='loaderFileTypePandatool.cxx', obj='ptloader_loaderFileTypePandatool.obj')
    EnqueueLink(dll='libptloader.dll', opts=['ADVAPI', 'NSPR'], obj=[
                 'ptloader_config_ptloader.obj',
                 'ptloader_loaderFileTypePandatool.obj',
                 'libfltegg.lib',
                 'libflt.lib',
                 'liblwoegg.lib',
                 'liblwo.lib',
                 'libdxfegg.lib',
                 'libdxf.lib',
                 'libvrmlegg.lib',
                 'libpvrml.lib',
                 'libxfileegg.lib',
                 'libxfile.lib',
                 'libconverter.lib',
                 'libpandatoolbase.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
    ])

#
# DIRECTORY: pandatool/src/mayaprogs/
#

for VER in ["5","6","65"]:
  if (OMIT.count('MAYA'+VER)==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/mayaprogs', 'pandatool/src/maya', 'pandatool/src/mayaegg',
           'pandatool/src/cvscopy']
    OPTS=['BUILDING_MISC', 'MAYA'+VER, 'NSPR']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaEggImport.cxx', obj='mayaeggimport'+VER+'_mayaeggimport.obj')
    EnqueueLink(opts=OPTS, dll='mayaeggimport'+VER+'.mll', obj=[
                'mayaegg'+VER+'_loader.obj',
                'mayaeggimport'+VER+'_mayaeggimport.obj',
                'libpandaegg.dll',
                'libpanda.dll',
                'libpandaexpress.dll',
                'libdtoolconfig.dll',
                'libdtool.dll',
                'libpystub.dll'
               ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='config_mayaloader.cxx', obj='mayaloader'+VER+'_config_mayaloader.obj')
    EnqueueLink(dll='libmayaloader'+VER+'.dll',                 opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
                 'mayaloader'+VER+'_config_mayaloader.obj',
                 'libmayaegg'+VER+'.lib',
                 'libptloader.lib',
                 'libconverter.lib',
                 'libpandatoolbase.lib',
                 'libmaya'+VER+'.lib',
                 'libfltegg.lib',
                 'libflt.lib',
                 'liblwoegg.lib',
                 'liblwo.lib',
                 'libdxfegg.lib',
                 'libdxf.lib',
                 'libvrmlegg.lib',
                 'libpvrml.lib',
                 'libxfileegg.lib',
                 'libxfile.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaPview.cxx', obj='mayapview'+VER+'_mayaPview.obj')
    EnqueueLink(dll='libmayapview'+VER+'.mll', opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
                 'mayapview'+VER+'_mayaPview.obj',
                 'libmayaegg'+VER+'.lib',
                 'libmaya'+VER+'.lib',
                 'libconverter.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libframework.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaToEgg.cxx', obj='maya2egg'+VER+'_mayaToEgg.obj')
    EnqueueLink(dll='maya2egg'+VER+'.exe',                 opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
                 'maya2egg'+VER+'_mayaToEgg.obj',
                 'libmayaegg'+VER+'.lib',
                 'libmaya'+VER+'.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libconverter.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaCopy.cxx', obj='mayacopy'+VER+'_mayaCopy.obj')
    EnqueueLink(dll='mayacopy'+VER+'.exe',  opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
                 'mayacopy'+VER+'_mayaCopy.obj',
                 'libcvscopy.lib',
                 'libmaya'+VER+'.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaSavePview.cxx', obj='mayasavepview'+VER+'_mayaSavePview.obj')
    EnqueueLink(dll='libmayasavepview'+VER+'.mll', opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
                 'mayasavepview'+VER+'_mayaSavePview.obj',
    ])


#
# DIRECTORY: pandatool/src/miscprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/miscprogs']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/miscprogs')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='binToC.cxx', obj='bin2c_binToC.obj')
    EnqueueLink(dll='bin2c.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'bin2c_binToC.obj',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandatool/src/pstatserver/
#

if (OMIT.count("NSPR")==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/pstatserver']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/pstatserver')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pstatserver_composite1.cxx', obj='pstatserver_composite1.obj')
    EnqueueLib(lib='libpstatserver.lib', obj=[ 'pstatserver_composite1.obj' ])

#
# DIRECTORY: pandatool/src/softprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/softprogs']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/softprogs')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='softCVS.cxx', obj='softcvs_softCVS.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='softFilename.cxx', obj='softcvs_softFilename.obj')
    EnqueueLink(opts=['ADVAPI', 'NSPR'], dll='softcvs.exe', obj=[
                 'softcvs_softCVS.obj',
                 'softcvs_softFilename.obj',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandatool/src/text-stats/
#

if (OMIT.count("NSPR")==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/text-stats']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/text-stats')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='textMonitor.cxx', obj='text-stats_textMonitor.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='textStats.cxx', obj='text-stats_textStats.obj')
    EnqueueLink(opts=['ADVAPI', 'NSPR'], dll='text-stats.exe', obj=[
                 'text-stats_textMonitor.obj',
                 'text-stats_textStats.obj',
                 'libprogbase.lib',
                 'libpstatserver.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandatool/src/vrmlprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/vrmlprogs', 'pandatool/src/vrml', 'pandatool/src/vrmlegg']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/vrmlprogs')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='vrmlTrans.cxx', obj='vrml-trans_vrmlTrans.obj')
    EnqueueLink(opts=['ADVAPI', 'NSPR'], dll='vrml-trans.exe', obj=[
                 'vrml-trans_vrmlTrans.obj',
                 'libprogbase.lib',
                 'libpvrml.lib',
                 'libpandatoolbase.lib',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='vrmlToEgg.cxx', obj='vrml2egg_vrmlToEgg.obj')
    EnqueueLink(opts=['ADVAPI', 'NSPR'], dll='vrml2egg.exe', obj=[
                 'vrml2egg_vrmlToEgg.obj',
                 'libvrmlegg.lib',
                 'libpvrml.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandatool/src/win-stats/
#

if (OMIT.count("NSPR")==0) and (OMIT.count("PANDATOOL")==0) and (sys.platform == "win32"):
    IPATH=['pandatool/src/win-stats']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/win-stats')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='winstats_composite1.cxx', obj='pstats_composite1.obj')
    EnqueueLink(opts=['WINSOCK', 'WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM', 'NSPR'],
                dll='pstats.exe', obj=[
                'pstats_composite1.obj',
                'libprogbase.lib',
                'libpstatserver.lib',
                'libpandatoolbase.lib',
                'libpandaexpress.dll',
                'libpanda.dll',
                'libdtoolconfig.dll',
                'libdtool.dll',
                'libpystub.dll',
                ])

#
# DIRECTORY: pandatool/src/xfileprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/xfileprogs', 'pandatool/src/xfile', 'pandatool/src/xfileegg']
    OPTS=['NSPR']
    CopyAllHeaders('pandatool/src/xfileprogs')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggToX.cxx', obj='egg2x_eggToX.obj')
    EnqueueLink(dll='egg2x.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'egg2x_eggToX.obj',
                 'libxfileegg.lib',
                 'libxfile.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='xFileTrans.cxx', obj='x-trans_xFileTrans.obj')
    EnqueueLink(dll='x-trans.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'x-trans_xFileTrans.obj',
                 'libprogbase.lib',
                 'libxfile.lib',
                 'libpandatoolbase.lib',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='xFileToEgg.cxx', obj='x2egg_xFileToEgg.obj')
    EnqueueLink(opts=['ADVAPI', 'NSPR'], dll='x2egg.exe', obj=[
                 'x2egg_xFileToEgg.obj',
                 'libxfileegg.lib',
                 'libxfile.lib',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandaapp/src/pandaappbase/
#

if (OMIT.count("PANDAAPP")==0):
    IPATH=['pandaapp/src/pandaappbase']
    OPTS=['NSPR']
    CopyAllHeaders('pandaapp/src/pandaappbase')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandaappbase.cxx', obj='pandaappbase_pandaappbase.obj')
    EnqueueLib(lib='libpandaappbase.lib', obj=['pandaappbase_pandaappbase.obj'])

#
# DIRECTORY: pandaapp/src/httpbackup/
#

if (OMIT.count("OPENSSL")==0) and (OMIT.count("PANDAAPP")==0):
    IPATH=['pandaapp/src/httpbackup', 'pandaapp/src/pandaappbase']
    OPTS=['OPENSSL', 'NSPR']
    CopyAllHeaders('pandaapp/src/httpbackup')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='backupCatalog.cxx', obj='httpbackup_backupCatalog.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='httpBackup.cxx', obj='httpbackup_httpBackup.obj')
    EnqueueLink(opts=['ADVAPI', 'NSPR', 'OPENSSL'], dll='httpbackup.exe', obj=[
                 'httpbackup_backupCatalog.obj',
                 'httpbackup_httpBackup.obj',
                 'libpandaappbase.lib',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libdtool.dll',
                 'libdtoolconfig.dll',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandaapp/src/indexify/
#

if (OMIT.count("FREETYPE")==0) and (OMIT.count("PANDAAPP")==0):
    IPATH=['pandaapp/src/indexify']
    OPTS=['NSPR', 'FREETYPE']
    CopyAllHeaders('pandaapp/src/indexify')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='default_font.cxx', obj='font-samples_default_font.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fontSamples.cxx', obj='font-samples_fontSamples.obj')
    EnqueueLink(opts=['ADVAPI', 'NSPR', 'FREETYPE'], dll='font-samples.exe', obj=[
                 'font-samples_default_font.obj',
                 'font-samples_fontSamples.obj',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtool.dll',
                 'libdtoolconfig.dll',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='default_index_icons.cxx', obj='indexify_default_index_icons.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='default_font.cxx', obj='indexify_default_font.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='indexImage.cxx', obj='indexify_indexImage.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='indexParameters.cxx', obj='indexify_indexParameters.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='indexify.cxx', obj='indexify_indexify.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='photo.cxx', obj='indexify_photo.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='rollDirectory.cxx', obj='indexify_rollDirectory.obj')
    EnqueueLink(opts=['ADVAPI', 'NSPR', 'FREETYPE'], dll='indexify.exe', obj=[
                 'indexify_default_index_icons.obj',
                 'indexify_default_font.obj',
                 'indexify_indexImage.obj',
                 'indexify_indexParameters.obj',
                 'indexify_indexify.obj',
                 'indexify_photo.obj',
                 'indexify_rollDirectory.obj',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtool.dll',
                 'libdtoolconfig.dll',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpystub.dll',
    ])

#
# DIRECTORY: pandaapp/src/stitchbase/
#

if (OMIT.count("PANDAAPP")==0):
    IPATH=['pandaapp/src/stitchbase', 'pandaapp/src/pandaappbase']
    OPTS=['NSPR']
    CopyAllHeaders('pandaapp/src/stitchbase')
    EnqueueBison(ipath=IPATH, opts=OPTS, pre='stitchyy', src='stitchParser.yxx', dsth='stitchParser.h', obj='stitchbase_stitchParser.obj')
    EnqueueFlex(ipath=IPATH, opts=OPTS, pre='stitchyy', src='stitchLexer.lxx', obj='stitchbase_stitchLexer.cxx', dashi=1)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='stitchbase_composite1.cxx', obj='stitchbase_composite1.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='stitchbase_composite2.cxx', obj='stitchbase_composite2.obj')
    EnqueueLib(lib='libstitchbase.lib', obj=[
                 'stitchbase_composite1.obj',
                 'stitchbase_composite2.obj',
                 'stitchbase_stitchParser.obj',
                 'stitchbase_stitchLexer.obj',
    ])

#
# DIRECTORY: pandaapp/src/stitch/
#

if (OMIT.count("PANDAAPP")==0):
    IPATH=['pandaapp/src/stitch', 'pandaapp/src/stitchbase', 'pandaapp/src/pandaappbase']
    OPTS=['NSPR']
    CopyAllHeaders('pandaapp/src/stitch')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='stitchCommandProgram.cxx', obj='stitch-command_stitchCommandProgram.obj')
    EnqueueLink(opts=['ADVAPI', 'NSPR', 'FFTW'], dll='stitch-command.exe', obj=[
                 'stitch-command_stitchCommandProgram.obj',
                 'libstitchbase.lib',
                 'libpandaappbase.lib',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpystub.dll',
    ])
    
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='stitchImageProgram.cxx', obj='stitch-image_stitchImageProgram.obj')
    EnqueueLink(opts=['ADVAPI', 'NSPR', 'FFTW'], dll='stitch-image.exe', obj=[
                 'stitch-image_stitchImageProgram.obj',
                 'libstitchbase.lib',
                 'libpandaappbase.lib',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpystub.dll',
    ])


#
# Generate the models directory
#

if (OMIT.count("PANDATOOL")==0):
    EnqueueBam("../=", "built/models/gui/dialog_box_gui.bam",  "dmodels/src/gui/dialog_box_gui.flt")
    EnqueueBam("../=", "built/models/misc/camera.bam",         "dmodels/src/misc/camera.flt")
    EnqueueBam("../=", "built/models/misc/fade.bam",           "dmodels/src/misc/fade.flt")
    EnqueueBam("../=", "built/models/misc/fade_sphere.bam",    "dmodels/src/misc/fade_sphere.flt")
    EnqueueBam("../=", "built/models/misc/gridBack.bam",       "dmodels/src/misc/gridBack.flt")
    EnqueueBam("../=", "built/models/misc/iris.bam",           "dmodels/src/misc/iris.flt")
    EnqueueBam("../=", "built/models/misc/lilsmiley.bam",      "dmodels/src/misc/lilsmiley.egg")
    EnqueueBam("../=", "built/models/misc/objectHandles.bam",  "dmodels/src/misc/objectHandles.flt")
    EnqueueBam("../=", "built/models/misc/rgbCube.bam",        "dmodels/src/misc/rgbCube.flt")
    EnqueueBam("../=", "built/models/misc/smiley.bam",         "dmodels/src/misc/smiley.egg")
    EnqueueBam("../=", "built/models/misc/sphere.bam",         "dmodels/src/misc/sphere.flt")
    EnqueueBam("../=", "built/models/misc/Pointlight.bam",     "dmodels/src/misc/Pointlight.egg")
    EnqueueBam("../=", "built/models/misc/Dirlight.bam",       "dmodels/src/misc/Dirlight.egg")
    EnqueueBam("../=", "built/models/misc/Spotlight.bam",      "dmodels/src/misc/Spotlight.egg")
    EnqueueBam("../=", "built/models/misc/xyzAxis.bam",        "dmodels/src/misc/xyzAxis.flt")
    
    CopyAllFiles("built/models/audio/sfx/",  "dmodels/src/audio/sfx/", ".wav")
    CopyAllFiles("built/models/icons/",      "dmodels/src/icons/",     ".gif")
    
    CopyAllFiles("built/models/",            "models/",                ".egg")
    CopyAllFiles("built/models/",            "models/",                ".bam")
    
    CopyAllFiles("built/models/maps/",       "models/maps/",           ".jpg")
    CopyAllFiles("built/models/maps/",       "models/maps/",           ".png")
    CopyAllFiles("built/models/maps/",       "models/maps/",           ".rgb")
    CopyAllFiles("built/models/maps/",       "models/maps/",           ".rgba")
    
    CopyAllFiles("built/models/maps/",       "dmodels/src/maps/",      ".jpg")
    CopyAllFiles("built/models/maps/",       "dmodels/src/maps/",      ".png")
    CopyAllFiles("built/models/maps/",       "dmodels/src/maps/",      ".rgb")
    CopyAllFiles("built/models/maps/",       "dmodels/src/maps/",      ".rgba")

##########################################################################################
#
# Dependency-Based Distributed Build System.
#
##########################################################################################

if (SLAVEBUILD!=0):
    for task in DEPENDENCYQUEUE:
        targets=task[2]
        if (targets.count(SLAVEBUILD)):
            apply(task[0], task[1])
    sys.exit(0)

def BuildWorker(taskqueue, donequeue, slave):
    print "Slave online: "+slave
    while (1):
        task = taskqueue.get()
        sys.stdout.flush()
        if (task == 0): return
        if (slave=="local"):
            apply(task[0],task[1])
        else:
            targetzero = task[2][0]
            
            oscmd(slave.replace("OPTIONS","--slavebuild "+targetzero))
        donequeue.put(task)

def AllSourcesReady(task, pending):
    sources = task[3]
    for x in sources:
        if (pending.has_key(x)):
            return 0
    return 1

def ParallelMake(tasklist):
    global BUILTANYTHING
    # Read the slave-file.
    slaves = []
    if (SLAVEFILE!=0):
        try:
            slavefile = open(SLAVEFILE, "r")
            for line in slavefile:
                line = line.strip(" \t\r\n")
                if (line != ""): slaves.append(line)
        except: exit("Could not read slave-file")
    else:
        for i in range(THREADCOUNT):
            slaves.append("local")
    # create the communication queues.
    donequeue=Queue.Queue()
    taskqueue=Queue.Queue()
    # build up a table listing all the pending targets
    pending = {}
    for task in tasklist:
        for target in task[2]:
            pending[target] = 1
    # create the workers
    for slave in slaves:
        th = threading.Thread(target=BuildWorker, args=[taskqueue,donequeue,slave])
        th.setDaemon(1)
        th.start()
    # feed tasks to the workers.
    tasksqueued = 0
    while (1):
        if (tasksqueued < len(slaves)*2):
            extras = []
            for task in tasklist:
                if (tasksqueued < len(slaves)*3) & (AllSourcesReady(task, pending)):
                    if (older(task[2], task[3])):
                        tasksqueued += 1
                        BUILTANYTHING=1
                        taskqueue.put(task)
                    else:
                        for target in task[2]:
                            del pending[target]
                else:
                    extras.append(task)
            tasklist = extras
        sys.stdout.flush()
        if (tasksqueued == 0): break
        donetask = donequeue.get()
        sys.stdout.flush()
        tasksqueued -= 1
        for target in donetask[2]:
            updatefiledate(target)
            del pending[target]
    # kill the workers.
    for slave in slaves:
        taskqueue.put(0)
    # make sure there aren't any unsatisfied tasks
    if (len(tasklist)>0):
        exit("Dependency problem - task unsatisfied: "+str(tasklist[0][2]))

def SequentialMake(tasklist):
    global BUILTANYTHING
    for task in tasklist:
        if (older(task[2], task[3])):
            BUILTANYTHING=1
            apply(task[0], task[1])
            for target in task[2]:
                updatefiledate(target)

def RunDependencyQueue(tasklist):
    if (SLAVEFILE!=0) or (THREADCOUNT!=0):
        ParallelMake(tasklist)
    else:
        SequentialMake(tasklist)

RunDependencyQueue(DEPENDENCYQUEUE)

##########################################################################################
#
# Copy Sounds, Icons, and Models into the build.
#
##########################################################################################


##########################################################################################
#
# Run genpycode
#
##########################################################################################

if (OMIT.count("PYTHON")==0):
    if (older(['built/pandac/PandaModules.py'],xpaths("built/pandac/input/",ALLIN,""))):
        if (GENMAN): oscmd("built/bin/genpycode -m")
        else       : oscmd("built/bin/genpycode")
        updatefiledate('built/pandac/PandaModules.py')

########################################################################
##
## Save the CXX include-cache for next time.
##
########################################################################

try: icache = open(iCachePath,'wb')
except: icache = 0
if (icache!=0):
    cPickle.dump(CXXINCLUDECACHE, icache, 1)
    icache.close()

##########################################################################################
#
# The Installers
#
# Under windows, we can build an 'exe' package using NSIS
# Under linux, we can build an 'deb' package using dpkg-deb
# Makepanda does not build RPMs. To do that, use 'rpm -tb' on the source tarball.
#
##########################################################################################

def MakeInstallerNSIS(file,fullname,smdirectory,installdir):
    print "Building "+fullname+" installer. This can take up to an hour."
    if (COMPRESSOR != "lzma"):
        print("Note: you are using zlib, which is faster, but lzma gives better compression.")
    if (os.path.exists(file)):
        os.remove(file)
    if (os.path.exists("nsis-output.exe")):
        os.remove("nsis-output.exe")
    psource=os.path.abspath(".")
    panda=os.path.abspath("built")
    cmd="thirdparty/win-nsis/makensis.exe /V2 "
    cmd=cmd+'/DCOMPRESSOR="'+COMPRESSOR+'" '
    cmd=cmd+'/DNAME="'+fullname+'" '
    cmd=cmd+'/DSMDIRECTORY="'+smdirectory+'" '
    cmd=cmd+'/DINSTALLDIR="'+installdir+'" '
    cmd=cmd+'/DOUTFILE="'+psource+'\\nsis-output.exe" '
    cmd=cmd+'/DLICENSE="'+panda+'\\LICENSE" '
    cmd=cmd+'/DLANGUAGE="Panda3DEnglish" '
    cmd=cmd+'/DRUNTEXT="Run the Panda Greeting Card" '
    cmd=cmd+'/DIBITMAP="panda-install.bmp" '
    cmd=cmd+'/DUBITMAP="panda-uninstall.bmp" '
    cmd=cmd+'/DPANDA="'+panda+'" '
    cmd=cmd+'/DPSOURCE="'+psource+'" '
    cmd=cmd+'/DPYEXTRAS="'+psource+'\\thirdparty\\win-extras" '
    cmd=cmd+'"'+psource+'\\direct\\src\\directscripts\\packpanda.nsi"'
    oscmd( cmd)
    os.rename("nsis-output.exe", file)

def MakeInstallerDPKG(file):
    DEB="""
Package: panda3d
Version: VERSION
Section: libdevel
Priority: optional
Architecture: i386
Essential: no
Depends: PYTHONV
Provides: panda3d
Maintainer: etc-panda3d@lists.andrew.cmu.edu
Description: The panda3D free 3D engine
"""
    import compileall
    PYTHONV=os.path.basename(PYTHONSDK)
    if (os.path.isdir("debtmp")): oscmd( "chmod -R 755 debtmp")
    oscmd("rm -rf debtmp data.tar.gz control.tar.gz ")
    oscmd("mkdir -p debtmp/usr/bin")
    oscmd("mkdir -p debtmp/usr/include")
    oscmd("mkdir -p debtmp/usr/share/panda3d")
    oscmd("mkdir -p debtmp/usr/lib/"+PYTHONV+"/lib-dynload")
    oscmd("mkdir -p debtmp/usr/lib/"+PYTHONV+"/site-packages")
    oscmd("mkdir -p debtmp/etc")
    oscmd("mkdir -p debtmp/DEBIAN")
    oscmd("sed -e 's@$THIS_PRC_DIR/[.][.]@/usr/share/panda3d@' < built/etc/Config.prc > debtmp/etc/Config.prc")
    oscmd("cp built/etc/Confauto.prc  debtmp/etc/Confauto.prc")
    oscmd("cp --recursive built/include debtmp/usr/include/panda3d")
    oscmd("cp --recursive direct        debtmp/usr/share/panda3d/direct")
    oscmd("cp --recursive built/pandac  debtmp/usr/share/panda3d/pandac")
    oscmd("cp --recursive built/Pmw     debtmp/usr/share/panda3d/Pmw")
    oscmd("cp --recursive built/epydoc  debtmp/usr/share/panda3d/epydoc")
    oscmd("cp built/direct/__init__.py  debtmp/usr/share/panda3d/direct/__init__.py")
    oscmd("cp --recursive SceneEditor   debtmp/usr/share/panda3d/SceneEditor")
    oscmd("cp --recursive built/models  debtmp/usr/share/panda3d/models")
    oscmd("cp --recursive samples       debtmp/usr/share/panda3d/samples")
    oscmd("cp doc/LICENSE               debtmp/usr/share/panda3d/LICENSE")
    oscmd("cp doc/LICENSE               debtmp/usr/include/panda3d/LICENSE")
    oscmd("cp doc/ReleaseNotes          debtmp/usr/share/panda3d/ReleaseNotes")
    oscmd("echo '/usr/share/panda3d' >  debtmp/usr/lib/"+PYTHONV+"/site-packages/panda3d.pth")
    oscmd("cp built/bin/*               debtmp/usr/bin/")
    for base in os.listdir("built/lib"):
        oscmd("ln -sf /usr/lib/"+base+" debtmp/usr/lib/"+PYTHONV+"/lib-dynload/"+base)
        oscmd("cp built/lib/"+base+" debtmp/usr/lib/"+base)
    for base in os.listdir("debtmp/usr/share/panda3d/direct/src"):
        if ((base != "extensions") and (base != "extensions_native")):
            compileall.compile_dir("debtmp/usr/share/panda3d/direct/src/"+base)
    compileall.compile_dir("debtmp/usr/share/panda3d/Pmw")
    compileall.compile_dir("debtmp/usr/share/panda3d/epydoc")
    compileall.compile_dir("debtmp/usr/share/panda3d/SceneEditor")
    oscmd("chmod -R 555 debtmp/usr/share/panda3d")
    oscmd("cd debtmp ; (find usr -type f -exec md5sum {} \;) >  DEBIAN/md5sums")
    oscmd("cd debtmp ; (find etc -type f -exec md5sum {} \;) >> DEBIAN/md5sums")
    WriteFile("debtmp/DEBIAN/conffiles","/etc/Config.prc\n")
    WriteFile("debtmp/DEBIAN/control",DEB[1:].replace("VERSION",str(VERSION)).replace("PYTHONV",PYTHONV))
    oscmd("dpkg-deb -b debtmp "+file)
    oscmd("chmod -R 755 debtmp")
    oscmd("rm -rf debtmp")


if ((BUILTANYTHING)&(INSTALLER != 0)):
    if (sys.platform == "win32"):
        MakeInstallerNSIS("Panda3D-"+VERSION+".exe", "Panda3D", "Panda3D "+VERSION, "C:\\Panda3D-"+VERSION)
    elif (sys.platform == "linux2") and (os.path.isfile("/usr/bin/dpkg-deb")):
        MakeInstallerDPKG("panda3d_"+VERSION+"_i386.deb")
    else:
        exit("Do not know how to make an installer for this platform")

##########################################################################################
#
# Print final status report.
#
##########################################################################################

WARNINGS.append("Elapsed Time: "+PrettyTime(time.time() - STARTTIME))
#WARNINGS.append("Time(EnqueueCxx): "+PrettyTime(TIMECOMPILEC))
#WARNINGS.append("Time(CompileLib): "+PrettyTime(TIMECOMPILELIB))
#WARNINGS.append("Time(EnqueueLink): "+PrettyTime(TIMECOMPILELINK))
#WARNINGS.append("Time(EnqueueIgate): "+PrettyTime(TIMEINTERROGATE))
#WARNINGS.append("Time(EnqueueImod): "+PrettyTime(TIMEINTERROGATEMODULE))

printStatus("Makepanda Final Status Report", WARNINGS)

