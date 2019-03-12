#!/usr/bin/env python
########################################################################
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
except KeyboardInterrupt:
    raise
except:
    print("You are either using an incomplete or an old version of Python!")
    print("Please install the development package of Python and try again.")
    exit(1)

from makepandacore import *
from distutils.util import get_platform
import time
import os
import sys

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
WHEEL=0
RUNTESTS=0
GENMAN=0
COMPRESSOR="zlib"
THREADCOUNT=0
CFLAGS=""
CXXFLAGS=""
LDFLAGS=""
RTDIST=0
RTDIST_VERSION=None
RUNTIME=0
DISTRIBUTOR=""
VERSION=None
DEBVERSION=None
WHLVERSION=None
RPMRELEASE="1"
GIT_COMMIT=None
P3DSUFFIX=None
MAJOR_VERSION=None
COREAPI_VERSION=None
PLUGIN_VERSION=None
OSXTARGET=None
HOST_URL=None
global STRDXSDKVERSION, BOOUSEINTELCOMPILER
STRDXSDKVERSION = 'default'
WINDOWS_SDK = None
MSVC_VERSION = None
BOOUSEINTELCOMPILER = False
OPENCV_VER_23 = False
PLATFORM = None
COPY_PYTHON = True

if "MACOSX_DEPLOYMENT_TARGET" in os.environ:
    OSXTARGET=os.environ["MACOSX_DEPLOYMENT_TARGET"]

PkgListSet(["PYTHON", "DIRECT",                        # Python support
  "GL", "GLES", "GLES2"] + DXVERSIONS + ["TINYDISPLAY", "NVIDIACG", # 3D graphics
  "EGL",                                               # OpenGL (ES) integration
  "EIGEN",                                             # Linear algebra acceleration
  "OPENAL", "FMODEX",                                  # Audio playback
  "VORBIS", "OPUS", "FFMPEG", "SWSCALE", "SWRESAMPLE", # Audio decoding
  "ODE", "PHYSX", "BULLET", "PANDAPHYSICS",            # Physics
  "SPEEDTREE",                                         # SpeedTree
  "ZLIB", "PNG", "JPEG", "TIFF", "OPENEXR", "SQUISH",  # 2D Formats support
  ] + MAYAVERSIONS + MAXVERSIONS + [ "FCOLLADA", "ASSIMP", "EGG", # 3D Formats support
  "FREETYPE", "HARFBUZZ",                              # Text rendering
  "VRPN", "OPENSSL",                                   # Transport
  "FFTW",                                              # Algorithm helpers
  "ARTOOLKIT", "OPENCV", "DIRECTCAM", "VISION",        # Augmented Reality
  "GTK2",                                              # GTK2 is used for PStats on Unix
  "MFC", "WX", "FLTK",                                 # Used for web plug-in only
  "ROCKET",                                            # GUI libraries
  "COCOA",                                             # Mac OS X toolkits
  "X11",                                               # Unix platform support
  "PANDATOOL", "PVIEW", "DEPLOYTOOLS",                 # Toolchain
  "SKEL",                                              # Example SKEL project
  "PANDAFX",                                           # Some distortion special lenses
  "PANDAPARTICLESYSTEM",                               # Built in particle system
  "CONTRIB",                                           # Experimental
  "SSE2", "NEON",                                      # Compiler features
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
        print("Error parsing command-line input: %s" % (problem))

    print("")
    print("Makepanda generates a 'built' subdirectory containing a")
    print("compiled copy of Panda3D.  Command-line arguments are:")
    print("")
    print("  --help            (print the help message you're reading now)")
    print("  --verbose         (print out more information)")
    print("  --runtime         (build a runtime build instead of an SDK build)")
    print("  --tests           (run the test suite)")
    print("  --installer       (build an installer)")
    print("  --wheel           (build a pip-installable .whl)")
    print("  --optimize X      (optimization level can be 1,2,3,4)")
    print("  --version X       (set the panda version number)")
    print("  --lzma            (use lzma compression when building Windows installer)")
    print("  --distributor X   (short string identifying the distributor of the build)")
    print("  --outputdir X     (use the specified directory instead of 'built')")
    print("  --host URL        (set the host url (runtime build only))")
    print("  --threads N       (use the multithreaded build system. see manual)")
    print("  --osxtarget N     (the OS X version number to build for (OS X only))")
    print("  --override \"O=V\"  (override dtool_config/prc option value)")
    print("  --static          (builds libraries for static linking)")
    print("  --target X        (experimental cross-compilation (android only))")
    print("  --arch X          (target architecture for cross-compilation)")
    print("")
    for pkg in PkgListGet():
        p = pkg.lower()
        print("  --use-%-9s   --no-%-9s (enable/disable use of %s)"%(p, p, pkg))
    if sys.platform != 'win32':
        print("  --<PKG>-incdir    (custom location for header files of thirdparty package)")
        print("  --<PKG>-libdir    (custom location for library files of thirdparty package)")
    print("")
    print("  --nothing         (disable every third-party lib)")
    print("  --everything      (enable every third-party lib)")
    print("  --directx-sdk=X   (specify version of DirectX SDK to use: jun2010, aug2009, mar2009, aug2006)")
    print("  --windows-sdk=X   (specify Windows SDK version, eg. 7.0, 7.1 or 10.  Default is 7.1)")
    print("  --msvc-version=X  (specify Visual C++ version, eg. 10, 11, 12, 14, 14.1, 14.2.  Default is 14)")
    print("  --use-icl         (experimental setting to use an intel compiler instead of MSVC on Windows)")
    print("")
    print("The simplest way to compile panda is to just type:")
    print("")
    print("  makepanda --everything")
    print("")
    os._exit(1)

def parseopts(args):
    global INSTALLER,WHEEL,RUNTESTS,RTDIST,RUNTIME,GENMAN,DISTRIBUTOR,VERSION
    global COMPRESSOR,THREADCOUNT,OSXTARGET,HOST_URL
    global DEBVERSION,WHLVERSION,RPMRELEASE,GIT_COMMIT,P3DSUFFIX,RTDIST_VERSION
    global STRDXSDKVERSION, WINDOWS_SDK, MSVC_VERSION, BOOUSEINTELCOMPILER
    global COPY_PYTHON

    # Options for which to display a deprecation warning.
    removedopts = [
        "use-touchinput", "no-touchinput", "no-awesomium", "no-directscripts",
        "no-carbon", "universal",
        ]

    # All recognized options.
    longopts = [
        "help","distributor=","verbose","runtime","osxtarget=","tests",
        "optimize=","everything","nothing","installer","wheel","rtdist","nocolor",
        "version=","lzma","no-python","threads=","outputdir=","override=",
        "static","host=","debversion=","rpmrelease=","p3dsuffix=","rtdist-version=",
        "directx-sdk=", "windows-sdk=", "msvc-version=", "clean", "use-icl",
        "target=", "arch=", "git-commit=", "no-copy-python",
        ] + removedopts

    anything = 0
    optimize = ""
    target = None
    target_arch = None
    clean_build = False
    for pkg in PkgListGet():
        longopts.append("use-" + pkg.lower())
        longopts.append("no-" + pkg.lower())
        longopts.append(pkg.lower() + "-incdir=")
        longopts.append(pkg.lower() + "-libdir=")

    try:
        opts, extras = getopt.getopt(args, "", longopts)
        for option, value in opts:
            if (option=="--help"): raise Exception
            elif (option=="--optimize"): optimize=value
            elif (option=="--installer"): INSTALLER=1
            elif (option=="--tests"): RUNTESTS=1
            elif (option=="--wheel"): WHEEL=1
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
            elif (option=="--target"): target = value.strip()
            elif (option=="--arch"): target_arch = value.strip()
            elif (option=="--nocolor"): DisableColors()
            elif (option=="--version"):
                match = re.match(r'^\d+\.\d+\.\d+', value)
                if not match:
                    usage("version requires three digits")
                WHLVERSION = value
                VERSION = match.group()
            elif (option=="--lzma"): COMPRESSOR="lzma"
            elif (option=="--override"): AddOverride(value.strip())
            elif (option=="--static"): SetLinkAllStatic(True)
            elif (option=="--host"): HOST_URL=value
            elif (option=="--debversion"): DEBVERSION=value
            elif (option=="--rpmrelease"): RPMRELEASE=value
            elif (option=="--git-commit"): GIT_COMMIT=value
            elif (option=="--p3dsuffix"): P3DSUFFIX=value
            elif (option=="--rtdist-version"): RTDIST_VERSION=value
            # Backward compatibility, OPENGL was renamed to GL
            elif (option=="--use-opengl"): PkgEnable("GL")
            elif (option=="--no-opengl"): PkgDisable("GL")
            elif (option=="--directx-sdk"):
                STRDXSDKVERSION = value.strip().lower()
                if STRDXSDKVERSION == '':
                    print("No DirectX SDK version specified. Using 'default' DirectX SDK search")
                    STRDXSDKVERSION = 'default'
            elif (option=="--windows-sdk"):
                WINDOWS_SDK = value.strip().lower()
            elif (option=="--msvc-version"):
                MSVC_VERSION = value.strip().lower()
            elif (option=="--use-icl"): BOOUSEINTELCOMPILER = True
            elif (option=="--clean"): clean_build = True
            elif (option=="--no-copy-python"): COPY_PYTHON = False
            elif (option[2:] in removedopts):
                Warn("Ignoring removed option %s" % (option))
            else:
                for pkg in PkgListGet():
                    if option == "--use-" + pkg.lower():
                        PkgEnable(pkg)
                        break
                    elif option == "--no-" + pkg.lower():
                        PkgDisable(pkg)
                        break
                    elif option == "--" + pkg.lower() + "-incdir":
                        PkgSetCustomLocation(pkg)
                        IncDirectory(pkg, value)
                        break
                    elif option == "--" + pkg.lower() + "-libdir":
                        PkgSetCustomLocation(pkg)
                        LibDirectory(pkg, value)
                        break
            if (option == "--everything" or option.startswith("--use-")
                or option == "--nothing" or option.startswith("--no-")):
                anything = 1
    except:
        usage(sys.exc_info()[1])

    if not anything:
        if RUNTIME:
            PkgEnableAll()
        else:
            usage("You should specify a list of packages to use or --everything to enable all packages.")

    if (RTDIST and RUNTIME):
        usage("Options --runtime and --rtdist cannot be specified at the same time!")
    if (optimize=="" and (RTDIST or RUNTIME)): optimize = "4"
    elif (optimize==""): optimize = "3"

    if OSXTARGET:
        try:
            maj, min = OSXTARGET.strip().split('.')
            OSXTARGET = int(maj), int(min)
            assert OSXTARGET[0] == 10
        except:
            usage("Invalid setting for OSXTARGET")
    else:
        OSXTARGET = None

    if target is not None or target_arch is not None:
        SetTarget(target, target_arch)

    try:
        SetOptimize(int(optimize))
        assert GetOptimize() in [1, 2, 3, 4]
    except:
        usage("Invalid setting for OPTIMIZE")

    if GIT_COMMIT is not None and not re.match("^[a-f0-9]{40}$", GIT_COMMIT):
        usage("Invalid SHA-1 hash given for --git-commit option!")

    if GetTarget() == 'windows':
        if not MSVC_VERSION:
            print("No MSVC version specified. Defaulting to 14 (Visual Studio 2015).")
            MSVC_VERSION = (14, 0)
        else:
            try:
                MSVC_VERSION = tuple(int(d) for d in MSVC_VERSION.split('.'))[:2]
                if (len(MSVC_VERSION) == 1):
                    MSVC_VERSION += (0,)
            except:
                usage("Invalid setting for --msvc-version")

        if MSVC_VERSION < (14, 0):
            warn_prefix = "%sERROR:%s " % (GetColor("red"), GetColor())
            print("=========================================================================")
            print(warn_prefix + "Support for MSVC versions before 2015 has been discontinued.")
            print(warn_prefix + "For more information, or any questions, please visit:")
            print(warn_prefix + "  https://github.com/panda3d/panda3d/issues/288")
            print("=========================================================================")
            sys.stdout.flush()
            time.sleep(1.0)
            sys.exit(1)

        if not WINDOWS_SDK:
            print("No Windows SDK version specified. Defaulting to '7.1'.")
            WINDOWS_SDK = '7.1'

    if clean_build and os.path.isdir(GetOutputDir()):
        print("Deleting %s" % (GetOutputDir()))
        shutil.rmtree(GetOutputDir())

parseopts(sys.argv[1:])

########################################################################
##
## Handle environment variables.
##
########################################################################

if ("CFLAGS" in os.environ):
    CFLAGS = os.environ["CFLAGS"].strip()

if ("CXXFLAGS" in os.environ):
    CXXFLAGS = os.environ["CXXFLAGS"].strip()

if ("RPM_OPT_FLAGS" in os.environ):
    CFLAGS += " " + os.environ["RPM_OPT_FLAGS"].strip()
    CXXFLAGS += " " + os.environ["RPM_OPT_FLAGS"].strip()

if ("LDFLAGS" in os.environ):
    LDFLAGS = os.environ["LDFLAGS"].strip()

os.environ["MAKEPANDA"] = os.path.abspath(sys.argv[0])
if GetHost() == "darwin" and OSXTARGET is not None:
    os.environ["MACOSX_DEPLOYMENT_TARGET"] = "%d.%d" % OSXTARGET

########################################################################
##
## Configure things based on the command-line parameters.
##
########################################################################

PLUGIN_VERSION = ParsePluginVersion("dtool/PandaVersion.pp")
COREAPI_VERSION = PLUGIN_VERSION + "." + ParseCoreapiVersion("dtool/PandaVersion.pp")

if VERSION is None:
    if RUNTIME:
        VERSION = PLUGIN_VERSION
    else:
        # Take the value from the setup.cfg file.
        VERSION = GetMetadataValue('version')

if WHLVERSION is None:
    WHLVERSION = VERSION

print("Version: %s" % VERSION)
if RUNTIME or RTDIST:
    print("Core API Version: %s" % COREAPI_VERSION)

if DEBVERSION is None:
    DEBVERSION = VERSION

MAJOR_VERSION = '.'.join(VERSION.split('.')[:2])

if P3DSUFFIX is None:
    P3DSUFFIX = MAJOR_VERSION

# Now determine the distutils-style platform tag for the target system.
target = GetTarget()
if target == 'windows':
    if GetTargetArch() == 'x64':
        PLATFORM = 'win-amd64'
    else:
        PLATFORM = 'win32'

elif target == 'darwin':
    if OSXTARGET:
        osxver = OSXTARGET
    else:
        maj, min = platform.mac_ver()[0].split('.')[:2]
        osxver = int(maj), int(min)

    arch_tag = GetTargetArch()
    PLATFORM = 'macosx-{0}.{1}-{2}'.format(osxver[0], osxver[1], arch_tag)

elif target == 'linux' and (os.path.isfile("/lib/libc-2.5.so") or os.path.isfile("/lib64/libc-2.5.so")) and os.path.isdir("/opt/python"):
    # This is manylinux1.  A bit of a sloppy check, though.
    if GetTargetArch() in ('x86_64', 'amd64'):
        PLATFORM = 'manylinux1-x86_64'
    else:
        PLATFORM = 'manylinux1-i686'

elif not CrossCompiling():
    if HasTargetArch():
        # Replace the architecture in the platform string.
        platform_parts = get_platform().rsplit('-', 1)
        target_arch = GetTargetArch()
        if target_arch == 'amd64':
            target_arch = 'x86_64'
        PLATFORM = platform_parts[0] + '-' + target_arch
    else:
        # We're not cross-compiling; just take the host arch.
        PLATFORM = get_platform()

else:
    target_arch = GetTargetArch()
    if target_arch == 'amd64':
        target_arch = 'x86_64'
    PLATFORM = '{0}-{1}'.format(target, target_arch)


print("Platform: %s" % PLATFORM)

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

if DISTRIBUTOR == "":
    DISTRIBUTOR = "makepanda"
elif not RTDIST_VERSION:
    RTDIST_VERSION = DISTRIBUTOR.strip() + "_" + MAJOR_VERSION

if not RTDIST_VERSION:
    RTDIST_VERSION = "dev"

if not IsCustomOutputDir():
    if GetTarget() == "windows" and GetTargetArch() == 'x64':
        outputdir_suffix += '_x64'

    SetOutputDir("built" + outputdir_suffix)

if (RUNTIME):
    for pkg in PkgListGet():
        if pkg in ["GTK2", "MFC"]:
            # Optional package(s) for runtime.
            pass
        elif pkg in ["OPENSSL", "ZLIB"]:
            # Required packages for runtime.
            if (PkgSkip(pkg)==1):
                exit("Runtime must be compiled with OpenSSL and ZLib support!")
        else:
            # Unused packages for runtime.
            PkgDisable(pkg)

if (INSTALLER and RTDIST):
    exit("Cannot build an installer for the rtdist build!")

if (WHEEL and RUNTIME):
    exit("Cannot build a wheel for the runtime build!")

if (WHEEL and RTDIST):
    exit("Cannot build a wheel for the rtdist build!")

if (INSTALLER) and (PkgSkip("PYTHON")) and (not RUNTIME) and GetTarget() == 'windows':
    exit("Cannot build installer on Windows without python")

if WHEEL and PkgSkip("PYTHON"):
    exit("Cannot build wheel without Python")

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
SdkLocateWindows(WINDOWS_SDK)
SdkLocatePhysX()
SdkLocateSpeedTree()
SdkLocateAndroid()

SdkAutoDisableDirectX()
SdkAutoDisableMaya()
SdkAutoDisableMax()
SdkAutoDisablePhysX()
SdkAutoDisableSpeedTree()

if RTDIST and DISTRIBUTOR == "cmu":
    # Some validation checks for the CMU builds.
    if (RTDIST_VERSION == "cmu_1.7" and SDK["PYTHONVERSION"] != "python2.6"):
        exit("The CMU 1.7 runtime distribution must be built against Python 2.6!")
    elif (RTDIST_VERSION == "cmu_1.8" and SDK["PYTHONVERSION"] != "python2.7"):
        exit("The CMU 1.8 runtime distribution must be built against Python 2.7!")
    elif (RTDIST_VERSION == "cmu_1.9" and SDK["PYTHONVERSION"] != "python2.7"):
        exit("The CMU 1.9 runtime distribution must be built against Python 2.7!")

if RTDIST and not HOST_URL:
    exit("You must specify a host URL when building the rtdist!")

if RUNTIME and not HOST_URL:
    # Set this to a nice default.
    HOST_URL = "https://runtime.panda3d.org/"

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
    SdkLocateVisualStudio(MSVC_VERSION)
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
    PkgDisable("GLES")
    PkgDisable("GLES2")
    PkgDisable("EGL")
    PkgDisable("COCOA")
    DefSymbol("FLEX", "YY_NO_UNISTD_H")
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
            if int(vnum) >= 9 and "GENERIC_DXERR_LIBRARY" in SDK:
                LibName(pkg, 'dxerr.lib')
            else:
                LibName(pkg, 'dxerrVNUM.lib'.replace("VNUM", vnum))
            #LibName(pkg, 'ddraw.lib')
            LibName(pkg, 'dxguid.lib')

            if SDK.get("VISUALSTUDIO_VERSION") >= (14,0):
                # dxerr needs this for __vsnwprintf definition.
                LibName(pkg, 'legacy_stdio_definitions.lib')

    if not PkgSkip("FREETYPE") and os.path.isdir(GetThirdpartyDir() + "freetype/include/freetype2"):
        IncDirectory("FREETYPE", GetThirdpartyDir() + "freetype/include/freetype2")

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
    LibName("SETUPAPI", "setupapi.lib")
    LibName("GL", "opengl32.lib")
    LibName("GLES", "libgles_cm.lib")
    LibName("GLES2", "libGLESv2.lib")
    LibName("EGL", "libEGL.lib")
    LibName("MSIMG", "msimg32.lib")
    if (PkgSkip("DIRECTCAM")==0): LibName("DIRECTCAM", "strmiids.lib")
    if (PkgSkip("DIRECTCAM")==0): LibName("DIRECTCAM", "quartz.lib")
    if (PkgSkip("DIRECTCAM")==0): LibName("DIRECTCAM", "odbc32.lib")
    if (PkgSkip("DIRECTCAM")==0): LibName("DIRECTCAM", "odbccp32.lib")
    if (PkgSkip("OPENSSL")==0):
        if os.path.isfile(GetThirdpartyDir() + "openssl/lib/libpandassl.lib"):
            LibName("OPENSSL", GetThirdpartyDir() + "openssl/lib/libpandassl.lib")
            LibName("OPENSSL", GetThirdpartyDir() + "openssl/lib/libpandaeay.lib")
        else:
            LibName("OPENSSL", GetThirdpartyDir() + "openssl/lib/libeay32.lib")
            LibName("OPENSSL", GetThirdpartyDir() + "openssl/lib/ssleay32.lib")
    if (PkgSkip("PNG")==0):
        if os.path.isfile(GetThirdpartyDir() + "png/lib/libpng16_static.lib"):
            LibName("PNG", GetThirdpartyDir() + "png/lib/libpng16_static.lib")
        else:
            LibName("PNG", GetThirdpartyDir() + "png/lib/libpng_static.lib")
    if (PkgSkip("TIFF")==0):
        if os.path.isfile(GetThirdpartyDir() + "tiff/lib/libtiff.lib"):
            LibName("TIFF", GetThirdpartyDir() + "tiff/lib/libtiff.lib")
        else:
            LibName("TIFF", GetThirdpartyDir() + "tiff/lib/tiff.lib")
    if (PkgSkip("OPENEXR")==0):
        suffix = ""
        if os.path.isfile(GetThirdpartyDir() + "openexr/lib/IlmImf-2_2.lib"):
            suffix = "-2_2"
        elif os.path.isfile(GetThirdpartyDir() + "openexr/lib/IlmImf-2_3.lib"):
            suffix = "-2_3"
        if os.path.isfile(GetThirdpartyDir() + "openexr/lib/IlmImf" + suffix + "_s.lib"):
            suffix += "_s"
        LibName("OPENEXR", GetThirdpartyDir() + "openexr/lib/IlmImf" + suffix + ".lib")
        LibName("OPENEXR", GetThirdpartyDir() + "openexr/lib/IlmThread" + suffix + ".lib")
        LibName("OPENEXR", GetThirdpartyDir() + "openexr/lib/Iex" + suffix + ".lib")
        if suffix == "-2_2":
            LibName("OPENEXR", GetThirdpartyDir() + "openexr/lib/Half.lib")
        else:
            LibName("OPENEXR", GetThirdpartyDir() + "openexr/lib/Half" + suffix + ".lib")
        IncDirectory("OPENEXR", GetThirdpartyDir() + "openexr/include/OpenEXR")
    if (PkgSkip("JPEG")==0):     LibName("JPEG",     GetThirdpartyDir() + "jpeg/lib/jpeg-static.lib")
    if (PkgSkip("ZLIB")==0):     LibName("ZLIB",     GetThirdpartyDir() + "zlib/lib/zlibstatic.lib")
    if (PkgSkip("VRPN")==0):     LibName("VRPN",     GetThirdpartyDir() + "vrpn/lib/vrpn.lib")
    if (PkgSkip("VRPN")==0):     LibName("VRPN",     GetThirdpartyDir() + "vrpn/lib/quat.lib")
    if (PkgSkip("NVIDIACG")==0): LibName("CGGL",     GetThirdpartyDir() + "nvidiacg/lib/cgGL.lib")
    if (PkgSkip("NVIDIACG")==0): LibName("CGDX9",    GetThirdpartyDir() + "nvidiacg/lib/cgD3D9.lib")
    if (PkgSkip("NVIDIACG")==0): LibName("NVIDIACG", GetThirdpartyDir() + "nvidiacg/lib/cg.lib")
    if (PkgSkip("FREETYPE")==0): LibName("FREETYPE", GetThirdpartyDir() + "freetype/lib/freetype.lib")
    if (PkgSkip("HARFBUZZ")==0):
        LibName("HARFBUZZ", GetThirdpartyDir() + "harfbuzz/lib/harfbuzz.lib")
        IncDirectory("HARFBUZZ", GetThirdpartyDir() + "harfbuzz/include/harfbuzz")
    if (PkgSkip("FFTW")==0):     LibName("FFTW",     GetThirdpartyDir() + "fftw/lib/fftw3.lib")
    if (PkgSkip("ARTOOLKIT")==0):LibName("ARTOOLKIT",GetThirdpartyDir() + "artoolkit/lib/libAR.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/cv.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/highgui.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/cvaux.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/ml.lib")
    if (PkgSkip("OPENCV")==0):   LibName("OPENCV",   GetThirdpartyDir() + "opencv/lib/cxcore.lib")
    if (PkgSkip("FFMPEG")==0):   LibName("FFMPEG",   GetThirdpartyDir() + "ffmpeg/lib/avcodec.lib")
    if (PkgSkip("FFMPEG")==0):   LibName("FFMPEG",   GetThirdpartyDir() + "ffmpeg/lib/avformat.lib")
    if (PkgSkip("FFMPEG")==0):   LibName("FFMPEG",   GetThirdpartyDir() + "ffmpeg/lib/avutil.lib")
    if (PkgSkip("SWSCALE")==0):  LibName("SWSCALE",  GetThirdpartyDir() + "ffmpeg/lib/swscale.lib")
    if (PkgSkip("SWRESAMPLE")==0):LibName("SWRESAMPLE",GetThirdpartyDir() + "ffmpeg/lib/swresample.lib")
    if (PkgSkip("FCOLLADA")==0):
        LibName("FCOLLADA", GetThirdpartyDir() + "fcollada/lib/FCollada.lib")
        IncDirectory("FCOLLADA", GetThirdpartyDir() + "fcollada/include/FCollada")
    if (PkgSkip("ASSIMP")==0):
        LibName("ASSIMP", GetThirdpartyDir() + "assimp/lib/assimp.lib")
        if os.path.isfile(GetThirdpartyDir() + "assimp/lib/IrrXML.lib"):
            LibName("ASSIMP", GetThirdpartyDir() + "assimp/lib/IrrXML.lib")
        IncDirectory("ASSIMP", GetThirdpartyDir() + "assimp/include")
    if (PkgSkip("SQUISH")==0):
        if GetOptimize() <= 2:
            LibName("SQUISH",   GetThirdpartyDir() + "squish/lib/squishd.lib")
        else:
            LibName("SQUISH",   GetThirdpartyDir() + "squish/lib/squish.lib")
    if (PkgSkip("ROCKET")==0):
        LibName("ROCKET", GetThirdpartyDir() + "rocket/lib/RocketCore.lib")
        LibName("ROCKET", GetThirdpartyDir() + "rocket/lib/RocketControls.lib")
        if (PkgSkip("PYTHON")==0):
            LibName("ROCKET", GetThirdpartyDir() + "rocket/lib/" + SDK["PYTHONVERSION"] + "/boost_python-vc100-mt-1_54.lib")
        if (GetOptimize() <= 3):
            LibName("ROCKET", GetThirdpartyDir() + "rocket/lib/RocketDebugger.lib")
    if (PkgSkip("OPENAL")==0):
        LibName("OPENAL", GetThirdpartyDir() + "openal/lib/OpenAL32.lib")
        if not os.path.isfile(GetThirdpartyDir() + "openal/bin/OpenAL32.dll"):
            # Link OpenAL Soft statically.
            DefSymbol("OPENAL", "AL_LIBTYPE_STATIC")
    if (PkgSkip("ODE")==0):
        LibName("ODE",      GetThirdpartyDir() + "ode/lib/ode_single.lib")
        DefSymbol("ODE",    "dSINGLE", "")
    if (PkgSkip("FMODEX")==0):
        if (GetTargetArch() == 'x64'):
            LibName("FMODEX",   GetThirdpartyDir() + "fmodex/lib/fmodex64_vc.lib")
        else:
            LibName("FMODEX",   GetThirdpartyDir() + "fmodex/lib/fmodex_vc.lib")
    if (PkgSkip("FLTK")==0 and RTDIST):
        LibName("FLTK", GetThirdpartyDir() + "fltk/lib/fltk.lib")
        if not PkgSkip("FLTK"):
            # If we have fltk, we don't need wx
            PkgDisable("WX")
    if (PkgSkip("WX")==0 and RTDIST):
        LibName("WX",       GetThirdpartyDir() + "wx/lib/wxbase28u.lib")
        LibName("WX",       GetThirdpartyDir() + "wx/lib/wxmsw28u_core.lib")
        DefSymbol("WX",     "__WXMSW__", "")
        DefSymbol("WX",     "_UNICODE", "")
        DefSymbol("WX",     "UNICODE", "")
    if (PkgSkip("VORBIS")==0):
        for lib in ('ogg', 'vorbis', 'vorbisfile'):
            path = GetThirdpartyDir() + "vorbis/lib/lib{0}_static.lib".format(lib)
            if not os.path.isfile(path):
                path = GetThirdpartyDir() + "vorbis/lib/{0}.lib".format(lib)
            LibName("VORBIS", path)
    if (PkgSkip("OPUS")==0):
        IncDirectory("OPUS", GetThirdpartyDir() + "opus/include/opus")
        for lib in ('ogg', 'opus', 'opusfile'):
            path = GetThirdpartyDir() + "opus/lib/lib{0}_static.lib".format(lib)
            if not os.path.isfile(path):
                path = GetThirdpartyDir() + "opus/lib/{0}.lib".format(lib)
            LibName("OPUS", path)
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
        if GetTargetArch() == 'x64' and os.path.isfile(GetThirdpartyDir() + "bullet/lib/BulletCollision_x64.lib"):
            suffix = '_x64.lib'
        LibName("BULLET", GetThirdpartyDir() + "bullet/lib/LinearMath" + suffix)
        LibName("BULLET", GetThirdpartyDir() + "bullet/lib/BulletCollision" + suffix)
        LibName("BULLET", GetThirdpartyDir() + "bullet/lib/BulletDynamics" + suffix)
        LibName("BULLET", GetThirdpartyDir() + "bullet/lib/BulletSoftBody" + suffix)

if (COMPILER=="GCC"):
    if GetTarget() != "darwin":
        PkgDisable("COCOA")
    elif RUNTIME:
        # We don't support Cocoa in the runtime yet.
        PkgDisable("COCOA")

    #if (PkgSkip("PYTHON")==0):
    #    IncDirectory("PYTHON", SDK["PYTHON"])
    if (GetHost() == "darwin"):
        if (PkgSkip("FREETYPE")==0 and not os.path.isdir(GetThirdpartyDir() + 'freetype')):
          IncDirectory("FREETYPE", "/usr/X11/include")
          IncDirectory("FREETYPE", "/usr/X11/include/freetype2")
          LibDirectory("FREETYPE", "/usr/X11/lib")

    if (GetHost() == "freebsd"):
        IncDirectory("ALWAYS", "/usr/local/include")
        LibDirectory("ALWAYS", "/usr/local/lib")
        if (os.path.isdir("/usr/PCBSD")):
            IncDirectory("ALWAYS", "/usr/PCBSD/local/include")
            LibDirectory("ALWAYS", "/usr/PCBSD/local/lib")

    if GetTarget() != "windows":
        PkgDisable("DIRECTCAM")

    fcollada_libs = ("FColladaD", "FColladaSD", "FColladaS")
    # WARNING! The order of the ffmpeg libraries matters!
    ffmpeg_libs = ("libavformat", "libavcodec", "libavutil")
    assimp_libs = ("libassimp", "libassimpd")

    #         Name         pkg-config   libs, include(dir)s
    if (not RUNTIME):
        SmartPkgEnable("EIGEN",     "eigen3",    (), ("Eigen/Dense",), target_pkg = 'ALWAYS')
        SmartPkgEnable("ARTOOLKIT", "",          ("AR"), "AR/ar.h")
        SmartPkgEnable("FCOLLADA",  "",          ChooseLib(fcollada_libs, "FCOLLADA"), ("FCollada", "FCollada/FCollada.h"))
        SmartPkgEnable("ASSIMP",    "assimp",    ChooseLib(assimp_libs, "ASSIMP"), "assimp/Importer.hpp")
        SmartPkgEnable("FFMPEG",    ffmpeg_libs, ffmpeg_libs, ("libavformat/avformat.h", "libavcodec/avcodec.h", "libavutil/avutil.h"))
        SmartPkgEnable("SWSCALE",   "libswscale", "libswscale", ("libswscale/swscale.h"), target_pkg = "FFMPEG", thirdparty_dir = "ffmpeg")
        SmartPkgEnable("SWRESAMPLE","libswresample", "libswresample", ("libswresample/swresample.h"), target_pkg = "FFMPEG", thirdparty_dir = "ffmpeg")
        SmartPkgEnable("FFTW",      "",          ("fftw3"), ("fftw.h"))
        SmartPkgEnable("FMODEX",    "",          ("fmodex"), ("fmodex", "fmodex/fmod.h"))
        SmartPkgEnable("FREETYPE",  "freetype2", ("freetype"), ("freetype2", "freetype2/freetype/freetype.h"))
        SmartPkgEnable("HARFBUZZ",  "harfbuzz",  ("harfbuzz"), ("harfbuzz", "harfbuzz/hb-ft.h"))
        SmartPkgEnable("GL",        "gl",        ("GL"), ("GL/gl.h"), framework = "OpenGL")
        SmartPkgEnable("GLES",      "glesv1_cm", ("GLESv1_CM"), ("GLES/gl.h"), framework = "OpenGLES")
        SmartPkgEnable("GLES2",     "glesv2",    ("GLESv2"), ("GLES2/gl2.h")) #framework = "OpenGLES"?
        SmartPkgEnable("EGL",       "egl",       ("EGL"), ("EGL/egl.h"))
        SmartPkgEnable("NVIDIACG",  "",          ("Cg"), "Cg/cg.h", framework = "Cg")
        SmartPkgEnable("ODE",       "",          ("ode"), "ode/ode.h", tool = "ode-config")
        SmartPkgEnable("OPENAL",    "openal",    ("openal"), "AL/al.h", framework = "OpenAL")
        SmartPkgEnable("SQUISH",    "",          ("squish"), "squish.h")
        SmartPkgEnable("TIFF",      "libtiff-4", ("tiff"), "tiff.h")
        SmartPkgEnable("OPENEXR",   "OpenEXR",   ("IlmImf", "Imath", "Half", "Iex", "IexMath", "IlmThread"), ("OpenEXR", "OpenEXR/ImfOutputFile.h"))
        SmartPkgEnable("VRPN",      "",          ("vrpn", "quat"), ("vrpn", "quat.h", "vrpn/vrpn_Types.h"))
        SmartPkgEnable("BULLET", "bullet", ("BulletSoftBody", "BulletDynamics", "BulletCollision", "LinearMath"), ("bullet", "bullet/btBulletDynamicsCommon.h"))
        SmartPkgEnable("VORBIS",    "vorbisfile",("vorbisfile", "vorbis", "ogg"), ("ogg/ogg.h", "vorbis/vorbisfile.h"))
        SmartPkgEnable("OPUS",      "opusfile",  ("opusfile", "opus", "ogg"), ("ogg/ogg.h", "opus/opusfile.h", "opus"))
        SmartPkgEnable("JPEG",      "",          ("jpeg"), "jpeglib.h")
        SmartPkgEnable("PNG",       "libpng",    ("png"), "png.h", tool = "libpng-config")

        if not PkgSkip("FFMPEG"):
            if GetTarget() == "darwin":
                LibName("FFMPEG", "-framework VideoDecodeAcceleration")
            elif os.path.isfile(GetThirdpartyDir() + "ffmpeg/lib/libavcodec.a"):
                # Needed when linking ffmpeg statically on Linux.
                LibName("FFMPEG", "-Wl,-Bsymbolic")

        if PkgSkip("FFMPEG") or GetTarget() == "darwin":
            cv_lib = ChooseLib(("opencv_core", "cv"), "OPENCV")
            if cv_lib == "opencv_core":
                OPENCV_VER_23 = True
                SmartPkgEnable("OPENCV", "opencv",   ("opencv_core", "opencv_highgui"), ("opencv2/core/core.hpp"))
            else:
                SmartPkgEnable("OPENCV", "opencv",   ("cv", "highgui", "cvaux", "ml", "cxcore"),
                               ("opencv", "opencv/cv.h", "opencv/cxcore.h", "opencv/highgui.h"))
        else:
            PkgDisable("OPENCV")

        if not PkgSkip("ASSIMP") and \
            os.path.isfile(GetThirdpartyDir() + "assimp/lib/libassimp.a"):
            # Also pick up IrrXML, which is needed when linking statically.
            irrxml = GetThirdpartyDir() + "assimp/lib/libIrrXML.a"
            if os.path.isfile(irrxml):
                LibName("ASSIMP", irrxml)

        rocket_libs = ("RocketCore", "RocketControls")
        if (GetOptimize() <= 3):
            rocket_libs += ("RocketDebugger",)
        rocket_libs += ("boost_python",)
        SmartPkgEnable("ROCKET",    "",          rocket_libs, "Rocket/Core.h")

        if not PkgSkip("PYTHON"):
            python_lib = SDK["PYTHONVERSION"]
            SmartPkgEnable("PYTHON", "", python_lib, (SDK["PYTHONVERSION"], SDK["PYTHONVERSION"] + "/Python.h"))

            if GetTarget() == "linux":
                LibName("PYTHON", "-lutil")
                LibName("PYTHON", "-lrt")

    SmartPkgEnable("OPENSSL",   "openssl",   ("ssl", "crypto"), ("openssl/ssl.h", "openssl/crypto.h"))
    SmartPkgEnable("ZLIB",      "zlib",      ("z"), "zlib.h")
    SmartPkgEnable("GTK2",      "gtk+-2.0")

    if (RTDIST):
        SmartPkgEnable("WX", tool = "wx-config")
        SmartPkgEnable("FLTK", "", ("fltk"), ("FL/Fl.H"), tool = "fltk-config")

    if GetTarget() != 'darwin':
        # CgGL is covered by the Cg framework, and we don't need X11 components on OSX
        if not PkgSkip("NVIDIACG") and not RUNTIME:
            SmartPkgEnable("CGGL", "", ("CgGL"), "Cg/cgGL.h", thirdparty_dir = "nvidiacg")
        if not RUNTIME:
            SmartPkgEnable("X11", "x11", "X11", ("X11", "X11/Xlib.h", "X11/XKBlib.h"))

    if GetHost() != "darwin":
        # Workaround for an issue where pkg-config does not include this path
        if GetTargetArch() in ("x86_64", "amd64"):
            if (os.path.isdir("/usr/lib64/glib-2.0/include")):
                IncDirectory("GTK2", "/usr/lib64/glib-2.0/include")
            if (os.path.isdir("/usr/lib64/gtk-2.0/include")):
                IncDirectory("GTK2", "/usr/lib64/gtk-2.0/include")

            if not PkgSkip("X11"):
                if (os.path.isdir("/usr/X11R6/lib64")):
                    LibDirectory("ALWAYS", "/usr/X11R6/lib64")
                else:
                    LibDirectory("ALWAYS", "/usr/X11R6/lib")
        elif not PkgSkip("X11"):
            LibDirectory("ALWAYS", "/usr/X11R6/lib")

    if RUNTIME:
        # For the runtime, these packages are required
        for pkg in ["OPENSSL", "ZLIB"]:
            skips = []
            if (pkg in PkgListGet() and PkgSkip(pkg)==1):
                skips.append(pkg)
            if skips:
                exit("Runtime must be compiled with OpenSSL and ZLib support (missing %s)" % (', '.join(skips)))

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
        LibName("AGL", "-framework AGL")
        LibName("CARBON", "-framework Carbon")
        LibName("COCOA", "-framework Cocoa")
        # Fix for a bug in OSX Leopard:
        LibName("GL", "-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")

    if GetTarget() == 'android':
        LibName("ALWAYS", '-llog')
        LibName("ANDROID", '-landroid')
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
    DefSymbol("ALWAYS", "LINK_ALL_STATIC")
if GetTarget() == 'android':
    DefSymbol("ALWAYS", "ANDROID")

if not PkgSkip("EIGEN"):
    if GetOptimize() >= 3:
        if COMPILER == "MSVC":
            # Squeeze out a bit more performance on MSVC builds...
            # Only do this if EIGEN_NO_DEBUG is also set, otherwise it
            # will turn them into runtime assertions.
            DefSymbol("ALWAYS", "EIGEN_NO_STATIC_ASSERT")

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
            if PkgSkip(x):
                tomit = tomit + x + " "
            else:
                tkeep = tkeep + x + " "

        if RTDIST:
            print("Makepanda: Runtime distribution build")
        elif RUNTIME:
            print("Makepanda: Runtime build")
        else:
            print("Makepanda: Regular build")

        print("Makepanda: Compiler: %s" % (COMPILER))
        print("Makepanda: Optimize: %d" % (GetOptimize()))
        print("Makepanda: Keep Pkg: %s" % (tkeep))
        print("Makepanda: Omit Pkg: %s" % (tomit))

        if GENMAN:
            print("Makepanda: Generate API reference manual")
        else:
            print("Makepanda: Don't generate API reference manual")

        if GetHost() == "windows" and not RTDIST:
            if INSTALLER:
                print("Makepanda: Build installer, using %s" % (COMPRESSOR))
            else:
                print("Makepanda: Don't build installer")

        print("Makepanda: Version ID: %s" % (VERSION))
        for x in warnings:
            print("Makepanda: %s" % (x))
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
            cmd += "/wd4996 /wd4275 /wd4273 "

            # We still target Windows XP.
            cmd += "/DWINVER=0x501 "
            # Work around a WinXP/2003 bug when using VS 2015+.
            if SDK.get("VISUALSTUDIO_VERSION") >= (14,0):
                cmd += "/Zc:threadSafeInit- "

            cmd += "/Fo" + obj + " /nologo /c"
            if GetTargetArch() != 'x64' and (not PkgSkip("SSE2") or 'SSE2' in opts):
                cmd += " /arch:SSE2"
            for x in ipath: cmd += " /I" + x
            for (opt,dir) in INCDIRECTORIES:
                if (opt=="ALWAYS") or (opt in opts): cmd += " /I" + BracketNameWithQuotes(dir)
            for (opt,var,val) in DEFSYMBOLS:
                if (opt=="ALWAYS") or (opt in opts): cmd += " /D" + var + "=" + val
            if (opts.count('MSFORSCOPE')): cmd += ' /Zc:forScope-'

            if (optlevel==1): cmd += " /MDd /Zi /RTCs /GS"
            if (optlevel==2): cmd += " /MDd /Zi"
            if (optlevel==3): cmd += " /MD /Zi /GS- /O2 /Ob2 /Oi /Ot /fp:fast"
            if (optlevel==4):
                cmd += " /MD /Zi /GS- /Ox /Ob2 /Oi /Ot /fp:fast /DFORCE_INLINING /DNDEBUG /GL"
                cmd += " /Oy /Zp16"      # jean-claude add /Zp16 insures correct static alignment for SSEx

            cmd += " /Fd" + os.path.splitext(obj)[0] + ".pdb"

            building = GetValueOption(opts, "BUILDING:")
            if (building):
                cmd += " /DBUILDING_" + building

            if ("BIGOBJ" in opts) or GetTargetArch() == 'x64':
                cmd += " /bigobj"

            cmd += " /Zm300 /DWIN32_VC /DWIN32"
            if 'EXCEPTIONS' in opts:
                cmd += " /EHsc"
            else:
                cmd += " /D_HAS_EXCEPTIONS=0"

            if 'RTTI' not in opts:
                 cmd += " /GR-"

            if GetTargetArch() == 'x64':
                cmd += " /DWIN64_VC /DWIN64"

            if WINDOWS_SDK.startswith('7.') and MSVC_VERSION > (10,):
                # To preserve Windows XP compatibility.
                cmd += " /D_USING_V110_SDK71_"

            cmd += " /W3 " + BracketNameWithQuotes(src)
            oscmd(cmd)
        else:
            cmd = "icl "
            if GetTargetArch() == 'x64':
                cmd += "/favor:blend "
            cmd += "/wd4996 /wd4275 /wd4267 /wd4101 /wd4273 "
            cmd += "/DWINVER=0x501 "
            cmd += "/Fo" + obj + " /c"
            for x in ipath: cmd += " /I" + x
            for (opt,dir) in INCDIRECTORIES:
                if (opt=="ALWAYS") or (opt in opts): cmd += " /I" + BracketNameWithQuotes(dir)
            for (opt,var,val) in DEFSYMBOLS:
                if (opt=="ALWAYS") or (opt in opts): cmd += " /D" + var + "=" + val
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
        else:                    cmd = GetCXX()+' -std=gnu++11 -ftemplate-depth-70 -fPIC -c -o ' + obj
        for (opt, dir) in INCDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts): cmd += ' -I' + BracketNameWithQuotes(dir)
        for (opt, dir) in FRAMEWORKDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts): cmd += ' -F' + BracketNameWithQuotes(dir)
        for (opt,var,val) in DEFSYMBOLS:
            if (opt=="ALWAYS") or (opt in opts): cmd += ' -D' + var + '=' + val
        for x in ipath: cmd += ' -I' + x

        if not GetLinkAllStatic() and 'NOHIDDEN' not in opts:
            cmd += ' -fvisibility=hidden'

        # Mac-specific flags.
        if GetTarget() == "darwin":
            cmd += " -Wno-deprecated-declarations"
            if OSXTARGET is not None:
                cmd += " -isysroot " + SDK["MACOSX"]
                cmd += " -mmacosx-version-min=%d.%d" % (OSXTARGET)

            # Use libc++ to enable C++11 features.
            cmd += " -stdlib=libc++"

            arch = GetTargetArch()
            cmd += " -arch %s" % arch

        if "SYSROOT" in SDK:
            if GetTarget() != "android":
                cmd += ' --sysroot=%s' % (SDK["SYSROOT"])
            else:
                ndk_dir = SDK["ANDROID_NDK"].replace('\\', '/')
                cmd += ' -isystem %s/sysroot/usr/include' % (ndk_dir)
                cmd += ' -isystem %s/sysroot/usr/include/%s' % (ndk_dir, SDK["ANDROID_TRIPLE"])
            cmd += ' -no-canonical-prefixes'

        # Android-specific flags.
        arch = GetTargetArch()

        if GetTarget() == "android":
            # Most of the specific optimization flags here were
            # just copied from the default Android Makefiles.
            if "ANDROID_API" in SDK:
                cmd += ' -D__ANDROID_API__=' + str(SDK["ANDROID_API"])
            if "ANDROID_GCC_TOOLCHAIN" in SDK:
                cmd += ' -gcc-toolchain ' + SDK["ANDROID_GCC_TOOLCHAIN"].replace('\\', '/')
            cmd += ' -ffunction-sections -funwind-tables'
            if arch == 'armv7a':
                cmd += ' -target armv7-none-linux-androideabi'
                cmd += ' -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16'
                cmd += ' -fno-integrated-as'
            elif arch == 'arm':
                cmd += ' -target armv5te-none-linux-androideabi'
                cmd += ' -march=armv5te -mtune=xscale -msoft-float'
                cmd += ' -fno-integrated-as'
            elif arch == 'aarch64':
                cmd += ' -target aarch64-none-linux-android'
            elif arch == 'mips':
                cmd += ' -target mipsel-none-linux-android'
                cmd += ' -mips32'
            elif arch == 'mips64':
                cmd += ' -target mips64el-none-linux-android'
                cmd += ' -fintegrated-as'
            elif arch == 'x86':
                cmd += ' -target i686-none-linux-android'
                cmd += ' -march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32'
                cmd += ' -mstackrealign'
            elif arch == 'x86_64':
                cmd += ' -target x86_64-none-linux-android'
                cmd += ' -march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=intel'

            cmd += " -Wa,--noexecstack"

            # Do we want thumb or arm instructions?
            if arch.startswith('arm'):
                if optlevel >= 3:
                    cmd += ' -mthumb'
                else:
                    cmd += ' -marm'

            # Enable SIMD instructions if requested
            if arch.startswith('arm') and PkgSkip("NEON") == 0:
                cmd += ' -mfpu=neon'

        else:
            cmd += " -pthread"

        if not src.endswith(".c"):
            # We don't use exceptions for most modules.
            if 'EXCEPTIONS' in opts:
                cmd += " -fexceptions"
            else:
                cmd += " -fno-exceptions"

                if src.endswith(".mm"):
                    # Work around Apple compiler bug.
                    cmd += " -U__EXCEPTIONS"

            target = GetTarget()
            if 'RTTI' not in opts and target != "darwin":
                # We always disable RTTI on Android for memory usage reasons.
                if optlevel >= 4 or target == "android":
                    cmd += " -fno-rtti"

        if ('SSE2' in opts or not PkgSkip("SSE2")) and not arch.startswith("arm") and arch != 'aarch64':
            cmd += " -msse2"

        # Needed by both Python, Panda, Eigen, all of which break aliasing rules.
        cmd += " -fno-strict-aliasing"

        if optlevel >= 3:
            cmd += " -ffast-math -fno-stack-protector"
        if optlevel == 3:
            # Fast math is nice, but we'd like to see NaN in dev builds.
            cmd += " -fno-finite-math-only"

        # Make sure this is off to avoid GCC/Eigen bug (see GitHub #228)
        cmd += " -fno-unsafe-math-optimizations"

        if (optlevel==1): cmd += " -ggdb -D_DEBUG"
        if (optlevel==2): cmd += " -O1 -D_DEBUG"
        if (optlevel==3): cmd += " -O2"
        if (optlevel==4): cmd += " -O3 -DNDEBUG"

        # Enable more warnings.
        cmd += " -Wall -Wno-unused-function"

        if not src.endswith(".c"):
            cmd += " -Wno-reorder"

        # Ignore unused variables in NDEBUG builds, often used in asserts.
        if optlevel == 4:
            cmd += " -Wno-unused-variable"

        if src.endswith(".c"):
            cmd += ' ' + CFLAGS
        else:
            cmd += ' ' + CXXFLAGS
        cmd = cmd.rstrip()

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
    CompileCxx(wobj, wdstc, opts + ["FLEX"])

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
    srcdir = GetValueOption(opts, "SRCDIR:")
    module = GetValueOption(opts, "IMOD:")
    library = GetValueOption(opts, "ILIB:")
    ipath = GetListOption(opts, "DIR:")
    if (PkgSkip("PYTHON")):
        WriteFile(woutc, "")
        WriteFile(woutd, "")
        ConditionalWriteFile(woutd, "")
        return

    if not CrossCompiling():
        # If we're compiling for this platform, we can use the one we've built.
        cmd = os.path.join(GetOutputDir(), 'bin', 'interrogate')
    else:
        # Assume that interrogate is on the PATH somewhere.
        cmd = 'interrogate'

    if GetVerbose():
        cmd += ' -v'

    cmd += ' -srcdir %s -I%s' % (srcdir, srcdir)
    cmd += ' -DCPPPARSER -D__STDC__=1 -D__cplusplus=201103L'
    if (COMPILER=="MSVC"):
        cmd += ' -DWIN32_VC -DWIN32 -D_WIN32'
        if GetTargetArch() == 'x64':
            cmd += ' -DWIN64_VC -DWIN64 -D_WIN64 -D_M_X64 -D_M_AMD64'
        else:
            cmd += ' -D_M_IX86'
        # NOTE: this 1600 value is the version number for VC2010.
        cmd += ' -D_MSC_VER=1600 -D"__declspec(param)=" -D__cdecl -D_near -D_far -D__near -D__far -D__stdcall'
    if (COMPILER=="GCC"):
        cmd += ' -D__attribute__\\(x\\)='
        target_arch = GetTargetArch()
        if target_arch in ("x86_64", "amd64"):
            cmd += ' -D_LP64'
        elif target_arch == 'aarch64':
            cmd += ' -D_LP64 -D__LP64__ -D__aarch64__'
        else:
            cmd += ' -D__i386__'

        target = GetTarget()
        if target == 'darwin':
            cmd += ' -D__APPLE__'
        elif target == 'android':
            cmd += ' -D__ANDROID__'

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

    #building = GetValueOption(opts, "BUILDING:")
    #if (building): cmd += " -DBUILDING_"+building
    cmd += ' -module ' + module + ' -library ' + library
    for x in wsrc:
        if (x.startswith("/")):
            cmd += ' ' + BracketNameWithQuotes(x)
        else:
            cmd += ' ' + BracketNameWithQuotes(os.path.basename(x))
    oscmd(cmd)

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

    cmd += ' -oc ' + woutc + ' -module ' + module + ' -library ' + library + ' -python-native'
    importmod = GetValueOption(opts, "IMPORT:")
    if importmod:
        cmd += ' -import ' + importmod
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
            if GetOptimizeOption(opts) == 4:
                cmd += " /LTCG"
            if HasTargetArch():
                cmd += " /MACHINE:" + GetTargetArch().upper()
            cmd += ' /OUT:' + BracketNameWithQuotes(lib)
            for x in obj:
                if not x.endswith('.lib'):
                    cmd += ' ' + BracketNameWithQuotes(x)
            oscmd(cmd)
        else:
            # Choose Intel linker; from Jean-Claude
            cmd = 'xilink /verbose:lib /lib '
            if HasTargetArch():
                cmd += " /MACHINE:" + GetTargetArch().upper()
            cmd += ' /OUT:' + BracketNameWithQuotes(lib)
            for x in obj: cmd += ' ' + BracketNameWithQuotes(x)
            cmd += ' /LIBPATH:"C:\\Program Files (x86)\\Intel\\Composer XE 2011 SP1\\ipp\\lib\\ia32"'
            cmd += ' /LIBPATH:"C:\\Program Files (x86)\\Intel\\Composer XE 2011 SP1\\TBB\\Lib\\ia32\\vc10"'
            cmd += ' /LIBPATH:"C:\\Program Files (x86)\\Intel\\Composer XE 2011 SP1\\compiler\\lib\\ia32"'
            oscmd(cmd)

    if (COMPILER=="GCC"):
        if GetTarget() == 'darwin':
            cmd = 'libtool -static -o ' + BracketNameWithQuotes(lib)
        else:
            cmd = GetAR() + ' cru ' + BracketNameWithQuotes(lib)
        for x in obj:
            if GetLinkAllStatic() and x.endswith('.a'):
                continue
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
            cmd += " /nod:libc /nod:libcmtd /nod:atlthunk /nod:atls /nod:atlsd"
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

            if not PkgSkip("PYTHON"):
                # If we're building without Python, don't pick it up implicitly.
                if "PYTHON" not in opts:
                    pythonv = SDK["PYTHONVERSION"].replace('.', '')
                    if optlevel <= 2:
                        cmd += ' /NOD:{}d.lib'.format(pythonv)
                    else:
                        cmd += ' /NOD:{}.lib'.format(pythonv)

                # Yes, we know we are importing "locally defined symbols".
                for x in obj:
                    if x.endswith('libp3pystub.lib'):
                        cmd += ' /ignore:4049,4217'
                        break

            # Set the subsystem.  Specify that we want to target Windows XP.
            subsystem = GetValueOption(opts, "SUBSYSTEM:") or "CONSOLE"
            cmd += " /SUBSYSTEM:" + subsystem
            if GetTargetArch() == 'x64':
                cmd += ",5.02"
            else:
                cmd += ",5.01"

            if dll.endswith(".dll") or dll.endswith(".pyd"):
                cmd += ' /IMPLIB:' + GetOutputDir() + '/lib/' + os.path.splitext(os.path.basename(dll))[0] + ".lib"

            for (opt, dir) in LIBDIRECTORIES:
                if (opt=="ALWAYS") or (opt in opts):
                    cmd += ' /LIBPATH:' + BracketNameWithQuotes(dir)

            for x in obj:
                if x.endswith(".dll") or x.endswith(".pyd"):
                    cmd += ' ' + GetOutputDir() + '/lib/' + os.path.splitext(os.path.basename(x))[0] + ".lib"
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
            cmd += ' /LIBPATH:"C:\\Program Files (x86)\\Intel\\Composer XE 2011 SP1\\ipp\\lib\\ia32"'
            cmd += ' /LIBPATH:"C:\\Program Files (x86)\\Intel\\Composer XE 2011 SP1\\TBB\\Lib\\ia32\\vc10"'
            cmd += ' /LIBPATH:"C:\\Program Files (x86)\\Intel\\Composer XE 2011 SP1\\compiler\\lib\\ia32"'
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
                if x.endswith(".dll") or x.endswith(".pyd"):
                    cmd += ' ' + GetOutputDir() + '/lib/' + os.path.splitext(os.path.basename(x))[0] + ".lib"
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
        if GetOrigExt(dll) == ".exe":
            cmd = cxx + ' -o ' + dll + ' -L' + GetOutputDir() + '/lib -L' + GetOutputDir() + '/tmp'
            if GetTarget() == "android":
                # Necessary to work around an issue with libandroid depending on vendor libraries
                cmd += ' -Wl,--allow-shlib-undefined'
        else:
            if (GetTarget() == "darwin"):
                cmd = cxx + ' -undefined dynamic_lookup'
                if ("BUNDLE" in opts or GetOrigExt(dll) == ".pyd"):
                    cmd += ' -bundle '
                else:
                    install_name = '@loader_path/../lib/' + os.path.basename(dll)
                    cmd += ' -dynamiclib -install_name ' + install_name
                    cmd += ' -compatibility_version ' + MAJOR_VERSION + ' -current_version ' + VERSION
                cmd += ' -o ' + dll + ' -L' + GetOutputDir() + '/lib -L' + GetOutputDir() + '/tmp'
            else:
                cmd = cxx + ' -shared'
                # Always set soname on Android to avoid a linker warning when loading the library.
                if "MODULE" not in opts or GetTarget() == 'android':
                    cmd += " -Wl,-soname=" + os.path.basename(dll)
                cmd += ' -o ' + dll + ' -L' + GetOutputDir() + '/lib -L' + GetOutputDir() + '/tmp'

        for x in obj:
            if GetOrigExt(x) != ".dat":
                cmd += ' ' + x

        if (GetOrigExt(dll) == ".exe" and GetTarget() == 'windows' and "NOICON" not in opts):
            cmd += " " + GetOutputDir() + "/tmp/pandaIcon.res"

        # Mac OS X specific flags.
        if GetTarget() == 'darwin':
            cmd += " -headerpad_max_install_names"
            if OSXTARGET is not None:
                cmd += " -isysroot " + SDK["MACOSX"] + " -Wl,-syslibroot," + SDK["MACOSX"]
                cmd += " -mmacosx-version-min=%d.%d" % (OSXTARGET)

            # Use libc++ to enable C++11 features.
            cmd += " -stdlib=libc++"

            arch = GetTargetArch()
            cmd += " -arch %s" % arch

        elif GetTarget() == 'android':
            arch = GetTargetArch()
            if "ANDROID_GCC_TOOLCHAIN" in SDK:
                cmd += ' -gcc-toolchain ' + SDK["ANDROID_GCC_TOOLCHAIN"].replace('\\', '/')
            cmd += " -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now"
            if arch == 'armv7a':
                cmd += ' -target armv7-none-linux-androideabi'
                cmd += " -march=armv7-a -Wl,--fix-cortex-a8"
            elif arch == 'arm':
                cmd += ' -target armv5te-none-linux-androideabi'
            elif arch == 'aarch64':
                cmd += ' -target aarch64-none-linux-android'
            elif arch == 'mips':
                cmd += ' -target mipsel-none-linux-android'
                cmd += ' -mips32'
            elif arch == 'mips64':
                cmd += ' -target mips64el-none-linux-android'
            elif arch == 'x86':
                cmd += ' -target i686-none-linux-android'
            elif arch == 'x86_64':
                cmd += ' -target x86_64-none-linux-android'
            cmd += ' -lc -lm'
        else:
            cmd += " -pthread"

        if "SYSROOT" in SDK:
            cmd += " --sysroot=%s -no-canonical-prefixes" % (SDK["SYSROOT"])

        if LDFLAGS != "":
            cmd += " " + LDFLAGS

        # Don't link libraries with Python, except on Android.
        if "PYTHON" in opts and GetOrigExt(dll) != ".exe" and not RTDIST and GetTarget() != 'android':
            opts = opts[:]
            opts.remove("PYTHON")

        for (opt, dir) in LIBDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts):
                cmd += ' -L' + BracketNameWithQuotes(dir)
        for (opt, dir) in FRAMEWORKDIRECTORIES:
            if (opt=="ALWAYS") or (opt in opts):
                cmd += ' -F' + BracketNameWithQuotes(dir)
        for (opt, name) in LIBNAMES:
            if (opt=="ALWAYS") or (opt in opts):
                cmd += ' ' + BracketNameWithQuotes(name)

        if GetTarget() != 'freebsd':
            cmd += " -ldl"

        oscmd(cmd)

        if GetOptimizeOption(opts) == 4 and GetTarget() in ('linux', 'android'):
            oscmd(GetStrip() + " --strip-unneeded " + BracketNameWithQuotes(dll))

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
    if os.path.isfile("/usr/bin/Rez"):
        cmd = "Rez -useDF"
    else:
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
# CompileJava (Android only)
#
##########################################################################################

def CompileJava(target, src, opts):
    """Compiles a .java file into a .class file."""
    if GetHost() == 'android':
        cmd = "ecj "
    else:
        cmd = "javac -bootclasspath " + BracketNameWithQuotes(SDK["ANDROID_JAR"]) + " "

    optlevel = GetOptimizeOption(opts)
    if optlevel >= 4:
        cmd += "-debug:none "

    cmd += "-cp " + GetOutputDir() + "/classes "
    cmd += "-d " + GetOutputDir() + "/classes "
    cmd += BracketNameWithQuotes(src)
    oscmd(cmd)

##########################################################################################
#
# FreezePy
#
##########################################################################################

def FreezePy(target, inputs, opts):
    assert len(inputs) > 0

    cmdstr = BracketNameWithQuotes(SDK["PYTHONEXEC"].replace('\\', '/')) + " "
    if sys.version_info >= (2, 6):
        cmdstr += "-B "

    cmdstr += os.path.join(GetOutputDir(), "direct", "dist", "pfreeze.py")

    if 'FREEZE_STARTUP' in opts:
        cmdstr += " -s"

    if GetOrigExt(target) == '.exe':
        src = inputs.pop(0)
    else:
        src = ""

    for i in inputs:
        i = os.path.splitext(i)[0]
        i = i.replace('/', '.')

        if i.startswith('direct.src'):
            i = i.replace('.src.', '.')

        cmdstr += " -i " + i

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
    command = BracketNameWithQuotes(SDK["PYTHONEXEC"]) + " "
    if GetOptimizeOption(opts) >= 4:
        command += "-OO "

    if sys.version_info >= (2, 6):
        command += "-B "

    command += "direct/src/p3d/ppackage.py"

    if not RTDIST:
        # Don't compile Python sources, because we might not running in the same
        # Python version as the selected host.
        command += " -N"

    if GetTarget() == "darwin":
        if SDK.get("MACOSX"):
            command += " -R \"%s\"" % SDK["MACOSX"]

        arch = GetTargetArch()
        if arch == "x86_64":
            arch = "amd64"
        command += " -P osx_%s" % arch

    command += " -i \"" + GetOutputDir() + "/stage\""
    if (P3DSUFFIX):
        command += ' -a "' + P3DSUFFIX + '"'

    command += " " + inputs[0]

    if GetOrigExt(target) == '.p3d':
        # Build a specific .p3d file.
        basename = os.path.basename(os.path.splitext(target)[0])
        command += " " + basename
        oscmd(command)

        if GetTarget() == 'windows':
            # Make an .exe that calls this .p3d.
            objfile = FindLocation('p3dWrapper_' + basename + '.obj', [])
            CompileCxx(objfile, 'direct/src/p3d/p3dWrapper.c', [])

            exefile = FindLocation(basename + '.exe', [])
            CompileLink(exefile, [objfile], ['ADVAPI'])

        # Move it to the bin directory.
        os.rename(GetOutputDir() + '/stage/' + basename + P3DSUFFIX + '.p3d', target)

        if sys.platform != 'win32':
            oscmd('chmod +x ' + BracketNameWithQuotes(target))
    else:
        # This is presumably a package or set of packages.
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

    elif (infile.endswith(".py")):
        if origsuffix == ".obj":
            source = os.path.splitext(target)[0] + ".c"
            SetOrigExt(source, ".c")
            ProgressOutput(progress, "Building frozen source", source)
            FreezePy(source, inputs, opts)
            ProgressOutput(progress, "Building C++ object", target)
            return CompileCxx(target, source, opts)

        if origsuffix == ".exe":
            ProgressOutput(progress, "Building frozen executable", target)
        else:
            ProgressOutput(progress, "Building frozen library", target)
        return FreezePy(target, inputs, opts)

    elif (infile.endswith(".idl")):
        ProgressOutput(progress, "Compiling MIDL file", infile)
        return CompileMIDL(target, infile, opts)
    elif (infile.endswith(".pdef")):
        if origsuffix == '.p3d':
            ProgressOutput(progress, "Building package", target)
        else:
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
    elif (origsuffix==".class"):
        ProgressOutput(progress, "Building Java class", target)
        return CompileJava(target, infile, opts)
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
    ("HAVE_SOFTIMAGE",                 'UNDEF',                  'UNDEF'),
    ("REPORT_OPENSSL_ERRORS",          '1',                      '1'),
    ("USE_PANDAFILESTREAM",            '1',                      '1'),
    ("USE_DELETED_CHAIN",              '1',                      '1'),
    ("HAVE_GLX",                       'UNDEF',                  '1'),
    ("HAVE_WGL",                       '1',                      'UNDEF'),
    ("HAVE_DX9",                       'UNDEF',                  'UNDEF'),
    ("HAVE_THREADS",                   '1',                      '1'),
    ("SIMPLE_THREADS",                 'UNDEF',                  'UNDEF'),
    ("OS_SIMPLE_THREADS",              '1',                      '1'),
    ("DEBUG_THREADS",                  'UNDEF',                  'UNDEF'),
    ("HAVE_POSIX_THREADS",             'UNDEF',                  '1'),
    ("MUTEX_SPINLOCK",                 'UNDEF',                  'UNDEF'),
    ("HAVE_AUDIO",                     '1',                      '1'),
    ("NOTIFY_DEBUG",                   'UNDEF',                  'UNDEF'),
    ("DO_PSTATS",                      'UNDEF',                  'UNDEF'),
    ("DO_DCAST",                       'UNDEF',                  'UNDEF'),
    ("DO_COLLISION_RECORDING",         'UNDEF',                  'UNDEF'),
    ("SUPPORT_IMMEDIATE_MODE",         'UNDEF',                  'UNDEF'),
    ("SUPPORT_FIXED_FUNCTION",         '1',                      '1'),
    ("DO_MEMORY_USAGE",                'UNDEF',                  'UNDEF'),
    ("DO_PIPELINING",                  '1',                      '1'),
    ("DEFAULT_PATHSEP",                '";"',                    '":"'),
    ("WORDS_BIGENDIAN",                'UNDEF',                  'UNDEF'),
    ("PHAVE_LOCKF",                    '1',                      '1'),
    ("SIMPLE_STRUCT_POINTERS",         '1',                      'UNDEF'),
    ("HAVE_DINKUM",                    'UNDEF',                  'UNDEF'),
    ("HAVE_STL_HASH",                  'UNDEF',                  'UNDEF'),
    ("GETTIMEOFDAY_ONE_PARAM",         'UNDEF',                  'UNDEF'),
    ("HAVE_GETOPT",                    'UNDEF',                  '1'),
    ("HAVE_GETOPT_LONG_ONLY",          'UNDEF',                  '1'),
    ("PHAVE_GETOPT_H",                 'UNDEF',                  '1'),
    ("PHAVE_LINUX_INPUT_H",            'UNDEF',                  '1'),
    ("IOCTL_TERMINAL_WIDTH",           'UNDEF',                  '1'),
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
    ("PHAVE_SSTREAM",                  '1',                      '1'),
    ("PHAVE_NEW",                      '1',                      '1'),
    ("PHAVE_SYS_TYPES_H",              '1',                      '1'),
    ("PHAVE_SYS_TIME_H",               'UNDEF',                  '1'),
    ("PHAVE_UNISTD_H",                 'UNDEF',                  '1'),
    ("PHAVE_UTIME_H",                  'UNDEF',                  '1'),
    ("PHAVE_GLOB_H",                   'UNDEF',                  '1'),
    ("PHAVE_DIRENT_H",                 'UNDEF',                  '1'),
    ("PHAVE_UCONTEXT_H",               'UNDEF',                  '1'),
    ("PHAVE_STDINT_H",                 '1',                      '1'),
    ("HAVE_RTTI",                      '1',                      '1'),
    ("HAVE_X11",                       'UNDEF',                  '1'),
    ("IS_LINUX",                       'UNDEF',                  '1'),
    ("IS_OSX",                         'UNDEF',                  'UNDEF'),
    ("IS_FREEBSD",                     'UNDEF',                  'UNDEF'),
    ("HAVE_EIGEN",                     'UNDEF',                  'UNDEF'),
    ("LINMATH_ALIGN",                  '1',                      '1'),
    ("HAVE_ZLIB",                      'UNDEF',                  'UNDEF'),
    ("HAVE_PNG",                       'UNDEF',                  'UNDEF'),
    ("HAVE_JPEG",                      'UNDEF',                  'UNDEF'),
    ("HAVE_VIDEO4LINUX",               'UNDEF',                  '1'),
    ("HAVE_TIFF",                      'UNDEF',                  'UNDEF'),
    ("HAVE_OPENEXR",                   'UNDEF',                  'UNDEF'),
    ("HAVE_SGI_RGB",                   '1',                      '1'),
    ("HAVE_TGA",                       '1',                      '1'),
    ("HAVE_IMG",                       '1',                      '1'),
    ("HAVE_SOFTIMAGE_PIC",             '1',                      '1'),
    ("HAVE_BMP",                       '1',                      '1'),
    ("HAVE_PNM",                       '1',                      '1'),
    ("HAVE_STB_IMAGE",                 '1',                      '1'),
    ("HAVE_VORBIS",                    'UNDEF',                  'UNDEF'),
    ("HAVE_OPUS",                      'UNDEF',                  'UNDEF'),
    ("HAVE_FREETYPE",                  'UNDEF',                  'UNDEF'),
    ("HAVE_FFTW",                      'UNDEF',                  'UNDEF'),
    ("HAVE_OPENSSL",                   'UNDEF',                  'UNDEF'),
    ("HAVE_NET",                       'UNDEF',                  'UNDEF'),
    ("WANT_NATIVE_NET",                '1',                      '1'),
    ("SIMULATE_NETWORK_DELAY",         'UNDEF',                  'UNDEF'),
    ("HAVE_CG",                        'UNDEF',                  'UNDEF'),
    ("HAVE_CGGL",                      'UNDEF',                  'UNDEF'),
    ("HAVE_CGDX9",                     'UNDEF',                  'UNDEF'),
    ("HAVE_ARTOOLKIT",                 'UNDEF',                  'UNDEF'),
    ("HAVE_DIRECTCAM",                 'UNDEF',                  'UNDEF'),
    ("HAVE_SQUISH",                    'UNDEF',                  'UNDEF'),
    ("HAVE_COCOA",                     'UNDEF',                  'UNDEF'),
    ("HAVE_OPENAL_FRAMEWORK",          'UNDEF',                  'UNDEF'),
    ("HAVE_ROCKET_PYTHON",             '1',                      '1'),
    ("HAVE_ROCKET_DEBUGGER",           'UNDEF',                  'UNDEF'),
    ("USE_TAU",                        'UNDEF',                  'UNDEF'),
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

    if GetTarget() not in ("linux", "android"):
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
        dtool_config["HAVE_GLX"] = 'UNDEF'
        dtool_config["IS_LINUX"] = 'UNDEF'
        dtool_config["HAVE_VIDEO4LINUX"] = 'UNDEF'
        dtool_config["PHAVE_LINUX_INPUT_H"] = 'UNDEF'
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
        dtool_config["PHAVE_LOCKF"] = 'UNDEF'
        dtool_config["HAVE_VIDEO4LINUX"] = 'UNDEF'

    if (GetOptimize() <= 2 and GetTarget() == "windows"):
        dtool_config["USE_DEBUG_PYTHON"] = '1'

    # This should probably be more sophisticated, such as based
    # on whether the libRocket Python modules are available.
    if (PkgSkip("PYTHON") != 0):
        dtool_config["HAVE_ROCKET_PYTHON"] = 'UNDEF'

    if (GetOptimize() <= 3):
        dtool_config["HAVE_ROCKET_DEBUGGER"] = '1'

    if (GetOptimize() <= 3):
        if (dtool_config["HAVE_NET"] != 'UNDEF'):
            dtool_config["DO_PSTATS"] = '1'

    if (GetOptimize() <= 3):
        dtool_config["DO_DCAST"] = '1'

    if (GetOptimize() <= 3):
        dtool_config["DO_COLLISION_RECORDING"] = '1'

    if (GetOptimize() <= 3):
        dtool_config["DO_MEMORY_USAGE"] = '1'

    if (GetOptimize() <= 3):
        dtool_config["NOTIFY_DEBUG"] = '1'

    if (GetOptimize() >= 4):
        dtool_config["PRC_SAVE_DESCRIPTIONS"] = 'UNDEF'

    if (GetOptimize() >= 4):
        # Disable RTTI on release builds.
        dtool_config["HAVE_RTTI"] = 'UNDEF'

    # Now that we have OS_SIMPLE_THREADS, we can support
    # SIMPLE_THREADS on exotic architectures like win64, so we no
    # longer need to disable it for this platform.
##     if GetTarget() == 'windows' and GetTargetArch() == 'x64':
##         dtool_config["SIMPLE_THREADS"] = 'UNDEF'

    if (RTDIST or RUNTIME):
        prc_parameters["DEFAULT_PRC_DIR"] = '""'
        if HOST_URL:
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

    # Finally, write a platform.dat with the platform we are compiling for.
    ConditionalWriteFile(GetOutputDir() + '/tmp/platform.dat', PLATFORM)

    # This is useful for tools like makepackage that need to know things about
    # the build parameters.
    ConditionalWriteFile(GetOutputDir() + '/tmp/optimize.dat', str(GetOptimize()))


WriteConfigSettings()

WarnConflictingFiles()
if SystemLibraryExists("dtoolbase"):
    Warn("Found conflicting Panda3D libraries from other ppremake build!")
if SystemLibraryExists("p3dtoolconfig"):
    Warn("Found conflicting Panda3D libraries from other makepanda build!")

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
EXPCL_DTOOL_DTOOLBASE int panda_version_$VERSION1_$VERSION2 = 0;
"""

CHECKPANDAVERSION_H="""
# ifndef CHECKPANDAVERSION_H
# define CHECKPANDAVERSION_H
# include "dtoolbase.h"
extern EXPCL_DTOOL_DTOOLBASE int panda_version_$VERSION1_$VERSION2;
// Hack to forcibly depend on the check
template<typename T>
class CheckPandaVersion {
public:
  int check_version() { return panda_version_$VERSION1_$VERSION2; }
};
template class CheckPandaVersion<void>;
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
    pandaversion_h = pandaversion_h.replace("$HOST_URL",(HOST_URL or ""))
    if (DISTRIBUTOR == "cmu"):
        pandaversion_h += "\n#define PANDA_OFFICIAL_VERSION\n"
    else:
        pandaversion_h += "\n#undef  PANDA_OFFICIAL_VERSION\n"

    if GIT_COMMIT:
        pandaversion_h += "\n#define PANDA_GIT_COMMIT_STR \"%s\"\n" % (GIT_COMMIT)

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
    CopyPythonTree(GetOutputDir() + '/direct', 'direct/src', threads=THREADCOUNT)
    ConditionalWriteFile(GetOutputDir() + '/direct/__init__.py', "")

    # This file used to be copied, but would nowadays cause conflicts.
    # Let's get it out of the way in case someone hasn't cleaned their build since.
    if os.path.isfile(GetOutputDir() + '/bin/panda3d.py'):
        os.remove(GetOutputDir() + '/bin/panda3d.py')
    if os.path.isfile(GetOutputDir() + '/lib/panda3d.py'):
        os.remove(GetOutputDir() + '/lib/panda3d.py')

    # This directory doesn't exist at all any more.
    if os.path.isdir(os.path.join(GetOutputDir(), 'direct', 'ffi')):
        shutil.rmtree(os.path.join(GetOutputDir(), 'direct', 'ffi'))

# These files used to exist; remove them to avoid conflicts.
del_files = ['core.py', 'core.pyc', 'core.pyo',
             '_core.pyd', '_core.so',
             'direct.py', 'direct.pyc', 'direct.pyo',
             '_direct.pyd', '_direct.so',
             'dtoolconfig.pyd', 'dtoolconfig.so']

for basename in del_files:
    path = os.path.join(GetOutputDir(), 'panda3d', basename)
    if os.path.isfile(path):
        print("Removing %s" % (path))
        os.remove(path)

# Write an appropriate panda3d/__init__.py
p3d_init = """"Python bindings for the Panda3D libraries"

__version__ = '%s'
""" % (WHLVERSION)

if GetTarget() == 'windows':
    p3d_init += """
if '__file__' in locals():
    import os

    bindir = os.path.join(os.path.dirname(__file__), '..', 'bin')
    if os.path.isdir(bindir):
        if not os.environ.get('PATH'):
            os.environ['PATH'] = bindir
        else:
            os.environ['PATH'] = bindir + os.pathsep + os.environ['PATH']
    del os, bindir
"""

if not PkgSkip("PYTHON"):
    ConditionalWriteFile(GetOutputDir() + '/panda3d/__init__.py', p3d_init)

    # Also add this file, for backward compatibility.
    ConditionalWriteFile(GetOutputDir() + '/panda3d/dtoolconfig.py', """
if __debug__:
    print("Warning: panda3d.dtoolconfig is deprecated, use panda3d.interrogatedb instead.")
from .interrogatedb import *
""")

# PandaModules is now deprecated; generate a shim for backward compatibility.
for fn in glob.glob(GetOutputDir() + '/pandac/*.py') + glob.glob(GetOutputDir() + '/pandac/*.py[co]'):
    if os.path.basename(fn) not in ('PandaModules.py', '__init__.py'):
        os.remove(fn)

panda_modules = ['core']
if not PkgSkip("PANDAPHYSICS"):
    panda_modules.append('physics')
if not PkgSkip('PANDAFX'):
    panda_modules.append('fx')
if not PkgSkip("DIRECT"):
    panda_modules.append('direct')
if not PkgSkip("VISION"):
    panda_modules.append('vision')
if not PkgSkip("SKEL"):
    panda_modules.append('skel')
if not PkgSkip("EGG"):
    panda_modules.append('egg')
if not PkgSkip("ODE"):
    panda_modules.append('ode')
if not PkgSkip("VRPN"):
    panda_modules.append('vrpn')

panda_modules_code = """
"This module is deprecated.  Import from panda3d.core and other panda3d.* modules instead."

if __debug__:
    print("Warning: pandac.PandaModules is deprecated, import from panda3d.core instead")
"""

for module in panda_modules:
    panda_modules_code += """
try:
    from panda3d.%s import *
except ImportError as err:
    if "No module named %s" not in str(err):
        raise""" % (module, module)

panda_modules_code += """

from direct.showbase import DConfig

def get_config_showbase():
    return DConfig

def get_config_express():
    return DConfig

getConfigShowbase = get_config_showbase
getConfigExpress = get_config_express
"""

exthelpers_code = """
"This module is deprecated.  Import from direct.extensions_native.extension_native_helpers instead."
from direct.extensions_native.extension_native_helpers import *
"""

if not PkgSkip("PYTHON"):
    ConditionalWriteFile(GetOutputDir() + '/pandac/PandaModules.py', panda_modules_code)
    ConditionalWriteFile(GetOutputDir() + '/pandac/extension_native_helpers.py', exthelpers_code)
    ConditionalWriteFile(GetOutputDir() + '/pandac/__init__.py', '')

##########################################################################################
#
# Write the dist-info directory.
#
##########################################################################################

# This is just some basic stuff since setuptools just needs this file to
# exist, otherwise it will not read the entry_points.txt file.  Maybe we will
# eventually want to merge this with the metadata generator in makewheel.py.
METADATA = """Metadata-Version: 2.0
Name: Panda3D
Version: {version}
License: BSD
Home-page: https://www.panda3d.org/
Author: Panda3D Team
Author-email: etc-panda3d@lists.andrew.cmu.edu
"""

ENTRY_POINTS = """[distutils.commands]
build_apps = direct.dist.commands:build_apps
bdist_apps = direct.dist.commands:bdist_apps
"""

if not PkgSkip("DIRECT"):
    dist_dir = os.path.join(GetOutputDir(), 'panda3d.dist-info')
    MakeDirectory(dist_dir)

    ConditionalWriteFile(os.path.join(dist_dir, 'METADATA'), METADATA.format(version=VERSION))
    ConditionalWriteFile(os.path.join(dist_dir, 'entry_points.txt'), ENTRY_POINTS)

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

if PkgSkip("ASSIMP"):
    confautoprc = confautoprc.replace("load-file-type p3assimp", "#load-file-type p3assimp")

if (os.path.isfile("makepanda/myconfig.in")):
    configprc = ReadFile("makepanda/myconfig.in")
else:
    configprc = ReadFile("makepanda/config.in")

if (GetTarget() == 'windows'):
    configprc = configprc.replace("$XDG_CACHE_HOME/panda3d", "$USER_APPDATA/Panda3D-%s" % MAJOR_VERSION)
else:
    configprc = configprc.replace("aux-display pandadx9", "")

if (GetTarget() == 'darwin'):
    configprc = configprc.replace("$XDG_CACHE_HOME/panda3d", "$HOME/Library/Caches/Panda3D-%s" % MAJOR_VERSION)

    # OpenAL is not yet working well on OSX for us, so let's do this for now.
    configprc = configprc.replace("p3openal_audio", "p3fmod_audio")

if GetTarget() == 'windows':
    # Convert to Windows newlines.
    ConditionalWriteFile(GetOutputDir()+"/etc/Config.prc", configprc, newline='\r\n')
    ConditionalWriteFile(GetOutputDir()+"/etc/Confauto.prc", confautoprc, newline='\r\n')
else:
    ConditionalWriteFile(GetOutputDir()+"/etc/Config.prc", configprc)
    ConditionalWriteFile(GetOutputDir()+"/etc/Confauto.prc", confautoprc)

##########################################################################################
#
# Copy the precompiled binaries and DLLs into the build.
#
##########################################################################################

tp_dir = GetThirdpartyDir()
if tp_dir is not None:
    dylibs = {}

    if GetTarget() == 'darwin':
        # Make a list of all the dylibs we ship, to figure out whether we should use
        # install_name_tool to correct the library reference to point to our copy.
        for lib in glob.glob(tp_dir + "/*/lib/*.dylib"):
            dylibs[os.path.basename(lib)] = os.path.basename(os.path.realpath(lib))

        if not PkgSkip("PYTHON"):
            for lib in glob.glob(tp_dir + "/*/lib/" + SDK["PYTHONVERSION"] + "/*.dylib"):
                dylibs[os.path.basename(lib)] = os.path.basename(os.path.realpath(lib))

    for pkg in PkgListGet():
        if PkgSkip(pkg):
            continue
        tp_pkg = tp_dir + pkg.lower()

        if GetTarget() == 'windows':
            if os.path.exists(tp_pkg + "/bin"):
                CopyAllFiles(GetOutputDir() + "/bin/", tp_pkg + "/bin/")
                if (PkgSkip("PYTHON")==0 and os.path.exists(tp_pkg + "/bin/" + SDK["PYTHONVERSION"])):
                    CopyAllFiles(GetOutputDir() + "/bin/", tp_pkg + "/bin/" + SDK["PYTHONVERSION"] + "/")

        elif GetTarget() == 'darwin':
            tp_libs = glob.glob(tp_pkg + "/lib/*.dylib")

            if not PkgSkip("PYTHON"):
                tp_libs += glob.glob(os.path.join(tp_pkg, "lib", SDK["PYTHONVERSION"], "*.dylib"))
                tp_libs += glob.glob(os.path.join(tp_pkg, "lib", SDK["PYTHONVERSION"], "*.so"))
                if pkg != 'PYTHON':
                    tp_libs += glob.glob(os.path.join(tp_pkg, "lib", SDK["PYTHONVERSION"], "*.py"))

            for tp_lib in tp_libs:
                basename = os.path.basename(tp_lib)
                if basename.endswith('.dylib'):
                    # It's a dynamic link library.  Put it in the lib directory.
                    target = GetOutputDir() + "/lib/" + basename
                    dep_prefix = "@loader_path/../lib/"
                    lib_id = dep_prefix + basename
                else:
                    # It's a Python module, like _rocketcore.so.  Copy it to the root, because
                    # nowadays the 'lib' directory may no longer be on the PYTHONPATH.
                    target = GetOutputDir() + "/" + basename
                    dep_prefix = "@loader_path/lib/"
                    lib_id = basename

                if not NeedsBuild([target], [tp_lib]):
                    continue

                CopyFile(target, tp_lib)
                if os.path.islink(target) or target.endswith('.py'):
                    continue

                # Correct the inter-library dependencies so that the build is relocatable.
                oscmd('install_name_tool -id %s %s' % (lib_id, target))
                oscmd("otool -L %s | grep .dylib > %s/tmp/otool-libs.txt" % (target, GetOutputDir()), True)

                for line in open(GetOutputDir() + "/tmp/otool-libs.txt", "r"):
                    line = line.strip()
                    if not line or line.startswith(dep_prefix) or line.endswith(":"):
                        continue

                    libdep = line.split(" ", 1)[0]
                    dep_basename = os.path.basename(libdep)
                    if dep_basename in dylibs:
                        dep_target = dylibs[dep_basename]
                        oscmd("install_name_tool -change %s %s%s %s" % (libdep, dep_prefix, dep_target, target), True)

                JustBuilt([target], [tp_lib])

            for fwx in glob.glob(tp_pkg + "/*.framework"):
                CopyTree(GetOutputDir() + "/Frameworks/" + os.path.basename(fwx), fwx)

        else:  # Linux / FreeBSD case.
            for tp_lib in glob.glob(tp_pkg + "/lib/*.so*"):
                CopyFile(GetOutputDir() + "/lib/" + os.path.basename(tp_lib), tp_lib)

            if not PkgSkip("PYTHON"):
                for tp_lib in glob.glob(os.path.join(tp_pkg, "lib", SDK["PYTHONVERSION"], "*.so*")):
                    base = os.path.basename(tp_lib)
                    if base.startswith('lib'):
                        CopyFile(GetOutputDir() + "/lib/" + base, tp_lib)
                    else:
                        # It's a Python module, like _rocketcore.so.
                        CopyFile(GetOutputDir() + "/" + base, tp_lib)

    if GetTarget() == 'windows':
        if os.path.isdir(os.path.join(tp_dir, "extras", "bin")):
            CopyAllFiles(GetOutputDir() + "/bin/", tp_dir + "extras/bin/")

        if not PkgSkip("PYTHON") and not RTDIST:
            # We need to copy the Python DLL to the bin directory for now.
            pydll = "/" + SDK["PYTHONVERSION"].replace(".", "")
            if GetOptimize() <= 2:
                pydll += "_d.dll"
            else:
                pydll += ".dll"
            CopyFile(GetOutputDir() + "/bin" + pydll, SDK["PYTHON"] + pydll)

            for fn in glob.glob(SDK["PYTHON"] + "/vcruntime*.dll"):
                CopyFile(GetOutputDir() + "/bin/", fn)

            # Copy the whole Python directory.
            if COPY_PYTHON:
                CopyTree(GetOutputDir() + "/python", SDK["PYTHON"])

            # NB: Python does not always ship with the correct manifest/dll.
            # Figure out the correct one to ship, and grab it from WinSxS dir.
            manifest = GetOutputDir() + '/tmp/python.manifest'
            if os.path.isfile(manifest):
                os.unlink(manifest)
            oscmd('mt -inputresource:"%s\\python.exe";#1 -out:"%s" -nologo' % (SDK["PYTHON"], manifest), True)

            if COPY_PYTHON and os.path.isfile(manifest):
                import xml.etree.ElementTree as ET
                tree = ET.parse(manifest)
                idents = tree.findall('./{urn:schemas-microsoft-com:asm.v1}dependency/{urn:schemas-microsoft-com:asm.v1}dependentAssembly/{urn:schemas-microsoft-com:asm.v1}assemblyIdentity')
            else:
                idents = ()

            for ident in idents:
                sxs_name = '_'.join([
                    ident.get('processorArchitecture'),
                    ident.get('name').lower(),
                    ident.get('publicKeyToken'),
                    ident.get('version'),
                ])

                # Find the manifest matching these parameters.
                pattern = os.path.join('C:' + os.sep, 'Windows', 'WinSxS', 'Manifests', sxs_name + '_*.manifest')
                manifests = glob.glob(pattern)
                if not manifests:
                    Warn("Could not locate manifest %s.  You may need to reinstall the Visual C++ Redistributable." % (pattern))
                    continue

                CopyFile(GetOutputDir() + "/python/" + ident.get('name') + ".manifest", manifests[0])

                # Also copy the corresponding msvcr dll.
                pattern = os.path.join('C:' + os.sep, 'Windows', 'WinSxS', sxs_name + '_*', 'msvcr*.dll')
                for file in glob.glob(pattern):
                    CopyFile(GetOutputDir() + "/python/", file)

            # Copy python.exe to ppython.exe.
            if COPY_PYTHON:
                if not os.path.isfile(SDK["PYTHON"] + "/ppython.exe") and os.path.isfile(SDK["PYTHON"] + "/python.exe"):
                    CopyFile(GetOutputDir() + "/python/ppython.exe", SDK["PYTHON"] + "/python.exe")
                if not os.path.isfile(SDK["PYTHON"] + "/ppythonw.exe") and os.path.isfile(SDK["PYTHON"] + "/pythonw.exe"):
                    CopyFile(GetOutputDir() + "/python/ppythonw.exe", SDK["PYTHON"] + "/pythonw.exe")
                ConditionalWriteFile(GetOutputDir() + "/python/panda.pth", "..\n../bin\n")

# Copy over the MSVC runtime.
if GetTarget() == 'windows' and "VISUALSTUDIO" in SDK:
    vsver = "%s%s" % SDK["VISUALSTUDIO_VERSION"]
    vcver = "%s%s" % (SDK["MSVC_VERSION"][0], 0)        # ignore minor version.
    crtname = "Microsoft.VC%s.CRT" % (vsver)
    if ("VCTOOLSVERSION" in SDK):
        dir = os.path.join(SDK["VISUALSTUDIO"], "VC", "Redist", "MSVC", SDK["VCTOOLSVERSION"], "onecore", GetTargetArch(), crtname)
    else:
        dir = os.path.join(SDK["VISUALSTUDIO"], "VC", "redist", GetTargetArch(), crtname)

    if os.path.isfile(os.path.join(dir, "msvcr" + vcver + ".dll")):
        CopyFile(GetOutputDir() + "/bin/", os.path.join(dir, "msvcr" + vcver + ".dll"))
    if os.path.isfile(os.path.join(dir, "msvcp" + vcver + ".dll")):
        CopyFile(GetOutputDir() + "/bin/", os.path.join(dir, "msvcp" + vcver + ".dll"))
    if os.path.isfile(os.path.join(dir, "vcruntime" + vcver + ".dll")):
        CopyFile(GetOutputDir() + "/bin/", os.path.join(dir, "vcruntime" + vcver + ".dll"))

########################################################################
##
## Copy various stuff into the build.
##
########################################################################

if GetTarget() == 'windows':
    # Convert to Windows newlines so they can be opened by notepad.
    WriteFile(GetOutputDir() + "/LICENSE", ReadFile("doc/LICENSE"), newline='\r\n')
    WriteFile(GetOutputDir() + "/ReleaseNotes", ReadFile("doc/ReleaseNotes"), newline='\r\n')
    CopyFile(GetOutputDir() + "/pandaIcon.ico", "panda/src/configfiles/pandaIcon.ico")
else:
    CopyFile(GetOutputDir()+"/", "doc/LICENSE")
    CopyFile(GetOutputDir()+"/", "doc/ReleaseNotes")

if (PkgSkip("PANDATOOL")==0):
    CopyAllFiles(GetOutputDir()+"/plugins/",  "pandatool/src/scripts/", ".mel")
    CopyAllFiles(GetOutputDir()+"/plugins/",  "pandatool/src/scripts/", ".ms")
if (PkgSkip("PYTHON")==0 and os.path.isdir(GetThirdpartyBase()+"/Pmw")):
    CopyTree(GetOutputDir()+'/Pmw',         GetThirdpartyBase()+'/Pmw')
ConditionalWriteFile(GetOutputDir()+'/include/ctl3d.h', '/* dummy file to make MAX happy */')

# Since Eigen is included by all sorts of core headers, as a convenience
# to C++ users on Win and Mac, we include it in the Panda include directory.
if not PkgSkip("EIGEN") and GetTarget() in ("windows", "darwin") and GetThirdpartyDir():
    CopyTree(GetOutputDir()+'/include/Eigen', GetThirdpartyDir()+'eigen/include/Eigen')

########################################################################
#
# Copy header files to the built/include/parser-inc directory.
#
########################################################################

CopyTree(GetOutputDir()+'/include/parser-inc','dtool/src/parser-inc')
DeleteVCS(GetOutputDir()+'/include/parser-inc')

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
CopyAllHeaders('panda/src/linmath')
CopyAllHeaders('panda/src/putil')
CopyAllHeaders('dtool/src/prckeys')
CopyAllHeaders('panda/src/audio')
CopyAllHeaders('panda/src/event')
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
CopyAllHeaders('panda/src/display')
CopyAllHeaders('panda/src/chan')
CopyAllHeaders('panda/src/char')
CopyAllHeaders('panda/src/dgraph')
CopyAllHeaders('panda/src/device')
CopyAllHeaders('panda/src/pnmtext')
CopyAllHeaders('panda/src/text')
CopyAllHeaders('panda/src/grutil')
if (PkgSkip("VISION")==0):
    CopyAllHeaders('panda/src/vision')
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
CopyAllHeaders('panda/src/dxgsg9')
CopyAllHeaders('panda/metalibs/pandadx9')
if not PkgSkip("EGG"):
    CopyAllHeaders('panda/src/egg')
    CopyAllHeaders('panda/src/egg2pg')
CopyAllHeaders('panda/src/framework')
CopyAllHeaders('panda/metalibs/pandafx')
CopyAllHeaders('panda/src/glstuff')
CopyAllHeaders('panda/src/glgsg')
CopyAllHeaders('panda/src/glesgsg')
CopyAllHeaders('panda/src/gles2gsg')
if not PkgSkip("EGG"):
    CopyAllHeaders('panda/metalibs/pandaegg')
if GetTarget() == 'windows':
    CopyAllHeaders('panda/src/wgldisplay')
elif GetTarget() == 'darwin':
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
    'libp3eggbase.lib',
    'libp3progbase.lib',
    'libp3converter.lib',
    'libp3pandatoolbase.lib',
    'libpandaegg.dll',
] + COMMON_PANDA_LIBS

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
# DIRECTORY: dtool/metalibs/dtoolconfig/
#

OPTS=['DIR:dtool/metalibs/dtoolconfig', 'BUILDING:DTOOLCONFIG']
TargetAdd('p3dtoolconfig_dtoolconfig.obj', opts=OPTS, input='dtoolconfig.cxx')
TargetAdd('libp3dtoolconfig.dll', input='p3dtoolconfig_dtoolconfig.obj')
TargetAdd('libp3dtoolconfig.dll', input='p3prc_composite1.obj')
TargetAdd('libp3dtoolconfig.dll', input='p3prc_composite2.obj')
TargetAdd('libp3dtoolconfig.dll', input='libp3dtool.dll')
TargetAdd('libp3dtoolconfig.dll', opts=['ADVAPI', 'OPENSSL', 'WINGDI', 'WINUSER'])

#
# DIRECTORY: dtool/src/interrogatedb/
#

OPTS=['DIR:dtool/src/interrogatedb', 'BUILDING:INTERROGATEDB']
TargetAdd('p3interrogatedb_composite1.obj', opts=OPTS, input='p3interrogatedb_composite1.cxx')
TargetAdd('p3interrogatedb_composite2.obj', opts=OPTS, input='p3interrogatedb_composite2.cxx')
TargetAdd('libp3interrogatedb.dll', input='p3interrogatedb_composite1.obj')
TargetAdd('libp3interrogatedb.dll', input='p3interrogatedb_composite2.obj')
TargetAdd('libp3interrogatedb.dll', input='libp3dtool.dll')
TargetAdd('libp3interrogatedb.dll', input='libp3dtoolconfig.dll')

# This used to be called dtoolconfig.pyd, but it just contains the interrogatedb
# stuff, so it has been renamed appropriately.
OPTS=['DIR:dtool/metalibs/dtoolconfig']
PyTargetAdd('interrogatedb_pydtool.obj', opts=OPTS, input="pydtool.cxx")
PyTargetAdd('interrogatedb.pyd', input='interrogatedb_pydtool.obj')
PyTargetAdd('interrogatedb.pyd', input='libp3dtool.dll')
PyTargetAdd('interrogatedb.pyd', input='libp3dtoolconfig.dll')
PyTargetAdd('interrogatedb.pyd', input='libp3interrogatedb.dll')

#
# DIRECTORY: dtool/src/pystub/
#

if not RUNTIME and not RTDIST:
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
  TargetAdd('interrogate.exe', input=COMMON_DTOOL_LIBS)
  TargetAdd('interrogate.exe', input='libp3interrogatedb.dll')
  TargetAdd('interrogate.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL', 'WINGDI', 'WINUSER'])

  preamble = WriteEmbeddedStringFile('interrogate_preamble_python_native', inputs=[
    'dtool/src/interrogatedb/py_panda.cxx',
    'dtool/src/interrogatedb/py_compat.cxx',
    'dtool/src/interrogatedb/py_wrappers.cxx',
    'dtool/src/interrogatedb/dtool_super_base.cxx',
  ])
  TargetAdd('interrogate_module_preamble_python_native.obj', opts=OPTS, input=preamble)
  TargetAdd('interrogate_module_interrogate_module.obj', opts=OPTS, input='interrogate_module.cxx')
  TargetAdd('interrogate_module.exe', input='interrogate_module_interrogate_module.obj')
  TargetAdd('interrogate_module.exe', input='interrogate_module_preamble_python_native.obj')
  TargetAdd('interrogate_module.exe', input='libp3cppParser.ilb')
  TargetAdd('interrogate_module.exe', input=COMMON_DTOOL_LIBS)
  TargetAdd('interrogate_module.exe', input='libp3interrogatedb.dll')
  TargetAdd('interrogate_module.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL', 'WINGDI', 'WINUSER'])

  if (not RTDIST):
    TargetAdd('parse_file_parse_file.obj', opts=OPTS, input='parse_file.cxx')
    TargetAdd('parse_file.exe', input='parse_file_parse_file.obj')
    TargetAdd('parse_file.exe', input='libp3cppParser.ilb')
    TargetAdd('parse_file.exe', input=COMMON_DTOOL_LIBS)
    TargetAdd('parse_file.exe', input='libp3interrogatedb.dll')
    TargetAdd('parse_file.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL', 'WINGDI', 'WINUSER'])

#
# DIRECTORY: dtool/src/prckeys/
#

if (PkgSkip("OPENSSL")==0 and not RUNTIME and not RTDIST):
  OPTS=['DIR:dtool/src/prckeys', 'OPENSSL']
  TargetAdd('make-prc-key_makePrcKey.obj', opts=OPTS, input='makePrcKey.cxx')
  TargetAdd('make-prc-key.exe', input='make-prc-key_makePrcKey.obj')
  TargetAdd('make-prc-key.exe', input=COMMON_DTOOL_LIBS)
  TargetAdd('make-prc-key.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL', 'WINGDI', 'WINUSER'])

#
# DIRECTORY: dtool/src/test_interrogate/
#

if (not RTDIST and not RUNTIME):
  OPTS=['DIR:dtool/src/test_interrogate']
  TargetAdd('test_interrogate_test_interrogate.obj', opts=OPTS, input='test_interrogate.cxx')
  TargetAdd('test_interrogate.exe', input='test_interrogate_test_interrogate.obj')
  TargetAdd('test_interrogate.exe', input='libp3interrogatedb.dll')
  TargetAdd('test_interrogate.exe', input=COMMON_DTOOL_LIBS)
  TargetAdd('test_interrogate.exe', opts=['ADVAPI',  'OPENSSL', 'WINSHELL', 'WINGDI', 'WINUSER'])

#
# DIRECTORY: dtool/src/dtoolbase/
#

OPTS=['DIR:dtool/src/dtoolbase']
IGATEFILES=GetDirectoryContents('dtool/src/dtoolbase', ["*_composite*.cxx"])
IGATEFILES += [
    "typeHandle.h",
    "typeHandle_ext.h",
    "typeRegistry.h",
    "typedObject.h",
    "neverFreeMemory.h",
]
TargetAdd('libp3dtoolbase.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libp3dtoolbase.in', opts=['IMOD:panda3d.core', 'ILIB:libp3dtoolbase', 'SRCDIR:dtool/src/dtoolbase'])
PyTargetAdd('p3dtoolbase_typeHandle_ext.obj', opts=OPTS, input='typeHandle_ext.cxx')

#
# DIRECTORY: dtool/src/dtoolutil/
#

OPTS=['DIR:dtool/src/dtoolutil']
IGATEFILES=GetDirectoryContents('dtool/src/dtoolutil', ["*_composite*.cxx"])
IGATEFILES += [
    "config_dtoolutil.h",
    "pandaSystem.h",
    "dSearchPath.h",
    "executionEnvironment.h",
    "textEncoder.h",
    "textEncoder_ext.h",
    "filename.h",
    "filename_ext.h",
    "globPattern.h",
    "globPattern_ext.h",
    "pandaFileStream.h",
    "lineStream.h",
]
TargetAdd('libp3dtoolutil.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libp3dtoolutil.in', opts=['IMOD:panda3d.core', 'ILIB:libp3dtoolutil', 'SRCDIR:dtool/src/dtoolutil'])
PyTargetAdd('p3dtoolutil_ext_composite.obj', opts=OPTS, input='p3dtoolutil_ext_composite.cxx')

#
# DIRECTORY: dtool/src/prc/
#

OPTS=['DIR:dtool/src/prc']
IGATEFILES=GetDirectoryContents('dtool/src/prc', ["*.h", "*_composite*.cxx"])
TargetAdd('libp3prc.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libp3prc.in', opts=['IMOD:panda3d.core', 'ILIB:libp3prc', 'SRCDIR:dtool/src/prc'])
PyTargetAdd('p3prc_ext_composite.obj', opts=OPTS, input='p3prc_ext_composite.cxx')

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

OPTS=['DIR:panda/src/express', 'OPENSSL', 'ZLIB']
IGATEFILES=GetDirectoryContents('panda/src/express', ["*.h", "*_composite*.cxx"])
TargetAdd('libp3express.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libp3express.in', opts=['IMOD:panda3d.core', 'ILIB:libp3express', 'SRCDIR:panda/src/express'])
PyTargetAdd('p3express_ext_composite.obj', opts=OPTS, input='p3express_ext_composite.cxx')

#
# DIRECTORY: panda/src/downloader/
#

OPTS=['DIR:panda/src/downloader', 'BUILDING:PANDAEXPRESS', 'OPENSSL', 'ZLIB']
TargetAdd('p3downloader_composite1.obj', opts=OPTS, input='p3downloader_composite1.cxx')
TargetAdd('p3downloader_composite2.obj', opts=OPTS, input='p3downloader_composite2.cxx')

OPTS=['DIR:panda/src/downloader', 'OPENSSL', 'ZLIB']
IGATEFILES=GetDirectoryContents('panda/src/downloader', ["*.h", "*_composite*.cxx"])
TargetAdd('libp3downloader.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libp3downloader.in', opts=['IMOD:panda3d.core', 'ILIB:libp3downloader', 'SRCDIR:panda/src/downloader'])
PyTargetAdd('p3downloader_stringStream_ext.obj', opts=OPTS, input='stringStream_ext.cxx')

#
# DIRECTORY: panda/metalibs/pandaexpress/
#

OPTS=['DIR:panda/metalibs/pandaexpress', 'BUILDING:PANDAEXPRESS', 'ZLIB']
TargetAdd('pandaexpress_pandaexpress.obj', opts=OPTS, input='pandaexpress.cxx')
TargetAdd('libpandaexpress.dll', input='pandaexpress_pandaexpress.obj')
TargetAdd('libpandaexpress.dll', input='p3downloader_composite1.obj')
TargetAdd('libpandaexpress.dll', input='p3downloader_composite2.obj')
TargetAdd('libpandaexpress.dll', input='p3express_composite1.obj')
TargetAdd('libpandaexpress.dll', input='p3express_composite2.obj')
TargetAdd('libpandaexpress.dll', input='p3pandabase_pandabase.obj')
TargetAdd('libpandaexpress.dll', input=COMMON_DTOOL_LIBS)
TargetAdd('libpandaexpress.dll', opts=['ADVAPI', 'WINSOCK2',  'OPENSSL', 'ZLIB', 'WINGDI', 'WINUSER', 'ANDROID'])

#
# DIRECTORY: panda/src/pipeline/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pipeline', 'BUILDING:PANDA']
  TargetAdd('p3pipeline_composite1.obj', opts=OPTS, input='p3pipeline_composite1.cxx')
  TargetAdd('p3pipeline_composite2.obj', opts=OPTS, input='p3pipeline_composite2.cxx')
  TargetAdd('p3pipeline_contextSwitch.obj', opts=OPTS, input='contextSwitch.c')

  OPTS=['DIR:panda/src/pipeline']
  IGATEFILES=GetDirectoryContents('panda/src/pipeline', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3pipeline.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pipeline.in', opts=['IMOD:panda3d.core', 'ILIB:libp3pipeline', 'SRCDIR:panda/src/pipeline'])
  PyTargetAdd('p3pipeline_pythonThread.obj', opts=OPTS, input='pythonThread.cxx')

#
# DIRECTORY: panda/src/linmath/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/linmath', 'BUILDING:PANDA']
  TargetAdd('p3linmath_composite1.obj', opts=OPTS, input='p3linmath_composite1.cxx')
  TargetAdd('p3linmath_composite2.obj', opts=OPTS, input='p3linmath_composite2.cxx')

  OPTS=['DIR:panda/src/linmath']
  IGATEFILES=GetDirectoryContents('panda/src/linmath', ["*.h", "*_composite*.cxx"])
  for ifile in IGATEFILES[:]:
      if "_src." in ifile:
          IGATEFILES.remove(ifile)
  IGATEFILES.remove('cast_to_double.h')
  IGATEFILES.remove('lmat_ops.h')
  IGATEFILES.remove('cast_to_float.h')
  TargetAdd('libp3linmath.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3linmath.in', opts=['IMOD:panda3d.core', 'ILIB:libp3linmath', 'SRCDIR:panda/src/linmath'])

#
# DIRECTORY: panda/src/putil/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/putil', 'BUILDING:PANDA',  'ZLIB']
  TargetAdd('p3putil_composite1.obj', opts=OPTS, input='p3putil_composite1.cxx')
  TargetAdd('p3putil_composite2.obj', opts=OPTS, input='p3putil_composite2.cxx')

  OPTS=['DIR:panda/src/putil', 'ZLIB']
  IGATEFILES=GetDirectoryContents('panda/src/putil', ["*.h", "*_composite*.cxx"])
  IGATEFILES.remove("test_bam.h")
  IGATEFILES.remove("config_util.h")
  TargetAdd('libp3putil.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3putil.in', opts=['IMOD:panda3d.core', 'ILIB:libp3putil', 'SRCDIR:panda/src/putil'])
  PyTargetAdd('p3putil_ext_composite.obj', opts=OPTS, input='p3putil_ext_composite.cxx')

#
# DIRECTORY: panda/src/audio/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/audio', 'BUILDING:PANDA']
  TargetAdd('p3audio_composite1.obj', opts=OPTS, input='p3audio_composite1.cxx')

  OPTS=['DIR:panda/src/audio']
  IGATEFILES=["audio.h"]
  TargetAdd('libp3audio.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3audio.in', opts=['IMOD:panda3d.core', 'ILIB:libp3audio', 'SRCDIR:panda/src/audio'])

#
# DIRECTORY: panda/src/event/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/event', 'BUILDING:PANDA']
  TargetAdd('p3event_composite1.obj', opts=OPTS, input='p3event_composite1.cxx')
  TargetAdd('p3event_composite2.obj', opts=OPTS, input='p3event_composite2.cxx')

  OPTS=['DIR:panda/src/event']
  PyTargetAdd('p3event_asyncFuture_ext.obj', opts=OPTS, input='asyncFuture_ext.cxx')
  PyTargetAdd('p3event_pythonTask.obj', opts=OPTS, input='pythonTask.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/event', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3event.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3event.in', opts=['IMOD:panda3d.core', 'ILIB:libp3event', 'SRCDIR:panda/src/event'])

#
# DIRECTORY: panda/src/mathutil/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/mathutil', 'BUILDING:PANDA', 'FFTW']
  TargetAdd('p3mathutil_composite1.obj', opts=OPTS, input='p3mathutil_composite1.cxx')
  TargetAdd('p3mathutil_composite2.obj', opts=OPTS, input='p3mathutil_composite2.cxx')

  OPTS=['DIR:panda/src/mathutil', 'FFTW']
  IGATEFILES=GetDirectoryContents('panda/src/mathutil', ["*.h", "*_composite*.cxx"])
  for ifile in IGATEFILES[:]:
      if "_src." in ifile:
          IGATEFILES.remove(ifile)
  TargetAdd('libp3mathutil.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3mathutil.in', opts=['IMOD:panda3d.core', 'ILIB:libp3mathutil', 'SRCDIR:panda/src/mathutil'])

#
# DIRECTORY: panda/src/gsgbase/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/gsgbase', 'BUILDING:PANDA']
  TargetAdd('p3gsgbase_composite1.obj', opts=OPTS, input='p3gsgbase_composite1.cxx')

  OPTS=['DIR:panda/src/gsgbase']
  IGATEFILES=GetDirectoryContents('panda/src/gsgbase', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3gsgbase.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3gsgbase.in', opts=['IMOD:panda3d.core', 'ILIB:libp3gsgbase', 'SRCDIR:panda/src/gsgbase'])

#
# DIRECTORY: panda/src/pnmimage/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pnmimage', 'BUILDING:PANDA',  'ZLIB']
  TargetAdd('p3pnmimage_composite1.obj', opts=OPTS, input='p3pnmimage_composite1.cxx')
  TargetAdd('p3pnmimage_composite2.obj', opts=OPTS, input='p3pnmimage_composite2.cxx')
  TargetAdd('p3pnmimage_convert_srgb_sse2.obj', opts=OPTS+['SSE2'], input='convert_srgb_sse2.cxx')

  OPTS=['DIR:panda/src/pnmimage', 'ZLIB']
  IGATEFILES=GetDirectoryContents('panda/src/pnmimage', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3pnmimage.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pnmimage.in', opts=['IMOD:panda3d.core', 'ILIB:libp3pnmimage', 'SRCDIR:panda/src/pnmimage'])
  PyTargetAdd('p3pnmimage_pfmFile_ext.obj', opts=OPTS, input='pfmFile_ext.cxx')

#
# DIRECTORY: panda/src/nativenet/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/nativenet', 'OPENSSL', 'BUILDING:PANDA']
  TargetAdd('p3nativenet_composite1.obj', opts=OPTS, input='p3nativenet_composite1.cxx')

  OPTS=['DIR:panda/src/nativenet', 'OPENSSL']
  IGATEFILES=GetDirectoryContents('panda/src/nativenet', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3nativenet.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3nativenet.in', opts=['IMOD:panda3d.core', 'ILIB:libp3nativenet', 'SRCDIR:panda/src/nativenet'])

#
# DIRECTORY: panda/src/net/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/net', 'BUILDING:PANDA']
  TargetAdd('p3net_composite1.obj', opts=OPTS, input='p3net_composite1.cxx')
  TargetAdd('p3net_composite2.obj', opts=OPTS, input='p3net_composite2.cxx')

  OPTS=['DIR:panda/src/net']
  IGATEFILES=GetDirectoryContents('panda/src/net', ["*.h", "*_composite*.cxx"])
  IGATEFILES.remove("datagram_ui.h")
  TargetAdd('libp3net.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3net.in', opts=['IMOD:panda3d.core', 'ILIB:libp3net', 'SRCDIR:panda/src/net'])

#
# DIRECTORY: panda/src/pstatclient/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pstatclient', 'BUILDING:PANDA']
  TargetAdd('p3pstatclient_composite1.obj', opts=OPTS, input='p3pstatclient_composite1.cxx')
  TargetAdd('p3pstatclient_composite2.obj', opts=OPTS, input='p3pstatclient_composite2.cxx')

  OPTS=['DIR:panda/src/pstatclient']
  IGATEFILES=GetDirectoryContents('panda/src/pstatclient', ["*.h", "*_composite*.cxx"])
  IGATEFILES.remove("config_pstats.h")
  TargetAdd('libp3pstatclient.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pstatclient.in', opts=['IMOD:panda3d.core', 'ILIB:libp3pstatclient', 'SRCDIR:panda/src/pstatclient'])

#
# DIRECTORY: panda/src/gobj/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/gobj', 'BUILDING:PANDA',  'NVIDIACG', 'ZLIB', 'SQUISH']
  TargetAdd('p3gobj_composite1.obj', opts=OPTS, input='p3gobj_composite1.cxx')
  TargetAdd('p3gobj_composite2.obj', opts=OPTS+['BIGOBJ'], input='p3gobj_composite2.cxx')

  OPTS=['DIR:panda/src/gobj', 'NVIDIACG', 'ZLIB', 'SQUISH']
  IGATEFILES=GetDirectoryContents('panda/src/gobj', ["*.h", "*_composite*.cxx"])
  if ("cgfx_states.h" in IGATEFILES): IGATEFILES.remove("cgfx_states.h")
  TargetAdd('libp3gobj.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3gobj.in', opts=['IMOD:panda3d.core', 'ILIB:libp3gobj', 'SRCDIR:panda/src/gobj'])
  PyTargetAdd('p3gobj_ext_composite.obj', opts=OPTS, input='p3gobj_ext_composite.cxx')

#
# DIRECTORY: panda/src/pgraphnodes/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pgraphnodes', 'BUILDING:PANDA']
  TargetAdd('p3pgraphnodes_composite1.obj', opts=OPTS, input='p3pgraphnodes_composite1.cxx')
  TargetAdd('p3pgraphnodes_composite2.obj', opts=OPTS, input='p3pgraphnodes_composite2.cxx')

  OPTS=['DIR:panda/src/pgraphnodes']
  IGATEFILES=GetDirectoryContents('panda/src/pgraphnodes', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3pgraphnodes.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pgraphnodes.in', opts=['IMOD:panda3d.core', 'ILIB:libp3pgraphnodes', 'SRCDIR:panda/src/pgraphnodes'])

#
# DIRECTORY: panda/src/pgraph/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pgraph', 'BUILDING:PANDA']
  TargetAdd('p3pgraph_nodePath.obj', opts=OPTS, input='nodePath.cxx')
  TargetAdd('p3pgraph_composite1.obj', opts=OPTS, input='p3pgraph_composite1.cxx')
  TargetAdd('p3pgraph_composite2.obj', opts=OPTS, input='p3pgraph_composite2.cxx')
  TargetAdd('p3pgraph_composite3.obj', opts=OPTS, input='p3pgraph_composite3.cxx')
  TargetAdd('p3pgraph_composite4.obj', opts=OPTS, input='p3pgraph_composite4.cxx')

  OPTS=['DIR:panda/src/pgraph']
  IGATEFILES=GetDirectoryContents('panda/src/pgraph', ["*.h", "nodePath.cxx", "*_composite*.cxx"])
  TargetAdd('libp3pgraph.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pgraph.in', opts=['IMOD:panda3d.core', 'ILIB:libp3pgraph', 'SRCDIR:panda/src/pgraph'])
  PyTargetAdd('p3pgraph_ext_composite.obj', opts=OPTS, input='p3pgraph_ext_composite.cxx')

#
# DIRECTORY: panda/src/cull/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/cull', 'BUILDING:PANDA']
  TargetAdd('p3cull_composite1.obj', opts=OPTS, input='p3cull_composite1.cxx')
  TargetAdd('p3cull_composite2.obj', opts=OPTS, input='p3cull_composite2.cxx')

  OPTS=['DIR:panda/src/cull']
  IGATEFILES=GetDirectoryContents('panda/src/cull', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3cull.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3cull.in', opts=['IMOD:panda3d.core', 'ILIB:libp3cull', 'SRCDIR:panda/src/cull'])

#
# DIRECTORY: panda/src/dgraph/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/dgraph', 'BUILDING:PANDA']
  TargetAdd('p3dgraph_composite1.obj', opts=OPTS, input='p3dgraph_composite1.cxx')
  TargetAdd('p3dgraph_composite2.obj', opts=OPTS, input='p3dgraph_composite2.cxx')

  OPTS=['DIR:panda/src/dgraph']
  IGATEFILES=GetDirectoryContents('panda/src/dgraph', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3dgraph.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3dgraph.in', opts=['IMOD:panda3d.core', 'ILIB:libp3dgraph', 'SRCDIR:panda/src/dgraph'])

#
# DIRECTORY: panda/src/device/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/device', 'BUILDING:PANDA']
  TargetAdd('p3device_composite1.obj', opts=OPTS, input='p3device_composite1.cxx')
  TargetAdd('p3device_composite2.obj', opts=OPTS, input='p3device_composite2.cxx')

  OPTS=['DIR:panda/src/device']
  IGATEFILES=GetDirectoryContents('panda/src/device', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3device.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3device.in', opts=['IMOD:panda3d.core', 'ILIB:libp3device', 'SRCDIR:panda/src/device'])

#
# DIRECTORY: panda/src/display/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/display', 'BUILDING:PANDA', 'X11']
  TargetAdd('p3display_graphicsStateGuardian.obj', opts=OPTS, input='graphicsStateGuardian.cxx')
  TargetAdd('p3display_composite1.obj', opts=OPTS, input='p3display_composite1.cxx')
  TargetAdd('p3display_composite2.obj', opts=OPTS, input='p3display_composite2.cxx')

  OPTS=['DIR:panda/src/display', 'X11']
  IGATEFILES=GetDirectoryContents('panda/src/display', ["*.h", "*_composite*.cxx"])
  IGATEFILES.remove("renderBuffer.h")
  TargetAdd('libp3display.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3display.in', opts=['IMOD:panda3d.core', 'ILIB:libp3display', 'SRCDIR:panda/src/display'])
  PyTargetAdd('p3display_ext_composite.obj', opts=OPTS, input='p3display_ext_composite.cxx')

  if RTDIST and GetTarget() == 'darwin':
    OPTS=['DIR:panda/src/display']
    TargetAdd('subprocessWindowBuffer.obj', opts=OPTS, input='subprocessWindowBuffer.cxx')
    TargetAdd('libp3subprocbuffer.ilb', input='subprocessWindowBuffer.obj')

#
# DIRECTORY: panda/src/chan/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/chan', 'BUILDING:PANDA']
  TargetAdd('p3chan_composite1.obj', opts=OPTS, input='p3chan_composite1.cxx')
  TargetAdd('p3chan_composite2.obj', opts=OPTS, input='p3chan_composite2.cxx')

  OPTS=['DIR:panda/src/chan']
  IGATEFILES=GetDirectoryContents('panda/src/chan', ["*.h", "*_composite*.cxx"])
  IGATEFILES.remove('movingPart.h')
  IGATEFILES.remove('animChannelFixed.h')
  TargetAdd('libp3chan.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3chan.in', opts=['IMOD:panda3d.core', 'ILIB:libp3chan', 'SRCDIR:panda/src/chan'])


# DIRECTORY: panda/src/char/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/char', 'BUILDING:PANDA']
  TargetAdd('p3char_composite1.obj', opts=OPTS, input='p3char_composite1.cxx')
  TargetAdd('p3char_composite2.obj', opts=OPTS, input='p3char_composite2.cxx')

  OPTS=['DIR:panda/src/char']
  IGATEFILES=GetDirectoryContents('panda/src/char', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3char.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3char.in', opts=['IMOD:panda3d.core', 'ILIB:libp3char', 'SRCDIR:panda/src/char'])

#
# DIRECTORY: panda/src/pnmtext/
#

if (PkgSkip("FREETYPE")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/pnmtext', 'BUILDING:PANDA',  'FREETYPE']
  TargetAdd('p3pnmtext_composite1.obj', opts=OPTS, input='p3pnmtext_composite1.cxx')

  OPTS=['DIR:panda/src/pnmtext', 'FREETYPE']
  IGATEFILES=GetDirectoryContents('panda/src/pnmtext', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3pnmtext.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pnmtext.in', opts=['IMOD:panda3d.core', 'ILIB:libp3pnmtext', 'SRCDIR:panda/src/pnmtext'])

#
# DIRECTORY: panda/src/text/
#

if (not RUNTIME):
  if not PkgSkip("HARFBUZZ"):
    DefSymbol("HARFBUZZ", "HAVE_HARFBUZZ")

  OPTS=['DIR:panda/src/text', 'BUILDING:PANDA', 'ZLIB',  'FREETYPE', 'HARFBUZZ']
  TargetAdd('p3text_composite1.obj', opts=OPTS, input='p3text_composite1.cxx')
  TargetAdd('p3text_composite2.obj', opts=OPTS, input='p3text_composite2.cxx')

  OPTS=['DIR:panda/src/text', 'ZLIB', 'FREETYPE']
  IGATEFILES=GetDirectoryContents('panda/src/text', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3text.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3text.in', opts=['IMOD:panda3d.core', 'ILIB:libp3text', 'SRCDIR:panda/src/text'])

#
# DIRECTORY: panda/src/movies/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/movies', 'BUILDING:PANDA', 'VORBIS', 'OPUS']
  TargetAdd('p3movies_composite1.obj', opts=OPTS, input='p3movies_composite1.cxx')

  OPTS=['DIR:panda/src/movies', 'VORBIS', 'OPUS']
  IGATEFILES=GetDirectoryContents('panda/src/movies', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3movies.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3movies.in', opts=['IMOD:panda3d.core', 'ILIB:libp3movies', 'SRCDIR:panda/src/movies'])

#
# DIRECTORY: panda/src/grutil/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/grutil', 'BUILDING:PANDA']
  TargetAdd('p3grutil_multitexReducer.obj', opts=OPTS, input='multitexReducer.cxx')
  TargetAdd('p3grutil_composite1.obj', opts=OPTS, input='p3grutil_composite1.cxx')
  TargetAdd('p3grutil_composite2.obj', opts=OPTS, input='p3grutil_composite2.cxx')

  OPTS=['DIR:panda/src/grutil']
  IGATEFILES=GetDirectoryContents('panda/src/grutil', ["*.h", "*_composite*.cxx"])
  if 'convexHull.h' in IGATEFILES: IGATEFILES.remove('convexHull.h')
  TargetAdd('libp3grutil.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3grutil.in', opts=['IMOD:panda3d.core', 'ILIB:libp3grutil', 'SRCDIR:panda/src/grutil'])

#
# DIRECTORY: panda/src/tform/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/tform', 'BUILDING:PANDA']
  TargetAdd('p3tform_composite1.obj', opts=OPTS, input='p3tform_composite1.cxx')
  TargetAdd('p3tform_composite2.obj', opts=OPTS, input='p3tform_composite2.cxx')

  OPTS=['DIR:panda/src/tform']
  IGATEFILES=GetDirectoryContents('panda/src/tform', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3tform.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3tform.in', opts=['IMOD:panda3d.core', 'ILIB:libp3tform', 'SRCDIR:panda/src/tform'])

#
# DIRECTORY: panda/src/collide/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/collide', 'BUILDING:PANDA']
  TargetAdd('p3collide_composite1.obj', opts=OPTS, input='p3collide_composite1.cxx')
  TargetAdd('p3collide_composite2.obj', opts=OPTS, input='p3collide_composite2.cxx')

  OPTS=['DIR:panda/src/collide']
  IGATEFILES=GetDirectoryContents('panda/src/collide', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3collide.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3collide.in', opts=['IMOD:panda3d.core', 'ILIB:libp3collide', 'SRCDIR:panda/src/collide'])

#
# DIRECTORY: panda/src/parametrics/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/parametrics', 'BUILDING:PANDA']
  TargetAdd('p3parametrics_composite1.obj', opts=OPTS, input='p3parametrics_composite1.cxx')
  TargetAdd('p3parametrics_composite2.obj', opts=OPTS, input='p3parametrics_composite2.cxx')

  OPTS=['DIR:panda/src/parametrics']
  IGATEFILES=GetDirectoryContents('panda/src/parametrics', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3parametrics.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3parametrics.in', opts=['IMOD:panda3d.core', 'ILIB:libp3parametrics', 'SRCDIR:panda/src/parametrics'])

#
# DIRECTORY: panda/src/pgui/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pgui', 'BUILDING:PANDA']
  TargetAdd('p3pgui_composite1.obj', opts=OPTS, input='p3pgui_composite1.cxx')
  TargetAdd('p3pgui_composite2.obj', opts=OPTS, input='p3pgui_composite2.cxx')

  OPTS=['DIR:panda/src/pgui']
  IGATEFILES=GetDirectoryContents('panda/src/pgui', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3pgui.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3pgui.in', opts=['IMOD:panda3d.core', 'ILIB:libp3pgui', 'SRCDIR:panda/src/pgui'])

#
# DIRECTORY: panda/src/pnmimagetypes/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/pnmimagetypes', 'DIR:panda/src/pnmimage', 'BUILDING:PANDA', 'PNG', 'ZLIB', 'JPEG', 'TIFF', 'OPENEXR', 'EXCEPTIONS']
  TargetAdd('p3pnmimagetypes_composite1.obj', opts=OPTS, input='p3pnmimagetypes_composite1.cxx')
  TargetAdd('p3pnmimagetypes_composite2.obj', opts=OPTS, input='p3pnmimagetypes_composite2.cxx')

#
# DIRECTORY: panda/src/recorder/
#

if (not RUNTIME):
  OPTS=['DIR:panda/src/recorder', 'BUILDING:PANDA']
  TargetAdd('p3recorder_composite1.obj', opts=OPTS, input='p3recorder_composite1.cxx')
  TargetAdd('p3recorder_composite2.obj', opts=OPTS, input='p3recorder_composite2.cxx')

  OPTS=['DIR:panda/src/recorder']
  IGATEFILES=GetDirectoryContents('panda/src/recorder', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3recorder.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3recorder.in', opts=['IMOD:panda3d.core', 'ILIB:libp3recorder', 'SRCDIR:panda/src/recorder'])

#
# DIRECTORY: panda/src/dxml/
#

DefSymbol("TINYXML", "TIXML_USE_STL", "")

OPTS=['DIR:panda/src/dxml', 'TINYXML']
TargetAdd('tinyxml_composite1.obj', opts=OPTS, input='tinyxml_composite1.cxx')
TargetAdd('libp3tinyxml.ilb', input='tinyxml_composite1.obj')

if (not RUNTIME):
  OPTS=['DIR:panda/src/dxml', 'BUILDING:PANDA', 'TINYXML']
  TargetAdd('p3dxml_composite1.obj', opts=OPTS, input='p3dxml_composite1.cxx')

  OPTS=['DIR:panda/src/dxml', 'TINYXML']
  IGATEFILES=GetDirectoryContents('panda/src/dxml', ["*.h", "p3dxml_composite1.cxx"])
  TargetAdd('libp3dxml.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3dxml.in', opts=['IMOD:panda3d.core', 'ILIB:libp3dxml', 'SRCDIR:panda/src/dxml'])

#
# DIRECTORY: panda/metalibs/panda/
#

if (not RUNTIME):
  OPTS=['DIR:panda/metalibs/panda', 'BUILDING:PANDA', 'JPEG', 'PNG', 'HARFBUZZ',
      'TIFF', 'OPENEXR', 'ZLIB', 'OPENSSL', 'FREETYPE', 'FFTW', 'ADVAPI', 'WINSOCK2',
      'SQUISH', 'NVIDIACG', 'VORBIS', 'OPUS', 'WINUSER', 'WINMM', 'WINGDI', 'IPHLPAPI',
      'SETUPAPI']

  TargetAdd('panda_panda.obj', opts=OPTS, input='panda.cxx')

  TargetAdd('libpanda.dll', input='panda_panda.obj')
  TargetAdd('libpanda.dll', input='p3recorder_composite1.obj')
  TargetAdd('libpanda.dll', input='p3recorder_composite2.obj')
  TargetAdd('libpanda.dll', input='p3pgraphnodes_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pgraphnodes_composite2.obj')
  TargetAdd('libpanda.dll', input='p3pgraph_nodePath.obj')
  TargetAdd('libpanda.dll', input='p3pgraph_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pgraph_composite2.obj')
  TargetAdd('libpanda.dll', input='p3pgraph_composite3.obj')
  TargetAdd('libpanda.dll', input='p3pgraph_composite4.obj')
  TargetAdd('libpanda.dll', input='p3cull_composite1.obj')
  TargetAdd('libpanda.dll', input='p3cull_composite2.obj')
  TargetAdd('libpanda.dll', input='p3movies_composite1.obj')
  TargetAdd('libpanda.dll', input='p3grutil_multitexReducer.obj')
  TargetAdd('libpanda.dll', input='p3grutil_composite1.obj')
  TargetAdd('libpanda.dll', input='p3grutil_composite2.obj')
  TargetAdd('libpanda.dll', input='p3chan_composite1.obj')
  TargetAdd('libpanda.dll', input='p3chan_composite2.obj')
  TargetAdd('libpanda.dll', input='p3pstatclient_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pstatclient_composite2.obj')
  TargetAdd('libpanda.dll', input='p3char_composite1.obj')
  TargetAdd('libpanda.dll', input='p3char_composite2.obj')
  TargetAdd('libpanda.dll', input='p3collide_composite1.obj')
  TargetAdd('libpanda.dll', input='p3collide_composite2.obj')
  TargetAdd('libpanda.dll', input='p3device_composite1.obj')
  TargetAdd('libpanda.dll', input='p3device_composite2.obj')
  TargetAdd('libpanda.dll', input='p3dgraph_composite1.obj')
  TargetAdd('libpanda.dll', input='p3dgraph_composite2.obj')
  TargetAdd('libpanda.dll', input='p3display_graphicsStateGuardian.obj')
  TargetAdd('libpanda.dll', input='p3display_composite1.obj')
  TargetAdd('libpanda.dll', input='p3display_composite2.obj')
  TargetAdd('libpanda.dll', input='p3pipeline_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pipeline_composite2.obj')
  TargetAdd('libpanda.dll', input='p3pipeline_contextSwitch.obj')
  TargetAdd('libpanda.dll', input='p3event_composite1.obj')
  TargetAdd('libpanda.dll', input='p3event_composite2.obj')
  TargetAdd('libpanda.dll', input='p3gobj_composite1.obj')
  TargetAdd('libpanda.dll', input='p3gobj_composite2.obj')
  TargetAdd('libpanda.dll', input='p3gsgbase_composite1.obj')
  TargetAdd('libpanda.dll', input='p3linmath_composite1.obj')
  TargetAdd('libpanda.dll', input='p3linmath_composite2.obj')
  TargetAdd('libpanda.dll', input='p3mathutil_composite1.obj')
  TargetAdd('libpanda.dll', input='p3mathutil_composite2.obj')
  TargetAdd('libpanda.dll', input='p3parametrics_composite1.obj')
  TargetAdd('libpanda.dll', input='p3parametrics_composite2.obj')
  TargetAdd('libpanda.dll', input='p3pnmimagetypes_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pnmimagetypes_composite2.obj')
  TargetAdd('libpanda.dll', input='p3pnmimage_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pnmimage_composite2.obj')
  TargetAdd('libpanda.dll', input='p3pnmimage_convert_srgb_sse2.obj')
  TargetAdd('libpanda.dll', input='p3text_composite1.obj')
  TargetAdd('libpanda.dll', input='p3text_composite2.obj')
  TargetAdd('libpanda.dll', input='p3tform_composite1.obj')
  TargetAdd('libpanda.dll', input='p3tform_composite2.obj')
  TargetAdd('libpanda.dll', input='p3putil_composite1.obj')
  TargetAdd('libpanda.dll', input='p3putil_composite2.obj')
  TargetAdd('libpanda.dll', input='p3audio_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pgui_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pgui_composite2.obj')
  TargetAdd('libpanda.dll', input='p3net_composite1.obj')
  TargetAdd('libpanda.dll', input='p3net_composite2.obj')
  TargetAdd('libpanda.dll', input='p3nativenet_composite1.obj')
  TargetAdd('libpanda.dll', input='p3pandabase_pandabase.obj')
  TargetAdd('libpanda.dll', input='libpandaexpress.dll')
  TargetAdd('libpanda.dll', input='p3dxml_composite1.obj')
  TargetAdd('libpanda.dll', input='libp3dtoolconfig.dll')
  TargetAdd('libpanda.dll', input='libp3dtool.dll')

  if PkgSkip("FREETYPE")==0:
    TargetAdd('libpanda.dll', input="p3pnmtext_composite1.obj")

  TargetAdd('libpanda.dll', dep='dtool_have_freetype.dat')
  TargetAdd('libpanda.dll', opts=OPTS)

  PyTargetAdd('core_module.obj', input='libp3dtoolbase.in')
  PyTargetAdd('core_module.obj', input='libp3dtoolutil.in')
  PyTargetAdd('core_module.obj', input='libp3prc.in')

  PyTargetAdd('core_module.obj', input='libp3downloader.in')
  PyTargetAdd('core_module.obj', input='libp3express.in')

  PyTargetAdd('core_module.obj', input='libp3recorder.in')
  PyTargetAdd('core_module.obj', input='libp3pgraphnodes.in')
  PyTargetAdd('core_module.obj', input='libp3pgraph.in')
  PyTargetAdd('core_module.obj', input='libp3cull.in')
  PyTargetAdd('core_module.obj', input='libp3grutil.in')
  PyTargetAdd('core_module.obj', input='libp3chan.in')
  PyTargetAdd('core_module.obj', input='libp3pstatclient.in')
  PyTargetAdd('core_module.obj', input='libp3char.in')
  PyTargetAdd('core_module.obj', input='libp3collide.in')
  PyTargetAdd('core_module.obj', input='libp3device.in')
  PyTargetAdd('core_module.obj', input='libp3dgraph.in')
  PyTargetAdd('core_module.obj', input='libp3display.in')
  PyTargetAdd('core_module.obj', input='libp3pipeline.in')
  PyTargetAdd('core_module.obj', input='libp3event.in')
  PyTargetAdd('core_module.obj', input='libp3gobj.in')
  PyTargetAdd('core_module.obj', input='libp3gsgbase.in')
  PyTargetAdd('core_module.obj', input='libp3linmath.in')
  PyTargetAdd('core_module.obj', input='libp3mathutil.in')
  PyTargetAdd('core_module.obj', input='libp3parametrics.in')
  PyTargetAdd('core_module.obj', input='libp3pnmimage.in')
  PyTargetAdd('core_module.obj', input='libp3text.in')
  PyTargetAdd('core_module.obj', input='libp3tform.in')
  PyTargetAdd('core_module.obj', input='libp3putil.in')
  PyTargetAdd('core_module.obj', input='libp3audio.in')
  PyTargetAdd('core_module.obj', input='libp3nativenet.in')
  PyTargetAdd('core_module.obj', input='libp3net.in')
  PyTargetAdd('core_module.obj', input='libp3pgui.in')
  PyTargetAdd('core_module.obj', input='libp3movies.in')
  PyTargetAdd('core_module.obj', input='libp3dxml.in')

  if PkgSkip("FREETYPE")==0:
    PyTargetAdd('core_module.obj', input='libp3pnmtext.in')

  PyTargetAdd('core_module.obj', opts=['IMOD:panda3d.core', 'ILIB:core'])

  PyTargetAdd('core.pyd', input='libp3dtoolbase_igate.obj')
  PyTargetAdd('core.pyd', input='p3dtoolbase_typeHandle_ext.obj')
  PyTargetAdd('core.pyd', input='libp3dtoolutil_igate.obj')
  PyTargetAdd('core.pyd', input='p3dtoolutil_ext_composite.obj')
  PyTargetAdd('core.pyd', input='libp3prc_igate.obj')
  PyTargetAdd('core.pyd', input='p3prc_ext_composite.obj')

  PyTargetAdd('core.pyd', input='libp3downloader_igate.obj')
  PyTargetAdd('core.pyd', input='p3downloader_stringStream_ext.obj')
  PyTargetAdd('core.pyd', input='p3express_ext_composite.obj')
  PyTargetAdd('core.pyd', input='libp3express_igate.obj')

  PyTargetAdd('core.pyd', input='libp3recorder_igate.obj')
  PyTargetAdd('core.pyd', input='libp3pgraphnodes_igate.obj')
  PyTargetAdd('core.pyd', input='libp3pgraph_igate.obj')
  PyTargetAdd('core.pyd', input='libp3movies_igate.obj')
  PyTargetAdd('core.pyd', input='libp3grutil_igate.obj')
  PyTargetAdd('core.pyd', input='libp3chan_igate.obj')
  PyTargetAdd('core.pyd', input='libp3pstatclient_igate.obj')
  PyTargetAdd('core.pyd', input='libp3char_igate.obj')
  PyTargetAdd('core.pyd', input='libp3collide_igate.obj')
  PyTargetAdd('core.pyd', input='libp3device_igate.obj')
  PyTargetAdd('core.pyd', input='libp3dgraph_igate.obj')
  PyTargetAdd('core.pyd', input='libp3display_igate.obj')
  PyTargetAdd('core.pyd', input='libp3pipeline_igate.obj')
  PyTargetAdd('core.pyd', input='libp3event_igate.obj')
  PyTargetAdd('core.pyd', input='libp3gobj_igate.obj')
  PyTargetAdd('core.pyd', input='libp3gsgbase_igate.obj')
  PyTargetAdd('core.pyd', input='libp3linmath_igate.obj')
  PyTargetAdd('core.pyd', input='libp3mathutil_igate.obj')
  PyTargetAdd('core.pyd', input='libp3parametrics_igate.obj')
  PyTargetAdd('core.pyd', input='libp3pnmimage_igate.obj')
  PyTargetAdd('core.pyd', input='libp3text_igate.obj')
  PyTargetAdd('core.pyd', input='libp3tform_igate.obj')
  PyTargetAdd('core.pyd', input='libp3putil_igate.obj')
  PyTargetAdd('core.pyd', input='libp3audio_igate.obj')
  PyTargetAdd('core.pyd', input='libp3pgui_igate.obj')
  PyTargetAdd('core.pyd', input='libp3net_igate.obj')
  PyTargetAdd('core.pyd', input='libp3nativenet_igate.obj')
  PyTargetAdd('core.pyd', input='libp3dxml_igate.obj')

  if PkgSkip("FREETYPE")==0:
    PyTargetAdd('core.pyd', input="libp3pnmtext_igate.obj")

  PyTargetAdd('core.pyd', input='p3pipeline_pythonThread.obj')
  PyTargetAdd('core.pyd', input='p3putil_ext_composite.obj')
  PyTargetAdd('core.pyd', input='p3pnmimage_pfmFile_ext.obj')
  PyTargetAdd('core.pyd', input='p3event_asyncFuture_ext.obj')
  PyTargetAdd('core.pyd', input='p3event_pythonTask.obj')
  PyTargetAdd('core.pyd', input='p3gobj_ext_composite.obj')
  PyTargetAdd('core.pyd', input='p3pgraph_ext_composite.obj')
  PyTargetAdd('core.pyd', input='p3display_ext_composite.obj')

  PyTargetAdd('core.pyd', input='core_module.obj')
  if not GetLinkAllStatic() and GetTarget() != 'emscripten':
     PyTargetAdd('core.pyd', input='libp3tinyxml.ilb')
  PyTargetAdd('core.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('core.pyd', input=COMMON_PANDA_LIBS)
  PyTargetAdd('core.pyd', opts=['WINSOCK2'])

#
# DIRECTORY: panda/src/vision/
#

if (PkgSkip("VISION") == 0) and (not RUNTIME):
  # We want to know whether we have ffmpeg so that we can override the .avi association.
  if not PkgSkip("FFMPEG"):
    DefSymbol("OPENCV", "HAVE_FFMPEG")
  if not PkgSkip("OPENCV"):
    DefSymbol("OPENCV", "HAVE_OPENCV")
    if OPENCV_VER_23:
        DefSymbol("OPENCV", "OPENCV_VER_23")

  OPTS=['DIR:panda/src/vision', 'BUILDING:VISION', 'ARTOOLKIT', 'OPENCV', 'DX9', 'DIRECTCAM', 'JPEG', 'EXCEPTIONS']
  TargetAdd('p3vision_composite1.obj', opts=OPTS, input='p3vision_composite1.cxx', dep=[
    'dtool_have_ffmpeg.dat',
    'dtool_have_opencv.dat',
    'dtool_have_directcam.dat',
  ])

  TargetAdd('libp3vision.dll', input='p3vision_composite1.obj')
  TargetAdd('libp3vision.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3vision.dll', opts=OPTS)

  OPTS=['DIR:panda/src/vision', 'ARTOOLKIT', 'OPENCV', 'DX9', 'DIRECTCAM', 'JPEG', 'EXCEPTIONS']
  IGATEFILES=GetDirectoryContents('panda/src/vision', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3vision.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3vision.in', opts=['IMOD:panda3d.vision', 'ILIB:libp3vision', 'SRCDIR:panda/src/vision'])


  PyTargetAdd('vision_module.obj', input='libp3vision.in')
  PyTargetAdd('vision_module.obj', opts=OPTS)
  PyTargetAdd('vision_module.obj', opts=['IMOD:panda3d.vision', 'ILIB:vision', 'IMPORT:panda3d.core'])

  PyTargetAdd('vision.pyd', input='vision_module.obj')
  PyTargetAdd('vision.pyd', input='libp3vision_igate.obj')
  PyTargetAdd('vision.pyd', input='libp3vision.dll')
  PyTargetAdd('vision.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('vision.pyd', input=COMMON_PANDA_LIBS)

#
# DIRECTORY: panda/src/rocket/
#

if (PkgSkip("ROCKET") == 0) and (not RUNTIME):
  OPTS=['DIR:panda/src/rocket', 'BUILDING:ROCKET', 'ROCKET', 'PYTHON']
  TargetAdd('p3rocket_composite1.obj', opts=OPTS, input='p3rocket_composite1.cxx')

  TargetAdd('libp3rocket.dll', input='p3rocket_composite1.obj')
  TargetAdd('libp3rocket.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3rocket.dll', opts=OPTS)

  OPTS=['DIR:panda/src/rocket', 'ROCKET', 'RTTI', 'EXCEPTIONS']
  IGATEFILES=GetDirectoryContents('panda/src/rocket', ["rocketInputHandler.h",
    "rocketInputHandler.cxx", "rocketRegion.h", "rocketRegion.cxx", "rocketRegion_ext.h"])
  TargetAdd('libp3rocket.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3rocket.in', opts=['IMOD:panda3d.rocket', 'ILIB:libp3rocket', 'SRCDIR:panda/src/rocket'])

  PyTargetAdd('p3rocket_rocketRegion_ext.obj', opts=OPTS, input='rocketRegion_ext.cxx')

  PyTargetAdd('rocket_module.obj', input='libp3rocket.in')
  PyTargetAdd('rocket_module.obj', opts=OPTS)
  PyTargetAdd('rocket_module.obj', opts=['IMOD:panda3d.rocket', 'ILIB:rocket', 'IMPORT:panda3d.core'])

  PyTargetAdd('rocket.pyd', input='rocket_module.obj')
  PyTargetAdd('rocket.pyd', input='libp3rocket_igate.obj')
  PyTargetAdd('rocket.pyd', input='p3rocket_rocketRegion_ext.obj')
  PyTargetAdd('rocket.pyd', input='libp3rocket.dll')
  PyTargetAdd('rocket.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('rocket.pyd', input=COMMON_PANDA_LIBS)
  PyTargetAdd('rocket.pyd', opts=['ROCKET'])

#
# DIRECTORY: panda/src/p3skel
#

if (PkgSkip('SKEL')==0) and (not RUNTIME):
  OPTS=['DIR:panda/src/skel', 'BUILDING:PANDASKEL', 'ADVAPI']
  TargetAdd('p3skel_composite1.obj', opts=OPTS, input='p3skel_composite1.cxx')

  OPTS=['DIR:panda/src/skel', 'ADVAPI']
  IGATEFILES=GetDirectoryContents("panda/src/skel", ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3skel.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3skel.in', opts=['IMOD:panda3d.skel', 'ILIB:libp3skel', 'SRCDIR:panda/src/skel'])

#
# DIRECTORY: panda/src/p3skel
#

if (PkgSkip('SKEL')==0) and (not RUNTIME):
  OPTS=['BUILDING:PANDASKEL', 'ADVAPI']
  TargetAdd('libpandaskel.dll', input='p3skel_composite1.obj')
  TargetAdd('libpandaskel.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaskel.dll', opts=OPTS)

  PyTargetAdd('skel_module.obj', input='libp3skel.in')
  PyTargetAdd('skel_module.obj', opts=['IMOD:panda3d.skel', 'ILIB:skel', 'IMPORT:panda3d.core'])

  PyTargetAdd('skel.pyd', input='skel_module.obj')
  PyTargetAdd('skel.pyd', input='libp3skel_igate.obj')
  PyTargetAdd('skel.pyd', input='libpandaskel.dll')
  PyTargetAdd('skel.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('skel.pyd', input=COMMON_PANDA_LIBS)

#
# DIRECTORY: panda/src/distort/
#

if (PkgSkip('PANDAFX')==0) and (not RUNTIME):
  OPTS=['DIR:panda/src/distort', 'BUILDING:PANDAFX']
  TargetAdd('p3distort_composite1.obj', opts=OPTS, input='p3distort_composite1.cxx')

  OPTS=['DIR:panda/metalibs/pandafx', 'DIR:panda/src/distort', 'NVIDIACG']
  IGATEFILES=GetDirectoryContents('panda/src/distort', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3distort.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3distort.in', opts=['IMOD:panda3d.fx', 'ILIB:libp3distort', 'SRCDIR:panda/src/distort'])

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
  PyTargetAdd('fx_module.obj', input='libp3distort.in')
  PyTargetAdd('fx_module.obj', opts=OPTS)
  PyTargetAdd('fx_module.obj', opts=['IMOD:panda3d.fx', 'ILIB:fx', 'IMPORT:panda3d.core'])

  PyTargetAdd('fx.pyd', input='fx_module.obj')
  PyTargetAdd('fx.pyd', input='libp3distort_igate.obj')
  PyTargetAdd('fx.pyd', input='libpandafx.dll')
  PyTargetAdd('fx.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('fx.pyd', input=COMMON_PANDA_LIBS)

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
  TargetAdd('libp3vrpn.in', opts=['IMOD:panda3d.vrpn', 'ILIB:libp3vrpn', 'SRCDIR:panda/src/vrpn'])


  PyTargetAdd('vrpn_module.obj', input='libp3vrpn.in')
  PyTargetAdd('vrpn_module.obj', opts=OPTS)
  PyTargetAdd('vrpn_module.obj', opts=['IMOD:panda3d.vrpn', 'ILIB:vrpn', 'IMPORT:panda3d.core'])

  PyTargetAdd('vrpn.pyd', input='vrpn_module.obj')
  PyTargetAdd('vrpn.pyd', input='libp3vrpn_igate.obj')
  PyTargetAdd('vrpn.pyd', input='libp3vrpn.dll')
  PyTargetAdd('vrpn.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('vrpn.pyd', input=COMMON_PANDA_LIBS)

#
# DIRECTORY: panda/src/ffmpeg
#
if PkgSkip("FFMPEG") == 0 and not RUNTIME:
  if not PkgSkip("SWSCALE"):
    DefSymbol("FFMPEG", "HAVE_SWSCALE")
  if not PkgSkip("SWRESAMPLE"):
    DefSymbol("FFMPEG", "HAVE_SWRESAMPLE")

  OPTS=['DIR:panda/src/ffmpeg', 'BUILDING:FFMPEG', 'FFMPEG', 'SWSCALE', 'SWRESAMPLE']
  TargetAdd('p3ffmpeg_composite1.obj', opts=OPTS, input='p3ffmpeg_composite1.cxx', dep=[
    'dtool_have_swscale.dat', 'dtool_have_swresample.dat'])

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
  TargetAdd('libp3openal_audio.dll', opts=['MODULE', 'ADVAPI', 'WINUSER', 'WINMM', 'WINSHELL', 'WINOLE', 'OPENAL'])

#
# DIRECTORY: panda/src/downloadertools/
#

if (PkgSkip("OPENSSL")==0 and not RTDIST and not RUNTIME and PkgSkip("DEPLOYTOOLS")==0):
  OPTS=['DIR:panda/src/downloadertools', 'OPENSSL', 'ZLIB', 'ADVAPI', 'WINSOCK2', 'WINSHELL', 'WINGDI', 'WINUSER']

  TargetAdd('apply_patch_apply_patch.obj', opts=OPTS, input='apply_patch.cxx')
  TargetAdd('apply_patch.exe', input=['apply_patch_apply_patch.obj'])
  TargetAdd('apply_patch.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('apply_patch.exe', opts=OPTS)

  TargetAdd('build_patch_build_patch.obj', opts=OPTS, input='build_patch.cxx')
  TargetAdd('build_patch.exe', input=['build_patch_build_patch.obj'])
  TargetAdd('build_patch.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('build_patch.exe', opts=OPTS)

  if not PkgSkip("ZLIB"):
    TargetAdd('check_adler_check_adler.obj', opts=OPTS, input='check_adler.cxx')
    TargetAdd('check_adler.exe', input=['check_adler_check_adler.obj'])
    TargetAdd('check_adler.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('check_adler.exe', opts=OPTS)

    TargetAdd('check_crc_check_crc.obj', opts=OPTS, input='check_crc.cxx')
    TargetAdd('check_crc.exe', input=['check_crc_check_crc.obj'])
    TargetAdd('check_crc.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('check_crc.exe', opts=OPTS)

  TargetAdd('check_md5_check_md5.obj', opts=OPTS, input='check_md5.cxx')
  TargetAdd('check_md5.exe', input=['check_md5_check_md5.obj'])
  TargetAdd('check_md5.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('check_md5.exe', opts=OPTS)

  TargetAdd('pdecrypt_pdecrypt.obj', opts=OPTS, input='pdecrypt.cxx')
  TargetAdd('pdecrypt.exe', input=['pdecrypt_pdecrypt.obj'])
  TargetAdd('pdecrypt.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('pdecrypt.exe', opts=OPTS)

  TargetAdd('pencrypt_pencrypt.obj', opts=OPTS, input='pencrypt.cxx')
  TargetAdd('pencrypt.exe', input=['pencrypt_pencrypt.obj'])
  TargetAdd('pencrypt.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('pencrypt.exe', opts=OPTS)

  TargetAdd('show_ddb_show_ddb.obj', opts=OPTS, input='show_ddb.cxx')
  TargetAdd('show_ddb.exe', input=['show_ddb_show_ddb.obj'])
  TargetAdd('show_ddb.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('show_ddb.exe', opts=OPTS)

#
# DIRECTORY: panda/src/downloadertools/
#

if (PkgSkip("ZLIB")==0 and not RTDIST and not RUNTIME and PkgSkip("DEPLOYTOOLS")==0):
  OPTS=['DIR:panda/src/downloadertools', 'ZLIB', 'OPENSSL', 'ADVAPI', 'WINSOCK2', 'WINSHELL', 'WINGDI', 'WINUSER']

  TargetAdd('multify_multify.obj', opts=OPTS, input='multify.cxx')
  TargetAdd('multify.exe', input=['multify_multify.obj'])
  TargetAdd('multify.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('multify.exe', opts=OPTS)

  TargetAdd('pzip_pzip.obj', opts=OPTS, input='pzip.cxx')
  TargetAdd('pzip.exe', input=['pzip_pzip.obj'])
  TargetAdd('pzip.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('pzip.exe', opts=OPTS)

  TargetAdd('punzip_punzip.obj', opts=OPTS, input='punzip.cxx')
  TargetAdd('punzip.exe', input=['punzip_punzip.obj'])
  TargetAdd('punzip.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('punzip.exe', opts=OPTS)

#
# DIRECTORY: panda/src/windisplay/
#

if (GetTarget() == 'windows' and not RUNTIME):
  OPTS=['DIR:panda/src/windisplay', 'BUILDING:PANDAWIN']
  TargetAdd('p3windisplay_composite1.obj', opts=OPTS+["BIGOBJ"], input='p3windisplay_composite1.cxx')
  TargetAdd('p3windisplay_windetectdx9.obj', opts=OPTS + ["DX9"], input='winDetectDx9.cxx')
  TargetAdd('libp3windisplay.dll', input='p3windisplay_composite1.obj')
  TargetAdd('libp3windisplay.dll', input='p3windisplay_windetectdx9.obj')
  TargetAdd('libp3windisplay.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3windisplay.dll', opts=['WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM',"BIGOBJ"])

#
# DIRECTORY: panda/metalibs/pandadx9/
#

if GetTarget() == 'windows' and PkgSkip("DX9")==0 and not RUNTIME:
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

if not RUNTIME and not PkgSkip("EGG"):
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
  TargetAdd('libp3egg.in', opts=['IMOD:panda3d.egg', 'ILIB:libp3egg', 'SRCDIR:panda/src/egg'])
  PyTargetAdd('p3egg_eggGroupNode_ext.obj', opts=OPTS, input='eggGroupNode_ext.cxx')

#
# DIRECTORY: panda/src/egg2pg/
#

if not RUNTIME and not PkgSkip("EGG"):
  OPTS=['DIR:panda/src/egg2pg', 'BUILDING:PANDAEGG']
  TargetAdd('p3egg2pg_composite1.obj', opts=OPTS, input='p3egg2pg_composite1.cxx')
  TargetAdd('p3egg2pg_composite2.obj', opts=OPTS, input='p3egg2pg_composite2.cxx')

  OPTS=['DIR:panda/src/egg2pg']
  IGATEFILES=['load_egg_file.h', 'save_egg_file.h']
  TargetAdd('libp3egg2pg.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3egg2pg.in', opts=['IMOD:panda3d.egg', 'ILIB:libp3egg2pg', 'SRCDIR:panda/src/egg2pg'])

#
# DIRECTORY: panda/src/framework/
#

if (not RUNTIME):
  deps = []
  # Framework wants to link in a renderer when building statically, so tell it what is available.
  if GetLinkAllStatic():
    deps = ['dtool_have_gl.dat', 'dtool_have_tinydisplay.dat', 'dtool_have_egg.dat']
    if not PkgSkip("GL"):
      DefSymbol("FRAMEWORK", "HAVE_GL")
    if not PkgSkip("TINYDISPLAY"):
      DefSymbol("FRAMEWORK", "HAVE_TINYDISPLAY")
    if not PkgSkip("EGG"):
      DefSymbol("FRAMEWORK", "HAVE_EGG")

  OPTS=['DIR:panda/src/framework', 'BUILDING:FRAMEWORK', 'FRAMEWORK']
  TargetAdd('p3framework_composite1.obj', opts=OPTS, input='p3framework_composite1.cxx', dep=deps)
  TargetAdd('libp3framework.dll', input='p3framework_composite1.obj')
  TargetAdd('libp3framework.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3framework.dll', opts=['ADVAPI'])

#
# DIRECTORY: panda/src/glgsg/
#

if (not RUNTIME and PkgSkip("GL")==0):
  OPTS=['DIR:panda/src/glgsg', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGL', 'GL', 'NVIDIACG']
  TargetAdd('p3glgsg_config_glgsg.obj', opts=OPTS, input='config_glgsg.cxx')
  TargetAdd('p3glgsg_glgsg.obj', opts=OPTS, input='glgsg.cxx')

#
# DIRECTORY: panda/src/glesgsg/
#

if (not RUNTIME and PkgSkip("GLES")==0):
  OPTS=['DIR:panda/src/glesgsg', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLES', 'GLES']
  TargetAdd('p3glesgsg_config_glesgsg.obj', opts=OPTS, input='config_glesgsg.cxx')
  TargetAdd('p3glesgsg_glesgsg.obj', opts=OPTS, input='glesgsg.cxx')

#
# DIRECTORY: panda/src/gles2gsg/
#

if (not RUNTIME and PkgSkip("GLES2")==0):
  OPTS=['DIR:panda/src/gles2gsg', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLES2', 'GLES2']
  TargetAdd('p3gles2gsg_config_gles2gsg.obj', opts=OPTS, input='config_gles2gsg.cxx')
  TargetAdd('p3gles2gsg_gles2gsg.obj', opts=OPTS, input='gles2gsg.cxx')

#
# DIRECTORY: panda/metalibs/pandaegg/
#

if not RUNTIME and not PkgSkip("EGG"):
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
  PyTargetAdd('egg_module.obj', input='libp3egg2pg.in')
  PyTargetAdd('egg_module.obj', input='libp3egg.in')
  PyTargetAdd('egg_module.obj', opts=OPTS)
  PyTargetAdd('egg_module.obj', opts=['IMOD:panda3d.egg', 'ILIB:egg', 'IMPORT:panda3d.core'])

  PyTargetAdd('egg.pyd', input='egg_module.obj')
  PyTargetAdd('egg.pyd', input='p3egg_eggGroupNode_ext.obj')
  PyTargetAdd('egg.pyd', input='libp3egg_igate.obj')
  PyTargetAdd('egg.pyd', input='libp3egg2pg_igate.obj')
  PyTargetAdd('egg.pyd', input='libpandaegg.dll')
  PyTargetAdd('egg.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('egg.pyd', input=COMMON_PANDA_LIBS)

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
  TargetAdd('libpandagl.dll', opts=['MODULE', 'GL', 'NVIDIACG', 'CGGL', 'X11'])

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
  TargetAdd('libpandagl.dll', opts=['MODULE', 'GL', 'NVIDIACG', 'CGGL', 'COCOA', 'CARBON'])

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
  TargetAdd('libpandagles.dll', input='p3x11display_composite1.obj')
  TargetAdd('libpandagles.dll', input='pandagles_pandagles.obj')
  TargetAdd('libpandagles.dll', input='p3glesgsg_config_glesgsg.obj')
  TargetAdd('libpandagles.dll', input='p3glesgsg_glesgsg.obj')
  TargetAdd('libpandagles.dll', input='pandagles_egldisplay_composite1.obj')
  TargetAdd('libpandagles.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagles.dll', opts=['MODULE', 'GLES', 'EGL', 'X11'])

#
# DIRECTORY: panda/src/egldisplay/
#

if (PkgSkip("EGL")==0 and PkgSkip("GLES2")==0 and PkgSkip("X11")==0 and not RUNTIME):
  DefSymbol('GLES2', 'OPENGLES_2', '')
  OPTS=['DIR:panda/src/egldisplay', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLES2',  'GLES2', 'EGL']
  TargetAdd('pandagles2_egldisplay_composite1.obj', opts=OPTS, input='p3egldisplay_composite1.cxx')
  OPTS=['DIR:panda/metalibs/pandagles2', 'BUILDING:PANDAGLES2', 'GLES2', 'EGL']
  TargetAdd('pandagles2_pandagles2.obj', opts=OPTS, input='pandagles2.cxx')
  TargetAdd('libpandagles2.dll', input='p3x11display_composite1.obj')
  TargetAdd('libpandagles2.dll', input='pandagles2_pandagles2.obj')
  TargetAdd('libpandagles2.dll', input='p3gles2gsg_config_gles2gsg.obj')
  TargetAdd('libpandagles2.dll', input='p3gles2gsg_gles2gsg.obj')
  TargetAdd('libpandagles2.dll', input='pandagles2_egldisplay_composite1.obj')
  TargetAdd('libpandagles2.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandagles2.dll', opts=['MODULE', 'GLES2', 'EGL', 'X11'])

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
  IGATEFILES.remove("odeHelperStructs.h")
  TargetAdd('libpandaode.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpandaode.in', opts=['IMOD:panda3d.ode', 'ILIB:libpandaode', 'SRCDIR:panda/src/ode'])
  PyTargetAdd('p3ode_ext_composite.obj', opts=OPTS, input='p3ode_ext_composite.cxx')

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
  PyTargetAdd('ode_module.obj', input='libpandaode.in')
  PyTargetAdd('ode_module.obj', opts=OPTS)
  PyTargetAdd('ode_module.obj', opts=['IMOD:panda3d.ode', 'ILIB:ode', 'IMPORT:panda3d.core'])

  PyTargetAdd('ode.pyd', input='ode_module.obj')
  PyTargetAdd('ode.pyd', input='libpandaode_igate.obj')
  PyTargetAdd('ode.pyd', input='p3ode_ext_composite.obj')
  PyTargetAdd('ode.pyd', input='libpandaode.dll')
  PyTargetAdd('ode.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('ode.pyd', input=COMMON_PANDA_LIBS)
  PyTargetAdd('ode.pyd', opts=['WINUSER', 'ODE'])

#
# DIRECTORY: panda/src/bullet/
#
if (PkgSkip("BULLET")==0 and not RUNTIME):
  OPTS=['DIR:panda/src/bullet', 'BUILDING:PANDABULLET', 'BULLET']
  TargetAdd('p3bullet_composite.obj', opts=OPTS, input='p3bullet_composite.cxx')

  OPTS=['DIR:panda/src/bullet', 'BULLET']
  IGATEFILES=GetDirectoryContents('panda/src/bullet', ["*.h", "*_composite*.cxx"])
  TargetAdd('libpandabullet.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpandabullet.in', opts=['IMOD:panda3d.bullet', 'ILIB:libpandabullet', 'SRCDIR:panda/src/bullet'])

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
  PyTargetAdd('bullet_module.obj', input='libpandabullet.in')
  PyTargetAdd('bullet_module.obj', opts=OPTS)
  PyTargetAdd('bullet_module.obj', opts=['IMOD:panda3d.bullet', 'ILIB:bullet', 'IMPORT:panda3d.core'])

  PyTargetAdd('bullet.pyd', input='bullet_module.obj')
  PyTargetAdd('bullet.pyd', input='libpandabullet_igate.obj')
  PyTargetAdd('bullet.pyd', input='libpandabullet.dll')
  PyTargetAdd('bullet.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('bullet.pyd', input=COMMON_PANDA_LIBS)
  PyTargetAdd('bullet.pyd', opts=['WINUSER', 'BULLET'])

#
# DIRECTORY: panda/src/physx/
#

if (PkgSkip("PHYSX")==0):
  OPTS=['DIR:panda/src/physx', 'BUILDING:PANDAPHYSX', 'PHYSX']
  TargetAdd('p3physx_composite.obj', opts=OPTS, input='p3physx_composite.cxx')

  OPTS=['DIR:panda/src/physx', 'PHYSX']
  IGATEFILES=GetDirectoryContents('panda/src/physx', ["*.h", "*_composite*.cxx"])
  TargetAdd('libpandaphysx.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpandaphysx.in', opts=['IMOD:panda3d.physx', 'ILIB:libpandaphysx', 'SRCDIR:panda/src/physx'])

#
# DIRECTORY: panda/metalibs/pandaphysx/
#

if (PkgSkip("PHYSX")==0):
  OPTS=['DIR:panda/metalibs/pandaphysx', 'BUILDING:PANDAPHYSX', 'PHYSX', 'PYTHON']
  TargetAdd('pandaphysx_pandaphysx.obj', opts=OPTS, input='pandaphysx.cxx')

  TargetAdd('libpandaphysx.dll', input='pandaphysx_pandaphysx.obj')
  TargetAdd('libpandaphysx.dll', input='p3physx_composite.obj')
  TargetAdd('libpandaphysx.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libpandaphysx.dll', opts=['WINUSER', 'PHYSX', 'PYTHON'])

  OPTS=['DIR:panda/metalibs/pandaphysx', 'PHYSX']
  PyTargetAdd('physx_module.obj', input='libpandaphysx.in')
  PyTargetAdd('physx_module.obj', opts=OPTS)
  PyTargetAdd('physx_module.obj', opts=['IMOD:panda3d.physx', 'ILIB:physx', 'IMPORT:panda3d.core'])

  PyTargetAdd('physx.pyd', input='physx_module.obj')
  PyTargetAdd('physx.pyd', input='libpandaphysx_igate.obj')
  PyTargetAdd('physx.pyd', input='libpandaphysx.dll')
  PyTargetAdd('physx.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('physx.pyd', input=COMMON_PANDA_LIBS)
  PyTargetAdd('physx.pyd', opts=['WINUSER', 'PHYSX'])

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
  TargetAdd('libp3physics.in', opts=['IMOD:panda3d.physics', 'ILIB:libp3physics', 'SRCDIR:panda/src/physics'])

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
  TargetAdd('libp3particlesystem.in', opts=['IMOD:panda3d.physics', 'ILIB:libp3particlesystem', 'SRCDIR:panda/src/particlesystem'])

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
  PyTargetAdd('physics_module.obj', input='libp3physics.in')
  if (PkgSkip("PANDAPARTICLESYSTEM")==0):
    PyTargetAdd('physics_module.obj', input='libp3particlesystem.in')
  PyTargetAdd('physics_module.obj', opts=OPTS)
  PyTargetAdd('physics_module.obj', opts=['IMOD:panda3d.physics', 'ILIB:physics', 'IMPORT:panda3d.core'])

  PyTargetAdd('physics.pyd', input='physics_module.obj')
  PyTargetAdd('physics.pyd', input='libp3physics_igate.obj')
  if (PkgSkip("PANDAPARTICLESYSTEM")==0):
    PyTargetAdd('physics.pyd', input='libp3particlesystem_igate.obj')
  PyTargetAdd('physics.pyd', input='libpandaphysics.dll')
  PyTargetAdd('physics.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('physics.pyd', input=COMMON_PANDA_LIBS)

#
# DIRECTORY: panda/src/speedtree/
#

if (PkgSkip("SPEEDTREE")==0):
  OPTS=['DIR:panda/src/speedtree', 'BUILDING:PANDASPEEDTREE', 'SPEEDTREE']
  TargetAdd('pandaspeedtree_composite1.obj', opts=OPTS, input='pandaspeedtree_composite1.cxx')
  IGATEFILES=GetDirectoryContents('panda/src/speedtree', ["*.h", "*_composite*.cxx"])
  TargetAdd('libpandaspeedtree.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libpandaspeedtree.in', opts=['IMOD:libpandaspeedtree', 'ILIB:libpandaspeedtree', 'SRCDIR:panda/src/speedtree'])

  PyTargetAdd('libpandaspeedtree_module.obj', input='libpandaspeedtree.in')
  PyTargetAdd('libpandaspeedtree_module.obj', opts=OPTS)
  PyTargetAdd('libpandaspeedtree_module.obj', opts=['IMOD:libpandaspeedtree', 'ILIB:libpandaspeedtree'])
  TargetAdd('libpandaspeedtree.dll', input='pandaspeedtree_composite1.obj')
  PyTargetAdd('libpandaspeedtree.dll', input='libpandaspeedtree_igate.obj')
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
  if not PkgSkip("EGG"):
    TargetAdd('pview.exe', input='libpandaegg.dll')
  TargetAdd('pview.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('pview.exe', opts=['ADVAPI', 'WINSOCK2', 'WINSHELL'])

  if GetLinkAllStatic() and not PkgSkip("GL"):
    TargetAdd('pview.exe', input='libpandagl.dll')

#
# DIRECTORY: panda/src/android/
#

if (not RUNTIME and GetTarget() == 'android'):
  OPTS=['DIR:panda/src/android']
  TargetAdd('org/panda3d/android/NativeIStream.class', opts=OPTS, input='NativeIStream.java')
  TargetAdd('org/panda3d/android/NativeOStream.class', opts=OPTS, input='NativeOStream.java')
  TargetAdd('org/panda3d/android/PandaActivity.class', opts=OPTS, input='PandaActivity.java')
  TargetAdd('org/panda3d/android/PythonActivity.class', opts=OPTS, input='PythonActivity.java')

  TargetAdd('p3android_composite1.obj', opts=OPTS, input='p3android_composite1.cxx')
  TargetAdd('libp3android.dll', input='p3android_composite1.obj')
  TargetAdd('libp3android.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3android.dll', opts=['JNIGRAPHICS'])

  TargetAdd('android_native_app_glue.obj', opts=OPTS + ['NOHIDDEN'], input='android_native_app_glue.c')
  TargetAdd('android_main.obj', opts=OPTS, input='android_main.cxx')

  if (not RTDIST and PkgSkip("PVIEW")==0):
    TargetAdd('libpview_pview.obj', opts=OPTS, input='pview.cxx')
    TargetAdd('libpview.dll', input='android_native_app_glue.obj')
    TargetAdd('libpview.dll', input='android_main.obj')
    TargetAdd('libpview.dll', input='libpview_pview.obj')
    TargetAdd('libpview.dll', input='libp3framework.dll')
    if not PkgSkip("EGG"):
      TargetAdd('libpview.dll', input='libpandaegg.dll')
    TargetAdd('libpview.dll', input='libp3android.dll')
    TargetAdd('libpview.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libpview.dll', opts=['MODULE', 'ANDROID'])

  if (not RTDIST and PkgSkip("PYTHON")==0):
    OPTS += ['PYTHON']
    TargetAdd('ppython_ppython.obj', opts=OPTS, input='python_main.cxx')
    TargetAdd('libppython.dll', input='android_native_app_glue.obj')
    TargetAdd('libppython.dll', input='android_main.obj')
    TargetAdd('libppython.dll', input='ppython_ppython.obj')
    TargetAdd('libppython.dll', input='libp3framework.dll')
    TargetAdd('libppython.dll', input='libp3android.dll')
    TargetAdd('libppython.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libppython.dll', opts=['MODULE', 'ANDROID'])

#
# DIRECTORY: panda/src/androiddisplay/
#

if (GetTarget() == 'android' and PkgSkip("EGL")==0 and PkgSkip("GLES")==0 and not RUNTIME):
  DefSymbol('GLES', 'OPENGLES_1', '')
  OPTS=['DIR:panda/src/androiddisplay', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLES',  'GLES', 'EGL']
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
  OPTS=['DIR:panda/src/tinydisplay', 'BUILDING:TINYDISPLAY', 'X11']
  TargetAdd('p3tinydisplay_composite1.obj', opts=OPTS, input='p3tinydisplay_composite1.cxx')
  TargetAdd('p3tinydisplay_composite2.obj', opts=OPTS, input='p3tinydisplay_composite2.cxx')
  TargetAdd('p3tinydisplay_ztriangle_1.obj', opts=OPTS, input='ztriangle_1.cxx')
  TargetAdd('p3tinydisplay_ztriangle_2.obj', opts=OPTS, input='ztriangle_2.cxx')
  TargetAdd('p3tinydisplay_ztriangle_3.obj', opts=OPTS, input='ztriangle_3.cxx')
  TargetAdd('p3tinydisplay_ztriangle_4.obj', opts=OPTS, input='ztriangle_4.cxx')
  TargetAdd('p3tinydisplay_ztriangle_table.obj', opts=OPTS, input='ztriangle_table.cxx')
  if GetTarget() == 'windows':
    TargetAdd('libp3tinydisplay.dll', input='libp3windisplay.dll')
    TargetAdd('libp3tinydisplay.dll', opts=['WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM'])
  elif GetTarget() != 'darwin':
    TargetAdd('libp3tinydisplay.dll', input='p3x11display_composite1.obj')
    TargetAdd('libp3tinydisplay.dll', opts=['X11'])
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
  OPTS=['DIR:direct/src/directbase']
  TargetAdd('p3directbase_directbase.obj', opts=OPTS+['BUILDING:DIRECT'], input='directbase.cxx')

#
# DIRECTORY: direct/src/dcparser/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/dcparser', 'BUILDING:DIRECT_DCPARSER', 'WITHINPANDA', 'BISONPREFIX_dcyy']
  CreateFile(GetOutputDir()+"/include/dcParser.h")
  PyTargetAdd('p3dcparser_dcParser.obj', opts=OPTS, input='dcParser.yxx')
  #TargetAdd('dcParser.h', input='p3dcparser_dcParser.obj', opts=['DEPENDENCYONLY'])
  PyTargetAdd('p3dcparser_dcLexer.obj', opts=OPTS, input='dcLexer.lxx')
  PyTargetAdd('p3dcparser_composite1.obj', opts=OPTS, input='p3dcparser_composite1.cxx')
  PyTargetAdd('p3dcparser_composite2.obj', opts=OPTS, input='p3dcparser_composite2.cxx')

  OPTS=['DIR:direct/src/dcparser', 'WITHINPANDA']
  IGATEFILES=GetDirectoryContents('direct/src/dcparser', ["*.h", "*_composite*.cxx"])
  if "dcParser.h" in IGATEFILES: IGATEFILES.remove("dcParser.h")
  if "dcmsgtypes.h" in IGATEFILES: IGATEFILES.remove('dcmsgtypes.h')
  TargetAdd('libp3dcparser.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3dcparser.in', opts=['IMOD:panda3d.direct', 'ILIB:libp3dcparser', 'SRCDIR:direct/src/dcparser'])

#
# DIRECTORY: direct/src/deadrec/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/deadrec', 'BUILDING:DIRECT']
  TargetAdd('p3deadrec_composite1.obj', opts=OPTS, input='p3deadrec_composite1.cxx')

  OPTS=['DIR:direct/src/deadrec']
  IGATEFILES=GetDirectoryContents('direct/src/deadrec', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3deadrec.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3deadrec.in', opts=['IMOD:panda3d.direct', 'ILIB:libp3deadrec', 'SRCDIR:direct/src/deadrec'])

#
# DIRECTORY: direct/src/distributed/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/distributed', 'DIR:direct/src/dcparser', 'WITHINPANDA', 'BUILDING:DIRECT', 'OPENSSL']
  TargetAdd('p3distributed_config_distributed.obj', opts=OPTS, input='config_distributed.cxx')
  PyTargetAdd('p3distributed_cConnectionRepository.obj', opts=OPTS, input='cConnectionRepository.cxx')
  PyTargetAdd('p3distributed_cDistributedSmoothNodeBase.obj', opts=OPTS, input='cDistributedSmoothNodeBase.cxx')

  OPTS=['DIR:direct/src/distributed', 'WITHINPANDA', 'OPENSSL']
  IGATEFILES=GetDirectoryContents('direct/src/distributed', ["*.h", "*.cxx"])
  TargetAdd('libp3distributed.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3distributed.in', opts=['IMOD:panda3d.direct', 'ILIB:libp3distributed', 'SRCDIR:direct/src/distributed'])

#
# DIRECTORY: direct/src/interval/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/interval', 'BUILDING:DIRECT']
  TargetAdd('p3interval_composite1.obj', opts=OPTS, input='p3interval_composite1.cxx')

  OPTS=['DIR:direct/src/interval']
  IGATEFILES=GetDirectoryContents('direct/src/interval', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3interval.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3interval.in', opts=['IMOD:panda3d.direct', 'ILIB:libp3interval', 'SRCDIR:direct/src/interval'])

#
# DIRECTORY: direct/src/showbase/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/showbase', 'BUILDING:DIRECT']
  TargetAdd('p3showbase_showBase.obj', opts=OPTS, input='showBase.cxx')
  if GetTarget() == 'darwin':
    TargetAdd('p3showbase_showBase_assist.obj', opts=OPTS, input='showBase_assist.mm')

  OPTS=['DIR:direct/src/showbase']
  IGATEFILES=GetDirectoryContents('direct/src/showbase', ["*.h", "showBase.cxx"])
  TargetAdd('libp3showbase.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3showbase.in', opts=['IMOD:panda3d.direct', 'ILIB:libp3showbase', 'SRCDIR:direct/src/showbase'])

#
# DIRECTORY: direct/src/motiontrail/
#

if (PkgSkip("DIRECT")==0):
  OPTS=['DIR:direct/src/motiontrail', 'BUILDING:DIRECT']
  TargetAdd('p3motiontrail_cMotionTrail.obj', opts=OPTS, input='cMotionTrail.cxx')
  TargetAdd('p3motiontrail_config_motiontrail.obj', opts=OPTS, input='config_motiontrail.cxx')

  OPTS=['DIR:direct/src/motiontrail']
  IGATEFILES=GetDirectoryContents('direct/src/motiontrail', ["*.h", "cMotionTrail.cxx"])
  TargetAdd('libp3motiontrail.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3motiontrail.in', opts=['IMOD:panda3d.direct', 'ILIB:libp3motiontrail', 'SRCDIR:direct/src/motiontrail'])

#
# DIRECTORY: direct/metalibs/direct/
#

if (PkgSkip("DIRECT")==0):
  TargetAdd('libp3direct.dll', input='p3directbase_directbase.obj')
  TargetAdd('libp3direct.dll', input='p3showbase_showBase.obj')
  if GetTarget() == 'darwin':
    TargetAdd('libp3direct.dll', input='p3showbase_showBase_assist.obj')
  TargetAdd('libp3direct.dll', input='p3deadrec_composite1.obj')
  TargetAdd('libp3direct.dll', input='p3interval_composite1.obj')
  TargetAdd('libp3direct.dll', input='p3motiontrail_config_motiontrail.obj')
  TargetAdd('libp3direct.dll', input='p3motiontrail_cMotionTrail.obj')
  TargetAdd('libp3direct.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3direct.dll', opts=['ADVAPI',  'OPENSSL', 'WINUSER', 'WINGDI'])

  PyTargetAdd('direct_module.obj', input='libp3dcparser.in')
  PyTargetAdd('direct_module.obj', input='libp3showbase.in')
  PyTargetAdd('direct_module.obj', input='libp3deadrec.in')
  PyTargetAdd('direct_module.obj', input='libp3interval.in')
  PyTargetAdd('direct_module.obj', input='libp3distributed.in')
  PyTargetAdd('direct_module.obj', input='libp3motiontrail.in')
  PyTargetAdd('direct_module.obj', opts=['IMOD:panda3d.direct', 'ILIB:direct', 'IMPORT:panda3d.core'])

  PyTargetAdd('direct.pyd', input='libp3dcparser_igate.obj')
  PyTargetAdd('direct.pyd', input='libp3showbase_igate.obj')
  PyTargetAdd('direct.pyd', input='libp3deadrec_igate.obj')
  PyTargetAdd('direct.pyd', input='libp3interval_igate.obj')
  PyTargetAdd('direct.pyd', input='libp3distributed_igate.obj')
  PyTargetAdd('direct.pyd', input='libp3motiontrail_igate.obj')

  # These are part of direct.pyd, not libp3direct.dll, because they rely on
  # the Python libraries.  If a C++ user needs these modules, we can move them
  # back and filter out the Python-specific code.
  PyTargetAdd('direct.pyd', input='p3dcparser_composite1.obj')
  PyTargetAdd('direct.pyd', input='p3dcparser_composite2.obj')
  PyTargetAdd('direct.pyd', input='p3dcparser_dcParser.obj')
  PyTargetAdd('direct.pyd', input='p3dcparser_dcLexer.obj')
  PyTargetAdd('direct.pyd', input='p3distributed_config_distributed.obj')
  PyTargetAdd('direct.pyd', input='p3distributed_cConnectionRepository.obj')
  PyTargetAdd('direct.pyd', input='p3distributed_cDistributedSmoothNodeBase.obj')

  PyTargetAdd('direct.pyd', input='direct_module.obj')
  PyTargetAdd('direct.pyd', input='libp3direct.dll')
  PyTargetAdd('direct.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('direct.pyd', input=COMMON_PANDA_LIBS)
  PyTargetAdd('direct.pyd', opts=['OPENSSL', 'WINUSER', 'WINGDI', 'WINSOCK2'])

#
# DIRECTORY: direct/src/dcparse/
#

if (PkgSkip("PYTHON")==0 and PkgSkip("DIRECT")==0 and not RTDIST and not RUNTIME):
  OPTS=['DIR:direct/src/dcparse', 'DIR:direct/src/dcparser', 'WITHINPANDA', 'ADVAPI']
  PyTargetAdd('dcparse_dcparse.obj', opts=OPTS, input='dcparse.cxx')
  PyTargetAdd('p3dcparse.exe', input='p3dcparser_composite1.obj')
  PyTargetAdd('p3dcparse.exe', input='p3dcparser_composite2.obj')
  PyTargetAdd('p3dcparse.exe', input='p3dcparser_dcParser.obj')
  PyTargetAdd('p3dcparse.exe', input='p3dcparser_dcLexer.obj')
  PyTargetAdd('p3dcparse.exe', input='dcparse_dcparse.obj')
  PyTargetAdd('p3dcparse.exe', input='libp3direct.dll')
  PyTargetAdd('p3dcparse.exe', input=COMMON_PANDA_LIBS)
  PyTargetAdd('p3dcparse.exe', input='libp3pystub.lib')
  PyTargetAdd('p3dcparse.exe', opts=['ADVAPI'])

#
# DIRECTORY: direct/src/plugin/
#

if (RTDIST or RUNTIME):
  # Explicitly define this as we don't include dtool_config.h here.
  if GetTarget() not in ('windows', 'darwin'):
    DefSymbol("RUNTIME", "HAVE_X11", "1")

  OPTS=['DIR:direct/src/plugin', 'BUILDING:P3D_PLUGIN', 'RUNTIME', 'OPENSSL']
  TargetAdd('plugin_common.obj', opts=OPTS, input='plugin_common_composite1.cxx')

  OPTS += ['ZLIB', 'MSIMG']
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
      TargetAdd(fname, opts=['OPENSSL', 'ZLIB', 'X11', 'ADVAPI', 'WINUSER', 'WINGDI', 'WINSHELL', 'WINCOMCTL', 'WINOLE', 'MSIMG'])
    TargetAdd("libp3d_plugin_static.ilb", input='plugin_get_twirl_data.obj')

  if (PkgSkip("PYTHON")==0 and RTDIST):
    # Freeze VFSImporter and its dependency modules into p3dpython.
    # Mark panda3d.core as a dependency to make sure to build that first.
    TargetAdd('p3dpython_frozen.obj', input='VFSImporter.py', opts=['DIR:direct/src/showbase', 'FREEZE_STARTUP'])
    TargetAdd('p3dpython_frozen.obj', dep='core.pyd')

    OPTS += ['PYTHON']
    TargetAdd('p3dpython_p3dpython_composite1.obj', opts=OPTS, input='p3dpython_composite1.cxx')
    TargetAdd('p3dpython_p3dPythonMain.obj', opts=OPTS, input='p3dPythonMain.cxx')
    TargetAdd('p3dpython.exe', input='p3dpython_p3dpython_composite1.obj')
    TargetAdd('p3dpython.exe', input='p3dpython_p3dPythonMain.obj')
    TargetAdd('p3dpython.exe', input='p3dpython_frozen.obj')
    TargetAdd('p3dpython.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('p3dpython.exe', input='libp3tinyxml.ilb')
    TargetAdd('p3dpython.exe', input='libp3interrogatedb.dll')
    TargetAdd('p3dpython.exe', opts=['PYTHON', 'WINUSER'])

    TargetAdd('libp3dpython.dll', input='p3dpython_p3dpython_composite1.obj')
    TargetAdd('libp3dpython.dll', input='p3dpython_frozen.obj')
    TargetAdd('libp3dpython.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libp3dpython.dll', input='libp3tinyxml.ilb')
    TargetAdd('libp3dpython.dll', input='libp3interrogatedb.dll')
    TargetAdd('libp3dpython.dll', opts=['PYTHON', 'WINUSER'])

    if GetTarget() == 'windows':
      DefSymbol("NON_CONSOLE", "NON_CONSOLE", "")
      OPTS.append("NON_CONSOLE")
      TargetAdd('p3dpythonw_p3dpython_composite1.obj', opts=OPTS, input='p3dpython_composite1.cxx')
      TargetAdd('p3dpythonw_p3dPythonMain.obj', opts=OPTS, input='p3dPythonMain.cxx')
      TargetAdd('p3dpythonw.exe', input='p3dpythonw_p3dpython_composite1.obj')
      TargetAdd('p3dpythonw.exe', input='p3dpythonw_p3dPythonMain.obj')
      TargetAdd('p3dpythonw.exe', input='p3dpython_frozen.obj')
      TargetAdd('p3dpythonw.exe', input=COMMON_PANDA_LIBS)
      TargetAdd('p3dpythonw.exe', input='libp3tinyxml.ilb')
      TargetAdd('p3dpythonw.exe', input='libp3interrogatedb.dll')
      TargetAdd('p3dpythonw.exe', opts=['SUBSYSTEM:WINDOWS', 'WINUSER'])

  if (PkgSkip("OPENSSL")==0 and RTDIST and False):
    OPTS=['DIR:direct/src/plugin', 'DIR:panda/src/express', 'OPENSSL']
    if GetTarget() == 'darwin':
        OPTS += ['OPT:2']
    if (PkgSkip("FLTK")==0):
      OPTS.append("FLTK")
      TargetAdd('plugin_p3dCert.obj', opts=OPTS, input='p3dCert.cxx')
      TargetAdd('plugin_p3dCert_strings.obj', opts=OPTS, input='p3dCert_strings.cxx')
      TargetAdd('p3dcert.exe', input='plugin_mkdir_complete.obj')
      TargetAdd('p3dcert.exe', input='plugin_wstring_encode.obj')
      TargetAdd('p3dcert.exe', input='plugin_p3dCert.obj')
      TargetAdd('p3dcert.exe', input='plugin_p3dCert_strings.obj')
      OPTS=['SUBSYSTEM:WINDOWS', 'OPENSSL', 'FLTK', 'X11', 'WINCOMCTL', 'WINSOCK', 'WINGDI', 'WINUSER', 'ADVAPI', 'WINOLE', 'WINSHELL', 'SUBSYSTEM:WINDOWS']
      if GetTarget() == 'darwin':
          OPTS += ['OPT:2']
      TargetAdd('p3dcert.exe', opts=OPTS)
    elif (PkgSkip("WX")==0):
      OPTS += ["WX", "RTTI"]
      TargetAdd('plugin_p3dCert.obj', opts=OPTS, input='p3dCert_wx.cxx')
      TargetAdd('p3dcert.exe', input='plugin_mkdir_complete.obj')
      TargetAdd('p3dcert.exe', input='plugin_wstring_encode.obj')
      TargetAdd('p3dcert.exe', input='plugin_p3dCert.obj')
      OPTS=['SUBSYSTEM:WINDOWS', 'OPENSSL', 'WX', 'CARBON', 'WINOLE', 'WINOLEAUT', 'WINUSER', 'ADVAPI', 'WINSHELL', 'WINCOMCTL', 'WINGDI', 'WINCOMDLG']
      if GetTarget() == "darwin":
          OPTS += ['GL', 'OPT:2']
      TargetAdd('p3dcert.exe', opts=OPTS)

#
# DIRECTORY: direct/src/plugin_npapi/
#

if RUNTIME:
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

  OPTS += ['GTK2']
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
  TargetAdd('nppanda3d.plugin', opts=['OPENSSL', 'WINGDI', 'WINUSER', 'WINSHELL', 'WINOLE', 'CARBON'])

#
# DIRECTORY: direct/src/plugin_activex/
#

if (RUNTIME and GetTarget() == 'windows' and PkgSkip("MFC")==0):
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
  #TargetAdd('panda3d.exe', input='libp3pystub.lib')
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
    #TargetAdd('Panda3D.app', input='libp3pystub.lib')
    TargetAdd('Panda3D.app', input='libp3tinyxml.ilb')
    TargetAdd('Panda3D.app', input='panda3d_mac.plist', ipath=OPTS)
    TargetAdd('Panda3D.app', input='models/plugin_images/panda3d.icns')
    TargetAdd('Panda3D.app', opts=['OPENSSL', 'ZLIB', 'WINGDI', 'WINUSER', 'WINSHELL', 'ADVAPI', 'WINSOCK2', 'WINOLE', 'CARBON'])
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
    #TargetAdd('panda3dw.exe', input='libp3pystub.lib')
    TargetAdd('panda3dw.exe', input='libp3tinyxml.ilb')
    TargetAdd('panda3dw.exe', opts=['SUBSYSTEM:WINDOWS', 'OPENSSL', 'ZLIB', 'WINGDI', 'WINUSER', 'WINSHELL', 'ADVAPI', 'WINSOCK2', 'WINOLE', 'CARBON'])

if (RTDIST):
  OPTS=['BUILDING:P3D_PLUGIN', 'DIR:direct/src/plugin_standalone', 'DIR:direct/src/plugin', 'DIR:dtool/src/dtoolbase', 'DIR:dtool/src/dtoolutil', 'DIR:dtool/src/pystub', 'DIR:dtool/src/prc', 'DIR:dtool/src/dconfig', 'DIR:panda/src/express', 'DIR:panda/src/downloader', 'RUNTIME', 'P3DEMBED', 'OPENSSL', 'ZLIB']
  # This is arguably a big fat ugly hack, but doing it otherwise would complicate the build process considerably.
  DefSymbol("P3DEMBED", "LINK_ALL_STATIC", "")
  TargetAdd('plugin_standalone_panda3dBase.obj', opts=OPTS, input='panda3dBase.cxx')
  TargetAdd('plugin_standalone_p3dEmbedMain.obj', opts=OPTS, input='p3dEmbedMain.cxx')
  TargetAdd('plugin_standalone_p3dEmbed.obj', opts=OPTS, input='p3dEmbed.cxx')
  #TargetAdd('plugin_standalone_pystub.obj', opts=OPTS, input='pystub.cxx')
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
  TargetAdd('plugin_standalone_express_composite1.obj', opts=OPTS, input='p3express_composite1.cxx')
  TargetAdd('plugin_standalone_express_composite2.obj', opts=OPTS, input='p3express_composite2.cxx')
  TargetAdd('plugin_standalone_downloader_composite1.obj', opts=OPTS, input='p3downloader_composite1.cxx')
  TargetAdd('plugin_standalone_downloader_composite2.obj', opts=OPTS, input='p3downloader_composite2.cxx')
  TargetAdd('p3dembed.exe', input='plugin_standalone_panda3dBase.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_p3dEmbedMain.obj')
  TargetAdd('p3dembed.exe', input='plugin_standalone_p3dEmbed.obj')
  #TargetAdd('p3dembed.exe', input='plugin_standalone_pystub.obj')
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
  TargetAdd('p3dembed.exe', opts=['NOICON', 'WINGDI', 'WINSOCK2', 'ZLIB', 'WINUSER', 'OPENSSL', 'WINOLE', 'CARBON', 'MSIMG', 'WINCOMCTL', 'ADVAPI', 'WINSHELL', 'X11'])

  if GetTarget() == 'windows':
    OPTS.append("P3DEMBEDW")
    DefSymbol("P3DEMBEDW", "P3DEMBEDW", "")
    TargetAdd('plugin_standalone_p3dEmbedWinMain.obj', opts=OPTS, input='p3dEmbedMain.cxx')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_panda3dBase.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_p3dEmbedWinMain.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_p3dEmbed.obj')
    #TargetAdd('p3dembedw.exe', input='plugin_standalone_pystub.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_dtoolbase_composite1.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_dtoolbase_composite2.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_lookup3.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_indent.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_dtoolutil_composite1.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_dtoolutil_composite2.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_prc_composite1.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_prc_composite2.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_express_composite1.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_express_composite2.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_downloader_composite1.obj')
    TargetAdd('p3dembedw.exe', input='plugin_standalone_downloader_composite2.obj')
    TargetAdd('p3dembedw.exe', input='plugin_common.obj')
    TargetAdd('p3dembedw.exe', input='libp3tinyxml.ilb')
    TargetAdd('p3dembedw.exe', input='libp3d_plugin_static.ilb')
    TargetAdd('p3dembedw.exe', opts=['SUBSYSTEM:WINDOWS', 'NOICON', 'WINGDI', 'WINSOCK2', 'ZLIB', 'WINUSER', 'OPENSSL', 'WINOLE', 'MSIMG', 'WINCOMCTL', 'ADVAPI', 'WINSHELL'])

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

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/converter']
  TargetAdd('p3converter_somethingToEggConverter.obj', opts=OPTS, input='somethingToEggConverter.cxx')
  TargetAdd('p3converter_eggToSomethingConverter.obj', opts=OPTS, input='eggToSomethingConverter.cxx')
  TargetAdd('libp3converter.lib', input='p3converter_somethingToEggConverter.obj')
  TargetAdd('libp3converter.lib', input='p3converter_eggToSomethingConverter.obj')

#
# DIRECTORY: pandatool/src/progbase/
#

if not PkgSkip("PANDATOOL"):
  OPTS=['DIR:pandatool/src/progbase', 'ZLIB']
  TargetAdd('p3progbase_composite1.obj', opts=OPTS, input='p3progbase_composite1.cxx')
  TargetAdd('libp3progbase.lib', input='p3progbase_composite1.obj')

#
# DIRECTORY: pandatool/src/eggbase/
#

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/eggbase']
  TargetAdd('p3eggbase_composite1.obj', opts=OPTS, input='p3eggbase_composite1.cxx')
  TargetAdd('libp3eggbase.lib', input='p3eggbase_composite1.obj')

#
# DIRECTORY: pandatool/src/bam/
#

if not PkgSkip("PANDATOOL"):
  OPTS=['DIR:pandatool/src/bam']
  TargetAdd('bam-info_bamInfo.obj', opts=OPTS, input='bamInfo.cxx')
  TargetAdd('bam-info.exe', input='bam-info_bamInfo.obj')
  TargetAdd('bam-info.exe', input='libp3progbase.lib')
  TargetAdd('bam-info.exe', input='libp3pandatoolbase.lib')
  TargetAdd('bam-info.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('bam-info.exe', opts=['ADVAPI', 'FFTW'])

  if not PkgSkip("EGG"):
    TargetAdd('bam2egg_bamToEgg.obj', opts=OPTS, input='bamToEgg.cxx')
    TargetAdd('bam2egg.exe', input='bam2egg_bamToEgg.obj')
    TargetAdd('bam2egg.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('bam2egg.exe', opts=['ADVAPI',  'FFTW'])

    TargetAdd('egg2bam_eggToBam.obj', opts=OPTS, input='eggToBam.cxx')
    TargetAdd('egg2bam.exe', input='egg2bam_eggToBam.obj')
    TargetAdd('egg2bam.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('egg2bam.exe', opts=['ADVAPI',  'FFTW'])

#
# DIRECTORY: pandatool/src/daeegg/
#
if not PkgSkip("PANDATOOL") and not PkgSkip("FCOLLADA") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/daeegg', 'FCOLLADA']
  TargetAdd('p3daeegg_composite1.obj', opts=OPTS, input='p3daeegg_composite1.cxx')
  TargetAdd('libp3daeegg.lib', input='p3daeegg_composite1.obj')
  TargetAdd('libp3daeegg.lib', opts=['FCOLLADA', 'CARBON'])

#
# DIRECTORY: pandatool/src/assimp
#
if not PkgSkip("PANDATOOL") and not PkgSkip("ASSIMP"):
  OPTS=['DIR:pandatool/src/assimp', 'BUILDING:ASSIMP', 'ASSIMP', 'MODULE']
  TargetAdd('p3assimp_composite1.obj', opts=OPTS, input='p3assimp_composite1.cxx')
  TargetAdd('libp3assimp.dll', input='p3assimp_composite1.obj')
  TargetAdd('libp3assimp.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3assimp.dll', opts=OPTS+['ZLIB', 'ADVAPI'])

#
# DIRECTORY: pandatool/src/daeprogs/
#
if not PkgSkip("PANDATOOL") and not PkgSkip("FCOLLADA") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/daeprogs', 'FCOLLADA']
  TargetAdd('dae2egg_daeToEgg.obj', opts=OPTS, input='daeToEgg.cxx')
  TargetAdd('dae2egg.exe', input='dae2egg_daeToEgg.obj')
  TargetAdd('dae2egg.exe', input='libp3daeegg.lib')
  TargetAdd('dae2egg.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('dae2egg.exe', opts=['WINUSER', 'FCOLLADA', 'CARBON'])

#
# DIRECTORY: pandatool/src/dxf/
#

if not PkgSkip("PANDATOOL"):
  OPTS=['DIR:pandatool/src/dxf']
  TargetAdd('p3dxf_composite1.obj', opts=OPTS, input='p3dxf_composite1.cxx')
  TargetAdd('libp3dxf.lib', input='p3dxf_composite1.obj')

#
# DIRECTORY: pandatool/src/dxfegg/
#

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/dxfegg']
  TargetAdd('p3dxfegg_dxfToEggConverter.obj', opts=OPTS, input='dxfToEggConverter.cxx')
  TargetAdd('p3dxfegg_dxfToEggLayer.obj', opts=OPTS, input='dxfToEggLayer.cxx')
  TargetAdd('libp3dxfegg.lib', input='p3dxfegg_dxfToEggConverter.obj')
  TargetAdd('libp3dxfegg.lib', input='p3dxfegg_dxfToEggLayer.obj')

#
# DIRECTORY: pandatool/src/dxfprogs/
#

if not PkgSkip("PANDATOOL"):
  OPTS=['DIR:pandatool/src/dxfprogs']
  TargetAdd('dxf-points_dxfPoints.obj', opts=OPTS, input='dxfPoints.cxx')
  TargetAdd('dxf-points.exe', input='dxf-points_dxfPoints.obj')
  TargetAdd('dxf-points.exe', input='libp3progbase.lib')
  TargetAdd('dxf-points.exe', input='libp3dxf.lib')
  TargetAdd('dxf-points.exe', input='libp3pandatoolbase.lib')
  TargetAdd('dxf-points.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('dxf-points.exe', opts=['ADVAPI',  'FFTW'])

  if not PkgSkip("EGG"):
    TargetAdd('dxf2egg_dxfToEgg.obj', opts=OPTS, input='dxfToEgg.cxx')
    TargetAdd('dxf2egg.exe', input='dxf2egg_dxfToEgg.obj')
    TargetAdd('dxf2egg.exe', input='libp3dxfegg.lib')
    TargetAdd('dxf2egg.exe', input='libp3dxf.lib')
    TargetAdd('dxf2egg.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('dxf2egg.exe', opts=['ADVAPI',  'FFTW'])

    TargetAdd('egg2dxf_eggToDXF.obj', opts=OPTS, input='eggToDXF.cxx')
    TargetAdd('egg2dxf_eggToDXFLayer.obj', opts=OPTS, input='eggToDXFLayer.cxx')
    TargetAdd('egg2dxf.exe', input='egg2dxf_eggToDXF.obj')
    TargetAdd('egg2dxf.exe', input='egg2dxf_eggToDXFLayer.obj')
    TargetAdd('egg2dxf.exe', input='libp3dxf.lib')
    TargetAdd('egg2dxf.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('egg2dxf.exe', opts=['ADVAPI',  'FFTW'])

#
# DIRECTORY: pandatool/src/objegg/
#

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
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

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/objprogs']
  TargetAdd('obj2egg_objToEgg.obj', opts=OPTS, input='objToEgg.cxx')
  TargetAdd('obj2egg.exe', input='obj2egg_objToEgg.obj')
  TargetAdd('obj2egg.exe', input='libp3objegg.lib')
  TargetAdd('obj2egg.exe', input=COMMON_EGG2X_LIBS)

  TargetAdd('egg2obj_eggToObj.obj', opts=OPTS, input='eggToObj.cxx')
  TargetAdd('egg2obj.exe', input='egg2obj_eggToObj.obj')
  TargetAdd('egg2obj.exe', input='libp3objegg.lib')
  TargetAdd('egg2obj.exe', input=COMMON_EGG2X_LIBS)

#
# DIRECTORY: pandatool/src/palettizer/
#

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/palettizer']
  TargetAdd('p3palettizer_composite1.obj', opts=OPTS, input='p3palettizer_composite1.cxx')
  TargetAdd('libp3palettizer.lib', input='p3palettizer_composite1.obj')

#
# DIRECTORY: pandatool/src/egg-mkfont/
#

if not PkgSkip("FREETYPE") and not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/egg-mkfont', 'DIR:pandatool/src/palettizer', 'FREETYPE']
  TargetAdd('egg-mkfont_eggMakeFont.obj', opts=OPTS, input='eggMakeFont.cxx')
  TargetAdd('egg-mkfont_rangeDescription.obj', opts=OPTS, input='rangeDescription.cxx')
  TargetAdd('egg-mkfont_rangeIterator.obj', opts=OPTS, input='rangeIterator.cxx')
  TargetAdd('egg-mkfont.exe', input='egg-mkfont_eggMakeFont.obj')
  TargetAdd('egg-mkfont.exe', input='egg-mkfont_rangeDescription.obj')
  TargetAdd('egg-mkfont.exe', input='egg-mkfont_rangeIterator.obj')
  TargetAdd('egg-mkfont.exe', input='libp3palettizer.lib')
  TargetAdd('egg-mkfont.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg-mkfont.exe', opts=['ADVAPI', 'FREETYPE'])

#
# DIRECTORY: pandatool/src/eggcharbase/
#

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/eggcharbase', 'ZLIB']
  TargetAdd('p3eggcharbase_composite1.obj', opts=OPTS, input='p3eggcharbase_composite1.cxx')
  TargetAdd('libp3eggcharbase.lib', input='p3eggcharbase_composite1.obj')

#
# DIRECTORY: pandatool/src/egg-optchar/
#

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
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
  TargetAdd('egg-optchar.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg-optchar.exe', opts=['ADVAPI', 'FREETYPE'])

#
# DIRECTORY: pandatool/src/egg-palettize/
#

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/egg-palettize', 'DIR:pandatool/src/palettizer']
  TargetAdd('egg-palettize_eggPalettize.obj', opts=OPTS, input='eggPalettize.cxx')
  TargetAdd('egg-palettize.exe', input='egg-palettize_eggPalettize.obj')
  TargetAdd('egg-palettize.exe', input='libp3palettizer.lib')
  TargetAdd('egg-palettize.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg-palettize.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/egg-qtess/
#

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/egg-qtess']
  TargetAdd('egg-qtess_composite1.obj', opts=OPTS, input='egg-qtess_composite1.cxx')
  TargetAdd('egg-qtess.exe', input='egg-qtess_composite1.obj')
  TargetAdd('egg-qtess.exe', input='libp3eggbase.lib')
  TargetAdd('egg-qtess.exe', input='libp3progbase.lib')
  TargetAdd('egg-qtess.exe', input='libp3converter.lib')
  TargetAdd('egg-qtess.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg-qtess.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/eggprogs/
#

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/eggprogs']
  TargetAdd('egg-crop_eggCrop.obj', opts=OPTS, input='eggCrop.cxx')
  TargetAdd('egg-crop.exe', input='egg-crop_eggCrop.obj')
  TargetAdd('egg-crop.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg-crop.exe', opts=['ADVAPI'])

  TargetAdd('egg-make-tube_eggMakeTube.obj', opts=OPTS, input='eggMakeTube.cxx')
  TargetAdd('egg-make-tube.exe', input='egg-make-tube_eggMakeTube.obj')
  TargetAdd('egg-make-tube.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg-make-tube.exe', opts=['ADVAPI'])

  TargetAdd('egg-texture-cards_eggTextureCards.obj', opts=OPTS, input='eggTextureCards.cxx')
  TargetAdd('egg-texture-cards.exe', input='egg-texture-cards_eggTextureCards.obj')
  TargetAdd('egg-texture-cards.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg-texture-cards.exe', opts=['ADVAPI'])

  TargetAdd('egg-topstrip_eggTopstrip.obj', opts=OPTS, input='eggTopstrip.cxx')
  TargetAdd('egg-topstrip.exe', input='egg-topstrip_eggTopstrip.obj')
  TargetAdd('egg-topstrip.exe', input='libp3eggcharbase.lib')
  TargetAdd('egg-topstrip.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg-topstrip.exe', opts=['ADVAPI'])

  TargetAdd('egg-trans_eggTrans.obj', opts=OPTS, input='eggTrans.cxx')
  TargetAdd('egg-trans.exe', input='egg-trans_eggTrans.obj')
  TargetAdd('egg-trans.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg-trans.exe', opts=['ADVAPI'])

  TargetAdd('egg2c_eggToC.obj', opts=OPTS, input='eggToC.cxx')
  TargetAdd('egg2c.exe', input='egg2c_eggToC.obj')
  TargetAdd('egg2c.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg2c.exe', opts=['ADVAPI'])

  TargetAdd('egg-rename_eggRename.obj', opts=OPTS, input='eggRename.cxx')
  TargetAdd('egg-rename.exe', input='egg-rename_eggRename.obj')
  TargetAdd('egg-rename.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg-rename.exe', opts=['ADVAPI'])

  TargetAdd('egg-retarget-anim_eggRetargetAnim.obj', opts=OPTS, input='eggRetargetAnim.cxx')
  TargetAdd('egg-retarget-anim.exe', input='egg-retarget-anim_eggRetargetAnim.obj')
  TargetAdd('egg-retarget-anim.exe', input='libp3eggcharbase.lib')
  TargetAdd('egg-retarget-anim.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg-retarget-anim.exe', opts=['ADVAPI'])

  TargetAdd('egg-list-textures_eggListTextures.obj', opts=OPTS, input='eggListTextures.cxx')
  TargetAdd('egg-list-textures.exe', input='egg-list-textures_eggListTextures.obj')
  TargetAdd('egg-list-textures.exe', input=COMMON_EGG2X_LIBS)
  TargetAdd('egg-list-textures.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/flt/
#

if not PkgSkip("PANDATOOL"):
  OPTS=['DIR:pandatool/src/flt', 'ZLIB']
  TargetAdd('p3flt_composite1.obj', opts=OPTS, input='p3flt_composite1.cxx')
  TargetAdd('libp3flt.lib', input=['p3flt_composite1.obj'])

#
# DIRECTORY: pandatool/src/fltegg/
#

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/fltegg']
  TargetAdd('p3fltegg_fltToEggConverter.obj', opts=OPTS, input='fltToEggConverter.cxx')
  TargetAdd('p3fltegg_fltToEggLevelState.obj', opts=OPTS, input='fltToEggLevelState.cxx')
  TargetAdd('libp3fltegg.lib', input=['p3fltegg_fltToEggConverter.obj', 'p3fltegg_fltToEggLevelState.obj'])

#
# DIRECTORY: pandatool/src/fltprogs/
#

if not PkgSkip("PANDATOOL"):
  OPTS=['DIR:pandatool/src/fltprogs', 'DIR:pandatool/src/flt']
  TargetAdd('flt-info_fltInfo.obj', opts=OPTS, input='fltInfo.cxx')
  TargetAdd('flt-info.exe', input='flt-info_fltInfo.obj')
  TargetAdd('flt-info.exe', input='libp3flt.lib')
  TargetAdd('flt-info.exe', input='libp3progbase.lib')
  TargetAdd('flt-info.exe', input='libp3pandatoolbase.lib')
  TargetAdd('flt-info.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('flt-info.exe', opts=['ADVAPI'])

  TargetAdd('flt-trans_fltTrans.obj', opts=OPTS, input='fltTrans.cxx')
  TargetAdd('flt-trans.exe', input='flt-trans_fltTrans.obj')
  TargetAdd('flt-trans.exe', input='libp3flt.lib')
  TargetAdd('flt-trans.exe', input='libp3progbase.lib')
  TargetAdd('flt-trans.exe', input='libp3pandatoolbase.lib')
  TargetAdd('flt-trans.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('flt-trans.exe', opts=['ADVAPI'])

  if not PkgSkip("EGG"):
    TargetAdd('egg2flt_eggToFlt.obj', opts=OPTS, input='eggToFlt.cxx')
    TargetAdd('egg2flt.exe', input='egg2flt_eggToFlt.obj')
    TargetAdd('egg2flt.exe', input='libp3flt.lib')
    TargetAdd('egg2flt.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('egg2flt.exe', opts=['ADVAPI'])

    TargetAdd('flt2egg_fltToEgg.obj', opts=OPTS, input='fltToEgg.cxx')
    TargetAdd('flt2egg.exe', input='flt2egg_fltToEgg.obj')
    TargetAdd('flt2egg.exe', input='libp3flt.lib')
    TargetAdd('flt2egg.exe', input='libp3fltegg.lib')
    TargetAdd('flt2egg.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('flt2egg.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/imagebase/
#

if not PkgSkip("PANDATOOL"):
  OPTS=['DIR:pandatool/src/imagebase']
  TargetAdd('p3imagebase_composite1.obj', opts=OPTS, input='p3imagebase_composite1.cxx')
  TargetAdd('libp3imagebase.lib', input='p3imagebase_composite1.obj')

#
# DIRECTORY: pandatool/src/imageprogs/
#

if not PkgSkip("PANDATOOL"):
  OPTS=['DIR:pandatool/src/imageprogs']
  TargetAdd('image-info_imageInfo.obj', opts=OPTS, input='imageInfo.cxx')
  TargetAdd('image-info.exe', input='image-info_imageInfo.obj')
  TargetAdd('image-info.exe', input='libp3imagebase.lib')
  TargetAdd('image-info.exe', input='libp3progbase.lib')
  TargetAdd('image-info.exe', input='libp3pandatoolbase.lib')
  TargetAdd('image-info.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('image-info.exe', opts=['ADVAPI'])

  TargetAdd('image-resize_imageResize.obj', opts=OPTS, input='imageResize.cxx')
  TargetAdd('image-resize.exe', input='image-resize_imageResize.obj')
  TargetAdd('image-resize.exe', input='libp3imagebase.lib')
  TargetAdd('image-resize.exe', input='libp3progbase.lib')
  TargetAdd('image-resize.exe', input='libp3pandatoolbase.lib')
  TargetAdd('image-resize.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('image-resize.exe', opts=['ADVAPI'])

  TargetAdd('image-trans_imageTrans.obj', opts=OPTS, input='imageTrans.cxx')
  TargetAdd('image-trans.exe', input='image-trans_imageTrans.obj')
  TargetAdd('image-trans.exe', input='libp3imagebase.lib')
  TargetAdd('image-trans.exe', input='libp3progbase.lib')
  TargetAdd('image-trans.exe', input='libp3pandatoolbase.lib')
  TargetAdd('image-trans.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('image-trans.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/pfmprogs/
#

if not PkgSkip("PANDATOOL"):
  OPTS=['DIR:pandatool/src/pfmprogs']
  TargetAdd('pfm-trans_pfmTrans.obj', opts=OPTS, input='pfmTrans.cxx')
  TargetAdd('pfm-trans.exe', input='pfm-trans_pfmTrans.obj')
  TargetAdd('pfm-trans.exe', input='libp3progbase.lib')
  TargetAdd('pfm-trans.exe', input='libp3pandatoolbase.lib')
  TargetAdd('pfm-trans.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('pfm-trans.exe', opts=['ADVAPI'])

  TargetAdd('pfm-bba_pfmBba.obj', opts=OPTS, input='pfmBba.cxx')
  TargetAdd('pfm-bba_config_pfmprogs.obj', opts=OPTS, input='config_pfmprogs.cxx')
  TargetAdd('pfm-bba.exe', input='pfm-bba_pfmBba.obj')
  TargetAdd('pfm-bba.exe', input='pfm-bba_config_pfmprogs.obj')
  TargetAdd('pfm-bba.exe', input='libp3progbase.lib')
  TargetAdd('pfm-bba.exe', input='libp3pandatoolbase.lib')
  TargetAdd('pfm-bba.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('pfm-bba.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/lwo/
#

if not PkgSkip("PANDATOOL"):
  OPTS=['DIR:pandatool/src/lwo']
  TargetAdd('p3lwo_composite1.obj', opts=OPTS, input='p3lwo_composite1.cxx')
  TargetAdd('libp3lwo.lib', input='p3lwo_composite1.obj')

#
# DIRECTORY: pandatool/src/lwoegg/
#

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
  OPTS=['DIR:pandatool/src/lwoegg']
  TargetAdd('p3lwoegg_composite1.obj', opts=OPTS, input='p3lwoegg_composite1.cxx')
  TargetAdd('libp3lwoegg.lib', input='p3lwoegg_composite1.obj')

#
# DIRECTORY: pandatool/src/lwoprogs/
#

if not PkgSkip("PANDATOOL"):
  OPTS=['DIR:pandatool/src/lwoprogs', 'DIR:pandatool/src/lwo']
  TargetAdd('lwo-scan_lwoScan.obj', opts=OPTS, input='lwoScan.cxx')
  TargetAdd('lwo-scan.exe', input='lwo-scan_lwoScan.obj')
  TargetAdd('lwo-scan.exe', input='libp3lwo.lib')
  TargetAdd('lwo-scan.exe', input='libp3progbase.lib')
  TargetAdd('lwo-scan.exe', input='libp3pandatoolbase.lib')
  TargetAdd('lwo-scan.exe', input=COMMON_PANDA_LIBS)
  TargetAdd('lwo-scan.exe', opts=['ADVAPI'])

  if not PkgSkip("EGG"):
    TargetAdd('lwo2egg_lwoToEgg.obj', opts=OPTS, input='lwoToEgg.cxx')
    TargetAdd('lwo2egg.exe', input='lwo2egg_lwoToEgg.obj')
    TargetAdd('lwo2egg.exe', input='libp3lwo.lib')
    TargetAdd('lwo2egg.exe', input='libp3lwoegg.lib')
    TargetAdd('lwo2egg.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('lwo2egg.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/maya/
#

for VER in MAYAVERSIONS:
  VNUM=VER[4:]
  if not PkgSkip(VER) and not PkgSkip("PANDATOOL"):
    OPTS=['DIR:pandatool/src/maya', VER]
    TargetAdd('maya'+VNUM+'_composite1.obj', opts=OPTS, input='p3maya_composite1.cxx')
    TargetAdd('libmaya'+VNUM+'.lib', input='maya'+VNUM+'_composite1.obj')

#
# DIRECTORY: pandatool/src/mayaegg/
#

for VER in MAYAVERSIONS:
  VNUM=VER[4:]
  if not PkgSkip(VER) and not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
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
  if not PkgSkip(VER) and not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
    OPTS=['DIR:pandatool/src/maxegg', VER,  "WINCOMCTL", "WINCOMDLG", "WINUSER", "MSFORSCOPE", "RTTI"]
    TargetAdd('maxEgg'+VNUM+'.res', opts=OPTS, input='maxEgg.rc')
    TargetAdd('maxegg'+VNUM+'_loader.obj', opts=OPTS, input='maxEggLoader.cxx')
    TargetAdd('maxegg'+VNUM+'_composite1.obj', opts=OPTS, input='p3maxegg_composite1.cxx')
    TargetAdd('maxegg'+VNUM+'.dlo', input='maxegg'+VNUM+'_composite1.obj')
    TargetAdd('maxegg'+VNUM+'.dlo', input='maxEgg'+VNUM+'.res')
    TargetAdd('maxegg'+VNUM+'.dlo', input='maxEgg.def', ipath=OPTS)
    TargetAdd('maxegg'+VNUM+'.dlo', input=COMMON_EGG2X_LIBS)
    TargetAdd('maxegg'+VNUM+'.dlo', opts=OPTS)

#
# DIRECTORY: pandatool/src/maxprogs/
#

for VER in MAXVERSIONS:
  VNUM=VER[3:]
  if not PkgSkip(VER) and not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
    OPTS=['DIR:pandatool/src/maxprogs', VER,  "WINCOMCTL", "WINCOMDLG", "WINUSER", "MSFORSCOPE", "RTTI"]
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

if not PkgSkip("PANDATOOL"):
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

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
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

if not PkgSkip("PANDATOOL"):
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

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
    OPTS=['DIR:pandatool/src/xfileegg', 'DIR:pandatool/src/xfile']
    TargetAdd('p3xfileegg_composite1.obj', opts=OPTS, input='p3xfileegg_composite1.cxx')
    TargetAdd('libp3xfileegg.lib', input='p3xfileegg_composite1.obj')

#
# DIRECTORY: pandatool/src/ptloader/
#

if not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
    if not PkgSkip("FCOLLADA"):
        DefSymbol("FCOLLADA", "HAVE_FCOLLADA")

    OPTS=['DIR:pandatool/src/ptloader', 'DIR:pandatool/src/flt', 'DIR:pandatool/src/lwo', 'DIR:pandatool/src/xfile', 'DIR:pandatool/src/xfileegg', 'DIR:pandatool/src/daeegg', 'BUILDING:PTLOADER', 'FCOLLADA']
    TargetAdd('p3ptloader_config_ptloader.obj', opts=OPTS, input='config_ptloader.cxx', dep='dtool_have_fcollada.dat')
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

# This is a bit of an esoteric tool, and it causes issues because
# it conflicts with tools of the same name in different packages.
#if (PkgSkip("PANDATOOL")==0):
#    OPTS=['DIR:pandatool/src/miscprogs']
#    TargetAdd('bin2c_binToC.obj', opts=OPTS, input='binToC.cxx')
#    TargetAdd('bin2c.exe', input='bin2c_binToC.obj')
#    TargetAdd('bin2c.exe', input='libp3progbase.lib')
#    TargetAdd('bin2c.exe', input='libp3pandatoolbase.lib')
#    TargetAdd('bin2c.exe', input=COMMON_PANDA_LIBS)
#    TargetAdd('bin2c.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/pstatserver/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/pstatserver']
    TargetAdd('p3pstatserver_composite1.obj', opts=OPTS, input='p3pstatserver_composite1.cxx')
    TargetAdd('libp3pstatserver.lib', input='p3pstatserver_composite1.obj')

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
    TargetAdd('text-stats.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('text-stats.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/vrmlprogs/
#

if not PkgSkip("PANDATOOL"):
    OPTS=['DIR:pandatool/src/vrmlprogs', 'DIR:pandatool/src/vrml', 'DIR:pandatool/src/vrmlegg']
    TargetAdd('vrml-trans_vrmlTrans.obj', opts=OPTS, input='vrmlTrans.cxx')
    TargetAdd('vrml-trans.exe', input='vrml-trans_vrmlTrans.obj')
    TargetAdd('vrml-trans.exe', input='libp3vrml.lib')
    TargetAdd('vrml-trans.exe', input='libp3progbase.lib')
    TargetAdd('vrml-trans.exe', input='libp3pandatoolbase.lib')
    TargetAdd('vrml-trans.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('vrml-trans.exe', opts=['ADVAPI'])

    if not PkgSkip("EGG"):
        TargetAdd('vrml2egg_vrmlToEgg.obj', opts=OPTS, input='vrmlToEgg.cxx')
        TargetAdd('vrml2egg.exe', input='vrml2egg_vrmlToEgg.obj')
        TargetAdd('vrml2egg.exe', input='libp3vrmlegg.lib')
        TargetAdd('vrml2egg.exe', input='libp3vrml.lib')
        TargetAdd('vrml2egg.exe', input=COMMON_EGG2X_LIBS)
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
    TargetAdd('pstats.exe', opts=['SUBSYSTEM:WINDOWS', 'WINSOCK', 'WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM', 'GTK2'])

#
# DIRECTORY: pandatool/src/xfileprogs/
#

if not PkgSkip("PANDATOOL"):
    OPTS=['DIR:pandatool/src/xfileprogs', 'DIR:pandatool/src/xfile', 'DIR:pandatool/src/xfileegg']
    TargetAdd('x-trans_xFileTrans.obj', opts=OPTS, input='xFileTrans.cxx')
    TargetAdd('x-trans.exe', input='x-trans_xFileTrans.obj')
    TargetAdd('x-trans.exe', input='libp3progbase.lib')
    TargetAdd('x-trans.exe', input='libp3xfile.lib')
    TargetAdd('x-trans.exe', input='libp3pandatoolbase.lib')
    TargetAdd('x-trans.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('x-trans.exe', opts=['ADVAPI'])

    if not PkgSkip("EGG"):
        TargetAdd('egg2x_eggToX.obj', opts=OPTS, input='eggToX.cxx')
        TargetAdd('egg2x.exe', input='egg2x_eggToX.obj')
        TargetAdd('egg2x.exe', input='libp3xfileegg.lib')
        TargetAdd('egg2x.exe', input='libp3xfile.lib')
        TargetAdd('egg2x.exe', input=COMMON_EGG2X_LIBS)
        TargetAdd('egg2x.exe', opts=['ADVAPI'])

        TargetAdd('x2egg_xFileToEgg.obj', opts=OPTS, input='xFileToEgg.cxx')
        TargetAdd('x2egg.exe', input='x2egg_xFileToEgg.obj')
        TargetAdd('x2egg.exe', input='libp3xfileegg.lib')
        TargetAdd('x2egg.exe', input='libp3xfile.lib')
        TargetAdd('x2egg.exe', input=COMMON_EGG2X_LIBS)
        TargetAdd('x2egg.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/mayaprogs/
#

for VER in MAYAVERSIONS:
  VNUM = VER[4:]
  if not PkgSkip(VER) and not PkgSkip("PANDATOOL") and not PkgSkip("EGG"):
    if GetTarget() == 'darwin' and int(VNUM) < 2009:
      # No x86_64 support.
      continue

    OPTS=['DIR:pandatool/src/mayaprogs', 'DIR:pandatool/src/maya', 'DIR:pandatool/src/mayaegg', 'BUILDING:MISC', VER]
    TargetAdd('mayaeggimport'+VNUM+'_mayaeggimport.obj', opts=OPTS, input='mayaEggImport.cxx')
    TargetAdd('mayaeggimport'+VNUM+'.mll', input='mayaegg'+VNUM+'_loader.obj')
    TargetAdd('mayaeggimport'+VNUM+'.mll', input='mayaeggimport'+VNUM+'_mayaeggimport.obj')
    TargetAdd('mayaeggimport'+VNUM+'.mll', input='libpandaegg.dll')
    TargetAdd('mayaeggimport'+VNUM+'.mll', input=COMMON_PANDA_LIBS)
    #if GetTarget() == 'windows':
    #  TargetAdd('mayaeggimport'+VNUM+'.mll', input='libp3pystub.lib')
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
      TargetAdd('libmayapview'+VNUM+'.mll', input=COMMON_EGG2X_LIBS)
    else:
      TargetAdd('libmayapview'+VNUM+'.mll', input=COMMON_EGG2X_LIBS)
    TargetAdd('libmayapview'+VNUM+'.mll', opts=['ADVAPI', VER])

    TargetAdd('maya2egg'+VNUM+'_mayaToEgg.obj', opts=OPTS, input='mayaToEgg.cxx')
    TargetAdd('maya2egg'+VNUM+'_bin.exe', input='maya2egg'+VNUM+'_mayaToEgg.obj')
    TargetAdd('maya2egg'+VNUM+'_bin.exe', input='libmayaegg'+VNUM+'.lib')
    TargetAdd('maya2egg'+VNUM+'_bin.exe', input='libmaya'+VNUM+'.lib')
    if GetTarget() == 'windows':
      TargetAdd('maya2egg'+VNUM+'_bin.exe', input=COMMON_EGG2X_LIBS)
    else:
      TargetAdd('maya2egg'+VNUM+'_bin.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('maya2egg'+VNUM+'_bin.exe', opts=['ADVAPI', VER])

    TargetAdd('egg2maya'+VNUM+'_eggToMaya.obj', opts=OPTS, input='eggToMaya.cxx')
    TargetAdd('egg2maya'+VNUM+'_bin.exe', input='egg2maya'+VNUM+'_eggToMaya.obj')
    TargetAdd('egg2maya'+VNUM+'_bin.exe', input='libmayaegg'+VNUM+'.lib')
    TargetAdd('egg2maya'+VNUM+'_bin.exe', input='libmaya'+VNUM+'.lib')
    if GetTarget() == 'windows':
      TargetAdd('egg2maya'+VNUM+'_bin.exe', input=COMMON_EGG2X_LIBS)
    else:
      TargetAdd('egg2maya'+VNUM+'_bin.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('egg2maya'+VNUM+'_bin.exe', opts=['ADVAPI', VER])

    TargetAdd('mayasavepview'+VNUM+'_mayaSavePview.obj', opts=OPTS, input='mayaSavePview.cxx')
    TargetAdd('libmayasavepview'+VNUM+'.mll', input='mayasavepview'+VNUM+'_mayaSavePview.obj')
    TargetAdd('libmayasavepview'+VNUM+'.mll', opts=['ADVAPI', VER])

    TargetAdd('mayapath'+VNUM+'.obj', opts=OPTS, input='mayapath.cxx')

    TargetAdd('maya2egg'+VNUM+'.exe', input='mayapath'+VNUM+'.obj')
    TargetAdd('maya2egg'+VNUM+'.exe', input='libpandaexpress.dll')
    TargetAdd('maya2egg'+VNUM+'.exe', input=COMMON_DTOOL_LIBS)
    TargetAdd('maya2egg'+VNUM+'.exe', opts=['ADVAPI'])

    TargetAdd('egg2maya'+VNUM+'.exe', input='mayapath'+VNUM+'.obj')
    TargetAdd('egg2maya'+VNUM+'.exe', input='libpandaexpress.dll')
    TargetAdd('egg2maya'+VNUM+'.exe', input=COMMON_DTOOL_LIBS)
    TargetAdd('egg2maya'+VNUM+'.exe', opts=['ADVAPI'])

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
  TargetAdd('libpandaai.in', opts=['IMOD:panda3d.ai', 'ILIB:libpandaai', 'SRCDIR:contrib/src/ai'])

  PyTargetAdd('ai_module.obj', input='libpandaai.in')
  PyTargetAdd('ai_module.obj', opts=OPTS)
  PyTargetAdd('ai_module.obj', opts=['IMOD:panda3d.ai', 'ILIB:ai', 'IMPORT:panda3d.core'])

  PyTargetAdd('ai.pyd', input='ai_module.obj')
  PyTargetAdd('ai.pyd', input='libpandaai_igate.obj')
  PyTargetAdd('ai.pyd', input='libpandaai.dll')
  PyTargetAdd('ai.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('ai.pyd', input=COMMON_PANDA_LIBS)

#
# DIRECTORY: contrib/src/rplight/
#
if not PkgSkip("CONTRIB") and not PkgSkip("PYTHON") and not RUNTIME:
  OPTS=['DIR:contrib/src/rplight', 'BUILDING:RPLIGHT']
  TargetAdd('p3rplight_composite1.obj', opts=OPTS, input='p3rplight_composite1.cxx')

  IGATEFILES=GetDirectoryContents('contrib/src/rplight', ["*.h", "*_composite*.cxx"])
  TargetAdd('libp3rplight.in', opts=OPTS, input=IGATEFILES)
  TargetAdd('libp3rplight.in', opts=['IMOD:panda3d._rplight', 'ILIB:libp3rplight', 'SRCDIR:contrib/src/rplight'])

  PyTargetAdd('rplight_module.obj', input='libp3rplight.in')
  PyTargetAdd('rplight_module.obj', opts=OPTS)
  PyTargetAdd('rplight_module.obj', opts=['IMOD:panda3d._rplight', 'ILIB:_rplight', 'IMPORT:panda3d.core'])

  PyTargetAdd('_rplight.pyd', input='rplight_module.obj')
  PyTargetAdd('_rplight.pyd', input='libp3rplight_igate.obj')
  PyTargetAdd('_rplight.pyd', input='p3rplight_composite1.obj')
  PyTargetAdd('_rplight.pyd', input='libp3interrogatedb.dll')
  PyTargetAdd('_rplight.pyd', input=COMMON_PANDA_LIBS)

#
# DIRECTORY: pandatool/src/deploy-stub
#
if PkgSkip("PYTHON") == 0:
    OPTS=['DIR:pandatool/src/deploy-stub', 'BUILDING:DEPLOYSTUB']
    PyTargetAdd('deploy-stub.obj', opts=OPTS, input='deploy-stub.c')
    if GetTarget() == 'windows':
        PyTargetAdd('frozen_dllmain.obj', opts=OPTS, input='frozen_dllmain.c')

    if GetTarget() == 'linux' or GetTarget() == 'freebsd':
        # Setup rpath so libs can be found in the same directory as the deployed game
        LibName('DEPLOYSTUB', "-Wl,-rpath,\\$ORIGIN")
        LibName('DEPLOYSTUB', "-Wl,-z,origin")
        LibName('DEPLOYSTUB', "-rdynamic")
    PyTargetAdd('deploy-stub.exe', input='deploy-stub.obj')
    if GetTarget() == 'windows':
        PyTargetAdd('deploy-stub.exe', input='frozen_dllmain.obj')
    PyTargetAdd('deploy-stub.exe', opts=['WINSHELL', 'DEPLOYSTUB', 'NOICON'])

    if GetTarget() == 'windows':
        PyTargetAdd('deploy-stubw.exe', input='deploy-stub.obj')
        PyTargetAdd('deploy-stubw.exe', input='frozen_dllmain.obj')
        PyTargetAdd('deploy-stubw.exe', opts=['SUBSYSTEM:WINDOWS', 'WINSHELL', 'DEPLOYSTUB', 'NOICON'])
    elif GetTarget() == 'darwin':
        DefSymbol('MACOS_APP_BUNDLE', 'MACOS_APP_BUNDLE')
        OPTS = OPTS + ['MACOS_APP_BUNDLE']
        PyTargetAdd('deploy-stubw.obj', opts=OPTS, input='deploy-stub.c')
        PyTargetAdd('deploy-stubw.exe', input='deploy-stubw.obj')
        PyTargetAdd('deploy-stubw.exe', opts=['MACOS_APP_BUNDLE', 'DEPLOYSTUB', 'NOICON'])

#
# Generate the models directory and samples directory
#

if not PkgSkip("DIRECT") and not RUNTIME and not PkgSkip("EGG"):
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

if not PkgSkip("DIRECT") and not RUNTIME:
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
# If we have a host URL and distributor, we can make .p3d deployment tools.
#

if not PkgSkip("DIRECT") and not PkgSkip("DEPLOYTOOLS") and not RUNTIME and not RTDIST and HOST_URL and DISTRIBUTOR:
    OPTS=['DIR:direct/src/p3d']

    TargetAdd('packp3d.p3d', opts=OPTS, input='panda3d.pdef')
    TargetAdd('pdeploy.p3d', opts=OPTS, input='panda3d.pdef')
    TargetAdd('pmerge.p3d', opts=OPTS, input='panda3d.pdef')
    TargetAdd('ppackage.p3d', opts=OPTS, input='panda3d.pdef')
    TargetAdd('ppatcher.p3d', opts=OPTS, input='panda3d.pdef')

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
finally:
    SaveDependencyCache()

# Run the test suite.
if RUNTESTS:
    cmdstr = BracketNameWithQuotes(SDK["PYTHONEXEC"].replace('\\', '/'))
    if sys.version_info >= (2, 6):
        cmdstr += " -B"
    cmdstr += " -m pytest tests"
    if GetVerbose():
        cmdstr += " --verbose"
    oscmd(cmdstr)

# Write out information about the Python versions in the built dir.
python_version_info = GetCurrentPythonVersionInfo()
UpdatePythonVersionInfoFile(python_version_info)

##########################################################################################
#
# The Installers
#
# Under windows, we can build an 'exe' package using NSIS
# Under linux, we can build a 'deb' package or an 'rpm' package.
# Under OSX, we can make a 'dmg' package.
#
##########################################################################################

if INSTALLER:
    ProgressOutput(100.0, "Building installer")
    from makepackage import MakeInstaller

    # When using the --installer flag, only install for the current version.
    python_versions = []
    if python_version_info:
        python_versions.append(python_version_info)

    MakeInstaller(version=VERSION, outputdir=GetOutputDir(),
                  optimize=GetOptimize(), compressor=COMPRESSOR,
                  debversion=DEBVERSION, rpmrelease=RPMRELEASE,
                  runtime=RUNTIME, python_versions=python_versions)

if WHEEL:
    ProgressOutput(100.0, "Building wheel")
    from makewheel import makewheel
    makewheel(WHLVERSION, GetOutputDir())

##########################################################################################
#
# Print final status report.
#
##########################################################################################

WARNINGS.append("Elapsed Time: "+PrettyTime(time.time() - STARTTIME))

printStatus("Makepanda Final Status Report", WARNINGS)
print(GetColor("green") + "Build successfully finished, elapsed time: " + PrettyTime(time.time() - STARTTIME) + GetColor())
