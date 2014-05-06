#!/usr/bin/env python
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
try:
    import sys, os, platform, time, stat, re, getopt, threading, signal, shutil
    if sys.platform == "darwin" or sys.version_info >= (2, 6):
        import plistlib
    if sys.version_info >= (3, 0):
        import queue
    else:
        import Queue as queue
except:
    print("You are either using an incomplete or an old version of Python!")
    print("Please install the development package of Python 2.x and try again.")
    exit(1)

from makepandacore import *
from installpanda import *
import time
import os
import sys

## jGenPyCode tries to get the directory for Direct from the sys.path. This only works if you 
## have installed the sdk using a installer. This would not work if the installer was 
## never used and everything was grabbed into a virgin environment using cvs.
sys.path.append(os.getcwd())

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
INSTALLER=0
GENMAN=0
COMPRESSOR="zlib"
THREADCOUNT=0
CFLAGS=""
LDFLAGS=""
RTDIST=0
RTDIST_VERSION="dev"
RUNTIME=0
DISTRIBUTOR=""
VERSION=None
DEBVERSION=None
RPMRELEASE="1"
P3DSUFFIX=None
MAJOR_VERSION=None
COREAPI_VERSION=None
PLUGIN_VERSION=None
OSXTARGET=None
UNIVERSAL=False
HOST_URL="https://runtime.panda3d.org/"
global STRDXSDKVERSION, STRMSPLATFORMVERSION, BOOUSEINTELCOMPILER
STRDXSDKVERSION = 'default'
STRMSPLATFORMVERSION = 'default'
BOOUSEINTELCOMPILER = False

if "MACOSX_DEPLOYMENT_TARGET" in os.environ:
    OSXTARGET=os.environ["MACOSX_DEPLOYMENT_TARGET"]

PkgListSet(["PYTHON", "DIRECT",                        # Python support
  "GL", "GLES", "GLES2"] + DXVERSIONS + ["TINYDISPLAY", "NVIDIACG", # 3D graphics
  "EGL",                                               # OpenGL (ES) integration
  "EIGEN",                                             # Linear algebra acceleration
  "OPENAL", "FMODEX",                                  # Audio playback
  "VORBIS", "FFMPEG", "SWSCALE", "SWRESAMPLE",         # Audio decoding
  "ODE", "PHYSX", "BULLET", "PANDAPHYSICS",            # Physics
  "SPEEDTREE",                                         # SpeedTree
  "ZLIB", "PNG", "JPEG", "TIFF", "SQUISH", "FREETYPE", # 2D Formats support
  ] + MAYAVERSIONS + MAXVERSIONS + [ "FCOLLADA",       # 3D Formats support
  "VRPN", "OPENSSL",                                   # Transport
  "FFTW",                                              # Algorithm helpers
  "ARTOOLKIT", "OPENCV", "DIRECTCAM", "VISION",        # Augmented Reality
  "NPAPI", "AWESOMIUM",                                # Browser embedding
  "GTK2", "WX", "FLTK",                                # Toolkit support
  "ROCKET",                                            # GUI libraries
  "CARBON", "COCOA",                                   # Mac OS X toolkits
  "OSMESA", "X11", "XF86DGA", "XRANDR", "XCURSOR",     # Unix platform support
  "PANDATOOL", "PVIEW", "DEPLOYTOOLS",                 # Toolchain
  "SKEL",                                              # Example SKEL project
  "PANDAFX",                                           # Some distortion special lenses 
  "PANDAPARTICLESYSTEM",                               # Built in particle system
  "CONTRIB",                                           # Experimental
  "SSE2", "NEON",                                      # Compiler features
  "TOUCHINPUT",                                        # Touchinput interface (requires Windows 7)
])

CheckPandaSourceTree()

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
        print("")
        print("Error parsing commandline input", problem)

    print("")
    print("Makepanda generates a 'built' subdirectory containing a")
    print("compiled copy of Panda3D.  Command-line arguments are:")
    print("")
    print("  --help            (print the help message you're reading now)")
    print("  --verbose         (print out more information)")
    print("  --runtime         (build a runtime build instead of an SDK build)")
    print("  --installer       (build an installer)")
    print("  --optimize X      (optimization level can be 1,2,3,4)")
    print("  --version X       (set the panda version number)")
    print("  --lzma            (use lzma compression when building Windows installer)")
    print("  --distributor X   (short string identifying the distributor of the build)")
    print("  --outputdir X     (use the specified directory instead of 'built')")
    print("  --host URL        (set the host url (runtime build only))")
    print("  --threads N       (use the multithreaded build system. see manual)")
    print("  --osxtarget N     (the OSX version number to build for (OSX only))")
    print("  --universal       (build universal binaries (OSX only))")
    print("  --override \"O=V\"  (override dtool_config/prc option value)")
    print("  --static          (builds libraries for static linking)")
    print("  --target X        (experimental cross-compilation (android only))")
    print("  --arch X          (target architecture for cross-compilation)")
    print("")
    for pkg in PkgListGet():
        p = pkg.lower()
        print("  --use-%-9s   --no-%-9s (enable/disable use of %s)"%(p, p, pkg))
    print("")
    print("  --nothing         (disable every third-party lib)")
    print("  --everything      (enable every third-party lib)")
    print("  --directx-sdk=X   (specify version of DX9 SDK to use: jun2010, aug2009, mar2009, aug2006)")
    print("  --platform-sdk=X  (specify MSPlatSdk to use: win71, win61, win60A, winserver2003r2)")
    print("  --use-icl         (experimental setting to use an intel compiler instead of MSVC on Windows)")
    print("")
    print("The simplest way to compile panda is to just type:")
    print("")
    print("  makepanda --everything")
    print("")
    os._exit(1)

def parseopts(args):
    global INSTALLER,RTDIST,RUNTIME,GENMAN,DISTRIBUTOR,VERSION
    global COMPRESSOR,THREADCOUNT,OSXTARGET,UNIVERSAL,HOST_URL
    global DEBVERSION,RPMRELEASE,P3DSUFFIX
    global STRDXSDKVERSION, STRMSPLATFORMVERSION, BOOUSEINTELCOMPILER
    longopts = [
        "help","distributor=","verbose","runtime","osxtarget=",
        "optimize=","everything","nothing","installer","rtdist","nocolor",
        "version=","lzma","no-python","threads=","outputdir=","override=",
        "static","host=","debversion=","rpmrelease=","p3dsuffix=",
        "directx-sdk=", "platform-sdk=", "use-icl",
        "universal", "target=", "arch="]
    anything = 0
    optimize = ""
    target = None
    target_arch = None
    for pkg in PkgListGet(): longopts.append("no-"+pkg.lower())
    for pkg in PkgListGet(): longopts.append("use-"+pkg.lower())
    try:
        opts, extras = getopt.getopt(args, "", longopts)
        for option,value in opts:
            if (option=="--help"): raise Exception
            elif (option=="--optimize"): optimize=value
            elif (option=="--installer"): INSTALLER=1
            elif (option=="--verbose"): SetVerbose(True)
            elif (option=="--distributor"): DISTRIBUTOR=value
            elif (option=="--rtdist"): RTDIST=1
            elif (option=="--runtime"): RUNTIME=1
            elif (option=="--genman"): GENMAN=1
            elif (option=="--everything"): PkgEnableAll()
            elif (option=="--nothing"): PkgDisableAll()
            elif (option=="--threads"): THREADCOUNT=int(value)
            elif (option=="--outputdir"): SetOutputDir(value.strip())
            elif (option=="--osxtarget"): OSXTARGET=value.strip()
            elif (option=="--universal"): UNIVERSAL=True
            elif (option=="--target"): target = value.strip()
            elif (option=="--arch"): target_arch = value.strip()
            elif (option=="--nocolor"): DisableColors()
            elif (option=="--version"):
                VERSION=value
                if (len(VERSION.split(".")) != 3): raise Exception
            elif (option=="--lzma"): COMPRESSOR="lzma"
            elif (option=="--override"): AddOverride(value.strip())
            elif (option=="--static"): SetLinkAllStatic(True)
            elif (option=="--host"): HOST_URL=value
            elif (option=="--debversion"): DEBVERSION=value
            elif (option=="--rpmrelease"): RPMRELEASE=value
            elif (option=="--p3dsuffix"): P3DSUFFIX=value
            # Backward compatibility, OPENGL was renamed to GL
            elif (option=="--use-opengl"): PkgEnable("GL")
            elif (option=="--no-opengl"): PkgDisable("GL")
            elif (option=="--directx-sdk"):
                STRDXSDKVERSION = value.strip().lower()
                if STRDXSDKVERSION == '':
                    print("No DirectX SDK version specified. Using 'default' DirectX SDK search")
                    STRDXSDKVERSION = 'default'
            elif (option=="--platform-sdk"): 
                STRMSPLATFORMVERSION = value.strip().lower()
                if STRMSPLATFORMVERSION == '':
                    print("No MS Platform SDK version specified. Using 'default' MS Platform SDK search")
                    STRMSPLATFORMVERSION = 'default'
            elif (option=="--use-icl"): BOOUSEINTELCOMPILER = True
            else:
                for pkg in PkgListGet():
                    if (option=="--use-"+pkg.lower()):
                        PkgEnable(pkg)
                        break
                for pkg in PkgListGet():
                    if (option=="--no-"+pkg.lower()):
                        PkgDisable(pkg)
                        break
            if  (option=="--everything" or option.startswith("--use-")
              or option=="--nothing" or option.startswith("--no-")):
              anything = 1
    except: 
        usage(0)
        print("Exception while parsing commandline:", sys.exc_info()[0])
    if (anything==0): usage(0)
    if (RTDIST and RUNTIME):
        usage("Options --runtime and --rtdist cannot be specified at the same time!")
    if (optimize=="" and (RTDIST or RUNTIME)): optimize = "4"
    elif (optimize==""): optimize = "3"
    if (OSXTARGET != None and OSXTARGET.strip() == ""):
        OSXTARGET = None
    elif (OSXTARGET != None):
        OSXTARGET = OSXTARGET.strip()
        if (len(OSXTARGET) != 4 or not OSXTARGET.startswith("10.")):
            usage("Invalid setting for OSXTARGET")
        try:
            OSXTARGET = "10.%d" % (int(OSXTARGET[-1]))
        except:
            usage("Invalid setting for OSXTARGET")
    try:
        SetOptimize(int(optimize))
        assert GetOptimize() in [1, 2, 3, 4]
    except:
        usage("Invalid setting for OPTIMIZE")

    if target is not None or target_arch is not None:
        SetTarget(target, target_arch)

    is_win7 = False
    if GetHost() == "windows":
        if (STRMSPLATFORMVERSION not in ['winserver2003r2', 'win60A']):
            platsdk = GetRegistryKey("SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v7.1", "InstallationFolder")

    if sys.platform == "win32":
        # Note: not available in cygwin.
        winver = sys.getwindowsversion()
        if platsdk and os.path.isdir(platsdk) and winver[0] >= 6 and winver[1] >= 1:
            is_win7 = True
    if not is_win7:
        PkgDisable("TOUCHINPUT")

parseopts(sys.argv[1:])

########################################################################
##
## Handle environment variables.
##
########################################################################

if ("CFLAGS" in os.environ):
    CFLAGS = os.environ["CFLAGS"]
if ("RPM_OPT_FLAGS" in os.environ):
    CFLAGS += " " + os.environ["RPM_OPT_FLAGS"]
CFLAGS = CFLAGS.strip()
if ("LDFLAGS" in os.environ):
    LDFLAGS = os.environ["LDFLAGS"]
LDFLAGS = LDFLAGS.strip()

os.environ["MAKEPANDA"] = os.path.abspath(sys.argv[0])
if (GetHost() == "darwin" and OSXTARGET != None):
    os.environ["MACOSX_DEPLOYMENT_TARGET"] = OSXTARGET

########################################################################
##
## Configure things based on the command-line parameters.
##
########################################################################

if (VERSION is None):
    if (RUNTIME):
        VERSION = ParsePluginVersion("dtool/PandaVersion.pp")
        COREAPI_VERSION = VERSION + "." + ParseCoreapiVersion("dtool/PandaVersion.pp")
        PLUGIN_VERSION = VERSION
    else:
        VERSION = ParsePandaVersion("dtool/PandaVersion.pp")
        PLUGIN_VERSION = ParsePluginVersion("dtool/PandaVersion.pp")

if (COREAPI_VERSION is None):
    COREAPI_VERSION = VERSION

if (DEBVERSION is None):
    DEBVERSION = VERSION

MAJOR_VERSION = VERSION[:3]

if (P3DSUFFIX is None):
    P3DSUFFIX = MAJOR_VERSION

outputdir_suffix = ""

if (RUNTIME or RTDIST):
    # Compiling Maya/Max is pointless in rtdist build
    for ver in MAYAVERSIONS + MAXVERSIONS:
        PkgDisable(ver)

    if (DISTRIBUTOR.strip() == ""):
        exit("You must provide a valid distributor name when making a runtime or rtdist build!")

    outputdir_suffix += "_" + DISTRIBUTOR.strip()
    if (RUNTIME):
        outputdir_suffix += "_rt"

    RTDIST_VERSION = DISTRIBUTOR.strip() + "_" + MAJOR_VERSION
elif (DISTRIBUTOR == ""):
    DISTRIBUTOR = "makepanda"

if not IsCustomOutputDir():
    if GetTarget() == "windows" and GetTargetArch() == 'x64':
        outputdir_suffix += '_x64'

    SetOutputDir("built" + outputdir_suffix)

if (RUNTIME):
    for pkg in PkgListGet():
        if pkg in ["GTK2"]:
            # Optional package(s) for runtime.
            pass
        elif pkg in ["OPENSSL", "ZLIB", "NPAPI", "JPEG", "PNG"]:
            # Required packages for runtime.
            if (PkgSkip(pkg)==1):
                exit("Runtime must be compiled with OpenSSL, ZLib, NPAPI, JPEG and PNG support!")
        else:
            # Unused packages for runtime.
            PkgDisable(pkg)

if (GetHost() == 'windows'):
    os.environ["BISON_SIMPLE"] = GetThirdpartyBase()+"/win-util/bison.simple"

if (INSTALLER and RTDIST):
    exit("Cannot build an installer for the rtdist build!")

if (INSTALLER) and (PkgSkip("PYTHON")) and (not RUNTIME):
    exit("Cannot build installer without python")

if (RTDIST) and (PkgSkip("JPEG")):
    exit("Cannot build rtdist without jpeg")

if (RTDIST) and (PkgSkip("WX") and PkgSkip("FLTK")):
    exit("Cannot build rtdist without wx or fltk")

if (RUNTIME):
    SetLinkAllStatic(True)

if not os.path.isdir("contrib"):
    PkgDisable("CONTRIB")

########################################################################
##
## Load the dependency cache.
##
########################################################################

LoadDependencyCache()

########################################################################
##
## Locate various SDKs.
##
########################################################################

MakeBuildTree()

SdkLocateDirectX(STRDXSDKVERSION)
SdkLocateMaya()
SdkLocateMax()
SdkLocateMacOSX(OSXTARGET)
SdkLocatePython(RTDIST)
SdkLocateVisualStudio()
SdkLocateMSPlatform(STRMSPLATFORMVERSION)
SdkLocatePhysX()
SdkLocateSpeedTree()
SdkLocateAndroid()

SdkAutoDisableDirectX()
SdkAutoDisableMaya()
SdkAutoDisableMax()
SdkAutoDisablePhysX()
SdkAutoDisableSpeedTree()

if (RTDIST and DISTRIBUTOR == "cmu"):
    if (RTDIST_VERSION == "cmu_1.7" and SDK["PYTHONVERSION"] != "python2.6"):
        exit("The CMU 1.7 runtime distribution must be built against Python 2.6!")
    elif (RTDIST_VERSION == "cmu_1.8" and SDK["PYTHONVERSION"] != "python2.7"):
        exit("The CMU 1.8 runtime distribution must be built against Python 2.7!")

########################################################################
##
## Choose a Compiler.
##
## This should also set up any environment variables needed to make
## the compiler work.
##
########################################################################

if GetHost() == 'windows' and GetTarget() == 'windows':
    COMPILER = "MSVC"
else:
    COMPILER = "GCC"

SetupBuildEnvironment(COMPILER)

########################################################################
##
## External includes, external libraries, and external defsyms.
##
########################################################################

IncDirectory("ALWAYS", GetOutputDir()+"/tmp")
IncDirectory("ALWAYS", GetOutputDir()+"/include")

if (COMPILER == "MSVC"):
    PkgDisable("X11")
    PkgDisable("XRANDR")
    PkgDisable("XF86DGA")
    PkgDisable("XCURSOR")
    PkgDisable("GLES")
    PkgDisable("GLES2")
    PkgDisable("EGL")
    PkgDisable("CARBON")
    PkgDisable("COCOA")
    if (PkgSkip("PYTHON")==0):
        IncDirectory("ALWAYS", SDK["PYTHON"] + "/include")
        LibDirectory("ALWAYS", SDK["PYTHON"] + "/libs")
    SmartPkgEnable("EIGEN",     "eigen3",     (), ("Eigen/Dense",), target_pkg = 'ALWAYS')
    for pkg in PkgListGet():
        if (PkgSkip(pkg)==0):
            if (pkg[:4]=="MAYA"):
                IncDirectory(pkg, SDK[pkg]      + "/include")
                DefSymbol(pkg, "MAYAVERSION", pkg)
                DefSymbol(pkg, "MLIBRARY_DONTUSE_MFC_MANIFEST", "")
            elif (pkg[:3]=="MAX"):
                IncDirectory(pkg, SDK[pkg]      + "/include")
                IncDirectory(pkg, SDK[pkg]      + "/include/CS")
                IncDirectory(pkg, SDK[pkg+"CS"] + "/include")
                IncDirectory(pkg, SDK[pkg+"CS"] + "/include/CS")
                DefSymbol(pkg, "MAX", pkg)
                if (int(pkg[3:]) >= 2013):
                    DefSymbol(pkg, "UNICODE", "")
                    DefSymbol(pkg, "_UNICODE", "")
            elif (pkg[:2]=="DX"):
                IncDirectory(pkg, SDK[pkg]      + "/include")
            elif GetThirdpartyDir() is not None:
                IncDirectory(pkg, GetThirdpartyDir() + pkg.lower() + "/include")
    for pkg in DXVERSIONS:
        if (PkgSkip(pkg)==0):
            vnum=pkg[2:]

            if GetTargetArch() == 'x64':
              LibDirectory(pkg, SDK[pkg] + '/lib/x64')
            else:
              LibDirectory(pkg, SDK[pkg] + '/lib/x86')
              LibDirectory(pkg, SDK[pkg] + '/lib')

            LibName(pkg, 'd3dVNUM.lib'.replace("VNUM", vnum))
            LibName(pkg, 'd3dxVNUM.lib'.replace("VNUM", vnum))
            if (vnum=="9" and "GENERIC_DXERR_LIBRARY" in SDK):
                LibName(pkg, 'dxerr.lib')
            else:
                LibName(pkg, 'dxerrVNUM.lib'.replace("VNUM", vnum))
            LibName(pkg, 'ddraw.lib')
            LibName(pkg, 'dxguid.lib')
    IncDirectory("ALWAYS", GetThirdpartyDir() + "extras/include")
    LibName("WINSOCK", "wsock32.lib")
    LibName("WINSOCK2", "wsock32.lib")
    LibName("WINSOCK2", "ws2_32.lib")
    LibName("WINCOMCTL", "comctl32.lib")
    LibName("WINCOMDLG", "comdlg32.lib")
    LibName("WINUSER", "user32.lib")
    LibName("WINMM", "winmm.lib")
    LibName("WINIMM", "imm32.lib")
    LibName("WINKERNEL", "kernel32.lib")
    LibName("WINOLE", "ole32.lib")
    LibName("WINOLEAUT", "oleaut32.lib")
    LibName("WINOLDNAMES", "oldnames.lib")
    LibName("WINSHELL", "shell32.lib")
    LibName("WINGDI", "gdi32.lib")
    LibName("ADVAPI", "advapi32.lib")
    LibName("IPHLPAPI", "iphlpapi.lib")
    LibName("GL", "opengl32.lib")
    LibName("GLES", "libgles_cm.lib")
    LibName("GLES2", "libGLESv2.lib")
    LibName("EGL", "libEGL.lib")
    LibName("MSIMG", "msimg32.lib")
    if (PkgSkip("DIRECTCAM")==0): LibName("DIRECTCAM", "strmiids.lib")
    if (PkgSkip("DIRECTCAM")==0): LibName("DIRECTCAM", "quartz.lib")
    if (PkgSkip("DIRECTCAM")==0): LibName("DIRECTCAM", "odbc32.lib")
    if (PkgSkip("DIRECTCAM")==0): LibName("DIRECTCAM", "odbccp32.lib")
    if (PkgSkip("PNG")==0):      LibName("PNG",      GetThirdpartyDir() + "png/lib/libpng_static.lib")
    if (PkgSkip("JPEG")==0):     LibName("JPEG",     GetThirdpartyDir() + "jpeg/lib/jpeg-static.lib")
    if (PkgSkip("TIFF")==0):     LibName("TIFF",     GetThirdpartyDir() + "tiff/lib/libtiff.lib")
    if (PkgSkip("ZLIB")==0):     LibName("ZLIB",     GetThirdpartyDir() + "zlib/lib/zlibstatic.lib")
    if (PkgSkip("VRPN")==0):     LibName("VRPN",     GetThirdpartyDir() + "vrpn/lib/vrpn.lib")
    if (PkgSkip("VRPN")==0):     LibName("VRPN",     GetThirdpartyDir() + "vrpn/lib/quat.lib")
    if (PkgSkip("NVIDIACG")==0): LibName("CGGL",     GetThirdpartyDir() + "nvidiacg/lib/cgGL.lib")
    if (PkgSkip("NVIDIACG")==0): LibName("CGDX9",    GetThirdpartyDir() + "nvidiacg/lib/cgD3D9.lib")
    if (PkgSkip("NVIDIACG")==0): LibName("NVIDIACG", GetThirdpartyDir() + "nvidiacg/lib/cg.lib")
    if (PkgSkip("OPENSSL")==0):  LibName("OPENSSL",  GetThirdpartyDir() + "openssl/lib/libpandassl.lib")
    if (PkgSkip("OPENSSL")==0):  LibName("OPENSSL",  GetThirdpartyDir() + "openssl/lib/libpandaeay.lib")
    if (PkgSkip("FREETYPE")==0): LibName("FREETYPE", GetThirdpartyDir() + "freetype/lib/freetype.lib")
    if (PkgSkip("FFTW")==0):     LibName("FFTW",     GetThirdpartyDir() + "fftw/lib/rfftw.lib")
    if (PkgSkip("FFTW")==0):     LibName("FFTW",     GetThirdpartyDir() + "fftw/lib/fftw.lib")
    if (PkgSkip("ARTOOLKIT")==0):LibName("ARTOOLKIT",GetThirdpartyDir() + "artoolkit/lib/libAR.lib")
    if (PkgSkip("FCOLLADA")==0): LibName("FCOLLADA", GetThirdpartyDir() + "fcollada/lib/FCollada.lib")
    if (PkgSkip("SQUISH")==0):   LibName("SQUISH",   GetThirdpartyDir() + "squish/lib/squish.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/cv.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/highgui.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/cvaux.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/ml.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/cxcore.lib")
    if (PkgSkip("AWESOMIUM")==0):LibName("AWESOMIUM",GetThirdpartyDir() + "awesomium/lib/Awesomium.lib")
    if (PkgSkip("FFMPEG")==0):   LibName("FFMPEG",   GetThirdpartyDir() + "ffmpeg/lib/avcodec.lib")
    if (PkgSkip("FFMPEG")==0):   LibName("FFMPEG",   GetThirdpartyDir() + "ffmpeg/lib/avformat.lib")
    if (PkgSkip("FFMPEG")==0):   LibName("FFMPEG",   GetThirdpartyDir() + "ffmpeg/lib/avutil.lib")
    if (PkgSkip("SWSCALE")==0):  LibName("SWSCALE",  GetThirdpartyDir() + "ffmpeg/lib/swscale.lib")
    if (PkgSkip("SWRESAMPLE")==0):LibName("SWRESAMPLE",GetThirdpartyDir() + "ffmpeg/lib/swresample.lib")
    if (PkgSkip("ROCKET")==0):
        LibName("ROCKET", GetThirdpartyDir() + "rocket/lib/RocketCore.lib")
        LibName("ROCKET", GetThirdpartyDir() + "rocket/lib/RocketControls.lib")
        if (PkgSkip("PYTHON")==0):
            LibName("ROCKET", GetThirdpartyDir() + "rocket/lib/" + SDK["PYTHONVERSION"] + "/boost_python-vc100-mt-1_54.lib")
        if (GetOptimize() <= 3):
            LibName("ROCKET", GetThirdpartyDir() + "rocket/lib/RocketDebugger.lib")
    if (PkgSkip("OPENAL")==0):   LibName("OPENAL",   GetThirdpartyDir() + "openal/lib/OpenAL32.lib")
    if (PkgSkip("ODE")==0):
        LibName("ODE",      GetThirdpartyDir() + "ode/lib/ode_single.lib")
        DefSymbol("ODE",    "dSINGLE", "")
    if (PkgSkip("FMODEX")==0):
        if (GetTargetArch() == 'x64'):
            LibName("FMODEX",   GetThirdpartyDir() + "fmodex/lib/fmodex64_vc.lib")
        else:
            LibName("FMODEX",   GetThirdpartyDir() + "fmodex/lib/fmodex_vc.lib")
    if (PkgSkip("WX")==0):
        LibName("WX",       GetThirdpartyDir() + "wx/lib/wxbase28u.lib")
        LibName("WX",       GetThirdpartyDir() + "wx/lib/wxmsw28u_core.lib")
        DefSymbol("WX",     "__WXMSW__", "")
        DefSymbol("WX",     "_UNICODE", "")
        DefSymbol("WX",     "UNICODE", "")
    if (PkgSkip("FLTK")==0):
        LibName("FLTK",     GetThirdpartyDir() + "fltk/lib/fltk.lib")
    if (PkgSkip("VORBIS")==0):
        LibName("VORBIS",   GetThirdpartyDir() + "vorbis/lib/libogg_static.lib")
        LibName("VORBIS",   GetThirdpartyDir() + "vorbis/lib/libvorbis_static.lib")
        LibName("VORBIS",   GetThirdpartyDir() + "vorbis/lib/libvorbisfile_static.lib")
    for pkg in MAYAVERSIONS:
        if (PkgSkip(pkg)==0):
            LibName(pkg, '"' + SDK[pkg] + '/lib/Foundation.lib"')
            LibName(pkg, '"' + SDK[pkg] + '/lib/OpenMaya.lib"')
            LibName(pkg, '"' + SDK[pkg] + '/lib/OpenMayaAnim.lib"')
            LibName(pkg, '"' + SDK[pkg] + '/lib/OpenMayaUI.lib"')
    for pkg in MAXVERSIONS:
        if (PkgSkip(pkg)==0):
            LibName(pkg, SDK[pkg] +  '/lib/core.lib')
            LibName(pkg, SDK[pkg] +  '/lib/edmodel.lib')
            LibName(pkg, SDK[pkg] +  '/lib/gfx.lib')
            LibName(pkg, SDK[pkg] +  '/lib/geom.lib')
            LibName(pkg, SDK[pkg] +  '/lib/mesh.lib')
            LibName(pkg, SDK[pkg] +  '/lib/maxutil.lib')
            LibName(pkg, SDK[pkg] +  '/lib/paramblk2.lib')
    if (PkgSkip("PHYSX")==0):
        if GetTargetArch() == 'x64':
            LibName("PHYSX",  SDK["PHYSXLIBS"] + "/PhysXLoader64.lib")
            LibName("PHYSX",  SDK["PHYSXLIBS"] + "/NxCharacter64.lib")
        else:
            LibName("PHYSX",  SDK["PHYSXLIBS"] + "/PhysXLoader.lib")
            LibName("PHYSX",  SDK["PHYSXLIBS"] + "/NxCharacter.lib")

        IncDirectory("PHYSX", SDK["PHYSX"] + "/Physics/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/PhysXLoader/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/NxCharacter/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/NxExtensions/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/Foundation/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/Cooking/include")

    if (PkgSkip("SPEEDTREE")==0):
        if GetTargetArch() == 'x64':
            libdir = SDK["SPEEDTREE"] + "/Lib/Windows/VC10.x64/"
            p64ext = '64'
        else:
            libdir = SDK["SPEEDTREE"] + "/Lib/Windows/VC10/"
            p64ext = ''

        debugext = ''
        if (GetOptimize() <= 2): debugext = "_d"
        libsuffix = "_v%s_VC100MT%s_Static%s.lib" % (
            SDK["SPEEDTREEVERSION"], p64ext, debugext)
        LibName("SPEEDTREE", "%sSpeedTreeCore%s" % (libdir, libsuffix))
        LibName("SPEEDTREE", "%sSpeedTreeForest%s" % (libdir, libsuffix))
        LibName("SPEEDTREE", "%sSpeedTree%sRenderer%s" % (libdir, SDK["SPEEDTREEAPI"], libsuffix))
        LibName("SPEEDTREE", "%sSpeedTreeRenderInterface%s" % (libdir, libsuffix))
        if (SDK["SPEEDTREEAPI"] == "OpenGL"):
            LibName("SPEEDTREE",  "%sglew32.lib" % (libdir))
            LibName("SPEEDTREE",  "glu32.lib")
        IncDirectory("SPEEDTREE", SDK["SPEEDTREE"] + "/Include")
    if (PkgSkip("BULLET")==0):
        suffix = '.lib'
        if GetTargetArch() == 'x64':
            suffix = '_x64.lib'
        LibName("BULLET", GetThirdpartyDir() + "bullet/lib/LinearMath" + suffix)
        LibName("BULLET", GetThirdpartyDir() + "bullet/lib/BulletCollision" + suffix)
        LibName("BULLET", GetThirdpartyDir() + "bullet/lib/BulletDynamics" + suffix)
        LibName("BULLET", GetThirdpartyDir() + "bullet/lib/BulletSoftBody" + suffix)

if (COMPILER=="GCC"):
    PkgDisable("AWESOMIUM")
    if (GetTarget() != "darwin"):
        PkgDisable("CARBON")
        PkgDisable("COCOA")
    elif (RTDIST or RUNTIME):
        # We don't support Cocoa in the runtime yet.
        PkgDisable("COCOA")
    elif (not UNIVERSAL and GetTargetArch() == 'x86_64'):
        # 64-bits OS X doesn't have Carbon.
        PkgDisable("CARBON")

    if (PkgSkip("PYTHON")==0):
        IncDirectory("ALWAYS", SDK["PYTHON"])
    if (GetHost() == "darwin"):
        if (PkgSkip("FREETYPE")==0 and not os.path.isdir(GetThirdpartyDir() + 'freetype')):
          IncDirectory("FREETYPE", "/usr/X11/include")
          IncDirectory("FREETYPE", "/usr/X11/include/freetype2")
          LibDirectory("FREETYPE", "/usr/X11/lib")

    if (os.path.isdir("/usr/PCBSD")):
        IncDirectory("ALWAYS", "/usr/PCBSD/local/include")
        LibDirectory("ALWAYS", "/usr/PCBSD/local/lib")

    if (GetHost() == "freebsd"):
        IncDirectory("ALWAYS", "/usr/local/include")
        LibDirectory("ALWAYS", "/usr/local/lib")

    if GetHost() != "darwin":
        # Workaround for an issue where pkg-config does not include this path
        if GetTargetArch() in ("x86_64", "amd64"):
            if (os.path.isdir("/usr/lib64/glib-2.0/include")):
                IncDirectory("GTK2", "/usr/lib64/glib-2.0/include")
            if (os.path.isdir("/usr/lib64/gtk-2.0/include")):
                IncDirectory("GTK2", "/usr/lib64/gtk-2.0/include")

            if (os.path.isdir("/usr/X11R6/lib64")):
                LibDirectory("ALWAYS", "/usr/X11R6/lib64")
            else:
                LibDirectory("ALWAYS", "/usr/X11R6/lib")
        else:
            LibDirectory("ALWAYS", "/usr/X11R6/lib")

    fcollada_libs = ("FColladaD", "FColladaSD", "FColladaS")
    # WARNING! The order of the ffmpeg libraries matters!
    ffmpeg_libs = ("libavformat", "libavcodec", "libavutil")

    #         Name         pkg-config   libs, include(dir)s
    if (not RUNTIME):
        SmartPkgEnable("EIGEN",     "eigen3",    (), ("Eigen/Dense",), target_pkg = 'ALWAYS')
        SmartPkgEnable("ARTOOLKIT", "",          ("AR"), "AR/ar.h")
        SmartPkgEnable("FCOLLADA",  "",          ChooseLib(fcollada_libs, "FCOLLADA"), ("FCollada", "FCollada.h"))
        SmartPkgEnable("FFMPEG",    ffmpeg_libs, ffmpeg_libs, ffmpeg_libs)
        SmartPkgEnable("SWSCALE",   "libswscale", "libswscale", ("libswscale", "libswscale/swscale.h"), target_pkg = "FFMPEG")
        SmartPkgEnable("SWRESAMPLE","libswresample", "libswresample", ("libswresample", "libswresample/swresample.h"), target_pkg = "FFMPEG")
        SmartPkgEnable("FFTW",      "",          ("fftw", "rfftw"), ("fftw.h", "rfftw.h"))
        SmartPkgEnable("FMODEX",    "",          ("fmodex"), ("fmodex", "fmodex/fmod.h"))
        SmartPkgEnable("FREETYPE",  "freetype2", ("freetype"), ("freetype2", "freetype2/freetype/freetype.h"))
        SmartPkgEnable("GL",        "gl",        ("GL"), ("GL/gl.h"), framework = "OpenGL")
        SmartPkgEnable("GLES",      "glesv1_cm", ("GLESv1_CM"), ("GLES/gl.h"), framework = "OpenGLES")
        SmartPkgEnable("GLES2",     "glesv2",    ("GLESv2"), ("GLES2/gl2.h")) #framework = "OpenGLES"?
        SmartPkgEnable("EGL",       "egl",       ("EGL"), ("EGL/egl.h"))
        SmartPkgEnable("OSMESA",    "osmesa",    ("OSMesa"), ("GL/osmesa.h"))
        SmartPkgEnable("NVIDIACG",  "",          ("Cg"), "Cg/cg.h", framework = "Cg")
        SmartPkgEnable("ODE",       "",          ("ode"), "ode/ode.h", tool = "ode-config")
        SmartPkgEnable("OPENAL",    "openal",    ("openal"), "AL/al.h", framework = "OpenAL")
        SmartPkgEnable("OPENCV",    "opencv",    ("cv", "highgui", "cvaux", "ml", "cxcore"), ("opencv", "opencv/cv.h"))
        SmartPkgEnable("SQUISH",    "",          ("squish"), "squish.h")
        SmartPkgEnable("TIFF",      "",          ("tiff"), "tiff.h")
        SmartPkgEnable("VRPN",      "",          ("vrpn", "quat"), ("vrpn", "quat.h", "vrpn/vrpn_Types.h"))
        SmartPkgEnable("BULLET", "bullet", ("BulletSoftBody", "BulletDynamics", "BulletCollision", "LinearMath"), ("bullet", "bullet/btBulletDynamicsCommon.h"))
        SmartPkgEnable("VORBIS",    "vorbisfile",("vorbisfile", "vorbis", "ogg"), ("ogg/ogg.h", "vorbis/vorbisfile.h"))

        rocket_libs = ("RocketCore", "RocketControls")
        if (GetOptimize() <= 3):
            rocket_libs += ("RocketDebugger",)
        if (GetHost() != "darwin"):
            # We use a statically linked libboost_python on OSX
            rocket_libs += ("boost_python",)
        SmartPkgEnable("ROCKET",    "",          rocket_libs, "Rocket/Core.h")

    SmartPkgEnable("JPEG",      "",          ("jpeg"), "jpeglib.h")
    SmartPkgEnable("OPENSSL",   "openssl",   ("ssl", "crypto"), ("openssl/ssl.h", "openssl/crypto.h"))
    SmartPkgEnable("PNG",       "libpng",    ("png"), "png.h", tool = "libpng-config")
    SmartPkgEnable("ZLIB",      "zlib",      ("z"), "zlib.h")
    SmartPkgEnable("GTK2",      "gtk+-2.0")

    if (RTDIST and GetHost() == "darwin" and "PYTHONVERSION" in SDK):
        # Don't use the framework for the OSX rtdist build. I'm afraid it gives problems somewhere.
        SmartPkgEnable("PYTHON",    "", SDK["PYTHONVERSION"], (SDK["PYTHONVERSION"], SDK["PYTHONVERSION"] + "/Python.h"), tool = SDK["PYTHONVERSION"] + "-config")
    elif("PYTHONVERSION" in SDK and not RUNTIME):
        SmartPkgEnable("PYTHON",    "", SDK["PYTHONVERSION"], (SDK["PYTHONVERSION"], SDK["PYTHONVERSION"] + "/Python.h"), tool = SDK["PYTHONVERSION"] + "-config", framework = "Python")

    if (RTDIST):
        SmartPkgEnable("WX",    tool = "wx-config")
        SmartPkgEnable("FLTK", "", ("fltk"), ("Fl/Fl.H"), tool = "fltk-config")

    if (RUNTIME):
        if (GetHost() == 'darwin'):
            SmartPkgEnable("NPAPI", "", (), ("npapi.h"))
            if not os.path.isdir(GetThirdpartyDir() + "npapi"):
                IncDirectory("NPAPI", "/System/Library/Frameworks/WebKit.framework/Headers")

        elif (GetHost() == "freebsd"):
            SmartPkgEnable("NPAPI", "mozilla-plugin", (), ("libxul/stable", "libxul/stable/npapi.h", "nspr/prtypes.h", "nspr"))
        else:
            SmartPkgEnable("NPAPI", "mozilla-plugin", (), ("xulrunner-*/stable", "xulrunner-*/stable/npapi.h", "nspr*/prtypes.h", "nspr*"))

    if GetTarget() != 'darwin':
        # CgGL is covered by the Cg framework, and we don't need X11 components on OSX
        if (PkgSkip("NVIDIACG")==0 and not RUNTIME):
            SmartPkgEnable("CGGL",  "",      ("CgGL"), "Cg/cgGL.h")
        if (not RUNTIME):
            SmartPkgEnable("X11",   "x11", "X11", ("X11", "X11/Xlib.h"))
            SmartPkgEnable("XRANDR", "xrandr", "Xrandr", "X11/extensions/Xrandr.h")
            SmartPkgEnable("XF86DGA", "xxf86dga", "Xxf86dga", "X11/extensions/xf86dga.h")
            SmartPkgEnable("XCURSOR", "xcursor", "Xcursor", "X11/Xcursor/Xcursor.h")

    if (RUNTIME):
        # For the runtime, all packages are required
        for pkg in ["OPENSSL", "ZLIB", "NPAPI", "JPEG", "PNG"]:
            skips = []
            if (pkg in PkgListGet() and PkgSkip(pkg)==1):
                skips.append(pkg)
            if skips:
                exit("Runtime must be compiled with OpenSSL, ZLib, NPAPI, JPEG and PNG support (missing %s)" % (', '.join(skips)))

    for pkg in MAYAVERSIONS:
        if (PkgSkip(pkg)==0 and (pkg in SDK)):
            if (GetHost() == "darwin"):
                # Sheesh, Autodesk really can't make up their mind
                # regarding the location of the Maya devkit on OS X.
                if (os.path.isdir(SDK[pkg] + "/Maya.app/Contents/lib")):
                    LibDirectory(pkg, SDK[pkg] + "/Maya.app/Contents/lib")
                if (os.path.isdir(SDK[pkg] + "/Maya.app/Contents/MacOS")):
                    LibDirectory(pkg, SDK[pkg] + "/Maya.app/Contents/MacOS")
                if (os.path.isdir(SDK[pkg] + "/lib")):
                    LibDirectory(pkg, SDK[pkg] + "/lib")
                if (os.path.isdir(SDK[pkg] + "/Maya.app/Contents/include/maya")):
                    IncDirectory(pkg, SDK[pkg] + "/Maya.app/Contents/include")
                if (os.path.isdir(SDK[pkg] + "/devkit/include/maya")):
                    IncDirectory(pkg, SDK[pkg] + "/devkit/include")
                if (os.path.isdir(SDK[pkg] + "/include/maya")):
                    IncDirectory(pkg, SDK[pkg] + "/include")
            else:
                LibDirectory(pkg, SDK[pkg] + "/lib")
                IncDirectory(pkg, SDK[pkg] + "/include")
            DefSymbol(pkg, "MAYAVERSION", pkg)

    if GetTarget() == 'darwin':
        LibName("ALWAYS", "-framework AppKit")
        if (PkgSkip("OPENCV")==0):
            LibName("OPENCV", "-framework QuickTime")
        LibName("AGL", "-framework AGL")
        LibName("CARBON", "-framework Carbon")
        LibName("COCOA", "-framework Cocoa")
        # Fix for a bug in OSX Leopard:
        LibName("GL", "-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")

    if GetTarget() == 'android':
        LibName("ALWAYS", '-llog')
        LibName("ALWAYS", '-landroid')
        LibName("JNIGRAPHICS", '-ljnigraphics')

    for pkg in MAYAVERSIONS:
        if (PkgSkip(pkg)==0 and (pkg in SDK)):
            if GetTarget() == 'darwin':
                LibName(pkg, "-Wl,-rpath," + SDK[pkg] + "/Maya.app/Contents/MacOS")
            else:
                LibName(pkg, "-Wl,-rpath," + SDK[pkg] + "/lib")
            LibName(pkg, "-lOpenMaya")
            LibName(pkg, "-lOpenMayaAnim")
            LibName(pkg, "-lAnimSlice")
            LibName(pkg, "-lDeformSlice")
            LibName(pkg, "-lModifiers")
            LibName(pkg, "-lDynSlice")
            LibName(pkg, "-lKinSlice")
            LibName(pkg, "-lModelSlice")
            LibName(pkg, "-lNurbsSlice")
            LibName(pkg, "-lPolySlice")
            LibName(pkg, "-lProjectSlice")
            LibName(pkg, "-lImage")
            LibName(pkg, "-lShared")
            LibName(pkg, "-lTranslators")
            LibName(pkg, "-lDataModel")
            LibName(pkg, "-lRenderModel")
            LibName(pkg, "-lNurbsEngine")
            LibName(pkg, "-lDependEngine")
            LibName(pkg, "-lCommandEngine")
            LibName(pkg, "-lFoundation")
            LibName(pkg, "-lIMFbase")
            if GetTarget() != 'darwin':
                LibName(pkg, "-lOpenMayalib")
            else:
                LibName(pkg, "-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")

    if (PkgSkip("PHYSX")==0):
        IncDirectory("PHYSX", SDK["PHYSX"] + "/Physics/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/PhysXLoader/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/NxCharacter/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/NxExtensions/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/Foundation/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/Cooking/include")
        LibDirectory("PHYSX", SDK["PHYSXLIBS"])
        if (GetHost() == "darwin"):
            LibName("PHYSX", SDK["PHYSXLIBS"] + "/osxstatic/PhysXCooking.a")
            LibName("PHYSX", SDK["PHYSXLIBS"] + "/osxstatic/PhysXCore.a")
        else:
            LibName("PHYSX", "-lPhysXLoader")
            LibName("PHYSX", "-lNxCharacter")

DefSymbol("WITHINPANDA", "WITHIN_PANDA", "1")
if GetLinkAllStatic():
    DefSymbol("ALWAYS", "LINK_ALL_STATIC", "")
if GetTarget() == 'android':
    DefSymbol("ALWAYS", "ANDROID", "")

########################################################################
##
## Give a Status Report on Command-Line Options
##
########################################################################

def printStatus(header,warnings):
    if GetVerbose():
        print("")
        print("-------------------------------------------------------------------")
        print(header)
        tkeep = ""
        tomit = ""
        for x in PkgListGet():
            if (PkgSkip(x)==0): tkeep = tkeep + x + " "
            else:                  tomit = tomit + x + " "
        if RTDIST:  print("Makepanda: Runtime distribution build")
        elif RUNTIME: print("Makepanda: Runtime build")
        else:        print("Makepanda: Regular build")
        print("Makepanda: Compiler:",COMPILER)
        print("Makepanda: Optimize:",GetOptimize())
        print("Makepanda: Keep Pkg:",tkeep)
        print("Makepanda: Omit Pkg:",tomit)
        if (GENMAN): print("Makepanda: Generate API reference manual")
        else       : print("Makepanda: Don't generate API reference manual")
        if (GetHost() == "windows" and not RTDIST):
            if INSTALLER:  print("Makepanda: Build installer, using",COMPRESSOR)
            else        :  print("Makepanda: Don't build installer")
        print("Makepanda: Version ID: "+VERSION)
        for x in warnings: print("Makepanda: "+x)
        print("-------------------------------------------------------------------")
        print("")
        sys.stdout.flush()

########################################################################
##
## BracketNameWithQuotes
##
########################################################################

def BracketNameWithQuotes(name):
    # Workaround for OSX bug - compiler doesn't like those flags quoted.
    if (name.startswith("-framework")): return name
    if (name.startswith("-dylib_file")): return name

    # Don't add quotes when it's not necessary.
    if " " not in name: return name

    # Account for quoted name (leave as is) but quote everything else (e.g., to protect spaces within paths from improper parsing)
    if (name.startswith('"') and name.endswith('"')): return name
    else: return '"' + name + '"'

########################################################################
##
## CompileCxx
##
########################################################################

def CompileCxx(obj,src,opts):
    ipath = GetListOption(opts, "DIR:")
    optlevel = GetOptimizeOption(opts)
    if (COMPILER=="MSVC"):
        if not BOOUSEINTELCOMPILER:
            cmd = "cl "
            if GetTargetArch() == 'x64':
                cmd += "/favor:blend "
            cmd += "/wd4996 /wd4275 /wd4267 /wd4101 /wd4273 "

            # Enable Windows 7 interfaces if we need Touchinput.
            if PkgSkip("TOUCHINPUT") == 0:
                cmd += "/DWINVER=0x601 "
            cmd += "/Fo" + obj + " /nologo /c"
            if (GetTargetArch() != 'x64' and PkgSkip("SSE2") == 0):
                cmd += " /arch:SSE2"            
            for x in ipath: cmd += " /I" + x
            for (opt,dir) in INCDIRECTORIES:
                if (opt=="ALWAYS") or (opt in opts): cmd += " /I" + BracketNameWithQuotes(dir)
            for (opt,var,val) in DEFSYMBOLS:
                if (opt=="ALWAYS") or (opt in opts): cmd += " /D" + var + "=" + val
            if (opts.count('NOFLOATWARN')): cmd += ' /wd4244 /wd4305'
            if (opts.count('MSFORSCOPE')): cmd += ' /Zc:forScope-'

            if (optlevel==1): cmd += " /MDd /Zi /RTCs /GS"
            if (optlevel==2): cmd += " /MDd /Zi"
            if (optlevel==3): cmd += " /MD /Zi /O2 /Ob2 /Oi /Ot /fp:fast /DFORCE_INLINING"
            if (optlevel==4): 
               cmd += " /MD /Zi /Ox /Ob2 /Oi /Ot /fp:fast /DFORCE_INLINING /DNDEBUG /GL"
               cmd += " /Oy /Zp16"      # jean-claude add /Zp16 insures correct static alignment for SSEx

            cmd += " /Fd" + os.path.splitext(obj)[0] + ".pdb"

            building = GetValueOption(opts, "BUILDING:")
            if (building):
                cmd += " /DBUILDING_" + building

            if ("BIGOBJ" in opts) or GetTargetArch() == 'x64':
                cmd += " /bigobj"

            cmd += " /EHa /Zm300 /DWIN32_VC /DWIN32"
            if GetTargetArch() == 'x64':
                cmd += " /DWIN64_VC /DWIN64"

            cmd += " /W3 " + BracketNameWithQuotes(src)
            oscmd(cmd)
        else:
            cmd = "icl "
            if GetTargetArch() == 'x64':
                cmd += "/favor:blend "
            cmd += "/wd4996 /wd4275 /wd4267 /wd4101 /wd4273 "

            # Enable Windows 7 interfaces if we need Touchinput.
            if PkgSkip("TOUCHINPUT") == 0:
                cmd += "/DWINVER=0x601 "
            cmd += "/Fo" + obj + " /c"
            for x in ipath: cmd += " /I" + x
            for (opt,dir) in INCDIRECTORIES:
                if (opt=="ALWAYS") or (opt in opts): cmd += " /I" + BracketNameWithQuotes(dir)
            for (opt,var,val) in DEFSYMBOLS:
                if (opt=="ALWAYS") or (opt in opts): cmd += " /D" + var + "=" + val
            if (opts.count('NOFLOATWARN')): cmd += ' /wd4244 /wd4305'
            if (opts.count('MSFORSCOPE')):  cmd += ' /Zc:forScope-'

            if (optlevel==1): cmd += " /MDd /Zi /RTCs /GS"
            if (optlevel==2): cmd += " /MDd /Zi /arch:SSE3"
            # core changes from jean-claude (dec 2011)
            # ----------------------------------------
            # performance will be seeked at level 3 & 4
            # -----------------------------------------
            if (optlevel==3):
                cmd += " /MD /Zi /O2 /Oi /Ot /arch:SSE3"
                cmd += " /Ob0"
                cmd += " /Qipo-"                            # beware of IPO !!!  
            ##      Lesson learned: Don't use /GL flag -> end result is MESSY
            ## ----------------------------------------------------------------
            if (optlevel==4):
                cmd += " /MD /Zi /O3 /Oi /Ot /Ob0 /Yc /DNDEBUG"  # /Ob0 a ete rajoute en cours de route a 47%                
                cmd += " /Qipo"                              # optimization multi file

            # for 3 & 4 optimization levels
            # -----------------------------
            if (optlevel>=3):
                cmd += " /fp:fast=2"
                cmd += " /Qftz"
                cmd += " /Qfp-speculation:fast"
                cmd += " /Qopt-matmul"                        # needs /O2 or /O3
                cmd += " /Qprec-div-"
                cmd += " /Qsimd"
                
                cmd += " /QxHost"                            # compile for target host; Compiling for distribs should probably strictly enforce /arch:..
                cmd += " /Quse-intel-optimized-headers"        # use intel optimized headers
                cmd += " /Qparallel"                        # enable parallelization
                cmd += " /Qvc10"                                # for Microsoft Visual C++ 2010

            ## PCH files coexistence: the /Qpchi option causes the Intel C++ Compiler to name its
            ## PCH files with a .pchi filename suffix and reduce build time.
            ## The /Qpchi option is on by default but interferes with Microsoft libs; so use /Qpchi- to turn it off. 
            ## I need to have a deeper look at this since the compile time is quite influenced by this setting !!!
            cmd += " /Qpchi-"                                 # keep it this way!

            ## Inlining seems to be an issue here ! (the linker doesn't find necessary info later on)
            ## ------------------------------------
            ## so don't use cmd += " /DFORCE_INLINING"        (need to check why with Panda developpers!)
            ## Inline expansion  /Ob1    :    Allow functions marked inline to be inlined.
            ## Inline any        /Ob2    :    Inline functions deemed appropriate by compiler.

            ## Ctor displacement /vd0    :    Disable constructor displacement.
            ## Choose this option only if no class constructors or destructors call virtual functions.
            ## Use /vd1 (default) to enable. Alternate: #pragma vtordisp

            ## Best case ptrs    /vmb    :    Use best case "pointer to class member" representation.
            ## Use this option if you always define a class before you declare a pointer to a member of the class.
            ## The compiler will issue an error if it encounters a pointer declaration before the class is defined.
            ## Alternate: #pragma pointers_to_members
      
            cmd += " /Fd" + os.path.splitext(obj)[0] + ".pdb"
            building = GetValueOption(opts, "BUILDING:")
            if (building): cmd += " /DBUILDING_" + building
            if ("BIGOBJ" in opts) or GetTargetArch() == 'x64':
                cmd += " /bigobj"

            # level of warnings and optimization reports
            if GetVerbose(): 
                cmd += " /W3 " # or /W4 or /Wall
                cmd += " /Qopt-report:2 /Qopt-report-phase:hlo /Qopt-report-phase:hpo"    # some optimization reports
            else:            
                cmd += " /W1 "
            cmd += " /EHa /Zm300 /DWIN32_VC /DWIN32"
            if GetTargetArch() == 'x64':
                cmd += " /DWIN64_VC /DWIN64"
            cmd += " " + BracketNameWithQuotes(src)

            oscmd(cmd)
            
    if (COMPILER=="GCC"):
        if (src.endswith(".c")): cmd = GetCC() +' -fPIC -c -o ' + obj
        else:                    cmd = GetCXX()+' -ftemplate-depth-30 -fPIC -c -o ' + obj
        for (opt, dir) in INCDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts): cmd += ' -I' + BracketNameWithQuotes(dir)
        for (opt,var,val) in DEFSYMBOLS:
            if (opt=="ALWAYS") or (opt in opts): cmd += ' -D' + var + '=' + val
        for x in ipath: cmd += ' -I' + x

        # Mac-specific flags.
        if GetTarget() == "darwin":
            cmd += " -Wno-deprecated-declarations"
            if (OSXTARGET != None):
                cmd += " -isysroot " + SDK["MACOSX"]
                cmd += " -mmacosx-version-min=" + OSXTARGET
            if UNIVERSAL:
                cmd += " -arch i386"
                if ("NOPPC" not in opts):
                    cmd += " -arch ppc"
            elif HasTargetArch():
                cmd += " -arch %s" % (GetTargetArch())

        if "SYSROOT" in SDK:
            cmd += ' --sysroot=%s -no-canonical-prefixes' % (SDK["SYSROOT"])

        # Android-specific flags.
        if GetTarget() == "android":
            cmd += ' -ffunction-sections -funwind-tables'
            arch = GetTargetArch()
            if arch == 'armv7a':
                cmd += ' -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__'
                cmd += ' -fstack-protector -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16'
            elif arch == 'arm':
                cmd += ' -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__'
                cmd += ' -fstack-protector -march=armv5te -mtune=xscale -msoft-float'
            elif arch == 'mips':
                cmd += ' -fno-strict-aliasing -finline-functions -fmessage-length=0'
                cmd += ' -fno-inline-functions-called-once -fgcse-after-reload'
                cmd += ' -frerun-cse-after-loop -frename-registers'

            if not src.endswith(".c"):
                cmd += " -fno-exceptions -fno-rtti"
            cmd += " -Wa,--noexecstack"

            # Now add specific release/debug flags.
            if optlevel >= 3:
                cmd += " -fomit-frame-pointer"
                if arch.startswith('arm'):
                    cmd += ' -fno-strict-aliasing -finline-limit=64 -mthumb'
                elif arch == 'x86':
                    cmd += ' -fstrict-aliasing'
                elif arch == 'mips':
                    cmd += ' -funswitch-loops -finline-limit=300'
            else:
                cmd += ' -fno-omit-frame-pointer'
                if arch.startswith('arm'):
                    cmd += ' -marm'
                elif arch == 'x86':
                    cmd += ' -fno-strict-aliasing'

            # Enable SIMD instructions if requested
            if arch.startswith('arm') and PkgSkip("NEON") == 0:
                cmd += ' -mfpu=neon'

        else:
            cmd += " -pthread"

        if PkgSkip("SSE2") == 0:
            cmd += " -msse2"
        
        if (optlevel==1): cmd += " -ggdb -D_DEBUG"
        if (optlevel==2): cmd += " -O1 -D_DEBUG"
        if (optlevel==3): cmd += " -O2"
        if (optlevel==4): cmd += " -O3 -DNDEBUG"
        if (CFLAGS !=""): cmd += " " + CFLAGS
        building = GetValueOption(opts, "BUILDING:")
        if (building): cmd += " -DBUILDING_" + building
        cmd += ' ' + BracketNameWithQuotes(src)
        oscmd(cmd)

########################################################################
##
## CompileBison
##
########################################################################

def CompileBison(wobj, wsrc, opts):
    ifile = os.path.basename(wsrc)
    wdsth = GetOutputDir()+"/include/" + ifile[:-4] + ".h"
    wdstc = GetOutputDir()+"/tmp/" + ifile + ".cxx"
    pre = GetValueOption(opts, "BISONPREFIX_")
    bison = GetBison()
    if bison is None:
        # We don't have bison.  See if there is a prebuilt file.
        base, ext = os.path.splitext(wsrc)
        if os.path.isfile(base + '.h.prebuilt') and \
           os.path.isfile(base + '.cxx.prebuilt'):
            CopyFile(wdstc, base + '.cxx.prebuilt')
            CopyFile(wdsth, base + '.h.prebuilt')
        else:
            exit('Could not find bison!')
    else:
        oscmd(bison + ' -y -d -o'+GetOutputDir()+'/tmp/'+ifile+'.c -p '+pre+' '+wsrc)
        CopyFile(wdstc, GetOutputDir()+"/tmp/"+ifile+".c")
        CopyFile(wdsth, GetOutputDir()+"/tmp/"+ifile+".h")

    # Finally, compile the generated source file.
    CompileCxx(wobj,wdstc,opts)

########################################################################
##
## CompileFlex
##
########################################################################

def CompileFlex(wobj,wsrc,opts):
    ifile = os.path.basename(wsrc)
    wdst = GetOutputDir()+"/tmp/"+ifile+".cxx"
    pre = GetValueOption(opts, "BISONPREFIX_")
    dashi = opts.count("FLEXDASHI")
    flex = GetFlex()
    if flex is None:
        # We don't have flex.  See if there is a prebuilt file.
        base, ext = os.path.splitext(wsrc)
        if os.path.isfile(base + '.cxx.prebuilt'):
            CopyFile(wdst, base + '.cxx.prebuilt')
        else:
            exit('Could not find flex!')
    else:
        if (dashi):
            oscmd(flex + " -i -P" + pre + " -o"+wdst+" "+wsrc)
        else:
            oscmd(flex +    " -P" + pre + " -o"+wdst+" "+wsrc)

    # Finally, compile the generated source file.
    CompileCxx(wobj,wdst,opts)

########################################################################
##
## CompileIgate
##
########################################################################

def CompileIgate(woutd,wsrc,opts):
    outbase = os.path.basename(woutd)[:-3]
    woutc = GetOutputDir()+"/tmp/"+outbase+"_igate.cxx"
    wobj = FindLocation(outbase + "_igate.obj", [])
    srcdir = GetValueOption(opts, "SRCDIR:")
    module = GetValueOption(opts, "IMOD:")
    library = GetValueOption(opts, "ILIB:")
    ipath = GetListOption(opts, "DIR:")
    if (PkgSkip("PYTHON")):
        WriteFile(woutc, "")
        WriteFile(woutd, "")
        CompileCxx(wobj, woutc, opts)
        ConditionalWriteFile(woutd, "")
        return

    if not CrossCompiling():
        # If we're compiling for this platform, we can use the one we've built.
        cmd = os.path.join(GetOutputDir(), 'bin', 'interrogate')
    else:
        # Assume that interrogate is on the PATH somewhere.
        cmd = 'interrogate'

    cmd += ' -srcdir %s -I%s -Dvolatile -Dmutable' % (srcdir, srcdir)
    if (COMPILER=="MSVC"):
        cmd += ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -D__inline -longlong __int64 -D_X86_ -DWIN32_VC -DWIN32 -D_WIN32'
        if GetTargetArch() == 'x64':
            cmd += ' -DWIN64_VC -DWIN64 -D_WIN64'
        # NOTE: this 1600 value is the version number for VC2010.
        cmd += ' -D_MSC_VER=1600 -D"_declspec(param)=" -D_near -D_far -D__near -D__far -D__stdcall'
    if (COMPILER=="GCC"):
        cmd += ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -D__inline -D__const=const'
        if GetTargetArch() in ("x86_64", "amd64"):
            cmd += ' -D_LP64'
        else:
            cmd += ' -D__i386__'

    optlevel = GetOptimizeOption(opts)
    if (optlevel==1): cmd += ' -D_DEBUG'
    if (optlevel==2): cmd += ' -D_DEBUG'
    if (optlevel==3): pass
    if (optlevel==4): cmd += ' -DNDEBUG'
    cmd += ' -oc ' + woutc + ' -od ' + woutd
    cmd += ' -fnames -string -refcount -assert -python-native'
    cmd += ' -S' + GetOutputDir() + '/include/parser-inc'

    # Add -I, -S and -D flags
    for x in ipath:
        cmd += ' -I' + BracketNameWithQuotes(x)
    for (opt,dir) in INCDIRECTORIES:
        if (opt=="ALWAYS") or (opt in opts):
            cmd += ' -S' + BracketNameWithQuotes(dir)
    for (opt,var,val) in DEFSYMBOLS:
        if (opt=="ALWAYS") or (opt in opts):
            cmd += ' -D' + var + '=' + val

    building = GetValueOption(opts, "BUILDING:")
    if (building): cmd += " -DBUILDING_"+building
    cmd += ' -module ' + module + ' -library ' + library
    for x in wsrc:
        if (x.startswith("/")):
            cmd += ' ' + BracketNameWithQuotes(x)
        else:
            cmd += ' ' + BracketNameWithQuotes(os.path.basename(x))
    oscmd(cmd)
    CompileCxx(wobj,woutc,opts)
    return

########################################################################
##
## CompileImod
##
########################################################################

def CompileImod(wobj, wsrc, opts):
    module = GetValueOption(opts, "IMOD:")
    library = GetValueOption(opts, "ILIB:")
    if (COMPILER=="MSVC"):
        woutc = wobj[:-4]+".cxx"
    if (COMPILER=="GCC"):
        woutc = wobj[:-2]+".cxx"
    if (PkgSkip("PYTHON")):
        WriteFile(woutc, "")
        CompileCxx(wobj, woutc, opts)
        return

    if not CrossCompiling():
        # If we're compiling for this platform, we can use the one we've built.
        cmd = os.path.join(GetOutputDir(), 'bin', 'interrogate_module')
    else:
        # Assume that interrogate_module is on the PATH somewhere.
        cmd = 'interrogate_module'

    cmd += ' -oc ' + woutc + ' -module ' + module + ' -library ' + library + ' -python-native '
    for x in wsrc: cmd += ' ' + BracketNameWithQuotes(x)
    oscmd(cmd)
    CompileCxx(wobj,woutc,opts)
    return

########################################################################
##
## CompileLib
##
########################################################################

def CompileLib(lib, obj, opts):
    if (COMPILER=="MSVC"):
        if not BOOUSEINTELCOMPILER:
            #Use MSVC Linker
            cmd = 'link /lib /nologo'
            if HasTargetArch():
                cmd += " /MACHINE:" + GetTargetArch().upper()
            cmd += ' /OUT:' + BracketNameWithQuotes(lib)
            for x in obj: cmd += ' ' + BracketNameWithQuotes(x)
            oscmd(cmd)
        else:
            # Choose Intel linker; from Jean-Claude
            cmd = 'xilink /verbose:lib /lib '
            if HasTargetArch():
                cmd += " /MACHINE:" + GetTargetArch().upper()
            cmd += ' /OUT:' + BracketNameWithQuotes(lib)
            for x in obj: cmd += ' ' + BracketNameWithQuotes(x)
            cmd += ' /LIBPATH:"C:\Program Files (x86)\Intel\Composer XE 2011 SP1\ipp\lib\ia32"'
            cmd += ' /LIBPATH:"C:\Program Files (x86)\Intel\Composer XE 2011 SP1\TBB\Lib\ia32\vc10"'
            cmd += ' /LIBPATH:"C:\Program Files (x86)\Intel\Composer XE 2011 SP1\compiler\lib\ia32"'
            oscmd(cmd)

    if (COMPILER=="GCC"):
        if GetTarget() == 'darwin':
            cmd = 'libtool -static -o ' + BracketNameWithQuotes(lib)
        else:
            cmd = GetAR() + ' cru ' + BracketNameWithQuotes(lib)
        for x in obj:
            cmd += ' ' + BracketNameWithQuotes(x)
        oscmd(cmd)

        oscmd(GetRanlib() + ' ' + BracketNameWithQuotes(lib))

########################################################################
##
## CompileLink
##
########################################################################

def CompileLink(dll, obj, opts):
    if (COMPILER=="MSVC"):
        if not BOOUSEINTELCOMPILER:
            cmd = "link /nologo "
            if HasTargetArch():
                cmd += " /MACHINE:" + GetTargetArch().upper()
            if ("MFC" not in opts):
                cmd += " /NOD:MFC90.LIB /NOD:MFC80.LIB /NOD:LIBCMT"
            cmd += " /NOD:LIBCI.LIB /DEBUG"
            cmd += " /nod:libc /nod:libcmtd /nod:atlthunk /nod:atls"
            if (GetOrigExt(dll) != ".exe"): cmd += " /DLL"
            optlevel = GetOptimizeOption(opts)
            if (optlevel==1): cmd += " /MAP /MAPINFO:EXPORTS /NOD:MSVCRT.LIB /NOD:MSVCPRT.LIB /NOD:MSVCIRT.LIB"
            if (optlevel==2): cmd += " /MAP:NUL /NOD:MSVCRT.LIB /NOD:MSVCPRT.LIB /NOD:MSVCIRT.LIB"
            if (optlevel==3): cmd += " /MAP:NUL /NOD:MSVCRTD.LIB /NOD:MSVCPRTD.LIB /NOD:MSVCIRTD.LIB"
            if (optlevel==4): cmd += " /MAP:NUL /LTCG /NOD:MSVCRTD.LIB /NOD:MSVCPRTD.LIB /NOD:MSVCIRTD.LIB"
            if ("MFC" in opts):
                if (optlevel<=2): cmd += " /NOD:MSVCRTD.LIB mfcs100d.lib MSVCRTD.lib"
                else: cmd += " /NOD:MSVCRT.LIB mfcs100.lib MSVCRT.lib"
            cmd += " /FIXED:NO /OPT:REF /STACK:4194304 /INCREMENTAL:NO "
            cmd += ' /OUT:' + BracketNameWithQuotes(dll)

            subsystem = GetValueOption(opts, "SUBSYSTEM:")
            if subsystem:
                cmd += " /SUBSYSTEM:" + subsystem

            if dll.endswith(".dll"):
                cmd += ' /IMPLIB:' + GetOutputDir() + '/lib/' + os.path.splitext(os.path.basename(dll))[0] + ".lib"

            for (opt, dir) in LIBDIRECTORIES:
                if (opt=="ALWAYS") or (opt in opts):
                    cmd += ' /LIBPATH:' + BracketNameWithQuotes(dir)

            for x in obj:
                if x.endswith(".dll"):
                    cmd += ' ' + GetOutputDir() + '/lib/' + os.path.splitext(os.path.basename(x))[0] + ".lib"
                elif x.endswith(".pyd"):
                    cmd += ' ' + os.path.splitext(x)[0] + ".lib"
                elif x.endswith(".lib"):
                    dname = os.path.splitext(os.path.basename(x))[0] + ".dll"
                    if (GetOrigExt(x) != ".ilb" and os.path.exists(GetOutputDir()+"/bin/" + dname)):
                        exit("Error: in makepanda, specify "+dname+", not "+x)
                    cmd += ' ' + BracketNameWithQuotes(x)
                elif x.endswith(".def"):
                    cmd += ' /DEF:' + BracketNameWithQuotes(x)
                elif x.endswith(".dat"):
                    pass
                else:
                    cmd += ' ' + BracketNameWithQuotes(x)

            if (GetOrigExt(dll)==".exe" and "NOICON" not in opts):
                cmd += " " + GetOutputDir() + "/tmp/pandaIcon.res"

            for (opt, name) in LIBNAMES:
                if (opt=="ALWAYS") or (opt in opts):
                    cmd += " " + BracketNameWithQuotes(name)

            oscmd(cmd)
        else:
            cmd = "xilink"
            if GetVerbose(): cmd += " /verbose:lib"            
            if HasTargetArch():
                cmd += " /MACHINE:" + GetTargetArch().upper()
            if ("MFC" not in opts):
                cmd += " /NOD:MFC90.LIB /NOD:MFC80.LIB /NOD:LIBCMT"
            cmd += " /NOD:LIBCI.LIB /DEBUG"
            cmd += " /nod:libc /nod:libcmtd /nod:atlthunk /nod:atls"
            cmd += ' /LIBPATH:"C:\Program Files (x86)\Intel\Composer XE 2011 SP1\ipp\lib\ia32"'
            cmd += ' /LIBPATH:"C:\Program Files (x86)\Intel\Composer XE 2011 SP1\TBB\Lib\ia32\vc10"'
            cmd += ' /LIBPATH:"C:\Program Files (x86)\Intel\Composer XE 2011 SP1\compiler\lib\ia32"'
            if (GetOrigExt(dll) != ".exe"): cmd += " /DLL"
            optlevel = GetOptimizeOption(opts)
            if (optlevel==1): cmd += " /MAP /MAPINFO:EXPORTS /NOD:MSVCRT.LIB /NOD:MSVCPRT.LIB /NOD:MSVCIRT.LIB"
            if (optlevel==2): cmd += " /MAP:NUL /NOD:MSVCRT.LIB /NOD:MSVCPRT.LIB /NOD:MSVCIRT.LIB"
            if (optlevel==3): cmd += " /MAP:NUL /NOD:MSVCRTD.LIB /NOD:MSVCPRTD.LIB /NOD:MSVCIRTD.LIB"
            if (optlevel==4): cmd += " /MAP:NUL /LTCG /NOD:MSVCRTD.LIB /NOD:MSVCPRTD.LIB /NOD:MSVCIRTD.LIB"
            if ("MFC" in opts):
                if (optlevel<=2): cmd += " /NOD:MSVCRTD.LIB mfcs100d.lib MSVCRTD.lib"
                else: cmd += " /NOD:MSVCRT.LIB mfcs100.lib MSVCRT.lib"
            cmd += " /FIXED:NO /OPT:REF /STACK:4194304 /INCREMENTAL:NO "
            cmd += ' /OUT:' + BracketNameWithQuotes(dll)

            subsystem = GetValueOption(opts, "SUBSYSTEM:")
            if subsystem:
                cmd += " /SUBSYSTEM:" + subsystem

            if dll.endswith(".dll"):
                cmd += ' /IMPLIB:' + GetOutputDir() + '/lib/' + os.path.splitext(os.path.basename(dll))[0] + ".lib"

            for (opt, dir) in LIBDIRECTORIES:
                if (opt=="ALWAYS") or (opt in opts):
                    cmd += ' /LIBPATH:' + BracketNameWithQuotes(dir)

            for x in obj:
                if x.endswith(".dll"):
                    cmd += ' ' + GetOutputDir() + '/lib/' + os.path.splitext(os.path.basename(x))[0] + ".lib"
                elif x.endswith(".pyd"):
                    cmd += ' ' + os.path.splitext(x)[0] + ".lib"
                elif x.endswith(".lib"):
                    dname = os.path.splitext(dll)[0]+".dll"
                    if (GetOrigExt(x) != ".ilb" and os.path.exists(GetOutputDir()+"/bin/" + os.path.splitext(os.path.basename(x))[0] + ".dll")):
                        exit("Error: in makepanda, specify "+dname+", not "+x)
                    cmd += ' ' + BracketNameWithQuotes(x)
                elif x.endswith(".def"):
                    cmd += ' /DEF:' + BracketNameWithQuotes(x)
                elif x.endswith(".dat"):
                    pass
                else:
                    cmd += ' ' + BracketNameWithQuotes(x)

            if (GetOrigExt(dll)==".exe" and "NOICON" not in opts):
                cmd += " " + GetOutputDir() + "/tmp/pandaIcon.res"

            for (opt, name) in LIBNAMES:
                if (opt=="ALWAYS") or (opt in opts):
                    cmd += " " + BracketNameWithQuotes(name)

            oscmd(cmd)

    if COMPILER == "GCC":
        cxx = GetCXX()
        if GetOrigExt(dll) == ".exe" and GetTarget() != 'android':
            cmd = cxx + ' -o ' + dll + ' -L' + GetOutputDir() + '/lib -L' + GetOutputDir() + '/tmp'
        else:
            if (GetTarget() == "darwin"):
                cmd = cxx + ' -undefined dynamic_lookup'
                if ("BUNDLE" in opts): cmd += ' -bundle '
                else:
                    if GetOrigExt(dll) == ".pyd":
                        install_name = '@loader_path/../panda3d/' + os.path.basename(dll)
                    else:
                        install_name = os.path.basename(dll)
                    cmd += ' -dynamiclib -install_name ' + install_name
                    cmd += ' -compatibility_version ' + MAJOR_VERSION + ' -current_version ' + VERSION
                cmd += ' -o ' + dll + ' -L' + GetOutputDir() + '/lib -L' + GetOutputDir() + '/tmp'
            else:
                cmd = cxx + ' -shared'
                if ("MODULE" not in opts): cmd += " -Wl,-soname=" + os.path.basename(dll)
                cmd += ' -o ' + dll + ' -L' + GetOutputDir() + '/lib -L' + GetOutputDir() + '/tmp'

        for x in obj:
            if GetOrigExt(x) != ".dat":
                cmd += ' ' + x

        if (GetOrigExt(dll) == ".exe" and GetTarget() == 'windows' and "NOICON" not in opts):
            cmd += " " + GetOutputDir() + "/tmp/pandaIcon.res"

        # Mac OS X specific flags.
        if GetTarget() == 'darwin':
            cmd += " -headerpad_max_install_names"
            if OSXTARGET != None:
                cmd += " -isysroot " + SDK["MACOSX"] + " -Wl,-syslibroot," + SDK["MACOSX"]
                cmd += " -mmacosx-version-min=" + OSXTARGET
            if UNIVERSAL:
                cmd += " -arch i386"
                if ("NOPPC" not in opts):
                    cmd += " -arch ppc"
            elif HasTargetArch():
                cmd += " -arch %s" % (GetTargetArch())

        if "SYSROOT" in SDK:
            cmd += " --sysroot=%s -no-canonical-prefixes" % (SDK["SYSROOT"])

        # Android-specific flags.
        if GetTarget() == 'android':
            cmd += " -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now"
            if GetTargetArch() == 'armv7a':
                cmd += " -march=armv7-a -Wl,--fix-cortex-a8"
            cmd += ' -lc -lm'
        else:
            cmd += " -pthread"

        if LDFLAGS != "":
            cmd += " " + LDFLAGS

        for (opt, dir) in LIBDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts):
                cmd += ' -L' + BracketNameWithQuotes(dir)
        for (opt, name) in LIBNAMES:
            if (opt=="ALWAYS") or (opt in opts):
                cmd += ' ' + BracketNameWithQuotes(name)

        if GetTarget() != 'freebsd':
            cmd += " -ldl"

        oscmd(cmd)

        if GetTarget() == 'android':
            # Copy the library to built/libs/$ANDROID_ABI and strip it.
            # This is the format that Android NDK projects should use.
            new_path = '%s/libs/%s/%s' % (GetOutputDir(), SDK["ANDROID_ABI"], os.path.basename(dll))
            CopyFile(new_path, dll)
            oscmd('%s --strip-unneeded %s' % (GetStrip(), BracketNameWithQuotes(new_path)))

        elif (GetOrigExt(dll)==".exe" and GetOptimizeOption(opts)==4 and "NOSTRIP" not in opts):
            oscmd(GetStrip() + " -x " + BracketNameWithQuotes(dll))

        os.system("chmod +x " + BracketNameWithQuotes(dll))

        if dll.endswith("." + MAJOR_VERSION + ".dylib"):
            newdll = dll[:-6-len(MAJOR_VERSION)] + "dylib"
            if os.path.isfile(newdll):
                os.remove(newdll)
            oscmd("ln -s " + BracketNameWithQuotes(os.path.basename(dll)) + " " + BracketNameWithQuotes(newdll))

        elif dll.endswith("." + MAJOR_VERSION):
            newdll = dll[:-len(MAJOR_VERSION)-1]
            if os.path.isfile(newdll):
                os.remove(newdll)
            oscmd("ln -s " + BracketNameWithQuotes(os.path.basename(dll)) + " " + BracketNameWithQuotes(newdll))

##########################################################################################
#
# CompileEgg
#
##########################################################################################

def CompileEgg(eggfile, src, opts):
    pz = False
    if eggfile.endswith(".pz"):
        pz = True
        eggfile = eggfile[:-3]

    # Determine the location of the pzip and flt2egg tools.
    if CrossCompiling():
        # We may not be able to use our generated versions of these tools,
        # so we'll expect them to already be present in the PATH.
        pzip = 'pzip'
        flt2egg = 'flt2egg'
    else:
        # If we're compiling for this machine, we can use the binaries we've built.
        pzip = os.path.join(GetOutputDir(), 'bin', 'pzip')
        flt2egg = os.path.join(GetOutputDir(), 'bin', 'flt2egg')
        if not os.path.isfile(pzip):
            pzip = 'pzip'
        if not os.path.isfile(flt2egg):
            flt2egg = 'flt2egg'

    if src.endswith(".egg"):
        CopyFile(eggfile, src)
    elif src.endswith(".flt"):
        oscmd(flt2egg + ' -ps keep -o ' + BracketNameWithQuotes(eggfile) + ' ' + BracketNameWithQuotes(src))

    if pz:
        oscmd(pzip + ' ' + BracketNameWithQuotes(eggfile))

##########################################################################################
#
# CompileRes, CompileRsrc
#
##########################################################################################

def CompileRes(target, src, opts):
    """Compiles a Windows .rc file into a .res file."""
    ipath = GetListOption(opts, "DIR:")
    if (COMPILER == "MSVC"):
        cmd = "rc"
        cmd += " /Fo" + BracketNameWithQuotes(target)
        for x in ipath: cmd += " /I" + x
        for (opt,dir) in INCDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts):
                cmd += " /I" + BracketNameWithQuotes(dir)
        for (opt,var,val) in DEFSYMBOLS:
            if (opt=="ALWAYS") or (opt in opts):
                cmd += " /D" + var + "=" + val
        cmd += " " + BracketNameWithQuotes(src)
    else:
        cmd = "windres"
        for x in ipath: cmd += " -I" + x
        for (opt,dir) in INCDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts):
                cmd += " -I" + BracketNameWithQuotes(dir)
        for (opt,var,val) in DEFSYMBOLS:
            if (opt=="ALWAYS") or (opt in opts):
                cmd += " -D" + var + "=" + val
        cmd += " -i " + BracketNameWithQuotes(src)
        cmd += " -o " + BracketNameWithQuotes(target)

    oscmd(cmd)

def CompileRsrc(target, src, opts):
    """Compiles a Mac OS .r file into an .rsrc file."""
    ipath = GetListOption(opts, "DIR:")
    cmd = "/Developer/Tools/Rez -useDF"
    cmd += " -o " + BracketNameWithQuotes(target)
    for x in ipath:
        cmd += " -i " + x
    for (opt,dir) in INCDIRECTORIES:
        if (opt=="ALWAYS") or (opt in opts):
            cmd += " -i " + BracketNameWithQuotes(dir)
    for (opt,var,val) in DEFSYMBOLS:
        if (opt=="ALWAYS") or (opt in opts):
            if (val == ""):
                cmd += " -d " + var
            else:
                cmd += " -d " + var + " = " + val

    cmd += " " + BracketNameWithQuotes(src)
    oscmd(cmd)

##########################################################################################
#
# RunGenPyCode
#
##########################################################################################

def RunGenPyCode(target, inputs, opts):
    if (PkgSkip("PYTHON") != 0):
        return

    cmdstr = sys.executable + " "
    if sys.version_info >= (2, 6):
        cmdstr += "-B "

    cmdstr += os.path.join(GetOutputDir(), "direct", "ffi", "jGenPyCode.py")
    if (GENMAN): cmdstr += " -d"
    cmdstr += " -r"
    for i in inputs:
        if (GetOrigExt(i)==".pyd"):
            cmdstr += " panda3d." + os.path.basename(os.path.splitext(i)[0])
        elif (GetOrigExt(i)==".dll"):
            cmdstr += " " + os.path.basename(os.path.splitext(i)[0].replace("_d",""))

    oscmd(cmdstr)

##########################################################################################
#
# FreezePy
#
##########################################################################################

def FreezePy(target, inputs, opts):
    assert len(inputs) > 0
    # Make sure this function isn't called before genpycode is run.
    cmdstr = sys.executable + " "
    if sys.version_info >= (2, 6):
        cmdstr += "-B "

    cmdstr += os.path.join("direct", "src", "showutil", "pfreeze.py")
    src = inputs.pop(0)
    for i in inputs:
      cmdstr += " -i " + os.path.splitext(i)[0]
    cmdstr += " -o " + target + " " + src

    if ("LINK_PYTHON_STATIC" in opts):
        os.environ["LINK_PYTHON_STATIC"] = "1"
    oscmd(cmdstr)
    if ("LINK_PYTHON_STATIC" in os.environ):
        del os.environ["LINK_PYTHON_STATIC"]

    if (not os.path.exists(target)):
        exit("FREEZER_ERROR")

##########################################################################################
#
# Package
#
##########################################################################################

def Package(target, inputs, opts):
    assert len(inputs) == 1
    # Invoke the ppackage script.
    command = sys.executable + " "
    if GetOptimizeOption(opts) >= 4:
        command += "-OO "

    if sys.version_info >= (2, 6):
        command += "-B "

    command += "direct/src/p3d/ppackage.py"

    if GetTarget() == "darwin":
        if SDK.get("MACOSX") is not None:
            command += " -R \"%s\"" % SDK["MACOSX"]
        if UNIVERSAL:
            command += " -u"

    command += " -i \"" + GetOutputDir() + "/stage\""
    if (P3DSUFFIX):
        command += ' -a "' + P3DSUFFIX + '"'
    command += " " + inputs[0]
    oscmd(command)

##########################################################################################
#
# CompileBundle
#
##########################################################################################

def CompileBundle(target, inputs, opts):
    assert GetTarget() == "darwin", 'bundles can only be made for Mac OS X'
    plist = None
    resources = []
    objects = []
    for i in inputs:
        if (i.endswith(".plist")):
            if (plist != None): exit("Only one plist file can be used when creating a bundle!")
            plist = i
        elif (i.endswith(".rsrc") or i.endswith(".icns")):
            resources.append(i)
        elif (GetOrigExt(i) == ".obj" or GetOrigExt(i) in SUFFIX_LIB or GetOrigExt(i) in SUFFIX_DLL):
            objects.append(i)
        else:
            exit("Don't know how to bundle file %s" % i)

    # Now link the object files to form the bundle.
    if (plist == None): exit("One plist file must be used when creating a bundle!")
    bundleName = plistlib.readPlist(plist)["CFBundleExecutable"]

    oscmd("rm -rf %s" % target)
    oscmd("mkdir -p %s/Contents/MacOS/" % target)
    oscmd("mkdir -p %s/Contents/Resources/" % target)
    if target.endswith(".app"):
        SetOrigExt("%s/Contents/MacOS/%s" % (target, bundleName), ".exe")
    else:
        SetOrigExt("%s/Contents/MacOS/%s" % (target, bundleName), ".dll")
    CompileLink("%s/Contents/MacOS/%s" % (target, bundleName), objects, opts + ["BUNDLE"])
    oscmd("cp %s %s/Contents/Info.plist" % (plist, target))
    for r in resources:
        oscmd("cp %s %s/Contents/Resources/" % (r, target))

##########################################################################################
#
# CompileMIDL
#
##########################################################################################

def CompileMIDL(target, src, opts):
    ipath = GetListOption(opts, "DIR:")
    if (COMPILER=="MSVC"):
        cmd = "midl"
        cmd += " /out" + BracketNameWithQuotes(os.path.dirname(target))
        for x in ipath: cmd += " /I" + x
        for (opt,dir) in INCDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts): cmd += " /I" + BracketNameWithQuotes(dir)
        for (opt,var,val) in DEFSYMBOLS:
            if (opt=="ALWAYS") or (opt in opts): cmd += " /D" + var + "=" + val
        cmd += " " + BracketNameWithQuotes(src)

        oscmd(cmd)

##########################################################################################
#
# CompileAnything
#
##########################################################################################

def CompileAnything(target, inputs, opts, progress = None):
    if (opts.count("DEPENDENCYONLY")):
        return
    if (len(inputs)==0):
        exit("No input files for target "+target)
    infile = inputs[0]
    origsuffix = GetOrigExt(target)
    if (len(inputs) == 1 and origsuffix == GetOrigExt(infile)):
        # It must be a simple copy operation.
        ProgressOutput(progress, "Copying file", target)
        CopyFile(target, infile)
        if (origsuffix==".exe" and GetHost() != "windows"):
            os.system("chmod +x \"%s\"" % target)
        return
    elif (target.endswith("pandac/PandaModules.py")):
        ProgressOutput(progress, "Generating 'pandac' tree")
        return RunGenPyCode(target, inputs, opts)
    elif (infile.endswith(".py")):
        if (origsuffix==".exe"):
            ProgressOutput(progress, "Building frozen executable", target)
        else:
            ProgressOutput(progress, "Building frozen library", target)
        return FreezePy(target, inputs, opts)
    elif (infile.endswith(".idl")):
        ProgressOutput(progress, "Compiling MIDL file", infile)
        return CompileMIDL(target, infile, opts)
    elif (infile.endswith(".pdef")):
        ProgressOutput(progress, "Building package from pdef file", infile)
        return Package(target, inputs, opts)
    elif origsuffix in SUFFIX_LIB:
        ProgressOutput(progress, "Linking static library", target)
        return CompileLib(target, inputs, opts)
    elif origsuffix in SUFFIX_DLL or (origsuffix==".plugin" and GetTarget() != "darwin"):
        if (origsuffix==".exe"):
            ProgressOutput(progress, "Linking executable", target)
        else:
            ProgressOutput(progress, "Linking dynamic library", target)

        # Add version number to the dynamic library, on unix
        if origsuffix == ".dll" and "MODULE" not in opts and not RTDIST:
            tplatform = GetTarget()
            if tplatform == "darwin":
                # On Mac, libraries are named like libpanda.1.2.dylib
                if target.lower().endswith(".dylib"):
                    target = target[:-5] + MAJOR_VERSION + ".dylib"
                    SetOrigExt(target, origsuffix)
            elif tplatform != "windows" and tplatform != "android":
                # On Linux, libraries are named like libpanda.so.1.2
                target += "." + MAJOR_VERSION
                SetOrigExt(target, origsuffix)
        return CompileLink(target, inputs, opts)
    elif (origsuffix==".in"):
        ProgressOutput(progress, "Building Interrogate database", target)
        return CompileIgate(target, inputs, opts)
    elif (origsuffix==".plugin" and GetTarget() == "darwin"):
        ProgressOutput(progress, "Building plugin bundle", target)
        return CompileBundle(target, inputs, opts)
    elif (origsuffix==".app"):
        ProgressOutput(progress, "Building application bundle", target)
        return CompileBundle(target, inputs, opts)
    elif (origsuffix==".pz"):
        ProgressOutput(progress, "Compressing", target)
        return CompileEgg(target, infile, opts)
    elif (origsuffix==".egg"):
        ProgressOutput(progress, "Converting", target)
        return CompileEgg(target, infile, opts)
    elif (origsuffix==".res"):
        ProgressOutput(progress, "Building resource object", target)
        return CompileRes(target, infile, opts)
    elif (origsuffix==".rsrc"):
        ProgressOutput(progress, "Building resource object", target)
        return CompileRsrc(target, infile, opts)
    elif (origsuffix==".obj"):
        if (infile.endswith(".cxx")):
            ProgressOutput(progress, "Building C++ object", target)
            return CompileCxx(target, infile, opts)
        elif (infile.endswith(".c")):
            ProgressOutput(progress, "Building C object", target)
            return CompileCxx(target, infile, opts)
        elif (infile.endswith(".mm")):
            ProgressOutput(progress, "Building Objective-C++ object", target)
            return CompileCxx(target, infile, opts)
        elif (infile.endswith(".yxx")):
            ProgressOutput(progress, "Building Bison object", target)
            return CompileBison(target, infile, opts)
        elif (infile.endswith(".lxx")):
            ProgressOutput(progress, "Building Flex object", target)
            return CompileFlex(target, infile, opts)
        elif (infile.endswith(".in")):
            ProgressOutput(progress, "Building Interrogate object", target)
            return CompileImod(target, inputs, opts)
        elif (infile.endswith(".rc")):
            ProgressOutput(progress, "Building resource object", target)
            return CompileRes(target, infile, opts)
        elif (infile.endswith(".r")):
            ProgressOutput(progress, "Building resource object", target)
            return CompileRsrc(target, infile, opts)
    exit("Don't know how to compile: %s from %s" % (target, inputs))

##########################################################################################
#
# Generate dtool_config.h, prc_parameters.h, and dtool_have_xxx.dat
#
##########################################################################################

DTOOL_CONFIG=[
    #_Variable_________________________Windows___________________Unix__________
    ("HAVE_PYTHON",                    '1',                      '1'),
    ("USE_DEBUG_PYTHON",               'UNDEF',                  'UNDEF'),
    ("PYTHON_FRAMEWORK",               'UNDEF',                  'UNDEF'),
    ("COMPILE_IN_DEFAULT_FONT",        '1',                      '1'),
    ("STDFLOAT_DOUBLE",                'UNDEF',                  'UNDEF'),
    ("HAVE_MAYA",                      '1',                      '1'),
    ("MAYA_PRE_5_0",                   'UNDEF',                  'UNDEF'),
    ("HAVE_SOFTIMAGE",                 'UNDEF',                  'UNDEF'),
    ("SSL_097",                        'UNDEF',                  'UNDEF'),
    ("REPORT_OPENSSL_ERRORS",          '1',                      '1'),
    ("USE_PANDAFILESTREAM",            '1',                      '1'),
    ("USE_DELETED_CHAIN",              '1',                      '1'),
    ("HAVE_WIN_TOUCHINPUT",            'UNDEF',                  'UNDEF'),
    ("HAVE_GL",                        '1',                      'UNDEF'),
    ("HAVE_GLES",                      'UNDEF',                  'UNDEF'),
    ("HAVE_GLES2",                     'UNDEF',                  'UNDEF'),
    ("HAVE_MESA",                      'UNDEF',                  'UNDEF'),
    ("MESA_MGL",                       'UNDEF',                  'UNDEF'),
    ("HAVE_SGIGL",                     'UNDEF',                  'UNDEF'),
    ("HAVE_GLX",                       'UNDEF',                  '1'),
    ("HAVE_EGL",                       'UNDEF',                  'UNDEF'),
    ("HAVE_WGL",                       '1',                      'UNDEF'),
    ("HAVE_DX8",                       'UNDEF',                  'UNDEF'),
    ("HAVE_DX9",                       'UNDEF',                  'UNDEF'),
    ("HAVE_CHROMIUM",                  'UNDEF',                  'UNDEF'),
    ("HAVE_THREADS",                   '1',                      '1'),
    ("SIMPLE_THREADS",                 'UNDEF',                  'UNDEF'),
    ("OS_SIMPLE_THREADS",              '1',                      '1'),
    ("DEBUG_THREADS",                  'UNDEF',                  'UNDEF'),
    ("HAVE_POSIX_THREADS",             'UNDEF',                  '1'),
    ("HAVE_AUDIO",                     '1',                      '1'),
    ("NOTIFY_DEBUG",                   'UNDEF',                  'UNDEF'),
    ("DO_PSTATS",                      'UNDEF',                  'UNDEF'),
    ("DO_DCAST",                       'UNDEF',                  'UNDEF'),
    ("DO_COLLISION_RECORDING",         'UNDEF',                  'UNDEF'),
    ("SUPPORT_IMMEDIATE_MODE",         '1',                      '1'),
    ("TRACK_IN_INTERPRETER",           'UNDEF',                  'UNDEF'),
    ("DO_MEMORY_USAGE",                'UNDEF',                  'UNDEF'),
    ("DO_PIPELINING",                  '1',                      '1'),
    ("EXPORT_TEMPLATES",               'yes',                    'yes'),
    ("LINK_IN_GL",                     'UNDEF',                  'UNDEF'),
    ("LINK_IN_PHYSICS",                'UNDEF',                  'UNDEF'),
    ("DEFAULT_PATHSEP",                '";"',                    '":"'),
    ("WORDS_BIGENDIAN",                'UNDEF',                  'UNDEF'),
    ("HAVE_NAMESPACE",                 '1',                      '1'),
    ("HAVE_OPEN_MASK",                 'UNDEF',                  'UNDEF'),
    ("HAVE_LOCKF",                     '1',                      '1'),
    ("HAVE_WCHAR_T",                   '1',                      '1'),
    ("HAVE_WSTRING",                   '1',                      '1'),
    ("HAVE_TYPENAME",                  '1',                      '1'),
    ("SIMPLE_STRUCT_POINTERS",         '1',                      'UNDEF'),
    ("HAVE_DINKUM",                    'UNDEF',                  'UNDEF'),
    ("HAVE_STL_HASH",                  'UNDEF',                  'UNDEF'),
    ("GETTIMEOFDAY_ONE_PARAM",         'UNDEF',                  'UNDEF'),
    ("HAVE_GETOPT",                    'UNDEF',                  '1'),
    ("HAVE_GETOPT_LONG_ONLY",          'UNDEF',                  '1'),
    ("PHAVE_GETOPT_H",                 'UNDEF',                  '1'),
    ("PHAVE_LINUX_INPUT_H",            'UNDEF',                  '1'),
    ("IOCTL_TERMINAL_WIDTH",           'UNDEF',                  '1'),
    ("HAVE_STREAMSIZE",                '1',                      '1'),
    ("HAVE_IOS_TYPEDEFS",              '1',                      '1'),
    ("HAVE_IOS_BINARY",                '1',                      '1'),
    ("STATIC_INIT_GETENV",             '1',                      'UNDEF'),
    ("HAVE_PROC_SELF_EXE",             'UNDEF',                  '1'),
    ("HAVE_PROC_SELF_MAPS",            'UNDEF',                  '1'),
    ("HAVE_PROC_SELF_ENVIRON",         'UNDEF',                  '1'),
    ("HAVE_PROC_SELF_CMDLINE",         'UNDEF',                  '1'),
    ("HAVE_PROC_CURPROC_FILE",         'UNDEF',                  'UNDEF'),
    ("HAVE_PROC_CURPROC_MAP",          'UNDEF',                  'UNDEF'),
    ("HAVE_PROC_SELF_CMDLINE",         'UNDEF',                  'UNDEF'),
    ("HAVE_GLOBAL_ARGV",               '1',                      'UNDEF'),
    ("PROTOTYPE_GLOBAL_ARGV",          'UNDEF',                  'UNDEF'),
    ("GLOBAL_ARGV",                    '__argv',                 'UNDEF'),
    ("GLOBAL_ARGC",                    '__argc',                 'UNDEF'),
    ("PHAVE_IO_H",                     '1',                      'UNDEF'),
    ("PHAVE_IOSTREAM",                 '1',                      '1'),
    ("PHAVE_STRING_H",                 'UNDEF',                  '1'),
    ("PHAVE_LIMITS_H",                 'UNDEF',                  '1'),
    ("PHAVE_STDLIB_H",                 'UNDEF',                  '1'),
    ("PHAVE_MALLOC_H",                 '1',                      '1'),
    ("PHAVE_SYS_MALLOC_H",             'UNDEF',                  'UNDEF'),
    ("PHAVE_ALLOCA_H",                 'UNDEF',                  '1'),
    ("PHAVE_LOCALE_H",                 'UNDEF',                  '1'),
    ("PHAVE_MINMAX_H",                 '1',                      'UNDEF'),
    ("PHAVE_SSTREAM",                  '1',                      '1'),
    ("PHAVE_NEW",                      '1',                      '1'),
    ("PHAVE_SYS_TYPES_H",              '1',                      '1'),
    ("PHAVE_SYS_TIME_H",               'UNDEF',                  '1'),
    ("PHAVE_UNISTD_H",                 'UNDEF',                  '1'),
    ("PHAVE_UTIME_H",                  'UNDEF',                  '1'),
    ("PHAVE_GLOB_H",                   'UNDEF',                  '1'),
    ("PHAVE_DIRENT_H",                 'UNDEF',                  '1'),
    ("PHAVE_SYS_SOUNDCARD_H",          'UNDEF',                  '1'),
    ("PHAVE_UCONTEXT_H",               'UNDEF',                  '1'),
    ("PHAVE_STDINT_H",                 'UNDEF',                  '1'),
    ("HAVE_RTTI",                      '1',                      '1'),
    ("HAVE_X11",                       'UNDEF',                  '1'),
    ("HAVE_XRANDR",                    'UNDEF',                  '1'),
    ("HAVE_XF86DGA",                   'UNDEF',                  '1'),
    ("HAVE_XCURSOR",                   'UNDEF',                  '1'),
    ("IS_LINUX",                       'UNDEF',                  '1'),
    ("IS_OSX",                         'UNDEF',                  'UNDEF'),
    ("IS_FREEBSD",                     'UNDEF',                  'UNDEF'),
    ("GLOBAL_OPERATOR_NEW_EXCEPTIONS", 'UNDEF',                  '1'),
    ("HAVE_EIGEN",                     'UNDEF',                  'UNDEF'),
    ("LINMATH_ALIGN",                  '1',                      '1'),
    ("HAVE_ZLIB",                      'UNDEF',                  'UNDEF'),
    ("HAVE_PNG",                       'UNDEF',                  'UNDEF'),
    ("HAVE_JPEG",                      'UNDEF',                  'UNDEF'),
    ("PHAVE_JPEGINT_H",                '1',                      '1'),
    ("HAVE_VIDEO4LINUX",               'UNDEF',                  '1'),
    ("HAVE_TIFF",                      'UNDEF',                  'UNDEF'),
    ("HAVE_SGI_RGB",                   '1',                      '1'),
    ("HAVE_TGA",                       '1',                      '1'),
    ("HAVE_IMG",                       '1',                      '1'),
    ("HAVE_SOFTIMAGE_PIC",             '1',                      '1'),
    ("HAVE_BMP",                       '1',                      '1'),
    ("HAVE_PNM",                       '1',                      '1'),
    ("HAVE_VORBIS",                    'UNDEF',                  'UNDEF'),
    ("HAVE_FMODEX",                    'UNDEF',                  'UNDEF'),
    ("HAVE_OPENAL",                    'UNDEF',                  'UNDEF'),
    ("HAVE_NVIDIACG",                  'UNDEF',                  'UNDEF'),
    ("HAVE_FREETYPE",                  'UNDEF',                  'UNDEF'),
    ("HAVE_SPEEDTREE",                 'UNDEF',                  'UNDEF'),
    ("HAVE_FFTW",                      'UNDEF',                  'UNDEF'),
    ("HAVE_OPENSSL",                   'UNDEF',                  'UNDEF'),
    ("HAVE_NET",                       'UNDEF',                  'UNDEF'),
    ("HAVE_EGG",                       '1',                      '1'),
    ("HAVE_CG",                        'UNDEF',                  'UNDEF'),
    ("HAVE_CGGL",                      'UNDEF',                  'UNDEF'),
    ("HAVE_CGDX9",                     'UNDEF',                  'UNDEF'),
    ("HAVE_FFMPEG",                    'UNDEF',                  'UNDEF'),
    ("HAVE_SWSCALE",                   'UNDEF',                  'UNDEF'),
    ("HAVE_SWRESAMPLE",                'UNDEF',                  'UNDEF'),
    ("HAVE_ARTOOLKIT",                 'UNDEF',                  'UNDEF'),
    ("HAVE_ODE",                       'UNDEF',                  'UNDEF'),
    ("HAVE_OPENCV",                    'UNDEF',                  'UNDEF'),
    ("HAVE_DIRECTCAM",                 'UNDEF',                  'UNDEF'),
    ("HAVE_SQUISH",                    'UNDEF',                  'UNDEF'),
    ("HAVE_FCOLLADA",                  'UNDEF',                  'UNDEF'),
    ("HAVE_CARBON",                    'UNDEF',                  'UNDEF'),
    ("HAVE_COCOA",                     'UNDEF',                  'UNDEF'),
    ("HAVE_OPENAL_FRAMEWORK",          'UNDEF',                  'UNDEF'),
    ("HAVE_ROCKET_PYTHON",             '1',                      '1'),
    ("HAVE_ROCKET_DEBUGGER",           'UNDEF',                  'UNDEF'),
    ("PRC_SAVE_DESCRIPTIONS",          '1',                      '1'),
#    ("_SECURE_SCL",                    '0',                      'UNDEF'),
#    ("_SECURE_SCL_THROWS",             '0',                      'UNDEF'),
    ("HAVE_P3D_PLUGIN",                'UNDEF',                  'UNDEF'),
]

PRC_PARAMETERS=[
    ("DEFAULT_PRC_DIR",                '"<auto>etc"',            '"<auto>etc"'),
    ("PRC_DIR_ENVVARS",                '"PANDA_PRC_DIR"',        '"PANDA_PRC_DIR"'),
    ("PRC_PATH_ENVVARS",               '"PANDA_PRC_PATH"',       '"PANDA_PRC_PATH"'),
    ("PRC_PATH2_ENVVARS",              '""',                     '""'),
    ("PRC_PATTERNS",                   '"*.prc"',                '"*.prc"'),
    ("PRC_ENCRYPTED_PATTERNS",         '"*.prc.pe"',             '"*.prc.pe"'),
    ("PRC_ENCRYPTION_KEY",             '""',                     '""'),
    ("PRC_EXECUTABLE_PATTERNS",        '""',                     '""'),
    ("PRC_EXECUTABLE_ARGS_ENVVAR",     '"PANDA_PRC_XARGS"',      '"PANDA_PRC_XARGS"'),
    ("PRC_PUBLIC_KEYS_FILENAME",       '""',                     '""'),
    ("PRC_RESPECT_TRUST_LEVEL",        'UNDEF',                  'UNDEF'),
    ("PRC_DCONFIG_TRUST_LEVEL",        '0',                      '0'),
    ("PRC_INC_TRUST_LEVEL",            '0',                      '0'),
]

def WriteConfigSettings():
    dtool_config={}
    prc_parameters={}
    speedtree_parameters={}
    plugin_config={}

    if (GetTarget() == 'windows'):
        for key,win,unix in DTOOL_CONFIG:
            dtool_config[key] = win
        for key,win,unix in PRC_PARAMETERS:
            prc_parameters[key] = win
    else:
        for key,win,unix in DTOOL_CONFIG:
            dtool_config[key] = unix
        for key,win,unix in PRC_PARAMETERS:
            prc_parameters[key] = unix

    for x in PkgListGet():
        if ("HAVE_"+x in dtool_config):
            if (PkgSkip(x)==0):
                dtool_config["HAVE_"+x] = '1'
            else:
                dtool_config["HAVE_"+x] = 'UNDEF'

    dtool_config["HAVE_NET"] = '1'

    if (PkgSkip("NVIDIACG")==0):
        dtool_config["HAVE_CG"] = '1'
        dtool_config["HAVE_CGGL"] = '1'
        dtool_config["HAVE_CGDX9"] = '1'

    if (GetTarget() != "linux"):
        dtool_config["HAVE_PROC_SELF_EXE"] = 'UNDEF'
        dtool_config["HAVE_PROC_SELF_MAPS"] = 'UNDEF'
        dtool_config["HAVE_PROC_SELF_CMDLINE"] = 'UNDEF'
        dtool_config["HAVE_PROC_SELF_ENVIRON"] = 'UNDEF'

    if (GetTarget() == "darwin"):
        dtool_config["PYTHON_FRAMEWORK"] = 'Python'
        dtool_config["PHAVE_MALLOC_H"] = 'UNDEF'
        dtool_config["PHAVE_SYS_MALLOC_H"] = '1'
        dtool_config["HAVE_OPENAL_FRAMEWORK"] = '1'
        dtool_config["HAVE_X11"] = 'UNDEF'  # We might have X11, but we don't need it.
        dtool_config["HAVE_XRANDR"] = 'UNDEF'
        dtool_config["HAVE_XF86DGA"] = 'UNDEF'
        dtool_config["HAVE_XCURSOR"] = 'UNDEF'
        dtool_config["HAVE_GLX"] = 'UNDEF'
        dtool_config["IS_LINUX"] = 'UNDEF'
        dtool_config["HAVE_VIDEO4LINUX"] = 'UNDEF'
        dtool_config["IS_OSX"] = '1'
        # 10.4 had a broken ucontext implementation
        if int(platform.mac_ver()[0][3]) <= 4:
            dtool_config["PHAVE_UCONTEXT_H"] = 'UNDEF'

    if (GetTarget() == "freebsd"):
        dtool_config["IS_LINUX"] = 'UNDEF'
        dtool_config["HAVE_VIDEO4LINUX"] = 'UNDEF'
        dtool_config["IS_FREEBSD"] = '1'
        dtool_config["PHAVE_ALLOCA_H"] = 'UNDEF'
        dtool_config["PHAVE_MALLOC_H"] = 'UNDEF'
        dtool_config["PHAVE_LINUX_INPUT_H"] = 'UNDEF'
        dtool_config["HAVE_PROC_CURPROC_FILE"] = '1'
        dtool_config["HAVE_PROC_CURPROC_MAP"] = '1'
        dtool_config["HAVE_PROC_CURPROC_CMDLINE"] = '1'

    if (GetTarget() == "android"):
        # Android does have RTTI, but we disable it anyway.
        dtool_config["HAVE_RTTI"] = 'UNDEF'
        dtool_config["PHAVE_GLOB_H"] = 'UNDEF'
        dtool_config["HAVE_LOCKF"] = 'UNDEF'
        dtool_config["HAVE_VIDEO4LINUX"] = 'UNDEF'

    if (GetOptimize() <= 2 and GetTarget() == "windows"):
        dtool_config["USE_DEBUG_PYTHON"] = '1'

    # This should probably be more sophisticated, such as based
    # on whether the libRocket Python modules are available.
    if (PkgSkip("PYTHON") != 0):
        dtool_config["HAVE_ROCKET_PYTHON"] = 'UNDEF'

    if (PkgSkip("TOUCHINPUT") == 0 and GetTarget() == "windows"):
        dtool_config["HAVE_WIN_TOUCHINPUT"] = '1'
    
    if (GetOptimize() <= 3):
        dtool_config["HAVE_ROCKET_DEBUGGER"] = '1'

    if (GetOptimize() <= 3):
        if (dtool_config["HAVE_NET"] != 'UNDEF'):
            dtool_config["DO_PSTATS"] = '1'

    if (GetOptimize() <= 3):
        dtool_config["DO_DCAST"] = '1'

    if (GetOptimize() <= 3):
        dtool_config["DO_COLLISION_RECORDING"] = '1'

    #if (GetOptimize() <= 2):
    #    dtool_config["TRACK_IN_INTERPRETER"] = '1'

    if (GetOptimize() <= 3):
        dtool_config["DO_MEMORY_USAGE"] = '1'

    if (GetOptimize() <= 3):
        dtool_config["NOTIFY_DEBUG"] = '1'

    if (GetOptimize() >= 4):
        dtool_config["PRC_SAVE_DESCRIPTIONS"] = 'UNDEF'

    # Now that we have OS_SIMPLE_THREADS, we can support
    # SIMPLE_THREADS on exotic architectures like win64, so we no
    # longer need to disable it for this platform.
##     if GetTarget() == 'windows' and GetTargetArch() == 'x64':
##         dtool_config["SIMPLE_THREADS"] = 'UNDEF'

    if (RTDIST or RUNTIME):
        prc_parameters["DEFAULT_PRC_DIR"] = '""'
        plugin_config["PANDA_PACKAGE_HOST_URL"] = HOST_URL
        #plugin_config["P3D_PLUGIN_LOG_DIRECTORY"] = ""
        plugin_config["P3D_PLUGIN_LOG_BASENAME1"] = ""
        plugin_config["P3D_PLUGIN_LOG_BASENAME2"] = ""
        plugin_config["P3D_PLUGIN_LOG_BASENAME3"] = ""
        plugin_config["P3D_PLUGIN_P3D_PLUGIN"] = ""
        plugin_config["P3D_PLUGIN_P3DPYTHON"] = ""
        plugin_config["P3D_COREAPI_VERSION_STR"] = COREAPI_VERSION
        plugin_config["P3D_PLUGIN_VERSION_STR"] = PLUGIN_VERSION
        if PkgSkip("GTK2") == 0:
            plugin_config["HAVE_GTK"] = '1'

    if (RUNTIME):
        dtool_config["HAVE_P3D_PLUGIN"] = '1'

    # Whether it's present on the system doesn't matter here,
    # as the runtime itself doesn't include or link with X11.
    if (RUNTIME and GetTarget() == 'linux'):
        dtool_config["HAVE_X11"] = '1'

    if ("GENERIC_DXERR_LIBRARY" in SDK):
        dtool_config["USE_GENERIC_DXERR_LIBRARY"] = "1"
    else:
        dtool_config["USE_GENERIC_DXERR_LIBRARY"] = "UNDEF"

    if (PkgSkip("SPEEDTREE")==0):
        speedtree_parameters["SPEEDTREE_OPENGL"] = "UNDEF"
        speedtree_parameters["SPEEDTREE_DIRECTX9"] = "UNDEF"
        if SDK["SPEEDTREEAPI"] == "OpenGL":
            speedtree_parameters["SPEEDTREE_OPENGL"] = "1"
        elif SDK["SPEEDTREEAPI"] == "DirectX9":
            speedtree_parameters["SPEEDTREE_DIRECTX9"] = "1"

        speedtree_parameters["SPEEDTREE_BIN_DIR"] = (SDK["SPEEDTREE"] + "/Bin")

    conf = "/* prc_parameters.h.  Generated automatically by makepanda.py */\n"
    for key in sorted(prc_parameters.keys()):
        if ((key == "DEFAULT_PRC_DIR") or (key[:4]=="PRC_")):
            val = OverrideValue(key, prc_parameters[key])
            if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
            else:                conf = conf + "#define " + key + " " + val + "\n"
    ConditionalWriteFile(GetOutputDir() + '/include/prc_parameters.h', conf)

    conf = "/* dtool_config.h.  Generated automatically by makepanda.py */\n"
    for key in sorted(dtool_config.keys()):
        val = OverrideValue(key, dtool_config[key])
        if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
        else:                conf = conf + "#define " + key + " " + val + "\n"
    ConditionalWriteFile(GetOutputDir() + '/include/dtool_config.h', conf)

    if (RTDIST or RUNTIME):
        conf = "/* p3d_plugin_config.h.  Generated automatically by makepanda.py */\n"
        for key in sorted(plugin_config.keys()):
            val = plugin_config[key]
            if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
            else:                conf = conf + "#define " + key + " \"" + val.replace("\\", "\\\\") + "\"\n"
        ConditionalWriteFile(GetOutputDir() + '/include/p3d_plugin_config.h', conf)

    if (PkgSkip("SPEEDTREE")==0):
        conf = "/* speedtree_parameters.h.  Generated automatically by makepanda.py */\n"
        for key in sorted(speedtree_parameters.keys()):
            val = OverrideValue(key, speedtree_parameters[key])
            if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
            else:                conf = conf + "#define " + key + " \"" + val.replace("\\", "\\\\") + "\"\n"
        ConditionalWriteFile(GetOutputDir() + '/include/speedtree_parameters.h', conf)

    for x in PkgListGet():
        if (PkgSkip(x)): ConditionalWriteFile(GetOutputDir() + '/tmp/dtool_have_'+x.lower()+'.dat', "0\n")
        else:            ConditionalWriteFile(GetOutputDir() + '/tmp/dtool_have_'+x.lower()+'.dat', "1\n")

WriteConfigSettings()

WarnConflictingFiles()
if SystemLibraryExists("dtoolbase"):
    print("%sWARNING:%s Found conflicting Panda3D libraries from other ppremake build!" % (GetColor("red"), GetColor()))
if SystemLibraryExists("p3dtoolconfig"):
    print("%sWARNING:%s Found conflicting Panda3D libraries from other makepanda build!" % (GetColor("red"), GetColor()))

##########################################################################################
#
# Generate pandaVersion.h, pythonversion, null.cxx, etc.
#
##########################################################################################

PANDAVERSION_H="""
#define PANDA_MAJOR_VERSION $VERSION1
#define PANDA_MINOR_VERSION $VERSION2
#define PANDA_SEQUENCE_VERSION $VERSION3
#define PANDA_VERSION $NVERSION
#define PANDA_NUMERIC_VERSION $NVERSION
#define PANDA_VERSION_STR "$VERSION1.$VERSION2.$VERSION3"
#define PANDA_ABI_VERSION_STR "$VERSION1.$VERSION2"
#define PANDA_DISTRIBUTOR "$DISTRIBUTOR"
#define PANDA_PACKAGE_VERSION_STR "$RTDIST_VERSION"
#define PANDA_PACKAGE_HOST_URL "$HOST_URL"
"""

PANDAVERSION_H_RUNTIME="""
#define PANDA_MAJOR_VERSION 0
#define PANDA_MINOR_VERSION 0
#define PANDA_SEQUENCE_VERSION 0
#define PANDA_VERSION_STR "0.0.0"
#define PANDA_ABI_VERSION_STR "0.0"
#define P3D_PLUGIN_MAJOR_VERSION $VERSION1
#define P3D_PLUGIN_MINOR_VERSION $VERSION2
#define P3D_PLUGIN_SEQUENCE_VERSION $VERSION3
#define P3D_PLUGIN_VERSION_STR "$VERSION1.$VERSION2.$VERSION3"
#define P3D_COREAPI_VERSION_STR "$COREAPI_VERSION"
#define PANDA_DISTRIBUTOR "$DISTRIBUTOR"
#define PANDA_PACKAGE_VERSION_STR ""
#define PANDA_PACKAGE_HOST_URL "$HOST_URL"
"""

CHECKPANDAVERSION_CXX="""
# include "dtoolbase.h"
EXPCL_DTOOL int panda_version_$VERSION1_$VERSION2 = 0;
"""

CHECKPANDAVERSION_H="""
# include "dtoolbase.h"
extern EXPCL_DTOOL int panda_version_$VERSION1_$VERSION2;
# ifndef WIN32
/* For Windows, exporting the symbol from the DLL is sufficient; the
      DLL will not load unless all expected public symbols are defined.
      Other systems may not mind if the symbol is absent unless we
      explictly write code that references it. */
static int check_panda_version = panda_version_$VERSION1_$VERSION2;
# endif
"""

P3DACTIVEX_RC="""#include "resource.h"
#define APSTUDIO_READONLY_SYMBOLS
#include "afxres.h"
#undef APSTUDIO_READONLY_SYMBOLS
#if !defined(AFX_RESOURCE_DLL) || defined(AFX_TARG_ENU)
#ifdef _WIN32
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US
#pragma code_page(1252)
#endif
#ifdef APSTUDIO_INVOKED
1 TEXTINCLUDE
BEGIN
    "resource.h\\0"
END
2 TEXTINCLUDE
BEGIN
   "#include ""afxres.h""\\r\\n"
    "\\0"
END
3 TEXTINCLUDE
BEGIN
    "1 TYPELIB ""P3DActiveX.tlb""\\r\\n"
    "\\0"
END
#endif
%s
IDB_P3DACTIVEX          BITMAP                  "P3DActiveXCtrl.bmp"
IDD_PROPPAGE_P3DACTIVEX DIALOG  0, 0, 250, 62
STYLE DS_SETFONT | WS_CHILD
FONT 8, "MS Sans Serif"
BEGIN
    LTEXT           "TODO: Place controls to manipulate properties of P3DActiveX Control on this dialog.",
                    IDC_STATIC,7,25,229,16
END
#ifdef APSTUDIO_INVOKED
GUIDELINES DESIGNINFO
BEGIN
    IDD_PROPPAGE_P3DACTIVEX, DIALOG
    BEGIN
        LEFTMARGIN, 7
        RIGHTMARGIN, 243
        TOPMARGIN, 7
        BOTTOMMARGIN, 55
    END
END
#endif
STRINGTABLE
BEGIN
    IDS_P3DACTIVEX          "P3DActiveX Control"
    IDS_P3DACTIVEX_PPG      "P3DActiveX Property Page"
END
STRINGTABLE
BEGIN
    IDS_P3DACTIVEX_PPG_CAPTION "General"
END
#endif
#ifndef APSTUDIO_INVOKED
1 TYPELIB "P3DActiveX.tlb"
#endif"""

def CreatePandaVersionFiles():
    version1=int(VERSION.split(".")[0])
    version2=int(VERSION.split(".")[1])
    version3=int(VERSION.split(".")[2])
    nversion=version1*1000000+version2*1000+version3
    if (DISTRIBUTOR != "cmu"):
        # Subtract 1 if we are not an official version.
        nversion -= 1

    if (RUNTIME):
        pandaversion_h = PANDAVERSION_H_RUNTIME
    else:
        pandaversion_h = PANDAVERSION_H
    pandaversion_h = pandaversion_h.replace("$VERSION1",str(version1))
    pandaversion_h = pandaversion_h.replace("$VERSION2",str(version2))
    pandaversion_h = pandaversion_h.replace("$VERSION3",str(version3))
    pandaversion_h = pandaversion_h.replace("$NVERSION",str(nversion))
    pandaversion_h = pandaversion_h.replace("$DISTRIBUTOR",DISTRIBUTOR)
    pandaversion_h = pandaversion_h.replace("$RTDIST_VERSION",RTDIST_VERSION)
    pandaversion_h = pandaversion_h.replace("$COREAPI_VERSION",COREAPI_VERSION)
    pandaversion_h = pandaversion_h.replace("$HOST_URL",HOST_URL)
    if (DISTRIBUTOR == "cmu"):
        pandaversion_h += "\n#define PANDA_OFFICIAL_VERSION\n"
    else:
        pandaversion_h += "\n#undef  PANDA_OFFICIAL_VERSION\n"

    if not RUNTIME:
        checkpandaversion_cxx = CHECKPANDAVERSION_CXX.replace("$VERSION1",str(version1))
        checkpandaversion_cxx = checkpandaversion_cxx.replace("$VERSION2",str(version2))
        checkpandaversion_cxx = checkpandaversion_cxx.replace("$VERSION3",str(version3))
        checkpandaversion_cxx = checkpandaversion_cxx.replace("$NVERSION",str(nversion))

        checkpandaversion_h = CHECKPANDAVERSION_H.replace("$VERSION1",str(version1))
        checkpandaversion_h = checkpandaversion_h.replace("$VERSION2",str(version2))
        checkpandaversion_h = checkpandaversion_h.replace("$VERSION3",str(version3))
        checkpandaversion_h = checkpandaversion_h.replace("$NVERSION",str(nversion))

    ConditionalWriteFile(GetOutputDir()+'/include/pandaVersion.h',        pandaversion_h)
    if RUNTIME:
        ConditionalWriteFile(GetOutputDir()+'/include/checkPandaVersion.cxx', '')
        ConditionalWriteFile(GetOutputDir()+'/include/checkPandaVersion.h', '')
    else:
        ConditionalWriteFile(GetOutputDir()+'/include/checkPandaVersion.cxx', checkpandaversion_cxx)
        ConditionalWriteFile(GetOutputDir()+'/include/checkPandaVersion.h',   checkpandaversion_h)
    ConditionalWriteFile(GetOutputDir()+"/tmp/null.cxx","")

    if RUNTIME:
        p3dactivex_rc = {"name" : "Panda3D Game Engine Plug-in",
                         "version" : VERSION,
                         "description" : "Runs 3-D games and interactive applets",
                         "filename" : "p3dactivex.ocx",
                         "mimetype" : "application/x-panda3d",
                         "extension" : "p3d",
                         "filedesc" : "Panda3D applet"}
        ConditionalWriteFile(GetOutputDir()+"/include/P3DActiveX.rc", P3DACTIVEX_RC % GenerateResourceFile(**p3dactivex_rc))

CreatePandaVersionFiles()

##########################################################################################
#
# Copy the "direct" tree
#
##########################################################################################

if (PkgSkip("DIRECT")==0):
    CopyPythonTree(GetOutputDir() + '/direct', 'direct/src', lib2to3_fixers=['all'])
    ConditionalWriteFile(GetOutputDir() + '/direct/__init__.py', "")

    # This file used to be copied, but would nowadays cause conflicts.
    # Let's get it out of the way in case someone hasn't cleaned their build since.
    if os.path.isfile(GetOutputDir() + '/bin/panda3d.py'):
        os.remove(GetOutputDir() + '/bin/panda3d.py')
    if os.path.isfile(GetOutputDir() + '/lib/panda3d.py'):
        os.remove(GetOutputDir() + '/lib/panda3d.py')

    # Don't copy this file, which would cause conflict with our 'panda3d' module.
    if os.path.isfile(GetOutputDir() + '/direct/ffi/panda3d.py'):
        os.remove(GetOutputDir() + '/direct/ffi/panda3d.py')
    if os.path.isfile(GetOutputDir() + '/direct/ffi/panda3d.pyc'):
        os.remove(GetOutputDir() + '/direct/ffi/panda3d.pyc')

##########################################################################################
#
# Generate the PRC files into the ETC directory.
#
##########################################################################################

confautoprc = ReadFile("makepanda/confauto.in")
if (PkgSkip("SPEEDTREE")==0):
    # If SpeedTree is available, enable it in the config file
    confautoprc = confautoprc.replace('#st#', '')
else:
    # otherwise, disable it.
    confautoprc = confautoprc.replace('#st#', '#')

if (os.path.isfile("makepanda/myconfig.in")):
    configprc = ReadFile("makepanda/myconfig.in")
else:
    configprc = ReadFile("makepanda/config.in")

if (GetTarget() == 'windows'):
    configprc = configprc.replace("$HOME/.panda3d", "$USER_APPDATA/Panda3D-%s" % MAJOR_VERSION)
else:
    configprc = configprc.replace("aux-display pandadx9", "")
    configprc = configprc.replace("aux-display pandadx8", "")

if (GetTarget() == 'darwin'):
    configprc = configprc.replace(".panda3d/cache", "Library/Caches/Panda3D-%s" % MAJOR_VERSION)

    # OpenAL is not yet working well on OSX for us, so let's do this for now.
    configprc = configprc.replace("p3openal_audio", "p3fmod_audio")

ConditionalWriteFile(GetOutputDir()+"/etc/Config.prc", configprc)
ConditionalWriteFile(GetOutputDir()+"/etc/Confauto.prc", confautoprc)

##########################################################################################
#
# Copy the precompiled binaries and DLLs into the build.
#
##########################################################################################

tp_dir = GetThirdpartyDir()
if tp_dir is not None:
    for pkg in PkgListGet():
        if PkgSkip(pkg):
            continue
        tp_pkg = tp_dir + pkg.lower()

        if GetTarget() == 'windows':
            if os.path.exists(tp_pkg + "/bin"):
                CopyAllFiles(GetOutputDir() + "/bin/", tp_pkg + "/bin/")
                if (PkgSkip("PYTHON")==0 and os.path.exists(tp_pkg + "/bin/" + SDK["PYTHONVERSION"])):
                    CopyAllFiles(GetOutputDir() + "/bin/", tp_pkg + "/bin/" + SDK["PYTHONVERSION"] + "/")
        else:
            if os.path.exists(tp_pkg + "/lib"):
                CopyAllFiles(GetOutputDir() + "/lib/", tp_pkg + "/lib/")
                if (PkgSkip("PYTHON")==0 and os.path.exists(tp_pkg + "/lib/" + SDK["PYTHONVERSION"])):
                    CopyAllFiles(GetOutputDir() + "/lib/", tp_pkg + "/lib/" + SDK["PYTHONVERSION"] + "/")

    if GetTarget() == 'windows':
        CopyAllFiles(GetOutputDir() + "/bin/", tp_dir + "extras/bin/")
        if PkgSkip("PYTHON") == 0:
            pydll = "/" + SDK["PYTHONVERSION"].replace(".", "")
            if (GetOptimize() <= 2): pydll += "_d.dll"
            else: pydll += ".dll"
            CopyFile(GetOutputDir() + "/bin" + pydll, SDK["PYTHON"] + pydll)
            if not RTDIST:
                CopyTree(GetOutputDir() + "/python", SDK["PYTHON"])
                ConditionalWriteFile(GetOutputDir() + "/python/panda.pth", "..\n../bin\n")

########################################################################
##
## Copy various stuff into the build.
##
########################################################################

CopyFile(GetOutputDir()+"/", "doc/LICENSE")
CopyFile(GetOutputDir()+"/", "doc/ReleaseNotes")
if (PkgSkip("PANDATOOL")==0):
    CopyAllFiles(GetOutputDir()+"/plugins/",  "pandatool/src/scripts/", ".mel")
    CopyAllFiles(GetOutputDir()+"/plugins/",  "pandatool/src/scripts/", ".ms")
if (PkgSkip("PYTHON")==0 and os.path.isdir(GetThirdpartyBase()+"/Pmw")):
    CopyTree(GetOutputDir()+'/Pmw',         GetThirdpartyBase()+'/Pmw')
ConditionalWriteFile(GetOutputDir()+'/include/ctl3d.h', '/* dummy file to make MAX happy */')

########################################################################
#
# Copy header files to the built/include/parser-inc directory.
#
########################################################################

CopyTree(GetOutputDir()+'/include/parser-inc','dtool/src/parser-inc')
MakeDirectory(GetOutputDir()+'/include/parser-inc/openssl')
MakeDirectory(GetOutputDir()+'/include/parser-inc/netinet')
MakeDirectory(GetOutputDir()+'/include/parser-inc/Cg')
MakeDirectory(GetOutputDir()+'/include/parser-inc/Core')
MakeDirectory(GetOutputDir()+'/include/parser-inc/Forest')
MakeDirectory(GetOutputDir()+'/include/parser-inc/Renderers')
MakeDirectory(GetOutputDir()+'/include/parser-inc/Renderers/OpenGL')
MakeDirectory(GetOutputDir()+'/include/parser-inc/Renderers/DirectX9')
MakeDirectory(GetOutputDir()+'/include/parser-inc/glew')
MakeDirectory(GetOutputDir()+'/include/parser-inc/Eigen')
MakeDirectory(GetOutputDir()+'/include/parser-inc/Rocket')
MakeDirectory(GetOutputDir()+'/include/parser-inc/Rocket/Core')
CopyAllFiles(GetOutputDir()+'/include/parser-inc/openssl/','dtool/src/parser-inc/')
CopyAllFiles(GetOutputDir()+'/include/parser-inc/netinet/','dtool/src/parser-inc/')
CopyFile(GetOutputDir()+'/include/parser-inc/Cg/','dtool/src/parser-inc/cg.h')
CopyFile(GetOutputDir()+'/include/parser-inc/Cg/','dtool/src/parser-inc/cgGL.h')
CopyFile(GetOutputDir()+'/include/parser-inc/Core/','dtool/src/parser-inc/Core.h')
CopyFile(GetOutputDir()+'/include/parser-inc/Forest/','dtool/src/parser-inc/Forest.h')
CopyFile(GetOutputDir()+'/include/parser-inc/Renderers/OpenGL/','dtool/src/parser-inc/OpenGLRenderer.h')
CopyFile(GetOutputDir()+'/include/parser-inc/Renderers/DirectX9/','dtool/src/parser-inc/DirectX9Renderer.h')
CopyFile(GetOutputDir()+'/include/parser-inc/glew/','dtool/src/parser-inc/glew.h')
CopyFile(GetOutputDir()+'/include/parser-inc/Eigen/','dtool/src/parser-inc/Dense')
CopyFile(GetOutputDir()+'/include/parser-inc/Eigen/','dtool/src/parser-inc/StdVector')
CopyFile(GetOutputDir()+'/include/parser-inc/Rocket/Core/','dtool/src/parser-inc/RenderInterface.h')
DeleteCVS(GetOutputDir()+'/include/parser-inc')

########################################################################
#
# Transfer all header files to the built/include directory.
#
########################################################################

CopyAllHeaders('dtool/src/dtoolbase')
CopyAllHeaders('dtool/src/dtoolutil', skip=["pandaVersion.h", "checkPandaVersion.h"])
CopyFile(GetOutputDir()+'/include/','dtool/src/dtoolutil/vector_src.cxx')
CopyAllHeaders('dtool/metalibs/dtool')
CopyAllHeaders('dtool/src/cppparser')
CopyAllHeaders('dtool/src/prc', skip=["prc_parameters.h"])
CopyAllHeaders('dtool/src/dconfig')
CopyAllHeaders('dtool/src/interrogatedb')
CopyAllHeaders('dtool/metalibs/dtoolconfig')
CopyAllHeaders('dtool/src/pystub')
CopyAllHeaders('dtool/src/interrogate')
CopyAllHeaders('dtool/src/test_interrogate')
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
CopyAllHeaders('panda/src/movies')
CopyAllHeaders('panda/src/pgraphnodes')
CopyAllHeaders('panda/src/pgraph')
CopyAllHeaders('panda/src/cull')
CopyAllHeaders('panda/src/chan')
CopyAllHeaders('panda/src/char')
CopyAllHeaders('panda/src/dgraph')
CopyAllHeaders('panda/src/display')
CopyAllHeaders('panda/src/device')
CopyAllHeaders('panda/src/pnmtext')
CopyAllHeaders('panda/src/text')
CopyAllHeaders('panda/src/grutil')
if (PkgSkip("VISION")==0):
    CopyAllHeaders('panda/src/vision')
CopyAllHeaders('panda/src/awesomium')
if (PkgSkip("FFMPEG")==0):
    CopyAllHeaders('panda/src/ffmpeg')
CopyAllHeaders('panda/src/tform')
CopyAllHeaders('panda/src/collide')
CopyAllHeaders('panda/src/parametrics')
CopyAllHeaders('panda/src/pgui')
CopyAllHeaders('panda/src/pnmimagetypes')
CopyAllHeaders('panda/src/recorder')
if (PkgSkip("ROCKET")==0):
    CopyAllHeaders('panda/src/rocket')
if (PkgSkip("VRPN")==0):
    CopyAllHeaders('panda/src/vrpn')
CopyAllHeaders('panda/src/wgldisplay')
CopyAllHeaders('panda/src/ode')
CopyAllHeaders('panda/metalibs/pandaode')
if (PkgSkip("PANDAPHYSICS")==0):
    CopyAllHeaders('panda/src/physics')
    if (PkgSkip("PANDAPARTICLESYSTEM")==0):
        CopyAllHeaders('panda/src/particlesystem')
CopyAllHeaders('panda/src/dxml')
CopyAllHeaders('panda/metalibs/panda')
CopyAllHeaders('panda/src/audiotraits')
CopyAllHeaders('panda/src/audiotraits')
CopyAllHeaders('panda/src/distort')
CopyAllHeaders('panda/src/downloadertools')
CopyAllHeaders('panda/src/windisplay')
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
CopyAllHeaders('panda/src/glesgsg')
CopyAllHeaders('panda/src/gles2gsg')
CopyAllHeaders('panda/metalibs/pandaegg')
if GetTarget() == 'windows':
    CopyAllHeaders('panda/src/wgldisplay')
elif GetTarget() == 'darwin':
    CopyAllHeaders('panda/src/osxdisplay')
    CopyAllHeaders('panda/src/cocoadisplay')
elif GetTarget() == 'android':
    CopyAllHeaders('panda/src/android')
    CopyAllHeaders('panda/src/androiddisplay')
else:
    CopyAllHeaders('panda/src/x11display')
    CopyAllHeaders('panda/src/glxdisplay')
CopyAllHeaders('panda/src/egldisplay')
CopyAllHeaders('panda/metalibs/pandagl')
CopyAllHeaders('panda/metalibs/pandagles')
CopyAllHeaders('panda/metalibs/pandagles2')

CopyAllHeaders('panda/metalibs/pandaphysics')
CopyAllHeaders('panda/src/testbed')

if (PkgSkip("PHYSX")==0):
    CopyAllHeaders('panda/src/physx')
    CopyAllHeaders('panda/metalibs/pandaphysx')

if (PkgSkip("BULLET")==0):
    CopyAllHeaders('panda/src/bullet')
    CopyAllHeaders('panda/metalibs/pandabullet')

if (PkgSkip("SPEEDTREE")==0):
    CopyAllHeaders('panda/src/speedtree')

if (PkgSkip("DIRECT")==0):
    CopyAllHeaders('direct/src/directbase')
    CopyAllHeaders('direct/src/dcparser')
    CopyAllHeaders('direct/src/deadrec')
    CopyAllHeaders('direct/src/distributed')
    CopyAllHeaders('direct/src/interval')
    CopyAllHeaders('direct/src/showbase')
    CopyAllHeaders('direct/metalibs/direct')
    CopyAllHeaders('direct/src/dcparse')

if (RUNTIME or RTDIST):
    CopyAllHeaders('direct/src/plugin', skip=["p3d_plugin_config.h"])
if (RUNTIME):
    CopyAllHeaders('direct/src/plugin_npapi')
    CopyAllHeaders('direct/src/plugin_standalone')

if (PkgSkip("PANDATOOL")==0):
    CopyAllHeaders('pandatool/src/pandatoolbase')
    CopyAllHeaders('pandatool/src/converter')
    CopyAllHeaders('pandatool/src/progbase')
    CopyAllHeaders('pandatool/src/eggbase')
    CopyAllHeaders('pandatool/src/bam')
    CopyAllHeaders('pandatool/src/cvscopy')
    CopyAllHeaders('pandatool/src/daeegg')
    CopyAllHeaders('pandatool/src/daeprogs')
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
    CopyAllHeaders('pandatool/src/pfmprogs')
    CopyAllHeaders('pandatool/src/lwo')
    CopyAllHeaders('pandatool/src/lwoegg')
    CopyAllHeaders('pandatool/src/lwoprogs')
    CopyAllHeaders('pandatool/src/maya')
    CopyAllHeaders('pandatool/src/mayaegg')
    CopyAllHeaders('pandatool/src/maxegg')
    CopyAllHeaders('pandatool/src/maxprogs')
    CopyAllHeaders('pandatool/src/objegg')
    CopyAllHeaders('pandatool/src/objprogs')
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

if (PkgSkip("CONTRIB")==0):
    CopyAllHeaders('contrib/src/contribbase')
    CopyAllHeaders('contrib/src/ai')

########################################################################
#
# Copy Java files, if applicable
#
########################################################################

if GetTarget() == 'android':
    CopyAllJavaSources('panda/src/android')

########################################################################
#
# These definitions are syntactic shorthand.  They make it easy
# to link with the usual libraries without listing them all.
#
########################################################################

COMMON_DTOOL_LIBS=[
    'libp3dtool.dll',
    'libp3dtoolconfig.dll',
]

COMMON_PANDA_LIBS=[
    'libpanda.dll',
    'libpandaexpress.dll'
] + COMMON_DTOOL_LIBS

COMMON_EGG2X_LIBS=[
    'libp3eggbase.lib',
    'libp3progbase.lib',
    'libp3converter.lib',
    'libp3pandatoolbase.lib',
    'libpandaegg.dll',
] + COMMON_PANDA_LIBS

COMMON_DTOOL_LIBS_PYSTUB = COMMON_DTOOL_LIBS + ['libp3pystub.lib']
COMMON_PANDA_LIBS_PYSTUB = COMMON_PANDA_LIBS + ['libp3pystub.lib']
COMMON_EGG2X_LIBS_PYSTUB = COMMON_EGG2X_LIBS + ['libp3pystub.lib']

########################################################################
#
# This section contains a list of all the files that need to be compiled.
#
########################################################################

print("Generating dependencies...")
sys.stdout.flush()

#
# Compile Panda icon resource file.
# We do it first because we need it at
# the time we compile an executable.
#

if GetTarget() == 'windows':
  OPTS=['DIR:panda/src/configfiles']
  TargetAdd('pandaIcon.res', opts=OPTS, input='pandaIcon.rc')

#
# DIRECTORY: dtool/src/dtoolbase/
#

OPTS=['DIR:dtool/src/dtoolbase', 'BUILDING:DTOOL']
TargetAdd('p3dtoolbase_composite1.obj', opts=OPTS, input='p3dtoolbase_composite1.cxx')
TargetAdd('p3dtoolbase_composite2.obj', opts=OPTS, input='p3dtoolbase_composite2.cxx')
TargetAdd('p3dtoolbase_lookup3.obj',    opts=OPTS, input='lookup3.c')
TargetAdd('p3dtoolbase_indent.obj',     opts=OPTS, input='indent.cxx')

#
# DIRECTORY: dtool/src/dtoolutil/
#

OPTS=['DIR:dtool/src/dtoolutil', 'BUILDING:DTOOL']
TargetAdd('p3dtoolutil_composite1.obj',   opts=OPTS, input='p3dtoolutil_composite1.cxx')
TargetAdd('p3dtoolutil_composite2.obj',   opts=OPTS, input='p3dtoolutil_composite2.cxx')
if GetTarget() == 'darwin':
  TargetAdd('p3dtoolutil_filename_assist.obj',   opts=OPTS, input='filename_assist.mm')

#
# DIRECTORY: dtool/metalibs/dtool/
#

OPTS=['DIR:dtool/metalibs/dtool', 'BUILDING:DTOOL']
TargetAdd('p3dtool_dtool.obj', opts=OPTS, input='dtool.cxx')
TargetAdd('libp3dtool.dll', input='p3dtool_dtool.obj')
TargetAdd('libp3dtool.dll', input='p3dtoolutil_composite1.obj')
TargetAdd('libp3dtool.dll', input='p3dtoolutil_composite2.obj')
if GetTarget() == 'darwin':
  TargetAdd('libp3dtool.dll', input='p3dtoolutil_filename_assist.obj')
TargetAdd('libp3dtool.dll', input='p3dtoolbase_composite1.obj')
TargetAdd('libp3dtool.dll', input='p3dtoolbase_composite2.obj')
TargetAdd('libp3dtool.dll', input='p3dtoolbase_indent.obj')
TargetAdd('libp3dtool.dll', input='p3dtoolbase_lookup3.obj')
TargetAdd('libp3dtool.dll', opts=['ADVAPI','WINSHELL','WINKERNEL'])

#
# DIRECTORY: dtool/src/cppparser/
#

if (not RUNTIME):
  OPTS=['DIR:dtool/src/cppparser', 'BISONPREFIX_cppyy']
  CreateFile(GetOutputDir()+"/include/cppBison.h")
  TargetAdd('p3cppParser_cppBison.obj',  opts=OPTS, input='cppBison.yxx')
  TargetAdd('cppBison.h', input='p3cppParser_cppBison.obj', opts=['DEPENDENCYONLY'])
  TargetAdd('p3cppParser_composite1.obj', opts=OPTS, input='p3cppParser_composite1.cxx')
  TargetAdd('p3cppParser_composite2.obj', opts=OPTS, input='p3cppParser_composite2.cxx')
  TargetAdd('libp3cppParser.ilb', input='p3cppParser_composite1.obj')
  TargetAdd('libp3cppParser.ilb', input='p3cppParser_composite2.obj')
  TargetAdd('libp3cppParser.ilb', input='p3cppParser_cppBison.obj')

#
# DIRECTORY: dtool/src/prc/
#

OPTS=['DIR:dtool/src/prc', 'BUILDING:DTOOLCONFIG', 'OPENSSL']
TargetAdd('p3prc_composite1.obj', opts=OPTS, input='p3prc_composite1.cxx')
TargetAdd('p3prc_composite2.obj', opts=OPTS, input='p3prc_composite2.cxx')

#
# DIRECTORY: dtool/src/dconfig/
#

OPTS=['DIR:dtool/src/dconfig', 'BUILDING:DTOOLCONFIG']
TargetAdd('p3dconfig_composite1.obj', opts=OPTS, input='p3dconfig_composite1.cxx')

#
# DIRECTORY: dtool/src/interrogatedb/
#

OPTS=['DIR:dtool/src/interrogatedb', 'BUILDING:DTOOLCONFIG']
TargetAdd('p3interrogatedb_composite1.obj', opts=OPTS, input='p3interrogatedb_composite1.cxx')
TargetAdd('p3interrogatedb_composite2.obj', opts=OPTS, input='p3interrogatedb_composite2.cxx')

#
# DIRECTORY: dtool/metalibs/dtoolconfig/
#

OPTS=['DIR:dtool/metalibs/dtoolconfig', 'BUILDING:DTOOLCONFIG']
if (PkgSkip("PYTHON")):
  TargetAdd('p3dtoolconfig_pydtool.obj', opts=OPTS, input="null.cxx")
else:
  TargetAdd('p3dtoolconfig_pydtool.obj', opts=OPTS, input="pydtool.cxx")
TargetAdd('p3dtoolconfig_dtoolconfig.obj', opts=OPTS, input='dtoolconfig.cxx')
TargetAdd('p3dtoolconfig_pydtool.obj', dep='dtool_have_python.dat')
TargetAdd('libp3dtoolconfig.dll', input='p3dtoolconfig_dtoolconfig.obj')
TargetAdd('libp3dtoolconfig.dll', input='p3dtoolconfig_pydtool.obj')
TargetAdd('libp3dtoolconfig.dll', input='p3interrogatedb_composite1.obj')
TargetAdd('libp3dtoolconfig.dll', input='p3interrogatedb_composite2.obj')
TargetAdd('libp3dtoolconfig.dll', input='p3dconfig_composite1.obj')
TargetAdd('libp3dtoolconfig.dll', input='p3prc_composite1.obj')
TargetAdd('libp3dtoolconfig.dll', input='p3prc_composite2.obj')
TargetAdd('libp3dtoolconfig.dll', input='libp3dtool.dll')
TargetAdd('libp3dtoolconfig.dll', opts=['ADVAPI', 'OPENSSL', 'WINGDI', 'WINUSER'])

#
# DIRECTORY: dtool/src/pystub/
#

OPTS=['DIR:dtool/src/pystub']
TargetAdd('p3pystub_pystub.obj', opts=OPTS, input='pystub.cxx')
TargetAdd('libp3pystub.lib', input='p3pystub_pystub.obj')
#TargetAdd('libp3pystub.lib', input='libp3dtool.dll')
TargetAdd('libp3pystub.lib', opts=['ADVAPI'])

#
# DIRECTORY: dtool/src/interrogate/
#

if (not RUNTIME):
  OPTS=['DIR:dtool/src/interrogate', 'DIR:dtool/src/cppparser', 'DIR:dtool/src/interrogatedb']
  TargetAdd('interrogate_composite1.obj', opts=OPTS, input='interrogate_composite1.cxx')
  TargetAdd('interrogate_composite2.obj', opts=OPTS, input='interrogate_composite2.cxx')
  TargetAdd('interrogate.exe', input='interrogate_composite1.obj')
  TargetAdd('interrogate.exe', input='interrogate_composite2.obj')
  TargetAdd('interrogate.exe', input='libp3cppParser.ilb')
  TargetAdd('interrogate.exe', input=COMMON_DTOOL_LIBS_PYSTUB)
  TargetAdd('interrogate.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL', 'WINGDI', 'WINUSER'])

  TargetAdd('interrogate_module_interrogate_module.obj', opts=OPTS, input='interrogate_module.cxx')
  TargetAdd('interrogate_module.exe', input='interrogate_module_interrogate_module.obj')
  TargetAdd('interrogate_module.exe', input='libp3cppParser.ilb')
  TargetAdd('interrogate_module.exe', input=COMMON_DTOOL_LIBS_PYSTUB)
  TargetAdd('interrogate_module.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL', 'WINGDI', 'WINUSER'])

  if (not RTDIST):
    TargetAdd('parse_file_parse_file.obj', opts=OPTS, input='parse_file.cxx')
    TargetAdd('parse_file.exe', input='parse_file_parse_file.obj')
    TargetAdd('parse_file.exe', input='libp3cppParser.ilb')
    TargetAdd('parse_file.exe', input=COMMON_DTOOL_LIBS_PYSTUB)
    TargetAdd('parse_file.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL', 'WINGDI', 'WINUSER'])

#
# DIRECTORY: dtool/src/prckeys/
#

if (PkgSkip("OPENSSL")==0 and not RUNTIME and not RTDIST):
  OPTS=['DIR:dtool/src/prckeys', 'OPENSSL']
  TargetAdd('make-prc-key_makePrcKey.obj', opts=OPTS, input='makePrcKey.cxx')
  TargetAdd('make-prc-key.exe', input='make-prc-key_makePrcKey.obj')
  TargetAdd('make-prc-key.exe', input=COMMON_DTOOL_LIBS_PYSTUB)
  TargetAdd('make-prc-key.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL', 'WINGDI', 'WINUSER'])

#
# DIRECTORY: dtool/src/test_interrogate/
#

if (not RTDIST and not RUNTIME):
  OPTS=['DIR:dtool/src/test_interrogate']
  TargetAdd('test_interrogate_test_interrogate.obj', opts=OPTS, input='test_interrogate.cxx')
  TargetAdd('test_interrogate.exe', input='test_interrogate_test_interrogate.obj')
  TargetAdd('test_interrogate.exe', input=COMMON_DTOOL_LIBS_PYSTUB)
  TargetAdd('test_interrogate.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL', 'WINGDI', 'WINUSER'])

#
# DIRECTORY: panda/src/pandabase/
#

OPTS=['DIR:panda/src/pandabase', 'BUILDING:PANDAEXPRESS']
TargetAdd('p3pandabase_pandabase.obj', opts=OPTS, input='pandabase.cxx')

#
# DIRECTORY: panda/src/express/
#

OPTS=['DIR:panda/src/express', 'BUILDING:PANDAEXPRESS', 'OPENSSL', 'ZLIB']
TargetAdd('p3express_composite1.obj', opts=OPTS, input='p3express_composite1.cxx')
TargetAdd('p3express_composite2.obj', opts=OPTS, input='p3express_composite2.cxx')
TargetAdd('p3express_ext_composite.obj', opts=OPTS, input='p3express_ext_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/express', ["*.h", "*_composite*.cxx"])
TargetAdd('libp3express.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libp3express.in', opts=['IMOD:core', 'ILIB:libp3express', 'SRCDIR:panda/src/express'])
TargetAdd('libp3express_igate.obj', input='libp3express.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/downloader/
#

OPTS=['DIR:panda/src/downloader', 'BUILDING:PANDAEXPRESS', 'OPENSSL', 'ZLIB']
TargetAdd('p3downloader_composite1.obj', opts=OPTS, input='p3downloader_composite1.cxx')
TargetAdd('p3downloader_composite2.obj', opts=OPTS, input='p3downloader_composite2.cxx')
IGATEFILES=GetDirectoryContents('panda/src/downloader', ["*.h", "*_composite*.cxx"])
TargetAdd('libp3downloader.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libp3downloader.in', opts=['IMOD:core', 'ILIB:libp3downloader', 'SRCDIR:panda/src/downloader'])
TargetAdd('libp3downloader_igate.obj', input='libp3downloader.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/pandaexpress/
#

OPTS=['DIR:panda/metalibs/pandaexpress', 'BUILDING:PANDAEXPRESS', 'ZLIB']
TargetAdd('pandaexpress_pandaexpress.obj', opts=OPTS, input='pandaexpress.cxx')
TargetAdd('libpandaexpress.dll', input='pandaexpress_pandaexpress.obj')
TargetAdd('libpandaexpress.dll', input='p3downloader_composite1.obj')
TargetAdd('libpandaexpress.dll', input='p3downloader_composite2.obj')
TargetAdd('libpandaexpress.dll', input='libp3downloader_igate.obj')
TargetAdd('libpandaexpress.dll', input='p3express_composite1.obj')
TargetAdd('libpandaexpress.dll', input='p3express_composite2.obj')
TargetAdd('libpandaexpress.dll', input='p3express_ext_composite.obj')
TargetAdd('libpandaexpress.dll', input='libp3express_igate.obj')
TargetAdd('libpandaexpress.dll', input='p3pandabase_pandabase.obj')
TargetAdd('libpandaexpress.dll', input=COMMON_DTOOL_LIBS)
TargetAdd('libpandaexpress.dll', opts=['ADVAPI', 'WINSOCK2',  'OPENSSL', 'ZLIB', 'WINGDI', 'WINUSER'])

#
# DIRECTORY: panda/src/pipeline/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pipeline', 'BUILDING:PANDA']
  TargetAdd('p3pipeline_composite1.obj', opts=OPTS, input='p3pipeline_composite1.cxx')
  TargetAdd('p3pipeline_composite2.obj', opts=OPTS, input='p3pipeline_composite2.cxx')
  TargetAdd('p3pipeline_contextSwitch.obj', opts=OPTS, input='contextSwitch.c')
  IGATEFILES=GetDirectoryContents('panda/src/pipeline', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3pipeline.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pipeline.in', opts=['IMOD:core', 'ILIB:libp3pipeline', 'SRCDIR:panda/src/pipeline'])
  TargetAdd('libp3pipeline_igate.obj', input='libp3pipeline.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/putil/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/putil', 'BUILDING:PANDA',  'ZLIB']
  TargetAdd('p3putil_composite1.obj', opts=OPTS, input='p3putil_composite1.cxx')
  TargetAdd('p3putil_composite2.obj', opts=OPTS, input='p3putil_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/putil', ["*.h", "*_composite*.cxx"])
  IGATEFILES.remove("test_bam.h")
  TargetAdd('libp3putil.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3putil.in', opts=['IMOD:core', 'ILIB:libp3putil', 'SRCDIR:panda/src/putil'])
  TargetAdd('libp3putil_igate.obj', input='libp3putil.in', opts=["DEPENDENCYONLY"])
  TargetAdd('p3putil_typedWritable_ext.obj', opts=OPTS, input='typedWritable_ext.cxx')

#
# DIRECTORY: panda/src/audio/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/audio', 'BUILDING:PANDA']
  TargetAdd('p3audio_composite1.obj', opts=OPTS, input='p3audio_composite1.cxx')
  IGATEFILES=["audio.h"]
  TargetAdd('libp3audio.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3audio.in', opts=['IMOD:core', 'ILIB:libp3audio', 'SRCDIR:panda/src/audio'])
  TargetAdd('libp3audio_igate.obj', input='libp3audio.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/event/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/event', 'BUILDING:PANDA']
  TargetAdd('p3event_composite1.obj', opts=OPTS, input='p3event_composite1.cxx')
  TargetAdd('p3event_composite2.obj', opts=OPTS, input='p3event_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/event', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3event.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3event.in', opts=['IMOD:core', 'ILIB:libp3event', 'SRCDIR:panda/src/event'])
  TargetAdd('libp3event_igate.obj', input='libp3event.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/linmath/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/linmath', 'BUILDING:PANDA']
  TargetAdd('p3linmath_composite1.obj', opts=OPTS, input='p3linmath_composite1.cxx')
  TargetAdd('p3linmath_composite2.obj', opts=OPTS, input='p3linmath_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/linmath', ["*.h", "*_composite*.cxx"])
  for ifile in IGATEFILES[:]:
      if "_src." in ifile:
          IGATEFILES.remove(ifile)
  IGATEFILES.remove('cast_to_double.h')
  IGATEFILES.remove('lmat_ops.h')
  IGATEFILES.remove('cast_to_float.h')
  TargetAdd('libp3linmath.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3linmath.in', opts=['IMOD:core', 'ILIB:libp3linmath', 'SRCDIR:panda/src/linmath'])
  TargetAdd('libp3linmath_igate.obj', input='libp3linmath.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/mathutil/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/mathutil', 'BUILDING:PANDA', 'FFTW']
  TargetAdd('p3mathutil_composite1.obj', opts=OPTS, input='p3mathutil_composite1.cxx')
  TargetAdd('p3mathutil_composite2.obj', opts=OPTS, input='p3mathutil_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/mathutil', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3mathutil.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3mathutil.in', opts=['IMOD:core', 'ILIB:libp3mathutil', 'SRCDIR:panda/src/mathutil'])
  TargetAdd('libp3mathutil_igate.obj', input='libp3mathutil.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/gsgbase/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/gsgbase', 'BUILDING:PANDA']
  TargetAdd('p3gsgbase_composite1.obj', opts=OPTS, input='p3gsgbase_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/gsgbase', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3gsgbase.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3gsgbase.in', opts=['IMOD:core', 'ILIB:libp3gsgbase', 'SRCDIR:panda/src/gsgbase'])
  TargetAdd('libp3gsgbase_igate.obj', input='libp3gsgbase.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pnmimage/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pnmimage', 'BUILDING:PANDA',  'ZLIB']
  TargetAdd('p3pnmimage_composite1.obj', opts=OPTS, input='p3pnmimage_composite1.cxx')
  TargetAdd('p3pnmimage_composite2.obj', opts=OPTS, input='p3pnmimage_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/pnmimage', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3pnmimage.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pnmimage.in', opts=['IMOD:core', 'ILIB:libp3pnmimage', 'SRCDIR:panda/src/pnmimage'])
  TargetAdd('libp3pnmimage_igate.obj', input='libp3pnmimage.in', opts=["DEPENDENCYONLY"])
  TargetAdd('p3pnmimage_pfmFile_ext.obj', opts=OPTS, input='pfmFile_ext.cxx')

#
# DIRECTORY: panda/src/nativenet/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/nativenet', 'OPENSSL', 'BUILDING:PANDA']
  TargetAdd('p3nativenet_composite1.obj', opts=OPTS, input='p3nativenet_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/nativenet', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3nativenet.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3nativenet.in', opts=['IMOD:core', 'ILIB:libp3nativenet', 'SRCDIR:panda/src/nativenet'])
  TargetAdd('libp3nativenet_igate.obj', input='libp3nativenet.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/net/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/net', 'BUILDING:PANDA']
  TargetAdd('p3net_composite1.obj', opts=OPTS, input='p3net_composite1.cxx')
  TargetAdd('p3net_composite2.obj', opts=OPTS, input='p3net_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/net', ["*.h", "*_composite*.cxx"])
  IGATEFILES.remove("datagram_ui.h")
  TargetAdd('libp3net.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3net.in', opts=['IMOD:core', 'ILIB:libp3net', 'SRCDIR:panda/src/net'])
  TargetAdd('libp3net_igate.obj', input='libp3net.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pstatclient/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pstatclient', 'BUILDING:PANDA']
  TargetAdd('p3pstatclient_composite1.obj', opts=OPTS, input='p3pstatclient_composite1.cxx')
  TargetAdd('p3pstatclient_composite2.obj', opts=OPTS, input='p3pstatclient_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/pstatclient', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3pstatclient.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pstatclient.in', opts=['IMOD:core', 'ILIB:libp3pstatclient', 'SRCDIR:panda/src/pstatclient'])
  TargetAdd('libp3pstatclient_igate.obj', input='libp3pstatclient.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/gobj/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/gobj', 'BUILDING:PANDA',  'NVIDIACG', 'ZLIB', 'SQUISH', "BIGOBJ"]
  TargetAdd('p3gobj_composite1.obj', opts=OPTS, input='p3gobj_composite1.cxx')
  TargetAdd('p3gobj_composite2.obj', opts=OPTS, input='p3gobj_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/gobj', ["*.h", "*_composite*.cxx"])
  if ("cgfx_states.h" in IGATEFILES): IGATEFILES.remove("cgfx_states.h")
  TargetAdd('libp3gobj.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3gobj.in', opts=['IMOD:core', 'ILIB:libp3gobj', 'SRCDIR:panda/src/gobj'])
  TargetAdd('libp3gobj_igate.obj', input='libp3gobj.in', opts=["DEPENDENCYONLY"])
  TargetAdd('p3gobj_geomVertexArrayData_ext.obj', opts=OPTS, input='geomVertexArrayData_ext.cxx')

#
# DIRECTORY: panda/src/pgraphnodes/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pgraphnodes', 'BUILDING:PANDA', "BIGOBJ"]
  TargetAdd('p3pgraphnodes_composite1.obj', opts=OPTS, input='p3pgraphnodes_composite1.cxx')
  TargetAdd('p3pgraphnodes_composite2.obj', opts=OPTS, input='p3pgraphnodes_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/pgraphnodes', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3pgraphnodes.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pgraphnodes.in', opts=['IMOD:core', 'ILIB:libp3pgraphnodes', 'SRCDIR:panda/src/pgraphnodes'])
  TargetAdd('libp3pgraphnodes_igate.obj', input='libp3pgraphnodes.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pgraph/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pgraph', 'BUILDING:PANDA','BIGOBJ']
  TargetAdd('p3pgraph_nodePath.obj', opts=OPTS, input='nodePath.cxx')
  TargetAdd('p3pgraph_composite1.obj', opts=OPTS, input='p3pgraph_composite1.cxx')
  TargetAdd('p3pgraph_composite2.obj', opts=OPTS, input='p3pgraph_composite2.cxx')
  TargetAdd('p3pgraph_composite3.obj', opts=OPTS, input='p3pgraph_composite3.cxx')
  TargetAdd('p3pgraph_composite4.obj', opts=OPTS, input='p3pgraph_composite4.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/pgraph', ["*.h", "nodePath.cxx", "*_composite*.cxx"])
  TargetAdd('libp3pgraph.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pgraph.in', opts=['IMOD:core', 'ILIB:libp3pgraph', 'SRCDIR:panda/src/pgraph'])
  TargetAdd('libp3pgraph_igate.obj', input='libp3pgraph.in', opts=["DEPENDENCYONLY","BIGOBJ"])
  TargetAdd('p3pgraph_ext_composite.obj', opts=OPTS, input='p3pgraph_ext_composite.cxx')

#
# DIRECTORY: panda/src/cull/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/cull', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('p3cull_composite1.obj', opts=OPTS, input='p3cull_composite1.cxx')
  TargetAdd('p3cull_composite2.obj', opts=OPTS, input='p3cull_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/cull', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3cull.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3cull.in', opts=['IMOD:core', 'ILIB:libp3cull', 'SRCDIR:panda/src/cull'])
  TargetAdd('libp3cull_igate.obj', input='libp3cull.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/chan/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/chan', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('p3chan_composite1.obj', opts=OPTS, input='p3chan_composite1.cxx')
  TargetAdd('p3chan_composite2.obj', opts=OPTS, input='p3chan_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/chan', ["*.h", "*_composite*.cxx"])
  IGATEFILES.remove('movingPart.h')
  IGATEFILES.remove('animChannelFixed.h')
  TargetAdd('libp3chan.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3chan.in', opts=['IMOD:core', 'ILIB:libp3chan', 'SRCDIR:panda/src/chan'])
  TargetAdd('libp3chan_igate.obj', input='libp3chan.in', opts=["DEPENDENCYONLY"])


# DIRECTORY: panda/src/char/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/char', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('p3char_composite1.obj', opts=OPTS, input='p3char_composite1.cxx')
  TargetAdd('p3char_composite2.obj', opts=OPTS, input='p3char_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/char', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3char.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3char.in', opts=['IMOD:core', 'ILIB:libp3char', 'SRCDIR:panda/src/char'])
  TargetAdd('libp3char_igate.obj', input='libp3char.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/dgraph/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/dgraph', 'BUILDING:PANDA']
  TargetAdd('p3dgraph_composite1.obj', opts=OPTS, input='p3dgraph_composite1.cxx')
  TargetAdd('p3dgraph_composite2.obj', opts=OPTS, input='p3dgraph_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/dgraph', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3dgraph.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3dgraph.in', opts=['IMOD:core', 'ILIB:libp3dgraph', 'SRCDIR:panda/src/dgraph'])
  TargetAdd('libp3dgraph_igate.obj', input='libp3dgraph.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/display/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/display', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('p3display_composite1.obj', opts=OPTS, input='p3display_composite1.cxx')
  TargetAdd('p3display_composite2.obj', opts=OPTS, input='p3display_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/display', ["*.h", "*_composite*.cxx"])
  IGATEFILES.remove("renderBuffer.h")
  TargetAdd('libp3display.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3display.in', opts=['IMOD:core', 'ILIB:libp3display', 'SRCDIR:panda/src/display'])
  TargetAdd('libp3display_igate.obj', input='libp3display.in', opts=["DEPENDENCYONLY"])
  TargetAdd('p3display_graphicsStateGuardian_ext.obj', opts=OPTS, input='graphicsStateGuardian_ext.cxx')

  if RTDIST and GetTarget() == 'darwin':
    OPTS=['DIR:panda/src/display']
    TargetAdd('subprocessWindowBuffer.obj', opts=OPTS, input='subprocessWindowBuffer.cxx')
    TargetAdd('libp3subprocbuffer.ilb', input='subprocessWindowBuffer.obj')

#
# DIRECTORY: panda/src/device/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/device', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('p3device_composite1.obj', opts=OPTS, input='p3device_composite1.cxx')
  TargetAdd('p3device_composite2.obj', opts=OPTS, input='p3device_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/device', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3device.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3device.in', opts=['IMOD:core', 'ILIB:libp3device', 'SRCDIR:panda/src/device'])
  TargetAdd('libp3device_igate.obj', input='libp3device.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pnmtext/
#

if (PkgSkip("FREETYPE")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/pnmtext', 'BUILDING:PANDA',  'FREETYPE']
  TargetAdd('p3pnmtext_composite1.obj', opts=OPTS, input='p3pnmtext_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/pnmtext', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3pnmtext.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pnmtext.in', opts=['IMOD:core', 'ILIB:libp3pnmtext', 'SRCDIR:panda/src/pnmtext'])
  TargetAdd('libp3pnmtext_igate.obj', input='libp3pnmtext.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/text/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/text', 'BUILDING:PANDA', 'ZLIB',  'FREETYPE', 'BIGOBJ']
  TargetAdd('p3text_composite1.obj', opts=OPTS, input='p3text_composite1.cxx')
  TargetAdd('p3text_composite2.obj', opts=OPTS, input='p3text_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/text', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3text.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3text.in', opts=['IMOD:core', 'ILIB:libp3text', 'SRCDIR:panda/src/text'])
  TargetAdd('libp3text_igate.obj', input='libp3text.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/movies/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/movies', 'BUILDING:PANDA', 'VORBIS']
  TargetAdd('p3movies_composite1.obj', opts=OPTS, input='p3movies_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/movies', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3movies.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3movies.in', opts=['IMOD:core', 'ILIB:libp3movies', 'SRCDIR:panda/src/movies'])
  TargetAdd('libp3movies_igate.obj', input='libp3movies.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/grutil/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/grutil', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('p3grutil_multitexReducer.obj', opts=OPTS, input='multitexReducer.cxx')
  TargetAdd('p3grutil_composite1.obj', opts=OPTS, input='p3grutil_composite1.cxx')
  TargetAdd('p3grutil_composite2.obj', opts=OPTS, input='p3grutil_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/grutil', ["*.h", "*_composite*.cxx"])
  if 'convexHull.h' in IGATEFILES: IGATEFILES.remove('convexHull.h')
  TargetAdd('libp3grutil.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3grutil.in', opts=['IMOD:core', 'ILIB:libp3grutil', 'SRCDIR:panda/src/grutil'])
  TargetAdd('libp3grutil_igate.obj', input='libp3grutil.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/tform/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/tform', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('p3tform_composite1.obj', opts=OPTS, input='p3tform_composite1.cxx')
  TargetAdd('p3tform_composite2.obj', opts=OPTS, input='p3tform_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/tform', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3tform.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3tform.in', opts=['IMOD:core', 'ILIB:libp3tform', 'SRCDIR:panda/src/tform'])
  TargetAdd('libp3tform_igate.obj', input='libp3tform.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/collide/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/collide', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('p3collide_composite1.obj', opts=OPTS, input='p3collide_composite1.cxx')
  TargetAdd('p3collide_composite2.obj', opts=OPTS, input='p3collide_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/collide', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3collide.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3collide.in', opts=['IMOD:core', 'ILIB:libp3collide', 'SRCDIR:panda/src/collide'])
  TargetAdd('libp3collide_igate.obj', input='libp3collide.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/parametrics/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/parametrics', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('p3parametrics_composite1.obj', opts=OPTS, input='p3parametrics_composite1.cxx')
  TargetAdd('p3parametrics_composite2.obj', opts=OPTS, input='p3parametrics_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/parametrics', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3parametrics.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3parametrics.in', opts=['IMOD:core', 'ILIB:libp3parametrics', 'SRCDIR:panda/src/parametrics'])
  TargetAdd('libp3parametrics_igate.obj', input='libp3parametrics.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pgui/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pgui', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('p3pgui_composite1.obj', opts=OPTS, input='p3pgui_composite1.cxx')
  TargetAdd('p3pgui_composite2.obj', opts=OPTS, input='p3pgui_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/pgui', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3pgui.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pgui.in', opts=['IMOD:core', 'ILIB:libp3pgui', 'SRCDIR:panda/src/pgui'])
  TargetAdd('libp3pgui_igate.obj', input='libp3pgui.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pnmimagetypes/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pnmimagetypes', 'DIR:panda/src/pnmimage', 'BUILDING:PANDA', 'PNG', 'ZLIB', 'JPEG', 'TIFF']
  TargetAdd('p3pnmimagetypes_composite1.obj', opts=OPTS, input='p3pnmimagetypes_composite1.cxx')
  TargetAdd('p3pnmimagetypes_composite2.obj', opts=OPTS, input='p3pnmimagetypes_composite2.cxx')

#
# DIRECTORY: panda/src/recorder/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/recorder', 'BUILDING:PANDA']
  TargetAdd('p3recorder_composite1.obj', opts=OPTS, input='p3recorder_composite1.cxx')
  TargetAdd('p3recorder_composite2.obj', opts=OPTS, input='p3recorder_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/recorder', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3recorder.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3recorder.in', opts=['IMOD:core', 'ILIB:libp3recorder', 'SRCDIR:panda/src/recorder'])
  TargetAdd('libp3recorder_igate.obj', input='libp3recorder.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/dxml/
#

DefSymbol("TINYXML", "TIXML_USE_STL", "")

if (RUNTIME or RTDIST):
  OPTS=['DIR:panda/src/dxml', 'TINYXML']
  TargetAdd('tinyxml_composite1.obj', opts=OPTS, input='tinyxml_composite1.cxx')
  TargetAdd('libp3tinyxml.ilb', input='tinyxml_composite1.obj')

if (not RUNTIME):
  OPTS=['DIR:panda/src/dxml', 'BUILDING:PANDA', 'TINYXML']
  TargetAdd('p3dxml_composite1.obj', opts=OPTS, input='p3dxml_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/dxml', ["*.h", "p3dxml_composite1.cxx"])
  TargetAdd('libp3dxml.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3dxml.in', opts=['IMOD:core', 'ILIB:libp3dxml', 'SRCDIR:panda/src/dxml'])
  TargetAdd('libp3dxml_igate.obj', input='libp3dxml.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/panda/
#

if (not RUNTIME):
  OPTS=['DIR:panda/metalibs/panda', 'BUILDING:PANDA', 'JPEG', 'PNG',
      'TIFF', 'ZLIB', 'OPENSSL', 'FREETYPE', 'FFTW', 'ADVAPI', 'WINSOCK2',
      'SQUISH', 'NVIDIACG', 'VORBIS', 'WINUSER', 'WINMM', 'WINGDI', 'IPHLPAPI']

  TargetAdd('panda_panda.obj', opts=OPTS, input='panda.cxx')

  TargetAdd('libpanda.dll', input='panda_panda.obj')
  TargetAdd('libpanda.dll', input='p3recorder_composite1.obj')
  TargetAdd('libpanda.dll', input='p3recorder_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3recorder_igate.obj')
  TargetAdd('libpanda.dll', input='p3pgraphnodes_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pgraphnodes_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3pgraphnodes_igate.obj')
  TargetAdd('libpanda.dll', input='p3pgraph_nodePath.obj')
  TargetAdd('libpanda.dll', input='p3pgraph_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pgraph_composite2.obj')
  TargetAdd('libpanda.dll', input='p3pgraph_composite3.obj')
  TargetAdd('libpanda.dll', input='p3pgraph_composite4.obj')
  TargetAdd('libpanda.dll', input='libp3pgraph_igate.obj')
  TargetAdd('libpanda.dll', input='p3cull_composite1.obj')
  TargetAdd('libpanda.dll', input='p3cull_composite2.obj')
  TargetAdd('libpanda.dll', input='p3movies_composite1.obj')
  TargetAdd('libpanda.dll', input='libp3movies_igate.obj')
  TargetAdd('libpanda.dll', input='p3grutil_multitexReducer.obj')
  TargetAdd('libpanda.dll', input='p3grutil_composite1.obj')
  TargetAdd('libpanda.dll', input='p3grutil_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3grutil_igate.obj')
  TargetAdd('libpanda.dll', input='p3chan_composite1.obj')
  TargetAdd('libpanda.dll', input='p3chan_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3chan_igate.obj')
  TargetAdd('libpanda.dll', input='p3pstatclient_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pstatclient_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3pstatclient_igate.obj')
  TargetAdd('libpanda.dll', input='p3char_composite1.obj')
  TargetAdd('libpanda.dll', input='p3char_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3char_igate.obj')
  TargetAdd('libpanda.dll', input='p3collide_composite1.obj')
  TargetAdd('libpanda.dll', input='p3collide_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3collide_igate.obj')
  TargetAdd('libpanda.dll', input='p3device_composite1.obj')
  TargetAdd('libpanda.dll', input='p3device_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3device_igate.obj')
  TargetAdd('libpanda.dll', input='p3dgraph_composite1.obj')
  TargetAdd('libpanda.dll', input='p3dgraph_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3dgraph_igate.obj')
  TargetAdd('libpanda.dll', input='p3display_composite1.obj')
  TargetAdd('libpanda.dll', input='p3display_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3display_igate.obj')
  TargetAdd('libpanda.dll', input='p3pipeline_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pipeline_composite2.obj')
  TargetAdd('libpanda.dll', input='p3pipeline_contextSwitch.obj')
  TargetAdd('libpanda.dll', input='libp3pipeline_igate.obj')
  TargetAdd('libpanda.dll', input='p3event_composite1.obj')
  TargetAdd('libpanda.dll', input='p3event_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3event_igate.obj')
  TargetAdd('libpanda.dll', input='p3gobj_composite1.obj')
  TargetAdd('libpanda.dll', input='p3gobj_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3gobj_igate.obj')
  TargetAdd('libpanda.dll', input='p3gsgbase_composite1.obj')
  TargetAdd('libpanda.dll', input='libp3gsgbase_igate.obj')
  TargetAdd('libpanda.dll', input='p3linmath_composite1.obj')
  TargetAdd('libpanda.dll', input='p3linmath_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3linmath_igate.obj')
  TargetAdd('libpanda.dll', input='p3mathutil_composite1.obj')
  TargetAdd('libpanda.dll', input='p3mathutil_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3mathutil_igate.obj')
  TargetAdd('libpanda.dll', input='p3parametrics_composite1.obj')
  TargetAdd('libpanda.dll', input='p3parametrics_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3parametrics_igate.obj')
  TargetAdd('libpanda.dll', input='p3pnmimagetypes_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pnmimagetypes_composite2.obj')
  TargetAdd('libpanda.dll', input='p3pnmimage_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pnmimage_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3pnmimage_igate.obj')
  TargetAdd('libpanda.dll', input='p3text_composite1.obj')
  TargetAdd('libpanda.dll', input='p3text_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3text_igate.obj')
  TargetAdd('libpanda.dll', input='p3tform_composite1.obj')
  TargetAdd('libpanda.dll', input='p3tform_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3tform_igate.obj')
  TargetAdd('libpanda.dll', input='p3putil_composite1.obj')
  TargetAdd('libpanda.dll', input='p3putil_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3putil_igate.obj')
  TargetAdd('libpanda.dll', input='p3audio_composite1.obj')
  TargetAdd('libpanda.dll', input='libp3audio_igate.obj')
  TargetAdd('libpanda.dll', input='p3pgui_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pgui_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3pgui_igate.obj')
  TargetAdd('libpanda.dll', input='p3net_composite1.obj')
  TargetAdd('libpanda.dll', input='p3net_composite2.obj')
  TargetAdd('libpanda.dll', input='libp3net_igate.obj')
  TargetAdd('libpanda.dll', input='p3nativenet_composite1.obj')
  TargetAdd('libpanda.dll', input='libp3nativenet_igate.obj')
  TargetAdd('libpanda.dll', input='p3pandabase_pandabase.obj')
  TargetAdd('libpanda.dll', input='libpandaexpress.dll')
  TargetAdd('libpanda.dll', input='p3dxml_composite1.obj')
  TargetAdd('libpanda.dll', input='libp3dxml_igate.obj')
  TargetAdd('libpanda.dll', input='libp3dtoolconfig.dll')
  TargetAdd('libpanda.dll', input='libp3dtool.dll')

  TargetAdd('libpanda.dll', input='p3putil_typedWritable_ext.obj')
  TargetAdd('libpanda.dll', input='p3pnmimage_pfmFile_ext.obj')
  TargetAdd('libpanda.dll', input='p3gobj_geomVertexArrayData_ext.obj')
  TargetAdd('libpanda.dll', input='p3pgraph_ext_composite.obj')
  TargetAdd('libpanda.dll', input='p3display_graphicsStateGuardian_ext.obj')

  if PkgSkip("FREETYPE")==0:
    TargetAdd('libpanda.dll', input="p3pnmtext_composite1.obj")
    TargetAdd('libpanda.dll', input="libp3pnmtext_igate.obj")

  TargetAdd('libpanda.dll', dep='dtool_have_freetype.dat')
  TargetAdd('libpanda.dll', opts=OPTS)

  TargetAdd('core_module.obj', input='libp3downloader.in')
  TargetAdd('core_module.obj', input='libp3express.in')

  TargetAdd('core_module.obj', input='libp3recorder.in')
  TargetAdd('core_module.obj', input='libp3pgraphnodes.in')
  TargetAdd('core_module.obj', input='libp3pgraph.in')
  TargetAdd('core_module.obj', input='libp3cull.in')
  TargetAdd('core_module.obj', input='libp3grutil.in')
  TargetAdd('core_module.obj', input='libp3chan.in')
  TargetAdd('core_module.obj', input='libp3pstatclient.in')
  TargetAdd('core_module.obj', input='libp3char.in')
  TargetAdd('core_module.obj', input='libp3collide.in')
  TargetAdd('core_module.obj', input='libp3device.in')
  TargetAdd('core_module.obj', input='libp3dgraph.in')
  TargetAdd('core_module.obj', input='libp3display.in')
  TargetAdd('core_module.obj', input='libp3pipeline.in')
  TargetAdd('core_module.obj', input='libp3event.in')
  TargetAdd('core_module.obj', input='libp3gobj.in')
  TargetAdd('core_module.obj', input='libp3gsgbase.in')
  TargetAdd('core_module.obj', input='libp3linmath.in')
  TargetAdd('core_module.obj', input='libp3mathutil.in')
  TargetAdd('core_module.obj', input='libp3parametrics.in')
  TargetAdd('core_module.obj', input='libp3pnmimage.in')
  TargetAdd('core_module.obj', input='libp3text.in')
  TargetAdd('core_module.obj', input='libp3tform.in')
  TargetAdd('core_module.obj', input='libp3putil.in')
  TargetAdd('core_module.obj', input='libp3audio.in')
  TargetAdd('core_module.obj', input='libp3nativenet.in')
  TargetAdd('core_module.obj', input='libp3net.in')
  TargetAdd('core_module.obj', input='libp3pgui.in')
  TargetAdd('core_module.obj', input='libp3movies.in')
  TargetAdd('core_module.obj', input='libp3dxml.in')

  if PkgSkip("FREETYPE")==0:
    TargetAdd('core_module.obj', input='libp3pnmtext.in')

  TargetAdd('core_module.obj', opts=['IMOD:core', 'ILIB:core'])

  TargetAdd('core.pyd', input='core_module.obj')
  TargetAdd('core.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('core.pyd', opts=['PYTHON'])

#
# DIRECTORY: panda/src/vision/
#

if (PkgSkip("VISION") == 0) and (not RUNTIME):
  OPTS=['DIR:panda/src/vision', 'BUILDING:VISION', 'ARTOOLKIT', 'OPENCV', 'DX9', 'DIRECTCAM', 'JPEG']
  TargetAdd('p3vision_composite1.obj', opts=OPTS, input='p3vision_composite1.cxx')

  TargetAdd('libp3vision.dll', input='p3vision_composite1.obj')
  TargetAdd('libp3vision.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3vision.dll', opts=OPTS)

  OPTS=['DIR:panda/src/vision', 'ARTOOLKIT', 'OPENCV', 'DX9', 'DIRECTCAM', 'JPEG']
  IGATEFILES=GetDirectoryContents('panda/src/vision', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3vision.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3vision.in', opts=['IMOD:vision', 'ILIB:libp3vision', 'SRCDIR:panda/src/vision'])
  TargetAdd('libp3vision_igate.obj', input='libp3vision.in', opts=["DEPENDENCYONLY"])

  TargetAdd('vision_module.obj', input='libp3vision.in')
  TargetAdd('vision_module.obj', opts=OPTS)
  TargetAdd('vision_module.obj', opts=['IMOD:vision', 'ILIB:vision'])

  TargetAdd('vision.pyd', input='vision_module.obj')
  TargetAdd('vision.pyd', input='libp3vision_igate.obj')
  TargetAdd('vision.pyd', input='libp3vision.dll')
  TargetAdd('vision.pyd', input='core.pyd')
  TargetAdd('vision.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('vision.pyd', opts=['PYTHON'])

#
# DIRECTORY: panda/src/rocket/
#

if (PkgSkip("ROCKET") == 0) and (not RUNTIME):
  OPTS=['DIR:panda/src/rocket', 'BUILDING:ROCKET', 'ROCKET']
  TargetAdd('p3rocket_composite1.obj', opts=OPTS, input='p3rocket_composite1.cxx')

  TargetAdd('libp3rocket.dll', input='p3rocket_composite1.obj')
  TargetAdd('libp3rocket.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3rocket.dll', opts=OPTS)

  OPTS=['DIR:panda/src/rocket', 'ROCKET']
  IGATEFILES=GetDirectoryContents('panda/src/rocket', ["rocketInputHandler.h",
    "rocketInputHandler.cxx", "rocketRegion.h", "rocketRegion.cxx", "rocketRegion_ext.h"])
  TargetAdd('libp3rocket.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3rocket.in', opts=['IMOD:rocket', 'ILIB:libp3rocket', 'SRCDIR:panda/src/rocket'])
  TargetAdd('libp3rocket_igate.obj', input='libp3rocket.in', opts=["DEPENDENCYONLY"])
  TargetAdd('p3rocket_rocketRegion_ext.obj', opts=OPTS, input='rocketRegion_ext.cxx')

  TargetAdd('rocket_module.obj', input='libp3rocket.in')
  TargetAdd('rocket_module.obj', opts=OPTS)
  TargetAdd('rocket_module.obj', opts=['IMOD:rocket', 'ILIB:rocket'])

  TargetAdd('rocket.pyd', input='rocket_module.obj')
  TargetAdd('rocket.pyd', input='libp3rocket_igate.obj')
  TargetAdd('rocket.pyd', input='p3rocket_rocketRegion_ext.obj')
  TargetAdd('rocket.pyd', input='libp3rocket.dll')
  TargetAdd('rocket.pyd', input='core.pyd')
  TargetAdd('rocket.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('rocket.pyd', opts=['PYTHON', 'ROCKET'])

#
# DIRECTORY: panda/src/p3awesomium
#
if PkgSkip("AWESOMIUM") == 0 and not RUNTIME:
  OPTS=['DIR:panda/src/awesomium', 'BUILDING:PANDAAWESOMIUM',  'AWESOMIUM']
  TargetAdd('pandaawesomium_composite1.obj', opts=OPTS, input='pandaawesomium_composite1.cxx')
  TargetAdd('libp3awesomium.dll', input='pandaawesomium_composite1.obj')
  TargetAdd('libp3awesomium.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3awesomium.dll', opts=OPTS)

  OPTS=['DIR:panda/src/awesomium', 'AWESOMIUM']
  IGATEFILES=GetDirectoryContents('panda/src/awesomium', ["*.h", "*_composite1.cxx"])
  TargetAdd('libp3awesomium.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3awesomium.in', opts=['IMOD:awesomium', 'ILIB:libp3awesomium', 'SRCDIR:panda/src/awesomium'])
  TargetAdd('libp3awesomium_igate.obj', input='libp3awesomium.in', opts=["DEPENDENCYONLY"])

  TargetAdd('awesomium_module.obj', input='libp3awesomium.in')
  TargetAdd('awesomium_module.obj', opts=OPTS)
  TargetAdd('awesomium_module.obj', opts=['IMOD:awesomium', 'ILIB:awesomium'])

  TargetAdd('awesomium.pyd', input='awesomium_module.obj')
  TargetAdd('awesomium.pyd', input='libp3awesomium_igate.obj')
  TargetAdd('awesomium.pyd', input='libp3awesomium.dll')
  TargetAdd('awesomium.pyd', input='core.pyd')
  TargetAdd('awesomium.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('awesomium.pyd', opts=['PYTHON'])

#
# DIRECTORY: panda/src/p3skel
#

if (PkgSkip('SKEL')==0) and (not RUNTIME):
  OPTS=['DIR:panda/src/skel', 'BUILDING:PANDASKEL', 'ADVAPI']
  TargetAdd('p3skel_composite1.obj', opts=OPTS, input='p3skel_composite1.cxx')

  OPTS=['DIR:panda/src/skel', 'ADVAPI']
  IGATEFILES=GetDirectoryContents("panda/src/skel", ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3skel.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3skel.in', opts=['IMOD:skel', 'ILIB:libp3skel', 'SRCDIR:panda/src/skel'])
  TargetAdd('libp3skel_igate.obj', input='libp3skel.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/p3skel
#

if (PkgSkip('SKEL')==0) and (not RUNTIME):
  OPTS=['BUILDING:PANDASKEL', 'ADVAPI']
  TargetAdd('libpandaskel.dll', input='p3skel_composite1.obj')
  TargetAdd('libpandaskel.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaskel.dll', opts=OPTS)

  TargetAdd('skel_module.obj', input='libp3skel.in')
  TargetAdd('skel_module.obj', opts=['IMOD:skel', 'ILIB:skel'])

  TargetAdd('skel.pyd', input='skel_module.obj')
  TargetAdd('skel.pyd', input='libp3skel_igate.obj')
  TargetAdd('skel.pyd', input='libpandaskel.dll')
  TargetAdd('skel.pyd', input='core.pyd')
  TargetAdd('skel.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('skel.pyd', opts=['PYTHON'])

#
# DIRECTORY: panda/src/distort/
#

if (PkgSkip('PANDAFX')==0) and (not RUNTIME):
  OPTS=['DIR:panda/src/distort', 'BUILDING:PANDAFX']
  TargetAdd('p3distort_composite1.obj', opts=OPTS, input='p3distort_composite1.cxx')

  OPTS=['DIR:panda/metalibs/pandafx', 'DIR:panda/src/distort', 'NVIDIACG']
  IGATEFILES=GetDirectoryContents('panda/src/distort', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3distort.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3distort.in', opts=['IMOD:fx', 'ILIB:libp3distort', 'SRCDIR:panda/src/distort'])
  TargetAdd('libp3distort_igate.obj', input='libp3distort.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/pandafx/
#

if (PkgSkip('PANDAFX')==0) and (not RUNTIME):
  OPTS=['DIR:panda/metalibs/pandafx', 'DIR:panda/src/distort', 'BUILDING:PANDAFX',  'NVIDIACG']
  TargetAdd('pandafx_pandafx.obj', opts=OPTS, input='pandafx.cxx')

  TargetAdd('libpandafx.dll', input='pandafx_pandafx.obj')
  TargetAdd('libpandafx.dll', input='p3distort_composite1.obj')
  TargetAdd('libpandafx.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandafx.dll', opts=['ADVAPI',  'NVIDIACG'])

  OPTS=['DIR:panda/metalibs/pandafx', 'DIR:panda/src/distort', 'NVIDIACG']
  TargetAdd('fx_module.obj', input='libp3distort.in')
  TargetAdd('fx_module.obj', opts=OPTS)
  TargetAdd('fx_module.obj', opts=['IMOD:fx', 'ILIB:fx'])

  TargetAdd('fx.pyd', input='fx_module.obj')
  TargetAdd('fx.pyd', input='libp3distort_igate.obj')
  TargetAdd('fx.pyd', input='libpandafx.dll')
  TargetAdd('fx.pyd', input='core.pyd')
  TargetAdd('fx.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('fx.pyd', opts=['PYTHON'])

#
# DIRECTORY: panda/src/vrpn/
#

if (PkgSkip("VRPN")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/vrpn', 'BUILDING:VRPN', 'VRPN']
  TargetAdd('p3vrpn_composite1.obj', opts=OPTS, input='p3vrpn_composite1.cxx')
  TargetAdd('libp3vrpn.dll', input='p3vrpn_composite1.obj')
  TargetAdd('libp3vrpn.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3vrpn.dll', opts=['VRPN'])

  OPTS=['DIR:panda/src/vrpn', 'VRPN']
  IGATEFILES=GetDirectoryContents('panda/src/vrpn', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3vrpn.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3vrpn.in', opts=['IMOD:vrpn', 'ILIB:libp3vrpn', 'SRCDIR:panda/src/vrpn'])
  TargetAdd('libp3vrpn_igate.obj', input='libp3vrpn.in', opts=["DEPENDENCYONLY"])

  TargetAdd('vrpn_module.obj', input='libp3vrpn.in')
  TargetAdd('vrpn_module.obj', opts=OPTS)
  TargetAdd('vrpn_module.obj', opts=['IMOD:vrpn', 'ILIB:vrpn'])

  TargetAdd('vrpn.pyd', input='vrpn_module.obj')
  TargetAdd('vrpn.pyd', input='libp3vrpn_igate.obj')
  TargetAdd('vrpn.pyd', input='libp3vrpn.dll')
  TargetAdd('vrpn.pyd', input='core.pyd')
  TargetAdd('vrpn.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('vrpn.pyd', opts=['PYTHON'])

#
# DIRECTORY: panda/src/ffmpeg
#
if PkgSkip("FFMPEG") == 0 and not RUNTIME:
  OPTS=['DIR:panda/src/ffmpeg', 'BUILDING:FFMPEG', 'FFMPEG', 'SWSCALE', 'SWRESAMPLE']
  TargetAdd('p3ffmpeg_composite1.obj', opts=OPTS, input='p3ffmpeg_composite1.cxx')
  TargetAdd('libp3ffmpeg.dll', input='p3ffmpeg_composite1.obj')
  TargetAdd('libp3ffmpeg.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3ffmpeg.dll', opts=OPTS)

#
# DIRECTORY: panda/src/audiotraits/
#

if PkgSkip("FMODEX") == 0 and not RUNTIME:
  OPTS=['DIR:panda/src/audiotraits', 'BUILDING:FMOD_AUDIO',  'FMODEX']
  TargetAdd('fmod_audio_fmod_audio_composite1.obj', opts=OPTS, input='fmod_audio_composite1.cxx')
  TargetAdd('libp3fmod_audio.dll', input='fmod_audio_fmod_audio_composite1.obj')
  TargetAdd('libp3fmod_audio.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3fmod_audio.dll', opts=['MODULE', 'ADVAPI', 'WINUSER', 'WINMM', 'FMODEX'])

if PkgSkip("OPENAL") == 0 and not RUNTIME:
  OPTS=['DIR:panda/src/audiotraits', 'BUILDING:OPENAL_AUDIO',  'OPENAL']
  TargetAdd('openal_audio_openal_audio_composite1.obj', opts=OPTS, input='openal_audio_composite1.cxx')
  TargetAdd('libp3openal_audio.dll', input='openal_audio_openal_audio_composite1.obj')
  TargetAdd('libp3openal_audio.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3openal_audio.dll', opts=['MODULE', 'ADVAPI', 'WINUSER', 'WINMM', 'OPENAL'])

#
# DIRECTORY: panda/src/downloadertools/
#

if (PkgSkip("OPENSSL")==0 and not RTDIST and not RUNTIME and PkgSkip("DEPLOYTOOLS")==0):
  OPTS=['DIR:panda/src/downloadertools', 'OPENSSL', 'ZLIB', 'ADVAPI', 'WINSOCK2', 'WINSHELL', 'WINGDI', 'WINUSER']

  TargetAdd('apply_patch_apply_patch.obj', opts=OPTS, input='apply_patch.cxx')
  TargetAdd('apply_patch.exe', input=['apply_patch_apply_patch.obj'])
  TargetAdd('apply_patch.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('apply_patch.exe', opts=OPTS)

  TargetAdd('build_patch_build_patch.obj', opts=OPTS, input='build_patch.cxx')
  TargetAdd('build_patch.exe', input=['build_patch_build_patch.obj'])
  TargetAdd('build_patch.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('build_patch.exe', opts=OPTS)

  TargetAdd('check_adler_check_adler.obj', opts=OPTS, input='check_adler.cxx')
  TargetAdd('check_adler.exe', input=['check_adler_check_adler.obj'])
  TargetAdd('check_adler.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('check_adler.exe', opts=OPTS)

  TargetAdd('check_crc_check_crc.obj', opts=OPTS, input='check_crc.cxx')
  TargetAdd('check_crc.exe', input=['check_crc_check_crc.obj'])
  TargetAdd('check_crc.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('check_crc.exe', opts=OPTS)

  TargetAdd('check_md5_check_md5.obj', opts=OPTS, input='check_md5.cxx')
  TargetAdd('check_md5.exe', input=['check_md5_check_md5.obj'])
  TargetAdd('check_md5.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('check_md5.exe', opts=OPTS)

  TargetAdd('pdecrypt_pdecrypt.obj', opts=OPTS, input='pdecrypt.cxx')
  TargetAdd('pdecrypt.exe', input=['pdecrypt_pdecrypt.obj'])
  TargetAdd('pdecrypt.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('pdecrypt.exe', opts=OPTS)

  TargetAdd('pencrypt_pencrypt.obj', opts=OPTS, input='pencrypt.cxx')
  TargetAdd('pencrypt.exe', input=['pencrypt_pencrypt.obj'])
  TargetAdd('pencrypt.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('pencrypt.exe', opts=OPTS)

  TargetAdd('show_ddb_show_ddb.obj', opts=OPTS, input='show_ddb.cxx')
  TargetAdd('show_ddb.exe', input=['show_ddb_show_ddb.obj'])
  TargetAdd('show_ddb.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('show_ddb.exe', opts=OPTS)

#
# DIRECTORY: panda/src/downloadertools/
#

if (PkgSkip("ZLIB")==0 and not RTDIST and not RUNTIME and PkgSkip("DEPLOYTOOLS")==0):
  OPTS=['DIR:panda/src/downloadertools', 'ZLIB', 'OPENSSL', 'ADVAPI', 'WINSOCK2', 'WINSHELL', 'WINGDI', 'WINUSER']

  TargetAdd('multify_multify.obj', opts=OPTS, input='multify.cxx')
  TargetAdd('multify.exe', input=['multify_multify.obj'])
  TargetAdd('multify.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('multify.exe', opts=OPTS)

  TargetAdd('pzip_pzip.obj', opts=OPTS, input='pzip.cxx')
  TargetAdd('pzip.exe', input=['pzip_pzip.obj'])
  TargetAdd('pzip.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('pzip.exe', opts=OPTS)

  TargetAdd('punzip_punzip.obj', opts=OPTS, input='punzip.cxx')
  TargetAdd('punzip.exe', input=['punzip_punzip.obj'])
  TargetAdd('punzip.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('punzip.exe', opts=OPTS)

#
# DIRECTORY: panda/src/windisplay/
#

if (GetTarget() == 'windows' and not RUNTIME):
  OPTS=['DIR:panda/src/windisplay', 'BUILDING:PANDAWIN']
  TargetAdd('p3windisplay_composite1.obj', opts=OPTS+["BIGOBJ"], input='p3windisplay_composite1.cxx')
  TargetAdd('p3windisplay_windetectdx8.obj', opts=OPTS + ["DX8"], input='winDetectDx8.cxx')
  TargetAdd('p3windisplay_windetectdx9.obj', opts=OPTS + ["DX9"], input='winDetectDx9.cxx')
  TargetAdd('libp3windisplay.dll', input='p3windisplay_composite1.obj')
  TargetAdd('libp3windisplay.dll', input='p3windisplay_windetectdx8.obj')
  TargetAdd('libp3windisplay.dll', input='p3windisplay_windetectdx9.obj')
  TargetAdd('libp3windisplay.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3windisplay.dll', opts=['WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM',"BIGOBJ"])

#
# DIRECTORY: panda/metalibs/pandadx8/
#

if PkgSkip("DX8")==0 and not RUNTIME:
  OPTS=['DIR:panda/src/dxgsg8', 'DIR:panda/metalibs/pandadx8', 'BUILDING:PANDADX', 'DX8']
  TargetAdd('p3dxgsg8_dxGraphicsStateGuardian8.obj', opts=OPTS, input='dxGraphicsStateGuardian8.cxx')
  TargetAdd('p3dxgsg8_composite1.obj', opts=OPTS, input='p3dxgsg8_composite1.cxx')
  TargetAdd('pandadx8_pandadx8.obj', opts=OPTS, input='pandadx8.cxx')
  TargetAdd('libpandadx8.dll', input='pandadx8_pandadx8.obj')
  TargetAdd('libpandadx8.dll', input='p3dxgsg8_dxGraphicsStateGuardian8.obj')
  TargetAdd('libpandadx8.dll', input='p3dxgsg8_composite1.obj')
  TargetAdd('libpandadx8.dll', input='libp3windisplay.dll')
  TargetAdd('libpandadx8.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandadx8.dll', opts=['MODULE', 'ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DX8'])

#
# DIRECTORY: panda/metalibs/pandadx9/
#

if PkgSkip("DX9")==0 and not RUNTIME:
  OPTS=['DIR:panda/src/dxgsg9', 'BUILDING:PANDADX', 'DX9',  'NVIDIACG', 'CGDX9']
  TargetAdd('p3dxgsg9_dxGraphicsStateGuardian9.obj', opts=OPTS, input='dxGraphicsStateGuardian9.cxx')
  TargetAdd('p3dxgsg9_composite1.obj', opts=OPTS, input='p3dxgsg9_composite1.cxx')
  OPTS=['DIR:panda/metalibs/pandadx9', 'BUILDING:PANDADX', 'DX9',  'NVIDIACG', 'CGDX9']
  TargetAdd('pandadx9_pandadx9.obj', opts=OPTS, input='pandadx9.cxx')
  TargetAdd('libpandadx9.dll', input='pandadx9_pandadx9.obj')
  TargetAdd('libpandadx9.dll', input='p3dxgsg9_dxGraphicsStateGuardian9.obj')
  TargetAdd('libpandadx9.dll', input='p3dxgsg9_composite1.obj')
  TargetAdd('libpandadx9.dll', input='libp3windisplay.dll')
  TargetAdd('libpandadx9.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandadx9.dll', opts=['MODULE', 'ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DX9',  'NVIDIACG', 'CGDX9'])

#
# DIRECTORY: panda/src/egg/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/egg', 'BUILDING:PANDAEGG', 'ZLIB', 'BISONPREFIX_eggyy', 'FLEXDASHI']
  CreateFile(GetOutputDir()+"/include/parser.h")
  TargetAdd('p3egg_parser.obj', opts=OPTS, input='parser.yxx')
  TargetAdd('parser.h', input='p3egg_parser.obj', opts=['DEPENDENCYONLY'])
  TargetAdd('p3egg_lexer.obj', opts=OPTS, input='lexer.lxx')
  TargetAdd('p3egg_composite1.obj', opts=OPTS, input='p3egg_composite1.cxx')
  TargetAdd('p3egg_composite2.obj', opts=OPTS, input='p3egg_composite2.cxx')

  OPTS=['DIR:panda/src/egg', 'ZLIB']
  IGATEFILES=GetDirectoryContents('panda/src/egg', ["*.h", "*_composite*.cxx"])
  if "parser.h" in IGATEFILES: IGATEFILES.remove("parser.h")
  TargetAdd('libp3egg.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3egg.in', opts=['IMOD:egg', 'ILIB:libp3egg', 'SRCDIR:panda/src/egg'])
  TargetAdd('libp3egg_igate.obj', input='libp3egg.in', opts=["DEPENDENCYONLY"])
  TargetAdd('p3egg_eggGroupNode_ext.obj', opts=OPTS, input='eggGroupNode_ext.cxx')

#
# DIRECTORY: panda/src/egg2pg/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/egg2pg', 'BUILDING:PANDAEGG']
  TargetAdd('p3egg2pg_composite1.obj', opts=OPTS, input='p3egg2pg_composite1.cxx')
  TargetAdd('p3egg2pg_composite2.obj', opts=OPTS, input='p3egg2pg_composite2.cxx')

  OPTS=['DIR:panda/src/egg2pg']
  IGATEFILES=['load_egg_file.h']
  TargetAdd('libp3egg2pg.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3egg2pg.in', opts=['IMOD:egg', 'ILIB:libp3egg2pg', 'SRCDIR:panda/src/egg2pg'])
  TargetAdd('libp3egg2pg_igate.obj', input='libp3egg2pg.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/framework/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/framework', 'BUILDING:FRAMEWORK']
  TargetAdd('p3framework_composite1.obj', opts=OPTS, input='p3framework_composite1.cxx')
  TargetAdd('libp3framework.dll', input='p3framework_composite1.obj')
  TargetAdd('libp3framework.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3framework.dll', opts=['ADVAPI'])

#
# DIRECTORY: panda/src/glgsg/
#

if (not RUNTIME and PkgSkip("GL")==0):
  OPTS=['DIR:panda/src/glgsg', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGL',  'NVIDIACG']
  TargetAdd('p3glgsg_config_glgsg.obj', opts=OPTS, input='config_glgsg.cxx')
  TargetAdd('p3glgsg_glgsg.obj', opts=OPTS, input='glgsg.cxx')

#
# DIRECTORY: panda/src/glesgsg/
#

if (not RUNTIME and PkgSkip("GLES")==0):
  OPTS=['DIR:panda/src/glesgsg', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLES']
  TargetAdd('p3glesgsg_config_glesgsg.obj', opts=OPTS, input='config_glesgsg.cxx')
  TargetAdd('p3glesgsg_glesgsg.obj', opts=OPTS, input='glesgsg.cxx')

#
# DIRECTORY: panda/src/gles2gsg/
#

if (not RUNTIME and PkgSkip("GLES2")==0):
  OPTS=['DIR:panda/src/gles2gsg', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLES2']
  TargetAdd('p3gles2gsg_config_gles2gsg.obj', opts=OPTS, input='config_gles2gsg.cxx')
  TargetAdd('p3gles2gsg_gles2gsg.obj', opts=OPTS, input='gles2gsg.cxx')

#
# DIRECTORY: panda/metalibs/pandaegg/
#

if (not RUNTIME):
  OPTS=['DIR:panda/metalibs/pandaegg', 'DIR:panda/src/egg', 'BUILDING:PANDAEGG']
  TargetAdd('pandaegg_pandaegg.obj', opts=OPTS, input='pandaegg.cxx')

  TargetAdd('libpandaegg.dll', input='pandaegg_pandaegg.obj')
  TargetAdd('libpandaegg.dll', input='p3egg2pg_composite1.obj')
  TargetAdd('libpandaegg.dll', input='p3egg2pg_composite2.obj')
  TargetAdd('libpandaegg.dll', input='p3egg_composite1.obj')
  TargetAdd('libpandaegg.dll', input='p3egg_composite2.obj')
  TargetAdd('libpandaegg.dll', input='p3egg_parser.obj')
  TargetAdd('libpandaegg.dll', input='p3egg_lexer.obj')
  TargetAdd('libpandaegg.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaegg.dll', opts=['ADVAPI'])

  OPTS=['DIR:panda/metalibs/pandaegg', 'DIR:panda/src/egg']
  TargetAdd('egg_module.obj', input='libp3egg2pg.in')
  TargetAdd('egg_module.obj', input='libp3egg.in')
  TargetAdd('egg_module.obj', opts=OPTS)
  TargetAdd('egg_module.obj', opts=['IMOD:egg', 'ILIB:egg'])

  TargetAdd('egg.pyd', input='egg_module.obj')
  TargetAdd('egg.pyd', input='p3egg_eggGroupNode_ext.obj')
  TargetAdd('egg.pyd', input='libp3egg_igate.obj')
  TargetAdd('egg.pyd', input='libp3egg2pg_igate.obj')
  TargetAdd('egg.pyd', input='libpandaegg.dll')
  TargetAdd('egg.pyd', input='core.pyd')
  TargetAdd('egg.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('egg.pyd', opts=['PYTHON'])

#
# DIRECTORY: panda/src/mesadisplay/
#

if (GetTarget() != 'windows' and PkgSkip("GL")==0 and PkgSkip("OSMESA")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/mesadisplay', 'DIR:panda/src/glstuff', 'BUILDING:PANDAMESA', 'NVIDIACG', 'GL', 'OSMESA']
  TargetAdd('p3mesadisplay_composite1.obj', opts=OPTS, input='p3mesadisplay_composite1.cxx')
  OPTS=['DIR:panda/metalibs/pandagl', 'BUILDING:PANDAMESA', 'NVIDIACG', 'GL']
  TargetAdd('libpandamesa.dll', input='p3mesadisplay_composite1.obj')
  if (PkgSkip('PANDAFX')==0):
    TargetAdd('libpandamesa.dll', input='libpandafx.dll')
  TargetAdd('libpandamesa.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandamesa.dll', opts=['MODULE', 'GL', 'OSMESA'])

#
# DIRECTORY: panda/src/x11display/
#

if (GetTarget() not in ['windows', 'darwin'] and PkgSkip("X11")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/x11display', 'BUILDING:PANDAX11', 'X11']
  TargetAdd('p3x11display_composite1.obj', opts=OPTS, input='p3x11display_composite1.cxx')

#
# DIRECTORY: panda/src/glxdisplay/
#

if (GetTarget() not in ['windows', 'darwin'] and PkgSkip("GL")==0 and PkgSkip("X11")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/glxdisplay', 'BUILDING:PANDAGL',  'GL', 'NVIDIACG', 'CGGL']
  TargetAdd('p3glxdisplay_composite1.obj', opts=OPTS, input='p3glxdisplay_composite1.cxx')
  OPTS=['DIR:panda/metalibs/pandagl', 'BUILDING:PANDAGL',  'GL', 'NVIDIACG', 'CGGL']
  TargetAdd('pandagl_pandagl.obj', opts=OPTS, input='pandagl.cxx')
  TargetAdd('libpandagl.dll', input='p3x11display_composite1.obj')
  TargetAdd('libpandagl.dll', input='pandagl_pandagl.obj')
  TargetAdd('libpandagl.dll', input='p3glgsg_config_glgsg.obj')
  TargetAdd('libpandagl.dll', input='p3glgsg_glgsg.obj')
  TargetAdd('libpandagl.dll', input='p3glxdisplay_composite1.obj')
  TargetAdd('libpandagl.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagl.dll', opts=['MODULE', 'GL', 'NVIDIACG', 'CGGL', 'X11', 'XRANDR', 'XF86DGA', 'XCURSOR'])

#
# DIRECTORY: panda/src/cocoadisplay/
#

if (GetTarget() == 'darwin' and PkgSkip("COCOA")==0 and PkgSkip("GL")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/cocoadisplay', 'BUILDING:PANDAGL', 'GL', 'NVIDIACG', 'CGGL']
  TargetAdd('p3cocoadisplay_composite1.obj', opts=OPTS, input='p3cocoadisplay_composite1.mm')
  OPTS=['DIR:panda/metalibs/pandagl', 'BUILDING:PANDAGL', 'GL', 'NVIDIACG', 'CGGL']
  TargetAdd('pandagl_pandagl.obj', opts=OPTS, input='pandagl.cxx')
  TargetAdd('libpandagl.dll', input='pandagl_pandagl.obj')
  TargetAdd('libpandagl.dll', input='p3glgsg_config_glgsg.obj')
  TargetAdd('libpandagl.dll', input='p3glgsg_glgsg.obj')
  TargetAdd('libpandagl.dll', input='p3cocoadisplay_composite1.obj')
  if (PkgSkip('PANDAFX')==0):
    TargetAdd('libpandagl.dll', input='libpandafx.dll')
  TargetAdd('libpandagl.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagl.dll', opts=['MODULE', 'GL', 'NVIDIACG', 'CGGL', 'COCOA'])

#
# DIRECTORY: panda/src/osxdisplay/
#

elif (GetTarget() == 'darwin' and PkgSkip("CARBON")==0 and PkgSkip("GL")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/osxdisplay', 'BUILDING:PANDAGL',  'GL', 'NVIDIACG', 'CGGL']
  TargetAdd('p3osxdisplay_composite1.obj', opts=OPTS, input='p3osxdisplay_composite1.cxx')
  TargetAdd('p3osxdisplay_osxGraphicsWindow.obj', opts=OPTS, input='osxGraphicsWindow.mm')
  OPTS=['DIR:panda/metalibs/pandagl', 'BUILDING:PANDAGL',  'GL', 'NVIDIACG', 'CGGL']
  TargetAdd('pandagl_pandagl.obj', opts=OPTS, input='pandagl.cxx')
  TargetAdd('libpandagl.dll', input='pandagl_pandagl.obj')
  TargetAdd('libpandagl.dll', input='p3glgsg_config_glgsg.obj')
  TargetAdd('libpandagl.dll', input='p3glgsg_glgsg.obj')
  TargetAdd('libpandagl.dll', input='p3osxdisplay_composite1.obj')
  TargetAdd('libpandagl.dll', input='p3osxdisplay_osxGraphicsWindow.obj')
  if (PkgSkip('PANDAFX')==0):
    TargetAdd('libpandagl.dll', input='libpandafx.dll')
  TargetAdd('libpandagl.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagl.dll', opts=['MODULE', 'GL', 'NVIDIACG', 'CGGL', 'CARBON', 'AGL', 'COCOA'])

#
# DIRECTORY: panda/src/wgldisplay/
#

if (GetTarget() == 'windows' and PkgSkip("GL")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/wgldisplay', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGL',  'NVIDIACG', 'CGGL']
  TargetAdd('p3wgldisplay_composite1.obj', opts=OPTS, input='p3wgldisplay_composite1.cxx')
  OPTS=['DIR:panda/metalibs/pandagl', 'BUILDING:PANDAGL',  'NVIDIACG', 'CGGL']
  TargetAdd('pandagl_pandagl.obj', opts=OPTS, input='pandagl.cxx')
  TargetAdd('libpandagl.dll', input='pandagl_pandagl.obj')
  TargetAdd('libpandagl.dll', input='p3glgsg_config_glgsg.obj')
  TargetAdd('libpandagl.dll', input='p3glgsg_glgsg.obj')
  TargetAdd('libpandagl.dll', input='p3wgldisplay_composite1.obj')
  TargetAdd('libpandagl.dll', input='libp3windisplay.dll')
  if (PkgSkip('PANDAFX')==0):
    TargetAdd('libpandagl.dll', input='libpandafx.dll')
  TargetAdd('libpandagl.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagl.dll', opts=['MODULE', 'WINGDI', 'GL', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM',  'NVIDIACG', 'CGGL'])

#
# DIRECTORY: panda/src/egldisplay/
#

if (PkgSkip("EGL")==0 and PkgSkip("GLES")==0 and PkgSkip("X11")==0 and not RUNTIME):
  DefSymbol('GLES', 'OPENGLES_1', '')
  OPTS=['DIR:panda/src/egldisplay', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLES',  'GLES', 'EGL']
  TargetAdd('pandagles_egldisplay_composite1.obj', opts=OPTS, input='p3egldisplay_composite1.cxx')
  OPTS=['DIR:panda/metalibs/pandagles', 'BUILDING:PANDAGLES', 'GLES', 'EGL']
  TargetAdd('pandagles_pandagles.obj', opts=OPTS, input='pandagles.cxx')
  # Uncomment this as soon as x11-specific stuff is removed from p3egldisplay
  #TargetAdd('libpandagles.dll', input='p3x11display_composite1.obj')
  TargetAdd('libpandagles.dll', input='pandagles_pandagles.obj')
  TargetAdd('libpandagles.dll', input='p3glesgsg_config_glesgsg.obj')
  TargetAdd('libpandagles.dll', input='p3glesgsg_glesgsg.obj')
  TargetAdd('libpandagles.dll', input='pandagles_egldisplay_composite1.obj')
  TargetAdd('libpandagles.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagles.dll', opts=['MODULE', 'GLES', 'EGL', 'X11', 'XRANDR', 'XF86DGA', 'XCURSOR'])

#
# DIRECTORY: panda/src/egldisplay/
#

if (PkgSkip("EGL")==0 and PkgSkip("GLES2")==0 and PkgSkip("X11")==0 and not RUNTIME):
  DefSymbol('GLES2', 'OPENGLES_2', '')
  OPTS=['DIR:panda/src/egldisplay', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLES2',  'GLES2', 'EGL']
  TargetAdd('pandagles2_egldisplay_composite1.obj', opts=OPTS, input='p3egldisplay_composite1.cxx')
  OPTS=['DIR:panda/metalibs/pandagles2', 'BUILDING:PANDAGLES2', 'GLES2', 'EGL']
  TargetAdd('pandagles2_pandagles2.obj', opts=OPTS, input='pandagles2.cxx')
  # Uncomment this as soon as x11-specific stuff is removed from p3egldisplay
  #TargetAdd('libpandagles2.dll', input='p3x11display_composite1.obj')
  TargetAdd('libpandagles2.dll', input='pandagles2_pandagles2.obj')
  TargetAdd('libpandagles2.dll', input='p3gles2gsg_config_gles2gsg.obj')
  TargetAdd('libpandagles2.dll', input='p3gles2gsg_gles2gsg.obj')
  TargetAdd('libpandagles2.dll', input='pandagles2_egldisplay_composite1.obj')
  TargetAdd('libpandagles2.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagles2.dll', opts=['MODULE', 'GLES2', 'EGL', 'X11', 'XRANDR', 'XF86DGA', 'XCURSOR'])

#
# DIRECTORY: panda/src/ode/
#
if (PkgSkip("ODE")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/ode', 'BUILDING:PANDAODE', 'ODE']
  TargetAdd('p3ode_composite1.obj', opts=OPTS, input='p3ode_composite1.cxx')
  TargetAdd('p3ode_composite2.obj', opts=OPTS, input='p3ode_composite2.cxx')
  TargetAdd('p3ode_composite3.obj', opts=OPTS, input='p3ode_composite3.cxx')

  OPTS=['DIR:panda/src/ode', 'ODE']
  IGATEFILES=GetDirectoryContents('panda/src/ode', ["*.h", "*_composite*.cxx"])
  IGATEFILES.remove("odeConvexGeom.h")
  IGATEFILES.remove("odeHeightFieldGeom.h")
  IGATEFILES.remove("odeHelperStructs.h")
  TargetAdd('libpandaode.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpandaode.in', opts=['IMOD:ode', 'ILIB:libpandaode', 'SRCDIR:panda/src/ode'])
  TargetAdd('libpandaode_igate.obj', input='libpandaode.in', opts=["DEPENDENCYONLY"])
  TargetAdd('p3ode_ext_composite.obj', opts=OPTS, input='p3ode_ext_composite.cxx')

#
# DIRECTORY: panda/metalibs/pandaode/
#
if (PkgSkip("ODE")==0 and not RUNTIME):
  OPTS=['DIR:panda/metalibs/pandaode', 'BUILDING:PANDAODE', 'ODE']
  TargetAdd('pandaode_pandaode.obj', opts=OPTS, input='pandaode.cxx')

  TargetAdd('libpandaode.dll', input='pandaode_pandaode.obj')
  TargetAdd('libpandaode.dll', input='p3ode_composite1.obj')
  TargetAdd('libpandaode.dll', input='p3ode_composite2.obj')
  TargetAdd('libpandaode.dll', input='p3ode_composite3.obj')
  TargetAdd('libpandaode.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaode.dll', opts=['WINUSER', 'ODE'])

  OPTS=['DIR:panda/metalibs/pandaode', 'ODE']
  TargetAdd('ode_module.obj', input='libpandaode.in')
  TargetAdd('ode_module.obj', opts=OPTS)
  TargetAdd('ode_module.obj', opts=['IMOD:ode', 'ILIB:ode'])

  TargetAdd('ode.pyd', input='ode_module.obj')
  TargetAdd('ode.pyd', input='libpandaode_igate.obj')
  TargetAdd('ode.pyd', input='p3ode_ext_composite.obj')
  TargetAdd('ode.pyd', input='libpandaode.dll')
  TargetAdd('ode.pyd', input='core.pyd')
  TargetAdd('ode.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('ode.pyd', opts=['PYTHON', 'WINUSER', 'ODE'])

#
# DIRECTORY: panda/src/bullet/
#
if (PkgSkip("BULLET")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/bullet', 'BUILDING:PANDABULLET', 'BULLET']
  TargetAdd('p3bullet_composite.obj', opts=OPTS, input='p3bullet_composite.cxx')

  OPTS=['DIR:panda/src/bullet', 'BULLET']
  IGATEFILES=GetDirectoryContents('panda/src/bullet', ["*.h", "*_composite*.cxx"])
  TargetAdd('libpandabullet.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpandabullet.in', opts=['IMOD:bullet', 'ILIB:libpandabullet', 'SRCDIR:panda/src/bullet'])
  TargetAdd('libpandabullet_igate.obj', input='libpandabullet.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/pandabullet/
#
if (PkgSkip("BULLET")==0 and not RUNTIME):
  OPTS=['DIR:panda/metalibs/pandabullet', 'BUILDING:PANDABULLET', 'BULLET']
  TargetAdd('pandabullet_pandabullet.obj', opts=OPTS, input='pandabullet.cxx')

  TargetAdd('libpandabullet.dll', input='pandabullet_pandabullet.obj')
  TargetAdd('libpandabullet.dll', input='p3bullet_composite.obj')
  TargetAdd('libpandabullet.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandabullet.dll', opts=['WINUSER', 'BULLET'])

  OPTS=['DIR:panda/metalibs/pandabullet', 'BULLET']
  TargetAdd('bullet_module.obj', input='libpandabullet.in')
  TargetAdd('bullet_module.obj', opts=OPTS)
  TargetAdd('bullet_module.obj', opts=['IMOD:bullet', 'ILIB:bullet'])

  TargetAdd('bullet.pyd', input='bullet_module.obj')
  TargetAdd('bullet.pyd', input='libpandabullet_igate.obj')
  TargetAdd('bullet.pyd', input='libpandabullet.dll')
  TargetAdd('bullet.pyd', input='core.pyd')
  TargetAdd('bullet.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('bullet.pyd', opts=['PYTHON', 'WINUSER', 'BULLET'])

#
# DIRECTORY: panda/src/physx/
#

if (PkgSkip("PHYSX")==0):
  OPTS=['DIR:panda/src/physx', 'BUILDING:PANDAPHYSX', 'PHYSX', 'NOPPC']
  TargetAdd('p3physx_composite.obj', opts=OPTS, input='p3physx_composite.cxx')

  OPTS=['DIR:panda/src/physx', 'PHYSX', 'NOPPC']
  IGATEFILES=GetDirectoryContents('panda/src/physx', ["*.h", "*_composite*.cxx"])
  TargetAdd('libpandaphysx.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpandaphysx.in', opts=['IMOD:physx', 'ILIB:libpandaphysx', 'SRCDIR:panda/src/physx'])
  TargetAdd('libpandaphysx_igate.obj', input='libpandaphysx.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/pandaphysx/
#

if (PkgSkip("PHYSX")==0):
  OPTS=['DIR:panda/metalibs/pandaphysx', 'BUILDING:PANDAPHYSX', 'PHYSX', 'NOPPC']
  TargetAdd('pandaphysx_pandaphysx.obj', opts=OPTS, input='pandaphysx.cxx')

  TargetAdd('libpandaphysx.dll', input='pandaphysx_pandaphysx.obj')
  TargetAdd('libpandaphysx.dll', input='p3physx_composite.obj')
  TargetAdd('libpandaphysx.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaphysx.dll', opts=['WINUSER', 'PHYSX', 'NOPPC'])

  OPTS=['DIR:panda/metalibs/pandaphysx', 'PHYSX', 'NOPPC']
  TargetAdd('physx_module.obj', input='libpandaphysx.in')
  TargetAdd('physx_module.obj', opts=OPTS)
  TargetAdd('physx_module.obj', opts=['IMOD:physx', 'ILIB:physx'])

  TargetAdd('physx.pyd', input='physx_module.obj')
  TargetAdd('physx.pyd', input='libpandaphysx_igate.obj')
  TargetAdd('physx.pyd', input='libpandaphysx.dll')
  TargetAdd('physx.pyd', input='core.pyd')
  TargetAdd('physx.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('physx.pyd', opts=['PYTHON', 'WINUSER', 'PHYSX', 'NOPPC'])

#
# DIRECTORY: panda/src/physics/
#

if (PkgSkip("PANDAPHYSICS")==0) and (not RUNTIME):
  OPTS=['DIR:panda/src/physics', 'BUILDING:PANDAPHYSICS']
  TargetAdd('p3physics_composite1.obj', opts=OPTS, input='p3physics_composite1.cxx')
  TargetAdd('p3physics_composite2.obj', opts=OPTS, input='p3physics_composite2.cxx')

  OPTS=['DIR:panda/src/physics']
  IGATEFILES=GetDirectoryContents('panda/src/physics', ["*.h", "*_composite*.cxx"])
  IGATEFILES.remove("forces.h")
  TargetAdd('libp3physics.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3physics.in', opts=['IMOD:physics', 'ILIB:libp3physics', 'SRCDIR:panda/src/physics'])
  TargetAdd('libp3physics_igate.obj', input='libp3physics.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/particlesystem/
#

if (PkgSkip("PANDAPHYSICS")==0) and (PkgSkip("PANDAPARTICLESYSTEM")==0) and (not RUNTIME):
  OPTS=['DIR:panda/src/particlesystem', 'BUILDING:PANDAPHYSICS']
  TargetAdd('p3particlesystem_composite1.obj', opts=OPTS, input='p3particlesystem_composite1.cxx')
  TargetAdd('p3particlesystem_composite2.obj', opts=OPTS, input='p3particlesystem_composite2.cxx')

  OPTS=['DIR:panda/src/particlesystem']
  IGATEFILES=GetDirectoryContents('panda/src/particlesystem', ["*.h", "*_composite*.cxx"])
  IGATEFILES.remove('orientedParticle.h')
  IGATEFILES.remove('orientedParticleFactory.h')
  IGATEFILES.remove('particlefactories.h')
  IGATEFILES.remove('emitters.h')
  IGATEFILES.remove('particles.h')
  TargetAdd('libp3particlesystem.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3particlesystem.in', opts=['IMOD:physics', 'ILIB:libp3particlesystem', 'SRCDIR:panda/src/particlesystem'])
  TargetAdd('libp3particlesystem_igate.obj', input='libp3particlesystem.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/pandaphysics/
#

if (PkgSkip("PANDAPHYSICS")==0) and (not RUNTIME):
  OPTS=['DIR:panda/metalibs/pandaphysics', 'BUILDING:PANDAPHYSICS']
  TargetAdd('pandaphysics_pandaphysics.obj', opts=OPTS, input='pandaphysics.cxx')

  TargetAdd('libpandaphysics.dll', input='pandaphysics_pandaphysics.obj')
  TargetAdd('libpandaphysics.dll', input='p3physics_composite1.obj')
  TargetAdd('libpandaphysics.dll', input='p3physics_composite2.obj')
  TargetAdd('libpandaphysics.dll', input='p3particlesystem_composite1.obj')
  TargetAdd('libpandaphysics.dll', input='p3particlesystem_composite2.obj')
  TargetAdd('libpandaphysics.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaphysics.dll', opts=['ADVAPI'])

  OPTS=['DIR:panda/metalibs/pandaphysics']
  TargetAdd('physics_module.obj', input='libp3physics.in')
  if (PkgSkip("PANDAPARTICLESYSTEM")==0):
    TargetAdd('physics_module.obj', input='libp3particlesystem.in')
  TargetAdd('physics_module.obj', opts=OPTS)
  TargetAdd('physics_module.obj', opts=['IMOD:physics', 'ILIB:physics'])

  TargetAdd('physics.pyd', input='physics_module.obj')
  TargetAdd('physics.pyd', input='libp3physics_igate.obj')
  if (PkgSkip("PANDAPARTICLESYSTEM")==0):
    TargetAdd('physics.pyd', input='libp3particlesystem_igate.obj')
  TargetAdd('physics.pyd', input='libpandaphysics.dll')
  TargetAdd('physics.pyd', input='core.pyd')
  TargetAdd('physics.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('physics.pyd', opts=['PYTHON'])

#
# DIRECTORY: panda/src/speedtree/
#

if (PkgSkip("SPEEDTREE")==0):
  OPTS=['DIR:panda/src/speedtree', 'BUILDING:PANDASPEEDTREE', 'SPEEDTREE']
  TargetAdd('pandaspeedtree_composite1.obj', opts=OPTS, input='pandaspeedtree_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/speedtree', ["*.h", "*_composite*.cxx"])
  TargetAdd('libpandaspeedtree.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpandaspeedtree.in', opts=['IMOD:pandaspeedtree', 'ILIB:libpandaspeedtree', 'SRCDIR:panda/src/speedtree'])
  TargetAdd('libpandaspeedtree_igate.obj', input='libpandaspeedtree.in', opts=["DEPENDENCYONLY"])
  TargetAdd('libpandaspeedtree_module.obj', input='libpandaspeedtree.in')
  TargetAdd('libpandaspeedtree_module.obj', opts=OPTS)
  TargetAdd('libpandaspeedtree_module.obj', opts=['IMOD:pandaspeedtree', 'ILIB:libpandaspeedtree'])
  TargetAdd('libpandaspeedtree.dll', input='pandaspeedtree_composite1.obj')
  TargetAdd('libpandaspeedtree.dll', input='libpandaspeedtree_igate.obj')
  TargetAdd('libpandaspeedtree.dll', input='libpandaspeedtree_module.obj')
  TargetAdd('libpandaspeedtree.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaspeedtree.dll', opts=['SPEEDTREE'])
  if SDK["SPEEDTREEAPI"] == 'OpenGL':
      TargetAdd('libpandaspeedtree.dll', opts=['GL', 'NVIDIACG', 'CGGL'])
  elif SDK["SPEEDTREEAPI"] == 'DirectX9':
      TargetAdd('libpandaspeedtree.dll', opts=['DX9',  'NVIDIACG', 'CGDX9'])

#
# DIRECTORY: panda/src/testbed/
#

if (not RTDIST and not RUNTIME and PkgSkip("PVIEW")==0 and GetTarget() != 'android'):
  OPTS=['DIR:panda/src/testbed']
  TargetAdd('pview_pview.obj', opts=OPTS, input='pview.cxx')
  TargetAdd('pview.exe', input='pview_pview.obj')
  TargetAdd('pview.exe', input='libp3framework.dll')
  TargetAdd('pview.exe', input='libpandaegg.dll')
  TargetAdd('pview.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('pview.exe', opts=['ADVAPI', 'WINSOCK2', 'WINSHELL'])

#
# DIRECTORY: panda/src/android/
#

if (not RUNTIME and GetTarget() == 'android'):
  native_app_glue = os.path.join(SDK['ANDROID_NDK'], 'sources', 'android', 'native_app_glue')
  OPTS=['DIR:panda/src/android', 'DIR:' + native_app_glue]

  TargetAdd('p3android_composite1.obj', opts=OPTS, input='p3android_composite1.cxx')
  TargetAdd('libp3android.dll', input='p3android_composite1.obj')
  TargetAdd('libp3android.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3android.dll', opts=['JNIGRAPHICS'])

  TargetAdd('android_native_app_glue.obj', opts=OPTS, input='android_native_app_glue.c')
  TargetAdd('android_main.obj', opts=OPTS, input='android_main.cxx')

  if (not RTDIST and PkgSkip("PVIEW")==0):
    TargetAdd('pview_pview.obj', opts=OPTS, input='pview.cxx')
    TargetAdd('pview.exe', input='android_native_app_glue.obj')
    TargetAdd('pview.exe', input='android_main.obj')
    TargetAdd('pview.exe', input='pview_pview.obj')
    TargetAdd('pview.exe', input='libp3framework.dll')
    TargetAdd('pview.exe', input='libpandaegg.dll')
    TargetAdd('pview.exe', input='libp3android.dll')
    TargetAdd('pview.exe', input=COMMON_PANDA_LIBS_PYSTUB)
    TargetAdd('AndroidManifest.xml', opts=OPTS, input='pview_manifest.xml')

#
# DIRECTORY: panda/src/androiddisplay/
#

if (GetTarget() == 'android' and PkgSkip("EGL")==0 and PkgSkip("GLES")==0 and not RUNTIME):
  DefSymbol('GLES', 'OPENGLES_1', '')
  OPTS=['DIR:panda/src/androiddisplay', 'DIR:panda/src/glstuff', 'DIR:' + native_app_glue, 'BUILDING:PANDAGLES',  'GLES', 'EGL']
  TargetAdd('pandagles_androiddisplay_composite1.obj', opts=OPTS, input='p3androiddisplay_composite1.cxx')
  OPTS=['DIR:panda/metalibs/pandagles', 'BUILDING:PANDAGLES', 'GLES', 'EGL']
  TargetAdd('pandagles_pandagles.obj', opts=OPTS, input='pandagles.cxx')
  TargetAdd('libpandagles.dll', input='pandagles_pandagles.obj')
  TargetAdd('libpandagles.dll', input='p3glesgsg_config_glesgsg.obj')
  TargetAdd('libpandagles.dll', input='p3glesgsg_glesgsg.obj')
  TargetAdd('libpandagles.dll', input='pandagles_androiddisplay_composite1.obj')
  TargetAdd('libpandagles.dll', input='libp3android.dll')
  TargetAdd('libpandagles.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagles.dll', opts=['MODULE', 'GLES', 'EGL'])

#
# DIRECTORY: panda/src/tinydisplay/
#

if (not RUNTIME and (GetTarget() in ('windows', 'darwin') or PkgSkip("X11")==0) and PkgSkip("TINYDISPLAY")==0):
  OPTS=['DIR:panda/src/tinydisplay', 'BUILDING:TINYDISPLAY']
  TargetAdd('p3tinydisplay_composite1.obj', opts=OPTS, input='p3tinydisplay_composite1.cxx')
  TargetAdd('p3tinydisplay_composite2.obj', opts=OPTS, input='p3tinydisplay_composite2.cxx')
  TargetAdd('p3tinydisplay_ztriangle_1.obj', opts=OPTS, input='ztriangle_1.cxx')
  TargetAdd('p3tinydisplay_ztriangle_2.obj', opts=OPTS, input='ztriangle_2.cxx')
  TargetAdd('p3tinydisplay_ztriangle_3.obj', opts=OPTS, input='ztriangle_3.cxx')
  TargetAdd('p3tinydisplay_ztriangle_4.obj', opts=OPTS, input='ztriangle_4.cxx')
  TargetAdd('p3tinydisplay_ztriangle_table.obj', opts=OPTS, input='ztriangle_table.cxx')
  if GetTarget() == 'darwin':
    TargetAdd('p3tinydisplay_tinyOsxGraphicsWindow.obj', opts=OPTS, input='tinyOsxGraphicsWindow.mm')
    TargetAdd('libp3tinydisplay.dll', input='p3tinydisplay_tinyOsxGraphicsWindow.obj')
    TargetAdd('libp3tinydisplay.dll', opts=['CARBON', 'AGL', 'COCOA'])
  elif GetTarget() == 'windows':
    TargetAdd('libp3tinydisplay.dll', input='libp3windisplay.dll')
    TargetAdd('libp3tinydisplay.dll', opts=['WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM'])
  else:
    TargetAdd('libp3tinydisplay.dll', input='p3x11display_composite1.obj')
    TargetAdd('libp3tinydisplay.dll', opts=['X11', 'XRANDR', 'XF86DGA', 'XCURSOR'])
  TargetAdd('libp3tinydisplay.dll', input='p3tinydisplay_composite1.obj')
  TargetAdd('libp3tinydisplay.dll', input='p3tinydisplay_composite2.obj')
  TargetAdd('libp3tinydisplay.dll', input='p3tinydisplay_ztriangle_1.obj')
  TargetAdd('libp3tinydisplay.dll', input='p3tinydisplay_ztriangle_2.obj')
  TargetAdd('libp3tinydisplay.dll', input='p3tinydisplay_ztriangle_3.obj')
  TargetAdd('libp3tinydisplay.dll', input='p3tinydisplay_ztriangle_4.obj')
  TargetAdd('libp3tinydisplay.dll', input='p3tinydisplay_ztriangle_table.obj')
  TargetAdd('libp3tinydisplay.dll', input=COMMON_PANDA_LIBS)

#
# DIRECTORY: direct/src/directbase/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/directbase', 'PYTHON']
  TargetAdd('p3directbase_directbase.obj', opts=OPTS+['BUILDING:DIRECT'], input='directbase.cxx')

  if (PkgSkip("PYTHON")==0 and not RTDIST and not RUNTIME):
    DefSymbol("BUILDING:PACKPANDA", "IMPORT_MODULE", "direct.directscripts.packpanda")
    TargetAdd('packpanda.obj', opts=OPTS+['BUILDING:PACKPANDA'], input='ppython.cxx')
    TargetAdd('packpanda.exe', input='packpanda.obj')
    TargetAdd('packpanda.exe', opts=['PYTHON'])
  
    DefSymbol("BUILDING:EGGCACHER", "IMPORT_MODULE", "direct.directscripts.eggcacher")
    TargetAdd('eggcacher.obj', opts=OPTS+['BUILDING:EGGCACHER'], input='ppython.cxx')
    TargetAdd('eggcacher.exe', input='eggcacher.obj')
    TargetAdd('eggcacher.exe', opts=['PYTHON'])

#
# DIRECTORY: direct/src/dcparser/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/dcparser', 'WITHINPANDA', 'BUILDING:DIRECT', 'BISONPREFIX_dcyy']
  CreateFile(GetOutputDir()+"/include/dcParser.h")
  TargetAdd('p3dcparser_dcParser.obj', opts=OPTS, input='dcParser.yxx')
  TargetAdd('dcParser.h', input='p3egg_parser.obj', opts=['DEPENDENCYONLY'])
  TargetAdd('p3dcparser_dcLexer.obj', opts=OPTS, input='dcLexer.lxx')
  TargetAdd('p3dcparser_composite1.obj', opts=OPTS, input='p3dcparser_composite1.cxx')
  TargetAdd('p3dcparser_composite2.obj', opts=OPTS, input='p3dcparser_composite2.cxx')
  IGATEFILES=GetDirectoryContents('direct/src/dcparser', ["*.h", "*_composite*.cxx"])
  if "dcParser.h" in IGATEFILES: IGATEFILES.remove("dcParser.h")
  if "dcmsgtypes.h" in IGATEFILES: IGATEFILES.remove('dcmsgtypes.h')
  TargetAdd('libp3dcparser.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3dcparser.in', opts=['IMOD:direct', 'ILIB:libp3dcparser', 'SRCDIR:direct/src/dcparser'])
  TargetAdd('libp3dcparser_igate.obj', input='libp3dcparser.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/src/deadrec/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/deadrec', 'BUILDING:DIRECT']
  TargetAdd('p3deadrec_composite1.obj', opts=OPTS, input='p3deadrec_composite1.cxx')
  IGATEFILES=GetDirectoryContents('direct/src/deadrec', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3deadrec.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3deadrec.in', opts=['IMOD:direct', 'ILIB:libp3deadrec', 'SRCDIR:direct/src/deadrec'])
  TargetAdd('libp3deadrec_igate.obj', input='libp3deadrec.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/src/distributed/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/distributed', 'DIR:direct/src/dcparser', 'WITHINPANDA', 'BUILDING:DIRECT', 'OPENSSL']
  TargetAdd('p3distributed_config_distributed.obj', opts=OPTS, input='config_distributed.cxx')
  TargetAdd('p3distributed_cConnectionRepository.obj', opts=OPTS, input='cConnectionRepository.cxx')
  TargetAdd('p3distributed_cDistributedSmoothNodeBase.obj', opts=OPTS, input='cDistributedSmoothNodeBase.cxx')
  IGATEFILES=GetDirectoryContents('direct/src/distributed', ["*.h", "*.cxx"])
  TargetAdd('libp3distributed.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3distributed.in', opts=['IMOD:direct', 'ILIB:libp3distributed', 'SRCDIR:direct/src/distributed'])
  TargetAdd('libp3distributed_igate.obj', input='libp3distributed.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/src/interval/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/interval', 'BUILDING:DIRECT']
  TargetAdd('p3interval_composite1.obj', opts=OPTS, input='p3interval_composite1.cxx')
  IGATEFILES=GetDirectoryContents('direct/src/interval', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3interval.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3interval.in', opts=['IMOD:direct', 'ILIB:libp3interval', 'SRCDIR:direct/src/interval'])
  TargetAdd('libp3interval_igate.obj', input='libp3interval.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/src/showbase/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/showbase', 'BUILDING:DIRECT']
  TargetAdd('p3showbase_showBase.obj', opts=OPTS, input='showBase.cxx')
  if GetTarget() == 'darwin':
    TargetAdd('p3showbase_showBase_assist.obj', opts=OPTS, input='showBase_assist.mm')
  IGATEFILES=GetDirectoryContents('direct/src/showbase', ["*.h", "showBase.cxx"])
  TargetAdd('libp3showbase.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3showbase.in', opts=['IMOD:direct', 'ILIB:libp3showbase', 'SRCDIR:direct/src/showbase'])
  TargetAdd('libp3showbase_igate.obj', input='libp3showbase.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/metalibs/direct/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/metalibs/direct', 'BUILDING:DIRECT']
  TargetAdd('p3direct_direct.obj', opts=OPTS, input='direct.cxx')

  TargetAdd('libp3direct.dll', input='p3direct_direct.obj')
  TargetAdd('libp3direct.dll', input='p3directbase_directbase.obj')
  TargetAdd('libp3direct.dll', input='p3dcparser_composite1.obj')
  TargetAdd('libp3direct.dll', input='p3dcparser_composite2.obj')
  TargetAdd('libp3direct.dll', input='p3dcparser_dcParser.obj')
  TargetAdd('libp3direct.dll', input='p3dcparser_dcLexer.obj')
  TargetAdd('libp3direct.dll', input='libp3dcparser_igate.obj')
  TargetAdd('libp3direct.dll', input='p3showbase_showBase.obj')
  if GetTarget() == 'darwin':
    TargetAdd('libp3direct.dll', input='p3showbase_showBase_assist.obj')
  TargetAdd('libp3direct.dll', input='libp3showbase_igate.obj')
  TargetAdd('libp3direct.dll', input='p3deadrec_composite1.obj')
  TargetAdd('libp3direct.dll', input='libp3deadrec_igate.obj')
  TargetAdd('libp3direct.dll', input='p3interval_composite1.obj')
  TargetAdd('libp3direct.dll', input='libp3interval_igate.obj')
  TargetAdd('libp3direct.dll', input='p3distributed_config_distributed.obj')
  TargetAdd('libp3direct.dll', input='p3distributed_cConnectionRepository.obj')
  TargetAdd('libp3direct.dll', input='p3distributed_cDistributedSmoothNodeBase.obj')
  TargetAdd('libp3direct.dll', input='libp3distributed_igate.obj')
  TargetAdd('libp3direct.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3direct.dll', opts=['ADVAPI',  'OPENSSL', 'WINUSER', 'WINGDI'])

  OPTS=['DIR:direct/metalibs/direct']
  TargetAdd('direct_module.obj', input='libp3dcparser.in')
  TargetAdd('direct_module.obj', input='libp3showbase.in')
  TargetAdd('direct_module.obj', input='libp3deadrec.in')
  TargetAdd('direct_module.obj', input='libp3interval.in')
  TargetAdd('direct_module.obj', input='libp3distributed.in')
  TargetAdd('direct_module.obj', opts=OPTS)
  TargetAdd('direct_module.obj', opts=['IMOD:direct', 'ILIB:direct'])

  TargetAdd('direct.pyd', input='direct_module.obj')
  TargetAdd('direct.pyd', input='libp3direct.dll')
  TargetAdd('direct.pyd', input='core.pyd')
  TargetAdd('direct.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('direct.pyd', opts=['PYTHON', 'OPENSSL', 'WINUSER', 'WINGDI'])

#
# DIRECTORY: direct/src/dcparse/
#

if (PkgSkip("PYTHON")==0 and PkgSkip("DIRECT")==0 and not RTDIST and not RUNTIME):
  OPTS=['DIR:direct/src/dcparse', 'DIR:direct/src/dcparser', 'WITHINPANDA', 'ADVAPI']
  TargetAdd('dcparse_dcparse.obj', opts=OPTS, input='dcparse.cxx')
  TargetAdd('p3dcparse.exe', input='dcparse_dcparse.obj')
  TargetAdd('p3dcparse.exe', input='libp3direct.dll')
  TargetAdd('p3dcparse.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('p3dcparse.exe', opts=['ADVAPI'])

#
# DIRECTORY: direct/src/plugin/
#

if (RTDIST or RUNTIME):
  # Explicitly define this as we don't include dtool_config.h here.
  if GetTarget() not in ('windows', 'darwin'):
    DefSymbol("RUNTIME", "HAVE_X11", "1")

  OPTS=['DIR:direct/src/plugin', 'BUILDING:P3D_PLUGIN', 'RUNTIME', 'OPENSSL']
  TargetAdd('plugin_common.obj', opts=OPTS, input='plugin_common_composite1.cxx')

  OPTS += ['ZLIB', 'JPEG', 'PNG', 'MSIMG']
  TargetAdd('plugin_plugin.obj', opts=OPTS, input='p3d_plugin_composite1.cxx')
  TargetAdd('plugin_mkdir_complete.obj', opts=OPTS, input='mkdir_complete.cxx')
  TargetAdd('plugin_wstring_encode.obj', opts=OPTS, input='wstring_encode.cxx')
  TargetAdd('plugin_parse_color.obj', opts=OPTS, input='parse_color.cxx')
  TargetAdd('plugin_get_twirl_data.obj', opts=OPTS, input='get_twirl_data.cxx')
  TargetAdd('plugin_find_root_dir.obj', opts=OPTS, input='find_root_dir.cxx')
  if GetTarget() == 'darwin':
    TargetAdd('plugin_find_root_dir_assist.obj', opts=OPTS, input='find_root_dir_assist.mm')
  TargetAdd('plugin_binaryXml.obj', opts=OPTS, input='binaryXml.cxx')
  TargetAdd('plugin_fileSpec.obj', opts=OPTS, input='fileSpec.cxx')
  TargetAdd('plugin_handleStream.obj', opts=OPTS, input='handleStream.cxx')
  TargetAdd('plugin_handleStreamBuf.obj', opts=OPTS, input='handleStreamBuf.cxx')
  if (RTDIST):
    for fname in ("p3d_plugin.dll", "libp3d_plugin_static.ilb"):
      TargetAdd(fname, input='plugin_plugin.obj')
      TargetAdd(fname, input='plugin_mkdir_complete.obj')
      TargetAdd(fname, input='plugin_wstring_encode.obj')
      TargetAdd(fname, input='plugin_parse_color.obj')
      TargetAdd(fname, input='plugin_find_root_dir.obj')
      if GetTarget() == 'darwin':
        TargetAdd(fname, input='plugin_find_root_dir_assist.obj')
      TargetAdd(fname, input='plugin_fileSpec.obj')
      TargetAdd(fname, input='plugin_binaryXml.obj')
      TargetAdd(fname, input='plugin_handleStream.obj')
      TargetAdd(fname, input='plugin_handleStreamBuf.obj')
      TargetAdd(fname, input='libp3tinyxml.ilb')
      if GetTarget() == 'darwin':
        TargetAdd(fname, input='libp3subprocbuffer.ilb')
      TargetAdd(fname, opts=['OPENSSL', 'ZLIB', 'JPEG', 'PNG', 'X11', 'ADVAPI', 'WINUSER', 'WINGDI', 'WINSHELL', 'WINCOMCTL', 'WINOLE', 'MSIMG'])
    TargetAdd("libp3d_plugin_static.ilb", input='plugin_get_twirl_data.obj')

  if (PkgSkip("PYTHON")==0 and RTDIST):
    TargetAdd('p3dpython_p3dpython_composite1.obj', opts=OPTS, input='p3dpython_composite1.cxx')
    TargetAdd('p3dpython_p3dPythonMain.obj', opts=OPTS, input='p3dPythonMain.cxx')
    TargetAdd('p3dpython.exe', input='p3dpython_p3dpython_composite1.obj')
    TargetAdd('p3dpython.exe', input='p3dpython_p3dPythonMain.obj')
    TargetAdd('p3dpython.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('p3dpython.exe', input='libp3tinyxml.ilb')
    TargetAdd('p3dpython.exe', opts=['NOSTRIP', 'PYTHON', 'WINUSER'])

    TargetAdd('libp3dpython.dll', input='p3dpython_p3dpython_composite1.obj')
    TargetAdd('libp3dpython.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libp3dpython.dll', input='libp3tinyxml.ilb')
    TargetAdd('libp3dpython.dll', opts=['PYTHON', 'WINUSER'])

    if GetTarget() == 'windows':
      DefSymbol("NON_CONSOLE", "NON_CONSOLE", "")
      OPTS.append("NON_CONSOLE")
      TargetAdd('p3dpythonw_p3dpython_composite1.obj', opts=OPTS, input='p3dpython_composite1.cxx')
      TargetAdd('p3dpythonw_p3dPythonMain.obj', opts=OPTS, input='p3dPythonMain.cxx')
      TargetAdd('p3dpythonw.exe', input='p3dpythonw_p3dpython_composite1.obj')
      TargetAdd('p3dpythonw.exe', input='p3dpythonw_p3dPythonMain.obj')
      TargetAdd('p3dpythonw.exe', input=COMMON_PANDA_LIBS)
      TargetAdd('p3dpythonw.exe', input='libp3tinyxml.ilb')
      TargetAdd('p3dpythonw.exe', opts=['SUBSYSTEM:WINDOWS', 'PYTHON', 'WINUSER'])

  if (PkgSkip("OPENSSL")==0 and RTDIST):
    OPTS=['DIR:direct/src/plugin', 'DIR:panda/src/express', 'OPENSSL']
    if GetTarget() == 'darwin':
        OPTS += ['OPT:2']
    if (PkgSkip("FLTK")==0):
      OPTS.append("FLTK")
      TargetAdd('plugin_p3dCert.obj', opts=OPTS, input='p3dCert.cxx')
      TargetAdd('p3dcert.exe', input='plugin_mkdir_complete.obj')
      TargetAdd('p3dcert.exe', input='plugin_wstring_encode.obj')
      TargetAdd('p3dcert.exe', input='plugin_p3dCert.obj')
      OPTS=['OPENSSL', 'FLTK', 'WINCOMCTL', 'WINSOCK', 'WINGDI', 'WINUSER', 'ADVAPI', 'WINOLE', 'WINSHELL', 'SUBSYSTEM:WINDOWS']
      if GetTarget() == 'darwin':
          OPTS += ['OPT:2']
      TargetAdd('p3dcert.exe', opts=OPTS)
    elif (PkgSkip("WX")==0):
      OPTS.append("WX")
      TargetAdd('plugin_p3dCert.obj', opts=OPTS, input='p3dCert_wx.cxx')
      TargetAdd('p3dcert.exe', input='plugin_mkdir_complete.obj')
      TargetAdd('p3dcert.exe', input='plugin_wstring_encode.obj')
      TargetAdd('p3dcert.exe', input='plugin_p3dCert.obj')
      OPTS=['NOSTRIP', 'OPENSSL', 'WX', 'CARBON', 'WINOLE', 'WINOLEAUT', 'WINUSER', 'ADVAPI', 'WINSHELL', 'WINCOMCTL', 'WINGDI', 'WINCOMDLG']
      if GetTarget() == "darwin":
          OPTS += ['GL', 'OPT:2']
      TargetAdd('p3dcert.exe', opts=OPTS)

#
# DIRECTORY: direct/src/plugin_npapi/
#

if (RUNTIME and PkgSkip("NPAPI")==0):
  OPTS=['DIR:direct/src/plugin_npapi', 'RUNTIME', 'GTK2']
  if GetTarget() == 'windows':
    nppanda3d_rc = {"name" : "Panda3D Game Engine Plug-in",
                    "version" : VERSION,
                    "description" : "Runs 3-D games and interactive applets",
                    "filename" : "nppanda3d.dll",
                    "mimetype" : "application/x-panda3d",
                    "extension" : "p3d",
                    "filedesc" : "Panda3D applet"}
    TargetAdd('nppanda3d.res', opts=OPTS, winrc=nppanda3d_rc)
  elif GetTarget() == 'darwin':
    TargetAdd('nppanda3d.rsrc', opts=OPTS, input='nppanda3d.r')

  OPTS += ['NPAPI', 'GTK2']
  TargetAdd('plugin_npapi_nppanda3d_composite1.obj', opts=OPTS, input='nppanda3d_composite1.cxx')

  TargetAdd('nppanda3d.plugin', input='plugin_common.obj')
  TargetAdd('nppanda3d.plugin', input='plugin_parse_color.obj')
  TargetAdd('nppanda3d.plugin', input='plugin_get_twirl_data.obj')
  TargetAdd('nppanda3d.plugin', input='plugin_wstring_encode.obj')
  TargetAdd('nppanda3d.plugin', input='plugin_npapi_nppanda3d_composite1.obj')
  if GetTarget() == 'windows':
    TargetAdd('nppanda3d.plugin', input='nppanda3d.res')
    TargetAdd('nppanda3d.plugin', input='nppanda3d.def', ipath=OPTS)
  elif GetTarget() == 'darwin':
    TargetAdd('nppanda3d.plugin', input='nppanda3d.rsrc')
    TargetAdd('nppanda3d.plugin', input='nppanda3d.plist', ipath=OPTS)
    TargetAdd('nppanda3d.plugin', input='plugin_find_root_dir_assist.obj')
  TargetAdd('nppanda3d.plugin', input='libp3tinyxml.ilb')
  TargetAdd('nppanda3d.plugin', opts=['NPAPI', 'OPENSSL', 'WINGDI', 'WINUSER', 'WINSHELL', 'WINOLE', 'CARBON'])

#
# DIRECTORY: direct/src/plugin_activex/
#

if (RUNTIME and GetTarget() == 'windows'):
  OPTS=['DIR:direct/src/plugin_activex', 'RUNTIME', 'ACTIVEX', 'MFC']
  DefSymbol('ACTIVEX', '_USRDLL', '')
  DefSymbol('ACTIVEX', '_WINDLL', '')
  DefSymbol('ACTIVEX', '_AFXDLL', '')
  DefSymbol('ACTIVEX', '_MBCS', '')
  TargetAdd('P3DActiveX.tlb', opts=OPTS, input='P3DActiveX.idl')
  TargetAdd('P3DActiveX.res', opts=OPTS, input='P3DActiveX.rc')

  TargetAdd('plugin_activex_p3dactivex_composite1.obj', opts=OPTS, input='p3dactivex_composite1.cxx')

  TargetAdd('p3dactivex.ocx', input='plugin_common.obj')
  TargetAdd('p3dactivex.ocx', input='plugin_parse_color.obj')
  TargetAdd('p3dactivex.ocx', input='plugin_get_twirl_data.obj')
  TargetAdd('p3dactivex.ocx', input='plugin_wstring_encode.obj')
  TargetAdd('p3dactivex.ocx', input='plugin_activex_p3dactivex_composite1.obj')
  TargetAdd('p3dactivex.ocx', input='P3DActiveX.res')
  TargetAdd('p3dactivex.ocx', input='P3DActiveX.def', ipath=OPTS)
  TargetAdd('p3dactivex.ocx', input='libp3tinyxml.ilb')
  TargetAdd('p3dactivex.ocx', opts=['MFC', 'WINSOCK2', 'OPENSSL', 'WINGDI', 'WINUSER'])

#
# DIRECTORY: direct/src/plugin_standalone/
#

if (RUNTIME):
  OPTS=['DIR:direct/src/plugin_standalone', 'RUNTIME', 'OPENSSL']
  TargetAdd('plugin_standalone_panda3d.obj', opts=OPTS, input='panda3d.cxx')
  TargetAdd('plugin_standalone_panda3dBase.obj', opts=OPTS, input='panda3dBase.cxx')

  if GetTarget() == 'windows':
    panda3d_rc = {"name" : "Panda3D Game Engine Plug-in",
                  "version" : VERSION,
                  "description" : "Runs 3-D games and interactive applets",
                  "filename" : "panda3d.exe",
                  "mimetype" : "application/x-panda3d",
                  "extension" : "p3d",
                  "filedesc" : "Panda3D applet",
                  "icon" : "panda3d.ico"}
    TargetAdd('panda3d.res', opts=OPTS, winrc=panda3d_rc)

  TargetAdd('plugin_standalone_panda3dMain.obj', opts=OPTS, input='panda3dMain.cxx')
  TargetAdd('panda3d.exe', input='plugin_standalone_panda3d.obj')
  TargetAdd('panda3d.exe', input='plugin_standalone_panda3dMain.obj')
  TargetAdd('panda3d.exe', input='plugin_standalone_panda3dBase.obj')
  TargetAdd('panda3d.exe', input='plugin_common.obj')
  TargetAdd('panda3d.exe', input='plugin_wstring_encode.obj')
  if GetTarget() == 'darwin':
    TargetAdd('panda3d.exe', input='plugin_find_root_dir_assist.obj')
  elif GetTarget() == 'windows':
    TargetAdd('panda3d.exe', input='panda3d.res')
  TargetAdd('panda3d.exe', input='libpandaexpress.dll')
  TargetAdd('panda3d.exe', input='libp3dtoolconfig.dll')
  TargetAdd('panda3d.exe', input='libp3dtool.dll')
  TargetAdd('panda3d.exe', input='libp3pystub.lib')
  TargetAdd('panda3d.exe', input='libp3tinyxml.ilb')
  TargetAdd('panda3d.exe', opts=['NOICON', 'OPENSSL', 'ZLIB', 'WINGDI', 'WINUSER', 'WINSHELL', 'ADVAPI', 'WINSOCK2', 'WINOLE', 'CARBON'])

  if (GetTarget() == 'darwin'):
    TargetAdd('plugin_standalone_panda3dMac.obj', opts=OPTS, input='panda3dMac.cxx')
    TargetAdd('Panda3D.app', input='plugin_standalone_panda3d.obj')
    TargetAdd('Panda3D.app', input='plugin_standalone_panda3dMac.obj')
    TargetAdd('Panda3D.app', input='plugin_standalone_panda3dBase.obj')
    TargetAdd('Panda3D.app', input='plugin_common.obj')
    TargetAdd('Panda3D.app', input='plugin_find_root_dir_assist.obj')
    TargetAdd('Panda3D.app', input='libpandaexpress.dll')
    TargetAdd('Panda3D.app', input='libp3dtoolconfig.dll')
    TargetAdd('Panda3D.app', input='libp3dtool.dll')
    TargetAdd('Panda3D.app', input='libp3pystub.lib')
    TargetAdd('Panda3D.app', input='libp3tinyxml.ilb')
    TargetAdd('Panda3D.app', input='panda3d_mac.plist', ipath=OPTS)
    TargetAdd('Panda3D.app', input='models/plugin_images/panda3d.icns')
    TargetAdd('Panda3D.app', opts=['NOSTRIP', 'OPENSSL', 'ZLIB', 'WINGDI', 'WINUSER', 'WINSHELL', 'ADVAPI', 'WINSOCK2', 'WINOLE', 'CARBON'])
  elif (GetTarget() == 'windows'):
    TargetAdd('plugin_standalone_panda3dWinMain.obj', opts=OPTS, input='panda3dWinMain.cxx')
    TargetAdd('panda3dw.exe', input='plugin_standalone_panda3d.obj')
    TargetAdd('panda3dw.exe', input='plugin_standalone_panda3dWinMain.obj')
    TargetAdd('panda3dw.exe', input='plugin_standalone_panda3dBase.obj')
    TargetAdd('panda3dw.exe', input='plugin_wstring_encode.obj')
    TargetAdd('panda3dw.exe', input='plugin_common.obj')
    TargetAdd('panda3dw.exe', input='libpandaexpress.dll')
    TargetAdd('panda3dw.exe', input='libp3dtoolconfig.dll')
    TargetAdd('panda3dw.exe', input='libp3dtool.dll')
    TargetAdd('panda3dw.exe', input='libp3pystub.lib')
    TargetAdd('panda3dw.exe', input='libp3tinyxml.ilb')
    TargetAdd('panda3dw.exe', opts=['OPENSSL', 'ZLIB', 'WINGDI', 'WINUSER', 'WINSHELL', 'ADVAPI', 'WINSOCK2', 'WINOLE', 'CARBON'])

if (RTDIST):
  OPTS=['BUILDING:P3D_PLUGIN', 'DIR:direct/src/plugin_standalone', 'DIR:direct/src/plugin', 'DIR:dtool/src/dtoolbase', 'DIR:dtool/src/dtoolutil', 'DIR:dtool/src/pystub', 'DIR:dtool/src/prc', 'DIR:dtool/src/dconfig', 'DIR:panda/src/express', 'DIR:panda/src/downloader', 'RUNTIME', 'P3DEMBED', 'OPENSSL', 'JPEG', 'PNG', 'ZLIB']
  # This is arguably a big fat ugly hack, but doing it otherwise would complicate the build process considerably.
  DefSymbol("P3DEMBED", "LINK_ALL_STATIC", "")
  TargetAdd('plugin_standalone_panda3dBase.obj', opts=OPTS, input='panda3dBase.cxx')
  TargetAdd('plugin_standalone_p3dEmbedMain.obj', opts=OPTS, input='p3dEmbedMain.cxx')
  TargetAdd('plugin_standalone_p3dEmbed.obj', opts=OPTS, input='p3dEmbed.cxx')
  TargetAdd('plugin_standalone_pystub.obj', opts=OPTS, input='pystub.cxx')
  TargetAdd('plugin_standalone_dtoolbase_composite1.obj', opts=OPTS, input='p3dtoolbase_composite1.cxx')
  TargetAdd('plugin_standalone_dtoolbase_composite2.obj', opts=OPTS, input='p3dtoolbase_composite2.cxx')
  TargetAdd('plugin_standalone_lookup3.obj', opts=OPTS, input='lookup3.c')
  TargetAdd('plugin_standalone_indent.obj', opts=OPTS, input='indent.cxx')
  TargetAdd('plugin_standalone_dtoolutil_composite1.obj', opts=OPTS, input='p3dtoolutil_composite1.cxx')
  TargetAdd('plugin_standalone_dtoolutil_composite2.obj', opts=OPTS, input='p3dtoolutil_composite2.cxx')
  if (GetTarget() == 'darwin'):
      TargetAdd('plugin_standalone_dtoolutil_filename_assist.obj', opts=OPTS, input='filename_assist.mm')
  TargetAdd('plugin_standalone_prc_composite1.obj', opts=OPTS, input='p3prc_composite1.cxx')
  TargetAdd('plugin_standalone_prc_composite2.obj', opts=OPTS, input='p3prc_composite2.cxx')
  TargetAdd('plugin_standalone_dconfig_composite1.obj', opts=OPTS, input='p3dconfig_composite1.cxx')
  TargetAdd('plugin_standalone_express_composite1.obj', opts=OPTS, input='p3express_composite1.cxx')
  TargetAdd('plugin_standalone_express_composite2.obj', opts=OPTS, input='p3express_composite2.cxx')
  TargetAdd('plugin_standalone_downloader_composite1.obj', opts=OPTS, input='p3downloader_composite1.cxx')
  TargetAdd('plugin_standalone_downloader_composite2.obj', opts=OPTS, input='p3downloader_composite2.cxx')
  TargetAdd('p3dembed.exe', input='plugin_standalone_panda3dBase.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_p3dEmbedMain.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_p3dEmbed.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_pystub.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_dtoolbase_composite1.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_dtoolbase_composite2.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_lookup3.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_indent.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_dtoolutil_composite1.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_dtoolutil_composite2.obj')
  if GetTarget() == 'darwin':
      TargetAdd('p3dembed.exe', input='plugin_standalone_dtoolutil_filename_assist.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_prc_composite1.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_prc_composite2.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_dconfig_composite1.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_express_composite1.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_express_composite2.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_downloader_composite1.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_downloader_composite2.obj')
  TargetAdd('p3dembed.exe', input='plugin_common.obj')
  if GetTarget() == 'darwin':
    TargetAdd('p3dembed.exe', input='plugin_find_root_dir_assist.obj')
    TargetAdd('p3dembed.exe', input='libp3subprocbuffer.ilb')
  TargetAdd('p3dembed.exe', input='libp3tinyxml.ilb')
  TargetAdd('p3dembed.exe', input='libp3d_plugin_static.ilb')
  TargetAdd('p3dembed.exe', opts=['NOICON', 'NOSTRIP', 'WINGDI', 'WINSOCK2', 'ZLIB', 'WINUSER', 'OPENSSL', 'JPEG', 'WINOLE', 'CARBON', 'MSIMG', 'WINCOMCTL', 'ADVAPI', 'WINSHELL', 'X11', 'PNG'])

  if GetTarget() == 'windows':
    OPTS.append("P3DEMBEDW")
    DefSymbol("P3DEMBEDW", "P3DEMBEDW", "")
    TargetAdd('plugin_standalone_p3dEmbedWinMain.obj', opts=OPTS, input='p3dEmbedMain.cxx')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_panda3dBase.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_p3dEmbedWinMain.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_p3dEmbed.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_pystub.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_dtoolbase_composite1.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_dtoolbase_composite2.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_lookup3.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_indent.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_dtoolutil_composite1.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_dtoolutil_composite2.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_prc_composite1.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_prc_composite2.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_dconfig_composite1.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_express_composite1.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_express_composite2.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_downloader_composite1.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_downloader_composite2.obj')
    TargetAdd('p3dembedw.exe', input='plugin_common.obj')
    TargetAdd('p3dembedw.exe', input='libp3tinyxml.ilb')
    TargetAdd('p3dembedw.exe', input='libp3d_plugin_static.ilb')
    TargetAdd('p3dembedw.exe', opts=['NOICON', 'NOSTRIP', 'WINGDI', 'WINSOCK2', 'ZLIB', 'WINUSER', 'OPENSSL', 'JPEG', 'WINOLE', 'MSIMG', 'WINCOMCTL', 'ADVAPI', 'WINSHELL', 'PNG'])

#
# DIRECTORY: pandatool/src/pandatoolbase/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/pandatoolbase']
  TargetAdd('p3pandatoolbase_composite1.obj', opts=OPTS, input='p3pandatoolbase_composite1.cxx')
  TargetAdd('libp3pandatoolbase.lib', input='p3pandatoolbase_composite1.obj')

#
# DIRECTORY: pandatool/src/converter/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/converter']
  TargetAdd('p3converter_somethingToEggConverter.obj', opts=OPTS, input='somethingToEggConverter.cxx')
  TargetAdd('p3converter_eggToSomethingConverter.obj', opts=OPTS, input='eggToSomethingConverter.cxx')
  TargetAdd('libp3converter.lib', input='p3converter_somethingToEggConverter.obj')
  TargetAdd('libp3converter.lib', input='p3converter_eggToSomethingConverter.obj')

#
# DIRECTORY: pandatool/src/progbase/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/progbase', 'ZLIB']
  TargetAdd('p3progbase_composite1.obj', opts=OPTS, input='p3progbase_composite1.cxx')
  TargetAdd('libp3progbase.lib', input='p3progbase_composite1.obj')

#
# DIRECTORY: pandatool/src/eggbase/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/eggbase']
  TargetAdd('p3eggbase_composite1.obj', opts=OPTS, input='p3eggbase_composite1.cxx')
  TargetAdd('libp3eggbase.lib', input='p3eggbase_composite1.obj')

#
# DIRECTORY: pandatool/src/bam/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/bam']
  TargetAdd('bam-info_bamInfo.obj', opts=OPTS, input='bamInfo.cxx')
  TargetAdd('bam-info.exe', input='bam-info_bamInfo.obj')
  TargetAdd('bam-info.exe', input='libp3progbase.lib')
  TargetAdd('bam-info.exe', input='libp3pandatoolbase.lib')
  TargetAdd('bam-info.exe', input='libpandaegg.dll')
  TargetAdd('bam-info.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('bam-info.exe', opts=['ADVAPI', 'FFTW'])

  TargetAdd('bam2egg_bamToEgg.obj', opts=OPTS, input='bamToEgg.cxx')
  TargetAdd('bam2egg.exe', input='bam2egg_bamToEgg.obj')
  TargetAdd('bam2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('bam2egg.exe', opts=['ADVAPI',  'FFTW'])

  TargetAdd('egg2bam_eggToBam.obj', opts=OPTS, input='eggToBam.cxx')
  TargetAdd('egg2bam.exe', input='egg2bam_eggToBam.obj')
  TargetAdd('egg2bam.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg2bam.exe', opts=['ADVAPI',  'FFTW'])

#
# DIRECTORY: pandatool/src/cvscopy/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/cvscopy']
  TargetAdd('p3cvscopy_composite1.obj', opts=OPTS, input='p3cvscopy_composite1.cxx')
  TargetAdd('libp3cvscopy.lib', input='p3cvscopy_composite1.obj')

#
# DIRECTORY: pandatool/src/daeegg/
#
if (PkgSkip("PANDATOOL")==0 and PkgSkip("FCOLLADA")==0):
  OPTS=['DIR:pandatool/src/daeegg', 'FCOLLADA']
  TargetAdd('p3daeegg_composite1.obj', opts=OPTS, input='p3daeegg_composite1.cxx')
  TargetAdd('libp3daeegg.lib', input='p3daeegg_composite1.obj')
  TargetAdd('libp3daeegg.lib', opts=['FCOLLADA', 'CARBON'])

#
# DIRECTORY: pandatool/src/daeprogs/
#
if (PkgSkip("PANDATOOL")==0 and PkgSkip("FCOLLADA")==0):
  OPTS=['DIR:pandatool/src/daeprogs', 'FCOLLADA']
  TargetAdd('dae2egg_daeToEgg.obj', opts=OPTS, input='daeToEgg.cxx')
  TargetAdd('dae2egg.exe', input='dae2egg_daeToEgg.obj')
  TargetAdd('dae2egg.exe', input='libp3daeegg.lib')
  TargetAdd('dae2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('dae2egg.exe', opts=['WINUSER', 'FCOLLADA', 'CARBON'])

#
# DIRECTORY: pandatool/src/dxf/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/dxf']
  TargetAdd('p3dxf_composite1.obj', opts=OPTS, input='p3dxf_composite1.cxx')
  TargetAdd('libp3dxf.lib', input='p3dxf_composite1.obj')

#
# DIRECTORY: pandatool/src/dxfegg/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/dxfegg']
  TargetAdd('p3dxfegg_dxfToEggConverter.obj', opts=OPTS, input='dxfToEggConverter.cxx')
  TargetAdd('p3dxfegg_dxfToEggLayer.obj', opts=OPTS, input='dxfToEggLayer.cxx')
  TargetAdd('libp3dxfegg.lib', input='p3dxfegg_dxfToEggConverter.obj')
  TargetAdd('libp3dxfegg.lib', input='p3dxfegg_dxfToEggLayer.obj')

#
# DIRECTORY: pandatool/src/dxfprogs/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/dxfprogs']
  TargetAdd('dxf-points_dxfPoints.obj', opts=OPTS, input='dxfPoints.cxx')
  TargetAdd('dxf-points.exe', input='dxf-points_dxfPoints.obj')
  TargetAdd('dxf-points.exe', input='libp3progbase.lib')
  TargetAdd('dxf-points.exe', input='libp3dxf.lib')
  TargetAdd('dxf-points.exe', input='libp3pandatoolbase.lib')
  TargetAdd('dxf-points.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('dxf-points.exe', opts=['ADVAPI',  'FFTW'])

  TargetAdd('dxf2egg_dxfToEgg.obj', opts=OPTS, input='dxfToEgg.cxx')
  TargetAdd('dxf2egg.exe', input='dxf2egg_dxfToEgg.obj')
  TargetAdd('dxf2egg.exe', input='libp3dxfegg.lib')
  TargetAdd('dxf2egg.exe', input='libp3dxf.lib')
  TargetAdd('dxf2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('dxf2egg.exe', opts=['ADVAPI',  'FFTW'])

  TargetAdd('egg2dxf_eggToDXF.obj', opts=OPTS, input='eggToDXF.cxx')
  TargetAdd('egg2dxf_eggToDXFLayer.obj', opts=OPTS, input='eggToDXFLayer.cxx')
  TargetAdd('egg2dxf.exe', input='egg2dxf_eggToDXF.obj')
  TargetAdd('egg2dxf.exe', input='egg2dxf_eggToDXFLayer.obj')
  TargetAdd('egg2dxf.exe', input='libp3dxf.lib')
  TargetAdd('egg2dxf.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg2dxf.exe', opts=['ADVAPI',  'FFTW'])

#
# DIRECTORY: pandatool/src/objegg/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/objegg']
  TargetAdd('p3objegg_objToEggConverter.obj', opts=OPTS, input='objToEggConverter.cxx')
  TargetAdd('p3objegg_eggToObjConverter.obj', opts=OPTS, input='eggToObjConverter.cxx')
  TargetAdd('p3objegg_config_objegg.obj', opts=OPTS, input='config_objegg.cxx')
  TargetAdd('libp3objegg.lib', input='p3objegg_objToEggConverter.obj')
  TargetAdd('libp3objegg.lib', input='p3objegg_eggToObjConverter.obj')
  TargetAdd('libp3objegg.lib', input='p3objegg_config_objegg.obj')

#
# DIRECTORY: pandatool/src/objprogs/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/objprogs']
  TargetAdd('obj2egg_objToEgg.obj', opts=OPTS, input='objToEgg.cxx')
  TargetAdd('obj2egg.exe', input='obj2egg_objToEgg.obj')
  TargetAdd('obj2egg.exe', input='libp3objegg.lib')
  TargetAdd('obj2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)

  TargetAdd('egg2obj_eggToObj.obj', opts=OPTS, input='eggToObj.cxx')
  TargetAdd('egg2obj.exe', input='egg2obj_eggToObj.obj')
  TargetAdd('egg2obj.exe', input='libp3objegg.lib')
  TargetAdd('egg2obj.exe', input=COMMON_EGG2X_LIBS_PYSTUB)

#
# DIRECTORY: pandatool/src/palettizer/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/palettizer']
  TargetAdd('p3palettizer_composite1.obj', opts=OPTS, input='p3palettizer_composite1.cxx')
  TargetAdd('libp3palettizer.lib', input='p3palettizer_composite1.obj')

#
# DIRECTORY: pandatool/src/egg-mkfont/
#

if (PkgSkip("FREETYPE")==0) and (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/egg-mkfont', 'DIR:pandatool/src/palettizer', 'FREETYPE']
  TargetAdd('egg-mkfont_eggMakeFont.obj', opts=OPTS, input='eggMakeFont.cxx')
  TargetAdd('egg-mkfont_rangeDescription.obj', opts=OPTS, input='rangeDescription.cxx')
  TargetAdd('egg-mkfont_rangeIterator.obj', opts=OPTS, input='rangeIterator.cxx')
  TargetAdd('egg-mkfont.exe', input='egg-mkfont_eggMakeFont.obj')
  TargetAdd('egg-mkfont.exe', input='egg-mkfont_rangeDescription.obj')
  TargetAdd('egg-mkfont.exe', input='egg-mkfont_rangeIterator.obj')
  TargetAdd('egg-mkfont.exe', input='libp3palettizer.lib')
  TargetAdd('egg-mkfont.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-mkfont.exe', opts=['ADVAPI', 'FREETYPE'])

#
# DIRECTORY: pandatool/src/eggcharbase/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/eggcharbase', 'ZLIB']
  TargetAdd('p3eggcharbase_composite1.obj', opts=OPTS, input='p3eggcharbase_composite1.cxx')
  TargetAdd('libp3eggcharbase.lib', input='p3eggcharbase_composite1.obj')

#
# DIRECTORY: pandatool/src/egg-optchar/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/egg-optchar']
  TargetAdd('egg-optchar_config_egg_optchar.obj', opts=OPTS, input='config_egg_optchar.cxx')
  TargetAdd('egg-optchar_eggOptchar.obj', opts=OPTS, input='eggOptchar.cxx')
  TargetAdd('egg-optchar_eggOptcharUserData.obj', opts=OPTS, input='eggOptcharUserData.cxx')
  TargetAdd('egg-optchar_vertexMembership.obj', opts=OPTS, input='vertexMembership.cxx')
  TargetAdd('egg-optchar.exe', input='egg-optchar_config_egg_optchar.obj')
  TargetAdd('egg-optchar.exe', input='egg-optchar_eggOptchar.obj')
  TargetAdd('egg-optchar.exe', input='egg-optchar_eggOptcharUserData.obj')
  TargetAdd('egg-optchar.exe', input='egg-optchar_vertexMembership.obj')
  TargetAdd('egg-optchar.exe', input='libp3eggcharbase.lib')
  TargetAdd('egg-optchar.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-optchar.exe', opts=['ADVAPI', 'FREETYPE'])

#
# DIRECTORY: pandatool/src/egg-palettize/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/egg-palettize', 'DIR:pandatool/src/palettizer']
  TargetAdd('egg-palettize_eggPalettize.obj', opts=OPTS, input='eggPalettize.cxx')
  TargetAdd('egg-palettize.exe', input='egg-palettize_eggPalettize.obj')
  TargetAdd('egg-palettize.exe', input='libp3palettizer.lib')
  TargetAdd('egg-palettize.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-palettize.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/egg-qtess/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/egg-qtess']
  TargetAdd('egg-qtess_composite1.obj', opts=OPTS, input='egg-qtess_composite1.cxx')
  TargetAdd('egg-qtess.exe', input='egg-qtess_composite1.obj')
  TargetAdd('egg-qtess.exe', input='libp3eggbase.lib')
  TargetAdd('egg-qtess.exe', input='libp3progbase.lib')
  TargetAdd('egg-qtess.exe', input='libp3converter.lib')
  TargetAdd('egg-qtess.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-qtess.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/eggprogs/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/eggprogs']
  TargetAdd('egg-crop_eggCrop.obj', opts=OPTS, input='eggCrop.cxx')
  TargetAdd('egg-crop.exe', input='egg-crop_eggCrop.obj')
  TargetAdd('egg-crop.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-crop.exe', opts=['ADVAPI'])

  TargetAdd('egg-make-tube_eggMakeTube.obj', opts=OPTS, input='eggMakeTube.cxx')
  TargetAdd('egg-make-tube.exe', input='egg-make-tube_eggMakeTube.obj')
  TargetAdd('egg-make-tube.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-make-tube.exe', opts=['ADVAPI'])

  TargetAdd('egg-texture-cards_eggTextureCards.obj', opts=OPTS, input='eggTextureCards.cxx')
  TargetAdd('egg-texture-cards.exe', input='egg-texture-cards_eggTextureCards.obj')
  TargetAdd('egg-texture-cards.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-texture-cards.exe', opts=['ADVAPI'])

  TargetAdd('egg-topstrip_eggTopstrip.obj', opts=OPTS, input='eggTopstrip.cxx')
  TargetAdd('egg-topstrip.exe', input='egg-topstrip_eggTopstrip.obj')
  TargetAdd('egg-topstrip.exe', input='libp3eggcharbase.lib')
  TargetAdd('egg-topstrip.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-topstrip.exe', opts=['ADVAPI'])

  TargetAdd('egg-trans_eggTrans.obj', opts=OPTS, input='eggTrans.cxx')
  TargetAdd('egg-trans.exe', input='egg-trans_eggTrans.obj')
  TargetAdd('egg-trans.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-trans.exe', opts=['ADVAPI'])

  TargetAdd('egg2c_eggToC.obj', opts=OPTS, input='eggToC.cxx')
  TargetAdd('egg2c.exe', input='egg2c_eggToC.obj')
  TargetAdd('egg2c.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg2c.exe', opts=['ADVAPI'])

  TargetAdd('egg-rename_eggRename.obj', opts=OPTS, input='eggRename.cxx')
  TargetAdd('egg-rename.exe', input='egg-rename_eggRename.obj')
  TargetAdd('egg-rename.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-rename.exe', opts=['ADVAPI'])

  TargetAdd('egg-retarget-anim_eggRetargetAnim.obj', opts=OPTS, input='eggRetargetAnim.cxx')
  TargetAdd('egg-retarget-anim.exe', input='egg-retarget-anim_eggRetargetAnim.obj')
  TargetAdd('egg-retarget-anim.exe', input='libp3eggcharbase.lib')
  TargetAdd('egg-retarget-anim.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-retarget-anim.exe', opts=['ADVAPI'])

  TargetAdd('egg-list-textures_eggListTextures.obj', opts=OPTS, input='eggListTextures.cxx')
  TargetAdd('egg-list-textures.exe', input='egg-list-textures_eggListTextures.obj')
  TargetAdd('egg-list-textures.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-list-textures.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/flt/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/flt', 'ZLIB']
  TargetAdd('p3flt_fltVectorRecord.obj', opts=OPTS, input='fltVectorRecord.cxx')
  TargetAdd('p3flt_composite1.obj', opts=OPTS, input='p3flt_composite1.cxx')
  TargetAdd('libp3flt.lib', input=['p3flt_fltVectorRecord.obj', 'p3flt_composite1.obj'])

#
# DIRECTORY: pandatool/src/fltegg/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/fltegg']
  TargetAdd('p3fltegg_fltToEggConverter.obj', opts=OPTS, input='fltToEggConverter.cxx')
  TargetAdd('p3fltegg_fltToEggLevelState.obj', opts=OPTS, input='fltToEggLevelState.cxx')
  TargetAdd('libp3fltegg.lib', input=['p3fltegg_fltToEggConverter.obj', 'p3fltegg_fltToEggLevelState.obj'])

#
# DIRECTORY: pandatool/src/fltprogs/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/fltprogs', 'DIR:pandatool/src/flt', 'DIR:pandatool/src/cvscopy']
  TargetAdd('egg2flt_eggToFlt.obj', opts=OPTS, input='eggToFlt.cxx')
  TargetAdd('egg2flt.exe', input='egg2flt_eggToFlt.obj')
  TargetAdd('egg2flt.exe', input='libp3flt.lib')
  TargetAdd('egg2flt.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg2flt.exe', opts=['ADVAPI'])

  TargetAdd('flt-info_fltInfo.obj', opts=OPTS, input='fltInfo.cxx')
  TargetAdd('flt-info.exe', input='flt-info_fltInfo.obj')
  TargetAdd('flt-info.exe', input='libp3flt.lib')
  TargetAdd('flt-info.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('flt-info.exe', opts=['ADVAPI'])

  TargetAdd('flt-trans_fltTrans.obj', opts=OPTS, input='fltTrans.cxx')
  TargetAdd('flt-trans.exe', input='flt-trans_fltTrans.obj')
  TargetAdd('flt-trans.exe', input='libp3flt.lib')
  TargetAdd('flt-trans.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('flt-trans.exe', opts=['ADVAPI'])

  TargetAdd('flt2egg_fltToEgg.obj', opts=OPTS, input='fltToEgg.cxx')
  TargetAdd('flt2egg.exe', input='flt2egg_fltToEgg.obj')
  TargetAdd('flt2egg.exe', input='libp3flt.lib')
  TargetAdd('flt2egg.exe', input='libp3fltegg.lib')
  TargetAdd('flt2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('flt2egg.exe', opts=['ADVAPI'])

  TargetAdd('fltcopy_fltCopy.obj', opts=OPTS, input='fltCopy.cxx')
  TargetAdd('fltcopy.exe', input='fltcopy_fltCopy.obj')
  TargetAdd('fltcopy.exe', input='libp3cvscopy.lib')
  TargetAdd('fltcopy.exe', input='libp3flt.lib')
  TargetAdd('fltcopy.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('fltcopy.exe', opts=['ADVAPI'])


#
# DIRECTORY: pandatool/src/imagebase/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/imagebase']
  TargetAdd('p3imagebase_composite1.obj', opts=OPTS, input='p3imagebase_composite1.cxx')
  TargetAdd('libp3imagebase.lib', input='p3imagebase_composite1.obj')

#
# DIRECTORY: pandatool/src/imageprogs/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/imageprogs']
  TargetAdd('image-info_imageInfo.obj', opts=OPTS, input='imageInfo.cxx')
  TargetAdd('image-info.exe', input='image-info_imageInfo.obj')
  TargetAdd('image-info.exe', input='libp3imagebase.lib')
  TargetAdd('image-info.exe', input='libp3progbase.lib')
  TargetAdd('image-info.exe', input='libp3pandatoolbase.lib')
  TargetAdd('image-info.exe', input='libpandaegg.dll')
  TargetAdd('image-info.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('image-info.exe', input='libp3pystub.lib')
  TargetAdd('image-info.exe', opts=['ADVAPI'])

  TargetAdd('image-resize_imageResize.obj', opts=OPTS, input='imageResize.cxx')
  TargetAdd('image-resize.exe', input='image-resize_imageResize.obj')
  TargetAdd('image-resize.exe', input='libp3imagebase.lib')
  TargetAdd('image-resize.exe', input='libp3progbase.lib')
  TargetAdd('image-resize.exe', input='libp3pandatoolbase.lib')
  TargetAdd('image-resize.exe', input='libpandaegg.dll')
  TargetAdd('image-resize.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('image-resize.exe', input='libp3pystub.lib')
  TargetAdd('image-resize.exe', opts=['ADVAPI'])

  TargetAdd('image-trans_imageTrans.obj', opts=OPTS, input='imageTrans.cxx')
  TargetAdd('image-trans.exe', input='image-trans_imageTrans.obj')
  TargetAdd('image-trans.exe', input='libp3imagebase.lib')
  TargetAdd('image-trans.exe', input='libp3progbase.lib')
  TargetAdd('image-trans.exe', input='libp3pandatoolbase.lib')
  TargetAdd('image-trans.exe', input='libpandaegg.dll')
  TargetAdd('image-trans.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('image-trans.exe', input='libp3pystub.lib')
  TargetAdd('image-trans.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/pfmprogs/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/pfmprogs']
  TargetAdd('pfm-trans_pfmTrans.obj', opts=OPTS, input='pfmTrans.cxx')
  TargetAdd('pfm-trans.exe', input='pfm-trans_pfmTrans.obj')
  TargetAdd('pfm-trans.exe', input='libp3progbase.lib')
  TargetAdd('pfm-trans.exe', input='libp3pandatoolbase.lib')
  TargetAdd('pfm-trans.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('pfm-trans.exe', input='libp3pystub.lib')
  TargetAdd('pfm-trans.exe', opts=['ADVAPI'])

  TargetAdd('pfm-bba_pfmBba.obj', opts=OPTS, input='pfmBba.cxx')
  TargetAdd('pfm-bba_config_pfm.obj', opts=OPTS, input='config_pfm.cxx')
  TargetAdd('pfm-bba.exe', input='pfm-bba_pfmBba.obj')
  TargetAdd('pfm-bba.exe', input='pfm-bba_config_pfm.obj')
  TargetAdd('pfm-bba.exe', input='libp3progbase.lib')
  TargetAdd('pfm-bba.exe', input='libp3pandatoolbase.lib')
  TargetAdd('pfm-bba.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('pfm-bba.exe', input='libp3pystub.lib')
  TargetAdd('pfm-bba.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/lwo/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/lwo']
  TargetAdd('p3lwo_composite1.obj', opts=OPTS, input='p3lwo_composite1.cxx')
  TargetAdd('libp3lwo.lib', input='p3lwo_composite1.obj')

#
# DIRECTORY: pandatool/src/lwoegg/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/lwoegg']
  TargetAdd('p3lwoegg_composite1.obj', opts=OPTS, input='p3lwoegg_composite1.cxx')
  TargetAdd('libp3lwoegg.lib', input='p3lwoegg_composite1.obj')

#
# DIRECTORY: pandatool/src/lwoprogs/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/lwoprogs', 'DIR:pandatool/src/lwo']
  TargetAdd('lwo-scan_lwoScan.obj', opts=OPTS, input='lwoScan.cxx')
  TargetAdd('lwo-scan.exe', input='lwo-scan_lwoScan.obj')
  TargetAdd('lwo-scan.exe', input='libp3lwo.lib')
  TargetAdd('lwo-scan.exe', input='libp3progbase.lib')
  TargetAdd('lwo-scan.exe', input='libp3pandatoolbase.lib')
  TargetAdd('lwo-scan.exe', input='libpandaegg.dll')
  TargetAdd('lwo-scan.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('lwo-scan.exe', input='libp3pystub.lib')
  TargetAdd('lwo-scan.exe', opts=['ADVAPI'])

  TargetAdd('lwo2egg_lwoToEgg.obj', opts=OPTS, input='lwoToEgg.cxx')
  TargetAdd('lwo2egg.exe', input='lwo2egg_lwoToEgg.obj')
  TargetAdd('lwo2egg.exe', input='libp3lwo.lib')
  TargetAdd('lwo2egg.exe', input='libp3lwoegg.lib')
  TargetAdd('lwo2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('lwo2egg.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/maya/
#

for VER in MAYAVERSIONS:
  VNUM=VER[4:]
  if (PkgSkip(VER)==0) and (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/maya', VER]
    TargetAdd('maya'+VNUM+'_composite1.obj', opts=OPTS, input='p3maya_composite1.cxx')
    TargetAdd('libmaya'+VNUM+'.lib', input='maya'+VNUM+'_composite1.obj')

#
# DIRECTORY: pandatool/src/mayaegg/
#

for VER in MAYAVERSIONS:
  VNUM=VER[4:]
  if (PkgSkip(VER)==0) and (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/mayaegg', 'DIR:pandatool/src/maya', VER]
    TargetAdd('mayaegg'+VNUM+'_loader.obj', opts=OPTS, input='mayaEggLoader.cxx')
    TargetAdd('mayaegg'+VNUM+'_composite1.obj', opts=OPTS, input='p3mayaegg_composite1.cxx')
    TargetAdd('libmayaegg'+VNUM+'.lib', input='mayaegg'+VNUM+'_loader.obj')
    TargetAdd('libmayaegg'+VNUM+'.lib', input='mayaegg'+VNUM+'_composite1.obj')

#
# DIRECTORY: pandatool/src/maxegg/
#

for VER in MAXVERSIONS:
  VNUM=VER[3:]
  if (PkgSkip(VER)==0) and (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/maxegg', VER,  "WINCOMCTL", "WINCOMDLG", "WINUSER", "MSFORSCOPE"]
    TargetAdd('maxEgg'+VNUM+'.res', opts=OPTS, input='maxEgg.rc')
    TargetAdd('maxegg'+VNUM+'_loader.obj', opts=OPTS, input='maxEggLoader.cxx')
    TargetAdd('maxegg'+VNUM+'_composite1.obj', opts=OPTS, input='p3maxegg_composite1.cxx')
    TargetAdd('maxegg'+VNUM+'.dlo', input='maxegg'+VNUM+'_composite1.obj')
    TargetAdd('maxegg'+VNUM+'.dlo', input='maxEgg'+VNUM+'.res')
    TargetAdd('maxegg'+VNUM+'.dlo', input='maxEgg.def', ipath=OPTS)
    TargetAdd('maxegg'+VNUM+'.dlo', input=COMMON_EGG2X_LIBS_PYSTUB)
    TargetAdd('maxegg'+VNUM+'.dlo', opts=OPTS)

#
# DIRECTORY: pandatool/src/maxprogs/
#

for VER in MAXVERSIONS:
  VNUM=VER[3:]
  if (PkgSkip(VER)==0) and (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/maxprogs', VER,  "WINCOMCTL", "WINCOMDLG", "WINUSER", "MSFORSCOPE"]
    TargetAdd('maxImportRes.res', opts=OPTS, input='maxImportRes.rc')
    TargetAdd('maxprogs'+VNUM+'_maxeggimport.obj', opts=OPTS, input='maxEggImport.cxx')
    TargetAdd('maxeggimport'+VNUM+'.dle', input='maxegg'+VNUM+'_loader.obj')
    TargetAdd('maxeggimport'+VNUM+'.dle', input='maxprogs'+VNUM+'_maxeggimport.obj')
    TargetAdd('maxeggimport'+VNUM+'.dle', input='libpandaegg.dll')
    TargetAdd('maxeggimport'+VNUM+'.dle', input='libpanda.dll')
    TargetAdd('maxeggimport'+VNUM+'.dle', input='libpandaexpress.dll')
    TargetAdd('maxeggimport'+VNUM+'.dle', input='maxImportRes.res')
    TargetAdd('maxeggimport'+VNUM+'.dle', input='maxEggImport.def', ipath=OPTS)
    TargetAdd('maxeggimport'+VNUM+'.dle', input=COMMON_DTOOL_LIBS)
    TargetAdd('maxeggimport'+VNUM+'.dle', opts=OPTS)

#
# DIRECTORY: pandatool/src/vrml/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/vrml', 'ZLIB', 'BISONPREFIX_vrmlyy']
  CreateFile(GetOutputDir()+"/include/vrmlParser.h")
  TargetAdd('p3vrml_vrmlParser.obj', opts=OPTS, input='vrmlParser.yxx')
  TargetAdd('vrmlParser.h', input='p3vrml_vrmlParser.obj', opts=['DEPENDENCYONLY'])
  TargetAdd('p3vrml_vrmlLexer.obj', opts=OPTS, input='vrmlLexer.lxx')
  TargetAdd('p3vrml_parse_vrml.obj', opts=OPTS, input='parse_vrml.cxx')
  TargetAdd('p3vrml_standard_nodes.obj', opts=OPTS, input='standard_nodes.cxx')
  TargetAdd('p3vrml_vrmlNode.obj', opts=OPTS, input='vrmlNode.cxx')
  TargetAdd('p3vrml_vrmlNodeType.obj', opts=OPTS, input='vrmlNodeType.cxx')
  TargetAdd('libp3vrml.lib', input='p3vrml_parse_vrml.obj')
  TargetAdd('libp3vrml.lib', input='p3vrml_standard_nodes.obj')
  TargetAdd('libp3vrml.lib', input='p3vrml_vrmlNode.obj')
  TargetAdd('libp3vrml.lib', input='p3vrml_vrmlNodeType.obj')
  TargetAdd('libp3vrml.lib', input='p3vrml_vrmlParser.obj')
  TargetAdd('libp3vrml.lib', input='p3vrml_vrmlLexer.obj')

#
# DIRECTORY: pandatool/src/vrmlegg/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/vrmlegg', 'DIR:pandatool/src/vrml']
  TargetAdd('p3vrmlegg_indexedFaceSet.obj', opts=OPTS, input='indexedFaceSet.cxx')
  TargetAdd('p3vrmlegg_vrmlAppearance.obj', opts=OPTS, input='vrmlAppearance.cxx')
  TargetAdd('p3vrmlegg_vrmlToEggConverter.obj', opts=OPTS, input='vrmlToEggConverter.cxx')
  TargetAdd('libp3vrmlegg.lib', input='p3vrmlegg_indexedFaceSet.obj')
  TargetAdd('libp3vrmlegg.lib', input='p3vrmlegg_vrmlAppearance.obj')
  TargetAdd('libp3vrmlegg.lib', input='p3vrmlegg_vrmlToEggConverter.obj')

#
# DIRECTORY: pandatool/src/xfile/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/xfile', 'ZLIB', 'BISONPREFIX_xyy', 'FLEXDASHI']
    CreateFile(GetOutputDir()+"/include/xParser.h")
    TargetAdd('p3xfile_xParser.obj', opts=OPTS, input='xParser.yxx')
    TargetAdd('xParser.h', input='p3xfile_xParser.obj', opts=['DEPENDENCYONLY'])
    TargetAdd('p3xfile_xLexer.obj', opts=OPTS, input='xLexer.lxx')
    TargetAdd('p3xfile_composite1.obj', opts=OPTS, input='p3xfile_composite1.cxx')
    TargetAdd('libp3xfile.lib', input='p3xfile_composite1.obj')
    TargetAdd('libp3xfile.lib', input='p3xfile_xParser.obj')
    TargetAdd('libp3xfile.lib', input='p3xfile_xLexer.obj')

#
# DIRECTORY: pandatool/src/xfileegg/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/xfileegg', 'DIR:pandatool/src/xfile']
    TargetAdd('p3xfileegg_composite1.obj', opts=OPTS, input='p3xfileegg_composite1.cxx')
    TargetAdd('libp3xfileegg.lib', input='p3xfileegg_composite1.obj')

#
# DIRECTORY: pandatool/src/ptloader/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/ptloader', 'DIR:pandatool/src/flt', 'DIR:pandatool/src/lwo', 'DIR:pandatool/src/xfile', 'DIR:pandatool/src/xfileegg', 'DIR:pandatool/src/daeegg', 'BUILDING:PTLOADER', 'FCOLLADA']
    TargetAdd('p3ptloader_config_ptloader.obj', opts=OPTS, input='config_ptloader.cxx')
    TargetAdd('p3ptloader_loaderFileTypePandatool.obj', opts=OPTS, input='loaderFileTypePandatool.cxx')
    TargetAdd('libp3ptloader.dll', input='p3ptloader_config_ptloader.obj')
    TargetAdd('libp3ptloader.dll', input='p3ptloader_loaderFileTypePandatool.obj')
    TargetAdd('libp3ptloader.dll', input='libp3fltegg.lib')
    TargetAdd('libp3ptloader.dll', input='libp3flt.lib')
    TargetAdd('libp3ptloader.dll', input='libp3lwoegg.lib')
    TargetAdd('libp3ptloader.dll', input='libp3lwo.lib')
    TargetAdd('libp3ptloader.dll', input='libp3dxfegg.lib')
    TargetAdd('libp3ptloader.dll', input='libp3dxf.lib')
    TargetAdd('libp3ptloader.dll', input='libp3objegg.lib')
    TargetAdd('libp3ptloader.dll', input='libp3vrmlegg.lib')
    TargetAdd('libp3ptloader.dll', input='libp3vrml.lib')
    TargetAdd('libp3ptloader.dll', input='libp3xfileegg.lib')
    TargetAdd('libp3ptloader.dll', input='libp3xfile.lib')
    if (PkgSkip("FCOLLADA")==0): TargetAdd('libp3ptloader.dll', input='libp3daeegg.lib')
    TargetAdd('libp3ptloader.dll', input='libp3eggbase.lib')
    TargetAdd('libp3ptloader.dll', input='libp3progbase.lib')
    TargetAdd('libp3ptloader.dll', input='libp3converter.lib')
    TargetAdd('libp3ptloader.dll', input='libp3pandatoolbase.lib')
    TargetAdd('libp3ptloader.dll', input='libpandaegg.dll')
    TargetAdd('libp3ptloader.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libp3ptloader.dll', opts=['MODULE', 'ADVAPI', 'FCOLLADA', 'WINUSER'])

#
# DIRECTORY: pandatool/src/miscprogs/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/miscprogs']
    TargetAdd('bin2c_binToC.obj', opts=OPTS, input='binToC.cxx')
    TargetAdd('bin2c.exe', input='bin2c_binToC.obj')
    TargetAdd('bin2c.exe', input='libp3progbase.lib')
    TargetAdd('bin2c.exe', input='libp3pandatoolbase.lib')
    TargetAdd('bin2c.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('bin2c.exe', input='libp3pystub.lib')
    TargetAdd('bin2c.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/pstatserver/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/pstatserver']
    TargetAdd('p3pstatserver_composite1.obj', opts=OPTS, input='p3pstatserver_composite1.cxx')
    TargetAdd('libp3pstatserver.lib', input='p3pstatserver_composite1.obj')

#
# DIRECTORY: pandatool/src/softprogs/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/softprogs', 'OPENSSL']
    TargetAdd('softcvs_softCVS.obj', opts=OPTS, input='softCVS.cxx')
    TargetAdd('softcvs_softFilename.obj', opts=OPTS, input='softFilename.cxx')
    TargetAdd('softcvs.exe', input='softcvs_softCVS.obj')
    TargetAdd('softcvs.exe', input='softcvs_softFilename.obj')
    TargetAdd('softcvs.exe', input='libp3progbase.lib')
    TargetAdd('softcvs.exe', input='libp3pandatoolbase.lib')
    TargetAdd('softcvs.exe', input='libpandaegg.dll')
    TargetAdd('softcvs.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('softcvs.exe', input='libp3pystub.lib')
    TargetAdd('softcvs.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/text-stats/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/text-stats']
    TargetAdd('text-stats_textMonitor.obj', opts=OPTS, input='textMonitor.cxx')
    TargetAdd('text-stats_textStats.obj', opts=OPTS, input='textStats.cxx')
    TargetAdd('text-stats.exe', input='text-stats_textMonitor.obj')
    TargetAdd('text-stats.exe', input='text-stats_textStats.obj')
    TargetAdd('text-stats.exe', input='libp3progbase.lib')
    TargetAdd('text-stats.exe', input='libp3pstatserver.lib')
    TargetAdd('text-stats.exe', input='libp3pandatoolbase.lib')
    TargetAdd('text-stats.exe', input='libpandaegg.dll')
    TargetAdd('text-stats.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('text-stats.exe', input='libp3pystub.lib')
    TargetAdd('text-stats.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/vrmlprogs/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/vrmlprogs', 'DIR:pandatool/src/vrml', 'DIR:pandatool/src/vrmlegg']
    TargetAdd('vrml-trans_vrmlTrans.obj', opts=OPTS, input='vrmlTrans.cxx')
    TargetAdd('vrml-trans.exe', input='vrml-trans_vrmlTrans.obj')
    TargetAdd('vrml-trans.exe', input='libp3vrml.lib')
    TargetAdd('vrml-trans.exe', input='libp3progbase.lib')
    TargetAdd('vrml-trans.exe', input='libp3pandatoolbase.lib')
    TargetAdd('vrml-trans.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('vrml-trans.exe', input='libp3pystub.lib')
    TargetAdd('vrml-trans.exe', opts=['ADVAPI'])

    TargetAdd('vrml2egg_vrmlToEgg.obj', opts=OPTS, input='vrmlToEgg.cxx')
    TargetAdd('vrml2egg.exe', input='vrml2egg_vrmlToEgg.obj')
    TargetAdd('vrml2egg.exe', input='libp3vrmlegg.lib')
    TargetAdd('vrml2egg.exe', input='libp3vrml.lib')
    TargetAdd('vrml2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
    TargetAdd('vrml2egg.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/win-stats/
# DIRECTORY: pandatool/src/gtk-stats/
#

if (PkgSkip("PANDATOOL")==0 and (GetTarget() == 'windows' or PkgSkip("GTK2")==0)):
    if GetTarget() == 'windows':
      OPTS=['DIR:pandatool/src/win-stats']
      TargetAdd('pstats_composite1.obj', opts=OPTS, input='winstats_composite1.cxx')
    else:
      OPTS=['DIR:pandatool/src/gtk-stats', 'GTK2']
      TargetAdd('pstats_composite1.obj', opts=OPTS, input='gtkstats_composite1.cxx')
    TargetAdd('pstats.exe', input='pstats_composite1.obj')
    TargetAdd('pstats.exe', input='libp3pstatserver.lib')
    TargetAdd('pstats.exe', input='libp3progbase.lib')
    TargetAdd('pstats.exe', input='libp3pandatoolbase.lib')
    TargetAdd('pstats.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('pstats.exe', input='libp3pystub.lib')
    TargetAdd('pstats.exe', opts=['WINSOCK', 'WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM', 'GTK2'])

#
# DIRECTORY: pandatool/src/xfileprogs/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/xfileprogs', 'DIR:pandatool/src/xfile', 'DIR:pandatool/src/xfileegg']
    TargetAdd('egg2x_eggToX.obj', opts=OPTS, input='eggToX.cxx')
    TargetAdd('egg2x.exe', input='egg2x_eggToX.obj')
    TargetAdd('egg2x.exe', input='libp3xfileegg.lib')
    TargetAdd('egg2x.exe', input='libp3xfile.lib')
    TargetAdd('egg2x.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
    TargetAdd('egg2x.exe', opts=['ADVAPI'])

    TargetAdd('x-trans_xFileTrans.obj', opts=OPTS, input='xFileTrans.cxx')
    TargetAdd('x-trans.exe', input='x-trans_xFileTrans.obj')
    TargetAdd('x-trans.exe', input='libp3progbase.lib')
    TargetAdd('x-trans.exe', input='libp3xfile.lib')
    TargetAdd('x-trans.exe', input='libp3pandatoolbase.lib')
    TargetAdd('x-trans.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('x-trans.exe', input='libp3pystub.lib')
    TargetAdd('x-trans.exe', opts=['ADVAPI'])

    TargetAdd('x2egg_xFileToEgg.obj', opts=OPTS, input='xFileToEgg.cxx')
    TargetAdd('x2egg.exe', input='x2egg_xFileToEgg.obj')
    TargetAdd('x2egg.exe', input='libp3xfileegg.lib')
    TargetAdd('x2egg.exe', input='libp3xfile.lib')
    TargetAdd('x2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
    TargetAdd('x2egg.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/mayaprogs/
#

for VER in MAYAVERSIONS:
  VNUM=VER[4:]
  if (PkgSkip(VER)==0) and (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/mayaprogs', 'DIR:pandatool/src/maya', 'DIR:pandatool/src/mayaegg', 'DIR:pandatool/src/cvscopy', 'BUILDING:MISC', VER]
    TargetAdd('mayaeggimport'+VNUM+'_mayaeggimport.obj', opts=OPTS, input='mayaEggImport.cxx')
    TargetAdd('mayaeggimport'+VNUM+'.mll', input='mayaegg'+VNUM+'_loader.obj')
    TargetAdd('mayaeggimport'+VNUM+'.mll', input='mayaeggimport'+VNUM+'_mayaeggimport.obj')
    TargetAdd('mayaeggimport'+VNUM+'.mll', input='libpandaegg.dll')
    TargetAdd('mayaeggimport'+VNUM+'.mll', input=COMMON_PANDA_LIBS)
    if GetTarget() == 'windows':
      TargetAdd('mayaeggimport'+VNUM+'.mll', input='libp3pystub.lib')
    TargetAdd('mayaeggimport'+VNUM+'.mll', opts=['ADVAPI', VER])

    TargetAdd('mayaloader'+VNUM+'_config_mayaloader.obj', opts=OPTS, input='config_mayaloader.cxx')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='mayaloader'+VNUM+'_config_mayaloader.obj')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libmayaegg'+VNUM+'.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3ptloader.dll')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libmaya'+VNUM+'.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3fltegg.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3flt.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3lwoegg.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3lwo.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3dxfegg.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3dxf.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3objegg.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3vrmlegg.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3vrml.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3xfileegg.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3xfile.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3eggbase.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3progbase.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3converter.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3pandatoolbase.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libpandaegg.dll')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libp3mayaloader'+VNUM+'.dll', opts=['ADVAPI', VER])

    TargetAdd('mayapview'+VNUM+'_mayaPview.obj', opts=OPTS, input='mayaPview.cxx')
    TargetAdd('libmayapview'+VNUM+'.mll', input='mayapview'+VNUM+'_mayaPview.obj')
    TargetAdd('libmayapview'+VNUM+'.mll', input='libmayaegg'+VNUM+'.lib')
    TargetAdd('libmayapview'+VNUM+'.mll', input='libmaya'+VNUM+'.lib')
    TargetAdd('libmayapview'+VNUM+'.mll', input='libp3framework.dll')
    if GetTarget() == 'windows':
      TargetAdd('libmayapview'+VNUM+'.mll', input=COMMON_EGG2X_LIBS_PYSTUB)
    else:
      TargetAdd('libmayapview'+VNUM+'.mll', input=COMMON_EGG2X_LIBS)
    TargetAdd('libmayapview'+VNUM+'.mll', opts=['ADVAPI', VER])

    TargetAdd('maya2egg'+VNUM+'_mayaToEgg.obj', opts=OPTS, input='mayaToEgg.cxx')
    TargetAdd('maya2egg'+VNUM+'_bin.exe', input='maya2egg'+VNUM+'_mayaToEgg.obj')
    TargetAdd('maya2egg'+VNUM+'_bin.exe', input='libmayaegg'+VNUM+'.lib')
    TargetAdd('maya2egg'+VNUM+'_bin.exe', input='libmaya'+VNUM+'.lib')
    if GetTarget() == 'windows':
      TargetAdd('maya2egg'+VNUM+'_bin.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
    else:
      TargetAdd('maya2egg'+VNUM+'_bin.exe', input=COMMON_EGG2X_LIBS)
    if GetTarget() == "darwin" and int(VNUM) >= 2009:
      TargetAdd('maya2egg'+VNUM+'_bin.exe', opts=['ADVAPI', 'NOPPC', VER])
    else:
      TargetAdd('maya2egg'+VNUM+'_bin.exe', opts=['ADVAPI', VER])

    TargetAdd('egg2maya'+VNUM+'_eggToMaya.obj', opts=OPTS, input='eggToMaya.cxx')
    TargetAdd('egg2maya'+VNUM+'_bin.exe', input='egg2maya'+VNUM+'_eggToMaya.obj')
    TargetAdd('egg2maya'+VNUM+'_bin.exe', input='libmayaegg'+VNUM+'.lib')
    TargetAdd('egg2maya'+VNUM+'_bin.exe', input='libmaya'+VNUM+'.lib')
    if GetTarget() == 'windows':
      TargetAdd('egg2maya'+VNUM+'_bin.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
    else:
      TargetAdd('egg2maya'+VNUM+'_bin.exe', input=COMMON_EGG2X_LIBS)
    if GetTarget() == 'darwin' and int(VNUM) >= 2009:
      TargetAdd('egg2maya'+VNUM+'_bin.exe', opts=['ADVAPI', 'NOPPC', VER])
    else:
      TargetAdd('egg2maya'+VNUM+'_bin.exe', opts=['ADVAPI', VER])

    TargetAdd('mayacopy'+VNUM+'_mayaCopy.obj', opts=OPTS, input='mayaCopy.cxx')
    TargetAdd('mayacopy'+VNUM+'_bin.exe', input='mayacopy'+VNUM+'_mayaCopy.obj')
    TargetAdd('mayacopy'+VNUM+'_bin.exe', input='libp3cvscopy.lib')
    TargetAdd('mayacopy'+VNUM+'_bin.exe', input='libmaya'+VNUM+'.lib')
    if GetTarget() == 'windows':
      TargetAdd('mayacopy'+VNUM+'_bin.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
    else:
      TargetAdd('mayacopy'+VNUM+'_bin.exe', input=COMMON_EGG2X_LIBS)
    if GetTarget() == 'darwin' and int(VNUM) >= 2009:
      TargetAdd('mayacopy'+VNUM+'_bin.exe', opts=['ADVAPI', 'NOPPC', VER])
    else:
      TargetAdd('mayacopy'+VNUM+'_bin.exe', opts=['ADVAPI', VER])

    TargetAdd('mayasavepview'+VNUM+'_mayaSavePview.obj', opts=OPTS, input='mayaSavePview.cxx')
    TargetAdd('libmayasavepview'+VNUM+'.mll', input='mayasavepview'+VNUM+'_mayaSavePview.obj')
    TargetAdd('libmayasavepview'+VNUM+'.mll', opts=['ADVAPI',  VER])

    TargetAdd('mayapath'+VNUM+'.obj', opts=OPTS, input='mayapath.cxx')

    TargetAdd('maya2egg'+VNUM+'.exe', input='mayapath'+VNUM+'.obj')
    TargetAdd('maya2egg'+VNUM+'.exe', opts=['ADVAPI'])
    TargetAdd('maya2egg'+VNUM+'.exe', input=COMMON_DTOOL_LIBS)
    TargetAdd('maya2egg'+VNUM+'.exe', input='libpandaexpress.dll')

    TargetAdd('egg2maya'+VNUM+'.exe', input='mayapath'+VNUM+'.obj')
    TargetAdd('egg2maya'+VNUM+'.exe', opts=['ADVAPI'])
    TargetAdd('egg2maya'+VNUM+'.exe', input=COMMON_DTOOL_LIBS)
    TargetAdd('egg2maya'+VNUM+'.exe', input='libpandaexpress.dll')

    TargetAdd('mayacopy'+VNUM+'.exe', input='mayapath'+VNUM+'.obj')
    TargetAdd('mayacopy'+VNUM+'.exe', opts=['ADVAPI'])
    TargetAdd('mayacopy'+VNUM+'.exe', input=COMMON_DTOOL_LIBS)
    TargetAdd('mayacopy'+VNUM+'.exe', input='libpandaexpress.dll')

#
# DIRECTORY: contrib/src/ai/
#
if (PkgSkip("CONTRIB")==0 and not RUNTIME):
  OPTS=['DIR:contrib/src/ai', 'BUILDING:PANDAAI']
  TargetAdd('p3ai_composite1.obj', opts=OPTS, input='p3ai_composite1.cxx')
  TargetAdd('libpandaai.dll', input='p3ai_composite1.obj')
  TargetAdd('libpandaai.dll', input=COMMON_PANDA_LIBS)

  OPTS=['DIR:contrib/src/ai']
  IGATEFILES=GetDirectoryContents('contrib/src/ai', ["*.h", "*_composite*.cxx"])
  TargetAdd('libpandaai.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpandaai.in', opts=['IMOD:ai', 'ILIB:libpandaai', 'SRCDIR:contrib/src/ai'])
  TargetAdd('libpandaai_igate.obj', input='libpandaai.in', opts=["DEPENDENCYONLY"])

  TargetAdd('ai_module.obj', input='libpandaai.in')
  TargetAdd('ai_module.obj', opts=OPTS)
  TargetAdd('ai_module.obj', opts=['IMOD:ai', 'ILIB:ai'])

  TargetAdd('ai.pyd', input='ai_module.obj')
  TargetAdd('ai.pyd', input='libpandaai_igate.obj')
  TargetAdd('ai.pyd', input='libpandaai.dll')
  TargetAdd('ai.pyd', input='core.pyd')
  TargetAdd('ai.pyd', input=COMMON_PANDA_LIBS)
  TargetAdd('ai.pyd', opts=['PYTHON'])

#
# Run genpycode
#

if (PkgSkip("PYTHON")==0 and not RUNTIME):
  # We're phasing out the concept of PandaModules, so do not
  # add new libraries here.  Only add new libraries when you've
  # split them out of libpanda and need to maintain backward
  # compatibility with old imports.  See direct/src/ffi/panda3d.py

  TargetAdd('PandaModules.py', input='core.pyd')
  if (PkgSkip("PANDAPHYSICS")==0):
    TargetAdd('PandaModules.py', input='physics.pyd')
  if (PkgSkip('PANDAFX')==0):
    TargetAdd('PandaModules.py', input='fx.pyd')
  if (PkgSkip("DIRECT")==0):
    TargetAdd('PandaModules.py', input='direct.pyd')
  if (PkgSkip("VISION")==0):  
    TargetAdd('PandaModules.py', input='vision.pyd')
  if (PkgSkip("SKEL")==0):
    TargetAdd('PandaModules.py', input='skel.pyd')
  TargetAdd('PandaModules.py', input='egg.pyd')
  if (PkgSkip("AWESOMIUM")==0):
    TargetAdd('PandaModules.py', input='awesomium.pyd')
  if (PkgSkip("ODE")==0):
    TargetAdd('PandaModules.py', input='ode.pyd')
  if (PkgSkip("VRPN")==0):
    TargetAdd('PandaModules.py', input='vrpn.pyd')

#
# Generate the models directory and samples directory
#

if (PkgSkip("DIRECT")==0 and not RUNTIME):
  model_extensions = ["*.egg"]
  # Check if we have access to an flt2egg utility, either self-compiled or on the system.
  if ((PkgSkip("PANDATOOL")==0 and GetHost()==GetTarget()) or LocateBinary('flt2egg')):
      model_extensions.append("*.flt")

  for model in GetDirectoryContents("dmodels/src/misc", model_extensions):
      if (PkgSkip("ZLIB")==0 and PkgSkip("DEPLOYTOOLS")==0 and not RTDIST):
          newname = model[:-4] + ".egg.pz"
      else:
          newname = model[:-4] + ".egg"
      TargetAdd(GetOutputDir()+"/models/misc/"+newname, input="dmodels/src/misc/"+model)

  for model in GetDirectoryContents("dmodels/src/gui", model_extensions):
      if (PkgSkip("ZLIB")==0 and PkgSkip("DEPLOYTOOLS")==0 and not RTDIST):
          newname = model[:-4] + ".egg.pz"
      else:
          newname = model[:-4] + ".egg"
      TargetAdd(GetOutputDir()+"/models/gui/"+newname, input="dmodels/src/gui/"+model)

  for model in GetDirectoryContents("models", model_extensions):
      if (PkgSkip("ZLIB")==0 and PkgSkip("DEPLOYTOOLS")==0 and not RTDIST):
          newname = model[:-4] + ".egg.pz"
      else:
          newname = model[:-4] + ".egg"
      TargetAdd(GetOutputDir()+"/models/"+newname, input="models/"+model)

  CopyAllFiles(GetOutputDir()+"/models/audio/sfx/",  "dmodels/src/audio/sfx/", ".wav")
  CopyAllFiles(GetOutputDir()+"/models/icons/",      "dmodels/src/icons/",     ".gif")

  CopyAllFiles(GetOutputDir()+"/models/maps/",       "models/maps/",           ".jpg")
  CopyAllFiles(GetOutputDir()+"/models/maps/",       "models/maps/",           ".png")
  CopyAllFiles(GetOutputDir()+"/models/maps/",       "models/maps/",           ".rgb")
  CopyAllFiles(GetOutputDir()+"/models/maps/",       "models/maps/",           ".rgba")

  CopyAllFiles(GetOutputDir()+"/models/maps/",       "dmodels/src/maps/",      ".jpg")
  CopyAllFiles(GetOutputDir()+"/models/maps/",       "dmodels/src/maps/",      ".png")
  CopyAllFiles(GetOutputDir()+"/models/maps/",       "dmodels/src/maps/",      ".rgb")
  CopyAllFiles(GetOutputDir()+"/models/maps/",       "dmodels/src/maps/",      ".rgba")

#
# Build the rtdist.
#

if (RTDIST):
  OPTS=['DIR:direct/src/p3d']
  TargetAdd('_panda3d', opts=OPTS, input='panda3d.pdef')
  TargetAdd('_coreapi', opts=OPTS, input='coreapi.pdef')
  TargetAdd('_thirdparty', opts=OPTS, input='thirdparty.pdef')

#
# Distribute prebuilt .p3d files as executable.
#

if (PkgSkip("DIRECT")==0 and not RUNTIME and not RTDIST):
  if GetTarget() == 'windows':
    OPTS=['DIR:direct/src/p3d']
    TargetAdd('p3dWrapper.obj', opts=OPTS, input='p3dWrapper.c')
    TargetAdd('p3dWrapper.exe', input='p3dWrapper.obj')
    TargetAdd('p3dWrapper.exe', opts=["ADVAPI"])

  for g in glob.glob("direct/src/p3d/*.p3d"):
    base = os.path.basename(g)
    base = base.split(".", 1)[0]

    if GetTarget() == 'windows':
      TargetAdd(base+".exe", input='p3dWrapper.exe')
      CopyFile(GetOutputDir()+"/bin/"+base+".p3d", g)
    else:
      CopyFile(GetOutputDir()+"/bin/"+base, g)
      oscmd("chmod +x "+GetOutputDir()+"/bin/"+base)

##########################################################################################
#
# Dependency-Based Distributed Build System.
#
##########################################################################################

DEPENDENCYQUEUE=[]

for target in TARGET_LIST:
    name = target.name
    inputs = target.inputs
    opts = target.opts
    deps = target.deps
    DEPENDENCYQUEUE.append([CompileAnything, [name, inputs, opts], [name], deps, []])

def BuildWorker(taskqueue, donequeue):
    while True:
        try:
            task = taskqueue.get(timeout=1)
        except:
            ProgressOutput(None, "Waiting for tasks...")
            task = taskqueue.get()
        sys.stdout.flush()
        if (task == 0): return
        try:
            task[0](*task[1])
            donequeue.put(task)
        except:
            donequeue.put(0)

def AllSourcesReady(task, pending):
    sources = task[3]
    for x in sources:
        if (x in pending):
            return 0
    sources = task[1][1]
    for x in sources:
        if (x in pending):
            return 0
    altsources = task[4]
    for x in altsources:
        if (x in pending):
            return 0
    return 1

def ParallelMake(tasklist):
    # Create the communication queues.
    donequeue = queue.Queue()
    taskqueue = queue.Queue()
    # Build up a table listing all the pending targets
    #task = [CompileAnything, [name, inputs, opts], [name], deps, []]
    # task[2] = [name]
    # task[3] = deps
    # The python tool package, in particular fltegg seems to throw parallelmake off
    # A hack for now is to divide the tasklist into two parts, one to be built in parallel
    # and another subpart to be built sequentially. The most time consuming part of the process
    # is the c++ code generation anyways.

    tasklist_seq = []
    i = 0
    while i < len(tasklist):
        if tasklist[i][2][0].endswith('.egg') | tasklist[i][2][0].endswith('.egg.pz'):
            break
        i += 1
    if i < len(tasklist):
        tasklist_seq = tasklist[i:]
        tasklist = tasklist[:i]
    iNumStartingTasks = len(tasklist)

    pending = {}
    for task in tasklist:
        for target in task[2]:
            pending[target] = 1
    # Create the workers
    for slave in range(THREADCOUNT):
        th = threading.Thread(target=BuildWorker, args=[taskqueue, donequeue])
        th.setDaemon(1)
        th.start()
    # Feed tasks to the workers.
    tasksqueued = 0
    while True:
        if (tasksqueued < THREADCOUNT):
            extras = []
            for task in tasklist:
                if (tasksqueued < THREADCOUNT) and (AllSourcesReady(task, pending)):
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
    # Kill the workers.
    for slave in range(THREADCOUNT):
        taskqueue.put(0)
    # Make sure there aren't any unsatisfied tasks
    if len(tasklist) > 0:
        exit("Dependency problems: " + str(len(tasklist)) + " tasks not finished. First task unsatisfied: "+str(tasklist[0][2]))
    SequentialMake(tasklist_seq)


def SequentialMake(tasklist):
    i = 0
    for task in tasklist:
        if (NeedsBuild(task[2], task[3])):
            task[0](*task[1] + [(i * 100.0) / len(tasklist)])
            JustBuilt(task[2], task[3])
        i += 1

def RunDependencyQueue(tasklist):
    if (THREADCOUNT!=0):
        ParallelMake(tasklist)
    else:
        SequentialMake(tasklist)

try:
    RunDependencyQueue(DEPENDENCYQUEUE)
except:
    SaveDependencyCache()
    raise

##########################################################################################
#
# The Installers
#
# Under windows, we can build an 'exe' package using NSIS
# Under linux, we can build a 'deb' package or an 'rpm' package.
# Under OSX, we can make a 'dmg' package.
#
##########################################################################################

def MakeInstallerNSIS(file, fullname, smdirectory, installdir):
    if (os.path.isfile(file)):
        os.remove(file)
    elif (os.path.isdir(file)):
        shutil.rmtree(file)

    if (RUNTIME):
        # Invoke the make_installer script.
        AddToPathEnv("PATH", GetOutputDir() + "\\bin")
        AddToPathEnv("PATH", GetOutputDir() + "\\plugins")
        oscmd(sys.executable + " -B direct\\src\\plugin_installer\\make_installer.py --version %s" % VERSION)
        shutil.move("direct\\src\\plugin_installer\\p3d-setup.exe", file)
        return

    print("Building "+fullname+" installer. This can take up to an hour.")
    if (COMPRESSOR != "lzma"):
        print("Note: you are using zlib, which is faster, but lzma gives better compression.")
    if (os.path.exists("nsis-output.exe")):
        os.remove("nsis-output.exe")
    WriteFile(GetOutputDir()+"/tmp/__init__.py", "")
    psource = os.path.abspath(".")
    panda = os.path.abspath(GetOutputDir())

    if GetTargetArch() == 'x64':
        regview = '64'
    else:
        regview = '32'

    nsis_defs = {
        'COMPRESSOR'  : COMPRESSOR,
        'NAME'        : fullname,
        'SMDIRECTORY' : smdirectory,
        'INSTALLDIR'  : installdir,
        'OUTFILE'     : os.path.join(psource, 'nsis-output.exe'),
        'LICENSE'     : os.path.join(panda, 'LICENSE'),
        'LANGUAGE'    : "English",
        'RUNTEXT'     : "Visit the Panda Manual",
        'IBITMAP'     : "panda-install.bmp",
        'UBITMAP'     : "panda-install.bmp",
        'PANDA'       : panda,
        'PYVER'       : SDK["PYTHONVERSION"][6:9],
        'PANDACONF'   : os.path.join(panda, 'etc'),
        'PSOURCE'     : psource,
        'PYEXTRAS'    : os.path.join(os.path.abspath(GetThirdpartyBase()), 'win-extras'),
        'REGVIEW'     : regview,
    }

    if GetHost() == 'windows':
        cmd = os.path.join(GetThirdpartyBase(), 'win-nsis', 'makensis') + ' /V2'
        for item in nsis_defs.items():
            cmd += ' /D%s="%s"' % item
    else:
        cmd = 'makensis -V2'
        for item in nsis_defs.items():
            cmd += ' -D%s="%s"' % item

    cmd += ' "%s"' % (os.path.join(psource, 'direct', 'src', 'directscripts', 'packpanda.nsi'))
    oscmd(cmd)
    os.rename("nsis-output.exe", file)


INSTALLER_DEB_FILE="""
Package: panda3dMAJOR
Version: VERSION
Section: libdevel
Priority: optional
Architecture: ARCH
Essential: no
Depends: DEPENDS
Recommends: panda3d-runtime, python-wxversion, python-profiler (>= PV), python-tk (>= PV), RECOMMENDS
Provides: panda3d
Conflicts: panda3d
Replaces: panda3d
Maintainer: rdb <me@rdb.name>
Installed-Size: INSTSIZE
Description: The Panda3D free 3D engine SDK
 Panda3D is a game engine which includes graphics, audio, I/O, collision detection, and other abilities relevant to the creation of 3D games. Panda3D is open source and free software under the revised BSD license, and can be used for both free and commercial game development at no financial cost.
 Panda3D's intended game-development language is Python. The engine itself is written in C++, and utilizes an automatic wrapper-generator to expose the complete functionality of the engine in a Python interface.
 .
 This package contains the SDK for development with Panda3D, install panda3d-runtime for the runtime files.

"""

RUNTIME_INSTALLER_DEB_FILE="""
Package: panda3d-runtime
Version: VERSION
Section: web
Priority: optional
Architecture: ARCH
Essential: no
Depends: DEPENDS
Provides: panda3d-runtime
Maintainer: rdb <me@rdb.name>
Installed-Size: INSTSIZE
Description: Runtime binary and browser plugin for the Panda3D Game Engine
 This package contains the runtime distribution and browser plugin of the Panda3D engine. It allows you view webpages that contain Panda3D content and to run games created with Panda3D that are packaged as .p3d file.

"""

# We're not putting "python" in the "Requires" field,
# since the rpm-based distros don't have a common
# naming for the Python package.
INSTALLER_SPEC_FILE="""
%{!?python_sitearch: %global python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(1)")}
Summary: The Panda3D free 3D engine SDK
Name: panda3d
Version: VERSION
Release: RPMRELEASE
License: BSD License
Group: Development/Libraries
BuildRoot: PANDASOURCE/targetroot
%description
Panda3D is a game engine which includes graphics, audio, I/O, collision detection, and other abilities relevant to the creation of 3D games. Panda3D is open source and free software under the revised BSD license, and can be used for both free and commercial game development at no financial cost.
Panda3D's intended game-development language is Python. The engine itself is written in C++, and utilizes an automatic wrapper-generator to expose the complete functionality of the engine in a Python interface.

This package contains the SDK for development with Panda3D, install panda3d-runtime for the runtime files.
%post
/sbin/ldconfig
%postun
/sbin/ldconfig
%files
%defattr(-,root,root)
/etc/Confauto.prc
/etc/Config.prc
/usr/share/panda3d
/usr/share/mime-info/panda3d.mime
/usr/share/mime-info/panda3d.keys
/usr/share/mime/packages/panda3d.xml
/usr/share/application-registry/panda3d.applications
/usr/share/applications/*.desktop
/etc/ld.so.conf.d/panda3d.conf
/usr/bin
/usr/%_lib/panda3d
%{python_sitearch}
/usr/include/panda3d
"""

RUNTIME_INSTALLER_SPEC_FILE="""
Summary: Runtime binary and browser plugin for the Panda3D Game Engine
Name: panda3d-runtime
Version: VERSION
Release: RPMRELEASE
License: BSD License
Group: Productivity/Graphics/Other
BuildRoot: PANDASOURCE/targetroot
%description
This package contains the runtime distribution and browser plugin of the Panda3D engine. It allows you view webpages that contain Panda3D content and to run games created with Panda3D that are packaged as .p3d file.
%files
%defattr(-,root,root)
/usr/bin/panda3d
/usr/%_lib/nppanda3d.so
/usr/%_lib/mozilla/plugins/nppanda3d.so
/usr/%_lib/mozilla-firefox/plugins/nppanda3d.so
/usr/%_lib/xulrunner-addons/plugins/nppanda3d.so
/usr/share/mime-info/panda3d-runtime.mime
/usr/share/mime-info/panda3d-runtime.keys
/usr/share/mime/packages/panda3d-runtime.xml
/usr/share/application-registry/panda3d-runtime.applications
/usr/share/applications/*.desktop
"""

# plist file for Mac OSX
Info_plist = """<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleIdentifier</key>
  <string>%(package_id)s</string>
  <key>CFBundleShortVersionString</key>
  <string>%(version)s</string>
  <key>IFPkgFlagRelocatable</key>
  <false/>
  <key>IFPkgFlagAuthorizationAction</key>
  <string>RootAuthorization</string>
  <key>IFPkgFlagAllowBackRev</key>
  <true/>
</dict>
</plist>
"""

# FreeBSD pkg-descr
INSTALLER_PKG_DESCR_FILE = """
Panda3D is a game engine which includes graphics, audio, I/O, collision detection, and other abilities relevant to the creation of 3D games. Panda3D is open source and free software under the revised BSD license, and can be used for both free and commercial game development at no financial cost.
Panda3D's intended game-development language is Python. The engine itself is written in C++, and utilizes an automatic wrapper-generator to expose the complete functionality of the engine in a Python interface.

This package contains the SDK for development with Panda3D, install panda3d-runtime for the runtime files.

WWW: http://www.panda3d.org/
"""

def MakeInstallerLinux():
    if RUNTIME: # No worries, it won't be used
        PYTHONV = "python"
    else:
        PYTHONV = SDK["PYTHONVERSION"]
    PV = PYTHONV.replace("python", "")
    if (os.path.isdir("targetroot")): oscmd("chmod -R 755 targetroot")
    oscmd("rm -rf targetroot data.tar.gz control.tar.gz panda3d.spec")
    oscmd("mkdir --mode=0755 targetroot")

    # Invoke installpanda.py to install it into a temporary dir
    if RUNTIME:
        InstallRuntime(destdir = "targetroot", prefix = "/usr", outputdir = GetOutputDir())
    else:
        InstallPanda(destdir = "targetroot", prefix = "/usr", outputdir = GetOutputDir())
        oscmd("chmod -R 755 targetroot/usr/share/panda3d")

    if (os.path.exists("/usr/bin/rpmbuild") and not os.path.exists("/usr/bin/dpkg-deb")):
        oscmd("rm -rf targetroot/DEBIAN")
        oscmd("rpm -E '%_target_cpu' > "+GetOutputDir()+"/tmp/architecture.txt")
        ARCH = ReadFile(GetOutputDir()+"/tmp/architecture.txt").strip()
        pandasource = os.path.abspath(os.getcwd())
        if (RUNTIME):
            txt = RUNTIME_INSTALLER_SPEC_FILE[1:]
        else:
            txt = INSTALLER_SPEC_FILE[1:]
        txt = txt.replace("VERSION", VERSION).replace("RPMRELEASE", RPMRELEASE).replace("PANDASOURCE", pandasource).replace("PV", PV)
        WriteFile("panda3d.spec", txt)
        oscmd("fakeroot rpmbuild --define '_rpmdir "+pandasource+"' --buildroot '"+os.path.abspath("targetroot")+"' -bb panda3d.spec")
        if (RUNTIME):
            oscmd("mv "+ARCH+"/panda3d-runtime-"+VERSION+"-"+RPMRELEASE+"."+ARCH+".rpm .")
        else:
            oscmd("mv "+ARCH+"/panda3d-"+VERSION+"-"+RPMRELEASE+"."+ARCH+".rpm .")
        oscmd("rm -rf "+ARCH, True)

    if (os.path.exists("/usr/bin/dpkg-deb")):
        oscmd("dpkg --print-architecture > "+GetOutputDir()+"/tmp/architecture.txt")
        ARCH = ReadFile(GetOutputDir()+"/tmp/architecture.txt").strip()
        if (RUNTIME):
            txt = RUNTIME_INSTALLER_DEB_FILE[1:]
        else:
            txt = INSTALLER_DEB_FILE[1:]
        txt = txt.replace("VERSION", DEBVERSION).replace("ARCH", ARCH).replace("PV", PV).replace("MAJOR", MAJOR_VERSION)
        txt = txt.replace("INSTSIZE", str(GetDirectorySize("targetroot") / 1024))
        oscmd("mkdir --mode=0755 -p targetroot/DEBIAN")
        oscmd("cd targetroot ; (find usr -type f -exec md5sum {} \;) >  DEBIAN/md5sums")
        if (not RUNTIME):
          oscmd("cd targetroot ; (find etc -type f -exec md5sum {} \;) >> DEBIAN/md5sums")
          WriteFile("targetroot/DEBIAN/conffiles","/etc/Config.prc\n")
        WriteFile("targetroot/DEBIAN/postinst","#!/bin/sh\necho running ldconfig\nldconfig\n")
        oscmd("cp targetroot/DEBIAN/postinst targetroot/DEBIAN/postrm")
        oscmd("mkdir targetroot/debian")
        WriteFile("targetroot/debian/control", "")
        if (RUNTIME):
            oscmd("ln -s .. targetroot/debian/panda3d-runtime")
            oscmd("cd targetroot ; dpkg-shlibdeps -xpanda3d-runtime debian/panda3d-runtime/usr/lib/*.so* debian/panda3d-runtime/usr/lib64/*.so* debian/panda3d-runtime/usr/bin/*")
            depends = ReadFile("targetroot/debian/substvars").replace("shlibs:Depends=", "").strip()
            WriteFile("targetroot/DEBIAN/control", txt.replace("DEPENDS", depends))
        else:
            oscmd("ln -s .. targetroot/debian/panda3d" + MAJOR_VERSION)
            oscmd("cd targetroot ; dpkg-gensymbols -v%s -ppanda3d%s -eusr/lib/panda3d/lib*.so* -eusr/lib64/panda3d/lib*.so* -ODEBIAN/symbols >/dev/null" % (DEBVERSION, MAJOR_VERSION))
            # Library dependencies are required, binary dependencies are recommended. Dunno why -xlibphysx-extras is needed, prolly a bug in their package
            oscmd("cd targetroot ; LD_LIBRARY_PATH=usr/lib/panda3d dpkg-shlibdeps --ignore-missing-info --warnings=2 -xpanda3d%s -xlibphysx-extras -Tdebian/substvars_dep debian/panda3d%s/usr/lib/panda3d/lib*.so* debian/panda3d%s/usr/lib64/panda3d/lib*.so*" % (MAJOR_VERSION, MAJOR_VERSION, MAJOR_VERSION))
            oscmd("cd targetroot ; LD_LIBRARY_PATH=usr/lib/panda3d dpkg-shlibdeps --ignore-missing-info --warnings=2 -xpanda3d%s -Tdebian/substvars_rec debian/panda3d%s/usr/bin/*" % (MAJOR_VERSION, MAJOR_VERSION))
            depends = ReadFile("targetroot/debian/substvars_dep").replace("shlibs:Depends=", "").strip()
            recommends = ReadFile("targetroot/debian/substvars_rec").replace("shlibs:Depends=", "").strip()
            if PkgSkip("PYTHON")==0:
                depends += ", " + PYTHONV + ", python-pmw"
            if PkgSkip("NVIDIACG")==0:
                depends += ", nvidia-cg-toolkit"
            if depends.startswith(', '):
                depends = depends[2:]
            WriteFile("targetroot/DEBIAN/control", txt.replace("DEPENDS", depends).replace("RECOMMENDS", recommends))
        oscmd("rm -rf targetroot/debian")
        oscmd("chmod -R 755 targetroot/DEBIAN")
        if (RUNTIME):
            oscmd("cd targetroot/DEBIAN ; chmod 644 control md5sums")
            oscmd("fakeroot dpkg-deb -b targetroot panda3d-runtime_"+DEBVERSION+"_"+ARCH+".deb")
        else:
            oscmd("cd targetroot/DEBIAN ; chmod 644 control md5sums conffiles symbols")
            oscmd("fakeroot dpkg-deb -b targetroot panda3d"+MAJOR_VERSION+"_"+DEBVERSION+"_"+ARCH+".deb")
        oscmd("chmod -R 755 targetroot")

    if not (os.path.exists("/usr/bin/rpmbuild") or os.path.exists("/usr/bin/dpkg-deb")):
        exit("To build an installer, either rpmbuild or dpkg-deb must be present on your system!")

#    oscmd("chmod -R 755 targetroot")
#    oscmd("rm -rf targetroot data.tar.gz control.tar.gz panda3d.spec "+ARCH)

def MakeInstallerOSX():
    if (RUNTIME):
        # Invoke the make_installer script.
        AddToPathEnv("DYLD_LIBRARY_PATH", GetOutputDir() + "/plugins")
        cmdstr = sys.executable + " "
        if sys.version_info >= (2, 6):
            cmdstr += "-B "

        cmdstr += "direct/src/plugin_installer/make_installer.py --version %s" % VERSION
        oscmd(cmdstr)
        return

    import compileall
    if (os.path.isfile("Panda3D-%s.dmg" % VERSION)): oscmd("rm -f Panda3D-%s.dmg" % VERSION)
    if (os.path.exists("dstroot")): oscmd("rm -rf dstroot")
    if (os.path.exists("Panda3D-rw.dmg")): oscmd('rm -f Panda3D-rw.dmg')

    oscmd("mkdir -p dstroot/base/Developer/Panda3D/lib")
    oscmd("mkdir -p dstroot/base/Developer/Panda3D/panda3d")
    oscmd("mkdir -p dstroot/base/Developer/Panda3D/etc")
    oscmd("cp %s/etc/Config.prc           dstroot/base/Developer/Panda3D/etc/Config.prc" % GetOutputDir())
    oscmd("cp %s/etc/Confauto.prc         dstroot/base/Developer/Panda3D/etc/Confauto.prc" % GetOutputDir())
    oscmd("cp -R %s/models                dstroot/base/Developer/Panda3D/models" % GetOutputDir())
    oscmd("cp -R doc/LICENSE              dstroot/base/Developer/Panda3D/LICENSE")
    oscmd("cp -R doc/ReleaseNotes         dstroot/base/Developer/Panda3D/ReleaseNotes")
    if os.path.isdir(GetOutputDir()+"/plugins"):
        oscmd("cp -R %s/plugins           dstroot/base/Developer/Panda3D/plugins" % GetOutputDir())

    install_libs = []
    for base in os.listdir(GetOutputDir()+"/lib"):
        if (not base.endswith(".a")):
            install_libs.append("lib/"+base)
    for base in os.listdir(GetOutputDir()+"/panda3d"):
        if (not base.endswith(".a")):
            install_libs.append("panda3d/"+base)

    for base in install_libs:
        libname = "dstroot/base/Developer/Panda3D/" + base
        # We really need to specify -R in order not to follow symlinks
        # On OSX, just specifying -P is not enough to do that.
        oscmd("cp -R -P " + GetOutputDir() + "/" + base + " " + libname)

        # Execute install_name_tool to make them reference an absolute path
        if (libname.endswith(".dylib") or libname.endswith(".so")) and not os.path.islink(libname):
            oscmd("install_name_tool -id /Developer/Panda3D/%s %s" % (base, libname), True)
            oscmd("otool -L %s | grep .dylib > %s/tmp/otool-libs.txt" % (libname, GetOutputDir()), True)
            for line in open(GetOutputDir()+"/tmp/otool-libs.txt", "r"):
                if len(line.strip()) > 0 and not line.strip().endswith(":"):
                    libdep = line.strip().split(" ", 1)[0]
                    if 'lib/' + os.path.basename(libdep) in install_libs:
                        oscmd("install_name_tool -change %s /Developer/Panda3D/lib/%s %s" % (libdep, os.path.basename(libdep), libname), True)

    oscmd("mkdir -p dstroot/tools/Developer/Tools/Panda3D")
    oscmd("mkdir -p dstroot/tools/Developer/Panda3D")
    oscmd("mkdir -p dstroot/tools/etc/paths.d")
    # Trailing newline is important, works around a bug in OSX
    WriteFile("dstroot/tools/etc/paths.d/Panda3D", "/Developer/Tools/Panda3D\n")

    for base in os.listdir(GetOutputDir()+"/bin"):
        binname = "dstroot/tools/Developer/Tools/Panda3D/" + base
        # OSX needs the -R argument to copy symbolic links correctly, it doesn't have -d. How weird.
        oscmd("cp -R " + GetOutputDir() + "/bin/" + base + " " + binname)

        # Execute install_name_tool to make the binaries reference an absolute path
        if (not os.path.islink(binname)):
            oscmd("otool -L %s | grep .dylib > %s/tmp/otool-libs.txt" % (binname, GetOutputDir()), True)
            for line in open(GetOutputDir()+"/tmp/otool-libs.txt", "r"):
                if len(line.strip()) > 0 and not line.strip().endswith(":"):
                    libdep = line.strip().split(" ", 1)[0]
                    if 'lib/' + os.path.basename(libdep) in install_libs:
                        oscmd("install_name_tool -change %s /Developer/Panda3D/lib/%s %s" % (libdep, os.path.basename(libdep), binname), True)

    if PkgSkip("PYTHON")==0:
        PV = SDK["PYTHONVERSION"].replace("python", "")
        oscmd("mkdir -p dstroot/pythoncode/usr/bin")
        oscmd("mkdir -p dstroot/pythoncode/Developer/Panda3D/direct")
        oscmd("mkdir -p dstroot/pythoncode/Library/Python/%s/site-packages" % PV)
        WriteFile("dstroot/pythoncode/Library/Python/%s/site-packages/Panda3D.pth" % PV, "/Developer/Panda3D")
        oscmd("cp -R %s/pandac                dstroot/pythoncode/Developer/Panda3D/pandac" % GetOutputDir())
        oscmd("cp -R direct/src/*             dstroot/pythoncode/Developer/Panda3D/direct")
        oscmd("ln -s %s                       dstroot/pythoncode/usr/bin/ppython" % SDK["PYTHONEXEC"])
        if os.path.isdir(GetOutputDir()+"/Pmw"):
            oscmd("cp -R %s/Pmw               dstroot/pythoncode/Developer/Panda3D/Pmw" % GetOutputDir())
            compileall.compile_dir("dstroot/pythoncode/Developer/Panda3D/Pmw")
        WriteFile("dstroot/pythoncode/Developer/Panda3D/direct/__init__.py", "")
        for base in os.listdir("dstroot/pythoncode/Developer/Panda3D/direct"):
            if ((base != "extensions") and (base != "extensions_native")):
                compileall.compile_dir("dstroot/pythoncode/Developer/Panda3D/direct/"+base)

    oscmd("mkdir -p dstroot/headers/Developer/Panda3D")
    oscmd("cp -R %s/include               dstroot/headers/Developer/Panda3D/include" % GetOutputDir())

    if os.path.isdir("samples"):
        oscmd("mkdir -p dstroot/samples/Developer/Examples/Panda3D")
        oscmd("cp -R samples/* dstroot/samples/Developer/Examples/Panda3D/")

    oscmd("chmod -R 0775 dstroot/*")
    DeleteCVS("dstroot")
    DeleteBuildFiles("dstroot")
    # We need to be root to perform a chown. Bleh.
    # Fortunately PackageMaker does it for us, on 10.5 and above.
    #oscmd("chown -R root:admin dstroot/*", True)

    oscmd("mkdir -p dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/")
    oscmd("mkdir -p dstroot/Panda3D/Panda3D.mpkg/Contents/Resources/en.lproj/")

    pkgs = ["base", "tools", "headers"]
    if PkgSkip("PYTHON")==0:     pkgs.append("pythoncode")
    if os.path.isdir("samples"): pkgs.append("samples")
    for pkg in pkgs:
        plist = open("/tmp/Info_plist", "w")
        plist.write(Info_plist % { "package_id" : "org.panda3d.panda3d.%s.pkg" % pkg, "version" : VERSION })
        plist.close()
        if not os.path.isdir("dstroot/" + pkg):
            os.makedirs("dstroot/" + pkg)

        if os.path.exists("/Developer/usr/bin/packagemaker"):
            cmd = '/Developer/usr/bin/packagemaker --info /tmp/Info_plist --version ' + VERSION + ' --out dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/' + pkg + '.pkg --target 10.4 --domain system --root dstroot/' + pkg + '/ --no-relocate'
        elif os.path.exists("/Applications/Xcode.app/Contents/Applications/PackageMaker.app/Contents/MacOS/PackageMaker"):
            cmd = '/Applications/Xcode.app/Contents/Applications/PackageMaker.app/Contents/MacOS/PackageMaker --info /tmp/Info_plist --version ' + VERSION + ' --out dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/' + pkg + '.pkg --target 10.4 --domain system --root dstroot/' + pkg + '/ --no-relocate'
        elif os.path.exists("/Developer/Tools/packagemaker"):
            cmd = '/Developer/Tools/packagemaker -build -f dstroot/' + pkg + '/ -p dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/' + pkg + '.pkg -i /tmp/Info_plist'
        else:
            exit("PackageMaker could not be found!")
        oscmd(cmd)

    if os.path.isfile("/tmp/Info_plist"):
        oscmd("rm -f /tmp/Info_plist")

    # Now that we've built all of the individual packages, build the metapackage.
    dist = open("dstroot/Panda3D/Panda3D.mpkg/Contents/distribution.dist", "w")
    dist.write('<?xml version="1.0" encoding="utf-8"?>\n')
    dist.write('<installer-script minSpecVersion="1.000000" authoringTool="com.apple.PackageMaker" authoringToolVersion="3.0.3" authoringToolBuild="174">\n')
    dist.write('    <title>Panda3D</title>\n')
    dist.write('    <options customize="always" allow-external-scripts="no" rootVolumeOnly="false"/>\n')
    dist.write('    <license language="en" mime-type="text/plain">%s</license>\n' % ReadFile("doc/LICENSE"))
    dist.write('    <choices-outline>\n')
    dist.write('        <line choice="base"/>\n')
    dist.write('        <line choice="tools"/>\n')
    if PkgSkip("PYTHON")==0:
        dist.write('        <line choice="pythoncode"/>\n')
    if os.path.isdir("samples"):
        dist.write('        <line choice="samples"/>\n')
    dist.write('        <line choice="headers"/>\n')
    dist.write('    </choices-outline>\n')
    dist.write('    <choice id="base" title="Panda3D Base Installation" description="This package contains the Panda3D libraries, configuration files and models/textures that are needed to use Panda3D. Location: /Developer/Panda3D/" start_enabled="false">\n')
    dist.write('        <pkg-ref id="org.panda3d.panda3d.base.pkg"/>\n')
    dist.write('    </choice>\n')
    dist.write('    <choice id="tools" title="Tools" tooltip="Useful tools and model converters to help with Panda3D development" description="This package contains the various utilities that ship with Panda3D, including packaging tools, model converters, and many more. Location: /Developer/Tools/Panda3D/">\n')
    dist.write('        <pkg-ref id="org.panda3d.panda3d.tools.pkg"/>\n')
    dist.write('    </choice>\n')
    if PkgSkip("PYTHON")==0:
        dist.write('    <choice id="pythoncode" title="Python Code" tooltip="Code you\'ll need for Python development" description="This package contains the \'direct\', \'pandac\' and \'panda3d\' python packages that are needed to do Python development with Panda3D. Location: /Developer/Panda3D/">\n')
        dist.write('        <pkg-ref id="org.panda3d.panda3d.pythoncode.pkg"/>\n')
        dist.write('    </choice>\n')
    if os.path.isdir("samples"):
        dist.write('    <choice id="samples" title="Sample Programs" tooltip="Python sample programs that use Panda3D" description="This package contains the Python sample programs that can help you with learning how to use Panda3D. Location: /Developer/Examples/Panda3D/">\n')
        dist.write('        <pkg-ref id="org.panda3d.panda3d.samples.pkg"/>\n')
        dist.write('    </choice>\n')
    dist.write('    <choice id="headers" title="C++ Header Files" tooltip="Header files for C++ development with Panda3D" description="This package contains the C++ header files that are needed in order to do C++ development with Panda3D. You don\'t need this if you want to develop in Python. Location: /Developer/Panda3D/include/" start_selected="false">\n')
    dist.write('        <pkg-ref id="org.panda3d.panda3d.headers.pkg"/>\n')
    dist.write('    </choice>\n')
    dist.write('    <pkg-ref id="org.panda3d.panda3d.base.pkg" installKBytes="%d" version="1" auth="Root">file:./Contents/Packages/base.pkg</pkg-ref>\n' % (GetDirectorySize("dstroot/base") // 1024))
    dist.write('    <pkg-ref id="org.panda3d.panda3d.tools.pkg" installKBytes="%d" version="1" auth="Root">file:./Contents/Packages/tools.pkg</pkg-ref>\n' % (GetDirectorySize("dstroot/tools") // 1024))
    if PkgSkip("PYTHON")==0:
        dist.write('    <pkg-ref id="org.panda3d.panda3d.pythoncode.pkg" installKBytes="%d" version="1" auth="Root">file:./Contents/Packages/pythoncode.pkg</pkg-ref>\n' % (GetDirectorySize("dstroot/pythoncode") // 1024))
    if os.path.isdir("samples"):
        dist.write('    <pkg-ref id="org.panda3d.panda3d.samples.pkg" installKBytes="%d" version="1" auth="Root">file:./Contents/Packages/samples.pkg</pkg-ref>\n' % (GetDirectorySize("dstroot/samples") // 1024))
    dist.write('    <pkg-ref id="org.panda3d.panda3d.headers.pkg" installKBytes="%d" version="1" auth="Root">file:./Contents/Packages/headers.pkg</pkg-ref>\n' % (GetDirectorySize("dstroot/headers") // 1024))
    dist.write('</installer-script>\n')
    dist.close()

    oscmd('hdiutil create Panda3D-rw.dmg -srcfolder dstroot/Panda3D')
    oscmd('hdiutil convert Panda3D-rw.dmg -format UDBZ -o Panda3D-%s.dmg' % VERSION)
    oscmd('rm -f Panda3D-rw.dmg')

def MakeInstallerFreeBSD():
    import compileall
    oscmd("rm -rf targetroot pkg-descr pkg-plist")
    oscmd("mkdir targetroot")

    # Invoke installpanda.py to install it into a temporary dir
    if RUNTIME:
        InstallRuntime(destdir = "targetroot", prefix = "/usr/local", outputdir = GetOutputDir())
    else:
        InstallPanda(destdir = "targetroot", prefix = "/usr/local", outputdir = GetOutputDir())

    if (not os.path.exists("/usr/sbin/pkg_create")):
        exit("Cannot create an installer without pkg_create")

    if (RUNTIME):
        descr_txt = RUNTIME_INSTALLER_PKG_DESCR_FILE[1:]
    else:
        descr_txt = INSTALLER_PKG_DESCR_FILE[1:]
    descr_txt = descr_txt.replace("VERSION", VERSION)
    if (RUNTIME):
        plist_txt = "@name panda3d-runtime-%s\n" % VERSION
    else:
        plist_txt = "@name panda3d-%s\n" % VERSION
    for root, dirs, files in os.walk("targetroot/usr/local/", True):
        for f in files:
            plist_txt += os.path.join(root, f)[21:] + "\n"

    if (not RUNTIME):
        plist_txt += "@exec /sbin/ldconfig -m /usr/local/lib\n"
        plist_txt += "@unexec /sbin/ldconfig -R\n"

        for remdir in ("lib/panda3d", "share/panda3d", "include/panda3d"):
            for root, dirs, files in os.walk("targetroot/usr/local/" + remdir, False):
                for d in dirs:
                    plist_txt += "@dirrm %s\n" % os.path.join(root, d)[21:]
            plist_txt += "@dirrm %s\n" % remdir

    WriteFile("pkg-plist", plist_txt)
    WriteFile("pkg-descr", descr_txt)
    cmd = "pkg_create"
    if GetVerbose():
        cmd += " --verbose"
    if PkgSkip("PYTHON")==0:
        # If this version of Python was installed from a package, let's mark it as dependency.
        oscmd("rm -f %s/tmp/python-pkg.txt" % GetOutputDir())
        oscmd("pkg_info -E 'python%s>=%s' > %s/tmp/python-pkg.txt" % (SDK["PYTHONVERSION"][6:9:2], SDK["PYTHONVERSION"][6:9], GetOutputDir()), True)
        if (os.path.isfile(GetOutputDir() + "/tmp/python-pkg.txt")):
            python_pkg = ReadFile(GetOutputDir() + "/tmp/python-pkg.txt").strip()
            if (python_pkg):
                cmd += " -P " + python_pkg
    cmd += " -p /usr/local -S \"%s\"" % os.path.abspath("targetroot")
    if (RUNTIME):
        cmd += " -c -\"The Panda3D free 3D engine runtime\" -o graphics/panda3d-runtime"
    else:
        cmd += " -c -\"The Panda3D free 3D engine SDK\" -o devel/panda3d"
    cmd += " -d pkg-descr -f pkg-plist panda3d-%s" % VERSION
    oscmd(cmd)

if (INSTALLER != 0):
    ProgressOutput(100.0, "Building installer")
    target = GetTarget()
    if (target == 'windows'):
        dbg = ""
        if (GetOptimize() <= 2): dbg = "-dbg"
        if GetTargetArch() == 'x64':
            if (RUNTIME):
                MakeInstallerNSIS("Panda3D-Runtime-"+VERSION+dbg+"-x64.exe", "Panda3D", "Panda3D "+VERSION, "C:\\Panda3D-"+VERSION+"-x64")
            else:
                MakeInstallerNSIS("Panda3D-"+VERSION+dbg+"-x64.exe", "Panda3D", "Panda3D "+VERSION, "C:\\Panda3D-"+VERSION+"-x64")
        else:
            if (RUNTIME):
                MakeInstallerNSIS("Panda3D-Runtime-"+VERSION+dbg+".exe", "Panda3D", "Panda3D "+VERSION, "C:\\Panda3D-"+VERSION)
            else:
                MakeInstallerNSIS("Panda3D-"+VERSION+dbg+".exe", "Panda3D", "Panda3D "+VERSION, "C:\\Panda3D-"+VERSION)
    elif (target == 'linux'):
        MakeInstallerLinux()
    elif (target == 'darwin'):
        MakeInstallerOSX()
    elif (target == 'freebsd'):
        MakeInstallerFreeBSD()
    else:
        exit("Do not know how to make an installer for this platform")

##########################################################################################
#
# Print final status report.
#
##########################################################################################

SaveDependencyCache()

WARNINGS.append("Elapsed Time: "+PrettyTime(time.time() - STARTTIME))

printStatus("Makepanda Final Status Report", WARNINGS)
print(GetColor("green") + "Build successfully finished, elapsed time: " + PrettyTime(time.time() - STARTTIME) + GetColor())
