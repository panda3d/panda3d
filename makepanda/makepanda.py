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
try:
    import sys,os,platform,time,stat,string,re,getopt,fnmatch,threading,Queue,signal,shutil
    if (sys.platform == "darwin"): import plistlib
except:
    print "You are either using an incomplete or an old version of Python!"
    print "Please install the development package of Python 2.x and try again."
    exit(1)

from makepandacore import *
from installpanda import *

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
P3DSUFFIX=""
MAJOR_VERSION=None
COREAPI_VERSION=None
OSXTARGET=None
HOST_URL="https://runtime.panda3d.org/"

if "MACOSX_DEPLOYMENT_TARGET" in os.environ:
    OSXTARGET=os.environ["MACOSX_DEPLOYMENT_TARGET"]

PkgListSet(["PYTHON", "DIRECT",                        # Python support
  "GL", "GLES", "GLES2"] + DXVERSIONS + ["TINYDISPLAY", "NVIDIACG", # 3D graphics
  "EGL",                                               # OpenGL (ES) integration
  "OPENAL", "FMODEX", "FFMPEG",                        # Multimedia
  "ODE", "PHYSX",                                      # Physics
  "SPEEDTREE",                                         # SpeedTree
  "ZLIB", "PNG", "JPEG", "TIFF", "SQUISH", "FREETYPE", # 2D Formats support
  ] + MAYAVERSIONS + MAXVERSIONS + [ "FCOLLADA",       # 3D Formats support
  "VRPN", "OPENSSL",                                   # Transport
  "FFTW", "SWSCALE",                                   # Algorithm helpers
  "ARTOOLKIT", "OPENCV", "DIRECTCAM",                  # Augmented Reality
  "NPAPI", "AWESOMIUM",                                # Browser embedding
  "GTK2", "WX", "FLTK",                                # Toolkit support
  "OSMESA", "X11", "XF86DGA", "XRANDR", "XCURSOR",     # Unix platform support
  "PANDATOOL", "PVIEW", "DEPLOYTOOLS",                 # Toolchain
  "CONTRIB"                                            # Experimental
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
        print ""
        print problem
    print ""
    print "Makepanda generates a 'built' subdirectory containing a"
    print "compiled copy of Panda3D.  Command-line arguments are:"
    print ""
    print "  --help            (print the help message you're reading now)"
    print "  --verbose         (print out more information)"
    print "  --runtime         (build a runtime build instead of an SDK build)"
    print "  --installer       (build an installer)"
    print "  --optimize X      (optimization level can be 1,2,3,4)"
    print "  --version X       (set the panda version number)"
    print "  --lzma            (use lzma compression when building Windows installer)"
    print "  --distributor X   (short string identifying the distributor of the build)"
    print "  --outputdir X     (use the specified directory instead of 'built')"
    print "  --host URL        (set the host url (runtime build only))"
    print "  --threads N       (use the multithreaded build system. see manual)"
    print "  --osxtarget N     (the OSX version number to build for (OSX only))"
    print "  --override \"O=V\"  (override dtool_config/prc option value)"
    print "  --static          (builds libraries for static linking)"
    print ""
    for pkg in PkgListGet():
        p = pkg.lower()
        print "  --use-%-9s   --no-%-9s (enable/disable use of %s)"%(p, p, pkg)
    print ""
    print "  --nothing         (disable every third-party lib)"
    print "  --everything      (enable every third-party lib)"
    print ""
    print "The simplest way to compile panda is to just type:"
    print ""
    print "  makepanda --everything"
    print ""
    os._exit(1)

def parseopts(args):
    global INSTALLER,RTDIST,RUNTIME,GENMAN,DISTRIBUTOR,VERSION
    global COMPRESSOR,THREADCOUNT,OSXTARGET,HOST_URL
    global DEBVERSION,RPMRELEASE,P3DSUFFIX
    longopts = [
        "help","distributor=","verbose","runtime","osxtarget=",
        "optimize=","everything","nothing","installer","rtdist","nocolor",
        "version=","lzma","no-python","threads=","outputdir=","override=",
        "static","host=","debversion=","rpmrelease=","p3dsuffix="]
    anything = 0
    optimize = ""
    for pkg in PkgListGet(): longopts.append("no-"+pkg.lower())
    for pkg in PkgListGet(): longopts.append("use-"+pkg.lower())
    try:
        opts, extras = getopt.getopt(args, "", longopts)
        for option,value in opts:
            if (option=="--help"): raise "usage"
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
            elif (option=="--nocolor"): DisableColors()
            elif (option=="--version"):
                VERSION=value
                if (len(VERSION.split(".")) != 3): raise "usage"
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
    except: usage(0)
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
            OSXTARGET = "10.%d" % int(OSXTARGET[-1])
        except:
            usage("Invalid setting for OSXTARGET")
    try:
        SetOptimize(int(optimize))
        assert GetOptimize() in [1, 2, 3, 4]
    except:
        usage("Invalid setting for OPTIMIZE")

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
if (sys.platform == "darwin" and OSXTARGET != None):
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
    else:
        VERSION = ParsePandaVersion("dtool/PandaVersion.pp")

if (COREAPI_VERSION is None):
    COREAPI_VERSION = VERSION

if (DEBVERSION is None):
    DEBVERSION = VERSION

MAJOR_VERSION = VERSION[:3]

if (RUNTIME or RTDIST):
    PkgDisable("PANDATOOL")

    if (DISTRIBUTOR.strip() == ""):
        exit("You must provide a valid distributor name when making a runtime or rtdist build!")

    if (not IsCustomOutputDir()):
        if (RTDIST):
            SetOutputDir("built_" + DISTRIBUTOR.strip())
        elif (RUNTIME):
            SetOutputDir("built_" + DISTRIBUTOR.strip() + "_rt")

    RTDIST_VERSION = DISTRIBUTOR.strip() + "_" + VERSION[:3]
elif (DISTRIBUTOR == ""):
    DISTRIBUTOR = "makepanda"

if (RUNTIME):
    for pkg in PkgListGet():
        if pkg not in ["OPENSSL", "ZLIB", "NPAPI", "JPEG", "PNG"]:
            PkgDisable(pkg)
        elif (PkgSkip(pkg)==1):
            exit("Runtime must be compiled with OpenSSL, ZLib, NPAPI, JPEG and PNG support!")

if (sys.platform.startswith("win")):
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

SdkLocateDirectX()
SdkLocateMaya()
SdkLocateMax()
SdkLocateMacOSX(OSXTARGET)
SdkLocatePython(RTDIST)
SdkLocateVisualStudio()
SdkLocateMSPlatform()
SdkLocatePhysX()
SdkLocateSpeedTree()

SdkAutoDisableDirectX()
SdkAutoDisableMaya()
SdkAutoDisableMax()
SdkAutoDisablePhysX()
SdkAutoDisableSpeedTree()

if (RTDIST and SDK["PYTHONVERSION"] != "python2.6" and DISTRIBUTOR == "cmu"):
    exit("The CMU rtdist distribution must be built against Python 2.6!")

########################################################################
##
## Choose a Compiler.
##
## This should also set up any environment variables needed to make
## the compiler work.
##
########################################################################

if (sys.platform == "win32"):
    SetupVisualStudioEnviron()
    COMPILER="MSVC"
else:
    CheckLinkerLibraryPath()
    COMPILER="LINUX"

builtdir = os.path.join(os.path.abspath(GetOutputDir()))
AddToPathEnv("PYTHONPATH", builtdir)
AddToPathEnv("PANDA_PRC_DIR", os.path.join(builtdir, "etc"))
if (sys.platform.startswith("win")):
    AddToPathEnv("PATH", os.path.join(builtdir, "plugins"))
    AddToPathEnv("PYTHONPATH", os.path.join(builtdir, "bin"))
else:
    AddToPathEnv("PATH", os.path.join(builtdir, "bin"))
    AddToPathEnv("PYTHONPATH", os.path.join(builtdir, "lib"))

########################################################################
##
## External includes, external libraries, and external defsyms.
##
########################################################################

if (COMPILER=="MSVC"):
    PkgDisable("X11")
    PkgDisable("XRANDR")
    PkgDisable("XF86DGA")
    PkgDisable("XCURSOR")
    PkgDisable("GLES")
    PkgDisable("GLES2")
    PkgDisable("EGL")
    if (PkgSkip("PYTHON")==0):
        IncDirectory("ALWAYS", SDK["PYTHON"] + "/include")
        LibDirectory("ALWAYS", SDK["PYTHON"] + "/libs")
    for pkg in PkgListGet():
        if (PkgSkip(pkg)==0):
            if (pkg[:4]=="MAYA"):
                IncDirectory(pkg, SDK[pkg]      + "/include")
                DefSymbol(pkg, "MAYAVERSION", pkg)
            elif (pkg[:3]=="MAX"):
                IncDirectory(pkg, SDK[pkg]      + "/include")
                IncDirectory(pkg, SDK[pkg]      + "/include/CS")
                IncDirectory(pkg, SDK[pkg+"CS"] + "/include")
                IncDirectory(pkg, SDK[pkg+"CS"] + "/include/CS")
                DefSymbol(pkg, "MAX", pkg)
            elif (pkg[:2]=="DX"):
                IncDirectory(pkg, SDK[pkg]      + "/include")
            else:
                IncDirectory(pkg, GetThirdpartyDir() + pkg.lower() + "/include")
    for pkg in DXVERSIONS:
        if (PkgSkip(pkg)==0):
            vnum=pkg[2:]
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
    LibName("GL", "opengl32.lib")
    LibName("GLES", "libgles_cm.lib")
    LibName("GLES2", "libGLESv2.lib")
    LibName("EGL", "libEGL.lib")
    LibName("MSIMG", "msimg32.lib")
    if (PkgSkip("DIRECTCAM")==0): LibName("DIRECTCAM", "strmiids.lib")
    if (PkgSkip("DIRECTCAM")==0): LibName("DIRECTCAM", "quartz.lib")
    if (PkgSkip("DIRECTCAM")==0): LibName("DIRECTCAM", "odbc32.lib")
    if (PkgSkip("DIRECTCAM")==0): LibName("DIRECTCAM", "odbccp32.lib")
    if (PkgSkip("PNG")==0):      LibName("PNG",      GetThirdpartyDir() + "png/lib/libpandapng.lib")
    if (PkgSkip("JPEG")==0):     LibName("JPEG",     GetThirdpartyDir() + "jpeg/lib/libpandajpeg.lib")
    if (PkgSkip("TIFF")==0):     LibName("TIFF",     GetThirdpartyDir() + "tiff/lib/libpandatiff.lib")
    if (PkgSkip("ZLIB")==0):     LibName("ZLIB",     GetThirdpartyDir() + "zlib/lib/libpandazlib1.lib")
    if (PkgSkip("VRPN")==0):     LibName("VRPN",     GetThirdpartyDir() + "vrpn/lib/vrpn.lib")
    if (PkgSkip("VRPN")==0):     LibName("VRPN",     GetThirdpartyDir() + "vrpn/lib/quat.lib")
    if (PkgSkip("FMODEX")==0):   LibName("FMODEX",   GetThirdpartyDir() + "fmodex/lib/fmodex_vc.lib")
    if (PkgSkip("NVIDIACG")==0): LibName("CGGL",     GetThirdpartyDir() + "nvidiacg/lib/cgGL.lib")
    if (PkgSkip("NVIDIACG")==0): LibName("CGDX9",    GetThirdpartyDir() + "nvidiacg/lib/cgD3D9.lib")
    if (PkgSkip("NVIDIACG")==0): LibName("NVIDIACG", GetThirdpartyDir() + "nvidiacg/lib/cg.lib")
    if (PkgSkip("OPENSSL")==0):  LibName("OPENSSL",  GetThirdpartyDir() + "openssl/lib/libpandassl.lib")
    if (PkgSkip("OPENSSL")==0):  LibName("OPENSSL",  GetThirdpartyDir() + "openssl/lib/libpandaeay.lib")
    if (PkgSkip("FREETYPE")==0): LibName("FREETYPE", GetThirdpartyDir() + "freetype/lib/freetype.lib")
    if (PkgSkip("FFTW")==0):     LibName("FFTW",     GetThirdpartyDir() + "fftw/lib/rfftw.lib")
    if (PkgSkip("FFTW")==0):     LibName("FFTW",     GetThirdpartyDir() + "fftw/lib/fftw.lib")
    if (PkgSkip("ARTOOLKIT")==0):LibName("ARTOOLKIT",GetThirdpartyDir() + "artoolkit/lib/libAR.lib")
    if (PkgSkip("ODE")==0):      LibName("ODE",      GetThirdpartyDir() + "ode/lib/ode.lib")
    if (PkgSkip("FCOLLADA")==0): LibName("FCOLLADA", GetThirdpartyDir() + "fcollada/lib/FCollada.lib")
    if (PkgSkip("SQUISH")==0):   LibName("SQUISH",   GetThirdpartyDir() + "squish/lib/squish.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/cv.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/highgui.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/cvaux.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/ml.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/cxcore.lib")
    if (PkgSkip("AWESOMIUM")==0):LibName("AWESOMIUM",   GetThirdpartyDir() + "awesomium/lib/Awesomium.lib")
    if (PkgSkip("FFMPEG")==0):   LibName("FFMPEG",   GetThirdpartyDir() + "ffmpeg/lib/avcodec-51-panda.lib")
    if (PkgSkip("FFMPEG")==0):   LibName("FFMPEG",   GetThirdpartyDir() + "ffmpeg/lib/avformat-50-panda.lib")
    if (PkgSkip("FFMPEG")==0):   LibName("FFMPEG",   GetThirdpartyDir() + "ffmpeg/lib/avutil-49-panda.lib")
    if (PkgSkip("SWSCALE")==0):  PkgDisable("SWSCALE")
    if (PkgSkip("OPENAL")==0):
        if (os.path.exists(GetThirdpartyDir() + "openal/lib/pandaopenal32.lib")):
            LibName("OPENAL",   GetThirdpartyDir() + "openal/lib/pandaopenal32.lib")
        else:
            LibName("OPENAL",   GetThirdpartyDir() + "openal/lib/OpenAL32.lib")
    if (PkgSkip("WX")==0):
        LibName("WX",       GetThirdpartyDir() + "wx/lib/wxbase28u.lib")
        LibName("WX",       GetThirdpartyDir() + "wx/lib/wxmsw28u_core.lib")
        DefSymbol("WX",     "__WXMSW__", "")
        DefSymbol("WX",     "_UNICODE", "")
        DefSymbol("WX",     "UNICODE", "")
    if (PkgSkip("FLTK")==0):
        LibName("FLTK",     GetThirdpartyDir() + "fltk/lib/fltk.lib")
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
        LibName("PHYSX",      SDK["PHYSX"] + "/lib/Win32/PhysXLoader.lib")
        LibName("PHYSX",      SDK["PHYSX"] + "/lib/Win32/NxCharacter.lib")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/Physics/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/PhysXLoader/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/NxCharacter/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/NxExtensions/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/Foundation/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/Cooking/include")
        # We need to be able to find NxCharacter.dll when importing code library libpandaphysx
        AddToPathEnv("PATH", SDK["PHYSX"]+"/../Bin/win32/")
    if (PkgSkip("SPEEDTREE")==0):
        win64 = (sys.platform.startswith("win") and platform.architecture()[0] == "64bit")
        if win64:
            libdir = SDK["SPEEDTREE"] + "/Lib/Windows/VC9.x64/"
            p64ext = '64'
        else:
            libdir = SDK["SPEEDTREE"] + "/Lib/Windows/VC9/"
            p64ext = ''

        debugext = ''
        if (GetOptimize() <= 2 and sys.platform.startswith("win")): debugext = "_d"
        libsuffix = "_v%s_VC90MT%s_Static%s.lib" % (
            SDK["SPEEDTREEVERSION"], p64ext, debugext)
        LibName("SPEEDTREE", "%sSpeedTreeCore%s" % (libdir, libsuffix))
        LibName("SPEEDTREE", "%sSpeedTreeForest%s" % (libdir, libsuffix))
        LibName("SPEEDTREE", "%sSpeedTree%sRenderer%s" % (libdir, SDK["SPEEDTREEAPI"], libsuffix))
        LibName("SPEEDTREE", "%sSpeedTreeRenderInterface%s" % (libdir, libsuffix))
        if (SDK["SPEEDTREEAPI"] == "OpenGL"):
            LibName("SPEEDTREE",  "%sglew32.lib" % (libdir))
        IncDirectory("SPEEDTREE", SDK["SPEEDTREE"] + "/Include")

if (COMPILER=="LINUX"):
    PkgDisable("AWESOMIUM")
    if (PkgSkip("PYTHON")==0):
        IncDirectory("ALWAYS", SDK["PYTHON"])
    if (sys.platform == "darwin"):
        if (PkgSkip("FREETYPE")==0):
          IncDirectory("FREETYPE", "/usr/X11R6/include")
          IncDirectory("FREETYPE", "/usr/X11/include")
          IncDirectory("FREETYPE", "/usr/X11/include/freetype2")
        IncDirectory("GL", "/usr/X11R6/include")

    if (os.path.isdir("/usr/PCBSD")):
        IncDirectory("ALWAYS", "/usr/PCBSD/local/include")
        LibDirectory("ALWAYS", "/usr/PCBSD/local/lib")

    if (sys.platform.startswith("freebsd")):
        IncDirectory("ALWAYS", "/usr/local/include")
        LibDirectory("ALWAYS", "/usr/local/lib")

    # Workaround for an issue where pkg-config does not include this path
    if (os.path.isdir("/usr/lib64/glib-2.0/include")):
        IncDirectory("GTK2", "/usr/lib64/glib-2.0/include")
    if (os.path.isdir("/usr/lib64/gtk-2.0/include")):
        IncDirectory("GTK2", "/usr/lib64/gtk-2.0/include")

    fcollada_libs = ("FColladaD", "FColladaSD")
    # WARNING! The order of the ffmpeg libraries matters!
    ffmpeg_libs = ("libavformat", "libavcodec", "libavutil")

    #         Name         pkg-config   libs, include(dir)s
    if (not RUNTIME):
        SmartPkgEnable("ARTOOLKIT", "",          ("AR"), "AR/ar.h")
        SmartPkgEnable("FCOLLADA",  "",          ChooseLib(*fcollada_libs), ("FCollada", "FCollada.h"))
        SmartPkgEnable("FFMPEG",    ffmpeg_libs, ffmpeg_libs, ffmpeg_libs)
        SmartPkgEnable("SWSCALE",   "libswscale", "libswscale", ("libswscale", "libswscale/swscale.h"), target_pkg = "FFMPEG")
        SmartPkgEnable("FFTW",      "",          ("fftw", "rfftw"), ("fftw.h", "rfftw.h"))
        SmartPkgEnable("FMODEX",    "",          ("fmodex"), ("fmodex", "fmodex/fmod.h"))
        SmartPkgEnable("FREETYPE",  "freetype2", ("freetype"), ("freetype2", "freetype2/freetype/freetype.h"))
        SmartPkgEnable("GL",        "gl",        ("GL"), ("GL/gl.h"), framework = "OpenGL")
        SmartPkgEnable("GLES",      "glesv1_cm", ("GLESv1_CM"), ("GLES/gl.h"), framework = "OpenGLES")
        SmartPkgEnable("GLES2",     "glesv2",    ("GLESv2"), ("GLES2/gl2.h")) #framework = "OpenGLES"?
        SmartPkgEnable("EGL",       "egl",       ("EGL"), ("EGL/egl.h"))
        SmartPkgEnable("OSMESA",    "osmesa",    ("OSMesa"), ("GL/osmesa.h"))
        SmartPkgEnable("GTK2",      "gtk+-2.0")
        SmartPkgEnable("NVIDIACG",  "",          ("Cg"), "Cg/cg.h", framework = "Cg")
        SmartPkgEnable("ODE",       "",          ("ode"), "ode/ode.h")
        SmartPkgEnable("OPENAL",    "openal",    ("openal"), "AL/al.h", framework = "OpenAL")
        SmartPkgEnable("OPENCV",    "",          ("cv", "highgui", "cvaux", "ml", "cxcore"), ("opencv", "opencv/cv.h"))
        SmartPkgEnable("SQUISH",    "",          ("squish"), "squish.h")
        SmartPkgEnable("TIFF",      "",          ("tiff"), "tiff.h")
        SmartPkgEnable("VRPN",      "",          ("vrpn", "quat"), ("vrpn", "quat.h", "vrpn/vrpn_Types.h"))
    SmartPkgEnable("JPEG",      "",          ("jpeg"), "jpeglib.h")
    SmartPkgEnable("OPENSSL",   "openssl",   ("ssl", "crypto"), ("openssl/ssl.h", "openssl/crypto.h"))
    SmartPkgEnable("PNG",       "libpng",    ("png"), "png.h")
    SmartPkgEnable("ZLIB",      "",          ("z"), "zlib.h")
    if (RTDIST and sys.platform == "darwin" and "PYTHONVERSION" in SDK):
        # Don't use the framework for the OSX rtdist build. I'm afraid it gives problems somewhere.
        SmartPkgEnable("PYTHON",    "", SDK["PYTHONVERSION"], (SDK["PYTHONVERSION"], SDK["PYTHONVERSION"] + "/Python.h"), tool = SDK["PYTHONVERSION"] + "-config")
    elif("PYTHONVERSION" in SDK and not RUNTIME):
        SmartPkgEnable("PYTHON",    "", SDK["PYTHONVERSION"], (SDK["PYTHONVERSION"], SDK["PYTHONVERSION"] + "/Python.h"), tool = SDK["PYTHONVERSION"] + "-config", framework = "Python")
    if (RTDIST):
        SmartPkgEnable("WX",    tool = "wx-config")
        SmartPkgEnable("FLTK", "", ("fltk"), ("Fl/Fl.H"), tool = "fltk-config")
    if (RUNTIME):
        if (sys.platform.startswith("freebsd")):
            SmartPkgEnable("NPAPI", "mozilla-plugin", (), ("libxul/stable", "libxul/stable/npapi.h", "nspr/prtypes.h", "nspr"))
        else:
            SmartPkgEnable("NPAPI", "mozilla-plugin", (), ("xulrunner-*/stable", "xulrunner-*/stable/npapi.h", "nspr*/prtypes.h", "nspr*"))
    if (sys.platform != "darwin"):
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
            if (pkg in PkgListGet() and PkgSkip(pkg)==1):
                exit("Runtime must be compiled with OpenSSL, ZLib, NPAPI, JPEG and PNG support!")

    if (not RUNTIME and not LocateBinary("bison")):
        exit("Could not locate bison!")
    if (not RUNTIME and not LocateBinary("flex")):
        exit("Could not locate flex!")

    for pkg in MAYAVERSIONS:
        if (PkgSkip(pkg)==0 and (pkg in SDK)):
            if (sys.platform == "darwin"):
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

    if (sys.platform == "darwin"):
        LibName("ALWAYS", "-framework AppKit")
        if (PkgSkip("OPENCV")==0):   LibName("OPENCV", "-framework QuickTime")
        LibName("AGL", "-framework AGL")
        LibName("CARBON", "-framework Carbon")
        LibName("COCOA", "-framework Cocoa")
        # Fix for a bug in OSX Leopard:
        LibName("GL", "-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")

    for pkg in MAYAVERSIONS:
        if (PkgSkip(pkg)==0 and (pkg in SDK)):
            LibName(pkg, "-Wl,-rpath," + SDK[pkg] + "/lib")
            if (sys.platform != "darwin"):
                LibName(pkg, "-lOpenMayalib")
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
            if (sys.platform == "darwin"):
                LibName(pkg, "-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
    if (PkgSkip("PHYSX")==0):
        IncDirectory("PHYSX", SDK["PHYSX"] + "/Physics/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/PhysXLoader/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/NxCharacter/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/NxExtensions/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/Foundation/include")
        IncDirectory("PHYSX", SDK["PHYSX"] + "/Cooking/include")
        LibDirectory("PHYSX", SDK["PHYSXLIBS"])
        if (sys.platform == "darwin"):
            LibName("PHYSX", SDK["PHYSXLIBS"] + "/osxstatic/PhysXCooking.a")
            LibName("PHYSX", SDK["PHYSXLIBS"] + "/osxstatic/PhysXCore.a")
        else:
            LibName("PHYSX", "-lPhysXLoader")
            LibName("PHYSX", "-lNxCharacter")

DefSymbol("ALWAYS", "MAKEPANDA", "")
DefSymbol("WITHINPANDA", "WITHIN_PANDA", "1")
IncDirectory("ALWAYS", GetOutputDir()+"/tmp")
IncDirectory("ALWAYS", GetOutputDir()+"/include")
if GetLinkAllStatic():
    DefSymbol("ALWAYS", "LINK_ALL_STATIC", "")

########################################################################
##
## Give a Status Report on Command-Line Options
##
########################################################################

def printStatus(header,warnings):
    if GetVerbose():
        print ""
        print "-------------------------------------------------------------------"
        print header
        tkeep = ""
        tomit = ""
        for x in PkgListGet():
            if (PkgSkip(x)==0): tkeep = tkeep + x + " "
            else:                  tomit = tomit + x + " "
        if RTDIST:  print "Makepanda: Runtime distribution build"
        elif RUNTIME: print "Makepanda: Runtime build"
        else:        print "Makepanda: Regular build"
        print "Makepanda: Compiler:",COMPILER
        print "Makepanda: Optimize:",GetOptimize()
        print "Makepanda: Keep Pkg:",tkeep
        print "Makepanda: Omit Pkg:",tomit
        if (GENMAN): print "Makepanda: Generate API reference manual"
        else       : print "Makepanda: Don't generate API reference manual"
        if (sys.platform == "win32" and not RTDIST):
            if INSTALLER:  print "Makepanda: Build installer, using",COMPRESSOR
            else        :  print "Makepanda: Don't build installer"
        print "Makepanda: Version ID: "+VERSION
        for x in warnings: print "Makepanda: "+x
        print "-------------------------------------------------------------------"
        print ""
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
    if (COMPILER=="MSVC"):
        cmd = "cl "
        if (platform.architecture()[0]=="64bit"):
            cmd += "/favor:blend "
        cmd += "/wd4996 /wd4275 /wd4267 /wd4101 /wd4273 "

        # Enables Windows 7 mode if SDK is detected.
        platsdk = GetRegistryKey("SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v7.0", "InstallationFolder")
        if platsdk and os.path.isdir(platsdk):
            cmd += "/DPANDA_WIN7 /DWINVER=0x601 "

        cmd += "/Fo" + obj + " /nologo /c"
        for x in ipath: cmd += " /I" + x
        for (opt,dir) in INCDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts): cmd += " /I" + BracketNameWithQuotes(dir)
        for (opt,var,val) in DEFSYMBOLS:
            if (opt=="ALWAYS") or (opt in opts): cmd += " /D" + var + "=" + val
        if (opts.count('NOFLOATWARN')): cmd += ' /wd4244 /wd4305'
        if (opts.count('MSFORSCOPE')): cmd += ' /Zc:forScope-'
        optlevel = GetOptimizeOption(opts)
        if (optlevel==1): cmd += " /MDd /Zi /RTCs /GS"
        if (optlevel==2): cmd += " /MDd /Zi"
        if (optlevel==3): cmd += " /MD /Zi /O2 /Ob2 /DFORCE_INLINING"
        if (optlevel==4): cmd += " /MD /Zi /Ox /Ob2 /DFORCE_INLINING /DNDEBUG /GL"
        cmd += " /Fd" + os.path.splitext(obj)[0] + ".pdb"
        building = GetValueOption(opts, "BUILDING:")
        if (building): cmd += " /DBUILDING_" + building
        if ("BIGOBJ" in opts): cmd += " /bigobj"
        cmd += " /EHa /Zm300 /DWIN32_VC /DWIN32 /W3 " + BracketNameWithQuotes(src)
        oscmd(cmd)
    if (COMPILER=="LINUX"):
        cc = os.environ.get('CC', 'gcc')
        cxx = os.environ.get('CXX', 'g++')
        if (src.endswith(".c")): cmd = cc +' -fPIC -c -o ' + obj
        else:                    cmd = cxx+' -ftemplate-depth-30 -fPIC -c -o ' + obj
        for (opt, dir) in INCDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts): cmd += ' -I' + BracketNameWithQuotes(dir)
        for (opt,var,val) in DEFSYMBOLS:
            if (opt=="ALWAYS") or (opt in opts): cmd += ' -D' + var + '=' + val
        for x in ipath: cmd += ' -I' + x
        if (sys.platform == "darwin"):
            cmd += " -Wno-deprecated-declarations"
            if (OSXTARGET != None):
                cmd += " -isysroot " + SDK["MACOSX"]
                cmd += " -mmacosx-version-min=" + OSXTARGET
            if int(platform.mac_ver()[0][3]) >= 6:
                cmd += " -arch i386" # 64-bits doesn't work well yet
            else:
                cmd += " -arch i386"
                if ("NOPPC" not in opts): cmd += " -arch ppc"
        cmd += " -pthread"
        optlevel = GetOptimizeOption(opts)
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
    if (COMPILER == "MSVC"):
        oscmd(GetThirdpartyBase()+'/win-util/bison -y -d -o'+GetOutputDir()+'/tmp/'+ifile+'.c -p '+pre+' '+wsrc)
        CopyFile(wdstc, GetOutputDir()+"/tmp/"+ifile+".c")
        CopyFile(wdsth, GetOutputDir()+"/tmp/"+ifile+".h")
    if (COMPILER == "LINUX"):
        oscmd("bison -y -d -o"+GetOutputDir()+"/tmp/"+ifile+".c -p "+pre+" "+wsrc)
        CopyFile(wdstc, GetOutputDir()+"/tmp/"+ifile+".c")
        CopyFile(wdsth, GetOutputDir()+"/tmp/"+ifile+".h")
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
    if (COMPILER == "MSVC"):
        if (dashi): oscmd(GetThirdpartyBase()+"/win-util/flex -i -P" + pre + " -o"+wdst+" "+wsrc)
        else:       oscmd(GetThirdpartyBase()+"/win-util/flex    -P" + pre + " -o"+wdst+" "+wsrc)
    if (COMPILER == "LINUX"):
        if (dashi): oscmd("flex -i -P" + pre + " -o"+wdst+" "+wsrc)
        else:       oscmd("flex    -P" + pre + " -o"+wdst+" "+wsrc)
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
        WriteFile(woutc,"")
        WriteFile(woutd,"")
        CompileCxx(wobj,woutc,opts)
        ConditionalWriteFile(woutd,"")
        return
    cmd = GetOutputDir()+"/bin/interrogate -srcdir "+srcdir+" -I"+srcdir
    cmd += ' -Dvolatile -Dmutable'
    if (COMPILER=="MSVC"):
        cmd += ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -D__inline -longlong __int64 -D_X86_ -DWIN32_VC -D_WIN32'
        #NOTE: this 1500 value is the version number for VC2008.
        cmd += ' -D_MSC_VER=1500 -D"_declspec(param)=" -D_near -D_far -D__near -D__far -D__stdcall'
    #FIXME: allow 64-bits on OSX
    if (COMPILER=="LINUX") and (platform.architecture()[0]=="64bit") and (sys.platform!="darwin"):
        cmd += ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -D__inline -D__const=const -D_LP64'
    if (COMPILER=="LINUX") and (platform.architecture()[0]=="32bit" or sys.platform=="darwin"):
        cmd += ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -D__inline -D__const=const -D__i386__'
    optlevel=GetOptimizeOption(opts)
    if (optlevel==1): cmd += ' -D_DEBUG'
    if (optlevel==2): cmd += ' -D_DEBUG'
    if (optlevel==3): pass
    if (optlevel==4): cmd += ' -DNDEBUG'
    cmd += ' -oc ' + woutc + ' -od ' + woutd
    cmd += ' -fnames -string -refcount -assert -python-native'
    cmd += ' -S' + GetOutputDir() + '/include/parser-inc'
    for x in ipath: cmd += ' -I' + BracketNameWithQuotes(x)
    for (opt,dir) in INCDIRECTORIES:
        if (opt=="ALWAYS") or (opt in opts): cmd += ' -S' + BracketNameWithQuotes(dir)
    for (opt,var,val) in DEFSYMBOLS:
        if (opt=="ALWAYS") or (opt in opts): cmd += ' -D' + var + '=' + val
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
    if (COMPILER=="LINUX"):
        woutc = wobj[:-2]+".cxx"
    if (PkgSkip("PYTHON")):
        WriteFile(woutc,"")
        CompileCxx(wobj,woutc,opts)
        return
    cmd = GetOutputDir() + '/bin/interrogate_module '
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
        cmd = 'link /lib /nologo '
        if (platform.architecture()[0] == "64bit"):
            cmd += "/MACHINE:X64 "
        cmd += '/OUT:' + BracketNameWithQuotes(lib)
        for x in obj: cmd += ' ' + BracketNameWithQuotes(x)
        oscmd(cmd)
    if (COMPILER=="LINUX"):
        if sys.platform == 'darwin':
            cmd = 'libtool -static -o ' + BracketNameWithQuotes(lib)
        else:
            cmd = 'ar cru ' + BracketNameWithQuotes(lib)
        for x in obj: cmd=cmd + ' ' + BracketNameWithQuotes(x)
        oscmd(cmd)

        oscmd("ranlib " + BracketNameWithQuotes(lib))

########################################################################
##
## CompileLink
##
########################################################################

def CompileLink(dll, obj, opts):
    if (COMPILER=="MSVC"):
        cmd = "link /nologo"
        if (platform.architecture()[0] == "64bit"):
            cmd += " /MACHINE:X64"
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
            if (optlevel<=2): cmd += " /NOD:MSVCRTD.LIB mfcs90d.lib MSVCRTD.lib"
            else: cmd += " /NOD:MSVCRT.LIB mfcs90.lib MSVCRT.lib"
        cmd += " /FIXED:NO /OPT:REF /STACK:4194304 /INCREMENTAL:NO "
        cmd += ' /OUT:' + BracketNameWithQuotes(dll)
        subsystem = GetValueOption(opts, "SUBSYSTEM:")
        if (subsystem): cmd += " /SUBSYSTEM:" + subsystem
        if (dll.endswith(".dll")):
            cmd += ' /IMPLIB:' + GetOutputDir() + '/lib/'+os.path.splitext(os.path.basename(dll))[0]+".lib"
        for (opt, dir) in LIBDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts): cmd += ' /LIBPATH:' + BracketNameWithQuotes(dir)
        for x in obj:
            if (x.endswith(".dll")):
                cmd += ' ' + GetOutputDir() + '/lib/' + os.path.splitext(os.path.basename(x))[0] + ".lib"
            elif (x.endswith(".lib")):
                dname = os.path.splitext(dll)[0]+".dll"
                if (GetOrigExt(x) != ".ilb" and os.path.exists(GetOutputDir()+"/bin/" + os.path.splitext(os.path.basename(x))[0] + ".dll")):
                    exit("Error: in makepanda, specify "+dname+", not "+x)
                cmd += ' ' + BracketNameWithQuotes(x)
            elif (x.endswith(".def")):
                cmd += ' /DEF:' + BracketNameWithQuotes(x)
            elif (x.endswith(".dat")):
                pass
            else: cmd += ' ' + BracketNameWithQuotes(x)
        if (GetOrigExt(dll)==".exe" and "NOICON" not in opts):
            cmd += " " + GetOutputDir() + "/tmp/pandaIcon.res"
        for (opt, name) in LIBNAMES:
            if (opt=="ALWAYS") or (opt in opts): cmd += " " + BracketNameWithQuotes(name)
        oscmd(cmd)
        SetVC90CRTVersion(dll+".manifest")
        mtcmd = "mt -manifest " + dll + ".manifest -outputresource:" + dll
        if (dll.endswith(".exe")==0): mtcmd = mtcmd + ";2"
        else:                          mtcmd = mtcmd + ";1"
        oscmd(mtcmd)
    if (COMPILER=="LINUX"):
        cxx = os.environ.get('CXX', 'g++')
        if (GetOrigExt(dll)==".exe"): cmd = cxx + ' -o ' + dll + ' -L' + GetOutputDir() + '/lib -L' + GetOutputDir() + '/tmp -L/usr/X11R6/lib'
        else:
            if (sys.platform == "darwin"):
                cmd = cxx + ' -undefined dynamic_lookup'
                if ("BUNDLE" in opts): cmd += ' -bundle '
                else:
                    cmd += ' -dynamiclib -install_name ' + os.path.basename(dll)
                    cmd += ' -compatibility_version ' + MAJOR_VERSION + ' -current_version ' + VERSION
                cmd += ' -o ' + dll + ' -L' + GetOutputDir() + '/lib -L' + GetOutputDir() + '/tmp -L/usr/X11R6/lib'
            else:
                cmd = cxx + ' -shared'
                if ("MODULE" not in opts): cmd += " -Wl,-soname=" + os.path.basename(dll)
                cmd += ' -o ' + dll + ' -L' + GetOutputDir() + '/lib -L' + GetOutputDir() + '/tmp -L/usr/X11R6/lib'
        for x in obj:
            if (GetOrigExt(x) != ".dat"):
                base = os.path.basename(x)
                if (base[-3:]==".so") and (base[:3]=="lib"):
                    cmd += ' -l' + base[3:-3]
                elif (base[-2:]==".a") and (base[:3]=="lib"):
                    cmd += ' -l' + base[3:-2]
                else:
                    cmd += ' ' + x

        if (sys.platform == "darwin"):
            cmd += " -headerpad_max_install_names"
            if (OSXTARGET != None):
                cmd += " -isysroot " + SDK["MACOSX"] + " -Wl,-syslibroot," + SDK["MACOSX"]
                cmd += " -mmacosx-version-min=" + OSXTARGET
            if int(platform.mac_ver()[0][3]) >= 6:
                cmd += " -arch i386" # 64-bits doesn't work well yet
            else:
                cmd += " -arch i386"
                if ("NOPPC" not in opts): cmd += " -arch ppc"
        if (LDFLAGS !=""): cmd += " " + LDFLAGS

        for (opt, dir) in LIBDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts): cmd += ' -L' + BracketNameWithQuotes(dir)
        for (opt, name) in LIBNAMES:
            if (opt=="ALWAYS") or (opt in opts): cmd += ' ' + BracketNameWithQuotes(name)
        cmd += " -pthread"
        if (not sys.platform.startswith("freebsd")):
            cmd += " -ldl"

        oscmd(cmd)
        if (GetOrigExt(dll)==".exe" and GetOptimizeOption(opts)==4 and "NOSTRIP" not in opts):
            oscmd("strip " + BracketNameWithQuotes(dll))
        os.system("chmod +x " + BracketNameWithQuotes(dll))

        if dll.endswith("." + MAJOR_VERSION + ".dylib"):
            newdll = dll[:-6-len(MAJOR_VERSION)] + "dylib"
            if (os.path.isfile(newdll)):
                os.remove(newdll)
            oscmd("ln -s " + BracketNameWithQuotes(os.path.basename(dll)) + " " + BracketNameWithQuotes(newdll))
        elif dll.endswith("." + MAJOR_VERSION):
            newdll = dll[:-len(MAJOR_VERSION)-1]
            if (os.path.isfile(newdll)):
                os.remove(newdll)
            oscmd("ln -s " + BracketNameWithQuotes(os.path.basename(dll)) + " " + BracketNameWithQuotes(newdll))

##########################################################################################
#
# CompileEggPZ
#
##########################################################################################

def CompileEggPZ(eggpz, src, opts):
    if (src.endswith(".egg")):
        CopyFile(eggpz[:-3], src)
    elif (src.endswith(".flt")):
        oscmd(GetOutputDir()+"/bin/flt2egg -ps keep -o " + BracketNameWithQuotes(eggpz[:-3]) + " " + BracketNameWithQuotes(src))
    oscmd(GetOutputDir()+"/bin/pzip " + BracketNameWithQuotes(eggpz[:-3]))

##########################################################################################
#
# CompileResource
#
##########################################################################################

def CompileResource(target, src, opts):
    ipath = GetListOption(opts, "DIR:")
    if (COMPILER=="MSVC"):
        cmd = "rc"
        cmd += " /Fo" + BracketNameWithQuotes(target)
        for x in ipath: cmd += " /I" + x
        for (opt,dir) in INCDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts): cmd += " /I" + BracketNameWithQuotes(dir)
        for (opt,var,val) in DEFSYMBOLS:
            if (opt=="ALWAYS") or (opt in opts): cmd += " /D" + var + "=" + val
        cmd += " " + BracketNameWithQuotes(src)

        oscmd(cmd)

    elif (sys.platform == "darwin"):
        cmd = "/Developer/Tools/Rez -useDF"
        cmd += " -o " + BracketNameWithQuotes(target)
        for x in ipath: cmd += " -i " + x
        for (opt,dir) in INCDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts): cmd += " -i " + BracketNameWithQuotes(dir)
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
    if (PkgSkip("PYTHON") != 0): return

    cmdstr = SDK["PYTHONEXEC"] + " " + os.path.join("direct", "src", "ffi", "jGenPyCode.py")
    if (GENMAN): cmdstr += " -d"
    cmdstr += " -r"
    for i in inputs:
        if (GetOrigExt(i)==".dll"):
            cmdstr += " " + os.path.basename(os.path.splitext(i)[0].replace("_d","").replace(GetOutputDir()+"/lib/",""))

    oscmd(cmdstr)

##########################################################################################
#
# FreezePy
#
##########################################################################################

def FreezePy(target, inputs, opts):
    assert len(inputs) > 0
    # Make sure this function isn't called before genpycode is run.
    cmdstr = SDK["PYTHONEXEC"] + " " + os.path.join("direct", "src", "showutil", "pfreeze.py")
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
        exit("")

##########################################################################################
#
# Package
#
##########################################################################################

def Package(target, inputs, opts):
    assert len(inputs) == 1
    # Invoke the ppackage script.
    command = SDK["PYTHONEXEC"]
    if (GetOptimizeOption(opts) >= 4):
        command += " -OO"
    command += " direct/src/p3d/ppackage.py -u"
    if (sys.platform == "darwin" and "MACOSX" in SDK and SDK["MACOSX"] != None and len(SDK["MACOSX"]) > 1):
        command += " -R \"%s\"" % SDK["MACOSX"]
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
    if (sys.platform != "darwin"): return
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
        if (origsuffix==".exe" and not sys.platform.startswith("win")):
            os.system("chmod +x \"%s\"" % target)
        return
    elif (target == "pandac/PandaModules.py"):
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
    elif origsuffix in SUFFIX_DLL or (origsuffix==".plugin" and sys.platform != "darwin"):
        if (origsuffix==".exe"):
            ProgressOutput(progress, "Linking executable", target)
        else:
            ProgressOutput(progress, "Linking dynamic library", target)

        # Add version number to the dynamic library, on unix
        if (origsuffix==".dll" and "MODULE" not in opts and not sys.platform.startswith("win") and not RTDIST):
            if (sys.platform == "darwin"):
                if (target.lower().endswith(".dylib")):
                    target = target[:-5] + MAJOR_VERSION + ".dylib"
                    SetOrigExt(target, origsuffix)
            else:
                target = target + "." + MAJOR_VERSION
                SetOrigExt(target, origsuffix)
        return CompileLink(target, inputs, opts)
    elif (origsuffix==".in"):
        ProgressOutput(progress, "Building Interrogate database", target)
        return CompileIgate(target, inputs, opts)
    elif (origsuffix==".plugin" and sys.platform == "darwin"):
        ProgressOutput(progress, "Building plugin bundle", target)
        return CompileBundle(target, inputs, opts)
    elif (origsuffix==".app"):
        ProgressOutput(progress, "Building application bundle", target)
        return CompileBundle(target, inputs, opts)
    elif (origsuffix==".pz"):
        ProgressOutput(progress, "Compressing", target)
        return CompileEggPZ(target, infile, opts)
    elif (origsuffix in [".res", ".rsrc"]):
        ProgressOutput(progress, "Building resource object", target)
        return CompileResource(target, infile, opts)
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
        elif (infile.endswith(".rc") or infile.endswith(".r")):
            ProgressOutput(progress, "Building resource object", target)
            return CompileResource(target, infile, opts)
    exit("Don't know how to compile: "+target)

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
    ("HAVE_MAYA",                      '1',                      '1'),
    ("MAYA_PRE_5_0",                   'UNDEF',                  'UNDEF'),
    ("HAVE_SOFTIMAGE",                 'UNDEF',                  'UNDEF'),
    ("SSL_097",                        'UNDEF',                  'UNDEF'),
    ("REPORT_OPENSSL_ERRORS",          '1',                      '1'),
    ("USE_PANDAFILESTREAM",            '1',                      '1'),
    ("USE_DELETED_CHAIN",              '1',                      '1'),
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
    ("SIMPLE_THREADS",                 '1',                      '1'),
    ("OS_SIMPLE_THREADS",              '1',                      '1'),
    ("DEBUG_THREADS",                  'UNDEF',                  'UNDEF'),
    ("HAVE_POSIX_THREADS",             'UNDEF',                  '1'),
    ("HAVE_AUDIO",                     '1',                      '1'),
    ("NOTIFY_DEBUG",                   'UNDEF',                  'UNDEF'),
    ("DO_PSTATS",                      'UNDEF',                  'UNDEF'),
    ("DO_COLLISION_RECORDING",         'UNDEF',                  'UNDEF'),
    ("SUPPORT_IMMEDIATE_MODE",         '1',                      '1'),
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
    ("USE_STL_ALLOCATOR",              '1',                      '1'),
    ("USE_MEMORY_DLMALLOC",            '1',                      'UNDEF'),
    ("USE_MEMORY_PTMALLOC",            'UNDEF',                  'UNDEF'),
    ("USE_MEMORY_MALLOC",              'UNDEF',                  '1'),
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
    ("HAVE_VRPN",                      'UNDEF',                  'UNDEF'),
    ("HAVE_FMODEX",                    'UNDEF',                  'UNDEF'),
    ("HAVE_OPENAL",                    'UNDEF',                  'UNDEF'),
    ("HAVE_NVIDIACG",                  'UNDEF',                  'UNDEF'),
    ("HAVE_FREETYPE",                  'UNDEF',                  'UNDEF'),
    ("HAVE_FFTW",                      'UNDEF',                  'UNDEF'),
    ("HAVE_OPENSSL",                   'UNDEF',                  'UNDEF'),
    ("HAVE_NET",                       'UNDEF',                  'UNDEF'),
    ("HAVE_EGG",                       '1',                      '1'),
    ("HAVE_CG",                        'UNDEF',                  'UNDEF'),
    ("HAVE_CGGL",                      'UNDEF',                  'UNDEF'),
    ("HAVE_CGDX9",                     'UNDEF',                  'UNDEF'),
    ("HAVE_FFMPEG",                    'UNDEF',                  'UNDEF'),
    ("HAVE_SWSCALE",                   'UNDEF',                  'UNDEF'),
    ("HAVE_ARTOOLKIT",                 'UNDEF',                  'UNDEF'),
    ("HAVE_ODE",                       'UNDEF',                  'UNDEF'),
    ("HAVE_OPENCV",                    'UNDEF',                  'UNDEF'),
    ("HAVE_DIRECTCAM",                 'UNDEF',                  'UNDEF'),
    ("HAVE_SQUISH",                    'UNDEF',                  'UNDEF'),
    ("HAVE_FCOLLADA",                  'UNDEF',                  'UNDEF'),
    ("HAVE_OPENAL_FRAMEWORK",          'UNDEF',                  'UNDEF'),
    ("PRC_SAVE_DESCRIPTIONS",          '1',                      '1'),
    ("_SECURE_SCL",                    '1',                      'UNDEF'),
    ("_SECURE_SCL_THROWS",             '0',                      'UNDEF'),
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

    if (sys.platform.startswith("win")):
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

    if (not sys.platform.startswith("linux")):
        dtool_config["HAVE_PROC_SELF_EXE"] = 'UNDEF'
        dtool_config["HAVE_PROC_SELF_MAPS"] = 'UNDEF'
        dtool_config["HAVE_PROC_SELF_CMDLINE"] = 'UNDEF'
        dtool_config["HAVE_PROC_SELF_ENVIRON"] = 'UNDEF'

    if (sys.platform == "darwin"):
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

    if (sys.platform.startswith("freebsd")):
        dtool_config["IS_LINUX"] = 'UNDEF'
        dtool_config["HAVE_VIDEO4LINUX"] = 'UNDEF'
        dtool_config["IS_FREEBSD"] = '1'
        dtool_config["PHAVE_ALLOCA_H"] = 'UNDEF'
        dtool_config["PHAVE_MALLOC_H"] = 'UNDEF'
        dtool_config["PHAVE_LINUX_INPUT_H"] = 'UNDEF'
        dtool_config["HAVE_PROC_CURPROC_FILE"] = '1'
        dtool_config["HAVE_PROC_CURPROC_MAP"] = '1'
        dtool_config["HAVE_PROC_CURPROC_CMDLINE"] = '1'

    if (GetOptimize() <= 2 and sys.platform.startswith("win")):
        dtool_config["USE_DEBUG_PYTHON"] = '1'

    if (GetOptimize() <= 3):
        if (dtool_config["HAVE_NET"] != 'UNDEF'):
            dtool_config["DO_PSTATS"] = '1'

    if (GetOptimize() <= 3):
        dtool_config["DO_COLLISION_RECORDING"] = '1'

    #if (GetOptimize() <= 2):
    #    dtool_config["TRACK_IN_INTERPRETER"] = '1'

    if (GetOptimize() <= 3):
        dtool_config["DO_MEMORY_USAGE"] = '1'

    #if (GetOptimize() <= 1):
    #    dtool_config["DO_PIPELINING"] = '1'

    if (GetOptimize() <= 3):
        dtool_config["NOTIFY_DEBUG"] = '1'

    # Now that we have OS_SIMPLE_THREADS, we can support
    # SIMPLE_THREADS on exotic architectures like win64, so we no
    # longer need to disable it for this platform.
##     if (sys.platform.startswith("win") and platform.architecture()[0] == "64bit"):
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

    if (RUNTIME):
        dtool_config["HAVE_P3D_PLUGIN"] = '1'

    # Whether it's present on the system doesn't matter here,
    # as the runtime itself doesn't include or link with X11.
    if (RUNTIME and sys.platform != "darwin" and not sys.platform.startswith("win")):
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
    for key in prc_parameters.keys():
        if ((key == "DEFAULT_PRC_DIR") or (key[:4]=="PRC_")):
            val = OverrideValue(key, prc_parameters[key])
            if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
            else:                conf = conf + "#define " + key + " " + val + "\n"
            del prc_parameters[key]
    ConditionalWriteFile(GetOutputDir() + '/include/prc_parameters.h', conf)

    conf = "/* dtool_config.h.  Generated automatically by makepanda.py */\n"
    for key in dtool_config.keys():
        val = OverrideValue(key, dtool_config[key])
        if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
        else:                conf = conf + "#define " + key + " " + val + "\n"
        del dtool_config[key]
    ConditionalWriteFile(GetOutputDir() + '/include/dtool_config.h', conf)

    if (RTDIST or RUNTIME):
        conf = "/* p3d_plugin_config.h.  Generated automatically by makepanda.py */\n"
        for key in plugin_config.keys():
            val = plugin_config[key]
            if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
            else:                conf = conf + "#define " + key + " \"" + val.replace("\\", "\\\\") + "\"\n"
            del plugin_config[key]
        ConditionalWriteFile(GetOutputDir() + '/include/p3d_plugin_config.h', conf)

    if (PkgSkip("SPEEDTREE")==0):
        conf = "/* speedtree_parameters.h.  Generated automatically by makepanda.py */\n"
        for key in speedtree_parameters.keys():
            val = OverrideValue(key, speedtree_parameters[key])
            if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
            else:                conf = conf + "#define " + key + " \"" + val.replace("\\", "\\\\") + "\"\n"
            del speedtree_parameters[key]
        ConditionalWriteFile(GetOutputDir() + '/include/speedtree_parameters.h', conf)

    for x in PkgListGet():
        if (PkgSkip(x)): ConditionalWriteFile(GetOutputDir() + '/tmp/dtool_have_'+x.lower()+'.dat',"0\n")
        else:            ConditionalWriteFile(GetOutputDir() + '/tmp/dtool_have_'+x.lower()+'.dat',"1\n")

WriteConfigSettings()

MoveAwayConflictingFiles()
if "libdtoolbase" in GetLibCache():
    print "%sWARNING:%s Found conflicting Panda3D libraries from other ppremake build!" % (GetColor("red"), GetColor())
if "libp3dtoolconfig" in GetLibCache():
    print "%sWARNING:%s Found conflicting Panda3D libraries from other makepanda build!" % (GetColor("red"), GetColor())

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
# Copy the "direct" tree and panda3d.py
#
##########################################################################################

if (PkgSkip("PYTHON")==0 and PkgSkip("DIRECT")==0):
    CopyTree(GetOutputDir()+'/direct', 'direct/src')
    ConditionalWriteFile(GetOutputDir() + '/direct/__init__.py', "")
    if (sys.platform.startswith("win")):
        CopyFile(GetOutputDir()+'/bin/panda3d.py', 'direct/src/ffi/panda3d.py')
    else:
        CopyFile(GetOutputDir()+'/lib/panda3d.py', 'direct/src/ffi/panda3d.py')

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

if (sys.platform.startswith("win")):
    configprc = configprc.replace("$HOME/.panda3d", "$USER_APPDATA/Panda3D-%s" % MAJOR_VERSION)
else:
    configprc = configprc.replace("aux-display pandadx9", "")
    configprc = configprc.replace("aux-display pandadx8", "")

if (sys.platform == "darwin"):
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

for pkg in PkgListGet():
    if (PkgSkip(pkg)==0):
        if (COMPILER == "MSVC"):
            if (os.path.exists(GetThirdpartyDir()+pkg.lower()+"/bin")):
                CopyAllFiles(GetOutputDir()+"/bin/",GetThirdpartyDir()+pkg.lower()+"/bin/")
        if (COMPILER == "LINUX"):
            if (os.path.exists(GetThirdpartyDir()+pkg.lower()+"/lib")):
                CopyAllFiles(GetOutputDir()+"/lib/",GetThirdpartyDir()+pkg.lower()+"/lib/")

if (COMPILER=="MSVC"):
    CopyAllFiles(GetOutputDir()+"/bin/", GetThirdpartyDir()+"extras"+"/bin/")
if (sys.platform.startswith("win")):
    if (PkgSkip("PYTHON")==0):
        pydll = "/" + SDK["PYTHONVERSION"].replace(".", "")
        if (GetOptimize() <= 2): pydll += "_d.dll"
        else: pydll += ".dll"
        CopyFile(GetOutputDir()+"/bin"+pydll, SDK["PYTHON"]+pydll)
        if not RTDIST:
            CopyTree(GetOutputDir()+"/python", SDK["PYTHON"])
            ConditionalWriteFile(GetOutputDir()+"/python/panda.pth", "..\n../bin\n")

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
CopyAllFiles(GetOutputDir()+'/include/parser-inc/openssl/','dtool/src/parser-inc/')
CopyAllFiles(GetOutputDir()+'/include/parser-inc/netinet/','dtool/src/parser-inc/')
CopyFile(GetOutputDir()+'/include/parser-inc/Cg/','dtool/src/parser-inc/cg.h')
CopyFile(GetOutputDir()+'/include/parser-inc/Cg/','dtool/src/parser-inc/cgGL.h')
CopyFile(GetOutputDir()+'/include/parser-inc/Core/','dtool/src/parser-inc/Core.h')
CopyFile(GetOutputDir()+'/include/parser-inc/Forest/','dtool/src/parser-inc/Forest.h')
CopyFile(GetOutputDir()+'/include/parser-inc/Renderers/OpenGL/','dtool/src/parser-inc/OpenGLRenderer.h')
CopyFile(GetOutputDir()+'/include/parser-inc/Renderers/DirectX9/','dtool/src/parser-inc/DirectX9Renderer.h')
CopyFile(GetOutputDir()+'/include/parser-inc/glew/','dtool/src/parser-inc/glew.h')
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
CopyAllHeaders('panda/src/lerp')
CopyAllHeaders('panda/src/pgraphnodes')
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
CopyAllHeaders('panda/src/vision')
CopyAllHeaders('panda/src/awesomium')
CopyAllHeaders('panda/src/tform')
CopyAllHeaders('panda/src/collide')
CopyAllHeaders('panda/src/parametrics')
CopyAllHeaders('panda/src/pgui')
CopyAllHeaders('panda/src/pnmimagetypes')
CopyAllHeaders('panda/src/recorder')
CopyAllHeaders('panda/src/vrpn')
CopyAllHeaders('panda/src/wgldisplay')
CopyAllHeaders('panda/src/ode')
CopyAllHeaders('panda/metalibs/pandaode')
CopyAllHeaders('panda/src/physics')
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
if (sys.platform.startswith("win")):
    CopyAllHeaders('panda/src/wgldisplay')
elif (sys.platform == "darwin"):
    CopyAllHeaders('panda/src/osxdisplay')
else:
    CopyAllHeaders('panda/src/x11display')
    CopyAllHeaders('panda/src/glxdisplay')
CopyAllHeaders('panda/src/egldisplay')
CopyAllHeaders('panda/metalibs/pandagl')
CopyAllHeaders('panda/metalibs/pandagles')
CopyAllHeaders('panda/metalibs/pandagles2')

CopyAllHeaders('panda/src/physics')
CopyAllHeaders('panda/src/particlesystem')
CopyAllHeaders('panda/metalibs/pandaphysics')
CopyAllHeaders('panda/src/testbed')

if (PkgSkip("PHYSX")==0):
    CopyAllHeaders('panda/src/physx')
    CopyAllHeaders('panda/metalibs/pandaphysx')

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
    CopyAllHeaders('direct/src/heapq')

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
    'libeggbase.lib',
    'libprogbase.lib',
    'libconverter.lib',
    'libpandatoolbase.lib',
    'libpandaegg.dll',
] + COMMON_PANDA_LIBS

COMMON_DTOOL_LIBS_PYSTUB = COMMON_DTOOL_LIBS + ['libp3pystub.dll']
COMMON_PANDA_LIBS_PYSTUB = COMMON_PANDA_LIBS + ['libp3pystub.dll']
COMMON_EGG2X_LIBS_PYSTUB = COMMON_EGG2X_LIBS + ['libp3pystub.dll']

########################################################################
#
# This section contains a list of all the files that need to be compiled.
#
########################################################################

print "Generating dependencies..."
sys.stdout.flush()

#
# Compile Panda icon resource file.
# We do it first because we need it at
# the time we compile an executable.
#

if (sys.platform.startswith("win")):
  OPTS=['DIR:panda/src/configfiles']
  TargetAdd('pandaIcon.res', opts=OPTS, input='pandaIcon.rc')

#
# DIRECTORY: dtool/src/dtoolbase/
#

OPTS=['DIR:dtool/src/dtoolbase', 'BUILDING:DTOOL']
TargetAdd('dtoolbase_composite1.obj', opts=OPTS, input='dtoolbase_composite1.cxx')
TargetAdd('dtoolbase_composite2.obj', opts=OPTS, input='dtoolbase_composite2.cxx')
TargetAdd('dtoolbase_lookup3.obj',    opts=OPTS, input='lookup3.c')
TargetAdd('dtoolbase_indent.obj',     opts=OPTS, input='indent.cxx')

#
# DIRECTORY: dtool/src/dtoolutil/
#

OPTS=['DIR:dtool/src/dtoolutil', 'BUILDING:DTOOL']
TargetAdd('dtoolutil_gnu_getopt.obj',  opts=OPTS, input='gnu_getopt.c')
TargetAdd('dtoolutil_gnu_getopt1.obj', opts=OPTS, input='gnu_getopt1.c')
TargetAdd('dtoolutil_composite.obj',   opts=OPTS, input='dtoolutil_composite.cxx')
if (sys.platform == 'darwin'):
  TargetAdd('dtoolutil_filename_assist.obj',   opts=OPTS, input='filename_assist.mm')

#
# DIRECTORY: dtool/metalibs/dtool/
#

OPTS=['DIR:dtool/metalibs/dtool', 'BUILDING:DTOOL']
TargetAdd('dtool_dtool.obj', opts=OPTS, input='dtool.cxx')
TargetAdd('libp3dtool.dll', input='dtool_dtool.obj')
TargetAdd('libp3dtool.dll', input='dtoolutil_gnu_getopt.obj')
TargetAdd('libp3dtool.dll', input='dtoolutil_gnu_getopt1.obj')
TargetAdd('libp3dtool.dll', input='dtoolutil_composite.obj')
if (sys.platform == 'darwin'):
  TargetAdd('libp3dtool.dll', input='dtoolutil_filename_assist.obj')
TargetAdd('libp3dtool.dll', input='dtoolbase_composite1.obj')
TargetAdd('libp3dtool.dll', input='dtoolbase_composite2.obj')
TargetAdd('libp3dtool.dll', input='dtoolbase_indent.obj')
TargetAdd('libp3dtool.dll', input='dtoolbase_lookup3.obj')
TargetAdd('libp3dtool.dll', opts=['ADVAPI','WINSHELL','WINKERNEL'])

#
# DIRECTORY: dtool/src/cppparser/
#

if (not RUNTIME):
  OPTS=['DIR:dtool/src/cppparser', 'BISONPREFIX_cppyy']
  CreateFile(GetOutputDir()+"/include/cppBison.h")
  TargetAdd('cppParser_cppBison.obj',  opts=OPTS, input='cppBison.yxx')
  TargetAdd('cppBison.h', input='cppParser_cppBison.obj', opts=['DEPENDENCYONLY'])
  TargetAdd('cppParser_composite.obj', opts=OPTS, input='cppParser_composite.cxx')
  TargetAdd('libcppParser.ilb', input='cppParser_composite.obj')
  TargetAdd('libcppParser.ilb', input='cppParser_cppBison.obj')

#
# DIRECTORY: dtool/src/prc/
#

OPTS=['DIR:dtool/src/prc', 'BUILDING:DTOOLCONFIG', 'OPENSSL']
TargetAdd('prc_composite.obj', opts=OPTS, input='prc_composite.cxx')

#
# DIRECTORY: dtool/src/dconfig/
#

OPTS=['DIR:dtool/src/dconfig', 'BUILDING:DTOOLCONFIG']
TargetAdd('dconfig_composite.obj', opts=OPTS, input='dconfig_composite.cxx')

#
# DIRECTORY: dtool/src/interrogatedb/
#

OPTS=['DIR:dtool/src/interrogatedb', 'BUILDING:DTOOLCONFIG']
TargetAdd('interrogatedb_composite.obj', opts=OPTS, input='interrogatedb_composite.cxx')

#
# DIRECTORY: dtool/metalibs/dtoolconfig/
#

OPTS=['DIR:dtool/metalibs/dtoolconfig', 'BUILDING:DTOOLCONFIG']
if (PkgSkip("PYTHON")):
  TargetAdd('dtoolconfig_pydtool.obj', opts=OPTS, input="null.cxx")
else:
  TargetAdd('dtoolconfig_pydtool.obj', opts=OPTS, input="pydtool.cxx")
TargetAdd('dtoolconfig_dtoolconfig.obj', opts=OPTS, input='dtoolconfig.cxx')
TargetAdd('dtoolconfig_pydtool.obj', dep='dtool_have_python.dat')
TargetAdd('libp3dtoolconfig.dll', input='dtoolconfig_dtoolconfig.obj')
TargetAdd('libp3dtoolconfig.dll', input='dtoolconfig_pydtool.obj')
TargetAdd('libp3dtoolconfig.dll', input='interrogatedb_composite.obj')
TargetAdd('libp3dtoolconfig.dll', input='dconfig_composite.obj')
TargetAdd('libp3dtoolconfig.dll', input='prc_composite.obj')
TargetAdd('libp3dtoolconfig.dll', input='libp3dtool.dll')
TargetAdd('libp3dtoolconfig.dll', opts=['ADVAPI',  'OPENSSL'])

#
# DIRECTORY: dtool/src/pystub/
#

OPTS=['DIR:dtool/src/pystub', 'BUILDING:DTOOLCONFIG']
TargetAdd('pystub_pystub.obj', opts=OPTS, input='pystub.cxx')
TargetAdd('libp3pystub.dll', input='pystub_pystub.obj')
TargetAdd('libp3pystub.dll', input='libp3dtool.dll')
TargetAdd('libp3pystub.dll', opts=['ADVAPI'])

#
# DIRECTORY: dtool/src/interrogate/
#

if (not RUNTIME):
  OPTS=['DIR:dtool/src/interrogate', 'DIR:dtool/src/cppparser', 'DIR:dtool/src/interrogatedb']
  TargetAdd('interrogate_composite.obj', opts=OPTS, input='interrogate_composite.cxx')
  TargetAdd('interrogate.exe', input='interrogate_composite.obj')
  TargetAdd('interrogate.exe', input='libcppParser.ilb')
  TargetAdd('interrogate.exe', input=COMMON_DTOOL_LIBS_PYSTUB)
  TargetAdd('interrogate.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL'])

  TargetAdd('interrogate_module_interrogate_module.obj', opts=OPTS, input='interrogate_module.cxx')
  TargetAdd('interrogate_module.exe', input='interrogate_module_interrogate_module.obj')
  TargetAdd('interrogate_module.exe', input='libcppParser.ilb')
  TargetAdd('interrogate_module.exe', input=COMMON_DTOOL_LIBS_PYSTUB)
  TargetAdd('interrogate_module.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL'])

  if (not RTDIST):
    TargetAdd('parse_file_parse_file.obj', opts=OPTS, input='parse_file.cxx')
    TargetAdd('parse_file.exe', input='parse_file_parse_file.obj')
    TargetAdd('parse_file.exe', input='libcppParser.ilb')
    TargetAdd('parse_file.exe', input=COMMON_DTOOL_LIBS_PYSTUB)
    TargetAdd('parse_file.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL'])

#
# DIRECTORY: dtool/src/prckeys/
#

if (PkgSkip("OPENSSL")==0 and not RUNTIME and not RTDIST):
  OPTS=['DIR:dtool/src/prckeys', 'OPENSSL']
  TargetAdd('make-prc-key_makePrcKey.obj', opts=OPTS, input='makePrcKey.cxx')
  TargetAdd('make-prc-key.exe', input='make-prc-key_makePrcKey.obj')
  TargetAdd('make-prc-key.exe', input=COMMON_DTOOL_LIBS_PYSTUB)
  TargetAdd('make-prc-key.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL'])

#
# DIRECTORY: dtool/src/test_interrogate/
#

if (not RTDIST and not RUNTIME):
  OPTS=['DIR:dtool/src/test_interrogate']
  TargetAdd('test_interrogate_test_interrogate.obj', opts=OPTS, input='test_interrogate.cxx')
  TargetAdd('test_interrogate.exe', input='test_interrogate_test_interrogate.obj')
  TargetAdd('test_interrogate.exe', input=COMMON_DTOOL_LIBS_PYSTUB)
  TargetAdd('test_interrogate.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL'])

#
# DIRECTORY: panda/src/pandabase/
#

OPTS=['DIR:panda/src/pandabase', 'BUILDING:PANDAEXPRESS']
TargetAdd('pandabase_pandabase.obj', opts=OPTS, input='pandabase.cxx')

#
# DIRECTORY: panda/src/express/
#

OPTS=['DIR:panda/src/express', 'BUILDING:PANDAEXPRESS', 'OPENSSL', 'ZLIB']
TargetAdd('express_composite1.obj', opts=OPTS, input='express_composite1.cxx')
TargetAdd('express_composite2.obj', opts=OPTS, input='express_composite2.cxx')
IGATEFILES=GetDirectoryContents('panda/src/express', ["*.h", "*_composite.cxx"])
TargetAdd('libexpress.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libexpress.in', opts=['IMOD:pandaexpress', 'ILIB:libexpress', 'SRCDIR:panda/src/express'])
TargetAdd('libexpress_igate.obj', input='libexpress.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/downloader/
#

OPTS=['DIR:panda/src/downloader', 'BUILDING:PANDAEXPRESS', 'OPENSSL', 'ZLIB']
TargetAdd('downloader_composite.obj', opts=OPTS, input='downloader_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/downloader', ["*.h", "*_composite.cxx"])
TargetAdd('libdownloader.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libdownloader.in', opts=['IMOD:pandaexpress', 'ILIB:libdownloader', 'SRCDIR:panda/src/downloader'])
TargetAdd('libdownloader_igate.obj', input='libdownloader.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/pandaexpress/
#

OPTS=['DIR:panda/metalibs/pandaexpress', 'BUILDING:PANDAEXPRESS', 'ZLIB']
TargetAdd('pandaexpress_pandaexpress.obj', opts=OPTS, input='pandaexpress.cxx')
TargetAdd('libpandaexpress_module.obj', input='libdownloader.in')
TargetAdd('libpandaexpress_module.obj', input='libexpress.in')
TargetAdd('libpandaexpress_module.obj', opts=['ADVAPI',  'OPENSSL'])
TargetAdd('libpandaexpress_module.obj', opts=['IMOD:pandaexpress', 'ILIB:libpandaexpress'])

TargetAdd('libpandaexpress.dll', input='pandaexpress_pandaexpress.obj')
TargetAdd('libpandaexpress.dll', input='libpandaexpress_module.obj')
TargetAdd('libpandaexpress.dll', input='downloader_composite.obj')
TargetAdd('libpandaexpress.dll', input='libdownloader_igate.obj')
TargetAdd('libpandaexpress.dll', input='express_composite1.obj')
TargetAdd('libpandaexpress.dll', input='express_composite2.obj')
TargetAdd('libpandaexpress.dll', input='libexpress_igate.obj')
TargetAdd('libpandaexpress.dll', input='pandabase_pandabase.obj')
TargetAdd('libpandaexpress.dll', input=COMMON_DTOOL_LIBS)
TargetAdd('libpandaexpress.dll', opts=['ADVAPI', 'WINSOCK2',  'OPENSSL', 'ZLIB'])

#
# DIRECTORY: panda/src/pipeline/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pipeline', 'BUILDING:PANDA']
  TargetAdd('pipeline_composite.obj', opts=OPTS, input='pipeline_composite.cxx')
  TargetAdd('pipeline_contextSwitch.obj', opts=OPTS, input='contextSwitch.c')
  IGATEFILES=GetDirectoryContents('panda/src/pipeline', ["*.h", "*_composite.cxx"])
  TargetAdd('libpipeline.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpipeline.in', opts=['IMOD:panda', 'ILIB:libpipeline', 'SRCDIR:panda/src/pipeline'])
  TargetAdd('libpipeline_igate.obj', input='libpipeline.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/putil/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/putil', 'BUILDING:PANDA',  'ZLIB']
  TargetAdd('putil_composite1.obj', opts=OPTS, input='putil_composite1.cxx')
  TargetAdd('putil_composite2.obj', opts=OPTS, input='putil_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/putil', ["*.h", "*_composite.cxx"])
  IGATEFILES.remove("test_bam.h")
  TargetAdd('libputil.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libputil.in', opts=['IMOD:panda', 'ILIB:libputil', 'SRCDIR:panda/src/putil'])
  TargetAdd('libputil_igate.obj', input='libputil.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/audio/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/audio', 'BUILDING:PANDA']
  TargetAdd('audio_composite.obj', opts=OPTS, input='audio_composite.cxx')
  IGATEFILES=["audio.h"]
  TargetAdd('libaudio.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libaudio.in', opts=['IMOD:panda', 'ILIB:libaudio', 'SRCDIR:panda/src/audio'])
  TargetAdd('libaudio_igate.obj', input='libaudio.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/event/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/event', 'BUILDING:PANDA']
  TargetAdd('event_composite.obj', opts=OPTS, input='event_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/event', ["*.h", "*_composite.cxx"])
  TargetAdd('libevent.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libevent.in', opts=['IMOD:panda', 'ILIB:libevent', 'SRCDIR:panda/src/event'])
  TargetAdd('libevent_igate.obj', input='libevent.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/linmath/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/linmath', 'BUILDING:PANDA']
  TargetAdd('linmath_composite.obj', opts=OPTS, input='linmath_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/linmath', ["*.h", "*_ext.I", "*_composite.cxx"])
  IGATEFILES.remove('lmat_ops_src.h')
  IGATEFILES.remove('cast_to_double.h')
  IGATEFILES.remove('lmat_ops.h')
  IGATEFILES.remove('cast_to_float.h')
  TargetAdd('liblinmath.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('liblinmath.in', opts=['IMOD:panda', 'ILIB:liblinmath', 'SRCDIR:panda/src/linmath'])
  TargetAdd('liblinmath_igate.obj', input='liblinmath.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/mathutil/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/mathutil', 'BUILDING:PANDA', 'FFTW']
  TargetAdd('mathutil_composite.obj', opts=OPTS, input='mathutil_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/mathutil', ["*.h", "*_composite.cxx"])
  TargetAdd('libmathutil.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libmathutil.in', opts=['IMOD:panda', 'ILIB:libmathutil', 'SRCDIR:panda/src/mathutil'])
  TargetAdd('libmathutil_igate.obj', input='libmathutil.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/gsgbase/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/gsgbase', 'BUILDING:PANDA']
  TargetAdd('gsgbase_composite.obj', opts=OPTS, input='gsgbase_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/gsgbase', ["*.h", "*_composite.cxx"])
  TargetAdd('libgsgbase.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libgsgbase.in', opts=['IMOD:panda', 'ILIB:libgsgbase', 'SRCDIR:panda/src/gsgbase'])
  TargetAdd('libgsgbase_igate.obj', input='libgsgbase.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pnmimage/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pnmimage', 'BUILDING:PANDA',  'ZLIB']
  TargetAdd('pnmimage_composite.obj', opts=OPTS, input='pnmimage_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/pnmimage', ["*.h", "*_composite.cxx"])
  TargetAdd('libpnmimage.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpnmimage.in', opts=['IMOD:panda', 'ILIB:libpnmimage', 'SRCDIR:panda/src/pnmimage'])
  TargetAdd('libpnmimage_igate.obj', input='libpnmimage.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/nativenet/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/nativenet', 'OPENSSL', 'BUILDING:PANDA']
  TargetAdd('nativenet_composite.obj', opts=OPTS, input='nativenet_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/nativenet', ["*.h", "*_composite.cxx"])
  TargetAdd('libnativenet.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libnativenet.in', opts=['IMOD:panda', 'ILIB:libnativenet', 'SRCDIR:panda/src/nativenet'])
  TargetAdd('libnativenet_igate.obj', input='libnativenet.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/net/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/net', 'BUILDING:PANDA']
  TargetAdd('net_composite.obj', opts=OPTS, input='net_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/net', ["*.h", "*_composite.cxx"])
  IGATEFILES.remove("datagram_ui.h")
  TargetAdd('libnet.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libnet.in', opts=['IMOD:panda', 'ILIB:libnet', 'SRCDIR:panda/src/net'])
  TargetAdd('libnet_igate.obj', input='libnet.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pstatclient/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pstatclient', 'BUILDING:PANDA']
  TargetAdd('pstatclient_composite.obj', opts=OPTS, input='pstatclient_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/pstatclient', ["*.h", "*_composite.cxx"])
  TargetAdd('libpstatclient.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpstatclient.in', opts=['IMOD:panda', 'ILIB:libpstatclient', 'SRCDIR:panda/src/pstatclient'])
  TargetAdd('libpstatclient_igate.obj', input='libpstatclient.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/gobj/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/gobj', 'BUILDING:PANDA',  'NVIDIACG', 'ZLIB', 'SQUISH', "BIGOBJ"]
  TargetAdd('gobj_composite1.obj', opts=OPTS, input='gobj_composite1.cxx')
  TargetAdd('gobj_composite2.obj', opts=OPTS, input='gobj_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/gobj', ["*.h", "*_composite.cxx"])
  if ("cgfx_states.h" in IGATEFILES): IGATEFILES.remove("cgfx_states.h")
  TargetAdd('libgobj.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libgobj.in', opts=['IMOD:panda', 'ILIB:libgobj', 'SRCDIR:panda/src/gobj'])
  TargetAdd('libgobj_igate.obj', input='libgobj.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/lerp/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/lerp', 'BUILDING:PANDA']
  TargetAdd('lerp_composite.obj', opts=OPTS, input='lerp_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/lerp', ["*.h", "*_composite.cxx"])
  IGATEFILES.remove("lerpchans.h")
  TargetAdd('liblerp.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('liblerp.in', opts=['IMOD:panda', 'ILIB:liblerp', 'SRCDIR:panda/src/lerp'])
  TargetAdd('liblerp_igate.obj', input='liblerp.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pgraphnodes/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pgraphnodes', 'BUILDING:PANDA', "BIGOBJ"]
  TargetAdd('pgraphnodes_composite1.obj', opts=OPTS, input='pgraphnodes_composite1.cxx')
  TargetAdd('pgraphnodes_composite2.obj', opts=OPTS, input='pgraphnodes_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/pgraphnodes', ["*.h", "*_composite.cxx"])
  TargetAdd('libpgraphnodes.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpgraphnodes.in', opts=['IMOD:panda', 'ILIB:libpgraphnodes', 'SRCDIR:panda/src/pgraphnodes'])
  TargetAdd('libpgraphnodes_igate.obj', input='libpgraphnodes.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pgraph/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pgraph', 'BUILDING:PANDA','BIGOBJ']
  TargetAdd('pgraph_nodePath.obj', opts=OPTS, input='nodePath.cxx')
  TargetAdd('pgraph_composite1.obj', opts=OPTS, input='pgraph_composite1.cxx')
  TargetAdd('pgraph_composite2.obj', opts=OPTS, input='pgraph_composite2.cxx')
  TargetAdd('pgraph_composite3.obj', opts=OPTS, input='pgraph_composite3.cxx')
  TargetAdd('pgraph_composite4.obj', opts=OPTS, input='pgraph_composite4.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/pgraph', ["*.h", "nodePath.cxx", "*_composite.cxx"])
  TargetAdd('libpgraph.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpgraph.in', opts=['IMOD:panda', 'ILIB:libpgraph', 'SRCDIR:panda/src/pgraph'])
  TargetAdd('libpgraph_igate.obj', input='libpgraph.in', opts=["DEPENDENCYONLY","BIGOBJ"])

#
# DIRECTORY: panda/src/cull/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/cull', 'BUILDING:PANDA']
  TargetAdd('cull_composite.obj', opts=OPTS, input='cull_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/cull', ["*.h", "*_composite.cxx"])
  TargetAdd('libcull.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libcull.in', opts=['IMOD:panda', 'ILIB:libcull', 'SRCDIR:panda/src/cull'])
  TargetAdd('libcull_igate.obj', input='libcull.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/chan/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/chan', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('chan_composite.obj', opts=OPTS, input='chan_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/chan', ["*.h", "*_composite.cxx"])
  IGATEFILES.remove('movingPart.h')
  IGATEFILES.remove('animChannelFixed.h')
  TargetAdd('libchan.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libchan.in', opts=['IMOD:panda', 'ILIB:libchan', 'SRCDIR:panda/src/chan'])
  TargetAdd('libchan_igate.obj', input='libchan.in', opts=["DEPENDENCYONLY"])


# DIRECTORY: panda/src/char/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/char', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('char_composite.obj', opts=OPTS, input='char_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/char', ["*.h", "*_composite.cxx"])
  TargetAdd('libchar.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libchar.in', opts=['IMOD:panda', 'ILIB:libchar', 'SRCDIR:panda/src/char'])
  TargetAdd('libchar_igate.obj', input='libchar.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/dgraph/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/dgraph', 'BUILDING:PANDA']
  TargetAdd('dgraph_composite.obj', opts=OPTS, input='dgraph_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/dgraph', ["*.h", "*_composite.cxx"])
  TargetAdd('libdgraph.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libdgraph.in', opts=['IMOD:panda', 'ILIB:libdgraph', 'SRCDIR:panda/src/dgraph'])
  TargetAdd('libdgraph_igate.obj', input='libdgraph.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/display/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/display', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('display_composite.obj', opts=OPTS, input='display_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/display', ["*.h", "*_composite.cxx"])
  IGATEFILES.remove("renderBuffer.h")
  TargetAdd('libdisplay.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libdisplay.in', opts=['IMOD:panda', 'ILIB:libdisplay', 'SRCDIR:panda/src/display'])
  TargetAdd('libdisplay_igate.obj', input='libdisplay.in', opts=["DEPENDENCYONLY"])

  if (RTDIST and sys.platform == "darwin"):
    OPTS=['DIR:panda/src/display']
    TargetAdd('subprocessWindowBuffer.obj', opts=OPTS, input='subprocessWindowBuffer.cxx')
    TargetAdd('libsubprocbuffer.ilb', input='subprocessWindowBuffer.obj')

#
# DIRECTORY: panda/src/device/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/device', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('device_composite.obj', opts=OPTS, input='device_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/device', ["*.h", "*_composite.cxx"])
  TargetAdd('libdevice.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libdevice.in', opts=['IMOD:panda', 'ILIB:libdevice', 'SRCDIR:panda/src/device'])
  TargetAdd('libdevice_igate.obj', input='libdevice.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pnmtext/
#

if (PkgSkip("FREETYPE")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/pnmtext', 'BUILDING:PANDA',  'FREETYPE']
  TargetAdd('pnmtext_composite.obj', opts=OPTS, input='pnmtext_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/pnmtext', ["*.h", "*_composite.cxx"])
  TargetAdd('libpnmtext.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpnmtext.in', opts=['IMOD:panda', 'ILIB:libpnmtext', 'SRCDIR:panda/src/pnmtext'])
  TargetAdd('libpnmtext_igate.obj', input='libpnmtext.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/text/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/text', 'BUILDING:PANDA', 'ZLIB',  'FREETYPE', 'BIGOBJ']
  TargetAdd('text_composite.obj', opts=OPTS, input='text_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/text', ["*.h", "*_composite.cxx"])
  TargetAdd('libtext.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libtext.in', opts=['IMOD:panda', 'ILIB:libtext', 'SRCDIR:panda/src/text'])
  TargetAdd('libtext_igate.obj', input='libtext.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/movies/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/movies', 'BUILDING:PANDA', 'FFMPEG']
  TargetAdd('movies_composite1.obj', opts=OPTS, input='movies_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/movies', ["*.h", "*_composite.cxx"])
  TargetAdd('libmovies.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libmovies.in', opts=['IMOD:panda', 'ILIB:libmovies', 'SRCDIR:panda/src/movies'])
  TargetAdd('libmovies_igate.obj', input='libmovies.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/grutil/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/grutil', 'BUILDING:PANDA', 'FFMPEG', 'BIGOBJ']
  TargetAdd('grutil_multitexReducer.obj', opts=OPTS, input='multitexReducer.cxx')
  TargetAdd('grutil_composite1.obj', opts=OPTS, input='grutil_composite1.cxx')
  TargetAdd('grutil_composite2.obj', opts=OPTS, input='grutil_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/grutil', ["*.h", "*_composite.cxx"])
  TargetAdd('libgrutil.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libgrutil.in', opts=['IMOD:panda', 'ILIB:libgrutil', 'SRCDIR:panda/src/grutil'])
  TargetAdd('libgrutil_igate.obj', input='libgrutil.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/tform/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/tform', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('tform_composite.obj', opts=OPTS, input='tform_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/tform', ["*.h", "*_composite.cxx"])
  TargetAdd('libtform.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libtform.in', opts=['IMOD:panda', 'ILIB:libtform', 'SRCDIR:panda/src/tform'])
  TargetAdd('libtform_igate.obj', input='libtform.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/collide/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/collide', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('collide_composite.obj', opts=OPTS, input='collide_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/collide', ["*.h", "*_composite.cxx"])
  TargetAdd('libcollide.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libcollide.in', opts=['IMOD:panda', 'ILIB:libcollide', 'SRCDIR:panda/src/collide'])
  TargetAdd('libcollide_igate.obj', input='libcollide.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/parametrics/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/parametrics', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('parametrics_composite.obj', opts=OPTS, input='parametrics_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/parametrics', ["*.h", "*_composite.cxx"])
  TargetAdd('libparametrics.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libparametrics.in', opts=['IMOD:panda', 'ILIB:libparametrics', 'SRCDIR:panda/src/parametrics'])
  TargetAdd('libparametrics_igate.obj', input='libparametrics.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pgui/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pgui', 'BUILDING:PANDA', 'BIGOBJ']
  TargetAdd('pgui_composite.obj', opts=OPTS, input='pgui_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/pgui', ["*.h", "*_composite.cxx"])
  TargetAdd('libpgui.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpgui.in', opts=['IMOD:panda', 'ILIB:libpgui', 'SRCDIR:panda/src/pgui'])
  TargetAdd('libpgui_igate.obj', input='libpgui.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pnmimagetypes/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pnmimagetypes', 'DIR:panda/src/pnmimage', 'BUILDING:PANDA', 'PNG', 'ZLIB', 'JPEG', 'ZLIB',  'JPEG', 'TIFF']
  TargetAdd('pnmimagetypes_composite.obj', opts=OPTS, input='pnmimagetypes_composite.cxx')

#
# DIRECTORY: panda/src/recorder/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/recorder', 'BUILDING:PANDA']
  TargetAdd('recorder_composite.obj', opts=OPTS, input='recorder_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/recorder', ["*.h", "*_composite.cxx"])
  TargetAdd('librecorder.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('librecorder.in', opts=['IMOD:panda', 'ILIB:librecorder', 'SRCDIR:panda/src/recorder'])
  TargetAdd('librecorder_igate.obj', input='librecorder.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/vrpn/
#

if (PkgSkip("VRPN")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/vrpn', 'BUILDING:PANDA',  'VRPN']
  TargetAdd('vrpn_composite.obj', opts=OPTS, input='vrpn_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/vrpn', ["*.h", "*_composite.cxx"])
  TargetAdd('libvrpn.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libvrpn.in', opts=['IMOD:panda', 'ILIB:libvrpn', 'SRCDIR:panda/src/vrpn'])
  TargetAdd('libvrpn_igate.obj', input='libvrpn.in', opts=["DEPENDENCYONLY"])

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
  TargetAdd('dxml_composite1.obj', opts=OPTS, input='dxml_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/dxml', ["*.h", "dxml_composite1.cxx"])
  TargetAdd('libdxml.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libdxml.in', opts=['IMOD:panda', 'ILIB:libdxml', 'SRCDIR:panda/src/dxml'])
  TargetAdd('libdxml_igate.obj', input='libdxml.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/panda/
#

if (not RUNTIME):
  OPTS=['DIR:panda/metalibs/panda', 'BUILDING:PANDA', 'VRPN', 'JPEG', 'PNG',
      'TIFF', 'ZLIB', 'OPENSSL', 'FREETYPE', 'FFTW', 'ADVAPI', 'WINSOCK2','SQUISH',
      'NVIDIACG', 'WINUSER', 'WINMM', 'FFMPEG']

  TargetAdd('panda_panda.obj', opts=OPTS, input='panda.cxx')

  TargetAdd('libpanda_module.obj', input='librecorder.in')
  TargetAdd('libpanda_module.obj', input='libpgraphnodes.in')
  TargetAdd('libpanda_module.obj', input='libpgraph.in')
  TargetAdd('libpanda_module.obj', input='libcull.in')
  TargetAdd('libpanda_module.obj', input='libgrutil.in')
  TargetAdd('libpanda_module.obj', input='libchan.in')
  TargetAdd('libpanda_module.obj', input='libpstatclient.in')
  TargetAdd('libpanda_module.obj', input='libchar.in')
  TargetAdd('libpanda_module.obj', input='libcollide.in')
  TargetAdd('libpanda_module.obj', input='libdevice.in')
  TargetAdd('libpanda_module.obj', input='libdgraph.in')
  TargetAdd('libpanda_module.obj', input='libdisplay.in')
  TargetAdd('libpanda_module.obj', input='libpipeline.in')
  TargetAdd('libpanda_module.obj', input='libevent.in')
  TargetAdd('libpanda_module.obj', input='libgobj.in')
  TargetAdd('libpanda_module.obj', input='libgsgbase.in')
  TargetAdd('libpanda_module.obj', input='liblinmath.in')
  TargetAdd('libpanda_module.obj', input='libmathutil.in')
  TargetAdd('libpanda_module.obj', input='libparametrics.in')
  TargetAdd('libpanda_module.obj', input='libpnmimage.in')
  TargetAdd('libpanda_module.obj', input='libtext.in')
  TargetAdd('libpanda_module.obj', input='libtform.in')
  TargetAdd('libpanda_module.obj', input='liblerp.in')
  TargetAdd('libpanda_module.obj', input='libputil.in')
  TargetAdd('libpanda_module.obj', input='libaudio.in')
  TargetAdd('libpanda_module.obj', input='libnativenet.in')
  TargetAdd('libpanda_module.obj', input='libnet.in')
  TargetAdd('libpanda_module.obj', input='libpgui.in')
  TargetAdd('libpanda_module.obj', input='libmovies.in')
  TargetAdd('libpanda_module.obj', input='libdxml.in')

  TargetAdd('libpanda.dll', input='panda_panda.obj')
  TargetAdd('libpanda.dll', input='libpanda_module.obj')
  TargetAdd('libpanda.dll', input='recorder_composite.obj')
  TargetAdd('libpanda.dll', input='librecorder_igate.obj')
  TargetAdd('libpanda.dll', input='pgraphnodes_composite1.obj')
  TargetAdd('libpanda.dll', input='pgraphnodes_composite2.obj')
  TargetAdd('libpanda.dll', input='libpgraphnodes_igate.obj')
  TargetAdd('libpanda.dll', input='pgraph_nodePath.obj')
  TargetAdd('libpanda.dll', input='pgraph_composite1.obj')
  TargetAdd('libpanda.dll', input='pgraph_composite2.obj')
  TargetAdd('libpanda.dll', input='pgraph_composite3.obj')
  TargetAdd('libpanda.dll', input='pgraph_composite4.obj')
  TargetAdd('libpanda.dll', input='libpgraph_igate.obj')
  TargetAdd('libpanda.dll', input='cull_composite.obj')
  TargetAdd('libpanda.dll', input='movies_composite1.obj')
  TargetAdd('libpanda.dll', input='libmovies_igate.obj')
  TargetAdd('libpanda.dll', input='grutil_multitexReducer.obj')
  TargetAdd('libpanda.dll', input='grutil_composite1.obj')
  TargetAdd('libpanda.dll', input='grutil_composite2.obj')
  TargetAdd('libpanda.dll', input='libgrutil_igate.obj')
  TargetAdd('libpanda.dll', input='chan_composite.obj')
  TargetAdd('libpanda.dll', input='libchan_igate.obj')
  TargetAdd('libpanda.dll', input='pstatclient_composite.obj')
  TargetAdd('libpanda.dll', input='libpstatclient_igate.obj')
  TargetAdd('libpanda.dll', input='char_composite.obj')
  TargetAdd('libpanda.dll', input='libchar_igate.obj')
  TargetAdd('libpanda.dll', input='collide_composite.obj')
  TargetAdd('libpanda.dll', input='libcollide_igate.obj')
  TargetAdd('libpanda.dll', input='device_composite.obj')
  TargetAdd('libpanda.dll', input='libdevice_igate.obj')
  TargetAdd('libpanda.dll', input='dgraph_composite.obj')
  TargetAdd('libpanda.dll', input='libdgraph_igate.obj')
  TargetAdd('libpanda.dll', input='display_composite.obj')
  TargetAdd('libpanda.dll', input='libdisplay_igate.obj')
  TargetAdd('libpanda.dll', input='pipeline_composite.obj')
  TargetAdd('libpanda.dll', input='pipeline_contextSwitch.obj')
  TargetAdd('libpanda.dll', input='libpipeline_igate.obj')
  TargetAdd('libpanda.dll', input='event_composite.obj')
  TargetAdd('libpanda.dll', input='libevent_igate.obj')
  TargetAdd('libpanda.dll', input='gobj_composite1.obj')
  TargetAdd('libpanda.dll', input='gobj_composite2.obj')
  TargetAdd('libpanda.dll', input='libgobj_igate.obj')
  TargetAdd('libpanda.dll', input='gsgbase_composite.obj')
  TargetAdd('libpanda.dll', input='libgsgbase_igate.obj')
  TargetAdd('libpanda.dll', input='linmath_composite.obj')
  TargetAdd('libpanda.dll', input='liblinmath_igate.obj')
  TargetAdd('libpanda.dll', input='mathutil_composite.obj')
  TargetAdd('libpanda.dll', input='libmathutil_igate.obj')
  TargetAdd('libpanda.dll', input='parametrics_composite.obj')
  TargetAdd('libpanda.dll', input='libparametrics_igate.obj')
  TargetAdd('libpanda.dll', input='pnmimagetypes_composite.obj')
  TargetAdd('libpanda.dll', input='pnmimage_composite.obj')
  TargetAdd('libpanda.dll', input='libpnmimage_igate.obj')
  TargetAdd('libpanda.dll', input='text_composite.obj')
  TargetAdd('libpanda.dll', input='libtext_igate.obj')
  TargetAdd('libpanda.dll', input='tform_composite.obj')
  TargetAdd('libpanda.dll', input='libtform_igate.obj')
  TargetAdd('libpanda.dll', input='lerp_composite.obj')
  TargetAdd('libpanda.dll', input='liblerp_igate.obj')
  TargetAdd('libpanda.dll', input='putil_composite1.obj')
  TargetAdd('libpanda.dll', input='putil_composite2.obj')
  TargetAdd('libpanda.dll', input='libputil_igate.obj')
  TargetAdd('libpanda.dll', input='audio_composite.obj')
  TargetAdd('libpanda.dll', input='libaudio_igate.obj')
  TargetAdd('libpanda.dll', input='pgui_composite.obj')
  TargetAdd('libpanda.dll', input='libpgui_igate.obj')
  TargetAdd('libpanda.dll', input='net_composite.obj')
  TargetAdd('libpanda.dll', input='libnet_igate.obj')
  TargetAdd('libpanda.dll', input='nativenet_composite.obj')
  TargetAdd('libpanda.dll', input='libnativenet_igate.obj')
  TargetAdd('libpanda.dll', input='pandabase_pandabase.obj')
  TargetAdd('libpanda.dll', input='libpandaexpress.dll')
  TargetAdd('libpanda.dll', input='dxml_composite1.obj')
  TargetAdd('libpanda.dll', input='libdxml_igate.obj')
  TargetAdd('libpanda.dll', input='libp3dtoolconfig.dll')
  TargetAdd('libpanda.dll', input='libp3dtool.dll')

  if PkgSkip("VRPN")==0:
    TargetAdd('libpanda.dll', input="vrpn_composite.obj")
    TargetAdd('libpanda.dll', input="libvrpn_igate.obj")
    TargetAdd('libpanda_module.obj', input='libvrpn.in')

  if PkgSkip("FREETYPE")==0:
    TargetAdd('libpanda.dll', input="pnmtext_composite.obj")
    TargetAdd('libpanda.dll', input="libpnmtext_igate.obj")
    TargetAdd('libpanda_module.obj', input='libpnmtext.in')

  TargetAdd('libpanda_module.obj', opts=OPTS)
  TargetAdd('libpanda_module.obj', opts=['IMOD:panda', 'ILIB:libpanda'])

  TargetAdd('libpanda.dll', dep='dtool_have_vrpn.dat')
  TargetAdd('libpanda.dll', dep='dtool_have_freetype.dat')
  TargetAdd('libpanda.dll', opts=OPTS)

#
# DIRECTORY: panda/src/vision/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/vision', 'BUILDING:VISION', 'ARTOOLKIT', 'OPENCV', 'DX9', 'DIRECTCAM', 'JPEG']
  TargetAdd('vision_composite1.obj', opts=OPTS, input='vision_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/vision', ["*.h", "*_composite.cxx"])
  TargetAdd('libvision.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libvision.in', opts=['IMOD:p3vision', 'ILIB:libvision', 'SRCDIR:panda/src/vision'])
  TargetAdd('libvision_igate.obj', input='libvision.in', opts=["DEPENDENCYONLY"])

  TargetAdd('libp3vision_module.obj', input='libvision.in')
  TargetAdd('libp3vision_module.obj', opts=OPTS)
  TargetAdd('libp3vision_module.obj', opts=['IMOD:p3vision', 'ILIB:libp3vision'])

  TargetAdd('libp3vision.dll', input='vision_composite1.obj')
  TargetAdd('libp3vision.dll', input='libvision_igate.obj')
  TargetAdd('libp3vision.dll', input='libp3vision_module.obj')
  TargetAdd('libp3vision.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3vision.dll', opts=OPTS)

#
# DIRECTORY: panda/src/awesomium
#
if PkgSkip("AWESOMIUM") == 0 and not RUNTIME:
  OPTS=['DIR:panda/src/awesomium', 'BUILDING:PANDAAWESOMIUM',  'AWESOMIUM']
  TargetAdd('pandaawesomium_composite1.obj', opts=OPTS, input='pandaawesomium_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/awesomium', ["*.h", "*_composite1.cxx"])
  TargetAdd('libawesomium.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libawesomium.in', opts=['IMOD:p3awesomium', 'ILIB:libawesomium', 'SRCDIR:panda/src/awesomium'])
  TargetAdd('libawesomium_igate.obj', input='libawesomium.in', opts=["DEPENDENCYONLY"])

  TargetAdd('libp3awesomium_module.obj', input='libawesomium.in')
  TargetAdd('libp3awesomium_module.obj', opts=OPTS)
  TargetAdd('libp3awesomium_module.obj', opts=['IMOD:p3awesomium', 'ILIB:libp3awesomium'])

  TargetAdd('libp3awesomium.dll', input='pandaawesomium_composite1.obj')
  TargetAdd('libp3awesomium.dll', input='libawesomium_igate.obj')
  TargetAdd('libp3awesomium.dll', input='libp3awesomium_module.obj')
  TargetAdd('libp3awesomium.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3awesomium.dll', opts=OPTS)


#
# DIRECTORY: panda/src/skel
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/skel', 'BUILDING:PANDASKEL', 'ADVAPI']
  TargetAdd('skel_composite.obj', opts=OPTS, input='skel_composite.cxx')
  IGATEFILES=GetDirectoryContents("panda/src/skel", ["*.h", "*_composite.cxx"])
  TargetAdd('libskel.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libskel.in', opts=['IMOD:pandaskel', 'ILIB:libskel', 'SRCDIR:panda/src/skel'])
  TargetAdd('libskel_igate.obj', input='libskel.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/skel
#

if (not RUNTIME):
  OPTS=['BUILDING:PANDASKEL', 'ADVAPI']

  TargetAdd('libpandaskel_module.obj', input='libskel.in')
  TargetAdd('libpandaskel_module.obj', opts=OPTS)
  TargetAdd('libpandaskel_module.obj', opts=['IMOD:pandaskel', 'ILIB:libpandaskel'])

  TargetAdd('libpandaskel.dll', input='skel_composite.obj')
  TargetAdd('libpandaskel.dll', input='libskel_igate.obj')
  TargetAdd('libpandaskel.dll', input='libpandaskel_module.obj')
  TargetAdd('libpandaskel.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaskel.dll', opts=OPTS)

#
# DIRECTORY: panda/src/distort/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/distort', 'BUILDING:PANDAFX']
  TargetAdd('distort_composite.obj', opts=OPTS, input='distort_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/distort', ["*.h", "*_composite.cxx"])
  TargetAdd('libdistort.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libdistort.in', opts=['IMOD:pandafx', 'ILIB:libdistort', 'SRCDIR:panda/src/distort'])
  TargetAdd('libdistort_igate.obj', input='libdistort.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/effects/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/effects', 'BUILDING:PANDAFX',  'NVIDIACG']
  TargetAdd('effects_composite.obj', opts=OPTS, input='effects_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/effects', ["*.h", "*_composite.cxx"])
  TargetAdd('libeffects.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libeffects.in', opts=['IMOD:pandafx', 'ILIB:libeffects', 'SRCDIR:panda/src/effects'])
  TargetAdd('libeffects_igate.obj', input='libeffects.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/pandafx/
#

if (not RUNTIME):
  OPTS=['DIR:panda/metalibs/pandafx', 'DIR:panda/src/distort', 'BUILDING:PANDAFX',  'NVIDIACG']
  TargetAdd('pandafx_pandafx.obj', opts=OPTS, input='pandafx.cxx')

  TargetAdd('libpandafx_module.obj', input='libdistort.in')
  TargetAdd('libpandafx_module.obj', input='libeffects.in')
  TargetAdd('libpandafx_module.obj', opts=OPTS)
  TargetAdd('libpandafx_module.obj', opts=['IMOD:pandafx', 'ILIB:libpandafx'])

  TargetAdd('libpandafx.dll', input='pandafx_pandafx.obj')
  TargetAdd('libpandafx.dll', input='libpandafx_module.obj')
  TargetAdd('libpandafx.dll', input='distort_composite.obj')
  TargetAdd('libpandafx.dll', input='libdistort_igate.obj')
  TargetAdd('libpandafx.dll', input='effects_composite.obj')
  TargetAdd('libpandafx.dll', input='libeffects_igate.obj')
  TargetAdd('libpandafx.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandafx.dll', opts=['ADVAPI',  'NVIDIACG'])

#
# DIRECTORY: panda/src/audiotraits/
#

if PkgSkip("FMODEX") == 0 and not RUNTIME:
  OPTS=['DIR:panda/src/audiotraits', 'BUILDING:FMOD_AUDIO',  'FMODEX']
  TargetAdd('fmod_audio_fmod_audio_composite.obj', opts=OPTS, input='fmod_audio_composite.cxx')
  TargetAdd('libp3fmod_audio.dll', input='fmod_audio_fmod_audio_composite.obj')
  TargetAdd('libp3fmod_audio.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3fmod_audio.dll', opts=['MODULE', 'ADVAPI', 'WINUSER', 'WINMM', 'FMODEX'])

if PkgSkip("OPENAL") == 0 and not RUNTIME:
  OPTS=['DIR:panda/src/audiotraits', 'BUILDING:OPENAL_AUDIO',  'OPENAL']
  TargetAdd('openal_audio_openal_audio_composite.obj', opts=OPTS, input='openal_audio_composite.cxx')
  TargetAdd('libp3openal_audio.dll', input='openal_audio_openal_audio_composite.obj')
  TargetAdd('libp3openal_audio.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3openal_audio.dll', opts=['MODULE', 'ADVAPI', 'WINUSER', 'WINMM', 'OPENAL'])

#
# DIRECTORY: panda/src/downloadertools/
#

if (PkgSkip("OPENSSL")==0 and not RTDIST and not RUNTIME and PkgSkip("DEPLOYTOOLS")==0):
  OPTS=['DIR:panda/src/downloadertools', 'OPENSSL', 'ZLIB', 'ADVAPI', 'WINSOCK2', 'WINSHELL']

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
  OPTS=['DIR:panda/src/downloadertools', 'ZLIB', 'OPENSSL', 'ADVAPI', 'WINSOCK2', 'WINSHELL']

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

if (sys.platform.startswith("win") and not RUNTIME):
  OPTS=['DIR:panda/src/windisplay', 'BUILDING:PANDAWIN']
  TargetAdd('windisplay_composite.obj', opts=OPTS+["BIGOBJ"], input='windisplay_composite.cxx')
  TargetAdd('windisplay_windetectdx8.obj', opts=OPTS + ["DX8"], input='winDetectDx8.cxx')
  TargetAdd('windisplay_windetectdx9.obj', opts=OPTS + ["DX9"], input='winDetectDx9.cxx')
  TargetAdd('libp3windisplay.dll', input='windisplay_composite.obj')
  TargetAdd('libp3windisplay.dll', input='windisplay_windetectdx8.obj')
  TargetAdd('libp3windisplay.dll', input='windisplay_windetectdx9.obj')
  TargetAdd('libp3windisplay.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3windisplay.dll', opts=['WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM',"BIGOBJ"])

#
# DIRECTORY: panda/metalibs/pandadx8/
#

if PkgSkip("DX8")==0 and not RUNTIME:
  OPTS=['DIR:panda/src/dxgsg8', 'DIR:panda/metalibs/pandadx8', 'BUILDING:PANDADX', 'DX8']
  TargetAdd('dxgsg8_dxGraphicsStateGuardian8.obj', opts=OPTS, input='dxGraphicsStateGuardian8.cxx')
  TargetAdd('dxgsg8_composite.obj', opts=OPTS, input='dxgsg8_composite.cxx')
  TargetAdd('pandadx8_pandadx8.obj', opts=OPTS, input='pandadx8.cxx')
  TargetAdd('libpandadx8.dll', input='pandadx8_pandadx8.obj')
  TargetAdd('libpandadx8.dll', input='dxgsg8_dxGraphicsStateGuardian8.obj')
  TargetAdd('libpandadx8.dll', input='dxgsg8_composite.obj')
  TargetAdd('libpandadx8.dll', input='libp3windisplay.dll')
  TargetAdd('libpandadx8.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandadx8.dll', opts=['MODULE', 'ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DX8'])

#
# DIRECTORY: panda/metalibs/pandadx9/
#

if PkgSkip("DX9")==0 and not RUNTIME:
  OPTS=['DIR:panda/src/dxgsg9', 'BUILDING:PANDADX', 'DX9',  'NVIDIACG', 'CGDX9']
  TargetAdd('dxgsg9_dxGraphicsStateGuardian9.obj', opts=OPTS, input='dxGraphicsStateGuardian9.cxx')
  TargetAdd('dxgsg9_composite.obj', opts=OPTS, input='dxgsg9_composite.cxx')
  OPTS=['DIR:panda/metalibs/pandadx9', 'BUILDING:PANDADX', 'DX9',  'NVIDIACG', 'CGDX9']
  TargetAdd('pandadx9_pandadx9.obj', opts=OPTS, input='pandadx9.cxx')
  TargetAdd('libpandadx9.dll', input='pandadx9_pandadx9.obj')
  TargetAdd('libpandadx9.dll', input='dxgsg9_dxGraphicsStateGuardian9.obj')
  TargetAdd('libpandadx9.dll', input='dxgsg9_composite.obj')
  TargetAdd('libpandadx9.dll', input='libp3windisplay.dll')
  TargetAdd('libpandadx9.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandadx9.dll', opts=['MODULE', 'ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DX9',  'NVIDIACG', 'CGDX9'])

#
# DIRECTORY: panda/src/egg/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/egg', 'BUILDING:PANDAEGG',  'ZLIB', 'BISONPREFIX_eggyy', 'FLEXDASHI']
  CreateFile(GetOutputDir()+"/include/parser.h")
  TargetAdd('egg_parser.obj', opts=OPTS, input='parser.yxx')
  TargetAdd('parser.h', input='egg_parser.obj', opts=['DEPENDENCYONLY'])
  TargetAdd('egg_lexer.obj', opts=OPTS, input='lexer.lxx')
  TargetAdd('egg_composite1.obj', opts=OPTS, input='egg_composite1.cxx')
  TargetAdd('egg_composite2.obj', opts=OPTS, input='egg_composite2.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/egg', ["*.h", "*_composite.cxx"])
  if "parser.h" in IGATEFILES: IGATEFILES.remove("parser.h")
  TargetAdd('libegg.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libegg.in', opts=['IMOD:pandaegg', 'ILIB:libegg', 'SRCDIR:panda/src/egg'])
  TargetAdd('libegg_igate.obj', input='libegg.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/egg2pg/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/egg2pg', 'BUILDING:PANDAEGG']
  TargetAdd('egg2pg_composite.obj', opts=OPTS, input='egg2pg_composite.cxx')
  IGATEFILES=['load_egg_file.h']
  TargetAdd('libegg2pg.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libegg2pg.in', opts=['IMOD:pandaegg', 'ILIB:libegg2pg', 'SRCDIR:panda/src/egg2pg'])
  TargetAdd('libegg2pg_igate.obj', input='libegg2pg.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/framework/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/framework', 'BUILDING:FRAMEWORK']
  TargetAdd('framework_composite.obj', opts=OPTS, input='framework_composite.cxx')
  TargetAdd('libp3framework.dll', input='framework_composite.obj')
  TargetAdd('libp3framework.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3framework.dll', opts=['ADVAPI'])

#
# DIRECTORY: panda/src/glstuff/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/glstuff',  'NVIDIACG', 'CGGL']
  TargetAdd('glstuff_glpure.obj', opts=OPTS, input='glpure.cxx')
  TargetAdd('libp3glstuff.dll', input='glstuff_glpure.obj')
  TargetAdd('libp3glstuff.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3glstuff.dll', opts=['ADVAPI', 'GL',  'NVIDIACG', 'CGGL'])

#
# DIRECTORY: panda/src/glgsg/
#

if (not RUNTIME and PkgSkip("GL")==0):
  OPTS=['DIR:panda/src/glgsg', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGL',  'NVIDIACG']
  TargetAdd('glgsg_config_glgsg.obj', opts=OPTS, input='config_glgsg.cxx')
  TargetAdd('glgsg_glgsg.obj', opts=OPTS, input='glgsg.cxx')

#
# DIRECTORY: panda/src/glesgsg/
#

if (not RUNTIME and PkgSkip("GLES")==0):
  OPTS=['DIR:panda/src/glesgsg', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLES']
  TargetAdd('glesgsg_config_glesgsg.obj', opts=OPTS, input='config_glesgsg.cxx')
  TargetAdd('glesgsg_glesgsg.obj', opts=OPTS, input='glesgsg.cxx')

#
# DIRECTORY: panda/src/gles2gsg/
#

if (not RUNTIME and PkgSkip("GLES2")==0):
  OPTS=['DIR:panda/src/gles2gsg', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLES2']
  TargetAdd('gles2gsg_config_gles2gsg.obj', opts=OPTS, input='config_gles2gsg.cxx')
  TargetAdd('gles2gsg_gles2gsg.obj', opts=OPTS, input='gles2gsg.cxx')

#
# DIRECTORY: panda/metalibs/pandaegg/
#

if (not RUNTIME):
  OPTS=['DIR:panda/metalibs/pandaegg', 'DIR:panda/src/egg', 'BUILDING:PANDAEGG']
  TargetAdd('pandaegg_pandaegg.obj', opts=OPTS, input='pandaegg.cxx')

  TargetAdd('libpandaegg_module.obj', input='libegg2pg.in')
  TargetAdd('libpandaegg_module.obj', input='libegg.in')
  TargetAdd('libpandaegg_module.obj', opts=OPTS)
  TargetAdd('libpandaegg_module.obj', opts=['IMOD:pandaegg', 'ILIB:libpandaegg'])

  TargetAdd('libpandaegg.dll', input='pandaegg_pandaegg.obj')
  TargetAdd('libpandaegg.dll', input='libpandaegg_module.obj')
  TargetAdd('libpandaegg.dll', input='egg2pg_composite.obj')
  TargetAdd('libpandaegg.dll', input='libegg2pg_igate.obj')
  TargetAdd('libpandaegg.dll', input='egg_composite1.obj')
  TargetAdd('libpandaegg.dll', input='egg_composite2.obj')
  TargetAdd('libpandaegg.dll', input='egg_parser.obj')
  TargetAdd('libpandaegg.dll', input='egg_lexer.obj')
  TargetAdd('libpandaegg.dll', input='libegg_igate.obj')
  TargetAdd('libpandaegg.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaegg.dll', opts=['ADVAPI'])

#
# DIRECTORY: panda/src/mesadisplay/
#

if (not sys.platform.startswith("win") and PkgSkip("GL")==0 and PkgSkip("OSMESA")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/mesadisplay', 'DIR:panda/src/glstuff', 'BUILDING:PANDAMESA', 'NVIDIACG', 'GL', 'OSMESA']
  TargetAdd('mesadisplay_composite.obj', opts=OPTS, input='mesadisplay_composite.cxx')
  OPTS=['DIR:panda/metalibs/pandagl', 'BUILDING:PANDAMESA', 'NVIDIACG', 'GL']
  TargetAdd('libpandamesa.dll', input='mesadisplay_composite.obj')
  TargetAdd('libpandamesa.dll', input='libp3glstuff.dll')
  TargetAdd('libpandamesa.dll', input='libpandafx.dll')
  TargetAdd('libpandamesa.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandamesa.dll', opts=['MODULE', 'GL', 'OSMESA'])

#
# DIRECTORY: panda/src/x11display/
#

if (sys.platform != "win32" and sys.platform != "darwin" and PkgSkip("X11")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/x11display', 'BUILDING:PANDAX11', 'X11']
  TargetAdd('x11display_composite.obj', opts=OPTS, input='x11display_composite.cxx')

#
# DIRECTORY: panda/src/glxdisplay/
#

if (sys.platform != "win32" and sys.platform != "darwin" and PkgSkip("GL")==0 and PkgSkip("X11")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/glxdisplay', 'BUILDING:PANDAGL',  'GL', 'NVIDIACG', 'CGGL']
  TargetAdd('glxdisplay_composite.obj', opts=OPTS, input='glxdisplay_composite.cxx')
  OPTS=['DIR:panda/metalibs/pandagl', 'BUILDING:PANDAGL',  'GL', 'NVIDIACG', 'CGGL']
  TargetAdd('pandagl_pandagl.obj', opts=OPTS, input='pandagl.cxx')
  TargetAdd('libpandagl.dll', input='x11display_composite.obj')
  TargetAdd('libpandagl.dll', input='pandagl_pandagl.obj')
  TargetAdd('libpandagl.dll', input='glgsg_config_glgsg.obj')
  TargetAdd('libpandagl.dll', input='glgsg_glgsg.obj')
  TargetAdd('libpandagl.dll', input='glxdisplay_composite.obj')
  TargetAdd('libpandagl.dll', input='libp3glstuff.dll')
  TargetAdd('libpandagl.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagl.dll', opts=['MODULE', 'GL', 'NVIDIACG', 'CGGL', 'X11', 'XRANDR', 'XF86DGA', 'XCURSOR'])

#
# DIRECTORY: panda/src/osxdisplay/
#

if (sys.platform == 'darwin' and PkgSkip("GL")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/osxdisplay', 'BUILDING:PANDAGL',  'GL', 'NVIDIACG', 'CGGL']
  TargetAdd('osxdisplay_composite1.obj', opts=OPTS, input='osxdisplay_composite1.cxx')
  TargetAdd('osxdisplay_osxGraphicsWindow.obj', opts=OPTS, input='osxGraphicsWindow.mm')
  OPTS=['DIR:panda/metalibs/pandagl', 'BUILDING:PANDAGL',  'GL', 'NVIDIACG', 'CGGL']
  TargetAdd('pandagl_pandagl.obj', opts=OPTS, input='pandagl.cxx')
  TargetAdd('libpandagl.dll', input='pandagl_pandagl.obj')
  TargetAdd('libpandagl.dll', input='glgsg_config_glgsg.obj')
  TargetAdd('libpandagl.dll', input='glgsg_glgsg.obj')
  TargetAdd('libpandagl.dll', input='osxdisplay_composite1.obj')
  TargetAdd('libpandagl.dll', input='osxdisplay_osxGraphicsWindow.obj')
  TargetAdd('libpandagl.dll', input='libp3glstuff.dll')
  TargetAdd('libpandagl.dll', input='libpandafx.dll')
  TargetAdd('libpandagl.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagl.dll', opts=['MODULE', 'GL', 'NVIDIACG', 'CGGL', 'CARBON', 'AGL', 'COCOA'])

#
# DIRECTORY: panda/src/wgldisplay/
#

if (sys.platform == "win32" and PkgSkip("GL")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/wgldisplay', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGL',  'NVIDIACG', 'CGGL']
  TargetAdd('wgldisplay_composite.obj', opts=OPTS, input='wgldisplay_composite.cxx')
  OPTS=['DIR:panda/metalibs/pandagl', 'BUILDING:PANDAGL',  'NVIDIACG', 'CGGL']
  TargetAdd('pandagl_pandagl.obj', opts=OPTS, input='pandagl.cxx')
  TargetAdd('libpandagl.dll', input='pandagl_pandagl.obj')
  TargetAdd('libpandagl.dll', input='glgsg_config_glgsg.obj')
  TargetAdd('libpandagl.dll', input='glgsg_glgsg.obj')
  TargetAdd('libpandagl.dll', input='wgldisplay_composite.obj')
  TargetAdd('libpandagl.dll', input='libp3windisplay.dll')
  TargetAdd('libpandagl.dll', input='libp3glstuff.dll')
  TargetAdd('libpandagl.dll', input='libpandafx.dll')
  TargetAdd('libpandagl.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagl.dll', opts=['MODULE', 'WINGDI', 'GL', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM',  'NVIDIACG', 'CGGL'])

#
# DIRECTORY: panda/src/egldisplay/
#

if (PkgSkip("EGL")==0 and PkgSkip("GLES")==0 and PkgSkip("X11")==0 and not RUNTIME):
  DefSymbol('GLES', 'OPENGLES_1', '')
  OPTS=['DIR:panda/src/egldisplay', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLES',  'GLES', 'EGL']
  TargetAdd('pandagles_egldisplay_composite1.obj', opts=OPTS, input='egldisplay_composite1.cxx')
  OPTS=['DIR:panda/metalibs/pandagles', 'BUILDING:PANDAGLES', 'GLES', 'EGL']
  TargetAdd('pandagles_pandagles.obj', opts=OPTS, input='pandagles.cxx')
  # Uncomment this as soon as x11-specific stuff is removed from egldisplay
  #TargetAdd('libpandagles.dll', input='x11display_composite.obj')
  TargetAdd('libpandagles.dll', input='pandagles_pandagles.obj')
  TargetAdd('libpandagles.dll', input='glesgsg_config_glesgsg.obj')
  TargetAdd('libpandagles.dll', input='glesgsg_glesgsg.obj')
  TargetAdd('libpandagles.dll', input='pandagles_egldisplay_composite1.obj')
  TargetAdd('libpandagles.dll', input='libp3glstuff.dll')
  TargetAdd('libpandagles.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagles.dll', opts=['MODULE', 'GLES', 'EGL', 'X11', 'XRANDR', 'XF86DGA', 'XCURSOR'])

#
# DIRECTORY: panda/src/egldisplay/
#

if (PkgSkip("EGL")==0 and PkgSkip("GLES2")==0 and PkgSkip("X11")==0 and not RUNTIME):
  DefSymbol('GLES2', 'OPENGLES_2', '')
  OPTS=['DIR:panda/src/egldisplay', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLES2',  'GLES2', 'EGL']
  TargetAdd('pandagles2_egldisplay_composite1.obj', opts=OPTS, input='egldisplay_composite1.cxx')
  OPTS=['DIR:panda/metalibs/pandagles2', 'BUILDING:PANDAGLES2', 'GLES2', 'EGL']
  TargetAdd('pandagles2_pandagles2.obj', opts=OPTS, input='pandagles2.cxx')
  # Uncomment this as soon as x11-specific stuff is removed from egldisplay
  #TargetAdd('libpandagles2.dll', input='x11display_composite.obj')
  TargetAdd('libpandagles2.dll', input='pandagles2_pandagles2.obj')
  TargetAdd('libpandagles2.dll', input='gles2gsg_config_gles2gsg.obj')
  TargetAdd('libpandagles2.dll', input='gles2gsg_gles2gsg.obj')
  TargetAdd('libpandagles2.dll', input='pandagles2_egldisplay_composite1.obj')
  TargetAdd('libpandagles2.dll', input='libp3glstuff.dll')
  TargetAdd('libpandagles2.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagles2.dll', opts=['MODULE', 'GLES2', 'EGL', 'X11', 'XRANDR', 'XF86DGA', 'XCURSOR'])

#
# DIRECTORY: panda/src/ode/
#
if (PkgSkip("ODE")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/ode', 'BUILDING:PANDAODE', 'ODE']
  TargetAdd('pode_composite1.obj', opts=OPTS, input='pode_composite1.cxx')
  TargetAdd('pode_composite2.obj', opts=OPTS, input='pode_composite2.cxx')
  TargetAdd('pode_composite3.obj', opts=OPTS, input='pode_composite3.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/ode', ["*.h", "*_composite.cxx"])
  IGATEFILES.remove("odeConvexGeom.h")
  IGATEFILES.remove("odeHeightFieldGeom.h")
  IGATEFILES.remove("odeHelperStructs.h")
  TargetAdd('libpandaode.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpandaode.in', opts=['IMOD:pandaode', 'ILIB:libpandaode', 'SRCDIR:panda/src/ode'])
  TargetAdd('libpandaode_igate.obj', input='libpandaode.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/pandaode/
#
if (PkgSkip("ODE")==0 and not RUNTIME):
  OPTS=['DIR:panda/metalibs/pandaode', 'BUILDING:PANDAODE', 'ODE']
  TargetAdd('pandaode_pandaode.obj', opts=OPTS, input='pandaode.cxx')

  TargetAdd('libpandaode_module.obj', input='libpandaode.in')
  TargetAdd('libpandaode_module.obj', opts=OPTS)
  TargetAdd('libpandaode_module.obj', opts=['IMOD:pandaode', 'ILIB:libpandaode'])

  TargetAdd('libpandaode.dll', input='pandaode_pandaode.obj')
  TargetAdd('libpandaode.dll', input='libpandaode_module.obj')
  TargetAdd('libpandaode.dll', input='pode_composite1.obj')
  TargetAdd('libpandaode.dll', input='pode_composite2.obj')
  TargetAdd('libpandaode.dll', input='pode_composite3.obj')
  TargetAdd('libpandaode.dll', input='libpandaode_igate.obj')
  TargetAdd('libpandaode.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaode.dll', opts=['WINUSER', 'ODE'])

#
# DIRECTORY: panda/src/physx/
#

if (PkgSkip("PHYSX")==0):
  OPTS=['DIR:panda/src/physx', 'BUILDING:PANDAPHYSX', 'PHYSX', 'NOPPC']
  TargetAdd('physx_composite.obj', opts=OPTS, input='physx_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/physx', ["*.h", "*_composite.cxx"])
  TargetAdd('libpandaphysx.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpandaphysx.in', opts=['IMOD:pandaphysx', 'ILIB:libpandaphysx', 'SRCDIR:panda/src/physx'])
  TargetAdd('libpandaphysx_igate.obj', input='libpandaphysx.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/pandaphysx/
#

if (PkgSkip("PHYSX")==0):
  OPTS=['DIR:panda/metalibs/pandaphysx', 'BUILDING:PANDAPHYSX', 'PHYSX', 'NOPPC']
  TargetAdd('pandaphysx_pandaphysx.obj', opts=OPTS, input='pandaphysx.cxx')

  TargetAdd('libpandaphysx_module.obj', input='libpandaphysx.in')
  TargetAdd('libpandaphysx_module.obj', opts=OPTS)
  TargetAdd('libpandaphysx_module.obj', opts=['IMOD:pandaphysx', 'ILIB:libpandaphysx'])

  TargetAdd('libpandaphysx.dll', input='pandaphysx_pandaphysx.obj')
  TargetAdd('libpandaphysx.dll', input='libpandaphysx_module.obj')
  TargetAdd('libpandaphysx.dll', input='physx_composite.obj')
  TargetAdd('libpandaphysx.dll', input='libpandaphysx_igate.obj')
  TargetAdd('libpandaphysx.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaphysx.dll', opts=['WINUSER', 'PHYSX', 'NOPPC'])

#
# DIRECTORY: panda/src/physics/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/physics', 'BUILDING:PANDAPHYSICS']
  TargetAdd('physics_composite.obj', opts=OPTS, input='physics_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/physics', ["*.h", "*_composite.cxx"])
  IGATEFILES.remove("forces.h")
  TargetAdd('libphysics.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libphysics.in', opts=['IMOD:pandaphysics', 'ILIB:libphysics', 'SRCDIR:panda/src/physics'])
  TargetAdd('libphysics_igate.obj', input='libphysics.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/particlesystem/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/particlesystem', 'BUILDING:PANDAPHYSICS']
  TargetAdd('particlesystem_composite.obj', opts=OPTS, input='particlesystem_composite.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/particlesystem', ["*.h", "*_composite.cxx"])
  IGATEFILES.remove('orientedParticle.h')
  IGATEFILES.remove('orientedParticleFactory.h')
  IGATEFILES.remove('particlefactories.h')
  IGATEFILES.remove('emitters.h')
  IGATEFILES.remove('particles.h')
  TargetAdd('libparticlesystem.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libparticlesystem.in', opts=['IMOD:pandaphysics', 'ILIB:libparticlesystem', 'SRCDIR:panda/src/particlesystem'])

#
# DIRECTORY: panda/metalibs/pandaphysics/
#

if (not RUNTIME):
  OPTS=['DIR:panda/metalibs/pandaphysics', 'BUILDING:PANDAPHYSICS']
  TargetAdd('pandaphysics_pandaphysics.obj', opts=OPTS, input='pandaphysics.cxx')

  TargetAdd('libpandaphysics_module.obj', input='libphysics.in')
  TargetAdd('libpandaphysics_module.obj', input='libparticlesystem.in')
  TargetAdd('libpandaphysics_module.obj', opts=OPTS)
  TargetAdd('libpandaphysics_module.obj', opts=['IMOD:pandaphysics', 'ILIB:libpandaphysics'])

  TargetAdd('libpandaphysics.dll', input='pandaphysics_pandaphysics.obj')
  TargetAdd('libpandaphysics.dll', input='libpandaphysics_module.obj')
  TargetAdd('libpandaphysics.dll', input='physics_composite.obj')
  TargetAdd('libpandaphysics.dll', input='libphysics_igate.obj')
  TargetAdd('libpandaphysics.dll', input='particlesystem_composite.obj')
  TargetAdd('libpandaphysics.dll', input='libparticlesystem_igate.obj')
  TargetAdd('libpandaphysics.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaphysics.dll', opts=['ADVAPI'])

#
# DIRECTORY: panda/src/speedtree/
#

if (PkgSkip("SPEEDTREE")==0):
  OPTS=['DIR:panda/src/speedtree', 'BUILDING:PANDASPEEDTREE', 'SPEEDTREE']
  TargetAdd('pandaspeedtree_composite1.obj', opts=OPTS, input='pandaspeedtree_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/speedtree', ["*.h", "*_composite.cxx"])
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

if (not RTDIST and not RUNTIME and PkgSkip("PVIEW")==0):
  OPTS=['DIR:panda/src/testbed']
  TargetAdd('pview_pview.obj', opts=OPTS, input='pview.cxx')
  TargetAdd('pview.exe', input='pview_pview.obj')
  TargetAdd('pview.exe', input='libp3framework.dll')
  TargetAdd('pview.exe', input='libpandaegg.dll')
  TargetAdd('pview.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('pview.exe', opts=['ADVAPI', 'WINSOCK2', 'WINSHELL'])

#
# DIRECTORY: panda/src/tinydisplay/
#

if (not RUNTIME and (sys.platform == "win32" or sys.platform == "darwin" or PkgSkip("X11")==0) and PkgSkip("TINYDISPLAY")==0):
  OPTS=['DIR:panda/src/tinydisplay', 'BUILDING:TINYDISPLAY']
  TargetAdd('tinydisplay_composite1.obj', opts=OPTS, input='tinydisplay_composite1.cxx')
  TargetAdd('tinydisplay_composite2.obj', opts=OPTS, input='tinydisplay_composite2.cxx')
  TargetAdd('tinydisplay_ztriangle_1.obj', opts=OPTS, input='ztriangle_1.cxx')
  TargetAdd('tinydisplay_ztriangle_2.obj', opts=OPTS, input='ztriangle_2.cxx')
  TargetAdd('tinydisplay_ztriangle_3.obj', opts=OPTS, input='ztriangle_3.cxx')
  TargetAdd('tinydisplay_ztriangle_4.obj', opts=OPTS, input='ztriangle_4.cxx')
  TargetAdd('tinydisplay_ztriangle_table.obj', opts=OPTS, input='ztriangle_table.cxx')
  if (sys.platform == "darwin"):
    TargetAdd('tinydisplay_tinyOsxGraphicsWindow.obj', opts=OPTS, input='tinyOsxGraphicsWindow.mm')
    TargetAdd('libtinydisplay.dll', input='tinydisplay_tinyOsxGraphicsWindow.obj')
    TargetAdd('libtinydisplay.dll', opts=['CARBON', 'AGL', 'COCOA'])
  elif (sys.platform == "win32"):
    TargetAdd('libtinydisplay.dll', input='libp3windisplay.dll')
    TargetAdd('libtinydisplay.dll', opts=['WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM'])
  else:
    TargetAdd('libtinydisplay.dll', input='x11display_composite.obj')
    TargetAdd('libtinydisplay.dll', opts=['X11', 'XRANDR', 'XF86DGA', 'XCURSOR'])
  TargetAdd('libtinydisplay.dll', input='tinydisplay_composite1.obj')
  TargetAdd('libtinydisplay.dll', input='tinydisplay_composite2.obj')
  TargetAdd('libtinydisplay.dll', input='tinydisplay_ztriangle_1.obj')
  TargetAdd('libtinydisplay.dll', input='tinydisplay_ztriangle_2.obj')
  TargetAdd('libtinydisplay.dll', input='tinydisplay_ztriangle_3.obj')
  TargetAdd('libtinydisplay.dll', input='tinydisplay_ztriangle_4.obj')
  TargetAdd('libtinydisplay.dll', input='tinydisplay_ztriangle_table.obj')
  TargetAdd('libtinydisplay.dll', input=COMMON_PANDA_LIBS)

#
# DIRECTORY: direct/src/directbase/
#

if (PkgSkip("PYTHON")==0 and PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/directbase', 'PYTHON']
  TargetAdd('directbase_directbase.obj', opts=OPTS+['BUILDING:DIRECT'], input='directbase.cxx')

  if (not RTDIST and not RUNTIME):
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

if (PkgSkip("PYTHON")==0 and PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/dcparser', 'WITHINPANDA', 'BUILDING:DIRECT', 'BISONPREFIX_dcyy']
  CreateFile(GetOutputDir()+"/include/dcParser.h")
  TargetAdd('dcparser_dcParser.obj', opts=OPTS, input='dcParser.yxx')
  TargetAdd('dcParser.h', input='egg_parser.obj', opts=['DEPENDENCYONLY'])
  TargetAdd('dcparser_dcLexer.obj', opts=OPTS, input='dcLexer.lxx')
  TargetAdd('dcparser_composite.obj', opts=OPTS, input='dcparser_composite.cxx')
  IGATEFILES=GetDirectoryContents('direct/src/dcparser', ["*.h", "*_composite.cxx"])
  if "dcParser.h" in IGATEFILES: IGATEFILES.remove("dcParser.h")
  if "dcmsgtypes.h" in IGATEFILES: IGATEFILES.remove('dcmsgtypes.h')
  TargetAdd('libdcparser.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libdcparser.in', opts=['IMOD:p3direct', 'ILIB:libdcparser', 'SRCDIR:direct/src/dcparser'])
  TargetAdd('libdcparser_igate.obj', input='libdcparser.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/src/deadrec/
#

if (PkgSkip("PYTHON")==0 and PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/deadrec', 'BUILDING:DIRECT']
  TargetAdd('deadrec_composite.obj', opts=OPTS, input='deadrec_composite.cxx')
  IGATEFILES=GetDirectoryContents('direct/src/deadrec', ["*.h", "*_composite.cxx"])
  TargetAdd('libdeadrec.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libdeadrec.in', opts=['IMOD:p3direct', 'ILIB:libdeadrec', 'SRCDIR:direct/src/deadrec'])
  TargetAdd('libdeadrec_igate.obj', input='libdeadrec.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/src/distributed/
#

if (PkgSkip("PYTHON")==0 and PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/distributed', 'DIR:direct/src/dcparser', 'WITHINPANDA', 'BUILDING:DIRECT', 'OPENSSL']
  TargetAdd('distributed_config_distributed.obj', opts=OPTS, input='config_distributed.cxx')
  TargetAdd('distributed_cConnectionRepository.obj', opts=OPTS, input='cConnectionRepository.cxx')
  TargetAdd('distributed_cDistributedSmoothNodeBase.obj', opts=OPTS, input='cDistributedSmoothNodeBase.cxx')
  IGATEFILES=GetDirectoryContents('direct/src/distributed', ["*.h", "*.cxx"])
  TargetAdd('libdistributed.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libdistributed.in', opts=['IMOD:p3direct', 'ILIB:libdistributed', 'SRCDIR:direct/src/distributed'])
  TargetAdd('libdistributed_igate.obj', input='libdistributed.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/src/interval/
#

if (PkgSkip("PYTHON")==0 and PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/interval', 'BUILDING:DIRECT']
  TargetAdd('interval_composite.obj', opts=OPTS, input='interval_composite.cxx')
  IGATEFILES=GetDirectoryContents('direct/src/interval', ["*.h", "*_composite.cxx"])
  TargetAdd('libinterval.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libinterval.in', opts=['IMOD:p3direct', 'ILIB:libinterval', 'SRCDIR:direct/src/interval'])
  TargetAdd('libinterval_igate.obj', input='libinterval.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/src/showbase/
#

if (PkgSkip("PYTHON")==0 and PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/showbase', 'BUILDING:DIRECT']
  TargetAdd('showbase_showBase.obj', opts=OPTS, input='showBase.cxx')
  if (sys.platform == "darwin"):
    TargetAdd('showbase_showBase_assist.obj', opts=OPTS, input='showBase_assist.mm')
  IGATEFILES=GetDirectoryContents('direct/src/showbase', ["*.h", "showBase.cxx"])
  TargetAdd('libshowbase.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libshowbase.in', opts=['IMOD:p3direct', 'ILIB:libshowbase', 'SRCDIR:direct/src/showbase'])
  TargetAdd('libshowbase_igate.obj', input='libshowbase.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/metalibs/direct/
#

if (PkgSkip("PYTHON")==0 and PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/metalibs/direct', 'BUILDING:DIRECT']
  TargetAdd('direct_direct.obj', opts=OPTS, input='direct.cxx')

  TargetAdd('libp3direct_module.obj', input='libdcparser.in')
  TargetAdd('libp3direct_module.obj', input='libshowbase.in')
  TargetAdd('libp3direct_module.obj', input='libdeadrec.in')
  TargetAdd('libp3direct_module.obj', input='libinterval.in')
  TargetAdd('libp3direct_module.obj', input='libdistributed.in')
  TargetAdd('libp3direct_module.obj', opts=OPTS)
  TargetAdd('libp3direct_module.obj', opts=['IMOD:p3direct', 'ILIB:libp3direct'])

  TargetAdd('libp3direct.dll', input='direct_direct.obj')
  TargetAdd('libp3direct.dll', input='libp3direct_module.obj')
  TargetAdd('libp3direct.dll', input='directbase_directbase.obj')
  TargetAdd('libp3direct.dll', input='dcparser_composite.obj')
  TargetAdd('libp3direct.dll', input='dcparser_dcParser.obj')
  TargetAdd('libp3direct.dll', input='dcparser_dcLexer.obj')
  TargetAdd('libp3direct.dll', input='libdcparser_igate.obj')
  TargetAdd('libp3direct.dll', input='showbase_showBase.obj')
  if (sys.platform == "darwin"):
    TargetAdd('libp3direct.dll', input='showbase_showBase_assist.obj')
  TargetAdd('libp3direct.dll', input='libshowbase_igate.obj')
  TargetAdd('libp3direct.dll', input='deadrec_composite.obj')
  TargetAdd('libp3direct.dll', input='libdeadrec_igate.obj')
  TargetAdd('libp3direct.dll', input='interval_composite.obj')
  TargetAdd('libp3direct.dll', input='libinterval_igate.obj')
  TargetAdd('libp3direct.dll', input='distributed_config_distributed.obj')
  TargetAdd('libp3direct.dll', input='distributed_cConnectionRepository.obj')
  TargetAdd('libp3direct.dll', input='distributed_cDistributedSmoothNodeBase.obj')
  TargetAdd('libp3direct.dll', input='libdistributed_igate.obj')
  TargetAdd('libp3direct.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3direct.dll', opts=['ADVAPI',  'OPENSSL', 'WINUSER'])

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
# DIRECTORY: direct/src/heapq/
#

if (PkgSkip("PYTHON")==0 and PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/heapq']
  TargetAdd('heapq_heapq.obj', opts=OPTS, input='heapq.cxx')
  TargetAdd('libp3heapq.dll', input='heapq_heapq.obj')
  TargetAdd('libp3heapq.dll', input='libpandaexpress.dll')
  TargetAdd('libp3heapq.dll', input=COMMON_DTOOL_LIBS)
  TargetAdd('libp3heapq.dll', opts=['ADVAPI'])

#
# DIRECTORY: direct/src/plugin/
#

if (RTDIST or RUNTIME):
  # Explicitly define this as we don't include dtool_config.h here.
  if (sys.platform != "darwin" and not sys.platform.startswith("win")):
    DefSymbol("RUNTIME", "HAVE_X11", "1")

  OPTS=['DIR:direct/src/plugin', 'BUILDING:P3D_PLUGIN', 'RUNTIME', 'OPENSSL']
  TargetAdd('plugin_common.obj', opts=OPTS, input='plugin_common_composite1.cxx')

  OPTS += ['ZLIB', 'JPEG', 'PNG', 'MSIMG']
  TargetAdd('plugin_plugin.obj', opts=OPTS, input='p3d_plugin_composite1.cxx')
  TargetAdd('plugin_mkdir_complete.obj', opts=OPTS, input='mkdir_complete.cxx')
  TargetAdd('plugin_find_root_dir.obj', opts=OPTS, input='find_root_dir.cxx')
  if (sys.platform == "darwin"):
    TargetAdd('plugin_find_root_dir_assist.obj', opts=OPTS, input='find_root_dir_assist.mm')
  TargetAdd('plugin_binaryXml.obj', opts=OPTS, input='binaryXml.cxx')
  TargetAdd('plugin_fileSpec.obj', opts=OPTS, input='fileSpec.cxx')
  TargetAdd('plugin_handleStream.obj', opts=OPTS, input='handleStream.cxx')
  TargetAdd('plugin_handleStreamBuf.obj', opts=OPTS, input='handleStreamBuf.cxx')
  if (RTDIST):
    for fname in ["p3d_plugin.dll", "libp3d_plugin_static.ilb"]:
      TargetAdd(fname, input='plugin_plugin.obj')
      TargetAdd(fname, input='plugin_mkdir_complete.obj')
      TargetAdd(fname, input='plugin_find_root_dir.obj')
      if (sys.platform == "darwin"):
        TargetAdd(fname, input='plugin_find_root_dir_assist.obj')
      TargetAdd(fname, input='plugin_fileSpec.obj')
      TargetAdd(fname, input='plugin_binaryXml.obj')
      TargetAdd(fname, input='plugin_handleStream.obj')
      TargetAdd(fname, input='plugin_handleStreamBuf.obj')
      TargetAdd(fname, input='libp3tinyxml.ilb')
      if (sys.platform == "darwin"):
        TargetAdd(fname, input='libsubprocbuffer.ilb')
      TargetAdd(fname, opts=['OPENSSL', 'ZLIB', 'JPEG', 'PNG', 'X11', 'ADVAPI', 'WINUSER', 'WINGDI', 'WINSHELL', 'WINCOMCTL', 'WINOLE', 'MSIMG'])

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

    if (sys.platform.startswith("win")):
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
    if (sys.platform=="darwin"): OPTS += ['OPT:2']
    if (PkgSkip("FLTK")==0):
      OPTS.append("FLTK")
      TargetAdd('plugin_p3dCert.obj', opts=OPTS, input='p3dCert.cxx')
      TargetAdd('p3dcert.exe', input='plugin_p3dCert.obj')
      OPTS=['NOSTRIP', 'OPENSSL', 'FLTK', 'WINCOMCTL', 'WINSOCK']
    else:
      OPTS.append("WX")
      TargetAdd('plugin_p3dCert.obj', opts=OPTS, input='p3dCert_wx.cxx')
      TargetAdd('p3dcert.exe', input='plugin_p3dCert.obj')
      OPTS=['NOSTRIP', 'OPENSSL', 'WX', 'CARBON', 'WINOLE', 'WINOLEAUT', 'WINUSER', 'ADVAPI', 'WINSHELL', 'WINCOMCTL', 'WINGDI', 'WINCOMDLG']

    if (sys.platform=="darwin"): OPTS += ['OPT:2']
    TargetAdd('p3dcert.exe', opts=OPTS)

#
# DIRECTORY: direct/src/plugin_npapi/
#

if (RUNTIME and PkgSkip("NPAPI")==0):
  OPTS=['DIR:direct/src/plugin_npapi', 'RUNTIME']
  if (sys.platform.startswith("win")):
    nppanda3d_rc = {"name" : "Panda3D Game Engine Plug-in",
                    "version" : VERSION,
                    "description" : "Runs 3-D games and interactive applets",
                    "filename" : "nppanda3d.dll",
                    "mimetype" : "application/x-panda3d",
                    "extension" : "p3d",
                    "filedesc" : "Panda3D applet"}
    TargetAdd('nppanda3d.res', opts=OPTS, winrc=nppanda3d_rc)
  elif (sys.platform=="darwin"):
    TargetAdd('nppanda3d.rsrc', opts=OPTS, input='nppanda3d.r')

  OPTS += ['NPAPI']
  TargetAdd('plugin_npapi_nppanda3d_composite1.obj', opts=OPTS, input='nppanda3d_composite1.cxx')

  TargetAdd('nppanda3d.plugin', input='plugin_common.obj')
  TargetAdd('nppanda3d.plugin', input='plugin_npapi_nppanda3d_composite1.obj')
  if (sys.platform.startswith("win")):
    TargetAdd('nppanda3d.plugin', input='nppanda3d.res')
    TargetAdd('nppanda3d.plugin', input='nppanda3d.def', ipath=OPTS)
  elif (sys.platform == "darwin"):
    TargetAdd('nppanda3d.plugin', input='nppanda3d.rsrc')
    TargetAdd('nppanda3d.plugin', input='nppanda3d.plist', ipath=OPTS)
    TargetAdd('nppanda3d.plugin', input='plugin_find_root_dir_assist.obj')
  TargetAdd('nppanda3d.plugin', input='libp3tinyxml.ilb')
  TargetAdd('nppanda3d.plugin', opts=['NPAPI', 'OPENSSL', 'WINUSER', 'WINSHELL', 'WINOLE', 'CARBON'])

#
# DIRECTORY: direct/src/plugin_activex/
#

if (RUNTIME and sys.platform.startswith("win")):
  OPTS=['DIR:direct/src/plugin_activex', 'RUNTIME', 'ACTIVEX']
  DefSymbol('ACTIVEX', '_USRDLL', '')
  DefSymbol('ACTIVEX', '_WINDLL', '')
  DefSymbol('ACTIVEX', '_AFXDLL', '')
  DefSymbol('ACTIVEX', '_MBCS', '')
  TargetAdd('P3DActiveX.tlb', opts=OPTS, input='P3DActiveX.idl')
  TargetAdd('P3DActiveX.res', opts=OPTS, input='P3DActiveX.rc')

  TargetAdd('plugin_activex_p3dactivex_composite1.obj', opts=OPTS, input='p3dactivex_composite1.cxx')

  TargetAdd('p3dactivex.ocx', input='plugin_common.obj')
  TargetAdd('p3dactivex.ocx', input='plugin_activex_p3dactivex_composite1.obj')
  TargetAdd('p3dactivex.ocx', input='P3DActiveX.res')
  TargetAdd('p3dactivex.ocx', input='P3DActiveX.def', ipath=OPTS)
  TargetAdd('p3dactivex.ocx', input='libp3tinyxml.ilb')
  TargetAdd('p3dactivex.ocx', opts=['MFC', 'WINSOCK2', 'OPENSSL'])

#
# DIRECTORY: direct/src/plugin_standalone/
#

if (RUNTIME):
  OPTS=['DIR:direct/src/plugin_standalone', 'RUNTIME', 'OPENSSL']
  TargetAdd('plugin_standalone_panda3d.obj', opts=OPTS, input='panda3d.cxx')
  TargetAdd('plugin_standalone_panda3dBase.obj', opts=OPTS, input='panda3dBase.cxx')

  if (sys.platform.startswith("win")):
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
  if (sys.platform == "darwin"):
    TargetAdd('panda3d.exe', input='plugin_find_root_dir_assist.obj')
  elif (sys.platform.startswith("win")):
    TargetAdd('panda3d.exe', input='panda3d.res')
  TargetAdd('panda3d.exe', input='libpandaexpress.dll')
  TargetAdd('panda3d.exe', input='libp3dtoolconfig.dll')
  TargetAdd('panda3d.exe', input='libp3dtool.dll')
  TargetAdd('panda3d.exe', input='libp3pystub.dll')
  TargetAdd('panda3d.exe', input='libp3tinyxml.ilb')
  TargetAdd('panda3d.exe', opts=['NOICON', 'OPENSSL', 'ZLIB', 'WINGDI', 'WINUSER', 'WINSHELL', 'ADVAPI', 'WINSOCK2', 'WINOLE', 'CARBON'])

  if (sys.platform == "darwin"):
    TargetAdd('plugin_standalone_panda3dMac.obj', opts=OPTS, input='panda3dMac.cxx')
    TargetAdd('Panda3D.app', input='plugin_standalone_panda3d.obj')
    TargetAdd('Panda3D.app', input='plugin_standalone_panda3dMac.obj')
    TargetAdd('Panda3D.app', input='plugin_standalone_panda3dBase.obj')
    TargetAdd('Panda3D.app', input='plugin_common.obj')
    TargetAdd('Panda3D.app', input='plugin_find_root_dir_assist.obj')
    TargetAdd('Panda3D.app', input='libpandaexpress.dll')
    TargetAdd('Panda3D.app', input='libp3dtoolconfig.dll')
    TargetAdd('Panda3D.app', input='libp3dtool.dll')
    TargetAdd('Panda3D.app', input='libp3pystub.dll')
    TargetAdd('Panda3D.app', input='libp3tinyxml.ilb')
    TargetAdd('Panda3D.app', input='panda3d_mac.plist', ipath=OPTS)
    TargetAdd('Panda3D.app', input='models/plugin_images/panda3d.icns')
    TargetAdd('Panda3D.app', opts=['NOSTRIP', 'OPENSSL', 'ZLIB', 'WINGDI', 'WINUSER', 'WINSHELL', 'ADVAPI', 'WINSOCK2', 'WINOLE', 'CARBON'])
  elif (sys.platform.startswith("win")):
    TargetAdd('plugin_standalone_panda3dWinMain.obj', opts=OPTS, input='panda3dWinMain.cxx')
    TargetAdd('panda3dw.exe', input='plugin_standalone_panda3d.obj')
    TargetAdd('panda3dw.exe', input='plugin_standalone_panda3dWinMain.obj')
    TargetAdd('panda3dw.exe', input='plugin_standalone_panda3dBase.obj')
    TargetAdd('panda3dw.exe', input='plugin_common.obj')
    TargetAdd('panda3dw.exe', input='libpandaexpress.dll')
    TargetAdd('panda3dw.exe', input='libp3dtoolconfig.dll')
    TargetAdd('panda3dw.exe', input='libp3dtool.dll')
    TargetAdd('panda3dw.exe', input='libp3pystub.dll')
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
  TargetAdd('plugin_standalone_dtoolbase_composite1.obj', opts=OPTS, input='dtoolbase_composite1.cxx')
  TargetAdd('plugin_standalone_dtoolbase_composite2.obj', opts=OPTS, input='dtoolbase_composite2.cxx')
  TargetAdd('plugin_standalone_lookup3.obj', opts=OPTS, input='lookup3.c')
  TargetAdd('plugin_standalone_indent.obj', opts=OPTS, input='indent.cxx')
  TargetAdd('plugin_standalone_dtoolutil_composite.obj', opts=OPTS, input='dtoolutil_composite.cxx')
  if (sys.platform == 'darwin'):
      TargetAdd('plugin_standalone_dtoolutil_filename_assist.obj', opts=OPTS, input='filename_assist.mm')
  TargetAdd('plugin_standalone_prc_composite.obj', opts=OPTS, input='prc_composite.cxx')
  TargetAdd('plugin_standalone_dconfig_composite.obj', opts=OPTS, input='dconfig_composite.cxx')
  TargetAdd('plugin_standalone_express_composite.obj', opts=OPTS, input='express_composite.cxx')
  TargetAdd('plugin_standalone_downloader_composite.obj', opts=OPTS, input='downloader_composite.cxx')
  TargetAdd('p3dembed.exe', input='plugin_standalone_panda3dBase.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_p3dEmbedMain.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_p3dEmbed.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_pystub.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_dtoolbase_composite1.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_dtoolbase_composite2.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_lookup3.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_indent.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_dtoolutil_composite.obj')
  if (sys.platform == 'darwin'):
      TargetAdd('p3dembed.exe', input='plugin_standalone_dtoolutil_filename_assist.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_prc_composite.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_dconfig_composite.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_express_composite.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_downloader_composite.obj')
  TargetAdd('p3dembed.exe', input='plugin_common.obj')
  if (sys.platform == "darwin"):
    TargetAdd('p3dembed.exe', input='plugin_find_root_dir_assist.obj')
  if (sys.platform == "darwin"):
    TargetAdd('p3dembed.exe', input='libsubprocbuffer.ilb')
  TargetAdd('p3dembed.exe', input='libp3tinyxml.ilb')
  TargetAdd('p3dembed.exe', input='libp3d_plugin_static.ilb')
  TargetAdd('p3dembed.exe', opts=['NOICON', 'NOSTRIP', 'WINGDI', 'WINSOCK2', 'ZLIB', 'WINUSER', 'OPENSSL', 'JPEG', 'WINOLE', 'CARBON', 'MSIMG', 'WINCOMCTL', 'ADVAPI', 'WINSHELL', 'X11', 'PNG'])

  if (sys.platform.startswith("win")):
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
    TargetAdd('p3dembedw.exe', input='plugin_standalone_dtoolutil_composite.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_prc_composite.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_dconfig_composite.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_express_composite.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_downloader_composite.obj')
    TargetAdd('p3dembedw.exe', input='plugin_common.obj')
    TargetAdd('p3dembedw.exe', input='libp3tinyxml.ilb')
    TargetAdd('p3dembedw.exe', input='libp3d_plugin_static.ilb')
    TargetAdd('p3dembedw.exe', opts=['NOICON', 'NOSTRIP', 'WINGDI', 'WINSOCK2', 'ZLIB', 'WINUSER', 'OPENSSL', 'JPEG', 'WINOLE', 'MSIMG', 'WINCOMCTL', 'ADVAPI', 'WINSHELL', 'PNG'])

#
# DIRECTORY: pandatool/src/pandatoolbase/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/pandatoolbase']
  TargetAdd('pandatoolbase_composite1.obj', opts=OPTS, input='pandatoolbase_composite1.cxx')
  TargetAdd('libpandatoolbase.lib', input='pandatoolbase_composite1.obj')

#
# DIRECTORY: pandatool/src/converter/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/converter']
  TargetAdd('converter_somethingToEggConverter.obj', opts=OPTS, input='somethingToEggConverter.cxx')
  TargetAdd('libconverter.lib', input='converter_somethingToEggConverter.obj')

#
# DIRECTORY: pandatool/src/progbase/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/progbase', 'ZLIB']
  TargetAdd('progbase_composite1.obj', opts=OPTS, input='progbase_composite1.cxx')
  TargetAdd('libprogbase.lib', input='progbase_composite1.obj')

#
# DIRECTORY: pandatool/src/eggbase/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/eggbase']
  TargetAdd('eggbase_composite1.obj', opts=OPTS, input='eggbase_composite1.cxx')
  TargetAdd('libeggbase.lib', input='eggbase_composite1.obj')

#
# DIRECTORY: pandatool/src/bam/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/bam']
  TargetAdd('bam-info_bamInfo.obj', opts=OPTS, input='bamInfo.cxx')
  TargetAdd('bam-info.exe', input='bam-info_bamInfo.obj')
  TargetAdd('bam-info.exe', input='libprogbase.lib')
  TargetAdd('bam-info.exe', input='libpandatoolbase.lib')
  TargetAdd('bam-info.exe', input='libpandaegg.dll')
  TargetAdd('bam-info.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('bam-info.exe', opts=['ADVAPI',  'FFTW'])

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
  TargetAdd('cvscopy_composite1.obj', opts=OPTS, input='cvscopy_composite1.cxx')
  TargetAdd('libcvscopy.lib', input='cvscopy_composite1.obj')

#
# DIRECTORY: pandatool/src/daeegg/
#
if (PkgSkip("PANDATOOL")==0 and PkgSkip("FCOLLADA")==0):
  OPTS=['DIR:pandatool/src/daeegg', 'FCOLLADA']
  TargetAdd('daeegg_composite1.obj', opts=OPTS, input='daeegg_composite1.cxx')
  TargetAdd('libdaeegg.lib', input='daeegg_composite1.obj')
  TargetAdd('libdaeegg.lib', opts=['FCOLLADA', 'CARBON'])

#
# DIRECTORY: pandatool/src/daeprogs/
#
if (PkgSkip("PANDATOOL")==0 and PkgSkip("FCOLLADA")==0):
  OPTS=['DIR:pandatool/src/daeprogs', 'FCOLLADA']
  TargetAdd('dae2egg_daeToEgg.obj', opts=OPTS, input='daeToEgg.cxx')
  TargetAdd('dae2egg.exe', input='dae2egg_daeToEgg.obj')
  TargetAdd('dae2egg.exe', input='libdaeegg.lib')
  TargetAdd('dae2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('dae2egg.exe', opts=['WINUSER', 'FCOLLADA', 'CARBON'])

#
# DIRECTORY: pandatool/src/dxf/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/dxf']
  TargetAdd('dxf_composite1.obj', opts=OPTS, input='dxf_composite1.cxx')
  TargetAdd('libdxf.lib', input='dxf_composite1.obj')

#
# DIRECTORY: pandatool/src/dxfegg/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/dxfegg']
  TargetAdd('dxfegg_dxfToEggConverter.obj', opts=OPTS, input='dxfToEggConverter.cxx')
  TargetAdd('dxfegg_dxfToEggLayer.obj', opts=OPTS, input='dxfToEggLayer.cxx')
  TargetAdd('libdxfegg.lib', input='dxfegg_dxfToEggConverter.obj')
  TargetAdd('libdxfegg.lib', input='dxfegg_dxfToEggLayer.obj')

#
# DIRECTORY: pandatool/src/dxfprogs/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/dxfprogs']
  TargetAdd('dxf-points_dxfPoints.obj', opts=OPTS, input='dxfPoints.cxx')
  TargetAdd('dxf-points.exe', input='dxf-points_dxfPoints.obj')
  TargetAdd('dxf-points.exe', input='libprogbase.lib')
  TargetAdd('dxf-points.exe', input='libdxf.lib')
  TargetAdd('dxf-points.exe', input='libpandatoolbase.lib')
  TargetAdd('dxf-points.exe', input=COMMON_PANDA_LIBS_PYSTUB)
  TargetAdd('dxf-points.exe', opts=['ADVAPI',  'FFTW'])

  TargetAdd('dxf2egg_dxfToEgg.obj', opts=OPTS, input='dxfToEgg.cxx')
  TargetAdd('dxf2egg.exe', input='dxf2egg_dxfToEgg.obj')
  TargetAdd('dxf2egg.exe', input='libdxfegg.lib')
  TargetAdd('dxf2egg.exe', input='libdxf.lib')
  TargetAdd('dxf2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('dxf2egg.exe', opts=['ADVAPI',  'FFTW'])

  TargetAdd('egg2dxf_eggToDXF.obj', opts=OPTS, input='eggToDXF.cxx')
  TargetAdd('egg2dxf_eggToDXFLayer.obj', opts=OPTS, input='eggToDXFLayer.cxx')
  TargetAdd('egg2dxf.exe', input='egg2dxf_eggToDXF.obj')
  TargetAdd('egg2dxf.exe', input='egg2dxf_eggToDXFLayer.obj')
  TargetAdd('egg2dxf.exe', input='libdxf.lib')
  TargetAdd('egg2dxf.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg2dxf.exe', opts=['ADVAPI',  'FFTW'])

#
# DIRECTORY: pandatool/src/objegg/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/objegg']
  TargetAdd('objegg_objToEggConverter.obj', opts=OPTS, input='objToEggConverter.cxx')
  TargetAdd('objegg_config_objegg.obj', opts=OPTS, input='config_objegg.cxx')
  TargetAdd('libobjegg.lib', input='objegg_objToEggConverter.obj')
  TargetAdd('libobjegg.lib', input='objegg_config_objegg.obj')

#
# DIRECTORY: pandatool/src/objprogs/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/objprogs']
  TargetAdd('obj2egg_objToEgg.obj', opts=OPTS, input='objToEgg.cxx')
  TargetAdd('obj2egg.exe', input='obj2egg_objToEgg.obj')
  TargetAdd('obj2egg.exe', input='libobjegg.lib')
  TargetAdd('obj2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)

#
# DIRECTORY: pandatool/src/palettizer/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/palettizer']
  TargetAdd('palettizer_composite1.obj', opts=OPTS, input='palettizer_composite1.cxx')
  TargetAdd('libpalettizer.lib', input='palettizer_composite1.obj')

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
  TargetAdd('egg-mkfont.exe', input='libpalettizer.lib')
  TargetAdd('egg-mkfont.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-mkfont.exe', opts=['ADVAPI', 'FREETYPE'])

#
# DIRECTORY: pandatool/src/eggcharbase/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/eggcharbase', 'ZLIB']
  TargetAdd('eggcharbase_composite1.obj', opts=OPTS, input='eggcharbase_composite1.cxx')
  TargetAdd('libeggcharbase.lib', input='eggcharbase_composite1.obj')

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
  TargetAdd('egg-optchar.exe', input='libeggcharbase.lib')
  TargetAdd('egg-optchar.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-optchar.exe', opts=['ADVAPI', 'FREETYPE'])

#
# DIRECTORY: pandatool/src/egg-palettize/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/egg-palettize', 'DIR:pandatool/src/palettizer']
  TargetAdd('egg-palettize_eggPalettize.obj', opts=OPTS, input='eggPalettize.cxx')
  TargetAdd('egg-palettize.exe', input='egg-palettize_eggPalettize.obj')
  TargetAdd('egg-palettize.exe', input='libpalettizer.lib')
  TargetAdd('egg-palettize.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-palettize.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/egg-qtess/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/egg-qtess']
  TargetAdd('egg-qtess_composite1.obj', opts=OPTS, input='egg-qtess_composite1.cxx')
  TargetAdd('egg-qtess.exe', input='egg-qtess_composite1.obj')
  TargetAdd('egg-qtess.exe', input='libeggbase.lib')
  TargetAdd('egg-qtess.exe', input='libprogbase.lib')
  TargetAdd('egg-qtess.exe', input='libconverter.lib')
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
  TargetAdd('egg-topstrip.exe', input='libeggcharbase.lib')
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
  TargetAdd('egg-retarget-anim.exe', input='libeggcharbase.lib')
  TargetAdd('egg-retarget-anim.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg-retarget-anim.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/flt/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/flt', 'ZLIB']
  TargetAdd('flt_fltVectorRecord.obj', opts=OPTS, input='fltVectorRecord.cxx')
  TargetAdd('flt_composite1.obj', opts=OPTS, input='flt_composite1.cxx')
  TargetAdd('libflt.lib', input=['flt_fltVectorRecord.obj', 'flt_composite1.obj'])

#
# DIRECTORY: pandatool/src/fltegg/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/fltegg']
  TargetAdd('fltegg_fltToEggConverter.obj', opts=OPTS, input='fltToEggConverter.cxx')
  TargetAdd('fltegg_fltToEggLevelState.obj', opts=OPTS, input='fltToEggLevelState.cxx')
  TargetAdd('libfltegg.lib', input=['fltegg_fltToEggConverter.obj', 'fltegg_fltToEggLevelState.obj'])

#
# DIRECTORY: pandatool/src/fltprogs/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/fltprogs', 'DIR:pandatool/src/flt', 'DIR:pandatool/src/cvscopy']
  TargetAdd('egg2flt_eggToFlt.obj', opts=OPTS, input='eggToFlt.cxx')
  TargetAdd('egg2flt.exe', input='egg2flt_eggToFlt.obj')
  TargetAdd('egg2flt.exe', input='libflt.lib')
  TargetAdd('egg2flt.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('egg2flt.exe', opts=['ADVAPI'])

  TargetAdd('flt-info_fltInfo.obj', opts=OPTS, input='fltInfo.cxx')
  TargetAdd('flt-info.exe', input='flt-info_fltInfo.obj')
  TargetAdd('flt-info.exe', input='libflt.lib')
  TargetAdd('flt-info.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('flt-info.exe', opts=['ADVAPI'])

  TargetAdd('flt-trans_fltTrans.obj', opts=OPTS, input='fltTrans.cxx')
  TargetAdd('flt-trans.exe', input='flt-trans_fltTrans.obj')
  TargetAdd('flt-trans.exe', input='libflt.lib')
  TargetAdd('flt-trans.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('flt-trans.exe', opts=['ADVAPI'])

  TargetAdd('flt2egg_fltToEgg.obj', opts=OPTS, input='fltToEgg.cxx')
  TargetAdd('flt2egg.exe', input='flt2egg_fltToEgg.obj')
  TargetAdd('flt2egg.exe', input='libflt.lib')
  TargetAdd('flt2egg.exe', input='libfltegg.lib')
  TargetAdd('flt2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('flt2egg.exe', opts=['ADVAPI'])

  TargetAdd('fltcopy_fltCopy.obj', opts=OPTS, input='fltCopy.cxx')
  TargetAdd('fltcopy.exe', input='fltcopy_fltCopy.obj')
  TargetAdd('fltcopy.exe', input='libcvscopy.lib')
  TargetAdd('fltcopy.exe', input='libflt.lib')
  TargetAdd('fltcopy.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('fltcopy.exe', opts=['ADVAPI'])


#
# DIRECTORY: pandatool/src/imagebase/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/imagebase']
  TargetAdd('imagebase_composite1.obj', opts=OPTS, input='imagebase_composite1.cxx')
  TargetAdd('libimagebase.lib', input='imagebase_composite1.obj')

#
# DIRECTORY: pandatool/src/imageprogs/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/imageprogs']
  TargetAdd('image-info_imageInfo.obj', opts=OPTS, input='imageInfo.cxx')
  TargetAdd('image-info.exe', input='image-info_imageInfo.obj')
  TargetAdd('image-info.exe', input='libimagebase.lib')
  TargetAdd('image-info.exe', input='libprogbase.lib')
  TargetAdd('image-info.exe', input='libpandatoolbase.lib')
  TargetAdd('image-info.exe', input='libpandaegg.dll')
  TargetAdd('image-info.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('image-info.exe', input='libp3pystub.dll')
  TargetAdd('image-info.exe', opts=['ADVAPI'])

  TargetAdd('image-resize_imageResize.obj', opts=OPTS, input='imageResize.cxx')
  TargetAdd('image-resize.exe', input='image-resize_imageResize.obj')
  TargetAdd('image-resize.exe', input='libimagebase.lib')
  TargetAdd('image-resize.exe', input='libprogbase.lib')
  TargetAdd('image-resize.exe', input='libpandatoolbase.lib')
  TargetAdd('image-resize.exe', input='libpandaegg.dll')
  TargetAdd('image-resize.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('image-resize.exe', input='libp3pystub.dll')
  TargetAdd('image-resize.exe', opts=['ADVAPI'])

  TargetAdd('image-trans_imageTrans.obj', opts=OPTS, input='imageTrans.cxx')
  TargetAdd('image-trans.exe', input='image-trans_imageTrans.obj')
  TargetAdd('image-trans.exe', input='libimagebase.lib')
  TargetAdd('image-trans.exe', input='libprogbase.lib')
  TargetAdd('image-trans.exe', input='libpandatoolbase.lib')
  TargetAdd('image-trans.exe', input='libpandaegg.dll')
  TargetAdd('image-trans.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('image-trans.exe', input='libp3pystub.dll')
  TargetAdd('image-trans.exe', opts=['ADVAPI'])


#
# DIRECTORY: pandatool/src/lwo/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/lwo']
  TargetAdd('lwo_composite1.obj', opts=OPTS, input='lwo_composite1.cxx')
  TargetAdd('liblwo.lib', input='lwo_composite1.obj')

#
# DIRECTORY: pandatool/src/lwoegg/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/lwoegg']
  TargetAdd('lwoegg_composite1.obj', opts=OPTS, input='lwoegg_composite1.cxx')
  TargetAdd('liblwoegg.lib', input='lwoegg_composite1.obj')

#
# DIRECTORY: pandatool/src/lwoprogs/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/lwoprogs', 'DIR:pandatool/src/lwo']
  TargetAdd('lwo-scan_lwoScan.obj', opts=OPTS, input='lwoScan.cxx')
  TargetAdd('lwo-scan.exe', input='lwo-scan_lwoScan.obj')
  TargetAdd('lwo-scan.exe', input='liblwo.lib')
  TargetAdd('lwo-scan.exe', input='libprogbase.lib')
  TargetAdd('lwo-scan.exe', input='libpandatoolbase.lib')
  TargetAdd('lwo-scan.exe', input='libpandaegg.dll')
  TargetAdd('lwo-scan.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('lwo-scan.exe', input='libp3pystub.dll')
  TargetAdd('lwo-scan.exe', opts=['ADVAPI'])

  TargetAdd('lwo2egg_lwoToEgg.obj', opts=OPTS, input='lwoToEgg.cxx')
  TargetAdd('lwo2egg.exe', input='lwo2egg_lwoToEgg.obj')
  TargetAdd('lwo2egg.exe', input='liblwo.lib')
  TargetAdd('lwo2egg.exe', input='liblwoegg.lib')
  TargetAdd('lwo2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
  TargetAdd('lwo2egg.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/maya/
#

for VER in MAYAVERSIONS:
  VNUM=VER[4:]
  if (PkgSkip(VER)==0) and (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/maya', VER]
    TargetAdd('maya'+VNUM+'_composite1.obj', opts=OPTS, input='maya_composite1.cxx')
    TargetAdd('libmaya'+VNUM+'.lib', input='maya'+VNUM+'_composite1.obj')

#
# DIRECTORY: pandatool/src/mayaegg/
#

for VER in MAYAVERSIONS:
  VNUM=VER[4:]
  if (PkgSkip(VER)==0) and (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/mayaegg', 'DIR:pandatool/src/maya', VER]
    TargetAdd('mayaegg'+VNUM+'_loader.obj', opts=OPTS, input='mayaEggLoader.cxx')
    TargetAdd('mayaegg'+VNUM+'_composite1.obj', opts=OPTS, input='mayaegg_composite1.cxx')
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
    TargetAdd('maxegg'+VNUM+'_composite1.obj', opts=OPTS, input='maxegg_composite1.cxx')
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
  TargetAdd('pvrml_vrmlParser.obj', opts=OPTS, input='vrmlParser.yxx')
  TargetAdd('vrmlParser.h', input='pvrml_vrmlParser.obj', opts=['DEPENDENCYONLY'])
  TargetAdd('pvrml_vrmlLexer.obj', opts=OPTS, input='vrmlLexer.lxx')
  TargetAdd('pvrml_parse_vrml.obj', opts=OPTS, input='parse_vrml.cxx')
  TargetAdd('pvrml_standard_nodes.obj', opts=OPTS, input='standard_nodes.cxx')
  TargetAdd('pvrml_vrmlNode.obj', opts=OPTS, input='vrmlNode.cxx')
  TargetAdd('pvrml_vrmlNodeType.obj', opts=OPTS, input='vrmlNodeType.cxx')
  TargetAdd('libpvrml.lib', input='pvrml_parse_vrml.obj')
  TargetAdd('libpvrml.lib', input='pvrml_standard_nodes.obj')
  TargetAdd('libpvrml.lib', input='pvrml_vrmlNode.obj')
  TargetAdd('libpvrml.lib', input='pvrml_vrmlNodeType.obj')
  TargetAdd('libpvrml.lib', input='pvrml_vrmlParser.obj')
  TargetAdd('libpvrml.lib', input='pvrml_vrmlLexer.obj')

#
# DIRECTORY: pandatool/src/vrmlegg/
#

if (PkgSkip("PANDATOOL")==0):
  OPTS=['DIR:pandatool/src/vrmlegg', 'DIR:pandatool/src/vrml']
  TargetAdd('vrmlegg_indexedFaceSet.obj', opts=OPTS, input='indexedFaceSet.cxx')
  TargetAdd('vrmlegg_vrmlAppearance.obj', opts=OPTS, input='vrmlAppearance.cxx')
  TargetAdd('vrmlegg_vrmlToEggConverter.obj', opts=OPTS, input='vrmlToEggConverter.cxx')
  TargetAdd('libvrmlegg.lib', input='vrmlegg_indexedFaceSet.obj')
  TargetAdd('libvrmlegg.lib', input='vrmlegg_vrmlAppearance.obj')
  TargetAdd('libvrmlegg.lib', input='vrmlegg_vrmlToEggConverter.obj')

#
# DIRECTORY: pandatool/src/xfile/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/xfile', 'ZLIB', 'BISONPREFIX_xyy', 'FLEXDASHI']
    CreateFile(GetOutputDir()+"/include/xParser.h")
    TargetAdd('xfile_xParser.obj', opts=OPTS, input='xParser.yxx')
    TargetAdd('xParser.h', input='xfile_xParser.obj', opts=['DEPENDENCYONLY'])
    TargetAdd('xfile_xLexer.obj', opts=OPTS, input='xLexer.lxx')
    TargetAdd('xfile_composite1.obj', opts=OPTS, input='xfile_composite1.cxx')
    TargetAdd('libxfile.lib', input='xfile_composite1.obj')
    TargetAdd('libxfile.lib', input='xfile_xParser.obj')
    TargetAdd('libxfile.lib', input='xfile_xLexer.obj')

#
# DIRECTORY: pandatool/src/xfileegg/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/xfileegg', 'DIR:pandatool/src/xfile']
    TargetAdd('xfileegg_composite1.obj', opts=OPTS, input='xfileegg_composite1.cxx')
    TargetAdd('libxfileegg.lib', input='xfileegg_composite1.obj')

#
# DIRECTORY: pandatool/src/ptloader/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/ptloader', 'DIR:pandatool/src/flt', 'DIR:pandatool/src/lwo', 'DIR:pandatool/src/xfile', 'DIR:pandatool/src/xfileegg', 'DIR:pandatool/src/daeegg', 'BUILDING:PTLOADER', 'FCOLLADA']
    TargetAdd('ptloader_config_ptloader.obj', opts=OPTS, input='config_ptloader.cxx')
    TargetAdd('ptloader_loaderFileTypePandatool.obj', opts=OPTS, input='loaderFileTypePandatool.cxx')
    TargetAdd('libp3ptloader.dll', input='ptloader_config_ptloader.obj')
    TargetAdd('libp3ptloader.dll', input='ptloader_loaderFileTypePandatool.obj')
    TargetAdd('libp3ptloader.dll', input='libfltegg.lib')
    TargetAdd('libp3ptloader.dll', input='libflt.lib')
    TargetAdd('libp3ptloader.dll', input='liblwoegg.lib')
    TargetAdd('libp3ptloader.dll', input='liblwo.lib')
    TargetAdd('libp3ptloader.dll', input='libdxfegg.lib')
    TargetAdd('libp3ptloader.dll', input='libdxf.lib')
    TargetAdd('libp3ptloader.dll', input='libobjegg.lib')
    TargetAdd('libp3ptloader.dll', input='libvrmlegg.lib')
    TargetAdd('libp3ptloader.dll', input='libpvrml.lib')
    TargetAdd('libp3ptloader.dll', input='libxfileegg.lib')
    TargetAdd('libp3ptloader.dll', input='libxfile.lib')
    if (PkgSkip("FCOLLADA")==0): TargetAdd('libp3ptloader.dll', input='libdaeegg.lib')
    TargetAdd('libp3ptloader.dll', input='libeggbase.lib')
    TargetAdd('libp3ptloader.dll', input='libprogbase.lib')
    TargetAdd('libp3ptloader.dll', input='libconverter.lib')
    TargetAdd('libp3ptloader.dll', input='libpandatoolbase.lib')
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
    TargetAdd('bin2c.exe', input='libprogbase.lib')
    TargetAdd('bin2c.exe', input='libpandatoolbase.lib')
    TargetAdd('bin2c.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('bin2c.exe', input='libp3pystub.dll')
    TargetAdd('bin2c.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/pstatserver/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/pstatserver']
    TargetAdd('pstatserver_composite1.obj', opts=OPTS, input='pstatserver_composite1.cxx')
    TargetAdd('libpstatserver.lib', input='pstatserver_composite1.obj')

#
# DIRECTORY: pandatool/src/softprogs/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/softprogs', 'OPENSSL']
    TargetAdd('softcvs_softCVS.obj', opts=OPTS, input='softCVS.cxx')
    TargetAdd('softcvs_softFilename.obj', opts=OPTS, input='softFilename.cxx')
    TargetAdd('softcvs.exe', input='softcvs_softCVS.obj')
    TargetAdd('softcvs.exe', input='softcvs_softFilename.obj')
    TargetAdd('softcvs.exe', input='libprogbase.lib')
    TargetAdd('softcvs.exe', input='libpandatoolbase.lib')
    TargetAdd('softcvs.exe', input='libpandaegg.dll')
    TargetAdd('softcvs.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('softcvs.exe', input='libp3pystub.dll')
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
    TargetAdd('text-stats.exe', input='libprogbase.lib')
    TargetAdd('text-stats.exe', input='libpstatserver.lib')
    TargetAdd('text-stats.exe', input='libpandatoolbase.lib')
    TargetAdd('text-stats.exe', input='libpandaegg.dll')
    TargetAdd('text-stats.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('text-stats.exe', input='libp3pystub.dll')
    TargetAdd('text-stats.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/vrmlprogs/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/vrmlprogs', 'DIR:pandatool/src/vrml', 'DIR:pandatool/src/vrmlegg']
    TargetAdd('vrml-trans_vrmlTrans.obj', opts=OPTS, input='vrmlTrans.cxx')
    TargetAdd('vrml-trans.exe', input='vrml-trans_vrmlTrans.obj')
    TargetAdd('vrml-trans.exe', input='libpvrml.lib')
    TargetAdd('vrml-trans.exe', input='libprogbase.lib')
    TargetAdd('vrml-trans.exe', input='libpandatoolbase.lib')
    TargetAdd('vrml-trans.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('vrml-trans.exe', input='libp3pystub.dll')
    TargetAdd('vrml-trans.exe', opts=['ADVAPI'])

    TargetAdd('vrml2egg_vrmlToEgg.obj', opts=OPTS, input='vrmlToEgg.cxx')
    TargetAdd('vrml2egg.exe', input='vrml2egg_vrmlToEgg.obj')
    TargetAdd('vrml2egg.exe', input='libvrmlegg.lib')
    TargetAdd('vrml2egg.exe', input='libpvrml.lib')
    TargetAdd('vrml2egg.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
    TargetAdd('vrml2egg.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/win-stats/
# DIRECTORY: pandatool/src/gtk-stats/
#

if (PkgSkip("PANDATOOL")==0 and (sys.platform.startswith("win") or PkgSkip("GTK2")==0)):
    if (sys.platform.startswith("win")):
      OPTS=['DIR:pandatool/src/win-stats']
      TargetAdd('pstats_composite1.obj', opts=OPTS, input='winstats_composite1.cxx')
    else:
      OPTS=['DIR:pandatool/src/gtk-stats', 'GTK2']
      TargetAdd('pstats_composite1.obj', opts=OPTS, input='gtkstats_composite1.cxx')
    TargetAdd('pstats.exe', input='pstats_composite1.obj')
    TargetAdd('pstats.exe', input='libpstatserver.lib')
    TargetAdd('pstats.exe', input='libprogbase.lib')
    TargetAdd('pstats.exe', input='libpandatoolbase.lib')
    TargetAdd('pstats.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('pstats.exe', input='libp3pystub.dll')
    TargetAdd('pstats.exe', opts=['WINSOCK', 'WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM', 'GTK2'])

#
# DIRECTORY: pandatool/src/xfileprogs/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/xfileprogs', 'DIR:pandatool/src/xfile', 'DIR:pandatool/src/xfileegg']
    TargetAdd('egg2x_eggToX.obj', opts=OPTS, input='eggToX.cxx')
    TargetAdd('egg2x.exe', input='egg2x_eggToX.obj')
    TargetAdd('egg2x.exe', input='libxfileegg.lib')
    TargetAdd('egg2x.exe', input='libxfile.lib')
    TargetAdd('egg2x.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
    TargetAdd('egg2x.exe', opts=['ADVAPI'])

    TargetAdd('x-trans_xFileTrans.obj', opts=OPTS, input='xFileTrans.cxx')
    TargetAdd('x-trans.exe', input='x-trans_xFileTrans.obj')
    TargetAdd('x-trans.exe', input='libprogbase.lib')
    TargetAdd('x-trans.exe', input='libxfile.lib')
    TargetAdd('x-trans.exe', input='libpandatoolbase.lib')
    TargetAdd('x-trans.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('x-trans.exe', input='libp3pystub.dll')
    TargetAdd('x-trans.exe', opts=['ADVAPI'])

    TargetAdd('x2egg_xFileToEgg.obj', opts=OPTS, input='xFileToEgg.cxx')
    TargetAdd('x2egg.exe', input='x2egg_xFileToEgg.obj')
    TargetAdd('x2egg.exe', input='libxfileegg.lib')
    TargetAdd('x2egg.exe', input='libxfile.lib')
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
    if sys.platform.startswith("win32"):
      TargetAdd('mayaeggimport'+VNUM+'.mll', input='libp3pystub.dll')
    TargetAdd('mayaeggimport'+VNUM+'.mll', opts=['ADVAPI', VER])

    TargetAdd('mayaloader'+VNUM+'_config_mayaloader.obj', opts=OPTS, input='config_mayaloader.cxx')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='mayaloader'+VNUM+'_config_mayaloader.obj')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libmayaegg'+VNUM+'.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libp3ptloader.dll')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libmaya'+VNUM+'.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libfltegg.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libflt.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='liblwoegg.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='liblwo.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libdxfegg.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libdxf.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libobjegg.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libvrmlegg.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libpvrml.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libxfileegg.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libxfile.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libeggbase.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libprogbase.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libconverter.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libpandatoolbase.lib')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input='libpandaegg.dll')
    TargetAdd('libp3mayaloader'+VNUM+'.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libp3mayaloader'+VNUM+'.dll', opts=['ADVAPI', VER])

    TargetAdd('mayapview'+VNUM+'_mayaPview.obj', opts=OPTS, input='mayaPview.cxx')
    TargetAdd('libmayapview'+VNUM+'.mll', input='mayapview'+VNUM+'_mayaPview.obj')
    TargetAdd('libmayapview'+VNUM+'.mll', input='libmayaegg'+VNUM+'.lib')
    TargetAdd('libmayapview'+VNUM+'.mll', input='libmaya'+VNUM+'.lib')
    TargetAdd('libmayapview'+VNUM+'.mll', input='libp3framework.dll')
    if (sys.platform.startswith("win")):
      TargetAdd('libmayapview'+VNUM+'.mll', input=COMMON_EGG2X_LIBS_PYSTUB)
    else:
      TargetAdd('libmayapview'+VNUM+'.mll', input=COMMON_EGG2X_LIBS)
    TargetAdd('libmayapview'+VNUM+'.mll', opts=['ADVAPI', VER])

    TargetAdd('maya2egg'+VNUM+'_mayaToEgg.obj', opts=OPTS, input='mayaToEgg.cxx')
    TargetAdd('maya2egg'+VNUM+'-wrapped.exe', input='maya2egg'+VNUM+'_mayaToEgg.obj')
    TargetAdd('maya2egg'+VNUM+'-wrapped.exe', input='libmayaegg'+VNUM+'.lib')
    TargetAdd('maya2egg'+VNUM+'-wrapped.exe', input='libmaya'+VNUM+'.lib')
    if (sys.platform.startswith("win")):
      TargetAdd('maya2egg'+VNUM+'-wrapped.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
    else:
      TargetAdd('maya2egg'+VNUM+'-wrapped.exe', input=COMMON_EGG2X_LIBS)
    if (sys.platform == "darwin" and int(VNUM) >= 2009):
      TargetAdd('maya2egg'+VNUM+'-wrapped.exe', opts=['ADVAPI', 'NOPPC', VER])
    else:
      TargetAdd('maya2egg'+VNUM+'-wrapped.exe', opts=['ADVAPI', VER])

    TargetAdd('egg2maya'+VNUM+'_eggToMaya.obj', opts=OPTS, input='eggToMaya.cxx')
    TargetAdd('egg2maya'+VNUM+'-wrapped.exe', input='egg2maya'+VNUM+'_eggToMaya.obj')
    TargetAdd('egg2maya'+VNUM+'-wrapped.exe', input='libmayaegg'+VNUM+'.lib')
    TargetAdd('egg2maya'+VNUM+'-wrapped.exe', input='libmaya'+VNUM+'.lib')
    if (sys.platform.startswith("win")):
      TargetAdd('egg2maya'+VNUM+'-wrapped.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
    else:
      TargetAdd('egg2maya'+VNUM+'-wrapped.exe', input=COMMON_EGG2X_LIBS)
    if (sys.platform == "darwin" and int(VNUM) >= 2009):
      TargetAdd('egg2maya'+VNUM+'-wrapped.exe', opts=['ADVAPI', 'NOPPC', VER])
    else:
      TargetAdd('egg2maya'+VNUM+'-wrapped.exe', opts=['ADVAPI', VER])

    TargetAdd('mayacopy'+VNUM+'_mayaCopy.obj', opts=OPTS, input='mayaCopy.cxx')
    TargetAdd('mayacopy'+VNUM+'-wrapped.exe', input='mayacopy'+VNUM+'_mayaCopy.obj')
    TargetAdd('mayacopy'+VNUM+'-wrapped.exe', input='libcvscopy.lib')
    TargetAdd('mayacopy'+VNUM+'-wrapped.exe', input='libmaya'+VNUM+'.lib')
    if sys.platform == "win32":
      TargetAdd('mayacopy'+VNUM+'-wrapped.exe', input=COMMON_EGG2X_LIBS_PYSTUB)
    else:
      TargetAdd('mayacopy'+VNUM+'-wrapped.exe', input=COMMON_EGG2X_LIBS)
    if (sys.platform == "darwin" and int(VNUM) >= 2009):
      TargetAdd('mayacopy'+VNUM+'-wrapped.exe', opts=['ADVAPI', 'NOPPC', VER])
    else:
      TargetAdd('mayacopy'+VNUM+'-wrapped.exe', opts=['ADVAPI', VER])

    TargetAdd('mayasavepview'+VNUM+'_mayaSavePview.obj', opts=OPTS, input='mayaSavePview.cxx')
    TargetAdd('libmayasavepview'+VNUM+'.mll', input='mayasavepview'+VNUM+'_mayaSavePview.obj')
    TargetAdd('libmayasavepview'+VNUM+'.mll', opts=['ADVAPI',  VER])

    TargetAdd('mayaWrapper'+VNUM+'.obj', opts=OPTS, input='mayaWrapper.cxx')

    TargetAdd('maya2egg'+VNUM+'.exe', input='mayaWrapper'+VNUM+'.obj')
    TargetAdd('maya2egg'+VNUM+'.exe', opts=['ADVAPI'])

    TargetAdd('mayacopy'+VNUM+'.exe', input='mayaWrapper'+VNUM+'.obj')
    TargetAdd('mayacopy'+VNUM+'.exe', opts=['ADVAPI'])

#
# DIRECTORY: contrib/src/ai/
#
if (PkgSkip("CONTRIB")==0 and not RUNTIME):
  OPTS=['DIR:contrib/src/ai', 'BUILDING:PANDAAI']
  TargetAdd('ai_composite1.obj', opts=OPTS, input='ai_composite1.cxx')
  IGATEFILES=GetDirectoryContents('contrib/src/ai', ["*.h", "*_composite.cxx"])
  TargetAdd('libpandaai.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpandaai.in', opts=['IMOD:pandaai', 'ILIB:libpandaai', 'SRCDIR:contrib/src/ai'])
  TargetAdd('libpandaai_igate.obj', input='libpandaai.in', opts=["DEPENDENCYONLY"])

  TargetAdd('libpandaai_module.obj', input='libpandaai.in')
  TargetAdd('libpandaai_module.obj', opts=OPTS)
  TargetAdd('libpandaai_module.obj', opts=['IMOD:pandaai', 'ILIB:libpandaai'])

  TargetAdd('libpandaai.dll', input='libpandaai_module.obj')
  TargetAdd('libpandaai.dll', input='ai_composite1.obj')
  TargetAdd('libpandaai.dll', input='libpandaai_igate.obj')
  TargetAdd('libpandaai.dll', input=COMMON_PANDA_LIBS)

#
# Run genpycode
#

if (PkgSkip("PYTHON")==0 and not RUNTIME):
  # We're phasing out the concept of PandaModules, so do not
  # add new libraries here. See direct/src/ffi/panda3d.py
  TargetAdd('PandaModules.py', input='libpandaexpress.dll')
  TargetAdd('PandaModules.py', input='libpanda.dll')
  TargetAdd('PandaModules.py', input='libpandaphysics.dll')
  TargetAdd('PandaModules.py', input='libpandafx.dll')
  if (PkgSkip("DIRECT")==0):
    TargetAdd('PandaModules.py', input='libp3direct.dll')
  TargetAdd('PandaModules.py', input='libp3vision.dll')
  TargetAdd('PandaModules.py', input='libpandaskel.dll')
  TargetAdd('PandaModules.py', input='libpandaegg.dll')
  if (PkgSkip("AWESOMIUM")==0):
    TargetAdd('PandaModules.py', input='libp3awesomium.dll')
  if (PkgSkip("ODE")==0):
    TargetAdd('PandaModules.py', input='libpandaode.dll')

#
# Generate the models directory and samples directory
#

if (PkgSkip("DIRECT")==0 and not RUNTIME):
  model_extensions = ["*.egg"]
  if (PkgSkip("PANDATOOL")==0):
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
  if (sys.platform.startswith("win")):
    OPTS=['DIR:direct/src/p3d']
    TargetAdd('p3dWrapper.obj', opts=OPTS, input='p3dWrapper.c')
    TargetAdd('p3dWrapper.exe', input='p3dWrapper.obj')
    TargetAdd('p3dWrapper.exe', opts=["ADVAPI"])

  for g in glob.glob("direct/src/p3d/*.p3d"):
    base = os.path.basename(g)
    base = base.split(".", 1)[0]

    if (sys.platform.startswith("win")):
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
        if (x in pending):
            return 0
    altsources = task[4]
    for x in altsources:
        if (x in pending):
            return 0
    return 1

def ParallelMake(tasklist):
    # Create the communication queues.
    donequeue=Queue.Queue()
    taskqueue=Queue.Queue()
    # Build up a table listing all the pending targets
    pending = {}
    for task in tasklist:
        for target in task[2]:
            pending[target] = 1
    # Create the workers
    for slave in range(THREADCOUNT):
        th = threading.Thread(target=BuildWorker, args=[taskqueue,donequeue])
        th.setDaemon(1)
        th.start()
    # Feed tasks to the workers.
    tasksqueued = 0
    while (1):
        if (tasksqueued < THREADCOUNT*2):
            extras = []
            for task in tasklist:
                if (tasksqueued < THREADCOUNT*3) & (AllSourcesReady(task, pending)):
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
    if (len(tasklist)>0):
        exit("Dependency problem - task unsatisfied: "+str(tasklist[0][2]))


def SequentialMake(tasklist):
    i = 0
    for task in tasklist:
        if (NeedsBuild(task[2], task[3])):
            apply(task[0], task[1] + [(i * 100.0) / len(tasklist)])
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
        oscmd(SDK["PYTHONEXEC"] + " direct\\src\\plugin_installer\\make_installer.py --version %s" % VERSION)
        shutil.move("direct\\src\\plugin_installer\\p3d-setup.exe", file)
        return

    print "Building "+fullname+" installer. This can take up to an hour."
    if (COMPRESSOR != "lzma"):
        print("Note: you are using zlib, which is faster, but lzma gives better compression.")
    if (os.path.exists("nsis-output.exe")):
        os.remove("nsis-output.exe")
    WriteFile(GetOutputDir()+"/tmp/__init__.py", "")
    psource=os.path.abspath(".")
    panda=os.path.abspath(GetOutputDir())
    cmd=GetThirdpartyBase()+"/win-nsis/makensis /V2 "
    cmd=cmd+'/DCOMPRESSOR="'+COMPRESSOR+'" '
    cmd=cmd+'/DNAME="'+fullname+'" '
    cmd=cmd+'/DSMDIRECTORY="'+smdirectory+'" '
    cmd=cmd+'/DINSTALLDIR="'+installdir+'" '
    cmd=cmd+'/DOUTFILE="'+psource+'\\nsis-output.exe" '
    cmd=cmd+'/DLICENSE="'+panda+'\\LICENSE" '
    cmd=cmd+'/DLANGUAGE="English" '
    cmd=cmd+'/DRUNTEXT="Visit the Panda Manual" '
    cmd=cmd+'/DIBITMAP="panda-install.bmp" '
    cmd=cmd+'/DUBITMAP="panda-install.bmp" '
    cmd=cmd+'/DPANDA="'+panda+'" '
    cmd=cmd+'/DPANDACONF="'+panda+'\\etc" '
    cmd=cmd+'/DPSOURCE="'+psource+'" '
    cmd=cmd+'/DPYEXTRAS="'+os.path.abspath(GetThirdpartyBase())+'\\win-extras" '
    cmd=cmd+'"'+psource+'\\direct\\src\\directscripts\\packpanda.nsi"'
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
Maintainer: etc-panda3d@lists.andrew.cmu.edu
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
Maintainer: etc-panda3d@lists.andrew.cmu.edu
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
            oscmd("cd targetroot ; dpkg-shlibdeps -xpanda3d-runtime debian/panda3d-runtime/usr/lib*/*.so* debian/panda3d-runtime/usr/bin/*")
            depends = ReadFile("targetroot/debian/substvars").replace("shlibs:Depends=", "").strip()
            WriteFile("targetroot/DEBIAN/control", txt.replace("DEPENDS", depends))
        else:
            oscmd("ln -s .. targetroot/debian/panda3d" + MAJOR_VERSION)
            oscmd("cd targetroot ; dpkg-gensymbols -v%s -ppanda3d%s -eusr%s/panda3d/lib*.so* -ODEBIAN/symbols >/dev/null" % (DEBVERSION, MAJOR_VERSION, libdir))
            # Library dependencies are required, binary dependencies are recommended. Dunno why -xlibphysx-extras is needed, prolly a bug in their package
            oscmd("cd targetroot ; LD_LIBRARY_PATH=usr%s/panda3d dpkg-shlibdeps --ignore-missing-info --warnings=2 -xpanda3d%s -xlibphysx-extras -Tdebian/substvars_dep debian/panda3d%s/usr/lib*/panda3d/lib*.so*" % (libdir, MAJOR_VERSION, MAJOR_VERSION))
            oscmd("cd targetroot ; LD_LIBRARY_PATH=usr%s/panda3d dpkg-shlibdeps --ignore-missing-info --warnings=2 -xpanda3d%s -Tdebian/substvars_rec debian/panda3d%s/usr/bin/*" % (libdir, MAJOR_VERSION, MAJOR_VERSION))
            depends = ReadFile("targetroot/debian/substvars_dep").replace("shlibs:Depends=", "").strip()
            recommends = ReadFile("targetroot/debian/substvars_rec").replace("shlibs:Depends=", "").strip()
            if PkgSkip("PYTHON")==0:
                depends += ", " + PYTHONV + ", python-pmw"
            if PkgSkip("NVIDIACG")==0:
                depends += ", nvidia-cg-toolkit"
            WriteFile("targetroot/DEBIAN/control", txt.replace("DEPENDS", depends).replace("RECOMMENDS", recommends))
        oscmd("rm -rf targetroot/debian")
        oscmd("chmod -R 755 targetroot/DEBIAN")
        if (RUNTIME):
            oscmd("fakeroot dpkg-deb -b targetroot panda3d-runtime_"+DEBVERSION+"_"+ARCH+".deb")
        else:
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
        oscmd(SDK["PYTHONEXEC"] + " direct/src/plugin_installer/make_installer.py --version %s" % VERSION)
        return

    import compileall
    if (os.path.isfile("Panda3D-%s.dmg" % VERSION)): oscmd("rm -f Panda3D-%s.dmg" % VERSION)
    if (os.path.exists("dstroot")): oscmd("rm -rf dstroot")
    if (os.path.exists("Panda3D-rw.dmg")): oscmd('rm -f Panda3D-rw.dmg')

    #TODO: add postflight script
    #oscmd("sed -e 's@\\$1@%s@' < direct/src/directscripts/profilepaths-osx.command >> Panda3D-tpl-rw/panda3dpaths.command" % VERSION)

    oscmd("mkdir -p dstroot/base/Developer/Panda3D/lib")
    oscmd("mkdir -p dstroot/base/Developer/Panda3D/etc")
    oscmd("cp %s/etc/Config.prc           dstroot/base/Developer/Panda3D/etc/Config.prc" % GetOutputDir())
    oscmd("cp %s/etc/Confauto.prc         dstroot/base/Developer/Panda3D/etc/Confauto.prc" % GetOutputDir())
    oscmd("cp -R %s/models                dstroot/base/Developer/Panda3D/models" % GetOutputDir())
    oscmd("cp -R doc/LICENSE              dstroot/base/Developer/Panda3D/LICENSE")
    oscmd("cp -R doc/ReleaseNotes         dstroot/base/Developer/Panda3D/ReleaseNotes")
    if os.path.isdir(GetOutputDir()+"/plugins"):
        oscmd("cp -R %s/plugins           dstroot/base/Developer/Panda3D/plugins" % GetOutputDir())
    for base in os.listdir(GetOutputDir()+"/lib"):
        if (not base.endswith(".a")):
            libname = "dstroot/base/Developer/Panda3D/lib/" + base
            # We really need to specify -R in order not to follow symlinks
            # On OSX, just specifying -P is not enough to do that.
            oscmd("cp -R -P " + GetOutputDir() + "/lib/" + base + " " + libname)

            # Execute install_name_tool to make them reference an absolute path
            if (libname.endswith(".dylib") and not os.path.islink(libname)):
                oscmd("install_name_tool -id /Developer/Panda3D/lib/%s %s" % (base, libname), True)
                oscmd("otool -L %s | grep .dylib > %s/tmp/otool-libs.txt" % (libname, GetOutputDir()), True)
                for line in open(GetOutputDir()+"/tmp/otool-libs.txt", "r"):
                    if len(line.strip()) > 0 and not line.strip().endswith(":"):
                        libdep = line.strip().split(" ", 1)[0]
                        if not libdep.startswith("/"):
                            oscmd("install_name_tool -change %s /Developer/Panda3D/lib/%s %s" % (libdep, os.path.basename(libdep), libname), True)

    # Temporary script that should clean up the poison that the early 1.7.0 builds injected into environment.plist
    oscmd("mkdir -p dstroot/scripts/base/")
    postinstall = open("dstroot/scripts/base/postinstall", "w")
    print >>postinstall, "#!/usr/bin/python"
    print >>postinstall, "import os, sys, plistlib"
    print >>postinstall, "home = os.environ['HOME']"
    print >>postinstall, "if not os.path.isdir(os.path.join(home, '.MacOSX')):"
    print >>postinstall, "    sys.exit()"
    print >>postinstall, "plist = dict()"
    print >>postinstall, "envfile = os.path.join(home, '.MacOSX', 'environment.plist')"
    print >>postinstall, "if os.path.exists(envfile):"
    print >>postinstall, "    try:"
    print >>postinstall, "        plist = plistlib.readPlist(envfile)"
    print >>postinstall, "    except: sys.exit(0)"
    print >>postinstall, "else:"
    print >>postinstall, "    sys.exit(0)"
    print >>postinstall, "paths = {'PATH' : '/Developer/Tools/Panda3D', 'DYLD_LIBRARY_PATH' : '/Developer/Panda3D/lib', 'PYTHONPATH' : '/Developer/Panda3D/lib',"
    print >>postinstall, "         'MAYA_SCRIPT_PATH' : '/Developer/Panda3D/plugins', 'MAYA_PLUG_IN_PATH' : '/Developer/Panda3D/plugins'}"
    print >>postinstall, "for env, path in paths.items():"
    print >>postinstall, "    if env in plist:"
    print >>postinstall, "        paths = plist[env].split(':')"
    print >>postinstall, "        if '' in paths: paths.remove('')"
    print >>postinstall, "        if path in paths: paths.remove(path)"
    print >>postinstall, "        if len(paths) == 0:"
    print >>postinstall, "            del plist[env]"
    print >>postinstall, "        else:"
    print >>postinstall, "            plist[env] = ':'.join(paths)"
    print >>postinstall, "if len(plist) == 0:"
    print >>postinstall, "    os.remove(envfile)"
    print >>postinstall, "else:"
    print >>postinstall, "    plistlib.writePlist(plist, envfile)"
    postinstall.close()
    postflight = open("dstroot/scripts/base/postflight", "w")
    print >>postflight, "#!/usr/bin/env bash\n"
    print >>postflight, "RESULT=`/usr/bin/open 'http://www.panda3d.org/wiki/index.php/Getting_Started_on_OSX'`"
    print >>postflight, "\nexit 0"
    postflight.close()
    oscmd("chmod +x dstroot/scripts/base/postinstall")
    oscmd("chmod +x dstroot/scripts/base/postflight")

    oscmd("mkdir -p dstroot/tools/Developer/Tools/Panda3D")
    oscmd("mkdir -p dstroot/tools/Developer/Panda3D")
    oscmd("mkdir -p dstroot/tools/etc/paths.d")
    WriteFile("dstroot/tools/etc/paths.d/Panda3D", "/Developer/Tools/Panda3D")
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
                    if not libdep.startswith("/"):
                        oscmd("install_name_tool -change %s /Developer/Panda3D/lib/%s %s" % (libdep, os.path.basename(libdep), binname), True)

    if PkgSkip("PYTHON")==0:
        PV = SDK["PYTHONVERSION"].replace("python", "")
        oscmd("mkdir -p dstroot/pythoncode/usr/bin")
        oscmd("mkdir -p dstroot/pythoncode/Developer/Panda3D/lib/direct")
        oscmd("mkdir -p dstroot/pythoncode/Library/Python/%s/site-packages" % PV)
        WriteFile("dstroot/pythoncode/Library/Python/%s/site-packages/Panda3D.pth" % PV, "/Developer/Panda3D/lib")
        oscmd("cp -R %s/pandac                dstroot/pythoncode/Developer/Panda3D/lib/pandac" % GetOutputDir())
        oscmd("cp -R direct/src/*             dstroot/pythoncode/Developer/Panda3D/lib/direct")
        oscmd("cp direct/src/ffi/panda3d.py   dstroot/pythoncode/Developer/Panda3D/lib/panda3d.py")
        oscmd("ln -s %s                       dstroot/pythoncode/usr/bin/ppython" % SDK["PYTHONEXEC"])
        if os.path.isdir(GetOutputDir()+"/Pmw"):
            oscmd("cp -R %s/Pmw               dstroot/pythoncode/Developer/Panda3D/lib/Pmw" % GetOutputDir())
            compileall.compile_dir("dstroot/pythoncode/Developer/Panda3D/lib/Pmw")
        WriteFile("dstroot/pythoncode/Developer/Panda3D/lib/direct/__init__.py", "")
        for base in os.listdir("dstroot/pythoncode/Developer/Panda3D/lib/direct"):
            if ((base != "extensions") and (base != "extensions_native")):
                compileall.compile_dir("dstroot/pythoncode/Developer/Panda3D/lib/direct/"+base)

    oscmd("mkdir -p dstroot/headers/Developer/Panda3D")
    oscmd("cp -R %s/include               dstroot/headers/Developer/Panda3D/include" % GetOutputDir())

    if os.path.isdir("samples"):
        oscmd("mkdir -p dstroot/samples/Developer/Examples/Panda3D")
        oscmd("cp -R samples/* dstroot/samples/Developer/Examples/Panda3D/")

    # Dummy package uninstall16 which just contains a preflight script to remove /Applications/Panda3D/ .
    oscmd("mkdir -p dstroot/scripts/uninstall16/")
    preflight = open("dstroot/scripts/uninstall16/preflight", "w")
    print >>preflight, "#!/usr/bin/python"
    print >>preflight, "import os, re, sys, shutil"
    print >>preflight, "if os.path.isdir('/Applications/Panda3D'): shutil.rmtree('/Applications/Panda3D')"
    print >>preflight, "bash_profile = os.path.join(os.environ['HOME'], '.bash_profile')"
    print >>preflight, "if not os.path.isfile(bash_profile): sys.exit(0)"
    print >>preflight, "pattern = re.compile('''PANDA_VERSION=[0-9][.][0-9][.][0-9]"
    print >>preflight, "PANDA_PATH=/Applications/Panda3D/[$A-Z.0-9_]+"
    print >>preflight, "if \[ -d \$PANDA_PATH \]"
    print >>preflight, "then(.+?)fi"
    print >>preflight, "''', flags = re.DOTALL | re.MULTILINE)"
    print >>preflight, "bpfile = open(bash_profile, 'r')"
    print >>preflight, "bpdata = bpfile.read()"
    print >>preflight, "bpfile.close()"
    print >>preflight, "newbpdata = pattern.sub('', bpdata)"
    print >>preflight, "if newbpdata == bpdata: sys.exit(0)"
    print >>preflight, "bpfile = open(bash_profile, 'w')"
    print >>preflight, "bpfile.write(newbpdata)"
    print >>preflight, "bpfile.close()"
    preflight.close()
    oscmd("chmod +x dstroot/scripts/uninstall16/preflight")

    oscmd("chmod -R 0775 dstroot/*")
    # We need to be root to perform a chown. Bleh.
    # Fortunately PackageMaker does it for us, on 10.5 and above.
    #oscmd("chown -R root:admin dstroot/*", True)

    oscmd("mkdir -p dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/")
    oscmd("mkdir -p dstroot/Panda3D/Panda3D.mpkg/Contents/Resources/en.lproj/")

    pkgs = ["base", "tools", "headers", "uninstall16"]
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
            if os.path.isdir("dstroot/scripts/" + pkg):
                cmd += ' --scripts dstroot/scripts/' + pkg
        elif os.path.exists("/Developer/Tools/packagemaker"):
            cmd = '/Developer/Tools/packagemaker -build -f dstroot/' + pkg + '/ -p dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/' + pkg + '.pkg -i /tmp/Info_plist'
        else:
            exit("PackageMaker could not be found!")
        oscmd(cmd)

    if os.path.isfile("/tmp/Info_plist"):
        oscmd("rm -f /tmp/Info_plist")

    dist = open("dstroot/Panda3D/Panda3D.mpkg/Contents/distribution.dist", "w")
    print >>dist, '<?xml version="1.0" encoding="utf-8"?>'
    print >>dist, '<installer-script minSpecVersion="1.000000" authoringTool="com.apple.PackageMaker" authoringToolVersion="3.0.3" authoringToolBuild="174">'
    print >>dist, '    <title>Panda3D</title>'
    print >>dist, '    <options customize="always" allow-external-scripts="no" rootVolumeOnly="false"/>'
    # The following script is to enable the "Uninstall 1.6.x" option only when Panda3D 1.6.x is actually installed.
    print >>dist, '''    <script>
function have16installed() {
  return system.files.fileExistsAtPath(my.target.mountpoint + '/Applications/Panda3D');
}</script>'''
    print >>dist, '    <license language="en" mime-type="text/plain">%s</license>' % ReadFile("doc/LICENSE")
    print >>dist, '    <choices-outline>'
    print >>dist, '        <line choice="uninstall16"/>'
    print >>dist, '        <line choice="base"/>'
    print >>dist, '        <line choice="tools"/>'
    if PkgSkip("PYTHON")==0:
        print >>dist, '        <line choice="pythoncode"/>'
    if os.path.isdir("samples"):
        print >>dist, '        <line choice="samples"/>'
    print >>dist, '        <line choice="headers"/>'
    print >>dist, '    </choices-outline>'
    print >>dist, '    <choice id="uninstall16" title="Uninstall Panda3D 1.6.x" tooltip="Uninstalls Panda3D 1.6.x before installing Panda3D %s" description="If this option is checked, Panda3D 1.6.x is removed from /Applications/Panda3D/ before the new version is installed. This is recommended to avoid library conflicts. WARNING: EVERYTHING UNDER /Applications/Panda3D WILL BE DELETED. MAKE SURE YOU HAVE BACKED UP IMPORTANT DATA!" selected="have16installed()" enabled="have16installed()" visible="have16installed()">' % VERSION
    print >>dist, '        <pkg-ref id="org.panda3d.panda3d.uninstall16.pkg"/>'
    print >>dist, '    </choice>'
    print >>dist, '    <choice id="base" title="Panda3D Base Installation" description="This package contains the Panda3D libraries, configuration files and models/textures that are needed to use Panda3D. Location: /Developer/Panda3D/" start_enabled="false">'
    print >>dist, '        <pkg-ref id="org.panda3d.panda3d.base.pkg"/>'
    print >>dist, '    </choice>'
    print >>dist, '    <choice id="tools" title="Tools" tooltip="Useful tools and model converters to help with Panda3D development" description="This package contains the various utilities that ship with Panda3D, including packaging tools, model converters, and many more. Location: /Developer/Tools/Panda3D/">'
    print >>dist, '        <pkg-ref id="org.panda3d.panda3d.tools.pkg"/>'
    print >>dist, '    </choice>'
    if PkgSkip("PYTHON")==0:
        print >>dist, '    <choice id="pythoncode" title="Python Code" tooltip="Code you\'ll need for Python development" description="This package contains the \'direct\', \'pandac\' and \'panda3d\' python packages that are needed to do Python development with Panda3D. Location: /Developer/Panda3D/">'
        print >>dist, '        <pkg-ref id="org.panda3d.panda3d.pythoncode.pkg"/>'
        print >>dist, '    </choice>'
    if os.path.isdir("samples"):
        print >>dist, '    <choice id="samples" title="Sample Programs" tooltip="Python sample programs that use Panda3D" description="This package contains the Python sample programs that can help you with learning how to use Panda3D. Location: /Developer/Examples/Panda3D/">'
        print >>dist, '        <pkg-ref id="org.panda3d.panda3d.samples.pkg"/>'
        print >>dist, '    </choice>'
    print >>dist, '    <choice id="headers" title="C++ Header Files" tooltip="Header files for C++ development with Panda3D" description="This package contains the C++ header files that are needed in order to do C++ development with Panda3D. You don\'t need this if you want to develop in Python. Location: /Developer/Panda3D/include/" start_selected="false">'
    print >>dist, '        <pkg-ref id="org.panda3d.panda3d.headers.pkg"/>'
    print >>dist, '    </choice>'
    print >>dist, '    <pkg-ref id="org.panda3d.panda3d.uninstall16.pkg" installKBytes="0" version="1" auth="Root">file:./Contents/Packages/uninstall16.pkg</pkg-ref>'
    print >>dist, '    <pkg-ref id="org.panda3d.panda3d.base.pkg" installKBytes="%d" version="1" auth="Root">file:./Contents/Packages/base.pkg</pkg-ref>' % (GetDirectorySize("dstroot/base") / 1024)
    print >>dist, '    <pkg-ref id="org.panda3d.panda3d.tools.pkg" installKBytes="%d" version="1" auth="Root">file:./Contents/Packages/tools.pkg</pkg-ref>' % (GetDirectorySize("dstroot/tools") / 1024)
    if PkgSkip("PYTHON")==0:
        print >>dist, '    <pkg-ref id="org.panda3d.panda3d.pythoncode.pkg" installKBytes="%d" version="1" auth="Root">file:./Contents/Packages/pythoncode.pkg</pkg-ref>' % (GetDirectorySize("dstroot/pythoncode") / 1024)
    if os.path.isdir("samples"):
        print >>dist, '    <pkg-ref id="org.panda3d.panda3d.samples.pkg" installKBytes="%d" version="1" auth="Root">file:./Contents/Packages/samples.pkg</pkg-ref>' % (GetDirectorySize("dstroot/samples") / 1024)
    print >>dist, '    <pkg-ref id="org.panda3d.panda3d.headers.pkg" installKBytes="%d" version="1" auth="Root">file:./Contents/Packages/headers.pkg</pkg-ref>' % (GetDirectorySize("dstroot/headers") / 1024)
    print >>dist, '</installer-script>'
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
        cmd += " -c -\"The Panda3D free 3D engine SDK\" -o devel/panda3d
    cmd += " -d pkg-descr -f pkg-plist panda3d-%s" % VERSION
    oscmd(cmd)

if (INSTALLER != 0):
    ProgressOutput(100.0, "Building installer")
    if (sys.platform.startswith("win")):
        dbg = ""
        if (GetOptimize() <= 2): dbg = "-dbg"
        if (platform.architecture()[0] == "64bit"):
            if (RUNTIME):
                MakeInstallerNSIS("Panda3D-Runtime-"+VERSION+dbg+"-x64.exe", "Panda3D", "Panda3D "+VERSION, "C:\\Panda3D-"+VERSION)
            else:
                MakeInstallerNSIS("Panda3D-"+VERSION+dbg+"-x64.exe", "Panda3D", "Panda3D "+VERSION, "C:\\Panda3D-"+VERSION)
        else:
            if (RUNTIME):
                MakeInstallerNSIS("Panda3D-Runtime-"+VERSION+dbg+".exe", "Panda3D", "Panda3D "+VERSION, "C:\\Panda3D-"+VERSION)
            else:
                MakeInstallerNSIS("Panda3D-"+VERSION+dbg+".exe", "Panda3D", "Panda3D "+VERSION, "C:\\Panda3D-"+VERSION)
    elif (sys.platform == "linux2"):
        MakeInstallerLinux()
    elif (sys.platform == "darwin"):
        MakeInstallerOSX()
    elif (sys.platform.startswith("freebsd")):
        MakeInstallerFreeBSD()
    else:
        exit("Do not know how to make an installer for this platform")

##########################################################################################
#
# Print final status report.
#
##########################################################################################

SaveDependencyCache()
MoveBackConflictingFiles()

WARNINGS.append("Elapsed Time: "+PrettyTime(time.time() - STARTTIME))

printStatus("Makepanda Final Status Report", WARNINGS)
print GetColor("green") + "Build successfully finished, elapsed time: " + PrettyTime(time.time() - STARTTIME) + GetColor()

