########################################################################
##
## This file, makepandacore, contains all the global state and
## global functions for the makepanda system.
##
########################################################################

import sys,os,time,stat,string,re,getopt,fnmatch,threading,signal,shutil,platform,glob,getpass,signal
import subprocess
from distutils import sysconfig

if sys.version_info >= (3, 0):
    import pickle
    import _thread as thread
    import configparser
else:
    import cPickle as pickle
    import thread
    import ConfigParser as configparser

SUFFIX_INC = [".cxx",".cpp",".c",".h",".I",".yxx",".lxx",".mm",".rc",".r"]
SUFFIX_DLL = [".dll",".dlo",".dle",".dli",".dlm",".mll",".exe",".pyd",".ocx"]
SUFFIX_LIB = [".lib",".ilb"]
VCS_DIRS = set(["CVS", "CVSROOT", ".git", ".hg", "__pycache__"])
VCS_FILES = set([".cvsignore", ".gitignore", ".gitmodules", ".hgignore"])
STARTTIME = time.time()
MAINTHREAD = threading.currentThread()
OUTPUTDIR = "built"
CUSTOM_OUTPUTDIR = False
THIRDPARTYBASE = None
THIRDPARTYDIR = None
OPTIMIZE = "3"
VERBOSE = False
LINK_ALL_STATIC = False
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
    # On OSX, platform.architecture reports '64bit' even if it is
    # currently running in 32-bit mode.  But sys.maxint is a reliable
    # indicator.
    if sys.version_info >= (3, 0):
        host_64 = (sys.maxsize > 0x100000000)
    else:
        host_64 = (sys.maxint > 0x100000000)
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
## Potentially Conflicting Files
##
## The next function can warn about the existence of files that are
## commonly generated by ppremake that may conflict with the build.
##
########################################################################

CONFLICTING_FILES=["dtool/src/dtoolutil/pandaVersion.h",
                   "dtool/src/dtoolutil/checkPandaVersion.h",
                   "dtool/src/dtoolutil/checkPandaVersion.cxx",
                   "dtool/src/prc/prc_parameters.h",
                   "panda/src/speedtree/speedtree_parameters.h",
                   "direct/src/plugin/p3d_plugin_config.h",
                   "direct/src/plugin_activex/P3DActiveX.rc",
                   "direct/src/plugin_npapi/nppanda3d.rc",
                   "direct/src/plugin_standalone/panda3d.rc"]

def WarnConflictingFiles(delete = False):
    for cfile in CONFLICTING_FILES:
        if os.path.exists(cfile):
            Warn("file may conflict with build:", cfile)
            if delete:
                os.unlink(cfile)
                print("Deleted.")

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
  if (SETF == None):
    SETF = curses.tigetstr("setaf")
  assert SETF != None
  HAVE_COLORS = sys.stdout.isatty()
except: pass

def DisableColors():
    global HAVE_COLORS
    HAVE_COLORS = False

def GetColor(color = None):
    if not HAVE_COLORS:
        return ""
    if color != None:
        color = color.lower()

    if (color == "blue"):
        token = curses.tparm(SETF, 1)
    elif (color == "green"):
        token = curses.tparm(SETF, 2)
    elif (color == "cyan"):
        token = curses.tparm(SETF, 3)
    elif (color == "red"):
        token = curses.tparm(SETF, 4)
    elif (color == "magenta"):
        token = curses.tparm(SETF, 5)
    elif (color == "yellow"):
        token = curses.tparm(SETF, 6)
    else:
        token = curses.tparm(curses.tigetstr("sgr0"))

    if sys.version_info >= (3, 0):
        return token.decode('ascii')
    else:
        return token

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

def ProgressOutput(progress, msg, target = None):
    sys.stdout.flush()
    sys.stderr.flush()
    prefix = ""
    thisthread = threading.currentThread()
    if thisthread is MAINTHREAD:
        if progress is None:
            prefix = ""
        elif (progress >= 100.0):
            prefix = "%s[%s%d%%%s] " % (GetColor("yellow"), GetColor("cyan"), progress, GetColor("yellow"))
        elif (progress < 10.0):
            prefix = "%s[%s  %d%%%s] " % (GetColor("yellow"), GetColor("cyan"), progress, GetColor("yellow"))
        else:
            prefix = "%s[%s %d%%%s] " % (GetColor("yellow"), GetColor("cyan"), progress, GetColor("yellow"))
    else:
        global THREADS

        ident = thread.get_ident()
        if (ident not in THREADS):
            THREADS[ident] = len(THREADS) + 1
        prefix = "%s[%sT%d%s] " % (GetColor("yellow"), GetColor("cyan"), THREADS[ident], GetColor("yellow"))

    if target is not None:
        suffix = ' ' + ColorText("green", target)
    else:
        suffix = GetColor()

    print(''.join((prefix, msg, suffix)))
    sys.stdout.flush()
    sys.stderr.flush()

def exit(msg = ""):
    sys.stdout.flush()
    sys.stderr.flush()
    if (threading.currentThread() == MAINTHREAD):
        SaveDependencyCache()
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

        if arch is not None:
            choices = ('i386', 'x86_64', 'ppc', 'ppc64')
            if arch not in choices:
                exit('Mac OS X architecture must be one of %s' % (', '.join(choices)))

    elif target == 'android' or target.startswith('android-'):
        if arch is None:
            # If compiling on Android, default to same architecture.  Otherwise, arm.
            if host == 'android':
                arch = host_arch
            else:
                arch = 'armv7a'

        # Did we specify an API level?
        global ANDROID_API
        target, _, api = target.partition('-')
        if api:
            ANDROID_API = int(api)
        elif arch in ('mips64', 'aarch64', 'x86_64'):
            # 64-bit platforms were introduced in Android 21.
            ANDROID_API = 21
        else:
            # Default to the lowest API level supported by NDK r16.
            ANDROID_API = 14

        # Determine the prefix for our gcc tools, eg. arm-linux-androideabi-gcc
        global ANDROID_ABI, ANDROID_TRIPLE
        if arch == 'armv7a':
            ANDROID_ABI = 'armeabi-v7a'
            ANDROID_TRIPLE = 'arm-linux-androideabi'
        elif arch == 'arm':
            ANDROID_ABI = 'armeabi'
            ANDROID_TRIPLE = 'arm-linux-androideabi'
        elif arch == 'aarch64':
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
            exit('Android architecture must be arm, armv7a, aarch64, mips, mips64, x86 or x86_64')

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

def HasTargetArch():
    """Returns True if the user specified an architecture to compile for."""
    return HAS_TARGET_ARCH

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

def GetCC():
    if TARGET in ('darwin', 'freebsd', 'android'):
        return os.environ.get('CC', TOOLCHAIN_PREFIX + 'clang')
    else:
        return os.environ.get('CC', TOOLCHAIN_PREFIX + 'gcc')

def GetCXX():
    if TARGET in ('darwin', 'freebsd', 'android'):
        return os.environ.get('CXX', TOOLCHAIN_PREFIX + 'clang++')
    else:
        return os.environ.get('CXX', TOOLCHAIN_PREFIX + 'g++')

def GetStrip():
    # Hack
    if TARGET == 'android':
        return TOOLCHAIN_PREFIX + 'strip'
    else:
        return 'strip'

def GetAR():
    # Hack
    if TARGET == 'android':
        return TOOLCHAIN_PREFIX + 'ar'
    else:
        return 'ar'

def GetRanlib():
    # Hack
    if TARGET == 'android':
        return TOOLCHAIN_PREFIX + 'ranlib'
    else:
        return 'ranlib'

BISON = None
def GetBison():
    global BISON
    if BISON is not None:
        return BISON

    # We now require a newer version of Bison than the one we previously
    # shipped in the win-util dir.  The new version has a 'data'
    # subdirectory, so check for that.
    win_util_data = os.path.join(GetThirdpartyBase(), 'win-util', 'data')
    if GetHost() == 'windows' and os.path.isdir(win_util_data):
        BISON = os.path.join(GetThirdpartyBase(), 'win-util', 'bison.exe')
    elif LocateBinary('bison'):
        BISON = 'bison'
    else:
        # We don't strictly need it, so don't give an error
        return None

    return BISON

FLEX = None
def GetFlex():
    global FLEX
    if FLEX is not None:
        return FLEX

    win_util = os.path.join(GetThirdpartyBase(), 'win-util')
    if GetHost() == 'windows' and os.path.isdir(win_util):
        FLEX = GetThirdpartyBase() + "/win-util/flex.exe"
    elif LocateBinary('flex'):
        FLEX = 'flex'
    else:
        # We don't strictly need it, so don't give an error
        return None

    return FLEX

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

        if cwd is not None:
            os.chdir(pwd)
    else:
        cmd = cmd.replace(';', '\\;')
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
    if (type(filters)==str):
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
            except: pass
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
    try: date = os.path.getmtime(path)
    except: date = 0
    TIMESTAMPCACHE[path] = date
    return date

def ClearTimestamp(path):
    del TIMESTAMPCACHE[path]

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
########################################################################

BUILTFROMCACHE = {}

def JustBuilt(files, others):
    dates = {}
    for file in files:
        del TIMESTAMPCACHE[file]
        dates[file] = GetTimestamp(file)
    for file in others:
        dates[file] = GetTimestamp(file)

    key = tuple(files)
    BUILTFROMCACHE[key] = dates

def NeedsBuild(files, others):
    dates = {}
    for file in files:
        dates[file] = GetTimestamp(file)
        if not os.path.exists(file):
            if DEBUG_DEPENDENCIES:
                print("rebuilding %s because it does not exist" % (file))
            return True

    for file in others:
        dates[file] = GetTimestamp(file)

    key = tuple(files)
    if key in BUILTFROMCACHE:
        cached = BUILTFROMCACHE[key]
        if cached == dates:
            return False
        elif DEBUG_DEPENDENCIES:
            print("rebuilding %s because:" % (key))
            for key in frozenset(cached.keys()) | frozenset(dates.keys()):
                if key not in cached:
                    print("    new dependency: %s" % (key))
                elif key not in dates:
                    print("    removed dependency: %s" % (key))
                elif cached[key] != dates[key]:
                    print("    dependency changed: %s" % (key))

        if VERBOSE and frozenset(cached) != frozenset(dates):
            Warn("file dependencies changed:", files)

    return True

########################################################################
##
## The CXX include cache:
##
## The following routine scans a CXX file and returns a list of
## the include-directives inside that file.  It's not recursive:
## it just returns the includes that are textually inside the
## file.  If you need recursive dependencies, you need the higher-level
## routine CxxCalcDependencies, defined elsewhere.
##
## Since scanning a CXX file is slow, we cache the result.  It records
## the date of the source file and the list of includes that it
## contains.  It assumes that if the file date hasn't changed, that
## the list of include-statements inside the file has not changed
## either.  Once again, this particular routine does not return
## recursive dependencies --- it only returns an explicit list of
## include statements that are textually inside the file.  That
## is what the cache stores, as well.
##
########################################################################

CXXINCLUDECACHE = {}

global CxxIncludeRegex
CxxIncludeRegex = re.compile('^[ \t]*[#][ \t]*include[ \t]+"([^"]+)"[ \t\r\n]*$')

def CxxGetIncludes(path):
    date = GetTimestamp(path)
    if (path in CXXINCLUDECACHE):
        cached = CXXINCLUDECACHE[path]
        if (cached[0]==date): return cached[1]
    try: sfile = open(path, 'r')
    except:
        exit("Cannot open source file \""+path+"\" for reading.")
    include = []
    try:
        for line in sfile:
            match = CxxIncludeRegex.match(line,0)
            if (match):
                incname = match.group(1)
                include.append(incname)
    except:
        print("Failed to determine dependencies of \""+path+"\".")
        raise

    sfile.close()
    CXXINCLUDECACHE[path] = [date, include]
    return include

JAVAIMPORTCACHE = {}

global JavaImportRegex
JavaImportRegex = re.compile('[ \t\r\n;]import[ \t]+([a-zA-Z][^;]+)[ \t\r\n]*;')

def JavaGetImports(path):
    date = GetTimestamp(path)
    if path in JAVAIMPORTCACHE:
        cached = JAVAIMPORTCACHE[path]
        if cached[0] == date:
            return cached[1]
    try:
        source = open(path, 'r').read()
    except:
        exit("Cannot open source file \"" + path + "\" for reading.")

    imports = []
    try:
        for match in JavaImportRegex.finditer(source, 0):
            impname = match.group(1)
            imports.append(impname.strip())
    except:
        print("Failed to determine dependencies of \"" + path  +"\".")
        raise

    JAVAIMPORTCACHE[path] = [date, imports]
    return imports

########################################################################
##
## SaveDependencyCache / LoadDependencyCache
##
## This actually saves both the dependency and cxx-include caches.
##
########################################################################

DCACHE_VERSION = 2
DCACHE_BACKED_UP = False

def SaveDependencyCache():
    global DCACHE_BACKED_UP
    if not DCACHE_BACKED_UP:
        try:
            if (os.path.exists(os.path.join(OUTPUTDIR, "tmp", "makepanda-dcache"))):
                os.rename(os.path.join(OUTPUTDIR, "tmp", "makepanda-dcache"),
                          os.path.join(OUTPUTDIR, "tmp", "makepanda-dcache-backup"))
        except: pass
        DCACHE_BACKED_UP = True

    try:
        icache = open(os.path.join(OUTPUTDIR, "tmp", "makepanda-dcache"),'wb')
    except:
        icache = None

    if icache is not None:
        print("Storing dependency cache.")
        pickle.dump(DCACHE_VERSION, icache, 0)
        pickle.dump(CXXINCLUDECACHE, icache, 2)
        pickle.dump(BUILTFROMCACHE, icache, 2)
        icache.close()

def LoadDependencyCache():
    global CXXINCLUDECACHE
    global BUILTFROMCACHE

    try:
        icache = open(os.path.join(OUTPUTDIR, "tmp", "makepanda-dcache"), 'rb')
    except:
        icache = None

    if icache is not None:
        ver = pickle.load(icache)
        if ver == DCACHE_VERSION:
            CXXINCLUDECACHE = pickle.load(icache)
            BUILTFROMCACHE = pickle.load(icache)
            icache.close()
        else:
            print("Cannot load dependency cache, version is too old!")

########################################################################
##
## CxxFindSource: given a source file name and a directory list,
## searches the directory list for the given source file.  Returns
## the full pathname of the located file.
##
## CxxFindHeader: given a source file, an include directive, and a
## directory list, searches the directory list for the given header
## file.  Returns the full pathname of the located file.
##
## Of course, CxxFindSource and CxxFindHeader cannot find a source
## file that has not been created yet.  This can cause dependency
## problems.  So the function CreateStubHeader can be used to create
## a file that CxxFindSource or CxxFindHeader can subsequently find.
##
########################################################################

def CxxFindSource(name, ipath):
    for dir in ipath:
        if (dir == "."): full = name
        else: full = dir + "/" + name
        if GetTimestamp(full) > 0: return full
    exit("Could not find source file: "+name)

def CxxFindHeader(srcfile, incfile, ipath):
    if (incfile.startswith(".")):
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
        if GetTimestamp(full) > 0: return full
        return 0
    else:
        for dir in ipath:
            full = dir + "/" + incfile
            if GetTimestamp(full) > 0: return full
        return 0

def JavaFindClasses(impspec, clspath):
    path = clspath + '/' + impspec.replace('.', '/') + '.class'
    if '*' in path:
        return glob.glob(path)
    else:
        return [path]

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
    if (srcfile in CxxDependencyCache):
        return CxxDependencyCache[srcfile]
    if (ignore.count(srcfile)): return []
    dep = {}
    dep[srcfile] = 1
    includes = CxxGetIncludes(srcfile)
    for include in includes:
        header = CxxFindHeader(srcfile, include, ipath)
        if (header!=0):
            if (ignore.count(header)==0):
                hdeps = CxxCalcDependencies(header, ipath, [srcfile]+ignore)
                for x in hdeps: dep[x] = 1
    result = list(dep.keys())
    CxxDependencyCache[srcfile] = result
    return result

global JavaDependencyCache
JavaDependencyCache = {}

def JavaCalcDependencies(srcfile, clspath):
    if srcfile in JavaDependencyCache:
        return JavaDependencyCache[srcfile]

    deps = set((srcfile,))
    JavaDependencyCache[srcfile] = deps

    imports = JavaGetImports(srcfile)
    for impspec in imports:
        for cls in JavaFindClasses(impspec, clspath):
            deps.add(cls)
    return deps

########################################################################
##
## Registry Key Handling
##
## Of course, these routines will fail if you call them on a
## non win32 platform.  If you use them on a win64 platform, they
## will look in the win32 private hive first, then look in the
## win64 hive.
##
########################################################################

if sys.platform == "win32":
    # Note: not supported on cygwin.
    if sys.version_info >= (3, 0):
        import winreg
    else:
        import _winreg as winreg

def TryRegistryKey(path):
    try:
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, path, 0, winreg.KEY_READ)
        return key
    except: pass
    try:
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, path, 0, winreg.KEY_READ | 256)
        return key
    except: pass
    return 0

def ListRegistryKeys(path):
    result=[]
    index=0
    key = TryRegistryKey(path)
    if (key != 0):
        try:
            while (1):
                result.append(winreg.EnumKey(key, index))
                index = index + 1
        except: pass
        winreg.CloseKey(key)
    return result

def ListRegistryValues(path):
    result = []
    index = 0
    key = TryRegistryKey(path)
    if (key != 0):
        try:
            while (1):
                result.append(winreg.EnumValue(key, index)[0])
                index = index + 1
        except: pass
        winreg.CloseKey(key)
    return result

def GetRegistryKey(path, subkey, override64=True):
    if (host_64 and override64):
        path = path.replace("SOFTWARE\\", "SOFTWARE\\Wow6432Node\\")
    k1=0
    key = TryRegistryKey(path)
    if (key != 0):
        try:
            k1, k2 = winreg.QueryValueEx(key, subkey)
        except: pass
        winreg.CloseKey(key)
    return k1

def GetProgramFiles():
    if ("PROGRAMFILES" in os.environ):
        return os.environ["PROGRAMFILES"]
    elif (os.path.isdir("C:\\Program Files")):
        return "C:\\Program Files"
    elif (os.path.isdir("D:\\Program Files")):
        return "D:\\Program Files"
    elif (os.path.isdir("E:\\Program Files")):
        return "E:\\Program Files"
    return 0

def GetProgramFiles_x86():
    if ("ProgramFiles(x86)" in os.environ):
        return os.environ["ProgramFiles(x86)"]
    elif (os.path.isdir("C:\\Program Files (x86)")):
        return "C:\\Program Files (x86)"
    elif (os.path.isdir("D:\\Program Files (x86)")):
        return "D:\\Program Files (x86)"
    elif (os.path.isdir("E:\\Program Files (x86)")):
        return "E:\\Program Files (x86)"
    return GetProgramFiles()

########################################################################
##
## Parsing Compiler Option Lists
##
########################################################################

def GetListOption(opts, prefix):
    res=[]
    for x in opts:
        if (x.startswith(prefix)):
            res.append(x[len(prefix):])
    return res

def GetValueOption(opts, prefix):
    for x in opts:
        if (x.startswith(prefix)):
            return x[len(prefix):]
    return 0

def GetOptimizeOption(opts):
    val = GetValueOption(opts, "OPT:")
    if (val == 0):
        return OPTIMIZE
    return val

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
        if sys.version_info >= (3, 0):
            dsthandle = open(wfile, "w", newline='')
        else:
            dsthandle = open(wfile, "w")
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

def ConditionalWriteFile(dest, data, newline=None):
    if newline is not None:
        data = data.replace('\r\n', '\n')
        data = data.replace('\r', '\n')
        data = data.replace('\n', newline)

    try:
        rfile = open(dest, 'r')
        contents = rfile.read(-1)
        rfile.close()
    except:
        contents = 0

    if contents != data:
        if VERBOSE:
            print("Writing %s" % (dest))
        sys.stdout.flush()
        WriteFile(dest, data)

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

def CreateFile(file):
    if (os.path.exists(file)==0):
        WriteFile(file, "")

########################################################################
#
# Create the panda build tree.
#
########################################################################

def MakeBuildTree():
    MakeDirectory(OUTPUTDIR)
    MakeDirectory(OUTPUTDIR + "/bin")
    MakeDirectory(OUTPUTDIR + "/lib")
    MakeDirectory(OUTPUTDIR + "/tmp")
    MakeDirectory(OUTPUTDIR + "/etc")
    MakeDirectory(OUTPUTDIR + "/plugins")
    MakeDirectory(OUTPUTDIR + "/include")
    MakeDirectory(OUTPUTDIR + "/models")
    MakeDirectory(OUTPUTDIR + "/models/audio")
    MakeDirectory(OUTPUTDIR + "/models/audio/sfx")
    MakeDirectory(OUTPUTDIR + "/models/icons")
    MakeDirectory(OUTPUTDIR + "/models/maps")
    MakeDirectory(OUTPUTDIR + "/models/misc")
    MakeDirectory(OUTPUTDIR + "/models/gui")
    MakeDirectory(OUTPUTDIR + "/pandac")
    MakeDirectory(OUTPUTDIR + "/pandac/input")
    MakeDirectory(OUTPUTDIR + "/panda3d")

    if GetTarget() == 'darwin':
        MakeDirectory(OUTPUTDIR + "/Frameworks")

    elif GetTarget() == 'android':
        MakeDirectory(OUTPUTDIR + "/classes")

########################################################################
#
# Make sure that you are in the root of the panda tree.
#
########################################################################

def CheckPandaSourceTree():
    dir = os.getcwd()
    if ((os.path.exists(os.path.join(dir, "makepanda/makepanda.py"))==0) or
        (os.path.exists(os.path.join(dir, "dtool", "src", "dtoolbase", "dtoolbase.h"))==0) or
        (os.path.exists(os.path.join(dir, "panda", "src", "pandabase", "pandabase.h"))==0)):
        exit("Current directory is not the root of the panda tree.")

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
    if (THIRDPARTYBASE != None):
        return THIRDPARTYBASE

    THIRDPARTYBASE = "thirdparty"
    if "MAKEPANDA_THIRDPARTY" in os.environ:
        THIRDPARTYBASE = os.environ["MAKEPANDA_THIRDPARTY"]

    return THIRDPARTYBASE

def GetThirdpartyDir():
    """Returns the thirdparty directory for the target platform,
    ie. thirdparty/win-libs-vc10/.  May return None in the future."""
    global THIRDPARTYDIR
    if THIRDPARTYDIR != None:
        return THIRDPARTYDIR

    base = GetThirdpartyBase()
    target = GetTarget()
    target_arch = GetTargetArch()

    if (target == 'windows'):
        vc = str(SDK["MSVC_VERSION"][0])

        if target_arch == 'x64':
            THIRDPARTYDIR = base + "/win-libs-vc" + vc + "-x64/"
        else:
            THIRDPARTYDIR = base + "/win-libs-vc" + vc + "/"

    elif (target == 'darwin'):
        # OSX thirdparty binaries are universal, where possible.
        THIRDPARTYDIR = base + "/darwin-libs-a/"

    elif (target == 'linux'):
        if (target_arch.startswith("arm")):
            THIRDPARTYDIR = base + "/linux-libs-arm/"
        elif (target_arch in ("x86_64", "amd64")):
            THIRDPARTYDIR = base + "/linux-libs-x64/"
        else:
            THIRDPARTYDIR = base + "/linux-libs-a/"

    elif (target == 'freebsd'):
        if (target_arch.startswith("arm")):
            THIRDPARTYDIR = base + "/freebsd-libs-arm/"
        elif (target_arch in ("x86_64", "amd64")):
            THIRDPARTYDIR = base + "/freebsd-libs-x64/"
        else:
            THIRDPARTYDIR = base + "/freebsd-libs-a/"

    elif (target == 'android'):
        THIRDPARTYDIR = GetThirdpartyBase()+"/android-libs-%s/" % (GetTargetArch())

    else:
        Warn("Unsupported platform:", target)
        return

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

def SetDebugDependencies(dd = True):
    global DEBUG_DEPENDENCIES
    DEBUG_DEPENDENCIES = dd

def GetLinkAllStatic():
    return LINK_ALL_STATIC

def SetLinkAllStatic(val = True):
    global LINK_ALL_STATIC
    LINK_ALL_STATIC = val

def UnsetLinkAllStatic():
    global LINK_ALL_STATIC
    LINK_ALL_STATIC = False

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
## DTOOL/PRC Option value override
##
## This facility enables makepanda to keep a list of parameters
## overriden by the command line.
##
########################################################################

OVERRIDES_LIST={}

def AddOverride(spec):
    if (spec.find("=")==-1):return
    pair = spec.split("=",1)
    OVERRIDES_LIST[pair[0]] = pair[1]

def OverrideValue(parameter, value):
    if parameter in OVERRIDES_LIST:
        print("Overriding value of key \"" + parameter + "\" with value \"" + OVERRIDES_LIST[parameter] + "\"")
        return OVERRIDES_LIST[parameter]
    else:
        return value

########################################################################
##
## These functions are for libraries which use pkg-config.
##
########################################################################

def PkgConfigHavePkg(pkgname, tool = "pkg-config"):
    """Returns a bool whether the pkg-config package is installed."""

    if (sys.platform == "win32" or CrossCompiling() or not LocateBinary(tool)):
        return False
    if (tool == "pkg-config"):
        handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --modversion " + pkgname)
    else:
        return bool(LocateBinary(tool) != None)
    result = handle.read().strip()
    returnval = handle.close()
    if returnval != None and returnval != 0:
        return False
    return bool(len(result) > 0)

def PkgConfigGetLibs(pkgname, tool = "pkg-config"):
    """Returns a list of libs for the package, prefixed by -l."""

    if (sys.platform == "win32" or CrossCompiling() or not LocateBinary(tool)):
        return []
    if (tool == "pkg-config"):
        handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --libs-only-l " + pkgname)
    elif (tool == "fltk-config"):
        handle = os.popen(LocateBinary("fltk-config") + " --ldstaticflags")
    else:
        handle = os.popen(LocateBinary(tool) + " --libs")
    result = handle.read().strip()
    handle.close()
    libs = []

    # Walk through the result arguments carefully.  Look for -lname as
    # well as -framework name.
    r = result.split(' ')
    ri = 0
    while ri < len(r):
        l = r[ri]
        if l.startswith("-l") or l.startswith("/"):
            libs.append(l)
        elif l == '-framework':
            libs.append(l)
            ri += 1
            libs.append(r[ri])
        ri += 1

    return libs

def PkgConfigGetIncDirs(pkgname, tool = "pkg-config"):
    """Returns a list of includes for the package, NOT prefixed by -I."""

    if (sys.platform == "win32" or CrossCompiling() or not LocateBinary(tool)):
        return []
    if (tool == "pkg-config"):
        handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --cflags-only-I " + pkgname)
    else:
        handle = os.popen(LocateBinary(tool) + " --cflags")
    result = handle.read().strip()
    if len(result) == 0: return []
    handle.close()
    dirs = []
    for opt in result.split(" "):
        if (opt.startswith("-I")):
            inc_dir = opt.replace("-I", "").replace("\"", "").strip()
            # Hack for ODE, otherwise -S/usr/include gets added to interrogate
            if inc_dir != '/usr/include' and inc_dir != '/usr/include/':
                dirs.append(inc_dir)
    return dirs

def PkgConfigGetLibDirs(pkgname, tool = "pkg-config"):
    """Returns a list of library paths for the package, NOT prefixed by -L."""

    if (sys.platform == "win32" or CrossCompiling() or not LocateBinary(tool)):
        return []
    if (tool == "pkg-config"):
        handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --libs-only-L " + pkgname)
    elif (tool == "wx-config" or tool == "ode-config"):
        return []
    else:
        handle = os.popen(LocateBinary(tool) + " --ldflags")
    result = handle.read().strip()
    handle.close()
    if len(result) == 0: return []
    libs = []
    for l in result.split(" "):
        if (l.startswith("-L")):
            libs.append(l.replace("-L", "").replace("\"", "").strip())
    return libs

def PkgConfigGetDefSymbols(pkgname, tool = "pkg-config"):
    """Returns a dictionary of preprocessor definitions."""

    if (sys.platform == "win32" or CrossCompiling() or not LocateBinary(tool)):
        return []
    if (tool == "pkg-config"):
        handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --cflags " + pkgname)
    else:
        handle = os.popen(LocateBinary(tool) + " --cflags")
    result = handle.read().strip()
    handle.close()
    if len(result) == 0: return {}
    defs = {}
    for l in result.split(" "):
        if (l.startswith("-D")):
            d = l.replace("-D", "").replace("\"", "").strip().split("=")

            if d[0] in ('NDEBUG', '_DEBUG'):
                # Setting one of these flags by accident could cause serious harm.
                if GetVerbose():
                    print("Ignoring %s flag provided by %s" % (l, tool))
            elif len(d) == 1:
                defs[d[0]] = ""
            else:
                defs[d[0]] = d[1]
    return defs

def PkgConfigEnable(opt, pkgname, tool = "pkg-config"):
    """Adds the libraries and includes to IncDirectory, LibName and LibDirectory."""
    for i in PkgConfigGetIncDirs(pkgname, tool):
        IncDirectory(opt, i)
    for i in PkgConfigGetLibDirs(pkgname, tool):
        LibDirectory(opt, i)
    for i in PkgConfigGetLibs(pkgname, tool):
        LibName(opt, i)
    for i, j in PkgConfigGetDefSymbols(pkgname, tool).items():
        DefSymbol(opt, i, j)

def LocateLibrary(lib, lpath=[], prefer_static=False):
    """Searches for the library in the search path, returning its path if found,
    or None if it was not found."""
    target = GetTarget()

    if prefer_static and target != 'windows':
        for dir in lpath:
            if os.path.isfile(os.path.join(dir, 'lib%s.a' % lib)):
                return os.path.join(dir, 'lib%s.a' % lib)

    for dir in lpath:
        if target == 'darwin' and os.path.isfile(os.path.join(dir, 'lib%s.dylib' % lib)):
            return os.path.join(dir, 'lib%s.dylib' % lib)
        elif target != 'darwin' and os.path.isfile(os.path.join(dir, 'lib%s.so' % lib)):
            return os.path.join(dir, 'lib%s.so' % lib)
        elif os.path.isfile(os.path.join(dir, 'lib%s.a' % lib)):
            return os.path.join(dir, 'lib%s.a' % lib)

    return None

def SystemLibraryExists(lib):
    result = LocateLibrary(lib, SYS_LIB_DIRS)
    if result is not None:
        return True

    if GetHost() == "android" and GetTarget() == "android":
        return ('lib%s.so' % lib) in ANDROID_SYS_LIBS

    return False

def ChooseLib(libs, thirdparty=None):
    """ Chooses a library from the parameters, in order of preference. Returns the first if none of them were found. """

    lpath = []
    if thirdparty is not None:
        lpath.append(os.path.join(GetThirdpartyDir(), thirdparty.lower(), "lib"))
    lpath += SYS_LIB_DIRS

    for l in libs:
        libname = l
        if l.startswith("lib"):
            libname = l[3:]
        if LocateLibrary(libname, lpath):
            return libname

    if len(libs) > 0:
        if VERBOSE:
            print(ColorText("cyan", "Couldn't find any of the libraries " + ", ".join(libs)))
        return libs[0]

def SmartPkgEnable(pkg, pkgconfig = None, libs = None, incs = None, defs = None, framework = None, target_pkg = None, tool = "pkg-config", thirdparty_dir = None):
    global PKG_LIST_ALL
    if (pkg in PkgListGet() and PkgSkip(pkg)):
        return
    if (target_pkg == "" or target_pkg == None):
        target_pkg = pkg
    if (pkgconfig == ""):
        pkgconfig = None
    if (framework == ""):
        framework = None
    if (libs == None or libs == ""):
        libs = ()
    elif (isinstance(libs, str)):
        libs = (libs, )
    if (incs == None or incs == ""):
        incs = ()
    elif (isinstance(incs, str)):
        incs = (incs, )
    if (defs == None or defs == "" or len(defs) == 0):
        defs = {}
    elif (isinstance(incs, str)):
        defs = {defs : ""}
    elif (isinstance(incs, list) or isinstance(incs, tuple) or isinstance(incs, set)):
        olddefs = defs
        defs = {}
        for d in olddefs:
            defs[d] = ""

    custom_loc = PkgHasCustomLocation(pkg)

    # Determine the location of the thirdparty directory.
    if not thirdparty_dir:
        thirdparty_dir = pkg.lower()
    pkg_dir = os.path.join(GetThirdpartyDir(), thirdparty_dir)

    # First check if the library can be found in the thirdparty directory.
    if not custom_loc and os.path.isdir(pkg_dir):
        if framework and os.path.isdir(os.path.join(pkg_dir, framework + ".framework")):
            FrameworkDirectory(target_pkg, pkg_dir)
            LibName(target_pkg, "-framework " + framework)
            return

        if os.path.isdir(os.path.join(pkg_dir, "include")):
            IncDirectory(target_pkg, os.path.join(pkg_dir, "include"))

            # Handle cases like freetype2 where the include dir is a subdir under "include"
            for i in incs:
                if os.path.isdir(os.path.join(pkg_dir, "include", i)):
                    IncDirectory(target_pkg, os.path.join(pkg_dir, "include", i))

        lpath = [os.path.join(pkg_dir, "lib")]

        if not PkgSkip("PYTHON"):
            py_lib_dir = os.path.join(pkg_dir, "lib", SDK["PYTHONVERSION"])
            if os.path.isdir(py_lib_dir):
                lpath.append(py_lib_dir)

        # TODO: check for a .pc file in the lib/pkgconfig/ dir
        if (tool != None and os.path.isfile(os.path.join(pkg_dir, "bin", tool))):
            tool = os.path.join(pkg_dir, "bin", tool)
            for i in PkgConfigGetLibs(None, tool):
                if i.startswith('-l'):
                    # To make sure we don't pick up the system copy, write out
                    # the full path instead.
                    libname = i[2:]
                    location = LocateLibrary(libname, lpath, prefer_static=True)
                    if location is not None:
                        LibName(target_pkg, location)
                    else:
                        print(GetColor("cyan") + "Couldn't find library lib" + libname + " in thirdparty directory " + pkg.lower() + GetColor())
                        LibName(target_pkg, i)
                else:
                    LibName(target_pkg, i)
            for i, j in PkgConfigGetDefSymbols(None, tool).items():
                DefSymbol(target_pkg, i, j)
            return

        # Now search for the libraries in the package's lib directories.
        for l in libs:
            libname = l
            if l.startswith("lib"):
                libname = l[3:]

            location = LocateLibrary(libname, lpath, prefer_static=True)
            if location is not None:
                # If it's a .so or .dylib we may have changed it and copied it to the built/lib dir.
                if location.endswith('.so') or location.endswith('.dylib'):
                    location = os.path.join(GetOutputDir(), "lib", os.path.basename(location))
                LibName(target_pkg, location)
            else:
                # This is for backward compatibility - in the thirdparty dir,
                # we kept some libs with "panda" prefix, like libpandatiff.
                location = LocateLibrary("panda" + libname, lpath, prefer_static=True)
                if location is not None:
                    if location.endswith('.so') or location.endswith('.dylib'):
                        location = os.path.join(GetOutputDir(), "lib", os.path.basename(location))
                    LibName(target_pkg, location)
                else:
                    print(GetColor("cyan") + "Couldn't find library lib" + libname + " in thirdparty directory " + thirdparty_dir + GetColor())

        for d, v in defs.values():
            DefSymbol(target_pkg, d, v)
        return

    elif not custom_loc and GetHost() == "darwin" and framework != None:
        prefix = SDK["MACOSX"]
        if (os.path.isdir(prefix + "/Library/Frameworks/%s.framework" % framework) or
            os.path.isdir(prefix + "/System/Library/Frameworks/%s.framework" % framework) or
            os.path.isdir(prefix + "/Developer/Library/Frameworks/%s.framework" % framework) or
            os.path.isdir(prefix + "/Users/%s/System/Library/Frameworks/%s.framework" % (getpass.getuser(), framework))):
            LibName(target_pkg, "-framework " + framework)
            for d, v in defs.values():
                DefSymbol(target_pkg, d, v)
            return

        elif VERBOSE:
            print(ColorText("cyan", "Couldn't find the framework %s" % (framework)))

    elif not custom_loc and LocateBinary(tool) != None and (tool != "pkg-config" or pkgconfig != None):
        if (isinstance(pkgconfig, str) or tool != "pkg-config"):
            if (PkgConfigHavePkg(pkgconfig, tool)):
                return PkgConfigEnable(target_pkg, pkgconfig, tool)
        else:
            have_all_pkgs = True
            for pc in pkgconfig:
                if (PkgConfigHavePkg(pc, tool)):
                    PkgConfigEnable(target_pkg, pc, tool)
                else:
                    have_all_pkgs = False
            if (have_all_pkgs):
                return

    if not custom_loc and pkgconfig is not None and not libs:
        # pkg-config is all we can do, abort if it wasn't found.
        if pkg in PkgListGet():
            Warn("Could not locate pkg-config package %s, excluding from build" % (pkgconfig))
            PkgDisable(pkg)
        else:
            Error("Could not locate pkg-config package %s, aborting build" % (pkgconfig))

    else:
        # Okay, our pkg-config attempts failed. Let's try locating the libs by ourselves.
        have_pkg = True
        for l in libs:
            libname = l
            if l.startswith("lib"):
                libname = l[3:]

            if custom_loc:
                # Try searching in the package's LibDirectories.
                lpath = [dir for ppkg, dir in LIBDIRECTORIES if pkg == ppkg]
                location = LocateLibrary(libname, lpath)
                if location is not None:
                    LibName(target_pkg, location)
                else:
                    have_pkg = False
                    print(GetColor("cyan") + "Couldn't find library lib" + libname + GetColor())

            elif SystemLibraryExists(libname):
                # It exists in a system library directory.
                LibName(target_pkg, "-l" + libname)
            else:
                # Try searching in the package's LibDirectories.
                lpath = [dir for ppkg, dir in LIBDIRECTORIES if pkg == ppkg or ppkg == "ALWAYS"]
                location = LocateLibrary(libname, lpath)
                if location is not None:
                    LibName(target_pkg, "-l" + libname)
                else:
                    have_pkg = False
                    if VERBOSE or custom_loc:
                        print(GetColor("cyan") + "Couldn't find library lib" + libname + GetColor())

        # Determine which include directories to look in.
        incdirs = []
        if not custom_loc:
            incdirs += list(SYS_INC_DIRS)

        for ppkg, pdir in INCDIRECTORIES[:]:
            if pkg == ppkg or (ppkg == "ALWAYS" and not custom_loc):
                incdirs.append(pdir)
                if custom_loc and pkg != target_pkg:
                    IncDirectory(target_pkg, pdir)

        # The incs list contains both subdirectories to explicitly add to
        # the include path and header files to check the existence of.
        for i in incs:
            incdir = None
            for dir in incdirs:
                if len(glob.glob(os.path.join(dir, i))) > 0:
                    incdir = sorted(glob.glob(os.path.join(dir, i)))[-1]

            # Note: It's possible to specify a file instead of a dir, for the sake of checking if it exists.
            if incdir is None and (i.endswith('/Dense') or i.endswith(".h")):
                have_pkg = False
                if VERBOSE or custom_loc:
                    print(GetColor("cyan") + "Couldn't find header file " + i + GetColor())

            if incdir is not None and os.path.isdir(incdir):
                IncDirectory(target_pkg, incdir)

        if not have_pkg:
            if custom_loc:
                Error("Could not locate thirdparty package %s in specified directory, aborting build" % (pkg.lower()))
            elif pkg in PkgListGet():
                Warn("Could not locate thirdparty package %s, excluding from build" % (pkg.lower()))
                PkgDisable(pkg)
            else:
                Error("Could not locate thirdparty package %s, aborting build" % (pkg.lower()))

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

def GetSdkDir(sdkname, sdkkey = None):
    # Returns the default SDK directory. If it exists,
    # and sdkkey is not None, it is put in SDK[sdkkey].
    # Note: return value may not be an existing path.
    sdkbase = "sdks"
    if "MAKEPANDA_SDKS" in os.environ:
        sdkbase = os.environ["MAKEPANDA_SDKS"]

    sdir = sdkbase[:]
    target = GetTarget()
    target_arch = GetTargetArch()
    if target == 'windows':
        if target_arch == 'x64':
            sdir += "/win64"
        else:
            sdir += "/win32"
    elif target == 'linux':
        sdir += "/linux"
        sdir += platform.architecture()[0][:2]
    elif target == 'darwin':
        sdir += "/macosx"
    sdir += "/" + sdkname

    # If it does not exist, try the old location.
    if (not os.path.isdir(sdir)):
        sdir = sdkbase + "/" + sdir
        if (target == 'linux'):
            sdir += "-linux"
            sdir += platform.architecture()[0][:2]
        elif (target == "darwin"):
            sdir += "-osx"

    if (sdkkey and os.path.isdir(sdir)):
        SDK[sdkkey] = sdir

    return sdir

def SdkLocateDirectX( strMode = 'default' ):
    if (GetHost() != "windows"): return
    if strMode == 'default':
        GetSdkDir("directx9", "DX9")
        if ("DX9" not in SDK):
            strMode = 'latest'
    if strMode == 'latest':
        ## We first try to locate the August SDK in 64 bits, then 32.
        if ("DX9" not in SDK):
            dir = GetRegistryKey("SOFTWARE\\Wow6432Node\\Microsoft\\DirectX\\Microsoft DirectX SDK (June 2010)", "InstallPath")
            if (dir != 0):
                print("Using DirectX SDK June 2010")
                SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
                SDK["GENERIC_DXERR_LIBRARY"] = 1;
        if ("DX9" not in SDK):
            dir = GetRegistryKey("SOFTWARE\\Microsoft\\DirectX\\Microsoft DirectX SDK (June 2010)", "InstallPath")
            if (dir != 0):
                print("Using DirectX SDK June 2010")
                SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
                SDK["GENERIC_DXERR_LIBRARY"] = 1;
        if ("DX9" not in SDK):
            dir = "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)"
            if os.path.isdir(dir):
                print("Using DirectX SDK June 2010")
                SDK["DX9"] = dir
                SDK["GENERIC_DXERR_LIBRARY"] = 1
        if ("DX9" not in SDK):
            dir = "C:/Program Files/Microsoft DirectX SDK (June 2010)"
            if os.path.isdir(dir):
                print("Using DirectX SDK June 2010")
                SDK["DX9"] = dir
                SDK["GENERIC_DXERR_LIBRARY"] = 1
        if ("DX9" not in SDK):
            dir = GetRegistryKey("SOFTWARE\\Wow6432Node\\Microsoft\\DirectX\\Microsoft DirectX SDK (August 2009)", "InstallPath")
            if (dir != 0):
                print("Using DirectX SDK Aug 2009")
                SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
                SDK["GENERIC_DXERR_LIBRARY"] = 1;
        if ("DX9" not in SDK):
            dir = GetRegistryKey("SOFTWARE\\Microsoft\\DirectX\\Microsoft DirectX SDK (August 2009)", "InstallPath")
            if (dir != 0):
                print("Using DirectX SDK Aug 2009")
                SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
                SDK["GENERIC_DXERR_LIBRARY"] = 1;
        if ("DX9" not in SDK):
            ## Try to locate the key within the "new" March 2009 location in the registry (yecch):
            dir = GetRegistryKey("SOFTWARE\\Microsoft\\DirectX\\Microsoft DirectX SDK (March 2009)", "InstallPath")
            if (dir != 0):
                print("Using DirectX SDK March 2009")
                SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
        archStr = GetTargetArch()
        if ("DX9" not in SDK):
            uninstaller = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
            for subdir in ListRegistryKeys(uninstaller):
                if (subdir[0]=="{"):
                    dir = GetRegistryKey(uninstaller+"\\"+subdir, "InstallLocation")
                    if (dir != 0):
                        if (("DX9" not in SDK) and
                            (os.path.isfile(dir+"\\Include\\d3d9.h")) and
                            (os.path.isfile(dir+"\\Include\\d3dx9.h")) and
                            (os.path.isfile(dir+"\\Include\\dxsdkver.h")) and
                            (os.path.isfile(dir+"\\Lib\\" + archStr + "\\d3d9.lib")) and
                            (os.path.isfile(dir+"\\Lib\\" + archStr + "\\d3dx9.lib"))):
                            SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
        if ("DX9" not in SDK):
            return

    elif strMode == 'jun2010':
        if ("DX9" not in SDK):
            dir = GetRegistryKey("SOFTWARE\\Wow6432Node\\Microsoft\\DirectX\\Microsoft DirectX SDK (June 2010)", "InstallPath")
            if (dir != 0):
                SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
                SDK["GENERIC_DXERR_LIBRARY"] = 1;
        if ("DX9" not in SDK):
            dir = GetRegistryKey("SOFTWARE\\Microsoft\\DirectX\\Microsoft DirectX SDK (June 2010)", "InstallPath")
            if (dir != 0):
                SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
                SDK["GENERIC_DXERR_LIBRARY"] = 1;
        if ("DX9" not in SDK):
            dir = "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)"
            if os.path.isdir(dir):
                SDK["DX9"] = dir
                SDK["GENERIC_DXERR_LIBRARY"] = 1
        if ("DX9" not in SDK):
            dir = "C:/Program Files/Microsoft DirectX SDK (June 2010)"
            if os.path.isdir(dir):
                SDK["DX9"] = dir
                SDK["GENERIC_DXERR_LIBRARY"] = 1
        if ("DX9" not in SDK):
            exit("Couldn't find DirectX June2010 SDK")
        else:
            print("Found DirectX SDK June 2010")
    elif strMode == 'aug2009':
        if ("DX9" not in SDK):
            dir = GetRegistryKey("SOFTWARE\\Wow6432Node\\Microsoft\\DirectX\\Microsoft DirectX SDK (August 2009)", "InstallPath")
            if (dir != 0):
                print("Found DirectX SDK Aug 2009")
                SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
                SDK["GENERIC_DXERR_LIBRARY"] = 1;
        if ("DX9" not in SDK):
            dir = GetRegistryKey("SOFTWARE\\Microsoft\\DirectX\\Microsoft DirectX SDK (August 2009)", "InstallPath")
            if (dir != 0):
                print("Found DirectX SDK Aug 2009")
                SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
                SDK["GENERIC_DXERR_LIBRARY"] = 1;
        if ("DX9" not in SDK):
            exit("Couldn't find DirectX Aug 2009 SDK")
    elif strMode == 'mar2009':
        if ("DX9" not in SDK):
            ## Try to locate the key within the "new" March 2009 location in the registry (yecch):
            dir = GetRegistryKey("SOFTWARE\\Microsoft\\DirectX\\Microsoft DirectX SDK (March 2009)", "InstallPath")
            if (dir != 0):
                print("Found DirectX SDK March 2009")
                SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
        if ("DX9" not in SDK):
            exit("Couldn't find DirectX March 2009 SDK")
    elif strMode == 'aug2006':
        archStr = GetTargetArch()
        if ("DX9" not in SDK):
            uninstaller = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
            for subdir in ListRegistryKeys(uninstaller):
                if (subdir[0]=="{"):
                    dir = GetRegistryKey(uninstaller+"\\"+subdir, "InstallLocation")
                    if (dir != 0):
                        if (("DX9" not in SDK) and
                            (os.path.isfile(dir+"\\Include\\d3d9.h")) and
                            (os.path.isfile(dir+"\\Include\\d3dx9.h")) and
                            (os.path.isfile(dir+"\\Include\\dxsdkver.h")) and
                            (os.path.isfile(dir+"\\Lib\\" + archStr + "\\d3d9.lib")) and
                            (os.path.isfile(dir+"\\Lib\\" + archStr + "\\d3dx9.lib"))):
                            SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
        if ("DX9" not in SDK):
            exit("Couldn't find a DirectX Aug 2006 SDK")
    if ("DX9" in SDK):
        SDK["DIRECTCAM"] = SDK["DX9"]

def SdkLocateMaya():
    for (ver, key) in MAYAVERSIONINFO:
        if (PkgSkip(ver)==0 and ver not in SDK):
            GetSdkDir(ver.lower().replace("x",""), ver)
            if (not ver in SDK):
                if (GetHost() == "windows"):
                    for dev in ["Alias|Wavefront","Alias","Autodesk"]:
                        fullkey="SOFTWARE\\"+dev+"\\Maya\\"+key+"\\Setup\\InstallPath"
                        res = GetRegistryKey(fullkey, "MAYA_INSTALL_LOCATION", override64=False)
                        if (res != 0):
                            res = res.replace("\\", "/").rstrip("/")
                            SDK[ver] = res
                elif (GetHost() == "darwin"):
                    ddir = "/Applications/Autodesk/maya"+key
                    if (os.path.isdir(ddir)): SDK[ver] = ddir
                else:
                    if (GetTargetArch() in ("x86_64", "amd64")):
                        ddir1 = "/usr/autodesk/maya"+key+"-x64"
                        ddir2 = "/usr/aw/maya"+key+"-x64"
                    else:
                        ddir1 = "/usr/autodesk/maya"+key
                        ddir2 = "/usr/aw/maya"+key

                    if (os.path.isdir(ddir1)):   SDK[ver] = ddir1
                    elif (os.path.isdir(ddir2)): SDK[ver] = ddir2

def SdkLocateMax():
    if (GetHost() != "windows"): return
    for version,key1,key2,subdir in MAXVERSIONINFO:
        if (PkgSkip(version)==0):
            if (version not in SDK):
                GetSdkDir("maxsdk"+version.lower()[3:], version)
                GetSdkDir("maxsdk"+version.lower()[3:], version+"CS")
                if (not version in SDK):
                    top = GetRegistryKey(key1,key2)
                    if (top != 0):
                        SDK[version] = top + "maxsdk"
                        if (os.path.isdir(top + "\\" + subdir)!=0):
                            SDK[version+"CS"] = top + subdir

def SdkLocatePython(prefer_thirdparty_python=False):
    if PkgSkip("PYTHON"):
        # We're not compiling with Python support.  We still need to set this
        # in case we want to run any scripts that use Python, though.
        SDK["PYTHONEXEC"] = os.path.realpath(sys.executable)
        return

    abiflags = getattr(sys, 'abiflags', '')

    if GetTarget() == 'windows':
        sdkdir = GetThirdpartyBase() + "/win-python"

        if sys.version_info >= (3, 0):
            # Python 3 build...
            sdkdir += "%d.%d" % sys.version_info[:2]

        if GetOptimize() <= 2:
            sdkdir += "-dbg"
        if GetTargetArch() == 'x64':
            sdkdir += "-x64"

        SDK["PYTHON"] = sdkdir
        SDK["PYTHONEXEC"] = SDK["PYTHON"].replace('\\', '/') + "/python"
        if (GetOptimize() <= 2):
            SDK["PYTHONEXEC"] += "_d.exe"
        else:
            SDK["PYTHONEXEC"] += ".exe"

        if (not os.path.isfile(SDK["PYTHONEXEC"])):
            exit("Could not find %s!" % SDK["PYTHONEXEC"])

        # Determine which version it is by checking which dll is in the directory.
        if (GetOptimize() <= 2):
            py_dlls = glob.glob(SDK["PYTHON"] + "/python[0-9][0-9]_d.dll")
        else:
            py_dlls = glob.glob(SDK["PYTHON"] + "/python[0-9][0-9].dll")

        if len(py_dlls) == 0:
            exit("Could not find the Python dll in %s." % (SDK["PYTHON"]))
        elif len(py_dlls) > 1:
            exit("Found multiple Python dlls in %s." % (SDK["PYTHON"]))

        py_dll = os.path.basename(py_dlls[0])
        ver = py_dll[6] + "." + py_dll[7]

        SDK["PYTHONVERSION"] = "python" + ver
        os.environ["PYTHONHOME"] = SDK["PYTHON"]

        if sys.version[:3] != ver:
            Warn("running makepanda with Python %s, but building Panda3D with Python %s." % (sys.version[:3], ver))

    elif CrossCompiling() or (prefer_thirdparty_python and os.path.isdir(os.path.join(GetThirdpartyDir(), "python"))):
        tp_python = os.path.join(GetThirdpartyDir(), "python")

        if GetTarget() == 'darwin':
            py_libs = glob.glob(tp_python + "/lib/libpython[0-9].[0-9].dylib")
        else:
            py_libs = glob.glob(tp_python + "/lib/libpython[0-9].[0-9].so")

        if len(py_libs) == 0:
            py_libs = glob.glob(tp_python + "/lib/libpython[0-9].[0-9].a")

        if len(py_libs) == 0:
            exit("Could not find the Python library in %s." % (tp_python))
        elif len(py_libs) > 1:
            exit("Found multiple Python libraries in %s." % (tp_python))

        py_lib = os.path.basename(py_libs[0])
        SDK["PYTHONVERSION"] = "python" + py_lib[9] + "." + py_lib[11]
        SDK["PYTHONEXEC"] = tp_python + "/bin/" + SDK["PYTHONVERSION"]
        SDK["PYTHON"] = tp_python + "/include/" + SDK["PYTHONVERSION"]

    elif GetTarget() == 'darwin' and not PkgHasCustomLocation("PYTHON"):
        # On macOS, search for the Python framework directory matching the
        # version number of our current Python version.
        sysroot = SDK.get("MACOSX", "")
        version = sysconfig.get_python_version()

        py_fwx = "{0}/System/Library/Frameworks/Python.framework/Versions/{1}".format(sysroot, version)

        if not os.path.exists(py_fwx):
            # Fall back to looking on the system.
            py_fwx = "/Library/Frameworks/Python.framework/Versions/" + version

        if not os.path.exists(py_fwx):
            exit("Could not locate Python installation at %s" % (py_fwx))

        SDK["PYTHON"] = py_fwx + "/Headers"
        SDK["PYTHONVERSION"] = "python" + version + abiflags
        SDK["PYTHONEXEC"] = py_fwx + "/bin/python" + version

        # Avoid choosing the one in the thirdparty package dir.
        PkgSetCustomLocation("PYTHON")
        IncDirectory("PYTHON", py_fwx + "/include")
        LibDirectory("PYTHON", py_fwx + "/lib")

    #elif GetTarget() == 'windows':
    #    SDK["PYTHON"] = os.path.dirname(sysconfig.get_python_inc())
    #    SDK["PYTHONVERSION"] = "python" + sysconfig.get_python_version()
    #    SDK["PYTHONEXEC"] = sys.executable

    else:
        SDK["PYTHON"] = sysconfig.get_python_inc()
        SDK["PYTHONVERSION"] = "python" + sysconfig.get_python_version() + abiflags
        SDK["PYTHONEXEC"] = os.path.realpath(sys.executable)

    if CrossCompiling():
        # We need a version of Python we can run.
        SDK["PYTHONEXEC"] = sys.executable
        host_version = "python" + sysconfig.get_python_version() + abiflags
        if SDK["PYTHONVERSION"] != host_version:
            exit("Host Python version (%s) must be the same as target Python version (%s)!" % (host_version, SDK["PYTHONVERSION"]))

    if GetVerbose():
        print("Using Python %s build located at %s" % (SDK["PYTHONVERSION"][6:9], SDK["PYTHON"]))
    else:
        print("Using Python %s" % (SDK["PYTHONVERSION"][6:9]))

def SdkLocateVisualStudio(version=(10,0)):
    if (GetHost() != "windows"): return

    try:
        msvcinfo = MSVCVERSIONINFO[version]
    except:
        exit("Couldn't get Visual Studio infomation with MSVC %s.%s version." % version)

    vsversion = msvcinfo["vsversion"]
    vsversion_str = "%s.%s" % vsversion
    version_str = "%s.%s" % version

    # try to use vswhere.exe
    vswhere_path = LocateBinary("vswhere.exe")
    if not vswhere_path:
        if sys.platform == 'cygwin':
            vswhere_path = "/cygdrive/c/Program Files/Microsoft Visual Studio/Installer/vswhere.exe"
        else:
            vswhere_path = "%s\\Microsoft Visual Studio\\Installer\\vswhere.exe" % GetProgramFiles()
        if not os.path.isfile(vswhere_path):
            vswhere_path = None

    if not vswhere_path:
        if sys.platform == 'cygwin':
            vswhere_path = "/cygdrive/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
        else:
            vswhere_path = "%s\\Microsoft Visual Studio\\Installer\\vswhere.exe" % GetProgramFiles_x86()
        if not os.path.isfile(vswhere_path):
            vswhere_path = None

    vsdir = 0
    if vswhere_path:
        min_vsversion = vsversion_str
        max_vsversion = "%s.%s" % (vsversion[0]+1, 0)
        vswhere_cmd = ["vswhere.exe", "-legacy", "-property", "installationPath",
            "-version", "[{},{})".format(min_vsversion, max_vsversion)]
        handle = subprocess.Popen(vswhere_cmd, executable=vswhere_path, stdout=subprocess.PIPE)
        found_paths = handle.communicate()[0].splitlines()
        if found_paths:
            vsdir = found_paths[0].decode("utf-8") + "\\"

    # try to use registry
    if (vsdir == 0):
        vsdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7", vsversion_str)
    vcdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VC7", version_str)

    if (vsdir != 0):
        SDK["VISUALSTUDIO"] = vsdir

    elif (vcdir != 0) and (vcdir[-4:] == "\\VC\\"):
        vcdir = vcdir[:-3]
        SDK["VISUALSTUDIO"] = vcdir

    elif (os.path.isfile("C:\\Program Files\\Microsoft Visual Studio %s\\VC\\bin\\cl.exe" % (vsversion_str))):
        SDK["VISUALSTUDIO"] = "C:\\Program Files\\Microsoft Visual Studio %s\\" % (vsversion_str)

    elif (os.path.isfile("C:\\Program Files (x86)\\Microsoft Visual Studio %s\\VC\\bin\\cl.exe" % (vsversion_str))):
        SDK["VISUALSTUDIO"] = "C:\\Program Files (x86)\\Microsoft Visual Studio %s\\" % (vsversion_str)

    elif "VCINSTALLDIR" in os.environ:
        vcdir = os.environ["VCINSTALLDIR"]
        if (vcdir[-3:] == "\\VC"):
            vcdir = vcdir[:-2]
        elif (vcdir[-4:] == "\\VC\\"):
            vcdir = vcdir[:-3]

        SDK["VISUALSTUDIO"] = vcdir

    else:
        exit("Couldn't find %s.  To use a different version, use the --msvc-version option." % msvcinfo["vsname"])

    SDK["MSVC_VERSION"] = version
    SDK["VISUALSTUDIO_VERSION"] = vsversion

    if GetVerbose():
        print("Using %s located at %s" % (msvcinfo["vsname"], SDK["VISUALSTUDIO"]))
    else:
        print("Using %s" % (msvcinfo["vsname"]))

    print("Using MSVC %s" % version_str)

def SdkLocateWindows(version = '7.1'):
    if GetTarget() != "windows" or GetHost() != "windows":
        return

    version = version.upper()

    if version == '10':
        version = '10.0'

    if version.startswith('10.') and version.count('.') == 1:
        # Choose the latest version of the Windows 10 SDK.
        platsdk = GetRegistryKey("SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots", "KitsRoot10")

        # Fallback in case we can't read the registry.
        if not platsdk or not os.path.isdir(platsdk):
            platsdk = "C:\\Program Files (x86)\\Windows Kits\\10\\"

        if platsdk and os.path.isdir(platsdk):
            incdirs = glob.glob(os.path.join(platsdk, 'Include', version + '.*.*'))
            max_version = ()
            for dir in incdirs:
                verstring = os.path.basename(dir)

                # Check that the important include directories exist.
                if not os.path.isdir(os.path.join(dir, 'ucrt')):
                    continue
                if not os.path.isdir(os.path.join(dir, 'shared')):
                    continue
                if not os.path.isdir(os.path.join(dir, 'um')):
                    continue
                if not os.path.isdir(os.path.join(platsdk, 'Lib', verstring, 'ucrt')):
                    continue
                if not os.path.isdir(os.path.join(platsdk, 'Lib', verstring, 'um')):
                    continue

                vertuple = tuple(map(int, verstring.split('.')))
                if vertuple > max_version:
                    version = verstring
                    max_version = vertuple

            if not max_version:
                # No suitable version found.
                platsdk = None

    elif version.startswith('10.'):
        # We chose a specific version of the Windows 10 SDK.  Verify it exists.
        platsdk = GetRegistryKey("SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots", "KitsRoot10")

        # Fallback in case we can't read the registry.
        if not platsdk or not os.path.isdir(platsdk):
            platsdk = "C:\\Program Files (x86)\\Windows Kits\\10\\"

        if version.count('.') == 2:
            version += '.0'

        if platsdk and not os.path.isdir(os.path.join(platsdk, 'Include', version)):
            platsdk = None

    elif version == '8.1':
        platsdk = GetRegistryKey("SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots", "KitsRoot81")

        # Fallback in case we can't read the registry.
        if not platsdk or not os.path.isdir(platsdk):
            platsdk = "C:\\Program Files (x86)\\Windows Kits\\8.1\\"

    elif version == '8.0':
        platsdk = GetRegistryKey("SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots", "KitsRoot")

    else:
        platsdk = GetRegistryKey("SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v" + version, "InstallationFolder")

        if not platsdk or not os.path.isdir(platsdk):
            # Most common location.  Worth a try.
            platsdk = GetProgramFiles() + "\\Microsoft SDKs\\Windows\\v" + version
            if not os.path.isdir(platsdk):
                if not version.endswith('A'):
                    # Try the stripped-down version that is bundled with Visual Studio.
                    return SdkLocateWindows(version + 'A')
                platsdk = None

    if not platsdk or not os.path.isdir(platsdk):
        exit("Couldn't find Windows SDK version %s.  To use a different version, use the --windows-sdk option." % (version))

    if not platsdk.endswith("\\"):
        platsdk += "\\"
    SDK["MSPLATFORM"] = platsdk
    SDK["MSPLATFORM_VERSION"] = version

    if GetVerbose():
        print("Using Windows SDK %s located at %s" % (version, platsdk))
    else:
        print("Using Windows SDK %s" % (version))

def SdkLocateMacOSX(osxtarget = None):
    if (GetHost() != "darwin"): return
    if (osxtarget != None):
        sdkname = "MacOSX%d.%d" % osxtarget
        if (os.path.exists("/Developer/SDKs/%su.sdk" % sdkname)):
            SDK["MACOSX"] = "/Developer/SDKs/%su.sdk" % sdkname
        elif (os.path.exists("/Developer/SDKs/%s.sdk" % sdkname)):
            SDK["MACOSX"] = "/Developer/SDKs/%s.sdk" % sdkname
        elif (os.path.exists("/Developer/SDKs/%s.0.sdk" % sdkname)):
            SDK["MACOSX"] = "/Developer/SDKs/%s.0.sdk" % sdkname
        elif (os.path.exists("/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/%s.sdk" % sdkname)):
            SDK["MACOSX"] = "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/%s.sdk" % sdkname
        else:
            handle = os.popen("xcode-select -print-path")
            result = handle.read().strip().rstrip('/')
            handle.close()
            if (os.path.exists("%s/Platforms/MacOSX.platform/Developer/SDKs/%s.sdk" % (result, sdkname))):
                SDK["MACOSX"] = "%s/Platforms/MacOSX.platform/Developer/SDKs/%s.sdk" % (result, sdkname)
            else:
                exit("Couldn't find any MacOSX SDK for OSX version %s!" % sdkname)
    else:
        SDK["MACOSX"] = ""

# Latest first
PHYSXVERSIONINFO = [
    ("PHYSX284", "v2.8.4"),
    ("PHYSX283", "v2.8.3"),
    ("PHYSX281", "v2.8.1"),
]

def SdkLocatePhysX():
    # First check for a physx directory in sdks.
    dir = GetSdkDir("physx")
    if (dir and os.path.isdir(dir)):
        SDK["PHYSX"] = dir
        SDK["PHYSXLIBS"] = dir + "/lib"
        return

    if CrossCompiling():
        return

    # Try to find a PhysX installation on the system.
    for (ver, key) in PHYSXVERSIONINFO:
        if (GetHost() == "windows"):
            folders = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\Folders"
            for folder in ListRegistryValues(folders):
                if folder.endswith("NVIDIA PhysX SDK\\%s\\SDKs\\" % key) or \
                   folder.endswith("NVIDIA PhysX SDK\\%s_win\\SDKs\\" % key):

                    SDK["PHYSX"] = folder
                    if GetTargetArch() == 'x64':
                        SDK["PHYSXLIBS"] = folder + "/lib/win64"
                        AddToPathEnv("PATH", folder + "/../Bin/win64/")
                    else:
                        SDK["PHYSXLIBS"] = folder + "/lib/win32"
                        AddToPathEnv("PATH", folder + "/../Bin/win32/")

                    return

        elif (GetHost() == "linux"):
            incpath = "/usr/include/PhysX/%s/SDKs" % key
            libpath = "/usr/lib/PhysX/%s" % key
            if (os.path.isdir(incpath) and os.path.isdir(libpath)):
                SDK["PHYSX"] = incpath
                SDK["PHYSXLIBS"] = libpath
                return

def SdkLocateSpeedTree():
    # Look for all of the SpeedTree SDK directories within the
    # sdks/win32/speedtree dir, and pick the highest-numbered one.
    dir = GetSdkDir("speedtree")
    if not os.path.exists(dir):
        return

    speedtrees = []
    for dirname in os.listdir(dir):
        if dirname.startswith('SpeedTree SDK v'):
            version = dirname[15:].split()[0]
            version = tuple(map(int, version.split('.')))
            speedtrees.append((version, dirname))
    if not speedtrees:
        # No installed SpeedTree SDK.
        return

    speedtrees.sort()
    version, dirname = speedtrees[-1]
    SDK["SPEEDTREE"] = os.path.join(dir, dirname)
    SDK["SPEEDTREEAPI"] = "OpenGL"
    SDK["SPEEDTREEVERSION"] = '%s.%s' % (version[0], version[1])

def SdkLocateAndroid():
    """This actually locates the Android NDK, not the Android SDK.
    NDK_ROOT must be set to its root directory."""

    global TOOLCHAIN_PREFIX

    if GetTarget() != 'android':
        return

    # Allow ANDROID_API/ANDROID_ABI to be used in makepanda.py.
    api = ANDROID_API
    SDK["ANDROID_API"] = api

    abi = ANDROID_ABI
    SDK["ANDROID_ABI"] = abi
    SDK["ANDROID_TRIPLE"] = ANDROID_TRIPLE

    if GetHost() == 'android':
        # Assume we're compiling from termux.
        prefix = os.environ.get("PREFIX", "/data/data/com.termux/files/usr")
        SDK["ANDROID_JAR"] = prefix + "/share/aapt/android.jar"
        return

    # Find the location of the Android SDK.
    sdk_root = os.environ.get('ANDROID_HOME')
    if not sdk_root or not os.path.isdir(sdk_root):
        sdk_root = os.environ.get('ANDROID_SDK_ROOT')

        # Try the default installation location on Windows.
        if not sdk_root and GetHost() == 'windows':
            sdk_root = os.path.expanduser(os.path.join('~', 'AppData', 'Local', 'Android', 'Sdk'))

        if not sdk_root:
            exit('ANDROID_SDK_ROOT must be set when compiling for Android!')
        elif not os.path.isdir(sdk_root):
            exit('Cannot find %s.  Please install Android SDK and set ANDROID_SDK_ROOT or ANDROID_HOME.' % (sdk_root))

    # Determine the NDK installation directory.
    if os.environ.get('NDK_ROOT') or os.environ.get('ANDROID_NDK_ROOT'):
        # We have an explicit setting from an environment variable.
        ndk_root = os.environ.get('ANDROID_NDK_ROOT')
        if not ndk_root or not os.path.isdir(ndk_root):
            ndk_root = os.environ.get('NDK_ROOT')
            if not ndk_root or not os.path.isdir(ndk_root):
                exit("Cannot find %s.  Please install Android NDK and set ANDROID_NDK_ROOT." % (ndk_root))
    else:
        # Often, it's installed in the ndk-bundle subdirectory of the SDK.
        ndk_root = os.path.join(sdk_root, 'ndk-bundle')

        if not os.path.isdir(os.path.join(ndk_root, 'toolchains')):
            exit('Cannot find the Android NDK.  Install it via the SDK manager or set the ANDROID_NDK_ROOT variable if you have installed it in a different location.')

    SDK["ANDROID_NDK"] = ndk_root

    # Determine the toolchain location.
    prebuilt_dir = os.path.join(ndk_root, 'toolchains', 'llvm', 'prebuilt')
    if not os.path.isdir(prebuilt_dir):
        exit('Not found: %s (is the Android NDK installed?)' % (prebuilt_dir))

    host_tag = GetHost() + '-x86'
    if host_64:
        host_tag += '_64'
    elif host_tag == 'windows-x86':
        host_tag = 'windows'

    prebuilt_dir = os.path.join(prebuilt_dir, host_tag)
    if host_tag == 'windows-x86_64' and not os.path.isdir(prebuilt_dir):
        # Try the 32-bits toolchain instead.
        host_tag = 'windows'
        prebuilt_dir = os.path.join(prebuilt_dir, host_tag)

    SDK["ANDROID_TOOLCHAIN"] = prebuilt_dir

    # And locate the GCC toolchain, which is needed for some tools (eg. as/ld)
    arch = GetTargetArch()
    for opt in (TOOLCHAIN_PREFIX + '4.9', arch + '-4.9', TOOLCHAIN_PREFIX + '4.8', arch + '-4.8'):
        if os.path.isdir(os.path.join(ndk_root, 'toolchains', opt)):
            SDK["ANDROID_GCC_TOOLCHAIN"] = os.path.join(ndk_root, 'toolchains', opt, 'prebuilt', host_tag)
            break

    # The prebuilt binaries have no toolchain prefix.
    TOOLCHAIN_PREFIX = ''

    # Determine the sysroot directory.
    if arch == 'armv7a':
        arch_dir = 'arch-arm'
    elif arch == 'aarch64':
        arch_dir = 'arch-arm64'
    else:
        arch_dir = 'arch-' + arch
    SDK["SYSROOT"] = os.path.join(ndk_root, 'platforms', 'android-%s' % (api), arch_dir).replace('\\', '/')
    #IncDirectory("ALWAYS", os.path.join(SDK["SYSROOT"], 'usr', 'include'))

    # Starting with NDK r16, libc++ is the recommended STL to use.
    stdlibc = os.path.join(ndk_root, 'sources', 'cxx-stl', 'llvm-libc++')
    IncDirectory("ALWAYS", os.path.join(stdlibc, 'include').replace('\\', '/'))
    LibDirectory("ALWAYS", os.path.join(stdlibc, 'libs', abi).replace('\\', '/'))

    stl_lib = os.path.join(stdlibc, 'libs', abi, 'libc++_shared.so')
    LibName("ALWAYS", stl_lib.replace('\\', '/'))
    CopyFile(os.path.join(GetOutputDir(), 'lib', 'libc++_shared.so'), stl_lib)

    # The Android support library polyfills C++ features not available in the
    # STL that ships with Android.
    support = os.path.join(ndk_root, 'sources', 'android', 'support', 'include')
    IncDirectory("ALWAYS", support.replace('\\', '/'))
    if api < 21:
        LibName("ALWAYS", "-landroid_support")

    # Determine the location of android.jar.
    SDK["ANDROID_JAR"] = os.path.join(sdk_root, 'platforms', 'android-%s' % (api), 'android.jar')
    if not os.path.isfile(SDK["ANDROID_JAR"]):
        exit("Cannot find %s.  Install platform API level %s via the SDK manager or change the targeted API level with --target=android-#" % (SDK["ANDROID_JAR"], api))

    # Which build tools versions do we have?  Pick the latest.
    versions = []
    for version in os.listdir(os.path.join(sdk_root, "build-tools")):
        match = re.match('([0-9]+)\\.([0-9]+)\\.([0-9]+)', version)
        if match:
            version_tuple = int(match.group(1)), int(match.group(2)), int(match.group(3))
            versions.append(version_tuple)

    versions.sort()
    if versions:
        version = versions[-1]
        SDK["ANDROID_BUILD_TOOLS"] = os.path.join(sdk_root, "build-tools", "{0}.{1}.{2}".format(*version))

    # And find the location of the Java compiler.
    if GetHost() == "windows":
        jdk_home = os.environ.get("JDK_HOME") or os.environ.get("JAVA_HOME")
        if not jdk_home:
            # Try to use the Java shipped with Android Studio.
            studio_path = GetRegistryKey("SOFTWARE\\Android Studio", "Path", override64=False)
            if studio_path and os.path.isdir(studio_path):
                jdk_home = os.path.join(studio_path, "jre")

        if not jdk_home or not os.path.isdir(jdk_home):
            exit("Cannot find JDK.  Please set JDK_HOME or JAVA_HOME.")

        javac = os.path.join(jdk_home, "bin", "javac.exe")
        if not os.path.isfile(javac):
            exit("Cannot find %s.  Install the JDK and set JDK_HOME or JAVA_HOME." % (javac))

        SDK["JDK"] = jdk_home

########################################################################
##
## SDK Auto-Disables
##
## Disable packages whose SDKs could not be found.
##
########################################################################

def SdkAutoDisableDirectX():
    for ver in DXVERSIONS + ["DIRECTCAM"]:
        if (PkgSkip(ver)==0):
            if (ver not in SDK):
                if (GetHost() == "windows"):
                    WARNINGS.append("I cannot locate SDK for "+ver)
                    WARNINGS.append("I have automatically added this command-line option: --no-"+ver.lower())
                PkgDisable(ver)
            else:
                WARNINGS.append("Using "+ver+" sdk: "+SDK[ver])

def SdkAutoDisableMaya():
    for (ver,key) in MAYAVERSIONINFO:
        if (ver not in SDK) and (PkgSkip(ver)==0):
            if (GetHost() == "windows"):
                WARNINGS.append("The registry does not appear to contain a pointer to the "+ver+" SDK.")
            else:
                WARNINGS.append("I cannot locate SDK for "+ver)
            WARNINGS.append("I have automatically added this command-line option: --no-"+ver.lower())
            PkgDisable(ver)

def SdkAutoDisableMax():
    for version,key1,key2,subdir in MAXVERSIONINFO:
        if (PkgSkip(version)==0) and ((version not in SDK) or (version+"CS" not in SDK)):
            if (GetHost() == "windows"):
                if (version in SDK):
                    WARNINGS.append("Your copy of "+version+" does not include the character studio SDK")
                else:
                    WARNINGS.append("The registry does not appear to contain a pointer to "+version)
                WARNINGS.append("I have automatically added this command-line option: --no-"+version.lower())
            PkgDisable(version)

def SdkAutoDisablePhysX():
    if ("PHYSX" not in SDK) and (PkgSkip("PHYSX")==0):
        PkgDisable("PHYSX")
        WARNINGS.append("I cannot locate SDK for PhysX")
        WARNINGS.append("I have automatically added this command-line option: --no-physx")

def SdkAutoDisableSpeedTree():
    if ("SPEEDTREE" not in SDK) and (PkgSkip("SPEEDTREE")==0):
        PkgDisable("SPEEDTREE")
        WARNINGS.append("I cannot locate SDK for SpeedTree")
        WARNINGS.append("I have automatically added this command-line option: --no-speedtree")

########################################################################
##
## Visual Studio comes with a script called VSVARS32.BAT, which
## you need to run before using visual studio command-line tools.
## The following python subroutine serves the same purpose.
##
########################################################################

def AddToPathEnv(path,add):
    if path in os.environ:
        if sys.platform == 'cygwin' and path != "PATH":
            # INCLUDE, LIB, etc. must remain in Windows-style in cygwin.
            os.environ[path] = add + ';' + os.environ[path]
        else:
            os.environ[path] = add + os.pathsep + os.environ[path]
    else:
        os.environ[path] = add

def SetupVisualStudioEnviron():
    if ("VISUALSTUDIO" not in SDK):
        exit("Could not find Visual Studio install directory")
    if ("MSPLATFORM" not in SDK):
        exit("Could not find the Microsoft Platform SDK")

    if (SDK["VISUALSTUDIO_VERSION"] >= (15,0)):
        try:
            vsver_file = open(os.path.join(SDK["VISUALSTUDIO"],
                "VC\\Auxiliary\\Build\\Microsoft.VCToolsVersion.default.txt"), "r")
            SDK["VCTOOLSVERSION"] = vsver_file.readline().strip()
            vcdir_suffix = "VC\\Tools\\MSVC\\%s\\" % SDK["VCTOOLSVERSION"]
        except:
            exit("Couldn't find tool version of %s." % MSVCVERSIONINFO[SDK["MSVC_VERSION"]]["vsname"])
    else:
        vcdir_suffix = "VC\\"

    os.environ["VCINSTALLDIR"] = SDK["VISUALSTUDIO"] + vcdir_suffix
    os.environ["WindowsSdkDir"] = SDK["MSPLATFORM"]

    winsdk_ver = SDK["MSPLATFORM_VERSION"]

    # Determine the directories to look in based on the architecture.
    arch = GetTargetArch()
    bindir = ""
    libdir = ""
    if ("VCTOOLSVERSION" in SDK):
        bindir = "Host" + GetHostArch().upper() + "\\" + arch
        libdir = arch
    else:
        if (arch == 'x64'):
            bindir = 'amd64'
            libdir = 'amd64'
        elif (arch != 'x86'):
            bindir = arch
            libdir = arch

        if (arch != 'x86' and GetHostArch() == 'x86'):
            # Special version of the tools that run on x86.
            bindir = 'x86_' + bindir

    vc_binpath = SDK["VISUALSTUDIO"] + vcdir_suffix + "bin"
    binpath = os.path.join(vc_binpath, bindir)
    if not os.path.isfile(binpath + "\\cl.exe"):
        # Try the x86 tools, those should work just as well.
        if arch == 'x64' and os.path.isfile(vc_binpath + "\\x86_amd64\\cl.exe"):
            binpath = "{0}\\x86_amd64;{0}".format(vc_binpath)
        elif winsdk_ver.startswith('10.'):
            exit("Couldn't find compilers in %s.  You may need to install the Windows SDK 7.1 and the Visual C++ 2010 SP1 Compiler Update for Windows SDK 7.1." % binpath)
        else:
            exit("Couldn't find compilers in %s." % binpath)

    AddToPathEnv("PATH",    binpath)
    AddToPathEnv("PATH",    SDK["VISUALSTUDIO"] + "Common7\\IDE")
    AddToPathEnv("INCLUDE", os.environ["VCINSTALLDIR"] + "include")
    AddToPathEnv("INCLUDE", os.environ["VCINSTALLDIR"] + "atlmfc\\include")
    AddToPathEnv("LIB",     os.environ["VCINSTALLDIR"] + "lib\\" + libdir)
    AddToPathEnv("LIB",     os.environ["VCINSTALLDIR"] + "atlmfc\\lib\\" + libdir)

    winsdk_ver = SDK["MSPLATFORM_VERSION"]
    if winsdk_ver.startswith('10.'):
        AddToPathEnv("PATH",    SDK["MSPLATFORM"] + "bin\\" + arch)
        AddToPathEnv("PATH",    SDK["MSPLATFORM"] + "bin\\" + winsdk_ver + "\\" + arch)

        # Windows Kit 10 introduces the "universal CRT".
        inc_dir = SDK["MSPLATFORM"] + "Include\\" + winsdk_ver + "\\"
        lib_dir = SDK["MSPLATFORM"] + "Lib\\" + winsdk_ver + "\\"
        AddToPathEnv("INCLUDE", inc_dir + "shared")
        AddToPathEnv("INCLUDE", inc_dir + "ucrt")
        AddToPathEnv("INCLUDE", inc_dir + "um")
        AddToPathEnv("LIB", lib_dir + "ucrt\\" + arch)
        AddToPathEnv("LIB", lib_dir + "um\\" + arch)
    elif winsdk_ver == '8.1':
        AddToPathEnv("PATH",    SDK["MSPLATFORM"] + "bin\\" + arch)

        inc_dir = SDK["MSPLATFORM"] + "Include\\"
        lib_dir = SDK["MSPLATFORM"] + "Lib\\winv6.3\\"
        AddToPathEnv("INCLUDE", inc_dir + "shared")
        AddToPathEnv("INCLUDE", inc_dir + "ucrt")
        AddToPathEnv("INCLUDE", inc_dir + "um")
        AddToPathEnv("LIB", lib_dir + "ucrt\\" + arch)
        AddToPathEnv("LIB", lib_dir + "um\\" + arch)
    else:
        AddToPathEnv("PATH",    SDK["MSPLATFORM"] + "bin")
        AddToPathEnv("INCLUDE", SDK["MSPLATFORM"] + "include")
        AddToPathEnv("INCLUDE", SDK["MSPLATFORM"] + "include\\atl")
        AddToPathEnv("INCLUDE", SDK["MSPLATFORM"] + "include\\mfc")

        if arch != 'x64':
            AddToPathEnv("LIB", SDK["MSPLATFORM"] + "lib")
            AddToPathEnv("PATH",SDK["VISUALSTUDIO"] + "VC\\redist\\x86\\Microsoft.VC100.CRT")
            AddToPathEnv("PATH",SDK["VISUALSTUDIO"] + "VC\\redist\\x86\\Microsoft.VC100.MFC")

        elif os.path.isdir(SDK["MSPLATFORM"] + "lib\\x64"):
            AddToPathEnv("LIB", SDK["MSPLATFORM"] + "lib\\x64")

        elif os.path.isdir(SDK["MSPLATFORM"] + "lib\\amd64"):
            AddToPathEnv("LIB", SDK["MSPLATFORM"] + "lib\\amd64")

        else:
            exit("Could not locate 64-bits libraries in Windows SDK directory!\nUsing directory: %s" % SDK["MSPLATFORM"])

    # Targeting the 7.1 SDK (which is the only way to have Windows XP support)
    # with Visual Studio 2015 requires use of the Universal CRT.
    if winsdk_ver in ('7.1', '7.1A') and SDK["VISUALSTUDIO_VERSION"] >= (14,0):
        win_kit = GetRegistryKey("SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots", "KitsRoot10")

        # Fallback in case we can't read the registry.
        if not win_kit or not os.path.isdir(win_kit):
            win_kit = "C:\\Program Files (x86)\\Windows Kits\\10\\"
        elif not win_kit.endswith('\\'):
            win_kit += '\\'

        for vnum in 10150, 10240, 10586, 14393, 15063, 16299, 17134, 17763:
            version = "10.0.{0}.0".format(vnum)
            if os.path.isfile(win_kit + "Include\\" + version + "\\ucrt\\assert.h"):
                print("Using Universal CRT %s" % (version))
                break

        AddToPathEnv("LIB", "%s\\Lib\\%s\\ucrt\\%s" % (win_kit, version, arch))
        AddToPathEnv("INCLUDE", "%s\\Include\\%s\\ucrt" % (win_kit, version))

        # Copy the DLLs to the bin directory.
        CopyAllFiles(GetOutputDir() + "/bin/", win_kit + "Redist\\ucrt\\DLLs\\" + arch + "\\")

########################################################################
#
# Include and Lib directories.
#
# These allow you to add include and lib directories to the
# compiler search paths.  These methods accept a "package"
# parameter, which specifies which package the directory is
# associated with.  The include/lib directory is not used
# if the package is not selected.  The package can be 'ALWAYS'.
#
########################################################################

INCDIRECTORIES = []
LIBDIRECTORIES = []
FRAMEWORKDIRECTORIES = []
LIBNAMES = []
DEFSYMBOLS = []

def IncDirectory(opt, dir):
    INCDIRECTORIES.append((opt, dir))

def LibDirectory(opt, dir):
    LIBDIRECTORIES.append((opt, dir))

def FrameworkDirectory(opt, dir):
    FRAMEWORKDIRECTORIES.append((opt, dir))

def LibName(opt, name):
    # Check to see if the lib file actually exists for the thirdparty library given
    # Are we a thirdparty library?
    if name.startswith(GetThirdpartyDir()):
        # Does this lib exist?
        if not os.path.exists(name):
            WARNINGS.append(name + " not found.  Skipping Package " + opt)
            if (opt in PkgListGet()):
                if not PkgSkip(opt):
                    Warn("Could not locate thirdparty package %s, excluding from build" % (opt.lower()))
                    PkgDisable(opt)
                return
            else:
                Error("Could not locate thirdparty package %s, aborting build" % (opt.lower()))
    LIBNAMES.append((opt, name))

def DefSymbol(opt, sym, val=""):
    DEFSYMBOLS.append((opt, sym, val))

########################################################################
#
# This subroutine prepares the environment for the build.
#
########################################################################

def SetupBuildEnvironment(compiler):
    if GetVerbose():
        print("Using compiler: %s" % compiler)
        print("Host OS: %s" % GetHost())
        print("Host arch: %s" % GetHostArch())

    target = GetTarget()
    if target != 'android':
        print("Target OS: %s" % GetTarget())
    else:
        print("Target OS: %s (API level %d)" % (GetTarget(), ANDROID_API))
    print("Target arch: %s" % GetTargetArch())

    # Set to English so we can safely parse the result of gcc commands.
    # Setting it to UTF-8 is necessary for Python 3 modules to import
    # correctly.
    os.environ["LC_ALL"] = "en_US.UTF-8"
    os.environ["LANGUAGE"] = "en"

    # In the case of Android, we have to put the toolchain on the PATH in order to use it.
    if GetTarget() == 'android' and GetHost() != 'android':
        AddToPathEnv("PATH", os.path.join(SDK["ANDROID_TOOLCHAIN"], "bin"))

        if "ANDROID_BUILD_TOOLS" in SDK:
            AddToPathEnv("PATH", SDK["ANDROID_BUILD_TOOLS"])

        if "JDK" in SDK:
            AddToPathEnv("PATH", os.path.join(SDK["JDK"], "bin"))
            os.environ["JAVA_HOME"] = SDK["JDK"]

    if compiler == "MSVC":
        # Add the visual studio tools to PATH et al.
        SetupVisualStudioEnviron()

    if compiler == "GCC":
        # Invoke gcc to determine the system library directories.
        global SYS_LIB_DIRS, SYS_INC_DIRS

        if sys.platform == "darwin":
            # We need to add this one explicitly for some reason.
            SYS_LIB_DIRS.append(SDK["MACOSX"] + "/usr/lib")

        if not SDK.get("MACOSX"):
            # gcc doesn't add this one, but we do want it:
            local_lib = SDK.get("SYSROOT", "") + "/usr/local/lib"
            if os.path.isdir(local_lib):
                SYS_LIB_DIRS.append(local_lib)

        sysroot_flag = ""

        if SDK.get("MACOSX"):
            # The default compiler in Leopard does not respect --sysroot correctly.
            sysroot_flag = " -isysroot " + SDK["MACOSX"]
        if SDK.get("SYSROOT"):
            sysroot_flag = ' --sysroot=%s -no-canonical-prefixes' % (SDK["SYSROOT"])

        # Extract the dirs from the line that starts with 'libraries: ='.
        cmd = GetCXX() + " -print-search-dirs" + sysroot_flag
        handle = os.popen(cmd)
        for line in handle:
            if not line.startswith('libraries: ='):
                continue

            line = line[12:].strip()
            libdirs = line.split(':')
            while libdirs:
                libdir = os.path.normpath(libdirs.pop(0))
                if os.path.isdir(libdir):
                    if libdir not in SYS_LIB_DIRS:
                        SYS_LIB_DIRS.append(libdir)
                elif len(libdir) == 1:
                    # Oops, is this a drive letter?  Prepend it to the next.
                    libdirs[0] = libdir + ':' + libdirs[0]
                elif GetVerbose():
                    print("Ignoring non-existent library directory %s" % (libdir))

        returnval = handle.close()
        if returnval != None and returnval != 0:
            Warn("%s failed" % (cmd))
            SYS_LIB_DIRS += [SDK.get("SYSROOT", "") + "/usr/lib"]

        # The Android toolchain on Windows doesn't actually add this one.
        if target == 'android' and GetHost() == 'windows':
            libdir = SDK.get("SYSROOT", "") + "/usr/lib"
            if GetTargetArch() == 'x86_64':
                libdir += '64'
            SYS_LIB_DIRS += [libdir]

        # Now extract the preprocessor's include directories.
        cmd = GetCXX() + " -x c++ -v -E " + os.devnull
        if "ANDROID_NDK" in SDK:
            ndk_dir = SDK["ANDROID_NDK"].replace('\\', '/')
            cmd += ' -isystem %s/sysroot/usr/include' % (ndk_dir)
            cmd += ' -isystem %s/sysroot/usr/include/%s' % (ndk_dir, SDK["ANDROID_TRIPLE"])
        else:
            cmd += sysroot_flag

        null = open(os.devnull, 'w')
        handle = subprocess.Popen(cmd, stdout=null, stderr=subprocess.PIPE, shell=True)
        scanning = False
        for line in handle.communicate()[1].splitlines():
            line = line.decode('utf-8', 'replace')

            # Start looking at a line that says:  #include "..." search starts here
            if not scanning:
                if line.startswith('#include'):
                    scanning = True
                continue

            if sys.platform == "win32":
                if not line.startswith(' '):
                    continue
            else:
                if not line.startswith(' /'):
                    continue

            line = line.strip()
            if line.endswith(" (framework directory)"):
                pass
            elif os.path.isdir(line):
                SYS_INC_DIRS.append(os.path.normpath(line))
            elif GetVerbose():
                print("Ignoring non-existent include directory %s" % (line))

        if handle.returncode != 0 or not SYS_INC_DIRS:
            Warn("%s failed or did not produce the expected result" % (cmd))
            sysroot = SDK.get("SYSROOT", "")
            # Add some sensible directories as a fallback.
            SYS_INC_DIRS = [
                sysroot + "/usr/include",
                sysroot + "/usr/local/include"
            ]
            pcbsd_inc = sysroot + "/usr/PCBSD/local/include"
            if os.path.isdir(pcbsd_inc):
                SYS_INC_DIRS.append(pcbsd_inc)

        null.close()

        # Print out the search paths
        if GetVerbose():
            print("System library search path:")
            for dir in SYS_LIB_DIRS:
                print("  " + dir)

            print("System include search path:")
            for dir in SYS_INC_DIRS:
                print("  " + dir)

    # If we're cross-compiling, no point in putting our output dirs on the path.
    if CrossCompiling():
        return

    # Add our output directories to the environment.
    builtdir = GetOutputDir()
    AddToPathEnv("PYTHONPATH", builtdir)
    AddToPathEnv("PANDA_PRC_DIR", os.path.join(builtdir, "etc"))
    AddToPathEnv("PATH", os.path.join(builtdir, "bin"))
    if GetHost() == 'windows':
        # extension_native_helpers.py currently expects to find libpandaexpress on sys.path.
        AddToPathEnv("PYTHONPATH", os.path.join(builtdir, "bin"))
        AddToPathEnv("PATH", os.path.join(builtdir, "plugins"))

    # Now for the special (DY)LD_LIBRARY_PATH on Unix-esque systems.
    if GetHost() != 'windows':
        # Get the current
        ldpath = os.environ.get("LD_LIBRARY_PATH", "").split(os.pathsep)
        if GetHost() == 'darwin':
            dyldpath = os.environ.get("DYLD_LIBRARY_PATH", "").split(os.pathsep)

        # Remove any potential current Panda installation lib dirs
        for i in ldpath[:]:
            if i.startswith("/usr/lib/panda3d") or \
               i.startswith("/usr/local/panda"):
                ldpath.remove(i)

        if GetHost() == 'darwin':
            for i in dyldpath[:]:
                if i.startswith("/Applications/Panda3D") or \
                   i.startswith("/Developer/Panda3D"):
                    dyldpath.remove(i)

        # Add built/lib/ to (DY)LD_LIBRARY_PATH
        ldpath.insert(0, os.path.join(builtdir, 'lib'))
        os.environ["LD_LIBRARY_PATH"] = os.pathsep.join(ldpath)

        if GetHost() == 'darwin':
            dyldpath.insert(0, os.path.join(builtdir, 'lib'))
            os.environ["DYLD_LIBRARY_PATH"] = os.pathsep.join(dyldpath)

            # OS X 10.11 removed DYLD_LIBRARY_PATH, but we still need to pass
            # on our lib directory to ppackage, so add it to PATH instead.
            os.environ["PATH"] = os.path.join(builtdir, 'lib') + ':' + os.environ.get("PATH", "")

        # Workaround around compile issue on PCBSD
        if (os.path.exists("/usr/PCBSD")):
            os.environ["LD_LIBRARY_PATH"] += os.pathsep + "/usr/PCBSD/local/lib"

########################################################################
##
## Routines to copy files into the build tree
##
########################################################################

def CopyFile(dstfile, srcfile):
    if dstfile[-1] == '/':
        dstfile += os.path.basename(srcfile)

    if NeedsBuild([dstfile], [srcfile]):
        if os.path.islink(srcfile):
            # Preserve symlinks
            if os.path.isfile(dstfile) or os.path.islink(dstfile):
                print("Removing file %s" % (dstfile))
                os.unlink(dstfile)
            elif os.path.isdir(dstfile):
                print("Removing directory %s" % (dstfile))
                shutil.rmtree(dstfile)
            os.symlink(os.readlink(srcfile), dstfile)
        else:
            WriteBinaryFile(dstfile, ReadBinaryFile(srcfile))

        if sys.platform == 'cygwin' and os.path.splitext(dstfile)[1].lower() in ('.dll', '.exe'):
            os.chmod(dstfile, 0o755)

        JustBuilt([dstfile], [srcfile])

def CopyAllFiles(dstdir, srcdir, suffix=""):
    for x in GetDirectoryContents(srcdir, ["*"+suffix]):
        CopyFile(dstdir + x, srcdir + x)

def CopyAllHeaders(dir, skip=[]):
    for filename in GetDirectoryContents(dir, ["*.h", "*.I", "*.T"], skip):
        srcfile = dir + "/" + filename
        dstfile = OUTPUTDIR + "/include/" + filename
        if (NeedsBuild([dstfile], [srcfile])):
            WriteBinaryFile(dstfile, ReadBinaryFile(srcfile))
            JustBuilt([dstfile], [srcfile])

def CopyTree(dstdir, srcdir, omitVCS=True):
    if os.path.isdir(dstdir):
        source_entries = os.listdir(srcdir)
        for entry in source_entries:
            srcpth = os.path.join(srcdir, entry)
            dstpth = os.path.join(dstdir, entry)

            if os.path.islink(srcpth) or os.path.isfile(srcpth):
                if not omitVCS or entry not in VCS_FILES:
                    CopyFile(dstpth, srcpth)
            else:
                if not omitVCS or entry not in VCS_DIRS:
                    CopyTree(dstpth, srcpth)

        # Delete files in dstdir that are not in srcdir.
        for entry in os.listdir(dstdir):
            if entry not in source_entries:
                path = os.path.join(dstdir, entry)
                if os.path.islink(path) or os.path.isfile(path):
                    os.remove(path)
                elif os.path.isdir(path):
                    shutil.rmtree(path)
    else:
        if GetHost() == 'windows':
            srcdir = srcdir.replace('/', '\\')
            dstdir = dstdir.replace('/', '\\')
            cmd = 'xcopy /I/Y/E/Q "' + srcdir + '" "' + dstdir + '"'
            oscmd(cmd)
        else:
            if subprocess.call(['cp', '-R', '-f', srcdir, dstdir]) != 0:
                exit("Copy failed.")

        if omitVCS:
            DeleteVCS(dstdir)

def CopyPythonTree(dstdir, srcdir, lib2to3_fixers=[], threads=0):
    if (not os.path.isdir(dstdir)):
        os.mkdir(dstdir)

    lib2to3 = None
    lib2to3_args = ['-w', '-n', '--no-diffs']

    if len(lib2to3_fixers) > 0 and sys.version_info >= (3, 0):
        from lib2to3.main import main as lib2to3

        if lib2to3_fixers == ['all']:
            lib2to3_args += ['-x', 'buffer', '-x', 'idioms', '-x', 'set_literal', '-x', 'ws_comma']
        else:
            for fixer in lib2to3_fixers:
                lib2to3_args += ['-f', fixer]

    if threads:
        lib2to3_args += ['-j', str(threads)]

    exclude_files = set(VCS_FILES)
    exclude_files.add('panda3d.py')

    refactor = []
    for entry in os.listdir(srcdir):
        srcpth = os.path.join(srcdir, entry)
        dstpth = os.path.join(dstdir, entry)
        if os.path.isfile(srcpth):
            base, ext = os.path.splitext(entry)
            if entry not in exclude_files and ext not in SUFFIX_INC + ['.pyc', '.pyo']:
                if (NeedsBuild([dstpth], [srcpth])):
                    WriteBinaryFile(dstpth, ReadBinaryFile(srcpth))

                    if ext == '.py' and not entry.endswith('-extensions.py'):
                        refactor.append((dstpth, srcpth))
                        lib2to3_args.append(dstpth)
                    else:
                        JustBuilt([dstpth], [srcpth])

        elif entry not in VCS_DIRS:
            CopyPythonTree(dstpth, srcpth, lib2to3_fixers, threads=threads)

    if refactor and lib2to3 is not None:
        ret = lib2to3("lib2to3.fixes", lib2to3_args)

        if ret != 0:
            for dstpth, srcpth in refactor:
                os.remove(dstpth)
                exit("Error in lib2to3.")
        else:
            for dstpth, srcpth in refactor:
                JustBuilt([dstpth], [srcpth])

########################################################################
##
## Parse PandaVersion.pp to extract the version number.
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

# This function is being phased out.
def ParsePandaVersion(fn):
    try:
        f = open(fn, "r")
        pattern = re.compile('^[ \t]*[#][ \t]*define[ \t]+PANDA_VERSION[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)')
        for line in f:
            match = pattern.match(line, 0)
            if (match):
                f.close()
                return match.group(1) + "." + match.group(2) + "." + match.group(3)
        f.close()
    except: pass
    return "0.0.0"

##########################################################################################
#
# Utility function to generate a resource file
#
##########################################################################################

RESOURCE_FILE_TEMPLATE = """VS_VERSION_INFO VERSIONINFO
 FILEVERSION %(commaversion)s
 PRODUCTVERSION %(commaversion)s
 FILEFLAGSMASK 0x3fL
 FILEFLAGS %(debugflag)s
 FILEOS 0x40004L
 FILETYPE 0x2L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904e4"
        BEGIN
            VALUE "FileDescription", "%(description)s\\0"
            VALUE "FileVersion", "%(dotversion)s"
            VALUE "LegalTrademarks", "\\0"
            VALUE "MIMEType", "%(mimetype)s\\0"
            VALUE "FileExtents", "%(extension)s\\0"
            VALUE "FileOpenName", "%(filedesc)s\\0"
            VALUE "OLESelfRegister", "\\0"
            VALUE "OriginalFilename", "%(filename)s\\0"
            VALUE "ProductName", "%(name)s %(version)s\\0"
            VALUE "ProductVersion", "%(dotversion)s"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1252
    END
END
"""

def GenerateResourceFile(**kwargs):
    if "debugflag" not in kwargs:
        if GetOptimize() <= 2:
            kwargs["debugflag"] = "0x1L"
        else:
            kwargs["debugflag"] = "0x0L"
    kwargs["dotversion"] = kwargs["version"]
    if len(kwargs["dotversion"].split(".")) == 3:
        kwargs["dotversion"] += ".0"
    if "commaversion" not in kwargs:
        kwargs["commaversion"] = kwargs["dotversion"].replace(".", ",")

    rcdata = ""
    if not "noinclude" in kwargs:
        rcdata += "#define APSTUDIO_READONLY_SYMBOLS\n"
        rcdata += "#include \"winresrc.h\"\n"
        rcdata += "#undef APSTUDIO_READONLY_SYMBOLS\n"
    rcdata += RESOURCE_FILE_TEMPLATE % kwargs

    if "icon" in kwargs:
        rcdata += "\nICON_FILE       ICON    \"%s\"\n" % kwargs["icon"]

    return rcdata


def WriteResourceFile(basename, **kwargs):
    if not basename.endswith(".rc"):
        basename += ".rc"
    basename = GetOutputDir() + "/include/" + basename
    ConditionalWriteFile(basename, GenerateResourceFile(**kwargs))
    return basename


def WriteEmbeddedStringFile(basename, inputs, string_name=None):
    if os.path.splitext(basename)[1] not in SUFFIX_INC:
        basename += '.cxx'
    target = GetOutputDir() + "/tmp/" + basename

    if string_name is None:
        string_name = os.path.basename(os.path.splitext(target)[0])
        string_name = string_name.replace('-', '_')

    data = bytearray()
    for input in inputs:
        fp = open(input, 'rb')

        # Insert a #line so that we get meaningful compile/assert errors when
        # the result is inserted by interrogate_module into generated code.
        if os.path.splitext(input)[1] in SUFFIX_INC:
            line = '#line 1 "%s"\n' % (input)
            data += bytearray(line.encode('ascii', 'replace'))

        data += bytearray(fp.read())
        fp.close()

    data.append(0)

    output = 'extern const char %s[] = {\n' % (string_name)

    i = 0
    for byte in data:
        if i == 0:
            output += ' '

        output += ' 0x%02x,' % (byte)
        i += 1
        if i >= 12:
            output += '\n'
            i = 0

    output += '\n};\n'
    ConditionalWriteFile(target, output)
    return target

########################################################################
##
## FindLocation
##
########################################################################

ORIG_EXT = {}
PYABI_SPECIFIC = set()
WARNED_FILES = set()

def GetOrigExt(x):
    return ORIG_EXT[x]

def SetOrigExt(x, v):
    ORIG_EXT[x] = v

def GetExtensionSuffix():
    if sys.version_info >= (3, 0):
        import _imp
        return _imp.extension_suffixes()[0]

    target = GetTarget()
    if target == 'windows':
        return '.pyd'
    else:
        return '.so'

def GetPythonABI():
    soabi = sysconfig.get_config_var('SOABI')
    if soabi:
        return soabi

    soabi = 'cpython-%d%d' % (sys.version_info[:2])

    if sys.version_info >= (3, 8):
        return soabi

    debug_flag = sysconfig.get_config_var('Py_DEBUG')
    if (debug_flag is None and hasattr(sys, 'gettotalrefcount')) or debug_flag:
        soabi += 'd'

    malloc_flag = sysconfig.get_config_var('WITH_PYMALLOC')
    if malloc_flag is None or malloc_flag:
        soabi += 'm'

    if sys.version_info < (3, 3):
        usize = sysconfig.get_config_var('Py_UNICODE_SIZE')
        if (usize is None and sys.maxunicode == 0x10ffff) or usize == 4:
            soabi += 'u'

    return soabi

def CalcLocation(fn, ipath):
    if fn.startswith("panda3d/") and fn.endswith(".py"):
        return OUTPUTDIR + "/" + fn

    if (fn.endswith(".class")):return OUTPUTDIR+"/classes/"+fn
    if (fn.count("/")): return fn
    dllext = ""
    target = GetTarget()
    if (GetOptimize() <= 2 and target == 'windows'): dllext = "_d"

    if (fn == "AndroidManifest.xml"): return OUTPUTDIR+"/"+fn
    if (fn.endswith(".cxx")): return CxxFindSource(fn, ipath)
    if (fn.endswith(".I")):   return CxxFindSource(fn, ipath)
    if (fn.endswith(".h")):   return CxxFindSource(fn, ipath)
    if (fn.endswith(".c")):   return CxxFindSource(fn, ipath)
    if (fn.endswith(".py")):  return CxxFindSource(fn, ipath)
    if (fn.endswith(".yxx")): return CxxFindSource(fn, ipath)
    if (fn.endswith(".lxx")): return CxxFindSource(fn, ipath)
    if (fn.endswith(".xml")): return CxxFindSource(fn, ipath)
    if (fn.endswith(".java")):return CxxFindSource(fn, ipath)
    if (fn.endswith(".egg")): return OUTPUTDIR+"/models/"+fn
    if (fn.endswith(".egg.pz")):return OUTPUTDIR+"/models/"+fn
    if (fn.endswith(".pyd")): return OUTPUTDIR+"/panda3d/"+fn[:-4]+GetExtensionSuffix()
    if (target == 'windows'):
        if (fn.endswith(".def")):   return CxxFindSource(fn, ipath)
        if (fn.endswith(".rc")):    return CxxFindSource(fn, ipath)
        if (fn.endswith(".idl")):   return CxxFindSource(fn, ipath)
        if (fn.endswith(".obj")):   return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".res")):   return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".tlb")):   return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".dll")):   return OUTPUTDIR+"/bin/"+fn[:-4]+dllext+".dll"
        if (fn.endswith(".ocx")):   return OUTPUTDIR+"/plugins/"+fn[:-4]+dllext+".ocx"
        if (fn.endswith(".mll")):   return OUTPUTDIR+"/plugins/"+fn[:-4]+dllext+".mll"
        if (fn.endswith(".dlo")):   return OUTPUTDIR+"/plugins/"+fn[:-4]+dllext+".dlo"
        if (fn.endswith(".dli")):   return OUTPUTDIR+"/plugins/"+fn[:-4]+dllext+".dli"
        if (fn.endswith(".dle")):   return OUTPUTDIR+"/plugins/"+fn[:-4]+dllext+".dle"
        if (fn.endswith(".plugin")):return OUTPUTDIR+"/plugins/"+fn[:-7]+dllext+".dll"
        if (fn.endswith(".exe")):   return OUTPUTDIR+"/bin/"+fn
        if (fn.endswith(".p3d")):   return OUTPUTDIR+"/bin/"+fn
        if (fn.endswith(".lib")):   return OUTPUTDIR+"/lib/"+fn[:-4]+dllext+".lib"
        if (fn.endswith(".ilb")):   return OUTPUTDIR+"/tmp/"+fn[:-4]+dllext+".lib"
    elif (target == 'darwin'):
        if (fn.endswith(".mm")):    return CxxFindSource(fn, ipath)
        if (fn.endswith(".r")):     return CxxFindSource(fn, ipath)
        if (fn.endswith(".plist")): return CxxFindSource(fn, ipath)
        if (fn.endswith(".obj")):   return OUTPUTDIR+"/tmp/"+fn[:-4]+".o"
        if (fn.endswith(".dll")):   return OUTPUTDIR+"/lib/"+fn[:-4]+".dylib"
        if (fn.endswith(".mll")):   return OUTPUTDIR+"/plugins/"+fn
        if (fn.endswith(".exe")):   return OUTPUTDIR+"/bin/"+fn[:-4]
        if (fn.endswith(".p3d")):   return OUTPUTDIR+"/bin/"+fn[:-4]
        if (fn.endswith(".lib")):   return OUTPUTDIR+"/lib/"+fn[:-4]+".a"
        if (fn.endswith(".ilb")):   return OUTPUTDIR+"/tmp/"+fn[:-4]+".a"
        if (fn.endswith(".rsrc")):  return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".plugin")):return OUTPUTDIR+"/plugins/"+fn
        if (fn.endswith(".app")):   return OUTPUTDIR+"/bin/"+fn
    else:
        if (fn.endswith(".obj")):   return OUTPUTDIR+"/tmp/"+fn[:-4]+".o"
        if (fn.endswith(".dll")):   return OUTPUTDIR+"/lib/"+fn[:-4]+".so"
        if (fn.endswith(".mll")):   return OUTPUTDIR+"/plugins/"+fn
        if (fn.endswith(".plugin")):return OUTPUTDIR+"/plugins/"+fn[:-7]+dllext+".so"
        if (fn.endswith(".exe")):   return OUTPUTDIR+"/bin/"+fn[:-4]
        if (fn.endswith(".p3d")):   return OUTPUTDIR+"/bin/"+fn[:-4]
        if (fn.endswith(".lib")):   return OUTPUTDIR+"/lib/"+fn[:-4]+".a"
        if (fn.endswith(".ilb")):   return OUTPUTDIR+"/tmp/"+fn[:-4]+".a"
    if (fn.endswith(".dat")):   return OUTPUTDIR+"/tmp/"+fn
    if (fn.endswith(".in")):    return OUTPUTDIR+"/pandac/input/"+fn
    return fn


def FindLocation(fn, ipath, pyabi=None):
    if GetLinkAllStatic():
        if fn.endswith(".dll"):
            fn = fn[:-4] + ".lib"
        elif fn.endswith(".pyd"):
            fn = "libpy.panda3d." \
               + os.path.splitext(fn[:-4] + GetExtensionSuffix())[0] + ".lib"

    loc = CalcLocation(fn, ipath)
    base, ext = os.path.splitext(fn)

    # If this is a target created with PyTargetAdd, we need to make sure it
    # it put in a Python-version-specific directory.
    if loc in PYABI_SPECIFIC:
        if loc.startswith(OUTPUTDIR + "/tmp"):
            if pyabi is not None:
                loc = OUTPUTDIR + "/tmp/" + pyabi + loc[len(OUTPUTDIR) + 4:]
            else:
                raise RuntimeError("%s is a Python-specific target, use PyTargetAdd instead of TargetAdd" % (fn))

        elif ext != ".pyd" and loc not in WARNED_FILES:
            WARNED_FILES.add(loc)
            Warn("file depends on Python but is not in an ABI-specific directory:", loc)

    ORIG_EXT[loc] = ext
    return loc


########################################################################
##
## These files maintain a python_versions.json file in the built/tmp
## directory that can be used by the other scripts in this directory.
##
########################################################################


def GetCurrentPythonVersionInfo():
    if PkgSkip("PYTHON"):
        return

    from distutils.sysconfig import get_python_lib
    return {
        "version": SDK["PYTHONVERSION"][6:9],
        "soabi": GetPythonABI(),
        "ext_suffix": GetExtensionSuffix(),
        "executable": sys.executable,
        "purelib": get_python_lib(False),
        "platlib": get_python_lib(True),
    }


def UpdatePythonVersionInfoFile(new_info):
    import json

    json_file = os.path.join(GetOutputDir(), "tmp", "python_versions.json")
    json_data = []
    if os.path.isfile(json_file) and not PkgSkip("PYTHON"):
        try:
            json_data = json.load(open(json_file, 'r'))
        except:
            json_data = []

        # Prune the list by removing the entries that conflict with our build,
        # plus the entries that no longer exist
        for version_info in json_data[:]:
            core_pyd = os.path.join(GetOutputDir(), "panda3d", "core" + version_info["ext_suffix"])
            if version_info["ext_suffix"] == new_info["ext_suffix"] or \
               version_info["soabi"] == new_info["soabi"] or \
               not os.path.isfile(core_pyd):
                json_data.remove(version_info)

    if not PkgSkip("PYTHON"):
        json_data.append(new_info)

    if VERBOSE:
        print("Writing %s" % (json_file))
    json.dump(json_data, open(json_file, 'w'), indent=4)


def ReadPythonVersionInfoFile():
    import json

    json_file = os.path.join(GetOutputDir(), "tmp", "python_versions.json")
    if os.path.isfile(json_file):
        try:
            return json.load(open(json_file, 'r'))
        except:
            pass

    return []


########################################################################
##
## TargetAdd
##
## Makepanda maintains a list of make-targets.  Each target has
## these attributes:
##
## name   - the name of the file being created.
## ext    - the original file extension, prior to OS-specific translation
## inputs - the names of the input files to the compiler
## deps   - other input files that the target also depends on
## opts   - compiler options, a catch-all category
##
## TargetAdd will create the target if it does not exist.  Then,
## depending on what options you pass, it will push data onto these
## various target attributes.  This is cumulative: for example, if
## you use TargetAdd to add compiler options, then use TargetAdd
## again with more compiler options, both sets of options will be
## included.
##
## TargetAdd does some automatic dependency generation on C++ files.
## It will scan these files for include-files and automatically push
## the include files onto the list of dependencies.  In order to do
## this, it needs an include-file search path.  So if you supply
## any C++ input, you also need to supply compiler options containing
## include-directories, or alternately, a separate ipath parameter.
##
## The main body of 'makepanda' is a long list of TargetAdd
## directives building up a giant list of make targets.  Then,
## finally, the targets are run and panda is built.
##
## Makepanda's dependency system does not understand multiple
## outputs from a single build step.  When a build step generates
## a primary output file and a secondary output file, it is
## necessary to trick the dependency system.  Insert a dummy
## build step that "generates" the secondary output file, using
## the primary output file as an input.  There is a special
## compiler option DEPENDENCYONLY that creates such a dummy
## build-step.  There are two cases where dummy build steps must
## be inserted: bison generates an OBJ and a secondary header
## file, interrogate generates an IN and a secondary IGATE.OBJ.
##
## PyTargetAdd is a special version for targets that depend on Python.
## It will create a target for each Python version we are building with,
## ensuring that builds with different Python versions won't conflict
## when we build for multiple Python ABIs side-by-side.
##
########################################################################

class Target:
    pass

TARGET_LIST = []
TARGET_TABLE = {}

def TargetAdd(target, dummy=0, opts=[], input=[], dep=[], ipath=None, winrc=None, pyabi=None):
    if (dummy != 0):
        exit("Syntax error in TargetAdd "+target)
    if ipath is None: ipath = opts
    if not ipath: ipath = []
    if (type(input) == str): input = [input]
    if (type(dep) == str): dep = [dep]

    if target.endswith(".pyd") and not pyabi:
        raise RuntimeError("Use PyTargetAdd to build .pyd targets")

    full = FindLocation(target, [OUTPUTDIR + "/include"], pyabi=pyabi)

    if (full not in TARGET_TABLE):
        t = Target()
        t.name = full
        t.inputs = []
        t.deps = {}
        t.opts = []
        TARGET_TABLE[full] = t
        TARGET_LIST.append(t)
    else:
        t = TARGET_TABLE[full]

    for x in opts:
        if x not in t.opts:
            t.opts.append(x)

    ipath = [OUTPUTDIR + "/tmp"] + GetListOption(ipath, "DIR:") + [OUTPUTDIR+"/include"]
    for x in input:
        fullinput = FindLocation(x, ipath, pyabi=pyabi)
        t.inputs.append(fullinput)
        # Don't re-link a library or binary if just its dependency dlls have been altered.
        # This should work out fine in most cases, and often reduces recompilation time.
        if (os.path.splitext(x)[-1] not in SUFFIX_DLL):
            t.deps[fullinput] = 1
            (base,suffix) = os.path.splitext(x)
            if (SUFFIX_INC.count(suffix)):
                for d in CxxCalcDependencies(fullinput, ipath, []):
                    t.deps[d] = 1
            elif suffix == '.java':
                for d in JavaCalcDependencies(fullinput, OUTPUTDIR + "/classes"):
                    t.deps[d] = 1

        # If we are linking statically, add the source DLL's dynamic dependencies.
        if GetLinkAllStatic() and ORIG_EXT[fullinput] == '.lib' and fullinput in TARGET_TABLE:
            tdep = TARGET_TABLE[fullinput]
            for y in tdep.inputs:
                if ORIG_EXT[y] == '.lib':
                    t.inputs.append(y)

            for opt, _ in LIBNAMES + LIBDIRECTORIES + FRAMEWORKDIRECTORIES:
                if opt in tdep.opts and opt not in t.opts:
                    t.opts.append(opt)

        if x.endswith(".in"):
            # Mark the _igate.cxx file as a dependency also.
            outbase = os.path.basename(x)[:-3]
            woutc = GetOutputDir()+"/tmp/"+outbase+"_igate.cxx"
            t.deps[woutc] = 1

        if target.endswith(".in"):
            # Add any .N files.
            base, ext = os.path.splitext(fullinput)
            fulln = base + ".N"
            if os.path.isfile(fulln):
                t.deps[fulln] = 1

    for x in dep:
        fulldep = FindLocation(x, ipath, pyabi=pyabi)
        t.deps[fulldep] = 1

    if winrc and GetTarget() == 'windows':
        TargetAdd(target, input=WriteResourceFile(target.split("/")[-1].split(".")[0], **winrc))

    ext = os.path.splitext(target)[1]
    if ext == ".in":
        if not CrossCompiling():
            t.deps[FindLocation("interrogate.exe", [])] = 1
        t.deps[FindLocation("dtool_have_python.dat", [])] = 1

    if ext in (".obj", ".tlb", ".res", ".plugin", ".app") or ext in SUFFIX_DLL or ext in SUFFIX_LIB:
        t.deps[FindLocation("platform.dat", [])] = 1

    if target.endswith(".obj") and any(x.endswith(".in") for x in input):
        if not CrossCompiling():
            t.deps[FindLocation("interrogate_module.exe", [])] = 1

    if target.endswith(".pz") and not CrossCompiling():
        t.deps[FindLocation("pzip.exe", [])] = 1

    if target.endswith(".in"):
        # Also add a target to compile the _igate.cxx file into an _igate.obj.
        outbase = os.path.basename(target)[:-3]
        woutc = OUTPUTDIR + "/tmp/" + outbase + "_igate.cxx"
        CxxDependencyCache[woutc] = []
        PyTargetAdd(outbase + "_igate.obj", opts=opts+['PYTHON','BIGOBJ'], input=woutc, dep=target)


def PyTargetAdd(target, opts=[], **kwargs):
    if PkgSkip("PYTHON"):
        return

    if 'PYTHON' not in opts:
        opts = opts + ['PYTHON']

    abi = GetPythonABI()

    MakeDirectory(OUTPUTDIR + "/tmp/" + abi)

    # Mark this target as being a Python-specific target.
    orig = CalcLocation(target, [OUTPUTDIR + "/include"])
    PYABI_SPECIFIC.add(orig)

    if orig.startswith(OUTPUTDIR + "/tmp/") and os.path.exists(orig):
        print("Removing file %s" % (orig))
        os.unlink(orig)

    TargetAdd(target, opts=opts, pyabi=abi, **kwargs)
