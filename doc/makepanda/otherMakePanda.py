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

import sys,os,time,stat,string,re,getopt,cPickle
from glob import glob

########################################################################
##
## Utility Routines
##
## filedate(f) - returns the last modified date of the specified file.
## youngest(f1,f2...) - returns date of most recently modified file.
## older(f,f1,f2...) - return true if f is older than all the others.
## xpaths(pre,pathlist,suf) - appends prefix and suffix to every path.
##
########################################################################

global FileDateCache
FileDateCache = {}

def filedate(path):
    global FileDateCache
    if FileDateCache.has_key(path):
        return FileDateCache[path]
    try: date = os.path.getmtime(path)
    except: date = 0
    FileDateCache[path] = date
    return date

def updatefiledate(path):
    global FileDateCache
    try: date = os.path.getmtime(path)
    except: date = 0
    FileDateCache[path] = date

def youngest(files):
    if type(files) == str:
        source = filedate(files)
        if (source==0):
            sys.exit("Error: source file not readable: "+files)
        return source
    result = 0
    for sfile in files:
        source = youngest(sfile)
        if (source > result): result = source
    return result

def older(file,others):
    return filedate(file)<youngest(others)

def xpaths(prefix,base,suffix):
    if type(base) == str:
        return prefix + base + suffix
    result = []
    for x in base:
        result.append(xpaths(prefix,x,suffix))
    return result

if sys.platform == "win32" or sys.platform == "cygwin":
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

def oslocalcmd(cd, cmd):
    if VERBOSE:
        if cd != ".":
            print "( cd "+cd+"; "+cmd+" )"
        else:
            print cmd
    if cd != ".":
        base=os.getcwd()
        os.chdir(cd)
    sys.stdout.flush()
    if sys.platform == "win32" or sys.platform == "cygwin":
        exe = cmd.split()[0]
        if os.path.isfile(exe)==0:
            for i in os.environ["PATH"].split(";"):
                if os.path.isfile(os.path.join(i, exe)):
                    exe = os.path.join(i, exe)
                    break
            if os.path.isfile(exe)==0:
                sys.exit("Cannot find "+exe+" on search path")
        res = os.spawnl(os.P_WAIT, exe, cmd)
    else:
        res = os.system(cmd)
    if res != 0:
        if not VERBOSE:
            print "\n------------- Command Failed ---------------"
            if cd != ".":
                print "( cd "+cd+"; "+cmd+" )"
            else:
                print cmd
            print "--------------------------------------------"
        sys.exit(res)
    if cd != ".":
        os.chdir(base)

def oscmd(cmd):
    oslocalcmd(".",cmd)

def osmove(src,dst):
    """
    Move src file or directory to dst.  dst will be removed if it
    exists (i.e. overwritten).
    """
    global VERBOSE
    if VERBOSE >= 1:
        print "Moving \"%s\" to \"%s\""%(src, dst)
    try: os.remove(dst)
    except OSError: pass
    os.rename(src, dst)

def replaceInFile(srcPath, dstPath, replaceA, withB):
    global VERBOSE
    if VERBOSE >= 1:
        print "Replacing '%s' in \"%s\" with '%s' and writing it to \"%s\""%(
            replaceA, srcPath, withB, dstPath)
    f=file(srcPath, "rb")
    data=f.read()
    f.close()
    data=data.replace(replaceA, withB)
    f=file(dstPath, "wb")
    f.write(data)
    f.close()

def buildingwhat(opts):
    building = 0
    for x in opts:
        if (x[:9]=="BUILDING_"): building = x[9:]
    return building

def ReadFile(wfile):
    try:
        srchandle = open(wfile, "rb")
        data = srchandle.read()
        srchandle.close()
        return data
    except: sys.exit("Cannot read "+wfile)

def WriteFile(wfile,data):
    try:
        dsthandle = open(wfile, "wb")
        dsthandle.write(data)
        dsthandle.close()
    except: sys.exit("Cannot write "+wfile)

def prettyTime(t):
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

########################################################################
##
## Default options:
##
## You might be tempted to change the defaults by editing them
## here.  Don't do it.  Instead, create a script that compiles
## panda with your preferred options.
##
########################################################################

if (sys.platform == "win32"): COMPILERS=["MSVC7", "MSVC71", "MINGW"]
elif (sys.platform == "cygwin"): COMPILERS=["MSVC7", "MSVC71", "GCC33"]
elif (sys.platform == "linux2"): COMPILERS=["LINUXA"]
else:
    print "which compiler should be used for %s"%(sys.platform,)
PREFIX="built"
COMPILER=COMPILERS[0]
OPTIMIZE="3"
INSTALLER=0
GENMAN=0
PPGAME=0
COMPLETE=0
THIRDPARTY="thirdparty"
VERSION="0.0.0"
VERBOSE=1
COMPRESSOR="zlib"
PACKAGES=["ZLIB","PNG","JPEG","TIFF","VRPN","FMOD","NVIDIACG","HELIX","NSPR",
          "SSL","FREETYPE","FFTW","MILES","MAYA5","MAYA6","MAYA65","MAX5","MAX6","MAX7"]
OMIT=PACKAGES[:]
WARNINGS=[]

SDK_LIB_PATH = {}

DIRECTXSDK = None
MAYASDK = {}
MAXSDK = {}
MAXSDKCS = {}
NSPR_SDK = None
PYTHONSDK = None

try:
    # If there is a makepandaPreferences.py, import it:
    from makepandaPreferences import *
except ImportError:
    # If it's not there, no problem:
    pass

STARTTIME=time.time()

##########################################################################################
#
# Read the default version number out of dtool/PandaVersion.pp
#
##########################################################################################

try:
    f = file("dtool/PandaVersion.pp","r")
    pattern = re.compile('^[ \t]*[#][ \t]*define[ \t]+PANDA_VERSION[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)')
    for line in f:
        match = pattern.match(line,0)
        if (match):
            VERSION = match.group(1)+"."+match.group(2)+"."+match.group(3)
            break
    f.close()
except: pass

##########################################################################################
#
# Initialize DTOOLCONFIG based on platform (Win/Unix)
#
# These are the defaults for the two broad classes of operating system.
# Subsequent analysis will cause these values to be tweaked.
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
    ("HAVE_SSL",                       'UNDEF',                  'UNDEF'),
    ("HAVE_NET",                       'UNDEF',                  'UNDEF'),
    ("HAVE_CG",                        'UNDEF',                  'UNDEF'),
    ("HAVE_CGGL",                      'UNDEF',                  'UNDEF'),
    ]

DTOOLCONFIG={}
if (sys.platform == "win32" or sys.platform == "cygwin"):
    for key,win,unix in DTOOLDEFAULTS:
        DTOOLCONFIG[key] = win
else:
    for key,win,unix in DTOOLDEFAULTS:
        DTOOLCONFIG[key] = unix

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
              "uri?"
              (for .??? files)

    MAYA5     Maya version 5
    MAYA6     Maya version 6
              "uri?"
              (for .??? files)

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
    HELIX
              "uri?"
              (for .??? files)
              A ??? library.

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
        print '***', problem, '***'
    print ""
    print "Makepanda generates a 'built' subdirectory containing a"
    print "compiled copy of Panda3D.  Command-line arguments are:"
    print ""
    print "  --help            (print the help message you're reading now)"
    print "  --package-info    (help info about the optional packages)"
    print "  --prefix X        (install into prefix dir, default \"built\")"
    print "  --compiler X      (currently, compiler can only be MSVC7,LINUXA)"
    print "  --optimize X      (optimization level can be 1,2,3,4)"
    print "  --thirdparty X    (directory containing third-party software)"
    print "  --complete        (copy samples and direct into the build)"
    print "  --installer       (build an executable installer)"
    print "  --ppgame X        (build a prepackaged game - see manual)"
    print "  --v1 X            (set the major version number)"
    print "  --v2 X            (set the minor version number)"
    print "  --v3 X            (set the sequence version number)"
    print "  --lzma            (use lzma compression when building installer)"
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
    sys.exit(1)

def parseopts(args):
    global PREFIX,COMPILER,OPTIMIZE,OMIT,THIRDPARTY,INSTALLER,GENMAN
    global PPGAME,COPYEXTRAS,VERSION,COMPRESSOR,DIRECTXSDK,VERBOSE
    longopts = [
        "help","package-info","prefix=","compiler=","directx-sdk=","thirdparty=",
        "optimize=","everything","nothing","installer","ppgame=","quiet","verbose",
        "complete","version=","lzma"]
    anything = 0
    for pkg in PACKAGES: longopts.append("no-"+pkg.lower())
    for pkg in PACKAGES: longopts.append("use-"+pkg.lower())
    try:
        opts, extras = getopt.getopt(args, "", longopts)
        for option,value in opts:
            if (option=="--help"): raise "usage"
            elif (option=="--package-info"): raise "package-info"
            elif (option=="--prefix"): PREFIX=value
            elif (option=="--compiler"): COMPILER=value
            elif (option=="--directx-sdk"): DIRECTXSDK=value
            elif (option=="--thirdparty"): THIRDPARTY=value
            elif (option=="--optimize"): OPTIMIZE=value
            elif (option=="--quiet"): VERBOSE-=1
            elif (option=="--verbose"): VERBOSE+=1
            elif (option=="--installer"): INSTALLER=1
            elif (option=="--genman"): GENMAN=1
            elif (option=="--ppgame"): PPGAME=value
            elif (option=="--complete"): COMPLETE=1
            elif (option=="--everything"): OMIT=[]
            elif (option=="--nothing"): OMIT=PACKAGES[:]
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
    except "help": usage('')
    except Exception, e: usage(e)
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
# Avoid trouble by not allowing weird --prefix or --thirdparty
#
# One of my goals for makepanda was for it to be maintainable.
# I found that trying to support arbitrary pathnames for "prefix"
# and "thirdparty" required the use of lots of backslashes and
# quotation marks, which were quite frankly hard to get right.
# I think it's better to simply rule out weird pathnames.
#
########################################################################

PREFIX     = PREFIX.replace("\\","/")
THIRDPARTY = THIRDPARTY.replace("\\","/")

if (PREFIX.count(" ")  or THIRDPARTY.count(" ")):
  sys.exit("The --prefix and --thirdparty may not contain spaces")
if (PREFIX.count('"')  or THIRDPARTY.count('"')):
  sys.exit("The --prefix and --thirdparty may not contain quotation marks")

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

if (sys.platform == "win32" or sys.platform == "cygwin") and DIRECTXSDK is None:
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
                sys.exit("The registry does not appear to contain a pointer to the DirectX 9.0 SDK.")
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
        if (sys.platform == "win32" or sys.platform == "cygwin"):
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
        if (sys.platform == "win32" or sys.platform == "cygwin"):
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
## Locate the NSPR SDK
##
########################################################################

if NSPR_SDK is None:
    if sys.platform == "win32" or sys.platform == "cygwin":
        nsprPaths = ["C:/Python22", 'thirdparty/nspr']
    else:
        nsprPaths = ["/usr/include/nspr", 'thirdparty/win-python']
    for p in nsprPaths:
        if os.path.isdir(p): NSPR_SDK = p
    if NSPR_SDK is None:
        sys.exit("Cannot find the NSPR SDK")

########################################################################
##
## Locate the Python SDK
##
########################################################################

if PYTHONSDK is None:
    if sys.platform == "win32" or sys.platform == "cygwin":
        pythonPaths = [
            "C:/Python22",
            "C:/Python23",
            "C:/Python24",
            "C:/Python25",
            'thirdparty/win-python']
    else:
        pythonPaths = [
            "/usr/include/python2.5",
            "/usr/include/python2.4",
            "/usr/include/python2.3",
            "/usr/include/python2.2"]
    for p in pythonPaths:
        if os.path.isdir(p): PYTHONSDK = p
    if PYTHONSDK is None:
        sys.exit("Cannot find the Python SDK")
            
########################################################################
##
## Locate the SSL SDK
##
########################################################################

if sys.platform == "win32" or sys.platform == "cygwin":
    SDK_SEARCH_PATHS = {
        'ssl': [
            "C:/openssl",
            'thirdparty/ssl']
else:
    SDK_SEARCH_PATHS = {
        'ssl': [
            "/usr/include/ssl",
            'thirdparty/win-python']
for package in packages:
    if SDK_LIB_PATH.get(package) is None:
        for p in SDK_SEARCH_PATHS.get(package, '.'):
            if os.path.isdir(p):
                SDK_LIB_PATH[package] = p
    if SDK_LIB_PATH.get(package) is None or not os.path.isdir(SDK_LIB_PATH.get(package)):
        sys.exit("The SDK for %s was not found."%(package,))

########################################################################
##
## Locate Visual Studio 7.0 or 7.1
##
## The visual studio compiler doesn't work unless you set up a
## couple of environment variables to point at the compiler.
##
########################################################################

if (COMPILER == "MSVC7" or COMPILER=="MSVC71"):
    vcdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\7.1", "InstallDir")
    if ((vcdir == 0) or (vcdir[-13:] != "\\Common7\\IDE\\")):
        vcdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\7.0", "InstallDir")
        if ((vcdir == 0) or (vcdir[-13:] != "\\Common7\\IDE\\")):
            sys.exit("The registry does not appear to contain a pointer to the Visual Studio 7 install directory")
    vcdir = vcdir[:-12]
    old_env_path    = ""
    old_env_include = ""
    old_env_lib     = ""
    if (os.environ.has_key("PATH")):    old_env_path    = os.environ["PATH"]
    if (os.environ.has_key("INCLUDE")): old_env_include = os.environ["INCLUDE"]
    if (os.environ.has_key("LIB")):     old_env_lib     = os.environ["LIB"]
    os.environ["PATH"] = vcdir + "vc7\\bin;" + vcdir + "Common7\\IDE;" + vcdir + "Common7\\Tools;" + vcdir + "Common7\\Tools\\bin\\prerelease;" + vcdir + "Common7\\Tools\\bin;" + old_env_path
    os.environ["INCLUDE"] = vcdir + "vc7\\ATLMFC\\INCLUDE;" + vcdir + "vc7\\include;" + vcdir + "vc7\\PlatformSDK\\include\\prerelease;" + vcdir + "vc7\\PlatformSDK\\include;" + old_env_include
    os.environ["LIB"] = vcdir + "vc7\\ATLMFC\\LIB;" + vcdir + "vc7\\LIB;" + vcdir + "vc7\\PlatformSDK\\lib\\prerelease;" + vcdir + "vc7\\PlatformSDK\\lib;" + old_env_lib
    sys.stdout.flush()

##########################################################################################
#
# Disable Helix unless running under Windows
#
##########################################################################################

if (sys.platform != "win32" or sys.platform == "cygwin"):
    if (OMIT.count("HELIX")==0):
        WARNINGS.append("HELIX not yet supported under linux")
        WARNINGS.append("I have automatically added this command-line option: --no-helix")
        OMIT.append("HELIX")

##########################################################################################
#
# See if there's a "MILES" subdirectory under 'thirdparty'
#
##########################################################################################

if (os.path.isdir(os.path.join(THIRDPARTY, "win-libs-vc7", "miles"))==0):
    if (OMIT.count("MILES")==0):
        WARNINGS.append("You do not have a copy of MILES sound system")
        WARNINGS.append("I have automatically added this command-line option: --no-miles")
        OMIT.append("MILES")

##########################################################################################
#
# Enable or Disable runtime debugging mechanisms based on optimize level.
#
##########################################################################################

for x in PACKAGES:
    if (OMIT.count(x)==0):
        if (DTOOLCONFIG.has_key("HAVE_"+x)):
            DTOOLCONFIG["HAVE_"+x] = '1'

DTOOLCONFIG["HAVE_NET"] = DTOOLCONFIG["HAVE_NSPR"]

if (OMIT.count("NVIDIACG")==0):
    DTOOLCONFIG["HAVE_CG"] = '1'
    DTOOLCONFIG["HAVE_CGGL"] = '1'

if (OPTIMIZE <= 3):
    if (DTOOLCONFIG["HAVE_NET"] != 'UNDEF'):
        DTOOLCONFIG["DO_PSTATS"] = '1'

if (OPTIMIZE <= 3):
    DTOOLCONFIG["DO_COLLISION_RECORDING"] = '1'

#if (OPTIMIZE <= 2):
#    DTOOLCONFIG["TRACK_IN_INTERPRETER"] = '1'

if (OPTIMIZE <= 3):
    DTOOLCONFIG["DO_MEMORY_USAGE"] = '1'

#if (OPTIMIZE <= 1):
#    DTOOLCONFIG["DO_PIPELINING"] = '1'

if (OPTIMIZE <= 3):
    DTOOLCONFIG["NOTIFY_DEBUG"] = '1'

##########################################################################################
#
# Verify that LD_LIBRARY_PATH contains the PREFIX/lib directory.
#
# If not, add it on a temporary basis, and issue a warning.
#
##########################################################################################

if (sys.platform != "win32" and sys.platform != "cygwin"):
    BUILTLIB = os.path.abspath(PREFIX+"/lib")
    try:
        LDPATH = []
        f = file("/etc/ld.so.conf","r")
        for line in f: LDPATH.append(line.rstrip())
        f.close()
    except: LDPATH = []
    if (os.environ.has_key("LD_LIBRARY_PATH")):
        LDPATH = LDPATH + os.environ["LD_LIBRARY_PATH"].split(":")
    if (LDPATH.count(BUILTLIB)==0):
        WARNINGS.append("Caution: the "+PREFIX+"/lib directory is not in LD_LIBRARY_PATH")
        WARNINGS.append("or /etc/ld.so.conf.  You must add it before using panda.")
        if (os.environ.has_key("LD_LIBRARY_PATH")):
            os.environ["LD_LIBRARY_PATH"] = BUILTLIB + ":" + os.environ["LD_LIBRARY_PATH"]
        else:
            os.environ["LD_LIBRARY_PATH"] = BUILTLIB

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
        print "Makepanda: Prefix Directory:",PREFIX
        print "Makepanda: Compiler:",COMPILER
        print "Makepanda: Optimize:",OPTIMIZE
        print "Makepanda: Keep Pkg:",tkeep
        print "Makepanda: Omit Pkg:",tomit
        print "Makepanda: Thirdparty dir:",THIRDPARTY
        print "Makepanda: DirectX SDK dir:",DIRECTXSDK
        print "Makepanda: Verbose vs. Quiet Level:",VERBOSE
        print "Makepanda: PYTHONSDK:", PYTHONSDK
        if (GENMAN): print "Makepanda: Generate API reference manual"
        else       : print "Makepanda: Don't generate API reference manual"
        if (sys.platform == "win32" or sys.platform == "cygwin"):
            if INSTALLER:  print "Makepanda: Build installer, using",COMPRESSOR
            else        :  print "Makepanda: Don't build installer"
            if PPGAME!=0:  print "Makepanda: Build pprepackaged game ",PPGAME,"using",COMPRESSOR
            else        :  print "Makepanda: Don't build pprepackaged game"
        print "Makepanda: Version ID: "+VERSION
        for x in warnings: print "Makepanda: "+x
        print "-------------------------------------------------------------------"
        print ""
        sys.stdout.flush()

printStatus("Makepanda Initial Status Report", WARNINGS)


##########################################################################################
#
# Create the directory tree
#
##########################################################################################

MakeDirectory(PREFIX)
MakeDirectory(PREFIX+"/bin")
MakeDirectory(PREFIX+"/lib")
MakeDirectory(PREFIX+"/etc")
MakeDirectory(PREFIX+"/plugins")
MakeDirectory(PREFIX+"/pandac")
MakeDirectory(PREFIX+"/pandac/input")
MakeDirectory(PREFIX+"/include")
MakeDirectory(PREFIX+"/include/parser-inc")
MakeDirectory(PREFIX+"/include/parser-inc/openssl")
MakeDirectory(PREFIX+"/include/parser-inc/Cg")
MakeDirectory(PREFIX+"/include/openssl")
MakeDirectory(PREFIX+"/direct")
MakeDirectory(PREFIX+"/tmp")

########################################################################
##
## PkgSelected(package-list,package)
##
## This function returns true if the package-list contains the
## package, AND the OMIT list does not contain the package.
##
########################################################################

def PkgSelected(pkglist, pkg):
    if (pkglist.count(pkg)==0): return 0
    if (OMIT.count(pkg)): return 0
    return 1

########################################################################
##
## These two globals accumulate a global list of everything compiled.
##
########################################################################

ALLIN=[]
ALLTARGETS=[]

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

global CxxIncludeCache
CxxIncludeCache = {}

iCachePath=PREFIX+"/tmp/makepanda-icache"
try: icache = open(iCachePath,'rb')
except: icache = 0
if (icache!=0):
    CxxIncludeCache = cPickle.load(icache)
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
    if (CxxIncludeCache.has_key(path)):
        cached = CxxIncludeCache[path]
        if (cached[0]==date): return cached[1]
    try: sfile = open(path, 'rb')
    except: sys.exit("Cannot open source file \""+path+"\" for reading.")
    include = []
    for line in sfile:
        match = CxxIncludeRegex.match(line,0)
        if (match):
            incname = match.group(1)
            include.append(incname)
    sfile.close()
    CxxIncludeCache[path] = [date, include]
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
        if (last < 0): sys.exit("CxxFindHeader cannot handle this case #1")
        srcdir = srcfile[:last+1]
        while (incfile[:1]=="."):
            if (incfile[:2]=="./"):
                incfile = incfile[2:]
            elif (incfile[:3]=="../"):
                incfile = incfile[3:]
                last = srcdir[:-1].rfind("/")
                if (last < 0): sys.exit("CxxFindHeader cannot handle this case #2")
                srcdir = srcdir[:last+1]
            else: sys.exit("CxxFindHeader cannot handle this case #3")
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
            if (header==0):
                print "CAUTION: header file "+include+" cannot be found."
            else:
                if (ignore.count(header)==0):
                    hdeps = CxxCalcDependencies(header, ipath, [srcfile]+ignore)
                    for x in hdeps: dep[x] = 1
    result = dep.keys()
    CxxDependencyCache[srcfile] = result
    return result

def CxxCalcDependenciesAll(srcfiles, ipath):
    dep = {}
    for srcfile in srcfiles:
        for x in CxxCalcDependencies(srcfile, ipath, []):
            dep[x] = 1
    return dep.keys()

########################################################################
##
## ConditionalWriteFile
##
## Creates the given file, but only if it doesn't already
## contain the correct contents.
##
########################################################################

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

########################################################################
##
## CopyFile
##
## Copy a file into the build tree.
##
########################################################################

def CopyFile(dstfile,srcfile):
    if (dstfile[-1]=='/'):
        dstdir = dstfile
        fnl = srcfile.rfind("/")
        if (fnl < 0): fn = srcfile
        else: fn = srcfile[fnl+1:]
        dstfile = dstdir + fn
    if (older(dstfile,srcfile)):
        global VERBOSE
        if VERBOSE >= 1:
            print "Copying \"%s\" --> \"%s\""%(srcfile, dstfile)
        WriteFile(dstfile,ReadFile(srcfile))
        updatefiledate(dstfile)
    ALLTARGETS.append(dstfile)

########################################################################
##
## CopyAllFiles
##
## Copy the contents of an entire directory into the build tree.
##
########################################################################

def CopyAllFiles(dstdir, srcdir, suffix=""):
    suflen = len(suffix)
    files = os.listdir(srcdir)
    for x in files:
        if (os.path.isfile(srcdir+x)):
            if (suflen==0) or (x[-suflen:]==suffix):
                CopyFile(dstdir+x, srcdir+x)

########################################################################
##
## CopyTree
##
## Copy a directory into the build tree.
##
########################################################################

def CopyTree(dstdir,srcdir):
    if (os.path.isdir(dstdir)): return 0
    if (COMPILER=="MSVC7" or COMPILER=="MSVC71"): cmd = 'xcopy.exe /I/Y/E/Q "' + srcdir + '" "' + dstdir + '"'
    elif (COMPILER=="LINUXA"): cmd = 'cp --recursive --force ' + srcdir + ' ' + dstdir
    oscmd(cmd)
    updatefiledate(dstdir)

def CompileBison(pre, dstc, dsth, src):
    """
    Generate a CXX file from a source YXX file.
    """
    (base, fn) = os.path.split(src)
    dstc=base+"/"+dstc
    dsth=base+"/"+dsth
    if (older(dstc,src) or older(dsth,src)):
        CopyFile(PREFIX+"/tmp/", src)
        if (COMPILER=="MSVC7"):
            CopyFile(PREFIX+"/tmp/", "thirdparty/win-util/bison.simple")
            bisonFullPath=os.path.abspath("thirdparty/win-util/bison.exe")
            oslocalcmd(PREFIX+"/tmp", bisonFullPath+" -y -d -p " + pre + " " + fn)
            osmove(PREFIX+"/tmp/y_tab.c", dstc)
            osmove(PREFIX+"/tmp/y_tab.h", dsth)
        elif (COMPILER=="LINUXA"):
            oslocalcmd(PREFIX+"/tmp", "bison -y -d -p "+pre+" "+fn)
            osmove(PREFIX+"/tmp/y.tab.c", dstc)
            osmove(PREFIX+"/tmp/y.tab.h", dsth)
        else:
            oslocalcmd(PREFIX+"/tmp", "bison -y -d -p "+pre+" "+fn)
            osmove(PREFIX+"/tmp/y.tab.c", dstc)
            osmove(PREFIX+"/tmp/y.tab.h", dsth)
        updatefiledate(dstc)
        updatefiledate(dsth)

def CompileFlex(pre,dst,src,dashi):
    """
    Generate a CXX file from a source LXX file.
    """
    last = src.rfind("/")
    fn = src[last+1:]
    dst = PREFIX+"/tmp/"+dst
    if (older(dst,src)):
        CopyFile(PREFIX+"/tmp/", src)
        if (COMPILER=="MSVC7" or COMPILER=="MSVC71"):
            flexFullPath=os.path.abspath("thirdparty/win-util/flex.exe")
            if (dashi): oslocalcmd(PREFIX+"/tmp", flexFullPath+" -i -P" + pre + " -olex.yy.c " + fn)
            else:       oslocalcmd(PREFIX+"/tmp", flexFullPath+"    -P" + pre + " -olex.yy.c " + fn)
            replaceInFile(PREFIX+'/tmp/lex.yy.c', dst, '#include <unistd.h>', '')
        elif (COMPILER=="LINUXA"):
            if (dashi): oslocalcmd(PREFIX+"/tmp", "flex -i -P" + pre + " -olex.yy.c " + fn)
            else:       oslocalcmd(PREFIX+"/tmp", "flex    -P" + pre + " -olex.yy.c " + fn)
            oscmd('cp '+PREFIX+'/tmp/lex.yy.c '+dst)
        updatefiledate(dst)

########################################################################
##
## CompileC
##
## Generate an OBJ file from a source CXX file.
##
########################################################################

priorIPath=None
def checkIfNewDir(path):
    global priorIPath
    if priorIPath != path:
        print "\nStarting compile in \"%s\" (%s):\n"%(path,prettyTime(time.time()-STARTTIME),)
    priorIPath=path

def CompileC(obj=0,src=0,ipath=[],opts=[]):
    global VERBOSE
    if ((obj==0)|(src==0)): sys.exit("syntax error in CompileC directive")
    ipath = [PREFIX+"/tmp"] + ipath + [PREFIX+"/include"]
    fullsrc = CxxFindSource(src, ipath)
    if (fullsrc == 0): sys.exit("Cannot find source file "+src)
    dep = CxxCalcDependencies(fullsrc, ipath, [])

    if (COMPILER=="MSVC7" or COMPILER=="MSVC71"):
        wobj = PREFIX+"/tmp/"+obj
        if (older(wobj, dep)):
            if VERBOSE >= 0:
                checkIfNewDir(ipath[1])
            cmd = "cl.exe /Fo" + wobj + " /nologo /c"
            cmd = cmd + " /I" + PREFIX + "/python/include"
            if (opts.count("DXSDK")): cmd = cmd + ' /I"' + DIRECTXSDK + '/include"'
            for ver in ["MAYA5","MAYA6","MAYA65"]:
              if (opts.count(ver)): cmd = cmd + ' /I"' + MAYASDK[ver] + '/include"'
            for max in ["MAX5","MAX6","MAX7"]:
                if (PkgSelected(opts,max)):
                    cmd = cmd + ' /I"' + MAXSDK[max] + '/include" /I"' + MAXSDKCS[max] + '" /D' + max
            for pkg in PACKAGES:
                if (pkg[:4] != "MAYA") and PkgSelected(opts,pkg):
                    cmd = cmd + " /I" + SDK_LIB_PATH.get(pkg.lower(), '.') + "/include"
            for x in ipath: cmd = cmd + " /I" + x
            if (opts.count('NOFLOATWARN')): cmd = cmd + ' /wd4244 /wd4305'
            if (opts.count("WITHINPANDA")): cmd = cmd + ' /DWITHIN_PANDA'
            if (OPTIMIZE==1): cmd = cmd + " /Zc:forScope /MD /Zi /O2 /Ob2 /DFORCE_INLINING /RTCs /GS"
            if (OPTIMIZE==2): cmd = cmd + " /Zc:forScope /MD /Zi /O2 /Ob2 /DFORCE_INLINING "
            if (OPTIMIZE==3): cmd = cmd + " /Zc:forScope /MD /Zi /O2 /Ob2 /DFORCE_INLINING "
            if (OPTIMIZE==4): cmd = cmd + " /Zc:forScope /MD /Zi /O2 /Ob2 /DFORCE_INLINING /GL /DNDEBUG "
            cmd = cmd + " /Fd" + wobj[:-4] + ".pdb"
            building = buildingwhat(opts)
            if (building): cmd = cmd + " /DBUILDING_" + building
            cmd = cmd + " /EHsc /Zm300 /DWIN32_VC /DWIN32 /W3 " + fullsrc
            oscmd(cmd)
            updatefiledate(wobj)
    elif (COMPILER=="LINUXA"):
        wobj = PREFIX+"/tmp/" + obj[:-4] + ".o"
        if (older(wobj, dep)):
            if VERBOSE >= 0:
                checkIfNewDir(ipath[1])
            if (src[-2:]==".c"): cmd = 'gcc -c -o ' + wobj
            else:                cmd = 'g++ -ftemplate-depth-30 -c -o ' + wobj
            cmd = cmd + ' -I"' + PYTHONSDK + '"'
            if (PkgSelected(opts,"VRPN")):     cmd = cmd + ' -I' + THIRDPARTY + '/linux-libs-a/vrpn/include'
            if (PkgSelected(opts,"FFTW")):     cmd = cmd + ' -I' + THIRDPARTY + '/linux-libs-a/fftw/include'
            if (PkgSelected(opts,"FMOD")):     cmd = cmd + ' -I' + THIRDPARTY + '/linux-libs-a/fmod/include'
            if (PkgSelected(opts,"NVIDIACG")): cmd = cmd + ' -I' + THIRDPARTY + '/linux-libs-a/nvidiacg/include'
            if (PkgSelected(opts,"NSPR")):     cmd = cmd + ' -I' + NSPR_SDK + '/include'
            if (PkgSelected(opts,"FREETYPE")): cmd = cmd + ' -I/usr/include/freetype2'
            for x in ipath: cmd = cmd + ' -I' + x
            if (opts.count("WITHINPANDA")): cmd = cmd + ' -DWITHIN_PANDA'
            if (OPTIMIZE==1): cmd = cmd + " -g"
            if (OPTIMIZE==2): cmd = cmd + " -O1"
            if (OPTIMIZE==3): cmd = cmd + " -O2"
            if (OPTIMIZE==4): cmd = cmd + " -O2"
            building = buildingwhat(opts)
            if (building): cmd = cmd + " -DBUILDING_" + building
            cmd = cmd + ' ' + fullsrc
            oscmd(cmd)
            updatefiledate(wobj)

########################################################################
##
## CompileRES
##
## Generate an RES file from a source RC file.
##
########################################################################

def CompileRES(obj=0,src=0,ipath=[],opts=[]):
    if ((obj==0)|(src==0)): sys.exit("syntax error in CompileRES directive")
    fullsrc = CxxFindSource(src, ipath)
    if (fullsrc == 0): sys.exit("Cannot find source file "+src)
    obj = PREFIX+"/tmp/"+obj
    wdep = CxxCalcDependencies(fullsrc, ipath, [])

    if (COMPILER=="MSVC7" or COMPILER=="MSVC71"):
        if (older(obj, wdep)):
            cmd = 'rc.exe /d "NDEBUG" /l 0x409'
            for x in ipath: cmd = cmd + " /I" + x
            cmd = cmd + ' /fo' + obj
            cmd = cmd + ' ' + fullsrc
            oscmd(cmd)
            updatefiledate(obj)
    elif (COMPILER=="LINUXA"):
        sys.exit("Can only compile RES files on Windows.")

########################################################################
##
## Interrogate
##
## Generate an IN file and a CXX-stub file from CXX source files
##
########################################################################

def Interrogate(ipath=0, opts=0, outd=0, outc=0, src=0, module=0, library=0, files=0):
    if ((ipath==0)|(opts==0)|(outd==0)|(outc==0)|(src==0)|(module==0)|(library==0)|(files==0)):
        sys.exit("syntax error in Interrogate directive")
    ALLIN.append(outd)
    ipath = [PREFIX+"/tmp"] + ipath + [PREFIX+"/include"]
    outd = PREFIX+"/pandac/input/"+outd
    outc = PREFIX+"/tmp/"+outc
    paths = xpaths(src+"/",files,"")
    dep = CxxCalcDependenciesAll(paths, ipath)
    dotdots = ""
    for i in range(0,src.count("/")+1): dotdots = dotdots + "../"
    building = 0
    for x in opts:
        if (x[:9]=="BUILDING_"): building = x[9:]
    if (older(outc, dep) or older(outd, dep)):
        if (COMPILER=="MSVC7"):
            cmd = dotdots + PREFIX + "/bin/interrogate.exe"
            cmd = cmd + ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -longlong __int64 -D_X86_ -DWIN32_VC -D_WIN32'
            cmd = cmd + ' -D"_declspec(param)=" -D_near -D_far -D__near -D__far -D__stdcall'
            if (OPTIMIZE==1): cmd = cmd + ' '
            if (OPTIMIZE==2): cmd = cmd + ' '
            if (OPTIMIZE==3): cmd = cmd + ' -DFORCE_INLINING'
            if (OPTIMIZE==4): cmd = cmd + ' -DFORCE_INLINING'
            cmd = cmd + ' -S' + dotdots + PREFIX + '/include/parser-inc'
            cmd = cmd + ' -I' + dotdots + PREFIX + '/python/include'
            for pkg in PACKAGES:
                if (PkgSelected(opts,pkg)):
                    cmd = cmd + ' -I' + dotdots + THIRDPARTY + "/win-libs-vc7/" + pkg.lower() + "/include"
        elif (COMPILER=="MSVC71"):
            cmd = dotdots + PREFIX + "/bin/interrogate.exe"
            cmd = cmd + ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -longlong __int64 -D_X86_ -DWIN32_VC -D_WIN32'
            cmd = cmd + ' -D"_declspec(param)=" -D_near -D_far -D__near -D__far -D__stdcall'
            if (OPTIMIZE==1): cmd = cmd + ' '
            if (OPTIMIZE==2): cmd = cmd + ' '
            if (OPTIMIZE==3): cmd = cmd + ' -DFORCE_INLINING'
            if (OPTIMIZE==4): cmd = cmd + ' -DFORCE_INLINING'
            cmd = cmd + ' -S' + dotdots + PREFIX + '/include/parser-inc'
            cmd = cmd + ' -I' + dotdots + PREFIX + '/python/include'
            for pkg in PACKAGES:
                if (PkgSelected(opts,pkg)):
                    cmd = cmd + ' -I' + dotdots + THIRDPARTY + "/win-libs-vc7/" + pkg.lower() + "/include"
        elif (COMPILER=="LINUXA"):
            cmd = dotdots + PREFIX + '/bin/interrogate'
            cmd = cmd + ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -D__i386__ -D__const=const'
            if (OPTIMIZE==1): cmd = cmd + ' '
            if (OPTIMIZE==2): cmd = cmd + ' '
            if (OPTIMIZE==3): cmd = cmd + ' '
            if (OPTIMIZE==4): cmd = cmd + ' '
            cmd = cmd + ' -S' + dotdots + PREFIX + '/include/parser-inc -S/usr/include'
            cmd = cmd + ' -I' + dotdots + PREFIX + '/python/include'
            for pkg in PACKAGES:
                if (PkgSelected(opts,pkg)):
                    cmd = cmd + ' -I' + dotdots + THIRDPARTY + "/linux-libs-a/" + pkg.lower() + "/include"
        else:
            cmd = dotdots + PREFIX + '/bin/interrogate'
            cmd = cmd + ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -D__i386__ -D__const=const'
            if (OPTIMIZE==1): cmd = cmd + ' '
            if (OPTIMIZE==2): cmd = cmd + ' '
            if (OPTIMIZE==3): cmd = cmd + ' '
            if (OPTIMIZE==4): cmd = cmd + ' '
            cmd = cmd + ' -S' + dotdots + PREFIX + '/include/parser-inc -S/usr/include'
            cmd = cmd + ' -I' + dotdots + PREFIX + '/python/include'
            for pkg in PACKAGES:
                if (PkgSelected(opts,pkg)):
                    cmd = cmd + ' -I' + dotdots + THIRDPARTY + "/linux-libs-a/" + pkg.lower() + "/include"
        cmd = cmd + ' -oc ' + dotdots + outc + ' -od ' + dotdots + outd
        cmd = cmd + ' -fnames -string -refcount -assert -python'
        for x in ipath: cmd = cmd + ' -I' + dotdots + x
        if (building): cmd = cmd + " -DBUILDING_"+building
        if (opts.count("WITHINPANDA")): cmd = cmd + " -DWITHIN_PANDA"
        cmd = cmd + ' -module ' + module + ' -library ' + library
        if ((COMPILER=="MSVC7" or COMPILER=="MSVC71") and opts.count("DXSDK")):
            cmd = cmd + ' -I"' + DIRECTXSDK + '/include"'
        for ver in ["MAYA5","MAYA6","MAYA65"]:
          if ((COMPILER=="MSVC7" or COMPILER=="MSVC71") and opts.count(ver)):
              cmd = cmd + ' -I"' + MAYASDK[ver] + '/include"'
        for x in files: cmd = cmd + ' ' + x
        oslocalcmd(src, cmd)
        updatefiledate(outd)
        updatefiledate(outc)

########################################################################
##
## InterrogateModule
##
## Generate a python-stub CXX file from a bunch of IN files.
##
########################################################################

def InterrogateModule(outc=0, module=0, library=0, files=0):
    if ((outc==0)|(module==0)|(library==0)|(files==0)):
        sys.exit("syntax error in InterrogateModule directive")
    outc = PREFIX+"/tmp/"+outc
    files = xpaths(PREFIX+"/pandac/input/",files,"")
    if (older(outc, files)):
        global VERBOSE
        if VERBOSE >= 1:
            print "Generating Python-stub cxx file for %s"%(library,)
        if (COMPILER=="MSVC7" or COMPILER=="MSVC71"):
            cmd = PREFIX + '/bin/interrogate_module.exe '
        elif (COMPILER=="LINUXA"):
            cmd = PREFIX + '/bin/interrogate_module '
        cmd = cmd + ' -oc ' + outc + ' -module ' + module + ' -library ' + library + ' -python '
        for x in files: cmd = cmd + ' ' + x
        oscmd(cmd)
        updatefiledate(outc)

########################################################################
##
## CompileLIB
##
## Generate a LIB file from a bunch of OBJ files.
##
########################################################################

def CompileLIB(lib=0, obj=[], opts=[]):
    if (lib==0): sys.exit("syntax error in CompileLIB directive")

    if (COMPILER=="MSVC7" or COMPILER=="MSVC71"):
        if (lib[-4:]==".ilb"): wlib = PREFIX+"/tmp/" + lib[:-4] + ".lib"
        else:                  wlib = PREFIX+"/lib/" + lib[:-4] + ".lib"
        wobj = xpaths(PREFIX+"/tmp/",obj,"")
        ALLTARGETS.append(wlib)
        if (older(wlib, wobj)):
            cmd = 'lib.exe /nologo /OUT:' + wlib
            if (OPTIMIZE==4): cmd = cmd + " /LTCG "
            for x in wobj: cmd = cmd + ' ' + x
            oscmd(cmd)
            updatefiledate(wlib)
    elif (COMPILER=="LINUXA"):
        if (lib[-4:]==".ilb"): wlib = PREFIX+"/tmp/" + lib[:-4] + ".a"
        else:                  wlib = PREFIX+"/lib/" + lib[:-4] + ".a"
        wobj = []
        for x in obj: wobj.append(PREFIX + "/tmp/" + x[:-4] + ".o")
        if (older(wlib, wobj)):
            cmd = 'ar cru ' + wlib
            for x in wobj: cmd=cmd + ' ' + x
            oscmd(cmd)
            updatefiledate(wlib)

########################################################################
##
## CompileLink
##
## Generate a DLL or EXE file from a bunch of OBJ and LIB files.
##
########################################################################

def CompileLink(dll=0, obj=[], opts=[], xdep=[]):
    if (dll==0): sys.exit("Syntax error in CompileLink directive")

    if (COMPILER=="MSVC7" or COMPILER=="MSVC71"):
        lib = PREFIX+"/lib/"+dll[:-4]+".lib"
        if ((dll[-4:] != ".exe") and (dll[-4:] != ".dll")):
            dll = PREFIX+"/plugins/"+dll
        else:
            dll = PREFIX+"/bin/"+dll
        ALLTARGETS.append(dll)
        wobj = []
        for x in obj:
            suffix = x[-4:]
            if   (suffix==".obj"): wobj.append(PREFIX+"/tmp/"+x)
            elif (suffix==".dll"): wobj.append(PREFIX+"/lib/"+x[:-4]+".lib")
            elif (suffix==".lib"): wobj.append(PREFIX+"/lib/"+x)
            elif (suffix==".ilb"): wobj.append(PREFIX+"/tmp/"+x[:-4]+".lib")
            elif (suffix==".res"): wobj.append(PREFIX+"/tmp/"+x)
            else: sys.exit("unknown suffix in object list.")
        if (older(dll, wobj+xdep)):
            cmd = 'link.exe /nologo /NODEFAULTLIB:LIBCI.LIB'
            if (dll[-4:]!=".exe"): cmd = cmd + " /DLL"
            if (OPTIMIZE==1): cmd = cmd + " /DEBUG /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF "
            if (OPTIMIZE==2): cmd = cmd + " /DEBUG /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF "
            if (OPTIMIZE==3): cmd = cmd + " /DEBUG /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF "
            if (OPTIMIZE==4): cmd = cmd + " /DEBUG /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF /LTCG "
            cmd = cmd + " /MAP /MAPINFO:EXPORTS /MAPINFO:LINES /fixed:no /incremental:no /stack:4194304 "
            if (opts.count("NOLIBCI")): cmd = cmd + " /NODEFAULTLIB:LIBCI.LIB "
            if (opts.count("MAXEGGDEF")): cmd = cmd + ' /DEF:pandatool/src/maxegg/MaxEgg.def'
            cmd = cmd + ' /OUT:' + dll + ' /IMPLIB:' + lib + ' /MAP:NUL'
            cmd = cmd + ' /LIBPATH:' + PREFIX + '/python/libs '
            for x in wobj: cmd = cmd + ' ' + x
            if (dll[-4:]==".exe"): cmd = cmd + ' ' + PREFIX + '/tmp/pandaIcon.res'
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
            if (PkgSelected(opts,"ZLIB")):     cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/zlib/lib/libz.lib'
            if (PkgSelected(opts,"PNG")):      cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/png/lib/libpng.lib'
            if (PkgSelected(opts,"JPEG")):     cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/jpeg/lib/libjpeg.lib'
            if (PkgSelected(opts,"TIFF")):     cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/tiff/lib/libtiff.lib'
            if (PkgSelected(opts,"VRPN")):
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/vrpn/lib/vrpn.lib'
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/vrpn/lib/quat.lib'
            if (PkgSelected(opts,"FMOD")):
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/fmod/lib/fmod.lib'
            if (PkgSelected(opts,"MILES")):
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/miles/lib/mss32.lib'
            if (PkgSelected(opts,"NVIDIACG")):
                if (opts.count("CGGL")):
                    cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/nvidiacg/lib/cgGL.lib'
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/nvidiacg/lib/cg.lib'
            if (PkgSelected(opts,"HELIX")):
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/helix/lib/runtlib.lib'
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/helix/lib/syslib.lib'
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/helix/lib/contlib.lib'
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/helix/lib/debuglib.lib'
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/helix/lib/utillib.lib'
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/helix/lib/stlport_vc7.lib'
            if (PkgSelected(opts,"NSPR")):
                cmd = cmd + ' ' + SDK_LIB_PATH['nspr'] + '/nspr4.lib'
            if (PkgSelected(opts,"SSL")):
                cmd = cmd + ' ' + SDK_LIB_PATH['ssl'] + '/ssleay32.lib'
                cmd = cmd + ' ' + SDK_LIB_PATH['ssl'] + '/libeay32.lib'
            if (PkgSelected(opts,"FREETYPE")):
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/freetype/lib/libfreetype.lib'
            if (PkgSelected(opts,"FFTW")):
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/fftw/lib/rfftw.lib'
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/fftw/lib/fftw.lib'
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
            updatefiledate(dll)
            if ((OPTIMIZE == 1) and (dll[-4:]==".dll")):
                CopyFile(dll[:-4]+"_d.dll", dll)
    elif (COMPILER=="LINUXA"):
        ALLTARGETS.append(PREFIX+"/lib/"+dll[:-4]+".so")
        if (dll[-4:]==".exe"): wdll = PREFIX+"/bin/"+dll[:-4]
        else: wdll = PREFIX+"/lib/"+dll[:-4]+".so"
        wobj = []
        for x in obj:
            suffix = x[-4:]
            if   (suffix==".obj"): wobj.append(PREFIX+"/tmp/"+x[:-4]+".o")
            elif (suffix==".dll"): wobj.append(PREFIX+"/lib/"+x[:-4]+".so")
            elif (suffix==".lib"): wobj.append(PREFIX+"/lib/"+x[:-4]+".a")
            elif (suffix==".ilb"): wobj.append(PREFIX+"/tmp/"+x[:-4]+".a")
            else: sys.exit("unknown suffix in object list.")
        if (older(wdll, wobj+xdep)):
            if (dll[-4:]==".exe"): cmd = 'g++ -o ' + wdll + ' -L' + PREFIX + '/lib'
            else:                  cmd = 'g++ -shared -o ' + wdll + ' -L' + PREFIX + '/lib'
            for x in obj:
                suffix = x[-4:]
                if   (suffix==".obj"): cmd = cmd + ' ' + PREFIX + '/tmp/' + x[:-4] + '.o'
                elif (suffix==".dll"): cmd = cmd + ' -l' + x[3:-4]
                elif (suffix==".lib"): cmd = cmd + ' ' + PREFIX + '/lib/' + x[:-4] + '.a'
                elif (suffix==".ilb"): cmd = cmd + ' ' + PREFIX + '/tmp/' + x[:-4] + '.a'
            if (PkgSelected(opts,"FMOD")):     cmd = cmd + ' -L' + THIRDPARTY + '/linux-libs-a/fmod/lib -lfmod-3.74'
            if (PkgSelected(opts,"NVIDIACG")):
                cmd = cmd + ' -L' + THIRDPARTY + 'nvidiacg/lib '
                if (opts.count("CGGL")): cmd = cmd + " -lCgGL"
                cmd = cmd + " -lCg"
            if (PkgSelected(opts,"NSPR")):     cmd = cmd + ' -L' + NSPR_SDK + '/lib -lpandanspr4'
            if (PkgSelected(opts,"ZLIB")):     cmd = cmd + " -lz"
            if (PkgSelected(opts,"PNG")):      cmd = cmd + " -lpng"
            if (PkgSelected(opts,"JPEG")):     cmd = cmd + " -ljpeg"
            if (PkgSelected(opts,"TIFF")):     cmd = cmd + " -ltiff"
            if (PkgSelected(opts,"SSL")):      cmd = cmd + " -lssl"
            if (PkgSelected(opts,"FREETYPE")): cmd = cmd + " -lfreetype"
            if (PkgSelected(opts,"VRPN")):     cmd = cmd + ' -L' + THIRDPARTY + '/linux-libs-a/vrpn/lib -lvrpn -lquat'
            if (PkgSelected(opts,"FFTW")):     cmd = cmd + ' -L' + THIRDPARTY + '/linux-libs-a/fftw/lib -lrfftw -lfftw'
            if (opts.count("GLUT")):           cmd = cmd + " -lGL -lGLU"
            oscmd(cmd)
            updatefiledate(wdll)

##########################################################################################
#
# CompileBAM
#
# Generate a BAM file from an EGG or FLT
#
##########################################################################################

def CompileBAM(preconv, bam, egg):
    dotexe = ".exe"
    if (sys.platform != "win32" or sys.platform == "cygwin"): dotexe = ""
    if (older(bam, egg)):
        if (egg[-4:]==".flt"):
            oscmd(PREFIX + "/bin/flt2egg" + dotexe + " -pr " + preconv + " -o " + PREFIX + "/tmp/tmp.egg" + " " + egg)
            oscmd(PREFIX + "/bin/egg2bam" + dotexe + " -o " + bam + " " + PREFIX + "/tmp/tmp.egg")
        else:
            oscmd(PREFIX + "/bin/egg2bam" + dotexe + " -pr " + preconv + " -o " + bam + " " + egg)

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
CxxIgnoreHeader["Max.h"] = 1
CxxIgnoreHeader["iparamb2.h"] = 1
CxxIgnoreHeader["iparamm2.h"] = 1
CxxIgnoreHeader["istdplug.h"] = 1
CxxIgnoreHeader["iskin.h"] = 1
CxxIgnoreHeader["stdmat.h"] = 1
CxxIgnoreHeader["phyexp.h"] = 1
CxxIgnoreHeader["bipexp.h"] = 1
CxxIgnoreHeader["windows.h"] = 1
CxxIgnoreHeader["windef.h"] = 1
CxxIgnoreHeader["modstack.h"] = 1
CxxIgnoreHeader["afxres.h"] = 1

##########################################################################################
#
# Generate pandaVersion.h
#
##########################################################################################

VERSION1=int(VERSION.split(".")[0])
VERSION2=int(VERSION.split(".")[1])
VERSION3=int(VERSION.split(".")[2])
NVERSION=VERSION1*1000000+VERSION2*1000+VERSION3

conf="""
#define PANDA_MAJOR_VERSION VERSION1
#define PANDA_MINOR_VERSION VERSION2
#define PANDA_SEQUENCE_VERSION VERSION2
#undef  PANDA_OFFICIAL_VERSION
#define PANDA_VERSION NVERSION
#define PANDA_VERSION_STR "VERSION1.VERSION2.VERSION3"
#define PANDA_DISTRIBUTOR "makepanda"
"""

conf = conf.replace("VERSION1",str(VERSION1))
conf = conf.replace("VERSION2",str(VERSION2))
conf = conf.replace("VERSION3",str(VERSION3))
conf = conf.replace("NVERSION",str(NVERSION))

ConditionalWriteFile(PREFIX+'/include/pandaVersion.h',conf)

conf="""
# include "dtoolbase.h"
EXPCL_DTOOL int panda_version_VERSION1_VERSION2_VERSION3 = 0;
"""

conf = conf.replace("VERSION1",str(VERSION1))
conf = conf.replace("VERSION2",str(VERSION2))
conf = conf.replace("VERSION3",str(VERSION3))
conf = conf.replace("NVERSION",str(NVERSION))

ConditionalWriteFile(PREFIX+'/include/checkPandaVersion.cxx',conf)

conf="""
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

conf = conf.replace("VERSION1",str(VERSION1))
conf = conf.replace("VERSION2",str(VERSION2))
conf = conf.replace("VERSION3",str(VERSION3))
conf = conf.replace("NVERSION",str(NVERSION))

ConditionalWriteFile(PREFIX+'/include/checkPandaVersion.h',conf)

ConditionalWriteFile(PREFIX + "/tmp/pythonversion", os.path.basename(PYTHONSDK))

##########################################################################################
#
# If running under windows, compile up the icon.
#
##########################################################################################

if (sys.platform == "win32" or sys.platform == "cygwin"):
  IPATH=["panda/src/configfiles"]
  OPTS=[]
  CompileRES(ipath=IPATH, opts=OPTS, src='pandaIcon.rc', obj='pandaIcon.res')

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
else: sys.exit("Cannot find the 'direct' tree")
"""
ConditionalWriteFile(PREFIX+'/direct/__init__.py', DIRECTINIT)

##########################################################################################
#
# Generate dtool_have_xxx.dat
#
##########################################################################################

for x in PACKAGES:
    if (OMIT.count(x)): ConditionalWriteFile(PREFIX+'/tmp/dtool_have_'+x.lower()+'.dat',"0\n")
    else:               ConditionalWriteFile(PREFIX+'/tmp/dtool_have_'+x.lower()+'.dat',"1\n")

##########################################################################################
#
# Generate dtool_config.h
#
##########################################################################################

conf = "/* dtool_config.h.  Generated automatically by makepanda.py */\n"
for key,win,unix in DTOOLDEFAULTS:
    val = DTOOLCONFIG[key]
    if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
    else:                conf = conf + "#define " + key + " " + val + "\n"
ConditionalWriteFile(PREFIX+'/include/dtool_config.h',conf)

##########################################################################################
#
# Generate the PRC files into the ETC directory.
#
##########################################################################################

CONFAUTOPRC="""
###########################################################
###                                                     ###
### Panda3D Configuration File - Auto-Generated Portion ###
###                                                     ###
### Editing this file is not recommended. Most of these ###
### directives can be overriden in Config.prc           ###
###                                                     ###
###########################################################

# Define the display types that have been compiled in.  Panda will
# pick one of these by going through the list in this order until one
# is found that works, unless the user specifically requests a
# particular display type with the load-display directive.

aux-display pandagl
aux-display pandadx9
aux-display pandadx8
aux-display pandadx7

# The egg loader is handy to have available by default.  This allows
# clients to load egg files.  (The bam loader is built-in so bam files
# are always loadable).

# By qualifying with the extension "egg", we indicate the egg loader
# should be made available only if you explicitly name a file with an
# .egg extension.

load-file-type egg pandaegg

# The following lines define some handy object types to use within the
# egg syntax.  This remaps <ObjectType> { name } into whatever egg
# syntax is given by egg-object-type-name, which makes a handy
# abbreviation for modeling packages (like Maya) to insert
# sophisticated egg syntax into the generated egg file, using a single
# object type string.

egg-object-type-portal          <Scalar> portal { 1 }
egg-object-type-polylight       <Scalar> polylight { 1 }
egg-object-type-seq24           <Switch> { 1 } <Scalar> fps { 24 }
egg-object-type-seq12           <Switch> { 1 } <Scalar> fps { 12 }
egg-object-type-indexed         <Scalar> indexed { 1 }

# These are just shortcuts to define the Model and DCS flags, which
# indicate nodes that should not be flattened out of the hierarchy
# during the conversion process.  DCS goes one step further and
# indicates that the node's transform is important and should be
# preserved (DCS stands for Dynamic Coordinate System).

egg-object-type-model           <Model> { 1 }
egg-object-type-dcs             <DCS> { 1 }

# The following define various kinds of collision geometry.  These
# mark the geometry at this level and below as invisible collision
# polygons, which can be used by Panda's collision system to detect
# collisions more optimally than regular visible polygons.

egg-object-type-barrier         <Collide> { Polyset descend }
egg-object-type-sphere          <Collide> { Sphere descend }
egg-object-type-invsphere       <Collide> { InvSphere descend }
egg-object-type-tube            <Collide> { Tube descend }

# As above, but these are flagged to be "intangible", so that they
# will trigger an event but not stop an object from passing through.

egg-object-type-trigger         <Collide> { Polyset descend intangible }
egg-object-type-trigger-sphere  <Collide> { Sphere descend intangible }

# "bubble" puts an invisible bubble around an object, but does not
# otherwise remove the geometry.

egg-object-type-bubble          <Collide> { Sphere keep descend }

# "ghost" turns off the normal collide bit that is set on visible
# geometry by default, so that if you are using visible geometry for
# collisions, this particular geometry will not be part of those
# collisions--it is ghostlike.

egg-object-type-ghost           <Scalar> collide-mask { 0 }

# This module allows direct loading of formats like .flt, .mb, or .dxf

load-file-type ptloader

# Define a new egg object type.  See the comments in _panda.prc about this.

egg-object-type-direct-widget   <Scalar> collide-mask { 0x80000000 } <Collide> { Polyset descend }

# Define a new cull bin that will render on top of everything else.

cull-bin gui-popup 60 unsorted
"""

CONFIGPRC="""
###########################################################
###                                                     ###
### Panda3D Configuration File -  User-Editable Portion ###
###                                                     ###
###########################################################

# Uncomment one of the following lines to choose whether you should
# run using OpenGL or DirectX rendering.

load-display pandagl

# These control the placement and size of the default rendering window.

win-origin 100 0
win-size 800 600

# Uncomment this line if you want to run Panda fullscreen instead of
# in a window.

fullscreen #f

# If you don't object to running OpenGL in software leave the keyword
# "software" in the following line, otherwise remove it to force
# hardware only.

framebuffer-mode rgba double-buffer depth multisample hardware software

# These control the amount of output Panda gives for some various
# categories.  The severity levels, in order, are "spam", "debug",
# "info", "warning", and "fatal"; the default is "info".  Uncomment
# one (or define a new one for the particular category you wish to
# change) to control this output.

notify-level warning
default-directnotify-level warning

# These specify where model files may be loaded from.  You probably
# want to set this to a sensible path for yourself.  $THIS_PRC_DIR is
# a special variable that indicates the same directory as this
# particular Config.prc file.

model-path    .
model-path    $THIS_PRC_DIR/..
model-path    $THIS_PRC_DIR/../models
sound-path    .
sound-path    $THIS_PRC_DIR/..
sound-path    $THIS_PRC_DIR/../models
texture-path  .
texture-path  $THIS_PRC_DIR/..
texture-path  $THIS_PRC_DIR/../models

# This enable the automatic creation of a TK window when running
# Direct.

want-directtools  #f
want-tk           #f

# This enables simple networked programs to easily provide a DC file

dc-file sample.dc

# Enable audio using the FMod audio library by default:

audio-library-name fmod_audio
"""

if (sys.platform != "win32" or sys.platform == "cygwin"):
    CONFAUTOPRC = CONFAUTOPRC.replace("aux-display pandadx9","")
    CONFAUTOPRC = CONFAUTOPRC.replace("aux-display pandadx8","")
    CONFAUTOPRC = CONFAUTOPRC.replace("aux-display pandadx7","")

ConditionalWriteFile(PREFIX + "/etc/Confauto.prc", CONFAUTOPRC)
ConditionalWriteFile(PREFIX + "/etc/Config.prc", CONFIGPRC)

##########################################################################################
#
# Copy the precompiled binaries and DLLs into the build.
#
##########################################################################################

for pkg in (PACKAGES + ["extras"]):
    if (OMIT.count(pkg)==0):
        if (COMPILER == "MSVC7" or COMPILER=="MSVC71"):
            if (os.path.exists(THIRDPARTY+"/win-libs-vc7/"+pkg.lower()+"/bin")):
                CopyAllFiles(PREFIX+"/bin/",THIRDPARTY+"/win-libs-vc7/"+pkg.lower()+"/bin/")
        elif (COMPILER == "LINUXA"):
            if (os.path.exists(THIRDPARTY+"/linux-libs-a/"+pkg.lower()+"/lib")):
                CopyAllFiles(PREFIX+"/lib/",THIRDPARTY+"/linux-libs-a/"+pkg.lower()+"/lib/")

if sys.platform == "win32" or sys.platform == "cygwin":
    CopyTree(PREFIX+'/python', PYTHONSDK)
    if os.path.isfile(PYTHONSDK+'/python22.dll'):
        CopyFile(PREFIX+'/bin/',   PYTHONSDK+'/python22.dll')

########################################################################
##
## Compile the 'ppython' executable and 'genpycode' executables
##
########################################################################

IPATH=['direct/src/directbase']
CompileC(ipath=IPATH, opts=['BUILDING_PPYTHON'], src='ppython.cxx', obj='ppython.obj')
CompileLink(opts=['WINUSER'], dll='ppython.exe', obj=['ppython.obj'])

IPATH=['direct/src/directbase']
CompileC(ipath=IPATH, opts=['BUILDING_GENPYCODE'], src='ppython.cxx', obj='genpycode.obj')
CompileLink(opts=['WINUSER'], dll='genpycode.exe', obj=['genpycode.obj'])

########################################################################
#
# Copy header files to the PREFIX/include directory.
#
# Are we just copying *ALL* headers into the include directory?
# If so, let's automate this.
#
# We're definitely not copying all headers.  We're only copying those
# same headers that are copied by ppremake.  But the bigger question
# is, did he *intend* to copy all headers?  Another good question is,
# could we just put a little tag into the header file itself, indicating
# that it is meant to be copied?
#
########################################################################

ConditionalWriteFile(PREFIX+'/include/ctl3d.h', '/* dummy file to make MAX happy */')

class FileList:
    allFiles=['*']
    includes=['/*_src.cxx', '/*.h', '/*.I', '/*.T']

    def __init__(self):
        self.files={}

    def add(self, srcPattern="*"):
        global VERBOSE
        if VERBOSE:
            print "FileList.add(\"%s\")"%(srcPattern,)
        for i in glob(srcPattern):
            self.files[i.replace("\\", "/")]=None

    def addIncludes(self, srcPattern="."):
        for i in self.includes:
            self.add(srcPattern+i)

    def omit(self, srcPattern="*"):
        global VERBOSE
        if VERBOSE:
            print "FileList.omit(\"%s\")"%(srcPattern,)
        for i in glob(srcPattern):
            try: del self.files[i.replace("\\", "/")]
            except: pass

    def omitIncludes(self, srcPattern="."):
        for i in self.includes:
            self.omit(srcPattern+i)

    def copyTo(self, dstDir="."):
        global VERBOSE
        if VERBOSE:
            print "FileList.copyTo(\"%s\")"%(dstDir,)
        for i in self.files.keys():
            if os.path.isfile(i):
                CopyFile(dstDir+'/'+os.path.split(i)[1], i)

fileList=FileList()
fileList.addIncludes('dtool/src/*')
fileList.addIncludes('panda/src/*')
fileList.addIncludes('pandatool/src/*')
fileList.addIncludes('pandaapp/src/*')
fileList.addIncludes('direct/src/*')
fileList.omitIncludes('dtool/src/parser-inc')
fileList.omitIncludes('dtool/src/parser-inc')
fileList.copyTo(PREFIX+'/include')
del fileList

fileList=FileList()
fileList.add('dtool/src/parser-inc/*')
fileList.copyTo(PREFIX+'/include/parser-inc')
fileList.copyTo(PREFIX+'/include/parser-inc/openssl')
del fileList

fileList=FileList()
fileList.add('dtool/src/parser-inc/cg.h')
fileList.add('dtool/src/parser-inc/cgGL.h')
fileList.copyTo(PREFIX+'/include/parser-inc/Cg')
del fileList


########################################################################
#
# This file contains a list of all the files that need to be compiled.
#
########################################################################

#
# DIRECTORY: dtool/src/dtoolbase/
#

IPATH=['dtool/src/dtoolbase']
OPTS=['BUILDING_DTOOL', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='dtoolbase.cxx', obj='dtoolbase_dtoolbase.obj')

#
# DIRECTORY: dtool/src/dtoolutil/
#

IPATH=['dtool/src/dtoolutil']
OPTS=['BUILDING_DTOOL', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='gnu_getopt.c',             obj='dtoolutil_gnu_getopt.obj')
CompileC(ipath=IPATH, opts=OPTS, src='gnu_getopt1.c',            obj='dtoolutil_gnu_getopt1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='dtoolutil_composite1.cxx', obj='dtoolutil_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='dtoolutil_composite2.cxx', obj='dtoolutil_composite2.obj')

#
# DIRECTORY: dtool/metalibs/dtool/
#

IPATH=['dtool/metalibs/dtool']
OPTS=['BUILDING_DTOOL', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='dtool.cxx', obj='dtool_dtool.obj')
CompileLink(opts=['ADVAPI', 'NSPR'], dll='libdtool.dll', obj=[
             'dtool_dtool.obj',
             'dtoolutil_gnu_getopt.obj',
             'dtoolutil_gnu_getopt1.obj',
             'dtoolutil_composite1.obj',
             'dtoolutil_composite2.obj',
             'dtoolbase_dtoolbase.obj',
])

#
# DIRECTORY: dtool/src/cppparser/
#

IPATH=['dtool/src/cppparser']
CompileBison(pre='cppyy', dstc='cppBison.cxx', dsth='cppBison.h', src='dtool/src/cppparser/cppBison.yxx')
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='cppParser_composite1.cxx', obj='cppParser_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='cppParser_composite2.cxx', obj='cppParser_composite2.obj')
CompileC(ipath=IPATH, opts=OPTS, src='cppBison.cxx', obj='cppParser_cppBison.obj')
CompileLIB(lib='libcppParser.ilb', obj=[
             'cppParser_composite1.obj',
             'cppParser_composite2.obj',
             'cppParser_cppBison.obj',
])

#
# DIRECTORY: dtool/src/prc/
#

IPATH=['dtool/src/prc']
OPTS=['BUILDING_DTOOLCONFIG', 'SSL', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='prc_composite1.cxx', obj='prc_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='prc_composite2.cxx', obj='prc_composite2.obj')

#
# DIRECTORY: dtool/src/dconfig/
#

IPATH=['dtool/src/dconfig']
OPTS=['BUILDING_DTOOLCONFIG', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='dconfig_composite1.cxx', obj='dconfig_composite1.obj')

#
# DIRECTORY: dtool/src/interrogatedb/
#

IPATH=['dtool/src/interrogatedb']
OPTS=['BUILDING_DTOOLCONFIG', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='interrogatedb_composite1.cxx', obj='interrogatedb_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='interrogatedb_composite2.cxx', obj='interrogatedb_composite2.obj')

#
# DIRECTORY: dtool/metalibs/dtoolconfig/
#

IPATH=['dtool/metalibs/dtoolconfig']
OPTS=['BUILDING_DTOOLCONFIG', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='dtoolconfig.cxx', obj='dtoolconfig_dtoolconfig.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pydtool.cxx', obj='dtoolconfig_pydtool.obj')
CompileLink(opts=['ADVAPI', 'NSPR', 'SSL'], dll='libdtoolconfig.dll', obj=[
             'dtoolconfig_dtoolconfig.obj',
             'dtoolconfig_pydtool.obj',
             'interrogatedb_composite1.obj',
             'interrogatedb_composite2.obj',
             'dconfig_composite1.obj',
             'prc_composite1.obj',
             'prc_composite2.obj',
             'libdtool.dll',
])

#
# DIRECTORY: dtool/src/pystub/
#

IPATH=['dtool/src/pystub']
OPTS=['BUILDING_DTOOLCONFIG', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='pystub.cxx', obj='pystub_pystub.obj')
CompileLink(opts=['ADVAPI', 'NSPR'], dll='libpystub.dll', obj=[
             'pystub_pystub.obj',
             'libdtool.dll',
])

#
# DIRECTORY: dtool/src/interrogate/
#

IPATH=['dtool/src/interrogate', 'dtool/src/cppparser', 'dtool/src/interrogatedb']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='interrogate_composite1.cxx', obj='interrogate_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='interrogate_composite2.cxx', obj='interrogate_composite2.obj')
CompileLink(opts=['ADVAPI', 'NSPR', 'SSL'], dll='interrogate.exe', obj=[
             'interrogate_composite1.obj',
             'interrogate_composite2.obj',
             'libcppParser.ilb',
             'libpystub.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

CompileC(ipath=IPATH, opts=OPTS, src='interrogate_module.cxx', obj='interrogate_module_interrogate_module.obj')
CompileLink(opts=['ADVAPI', 'NSPR', 'SSL'], dll='interrogate_module.exe', obj=[
             'interrogate_module_interrogate_module.obj',
             'libcppParser.ilb',
             'libpystub.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

CompileC(ipath=IPATH, opts=OPTS, src='parse_file.cxx', obj='parse_file_parse_file.obj')
CompileLink(opts=['ADVAPI', 'NSPR', 'SSL'], dll='parse_file.exe', obj=[
             'parse_file_parse_file.obj',
             'libcppParser.ilb',
             'libpystub.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

#
# DIRECTORY: dtool/src/prckeys/
#

if (OMIT.count("SSL")==0):
  IPATH=['dtool/src/prckeys']
  OPTS=['SSL', 'NSPR']
  CompileC(ipath=IPATH, opts=OPTS, src='makePrcKey.cxx', obj='make-prc-key_makePrcKey.obj')
  CompileLink(opts=['ADVAPI', 'NSPR', 'SSL'], dll='make-prc-key.exe', obj=[
               'make-prc-key_makePrcKey.obj',
               'libpystub.dll',
               'libdtool.dll',
               'libdtoolconfig.dll',
               ])

#
# DIRECTORY: dtool/src/test_interrogate/
#

IPATH=['dtool/src/test_interrogate']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='test_interrogate.cxx', obj='test_interrogate_test_interrogate.obj')
CompileLink(opts=['ADVAPI', 'NSPR', 'SSL'], dll='test_interrogate.exe', obj=[
             'test_interrogate_test_interrogate.obj',
             'libpystub.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

#
# DIRECTORY: panda/src/pandabase/
#

IPATH=['panda/src/pandabase']
OPTS=['BUILDING_PANDAEXPRESS', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='pandabase.cxx', obj='pandabase_pandabase.obj')

#
# DIRECTORY: panda/src/express/
#

IPATH=['panda/src/express']
OPTS=['BUILDING_PANDAEXPRESS', 'SSL', 'ZLIB', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='express_composite1.cxx', obj='express_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='express_composite2.cxx', obj='express_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libexpress.in', outc='libexpress_igate.cxx',
            src='panda/src/express',  module='pandaexpress', library='libexpress',
            files=['atomicAdjustDummyImpl.h', 'atomicAdjust.h', 'atomicAdjustImpl.h', 'atomicAdjustNsprImpl.h', 'bigEndian.h', 'buffer.h', 'checksumHashGenerator.h', 'circBuffer.h', 'clockObject.h', 'conditionVarDummyImpl.h', 'conditionVar.h', 'conditionVarImpl.h', 'conditionVarNsprImpl.h', 'config_express.h', 'datagram.h', 'datagramGenerator.h', 'datagramIterator.h', 'datagramSink.h', 'dcast.h', 'encryptStreamBuf.h', 'encryptStream.h', 'error_utils.h', 'hashGeneratorBase.h', 'hashVal.h', 'indent.h', 'indirectLess.h', 'littleEndian.h', 'memoryInfo.h', 'memoryUsage.h', 'memoryUsagePointerCounts.h', 'memoryUsagePointers.h', 'multifile.h', 'mutexDummyImpl.h', 'pmutex.h', 'mutexHolder.h', 'mutexImpl.h', 'mutexNsprImpl.h', 'namable.h', 'nativeNumericData.h', 'numeric_types.h', 'ordered_vector.h', 'password_hash.h', 'patchfile.h', 'pointerTo.h', 'pointerToArray.h', 'pointerToBase.h', 'pointerToVoid.h', 'profileTimer.h', 'pta_uchar.h', 'ramfile.h', 'referenceCount.h', 'register_type.h', 'reversedNumericData.h', 'selectThreadImpl.h', 'streamReader.h', 'streamWriter.h', 'stringDecoder.h', 'subStream.h', 'subStreamBuf.h', 'textEncoder.h', 'threadDummyImpl.h', 'thread.h', 'threadImpl.h', 'threadNsprImpl.h', 'threadPriority.h', 'tokenBoard.h', 'trueClock.h', 'typeHandle.h', 'typedObject.h', 'typedReferenceCount.h', 'typedef.h', 'typeRegistry.h', 'typeRegistryNode.h', 'unicodeLatinMap.h', 'vector_uchar.h', 'virtualFileComposite.h', 'virtualFile.h', 'virtualFileList.h', 'virtualFileMount.h', 'virtualFileMountMultifile.h', 'virtualFileMountSystem.h', 'virtualFileSimple.h', 'virtualFileSystem.h', 'weakPointerTo.h', 'weakPointerToBase.h', 'weakPointerToVoid.h', 'weakReferenceList.h', 'windowsRegistry.h', 'zStream.h', 'zStreamBuf.h', 'express_composite1.cxx', 'express_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libexpress_igate.cxx', obj='libexpress_igate.obj')

#
# DIRECTORY: panda/src/downloader/
#

IPATH=['panda/src/downloader']
OPTS=['BUILDING_PANDAEXPRESS', 'SSL', 'ZLIB', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='downloader_composite1.cxx', obj='downloader_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='downloader_composite2.cxx', obj='downloader_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libdownloader.in', outc='libdownloader_igate.cxx',
            src='panda/src/downloader',  module='pandaexpress', library='libdownloader', files=[
            'config_downloader.h', 'asyncUtility.h', 'bioPtr.h', 'bioStreamPtr.h', 'bioStream.h', 'bioStreamBuf.h',
            'chunkedStream.h', 'chunkedStreamBuf.h', 'decompressor.h', 'documentSpec.h', 'downloadDb.h',
            'download_utils.h', 'extractor.h', 'httpAuthorization.h', 'httpBasicAuthorization.h', 'httpChannel.h',
            'httpClient.h', 'httpCookie.h', 'httpDate.h', 'httpDigestAuthorization.h', 'httpEntityTag.h',
            'httpEnum.h', 'identityStream.h', 'identityStreamBuf.h', 'multiplexStream.h', 'multiplexStreamBuf.h',
            'patcher.h', 'socketStream.h', 'ssl_utils.h', 'urlSpec.h',
            'downloader_composite1.cxx', 'downloader_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libdownloader_igate.cxx', obj='libdownloader_igate.obj')

#
# DIRECTORY: panda/metalibs/pandaexpress/
#

IPATH=['panda/metalibs/pandaexpress']
OPTS=['BUILDING_PANDAEXPRESS', 'ZLIB', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='pandaexpress.cxx', obj='pandaexpress_pandaexpress.obj')
InterrogateModule(outc='libpandaexpress_module.cxx', module='pandaexpress', library='libpandaexpress',
                  files=['libdownloader.in', 'libexpress.in'])
CompileC(ipath=IPATH, opts=OPTS, src='libpandaexpress_module.cxx', obj='libpandaexpress_module.obj')
CompileLink(opts=['ADVAPI', 'WINSOCK2', 'NSPR', 'SSL', 'ZLIB'], dll='libpandaexpress.dll', obj=[
             'pandaexpress_pandaexpress.obj',
             'libpandaexpress_module.obj',
             'downloader_composite1.obj',
             'downloader_composite2.obj',
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
CompileC(ipath=IPATH, opts=OPTS, src='putil_composite1.cxx', obj='putil_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='putil_composite2.cxx', obj='putil_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libputil.in', outc='libputil_igate.cxx',
            src='panda/src/putil',  module='panda', library='libputil', files=[
            'bam.h', 'bamReader.h', 'bamReaderParam.h', 'bamWriter.h', 'bitMask.h', 'buttonHandle.h',
            'buttonRegistry.h', 'cachedTypedWritableReferenceCount.h', 'collideMask.h', 'portalMask.h',
            'compareTo.h', 'config_util.h', 'configurable.h', 'cycleData.h', 'cycleDataReader.h',
            'cycleDataWriter.h', 'datagramInputFile.h', 'datagramOutputFile.h', 'drawMask.h', 'factoryBase.h',
            'factoryParam.h', 'factoryParams.h', 'firstOfPairCompare.h', 'firstOfPairLess.h',
            'globalPointerRegistry.h', 'indirectCompareNames.h', 'indirectCompareTo.h', 'ioPtaDatagramFloat.h',
            'ioPtaDatagramInt.h', 'ioPtaDatagramShort.h', 'keyboardButton.h', 'lineStream.h', 'lineStreamBuf.h',
            'load_prc_file.h', 'modifierButtons.h', 'mouseButton.h', 'mouseData.h', 'nameUniquifier.h',
            'pipeline.h', 'pipelineCycler.h', 'pipelineCyclerBase.h', 'pta_double.h', 'pta_float.h',
            'pta_int.h', 'string_utils.h', 'timedCycle.h', 'typedWritable.h', 'typedWritableReferenceCount.h',
            'updateSeq.h', 'uniqueIdAllocator.h', 'vector_double.h', 'vector_float.h', 'vector_typedWritable.h',
            'vector_ushort.h', 'vector_writable.h', 'writableConfigurable.h', 'writableParam.h',
            'putil_composite1.cxx', 'putil_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libputil_igate.cxx', obj='libputil_igate.obj')

#
# DIRECTORY: panda/src/audio/
#

IPATH=['panda/src/audio']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='audio_composite1.cxx', obj='audio_composite1.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libaudio.in', outc='libaudio_igate.cxx',
            src='panda/src/audio',  module='panda', library='libaudio',
            files=['audio.h'])
CompileC(ipath=IPATH, opts=OPTS, src='libaudio_igate.cxx', obj='libaudio_igate.obj')

#
# DIRECTORY: panda/src/event/
#

IPATH=['panda/src/event']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='event_composite1.cxx', obj='event_composite1.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libevent.in', outc='libevent_igate.cxx',
            src='panda/src/event',  module='panda', library='libevent', files=[
            'config_event.h', 'buttonEvent.h', 'buttonEventList.h', 'event.h', 'eventHandler.h',
            'eventParameter.h', 'eventQueue.h', 'eventReceiver.h', 'pt_Event.h', 'throw_event.h', 'event_composite1.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libevent_igate.cxx', obj='libevent_igate.obj')

#
# DIRECTORY: panda/src/linmath/
#

IPATH=['panda/src/linmath']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='linmath_composite1.cxx', obj='linmath_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='linmath_composite2.cxx', obj='linmath_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='liblinmath.in', outc='liblinmath_igate.cxx',
            src='panda/src/linmath',  module='panda', library='liblinmath', files=[
            'compose_matrix.h', 'compose_matrix_src.h', 'config_linmath.h', 'coordinateSystem.h', 'dbl2fltnames.h', 'dblnames.h', 'deg_2_rad.h', 'flt2dblnames.h', 'fltnames.h', 'ioPtaDatagramLinMath.h', 'lcast_to.h', 'lcast_to_src.h', 'lmatrix.h', 'lmatrix3.h', 'lmatrix3_src.h', 'lmatrix4.h', 'lmatrix4_src.h', 'lorientation.h', 'lorientation_src.h', 'lpoint2.h', 'lpoint2_src.h', 'lpoint3.h', 'lpoint3_src.h', 'lpoint4.h', 'lpoint4_src.h', 'lquaternion.h', 'lquaternion_src.h', 'lrotation.h', 'lrotation_src.h', 'luse.h', 'lvec2_ops.h', 'lvec2_ops_src.h', 'lvec3_ops.h', 'lvec3_ops_src.h', 'lvec4_ops.h', 'lvec4_ops_src.h', 'lvecBase2.h', 'lvecBase2_src.h', 'lvecBase3.h', 'lvecBase3_src.h', 'lvecBase4.h', 'lvecBase4_src.h', 'lvector2.h', 'lvector2_src.h', 'lvector3.h', 'lvector3_src.h', 'lvector4.h', 'lvector4_src.h', 'mathNumbers.h', 'pta_Colorf.h', 'pta_Normalf.h', 'pta_TexCoordf.h', 'pta_Vertexf.h', 'vector_Colorf.h', 'vector_LPoint2f.h', 'vector_LVecBase3f.h', 'vector_Normalf.h', 'vector_TexCoordf.h', 'vector_Vertexf.h', 'linmath_composite1.cxx', 'linmath_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='liblinmath_igate.cxx', obj='liblinmath_igate.obj')

#
# DIRECTORY: panda/src/mathutil/
#

IPATH=['panda/src/mathutil']
OPTS=['BUILDING_PANDA', 'FFTW', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='mathutil_composite1.cxx', obj='mathutil_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='mathutil_composite2.cxx', obj='mathutil_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libmathutil.in', outc='libmathutil_igate.cxx',
            src='panda/src/mathutil',  module='panda', library='libmathutil', files=[
            'boundingHexahedron.h', 'boundingLine.h', 'boundingSphere.h', 'boundingVolume.h', 'config_mathutil.h', 'fftCompressor.h', 'finiteBoundingVolume.h', 'frustum.h', 'frustum_src.h', 'geometricBoundingVolume.h', 'linmath_events.h', 'look_at.h', 'look_at_src.h', 'omniBoundingVolume.h', 'plane.h', 'plane_src.h', 'rotate_to.h', 'mathutil_composite1.cxx', 'mathutil_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libmathutil_igate.cxx', obj='libmathutil_igate.obj')

#
# DIRECTORY: panda/src/gsgbase/
#

IPATH=['panda/src/gsgbase']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='gsgbase_composite1.cxx', obj='gsgbase_composite1.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libgsgbase.in', outc='libgsgbase_igate.cxx',
            src='panda/src/gsgbase',  module='panda', library='libgsgbase', files=[
            'config_gsgbase.h', 'graphicsStateGuardianBase.h', 'gsgbase_composite1.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libgsgbase_igate.cxx', obj='libgsgbase_igate.obj')

#
# DIRECTORY: panda/src/pnmimage/
#

IPATH=['panda/src/pnmimage']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='pnmimage_composite1.cxx', obj='pnmimage_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pnmimage_composite2.cxx', obj='pnmimage_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libpnmimage.in', outc='libpnmimage_igate.cxx',
            src='panda/src/pnmimage',  module='panda', library='libpnmimage', files=[
            'config_pnmimage.h', 'pnmbitio.h', 'pnmFileType.h', 'pnmFileTypeRegistry.h', 'pnmImage.h', 'pnmImageHeader.h', 'pnmReader.h', 'pnmWriter.h', 'pnmimage_base.h', 'ppmcmap.h', 'pnmimage_composite1.cxx', 'pnmimage_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libpnmimage_igate.cxx', obj='libpnmimage_igate.obj')

#
# DIRECTORY: panda/src/net/
#

if (OMIT.count("NSPR")==0):
    IPATH=['panda/src/net']
    OPTS=['BUILDING_PANDA', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='net_composite1.cxx', obj='net_composite1.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='net_composite2.cxx', obj='net_composite2.obj')
    Interrogate(ipath=IPATH, opts=OPTS, outd='libnet.in', outc='libnet_igate.cxx',
                src='panda/src/net',  module='panda', library='libnet', files=[
                'config_net.h', 'connection.h', 'connectionListener.h', 'connectionManager.h',
                'connectionReader.h', 'connectionWriter.h', 'datagramQueue.h', 'datagramTCPHeader.h',
                'datagramUDPHeader.h', 'netAddress.h', 'netDatagram.h', 'pprerror.h', 'queuedConnectionListener.h',
                'queuedConnectionManager.h', 'queuedConnectionReader.h', 'recentConnectionReader.h',
                'queuedReturn.h', 'net_composite1.cxx', 'net_composite2.cxx'])
    CompileC(ipath=IPATH, opts=OPTS, src='libnet_igate.cxx', obj='libnet_igate.obj')

#
# DIRECTORY: panda/src/pstatclient/
#

IPATH=['panda/src/pstatclient']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='pstatclient_composite1.cxx', obj='pstatclient_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pstatclient_composite2.cxx', obj='pstatclient_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libpstatclient.in', outc='libpstatclient_igate.cxx',
            src='panda/src/pstatclient',  module='panda', library='libpstatclient', files=[
            'config_pstats.h', 'pStatClient.h', 'pStatClientImpl.h', 'pStatClientVersion.h', 'pStatClientControlMessage.h', 'pStatCollector.h', 'pStatCollectorDef.h', 'pStatFrameData.h', 'pStatProperties.h', 'pStatServerControlMessage.h', 'pStatThread.h', 'pStatTimer.h', 'pstatclient_composite1.cxx', 'pstatclient_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libpstatclient_igate.cxx', obj='libpstatclient_igate.obj')

#
# DIRECTORY: panda/src/gobj/
#

IPATH=['panda/src/gobj']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='gobj_composite1.cxx', obj='gobj_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='gobj_composite2.cxx', obj='gobj_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libgobj.in', outc='libgobj_igate.cxx',
            src='panda/src/gobj',  module='panda', library='libgobj', files=[
            'boundedObject.h', 'config_gobj.h', 'drawable.h', 'geom.h',
            'geomContext.h', 'geomLine.h', 'geomLinestrip.h', 'geomPoint.h',
            'geomPolygon.h', 'geomQuad.h', 'geomSphere.h', 'geomSprite.h',
            'geomTri.h', 'geomTrifan.h', 'geomTristrip.h', 'imageBuffer.h',
            'material.h', 'materialPool.h', 'matrixLens.h', 'orthographicLens.h',
            'perspectiveLens.h', 'pixelBuffer.h', 'preparedGraphicsObjects.h',
            'lens.h', 'savedContext.h', 'texture.h', 'textureContext.h',
            'texturePool.h', 'texCoordName.h', 'textureStage.h',
            'gobj_composite1.cxx', 'gobj_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libgobj_igate.cxx', obj='libgobj_igate.obj')

#
# DIRECTORY: panda/src/lerp/
#

IPATH=['panda/src/lerp']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='lerp_composite1.cxx', obj='lerp_composite1.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='liblerp.in', outc='liblerp_igate.cxx',
            src='panda/src/lerp',  module='panda', library='liblerp', files=[
            'config_lerp.h', 'lerp.h', 'lerpblend.h', 'lerpfunctor.h', 'lerp_composite1.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='liblerp_igate.cxx', obj='liblerp_igate.obj')

#
# DIRECTORY: panda/src/pgraph/
#

IPATH=['panda/src/pgraph']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='nodePath.cxx', obj='pgraph_nodePath.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pgraph_composite1.cxx', obj='pgraph_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pgraph_composite2.cxx', obj='pgraph_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libpgraph.in', outc='libpgraph_igate.cxx',
            src='panda/src/pgraph',  module='panda', library='libpgraph', files=[
            'accumulatedAttribs.h', 'alphaTestAttrib.h', 'ambientLight.h', 'auxSceneData.h', 'bamFile.h', 'billboardEffect.h', 'binCullHandler.h', 'camera.h', 'clipPlaneAttrib.h', 'colorAttrib.h', 'colorBlendAttrib.h', 'colorScaleAttrib.h', 'colorWriteAttrib.h', 'compassEffect.h', 'config_pgraph.h', 'cullBin.h', 'cullBinAttrib.h', 'cullBinBackToFront.h', 'cullBinFixed.h', 'cullBinFrontToBack.h', 'cullBinManager.h', 'cullBinUnsorted.h', 'cullFaceAttrib.h', 'cullHandler.h', 'cullResult.h', 'cullTraverser.h', 'cullTraverserData.h', 'cullableObject.h', 'decalEffect.h', 'depthOffsetAttrib.h', 'depthTestAttrib.h', 'depthWriteAttrib.h', 'directionalLight.h', 'drawCullHandler.h', 'fadeLodNode.h', 'fadeLodNodeData.h', 'findApproxLevelEntry.h', 'findApproxPath.h', 'fog.h', 'fogAttrib.h', 'geomNode.h', 'geomTransformer.h', 'lensNode.h', 'light.h', 'lightAttrib.h', 'lightLensNode.h', 'lightNode.h', 'loader.h', 'loaderFileType.h', 'loaderFileTypeBam.h', 'loaderFileTypeRegistry.h', 'lodNode.h', 'materialAttrib.h', 'modelNode.h', 'modelPool.h', 'modelRoot.h', 'nodePath.h', 'nodePath.cxx', 'nodePathCollection.h', 'nodePathComponent.h', 'nodePathLerps.h', 'pandaNode.h', 'planeNode.h', 'pointLight.h', 'polylightNode.h', 'polylightEffect.h', 'portalNode.h', 'portalClipper.h', 'renderAttrib.h', 'renderEffect.h', 'renderEffects.h', 'renderModeAttrib.h', 'renderState.h', 'rescaleNormalAttrib.h', 'sceneGraphAnalyzer.h', 'sceneGraphReducer.h', 'sceneSetup.h', 'selectiveChildNode.h', 'sequenceNode.h', 'showBoundsEffect.h', 'spotlight.h', 'switchNode.h', 'texMatrixAttrib.h', 'texProjectorEffect.h', 'textureApplyAttrib.h', 'textureAttrib.h', 'texGenAttrib.h', 'textureCollection.h', 'textureStageCollection.h', 'transformState.h', 'transparencyAttrib.h', 'weakNodePath.h', 'workingNodePath.h', 'pgraph_composite1.cxx', 'pgraph_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libpgraph_igate.cxx', obj='libpgraph_igate.obj')

#
# DIRECTORY: panda/src/chan/
#

IPATH=['panda/src/chan']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='chan_composite1.cxx', obj='chan_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='chan_composite2.cxx', obj='chan_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libchan.in', outc='libchan_igate.cxx',
            src='panda/src/chan',  module='panda', library='libchan', files=[
            'animBundle.h', 'animBundleNode.h', 'animChannel.h', 'animChannelBase.h', 'animChannelMatrixDynamic.h', 'animChannelMatrixXfmTable.h', 'animChannelScalarDynamic.h', 'animChannelScalarTable.h', 'animControl.h', 'animControlCollection.h', 'animGroup.h', 'auto_bind.h', 'config_chan.h', 'movingPartBase.h', 'movingPartMatrix.h', 'movingPartScalar.h', 'partBundle.h', 'partBundleNode.h', 'partGroup.h', 'vector_PartGroupStar.h', 'chan_composite1.cxx', 'chan_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libchan_igate.cxx', obj='libchan_igate.obj')

#
# DIRECTORY: panda/src/char/
#

IPATH=['panda/src/char']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='char_composite1.cxx', obj='char_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='char_composite2.cxx', obj='char_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libchar.in', outc='libchar_igate.cxx',
            src='panda/src/char',  module='panda', library='libchar', files=[
            'character.h', 'characterJoint.h', 'characterJointBundle.h', 'characterSlider.h', 'computedVertices.h', 'computedVerticesMorph.h', 'config_char.h', 'dynamicVertices.h', 'char_composite1.cxx', 'char_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libchar_igate.cxx', obj='libchar_igate.obj')

#
# DIRECTORY: panda/src/dgraph/
#

IPATH=['panda/src/dgraph']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='dgraph_composite1.cxx', obj='dgraph_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='dgraph_composite2.cxx', obj='dgraph_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libdgraph.in', outc='libdgraph_igate.cxx',
            src='panda/src/dgraph',  module='panda', library='libdgraph', files=[
            'config_dgraph.h', 'dataGraphTraverser.h', 'dataNode.h', 'dataNodeTransmit.h', 'dgraph_composite1.cxx', 'dgraph_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libdgraph_igate.cxx', obj='libdgraph_igate.obj')

#
# DIRECTORY: panda/src/display/
#

IPATH=['panda/src/display']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='display_composite1.cxx', obj='display_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='display_composite2.cxx', obj='display_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libdisplay.in', outc='libdisplay_igate.cxx',
            src='panda/src/display',  module='panda', library='libdisplay', files=[
            'config_display.h', 'drawableRegion.h', 'displayRegion.h', 'displayRegionStack.h', 'frameBufferProperties.h', 'frameBufferStack.h', 'graphicsEngine.h', 'graphicsOutput.h', 'graphicsBuffer.h', 'graphicsPipe.h', 'graphicsPipeSelection.h', 'graphicsStateGuardian.h', 'graphicsThreadingModel.h', 'graphicsWindow.h', 'graphicsWindowInputDevice.h', 'graphicsDevice.h', 'parasiteBuffer.h', 'windowProperties.h', 'lensStack.h', 'savedFrameBuffer.h', 'display_composite1.cxx', 'display_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libdisplay_igate.cxx', obj='libdisplay_igate.obj')

#
# DIRECTORY: panda/src/device/
#

IPATH=['panda/src/device']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='device_composite1.cxx', obj='device_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='device_composite2.cxx', obj='device_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libdevice.in', outc='libdevice_igate.cxx',
            src='panda/src/device',  module='panda', library='libdevice', files=[
            'analogNode.h', 'buttonNode.h', 'clientAnalogDevice.h', 'clientBase.h', 'clientButtonDevice.h', 'clientDevice.h', 'clientDialDevice.h', 'clientTrackerDevice.h', 'config_device.h', 'dialNode.h', 'mouseAndKeyboard.h', 'trackerData.h', 'trackerNode.h', 'virtualMouse.h', 'device_composite1.cxx', 'device_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libdevice_igate.cxx', obj='libdevice_igate.obj')

#
# DIRECTORY: panda/src/tform/
#

IPATH=['panda/src/tform']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='tform_composite1.cxx', obj='tform_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='tform_composite2.cxx', obj='tform_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libtform.in', outc='libtform_igate.cxx',
            src='panda/src/tform',  module='panda', library='libtform', files=[
            'buttonThrower.h', 'config_tform.h', 'driveInterface.h', 'mouseInterfaceNode.h', 'mouseWatcher.h', 'mouseWatcherGroup.h', 'mouseWatcherParameter.h', 'mouseWatcherRegion.h', 'trackball.h', 'transform2sg.h', 'tform_composite1.cxx', 'tform_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libtform_igate.cxx', obj='libtform_igate.obj')

#
# DIRECTORY: panda/src/collide/
#

IPATH=['panda/src/collide']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='collide_composite1.cxx', obj='collide_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='collide_composite2.cxx', obj='collide_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libcollide.in', outc='libcollide_igate.cxx',
            src='panda/src/collide',  module='panda', library='libcollide', files=[
            'collisionEntry.h', 'collisionHandler.h', 'collisionHandlerEvent.h', 'collisionHandlerFloor.h', 'collisionHandlerGravity.h', 'collisionHandlerPhysical.h', 'collisionHandlerPusher.h', 'collisionHandlerQueue.h', 'collisionInvSphere.h', 'collisionLine.h', 'collisionLevelState.h', 'collisionNode.h', 'collisionPlane.h', 'collisionPolygon.h', 'collisionRay.h', 'collisionRecorder.h', 'collisionSegment.h', 'collisionSolid.h', 'collisionSphere.h', 'collisionTraverser.h', 'collisionTube.h', 'collisionVisualizer.h', 'config_collide.h', 'collide_composite1.cxx', 'collide_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libcollide_igate.cxx', obj='libcollide_igate.obj')

#
# DIRECTORY: panda/src/pnmtext/
#

if (OMIT.count("FREETYPE")==0):
    IPATH=['panda/src/pnmtext']
    OPTS=['BUILDING_PANDA', 'NSPR', 'FREETYPE']
    CompileC(ipath=IPATH, opts=OPTS, src='config_pnmtext.cxx', obj='pnmtext_config_pnmtext.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='freetypeFont.cxx', obj='pnmtext_freetypeFont.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='pnmTextGlyph.cxx', obj='pnmtext_pnmTextGlyph.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='pnmTextMaker.cxx', obj='pnmtext_pnmTextMaker.obj')

#
# DIRECTORY: panda/src/text/
#

IPATH=['panda/src/text']
OPTS=['BUILDING_PANDA', 'ZLIB', 'NSPR', 'FREETYPE']
CompileC(ipath=IPATH, opts=OPTS, src='text_composite1.cxx', obj='text_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='text_composite2.cxx', obj='text_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libtext.in', outc='libtext_igate.cxx',
            src='panda/src/text',  module='panda', library='libtext', files=[
            'config_text.h', 'default_font.h', 'dynamicTextFont.h', 'dynamicTextGlyph.h', 'dynamicTextPage.h', 'fontPool.h', 'geomTextGlyph.h', 'staticTextFont.h', 'textAssembler.h', 'textFont.h', 'textGlyph.h', 'textNode.h', 'textProperties.h', 'textPropertiesManager.h', 'text_composite1.cxx', 'text_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libtext_igate.cxx', obj='libtext_igate.obj')

#
# DIRECTORY: panda/src/grutil/
#

IPATH=['panda/src/grutil']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='multitexReducer.cxx', obj='grutil_multitexReducer.obj')
CompileC(ipath=IPATH, opts=OPTS, src='grutil_composite1.cxx', obj='grutil_composite1.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libgrutil.in', outc='libgrutil_igate.cxx',
            src='panda/src/grutil',  module='panda', library='libgrutil', files=[
            'cardMaker.h', 'config_grutil.h', 'frameRateMeter.h', 'lineSegs.h', 'multitexReducer.h', 'multitexReducer.cxx', 'grutil_composite1.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libgrutil_igate.cxx', obj='libgrutil_igate.obj')

#
# DIRECTORY: panda/src/gsgmisc/
#

IPATH=['panda/src/gsgmisc']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='geomIssuer.cxx', obj='gsgmisc_geomIssuer.obj')

#
# DIRECTORY: panda/src/helix/
#

if (OMIT.count("HELIX")==0):
  IPATH=['panda/src/helix']
  OPTS=['BUILDING_PANDA', 'NSPR', 'HELIX']
  CompileC(ipath=IPATH, opts=OPTS, src='config_helix.cxx', obj='helix_config_helix.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='fivemmap.cxx', obj='helix_fivemmap.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='HelixClient.cxx', obj='helix_HelixClient.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='HxAdviseSink.cxx', obj='helix_HxAdviseSink.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='HxAuthenticationManager.cxx', obj='helix_HxAuthenticationManager.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='HxClientContext.cxx', obj='helix_HxClientContext.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='HxErrorSink.cxx', obj='helix_HxErrorSink.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='HxSiteSupplier.cxx', obj='helix_HxSiteSupplier.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='iids.cxx', obj='helix_iids.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='print.cxx', obj='helix_print.obj')
  Interrogate(ipath=IPATH, opts=OPTS, outd='libhelix.in', outc='libhelix_igate.cxx',
              src='panda/src/helix',  module='panda', library='libhelix', files=['HelixClient.cxx'])
  CompileC(ipath=IPATH, opts=OPTS, src='libhelix_igate.cxx', obj='libhelix_igate.obj')
  CompileLIB(lib='libhelix.ilb', obj=[
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
# DIRECTORY: panda/src/parametrics/
#

IPATH=['panda/src/parametrics']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='parametrics_composite1.cxx', obj='parametrics_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='parametrics_composite2.cxx', obj='parametrics_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libparametrics.in', outc='libparametrics_igate.cxx',
            src='panda/src/parametrics',  module='panda', library='libparametrics', files=[
            'classicNurbsCurve.h', 'config_parametrics.h', 'cubicCurveseg.h', 'parametricCurveDrawer.h',
            'curveFitter.h', 'hermiteCurve.h', 'nurbsCurve.h', 'nurbsCurveDrawer.h', 'nurbsCurveEvaluator.h',
            'nurbsCurveInterface.h', 'nurbsCurveResult.h', 'nurbsBasisVector.h', 'nurbsSurfaceEvaluator.h',
            'nurbsSurfaceResult.h', 'nurbsVertex.h', 'parametricCurve.h', 'parametricCurveCollection.h',
            'piecewiseCurve.h', 'ropeNode.h', 'sheetNode.h', 'parametrics_composite1.cxx', 'parametrics_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libparametrics_igate.cxx', obj='libparametrics_igate.obj')

#
# DIRECTORY: panda/src/pgui/
#

IPATH=['panda/src/pgui']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='pgui_composite1.cxx', obj='pgui_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pgui_composite2.cxx', obj='pgui_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libpgui.in', outc='libpgui_igate.cxx',
            src='panda/src/pgui',  module='panda', library='libpgui', files=[
            'config_pgui.h', 'pgButton.h', 'pgSliderButton.h', 'pgCullTraverser.h', 'pgEntry.h',
            'pgMouseWatcherGroup.h', 'pgMouseWatcherParameter.h', 'pgFrameStyle.h', 'pgItem.h',
            'pgMouseWatcherBackground.h', 'pgMouseWatcherRegion.h', 'pgTop.h', 'pgWaitBar.h', 'pgSliderBar.h',
            'pgui_composite1.cxx', 'pgui_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libpgui_igate.cxx', obj='libpgui_igate.obj')

#
# DIRECTORY: panda/src/pnmimagetypes/
#

IPATH=['panda/src/pnmimagetypes', 'panda/src/pnmimage']
OPTS=['BUILDING_PANDA', 'PNG', 'ZLIB', 'JPEG', 'ZLIB', 'NSPR', 'JPEG', 'TIFF']
CompileC(ipath=IPATH, opts=OPTS, src='pnmFileTypePNG.cxx', obj='pnmimagetypes_pnmFileTypePNG.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pnmFileTypeTIFF.cxx', obj='pnmimagetypes_pnmFileTypeTIFF.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pnmimagetypes_composite1.cxx', obj='pnmimagetypes_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pnmimagetypes_composite2.cxx', obj='pnmimagetypes_composite2.obj')

#
# DIRECTORY: panda/src/recorder/
#

IPATH=['panda/src/recorder']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='recorder_composite1.cxx', obj='recorder_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='recorder_composite2.cxx', obj='recorder_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='librecorder.in', outc='librecorder_igate.cxx',
            src='panda/src/recorder',  module='panda', library='librecorder', files=[
            'config_recorder.h', 'mouseRecorder.h', 'recorderBase.h', 'recorderController.h', 'recorderFrame.h', 'recorderHeader.h', 'recorderTable.h', 'socketStreamRecorder.h', 'recorder_composite1.cxx', 'recorder_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='librecorder_igate.cxx', obj='librecorder_igate.obj')

#
# DIRECTORY: panda/src/vrpn/
#

if (OMIT.count("VRPN")==0):
    IPATH=['panda/src/vrpn']
    OPTS=['BUILDING_PANDA', 'NSPR', 'VRPN']
    CompileC(ipath=IPATH, opts=OPTS, src='vrpn_composite1.cxx', obj='pvrpn_composite1.obj')
    Interrogate(ipath=IPATH, opts=OPTS, outd='libpvrpn.in', outc='libpvrpn_igate.cxx',
                src='panda/src/vrpn',  module='panda', library='libpvrpn', files=[
                'config_vrpn.cxx', 'config_vrpn.h', 'vrpnClient.cxx', 'vrpnAnalog.cxx', 'vrpnAnalog.h',
                'vrpnAnalogDevice.cxx', 'vrpnAnalogDevice.h', 'vrpnButton.cxx', 'vrpnButton.h',
                'vrpnButtonDevice.cxx', 'vrpnButtonDevice.h', 'vrpnClient.h', 'vrpnDial.cxx', 'vrpnDial.h',
                'vrpnDialDevice.cxx', 'vrpnDialDevice.h', 'vrpnTracker.cxx', 'vrpnTracker.h', 'vrpnTrackerDevice.cxx',
                'vrpnTrackerDevice.h', 'vrpn_interface.h'])
    CompileC(ipath=IPATH, opts=OPTS, src='libpvrpn_igate.cxx', obj='libpvrpn_igate.obj')

#
# DIRECTORY: panda/metalibs/panda/
#

IPATH=['panda/metalibs/panda']
OPTS=['BUILDING_PANDA', 'ZLIB', 'VRPN', 'JPEG', 'PNG', 'TIFF', 'NSPR', 'FREETYPE', 'HELIX', 'FFTW',
      'ADVAPI', 'WINSOCK2', 'WINUSER', 'WINMM']
INFILES=['librecorder.in', 'libpgraph.in', 'libgrutil.in', 'libchan.in', 'libpstatclient.in',
         'libchar.in', 'libcollide.in', 'libdevice.in', 'libdgraph.in', 'libdisplay.in', 'libevent.in',
         'libgobj.in', 'libgsgbase.in', 'liblinmath.in', 'libmathutil.in', 'libparametrics.in',
         'libpnmimage.in', 'libtext.in', 'libtform.in', 'liblerp.in', 'libputil.in', 'libaudio.in',
         'libpgui.in']
OBJFILES=['panda_panda.obj', 'libpanda_module.obj', 'recorder_composite1.obj',
          'recorder_composite2.obj', 'librecorder_igate.obj',
          'pgraph_nodePath.obj', 'pgraph_composite1.obj', 'pgraph_composite2.obj', 'libpgraph_igate.obj',
          'grutil_multitexReducer.obj', 'grutil_composite1.obj', 'libgrutil_igate.obj',
          'chan_composite1.obj', 'chan_composite2.obj', 'libchan_igate.obj', 'pstatclient_composite1.obj',
          'pstatclient_composite2.obj', 'libpstatclient_igate.obj', 'char_composite1.obj',
          'char_composite2.obj', 'libchar_igate.obj', 'collide_composite1.obj', 'collide_composite2.obj',
          'libcollide_igate.obj', 'device_composite1.obj', 'device_composite2.obj', 'libdevice_igate.obj',
          'dgraph_composite1.obj', 'dgraph_composite2.obj', 'libdgraph_igate.obj', 'display_composite1.obj',
          'display_composite2.obj', 'libdisplay_igate.obj', 'event_composite1.obj', 'libevent_igate.obj',
          'gobj_composite1.obj', 'gobj_composite2.obj', 'libgobj_igate.obj', 'gsgbase_composite1.obj',
          'libgsgbase_igate.obj', 'gsgmisc_geomIssuer.obj', 'linmath_composite1.obj',
          'linmath_composite2.obj', 'liblinmath_igate.obj',
          'mathutil_composite1.obj', 'mathutil_composite2.obj', 'libmathutil_igate.obj',
          'parametrics_composite1.obj', 'parametrics_composite2.obj', 'libparametrics_igate.obj',
          'pnmimagetypes_pnmFileTypePNG.obj', 'pnmimagetypes_pnmFileTypeTIFF.obj', 'pnmimagetypes_composite1.obj',
          'pnmimagetypes_composite2.obj', 'pnmimage_composite1.obj', 'pnmimage_composite2.obj', 'libpnmimage_igate.obj',
          'text_composite1.obj', 'text_composite2.obj', 'libtext_igate.obj',
          'tform_composite1.obj', 'tform_composite2.obj',
          'libtform_igate.obj', 'lerp_composite1.obj', 'liblerp_igate.obj',
          'putil_composite1.obj', 'putil_composite2.obj', 'libputil_igate.obj',
          'audio_composite1.obj', 'libaudio_igate.obj', 'pgui_composite1.obj', 'pgui_composite2.obj',
          'libpgui_igate.obj', 'pandabase_pandabase.obj', 'libpandaexpress.dll', 'libdtoolconfig.dll', 'libdtool.dll']
if OMIT.count("HELIX")==0:
    OBJFILES.append("libhelix.ilb")
    INFILES.append("libhelix.in")
if OMIT.count("VRPN")==0:
    OBJFILES.append("pvrpn_composite1.obj")
    OBJFILES.append("libpvrpn_igate.obj")
    INFILES.append("libpvrpn.in")
if OMIT.count("NSPR")==0:
    OBJFILES.append("net_composite1.obj")
    OBJFILES.append("net_composite2.obj")
    OBJFILES.append("libnet_igate.obj")
    INFILES.append("libnet.in")
if OMIT.count("FREETYPE")==0:
    OBJFILES.append("pnmtext_config_pnmtext.obj")
    OBJFILES.append("pnmtext_freetypeFont.obj")
    OBJFILES.append("pnmtext_pnmTextGlyph.obj")
    OBJFILES.append("pnmtext_pnmTextMaker.obj")
InterrogateModule(outc='libpanda_module.cxx', module='panda', library='libpanda', files=INFILES)
CompileC(ipath=IPATH, opts=OPTS, src='panda.cxx', obj='panda_panda.obj')
CompileC(ipath=IPATH, opts=OPTS, src='libpanda_module.cxx', obj='libpanda_module.obj')
CompileLink(opts=OPTS, dll='libpanda.dll', obj=OBJFILES, xdep=[
        PREFIX+'/tmp/dtool_have_helix.dat',
        PREFIX+'/tmp/dtool_have_vrpn.dat',
        PREFIX+'/tmp/dtool_have_nspr.dat',
        PREFIX+'/tmp/dtool_have_freetype.dat',
])

#
# DIRECTORY: panda/src/audiotraits/
#

if OMIT.count("FMOD") == 0:
  IPATH=['panda/src/audiotraits']
  OPTS=['BUILDING_FMOD_AUDIO', 'NSPR', 'FMOD']
  CompileC(ipath=IPATH, opts=OPTS, src='fmod_audio_composite1.cxx', obj='fmod_audio_fmod_audio_composite1.obj')
  CompileLink(opts=['ADVAPI', 'WINUSER', 'WINMM', 'FMOD', 'NSPR'], dll='libfmod_audio.dll', obj=[
               'fmod_audio_fmod_audio_composite1.obj',
               'libpanda.dll',
               'libpandaexpress.dll',
               'libdtoolconfig.dll',
               'libdtool.dll',
  ])

if OMIT.count("MILES") == 0:
  IPATH=['panda/src/audiotraits']
  OPTS=['BUILDING_MILES_AUDIO', 'NSPR', 'MILES']
  CompileC(ipath=IPATH, opts=OPTS, src='miles_audio_composite1.cxx', obj='miles_audio_miles_audio_composite1.obj')
  CompileLink(opts=['ADVAPI', 'WINUSER', 'WINMM', 'MILES', 'NSPR'], dll='libmiles_audio.dll', obj=[
               'miles_audio_miles_audio_composite1.obj',
               'libpanda.dll',
               'libpandaexpress.dll',
               'libdtoolconfig.dll',
               'libdtool.dll',
  ])

#
# DIRECTORY: panda/src/builder/
#

IPATH=['panda/src/builder']
OPTS=['BUILDING_PANDAEGG', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='builder_composite1.cxx', obj='builder_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='builder_composite2.cxx', obj='builder_composite2.obj')

#
# DIRECTORY: panda/src/distort/
#

IPATH=['panda/src/distort']
OPTS=['BUILDING_PANDAFX', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='distort_composite1.cxx', obj='distort_composite1.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libdistort.in', outc='libdistort_igate.cxx',
            src='panda/src/distort',  module='pandafx', library='libdistort',
            files=['config_distort.h', 'projectionScreen.h', 'cylindricalLens.h', 'fisheyeLens.h', 'nonlinearImager.h', 'pSphereLens.h', 'distort_composite1.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libdistort_igate.cxx', obj='libdistort_igate.obj')

#
# DIRECTORY: panda/src/downloadertools/
#

if OMIT.count("SSL")==0:
    IPATH=['panda/src/downloadertools']
    OPTS=['SSL', 'ZLIB', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='apply_patch.cxx', obj='apply_patch_apply_patch.obj')
    CompileLink(dll='apply_patch.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'apply_patch_apply_patch.obj',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    CompileC(ipath=IPATH, opts=OPTS, src='build_patch.cxx', obj='build_patch_build_patch.obj')
    CompileLink(dll='build_patch.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'build_patch_build_patch.obj',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    IPATH=['panda/src/downloadertools']
    OPTS=['SSL', 'ZLIB', 'ZLIB', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='check_adler.cxx', obj='check_adler_check_adler.obj')
    CompileLink(dll='check_adler.exe', opts=['ADVAPI', 'NSPR', 'ZLIB'], obj=[
                 'check_adler_check_adler.obj',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    CompileC(ipath=IPATH, opts=OPTS, src='check_crc.cxx', obj='check_crc_check_crc.obj')
    CompileLink(dll='check_crc.exe', opts=['ADVAPI', 'NSPR', 'ZLIB'], obj=[
                 'check_crc_check_crc.obj',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    IPATH=['panda/src/downloadertools']
    OPTS=['SSL', 'ZLIB', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='check_md5.cxx', obj='check_md5_check_md5.obj')
    CompileLink(dll='check_md5.exe', opts=['ADVAPI', 'NSPR', 'SSL'], obj=[
                 'check_md5_check_md5.obj',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    CompileC(ipath=IPATH, opts=OPTS, src='multify.cxx', obj='multify_multify.obj')
    CompileLink(dll='multify.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'multify_multify.obj',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    IPATH=['panda/src/downloadertools']
    OPTS=['SSL', 'ZLIB', 'ZLIB', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='pcompress.cxx', obj='pcompress_pcompress.obj')
    CompileLink(dll='pcompress.exe', opts=['ADVAPI', 'NSPR', 'ZLIB'], obj=[
                 'pcompress_pcompress.obj',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    CompileC(ipath=IPATH, opts=OPTS, src='pdecompress.cxx', obj='pdecompress_pdecompress.obj')
    CompileLink(dll='pdecompress.exe', opts=['ADVAPI', 'NSPR', 'ZLIB'], obj=[
                 'pdecompress_pdecompress.obj',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    IPATH=['panda/src/downloadertools']
    OPTS=['SSL', 'ZLIB', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='pdecrypt.cxx', obj='pdecrypt_pdecrypt.obj')
    CompileLink(dll='pdecrypt.exe', opts=['ADVAPI', 'NSPR', 'SSL'], obj=[
                 'pdecrypt_pdecrypt.obj',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    CompileC(ipath=IPATH, opts=OPTS, src='pencrypt.cxx', obj='pencrypt_pencrypt.obj')
    CompileLink(dll='pencrypt.exe', opts=['ADVAPI', 'NSPR', 'SSL'], obj=[
                 'pencrypt_pencrypt.obj',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])
    CompileC(ipath=IPATH, opts=OPTS, src='show_ddb.cxx', obj='show_ddb_show_ddb.obj')
    CompileLink(dll='show_ddb.exe', opts=['ADVAPI', 'NSPR'], obj=[
                 'show_ddb_show_ddb.obj',
                 'libpandaexpress.dll',
                 'libpanda.dll',
                 'libdtoolconfig.dll',
                 'libdtool.dll',
                 'libpystub.dll',
    ])

#
# DIRECTORY: panda/src/glgsg/
#

IPATH=['panda/src/glgsg', 'panda/src/glstuff', 'panda/src/gobj']
OPTS=['BUILDING_PANDAGL', 'NSPR', 'NVIDIACG']
CompileC(ipath=IPATH, opts=OPTS, src='config_glgsg.cxx', obj='glgsg_config_glgsg.obj')
CompileC(ipath=IPATH, opts=OPTS, src='glgsg.cxx', obj='glgsg_glgsg.obj')

#
# DIRECTORY: panda/src/effects/
#

IPATH=['panda/src/effects']
OPTS=['BUILDING_PANDAFX', 'NSPR', 'NVIDIACG']
CompileC(ipath=IPATH, opts=OPTS, src='effects_composite1.cxx', obj='effects_composite1.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libeffects.in', outc='libeffects_igate.cxx',
            src='panda/src/effects',  module='pandafx', library='libeffects',
            files=['config_effects.h', 'cgShader.h', 'cgShaderAttrib.h', 'cgShaderContext.h', 'lensFlareNode.h', 'effects_composite1.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libeffects_igate.cxx', obj='libeffects_igate.obj')

#
# DIRECTORY: panda/metalibs/pandafx/
#

IPATH=['panda/metalibs/pandafx', 'panda/src/distort']
OPTS=['BUILDING_PANDAFX', 'NSPR', 'NVIDIACG']
CompileC(ipath=IPATH, opts=OPTS, src='pandafx.cxx', obj='pandafx_pandafx.obj')
InterrogateModule(outc='libpandafx_module.cxx', module='pandafx', library='libpandafx',
                  files=['libdistort.in', 'libeffects.in'])
CompileC(ipath=IPATH, opts=OPTS, src='libpandafx_module.cxx', obj='libpandafx_module.obj')
CompileLink(dll='libpandafx.dll', opts=['ADVAPI', 'NSPR', 'NVIDIACG'], obj=[
             'pandafx_pandafx.obj',
             'libpandafx_module.obj',
             'distort_composite1.obj',
             'libdistort_igate.obj',
             'effects_composite1.obj',
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
CompileC(ipath=IPATH, opts=OPTS, src='glpure.cxx', obj='glstuff_glpure.obj')
CompileLink(dll='libglstuff.dll', opts=['ADVAPI', 'GLUT', 'NSPR', 'NVIDIACG', 'CGGL'], obj=[
             'glstuff_glpure.obj',
             'libpanda.dll',
             'libpandafx.dll',
             'libpandaexpress.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

#
# DIRECTORY: panda/src/windisplay/
#

if (sys.platform == "win32" or sys.platform == "cygwin"):
    IPATH=['panda/src/windisplay']
    OPTS=['BUILDING_PANDAWIN', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='winGraphicsWindow.cxx', obj='windisplay_winGraphicsWindow.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='config_windisplay.cxx', obj='windisplay_config_windisplay.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='winGraphicsPipe.cxx', obj='windisplay_winGraphicsPipe.obj')
    CompileLink(opts=['WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM', 'NSPR'],
                dll='libwindisplay.dll', obj=[
      'windisplay_winGraphicsWindow.obj',
      'windisplay_config_windisplay.obj',
      'windisplay_winGraphicsPipe.obj',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libdtoolconfig.dll',
      'libdtool.dll',
      ])

#
# DIRECTORY: panda/src/glxdisplay/
#

if (sys.platform != "win32" and sys.platform != "cygwin"):
    IPATH=['panda/src/glxdisplay', 'panda/src/gobj']
    OPTS=['BUILDING_PANDAGLUT', 'NSPR', 'GLUT', 'NVIDIACG', 'CGGL']
    CompileC(ipath=IPATH, opts=OPTS, src='glxdisplay_composite1.cxx',     obj='glxdisplay_composite1.obj')
    Interrogate(ipath=IPATH, opts=OPTS, outd='libglxdisplay.in', outc='libglxdisplay_igate.cxx',
                src='panda/src/glxdisplay',  module='pandagl', library='libglxdisplay', files=['glxGraphicsPipe.h'])
    CompileC(ipath=IPATH, opts=OPTS, src='libglxdisplay_igate.cxx',      obj='libglxdisplay_igate.obj')

    IPATH=['panda/metalibs/pandagl']
    OPTS=['BUILDING_PANDAGL', 'NSPR', 'NVIDIACG', 'CGGL']
    CompileC(ipath=IPATH, opts=OPTS, src='pandagl.cxx', obj='pandagl_pandagl.obj')
    CompileLink(opts=['GLUT', 'NVIDIACG', 'CGGL', 'NSPR'], dll='libpandagl.dll', obj=[
      'pandagl_pandagl.obj',
      'glgsg_config_glgsg.obj',
      'glgsg_glgsg.obj',
      'glxdisplay_composite1.obj',
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

if (sys.platform == "win32" or sys.platform == "cygwin"):
    IPATH=['panda/src/wgldisplay', 'panda/src/glstuff', 'panda/src/gobj']
    OPTS=['BUILDING_PANDAGL', 'NSPR', 'NVIDIACG', 'CGGL']
    CompileC(ipath=IPATH, opts=OPTS, src='wgldisplay_composite1.cxx', obj='wgldisplay_composite1.obj')

    IPATH=['panda/metalibs/pandagl']
    OPTS=['BUILDING_PANDAGL', 'NSPR', 'NVIDIACG', 'CGGL']
    CompileC(ipath=IPATH, opts=OPTS, src='pandagl.cxx', obj='pandagl_pandagl.obj')
    CompileLink(opts=['WINGDI', 'GLUT', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM', 'NSPR', 'NVIDIACG', 'CGGL'],
                dll='libpandagl.dll', obj=[
      'pandagl_pandagl.obj',
      'glgsg_config_glgsg.obj',
      'glgsg_glgsg.obj',
      'wgldisplay_composite1.obj',
      'libwindisplay.dll',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libglstuff.dll',
      'libpandafx.dll',
      'libdtoolconfig.dll',
      'libdtool.dll',
      ])

#
# DIRECTORY: panda/metalibs/pandadx7/
#

if (sys.platform == "win32" or sys.platform == "cygwin"):
    IPATH=['panda/src/dxgsg7']
    OPTS=['BUILDING_PANDADX', 'DXSDK', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='dxGraphicsStateGuardian7.cxx', obj='dxgsg7_dxGraphicsStateGuardian7.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='dxgsg7_composite1.cxx', obj='dxgsg7_composite1.obj')

    IPATH=['panda/metalibs/pandadx7']
    OPTS=['BUILDING_PANDADX', 'DXSDK', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='pandadx7.cxx', obj='pandadx7_pandadx7.obj')
    CompileLink(dll='libpandadx7.dll', opts=['ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DXDRAW', 'DXGUID', 'D3D8', 'NSPR'], obj=[
      'pandadx7_pandadx7.obj',
      'dxgsg7_dxGraphicsStateGuardian7.obj',
      'dxgsg7_composite1.obj',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libwindisplay.dll',
      'libdtoolconfig.dll',
      'libdtool.dll',
      ])

#
# DIRECTORY: panda/metalibs/pandadx8/
#

if (sys.platform == "win32" or sys.platform == "cygwin"):
    IPATH=['panda/src/dxgsg8']
    OPTS=['BUILDING_PANDADX', 'DXSDK', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='dxGraphicsStateGuardian8.cxx', obj='dxgsg8_dxGraphicsStateGuardian8.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='dxgsg8_composite1.cxx', obj='dxgsg8_composite1.obj')

    IPATH=['panda/metalibs/pandadx8']
    OPTS=['BUILDING_PANDADX', 'DXSDK', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='pandadx8.cxx', obj='pandadx8_pandadx8.obj')
    CompileLink(dll='libpandadx8.dll',
      opts=['ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DXDRAW', 'DXGUID', 'D3D8', 'NSPR'], obj=[
      'pandadx8_pandadx8.obj',
      'dxgsg8_dxGraphicsStateGuardian8.obj',
      'dxgsg8_composite1.obj',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libwindisplay.dll',
      'libdtoolconfig.dll',
      'libdtool.dll',
      ])

#
# DIRECTORY: panda/metalibs/pandadx9/
#

if (sys.platform == "win32" or sys.platform == "cygwin"):
    IPATH=['panda/src/dxgsg9']
    OPTS=['BUILDING_PANDADX', 'DXSDK', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='dxGraphicsStateGuardian9.cxx', obj='dxgsg9_dxGraphicsStateGuardian9.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='dxgsg9_composite1.cxx', obj='dxgsg9_composite1.obj')

    IPATH=['panda/metalibs/pandadx9']
    OPTS=['BUILDING_PANDADX', 'DXSDK', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='pandadx9.cxx', obj='pandadx9_pandadx9.obj')
    CompileLink(dll='libpandadx9.dll',
      opts=['ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DXDRAW', 'DXGUID', 'D3D9', 'NSPR'], obj=[
      'pandadx9_pandadx9.obj',
      'dxgsg9_dxGraphicsStateGuardian9.obj',
      'dxgsg9_composite1.obj',
      'libpanda.dll',
      'libpandaexpress.dll',
      'libwindisplay.dll',
      'libdtoolconfig.dll',
      'libdtool.dll',
      ])

#
# DIRECTORY: panda/src/egg/
#

IPATH=['panda/src/egg']
CompileBison(pre='eggyy', dstc='parser.cxx', dsth='parser.h', src='panda/src/egg/parser.yxx')
CompileFlex(pre='eggyy', dst='lexer.cxx', src='panda/src/egg/lexer.lxx', dashi=1)
OPTS=['BUILDING_PANDAEGG', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='egg_composite1.cxx', obj='egg_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='egg_composite2.cxx', obj='egg_composite2.obj')
CompileC(ipath=IPATH, opts=OPTS, src='parser.cxx', obj='egg_parser.obj')
CompileC(ipath=IPATH, opts=OPTS, src='lexer.cxx', obj='egg_lexer.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libegg.in', outc='libegg_igate.cxx',
            src='panda/src/egg',  module='pandaegg', library='libegg', files=[
            'config_egg.h', 'eggAnimData.h', 'eggAttributes.h', 'eggBin.h', 'eggBinMaker.h', 'eggComment.h',
            'eggCoordinateSystem.h', 'eggCurve.h', 'eggData.h', 'eggExternalReference.h', 'eggFilenameNode.h',
            'eggGroup.h', 'eggGroupNode.h', 'eggGroupUniquifier.h', 'eggLine.h', 'eggMaterial.h',
            'eggMaterialCollection.h', 'eggMiscFuncs.h', 'eggMorph.h', 'eggMorphList.h', 'eggNamedObject.h',
            'eggNameUniquifier.h', 'eggNode.h', 'eggNurbsCurve.h', 'eggNurbsSurface.h', 'eggObject.h',
            'eggParameters.h', 'eggPoint.h', 'eggPolygon.h', 'eggPolysetMaker.h', 'eggPoolUniquifier.h',
            'eggPrimitive.h', 'eggRenderMode.h', 'eggSAnimData.h', 'eggSurface.h', 'eggSwitchCondition.h',
            'eggTable.h', 'eggTexture.h', 'eggTextureCollection.h', 'eggTransform3d.h', 'eggUserData.h',
            'eggUtilities.h', 'eggVertex.h', 'eggVertexPool.h', 'eggVertexUV.h', 'eggXfmAnimData.h',
            'eggXfmSAnim.h', 'parserDefs.h', 'lexerDefs.h', 'pt_EggMaterial.h', 'vector_PT_EggMaterial.h',
            'pt_EggTexture.h', 'vector_PT_EggTexture.h', 'pt_EggVertex.h', 'vector_PT_EggVertex.h',
            'egg_composite1.cxx', 'egg_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libegg_igate.cxx', obj='libegg_igate.obj')

#
# DIRECTORY: panda/src/egg2pg/
#

IPATH=['panda/src/egg2pg']
OPTS=['BUILDING_PANDAEGG', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='egg2pg_composite1.cxx', obj='egg2pg_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='egg2pg_composite2.cxx', obj='egg2pg_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libegg2pg.in', outc='libegg2pg_igate.cxx',
            src='panda/src/egg2pg',  module='pandaegg', library='libegg2pg', files=['load_egg_file.h'])
CompileC(ipath=IPATH, opts=OPTS, src='libegg2pg_igate.cxx', obj='libegg2pg_igate.obj')

#
# DIRECTORY: panda/src/framework/
#

IPATH=['panda/src/framework']
OPTS=['BUILDING_FRAMEWORK', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='config_framework.cxx', obj='framework_config_framework.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pandaFramework.cxx', obj='framework_pandaFramework.obj')
CompileC(ipath=IPATH, opts=OPTS, src='windowFramework.cxx', obj='framework_windowFramework.obj')
CompileLink(dll='libframework.dll', opts=['ADVAPI', 'NSPR'], obj=[
             'framework_config_framework.obj',
             'framework_pandaFramework.obj',
             'framework_windowFramework.obj',
             'libpanda.dll',
             'libpandaexpress.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
             ])

#
# DIRECTORY: panda/metalibs/pandaegg/
#

IPATH=['panda/metalibs/pandaegg', 'panda/src/egg']
OPTS=['BUILDING_PANDAEGG', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='pandaegg.cxx', obj='pandaegg_pandaegg.obj')
InterrogateModule(outc='libpandaegg_module.cxx', module='pandaegg', library='libpandaegg',
                  files=['libegg2pg.in', 'libegg.in'])
CompileC(ipath=IPATH, opts=OPTS, src='libpandaegg_module.cxx', obj='libpandaegg_module.obj')
CompileLink(dll='libpandaegg.dll', opts=['ADVAPI', 'NSPR'], obj=[
             'pandaegg_pandaegg.obj',
             'libpandaegg_module.obj',
             'egg2pg_composite1.obj',
             'egg2pg_composite2.obj',
             'libegg2pg_igate.obj',
             'egg_composite1.obj',
             'egg_composite2.obj',
             'egg_parser.obj',
             'egg_lexer.obj',
             'libegg_igate.obj',
             'builder_composite1.obj',
             'builder_composite2.obj',
             'libpanda.dll',
             'libpandaexpress.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

#
# DIRECTORY: panda/src/physics/
#

IPATH=['panda/src/physics']
OPTS=['BUILDING_PANDAPHYSICS', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='physics_composite1.cxx', obj='physics_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='physics_composite2.cxx', obj='physics_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libphysics.in', outc='libphysics_igate.cxx',
            src='panda/src/physics',  module='pandaphysics', library='libphysics',
            files=['actorNode.h', 'angularEulerIntegrator.h', 'angularForce.h', 'angularIntegrator.h', 'angularVectorForce.h', 'baseForce.h', 'baseIntegrator.h', 'config_physics.h', 'forceNode.h', 'linearCylinderVortexForce.h', 'linearDistanceForce.h', 'linearEulerIntegrator.h', 'linearForce.h', 'linearFrictionForce.h', 'linearIntegrator.h', 'linearJitterForce.h', 'linearNoiseForce.h', 'linearRandomForce.h', 'linearSinkForce.h', 'linearSourceForce.h', 'linearUserDefinedForce.h', 'linearVectorForce.h', 'physical.h', 'physicalNode.h', 'physicsCollisionHandler.h', 'physicsManager.h', 'physicsObject.h', 'physics_composite1.cxx', 'physics_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libphysics_igate.cxx', obj='libphysics_igate.obj')

#
# DIRECTORY: panda/src/particlesystem/
#

IPATH=['panda/src/particlesystem']
OPTS=['BUILDING_PANDAPHYSICS', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='particlesystem_composite1.cxx', obj='particlesystem_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='particlesystem_composite2.cxx', obj='particlesystem_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libparticlesystem.in', outc='libparticlesystem_igate.cxx',
            src='panda/src/particlesystem',  module='pandaphysics', library='libparticlesystem',
            files=['baseParticle.h', 'baseParticleEmitter.h', 'baseParticleFactory.h', 'baseParticleRenderer.h', 'boxEmitter.h', 'config_particlesystem.h', 'discEmitter.h', 'geomParticleRenderer.h', 'lineEmitter.h', 'lineParticleRenderer.h', 'particleSystem.h', 'particleSystemManager.h', 'pointEmitter.h', 'pointParticle.h', 'pointParticleFactory.h', 'pointParticleRenderer.h', 'rectangleEmitter.h', 'ringEmitter.h', 'sparkleParticleRenderer.h', 'sphereSurfaceEmitter.h', 'sphereVolumeEmitter.h', 'spriteParticleRenderer.h', 'tangentRingEmitter.h', 'zSpinParticle.h', 'zSpinParticleFactory.h', 'particleCommonFuncs.h', 'particlesystem_composite1.cxx', 'particlesystem_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libparticlesystem_igate.cxx', obj='libparticlesystem_igate.obj')

#
# DIRECTORY: panda/metalibs/pandaphysics/
#

IPATH=['panda/metalibs/pandaphysics']
OPTS=['BUILDING_PANDAPHYSICS', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='pandaphysics.cxx', obj='pandaphysics_pandaphysics.obj')
InterrogateModule(outc='libpandaphysics_module.cxx', module='pandaphysics', library='libpandaphysics',
                  files=['libphysics.in', 'libparticlesystem.in'])
CompileC(ipath=IPATH, opts=OPTS, src='libpandaphysics_module.cxx', obj='libpandaphysics_module.obj')
CompileLink(dll='libpandaphysics.dll', opts=['ADVAPI', 'NSPR'], obj=[
             'pandaphysics_pandaphysics.obj',
             'libpandaphysics_module.obj',
             'physics_composite1.obj',
             'physics_composite2.obj',
             'libphysics_igate.obj',
             'particlesystem_composite1.obj',
             'particlesystem_composite2.obj',
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
CompileC(ipath=IPATH, opts=OPTS, src='pview.cxx', obj='pview_pview.obj')
CompileLink(dll='pview.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

IPATH=['direct/src/directbase']
OPTS=['BUILDING_DIRECT', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='directbase.cxx', obj='directbase_directbase.obj')

#
# DIRECTORY: direct/src/dcparser/
#

CompileBison(pre='dcyy', dstc='dcParser.cxx', dsth='dcParser.h', src='direct/src/dcparser/dcParser.yxx')
CompileFlex(pre='dcyy', dst='dcLexer.cxx', src='direct/src/dcparser/dcLexer.lxx', dashi=0)
IPATH=['direct/src/dcparser']
OPTS=['WITHINPANDA', 'BUILDING_DIRECT', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='dcparser_composite1.cxx', obj='dcparser_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='dcparser_composite2.cxx', obj='dcparser_composite2.obj')
CompileC(ipath=IPATH, opts=OPTS, src='dcParser.cxx', obj='dcparser_dcParser.obj')
CompileC(ipath=IPATH, opts=OPTS, src='dcLexer.cxx', obj='dcparser_dcLexer.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libdcparser.in', outc='libdcparser_igate.cxx',
            src='direct/src/dcparser',  module='direct', library='libdcparser',
            files=['dcAtomicField.h', 'dcClass.h', 'dcDeclaration.h', 'dcField.h', 'dcFile.h',
            'dcLexerDefs.h', 'dcMolecularField.h', 'dcParserDefs.h', 'dcSubatomicType.h',
            'dcPackData.h', 'dcPacker.h', 'dcPackerCatalog.h', 'dcPackerInterface.h',
            'dcParameter.h', 'dcClassParameter.h', 'dcArrayParameter.h', 'dcSimpleParameter.h',
            'dcSwitchParameter.h', 'dcNumericRange.h', 'dcSwitch.h', 'dcTypedef.h', 'dcPython.h',
            'dcbase.h', 'dcindent.h', 'hashGenerator.h', 'primeNumberGenerator.h',
            'dcparser_composite1.cxx', 'dcparser_composite2.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libdcparser_igate.cxx', obj='libdcparser_igate.obj')

#
# DIRECTORY: direct/src/deadrec/
#

IPATH=['direct/src/deadrec']
OPTS=['BUILDING_DIRECT', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='deadrec_composite1.cxx', obj='deadrec_composite1.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libdeadrec.in', outc='libdeadrec_igate.cxx',
            src='direct/src/deadrec',  module='direct', library='libdeadrec',
            files=['smoothMover.h', 'deadrec_composite1.cxx'])
CompileC(ipath=IPATH, opts=OPTS, src='libdeadrec_igate.cxx', obj='libdeadrec_igate.obj')

#
# DIRECTORY: direct/src/distributed/
#

IPATH=['direct/src/distributed', 'direct/src/dcparser']
OPTS=['WITHINPANDA', 'BUILDING_DIRECT', 'SSL', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='config_distributed.cxx', obj='distributed_config_distributed.obj')
CompileC(ipath=IPATH, opts=OPTS, src='cConnectionRepository.cxx', obj='distributed_cConnectionRepository.obj')
CompileC(ipath=IPATH, opts=OPTS, src='cDistributedSmoothNodeBase.cxx', obj='distributed_cDistributedSmoothNodeBase.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libdistributed.in', outc='libdistributed_igate.cxx',
            src='direct/src/distributed',  module='direct', library='libdistributed',
            files=['config_distributed.cxx', 'config_distributed.h', 'cConnectionRepository.cxx',
            'cConnectionRepository.h', 'cDistributedSmoothNodeBase.cxx', 'cDistributedSmoothNodeBase.h'])
CompileC(ipath=IPATH, opts=OPTS, src='libdistributed_igate.cxx', obj='libdistributed_igate.obj')

#
# DIRECTORY: direct/src/interval/
#

IPATH=['direct/src/interval']
OPTS=['BUILDING_DIRECT', 'NSPR']
#CompileC(ipath=IPATH, opts=OPTS, src='config_interval.cxx', obj='interval_config_interval.obj')
#CompileC(ipath=IPATH, opts=OPTS, src='cInterval.cxx', obj='interval_cInterval.obj')
#CompileC(ipath=IPATH, opts=OPTS, src='cIntervalManager.cxx', obj='interval_cIntervalManager.obj')
#CompileC(ipath=IPATH, opts=OPTS, src='cLerpInterval.cxx', obj='interval_cLerpInterval.obj')
#CompileC(ipath=IPATH, opts=OPTS, src='cLerpNodePathInterval.cxx', obj='interval_cLerpNodePathInterval.obj')
#CompileC(ipath=IPATH, opts=OPTS, src='cLerpAnimEffectInterval.cxx', obj='interval_cLerpAnimEffectInterval.obj')
#CompileC(ipath=IPATH, opts=OPTS, src='cMetaInterval.cxx', obj='interval_cMetaInterval.obj')
#CompileC(ipath=IPATH, opts=OPTS, src='hideInterval.cxx', obj='interval_hideInterval.obj')
#CompileC(ipath=IPATH, opts=OPTS, src='showInterval.cxx', obj='interval_showInterval.obj')
#CompileC(ipath=IPATH, opts=OPTS, src='waitInterval.cxx', obj='interval_waitInterval.obj')
CompileC(ipath=IPATH, opts=OPTS, src='interval_composite1.cxx', obj='interval_composite1.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libinterval.in', outc='libinterval_igate.cxx',
            src='direct/src/interval',  module='direct', library='libinterval',
            files=['config_interval.cxx', 'config_interval.h', 'cInterval.cxx', 'cInterval.h',
            'cIntervalManager.cxx', 'cIntervalManager.h', 'cLerpInterval.cxx', 'cLerpInterval.h',
            'cLerpNodePathInterval.cxx', 'cLerpNodePathInterval.h', 'cLerpAnimEffectInterval.cxx',
            'cLerpAnimEffectInterval.h', 'cMetaInterval.cxx', 'cMetaInterval.h', 'hideInterval.cxx',
            'hideInterval.h', 'showInterval.cxx', 'showInterval.h', 'waitInterval.cxx', 'waitInterval.h',
            'lerp_helpers.h'])
CompileC(ipath=IPATH, opts=OPTS, src='libinterval_igate.cxx', obj='libinterval_igate.obj')

#
# DIRECTORY: direct/src/showbase/
#

IPATH=['direct/src/showbase']
OPTS=['BUILDING_DIRECT', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='showBase.cxx', obj='showbase_showBase.obj')
CompileC(ipath=IPATH, opts=OPTS, src='mersenne.cxx', obj='showbase_mersenne.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libshowbase.in', outc='libshowbase_igate.cxx',
            src='direct/src/showbase', module='direct', library='libshowbase',
            files=['showBase.cxx', 'showBase.h', 'mersenne.cxx', 'mersenne.h'])
CompileC(ipath=IPATH, opts=OPTS, src='libshowbase_igate.cxx', obj='libshowbase_igate.obj')

#
# DIRECTORY: direct/metalibs/direct/
#

IPATH=['direct/metalibs/direct']
OPTS=['BUILDING_DIRECT', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='direct.cxx', obj='direct_direct.obj')
InterrogateModule(outc='libdirect_module.cxx', module='direct', library='libdirect',
                  files=['libdcparser.in', 'libshowbase.in', 'libdeadrec.in', 'libinterval.in', 'libdistributed.in'])
CompileC(ipath=IPATH, opts=OPTS, src='libdirect_module.cxx', obj='libdirect_module.obj')
CompileLink(dll='libdirect.dll', opts=['ADVAPI', 'NSPR', 'SSL'], obj=[
             'direct_direct.obj',
             'libdirect_module.obj',
             'directbase_directbase.obj',
             'dcparser_composite1.obj',
             'dcparser_composite2.obj',
             'dcparser_dcParser.obj',
             'dcparser_dcLexer.obj',
             'libdcparser_igate.obj',
             'showbase_showBase.obj',
             'showbase_mersenne.obj',
             'libshowbase_igate.obj',
             'deadrec_composite1.obj',
             'libdeadrec_igate.obj',
             'interval_composite1.obj',
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

IPATH=['direct/src/dcparse', 'direct/src/dcparser']
OPTS=['WITHINPANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='dcparse.cxx', obj='dcparse_dcparse.obj')
CompileLink(dll='dcparse.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

IPATH=['direct/src/heapq']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='heapq.cxx', obj='heapq_heapq.obj')
CompileLink(dll='libheapq.dll', opts=['ADVAPI', 'NSPR'], obj=[
             'heapq_heapq.obj',
             'libpandaexpress.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

#
# DIRECTORY: pandatool/src/pandatoolbase/
#

IPATH=['pandatool/src/pandatoolbase']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='pandatoolbase_composite1.cxx', obj='pandatoolbase_composite1.obj')
CompileLIB(lib='libpandatoolbase.lib', obj=['pandatoolbase_composite1.obj'])

#
# DIRECTORY: pandatool/src/converter/
#

IPATH=['pandatool/src/converter']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='somethingToEggConverter.cxx', obj='converter_somethingToEggConverter.obj')
CompileLIB(lib='libconverter.lib', obj=['converter_somethingToEggConverter.obj'])

#
# DIRECTORY: pandatool/src/progbase/
#

IPATH=['pandatool/src/progbase']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='progbase_composite1.cxx', obj='progbase_composite1.obj')
CompileLIB(lib='libprogbase.lib', obj=['progbase_composite1.obj'])

#
# DIRECTORY: pandatool/src/eggbase/
#

IPATH=['pandatool/src/eggbase']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='eggbase_composite1.cxx', obj='eggbase_composite1.obj')
CompileLIB(lib='libeggbase.lib', obj=['eggbase_composite1.obj'])

#
# DIRECTORY: pandatool/src/bam/
#

IPATH=['pandatool/src/bam']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='bamInfo.cxx', obj='bam-info_bamInfo.obj')
CompileLink(dll='bam-info.exe', opts=['ADVAPI', 'NSPR', 'FFTW'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='bamToEgg.cxx', obj='bam2egg_bamToEgg.obj')
CompileLink(dll='bam2egg.exe', opts=['ADVAPI', 'NSPR', 'FFTW'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='eggToBam.cxx', obj='egg2bam_eggToBam.obj')
CompileLink(dll='egg2bam.exe', opts=['ADVAPI', 'NSPR', 'FFTW'], obj=[
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

IPATH=['pandatool/src/cvscopy']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='cvscopy_composite1.cxx', obj='cvscopy_composite1.obj')
CompileLIB(lib='libcvscopy.lib', obj=['cvscopy_composite1.obj'])

#
# DIRECTORY: pandatool/src/dxf/
#

IPATH=['pandatool/src/dxf']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='dxf_composite1.cxx', obj='dxf_composite1.obj')
CompileLIB(lib='libdxf.lib', obj=['dxf_composite1.obj'])

#
# DIRECTORY: pandatool/src/dxfegg/
#

IPATH=['pandatool/src/dxfegg']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='dxfToEggConverter.cxx', obj='dxfegg_dxfToEggConverter.obj')
CompileC(ipath=IPATH, opts=OPTS, src='dxfToEggLayer.cxx', obj='dxfegg_dxfToEggLayer.obj')
CompileLIB(lib='libdxfegg.lib', obj=[
             'dxfegg_dxfToEggConverter.obj',
             'dxfegg_dxfToEggLayer.obj',
])

#
# DIRECTORY: pandatool/src/dxfprogs/
#

IPATH=['pandatool/src/dxfprogs']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='dxfPoints.cxx', obj='dxf-points_dxfPoints.obj')
CompileLink(dll='dxf-points.exe', opts=['ADVAPI', 'NSPR', 'FFTW'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='dxfToEgg.cxx', obj='dxf2egg_dxfToEgg.obj')
CompileLink(dll='dxf2egg.exe', opts=['ADVAPI', 'NSPR', 'FFTW'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='eggToDXF.cxx', obj='egg2dxf_eggToDXF.obj')
CompileC(ipath=IPATH, opts=OPTS, src='eggToDXFLayer.cxx', obj='egg2dxf_eggToDXFLayer.obj')
CompileLink(dll='egg2dxf.exe', opts=['ADVAPI', 'NSPR', 'FFTW'], obj=[
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

IPATH=['pandatool/src/palettizer']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='palettizer_composite1.cxx', obj='palettizer_composite1.obj')
CompileLIB(lib='libpalettizer.lib', obj=['palettizer_composite1.obj'])

#
# DIRECTORY: pandatool/src/egg-mkfont/
#

if OMIT.count("FREETYPE")==0:
    IPATH=['pandatool/src/egg-mkfont', 'pandatool/src/palettizer']
    OPTS=['NSPR', 'FREETYPE']
    CompileC(ipath=IPATH, opts=OPTS, src='eggMakeFont.cxx', obj='egg-mkfont_eggMakeFont.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='rangeDescription.cxx', obj='egg-mkfont_rangeDescription.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='rangeIterator.cxx', obj='egg-mkfont_rangeIterator.obj')
    CompileLink(dll='egg-mkfont.exe', opts=['ADVAPI', 'NSPR', 'FREETYPE'], obj=[
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

IPATH=['pandatool/src/eggcharbase']
OPTS=['ZLIB', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='eggcharbase_composite1.cxx', obj='eggcharbase_composite1.obj')
CompileLIB(lib='libeggcharbase.lib', obj=['eggcharbase_composite1.obj'])

#
# DIRECTORY: pandatool/src/egg-optchar/
#

IPATH=['pandatool/src/egg-optchar']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='config_egg_optchar.cxx', obj='egg-optchar_config_egg_optchar.obj')
CompileC(ipath=IPATH, opts=OPTS, src='eggOptchar.cxx', obj='egg-optchar_eggOptchar.obj')
CompileC(ipath=IPATH, opts=OPTS, src='eggOptcharUserData.cxx', obj='egg-optchar_eggOptcharUserData.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vertexMembership.cxx', obj='egg-optchar_vertexMembership.obj')
CompileLink(dll='egg-optchar.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

IPATH=['pandatool/src/egg-palettize', 'pandatool/src/palettizer']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='eggPalettize.cxx', obj='egg-palettize_eggPalettize.obj')
CompileLink(dll='egg-palettize.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

IPATH=['pandatool/src/egg-qtess']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='egg-qtess_composite1.cxx', obj='egg-qtess_composite1.obj')
CompileLink(dll='egg-qtess.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

IPATH=['pandatool/src/eggprogs']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='eggCrop.cxx', obj='egg-crop_eggCrop.obj')
CompileLink(dll='egg-crop.exe', opts=['ADVAPI', 'NSPR'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='eggMakeTube.cxx', obj='egg-make-tube_eggMakeTube.obj')
CompileLink(dll='egg-make-tube.exe', opts=['ADVAPI', 'NSPR'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='eggTextureCards.cxx', obj='egg-texture-cards_eggTextureCards.obj')
CompileLink(dll='egg-texture-cards.exe', opts=['ADVAPI', 'NSPR'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='eggTopstrip.cxx', obj='egg-topstrip_eggTopstrip.obj')
CompileLink(dll='egg-topstrip.exe', opts=['ADVAPI', 'NSPR'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='eggTrans.cxx', obj='egg-trans_eggTrans.obj')
CompileLink(dll='egg-trans.exe', opts=['ADVAPI', 'NSPR'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='eggToC.cxx', obj='egg2c_eggToC.obj')
CompileLink(dll='egg2c.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

IPATH=['pandatool/src/flt']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='fltVectorRecord.cxx', obj='flt_fltVectorRecord.obj')
CompileC(ipath=IPATH, opts=OPTS, src='flt_composite1.cxx', obj='flt_composite1.obj')
CompileLIB(lib='libflt.lib', obj=['flt_fltVectorRecord.obj', 'flt_composite1.obj'])

#
# DIRECTORY: pandatool/src/fltegg/
#

IPATH=['pandatool/src/fltegg']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='fltToEggConverter.cxx', obj='fltegg_fltToEggConverter.obj')
CompileC(ipath=IPATH, opts=OPTS, src='fltToEggLevelState.cxx', obj='fltegg_fltToEggLevelState.obj')
CompileLIB(lib='libfltegg.lib', obj=['fltegg_fltToEggConverter.obj', 'fltegg_fltToEggLevelState.obj'])

#
# DIRECTORY: pandatool/src/fltprogs/
#

IPATH=['pandatool/src/fltprogs', 'pandatool/src/flt', 'pandatool/src/cvscopy']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='eggToFlt.cxx', obj='egg2flt_eggToFlt.obj')
CompileLink(dll='egg2flt.exe', opts=['ADVAPI', 'NSPR'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='fltInfo.cxx', obj='flt-info_fltInfo.obj')
CompileLink(dll='flt-info.exe', opts=['ADVAPI', 'NSPR'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='fltTrans.cxx', obj='flt-trans_fltTrans.obj')
CompileLink(dll='flt-trans.exe', opts=['ADVAPI', 'NSPR'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='fltToEgg.cxx', obj='flt2egg_fltToEgg.obj')
CompileLink(dll='flt2egg.exe', opts=['ADVAPI', 'NSPR'], obj=[
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
CompileC(ipath=IPATH, opts=OPTS, src='fltCopy.cxx', obj='fltcopy_fltCopy.obj')
CompileLink(dll='fltcopy.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

IPATH=['pandatool/src/imagebase']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='imagebase_composite1.cxx', obj='imagebase_composite1.obj')
CompileLIB(lib='libimagebase.lib', obj=['imagebase_composite1.obj'])

#
# DIRECTORY: pandatool/src/imageprogs/
#

IPATH=['pandatool/src/imageprogs']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='imageInfo.cxx', obj='image-info_imageInfo.obj')
CompileLink(dll='image-info.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

CompileC(ipath=IPATH, opts=OPTS, src='imageResize.cxx', obj='image-resize_imageResize.obj')
CompileLink(dll='image-resize.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

CompileC(ipath=IPATH, opts=OPTS, src='imageTrans.cxx', obj='image-trans_imageTrans.obj')
CompileLink(dll='image-trans.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

IPATH=['pandatool/src/lwo']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='lwo_composite1.cxx', obj='lwo_composite1.obj')
CompileLIB(lib='liblwo.lib', obj=['lwo_composite1.obj'])

#
# DIRECTORY: pandatool/src/lwoegg/
#

IPATH=['pandatool/src/lwoegg']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='lwoegg_composite1.cxx', obj='lwoegg_composite1.obj')
CompileLIB(lib='liblwoegg.lib', obj=['lwoegg_composite1.obj'])

#
# DIRECTORY: pandatool/src/lwoprogs/
#

IPATH=['pandatool/src/lwoprogs', 'pandatool/src/lwo']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='lwoScan.cxx', obj='lwo-scan_lwoScan.obj')
CompileLink(dll='lwo-scan.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

CompileC(ipath=IPATH, opts=OPTS, src='lwoToEgg.cxx', obj='lwo2egg_lwoToEgg.obj')
CompileLink(dll='lwo2egg.exe', opts=['ADVAPI', 'NSPR'], obj=[
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
  if (OMIT.count("MAYA"+VER)==0):
    IPATH=['pandatool/src/maya']
    OPTS=['MAYA'+VER, 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='maya_composite1.cxx',    obj='maya'+VER+'_composite1.obj')
    CompileLIB(lib='libmaya'+VER+'.lib', obj=[ 'maya'+VER+'_composite1.obj' ])

#
# DIRECTORY: pandatool/src/mayaegg/
#

for VER in ["5","6","65"]:
  if (OMIT.count("MAYA"+VER)==0):
    IPATH=['pandatool/src/mayaegg', 'pandatool/src/maya']
    OPTS=['MAYA'+VER, 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='mayaegg_composite1.cxx',   obj='mayaegg'+VER+'_composite1.obj')
    CompileLIB(lib='libmayaegg'+VER+'.lib', obj=[ 'mayaegg'+VER+'_composite1.obj' ])

#
# DIRECTORY: pandatool/src/maxegg/
#

for VER in ["5", "6", "7"]:
  if (OMIT.count("MAX"+VER)==0):
    IPATH=['pandatool/src/maxegg']
    OPTS=['MAX'+VER, 'NSPR', "WINCOMCTL", "WINCOMDLG", "WINUSER", "MAXEGGDEF"]
    CompileRES(ipath=IPATH, opts=OPTS, src='MaxEgg.rc', obj='maxegg'+VER+'_MaxEgg.res')
    CompileC(ipath=IPATH, opts=OPTS, src='maxegg_composite1.cxx',obj='maxegg'+VER+'_composite1.obj')
    CompileLink(opts=OPTS, dll='maxegg'+VER+'.dlo', obj=[
                'maxegg'+VER+'_composite1.obj',
                'maxegg'+VER+'_MaxEgg.res',
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
# DIRECTORY: pandatool/src/vrml/
#

CompileBison(pre='vrmlyy', dstc='vrmlParser.cxx', dsth='vrmlParser.h', src='pandatool/src/vrml/vrmlParser.yxx')
CompileFlex(pre='vrmlyy', dst='vrmlLexer.cxx', src='pandatool/src/vrml/vrmlLexer.lxx', dashi=0)
IPATH=['pandatool/src/vrml']
OPTS=['ZLIB', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='parse_vrml.cxx', obj='pvrml_parse_vrml.obj')
CompileC(ipath=IPATH, opts=OPTS, src='standard_nodes.cxx', obj='pvrml_standard_nodes.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrmlNode.cxx', obj='pvrml_vrmlNode.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrmlNodeType.cxx', obj='pvrml_vrmlNodeType.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrmlParser.cxx', obj='pvrml_vrmlParser.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrmlLexer.cxx', obj='pvrml_vrmlLexer.obj')
CompileLIB(lib='libpvrml.lib', obj=[
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

IPATH=['pandatool/src/vrmlegg', 'pandatool/src/vrml']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='indexedFaceSet.cxx', obj='vrmlegg_indexedFaceSet.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrmlAppearance.cxx', obj='vrmlegg_vrmlAppearance.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrmlToEggConverter.cxx', obj='vrmlegg_vrmlToEggConverter.obj')
CompileLIB(lib='libvrmlegg.lib', obj=[
             'vrmlegg_indexedFaceSet.obj',
             'vrmlegg_vrmlAppearance.obj',
             'vrmlegg_vrmlToEggConverter.obj',
])

#
# DIRECTORY: pandatool/src/xfile/
#

CompileBison(pre='xyy', dstc='xParser.cxx', dsth='xParser.h', src='pandatool/src/xfile/xParser.yxx')
CompileFlex(pre='xyy', dst='xLexer.cxx', src='pandatool/src/xfile/xLexer.lxx', dashi=1)
IPATH=['pandatool/src/xfile']
OPTS=['ZLIB', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='xfile_composite1.cxx', obj='xfile_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='xParser.cxx', obj='xfile_xParser.obj')
CompileC(ipath=IPATH, opts=OPTS, src='xLexer.cxx', obj='xfile_xLexer.obj')
CompileLIB(lib='libxfile.lib', obj=[
             'xfile_composite1.obj',
             'xfile_xParser.obj',
             'xfile_xLexer.obj',
])

#
# DIRECTORY: pandatool/src/xfileegg/
#

IPATH=['pandatool/src/xfileegg', 'pandatool/src/xfile']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='xfileegg_composite1.cxx', obj='xfileegg_composite1.obj')
CompileLIB(lib='libxfileegg.lib', obj=[
             'xfileegg_composite1.obj',
])

#
# DIRECTORY: pandatool/src/ptloader/
#

IPATH=['pandatool/src/ptloader', 'pandatool/src/flt', 'pandatool/src/lwo', 'pandatool/src/xfile', 'pandatool/src/xfileegg']
OPTS=['BUILDING_PTLOADER', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='config_ptloader.cxx', obj='ptloader_config_ptloader.obj')
CompileC(ipath=IPATH, opts=OPTS, src='loaderFileTypePandatool.cxx', obj='ptloader_loaderFileTypePandatool.obj')
CompileLink(dll='libptloader.dll', opts=['ADVAPI', 'NSPR'], obj=[
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
  if (OMIT.count('MAYA'+VER)==0):
    IPATH=['pandatool/src/mayaprogs', 'pandatool/src/maya', 'pandatool/src/mayaegg',
           'pandatool/src/cvscopy']
    OPTS=['BUILDING_MISC', 'MAYA'+VER, 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='config_mayaloader.cxx', obj='mayaloader'+VER+'_config_mayaloader.obj')
    CompileLink(dll='libmayaloader'+VER+'.dll',                 opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
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
    CompileC(ipath=IPATH, opts=OPTS, src='mayaPview.cxx', obj='mayapview'+VER+'_mayaPview.obj')
    CompileLink(dll='libmayapview'+VER+'.mll', opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
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
    IPATH=['pandatool/src/mayaprogs', 'pandatool/src/maya', 'pandatool/src/mayaegg',
           'pandatool/src/cvscopy']
    OPTS=['MAYA'+VER, 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='mayaSavePview.cxx', obj='mayasavepview'+VER+'_mayaSavePview.obj')
    CompileLink(dll='libmayasavepview'+VER+'.mll', opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
                 'mayasavepview'+VER+'_mayaSavePview.obj',
    ])
    CompileC(ipath=IPATH, opts=OPTS, src='mayaToEgg.cxx', obj='maya2egg'+VER+'_mayaToEgg.obj')
    CompileLink(dll='maya2egg'+VER+'.exe',                 opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
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
    CompileC(ipath=IPATH, opts=OPTS, src='mayaCopy.cxx', obj='mayacopy'+VER+'_mayaCopy.obj')
    CompileLink(dll='mayacopy'+VER+'.exe',  opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
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

#
# DIRECTORY: pandatool/src/miscprogs/
#

IPATH=['pandatool/src/miscprogs']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='binToC.cxx', obj='bin2c_binToC.obj')
CompileLink(dll='bin2c.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

if OMIT.count("NSPR")==0:
    IPATH=['pandatool/src/pstatserver']
    OPTS=['NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='pstatserver_composite1.cxx', obj='pstatserver_composite1.obj')
    CompileLIB(lib='libpstatserver.lib', obj=[ 'pstatserver_composite1.obj' ])

#
# DIRECTORY: pandatool/src/softprogs/
#

IPATH=['pandatool/src/softprogs']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='softCVS.cxx', obj='softcvs_softCVS.obj')
CompileC(ipath=IPATH, opts=OPTS, src='softFilename.cxx', obj='softcvs_softFilename.obj')
CompileLink(opts=['ADVAPI', 'NSPR'], dll='softcvs.exe', obj=[
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

if OMIT.count("NSPR")==0:
    IPATH=['pandatool/src/text-stats']
    OPTS=['NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='textMonitor.cxx', obj='text-stats_textMonitor.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='textStats.cxx', obj='text-stats_textStats.obj')
    CompileLink(opts=['ADVAPI', 'NSPR'], dll='text-stats.exe', obj=[
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

IPATH=['pandatool/src/vrmlprogs', 'pandatool/src/vrml', 'pandatool/src/vrmlegg']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='vrmlTrans.cxx', obj='vrml-trans_vrmlTrans.obj')
CompileLink(opts=['ADVAPI', 'NSPR'], dll='vrml-trans.exe', obj=[
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

CompileC(ipath=IPATH, opts=OPTS, src='vrmlToEgg.cxx', obj='vrml2egg_vrmlToEgg.obj')
CompileLink(opts=['ADVAPI', 'NSPR'], dll='vrml2egg.exe', obj=[
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

if (OMIT.count("NSPR")==0) and (sys.platform == "win32" or sys.platform == "cygwin"):
    IPATH=['pandatool/src/win-stats']
    OPTS=['NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='winstats_composite1.cxx', obj='pstats_composite1.obj')
    CompileLink(opts=['WINSOCK', 'WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM', 'NSPR'],
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

IPATH=['pandatool/src/xfileprogs', 'pandatool/src/xfile', 'pandatool/src/xfileegg']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='eggToX.cxx', obj='egg2x_eggToX.obj')
CompileLink(dll='egg2x.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

CompileC(ipath=IPATH, opts=OPTS, src='xFileTrans.cxx', obj='x-trans_xFileTrans.obj')
CompileLink(dll='x-trans.exe', opts=['ADVAPI', 'NSPR'], obj=[
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

CompileC(ipath=IPATH, opts=OPTS, src='xFileToEgg.cxx', obj='x2egg_xFileToEgg.obj')
CompileLink(opts=['ADVAPI', 'NSPR'], dll='x2egg.exe', obj=[
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

IPATH=['pandaapp/src/pandaappbase']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='pandaappbase.cxx', obj='pandaappbase_pandaappbase.obj')
CompileLIB(lib='libpandaappbase.lib', obj=['pandaappbase_pandaappbase.obj'])

#
# DIRECTORY: pandaapp/src/httpbackup/
#

if OMIT.count("SSL")==0:
    IPATH=['pandaapp/src/httpbackup', 'pandaapp/src/pandaappbase']
    OPTS=['SSL', 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='backupCatalog.cxx', obj='httpbackup_backupCatalog.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='httpBackup.cxx', obj='httpbackup_httpBackup.obj')
    CompileLink(opts=['ADVAPI', 'NSPR', 'SSL'], dll='httpbackup.exe', obj=[
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

if OMIT.count("FREETYPE")==0:
    IPATH=['pandaapp/src/indexify']
    OPTS=['NSPR', 'FREETYPE']
    CompileC(ipath=IPATH, opts=OPTS, src='default_font.cxx', obj='font-samples_default_font.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='fontSamples.cxx', obj='font-samples_fontSamples.obj')
    CompileLink(opts=['ADVAPI', 'NSPR', 'FREETYPE'], dll='font-samples.exe', obj=[
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

    CompileC(ipath=IPATH, opts=OPTS, src='default_index_icons.cxx', obj='indexify_default_index_icons.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='default_font.cxx', obj='indexify_default_font.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='indexImage.cxx', obj='indexify_indexImage.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='indexParameters.cxx', obj='indexify_indexParameters.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='indexify.cxx', obj='indexify_indexify.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='photo.cxx', obj='indexify_photo.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='rollDirectory.cxx', obj='indexify_rollDirectory.obj')
    CompileLink(opts=['ADVAPI', 'NSPR', 'FREETYPE'], dll='indexify.exe', obj=[
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

CompileBison(pre='stitchyy', dstc='stitchParser.cxx', dsth='stitchParser.h', src='pandaapp/src/stitchbase/stitchParser.yxx')
CompileFlex(pre='stitchyy', dst='stitchLexer.cxx', src='pandaapp/src/stitchbase/stitchLexer.lxx', dashi=1)
IPATH=['pandaapp/src/stitchbase', 'pandaapp/src/pandaappbase']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='stitchbase_composite1.cxx', obj='stitchbase_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchbase_composite2.cxx', obj='stitchbase_composite2.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchParser.cxx', obj='stitchbase_stitchParser.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchLexer.cxx', obj='stitchbase_stitchLexer.obj')
CompileLIB(lib='libstitchbase.lib', obj=[
             'stitchbase_composite1.obj',
             'stitchbase_composite2.obj',
             'stitchbase_stitchParser.obj',
             'stitchbase_stitchLexer.obj',
])

#
# DIRECTORY: pandaapp/src/stitch/
#

IPATH=['pandaapp/src/stitch', 'pandaapp/src/stitchbase', 'pandaapp/src/pandaappbase']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='stitchCommandProgram.cxx', obj='stitch-command_stitchCommandProgram.obj')
CompileLink(opts=['ADVAPI', 'NSPR', 'FFTW'], dll='stitch-command.exe', obj=[
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

CompileC(ipath=IPATH, opts=OPTS, src='stitchImageProgram.cxx', obj='stitch-image_stitchImageProgram.obj')
CompileLink(opts=['ADVAPI', 'NSPR', 'FFTW'], dll='stitch-image.exe', obj=[
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

##########################################################################################
#
# Copy Sounds, Icons, and Models into the build.
#
##########################################################################################

MakeDirectory(PREFIX+"/models")
MakeDirectory(PREFIX+"/models/audio")
MakeDirectory(PREFIX+"/models/audio/sfx")
MakeDirectory(PREFIX+"/models/icons")
MakeDirectory(PREFIX+"/models/maps")
MakeDirectory(PREFIX+"/models/misc")
MakeDirectory(PREFIX+"/models/gui")

CopyAllFiles(PREFIX+"/models/audio/sfx/",  "dmodels/src/audio/sfx/", ".wav")
CopyAllFiles(PREFIX+"/models/icons/",      "dmodels/src/icons/",     ".gif")

CopyAllFiles(PREFIX+"/models/",            "models/",                ".egg")
CopyAllFiles(PREFIX+"/models/",            "models/",                ".bam")

CopyAllFiles(PREFIX+"/models/maps/",       "models/maps/",           ".jpg")
CopyAllFiles(PREFIX+"/models/maps/",       "models/maps/",           ".png")
CopyAllFiles(PREFIX+"/models/maps/",       "models/maps/",           ".rgb")
CopyAllFiles(PREFIX+"/models/maps/",       "models/maps/",           ".rgba")

CopyAllFiles(PREFIX+"/models/maps/",       "dmodels/src/maps/",      ".jpg")
CopyAllFiles(PREFIX+"/models/maps/",       "dmodels/src/maps/",      ".png")
CopyAllFiles(PREFIX+"/models/maps/",       "dmodels/src/maps/",      ".rgb")
CopyAllFiles(PREFIX+"/models/maps/",       "dmodels/src/maps/",      ".rgba")

CompileBAM("../=", PREFIX+"/models/gui/dialog_box_gui.bam",  "dmodels/src/gui/dialog_box_gui.flt")

CompileBAM("../=", PREFIX+"/models/misc/camera.bam",         "dmodels/src/misc/camera.flt")
CompileBAM("../=", PREFIX+"/models/misc/fade.bam",           "dmodels/src/misc/fade.flt")
CompileBAM("../=", PREFIX+"/models/misc/fade_sphere.bam",    "dmodels/src/misc/fade_sphere.flt")
CompileBAM("../=", PREFIX+"/models/misc/gridBack.bam",       "dmodels/src/misc/gridBack.flt")
CompileBAM("../=", PREFIX+"/models/misc/iris.bam",           "dmodels/src/misc/iris.flt")
CompileBAM("../=", PREFIX+"/models/misc/lilsmiley.bam",      "dmodels/src/misc/lilsmiley.egg")
CompileBAM("../=", PREFIX+"/models/misc/objectHandles.bam",  "dmodels/src/misc/objectHandles.flt")
CompileBAM("../=", PREFIX+"/models/misc/rgbCube.bam",        "dmodels/src/misc/rgbCube.flt")
CompileBAM("../=", PREFIX+"/models/misc/smiley.bam",         "dmodels/src/misc/smiley.egg")
CompileBAM("../=", PREFIX+"/models/misc/sphere.bam",         "dmodels/src/misc/sphere.flt")
CompileBAM("../=", PREFIX+"/models/misc/xyzAxis.bam",        "dmodels/src/misc/xyzAxis.flt")
CompileBAM("../=", PREFIX+"/models/misc/Pointlight.bam",     "dmodels/src/misc/Pointlight.egg")
CompileBAM("../=", PREFIX+"/models/misc/Dirlight.bam",       "dmodels/src/misc/Dirlight.egg")
CompileBAM("../=", PREFIX+"/models/misc/Spotlight.bam",      "dmodels/src/misc/Spotlight.egg")

##########################################################################################
#
# Run genpycode
#
##########################################################################################

if (older(PREFIX+'/pandac/PandaModules.pyz',xpaths(PREFIX+"/pandac/input/",ALLIN,""))):
    ALLTARGETS.append(PREFIX+'/pandac/PandaModules.pyz')
    if (sys.platform=="win32" or sys.platform == "cygwin"):
        if (GENMAN): oscmd(PREFIX+"/bin/genpycode.exe -m")
        else       : oscmd(PREFIX+"/bin/genpycode.exe")
    else:
        if (GENMAN): oscmd(PREFIX+"/bin/genpycode -m")
        else       : oscmd(PREFIX+"/bin/genpycode")
    updatefiledate(PREFIX+'/pandac/PandaModules.pyz')

########################################################################
##
## Save the CXX include-cache for next time.
##
########################################################################

try: icache = open(iCachePath,'wb')
except: icache = 0
if (icache!=0):
    cPickle.dump(CxxIncludeCache, icache, 1)
    icache.close()

##########################################################################################
#
# 'Complete' mode.
#
# Copies the samples and direct into the build. Note that
# this isn't usually what you want.  It is usually better to let the
# compiled panda load this stuff directly from the source tree.
# The only time you really want to do this is if you plan to move
# the build somewhere and leave the source tree behind.
#
##########################################################################################

if (COMPLETE):
    CopyFile(PREFIX+'/', 'InstallerNotes')
    CopyFile(PREFIX+'/', 'LICENSE')
    CopyFile(PREFIX+'/', 'README')
    CopyTree(PREFIX+'/samples', 'samples')
    CopyTree(PREFIX+'/direct/src', 'direct/src')
    CopyTree(PREFIX+'/SceneEditor', 'SceneEditor')

##########################################################################################
#
# The Installers
#
##########################################################################################

if (sys.platform == "win32" or sys.platform == "cygwin"):

    def MakeInstaller(file,fullname,smdirectory,uninstallkey,installdir,ppgame):
        if (older(file, ALLTARGETS)):
            print "Building "+fullname+" installer. This can take up to an hour."
            if (COMPRESSOR != "lzma"):
                print("Note: you are using zlib, which is faster, but lzma gives better compression.")
            if (os.path.exists(file)):
                os.remove(file)
            if (os.path.exists("nsis-output.exe")):
                os.remove("nsis-output.exe")
            def0 = '/DCOMPRESSOR="'   + COMPRESSOR   + '" '
            def1 = '/DFULLNAME="'     + fullname     + '" '
            def2 = '/DSMDIRECTORY="'  + smdirectory  + '" '
            def3 = '/DUNINSTALLKEY="' + uninstallkey + '" '
            def4 = '/DINSTALLDIR="'   + installdir   + '" '
            def5 = ''
            if (ppgame): def5 = '/DPPGAME="' + ppgame + '" '
            oscmd("thirdparty/win-nsis/makensis.exe /V2 "+def0+def1+def2+def3+def4+def5+" makepanda/panda.nsi")
            os.rename("nsis-output.exe", file)

    if (INSTALLER!=0):
        MakeInstaller("Panda3D-"+VERSION+".exe", "Panda3D", "Panda3D "+VERSION,
                      "Panda3D "+VERSION, "C:\\Panda3D-"+VERSION, 0)

    if (PPGAME!=0):
        if (os.path.isdir(PPGAME)==0):
            sys.exit("No such directory "+PPGAME)
        if (os.path.exists(os.path.join(PPGAME,PPGAME+".py"))==0):
            sys.exit("No such file "+PPGAME+"/"+PPGAME+".py")
        MakeInstaller(PPGAME+"-"+VERSION+".exe", PPGAME, PPGAME+" "+VERSION,
                      PPGAME+" "+VERSION, "C:\\"+PPGAME+"-"+VERSION, PPGAME)


##########################################################################################
#
# Print final status report.
#
##########################################################################################

WARNINGS.append("Elapsed Time: "+prettyTime(time.time() - STARTTIME))
printStatus("Makepanda Final Status Report", WARNINGS)


