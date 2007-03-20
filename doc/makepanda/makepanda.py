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

import sys,os,time,stat,string,re,getopt,cPickle,fnmatch,threading,Queue,signal
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

COMPILER=0
THIRDPARTYLIBS=0
OPTIMIZE="3"
INSTALLER=0
GENMAN=0
VERSION=0
VERBOSE=1
COMPRESSOR="zlib"
PACKAGES=["PYTHON","ZLIB","PNG","JPEG","TIFF","VRPN","FMODEX","NVIDIACG",
          "OPENSSL","FREETYPE","FFTW","MILES",
          "MAYA6","MAYA65","MAYA7","MAYA8","MAX6","MAX7","MAX8",
          "BISON","FLEX","FFMPEG","PANDATOOL","PANDAAPP","DX8","DX9"]
OMIT=PACKAGES[:]
WARNINGS=[]
DIRECTXSDK = {}
MAYASDK = {}
MAXSDK = {}
MAXSDKCS = {}
PYTHONSDK=0
STARTTIME=time.time()
SLAVEFILE=0
DEPENDENCYQUEUE=[]
FILEDATECACHE = {}
BUILTFROMCACHE = {}
CXXINCLUDECACHE = {}
THREADCOUNT=0
DXVERSIONS=["8","9"]
MAYAVERSIONS=["6","65","7","8"]
MAXVERSIONS=["6","7","8"]
ICACHEPATH="built/tmp/makepanda-dcache"
INTERRUPT=0
MAINTHREAD=threading.currentThread()

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
## The Dependency cache.
##
## Makepanda's strategy for file dependencies is different from most
## make-utilities.  Whenever a file is built, makepanda records
## that the file was built, and it records what the input files were,
## and what their dates were.  Whenever a file is about to be built,
## panda compares the current list of input files and their dates,
## to the previous list of input files and their dates.  If they match,
## there is no need to build the file.
##
## Also, makepanda automatically calculates dependencies from CXX
## include files.  It does so by scanning the CXX files.  After it scans,
## it records the date of the source file and the list of includes that
## it contains.  It assumes that if the file date hasn't changed, that
## the list of include-statements inside the file has not changed
## either.
##
########################################################################

def SaveDependencyCache():
    try: icache = open(ICACHEPATH,'wb')
    except: icache = 0
    if (icache!=0):
        print "Storing dependency cache."
        cPickle.dump(CXXINCLUDECACHE, icache, 1)
        cPickle.dump(BUILTFROMCACHE, icache, 1)
        icache.close()

def LoadDependencyCache():
    global CXXINCLUDECACHE
    global BUILTFROMCACHE
    try: icache = open(ICACHEPATH,'rb')
    except: icache = 0
    if (icache!=0):
        CXXINCLUDECACHE = cPickle.load(icache)
        BUILTFROMCACHE = cPickle.load(icache)
        icache.close()

LoadDependencyCache()

########################################################################
##
## Error exit has to be done carefully, because the 
## dependency cache must be saved, and because this can
## only be done by the main thread.
##
########################################################################

def exit(msg):
    if (threading.currentThread() == MAINTHREAD):
        SaveDependencyCache()
        print "Elapsed Time: "+PrettyTime(time.time() - STARTTIME)
        print msg
        sys.stdout.flush()
        sys.stderr.flush()
        os._exit(1)
    else:
        print msg
        raise "initiate-exit"

########################################################################
##
## Utility Routines
##
########################################################################

def filedate(path):
    if FILEDATECACHE.has_key(path):
        return FILEDATECACHE[path]
    try: date = os.path.getmtime(path)
    except: date = 0
    FILEDATECACHE[path] = date
    return date

def JustBuilt(files,others):
    dates = []
    for file in files:
        del FILEDATECACHE[file]
        dates.append(filedate(file))
    for file in others:
        dates.append(filedate(file))
    key = tuple(files)
    BUILTFROMCACHE[key] = [others,dates]

def NeedsBuild(files,others):
    dates = []
    for file in files:
        dates.append(filedate(file))
    for file in others:
        dates.append(filedate(file))
    key = tuple(files)
    if (BUILTFROMCACHE.has_key(key)):
        if (BUILTFROMCACHE[key] == [others,dates]):
            return 0
        else:
            oldothers = BUILTFROMCACHE[key][0]
            if (oldothers != others):
                add = SetDifference(others, oldothers)
                sub = SetDifference(oldothers, others)
                for f in files:
                    print "CAUTION: file dependencies changed: "+f
                if (VERBOSE > 1):
                    print " - Add: "+str(add)
                    print " - Sub: "+str(sub)
    return 1

def xpaths(prefix,base,suffix):
    if type(base) == str:
        return prefix + base + suffix
    result = []
    for x in base:
        result.append(xpaths(prefix,x,suffix))
    return result

if sys.platform == "win32":
    import _winreg

    def ListRegistryKeys(path):
        result=[]
        index=0
        try:
            key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, path, 0, _winreg.KEY_READ)
            while (1):
                result.append(_winreg.EnumKey(key, index))
                index = index + 1
        except: pass
        if (key!=0): _winreg.CloseKey(key)
        return result

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
    except: exit("Cannot write "+wfile)

def CreatePlaceHolder(file):
    if (os.path.isfile(file)==0):
        WriteFile(file,"")

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

def DependencyQueue(fn, args, targets, sources, altsrc):
    DEPENDENCYQUEUE.append([fn,args,targets,sources, altsrc])

def keyboardInterruptHandler(x,y):
    exit("keyboard interrupt")

signal.signal(signal.SIGINT, keyboardInterruptHandler)

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
    global OPTIMIZE,OMIT,INSTALLER,GENMAN
    global VERSION,COMPRESSOR,VERBOSE,SLAVEFILE,THREADCOUNT
    longopts = [
        "help","package-info",
        "optimize=","everything","nothing","installer","quiet","verbose",
        "version=","lzma","no-python","slaves=","threads="]
    anything = 0
    for pkg in PACKAGES: longopts.append("no-"+pkg.lower())
    for pkg in PACKAGES: longopts.append("use-"+pkg.lower())
    try:
        opts, extras = getopt.getopt(args, "", longopts)
        for option,value in opts:
            if (option=="--help"): raise "usage"
            elif (option=="--optimize"): OPTIMIZE=value
            elif (option=="--quiet"): VERBOSE-=1
            elif (option=="--verbose"): VERBOSE+=1
            elif (option=="--installer"): INSTALLER=1
            elif (option=="--genman"): GENMAN=1
            elif (option=="--everything"): OMIT=[]
            elif (option=="--nothing"): OMIT=PACKAGES[:]
            elif (option=="--slaves"): SLAVEFILE=value
            elif (option=="--threads"): THREADCOUNT=int(value)
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
    except: usage(0)
    if (anything==0): usage(0)
    if   (OPTIMIZE=="1"): OPTIMIZE=1
    elif (OPTIMIZE=="2"): OPTIMIZE=2
    elif (OPTIMIZE=="3"): OPTIMIZE=3
    elif (OPTIMIZE=="4"): OPTIMIZE=4
    else: usage("Invalid setting for OPTIMIZE")

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
    DIRECTXSDK["DX8"] = "sdks/directx8"
    DIRECTXSDK["DX9"] = "sdks/directx9"
    MAXSDKCS["MAX6"] = "sdks/maxsdk6"
    MAXSDKCS["MAX7"] = "sdks/maxsdk7"
    MAXSDKCS["MAX8"] = "sdks/maxsdk8"
    MAXSDK["MAX6"]   = "sdks/maxsdk6"
    MAXSDK["MAX7"]   = "sdks/maxsdk7"
    MAXSDK["MAX8"]   = "sdks/maxsdk8"
    MAYASDK["MAYA6"]  = "sdks/maya6"
    MAYASDK["MAYA65"] = "sdks/maya65"
    MAYASDK["MAYA7"]  = "sdks/maya7"

########################################################################
##
## Locate the DirectX SDK
##
## Microsoft keeps changing the &*#$*& registry key for the DirectX SDK.
## The only way to reliably find it is to search through the installer's
## uninstall-directories, look in each one, and see if it contains the
## relevant files.
##
########################################################################

if sys.platform == "win32":
    uninstaller = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    for subdir in ListRegistryKeys(uninstaller):
        if (subdir[0]=="{"):
            dir = GetRegistryKey(uninstaller+"\\"+subdir, "InstallLocation")
            if (dir != 0):
                if ((DIRECTXSDK.has_key("DX8")==0) and
                    (os.path.isfile(dir+"\\Include\\d3d8.h")) and
                    (os.path.isfile(dir+"\\Include\\d3dx8.h")) and
                    (os.path.isfile(dir+"\\Lib\\d3d8.lib")) and
                    (os.path.isfile(dir+"\\Lib\\d3dx8.lib"))):
                   DIRECTXSDK["DX8"] = dir.replace("\\", "/").rstrip("/")
                if ((DIRECTXSDK.has_key("DX9")==0) and
                    (os.path.isfile(dir+"\\Include\\d3d9.h")) and
                    (os.path.isfile(dir+"\\Include\\d3dx9.h")) and
                    (os.path.isfile(dir+"\\Include\\dxsdkver.h")) and
                    (os.path.isfile(dir+"\\Lib\\x86\\d3d9.lib")) and
                    (os.path.isfile(dir+"\\Lib\\x86\\d3dx9.lib"))):
                   DIRECTXSDK["DX9"] = dir.replace("\\", "/").rstrip("/")
    for ver in DXVERSIONS:
        if (OMIT.count("DX"+ver)==0):
            if (DIRECTXSDK.has_key("DX"+ver)==0):
                WARNINGS.append("I cannot locate SDK for DX"+ver)
                WARNINGS.append("I have automatically added this command-line option: --no-dx"+ver)
                OMIT.append("DX"+ver)
            else:
                WARNINGS.append("Using DX"+ver+" sdk: "+DIRECTXSDK["DX"+ver])

########################################################################
##
## Locate the Maya SDK
##
########################################################################

MAYAVERSIONINFO=[("MAYA6",  "SOFTWARE\\Alias|Wavefront\\Maya\\6.0\\Setup\\InstallPath"),
                 ("MAYA65", "SOFTWARE\\Alias|Wavefront\\Maya\\6.5\\Setup\\InstallPath"),
                 ("MAYA7",  "SOFTWARE\\Alias|Wavefront\\Maya\\7.0\\Setup\\InstallPath"),
                 ("MAYA8",  "SOFTWARE\\Alias\\Maya\\8.0\\Setup\\InstallPath")
]

for (ver,key) in MAYAVERSIONINFO:
    print "Checking for "+ver
    if (OMIT.count(ver)==0):
        if (sys.platform == "win32"):
            if (MAYASDK.has_key(ver)==0):
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

MAXVERSIONINFO = [("MAX6", "SOFTWARE\\Autodesk\\3DSMAX\\6.0",            "installdir",    "maxsdk\\cssdk\\include"),
                  ("MAX7", "SOFTWARE\\Autodesk\\3DSMAX\\7.0",            "Installdir",    "maxsdk\\include\\CS"),
                  ("MAX8", "SOFTWARE\\Autodesk\\3DSMAX\\8.0",            "Installdir",    "maxsdk\\include\\CS"),
]

for version,key1,key2,subdir in MAXVERSIONINFO:
    if (OMIT.count(version)==0):
        if (sys.platform == "win32"):
            if (MAXSDK.has_key(version)==0):
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
## Choose a Compiler.
##
## This should also set up any environment variables needed to make
## the compiler work.
##
########################################################################


def AddToPathEnv(path,add):
    if (os.environ.has_key(path)):
        os.environ[path] = add + ";" + os.environ[path]
    else:
        os.environ[path] = add

def ChooseCompiler():
    global COMPILER, THIRDPARTYLIBS

    # Try to use Linux GCC

    if (sys.platform[:5] == "linux"):
	COMPILER="LINUX"
	THIRDPARTYLIBS="thirdparty/linux-libs-a/"
	return

    # Try to use Visual Studio 8

    vcdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VC7", "8.0")
    print "VCDIR=",vcdir
    if (vcdir != 0) and (vcdir[-4:] == "\\VC\\"):
	vcdir = vcdir[:-3]
        platsdk=GetRegistryKey("SOFTWARE\\Microsoft\\MicrosoftSDK\\InstalledSDKs\\D2FF9F89-8AA2-4373-8A31-C838BF4DBBE1",
                               "Install Dir")
        if (platsdk == 0): exit("Found VC Toolkit, but cannot locate MS Platform SDK")
	WARNINGS.append("Using visual studio: "+vcdir)
        AddToPathEnv("PATH",    vcdir + "VC\\bin")
        AddToPathEnv("PATH",    vcdir + "Common7\\IDE")
        AddToPathEnv("INCLUDE", vcdir + "VC\\include")
        AddToPathEnv("LIB",     vcdir + "VC\\lib")
        AddToPathEnv("INCLUDE", platsdk + "include")
        AddToPathEnv("LIB",     platsdk + "lib")
	COMPILER="MSVC"
	THIRDPARTYLIBS="thirdparty/win-libs-vc8/"
	return

    # Try to use Visual Studio 7

    vcdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\7.1", "InstallDir")
    if (vcdir == 0):
        vcdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\7.0", "InstallDir")
    if (vcdir != 0) and (vcdir[-13:] == "\\Common7\\IDE\\"):
        vcdir = vcdir[:-12]
        WARNINGS.append("Using visual studio: "+vcdir)
        AddToPathEnv("PATH",    vcdir + "vc7\\bin")
        AddToPathEnv("PATH",    vcdir + "Common7\\IDE")
        AddToPathEnv("PATH",    vcdir + "Common7\\Tools")
        AddToPathEnv("PATH",    vcdir + "Common7\\Tools\\bin\\prerelease")
        AddToPathEnv("PATH",    vcdir + "Common7\\Tools\\bin")
        AddToPathEnv("INCLUDE", vcdir + "vc7\\ATLMFC\\INCLUDE")
        AddToPathEnv("INCLUDE", vcdir + "vc7\\include")
        AddToPathEnv("INCLUDE", vcdir + "vc7\\PlatformSDK\\include\\prerelease")
        AddToPathEnv("INCLUDE", vcdir + "vc7\\PlatformSDK\\include")
        AddToPathEnv("LIB",     vcdir + "vc7\\ATLMFC\\LIB")
        AddToPathEnv("LIB",     vcdir + "vc7\\LIB")
        AddToPathEnv("LIB",     vcdir + "vc7\\PlatformSDK\\lib\\prerelease")
        AddToPathEnv("LIB",     vcdir + "vc7\\PlatformSDK\\lib")
	COMPILER="MSVC"
	THIRDPARTYLIBS="thirdparty/win-libs-vc7/"
        return

    # Try to use the Visual Toolkit 2003

    vcdir = GetRegistryKey("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment","VCToolkitInstallDir")
    if (vcdir != 0) or (os.environ.has_key("VCTOOLKITINSTALLDIR")):
        if (vcdir == 0): vcdir = os.environ["VCTOOLKITINSTALLDIR"]
        platsdk=GetRegistryKey("SOFTWARE\\Microsoft\\MicrosoftSDK\\InstalledSDKs\\8F9E5EF3-A9A5-491B-A889-C58EFFECE8B3",
                               "Install Dir")
        if (platsdk == 0): exit("Found VC Toolkit, but cannot locate MS Platform SDK")
        WARNINGS.append("Using visual toolkit: "+vcdir)
        WARNINGS.append("Using MS Platform SDK: "+platsdk)
        AddToPathEnv("PATH", vcdir + "\\bin")
        AddToPathEnv("INCLUDE", platsdk + "\\include")
        AddToPathEnv("INCLUDE", vcdir + "\\include")
        AddToPathEnv("LIB",     platsdk + "\\lib")
        AddToPathEnv("LIB",     vcdir + "\\lib")
        AddToPathEnv("LIB",     "thirdparty\\win-libs-vc7\\extras\\lib")
	COMPILER="MSVC"
	THIRDPARTYLIBS="thirdparty/win-libs-vc7/"
        return

    # Give up
    exit("Cannot locate Microsoft Visual Studio 7.0, 7.1, or the Visual Toolkit 2003")


ChooseCompiler()

##########################################################################################
#
# Disable packages that are currently broken or not supported.
#
##########################################################################################

if (OMIT.count("MILES")==0):
    WARNINGS.append("makepanda currently does not support miles sound system")
    WARNINGS.append("I have automatically added this command-line option: --no-miles")
    OMIT.append("MILES")

if (sys.platform != "win32"):
    if (OMIT.count("DX8")==0): OMIT.append("DX8")
    if (OMIT.count("DX9")==0): OMIT.append("DX9")

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

printStatus("Makepanda Initial Status Report", WARNINGS)


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
    except:
        exit("Cannot open source file \""+path+"\" for reading.")
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
                if (VERBOSE > 1):
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
    if (NeedsBuild([dstfile],[srcfile])):
        global VERBOSE
        if VERBOSE >= 1:
            print "Copying \"%s\" --> \"%s\""%(srcfile, dstfile)
        WriteFile(dstfile,ReadFile(srcfile))
        JustBuilt([dstfile], [srcfile])

def CopyAllFiles(dstdir, srcdir, suffix=""):
    suflen = len(suffix)
    files = os.listdir(srcdir)
    for x in files:
        if (os.path.isfile(srcdir+x)):
            if (suflen==0) or (x[-suflen:]==suffix):
                CopyFile(dstdir+x, srcdir+x)

def CopyAllHeaders(dir, skip=[]):
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
                if (NeedsBuild([dstfile],[srcfile])):
                    copied.append(filename)
                    WriteFile(dstfile,ReadFile(srcfile))
                    JustBuilt([dstfile],[srcfile])
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
    if (COMPILER=="MSVC"): cmd = 'xcopy /I/Y/E/Q "' + srcdir + '" "' + dstdir + '"'
    if (COMPILER=="LINUX"): cmd = 'cp --recursive --force ' + srcdir + ' ' + dstdir
    oscmd(cmd)

########################################################################
##
## EnqueueCxx
##
########################################################################

def CompileCxxMSVC(wobj,fullsrc,ipath,opts):
    cmd = "cl /wd4996 /Fo" + wobj + " /nologo /c"
    if (OMIT.count("PYTHON")==0): cmd = cmd + " /Ithirdparty/win-python/include"
    for ver in DXVERSIONS:
        if (PkgSelected(opts,"DX"+ver)):
            cmd = cmd + ' /I"' + DIRECTXSDK["DX"+ver] + '/include"'
    for ver in MAYAVERSIONS:
        if (PkgSelected(opts,"MAYA"+ver)):
            cmd = cmd + ' /I"' + MAYASDK["MAYA"+ver] + '/include"'
    for ver in MAXVERSIONS:
        if (PkgSelected(opts,"MAX"+ver)):
            cmd = cmd + ' /I"' + MAXSDK["MAX"+ver] + '/include" /I"' + MAXSDKCS["MAX"+ver] + '" /DMAX' + ver
    for pkg in PACKAGES:
        if (pkg[:4] != "MAYA") and (pkg[:3]!="MAX") and (pkg[:2]!="DX") and PkgSelected(opts,pkg):
            cmd = cmd + " /I" + THIRDPARTYLIBS + pkg.lower() + "/include"
    for x in ipath: cmd = cmd + " /I" + x
    if (opts.count('NOFLOATWARN')): cmd = cmd + ' /wd4244 /wd4305'
    if (opts.count("WITHINPANDA")): cmd = cmd + ' /DWITHIN_PANDA'
    if (opts.count("MSFORSCOPE")): cmd = cmd + ' /Zc:forScope-'
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

def CompileCxxLINUX(wobj,fullsrc,ipath,opts):
    if (fullsrc[-2:]==".c"): cmd = 'gcc -fPIC -c -o ' + wobj
    else:                    cmd = 'g++ -ftemplate-depth-30 -fPIC -c -o ' + wobj
    if (OMIT.count("PYTHON")==0): cmd = cmd + ' -I"' + PYTHONSDK + '"'
    if (PkgSelected(opts,"VRPN")):     cmd = cmd + ' -I' + THIRDPARTYLIBS + 'vrpn/include'
    if (PkgSelected(opts,"FFTW")):     cmd = cmd + ' -I' + THIRDPARTYLIBS + 'fftw/include'
    if (PkgSelected(opts,"FMODEX")):   cmd = cmd + ' -I' + THIRDPARTYLIBS + 'fmodex/include'
    if (PkgSelected(opts,"NVIDIACG")): cmd = cmd + ' -I' + THIRDPARTYLIBS + 'nvidiacg/include'
    if (PkgSelected(opts,"FFMPEG")):   cmd = cmd + ' -I' + THIRDPARTYLIBS + 'ffmpeg/include'
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
    if ((obj==0)|(src==0)):
        exit("syntax error in EnqueueCxx directive")
    if (COMPILER=="MSVC"):
        wobj = "built/tmp/"+obj
        fn = CompileCxxMSVC
    if (COMPILER=="LINUX"):
        wobj = "built/tmp/" + obj[:-4] + ".o"
        fn = CompileCxxLINUX
    ipath = ["built/tmp"] + ipath + ["built/include"]
    fullsrc = CxxFindSource(src, ipath)
    if (fullsrc == 0):
        exit("Cannot find source file "+src)
    dep = CxxCalcDependencies(fullsrc, ipath, []) + xdep
    DependencyQueue(fn, [wobj,fullsrc,ipath,opts], [wobj], dep, [])

########################################################################
##
## CompileBison
##
########################################################################

def CompileBisonMSVC(pre, dsth, dstc, wobj, ipath, opts, src):
    ifile = os.path.basename(src)
    oscmd('thirdparty/win-util/bison -y -d -obuilt/tmp/'+ifile+'.c -p '+pre+' '+src)
    CopyFile(dstc, "built/tmp/"+ifile+".c")
    CopyFile(dsth, "built/tmp/"+ifile+".h")
    CompileCxxMSVC(wobj,dstc,ipath,opts)

def CompileBisonLINUX(pre, dsth, dstc, wobj, ipath, opts, src):
    ifile = os.path.basename(src)
    oscmd("bison -y -d -obuilt/tmp/"+ifile+".c -p "+pre+" "+src)
    CopyFile(dstc, "built/tmp/"+ifile+".c")
    CopyFile(dsth, "built/tmp/"+ifile+".h")
    CompileCxxLINUX(wobj,dstc,ipath,opts)

def EnqueueBison(ipath=0,opts=0,pre=0,obj=0,dsth=0,src=0):
    if ((ipath==0)|(opts==0)|(pre==0)|(obj==0)|(dsth==0)|(src==0)):
        exit("syntax error in EnqueueBison directive")
    if (COMPILER=="MSVC"):
        wobj="built/tmp/"+obj
        fn = CompileBisonMSVC
    if (COMPILER=="LINUX"):
        wobj="built/tmp/"+obj[:-4]+".o"
        fn = CompileBisonLINUX
    ipath = ["built/tmp"] + ipath + ["built/include"]
    fullsrc = CxxFindSource(src, ipath)
    if (fullsrc == 0):
        exit("Cannot find source file "+src)
    if (OMIT.count("BISON")):
        dir = os.path.dirname(fullsrc)
        CopyFile("built/tmp/"+obj[:-4]+".cxx", dir+"/"+src[:-4]+".cxx.prebuilt")
        CopyFile("built/include/"+src[:-4]+".h", dir+"/"+src[:-4]+".h.prebuilt")
        EnqueueCxx(ipath=ipath,opts=opts,obj=obj,src=obj[:-4]+".cxx")
    else:
        dstc = "built/tmp/"+obj[:-4]+".cxx"
        dsth = "built/include/"+src[:-4]+".h"
        CreatePlaceHolder(dsth)
        CreatePlaceHolder(dstc)
        DependencyQueue(fn, [pre,dsth,dstc,wobj,ipath,opts,fullsrc], [wobj, dsth], [fullsrc], [])

########################################################################
##
## CompileFlex
##
########################################################################

def CompileFlexMSVC(pre,dst,src,wobj,ipath,opts,dashi):
    if (dashi): oscmd("thirdparty/win-util/flex -i -P" + pre + " -o"+dst+" "+src)
    else:       oscmd("thirdparty/win-util/flex    -P" + pre + " -o"+dst+" "+src)
    CompileCxxMSVC(wobj,dst,ipath,opts)

def CompileFlexLINUX(pre,dst,src,wobj,ipath,opts,dashi):
    if (dashi): oscmd("flex -i -P" + pre + " -o"+dst+" "+src)
    else:       oscmd("flex    -P" + pre + " -o"+dst+" "+src)
    CompileCxxLINUX(wobj,dst,ipath,opts)

def EnqueueFlex(ipath=0,opts=0,pre=0,obj=0,src=0,dashi=0):
    if ((ipath==0)|(opts==0)|(pre==0)|(obj==0)|(src==0)):
        exit("syntax error in EnqueueFlex directive")
    if (COMPILER=="MSVC"):
        wobj="built/tmp/"+obj[:-4]+".obj"
        fn=CompileFlexMSVC
    if (COMPILER=="LINUX"):
        wobj="built/tmp/"+obj[:-4]+".o"
        fn=CompileFlexLINUX
    ipath = ["built/tmp"] + ipath + ["built/include"]
    fullsrc = CxxFindSource(src, ipath)
    if (fullsrc == 0):
        exit("Cannot find source file "+src)
    if (OMIT.count("FLEX")):
        dir = os.path.dirname(fullsrc)
        CopyFile("built/tmp/"+obj[:-4]+".cxx", dir+"/"+src[:-4]+".cxx.prebuilt")
        EnqueueCxx(ipath=IPATH, opts=OPTS, obj=obj, src=obj[:-4]+".cxx")
    else:
        dst = "built/tmp/"+obj[:-4]+".cxx"
        CreatePlaceHolder(dst)
        DependencyQueue(fn, [pre,dst,fullsrc,wobj,ipath,opts,dashi], [wobj], [fullsrc], [])

########################################################################
##
## EnqueueIgate
##
########################################################################

def CompileIgateMSVC(ipath,opts,outd,outc,wobj,src,module,library,files):
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
        cmd = cmd + ' -Ithirdparty/win-python/include'
        for pkg in PACKAGES:
            if (PkgSelected(opts,pkg)):
                cmd = cmd + " -I" + THIRDPARTYLIBS + pkg.lower() + "/include"
        cmd = cmd + ' -oc ' + outc + ' -od ' + outd
        cmd = cmd + ' -fnames -string -refcount -assert -python-native'
        for x in ipath: cmd = cmd + ' -I' + x
        building = getbuilding(opts)
        if (building): cmd = cmd + " -DBUILDING_"+building
        if (opts.count("WITHINPANDA")): cmd = cmd + " -DWITHIN_PANDA"
        cmd = cmd + ' -module ' + module + ' -library ' + library
        for ver in DXVERSIONS:
            if ((COMPILER=="MSVC") and PkgSelected(opts,"DX"+ver)):
                cmd = cmd + ' -I"' + DIRECTXSDK["DX"+ver] + '/include"'
        for ver in MAYAVERSIONS:
            if ((COMPILER=="MSVC") and PkgSelected(opts,"MAYA"+ver)):
                cmd = cmd + ' -I"' + MAYASDK["MAYA"+ver] + '/include"'
        for x in files: cmd = cmd + ' ' + x
        oscmd(cmd)
    CompileCxxMSVC(wobj,outc,ipath,opts)

def CompileIgateLINUX(ipath,opts,outd,outc,wobj,src,module,library,files):
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
        cmd = cmd + ' -Ithirdparty/win-python/include'
        for pkg in PACKAGES:
            if (PkgSelected(opts,pkg)):
                cmd = cmd + " -I" + THIRDPARTYLIBS + pkg.lower() + "/include"
        cmd = cmd + ' -oc ' + outc + ' -od ' + outd
        cmd = cmd + ' -fnames -string -refcount -assert -python-native'
        for x in ipath: cmd = cmd + ' -I' + x
        building = getbuilding(opts)
        if (building): cmd = cmd + " -DBUILDING_"+building
        if (opts.count("WITHINPANDA")): cmd = cmd + " -DWITHIN_PANDA"
        cmd = cmd + ' -module ' + module + ' -library ' + library
        for ver in MAYAVERSIONS:
            if (PkgSelected(opts, "MAYA"+ver)):
                cmd = cmd + ' -I"' + MAYASDK["MAYA"+ver] + '/include"'
        for x in files: cmd = cmd + ' ' + x
        oscmd(cmd)
    CompileCxxLINUX(wobj,outc,ipath,opts)

def EnqueueIgate(ipath=0, opts=0, outd=0, obj=0, src=0, module=0, library=0, also=0, skip=0):
    if ((ipath==0)|(opts==0)|(outd==0)|(obj==0)|(src==0)|(module==0)|(library==0)|(also==0)|(skip==0)):
        exit("syntax error in EnqueueIgate directive")
    if (COMPILER=="MSVC"):
        altdep = "built/bin/interrogate.exe"
        wobj = "built/tmp/"+obj
        fn = CompileIgateMSVC
    if (COMPILER=="LINUX"):
        altdep = "built/bin/interrogate"
        wobj = "built/tmp/"+obj[:-4]+".o"
        fn = CompileIgateLINUX
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
    dep = ["built/tmp/dtool_have_python.dat"]
    dep = dep + CxxCalcDependenciesAll(xpaths(src+"/",files,""), ipath)
    outc = "built/tmp/"+obj[:-4]+".cxx"
    DependencyQueue(fn, [ipath,opts,outd,outc,wobj,src,module,library,files], [wobj, outd], dep, [altdep])

########################################################################
##
## EnqueueImod
##
########################################################################

def CompileImodMSVC(outc, wobj, module, library, ipath, opts, files):
    if (OMIT.count("PYTHON")):
        WriteFile(outc,"")
    else:
        cmd = 'built/bin/interrogate_module '
        cmd = cmd + ' -oc ' + outc + ' -module ' + module + ' -library ' + library + ' -python-native '
        for x in files: cmd = cmd + ' ' + x
        oscmd(cmd)
    CompileCxxMSVC(wobj,outc,ipath,opts)

def CompileImodLINUX(outc, wobj, module, library, ipath, opts, files):
    if (OMIT.count("PYTHON")):
        WriteFile(outc,"")
    else:
        cmd = 'built/bin/interrogate_module '
        cmd = cmd + ' -oc ' + outc + ' -module ' + module + ' -library ' + library + ' -python-native '
        for x in files: cmd = cmd + ' ' + x
        oscmd(cmd)
    CompileCxxLINUX(wobj,outc,ipath,opts)

def EnqueueImod(ipath=0, opts=0, obj=0, module=0, library=0, files=0):
    if ((ipath==0)|(opts==0)|(obj==0)|(module==0)|(library==0)|(files==0)):
        exit("syntax error in EnqueueImod directive")
    if (COMPILER=="MSVC"):
        altdep = "built/bin/interrogate_module.exe"
        wobj = "built/tmp/"+obj[:-4]+".obj"
        fn = CompileImodMSVC
    if (COMPILER=="LINUX"):
        altdep = "built/bin/interrogate_module"
        wobj = "built/tmp/"+obj[:-4]+".o"
        fn = CompileImodLINUX
    ipath = ["built/tmp"] + ipath + ["built/include"]
    outc = "built/tmp/"+obj[:-4]+".cxx"
    files = xpaths("built/pandac/input/",files,"")
    dep = files + ["built/tmp/dtool_have_python.dat"]
    DependencyQueue(fn, [outc, wobj, module, library, ipath, opts, files], [wobj], dep, [altdep])

########################################################################
##
## EnqueueLib
##
########################################################################

def CompileLibMSVC(wlib, wobj, opts):
    cmd = 'link /lib /nologo /OUT:' + wlib
    optlevel = getoptlevel(opts,OPTIMIZE)
    if (optlevel==4): cmd = cmd + " /LTCG "
    for x in wobj: cmd = cmd + ' ' + x
    oscmd(cmd)

def CompileLibLINUX(wlib, wobj, opts):
    cmd = 'ar cru ' + wlib
    for x in wobj: cmd=cmd + ' ' + x
    oscmd(cmd)

def EnqueueLib(lib=0, obj=[], opts=[]):
    if (lib==0): exit("syntax error in EnqueueLib directive")

    if (COMPILER=="MSVC"):
        if (lib[-4:]==".ilb"): wlib = "built/tmp/" + lib[:-4] + ".lib"
        else:                  wlib = "built/lib/" + lib[:-4] + ".lib"
        wobj = xpaths("built/tmp/",obj,"")
        DependencyQueue(CompileLibMSVC, [wlib, wobj, opts], [wlib], wobj, [])

    if (COMPILER=="LINUX"):
        if (lib[-4:]==".ilb"): wlib = "built/tmp/" + lib[:-4] + ".a"
        else:                  wlib = "built/lib/" + lib[:-4] + ".a"
        wobj = []
        for x in obj: wobj.append("built/tmp/" + x[:-4] + ".o")
        DependencyQueue(CompileLibLINUX, [wlib, wobj, opts], [wlib], wobj, [])

########################################################################
##
## EnqueueLink
##
########################################################################

def CompileLinkMSVC(wdll, wlib, wobj, opts, dll, ldef):
    cmd = 'link /nologo /NODEFAULTLIB:LIBCI.LIB /NODEFAULTLIB:MSVCRTD.LIB /DEBUG '
    if (THIRDPARTYLIBS=="thirdparty/win-libs-vc8/"): cmd = cmd + " /nod:libc /nod:libcmtd"
    if (wdll[-4:]!=".exe"): cmd = cmd + " /DLL"
    optlevel = getoptlevel(opts,OPTIMIZE)
    if (optlevel==1): cmd = cmd + " /MAP /MAPINFO:EXPORTS"
    if (optlevel==2): cmd = cmd + " /MAP:NUL "
    if (optlevel==3): cmd = cmd + " /MAP:NUL "
    if (optlevel==4): cmd = cmd + " /MAP:NUL /LTCG "
    cmd = cmd + " /FIXED:NO /OPT:REF /STACK:4194304 /INCREMENTAL:NO "
    if (ldef!=0): cmd = cmd + ' /DEF:"' + ldef + '"'
    cmd = cmd + ' /OUT:' + wdll
    if (wlib != 0): cmd = cmd + ' /IMPLIB:' + wlib
    if (OMIT.count("PYTHON")==0): cmd = cmd + ' /LIBPATH:thirdparty/win-python/libs '
    for x in wobj: cmd = cmd + ' ' + x
    if (wdll[-4:]==".exe"): cmd = cmd + ' panda/src/configfiles/pandaIcon.obj'
    for ver in DXVERSIONS:
        if (PkgSelected(opts,"DX"+ver)):
            cmd = cmd + ' /LIBPATH:"' + DIRECTXSDK["DX"+ver] + '/lib/x86"'
            cmd = cmd + ' /LIBPATH:"' + DIRECTXSDK["DX"+ver] + '/lib"'
            cmd = cmd + ' d3dVER.lib d3dxVER.lib dxerrVER.lib ddraw.lib dxguid.lib'.replace("VER",ver)
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
    if (PkgSelected(opts,"PNG")):      cmd = cmd + ' ' + THIRDPARTYLIBS + 'png/lib/libpandapng.lib'
    if (PkgSelected(opts,"JPEG")):     cmd = cmd + ' ' + THIRDPARTYLIBS + 'jpeg/lib/libpandajpeg.lib'
    if (PkgSelected(opts,"TIFF")):     cmd = cmd + ' ' + THIRDPARTYLIBS + 'tiff/lib/libpandatiff.lib'
    if (PkgSelected(opts,"ZLIB")):     cmd = cmd + ' ' + THIRDPARTYLIBS + 'zlib/lib/libpandazlib1.lib'
    if (PkgSelected(opts,"VRPN")):
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'vrpn/lib/vrpn.lib'
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'vrpn/lib/quat.lib'
    if (PkgSelected(opts,"FMODEX")):
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'fmodex/lib/fmodex_vc.lib'
    if (PkgSelected(opts,"MILES")):
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'miles/lib/mss32.lib'
    if (PkgSelected(opts,"NVIDIACG")):
        if (opts.count("CGGL")):
            cmd = cmd + ' ' + THIRDPARTYLIBS + 'nvidiacg/lib/cgGL.lib'
        if (opts.count("CGDX9")):
            cmd = cmd + ' ' + THIRDPARTYLIBS + 'nvidiacg/lib/cgD3D9.lib'
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'nvidiacg/lib/cg.lib'
    if (PkgSelected(opts,"OPENSSL")):
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'openssl/lib/libpandassl.lib'
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'openssl/lib/libpandaeay.lib'
    if (PkgSelected(opts,"FREETYPE")):
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'freetype/lib/freetype.lib'
    if (PkgSelected(opts,"FFTW")):
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'fftw/lib/rfftw.lib'
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'fftw/lib/fftw.lib'
    if (PkgSelected(opts,"FFMPEG")):
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'ffmpeg/lib/avcodec-51-panda.lib'
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'ffmpeg/lib/avformat-50-panda.lib'
        cmd = cmd + ' ' + THIRDPARTYLIBS + 'ffmpeg/lib/avutil-49-panda.lib'
    for ver in MAYAVERSIONS:
        if (PkgSelected(opts,"MAYA"+ver)):
            cmd = cmd + ' "' + MAYASDK["MAYA"+ver] +  '/lib/Foundation.lib"'
            cmd = cmd + ' "' + MAYASDK["MAYA"+ver] +  '/lib/OpenMaya.lib"'
            cmd = cmd + ' "' + MAYASDK["MAYA"+ver] +  '/lib/OpenMayaAnim.lib"'
            cmd = cmd + ' "' + MAYASDK["MAYA"+ver] +  '/lib/OpenMayaUI.lib"'
    for ver in MAXVERSIONS:
        if (PkgSelected(opts,"MAX"+ver)):
            cmd = cmd + ' "' + MAXSDK["MAX"+ver] +  '/lib/core.lib"'
            cmd = cmd + ' "' + MAXSDK["MAX"+ver] +  '/lib/edmodel.lib"'
            cmd = cmd + ' "' + MAXSDK["MAX"+ver] +  '/lib/gfx.lib"'
            cmd = cmd + ' "' + MAXSDK["MAX"+ver] +  '/lib/geom.lib"'
            cmd = cmd + ' "' + MAXSDK["MAX"+ver] +  '/lib/mesh.lib"'
            cmd = cmd + ' "' + MAXSDK["MAX"+ver] +  '/lib/maxutil.lib"'
            cmd = cmd + ' "' + MAXSDK["MAX"+ver] +  '/lib/paramblk2.lib"'
    oscmd(cmd)
    mtcmd = 'mt -manifest ' + wdll + '.manifest -outputresource:' + wdll
    if (wdll[-4:]!=".exe"): mtcmd = mtcmd + ';2'
    else:                   mtcmd = mtcmd + ';1'
    oscmd(mtcmd)

def CompileLinkLINUX(wdll, obj, wobj, opts, dll, ldef):
    if (dll[-4:]==".exe"): cmd = 'g++ -o ' + wdll + ' -Lbuilt/lib -L/usr/X11R6/lib'
    else:                  cmd = 'g++ -shared -o ' + wdll + ' -Lbuilt/lib -L/usr/X11R6/lib'
    for x in obj:
        suffix = x[-4:]
        if   (suffix==".obj"): cmd = cmd + ' built/tmp/' + x[:-4] + '.o'
        elif (suffix==".dll"): cmd = cmd + ' -l' + x[3:-4]
        elif (suffix==".lib"): cmd = cmd + ' built/lib/' + x[:-4] + '.a'
        elif (suffix==".ilb"): cmd = cmd + ' built/tmp/' + x[:-4] + '.a'
    if (PkgSelected(opts,"FMODEX")):   cmd = cmd + ' -L' + THIRDPARTYLIBS + 'fmodex/lib -lfmodex'
    if (PkgSelected(opts,"NVIDIACG")):
        cmd = cmd + ' -Lthirdparty/nvidiacg/lib '
        if (opts.count("CGGL")):  cmd = cmd + " -lCgGL"
        cmd = cmd + " -lCg "
    if (PkgSelected(opts,"FFMPEG")):   cmd = cmd + ' -L' + THIRDPARTYLIBS + 'ffmpeg/lib -lavformat -lavcodec -lavformat -lavutil'
    if (PkgSelected(opts,"ZLIB")):     cmd = cmd + " -lz"
    if (PkgSelected(opts,"PNG")):      cmd = cmd + " -lpng"
    if (PkgSelected(opts,"JPEG")):     cmd = cmd + " -ljpeg"
    if (PkgSelected(opts,"TIFF")):     cmd = cmd + " -ltiff"
    if (PkgSelected(opts,"OPENSSL")):  cmd = cmd + " -lssl"
    if (PkgSelected(opts,"FREETYPE")): cmd = cmd + " -lfreetype"
    if (PkgSelected(opts,"VRPN")):     cmd = cmd + ' -L' + THIRDPARTYLIBS + 'vrpn/lib -lvrpn -lquat'
    if (PkgSelected(opts,"FFTW")):     cmd = cmd + ' -L' + THIRDPARTYLIBS + 'fftw/lib -lrfftw -lfftw'
    if (opts.count("GLUT")):           cmd = cmd + " -lGL -lGLU"
    cmd = cmd + " -lpthread"
    oscmd(cmd)

def EnqueueLink(dll=0, obj=[], opts=[], xdep=[], ldef=0):
    if (dll==0): exit("syntax error in EnqueueLink directive")

    if (COMPILER=="MSVC"):
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
            DependencyQueue(CompileLinkMSVC, [wdll, 0,    wobj, opts, dll, ldef], [wdll], wobj, [])
        elif (dll[-4:]==".dll"):
            wdll = "built/bin/"+dll
            wlib = "built/lib/"+dll[:-4]+".lib"
            DependencyQueue(CompileLinkMSVC, [wdll, wlib, wobj, opts, dll, ldef], [wdll, wlib], wobj, [])
        else:
            wdll = "built/plugins/"+dll
            DependencyQueue(CompileLinkMSVC, [wdll, 0, wobj, opts, dll, ldef], [wdll], wobj, [])

    if (COMPILER=="LINUX"):
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
        DependencyQueue(CompileLinkLINUX, [wdll, obj, wobj, opts, dll, ldef], [wdll], wobj, [])


##########################################################################################
#
# EnqueueBam
#
##########################################################################################

def CompileBam(preconv, bam, egg):
    if (egg[-4:] == ".flt"):
        ifile = os.path.basename(egg)
        oscmd("built/bin/flt2egg " + preconv + " -o built/tmp/"+ifile+".egg" + " " + egg)
        oscmd("built/bin/egg2bam -o " + bam + " built/tmp/"+ifile+".egg")
    else:
        oscmd("built/bin/egg2bam " + preconv + " -o " + bam + " " + egg)

def EnqueueBam(preconv, bam, egg):
    if (sys.platform == "win32"):
        flt2egg = "built/bin/flt2egg.exe"
        egg2bam = "built/bin/egg2bam.exe"
    else:
        flt2egg = "built/bin/flt2egg"
        egg2bam = "built/bin/egg2bam"
    DependencyQueue(CompileBam, [preconv, bam, egg], [bam], [egg], [flt2egg, egg2bam])

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
MakeDirectory("built/include/parser-inc/netinet")
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

##########################################################################################
#
# Generate dtool_config.h, prc_parameters.h, and dtool_have_xxx.dat
#
##########################################################################################

DEFAULT_SETTINGS=[
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
    ("HAVE_DX8",                       'UNDEF',                  'UNDEF'),
    ("HAVE_DX9",                       'UNDEF',                  'UNDEF'),
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
    ("IS_LINUX",                       'UNDEF',                  '1'),
    ("GLOBAL_OPERATOR_NEW_EXCEPTIONS", 'UNDEF',                  '1'),
    ("USE_STL_ALLOCATOR",              '1',                      '1'),
    ("USE_MEMORY_DLMALLOC",            '1',                      'UNDEF'),
    ("USE_MEMORY_PTMALLOC",            'UNDEF',                  'UNDEF'),
    ("USE_MEMORY_MALLOC",              'UNDEF',                  '1'),
    ("HAVE_ZLIB",                      'UNDEF',                  'UNDEF'),
    ("HAVE_PNG",                       'UNDEF',                  'UNDEF'),
    ("HAVE_JPEG",                      'UNDEF',                  'UNDEF'),
    ("HAVE_TIFF",                      'UNDEF',                  'UNDEF'),
    ("HAVE_VRPN",                      'UNDEF',                  'UNDEF'),
    ("HAVE_FMODEX",                    'UNDEF',                  'UNDEF'),
    ("HAVE_NVIDIACG",                  'UNDEF',                  'UNDEF'),
    ("HAVE_FREETYPE",                  'UNDEF',                  'UNDEF'),
    ("HAVE_FFTW",                      'UNDEF',                  'UNDEF'),
    ("HAVE_OPENSSL",                   'UNDEF',                  'UNDEF'),
    ("HAVE_NET",                       'UNDEF',                  'UNDEF'),
    ("HAVE_CG",                        'UNDEF',                  'UNDEF'),
    ("HAVE_CGGL",                      'UNDEF',                  'UNDEF'),
    ("HAVE_CGDX9",                     'UNDEF',                  'UNDEF'),
    ("HAVE_FFMPEG",                    'UNDEF',                  'UNDEF'),
    ("DEFAULT_PRC_DIR",                '"<auto>etc"',            '"<auto>etc"'),
    ("PRC_DIR_ENVVARS",                '"PANDA_PRC_DIR"',        '"PANDA_PRC_DIR"'),
    ("PRC_PATH_ENVVARS",               '"PANDA_PRC_PATH"',       '"PANDA_PRC_PATH"'),
    ("PRC_PATTERNS",                   '"*.prc"',                '"*.prc"'),
    ("PRC_ENCRYPTED_PATTERNS",         '"*.prc.pe"',             '"*.prc.pe"'),
    ("PRC_ENCRYPTION_KEY",             '""',                     '""'),
    ("PRC_EXECUTABLE_PATTERNS",        '""',                     '""'),
    ("PRC_EXECUTABLE_ARGS_ENVVAR",     '"PANDA_PRC_XARGS"',      '"PANDA_PRC_XARGS"'),
    ("PRC_PUBLIC_KEYS_FILENAME",       '""',                     '""'),
    ("PRC_RESPECT_TRUST_LEVEL",        'UNDEF',                  'UNDEF'),
    ("PRC_DCONFIG_TRUST_LEVEL",        '0',                      '0'),
    ("PRC_INC_TRUST_LEVEL",            '0',                      '0'),
    ("PRC_SAVE_DESCRIPTIONS",          '1',                      '1'),
]

def WriteConfigSettings():
    settings={}
    if (sys.platform == "win32"):
        for key,win,unix in DEFAULT_SETTINGS:
            settings[key] = win
    else:
        for key,win,unix in DEFAULT_SETTINGS:
            settings[key] = unix
    
    for x in PACKAGES:
        if (OMIT.count(x)==0):
            if (settings.has_key("HAVE_"+x)):
                settings["HAVE_"+x] = '1'
    
    settings["HAVE_NET"] = '1'
    
    if (OMIT.count("NVIDIACG")==0):
        settings["HAVE_CG"] = '1'
        settings["HAVE_CGGL"] = '1'
        settings["HAVE_CGDX9"] = '1'
    
    if (OPTIMIZE <= 3):
        if (settings["HAVE_NET"] != 'UNDEF'):
            settings["DO_PSTATS"] = '1'
    
    if (OPTIMIZE <= 3):
        settings["DO_COLLISION_RECORDING"] = '1'
    
    #if (OPTIMIZE <= 2):
    #    settings["TRACK_IN_INTERPRETER"] = '1'
    
    if (OPTIMIZE <= 3):
        settings["DO_MEMORY_USAGE"] = '1'
    
    #if (OPTIMIZE <= 1):
    #    settings["DO_PIPELINING"] = '1'
    
    if (OPTIMIZE <= 3):
        settings["NOTIFY_DEBUG"] = '1'

    conf = "/* prc_parameters.h.  Generated automatically by makepanda.py */\n"
    for key in settings.keys():
        if ((key == "DEFAULT_PRC_DIR") or (key[:4]=="PRC_")):
            val = settings[key]
            if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
            else:                conf = conf + "#define " + key + " " + val + "\n"
            del settings[key]
    ConditionalWriteFile('built/include/prc_parameters.h', conf)

    conf = "/* dtool_config.h.  Generated automatically by makepanda.py */\n"
    for key in settings.keys():
        val = settings[key]
        if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
        else:                conf = conf + "#define " + key + " " + val + "\n"
        del settings[key]
    ConditionalWriteFile('built/include/dtool_config.h', conf)

    for x in PACKAGES:
        if (OMIT.count(x)): ConditionalWriteFile('built/tmp/dtool_have_'+x.lower()+'.dat',"0\n")
        else:               ConditionalWriteFile('built/tmp/dtool_have_'+x.lower()+'.dat',"1\n")


WriteConfigSettings()


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


CreatePandaVersionFiles()

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

if (OMIT.count("PYTHON")==0):
    ConditionalWriteFile('built/direct/__init__.py', DIRECTINIT)

##########################################################################################
#
# Generate the PRC files into the ETC directory.
#
##########################################################################################

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

for pkg in (PACKAGES + ["extras"]):
    if (OMIT.count(pkg)==0):
        if (COMPILER == "MSVC"):
            if (os.path.exists(THIRDPARTYLIBS+pkg.lower()+"/bin")):
                CopyAllFiles("built/bin/",THIRDPARTYLIBS+pkg.lower()+"/bin/")
        if (COMPILER == "LINUX"):
            if (os.path.exists(THIRDPARTYLIBS+pkg.lower()+"/lib")):
                CopyAllFiles("built/lib/",THIRDPARTYLIBS+pkg.lower()+"/lib/")
if (sys.platform == "win32"):
    if (OMIT.count("PYTHON")==0):
        CopyFile('built/bin/python24.dll', 'thirdparty/win-python/python24.dll')
        CopyTree('built/python', 'thirdparty/win-python')
        CopyFile('built/python/ppython.exe', 'thirdparty/win-python/python.exe')
        CopyFile('built/python/ppythonw.exe', 'thirdparty/win-python/pythonw.exe')
        ConditionalWriteFile('built/python/panda.pth',"..\n../bin\n")

########################################################################
##
## Copy various stuff into the build.
##
########################################################################

CopyFile("built/", "doc/LICENSE")
CopyFile("built/", "doc/ReleaseNotes")
CopyAllFiles("built/plugins/",  "pandatool/src/scripts/", ".mel")
CopyAllFiles("built/plugins/",  "pandatool/src/scripts/", ".ms")
if (OMIT.count("PYTHON")==0):
    CopyTree('built/Pmw',         'thirdparty/Pmw')
    CopyTree('built/SceneEditor', 'SceneEditor')
ConditionalWriteFile('built/include/ctl3d.h', '/* dummy file to make MAX happy */')

########################################################################
#
# Copy header files to the built/include/parser-inc directory.
#
########################################################################

CopyAllFiles('built/include/parser-inc/','dtool/src/parser-inc/')
CopyAllFiles('built/include/parser-inc/openssl/','dtool/src/parser-inc/')
CopyAllFiles('built/include/parser-inc/netinet/','dtool/src/parser-inc/')
CopyFile('built/include/parser-inc/Cg/','dtool/src/parser-inc/cg.h')
CopyFile('built/include/parser-inc/Cg/','dtool/src/parser-inc/cgGL.h')

########################################################################
#
# Transfer all header files to the built/include directory.
#
########################################################################

CopyAllHeaders('dtool/src/dtoolbase')
CopyAllHeaders('dtool/src/dtoolutil', skip=["pandaVersion.h", "checkPandaVersion.h"])
CopyAllHeaders('dtool/metalibs/dtool')
CopyAllHeaders('dtool/src/cppparser', skip="ALL")
CopyAllHeaders('dtool/src/prc')
CopyAllHeaders('dtool/src/dconfig')
CopyAllHeaders('dtool/src/interrogatedb')
CopyAllHeaders('dtool/metalibs/dtoolconfig')
CopyAllHeaders('dtool/src/pystub')
CopyAllHeaders('dtool/src/interrogate')
CopyAllHeaders('dtool/src/test_interrogate', skip="ALL")
CopyAllHeaders('panda/src/putil')
CopyAllHeaders('panda/src/pandabase')
CopyAllHeaders('panda/src/express')
CopyAllHeaders('panda/src/downloader')
CopyAllHeaders('panda/metalibs/pandaexpress')
CopyAllHeaders('panda/src/pipeline')
CopyAllHeaders('panda/src/putil')
CopyAllHeaders('dtool/src/prckeys')
CopyAllHeaders('panda/src/audio')
CopyAllHeaders('panda/src/event')
CopyAllHeaders('panda/src/linmath')
CopyAllHeaders('panda/src/mathutil')
CopyAllHeaders('panda/src/gsgbase')
CopyAllHeaders('panda/src/pnmimage')
CopyAllHeaders('panda/src/nativenet')
CopyAllHeaders('panda/src/net')
CopyAllHeaders('panda/src/pstatclient')
CopyAllHeaders('panda/src/gobj')
CopyAllHeaders('panda/src/lerp')
CopyAllHeaders('panda/src/pgraph')
CopyAllHeaders('panda/src/cull')
CopyAllHeaders('panda/src/effects')
CopyAllHeaders('panda/src/chan')
CopyAllHeaders('panda/src/char')
CopyAllHeaders('panda/src/dgraph')
CopyAllHeaders('panda/src/display')
CopyAllHeaders('panda/src/device')
CopyAllHeaders('panda/src/pnmtext')
CopyAllHeaders('panda/src/text')
CopyAllHeaders('panda/src/grutil')
CopyAllHeaders('panda/src/tform')
CopyAllHeaders('panda/src/collide')
CopyAllHeaders('panda/src/parametrics')
CopyAllHeaders('panda/src/pgui')
CopyAllHeaders('panda/src/pnmimagetypes')
CopyAllHeaders('panda/src/recorder')
CopyAllHeaders('panda/src/vrpn')
CopyAllHeaders('panda/src/helix')
CopyAllHeaders('panda/src/glgsg')
CopyAllHeaders('panda/src/wgldisplay')
CopyAllHeaders('panda/src/physics')
CopyAllHeaders('panda/src/particlesystem')
CopyAllHeaders('panda/metalibs/panda')
CopyAllHeaders('panda/src/audiotraits')
CopyAllHeaders('panda/src/audiotraits')
CopyAllHeaders('panda/src/distort')
CopyAllHeaders('panda/src/downloadertools')
CopyAllHeaders('panda/src/windisplay')
#     CopyAllHeaders('panda/src/dxgsg7')
#     CopyAllHeaders('panda/metalibs/pandadx7')
CopyAllHeaders('panda/src/dxgsg8')
CopyAllHeaders('panda/metalibs/pandadx8')
CopyAllHeaders('panda/src/dxgsg9')
CopyAllHeaders('panda/metalibs/pandadx9')
CopyAllHeaders('panda/src/egg')
CopyAllHeaders('panda/src/egg2pg')
CopyAllHeaders('panda/src/framework')
CopyAllHeaders('panda/metalibs/pandafx')
CopyAllHeaders('panda/src/glstuff')
CopyAllHeaders('panda/src/glgsg')
CopyAllHeaders('panda/metalibs/pandaegg')
if (sys.platform != "win32"):
    CopyAllHeaders('panda/src/glxdisplay')
else:
    CopyAllHeaders('panda/src/wgldisplay')
CopyAllHeaders('panda/metalibs/pandagl')

CopyAllHeaders('panda/src/physics')
CopyAllHeaders('panda/src/particlesystem')
CopyAllHeaders('panda/metalibs/pandaphysics')
CopyAllHeaders('panda/src/testbed')

CopyAllHeaders('direct/src/directbase')
CopyAllHeaders('direct/src/dcparser')
CopyAllHeaders('direct/src/deadrec')
CopyAllHeaders('direct/src/distributed')
CopyAllHeaders('direct/src/interval')
CopyAllHeaders('direct/src/showbase')
CopyAllHeaders('direct/metalibs/direct')
CopyAllHeaders('direct/src/dcparse')
CopyAllHeaders('direct/src/heapq')

CopyAllHeaders('pandatool/src/pandatoolbase')
CopyAllHeaders('pandatool/src/converter')
CopyAllHeaders('pandatool/src/progbase')
CopyAllHeaders('pandatool/src/eggbase')
CopyAllHeaders('pandatool/src/bam')
CopyAllHeaders('pandatool/src/cvscopy')
CopyAllHeaders('pandatool/src/dxf')
CopyAllHeaders('pandatool/src/dxfegg')
CopyAllHeaders('pandatool/src/dxfprogs')
CopyAllHeaders('pandatool/src/palettizer')
CopyAllHeaders('pandatool/src/egg-mkfont')
CopyAllHeaders('pandatool/src/eggcharbase')
CopyAllHeaders('pandatool/src/egg-optchar')
CopyAllHeaders('pandatool/src/egg-palettize')
CopyAllHeaders('pandatool/src/egg-qtess')
CopyAllHeaders('pandatool/src/eggprogs')
CopyAllHeaders('pandatool/src/flt')
CopyAllHeaders('pandatool/src/fltegg')
CopyAllHeaders('pandatool/src/fltprogs')
CopyAllHeaders('pandatool/src/imagebase')
CopyAllHeaders('pandatool/src/imageprogs')
CopyAllHeaders('pandatool/src/lwo')
CopyAllHeaders('pandatool/src/lwoegg')
CopyAllHeaders('pandatool/src/lwoprogs')
CopyAllHeaders('pandatool/src/maya')
CopyAllHeaders('pandatool/src/mayaegg')
CopyAllHeaders('pandatool/src/maxegg')
CopyAllHeaders('pandatool/src/maxprogs')
CopyAllHeaders('pandatool/src/vrml')
CopyAllHeaders('pandatool/src/vrmlegg')
CopyAllHeaders('pandatool/src/xfile')
CopyAllHeaders('pandatool/src/xfileegg')
CopyAllHeaders('pandatool/src/ptloader')
CopyAllHeaders('pandatool/src/miscprogs')
CopyAllHeaders('pandatool/src/pstatserver')
CopyAllHeaders('pandatool/src/softprogs')
CopyAllHeaders('pandatool/src/text-stats')
CopyAllHeaders('pandatool/src/vrmlprogs')
CopyAllHeaders('pandatool/src/win-stats')
CopyAllHeaders('pandatool/src/xfileprogs')

########################################################################
#
# This section contains a list of all the files that need to be compiled.
#
########################################################################

print "Generating dependencies..."
sys.stdout.flush()

#
# DIRECTORY: dtool/src/dtoolbase/
#

IPATH=['dtool/src/dtoolbase']
OPTS=['BUILDING_DTOOL',  'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='dtoolbase_composite1.cxx', obj='dtoolbase_composite1.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='dtoolbase_composite2.cxx', obj='dtoolbase_composite2.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='lookup3.c',                obj='dtoolbase_lookup3.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='indent.cxx',               obj='dtoolbase_indent.obj')
if (sys.platform == "win32"):
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dlmalloc.c', obj='dtoolbase_allocator.obj')
else:
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='null.cxx', obj='dtoolbase_allocator.obj')


#
# DIRECTORY: dtool/src/dtoolutil/
#

IPATH=['dtool/src/dtoolutil']
OPTS=['BUILDING_DTOOL',  'OPT3']
CopyFile('built/include/','dtool/src/dtoolutil/vector_src.cxx')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='gnu_getopt.c',             obj='dtoolutil_gnu_getopt.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='gnu_getopt1.c',            obj='dtoolutil_gnu_getopt1.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='dtoolutil_composite.cxx',  obj='dtoolutil_composite.obj')

#
# DIRECTORY: dtool/metalibs/dtool/
#

IPATH=['dtool/metalibs/dtool']
OPTS=['BUILDING_DTOOL',  'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='dtool.cxx', obj='dtool_dtool.obj')
EnqueueLink(opts=['ADVAPI',  'OPT3'], dll='libp3dtool.dll', obj=[
             'dtool_dtool.obj',
             'dtoolutil_gnu_getopt.obj',
             'dtoolutil_gnu_getopt1.obj',
             'dtoolutil_composite.obj',
             'dtoolbase_allocator.obj',
             'dtoolbase_composite1.obj',
             'dtoolbase_composite2.obj',
             'dtoolbase_indent.obj',
             'dtoolbase_lookup3.obj'
])

#
# DIRECTORY: dtool/src/cppparser/
#

IPATH=['dtool/src/cppparser']
OPTS=['OPT3']
EnqueueBison(ipath=IPATH, opts=OPTS, pre='cppyy', src='cppBison.yxx', dsth='cppBison.h', obj='cppParser_cppBison.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='cppParser_composite.cxx', obj='cppParser_composite.obj')
EnqueueLib(lib='libcppParser.ilb', obj=[
             'cppParser_composite.obj',
             'cppParser_cppBison.obj',
])

#
# DIRECTORY: dtool/src/prc/
#

IPATH=['dtool/src/prc']
OPTS=['BUILDING_DTOOLCONFIG', 'OPENSSL',  'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='prc_composite.cxx', obj='prc_composite.obj')

#
# DIRECTORY: dtool/src/dconfig/
#

IPATH=['dtool/src/dconfig']
OPTS=['BUILDING_DTOOLCONFIG',  'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='dconfig_composite.cxx', obj='dconfig_composite.obj')

#
# DIRECTORY: dtool/src/interrogatedb/
#

IPATH=['dtool/src/interrogatedb']
OPTS=['BUILDING_DTOOLCONFIG',  'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='interrogatedb_composite.cxx', obj='interrogatedb_composite.obj')

#
# DIRECTORY: dtool/metalibs/dtoolconfig/
#

IPATH=['dtool/metalibs/dtoolconfig']
OPTS=['BUILDING_DTOOLCONFIG',  'OPT3']
SRCFILE="pydtool.cxx"
if (OMIT.count("PYTHON")): SRCFILE="null.cxx"
EnqueueCxx(ipath=IPATH, opts=OPTS, src='dtoolconfig.cxx', obj='dtoolconfig_dtoolconfig.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src=SRCFILE, obj='dtoolconfig_pydtool.obj', xdep=["built/tmp/dtool_have_python.dat"])
EnqueueLink(opts=['ADVAPI',  'OPENSSL', 'OPT3'], dll='libp3dtoolconfig.dll', obj=[
             'dtoolconfig_dtoolconfig.obj',
             'dtoolconfig_pydtool.obj',
             'interrogatedb_composite.obj',
             'dconfig_composite.obj',
             'prc_composite.obj',
             'libp3dtool.dll',
])

#
# DIRECTORY: dtool/src/pystub/
#

IPATH=['dtool/src/pystub']
OPTS=['BUILDING_DTOOLCONFIG',  'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pystub.cxx', obj='pystub_pystub.obj')
EnqueueLink(opts=['ADVAPI',  'OPT3'], dll='libp3pystub.dll', obj=[
             'pystub_pystub.obj',
             'libp3dtool.dll',
])

#
# DIRECTORY: dtool/src/interrogate/
#

IPATH=['dtool/src/interrogate', 'dtool/src/cppparser', 'dtool/src/interrogatedb']
OPTS=[ 'OPT3']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='interrogate_composite.cxx', obj='interrogate_composite.obj')
EnqueueLink(opts=['ADVAPI',  'OPENSSL', 'OPT3'], dll='interrogate.exe', obj=[
             'interrogate_composite.obj',
             'libcppParser.ilb',
             'libp3pystub.dll',
             'libp3dtoolconfig.dll',
             'libp3dtool.dll',
])

EnqueueCxx(ipath=IPATH, opts=OPTS, src='interrogate_module.cxx', obj='interrogate_module_interrogate_module.obj')
EnqueueLink(opts=['ADVAPI',  'OPENSSL', 'OPT3'], dll='interrogate_module.exe', obj=[
             'interrogate_module_interrogate_module.obj',
             'libcppParser.ilb',
             'libp3pystub.dll',
             'libp3dtoolconfig.dll',
             'libp3dtool.dll',
])

EnqueueCxx(ipath=IPATH, opts=OPTS, src='parse_file.cxx', obj='parse_file_parse_file.obj')
EnqueueLink(opts=['ADVAPI',  'OPENSSL', 'OPT3'], dll='parse_file.exe', obj=[
             'parse_file_parse_file.obj',
             'libcppParser.ilb',
             'libp3pystub.dll',
             'libp3dtoolconfig.dll',
             'libp3dtool.dll',
])

#
# DIRECTORY: dtool/src/prckeys/
#

if (OMIT.count("OPENSSL")==0):
    IPATH=['dtool/src/prckeys']
    OPTS=['OPENSSL']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='makePrcKey.cxx', obj='make-prc-key_makePrcKey.obj')
    EnqueueLink(opts=['ADVAPI',  'OPENSSL'], dll='make-prc-key.exe', obj=[
                 'make-prc-key_makePrcKey.obj',
                 'libp3pystub.dll',
                 'libp3dtool.dll',
                 'libp3dtoolconfig.dll',
                 ])

#
# DIRECTORY: dtool/src/test_interrogate/
#

IPATH=['dtool/src/test_interrogate']
OPTS=[]
EnqueueCxx(ipath=IPATH, opts=OPTS, src='test_interrogate.cxx', obj='test_interrogate_test_interrogate.obj')
EnqueueLink(opts=['ADVAPI',  'OPENSSL'], dll='test_interrogate.exe', obj=[
             'test_interrogate_test_interrogate.obj',
             'libp3pystub.dll',
             'libp3dtoolconfig.dll',
             'libp3dtool.dll',
])

#
# DIRECTORY: panda/src/pandabase/
#

IPATH=['panda/src/pandabase']
OPTS=['BUILDING_PANDAEXPRESS']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandabase.cxx', obj='pandabase_pandabase.obj')

#
# DIRECTORY: panda/src/express/
#

IPATH=['panda/src/express']
OPTS=['BUILDING_PANDAEXPRESS', 'OPENSSL', 'ZLIB']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='express_composite1.cxx', obj='express_composite1.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='express_composite2.cxx', obj='express_composite2.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libexpress.in', obj='libexpress_igate.obj',
            src='panda/src/express',  module='pandaexpress', library='libexpress',
            skip=[], also=["express_composite1.cxx", "express_composite2.cxx"])

#
# DIRECTORY: panda/src/downloader/
#

IPATH=['panda/src/downloader']
OPTS=['BUILDING_PANDAEXPRESS', 'OPENSSL', 'ZLIB']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='downloader_composite.cxx', obj='downloader_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdownloader.in', obj='libdownloader_igate.obj',
            src='panda/src/downloader',  module='pandaexpress', library='libdownloader',
            skip=[], also=["downloader_composite.cxx"])

#
# DIRECTORY: panda/metalibs/pandaexpress/
#

IPATH=['panda/metalibs/pandaexpress']
OPTS=['BUILDING_PANDAEXPRESS', 'ZLIB']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandaexpress.cxx', obj='pandaexpress_pandaexpress.obj')
EnqueueImod(ipath=IPATH, opts=OPTS, obj='libpandaexpress_module.obj',
            module='pandaexpress', library='libpandaexpress',
            files=['libdownloader.in', 'libexpress.in'])
EnqueueLink(opts=['ADVAPI', 'WINSOCK2',  'OPENSSL', 'ZLIB'], dll='libpandaexpress.dll', obj=[
             'pandaexpress_pandaexpress.obj',
             'libpandaexpress_module.obj',
             'downloader_composite.obj',
             'libdownloader_igate.obj',
             'express_composite1.obj',
             'express_composite2.obj',
             'libexpress_igate.obj',
             'pandabase_pandabase.obj',
             'libp3dtoolconfig.dll',
             'libp3dtool.dll',
])

#
# DIRECTORY: panda/src/pipeline/
#

IPATH=['panda/src/pipeline']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pipeline_composite.cxx', obj='pipeline_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libpipeline.in', obj='libpipeline_igate.obj',
            src='panda/src/pipeline',  module='panda', library='libpipeline',
            skip=[], also=["pipeline_composite.cxx"])

#
# DIRECTORY: panda/src/putil/
#

IPATH=['panda/src/putil']
OPTS=['BUILDING_PANDA',  'ZLIB']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='putil_composite1.cxx', obj='putil_composite1.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='putil_composite2.cxx', obj='putil_composite2.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libputil.in', obj='libputil_igate.obj',
            src='panda/src/putil',  module='panda', library='libputil',
            skip=["test_bam.h"], also=["putil_composite1.cxx", "putil_composite2.cxx"])

#
# DIRECTORY: panda/src/audio/
#

IPATH=['panda/src/audio']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='audio_composite.cxx', obj='audio_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libaudio.in', obj='libaudio_igate.obj',
            src='panda/src/audio',  module='panda', library='libaudio',
            skip="ALL", also=["audio.h"])

#
# DIRECTORY: panda/src/event/
#

IPATH=['panda/src/event']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='event_composite.cxx', obj='event_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libevent.in', obj='libevent_igate.obj',
            src='panda/src/event',  module='panda', library='libevent',
            skip=[], also=["event_composite.cxx"])

#
# DIRECTORY: panda/src/linmath/
#

IPATH=['panda/src/linmath']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='linmath_composite.cxx', obj='linmath_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='liblinmath.in', obj='liblinmath_igate.obj',
            src='panda/src/linmath',  module='panda', library='liblinmath',
            skip=['lmat_ops_src.h', 'cast_to_double.h', 'lmat_ops.h', 'cast_to_float.h'],
            also=["linmath_composite.cxx"])

#
# DIRECTORY: panda/src/mathutil/
#

IPATH=['panda/src/mathutil']
OPTS=['BUILDING_PANDA', 'FFTW']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='mathutil_composite.cxx', obj='mathutil_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libmathutil.in', obj='libmathutil_igate.obj',
            src='panda/src/mathutil',  module='panda', library='libmathutil',
            skip=['mathHelpers.h'], also=["mathutil_composite.cxx"])

#
# DIRECTORY: panda/src/gsgbase/
#

IPATH=['panda/src/gsgbase']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='gsgbase_composite.cxx', obj='gsgbase_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libgsgbase.in', obj='libgsgbase_igate.obj',
            src='panda/src/gsgbase',  module='panda', library='libgsgbase',
            skip=[], also=["gsgbase_composite.cxx"])

#
# DIRECTORY: panda/src/pnmimage/
#

IPATH=['panda/src/pnmimage']
OPTS=['BUILDING_PANDA',  'ZLIB']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pnmimage_composite.cxx', obj='pnmimage_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libpnmimage.in', obj='libpnmimage_igate.obj',
            src='panda/src/pnmimage',  module='panda', library='libpnmimage',
            skip=[], also=["pnmimage_composite.cxx"])

#
# DIRECTORY: panda/src/nativenet/
#
 
IPATH=['panda/src/nativenet']
OPTS=['OPENSSL', 'BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='nativenet_composite1.cxx', obj='nativenet_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libnativenet.in', obj='libnativenet_igate.obj',
            src='panda/src/nativenet',  module='panda', library='libnativenet',
            skip=[], also=["nativenet_composite1.cxx"])

#
# DIRECTORY: panda/src/net/
#

IPATH=['panda/src/net']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='net_composite.cxx', obj='net_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libnet.in', obj='libnet_igate.obj',
            src='panda/src/net',  module='panda', library='libnet',
            skip=["datagram_ui.h"], also=["net_composite.cxx"])

#
# DIRECTORY: panda/src/pstatclient/
#

IPATH=['panda/src/pstatclient']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pstatclient_composite.cxx', obj='pstatclient_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libpstatclient.in', obj='libpstatclient_igate.obj',
            src='panda/src/pstatclient',  module='panda', library='libpstatclient',
            skip=[], also=["pstatclient_composite.cxx"])

#
# DIRECTORY: panda/src/gobj/
#

IPATH=['panda/src/gobj']
OPTS=['BUILDING_PANDA',  'NVIDIACG', 'ZLIB']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='gobj_composite1.cxx', obj='gobj_composite1.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='gobj_composite2.cxx', obj='gobj_composite2.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libgobj.in', obj='libgobj_igate.obj',
            src='panda/src/gobj',  module='panda', library='libgobj',
            skip=[], also=["gobj_composite1.cxx", "gobj_composite2.cxx"])

#
# DIRECTORY: panda/src/lerp/
#

IPATH=['panda/src/lerp']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='lerp_composite.cxx', obj='lerp_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='liblerp.in', obj='liblerp_igate.obj',
            src='panda/src/lerp',  module='panda', library='liblerp',
            skip=["lerp_headers.h","lerpchans.h"], also=["lerp_composite.cxx"])

#
# DIRECTORY: panda/src/pgraph/
#

IPATH=['panda/src/pgraph']
OPTS=['BUILDING_PANDA']
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
# DIRECTORY: panda/src/cull/
#

IPATH=['panda/src/cull']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='cull_composite.cxx', obj='cull_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libcull.in', obj='libcull_igate.obj',
            src='panda/src/cull',  module='panda', library='libcull',
            skip=[], also=["cull_composite.cxx"])

#
# DIRECTORY: panda/src/effects/
#

IPATH=['panda/src/effects']
OPTS=['BUILDING_PANDAFX',  'NVIDIACG']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='effects_composite.cxx', obj='effects_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libeffects.in', obj='libeffects_igate.obj',
            src='panda/src/effects',  module='pandafx', library='libeffects',
            skip=["cgShader.h", "cgShaderAttrib.h", "cgShaderContext.h"],
            also=["effects_composite.cxx"])

#
# DIRECTORY: panda/src/chan/
#

IPATH=['panda/src/chan']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='chan_composite.cxx', obj='chan_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libchan.in', obj='libchan_igate.obj',
            src='panda/src/chan',  module='panda', library='libchan',
            skip=['movingPart.h', 'chan_headers.h', 'animChannelFixed.h'],
            also=["chan_composite.cxx"])

#
# DIRECTORY: panda/src/char/
#

IPATH=['panda/src/char']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='char_composite.cxx', obj='char_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libchar.in', obj='libchar_igate.obj',
            src='panda/src/char',  module='panda', library='libchar',
            skip=[], also=["char_composite.cxx"])

#
# DIRECTORY: panda/src/dgraph/
#

IPATH=['panda/src/dgraph']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='dgraph_composite.cxx', obj='dgraph_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdgraph.in', obj='libdgraph_igate.obj',
            src='panda/src/dgraph',  module='panda', library='libdgraph',
            skip=[], also=["dgraph_composite.cxx"])

#
# DIRECTORY: panda/src/display/
#

IPATH=['panda/src/display']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='display_composite.cxx', obj='display_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdisplay.in', obj='libdisplay_igate.obj',
            src='panda/src/display',  module='panda', library='libdisplay',
            skip=['renderBuffer.h'], also=["display_composite.cxx"])

#
# DIRECTORY: panda/src/device/
#

IPATH=['panda/src/device']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='device_composite.cxx', obj='device_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdevice.in', obj='libdevice_igate.obj',
            src='panda/src/device',  module='panda', library='libdevice',
            skip=[], also=["device_composite.cxx"])

#
# DIRECTORY: panda/src/pnmtext/
#

if (OMIT.count("FREETYPE")==0):
    IPATH=['panda/src/pnmtext']
    OPTS=['BUILDING_PANDA',  'FREETYPE']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pnmtext_composite.cxx', obj='pnmtext_composite.obj')

#
# DIRECTORY: panda/src/text/
#

IPATH=['panda/src/text']
OPTS=['BUILDING_PANDA', 'ZLIB',  'FREETYPE']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='text_composite.cxx', obj='text_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libtext.in', obj='libtext_igate.obj',
            src='panda/src/text',  module='panda', library='libtext',
            skip=[], also=["text_composite.cxx"])

#
# DIRECTORY: panda/src/grutil/
#

IPATH=['panda/src/grutil']
OPTS=['BUILDING_PANDA',  'FFMPEG']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='multitexReducer.cxx', obj='grutil_multitexReducer.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='grutil_composite.cxx', obj='grutil_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libgrutil.in', obj='libgrutil_igate.obj',
            src='panda/src/grutil',  module='panda', library='libgrutil',
            skip=[], also=["multitexReducer.cxx","grutil_composite.cxx"])

#
# DIRECTORY: panda/src/tform/
#

IPATH=['panda/src/tform']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='tform_composite.cxx', obj='tform_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libtform.in', obj='libtform_igate.obj',
            src='panda/src/tform',  module='panda', library='libtform',
            skip=[], also=["tform_composite.cxx"])

#
# DIRECTORY: panda/src/collide/
#

IPATH=['panda/src/collide']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='collide_composite.cxx', obj='collide_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libcollide.in', obj='libcollide_igate.obj',
            src='panda/src/collide',  module='panda', library='libcollide',
            skip=["collide_headers.h"], also=["collide_composite.cxx"])

#
# DIRECTORY: panda/src/parametrics/
#

IPATH=['panda/src/parametrics']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='parametrics_composite.cxx', obj='parametrics_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libparametrics.in', obj='libparametrics_igate.obj',
            src='panda/src/parametrics',  module='panda', library='libparametrics',
            skip=['nurbsPPCurve.h'], also=["parametrics_composite.cxx"])

#
# DIRECTORY: panda/src/pgui/
#

IPATH=['panda/src/pgui']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pgui_composite.cxx', obj='pgui_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libpgui.in', obj='libpgui_igate.obj',
            src='panda/src/pgui',  module='panda', library='libpgui',
            skip=[], also=["pgui_composite.cxx"])

#
# DIRECTORY: panda/src/pnmimagetypes/
#

IPATH=['panda/src/pnmimagetypes', 'panda/src/pnmimage']
OPTS=['BUILDING_PANDA', 'PNG', 'ZLIB', 'JPEG', 'ZLIB',  'JPEG', 'TIFF']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pnmFileTypePNG.cxx', obj='pnmimagetypes_pnmFileTypePNG.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pnmFileTypeTIFF.cxx', obj='pnmimagetypes_pnmFileTypeTIFF.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pnmimagetypes_composite.cxx', obj='pnmimagetypes_composite.obj')

#
# DIRECTORY: panda/src/recorder/
#

IPATH=['panda/src/recorder']
OPTS=['BUILDING_PANDA']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='recorder_composite.cxx', obj='recorder_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='librecorder.in', obj='librecorder_igate.obj',
            src='panda/src/recorder',  module='panda', library='librecorder',
            skip=[], also=["recorder_composite.cxx"])

#
# DIRECTORY: panda/src/vrpn/
#

if (OMIT.count("VRPN")==0):
    IPATH=['panda/src/vrpn']
    OPTS=['BUILDING_PANDA',  'VRPN']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='vrpn_composite.cxx', obj='pvrpn_composite.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libpvrpn.in', obj='libpvrpn_igate.obj',
                src='panda/src/vrpn',  module='panda', library='libpvrpn',
                skip=[], also=["vrpn_composite.cxx"])


#
# DIRECTORY: panda/metalibs/panda/
#

IPATH=['panda/metalibs/panda']
OPTS=['BUILDING_PANDA', 'ZLIB', 'VRPN', 'JPEG', 'PNG', 'TIFF', 'ZLIB',  'NVIDIACG', 'OPENSSL', 'FREETYPE', 'FFTW', 'ADVAPI', 'WINSOCK2', 'WINUSER', 'WINMM', 'FFMPEG']
INFILES=['librecorder.in', 'libpgraph.in', 'libcull.in', 'libgrutil.in', 'libchan.in', 'libpstatclient.in',
         'libchar.in', 'libcollide.in', 'libdevice.in', 'libdgraph.in', 'libdisplay.in', 'libpipeline.in', 'libevent.in',
         'libgobj.in', 'libgsgbase.in', 'liblinmath.in', 'libmathutil.in', 'libparametrics.in',
         'libpnmimage.in', 'libtext.in', 'libtform.in', 'liblerp.in', 'libputil.in', 'libaudio.in',
         'libnativenet.in', 'libnet.in', 'libpgui.in']
OBJFILES=['panda_panda.obj', 'libpanda_module.obj',
          'recorder_composite.obj', 'librecorder_igate.obj',
          'pgraph_nodePath.obj', 
          'pgraph_composite1.obj', 'pgraph_composite2.obj', 'pgraph_composite3.obj', 'pgraph_composite4.obj', 'libpgraph_igate.obj',
          'cull_composite.obj',
          'grutil_multitexReducer.obj', 'grutil_composite.obj', 'libgrutil_igate.obj',
          'chan_composite.obj', 'libchan_igate.obj',
          'pstatclient_composite.obj', 'libpstatclient_igate.obj',
          'char_composite.obj', 'libchar_igate.obj',
          'collide_composite.obj', 'libcollide_igate.obj',
          'device_composite.obj', 'libdevice_igate.obj',
          'dgraph_composite.obj', 'libdgraph_igate.obj',
          'display_composite.obj', 'libdisplay_igate.obj',
          'pipeline_composite.obj', 'libpipeline_igate.obj',
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
          'net_composite.obj', 'libnet_igate.obj',
          'nativenet_composite.obj', 'libnativenet_igate.obj',
          'pandabase_pandabase.obj', 'libpandaexpress.dll', 'libp3dtoolconfig.dll', 'libp3dtool.dll']
if OMIT.count("VRPN")==0:
    OBJFILES.append("pvrpn_composite.obj")
    OBJFILES.append("libpvrpn_igate.obj")
    INFILES.append("libpvrpn.in")
if OMIT.count("FREETYPE")==0:
    OBJFILES.append("pnmtext_composite.obj")
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
# DIRECTORY: panda/src/skel
#

IPATH=['panda/src/skel']
OPTS=['BUILDING_PANDASKEL']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='skel_composite.cxx', obj='skel_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libskel.in', obj='libskel_igate.obj',
             src='panda/src/skel',  module='pandaskel', library='libskel',
             skip=[], also=["skel_composite.cxx"])
EnqueueImod(ipath=IPATH, opts=OPTS, obj='libpandaskel_module.obj',
            module='pandaskel', library='libpandaskel', files=["libskel.in"])
EnqueueLink(dll='libpandaskel.dll', opts=['ADVAPI'], obj=[
    'skel_composite.obj',
    'libskel_igate.obj',
    'libpandaskel_module.obj',
    'libpanda.dll',
    'libpandaexpress.dll',
    'libp3dtoolconfig.dll',
    'libp3dtool.dll',
    ])

#
# DIRECTORY: panda/src/audiotraits/
#

if OMIT.count("FMODEX") == 0:
  IPATH=['panda/src/audiotraits']
  OPTS=['BUILDING_FMOD_AUDIO',  'FMODEX']
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='fmod_audio_composite.cxx', obj='fmod_audio_fmod_audio_composite.obj')
  EnqueueLink(opts=['ADVAPI', 'WINUSER', 'WINMM', 'FMODEX'], dll='libp3fmod_audio.dll', obj=[
               'fmod_audio_fmod_audio_composite.obj',
               'libpanda.dll',
               'libpandaexpress.dll',
               'libp3dtoolconfig.dll',
               'libp3dtool.dll',
  ])

if OMIT.count("MILES") == 0:
  IPATH=['panda/src/audiotraits']
  OPTS=['BUILDING_MILES_AUDIO',  'MILES']
  EnqueueCxx(ipath=IPATH, opts=OPTS, src='miles_audio_composite.cxx', obj='miles_audio_miles_audio_composite.obj')
  EnqueueLink(opts=['ADVAPI', 'WINUSER', 'WINMM', 'MILES'], dll='libp3miles_audio.dll', obj=[
               'miles_audio_miles_audio_composite.obj',
               'libpanda.dll',
               'libpandaexpress.dll',
               'libp3dtoolconfig.dll',
               'libp3dtool.dll',
  ])

#
# DIRECTORY: panda/src/distort/
#

IPATH=['panda/src/distort']
OPTS=['BUILDING_PANDAFX']
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
    OPTS=['OPENSSL', 'ZLIB', 'ADVAPI']
    LIBS=['libpandaexpress.dll', 'libpanda.dll', 'libp3dtoolconfig.dll', 'libp3dtool.dll', 'libp3pystub.dll']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='apply_patch.cxx', obj='apply_patch_apply_patch.obj')
    EnqueueLink(dll='apply_patch.exe', opts=OPTS, obj=['apply_patch_apply_patch.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='build_patch.cxx', obj='build_patch_build_patch.obj')
    EnqueueLink(dll='build_patch.exe', opts=OPTS, obj=['build_patch_build_patch.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='check_adler.cxx', obj='check_adler_check_adler.obj')
    EnqueueLink(dll='check_adler.exe', opts=OPTS, obj=['check_adler_check_adler.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='check_crc.cxx', obj='check_crc_check_crc.obj')
    EnqueueLink(dll='check_crc.exe', opts=OPTS, obj=['check_crc_check_crc.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='check_md5.cxx', obj='check_md5_check_md5.obj')
    EnqueueLink(dll='check_md5.exe', opts=OPTS, obj=['check_md5_check_md5.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='multify.cxx', obj='multify_multify.obj')
    EnqueueLink(dll='multify.exe', opts=OPTS, obj=['multify_multify.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pzip.cxx', obj='pzip_pzip.obj')
    EnqueueLink(dll='pzip.exe', opts=OPTS, obj=['pzip_pzip.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='punzip.cxx', obj='punzip_punzip.obj')
    EnqueueLink(dll='punzip.exe', opts=OPTS, obj=['punzip_punzip.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pdecrypt.cxx', obj='pdecrypt_pdecrypt.obj')
    EnqueueLink(dll='pdecrypt.exe', opts=OPTS, obj=['pdecrypt_pdecrypt.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pencrypt.cxx', obj='pencrypt_pencrypt.obj')
    EnqueueLink(dll='pencrypt.exe', opts=OPTS, obj=['pencrypt_pencrypt.obj']+LIBS)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='show_ddb.cxx', obj='show_ddb_show_ddb.obj')
    EnqueueLink(dll='show_ddb.exe', opts=OPTS, obj=['show_ddb_show_ddb.obj']+LIBS)

#
# DIRECTORY: panda/src/windisplay/
#

if (sys.platform == "win32"):
    IPATH=['panda/src/windisplay']
    OPTS=['BUILDING_PANDAWIN']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='windisplay_composite.cxx', obj='windisplay_composite.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS + ["DX8"], src='winDetectDx8.cxx', obj='windisplay_windetectdx8.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS + ["DX9"], src='winDetectDx9.cxx', obj='windisplay_windetectdx9.obj')
    EnqueueLink(opts=['WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM'],
                dll='libp3windisplay.dll', obj=[
      'windisplay_composite.obj',
      'windisplay_windetectdx8.obj',
      'windisplay_windetectdx9.obj',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libp3dtoolconfig.dll',
      'libp3dtool.dll',
      ])

#
# DIRECTORY: panda/metalibs/pandadx7/
#
# 
# if OMIT.count("DX7")==0:
#     IPATH=['panda/src/dxgsg7']
#     OPTS=['BUILDING_PANDADX', 'DXSDK']
#     EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxGraphicsStateGuardian7.cxx', obj='dxgsg7_dxGraphicsStateGuardian7.obj')
#     EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxgsg7_composite.cxx', obj='dxgsg7_composite.obj')
# 
#     IPATH=['panda/metalibs/pandadx7']
#     OPTS=['BUILDING_PANDADX', 'DXSDK']
#     EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandadx7.cxx', obj='pandadx7_pandadx7.obj')
#     EnqueueLink(dll='libpandadx7.dll', opts=['ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DXDRAW', 'DXGUID', 'D3D8'], obj=[
#       'pandadx7_pandadx7.obj',
#       'dxgsg7_dxGraphicsStateGuardian7.obj',
#       'dxgsg7_composite.obj',
#       'libpanda.dll',
#       'libpandaexpress.dll',
#       'libp3windisplay.dll',
#       'libp3dtoolconfig.dll',
#       'libp3dtool.dll',
#       ])
# 

#
# DIRECTORY: panda/metalibs/pandadx8/
#

if OMIT.count("DX8")==0:
    IPATH=['panda/src/dxgsg8', 'panda/metalibs/pandadx8']
    OPTS=['BUILDING_PANDADX', 'DX8']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxGraphicsStateGuardian8.cxx', obj='dxgsg8_dxGraphicsStateGuardian8.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxgsg8_composite.cxx', obj='dxgsg8_composite.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandadx8.cxx', obj='pandadx8_pandadx8.obj')
    EnqueueLink(dll='libpandadx8.dll',
      opts=['ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DX8'], obj=[
      'pandadx8_pandadx8.obj',
      'dxgsg8_dxGraphicsStateGuardian8.obj',
      'dxgsg8_composite.obj',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libp3windisplay.dll',
      'libp3dtoolconfig.dll',
      'libp3dtool.dll',
      ])

#
# DIRECTORY: panda/metalibs/pandadx9/
#

if OMIT.count("DX9")==0:
    IPATH=['panda/src/dxgsg9']
    OPTS=['BUILDING_PANDADX', 'DX9',  'NVIDIACG', 'CGDX9']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxGraphicsStateGuardian9.cxx', obj='dxgsg9_dxGraphicsStateGuardian9.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxgsg9_composite.cxx', obj='dxgsg9_composite.obj')
    IPATH=['panda/metalibs/pandadx9']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandadx9.cxx', obj='pandadx9_pandadx9.obj')
    EnqueueLink(dll='libpandadx9.dll',
      opts=['ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DX9',  'NVIDIACG', 'CGDX9'], obj=[
      'pandadx9_pandadx9.obj',
      'dxgsg9_dxGraphicsStateGuardian9.obj',
      'dxgsg9_composite.obj',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libp3windisplay.dll',
      'libp3dtoolconfig.dll',
      'libp3dtool.dll',
      ])

#
# DIRECTORY: panda/src/egg/
#

IPATH=['panda/src/egg']
OPTS=['BUILDING_PANDAEGG',  'ZLIB']
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
OPTS=['BUILDING_PANDAEGG']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='egg2pg_composite.cxx', obj='egg2pg_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libegg2pg.in', obj='libegg2pg_igate.obj',
            src='panda/src/egg2pg',  module='pandaegg', library='libegg2pg',
            skip="ALL", also=['load_egg_file.h'])

#
# DIRECTORY: panda/src/framework/
#

IPATH=['panda/src/framework']
OPTS=['BUILDING_FRAMEWORK']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='framework_composite.cxx', obj='framework_composite.obj')
EnqueueLink(dll='libp3framework.dll', opts=['ADVAPI'], obj=[
             'framework_composite.obj',
             'libpanda.dll',
             'libpandaexpress.dll',
             'libp3dtoolconfig.dll',
             'libp3dtool.dll',
             ])

#
# DIRECTORY: panda/metalibs/pandafx/
#

IPATH=['panda/metalibs/pandafx', 'panda/src/distort']
OPTS=['BUILDING_PANDAFX',  'NVIDIACG']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandafx.cxx', obj='pandafx_pandafx.obj')
EnqueueImod(ipath=IPATH, opts=OPTS, obj='libpandafx_module.obj',
            module='pandafx', library='libpandafx',
            files=['libdistort.in', 'libeffects.in'])
EnqueueLink(dll='libpandafx.dll', opts=['ADVAPI',  'NVIDIACG'], obj=[
             'pandafx_pandafx.obj',
             'libpandafx_module.obj',
             'distort_composite.obj',
             'libdistort_igate.obj',
             'effects_composite.obj',
             'libeffects_igate.obj',
             'libpanda.dll',
             'libpandaexpress.dll',
             'libp3dtoolconfig.dll',
             'libp3dtool.dll',
])

#
# DIRECTORY: panda/src/glstuff/
#

IPATH=['panda/src/glstuff']
OPTS=[ 'NVIDIACG', 'CGGL']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='glpure.cxx', obj='glstuff_glpure.obj')
EnqueueLink(dll='libp3glstuff.dll', opts=['ADVAPI', 'GLUT',  'NVIDIACG', 'CGGL'], obj=[
             'glstuff_glpure.obj',
             'libpanda.dll',
             'libpandafx.dll',
             'libpandaexpress.dll',
             'libp3dtoolconfig.dll',
             'libp3dtool.dll',
])

#
# DIRECTORY: panda/src/glgsg/
#

IPATH=['panda/src/glgsg', 'panda/src/glstuff', 'panda/src/gobj']
OPTS=['BUILDING_PANDAGL',  'NVIDIACG']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='config_glgsg.cxx', obj='glgsg_config_glgsg.obj')
EnqueueCxx(ipath=IPATH, opts=OPTS, src='glgsg.cxx', obj='glgsg_glgsg.obj')

#
# DIRECTORY: panda/metalibs/pandaegg/
#

IPATH=['panda/metalibs/pandaegg', 'panda/src/egg']
OPTS=['BUILDING_PANDAEGG']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandaegg.cxx', obj='pandaegg_pandaegg.obj')
EnqueueImod(ipath=IPATH, opts=OPTS, obj='libpandaegg_module.obj',
            module='pandaegg', library='libpandaegg',
            files=['libegg2pg.in', 'libegg.in'])
EnqueueLink(dll='libpandaegg.dll', opts=['ADVAPI'], obj=[
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
             'libp3dtoolconfig.dll',
             'libp3dtool.dll',
])

#
# DIRECTORY: panda/metalibs/panda/
#

IPATH=['panda/metalibs/panda']
OPTS=['BUILDING_PANDA',  'FFTW', 'PNG', 'JPEG', 'TIFF', 'ZLIB', 'ADVAPI', 'WINSOCK2', 'WINUSER', 'WINMM']
OBJFILES=[
          'pipeline_composite.obj',
          'event_composite.obj',
          'linmath_composite.obj',
          'mathutil_composite.obj',
          'putil_composite1.obj', 'putil_composite2.obj',
          'pnmimagetypes_composite.obj', 'pnmimagetypes_pnmFileTypePNG.obj', 'pnmimagetypes_pnmFileTypeTIFF.obj',
          'pnmimage_composite.obj',
          'pandabase_pandabase.obj', 'libpandaexpress.dll', 'libp3dtoolconfig.dll', 'libp3dtool.dll']
EnqueueLink(opts=OPTS, dll='libpandastripped.dll', obj=OBJFILES, xdep=[
        'built/tmp/dtool_have_helix.dat',
        'built/tmp/dtool_have_vrpn.dat',
        'built/tmp/dtool_have_freetype.dat',
])

#
# DIRECTORY: panda/metalibs/pandaegg/
#

IPATH=['panda/metalibs/pandaegg', 'panda/src/egg']
OPTS=['BUILDING_PANDAEGG']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandaeggnopg.cxx', obj='pandaegg_pandaeggnopg.obj')
EnqueueLink(dll='libpandaeggstripped.dll', opts=['ADVAPI'], obj=[
             'pandaegg_pandaeggnopg.obj',
#             'egg2pg_composite.obj',
             'egg_composite1.obj',
             'egg_composite2.obj',
             'egg_parser.obj',
             'egg_lexer.obj',
             'libpandastripped.dll',
             'libpandaexpress.dll',
             'libp3dtoolconfig.dll',
             'libp3dtool.dll',
])

#
# DIRECTORY: panda/src/mesadisplay/
#

if (sys.platform != "win32"):
    IPATH=['panda/src/mesadisplay', 'panda/src/glstuff']
    OPTS=['BUILDING_PANDAGLUT', 'NVIDIACG', 'GLUT']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mesadisplay_composite.cxx', obj='mesadisplay_composite.obj')
    IPATH=['panda/metalibs/pandagl']
    EnqueueLink(opts=['GLUT'], dll='libpandamesa.dll', obj=[
      'mesadisplay_composite.obj',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libp3glstuff.dll',
      'libpandafx.dll',
      'libp3dtoolconfig.dll',
      'libp3dtool.dll',
      ])

#
# DIRECTORY: panda/src/glxdisplay/
#

if (sys.platform != "win32"):
    IPATH=['panda/src/glxdisplay']
    OPTS=['BUILDING_PANDAGLUT',  'GLUT', 'NVIDIACG', 'CGGL']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='glxdisplay_composite.cxx', obj='glxdisplay_composite.obj')
    IPATH=['panda/metalibs/pandagl']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandagl.cxx', obj='pandagl_pandagl.obj')
    EnqueueLink(opts=['GLUT', 'NVIDIACG', 'CGGL'], dll='libpandagl.dll', obj=[
      'pandagl_pandagl.obj',
      'glgsg_config_glgsg.obj',
      'glgsg_glgsg.obj',
      'glxdisplay_composite.obj',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libp3glstuff.dll',
      'libpandafx.dll',
      'libp3dtoolconfig.dll',
      'libp3dtool.dll',
      ])

#
# DIRECTORY: panda/src/wgldisplay/
#

if (sys.platform == "win32"):
    IPATH=['panda/src/wgldisplay', 'panda/src/glstuff']
    OPTS=['BUILDING_PANDAGL',  'NVIDIACG', 'CGGL']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='wgldisplay_composite.cxx', obj='wgldisplay_composite.obj')
    IPATH=['panda/metalibs/pandagl']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandagl.cxx', obj='pandagl_pandagl.obj')
    EnqueueLink(opts=['WINGDI', 'GLUT', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM',  'NVIDIACG', 'CGGL'],
                dll='libpandagl.dll', obj=[
      'pandagl_pandagl.obj',
      'glgsg_config_glgsg.obj',
      'glgsg_glgsg.obj',
      'wgldisplay_composite.obj',
      'libp3windisplay.dll',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libp3glstuff.dll',
      'libpandafx.dll',
      'libp3dtoolconfig.dll',
      'libp3dtool.dll',
      ])

#
# DIRECTORY: panda/src/physics/
#

IPATH=['panda/src/physics']
OPTS=['BUILDING_PANDAPHYSICS']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='physics_composite.cxx', obj='physics_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libphysics.in', obj='libphysics_igate.obj',
            src='panda/src/physics',  module='pandaphysics', library='libphysics',
            skip=["forces.h"], also=["physics_composite.cxx"])

#
# DIRECTORY: panda/src/particlesystem/
#

IPATH=['panda/src/particlesystem']
OPTS=['BUILDING_PANDAPHYSICS']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='particlesystem_composite.cxx', obj='particlesystem_composite.obj')
EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libparticlesystem.in', obj='libparticlesystem_igate.obj',
            src='panda/src/particlesystem',  module='pandaphysics', library='libparticlesystem',
            skip=['orientedParticle.h', 'orientedParticleFactory.h', 'particlefactories.h', 'emitters.h', 'particles.h'],
            also=["particlesystem_composite.cxx"])

#
# DIRECTORY: panda/metalibs/pandaphysics/
#

IPATH=['panda/metalibs/pandaphysics']
OPTS=['BUILDING_PANDAPHYSICS']
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandaphysics.cxx', obj='pandaphysics_pandaphysics.obj')
EnqueueImod(ipath=IPATH, opts=OPTS, obj='libpandaphysics_module.obj',
            module='pandaphysics', library='libpandaphysics',
            files=['libphysics.in', 'libparticlesystem.in'])
EnqueueLink(dll='libpandaphysics.dll', opts=['ADVAPI'], obj=[
             'pandaphysics_pandaphysics.obj',
             'libpandaphysics_module.obj',
             'physics_composite.obj',
             'libphysics_igate.obj',
             'particlesystem_composite.obj',
             'libparticlesystem_igate.obj',
             'libpanda.dll',
             'libpandaexpress.dll',
             'libp3dtoolconfig.dll',
             'libp3dtool.dll',
])

#
# DIRECTORY: panda/src/testbed/
#

IPATH=['panda/src/testbed']
OPTS=[]
EnqueueCxx(ipath=IPATH, opts=OPTS, src='pview.cxx', obj='pview_pview.obj')
EnqueueLink(dll='pview.exe', opts=['ADVAPI'], obj=[
             'pview_pview.obj',
             'libp3framework.dll',
             'libpanda.dll',
             'libpandafx.dll',
             'libpandaexpress.dll',
             'libp3dtoolconfig.dll',
             'libp3dtool.dll',
             'libp3pystub.dll',
])

#
# DIRECTORY: direct/src/directbase/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/directbase']
    OPTS=['BUILDING_DIRECT']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='directbase.cxx', obj='directbase_directbase.obj')
    if (sys.platform != "win32"):
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
    OPTS=['WITHINPANDA', 'BUILDING_DIRECT']
    EnqueueBison(ipath=IPATH, opts=OPTS, pre='dcyy', src='dcParser.yxx', dsth='dcParser.h', obj='dcparser_dcParser.obj')
    EnqueueFlex(ipath=IPATH, opts=OPTS, pre='dcyy', src='dcLexer.lxx', obj='dcparser_dcLexer.obj', dashi=0)
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dcparser_composite.cxx', obj='dcparser_composite.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdcparser.in', obj='libdcparser_igate.obj',
                src='direct/src/dcparser',  module='p3direct', library='libdcparser',
                skip=['dcmsgtypes.h'],
                also=["dcparser_composite.cxx"])

#
# DIRECTORY: direct/src/deadrec/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/deadrec']
    OPTS=['BUILDING_DIRECT']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='deadrec_composite.cxx', obj='deadrec_composite.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdeadrec.in', obj='libdeadrec_igate.obj',
                src='direct/src/deadrec',  module='p3direct', library='libdeadrec',
                skip=[], also=["deadrec_composite.cxx"])

#
# DIRECTORY: direct/src/distributed/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/distributed', 'direct/src/dcparser']
    OPTS=['WITHINPANDA', 'BUILDING_DIRECT', 'OPENSSL']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='config_distributed.cxx', obj='distributed_config_distributed.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='cConnectionRepository.cxx', obj='distributed_cConnectionRepository.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='cDistributedSmoothNodeBase.cxx', obj='distributed_cDistributedSmoothNodeBase.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libdistributed.in', obj='libdistributed_igate.obj',
                src='direct/src/distributed',  module='p3direct', library='libdistributed',
                skip=[], also=['config_distributed.cxx', 'cConnectionRepository.cxx', 'cDistributedSmoothNodeBase.cxx'])

#
# DIRECTORY: direct/src/interval/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/interval']
    OPTS=['BUILDING_DIRECT']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='interval_composite.cxx', obj='interval_composite.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libinterval.in', obj='libinterval_igate.obj',
                src='direct/src/interval',  module='p3direct', library='libinterval',
                skip=[], also=["interval_composite.cxx"])

#
# DIRECTORY: direct/src/showbase/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/showbase']
    OPTS=['BUILDING_DIRECT']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='showBase.cxx', obj='showbase_showBase.obj')
    EnqueueIgate(ipath=IPATH, opts=OPTS, outd='libshowbase.in', obj='libshowbase_igate.obj',
                src='direct/src/showbase', module='p3direct', library='libshowbase',
                skip=[], also=["showBase.cxx"])

#
# DIRECTORY: direct/metalibs/direct/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/metalibs/direct']
    OPTS=['BUILDING_DIRECT']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='direct.cxx', obj='direct_direct.obj')
    EnqueueImod(ipath=IPATH, opts=OPTS, obj='libp3direct_module.obj',
                module='p3direct', library='libp3direct',
                files=['libdcparser.in', 'libshowbase.in', 'libdeadrec.in', 'libinterval.in', 'libdistributed.in'])
    EnqueueLink(dll='libp3direct.dll', opts=['ADVAPI',  'OPENSSL'], obj=[
                 'direct_direct.obj',
                 'libp3direct_module.obj',
                 'directbase_directbase.obj',
                 'dcparser_composite.obj',
                 'dcparser_dcParser.obj',
                 'dcparser_dcLexer.obj',
                 'libdcparser_igate.obj',
                 'showbase_showBase.obj',
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
    ])

#
# DIRECTORY: direct/src/dcparse/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/dcparse', 'direct/src/dcparser']
    OPTS=['WITHINPANDA', 'ADVAPI']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dcparse.cxx', obj='dcparse_dcparse.obj')
    EnqueueLink(dll='dcparse.exe', opts=OPTS, obj=[
                 'dcparse_dcparse.obj',
                 'libp3direct.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: direct/src/heapq/
#

if (OMIT.count("PYTHON")==0):
    IPATH=['direct/src/heapq']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='heapq.cxx', obj='heapq_heapq.obj')
    EnqueueLink(dll='libp3heapq.dll', opts=['ADVAPI'], obj=[
                 'heapq_heapq.obj',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
    ])

#
# DIRECTORY: pandatool/src/pandatoolbase/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/pandatoolbase']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandatoolbase_composite1.cxx', obj='pandatoolbase_composite1.obj')
    EnqueueLib(lib='libpandatoolbase.lib', obj=['pandatoolbase_composite1.obj'])

#
# DIRECTORY: pandatool/src/converter/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/converter']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='somethingToEggConverter.cxx', obj='converter_somethingToEggConverter.obj')
    EnqueueLib(lib='libconverter.lib', obj=['converter_somethingToEggConverter.obj'])

#
# DIRECTORY: pandatool/src/progbase/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/progbase']
    OPTS=[ 'ZLIB']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='progbase_composite1.cxx', obj='progbase_composite1.obj')
    EnqueueLib(lib='libprogbase.lib', obj=['progbase_composite1.obj'])

#
# DIRECTORY: pandatool/src/eggbase/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/eggbase']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggbase_composite1.cxx', obj='eggbase_composite1.obj')
    EnqueueLib(lib='libeggbase.lib', obj=['eggbase_composite1.obj'])

#
# DIRECTORY: pandatool/src/bam/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/bam']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='bamInfo.cxx', obj='bam-info_bamInfo.obj')
    EnqueueLink(dll='bam-info.exe', opts=['ADVAPI',  'FFTW'], obj=[
                 'bam-info_bamInfo.obj',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='bamToEgg.cxx', obj='bam2egg_bamToEgg.obj')
    EnqueueLink(dll='bam2egg.exe', opts=['ADVAPI',  'FFTW'], obj=[
                 'bam2egg_bamToEgg.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggToBam.cxx', obj='egg2bam_eggToBam.obj')
    EnqueueLink(dll='egg2bam.exe', opts=['ADVAPI',  'FFTW'], obj=[
                 'egg2bam_eggToBam.obj',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libconverter.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    
#
# DIRECTORY: pandatool/src/cvscopy/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/cvscopy']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='cvscopy_composite1.cxx', obj='cvscopy_composite1.obj')
    EnqueueLib(lib='libcvscopy.lib', obj=['cvscopy_composite1.obj'])

#
# DIRECTORY: pandatool/src/dxf/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/dxf']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxf_composite1.cxx', obj='dxf_composite1.obj')
    EnqueueLib(lib='libdxf.lib', obj=['dxf_composite1.obj'])

#
# DIRECTORY: pandatool/src/dxfegg/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/dxfegg']
    OPTS=[]
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
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxfPoints.cxx', obj='dxf-points_dxfPoints.obj')
    EnqueueLink(dll='dxf-points.exe', opts=['ADVAPI',  'FFTW'], obj=[
                 'dxf-points_dxfPoints.obj',
                 'libprogbase.lib',
                 'libdxf.lib',
                 'libpandatoolbase.lib',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='dxfToEgg.cxx', obj='dxf2egg_dxfToEgg.obj')
    EnqueueLink(dll='dxf2egg.exe', opts=['ADVAPI',  'FFTW'], obj=[
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggToDXF.cxx', obj='egg2dxf_eggToDXF.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggToDXFLayer.cxx', obj='egg2dxf_eggToDXFLayer.obj')
    EnqueueLink(dll='egg2dxf.exe', opts=['ADVAPI',  'FFTW'], obj=[
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandatool/src/palettizer/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/palettizer']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='palettizer_composite1.cxx', obj='palettizer_composite1.obj')
    EnqueueLib(lib='libpalettizer.lib', obj=['palettizer_composite1.obj'])

#
# DIRECTORY: pandatool/src/egg-mkfont/
#

if (OMIT.count("FREETYPE")==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/egg-mkfont', 'pandatool/src/palettizer']
    OPTS=[ 'FREETYPE']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggMakeFont.cxx', obj='egg-mkfont_eggMakeFont.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='rangeDescription.cxx', obj='egg-mkfont_rangeDescription.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='rangeIterator.cxx', obj='egg-mkfont_rangeIterator.obj')
    EnqueueLink(dll='egg-mkfont.exe', opts=['ADVAPI',  'FREETYPE'], obj=[
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandatool/src/eggcharbase/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/eggcharbase']
    OPTS=['ZLIB']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggcharbase_composite1.cxx', obj='eggcharbase_composite1.obj')
    EnqueueLib(lib='libeggcharbase.lib', obj=['eggcharbase_composite1.obj'])

#
# DIRECTORY: pandatool/src/egg-optchar/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/egg-optchar']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='config_egg_optchar.cxx', obj='egg-optchar_config_egg_optchar.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggOptchar.cxx', obj='egg-optchar_eggOptchar.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggOptcharUserData.cxx', obj='egg-optchar_eggOptcharUserData.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='vertexMembership.cxx', obj='egg-optchar_vertexMembership.obj')
    EnqueueLink(dll='egg-optchar.exe', opts=['ADVAPI'], obj=[
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandatool/src/egg-palettize/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/egg-palettize', 'pandatool/src/palettizer']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggPalettize.cxx', obj='egg-palettize_eggPalettize.obj')
    EnqueueLink(dll='egg-palettize.exe', opts=['ADVAPI'], obj=[
                 'egg-palettize_eggPalettize.obj',
                 'libpalettizer.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandatool/src/egg-qtess/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/egg-qtess']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='egg-qtess_composite1.cxx', obj='egg-qtess_composite1.obj')
    EnqueueLink(dll='egg-qtess.exe', opts=['ADVAPI'], obj=[
                 'egg-qtess_composite1.obj',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libconverter.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    
#
# DIRECTORY: pandatool/src/eggprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/eggprogs']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggCrop.cxx', obj='egg-crop_eggCrop.obj')
    EnqueueLink(dll='egg-crop.exe', opts=['ADVAPI'], obj=[
                 'egg-crop_eggCrop.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggMakeTube.cxx', obj='egg-make-tube_eggMakeTube.obj')
    EnqueueLink(dll='egg-make-tube.exe', opts=['ADVAPI'], obj=[
                 'egg-make-tube_eggMakeTube.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggTextureCards.cxx', obj='egg-texture-cards_eggTextureCards.obj')
    EnqueueLink(dll='egg-texture-cards.exe', opts=['ADVAPI'], obj=[
                 'egg-texture-cards_eggTextureCards.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggTopstrip.cxx', obj='egg-topstrip_eggTopstrip.obj')
    EnqueueLink(dll='egg-topstrip.exe', opts=['ADVAPI'], obj=[
                 'egg-topstrip_eggTopstrip.obj',
                 'libeggcharbase.lib',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggTrans.cxx', obj='egg-trans_eggTrans.obj')
    EnqueueLink(dll='egg-trans.exe', opts=['ADVAPI'], obj=[
                 'egg-trans_eggTrans.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggToC.cxx', obj='egg2c_eggToC.obj')
    EnqueueLink(dll='egg2c.exe', opts=['ADVAPI'], obj=[
                 'egg2c_eggToC.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggRename.cxx', obj='egg-rename_eggRename.obj')
    EnqueueLink(dll='egg-rename.exe', opts=['ADVAPI'], obj=[
                 'egg-rename_eggRename.obj',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggRetargetAnim.cxx', obj='egg-retarget-anim_eggRetargetAnim.obj')
    EnqueueLink(dll='egg-retarget-anim.exe', opts=['ADVAPI'], obj=[
                 'egg-retarget-anim_eggRetargetAnim.obj',
                 'libeggcharbase.lib',
                 'libconverter.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandatool/src/flt/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/flt']
    OPTS=[ 'ZLIB']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltVectorRecord.cxx', obj='flt_fltVectorRecord.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='flt_composite1.cxx', obj='flt_composite1.obj')
    EnqueueLib(lib='libflt.lib', obj=['flt_fltVectorRecord.obj', 'flt_composite1.obj'])

#
# DIRECTORY: pandatool/src/fltegg/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/fltegg']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltToEggConverter.cxx', obj='fltegg_fltToEggConverter.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltToEggLevelState.cxx', obj='fltegg_fltToEggLevelState.obj')
    EnqueueLib(lib='libfltegg.lib', obj=['fltegg_fltToEggConverter.obj', 'fltegg_fltToEggLevelState.obj'])

#
# DIRECTORY: pandatool/src/fltprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/fltprogs', 'pandatool/src/flt', 'pandatool/src/cvscopy']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggToFlt.cxx', obj='egg2flt_eggToFlt.obj')
    EnqueueLink(dll='egg2flt.exe', opts=['ADVAPI'], obj=[
                 'egg2flt_eggToFlt.obj',
                 'libflt.lib',
                 'libeggbase.lib',
                 'libprogbase.lib',
                 'libconverter.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltInfo.cxx', obj='flt-info_fltInfo.obj')
    EnqueueLink(dll='flt-info.exe', opts=['ADVAPI'], obj=[
                 'flt-info_fltInfo.obj',
                 'libprogbase.lib',
                 'libflt.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltTrans.cxx', obj='flt-trans_fltTrans.obj')
    EnqueueLink(dll='flt-trans.exe', opts=['ADVAPI'], obj=[
                 'flt-trans_fltTrans.obj',
                 'libprogbase.lib',
                 'libflt.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltToEgg.cxx', obj='flt2egg_fltToEgg.obj')
    EnqueueLink(dll='flt2egg.exe', opts=['ADVAPI'], obj=[
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fltCopy.cxx', obj='fltcopy_fltCopy.obj')
    EnqueueLink(dll='fltcopy.exe', opts=['ADVAPI'], obj=[
                 'fltcopy_fltCopy.obj',
                 'libcvscopy.lib',
                 'libflt.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandatool/src/imagebase/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/imagebase']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='imagebase_composite1.cxx', obj='imagebase_composite1.obj')
    EnqueueLib(lib='libimagebase.lib', obj=['imagebase_composite1.obj'])

#
# DIRECTORY: pandatool/src/imageprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/imageprogs']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='imageInfo.cxx', obj='image-info_imageInfo.obj')
    EnqueueLink(dll='image-info.exe', opts=['ADVAPI'], obj=[
                 'image-info_imageInfo.obj',
                 'libimagebase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='imageResize.cxx', obj='image-resize_imageResize.obj')
    EnqueueLink(dll='image-resize.exe', opts=['ADVAPI'], obj=[
                 'image-resize_imageResize.obj',
                 'libimagebase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='imageTrans.cxx', obj='image-trans_imageTrans.obj')
    EnqueueLink(dll='image-trans.exe', opts=['ADVAPI'], obj=[
                 'image-trans_imageTrans.obj',
                 'libimagebase.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandatool/src/lwo/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/lwo']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='lwo_composite1.cxx', obj='lwo_composite1.obj')
    EnqueueLib(lib='liblwo.lib', obj=['lwo_composite1.obj'])

#
# DIRECTORY: pandatool/src/lwoegg/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/lwoegg']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='lwoegg_composite1.cxx', obj='lwoegg_composite1.obj')
    EnqueueLib(lib='liblwoegg.lib', obj=['lwoegg_composite1.obj'])

#
# DIRECTORY: pandatool/src/lwoprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/lwoprogs', 'pandatool/src/lwo']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='lwoScan.cxx', obj='lwo-scan_lwoScan.obj')
    EnqueueLink(dll='lwo-scan.exe', opts=['ADVAPI'], obj=[
                 'lwo-scan_lwoScan.obj',
                 'liblwo.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='lwoToEgg.cxx', obj='lwo2egg_lwoToEgg.obj')
    EnqueueLink(dll='lwo2egg.exe', opts=['ADVAPI'], obj=[
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandatool/src/maya/
#

for VER in MAYAVERSIONS:
  if (OMIT.count("MAYA"+VER)==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/maya']
    OPTS=['MAYA'+VER]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='maya_composite1.cxx',    obj='maya'+VER+'_composite1.obj')
    EnqueueLib(lib='libmaya'+VER+'.lib', obj=[ 'maya'+VER+'_composite1.obj' ])
    
#
# DIRECTORY: pandatool/src/mayaegg/
#

for VER in MAYAVERSIONS:
  if (OMIT.count("MAYA"+VER)==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/mayaegg', 'pandatool/src/maya']
    OPTS=['MAYA'+VER]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaEggLoader.cxx', obj='mayaegg'+VER+'_loader.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaegg_composite1.cxx',   obj='mayaegg'+VER+'_composite1.obj')
    EnqueueLib(lib='libmayaegg'+VER+'.lib', obj=[ 'mayaegg'+VER+'_composite1.obj' ])

#
# DIRECTORY: pandatool/src/maxegg/
#

for VER in MAXVERSIONS:
  if (OMIT.count("MAX"+VER)==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/maxegg']
    OPTS=['MAX'+VER,  "WINCOMCTL", "WINCOMDLG", "WINUSER", "MSFORSCOPE"]
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
                'libpandaeggstripped.dll',
                'libpandastripped.dll',
                'libpandaexpress.dll',
                'libp3dtoolconfig.dll',
                'libp3dtool.dll',
                'libp3pystub.dll'
               ])

#
# DIRECTORY: pandatool/src/maxprogs/
#

for VER in MAXVERSIONS:
  if (OMIT.count("MAX"+VER)==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/maxprogs']
    OPTS=['MAX'+VER,  "WINCOMCTL", "WINCOMDLG", "WINUSER", "MSFORSCOPE"]
    CopyFile("built/tmp/maxImportRes.obj", "pandatool/src/maxprogs/maxImportRes.obj")
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='maxEggImport.cxx',obj='maxprogs'+VER+'_maxeggimport.obj')
    EnqueueLink(opts=OPTS, dll='maxeggimport'+VER+'.dle', ldef="pandatool/src/maxprogs/maxEggImport.def", obj=[
                'maxegg'+VER+'_loader.obj',
                'maxprogs'+VER+'_maxeggimport.obj',
                'maxImportRes.obj',
                'libpandaeggstripped.dll',
                'libpandastripped.dll',
                'libpandaexpress.dll',
                'libp3dtoolconfig.dll',
                'libp3dtool.dll',
                'libp3pystub.dll'
               ])

#
# DIRECTORY: pandatool/src/vrml/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/vrml']
    OPTS=['ZLIB']
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
    OPTS=[]
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
    OPTS=['ZLIB']
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
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='xfileegg_composite1.cxx', obj='xfileegg_composite1.obj')
    EnqueueLib(lib='libxfileegg.lib', obj=[
                 'xfileegg_composite1.obj',
    ])

#
# DIRECTORY: pandatool/src/ptloader/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/ptloader', 'pandatool/src/flt', 'pandatool/src/lwo', 'pandatool/src/xfile', 'pandatool/src/xfileegg']
    OPTS=['BUILDING_PTLOADER']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='config_ptloader.cxx', obj='ptloader_config_ptloader.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='loaderFileTypePandatool.cxx', obj='ptloader_loaderFileTypePandatool.obj')
    EnqueueLink(dll='libp3ptloader.dll', opts=['ADVAPI'], obj=[
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
    ])

#
# DIRECTORY: pandatool/src/mayaprogs/
#

for VER in MAYAVERSIONS:
  if (OMIT.count('MAYA'+VER)==0) and (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/mayaprogs', 'pandatool/src/maya', 'pandatool/src/mayaegg',
           'pandatool/src/cvscopy']
    OPTS=['BUILDING_MISC', 'MAYA'+VER]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaEggImport.cxx', obj='mayaeggimport'+VER+'_mayaeggimport.obj')
    EnqueueLink(opts=OPTS, dll='mayaeggimport'+VER+'.mll', obj=[
                'mayaegg'+VER+'_loader.obj',
                'mayaeggimport'+VER+'_mayaeggimport.obj',
                'libpandaegg.dll',
                'libpanda.dll',
                'libpandaexpress.dll',
                'libp3dtoolconfig.dll',
                'libp3dtool.dll',
                'libp3pystub.dll'
               ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='config_mayaloader.cxx', obj='mayaloader'+VER+'_config_mayaloader.obj')
    EnqueueLink(dll='libp3mayaloader'+VER+'.dll',                 opts=['ADVAPI',  'MAYA'+VER], obj=[
                 'mayaloader'+VER+'_config_mayaloader.obj',
                 'libmayaegg'+VER+'.lib',
                 'libp3ptloader.lib',
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaPview.cxx', obj='mayapview'+VER+'_mayaPview.obj')
    EnqueueLink(dll='libmayapview'+VER+'.mll', opts=['ADVAPI',  'MAYA'+VER], obj=[
                 'mayapview'+VER+'_mayaPview.obj',
                 'libmayaegg'+VER+'.lib',
                 'libmaya'+VER+'.lib',
                 'libconverter.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libp3framework.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaToEgg.cxx', obj='maya2egg'+VER+'_mayaToEgg.obj')
    EnqueueLink(dll='maya2egg'+VER+'.exe',                 opts=['ADVAPI',  'MAYA'+VER], obj=[
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaCopy.cxx', obj='mayacopy'+VER+'_mayaCopy.obj')
    EnqueueLink(dll='mayacopy'+VER+'.exe',  opts=['ADVAPI',  'MAYA'+VER], obj=[
                 'mayacopy'+VER+'_mayaCopy.obj',
                 'libcvscopy.lib',
                 'libmaya'+VER+'.lib',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libconverter.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='mayaSavePview.cxx', obj='mayasavepview'+VER+'_mayaSavePview.obj')
    EnqueueLink(dll='libmayasavepview'+VER+'.mll', opts=['ADVAPI',  'MAYA'+VER], obj=[
                 'mayasavepview'+VER+'_mayaSavePview.obj',
    ])


#
# DIRECTORY: pandatool/src/miscprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/miscprogs']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='binToC.cxx', obj='bin2c_binToC.obj')
    EnqueueLink(dll='bin2c.exe', opts=['ADVAPI'], obj=[
                 'bin2c_binToC.obj',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandatool/src/pstatserver/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/pstatserver']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pstatserver_composite1.cxx', obj='pstatserver_composite1.obj')
    EnqueueLib(lib='libpstatserver.lib', obj=[ 'pstatserver_composite1.obj' ])

#
# DIRECTORY: pandatool/src/softprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/softprogs']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='softCVS.cxx', obj='softcvs_softCVS.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='softFilename.cxx', obj='softcvs_softFilename.obj')
    EnqueueLink(opts=['ADVAPI'], dll='softcvs.exe', obj=[
                 'softcvs_softCVS.obj',
                 'softcvs_softFilename.obj',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandatool/src/text-stats/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/text-stats']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='textMonitor.cxx', obj='text-stats_textMonitor.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='textStats.cxx', obj='text-stats_textStats.obj')
    EnqueueLink(opts=['ADVAPI'], dll='text-stats.exe', obj=[
                 'text-stats_textMonitor.obj',
                 'text-stats_textStats.obj',
                 'libprogbase.lib',
                 'libpstatserver.lib',
                 'libpandatoolbase.lib',
                 'libpandaegg.dll',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandatool/src/vrmlprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/vrmlprogs', 'pandatool/src/vrml', 'pandatool/src/vrmlegg']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='vrmlTrans.cxx', obj='vrml-trans_vrmlTrans.obj')
    EnqueueLink(opts=['ADVAPI'], dll='vrml-trans.exe', obj=[
                 'vrml-trans_vrmlTrans.obj',
                 'libprogbase.lib',
                 'libpvrml.lib',
                 'libpandatoolbase.lib',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='vrmlToEgg.cxx', obj='vrml2egg_vrmlToEgg.obj')
    EnqueueLink(opts=['ADVAPI'], dll='vrml2egg.exe', obj=[
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandatool/src/win-stats/
#

if (OMIT.count("PANDATOOL")==0) and (sys.platform == "win32"):
    IPATH=['pandatool/src/win-stats']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='winstats_composite1.cxx', obj='pstats_composite1.obj')
    EnqueueLink(opts=['WINSOCK', 'WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM'],
                dll='pstats.exe', obj=[
                'pstats_composite1.obj',
                'libprogbase.lib',
                'libpstatserver.lib',
                'libpandatoolbase.lib',
                'libpandaexpress.dll',
                'libpanda.dll',
                'libp3dtoolconfig.dll',
                'libp3dtool.dll',
                'libp3pystub.dll',
                ])

#
# DIRECTORY: pandatool/src/xfileprogs/
#

if (OMIT.count("PANDATOOL")==0):
    IPATH=['pandatool/src/xfileprogs', 'pandatool/src/xfile', 'pandatool/src/xfileegg']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='eggToX.cxx', obj='egg2x_eggToX.obj')
    EnqueueLink(dll='egg2x.exe', opts=['ADVAPI'], obj=[
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='xFileTrans.cxx', obj='x-trans_xFileTrans.obj')
    EnqueueLink(dll='x-trans.exe', opts=['ADVAPI'], obj=[
                 'x-trans_xFileTrans.obj',
                 'libprogbase.lib',
                 'libxfile.lib',
                 'libpandatoolbase.lib',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='xFileToEgg.cxx', obj='x2egg_xFileToEgg.obj')
    EnqueueLink(opts=['ADVAPI'], dll='x2egg.exe', obj=[
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
                 'libp3dtoolconfig.dll',
                 'libp3dtool.dll',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandaapp/src/pandaappbase/
#

if (OMIT.count("PANDAAPP")==0):
    IPATH=['pandaapp/src/pandaappbase']
    OPTS=[]
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='pandaappbase.cxx', obj='pandaappbase_pandaappbase.obj')
    EnqueueLib(lib='libpandaappbase.lib', obj=['pandaappbase_pandaappbase.obj'])

#
# DIRECTORY: pandaapp/src/httpbackup/
#

if (OMIT.count("OPENSSL")==0) and (OMIT.count("PANDAAPP")==0):
    IPATH=['pandaapp/src/httpbackup', 'pandaapp/src/pandaappbase']
    OPTS=['OPENSSL']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='backupCatalog.cxx', obj='httpbackup_backupCatalog.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='httpBackup.cxx', obj='httpbackup_httpBackup.obj')
    EnqueueLink(opts=['ADVAPI',  'OPENSSL'], dll='httpbackup.exe', obj=[
                 'httpbackup_backupCatalog.obj',
                 'httpbackup_httpBackup.obj',
                 'libpandaappbase.lib',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libp3dtool.dll',
                 'libp3dtoolconfig.dll',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libp3pystub.dll',
    ])

#
# DIRECTORY: pandaapp/src/indexify/
#

if (OMIT.count("FREETYPE")==0) and (OMIT.count("PANDAAPP")==0):
    IPATH=['pandaapp/src/indexify']
    OPTS=[ 'FREETYPE']
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='default_font.cxx', obj='font-samples_default_font.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='fontSamples.cxx', obj='font-samples_fontSamples.obj')
    EnqueueLink(opts=['ADVAPI',  'FREETYPE'], dll='font-samples.exe', obj=[
                 'font-samples_default_font.obj',
                 'font-samples_fontSamples.obj',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtool.dll',
                 'libp3dtoolconfig.dll',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libp3pystub.dll',
    ])
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='default_index_icons.cxx', obj='indexify_default_index_icons.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='default_font.cxx', obj='indexify_default_font.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='indexImage.cxx', obj='indexify_indexImage.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='indexParameters.cxx', obj='indexify_indexParameters.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='indexify.cxx', obj='indexify_indexify.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='photo.cxx', obj='indexify_photo.obj')
    EnqueueCxx(ipath=IPATH, opts=OPTS, src='rollDirectory.cxx', obj='indexify_rollDirectory.obj')
    EnqueueLink(opts=['ADVAPI',  'FREETYPE'], dll='indexify.exe', obj=[
                 'indexify_default_index_icons.obj',
                 'indexify_default_font.obj',
                 'indexify_indexImage.obj',
                 'indexify_indexParameters.obj',
                 'indexify_indexify.obj',
                 'indexify_photo.obj',
                 'indexify_rollDirectory.obj',
                 'libpanda.dll',
                 'libpandaexpress.dll',
                 'libp3dtool.dll',
                 'libp3dtoolconfig.dll',
                 'libprogbase.lib',
                 'libpandatoolbase.lib',
                 'libp3pystub.dll',
    ])



#
# Generate the models directory
#

if (OMIT.count("PANDATOOL")==0):
    EnqueueBam("-pr ../=", "built/models/gui/dialog_box_gui.bam",  "dmodels/src/gui/dialog_box_gui.flt")
    EnqueueBam("-pr ../=", "built/models/misc/camera.bam",         "dmodels/src/misc/camera.flt")
    EnqueueBam("-pr ../=", "built/models/misc/fade.bam",           "dmodels/src/misc/fade.flt")
    EnqueueBam("-pr ../=", "built/models/misc/fade_sphere.bam",    "dmodels/src/misc/fade_sphere.flt")
    EnqueueBam("-pr ../=", "built/models/misc/gridBack.bam",       "dmodels/src/misc/gridBack.flt")
    EnqueueBam("-pr ../=", "built/models/misc/iris.bam",           "dmodels/src/misc/iris.flt")
    EnqueueBam("-pr ../=", "built/models/misc/lilsmiley.bam",      "dmodels/src/misc/lilsmiley.egg")
    EnqueueBam("-pr ../=", "built/models/misc/objectHandles.bam",  "dmodels/src/misc/objectHandles.flt")
    EnqueueBam("-pr ../=", "built/models/misc/rgbCube.bam",        "dmodels/src/misc/rgbCube.flt")
    EnqueueBam("-pr ../=", "built/models/misc/smiley.bam",         "dmodels/src/misc/smiley.egg")
    EnqueueBam("-pr ../=", "built/models/misc/sphere.bam",         "dmodels/src/misc/sphere.flt")
    EnqueueBam("-pr ../=", "built/models/misc/Pointlight.bam",     "dmodels/src/misc/Pointlight.egg")
    EnqueueBam("-pr ../=", "built/models/misc/Dirlight.bam",       "dmodels/src/misc/Dirlight.egg")
    EnqueueBam("-pr ../=", "built/models/misc/Spotlight.bam",      "dmodels/src/misc/Spotlight.egg")
    EnqueueBam("-pr ../=", "built/models/misc/xyzAxis.bam",        "dmodels/src/misc/xyzAxis.flt")
    
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

def BuildWorker(taskqueue, donequeue, slave):
    print "Slave online: "+slave
    while (1):
        task = taskqueue.get()
        sys.stdout.flush()
        if (task == 0): return
        try:
            apply(task[0],task[1])
            donequeue.put(task)
        except:
            donequeue.put(0)

def AllSourcesReady(task, pending):
    sources = task[3]
    for x in sources:
        if (pending.has_key(x)):
            return 0
    altsources = task[4]
    for x in altsources:
        if (pending.has_key(x)):
            return 0
    return 1

def ParallelMake(tasklist):
    # Read the slave-file.
    slaves = []
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
                    if (NeedsBuild(task[2], task[3])):
                        tasksqueued += 1
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
        if (donetask == 0):
            exit("Build process aborting.")
        sys.stdout.flush()
        tasksqueued -= 1
        JustBuilt(donetask[2], donetask[3])
        for target in donetask[2]:
            del pending[target]
    # kill the workers.
    for slave in slaves:
        taskqueue.put(0)
    # make sure there aren't any unsatisfied tasks
    if (len(tasklist)>0):
        exit("Dependency problem - task unsatisfied: "+str(tasklist[0][2]))

def SequentialMake(tasklist):
    for task in tasklist:
        if (NeedsBuild(task[2], task[3])):
            apply(task[0], task[1])
            JustBuilt(task[2], task[3])

def RunDependencyQueue(tasklist):
    if (THREADCOUNT!=0):
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
    allin = os.listdir("built/pandac/input")
    inputs = xpaths("built/pandac/input/",allin,"")
    if (NeedsBuild(['built/pandac/PandaModules.py'],inputs)):
        if (GENMAN): oscmd("built/bin/genpycode -d")
        else       : oscmd("built/bin/genpycode")
        JustBuilt(['built/pandac/PandaModules.py'],inputs)

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
    cmd="thirdparty/win-nsis/makensis /V2 "
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


INSTALLER_DEB_FILE="""
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

INSTALLER_SPEC_FILE="""
Summary: Panda 3D Engine
Name: panda3d
Version: VERSION
Release: 1
License: Panda3D License
Group: Development/Libraries
BuildRoot: linuxroot
%description
The Panda3D engine.
%prep
%setup -q
%build
true
%install
true
%post
/sbin/ldconfig
%postun
/sbin/ldconfig
%clean
true
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


def MakeInstallerLinux():
    import compileall
    PYTHONV=os.path.basename(PYTHONSDK)
    if (os.path.isdir("linuxroot")): oscmd("chmod -R 755 linuxroot")
    oscmd("rm -rf linuxroot data.tar.gz control.tar.gz ")
    oscmd("mkdir -p linuxroot/usr/bin")
    oscmd("mkdir -p linuxroot/usr/include")
    oscmd("mkdir -p linuxroot/usr/share/panda3d")
    oscmd("mkdir -p linuxroot/usr/lib/"+PYTHONV+"/lib-dynload")
    oscmd("mkdir -p linuxroot/usr/lib/"+PYTHONV+"/site-packages")
    oscmd("mkdir -p linuxroot/etc")
    oscmd("sed -e 's@$THIS_PRC_DIR/[.][.]@/usr/share/panda3d@' < built/etc/Config.prc > linuxroot/etc/Config.prc")
    oscmd("cp built/etc/Confauto.prc  linuxroot/etc/Confauto.prc")
    oscmd("cp --recursive built/include linuxroot/usr/include/panda3d")
    oscmd("cp --recursive direct        linuxroot/usr/share/panda3d/direct")
    oscmd("cp --recursive built/pandac  linuxroot/usr/share/panda3d/pandac")
    oscmd("cp --recursive built/Pmw     linuxroot/usr/share/panda3d/Pmw")
    oscmd("cp built/direct/__init__.py  linuxroot/usr/share/panda3d/direct/__init__.py")
    oscmd("cp --recursive SceneEditor   linuxroot/usr/share/panda3d/SceneEditor")
    oscmd("cp --recursive built/models  linuxroot/usr/share/panda3d/models")
    oscmd("cp --recursive samples       linuxroot/usr/share/panda3d/samples")
    oscmd("cp doc/LICENSE               linuxroot/usr/share/panda3d/LICENSE")
    oscmd("cp doc/LICENSE               linuxroot/usr/include/panda3d/LICENSE")
    oscmd("cp doc/ReleaseNotes          linuxroot/usr/share/panda3d/ReleaseNotes")
    oscmd("echo '/usr/share/panda3d' >  linuxroot/usr/lib/"+PYTHONV+"/site-packages/panda3d.pth")
    oscmd("cp built/bin/*               linuxroot/usr/bin/")
    for base in os.listdir("built/lib"):
        oscmd("ln -sf /usr/lib/"+base+" linuxroot/usr/lib/"+PYTHONV+"/lib-dynload/"+base)
        oscmd("cp built/lib/"+base+" linuxroot/usr/lib/"+base)
    for base in os.listdir("linuxroot/usr/share/panda3d/direct/src"):
        if ((base != "extensions") and (base != "extensions_native")):
            compileall.compile_dir("linuxroot/usr/share/panda3d/direct/src/"+base)
    compileall.compile_dir("linuxroot/usr/share/panda3d/Pmw")
    compileall.compile_dir("linuxroot/usr/share/panda3d/SceneEditor")
    oscmd("chmod -R 555 linuxroot/usr/share/panda3d")

    if (os.path.exists("/usr/bin/dpkg-deb")):
        txt = INSTALLER_DEB_FILE[1:].replace("VERSION",str(VERSION)).replace("PYTHONV",PYTHONV)
        oscmd("mkdir -p linuxroot/DEBIAN")
        oscmd("cd linuxroot ; (find usr -type f -exec md5sum {} \;) >  DEBIAN/md5sums")
        oscmd("cd linuxroot ; (find etc -type f -exec md5sum {} \;) >> DEBIAN/md5sums")
        WriteFile("linuxroot/DEBIAN/conffiles","/etc/Config.prc\n")
        WriteFile("linuxroot/DEBIAN/control",txt)
        oscmd("dpkg-deb -b linuxroot panda3d_"+VERSION+"_i386.deb")
        oscmd("chmod -R 755 linuxroot")

    if (os.path.exists("/usr/bin/rpmbuild")):
        txt = INSTALLER_SPEC_FILE[1:].replace("VERSION",str(VERSION)).replace("PYTHONV",PYTHONV)
        WriteFile("panda3d.spec", SPEC)


if (INSTALLER != 0):
    if (sys.platform == "win32"):
        MakeInstallerNSIS("Panda3D-"+VERSION+".exe", "Panda3D", "Panda3D "+VERSION, "C:\\Panda3D-"+VERSION)
    elif (sys.platform == "linux2"):
        MakeInstallerLinux()
    else:
        exit("Do not know how to make an installer for this platform")

##########################################################################################
#
# Print final status report.
#
##########################################################################################

SaveDependencyCache()

WARNINGS.append("Elapsed Time: "+PrettyTime(time.time() - STARTTIME))
#WARNINGS.append("Time(EnqueueCxx): "+PrettyTime(TIMECOMPILEC))
#WARNINGS.append("Time(CompileLib): "+PrettyTime(TIMECOMPILELIB))
#WARNINGS.append("Time(EnqueueLink): "+PrettyTime(TIMECOMPILELINK))
#WARNINGS.append("Time(EnqueueIgate): "+PrettyTime(TIMEINTERROGATE))
#WARNINGS.append("Time(EnqueueImod): "+PrettyTime(TIMEINTERROGATEMODULE))

printStatus("Makepanda Final Status Report", WARNINGS)

