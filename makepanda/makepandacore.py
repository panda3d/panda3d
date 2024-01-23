########################################################################
##
## This file, makepandacore, contains all the global state and
## global functions for the makepanda system.
##
########################################################################

import configparser
import fnmatch
import glob
import os
import pickle
import platform
import re
import shutil
import signal
import subprocess
import sys
import threading
import _thread as thread
import time
import locations

VCS_DIRS = set(["CVS", "CVSROOT", ".git", ".hg", "__pycache__"])
VCS_FILES = set([".cvsignore", ".gitignore", ".gitmodules", ".hgignore"])
STARTTIME = time.time()
MAINTHREAD = threading.current_thread()
OUTPUTDIR = "built"
CUSTOM_OUTPUTDIR = False
THIRDPARTYBASE = None
THIRDPARTYDIR = None
OPTIMIZE = "3"
VERBOSE = False
TARGET = None
TARGET_ARCH = None
HAS_TARGET_ARCH = False
TOOLCHAIN_PREFIX = ""
ANDROID_ABI = None
ANDROID_TRIPLE = None
ANDROID_API = None
SYS_LIB_DIRS = []
SYS_INC_DIRS = []
DEBUG_DEPENDENCIES = False

# Is the current Python a 32-bit or 64-bit build?  There doesn't
# appear to be a universal test for this.
if sys.platform == 'darwin':
    # On macOS, platform.architecture reports '64bit' even if it is
    # currently running in 32-bit mode.  But sys.maxint is a reliable
    # indicator.
    host_64 = (sys.maxsize > 0x100000000)
else:
    # On Windows (and Linux?) sys.maxint reports 0x7fffffff even on a
    # 64-bit build.  So we stick with platform.architecture in that
    # case.
    host_64 = (platform.architecture()[0] == '64bit')

# On Android, get a list of all the public system libraries.
ANDROID_SYS_LIBS = []
if os.path.exists("/etc/public.libraries.txt"):
    for line in open("/etc/public.libraries.txt", "r"):
        line = line.strip()
        ANDROID_SYS_LIBS.append(line)

########################################################################
##
## Visual C++ Version (MSVC) and Visual Studio Information Map
##
########################################################################

MSVCVERSIONINFO = {
    (10,0): {"vsversion":(10,0), "vsname":"Visual Studio 2010"},
    (11,0): {"vsversion":(11,0), "vsname":"Visual Studio 2012"},
    (12,0): {"vsversion":(12,0), "vsname":"Visual Studio 2013"},
    (14,0): {"vsversion":(14,0), "vsname":"Visual Studio 2015"},
    (14,1): {"vsversion":(15,0), "vsname":"Visual Studio 2017"},
    (14,2): {"vsversion":(16,0), "vsname":"Visual Studio 2019"},
    (14,3): {"vsversion":(17,0), "vsname":"Visual Studio 2022"},
}

########################################################################
##
## Maya and Max Version List (with registry keys)
##
########################################################################

MAYAVERSIONINFO = [("MAYA6",   "6.0"),
                   ("MAYA65",  "6.5"),
                   ("MAYA7",   "7.0"),
                   ("MAYA8",   "8.0"),
                   ("MAYA85",  "8.5"),
                   ("MAYA2008","2008"),
                   ("MAYA2009","2009"),
                   ("MAYA2010","2010"),
                   ("MAYA2011","2011"),
                   ("MAYA2012","2012"),
                   ("MAYA2013","2013"),
                   ("MAYA20135","2013.5"),
                   ("MAYA2014","2014"),
                   ("MAYA2015","2015"),
                   ("MAYA2016","2016"),
                   ("MAYA20165","2016.5"),
                   ("MAYA2017","2017"),
                   ("MAYA2018","2018"),
                   ("MAYA2019","2019"),
                   ("MAYA2020","2020"),
                   ("MAYA2022","2022"),
]

MAXVERSIONINFO = [("MAX6", "SOFTWARE\\Autodesk\\3DSMAX\\6.0", "installdir", "maxsdk\\cssdk\\include"),
                  ("MAX7", "SOFTWARE\\Autodesk\\3DSMAX\\7.0", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX8", "SOFTWARE\\Autodesk\\3DSMAX\\8.0", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX9", "SOFTWARE\\Autodesk\\3DSMAX\\9.0", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX2009", "SOFTWARE\\Autodesk\\3DSMAX\\11.0\\MAX-1:409", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX2010", "SOFTWARE\\Autodesk\\3DSMAX\\12.0\\MAX-1:409", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX2011", "SOFTWARE\\Autodesk\\3DSMAX\\13.0\\MAX-1:409", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX2012", "SOFTWARE\\Autodesk\\3DSMAX\\14.0\\MAX-1:409", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX2013", "SOFTWARE\\Autodesk\\3DSMAX\\15.0\\MAX-1:409", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX2014", "SOFTWARE\\Autodesk\\3DSMAX\\16.0\\MAX-1:409", "Installdir", "maxsdk\\include\\CS"),
]

MAYAVERSIONS = []
MAXVERSIONS = []
DXVERSIONS = ["DX9"]

for (ver,key) in MAYAVERSIONINFO:
    MAYAVERSIONS.append(ver)

for (ver,key1,key2,subdir) in MAXVERSIONINFO:
    MAXVERSIONS.append(ver)

########################################################################
##
## The exit routine will normally
##
## - print a message
## - save the dependency cache
## - exit
##
## However, if it is invoked inside a thread, it instead:
##
## - prints a message
## - raises the "initiate-exit" exception
##
## If you create a thread, you must be prepared to catch this
## exception, save the dependency cache, and exit.
##
########################################################################

WARNINGS = []
THREADS = {}
HAVE_COLORS = False
SETF = ""
try:
    import curses
    curses.setupterm()
    SETF = curses.tigetstr("setf")
    if SETF is None:
        SETF = curses.tigetstr("setaf")
    assert SETF is not None
    HAVE_COLORS = sys.stdout.isatty()
except:
    pass

def DisableColors():
    global HAVE_COLORS
    HAVE_COLORS = False

def GetColor(color = None):
    if not HAVE_COLORS:
        return ""
    if color is not None:
        color = color.lower()

    if color == "blue":
        token = curses.tparm(SETF, 1)
    elif color == "green":
        token = curses.tparm(SETF, 2)
    elif color == "cyan":
        token = curses.tparm(SETF, 3)
    elif color == "red":
        token = curses.tparm(SETF, 4)
    elif color == "magenta":
        token = curses.tparm(SETF, 5)
    elif color == "yellow":
        token = curses.tparm(SETF, 6)
    else:
        token = curses.tparm(curses.tigetstr("sgr0"))

    return token.decode('ascii')

def ColorText(color, text, reset=True):
    if reset is True:
        return ''.join((GetColor(color), text, GetColor()))
    else:
        return ''.join((GetColor(color), text))

def PrettyTime(t):
    t = int(t)
    hours = t // 3600
    t -= hours * 3600
    minutes = t // 60
    t -= minutes * 60
    seconds = t
    if hours:
        return "%d hours %d min" % (hours, minutes)
    if minutes:
        return "%d min %d sec" % (minutes, seconds)
    return "%d sec" % (seconds)

def exit(msg = ""):
    sys.stdout.flush()
    sys.stderr.flush()
    if threading.current_thread() == MAINTHREAD:
        print("Elapsed Time: " + PrettyTime(time.time() - STARTTIME))
        print(msg)
        print(ColorText("red", "Build terminated."))
        sys.stdout.flush()
        sys.stderr.flush()
        ##Don't quit the interperter if I'm running this file directly (debugging)
        if __name__ != '__main__':
            os._exit(1)
    else:
        print(msg)
        raise "initiate-exit"

def Warn(msg, extra=None):
    if extra is not None:
        print("%sWARNING:%s %s %s%s%s" % (GetColor("red"), GetColor(), msg, GetColor("green"), extra, GetColor()))
    else:
        print("%sWARNING:%s %s" % (GetColor("red"), GetColor(), msg))
    sys.stdout.flush()

def Error(msg, extra=None):
    if extra is not None:
        print("%sERROR:%s %s %s%s%s" % (GetColor("red"), GetColor(), msg, GetColor("green"), extra, GetColor()))
    else:
        print("%sERROR:%s %s" % (GetColor("red"), GetColor(), msg))
    exit()

########################################################################
##
## SetTarget, GetTarget, GetHost
##
## These functions control cross-compilation.
##
########################################################################

def GetHost():
    """Returns the host platform, ie. the one we're compiling on."""
    if sys.platform == 'win32' or sys.platform == 'cygwin':
        # sys.platform is win32 on 64-bits Windows as well.
        return 'windows'
    elif sys.platform == 'darwin':
        return 'darwin'
    elif sys.platform.startswith('linux'):
        try:
            # Python seems to offer no built-in way to check this.
            osname = subprocess.check_output(["uname", "-o"])
            if osname.strip().lower() == b'android':
                return 'android'
            else:
                return 'linux'
        except:
            return 'linux'
    elif sys.platform.startswith('freebsd'):
        return 'freebsd'
    else:
        exit('Unrecognized sys.platform: %s' % (sys.platform))

def GetHostArch():
    """Returns the architecture we're compiling on.
    Its value is also platform-dependent, as different platforms
    have different architecture naming."""

    target = GetTarget()
    if target == 'windows':
        return 'x64' if host_64 else 'x86'

    machine = platform.machine()
    if machine.startswith('armv7'):
        return 'armv7a'
    else:
        return machine

def SetTarget(target, arch=None):
    """Sets the target platform; the one we're compiling for.  Also
    sets the target architecture (None for default, if any).  Should
    be called *before* any calls are made to GetOutputDir, GetCC, etc."""
    global TARGET, TARGET_ARCH, HAS_TARGET_ARCH
    global TOOLCHAIN_PREFIX

    host = GetHost()
    host_arch = GetHostArch()
    if target is None:
        target = host
    else:
        target = target.lower()

    if arch is not None:
        HAS_TARGET_ARCH = True

    TOOLCHAIN_PREFIX = ''

    if target == 'windows':
        if arch == 'i386':
            arch = 'x86'
        elif arch == 'amd64':
            arch = 'x64'

        if arch is not None and arch != 'x86' and arch != 'x64':
            exit("Windows architecture must be x86 or x64")

    elif target == 'darwin':
        if arch == 'amd64':
            arch = 'x86_64'
        if arch == 'aarch64':
            arch = 'arm64'

        if arch is not None:
            choices = ('i386', 'x86_64', 'ppc', 'ppc64', 'arm64')
            if arch not in choices:
                exit('macOS architecture must be one of %s' % (', '.join(choices)))

    elif target == 'android' or target.startswith('android-'):
        if arch is None:
            # If compiling on Android, default to same architecture.  Otherwise, arm.
            if host == 'android':
                arch = host_arch
            else:
                arch = 'armv7a'

        if arch == 'aarch64':
            arch = 'arm64'

        # Did we specify an API level?
        global ANDROID_API
        target, _, api = target.partition('-')
        if api:
            ANDROID_API = int(api)
        elif arch in ('mips64', 'arm64', 'x86_64'):
            # 64-bit platforms were introduced in Android 21.
            ANDROID_API = 21
        else:
            # Default to the lowest API level still supported by Google.
            ANDROID_API = 19

        # Determine the prefix for our gcc tools, eg. arm-linux-androideabi-gcc
        global ANDROID_ABI, ANDROID_TRIPLE
        if arch == 'armv7a':
            ANDROID_ABI = 'armeabi-v7a'
            ANDROID_TRIPLE = 'armv7a-linux-androideabi'
        elif arch == 'arm':
            ANDROID_ABI = 'armeabi'
            ANDROID_TRIPLE = 'arm-linux-androideabi'
        elif arch == 'arm64':
            ANDROID_ABI = 'arm64-v8a'
            ANDROID_TRIPLE = 'aarch64-linux-android'
        elif arch == 'mips':
            ANDROID_ABI = 'mips'
            ANDROID_TRIPLE = 'mipsel-linux-android'
        elif arch == 'mips64':
            ANDROID_ABI = 'mips64'
            ANDROID_TRIPLE = 'mips64el-linux-android'
        elif arch == 'x86':
            ANDROID_ABI = 'x86'
            ANDROID_TRIPLE = 'i686-linux-android'
        elif arch == 'x86_64':
            ANDROID_ABI = 'x86_64'
            ANDROID_TRIPLE = 'x86_64-linux-android'
        else:
            exit('Android architecture must be arm, armv7a, arm64, mips, mips64, x86 or x86_64, use --arch to specify')

        ANDROID_TRIPLE += str(ANDROID_API)
        TOOLCHAIN_PREFIX = ANDROID_TRIPLE + '-'

    elif target == 'linux':
        if arch is not None:
            TOOLCHAIN_PREFIX = '%s-linux-gnu-' % arch

        elif host != 'linux':
            exit('Should specify an architecture when building for Linux')

    elif target == host:
        if arch is None or arch == host_arch:
            # Not a cross build.
            pass
        else:
            exit('Cannot cross-compile for %s-%s from %s-%s' % (target, arch, host, host_arch))

    else:
        exit('Cannot cross-compile for %s from %s' % (target, host))

    if arch is None:
        arch = host_arch

    TARGET = target
    TARGET_ARCH = arch

def GetTarget():
    """Returns the platform we're compiling for.  Defaults to GetHost()."""
    global TARGET
    if TARGET is None:
        TARGET = GetHost()
    return TARGET

def GetTargetArch():
    """Returns the architecture we're compiling for.  Defaults to GetHostArch().
    Its value is also dependent on that of GetTarget(), as different platforms
    use a different architecture naming."""
    global TARGET_ARCH
    if TARGET_ARCH is None:
        TARGET_ARCH = GetHostArch()
    return TARGET_ARCH

def CrossCompiling():
    """Returns True if we're cross-compiling."""
    return GetTarget() != GetHost()

def GetStrip():
    # Hack
    if TARGET == 'android':
        return 'llvm-strip'
    else:
        return 'strip'

SEVENZIP = None
def GetSevenZip():
    global SEVENZIP
    if SEVENZIP is not None:
        return SEVENZIP

    win_util = os.path.join(GetThirdpartyBase(), 'win-util')
    if GetHost() == 'windows' and os.path.isdir(win_util):
        SEVENZIP = GetThirdpartyBase() + "/win-util/7za.exe"
    elif LocateBinary('7z'):
        SEVENZIP = '7z'
    else:
        # We don't strictly need it, so don't give an error
        return None

    return SEVENZIP

def HasSevenZip():
    return GetSevenZip() is not None

########################################################################
##
## LocateBinary
##
## This function searches the system PATH for the binary. Returns its
## full path when it is found, or None when it was not found.
##
########################################################################

def LocateBinary(binary):
    if os.path.isfile(binary):
        return binary

    if "PATH" not in os.environ or os.environ["PATH"] == "":
        p = os.defpath
    else:
        p = os.environ["PATH"]

    pathList = p.split(os.pathsep)
    suffixes = ['']

    if GetHost() == 'windows':
        if not binary.lower().endswith('.exe') and not binary.lower().endswith('.bat'):
            # Append .exe if necessary
            suffixes = ['.exe', '.bat']

        # On Windows the current directory is always implicitly
        # searched before anything else on PATH.
        pathList = ['.'] + pathList

    for path in pathList:
        binpath = os.path.join(os.path.expanduser(path), binary)
        for suffix in suffixes:
            if os.access(binpath + suffix, os.X_OK):
                return os.path.abspath(os.path.realpath(binpath + suffix))
    return None

########################################################################
##
## Run a command.
##
########################################################################

def oscmd(cmd, ignoreError = False, cwd=None):
    if VERBOSE:
        print(GetColor("blue") + cmd.split(" ", 1)[0] + " " + GetColor("magenta") + cmd.split(" ", 1)[1] + GetColor())
    sys.stdout.flush()

    if sys.platform == "win32":
        if cmd[0] == '"':
            exe = cmd[1 : cmd.index('"', 1)]
        else:
            exe = cmd.split()[0]
        exe_path = LocateBinary(exe)
        if exe_path is None:
            exit("Cannot find "+exe+" on search path")

        if cwd is not None:
            pwd = os.getcwd()
            os.chdir(cwd)

        res = os.spawnl(os.P_WAIT, exe_path, cmd)

        if res == -1073741510: # 0xc000013a
            exit("keyboard interrupt")

        if cwd is not None:
            os.chdir(pwd)
    else:
        cmd = cmd.replace(';', '\\;')
        cmd = cmd.replace('$', '\\$')
        res = subprocess.call(cmd, cwd=cwd, shell=True)
        sig = res & 0x7F
        if (GetVerbose() and res != 0):
            print(ColorText("red", "Process exited with exit status %d and signal code %d" % ((res & 0xFF00) >> 8, sig)))
        if (sig == signal.SIGINT):
            exit("keyboard interrupt")

        # Don't ask me where the 35584 or 34304 come from...
        if (sig == signal.SIGSEGV or res == 35584 or res == 34304):
            if (LocateBinary("gdb") and GetVerbose() and GetHost() != "windows"):
                print(ColorText("red", "Received SIGSEGV, retrieving traceback..."))
                os.system("gdb -batch -ex 'handle SIG33 pass nostop noprint' -ex 'set pagination 0' -ex 'run' -ex 'bt full' -ex 'info registers' -ex 'thread apply all backtrace' -ex 'quit' --args %s < /dev/null" % cmd)
            else:
                print(ColorText("red", "Received SIGSEGV"))
            exit("")

    if res != 0 and not ignoreError:
        if "interrogate" in cmd.split(" ", 1)[0] and GetVerbose():
            print(ColorText("red", "Interrogate failed, retrieving debug output..."))
            sys.stdout.flush()
            verbose_cmd = cmd.split(" ", 1)[0] + " -vv " + cmd.split(" ", 1)[1]
            if sys.platform == "win32":
                os.spawnl(os.P_WAIT, exe_path, verbose_cmd)
            else:
                subprocess.call(verbose_cmd, shell=True)
        exit("The following command returned a non-zero value: " + str(cmd))

    return res

########################################################################
##
## GetDirectoryContents
##
########################################################################

def GetDirectoryContents(dir, filters="*", skip=[]):
    if isinstance(filters, str):
        filters = [filters]
    actual = {}
    files = os.listdir(dir)
    for filter in filters:
        for file in fnmatch.filter(files, filter):
            if (skip.count(file)==0) and (os.path.isfile(dir + "/" + file)):
                actual[file] = 1

    results = list(actual.keys())
    results.sort()
    return results

def GetDirectorySize(dir):
    if not os.path.isdir(dir):
        return 0
    size = 0
    for (path, dirs, files) in os.walk(dir):
        for file in files:
            try:
                size += os.path.getsize(os.path.join(path, file))
            except:
                pass
    return size

########################################################################
##
## The Timestamp Cache
##
## The make utility is constantly fetching the timestamps of files.
## This can represent the bulk of the file accesses during the make
## process.  The timestamp cache eliminates redundant checks.
##
########################################################################

TIMESTAMPCACHE = {}

def GetTimestamp(path):
    if path in TIMESTAMPCACHE:
        return TIMESTAMPCACHE[path]
    try:
        date = int(os.path.getmtime(path))
    except:
        date = 0
    TIMESTAMPCACHE[path] = date
    return date

def ClearTimestamp(path):
    del TIMESTAMPCACHE[path]

########################################################################
##
## General File Manipulation
##
########################################################################

def MakeDirectory(path, mode=None, recursive=False):
    if os.path.isdir(path):
        return

    if recursive:
        parent = os.path.dirname(path)
        if parent and not os.path.isdir(parent):
            MakeDirectory(parent, mode=mode, recursive=True)

    if mode is not None:
        os.mkdir(path, mode)
    else:
        os.mkdir(path)

def ReadFile(wfile):
    try:
        srchandle = open(wfile, "r")
        data = srchandle.read()
        srchandle.close()
        return data
    except:
        ex = sys.exc_info()[1]
        exit("Cannot read %s: %s" % (wfile, ex))

def ReadBinaryFile(wfile):
    try:
        srchandle = open(wfile, "rb")
        data = srchandle.read()
        srchandle.close()
        return data
    except:
        ex = sys.exc_info()[1]
        exit("Cannot read %s: %s" % (wfile, ex))

def WriteFile(wfile, data, newline=None):
    if newline is not None:
        data = data.replace('\r\n', '\n')
        data = data.replace('\r', '\n')
        data = data.replace('\n', newline)

    try:
        dsthandle = open(wfile, "w", newline='')
        dsthandle.write(data)
        dsthandle.close()
    except:
        ex = sys.exc_info()[1]
        exit("Cannot write to %s: %s" % (wfile, ex))

def WriteBinaryFile(wfile, data):
    try:
        dsthandle = open(wfile, "wb")
        dsthandle.write(data)
        dsthandle.close()
    except:
        ex = sys.exc_info()[1]
        exit("Cannot write to %s: %s" % (wfile, ex))

def DeleteVCS(dir):
    if dir == "": dir = "."
    for entry in os.listdir(dir):
        subdir = os.path.join(dir, entry)
        if (os.path.isdir(subdir)):
            if entry in VCS_DIRS:
                shutil.rmtree(subdir)
            else:
                DeleteVCS(subdir)
        elif (os.path.isfile(subdir) and (entry in VCS_FILES or entry.startswith(".#"))):
            os.remove(subdir)

def DeleteBuildFiles(dir):
    if dir == "": dir = "."
    for entry in os.listdir(dir):
        subdir = os.path.join(dir, entry)
        if (os.path.isfile(subdir) and os.path.splitext(subdir)[-1] in [".pp", ".moved"]):
            os.remove(subdir)
        elif (os.path.isdir(subdir)):
            if (os.path.basename(subdir)[:3] == "Opt" and os.path.basename(subdir)[4] == "-"):
                shutil.rmtree(subdir)
            else:
                DeleteBuildFiles(subdir)

def DeleteEmptyDirs(dir):
    if dir == "": dir = "."
    entries = os.listdir(dir)
    if not entries:
        os.rmdir(dir)
        return
    for entry in entries:
        subdir = os.path.join(dir, entry)
        if (os.path.isdir(subdir)):
            if (not os.listdir(subdir)):
                os.rmdir(subdir)
            else:
                DeleteEmptyDirs(subdir)


########################################################################
##
## Thirdparty libraries paths
##
########################################################################

def GetThirdpartyBase():
    """Returns the location of the 'thirdparty' directory.
    Normally, this is simply the thirdparty directory relative
    to the root of the source root, but if a MAKEPANDA_THIRDPARTY
    environment variable was set, it is used as the location of the
    thirdparty directory.  This is useful when wanting to use a single
    system-wide thirdparty directory, for instance on a build machine."""
    global THIRDPARTYBASE
    if (THIRDPARTYBASE is not None):
        return THIRDPARTYBASE

    THIRDPARTYBASE = "thirdparty"
    if "MAKEPANDA_THIRDPARTY" in os.environ:
        THIRDPARTYBASE = os.environ["MAKEPANDA_THIRDPARTY"]

    return THIRDPARTYBASE

def GetThirdpartyDir():
    """Returns the thirdparty directory for the target platform,
    ie. thirdparty/win-libs-vc10/.  May return None in the future."""
    global THIRDPARTYDIR
    if THIRDPARTYDIR is not None:
        return THIRDPARTYDIR

    THIRDPARTYDIR = ReadCMakeVar(GetOutputDir(), 'THIRDPARTY_DIRECTORY')

    if (GetVerbose()):
        print("Using thirdparty directory: %s" % THIRDPARTYDIR)

    return THIRDPARTYDIR

########################################################################
##
## Gets or sets the output directory, by default "built".
## Gets or sets the optimize level.
## Gets or sets the verbose flag.
##
########################################################################

def GetOutputDir():
    return OUTPUTDIR

def IsCustomOutputDir():
    return CUSTOM_OUTPUTDIR

def SetOutputDir(outputdir):
    global OUTPUTDIR, CUSTOM_OUTPUTDIR
    OUTPUTDIR = outputdir
    CUSTOM_OUTPUTDIR = True

def GetOptimize():
    return int(OPTIMIZE)

def SetOptimize(optimize):
    global OPTIMIZE
    OPTIMIZE = optimize

def GetVerbose():
    return VERBOSE

def SetVerbose(verbose):
    global VERBOSE
    VERBOSE = verbose

########################################################################
##
## Package Selection
##
## This facility enables makepanda to keep a list of packages selected
## by the user for inclusion or omission.
##
########################################################################

PKG_LIST_ALL = []
PKG_LIST_OMIT = {}
PKG_LIST_CUSTOM = set()

def PkgListSet(pkgs):
    global PKG_LIST_ALL
    global PKG_LIST_OMIT
    PKG_LIST_ALL=pkgs
    PKG_LIST_OMIT={}
    PkgEnableAll()

def PkgListGet():
    return PKG_LIST_ALL

def PkgEnableAll():
    for x in PKG_LIST_ALL:
        PKG_LIST_OMIT[x] = 0

def PkgDisableAll():
    for x in PKG_LIST_ALL:
        PKG_LIST_OMIT[x] = 1

def PkgEnable(pkg):
    PKG_LIST_OMIT[pkg] = 0

def PkgDisable(pkg):
    PKG_LIST_OMIT[pkg] = 1

def PkgSetCustomLocation(pkg):
    PKG_LIST_CUSTOM.add(pkg)

def PkgHasCustomLocation(pkg):
    return pkg in PKG_LIST_CUSTOM

def PkgSkip(pkg):
    return PKG_LIST_OMIT[pkg]

def PkgSelected(pkglist, pkg):
    if (pkglist.count(pkg)==0): return 0
    if (PKG_LIST_OMIT[pkg]): return 0
    return 1

########################################################################
##
## SDK Location
##
## This section is concerned with locating the install directories
## for various third-party packages.  The results are stored in the
## SDK table.
##
## Microsoft keeps changing the &*#$*& registry key for the DirectX SDK.
## The only way to reliably find it is to search through the installer's
## uninstall-directories, look in each one, and see if it contains the
## relevant files.
##
########################################################################

SDK = {}


########################################################################
##
## Parse setup.cfg to extract the version number.
##
########################################################################

cfg_parser = None

def GetMetadataValue(key):
    global cfg_parser
    if not cfg_parser:
        # Parse the metadata from the setup.cfg file.
        cfg_parser = configparser.ConfigParser()
        path = os.path.join(os.path.dirname(__file__), '..', 'setup.cfg')
        assert cfg_parser.read(path), "Could not read setup.cfg file."

    value = cfg_parser.get('metadata', key)
    if key == 'classifiers':
        value = value.strip().split('\n')
    return value

########################################################################
##
## Python utils
##
########################################################################

def GetExtensionSuffix():
    if GetTarget() == 'windows':
        if GetOptimize() <= 2:
            dllext = '_d'
        else:
            dllext = ''

        if GetTargetArch() == 'x64':
            return dllext + '.cp%d%d-win_amd64.pyd' % (sys.version_info[:2])
        else:
            return dllext + '.cp%d%d-win32.pyd' % (sys.version_info[:2])
    elif CrossCompiling():
        return '.{0}.so'.format(GetPythonABI())
    else:
        import _imp
        return _imp.extension_suffixes()[0]

def GetPythonABI():
    if not CrossCompiling():
        soabi = locations.get_config_var('SOABI')
        if soabi:
            return soabi

    return 'cpython-%d%d' % (sys.version_info[:2])

########################################################################
##
## These files maintain a python_versions.json file in the built/tmp
## directory that can be used by the other scripts in this directory.
##
########################################################################


def GetCurrentPythonVersionInfo():
    if PkgSkip("PYTHON"):
        return

    verinfo = sys.version_info
    return {
        "version": f'{verinfo.major}.{verinfo.minor}',
        "soabi": GetPythonABI(),
        "ext_suffix": GetExtensionSuffix(),
        "executable": sys.executable,
        "purelib": locations.get_python_lib(False),
        "platlib": locations.get_python_lib(True),
    }


def UpdatePythonVersionInfoFile(new_info):
    import json

    json_file = os.path.join(GetOutputDir(), "tmp", "python_versions.json")
    json_data = []
    if os.path.isfile(json_file) and not PkgSkip("PYTHON"):
        try:
            with open(json_file, 'r') as fh:
                json_data = json.load(fh)
        except:
            json_data = []

        # Prune the list by removing the entries that conflict with our build,
        # plus the entries that no longer exist, and the EOL Python versions
        for version_info in json_data[:]:
            core_pyd = os.path.join(GetOutputDir(), "panda3d", "core" + version_info["ext_suffix"])
            if version_info["ext_suffix"] == new_info["ext_suffix"] or \
               version_info["soabi"] == new_info["soabi"] or \
               not os.path.isfile(core_pyd) or \
               version_info["version"].split(".", 1)[0] == "2" or \
               version_info["version"] in ("3.0", "3.1", "3.2", "3.3", "3.4", "3.5", "3.6", "3.7"):
                json_data.remove(version_info)

    if not PkgSkip("PYTHON"):
        json_data.append(new_info)

    if VERBOSE:
        print("Writing %s" % (json_file))

    with open(json_file, 'w') as fh:
        json.dump(json_data, fh, indent=4)


def ReadPythonVersionInfoFile():
    import json

    json_file = os.path.join(GetOutputDir(), "tmp", "python_versions.json")
    if os.path.isfile(json_file):
        try:
            json_data = json.load(open(json_file, 'r'))
        except:
            pass

        # Don't include unsupported versions of Python.
        for version_info in json_data[:]:
            if version_info["version"] in ("2.6", "2.7", "3.0", "3.1", "3.2", "3.3", "3.4", "3.5", "3.6", "3.7"):
                json_data.remove(version_info)

        return json_data

    return []

########################################################################
##
## Utils for interacting with CMake builds
##
########################################################################

def GetCMakeCache(outputdir):
    return ReadFile(os.path.join(outputdir, 'CMakeCache.txt'))

def ReadCMakeVar(outputdir, varname):
    cmake_cache = GetCMakeCache(outputdir)
    for line in cmake_cache.splitlines():
        if not line.startswith(varname):
            continue

        try:
            var_name, var_type_value = line.split(':')
            var_type, var_value = var_type_value.split('=')
        except ValueError as exc:
            print(line)
            raise exc

        if var_name == varname:
            if var_type == 'BOOL':
                return var_value == 'ON'
            return var_value
    else:
        raise RuntimeError(f'Unable to find {varname} in CMakeCache.txt')
