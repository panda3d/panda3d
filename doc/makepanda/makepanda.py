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
    if (FileDateCache.has_key(path)):
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
    if (type(files) == str):
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
    return (filedate(file)<youngest(others))

def xpaths(prefix,base,suffix):
    if (type(base) == str):
        return prefix + base + suffix
    result = []
    for x in base:
        result.append(xpaths(prefix,x,suffix))
    return result

if (sys.platform == "win32"):
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
    if (cd != "."):
        print "cd "+cd+" ; "+cmd
        base=os.getcwd()
        os.chdir(cd)
    else:
        print cmd
    sys.stdout.flush()
    if (sys.platform == "win32"):
        exe = cmd.split()[0]
        if (os.path.isfile(exe)==0):
            for i in os.environ["PATH"].split(";"):
                if os.path.isfile(os.path.join(i, exe)):
                    exe = os.path.join(i, exe)
                    break
            if (os.path.isfile(exe)==0):
                sys.exit("Cannot find "+exe+" on search path")
        res = os.spawnl(os.P_WAIT, exe, cmd)
    else: res = os.system(cmd)
    if (res != 0):
        sys.exit(1)
    if (cd != "."):
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

########################################################################
##
## MakeDirectory
##
## Make a directory in the build tree
##
########################################################################

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

if (sys.platform == "win32"): COMPILERS=["MSVC7"]
if (sys.platform == "linux2"): COMPILERS=["LINUXA"]
PREFIX="built"
COMPILER=COMPILERS[0]
OPTIMIZE="3"
INSTALLER=0
PPGAME=0
THIRDPARTY="thirdparty"
VERSION="0.0.0"
VERBOSE=1
COMPRESSOR="zlib"
PACKAGES=["ZLIB","PNG","JPEG","TIFF","VRPN","FMOD","NVIDIACG","HELIX","NSPR",
          "SSL","FREETYPE","FFTW","MILES","MAYA5","MAYA6","MAX5","MAX6","MAX7"]
OMIT=PACKAGES[:]
WARNINGS=[]
DIRECTXSDK = None
MAYASDK = {}
MAXSDK = {}
MAXSDKCS = {}
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
if (sys.platform == "win32"):
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
    sys.exit(1)

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
    print "  --prefix X        (install into prefix dir, default \"built\")"
    print "  --compiler X      (currently, compiler can only be MSVC7,LINUXA)"
    print "  --optimize X      (optimization level can be 1,2,3,4)"
    print "  --thirdparty X    (directory containing third-party software)"
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
    global PREFIX,COMPILER,OPTIMIZE,OMIT,THIRDPARTY,INSTALLER,PPGAME
    global COPYEXTRAS,VERSION,COMPRESSOR,DIRECTXSDK,VERBOSE
    longopts = [
        "help","package-info","prefix=","compiler=","directx-sdk=","thirdparty=",
        "optimize=","everything","nothing","installer","ppgame=","quiet","verbose",
        "version=","lzma"]
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
            elif (option=="--ppgame"): PPGAME=value
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
                sys.exit("The registry does not appear to contain a pointer to the DirectX 9.0 SDK.")
    DIRECTXSDK=DIRECTXSDK.replace("\\", "/").rstrip("/")

########################################################################
##
## Locate the Maya 5.0 and Maya 6.0 SDK
##
########################################################################

for ver in ["MAYA5","MAYA6"]:
    if (OMIT.count(ver)==0) and (MAYASDK.has_key(ver)==0):
        if (sys.platform == "win32"):
            MAYASDK[ver]=GetRegistryKey("SOFTWARE\\Alias|Wavefront\\Maya\\5.0\\Setup\\InstallPath","MAYA_INSTALL_LOCATION")
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
## Locate Visual Studio 7.0 or 7.1
##
## The visual studio compiler doesn't work unless you set up a
## couple of environment variables to point at the compiler.
##
########################################################################

if (COMPILER == "MSVC7"):
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

if (sys.platform != "win32"):
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
# Add the package selections to DTOOLCONFIG
#
##########################################################################################

for x in PACKAGES:
    if (OMIT.count(x)==0):
        if (DTOOLCONFIG.has_key("HAVE_"+x)):
            DTOOLCONFIG["HAVE_"+x] = '1'

##########################################################################################
#
# Verify that LD_LIBRARY_PATH contains the PREFIX/lib directory.
#
# If not, add it on a temporary basis, and issue a warning.
#
##########################################################################################

if (sys.platform != "win32"):
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
        if (sys.platform == "win32"):
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
    except: contents=0
    if (contents != desiredcontents):
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
            print "Copying "+srcfile+" -> "+dstfile+"..."
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

def CopyAllFiles(dstdir,srcdir,suffix=""):
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
    if (COMPILER=="MSVC7"): cmd = 'xcopy.exe /I/Y/E/Q "' + srcdir + '" "' + dstdir + '"'
    if (COMPILER=="LINUXA"): cmd = 'cp --recursive --force ' + srcdir + ' ' + dstdir
    oscmd(cmd)
    updatefiledate(dstdir)

def CompileBison(pre,dstc,dsth,src):
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
        if (COMPILER=="LINUXA"):
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
        if (COMPILER=="MSVC7"):
            flexFullPath=os.path.abspath("thirdparty/win-util/flex.exe")
            if (dashi): oslocalcmd(PREFIX+"/tmp", flexFullPath+" -i -P" + pre + " -olex.yy.c " + fn)
            else:       oslocalcmd(PREFIX+"/tmp", flexFullPath+"    -P" + pre + " -olex.yy.c " + fn)
            replaceInFile(PREFIX+'/tmp/lex.yy.c', dst, '#include <unistd.h>', '')
        if (COMPILER=="LINUXA"):
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

    if (COMPILER=="MSVC7"):
        wobj = PREFIX+"/tmp/"+obj
        if (older(wobj, dep)):
            if VERBOSE >= 0:
                checkIfNewDir(ipath[1])
            cmd = "cl.exe /Fo" + wobj + " /nologo /c"
            cmd = cmd + " /I" + PREFIX + "/python/include"
            if (opts.count("DXSDK")): cmd = cmd + ' /I"' + DIRECTXSDK + '/include"'
            if (opts.count("MAYA5")): cmd = cmd + ' /I"' + MAYASDK["MAYA5"] + '/include"'
            if (opts.count("MAYA6")): cmd = cmd + ' /I"' + MAYASDK["MAYA6"] + '/include"'
            for max in ["MAX5","MAX6","MAX7"]:
                if (PkgSelected(opts,max)):
                    cmd = cmd + ' /I"' + MAXSDK[max] + '/include" /I"' + MAXSDKCS[max] + '" /D' + max
            for pkg in PACKAGES:
                if (pkg != "MAYA5") and (pkg != "MAYA6") and PkgSelected(opts,pkg):
                    cmd = cmd + " /I" + THIRDPARTY + "/win-libs-vc7/" + pkg.lower() + "/include"
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

    if (COMPILER=="LINUXA"):
        wobj = PREFIX+"/tmp/" + obj[:-4] + ".o"
        if (older(wobj, dep)):
            if VERBOSE >= 0:
                checkIfNewDir(ipath[1])
            if (src[-2:]==".c"): cmd = 'gcc -c -o ' + wobj
            else:                cmd = 'g++ -ftemplate-depth-30 -c -o ' + wobj
            cmd = cmd + ' -I"' + PythonSDK + '"'
            if (PkgSelected(opts,"VRPN")):     cmd = cmd + ' -I' + THIRDPARTY + '/linux-libs-a/vrpn/include'
            if (PkgSelected(opts,"FFTW")):     cmd = cmd + ' -I' + THIRDPARTY + '/linux-libs-a/fftw/include'
            if (PkgSelected(opts,"FMOD")):     cmd = cmd + ' -I' + THIRDPARTY + '/linux-libs-a/fmod/include'
            if (PkgSelected(opts,"NVIDIACG")): cmd = cmd + ' -I' + THIRDPARTY + '/linux-libs-a/nvidiacg/include'
            if (PkgSelected(opts,"NSPR")):     cmd = cmd + ' -I' + THIRDPARTY + '/linux-libs-a/nspr/include'
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

    if (COMPILER=="MSVC7"):
        if (older(obj, wdep)):
            cmd = 'rc.exe /d "NDEBUG" /l 0x409'
            for x in ipath: cmd = cmd + " /I" + x
            cmd = cmd + ' /fo' + obj
            cmd = cmd + ' ' + fullsrc
            oscmd(cmd)
            updatefiledate(obj)

    if (COMPILER=="LINUXA"):
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
        if (COMPILER=="LINUXA"):
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
        if ((COMPILER=="MSVC7") and opts.count("DXSDK")): cmd = cmd + ' -I"' + DIRECTXSDK + '/include"'
        if ((COMPILER=="MSVC7") and opts.count("MAYA5")): cmd = cmd + ' -I"' + MAYASDK["MAYA5"] + '/include"'
        if ((COMPILER=="MSVC7") and opts.count("MAYA6")): cmd = cmd + ' -I"' + MAYASDK["MAYA6"] + '/include"'
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
        if (COMPILER=="MSVC7"):
                cmd = PREFIX + '/bin/interrogate_module.exe '
        if (COMPILER=="LINUXA"):
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

    if (COMPILER=="MSVC7"):
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

    if (COMPILER=="LINUXA"):
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

    if (COMPILER=="MSVC7"):
        ALLTARGETS.append(PREFIX+"/bin/"+dll)
        lib = PREFIX+"/lib/"+dll[:-4]+".lib"
        dll = PREFIX+"/bin/"+dll
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
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/nspr/lib/libnspr4.lib'
            if (PkgSelected(opts,"SSL")):
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/ssl/lib/ssleay32.lib'
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/ssl/lib/libeay32.lib'
            if (PkgSelected(opts,"FREETYPE")):
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/freetype/lib/libfreetype.lib'
            if (PkgSelected(opts,"FFTW")):
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/fftw/lib/rfftw.lib'
                cmd = cmd + ' ' + THIRDPARTY + '/win-libs-vc7/fftw/lib/fftw.lib'
            for maya in ["MAYA5","MAYA6"]:
                if (PkgSelected(opts,maya)):
                    cmd = cmd + ' "' + MAYASDK[maya] +  '/lib/Foundation.lib"'
                    cmd = cmd + ' "' + MAYASDK[maya] +  '/lib/OpenMaya.lib"'
                    cmd = cmd + ' "' + MAYASDK[maya] +  '/lib/OpenMayaAnim.lib"'
            for max in ["MAX5","MAX6","MAX7"]:
                if PkgSelected(opts,max):
                    cmd = cmd + ' "' + MAXSDK[max] +  '/lib/core.lib"'
                    cmd = cmd + ' "' + MAXSDK[max] +  '/lib/mesh.lib"'
                    cmd = cmd + ' "' + MAXSDK[max] +  '/lib/maxutil.lib"'
                    cmd = cmd + ' "' + MAXSDK[max] +  '/lib/paramblk2.lib"'
            oscmd(cmd)
            updatefiledate(dll)
            if ((OPTIMIZE == 1) and (dll[-4:]==".dll")):
                CopyFile(dll[:-4]+"_d.dll", dll)

    if (COMPILER=="LINUXA"):
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
            if (PkgSelected(opts,"NSPR")):     cmd = cmd + ' -L' + THIRDPARTY + '/linux-libs-a/nspr/lib -lpandanspr4'
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
    if (sys.platform != "win32"): dotexe = ""
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

ConditionalWriteFile(PREFIX + "/tmp/pythonversion", os.path.basename(PythonSDK))

##########################################################################################
#
# If running under windows, compile up the icon.
#
##########################################################################################

if (sys.platform == "win32"):
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

if (sys.platform != "win32"):
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
        if (COMPILER == "MSVC7"):
            if (os.path.exists(THIRDPARTY+"/win-libs-vc7/"+pkg.lower()+"/bin")):
                CopyAllFiles(PREFIX+"/bin/",THIRDPARTY+"/win-libs-vc7/"+pkg.lower()+"/bin/")
        if (COMPILER == "LINUXA"):
            if (os.path.exists(THIRDPARTY+"/linux-libs-a/"+pkg.lower()+"/lib")):
                CopyAllFiles(PREFIX+"/lib/",THIRDPARTY+"/linux-libs-a/"+pkg.lower()+"/lib/")

if (sys.platform == "win32"):
    CopyTree(PREFIX+'/python',         'thirdparty/win-python')
    CopyFile(PREFIX+'/bin/',           'thirdparty/win-python/python22.dll')

########################################################################
##
## Copy various stuff into the build.
##
########################################################################

CopyFile(PREFIX+"/", "doc/LICENSE")
CopyFile(PREFIX+"/", "doc/ReleaseNotes")
CopyFile(PREFIX+"/", "doc/InstallerNotes")
CopyTree(PREFIX+'/pmw', 'thirdparty/pmw')
CopyTree(PREFIX+'/SceneEditor', 'SceneEditor')

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

CopyAllFiles(PREFIX+'/include/parser-inc/','dtool/src/parser-inc/')
CopyAllFiles(PREFIX+'/include/parser-inc/openssl/','dtool/src/parser-inc/')
CopyFile(PREFIX+'/include/parser-inc/Cg/','dtool/src/parser-inc/cg.h')
CopyFile(PREFIX+'/include/parser-inc/Cg/','dtool/src/parser-inc/cgGL.h')

CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/cmath.I')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/cmath.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/dallocator.T')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/dallocator.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/dtoolbase.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/dtoolbase_cc.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/dtoolsymbols.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/fakestringstream.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/nearly_zero.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/stl_compares.I')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/stl_compares.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/pallocator.T')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/pallocator.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/pdeque.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/plist.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/pmap.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/pset.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolbase/pvector.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/executionEnvironment.I')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/executionEnvironment.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/filename.I')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/filename.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/load_dso.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/dSearchPath.I')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/dSearchPath.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/pfstream.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/pfstream.I')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/vector_string.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/gnu_getopt.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/pfstreamBuf.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/vector_src.cxx')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/vector_src.h')
CopyFile(PREFIX+'/include/','dtool/src/dtoolutil/pandaSystem.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/config_prc.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configDeclaration.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configDeclaration.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configFlags.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configFlags.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configPage.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configPage.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configPageManager.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configPageManager.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariable.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariable.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableBase.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableBase.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableBool.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableBool.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableCore.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableCore.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableDouble.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableDouble.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableEnum.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableEnum.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableFilename.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableFilename.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableInt.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableInt.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableList.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableList.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableManager.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableManager.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableSearchPath.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableSearchPath.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableString.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/configVariableString.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/globPattern.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/globPattern.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/notify.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/notify.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/notifyCategory.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/notifyCategory.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/notifyCategoryProxy.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/notifyCategoryProxy.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/notifySeverity.h')
CopyFile(PREFIX+'/include/','dtool/src/prc/prcKeyRegistry.I')
CopyFile(PREFIX+'/include/','dtool/src/prc/prcKeyRegistry.h')
CopyFile(PREFIX+'/include/','dtool/src/dconfig/configTable.I')
CopyFile(PREFIX+'/include/','dtool/src/dconfig/configTable.h')
CopyFile(PREFIX+'/include/','dtool/src/dconfig/config_dconfig.h')
CopyFile(PREFIX+'/include/','dtool/src/dconfig/config_setup.h')
CopyFile(PREFIX+'/include/','dtool/src/dconfig/dconfig.I')
CopyFile(PREFIX+'/include/','dtool/src/dconfig/dconfig.h')
CopyFile(PREFIX+'/include/','dtool/src/dconfig/serialization.I')
CopyFile(PREFIX+'/include/','dtool/src/dconfig/serialization.h')
CopyFile(PREFIX+'/include/','dtool/src/dconfig/symbolEnt.I')
CopyFile(PREFIX+'/include/','dtool/src/dconfig/symbolEnt.h')
CopyFile(PREFIX+'/include/','dtool/src/interrogatedb/interrogate_interface.h')
CopyFile(PREFIX+'/include/','dtool/src/interrogatedb/interrogate_request.h')
CopyFile(PREFIX+'/include/','dtool/src/interrogatedb/vector_int.h')
CopyFile(PREFIX+'/include/','dtool/src/interrogatedb/config_interrogatedb.h')
CopyFile(PREFIX+'/include/','dtool/src/pystub/pystub.h')
CopyFile(PREFIX+'/include/','dtool/src/prckeys/signPrcFile_src.cxx')
CopyFile(PREFIX+'/include/','panda/src/pandabase/pandabase.h')
CopyFile(PREFIX+'/include/','panda/src/pandabase/pandasymbols.h')
CopyFile(PREFIX+'/include/','panda/src/express/atomicAdjustDummyImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/atomicAdjustDummyImpl.I')
CopyFile(PREFIX+'/include/','panda/src/express/atomicAdjust.h')
CopyFile(PREFIX+'/include/','panda/src/express/atomicAdjust.I')
CopyFile(PREFIX+'/include/','panda/src/express/atomicAdjustImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/atomicAdjustNsprImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/atomicAdjustNsprImpl.I')
CopyFile(PREFIX+'/include/','panda/src/express/bigEndian.h')
CopyFile(PREFIX+'/include/','panda/src/express/buffer.I')
CopyFile(PREFIX+'/include/','panda/src/express/buffer.h')
CopyFile(PREFIX+'/include/','panda/src/express/checksumHashGenerator.I')
CopyFile(PREFIX+'/include/','panda/src/express/checksumHashGenerator.h')
CopyFile(PREFIX+'/include/','panda/src/express/circBuffer.I')
CopyFile(PREFIX+'/include/','panda/src/express/circBuffer.h')
CopyFile(PREFIX+'/include/','panda/src/express/clockObject.I')
CopyFile(PREFIX+'/include/','panda/src/express/clockObject.h')
CopyFile(PREFIX+'/include/','panda/src/express/conditionVarDummyImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/conditionVarDummyImpl.I')
CopyFile(PREFIX+'/include/','panda/src/express/conditionVar.h')
CopyFile(PREFIX+'/include/','panda/src/express/conditionVar.I')
CopyFile(PREFIX+'/include/','panda/src/express/conditionVarImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/conditionVarNsprImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/conditionVarNsprImpl.I')
CopyFile(PREFIX+'/include/','panda/src/express/config_express.h')
CopyFile(PREFIX+'/include/','panda/src/express/datagram.I')
CopyFile(PREFIX+'/include/','panda/src/express/datagram.h')
CopyFile(PREFIX+'/include/','panda/src/express/datagramGenerator.I')
CopyFile(PREFIX+'/include/','panda/src/express/datagramGenerator.h')
CopyFile(PREFIX+'/include/','panda/src/express/datagramIterator.I')
CopyFile(PREFIX+'/include/','panda/src/express/datagramIterator.h')
CopyFile(PREFIX+'/include/','panda/src/express/datagramSink.I')
CopyFile(PREFIX+'/include/','panda/src/express/datagramSink.h')
CopyFile(PREFIX+'/include/','panda/src/express/dcast.T')
CopyFile(PREFIX+'/include/','panda/src/express/dcast.h')
CopyFile(PREFIX+'/include/','panda/src/express/encryptStreamBuf.h')
CopyFile(PREFIX+'/include/','panda/src/express/encryptStreamBuf.I')
CopyFile(PREFIX+'/include/','panda/src/express/encryptStream.h')
CopyFile(PREFIX+'/include/','panda/src/express/encryptStream.I')
CopyFile(PREFIX+'/include/','panda/src/express/error_utils.h')
CopyFile(PREFIX+'/include/','panda/src/express/hashGeneratorBase.I')
CopyFile(PREFIX+'/include/','panda/src/express/hashGeneratorBase.h')
CopyFile(PREFIX+'/include/','panda/src/express/hashVal.I')
CopyFile(PREFIX+'/include/','panda/src/express/hashVal.h')
CopyFile(PREFIX+'/include/','panda/src/express/indent.I')
CopyFile(PREFIX+'/include/','panda/src/express/indent.h')
CopyFile(PREFIX+'/include/','panda/src/express/indirectLess.I')
CopyFile(PREFIX+'/include/','panda/src/express/indirectLess.h')
CopyFile(PREFIX+'/include/','panda/src/express/littleEndian.h')
CopyFile(PREFIX+'/include/','panda/src/express/memoryInfo.I')
CopyFile(PREFIX+'/include/','panda/src/express/memoryInfo.h')
CopyFile(PREFIX+'/include/','panda/src/express/memoryUsage.I')
CopyFile(PREFIX+'/include/','panda/src/express/memoryUsage.h')
CopyFile(PREFIX+'/include/','panda/src/express/memoryUsagePointerCounts.I')
CopyFile(PREFIX+'/include/','panda/src/express/memoryUsagePointerCounts.h')
CopyFile(PREFIX+'/include/','panda/src/express/memoryUsagePointers.I')
CopyFile(PREFIX+'/include/','panda/src/express/memoryUsagePointers.h')
CopyFile(PREFIX+'/include/','panda/src/express/multifile.I')
CopyFile(PREFIX+'/include/','panda/src/express/multifile.h')
CopyFile(PREFIX+'/include/','panda/src/express/mutexDummyImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/mutexDummyImpl.I')
CopyFile(PREFIX+'/include/','panda/src/express/pmutex.h')
CopyFile(PREFIX+'/include/','panda/src/express/mutexHolder.h')
CopyFile(PREFIX+'/include/','panda/src/express/mutexHolder.I')
CopyFile(PREFIX+'/include/','panda/src/express/pmutex.I')
CopyFile(PREFIX+'/include/','panda/src/express/mutexImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/mutexNsprImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/mutexNsprImpl.I')
CopyFile(PREFIX+'/include/','panda/src/express/namable.I')
CopyFile(PREFIX+'/include/','panda/src/express/namable.h')
CopyFile(PREFIX+'/include/','panda/src/express/nativeNumericData.I')
CopyFile(PREFIX+'/include/','panda/src/express/nativeNumericData.h')
CopyFile(PREFIX+'/include/','panda/src/express/numeric_types.h')
CopyFile(PREFIX+'/include/','panda/src/express/ordered_vector.h')
CopyFile(PREFIX+'/include/','panda/src/express/ordered_vector.I')
CopyFile(PREFIX+'/include/','panda/src/express/ordered_vector.T')
CopyFile(PREFIX+'/include/','panda/src/express/password_hash.h')
CopyFile(PREFIX+'/include/','panda/src/express/patchfile.I')
CopyFile(PREFIX+'/include/','panda/src/express/patchfile.h')
CopyFile(PREFIX+'/include/','panda/src/express/pointerTo.I')
CopyFile(PREFIX+'/include/','panda/src/express/pointerTo.h')
CopyFile(PREFIX+'/include/','panda/src/express/pointerToArray.I')
CopyFile(PREFIX+'/include/','panda/src/express/pointerToArray.h')
CopyFile(PREFIX+'/include/','panda/src/express/pointerToBase.I')
CopyFile(PREFIX+'/include/','panda/src/express/pointerToBase.h')
CopyFile(PREFIX+'/include/','panda/src/express/pointerToVoid.I')
CopyFile(PREFIX+'/include/','panda/src/express/pointerToVoid.h')
CopyFile(PREFIX+'/include/','panda/src/express/profileTimer.I')
CopyFile(PREFIX+'/include/','panda/src/express/profileTimer.h')
CopyFile(PREFIX+'/include/','panda/src/express/pta_uchar.h')
CopyFile(PREFIX+'/include/','panda/src/express/ramfile.I')
CopyFile(PREFIX+'/include/','panda/src/express/ramfile.h')
CopyFile(PREFIX+'/include/','panda/src/express/referenceCount.I')
CopyFile(PREFIX+'/include/','panda/src/express/referenceCount.h')
CopyFile(PREFIX+'/include/','panda/src/express/register_type.I')
CopyFile(PREFIX+'/include/','panda/src/express/register_type.h')
CopyFile(PREFIX+'/include/','panda/src/express/reversedNumericData.I')
CopyFile(PREFIX+'/include/','panda/src/express/reversedNumericData.h')
CopyFile(PREFIX+'/include/','panda/src/express/selectThreadImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/streamReader.I')
CopyFile(PREFIX+'/include/','panda/src/express/streamReader.h')
CopyFile(PREFIX+'/include/','panda/src/express/streamWriter.I')
CopyFile(PREFIX+'/include/','panda/src/express/streamWriter.h')
CopyFile(PREFIX+'/include/','panda/src/express/stringDecoder.h')
CopyFile(PREFIX+'/include/','panda/src/express/stringDecoder.I')
CopyFile(PREFIX+'/include/','panda/src/express/subStream.I')
CopyFile(PREFIX+'/include/','panda/src/express/subStream.h')
CopyFile(PREFIX+'/include/','panda/src/express/subStreamBuf.h')
CopyFile(PREFIX+'/include/','panda/src/express/textEncoder.h')
CopyFile(PREFIX+'/include/','panda/src/express/textEncoder.I')
CopyFile(PREFIX+'/include/','panda/src/express/threadDummyImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/threadDummyImpl.I')
CopyFile(PREFIX+'/include/','panda/src/express/thread.h')
CopyFile(PREFIX+'/include/','panda/src/express/thread.I')
CopyFile(PREFIX+'/include/','panda/src/express/threadImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/threadNsprImpl.h')
CopyFile(PREFIX+'/include/','panda/src/express/threadNsprImpl.I')
CopyFile(PREFIX+'/include/','panda/src/express/threadPriority.h')
CopyFile(PREFIX+'/include/','panda/src/express/tokenBoard.I')
CopyFile(PREFIX+'/include/','panda/src/express/tokenBoard.h')
CopyFile(PREFIX+'/include/','panda/src/express/trueClock.I')
CopyFile(PREFIX+'/include/','panda/src/express/trueClock.h')
CopyFile(PREFIX+'/include/','panda/src/express/typeHandle.I')
CopyFile(PREFIX+'/include/','panda/src/express/typeHandle.h')
CopyFile(PREFIX+'/include/','panda/src/express/typedObject.I')
CopyFile(PREFIX+'/include/','panda/src/express/typedObject.h')
CopyFile(PREFIX+'/include/','panda/src/express/typedReferenceCount.I')
CopyFile(PREFIX+'/include/','panda/src/express/typedReferenceCount.h')
CopyFile(PREFIX+'/include/','panda/src/express/typedef.h')
CopyFile(PREFIX+'/include/','panda/src/express/typeRegistry.I')
CopyFile(PREFIX+'/include/','panda/src/express/typeRegistry.h')
CopyFile(PREFIX+'/include/','panda/src/express/typeRegistryNode.I')
CopyFile(PREFIX+'/include/','panda/src/express/typeRegistryNode.h')
CopyFile(PREFIX+'/include/','panda/src/express/unicodeLatinMap.h')
CopyFile(PREFIX+'/include/','panda/src/express/vector_uchar.h')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileComposite.h')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileComposite.I')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFile.h')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFile.I')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileList.I')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileList.h')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileMount.h')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileMount.I')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileMountMultifile.h')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileMountMultifile.I')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileMountSystem.h')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileMountSystem.I')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileSimple.h')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileSimple.I')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileSystem.h')
CopyFile(PREFIX+'/include/','panda/src/express/virtualFileSystem.I')
CopyFile(PREFIX+'/include/','panda/src/express/weakPointerTo.I')
CopyFile(PREFIX+'/include/','panda/src/express/weakPointerTo.h')
CopyFile(PREFIX+'/include/','panda/src/express/weakPointerToBase.I')
CopyFile(PREFIX+'/include/','panda/src/express/weakPointerToBase.h')
CopyFile(PREFIX+'/include/','panda/src/express/weakPointerToVoid.I')
CopyFile(PREFIX+'/include/','panda/src/express/weakPointerToVoid.h')
CopyFile(PREFIX+'/include/','panda/src/express/weakReferenceList.I')
CopyFile(PREFIX+'/include/','panda/src/express/weakReferenceList.h')
CopyFile(PREFIX+'/include/','panda/src/express/windowsRegistry.h')
CopyFile(PREFIX+'/include/','panda/src/express/zStream.I')
CopyFile(PREFIX+'/include/','panda/src/express/zStream.h')
CopyFile(PREFIX+'/include/','panda/src/express/zStreamBuf.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/asyncUtility.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/asyncUtility.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/bioPtr.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/bioPtr.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/bioStreamPtr.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/bioStreamPtr.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/bioStream.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/bioStream.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/bioStreamBuf.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/chunkedStream.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/chunkedStream.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/chunkedStreamBuf.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/config_downloader.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/decompressor.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/decompressor.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/documentSpec.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/documentSpec.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/download_utils.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/downloadDb.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/downloadDb.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/extractor.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpAuthorization.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpAuthorization.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpBasicAuthorization.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpBasicAuthorization.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpChannel.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpChannel.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpClient.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpClient.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpCookie.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpCookie.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpDate.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpDate.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpDigestAuthorization.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpDigestAuthorization.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpEntityTag.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpEntityTag.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/httpEnum.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/identityStream.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/identityStream.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/identityStreamBuf.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/multiplexStream.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/multiplexStream.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/multiplexStreamBuf.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/multiplexStreamBuf.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/patcher.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/patcher.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/socketStream.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/socketStream.I')
CopyFile(PREFIX+'/include/','panda/src/downloader/ssl_utils.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/urlSpec.h')
CopyFile(PREFIX+'/include/','panda/src/downloader/urlSpec.I')
CopyFile(PREFIX+'/include/','panda/src/putil/bam.h')
CopyFile(PREFIX+'/include/','panda/src/putil/bamReader.I')
CopyFile(PREFIX+'/include/','panda/src/putil/bamReader.h')
CopyFile(PREFIX+'/include/','panda/src/putil/bamReaderParam.I')
CopyFile(PREFIX+'/include/','panda/src/putil/bamReaderParam.h')
CopyFile(PREFIX+'/include/','panda/src/putil/bamWriter.I')
CopyFile(PREFIX+'/include/','panda/src/putil/bamWriter.h')
CopyFile(PREFIX+'/include/','panda/src/putil/bitMask.I')
CopyFile(PREFIX+'/include/','panda/src/putil/bitMask.h')
CopyFile(PREFIX+'/include/','panda/src/putil/buttonHandle.I')
CopyFile(PREFIX+'/include/','panda/src/putil/buttonHandle.h')
CopyFile(PREFIX+'/include/','panda/src/putil/buttonRegistry.I')
CopyFile(PREFIX+'/include/','panda/src/putil/buttonRegistry.h')
CopyFile(PREFIX+'/include/','panda/src/putil/collideMask.h')
CopyFile(PREFIX+'/include/','panda/src/putil/portalMask.h')
CopyFile(PREFIX+'/include/','panda/src/putil/compareTo.I')
CopyFile(PREFIX+'/include/','panda/src/putil/compareTo.h')
CopyFile(PREFIX+'/include/','panda/src/putil/config_util.h')
CopyFile(PREFIX+'/include/','panda/src/putil/configurable.h')
CopyFile(PREFIX+'/include/','panda/src/putil/factory.I')
CopyFile(PREFIX+'/include/','panda/src/putil/factory.h')
CopyFile(PREFIX+'/include/','panda/src/putil/cachedTypedWritableReferenceCount.h')
CopyFile(PREFIX+'/include/','panda/src/putil/cachedTypedWritableReferenceCount.I')
CopyFile(PREFIX+'/include/','panda/src/putil/cycleData.h')
CopyFile(PREFIX+'/include/','panda/src/putil/cycleData.I')
CopyFile(PREFIX+'/include/','panda/src/putil/cycleDataReader.h')
CopyFile(PREFIX+'/include/','panda/src/putil/cycleDataReader.I')
CopyFile(PREFIX+'/include/','panda/src/putil/cycleDataWriter.h')
CopyFile(PREFIX+'/include/','panda/src/putil/cycleDataWriter.I')
CopyFile(PREFIX+'/include/','panda/src/putil/datagramInputFile.I')
CopyFile(PREFIX+'/include/','panda/src/putil/datagramInputFile.h')
CopyFile(PREFIX+'/include/','panda/src/putil/datagramOutputFile.I')
CopyFile(PREFIX+'/include/','panda/src/putil/datagramOutputFile.h')
CopyFile(PREFIX+'/include/','panda/src/putil/drawMask.h')
CopyFile(PREFIX+'/include/','panda/src/putil/factoryBase.I')
CopyFile(PREFIX+'/include/','panda/src/putil/factoryBase.h')
CopyFile(PREFIX+'/include/','panda/src/putil/factoryParam.I')
CopyFile(PREFIX+'/include/','panda/src/putil/factoryParam.h')
CopyFile(PREFIX+'/include/','panda/src/putil/factoryParams.I')
CopyFile(PREFIX+'/include/','panda/src/putil/factoryParams.h')
CopyFile(PREFIX+'/include/','panda/src/putil/firstOfPairCompare.I')
CopyFile(PREFIX+'/include/','panda/src/putil/firstOfPairCompare.h')
CopyFile(PREFIX+'/include/','panda/src/putil/firstOfPairLess.I')
CopyFile(PREFIX+'/include/','panda/src/putil/firstOfPairLess.h')
CopyFile(PREFIX+'/include/','panda/src/putil/globalPointerRegistry.I')
CopyFile(PREFIX+'/include/','panda/src/putil/globalPointerRegistry.h')
CopyFile(PREFIX+'/include/','panda/src/putil/indirectCompareNames.I')
CopyFile(PREFIX+'/include/','panda/src/putil/indirectCompareNames.h')
CopyFile(PREFIX+'/include/','panda/src/putil/indirectCompareTo.I')
CopyFile(PREFIX+'/include/','panda/src/putil/indirectCompareTo.h')
CopyFile(PREFIX+'/include/','panda/src/putil/ioPtaDatagramFloat.h')
CopyFile(PREFIX+'/include/','panda/src/putil/ioPtaDatagramInt.h')
CopyFile(PREFIX+'/include/','panda/src/putil/ioPtaDatagramShort.h')
CopyFile(PREFIX+'/include/','panda/src/putil/iterator_types.h')
CopyFile(PREFIX+'/include/','panda/src/putil/keyboardButton.h')
CopyFile(PREFIX+'/include/','panda/src/putil/lineStream.I')
CopyFile(PREFIX+'/include/','panda/src/putil/lineStream.h')
CopyFile(PREFIX+'/include/','panda/src/putil/lineStreamBuf.I')
CopyFile(PREFIX+'/include/','panda/src/putil/lineStreamBuf.h')
CopyFile(PREFIX+'/include/','panda/src/putil/load_prc_file.h')
CopyFile(PREFIX+'/include/','panda/src/putil/modifierButtons.I')
CopyFile(PREFIX+'/include/','panda/src/putil/modifierButtons.h')
CopyFile(PREFIX+'/include/','panda/src/putil/mouseButton.h')
CopyFile(PREFIX+'/include/','panda/src/putil/mouseData.I')
CopyFile(PREFIX+'/include/','panda/src/putil/mouseData.h')
CopyFile(PREFIX+'/include/','panda/src/putil/nameUniquifier.I')
CopyFile(PREFIX+'/include/','panda/src/putil/nameUniquifier.h')
CopyFile(PREFIX+'/include/','panda/src/putil/pipeline.h')
CopyFile(PREFIX+'/include/','panda/src/putil/pipeline.I')
CopyFile(PREFIX+'/include/','panda/src/putil/pipelineCycler.h')
CopyFile(PREFIX+'/include/','panda/src/putil/pipelineCycler.I')
CopyFile(PREFIX+'/include/','panda/src/putil/pipelineCyclerBase.h')
CopyFile(PREFIX+'/include/','panda/src/putil/pipelineCyclerBase.I')
CopyFile(PREFIX+'/include/','panda/src/putil/pta_double.h')
CopyFile(PREFIX+'/include/','panda/src/putil/pta_float.h')
CopyFile(PREFIX+'/include/','panda/src/putil/pta_int.h')
CopyFile(PREFIX+'/include/','panda/src/putil/pta_ushort.h')
CopyFile(PREFIX+'/include/','panda/src/putil/string_utils.I')
CopyFile(PREFIX+'/include/','panda/src/putil/string_utils.h')
CopyFile(PREFIX+'/include/','panda/src/putil/timedCycle.I')
CopyFile(PREFIX+'/include/','panda/src/putil/timedCycle.h')
CopyFile(PREFIX+'/include/','panda/src/putil/typedWritable.I')
CopyFile(PREFIX+'/include/','panda/src/putil/typedWritable.h')
CopyFile(PREFIX+'/include/','panda/src/putil/typedWritableReferenceCount.I')
CopyFile(PREFIX+'/include/','panda/src/putil/typedWritableReferenceCount.h')
CopyFile(PREFIX+'/include/','panda/src/putil/updateSeq.I')
CopyFile(PREFIX+'/include/','panda/src/putil/updateSeq.h')
CopyFile(PREFIX+'/include/','panda/src/putil/uniqueIdAllocator.h')
CopyFile(PREFIX+'/include/','panda/src/putil/vector_double.h')
CopyFile(PREFIX+'/include/','panda/src/putil/vector_float.h')
CopyFile(PREFIX+'/include/','panda/src/putil/vector_typedWritable.h')
CopyFile(PREFIX+'/include/','panda/src/putil/vector_ushort.h')
CopyFile(PREFIX+'/include/','panda/src/putil/vector_writable.h')
CopyFile(PREFIX+'/include/','panda/src/putil/writableConfigurable.h')
CopyFile(PREFIX+'/include/','panda/src/putil/writableParam.I')
CopyFile(PREFIX+'/include/','panda/src/putil/writableParam.h')
CopyFile(PREFIX+'/include/','panda/src/audio/config_audio.h')
CopyFile(PREFIX+'/include/','panda/src/audio/audio.h')
CopyFile(PREFIX+'/include/','panda/src/audio/audioManager.h')
CopyFile(PREFIX+'/include/','panda/src/audio/audioSound.h')
CopyFile(PREFIX+'/include/','panda/src/audio/nullAudioManager.h')
CopyFile(PREFIX+'/include/','panda/src/audio/nullAudioSound.h')
CopyFile(PREFIX+'/include/','panda/src/event/buttonEvent.I')
CopyFile(PREFIX+'/include/','panda/src/event/buttonEvent.h')
CopyFile(PREFIX+'/include/','panda/src/event/buttonEventList.I')
CopyFile(PREFIX+'/include/','panda/src/event/buttonEventList.h')
CopyFile(PREFIX+'/include/','panda/src/event/event.I')
CopyFile(PREFIX+'/include/','panda/src/event/event.h')
CopyFile(PREFIX+'/include/','panda/src/event/eventHandler.h')
CopyFile(PREFIX+'/include/','panda/src/event/eventHandler.I')
CopyFile(PREFIX+'/include/','panda/src/event/eventParameter.I')
CopyFile(PREFIX+'/include/','panda/src/event/eventParameter.h')
CopyFile(PREFIX+'/include/','panda/src/event/eventQueue.I')
CopyFile(PREFIX+'/include/','panda/src/event/eventQueue.h')
CopyFile(PREFIX+'/include/','panda/src/event/eventReceiver.h')
CopyFile(PREFIX+'/include/','panda/src/event/pt_Event.h')
CopyFile(PREFIX+'/include/','panda/src/event/throw_event.I')
CopyFile(PREFIX+'/include/','panda/src/event/throw_event.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/compose_matrix.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/compose_matrix_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/compose_matrix_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/config_linmath.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/coordinateSystem.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/dbl2fltnames.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/dblnames.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/deg_2_rad.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/flt2dblnames.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/fltnames.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/ioPtaDatagramLinMath.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/ioPtaDatagramLinMath.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lcast_to.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lcast_to_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lcast_to_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lmat_ops.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lmat_ops_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lmat_ops_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lmatrix.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lmatrix3.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lmatrix3_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lmatrix3_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lmatrix4.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lmatrix4_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lmatrix4_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lorientation.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lorientation_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lorientation_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lpoint2.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lpoint2_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lpoint2_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lpoint3.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lpoint3_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lpoint3_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lpoint4.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lpoint4_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lpoint4_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lquaternion.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lquaternion_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lquaternion_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lrotation.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lrotation_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lrotation_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/luse.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/luse.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvec2_ops.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvec2_ops_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvec2_ops_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvec3_ops.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvec3_ops_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvec3_ops_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvec4_ops.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvec4_ops_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvec4_ops_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvecBase2.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvecBase2_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvecBase2_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvecBase3.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvecBase3_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvecBase3_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvecBase4.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvecBase4_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvecBase4_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvector2.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvector2_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvector2_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvector3.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvector3_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvector3_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvector4.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvector4_src.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/lvector4_src.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/mathNumbers.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/mathNumbers.I')
CopyFile(PREFIX+'/include/','panda/src/linmath/pta_Colorf.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/pta_Normalf.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/pta_TexCoordf.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/pta_Vertexf.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/vector_Colorf.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/vector_LPoint2f.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/vector_LVecBase3f.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/vector_Normalf.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/vector_TexCoordf.h')
CopyFile(PREFIX+'/include/','panda/src/linmath/vector_Vertexf.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/boundingHexahedron.I')
CopyFile(PREFIX+'/include/','panda/src/mathutil/boundingHexahedron.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/boundingLine.I')
CopyFile(PREFIX+'/include/','panda/src/mathutil/boundingLine.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/boundingSphere.I')
CopyFile(PREFIX+'/include/','panda/src/mathutil/boundingSphere.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/boundingVolume.I')
CopyFile(PREFIX+'/include/','panda/src/mathutil/boundingVolume.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/config_mathutil.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/fftCompressor.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/finiteBoundingVolume.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/frustum.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/frustum_src.I')
CopyFile(PREFIX+'/include/','panda/src/mathutil/frustum_src.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/geometricBoundingVolume.I')
CopyFile(PREFIX+'/include/','panda/src/mathutil/geometricBoundingVolume.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/look_at.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/look_at_src.I')
CopyFile(PREFIX+'/include/','panda/src/mathutil/look_at_src.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/linmath_events.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/mathHelpers.I')
CopyFile(PREFIX+'/include/','panda/src/mathutil/mathHelpers.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/omniBoundingVolume.I')
CopyFile(PREFIX+'/include/','panda/src/mathutil/omniBoundingVolume.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/plane.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/plane_src.I')
CopyFile(PREFIX+'/include/','panda/src/mathutil/plane_src.h')
CopyFile(PREFIX+'/include/','panda/src/mathutil/rotate_to.h')
CopyFile(PREFIX+'/include/','panda/src/gsgbase/graphicsStateGuardianBase.h')
CopyFile(PREFIX+'/include/','panda/src/pnmimage/config_pnmimage.h')
CopyFile(PREFIX+'/include/','panda/src/pnmimage/pnmFileType.h')
CopyFile(PREFIX+'/include/','panda/src/pnmimage/pnmFileTypeRegistry.h')
CopyFile(PREFIX+'/include/','panda/src/pnmimage/pnmImage.I')
CopyFile(PREFIX+'/include/','panda/src/pnmimage/pnmImage.h')
CopyFile(PREFIX+'/include/','panda/src/pnmimage/pnmImageHeader.I')
CopyFile(PREFIX+'/include/','panda/src/pnmimage/pnmImageHeader.h')
CopyFile(PREFIX+'/include/','panda/src/pnmimage/pnmReader.I')
CopyFile(PREFIX+'/include/','panda/src/pnmimage/pnmReader.h')
CopyFile(PREFIX+'/include/','panda/src/pnmimage/pnmWriter.I')
CopyFile(PREFIX+'/include/','panda/src/pnmimage/pnmWriter.h')
CopyFile(PREFIX+'/include/','panda/src/pnmimage/pnmimage_base.h')
CopyFile(PREFIX+'/include/','panda/src/pnmimagetypes/sgi.h')
CopyFile(PREFIX+'/include/','panda/src/net/config_net.h')
CopyFile(PREFIX+'/include/','panda/src/net/connection.h')
CopyFile(PREFIX+'/include/','panda/src/net/connectionListener.h')
CopyFile(PREFIX+'/include/','panda/src/net/connectionManager.h')
CopyFile(PREFIX+'/include/','panda/src/net/connectionReader.h')
CopyFile(PREFIX+'/include/','panda/src/net/connectionWriter.h')
CopyFile(PREFIX+'/include/','panda/src/net/datagramQueue.h')
CopyFile(PREFIX+'/include/','panda/src/net/datagramTCPHeader.I')
CopyFile(PREFIX+'/include/','panda/src/net/datagramTCPHeader.h')
CopyFile(PREFIX+'/include/','panda/src/net/datagramUDPHeader.I')
CopyFile(PREFIX+'/include/','panda/src/net/datagramUDPHeader.h')
CopyFile(PREFIX+'/include/','panda/src/net/netAddress.h')
CopyFile(PREFIX+'/include/','panda/src/net/netDatagram.I')
CopyFile(PREFIX+'/include/','panda/src/net/netDatagram.h')
CopyFile(PREFIX+'/include/','panda/src/net/pprerror.h')
CopyFile(PREFIX+'/include/','panda/src/net/queuedConnectionListener.I')
CopyFile(PREFIX+'/include/','panda/src/net/queuedConnectionListener.h')
CopyFile(PREFIX+'/include/','panda/src/net/queuedConnectionManager.h')
CopyFile(PREFIX+'/include/','panda/src/net/queuedConnectionReader.h')
CopyFile(PREFIX+'/include/','panda/src/net/queuedReturn.I')
CopyFile(PREFIX+'/include/','panda/src/net/queuedReturn.h')
CopyFile(PREFIX+'/include/','panda/src/net/recentConnectionReader.h')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/config_pstats.h')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatClient.I')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatClient.h')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatClientImpl.I')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatClientImpl.h')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatClientVersion.I')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatClientVersion.h')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatClientControlMessage.h')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatCollector.I')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatCollector.h')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatCollectorDef.h')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatFrameData.I')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatFrameData.h')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatProperties.h')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatServerControlMessage.h')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatThread.I')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatThread.h')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatTimer.I')
CopyFile(PREFIX+'/include/','panda/src/pstatclient/pStatTimer.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/boundedObject.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/boundedObject.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/config_gobj.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/drawable.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/geom.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/geom.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/textureContext.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/textureContext.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/geomLine.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/geomLinestrip.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/geomPoint.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/geomPolygon.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/geomQuad.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/geomSphere.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/geomSprite.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/geomSprite.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/geomTri.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/geomTrifan.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/geomTristrip.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/geomprimitives.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/imageBuffer.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/imageBuffer.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/material.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/material.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/materialPool.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/materialPool.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/matrixLens.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/matrixLens.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/orthographicLens.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/orthographicLens.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/perspectiveLens.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/perspectiveLens.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/pixelBuffer.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/pixelBuffer.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/preparedGraphicsObjects.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/preparedGraphicsObjects.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/lens.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/lens.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/savedContext.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/savedContext.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/texture.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/texture.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/texturePool.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/texturePool.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/texCoordName.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/texCoordName.h')
CopyFile(PREFIX+'/include/','panda/src/gobj/textureStage.I')
CopyFile(PREFIX+'/include/','panda/src/gobj/textureStage.h')
CopyFile(PREFIX+'/include/','panda/src/lerp/lerp.h')
CopyFile(PREFIX+'/include/','panda/src/lerp/lerpblend.h')
CopyFile(PREFIX+'/include/','panda/src/lerp/lerpfunctor.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/accumulatedAttribs.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/accumulatedAttribs.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/alphaTestAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/alphaTestAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/antialiasAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/antialiasAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/ambientLight.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/ambientLight.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/auxSceneData.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/auxSceneData.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/bamFile.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/bamFile.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/billboardEffect.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/billboardEffect.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/binCullHandler.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/binCullHandler.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/camera.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/camera.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/clipPlaneAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/clipPlaneAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/colorAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/colorAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/colorBlendAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/colorBlendAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/colorScaleAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/colorScaleAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/colorWriteAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/colorWriteAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/compassEffect.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/compassEffect.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/config_pgraph.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBin.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBin.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBinAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBinAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBinBackToFront.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBinBackToFront.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBinFixed.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBinFixed.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBinFrontToBack.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBinFrontToBack.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBinManager.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBinManager.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBinUnsorted.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullBinUnsorted.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullFaceAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullFaceAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullHandler.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullHandler.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullResult.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullResult.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullTraverser.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullTraverser.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullTraverserData.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullTraverserData.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullableObject.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/cullableObject.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/decalEffect.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/decalEffect.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/depthOffsetAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/depthOffsetAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/depthTestAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/depthTestAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/depthWriteAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/depthWriteAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/directionalLight.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/directionalLight.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/drawCullHandler.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/drawCullHandler.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/fadeLodNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/fadeLodNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/fadeLodNodeData.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/fog.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/fog.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/fogAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/fogAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/geomNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/geomNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/geomTransformer.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/geomTransformer.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/lensNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/lensNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/light.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/light.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/lightAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/lightAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/lightLensNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/lightLensNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/lightNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/lightNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/loader.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/loader.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/loaderFileType.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/loaderFileTypeBam.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/loaderFileTypeRegistry.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/lodNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/lodNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/materialAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/materialAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/modelNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/modelNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/modelPool.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/modelPool.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/modelRoot.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/modelRoot.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/nodePath.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/nodePath.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/nodePathCollection.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/nodePathCollection.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/nodePathComponent.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/nodePathComponent.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/nodePathLerps.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/pandaNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/pandaNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/planeNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/planeNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/pointLight.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/pointLight.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/polylightNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/polylightNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/polylightEffect.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/polylightEffect.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/portalNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/portalNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/portalClipper.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/portalClipper.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/renderAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/renderAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/renderEffect.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/renderEffect.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/renderEffects.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/renderEffects.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/renderModeAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/renderModeAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/renderState.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/renderState.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/rescaleNormalAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/rescaleNormalAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/sceneGraphAnalyzer.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/sceneGraphReducer.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/sceneGraphReducer.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/sceneSetup.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/sceneSetup.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/selectiveChildNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/selectiveChildNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/sequenceNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/sequenceNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/showBoundsEffect.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/showBoundsEffect.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/spotlight.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/spotlight.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/switchNode.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/switchNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/texMatrixAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/texMatrixAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/texProjectorEffect.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/texProjectorEffect.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/textureApplyAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/textureApplyAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/textureAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/textureAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/texGenAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/texGenAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/textureCollection.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/textureCollection.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/textureStageCollection.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/textureStageCollection.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/transformState.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/transformState.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/transparencyAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/transparencyAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/weakNodePath.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/weakNodePath.h')
CopyFile(PREFIX+'/include/','panda/src/pgraph/workingNodePath.I')
CopyFile(PREFIX+'/include/','panda/src/pgraph/workingNodePath.h')
CopyFile(PREFIX+'/include/','panda/src/chan/animBundle.I')
CopyFile(PREFIX+'/include/','panda/src/chan/animBundle.h')
CopyFile(PREFIX+'/include/','panda/src/chan/animBundleNode.I')
CopyFile(PREFIX+'/include/','panda/src/chan/animBundleNode.h')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannel.I')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannel.h')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannelBase.I')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannelBase.h')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannelFixed.I')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannelFixed.h')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannelMatrixDynamic.I')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannelMatrixDynamic.h')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannelMatrixXfmTable.I')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannelMatrixXfmTable.h')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannelScalarDynamic.I')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannelScalarDynamic.h')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannelScalarTable.I')
CopyFile(PREFIX+'/include/','panda/src/chan/animChannelScalarTable.h')
CopyFile(PREFIX+'/include/','panda/src/chan/animControl.I')
CopyFile(PREFIX+'/include/','panda/src/chan/animControl.h')
CopyFile(PREFIX+'/include/','panda/src/chan/animControlCollection.I')
CopyFile(PREFIX+'/include/','panda/src/chan/animControlCollection.h')
CopyFile(PREFIX+'/include/','panda/src/chan/animGroup.I')
CopyFile(PREFIX+'/include/','panda/src/chan/animGroup.h')
CopyFile(PREFIX+'/include/','panda/src/chan/auto_bind.h')
CopyFile(PREFIX+'/include/','panda/src/chan/config_chan.h')
CopyFile(PREFIX+'/include/','panda/src/chan/movingPart.I')
CopyFile(PREFIX+'/include/','panda/src/chan/movingPart.h')
CopyFile(PREFIX+'/include/','panda/src/chan/movingPartBase.I')
CopyFile(PREFIX+'/include/','panda/src/chan/movingPartBase.h')
CopyFile(PREFIX+'/include/','panda/src/chan/movingPartMatrix.I')
CopyFile(PREFIX+'/include/','panda/src/chan/movingPartMatrix.h')
CopyFile(PREFIX+'/include/','panda/src/chan/movingPartScalar.I')
CopyFile(PREFIX+'/include/','panda/src/chan/movingPartScalar.h')
CopyFile(PREFIX+'/include/','panda/src/chan/partBundle.I')
CopyFile(PREFIX+'/include/','panda/src/chan/partBundle.h')
CopyFile(PREFIX+'/include/','panda/src/chan/partBundleNode.I')
CopyFile(PREFIX+'/include/','panda/src/chan/partBundleNode.h')
CopyFile(PREFIX+'/include/','panda/src/chan/partGroup.I')
CopyFile(PREFIX+'/include/','panda/src/chan/partGroup.h')
CopyFile(PREFIX+'/include/','panda/src/chan/vector_PartGroupStar.h')
CopyFile(PREFIX+'/include/','panda/src/char/character.I')
CopyFile(PREFIX+'/include/','panda/src/char/character.h')
CopyFile(PREFIX+'/include/','panda/src/char/characterJoint.h')
CopyFile(PREFIX+'/include/','panda/src/char/characterJointBundle.I')
CopyFile(PREFIX+'/include/','panda/src/char/characterJointBundle.h')
CopyFile(PREFIX+'/include/','panda/src/char/characterSlider.h')
CopyFile(PREFIX+'/include/','panda/src/char/computedVertices.I')
CopyFile(PREFIX+'/include/','panda/src/char/computedVertices.h')
CopyFile(PREFIX+'/include/','panda/src/char/computedVerticesMorph.I')
CopyFile(PREFIX+'/include/','panda/src/char/computedVerticesMorph.h')
CopyFile(PREFIX+'/include/','panda/src/char/config_char.h')
CopyFile(PREFIX+'/include/','panda/src/char/dynamicVertices.h')
CopyFile(PREFIX+'/include/','panda/src/dgraph/config_dgraph.h')
CopyFile(PREFIX+'/include/','panda/src/dgraph/dataGraphTraverser.I')
CopyFile(PREFIX+'/include/','panda/src/dgraph/dataGraphTraverser.h')
CopyFile(PREFIX+'/include/','panda/src/dgraph/dataNode.I')
CopyFile(PREFIX+'/include/','panda/src/dgraph/dataNode.h')
CopyFile(PREFIX+'/include/','panda/src/dgraph/dataNodeTransmit.I')
CopyFile(PREFIX+'/include/','panda/src/dgraph/dataNodeTransmit.h')
CopyFile(PREFIX+'/include/','panda/src/display/config_display.h')
CopyFile(PREFIX+'/include/','panda/src/display/drawableRegion.I')
CopyFile(PREFIX+'/include/','panda/src/display/drawableRegion.h')
CopyFile(PREFIX+'/include/','panda/src/display/displayRegion.I')
CopyFile(PREFIX+'/include/','panda/src/display/displayRegion.h')
CopyFile(PREFIX+'/include/','panda/src/display/displayRegionStack.I')
CopyFile(PREFIX+'/include/','panda/src/display/displayRegionStack.h')
CopyFile(PREFIX+'/include/','panda/src/display/frameBufferProperties.I')
CopyFile(PREFIX+'/include/','panda/src/display/frameBufferProperties.h')
CopyFile(PREFIX+'/include/','panda/src/display/frameBufferStack.I')
CopyFile(PREFIX+'/include/','panda/src/display/frameBufferStack.h')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsEngine.I')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsEngine.h')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsOutput.I')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsOutput.h')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsBuffer.I')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsBuffer.h')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsPipe.I')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsPipe.h')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsPipeSelection.I')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsPipeSelection.h')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsStateGuardian.I')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsStateGuardian.h')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsWindow.I')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsWindow.h')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsThreadingModel.I')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsThreadingModel.h')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsWindowInputDevice.I')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsWindowInputDevice.h')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsDevice.I')
CopyFile(PREFIX+'/include/','panda/src/display/graphicsDevice.h')
CopyFile(PREFIX+'/include/','panda/src/display/parasiteBuffer.I')
CopyFile(PREFIX+'/include/','panda/src/display/parasiteBuffer.h')
CopyFile(PREFIX+'/include/','panda/src/display/windowProperties.I')
CopyFile(PREFIX+'/include/','panda/src/display/windowProperties.h')
CopyFile(PREFIX+'/include/','panda/src/display/lensStack.I')
CopyFile(PREFIX+'/include/','panda/src/display/lensStack.h')
CopyFile(PREFIX+'/include/','panda/src/display/renderBuffer.h')
CopyFile(PREFIX+'/include/','panda/src/display/savedFrameBuffer.I')
CopyFile(PREFIX+'/include/','panda/src/display/savedFrameBuffer.h')
CopyFile(PREFIX+'/include/','panda/src/device/analogNode.I')
CopyFile(PREFIX+'/include/','panda/src/device/analogNode.h')
CopyFile(PREFIX+'/include/','panda/src/device/buttonNode.I')
CopyFile(PREFIX+'/include/','panda/src/device/buttonNode.h')
CopyFile(PREFIX+'/include/','panda/src/device/clientAnalogDevice.I')
CopyFile(PREFIX+'/include/','panda/src/device/clientAnalogDevice.h')
CopyFile(PREFIX+'/include/','panda/src/device/clientBase.I')
CopyFile(PREFIX+'/include/','panda/src/device/clientBase.h')
CopyFile(PREFIX+'/include/','panda/src/device/clientButtonDevice.I')
CopyFile(PREFIX+'/include/','panda/src/device/clientButtonDevice.h')
CopyFile(PREFIX+'/include/','panda/src/device/clientDevice.I')
CopyFile(PREFIX+'/include/','panda/src/device/clientDevice.h')
CopyFile(PREFIX+'/include/','panda/src/device/clientDialDevice.I')
CopyFile(PREFIX+'/include/','panda/src/device/clientDialDevice.h')
CopyFile(PREFIX+'/include/','panda/src/device/clientTrackerDevice.I')
CopyFile(PREFIX+'/include/','panda/src/device/clientTrackerDevice.h')
CopyFile(PREFIX+'/include/','panda/src/device/config_device.h')
CopyFile(PREFIX+'/include/','panda/src/device/mouseAndKeyboard.h')
CopyFile(PREFIX+'/include/','panda/src/device/dialNode.I')
CopyFile(PREFIX+'/include/','panda/src/device/dialNode.h')
CopyFile(PREFIX+'/include/','panda/src/device/trackerData.I')
CopyFile(PREFIX+'/include/','panda/src/device/trackerData.h')
CopyFile(PREFIX+'/include/','panda/src/device/trackerNode.I')
CopyFile(PREFIX+'/include/','panda/src/device/trackerNode.h')
CopyFile(PREFIX+'/include/','panda/src/device/virtualMouse.h')
CopyFile(PREFIX+'/include/','panda/src/tform/buttonThrower.I')
CopyFile(PREFIX+'/include/','panda/src/tform/buttonThrower.h')
CopyFile(PREFIX+'/include/','panda/src/tform/driveInterface.I')
CopyFile(PREFIX+'/include/','panda/src/tform/driveInterface.h')
CopyFile(PREFIX+'/include/','panda/src/tform/mouseInterfaceNode.I')
CopyFile(PREFIX+'/include/','panda/src/tform/mouseInterfaceNode.h')
CopyFile(PREFIX+'/include/','panda/src/tform/mouseWatcher.I')
CopyFile(PREFIX+'/include/','panda/src/tform/mouseWatcher.h')
CopyFile(PREFIX+'/include/','panda/src/tform/mouseWatcherGroup.h')
CopyFile(PREFIX+'/include/','panda/src/tform/mouseWatcherParameter.I')
CopyFile(PREFIX+'/include/','panda/src/tform/mouseWatcherParameter.h')
CopyFile(PREFIX+'/include/','panda/src/tform/mouseWatcherRegion.I')
CopyFile(PREFIX+'/include/','panda/src/tform/mouseWatcherRegion.h')
CopyFile(PREFIX+'/include/','panda/src/tform/trackball.h')
CopyFile(PREFIX+'/include/','panda/src/tform/transform2sg.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionEntry.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionEntry.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionHandler.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionHandlerEvent.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionHandlerEvent.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionHandlerFloor.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionHandlerFloor.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionHandlerGravity.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionHandlerGravity.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionHandlerPhysical.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionHandlerPhysical.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionHandlerPusher.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionHandlerPusher.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionHandlerQueue.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionInvSphere.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionInvSphere.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionLevelState.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionLevelState.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionLine.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionLine.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionNode.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionNode.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionPlane.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionPlane.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionPolygon.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionPolygon.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionRay.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionRay.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionRecorder.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionRecorder.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionSegment.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionSegment.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionSolid.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionSolid.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionSphere.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionSphere.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionTraverser.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionTraverser.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionTube.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionTube.h')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionVisualizer.I')
CopyFile(PREFIX+'/include/','panda/src/collide/collisionVisualizer.h')
CopyFile(PREFIX+'/include/','panda/src/collide/config_collide.h')
CopyFile(PREFIX+'/include/','panda/src/pnmtext/config_pnmtext.h')
CopyFile(PREFIX+'/include/','panda/src/pnmtext/freetypeFont.h')
CopyFile(PREFIX+'/include/','panda/src/pnmtext/freetypeFont.I')
CopyFile(PREFIX+'/include/','panda/src/pnmtext/pnmTextGlyph.h')
CopyFile(PREFIX+'/include/','panda/src/pnmtext/pnmTextGlyph.I')
CopyFile(PREFIX+'/include/','panda/src/pnmtext/pnmTextMaker.h')
CopyFile(PREFIX+'/include/','panda/src/pnmtext/pnmTextMaker.I')
CopyFile(PREFIX+'/include/','panda/src/text/config_text.h')
CopyFile(PREFIX+'/include/','panda/src/text/dynamicTextFont.I')
CopyFile(PREFIX+'/include/','panda/src/text/dynamicTextFont.h')
CopyFile(PREFIX+'/include/','panda/src/text/dynamicTextGlyph.I')
CopyFile(PREFIX+'/include/','panda/src/text/dynamicTextGlyph.h')
CopyFile(PREFIX+'/include/','panda/src/text/dynamicTextPage.I')
CopyFile(PREFIX+'/include/','panda/src/text/dynamicTextPage.h')
CopyFile(PREFIX+'/include/','panda/src/text/fontPool.I')
CopyFile(PREFIX+'/include/','panda/src/text/fontPool.h')
CopyFile(PREFIX+'/include/','panda/src/text/geomTextGlyph.I')
CopyFile(PREFIX+'/include/','panda/src/text/geomTextGlyph.h')
CopyFile(PREFIX+'/include/','panda/src/text/staticTextFont.I')
CopyFile(PREFIX+'/include/','panda/src/text/staticTextFont.h')
CopyFile(PREFIX+'/include/','panda/src/text/textAssembler.I')
CopyFile(PREFIX+'/include/','panda/src/text/textAssembler.h')
CopyFile(PREFIX+'/include/','panda/src/text/textFont.I')
CopyFile(PREFIX+'/include/','panda/src/text/textFont.h')
CopyFile(PREFIX+'/include/','panda/src/text/textGlyph.I')
CopyFile(PREFIX+'/include/','panda/src/text/textGlyph.h')
CopyFile(PREFIX+'/include/','panda/src/text/textNode.I')
CopyFile(PREFIX+'/include/','panda/src/text/textNode.h')
CopyFile(PREFIX+'/include/','panda/src/text/textProperties.I')
CopyFile(PREFIX+'/include/','panda/src/text/textProperties.h')
CopyFile(PREFIX+'/include/','panda/src/text/textPropertiesManager.I')
CopyFile(PREFIX+'/include/','panda/src/text/textPropertiesManager.h')
CopyFile(PREFIX+'/include/','panda/src/grutil/cardMaker.I')
CopyFile(PREFIX+'/include/','panda/src/grutil/cardMaker.h')
CopyFile(PREFIX+'/include/','panda/src/grutil/frameRateMeter.I')
CopyFile(PREFIX+'/include/','panda/src/grutil/frameRateMeter.h')
CopyFile(PREFIX+'/include/','panda/src/grutil/lineSegs.I')
CopyFile(PREFIX+'/include/','panda/src/grutil/lineSegs.h')
CopyFile(PREFIX+'/include/','panda/src/grutil/multitexReducer.I')
CopyFile(PREFIX+'/include/','panda/src/grutil/multitexReducer.h')
CopyFile(PREFIX+'/include/','panda/src/gsgmisc/geomIssuer.I')
CopyFile(PREFIX+'/include/','panda/src/gsgmisc/geomIssuer.h')
CopyFile(PREFIX+'/include/','panda/src/helix/config_helix.h')
CopyFile(PREFIX+'/include/','panda/src/helix/HelixClient.h')
CopyFile(PREFIX+'/include/','panda/src/vrpn/config_vrpn.h')
CopyFile(PREFIX+'/include/','panda/src/vrpn/vrpnClient.I')
CopyFile(PREFIX+'/include/','panda/src/vrpn/vrpnClient.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/classicNurbsCurve.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/classicNurbsCurve.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/config_parametrics.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/cubicCurveseg.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/parametricCurveDrawer.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/parametricCurveDrawer.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/curveFitter.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/curveFitter.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/hermiteCurve.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsCurve.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsCurveDrawer.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsCurveDrawer.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsCurveEvaluator.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsCurveEvaluator.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsCurveInterface.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsCurveInterface.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsCurveResult.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsCurveResult.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsBasisVector.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsBasisVector.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsSurfaceEvaluator.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsSurfaceEvaluator.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsSurfaceResult.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsSurfaceResult.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsVertex.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsVertex.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/nurbsPPCurve.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/parametricCurve.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/parametricCurveCollection.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/parametricCurveCollection.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/piecewiseCurve.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/ropeNode.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/ropeNode.h')
CopyFile(PREFIX+'/include/','panda/src/parametrics/sheetNode.I')
CopyFile(PREFIX+'/include/','panda/src/parametrics/sheetNode.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgButton.I')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgButton.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgSliderButton.I')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgSliderButton.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgCullTraverser.I')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgCullTraverser.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgEntry.I')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgEntry.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgMouseWatcherGroup.I')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgMouseWatcherGroup.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgMouseWatcherParameter.I')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgMouseWatcherParameter.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgFrameStyle.I')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgFrameStyle.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgItem.I')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgItem.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgMouseWatcherBackground.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgMouseWatcherRegion.I')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgMouseWatcherRegion.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgTop.I')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgTop.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgWaitBar.I')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgWaitBar.h')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgSliderBar.I')
CopyFile(PREFIX+'/include/','panda/src/pgui/pgSliderBar.h')
CopyFile(PREFIX+'/include/','panda/src/pnmimagetypes/config_pnmimagetypes.h')
CopyFile(PREFIX+'/include/','panda/src/recorder/mouseRecorder.h')
CopyFile(PREFIX+'/include/','panda/src/recorder/recorderBase.h')
CopyFile(PREFIX+'/include/','panda/src/recorder/recorderBase.I')
CopyFile(PREFIX+'/include/','panda/src/recorder/recorderController.h')
CopyFile(PREFIX+'/include/','panda/src/recorder/recorderController.I')
CopyFile(PREFIX+'/include/','panda/src/recorder/recorderFrame.h')
CopyFile(PREFIX+'/include/','panda/src/recorder/recorderFrame.I')
CopyFile(PREFIX+'/include/','panda/src/recorder/recorderHeader.h')
CopyFile(PREFIX+'/include/','panda/src/recorder/recorderHeader.I')
CopyFile(PREFIX+'/include/','panda/src/recorder/recorderTable.h')
CopyFile(PREFIX+'/include/','panda/src/recorder/recorderTable.I')
CopyFile(PREFIX+'/include/','panda/src/recorder/socketStreamRecorder.h')
CopyFile(PREFIX+'/include/','panda/src/recorder/socketStreamRecorder.I')
CopyFile(PREFIX+'/include/','panda/metalibs/panda/panda.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builder.I')
CopyFile(PREFIX+'/include/','panda/src/builder/builder.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builder_compare.I')
CopyFile(PREFIX+'/include/','panda/src/builder/builder_compare.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builderAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/builder/builderAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builderAttribTempl.I')
CopyFile(PREFIX+'/include/','panda/src/builder/builderAttribTempl.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builderBucket.I')
CopyFile(PREFIX+'/include/','panda/src/builder/builderBucket.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builderBucketNode.I')
CopyFile(PREFIX+'/include/','panda/src/builder/builderBucketNode.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builderNormalVisualizer.I')
CopyFile(PREFIX+'/include/','panda/src/builder/builderNormalVisualizer.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builderPrim.I')
CopyFile(PREFIX+'/include/','panda/src/builder/builderPrim.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builderPrimTempl.I')
CopyFile(PREFIX+'/include/','panda/src/builder/builderPrimTempl.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builderProperties.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builderTypes.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builderVertex.I')
CopyFile(PREFIX+'/include/','panda/src/builder/builderVertex.h')
CopyFile(PREFIX+'/include/','panda/src/builder/builderVertexTempl.I')
CopyFile(PREFIX+'/include/','panda/src/builder/builderVertexTempl.h')
CopyFile(PREFIX+'/include/','panda/src/builder/config_builder.h')
CopyFile(PREFIX+'/include/','panda/src/windisplay/config_windisplay.h')
CopyFile(PREFIX+'/include/','panda/src/windisplay/winGraphicsPipe.I')
CopyFile(PREFIX+'/include/','panda/src/windisplay/winGraphicsPipe.h')
CopyFile(PREFIX+'/include/','panda/src/windisplay/winGraphicsWindow.I')
CopyFile(PREFIX+'/include/','panda/src/windisplay/winGraphicsWindow.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg7/config_dxgsg7.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg7/dxGraphicsStateGuardian7.I')
CopyFile(PREFIX+'/include/','panda/src/dxgsg7/dxGraphicsStateGuardian7.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg7/dxTextureContext7.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg7/dxgsg7base.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg8/dxgsg8base.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg8/config_dxgsg8.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg8/dxGraphicsStateGuardian8.I')
CopyFile(PREFIX+'/include/','panda/src/dxgsg8/dxGraphicsStateGuardian8.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg8/dxTextureContext8.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg8/d3dfont8.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg8/dxGraphicsDevice8.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg9/dxgsg9base.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg9/config_dxgsg9.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg9/dxGraphicsStateGuardian9.I')
CopyFile(PREFIX+'/include/','panda/src/dxgsg9/dxGraphicsStateGuardian9.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg9/dxTextureContext9.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg9/d3dfont9.h')
CopyFile(PREFIX+'/include/','panda/src/dxgsg9/dxGraphicsDevice9.h')
CopyFile(PREFIX+'/include/','panda/src/effects/config_effects.h')
CopyFile(PREFIX+'/include/','panda/src/effects/cgShader.I')
CopyFile(PREFIX+'/include/','panda/src/effects/cgShader.h')
CopyFile(PREFIX+'/include/','panda/src/effects/cgShaderAttrib.I')
CopyFile(PREFIX+'/include/','panda/src/effects/cgShaderAttrib.h')
CopyFile(PREFIX+'/include/','panda/src/effects/cgShaderContext.I')
CopyFile(PREFIX+'/include/','panda/src/effects/cgShaderContext.h')
CopyFile(PREFIX+'/include/','panda/src/effects/lensFlareNode.I')
CopyFile(PREFIX+'/include/','panda/src/effects/lensFlareNode.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggAnimData.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggAnimData.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggAttributes.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggAttributes.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggBin.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggBinMaker.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggComment.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggComment.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggCoordinateSystem.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggCoordinateSystem.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggCurve.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggCurve.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggData.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggData.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggExternalReference.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggExternalReference.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggFilenameNode.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggFilenameNode.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggGroup.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggGroup.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggGroupNode.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggGroupNode.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggGroupUniquifier.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggLine.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggLine.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggMaterial.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggMaterial.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggMaterialCollection.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggMaterialCollection.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggMorph.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggMorph.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggMorphList.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggMorphList.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggNamedObject.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggNamedObject.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggNameUniquifier.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggNode.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggNode.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggNurbsCurve.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggNurbsCurve.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggNurbsSurface.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggNurbsSurface.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggObject.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggObject.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggParameters.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggPoint.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggPoint.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggPolygon.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggPolygon.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggPolysetMaker.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggPoolUniquifier.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggPrimitive.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggPrimitive.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggRenderMode.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggRenderMode.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggSAnimData.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggSAnimData.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggSurface.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggSurface.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggSwitchCondition.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggTable.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggTable.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggTexture.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggTexture.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggTextureCollection.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggTextureCollection.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggTransform3d.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggTransform3d.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggUserData.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggUserData.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggUtilities.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggUtilities.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggVertex.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggVertex.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggVertexPool.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggVertexPool.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggVertexUV.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggVertexUV.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggXfmAnimData.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggXfmAnimData.h')
CopyFile(PREFIX+'/include/','panda/src/egg/eggXfmSAnim.I')
CopyFile(PREFIX+'/include/','panda/src/egg/eggXfmSAnim.h')
CopyFile(PREFIX+'/include/','panda/src/egg/pt_EggMaterial.h')
CopyFile(PREFIX+'/include/','panda/src/egg/vector_PT_EggMaterial.h')
CopyFile(PREFIX+'/include/','panda/src/egg/pt_EggTexture.h')
CopyFile(PREFIX+'/include/','panda/src/egg/vector_PT_EggTexture.h')
CopyFile(PREFIX+'/include/','panda/src/egg/pt_EggVertex.h')
CopyFile(PREFIX+'/include/','panda/src/egg/vector_PT_EggVertex.h')
CopyFile(PREFIX+'/include/','panda/src/egg2pg/egg_parametrics.h')
CopyFile(PREFIX+'/include/','panda/src/egg2pg/load_egg_file.h')
CopyFile(PREFIX+'/include/','panda/src/egg2pg/config_egg2pg.h')
CopyFile(PREFIX+'/include/','panda/src/framework/pandaFramework.I')
CopyFile(PREFIX+'/include/','panda/src/framework/pandaFramework.h')
CopyFile(PREFIX+'/include/','panda/src/framework/windowFramework.I')
CopyFile(PREFIX+'/include/','panda/src/framework/windowFramework.h')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glext.h')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glmisc_src.cxx')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glmisc_src.h')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glstuff_src.cxx')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glstuff_src.h')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glstuff_undef_src.h')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glGeomContext_src.cxx')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glGeomContext_src.I')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glGeomContext_src.h')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glGraphicsStateGuardian_src.cxx')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glGraphicsStateGuardian_src.I')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glGraphicsStateGuardian_src.h')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glSavedFrameBuffer_src.cxx')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glSavedFrameBuffer_src.I')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glSavedFrameBuffer_src.h')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glTextureContext_src.cxx')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glTextureContext_src.I')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glTextureContext_src.h')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glCgShaderContext_src.cxx')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glCgShaderContext_src.h')
CopyFile(PREFIX+'/include/','panda/src/glstuff/glCgShaderContext_src.I')
CopyFile(PREFIX+'/include/','panda/src/glgsg/config_glgsg.h')
CopyFile(PREFIX+'/include/','panda/src/glgsg/glgsg.h')
CopyFile(PREFIX+'/include/','panda/metalibs/pandaegg/pandaegg.h')
CopyFile(PREFIX+'/include/','panda/src/wgldisplay/config_wgldisplay.h')
CopyFile(PREFIX+'/include/','panda/src/wgldisplay/wglGraphicsBuffer.I')
CopyFile(PREFIX+'/include/','panda/src/wgldisplay/wglGraphicsBuffer.h')
CopyFile(PREFIX+'/include/','panda/src/wgldisplay/wglGraphicsPipe.I')
CopyFile(PREFIX+'/include/','panda/src/wgldisplay/wglGraphicsPipe.h')
CopyFile(PREFIX+'/include/','panda/src/wgldisplay/wglGraphicsStateGuardian.I')
CopyFile(PREFIX+'/include/','panda/src/wgldisplay/wglGraphicsStateGuardian.h')
CopyFile(PREFIX+'/include/','panda/src/wgldisplay/wglGraphicsWindow.I')
CopyFile(PREFIX+'/include/','panda/src/wgldisplay/wglGraphicsWindow.h')
CopyFile(PREFIX+'/include/','panda/src/wgldisplay/wglext.h')
CopyFile(PREFIX+'/include/','panda/metalibs/pandagl/pandagl.h')
CopyFile(PREFIX+'/include/','panda/src/physics/actorNode.I')
CopyFile(PREFIX+'/include/','panda/src/physics/actorNode.h')
CopyFile(PREFIX+'/include/','panda/src/physics/angularEulerIntegrator.h')
CopyFile(PREFIX+'/include/','panda/src/physics/angularForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/angularIntegrator.h')
CopyFile(PREFIX+'/include/','panda/src/physics/angularVectorForce.I')
CopyFile(PREFIX+'/include/','panda/src/physics/angularVectorForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/baseForce.I')
CopyFile(PREFIX+'/include/','panda/src/physics/baseForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/baseIntegrator.I')
CopyFile(PREFIX+'/include/','panda/src/physics/baseIntegrator.h')
CopyFile(PREFIX+'/include/','panda/src/physics/config_physics.h')
CopyFile(PREFIX+'/include/','panda/src/physics/forceNode.I')
CopyFile(PREFIX+'/include/','panda/src/physics/forceNode.h')
CopyFile(PREFIX+'/include/','panda/src/physics/forces.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearCylinderVortexForce.I')
CopyFile(PREFIX+'/include/','panda/src/physics/linearCylinderVortexForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearDistanceForce.I')
CopyFile(PREFIX+'/include/','panda/src/physics/linearDistanceForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearEulerIntegrator.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearForce.I')
CopyFile(PREFIX+'/include/','panda/src/physics/linearForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearFrictionForce.I')
CopyFile(PREFIX+'/include/','panda/src/physics/linearFrictionForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearIntegrator.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearJitterForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearNoiseForce.I')
CopyFile(PREFIX+'/include/','panda/src/physics/linearNoiseForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearRandomForce.I')
CopyFile(PREFIX+'/include/','panda/src/physics/linearRandomForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearSinkForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearSourceForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearUserDefinedForce.I')
CopyFile(PREFIX+'/include/','panda/src/physics/linearUserDefinedForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/linearVectorForce.I')
CopyFile(PREFIX+'/include/','panda/src/physics/linearVectorForce.h')
CopyFile(PREFIX+'/include/','panda/src/physics/physical.I')
CopyFile(PREFIX+'/include/','panda/src/physics/physical.h')
CopyFile(PREFIX+'/include/','panda/src/physics/physicalNode.I')
CopyFile(PREFIX+'/include/','panda/src/physics/physicalNode.h')
CopyFile(PREFIX+'/include/','panda/src/physics/physicsCollisionHandler.I')
CopyFile(PREFIX+'/include/','panda/src/physics/physicsCollisionHandler.h')
CopyFile(PREFIX+'/include/','panda/src/physics/physicsManager.I')
CopyFile(PREFIX+'/include/','panda/src/physics/physicsManager.h')
CopyFile(PREFIX+'/include/','panda/src/physics/physicsObject.I')
CopyFile(PREFIX+'/include/','panda/src/physics/physicsObject.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/baseParticle.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/baseParticle.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/baseParticleEmitter.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/baseParticleEmitter.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/baseParticleFactory.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/baseParticleFactory.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/baseParticleRenderer.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/baseParticleRenderer.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/boxEmitter.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/boxEmitter.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/config_particlesystem.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/discEmitter.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/discEmitter.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/emitters.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/geomParticleRenderer.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/geomParticleRenderer.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/lineEmitter.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/lineEmitter.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/lineParticleRenderer.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/lineParticleRenderer.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/particleSystem.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/particleSystem.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/particleSystemManager.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/particleSystemManager.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/particlefactories.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/particles.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/pointEmitter.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/pointEmitter.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/pointParticle.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/pointParticleFactory.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/pointParticleRenderer.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/pointParticleRenderer.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/rectangleEmitter.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/rectangleEmitter.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/ringEmitter.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/ringEmitter.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/sparkleParticleRenderer.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/sparkleParticleRenderer.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/sphereSurfaceEmitter.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/sphereSurfaceEmitter.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/sphereVolumeEmitter.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/sphereVolumeEmitter.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/spriteParticleRenderer.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/spriteParticleRenderer.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/tangentRingEmitter.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/tangentRingEmitter.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/zSpinParticle.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/zSpinParticle.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/zSpinParticleFactory.I')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/zSpinParticleFactory.h')
CopyFile(PREFIX+'/include/','panda/src/particlesystem/particleCommonFuncs.h')
CopyFile(PREFIX+'/include/','panda/metalibs/pandaphysics/pandaphysics.h')
CopyFile(PREFIX+'/include/','direct/src/directbase/directbase.h')
CopyFile(PREFIX+'/include/','direct/src/directbase/directsymbols.h')
CopyFile(PREFIX+'/include/','direct/src/deadrec/smoothMover.h')
CopyFile(PREFIX+'/include/','direct/src/deadrec/smoothMover.I')
CopyFile(PREFIX+'/include/','direct/src/interval/config_interval.h')
CopyFile(PREFIX+'/include/','direct/src/interval/cInterval.I')
CopyFile(PREFIX+'/include/','direct/src/interval/cInterval.h')
CopyFile(PREFIX+'/include/','direct/src/interval/cIntervalManager.I')
CopyFile(PREFIX+'/include/','direct/src/interval/cIntervalManager.h')
CopyFile(PREFIX+'/include/','direct/src/interval/cLerpInterval.I')
CopyFile(PREFIX+'/include/','direct/src/interval/cLerpInterval.h')
CopyFile(PREFIX+'/include/','direct/src/interval/cLerpNodePathInterval.I')
CopyFile(PREFIX+'/include/','direct/src/interval/cLerpNodePathInterval.h')
CopyFile(PREFIX+'/include/','direct/src/interval/cLerpAnimEffectInterval.I')
CopyFile(PREFIX+'/include/','direct/src/interval/cLerpAnimEffectInterval.h')
CopyFile(PREFIX+'/include/','direct/src/interval/cMetaInterval.I')
CopyFile(PREFIX+'/include/','direct/src/interval/cMetaInterval.h')
CopyFile(PREFIX+'/include/','direct/src/interval/hideInterval.I')
CopyFile(PREFIX+'/include/','direct/src/interval/hideInterval.h')
CopyFile(PREFIX+'/include/','direct/src/interval/showInterval.I')
CopyFile(PREFIX+'/include/','direct/src/interval/showInterval.h')
CopyFile(PREFIX+'/include/','direct/src/interval/waitInterval.I')
CopyFile(PREFIX+'/include/','direct/src/interval/waitInterval.h')
CopyFile(PREFIX+'/include/','pandatool/src/pandatoolbase/animationConvert.h')
CopyFile(PREFIX+'/include/','pandatool/src/pandatoolbase/config_pandatoolbase.h')
CopyFile(PREFIX+'/include/','pandatool/src/pandatoolbase/distanceUnit.h')
CopyFile(PREFIX+'/include/','pandatool/src/pandatoolbase/pandatoolbase.h')
CopyFile(PREFIX+'/include/','pandatool/src/pandatoolbase/pandatoolsymbols.h')
CopyFile(PREFIX+'/include/','pandatool/src/pandatoolbase/pathReplace.I')
CopyFile(PREFIX+'/include/','pandatool/src/pandatoolbase/pathReplace.h')
CopyFile(PREFIX+'/include/','pandatool/src/pandatoolbase/pathStore.h')
CopyFile(PREFIX+'/include/','pandatool/src/converter/somethingToEggConverter.I')
CopyFile(PREFIX+'/include/','pandatool/src/converter/somethingToEggConverter.h')
CopyFile(PREFIX+'/include/','pandatool/src/progbase/programBase.I')
CopyFile(PREFIX+'/include/','pandatool/src/progbase/programBase.h')
CopyFile(PREFIX+'/include/','pandatool/src/progbase/withOutputFile.I')
CopyFile(PREFIX+'/include/','pandatool/src/progbase/withOutputFile.h')
CopyFile(PREFIX+'/include/','pandatool/src/progbase/wordWrapStream.h')
CopyFile(PREFIX+'/include/','pandatool/src/progbase/wordWrapStreamBuf.I')
CopyFile(PREFIX+'/include/','pandatool/src/progbase/wordWrapStreamBuf.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggbase/eggBase.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggbase/eggConverter.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggbase/eggFilter.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggbase/eggMakeSomething.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggbase/eggMultiBase.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggbase/eggMultiFilter.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggbase/eggReader.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggbase/eggSingleBase.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggbase/eggToSomething.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggbase/eggWriter.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggbase/somethingToEgg.h')
CopyFile(PREFIX+'/include/','pandatool/src/cvscopy/cvsCopy.h')
CopyFile(PREFIX+'/include/','pandatool/src/dxf/dxfFile.h')
CopyFile(PREFIX+'/include/','pandatool/src/dxf/dxfLayer.h')
CopyFile(PREFIX+'/include/','pandatool/src/dxf/dxfLayerMap.h')
CopyFile(PREFIX+'/include/','pandatool/src/dxf/dxfVertex.h')
CopyFile(PREFIX+'/include/','pandatool/src/dxfegg/dxfToEggConverter.h')
CopyFile(PREFIX+'/include/','pandatool/src/dxfegg/dxfToEggLayer.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggBackPointer.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggCharacterCollection.I')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggCharacterCollection.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggCharacterData.I')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggCharacterData.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggCharacterFilter.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggComponentData.I')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggComponentData.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggJointData.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggJointData.I')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggJointPointer.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggJointPointer.I')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggJointNodePointer.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggMatrixTablePointer.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggScalarTablePointer.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggSliderData.I')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggSliderData.h')
CopyFile(PREFIX+'/include/','pandatool/src/eggcharbase/eggVertexPointer.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltBead.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltBeadID.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltCurve.I')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltCurve.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltError.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltExternalReference.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltEyepoint.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltFace.I')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltFace.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltGeometry.I')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltGeometry.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltGroup.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltHeader.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltInstanceDefinition.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltInstanceRef.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltLOD.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltLightSourceDefinition.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltLocalVertexPool.I')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltLocalVertexPool.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltMaterial.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltMesh.I')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltMesh.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltMeshPrimitive.I')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltMeshPrimitive.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltObject.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltOpcode.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltPackedColor.I')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltPackedColor.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltRecord.I')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltRecord.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltRecordReader.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltRecordWriter.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltTexture.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltTrackplane.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltTransformGeneralMatrix.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltTransformPut.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltTransformRecord.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltTransformRotateAboutEdge.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltTransformRotateAboutPoint.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltTransformRotateScale.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltTransformScale.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltTransformTranslate.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltUnsupportedRecord.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltVectorRecord.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltVertex.I')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltVertex.h')
CopyFile(PREFIX+'/include/','pandatool/src/flt/fltVertexList.h')
CopyFile(PREFIX+'/include/','pandatool/src/fltegg/fltToEggConverter.I')
CopyFile(PREFIX+'/include/','pandatool/src/fltegg/fltToEggConverter.h')
CopyFile(PREFIX+'/include/','pandatool/src/fltegg/fltToEggLevelState.I')
CopyFile(PREFIX+'/include/','pandatool/src/fltegg/fltToEggLevelState.h')
CopyFile(PREFIX+'/include/','pandatool/src/imagebase/imageBase.h')
CopyFile(PREFIX+'/include/','pandatool/src/imagebase/imageFilter.h')
CopyFile(PREFIX+'/include/','pandatool/src/imagebase/imageReader.h')
CopyFile(PREFIX+'/include/','pandatool/src/imagebase/imageWriter.I')
CopyFile(PREFIX+'/include/','pandatool/src/imagebase/imageWriter.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/iffChunk.I')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/iffChunk.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/iffGenericChunk.I')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/iffGenericChunk.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/iffId.I')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/iffId.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/iffInputFile.I')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/iffInputFile.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoBoundingBox.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoChunk.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoClip.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoDiscontinuousVertexMap.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoGroupChunk.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoHeader.I')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoHeader.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoInputFile.I')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoInputFile.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoLayer.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoPoints.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoPolygons.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoPolygonTags.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoTags.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoStillImage.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurface.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlock.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockAxis.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockChannel.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockCoordSys.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockEnabled.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockImage.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockOpacity.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockProjection.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockHeader.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockRefObj.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockRepeat.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockTMap.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockTransform.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockVMapName.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceBlockWrap.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceColor.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceParameter.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceSidedness.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoSurfaceSmoothingAngle.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwo/lwoVertexMap.h')
CopyFile(PREFIX+'/include/','pandatool/src/lwoegg/lwoToEggConverter.I')
CopyFile(PREFIX+'/include/','pandatool/src/lwoegg/lwoToEggConverter.h')
CopyFile(PREFIX+'/include/','pandatool/src/vrmlegg/indexedFaceSet.h')
CopyFile(PREFIX+'/include/','pandatool/src/vrmlegg/vrmlAppearance.h')
CopyFile(PREFIX+'/include/','pandatool/src/vrmlegg/vrmlToEggConverter.h')
CopyFile(PREFIX+'/include/','pandatool/src/ptloader/config_ptloader.h')
CopyFile(PREFIX+'/include/','pandatool/src/ptloader/loaderFileTypePandatool.h')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatClientData.h')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatGraph.I')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatGraph.h')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatListener.h')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatMonitor.I')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatMonitor.h')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatPianoRoll.I')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatPianoRoll.h')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatReader.h')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatServer.h')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatStripChart.I')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatStripChart.h')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatThreadData.I')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatThreadData.h')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatView.I')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatView.h')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatViewLevel.I')
CopyFile(PREFIX+'/include/','pandatool/src/pstatserver/pStatViewLevel.h')
CopyFile(PREFIX+'/include/','pandaapp/src/pandaappbase/pandaappbase.h')
CopyFile(PREFIX+'/include/','pandaapp/src/pandaappbase/pandaappsymbols.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/config_stitch.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/fadeImagePool.I')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/fadeImagePool.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/fixedPoint.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/layeredImage.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/morphGrid.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchCommand.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchCommandReader.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchCylindricalLens.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchFile.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchFisheyeLens.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchImage.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchImageCommandOutput.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchImageOutputter.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchImageRasterizer.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchLens.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchLexerDefs.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchPSphereLens.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchParserDefs.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchPerspectiveLens.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchPoint.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitcher.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/triangle.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/triangleRasterizer.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchCylindricalScreen.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchFlatScreen.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchMultiScreen.h')
CopyFile(PREFIX+'/include/','pandaapp/src/stitchbase/stitchScreen.h')

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
            'boundedObject.h', 'config_gobj.h', 'drawable.h', 'geom.h', 'geomContext.h', 'geomLine.h', 'geomLinestrip.h', 'geomPoint.h', 'geomPolygon.h', 'geomQuad.h', 'geomSphere.h', 'geomSprite.h', 'geomTri.h', 'geomTrifan.h', 'geomTristrip.h', 'imageBuffer.h', 'material.h', 'materialPool.h', 'matrixLens.h', 'orthographicLens.h', 'perspectiveLens.h', 'pixelBuffer.h', 'preparedGraphicsObjects.h', 'lens.h', 'savedContext.h', 'texture.h', 'textureContext.h', 'texturePool.h', 'texCoordName.h', 'textureStage.h', 'gobj_composite1.cxx', 'gobj_composite2.cxx'])
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

if (sys.platform == "win32"):
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

if (sys.platform != "win32"):
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

if (sys.platform == "win32"):
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

if (sys.platform == "win32"):
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

if (sys.platform == "win32"):
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

if (sys.platform == "win32"):
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

for VER in ["5","6"]:
  if (OMIT.count("MAYA"+VER)==0):
    IPATH=['pandatool/src/maya']
    OPTS=['MAYA'+VER, 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='maya_composite1.cxx',    obj='maya'+VER+'_composite1.obj')
    CompileLIB(lib='libmaya'+VER+'.lib', obj=[ 'maya'+VER+'_composite1.obj' ])

#
# DIRECTORY: pandatool/src/mayaegg/
#

for VER in ["5","6"]:
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
    OPTS=['MAX'+VER, 'NSPR', "WINCOMCTL", "WINUSER", "MAXEGGDEF"]
    CompileRES(ipath=IPATH, opts=OPTS, src='MaxEgg.rc', obj='maxegg'+VER+'_MaxEgg.res')
    CompileC(ipath=IPATH, opts=OPTS, src='maxegg_composite1.cxx',obj='maxegg'+VER+'_composite1.obj')
    CompileLink(opts=OPTS, dll='maxegg'+VER+'.dle', obj=[
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

for VER in ["5","6"]:
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
    CompileLink(dll='libmayapview'+VER+'.dlm', opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
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
    CompileLink(dll='libmayasavepview.dlm', opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
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

if (OMIT.count("NSPR")==0) and (sys.platform == "win32"):
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

MakeDirectory(PREFIX+"/audio")
MakeDirectory(PREFIX+"/audio/sfx")
MakeDirectory(PREFIX+"/icons")
MakeDirectory(PREFIX+"/maps")
MakeDirectory(PREFIX+"/models")
MakeDirectory(PREFIX+"/models/misc")
MakeDirectory(PREFIX+"/models/gui")

CopyAllFiles(PREFIX+"/audio/sfx/",   "dmodels/src/audio/sfx/", ".wav")
CopyAllFiles(PREFIX+"/icons/",       "dmodels/src/icons/",     ".gif")

CopyAllFiles(PREFIX+"/models/",     "models/",                ".egg")
CopyAllFiles(PREFIX+"/models/",     "models/",                ".bam")

CopyAllFiles(PREFIX+"/maps/",       "models/maps/",           ".jpg")
CopyAllFiles(PREFIX+"/maps/",       "models/maps/",           ".png")
CopyAllFiles(PREFIX+"/maps/",       "models/maps/",           ".rgb")
CopyAllFiles(PREFIX+"/maps/",       "models/maps/",           ".rgba")

CopyAllFiles(PREFIX+"/maps/",       "dmodels/src/maps/",      ".jpg")
CopyAllFiles(PREFIX+"/maps/",       "dmodels/src/maps/",      ".png")
CopyAllFiles(PREFIX+"/maps/",       "dmodels/src/maps/",      ".rgb")
CopyAllFiles(PREFIX+"/maps/",       "dmodels/src/maps/",      ".rgba")

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
    if (sys.platform=="win32"):
        oscmd(PREFIX+"/bin/genpycode.exe")
    else:
        oscmd(PREFIX+"/bin/genpycode")
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
# The Installers
#
##########################################################################################

if (sys.platform == "win32"):

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
        MakeInstaller(PPGAME+"-"+VERSION+".exe", PPGAME+" "+VERSION, PPGAME+" "+VERSION,
                      PPGAME+" "+VERSION, "C:\\"+PPGAME+"-"+VERSION, PPGAME)


##########################################################################################
#
# Print final status report.
#
##########################################################################################

WARNINGS.append("Elapsed Time: "+prettyTime(time.time() - STARTTIME))
printStatus("Makepanda Final Status Report", WARNINGS)

