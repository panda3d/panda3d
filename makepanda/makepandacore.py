########################################################################
##
## Caution: there are two separate, independent build systems:
## 'makepanda', and 'ppremake'.  Use one or the other, do not attempt
## to use both.  This file is part of the 'makepanda' system.
##
## This file, makepandacore, contains all the global state and
## global functions for the makepanda system.
##
########################################################################

import sys,os,time,stat,string,re,getopt,cPickle,fnmatch,threading,Queue,signal,shutil,platform
from distutils import sysconfig

SUFFIX_INC=[".cxx",".c",".h",".I",".yxx",".lxx",".mm",".rc",".r",".plist"]
SUFFIX_DLL=[".dll",".dlo",".dle",".dli",".dlm",".mll",".exe",".pyd"]
SUFFIX_LIB=[".lib",".ilb"]
STARTTIME=time.time()
MAINTHREAD=threading.currentThread()
OUTPUTDIR="built"
CUSTOM_OUTPUTDIR=False
OPTIMIZE="3"
VERBOSE=False

########################################################################
##
## Maya and Max Version List (with registry keys)
##
########################################################################

MAYAVERSIONINFO=[("MAYA6",   "6.0"),
                 ("MAYA65",  "6.5"),
                 ("MAYA7",   "7.0"),
                 ("MAYA8",   "8.0"),
                 ("MAYA85",  "8.5"),
                 ("MAYA2008","2008"),
                 ("MAYA2009","2009"),
]

MAXVERSIONINFO = [("MAX6", "SOFTWARE\\Autodesk\\3DSMAX\\6.0", "installdir", "maxsdk\\cssdk\\include"),
                  ("MAX7", "SOFTWARE\\Autodesk\\3DSMAX\\7.0", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX8", "SOFTWARE\\Autodesk\\3DSMAX\\8.0", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX9", "SOFTWARE\\Autodesk\\3DSMAX\\9.0", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX2009", "SOFTWARE\\Autodesk\\3DSMAX\\11.0\\MAX-1:409", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX2010", "SOFTWARE\\Autodesk\\3DSMAX\\12.0\\MAX-1:409", "Installdir", "maxsdk\\include\\CS"),
]

MAYAVERSIONS=[]
MAXVERSIONS=[]
DXVERSIONS=["DX8","DX9"]

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

WARNINGS=[]
THREADS={}
HAVE_COLORS=False
try:
  import curses
  curses.setupterm()
  HAVE_COLORS=sys.stdout.isatty()
except: pass

def GetColor(color = None):
    if not HAVE_COLORS: return ""
    if color != None: color = color.lower()
    if (color == "blue"):
      return curses.tparm(curses.tigetstr("setf"), 1)
    elif (color == "green"):
      return curses.tparm(curses.tigetstr("setf"), 2)
    elif (color == "cyan"):
      return curses.tparm(curses.tigetstr("setf"), 3)
    elif (color == "red"):
      return curses.tparm(curses.tigetstr("setf"), 4)
    elif (color == "magenta"):
      return curses.tparm(curses.tigetstr("setf"), 5)
    elif (color == "yellow"):
      return curses.tparm(curses.tigetstr("setf"), 6)
    else:
      return curses.tparm(curses.tigetstr("sgr0"))

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

def ProgressOutput(progress, msg, target = None):
    if (threading.currentThread() == MAINTHREAD):
        if (progress == None):
            print msg
        elif (progress >= 100.0):
            print "%s[%s%d%%%s] %s" % (GetColor("yellow"), GetColor("cyan"), progress, GetColor("yellow"), msg),
        elif (progress < 10.0):
            print "%s[%s  %d%%%s] %s" % (GetColor("yellow"), GetColor("cyan"), progress, GetColor("yellow"), msg),
        else:
            print "%s[%s %d%%%s] %s" % (GetColor("yellow"), GetColor("cyan"), progress, GetColor("yellow"), msg),
    else:
        global THREADS
        ident = threading.currentThread().ident
        if (ident not in THREADS):
            THREADS[ident] = len(THREADS) + 1
        print "%s[%sT%d%s] %s" % (GetColor("yellow"), GetColor("cyan"), THREADS[ident], GetColor("yellow"), msg),
    if (target == None):
        print GetColor()
    else:
        print "%s%s%s" % (GetColor("green"), target, GetColor())

def exit(msg = ""):
    if (threading.currentThread() == MAINTHREAD):
        SaveDependencyCache()
        # Move any files we've moved away back.
        if os.path.isfile("dtool/src/dtoolutil/pandaVersion.h.moved"):
          os.rename("dtool/src/dtoolutil/pandaVersion.h.moved", "dtool/src/dtoolutil/pandaVersion.h")
        if os.path.isfile("dtool/src/dtoolutil/checkPandaVersion.h.moved"):
          os.rename("dtool/src/dtoolutil/checkPandaVersion.h.moved", "dtool/src/dtoolutil/checkPandaVersion.h")
        if os.path.isfile("dtool/src/dtoolutil/checkPandaVersion.cxx.moved"):
          os.rename("dtool/src/dtoolutil/checkPandaVersion.cxx.moved", "dtool/src/dtoolutil/checkPandaVersion.cxx")
        if os.path.isfile("dtool/src/prc/prc_parameters.h.moved"):
          os.rename("dtool/src/prc/prc_parameters.h.moved", "dtool/src/prc/prc_parameters.h")
        if os.path.isfile("direct/src/plugin/p3d_plugin_config.h.moved"):
          os.rename("direct/src/plugin/p3d_plugin_config.h.moved", "direct/src/plugin/p3d_plugin_config.h")
        print "Elapsed Time: "+PrettyTime(time.time() - STARTTIME)
        print msg
        print GetColor("red") + "Build terminated." + GetColor()
        sys.stdout.flush()
        sys.stderr.flush()
        os._exit(1)
    else:
        print msg
        raise "initiate-exit"

########################################################################
##
## Run a command.
##
########################################################################

def oscmd(cmd, ignoreError = False):
    if VERBOSE:
        print GetColor("blue") + cmd.split(" ", 1)[0] + " " + GetColor("magenta") + cmd.split(" ", 1)[1] + GetColor()
    sys.stdout.flush()
    if sys.platform == "win32":
        exe = cmd.split()[0]
        if not (len(exe) > 4 and exe[-4:] == ".exe"):
            exe += ".exe"
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
    if res != 0 and not ignoreError:
        exit("")

########################################################################
##
## GetDirectoryContents
##
## At times, makepanda will use a function like "os.listdir" to process
## all the files in a directory.  Unfortunately, that means that any
## accidental addition of a file to a directory could cause makepanda
## to misbehave without warning.
##
## To alleviate this weakness, we created GetDirectoryContents.  This
## uses "os.listdir" to fetch the directory contents, but then it
## compares the results to the appropriate CVS/Entries to see if
## they match.  If not, it prints a big warning message.
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
    if (os.path.isfile(dir + "/CVS/Entries")):
        cvs = {}
        srchandle = open(dir+"/CVS/Entries", "r")
        files = []
        for line in srchandle:
            if (line[0]=="/"):
                s = line.split("/",2)
                if (len(s)==3):
                    files.append(s[1])
        srchandle.close()
        for filter in filters:
            for file in fnmatch.filter(files, filter):
                if (skip.count(file)==0):
                    cvs[file] = 1
        for file in actual.keys():
            if (file not in cvs and VERBOSE):
                msg = "%sWARNING: %s is in %s, but not in CVS%s" % (GetColor("red"), GetColor("green") + file + GetColor(), GetColor("green") + dir + GetColor(), GetColor())
                print msg
                WARNINGS.append(msg)
        for file in cvs.keys():
            if (file not in actual and VERBOSE):
                msg = "%sWARNING: %s is not in %s, but is in CVS%s" % (GetColor("red"), GetColor("green") + file + GetColor(), GetColor("green") + dir + GetColor(), GetColor())
                print msg
                WARNINGS.append(msg)
    results = actual.keys()
    results.sort()
    return results

########################################################################
##
## LocateBinary
##
## This function searches the system PATH for the binary. Returns its
## full path when it is found, or None when it was not found.
##
########################################################################

def LocateBinary(binary):
    if "PATH" not in os.environ or os.environ["PATH"] == "":
        p = os.defpath
    else:
        p = os.environ["PATH"]
    
    for path in p.split(os.pathsep):
        if os.access(os.path.join(path, binary), os.X_OK):
            return os.path.abspath(os.path.realpath(os.path.join(path, binary)))
    return None

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

def JustBuilt(files,others):
    dates = []
    for file in files:
        del TIMESTAMPCACHE[file]
        dates.append(GetTimestamp(file))
    for file in others:
        dates.append(GetTimestamp(file))
    key = tuple(files)
    BUILTFROMCACHE[key] = [others,dates]

def NeedsBuild(files,others):
    dates = []
    for file in files:
        dates.append(GetTimestamp(file))
        if (not os.path.exists(file)): return 1
    for file in others:
        dates.append(GetTimestamp(file))
    key = tuple(files)
    if (key in BUILTFROMCACHE):
        if (BUILTFROMCACHE[key] == [others,dates]):
            return 0
        else:
            oldothers = BUILTFROMCACHE[key][0]
            if (oldothers != others and VERBOSE):
                print "%sCAUTION:%s file dependencies changed: %s%s%s" % (GetColor("red"), GetColor(), GetColor("green"), str(files), GetColor())
    return 1

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
## SaveDependencyCache / LoadDependencyCache
##
## This actually saves both the dependency and cxx-include caches.
##
########################################################################

def SaveDependencyCache():
    try:
        if (os.path.exists(os.path.join(OUTPUTDIR, "tmp", "makepanda-dcache"))):
            os.rename(os.path.join(OUTPUTDIR, "tmp", "makepanda-dcache"),
                      os.path.join(OUTPUTDIR, "tmp", "makepanda-dcache-backup"))
    except: pass
    try: icache = open(os.path.join(OUTPUTDIR, "tmp", "makepanda-dcache"),'wb')
    except: icache = 0
    if (icache!=0):
        print "Storing dependency cache."
        cPickle.dump(CXXINCLUDECACHE, icache, 1)
        cPickle.dump(BUILTFROMCACHE, icache, 1)
        icache.close()

def LoadDependencyCache():
    global CXXINCLUDECACHE
    global BUILTFROMCACHE
    try: icache = open(os.path.join(OUTPUTDIR, "tmp", "makepanda-dcache"),'rb')
    except: icache = 0
    if (icache!=0):
        CXXINCLUDECACHE = cPickle.load(icache)
        BUILTFROMCACHE = cPickle.load(icache)
        icache.close()

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
    result = dep.keys()
    CxxDependencyCache[srcfile] = result
    return result

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
    import _winreg

def TryRegistryKey(path):
    try:
        key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, path, 0, _winreg.KEY_READ)
        return key
    except: pass
    try:
        key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, path, 0, _winreg.KEY_READ | 256)
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
                result.append(_winreg.EnumKey(key, index))
                index = index + 1
        except: pass
        _winreg.CloseKey(key)
    return result

def GetRegistryKey(path, subkey):
    if (platform.architecture()[0]=="64bit"):
        path = path.replace("SOFTWARE\\", "SOFTWARE\\Wow6432Node\\")
    k1=0
    key = TryRegistryKey(path)
    if (key != 0):
        try:
            k1, k2 = _winreg.QueryValueEx(key, subkey)
        except: pass
        _winreg.CloseKey(key)
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

def ConditionalWriteFile(dest,desiredcontents):
    try:
        rfile = open(dest, 'rb')
        contents = rfile.read(-1)
        rfile.close()
    except:
        contents=0
    if contents != desiredcontents:
        sys.stdout.flush()
        WriteFile(dest,desiredcontents)

def DeleteCVS(dir):
    if dir == "": dir = "."
    for entry in os.listdir(dir):
        if (entry != ".") and (entry != ".."):
            subdir = dir + "/" + entry
            if (os.path.isdir(subdir)):
                if (entry == "CVS"):
                    shutil.rmtree(subdir)
                else:
                    DeleteCVS(subdir)
            elif (os.path.isfile(subdir) and (entry == ".cvsignore" or entry.startswith(".#"))):
                os.remove(subdir)

def DeleteBuildFiles(dir):
    for entry in os.listdir(dir):
        if (entry != ".") and (entry != ".."):
            subdir = dir + "/" + entry
            if (os.path.isfile(subdir) and os.path.splitext(subdir)[-1] in SUFFIX_INC+[".pp", ".moved"]):
                os.remove(subdir)
            elif (os.path.isdir(subdir)):
                DeleteBuildFiles(subdir)

def CreateFile(file):
    if (os.path.exists(file)==0):
        WriteFile(file,"")

########################################################################
#
# Create the panda build tree.
#
########################################################################

def MakeBuildTree():
    MakeDirectory(OUTPUTDIR)
    MakeDirectory(OUTPUTDIR+"/bin")
    MakeDirectory(OUTPUTDIR+"/lib")
    MakeDirectory(OUTPUTDIR+"/tmp")
    MakeDirectory(OUTPUTDIR+"/etc")
    MakeDirectory(OUTPUTDIR+"/plugins")
    MakeDirectory(OUTPUTDIR+"/include")
    MakeDirectory(OUTPUTDIR+"/models")
    MakeDirectory(OUTPUTDIR+"/models/audio")
    MakeDirectory(OUTPUTDIR+"/models/audio/sfx")
    MakeDirectory(OUTPUTDIR+"/models/icons")
    MakeDirectory(OUTPUTDIR+"/models/maps")
    MakeDirectory(OUTPUTDIR+"/models/misc")
    MakeDirectory(OUTPUTDIR+"/models/gui")
    MakeDirectory(OUTPUTDIR+"/direct")
    MakeDirectory(OUTPUTDIR+"/pandac")
    MakeDirectory(OUTPUTDIR+"/pandac/input")

########################################################################
#
# Make sure that you are in the root of the panda tree.
#
########################################################################

def CheckPandaSourceTree():
    dir = os.getcwd()
    if ((os.path.exists(os.path.join(dir, "makepanda/makepanda.py"))==0) or
        (os.path.exists(os.path.join(dir, "dtool","src","dtoolbase","dtoolbase.h"))==0) or
        (os.path.exists(os.path.join(dir, "panda","src","pandabase","pandabase.h"))==0)):
        exit("Current directory is not the root of the panda tree.")

########################################################################
##
## Visual Studio Manifest Manipulation.
##
########################################################################

VC90CRTVERSIONRE=re.compile("name=['\"]Microsoft.VC90.CRT['\"]\\s+version=['\"]([0-9.]+)['\"]")

def GetVC90CRTVersion(fn):
    manifest = ReadFile(fn)
    version = VC90CRTVERSIONRE.search(manifest)
    if (version == None):
        exit("Cannot locate version number in "+fn)
    return version.group(1)

def SetVC90CRTVersion(fn, ver):
    manifest = ReadFile(fn)
    subst = " name='Microsoft.VC90.CRT' version='"+ver+"' "
    manifest = VC90CRTVERSIONRE.sub(subst, manifest)
    WriteFile(fn, manifest)

########################################################################
##
## Gets or sets the output directory, by default "built".
## Gets or sets the optimize level.
## Gets or sets the verbose flag.
##
########################################################################

def GetOutputDir():
  return OUTPUTDIR

def SetOutputDir(outputdir):
  global OUTPUTDIR, CUSTOM_OUTPUTDIR
  OUTPUTDIR=outputdir
  CUSTOM_OUTPUTDIR=True

def GetOptimize():
  return int(OPTIMIZE)

def SetOptimize(optimize):
  global OPTIMIZE
  OPTIMIZE=optimize

def SetVerbose(verbose):
  global VERBOSE
  VERBOSE=verbose

########################################################################
##
## Package Selection
##
## This facility enables makepanda to keep a list of packages selected
## by the user for inclusion or omission.
##
########################################################################

PKG_LIST_ALL=0
PKG_LIST_OMIT=0

def PkgListSet(pkgs):
    global PKG_LIST_ALL
    global PKG_LIST_OMIT
    PKG_LIST_ALL=pkgs
    PKG_LIST_OMIT={}
    PkgDisableAll()

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

def PkgSkip(pkg):
    return PKG_LIST_OMIT[pkg]

def PkgSelected(pkglist, pkg):
    if (pkglist.count(pkg)==0): return 0
    if (PKG_LIST_OMIT[pkg]): return 0
    return 1
if os.path.isfile("dtool/src/prc/prc_parameters.h.moved"):
  os.rename("dtool/src/prc/prc_parameters.h.moved", "dtool/src/prc/prc_parameters.h")
if os.path.isfile("direct/src/plugin/p3d_plugin_config.h.moved"):
  os.rename("direct/src/plugin/p3d_plugin_config.h.moved", "direct/src/plugin/p3d_plugin_config.h")
########################################################################
##
## These functions are for libraries which use pkg-config.
##
########################################################################

def PkgConfigHavePkg(pkgname):
    """Returns a bool whether the pkg-config package is installed."""
    if (sys.platform == "win32" or not LocateBinary("pkg-config")):
        return False
    handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --modversion " + pkgname)
    result = handle.read().strip()
    handle.close()
    return bool(len(result) > 0)

def PkgConfigGetLibs(pkgname):
    """Returns a list of libs for the package, prefixed by -l."""
    if (sys.platform == "win32" or not LocateBinary("pkg-config")):
        return []
    handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --libs-only-l " + pkgname)
    result = handle.read().strip()
    handle.close()
    libs = []
    for l in result.split(" "):
        libs.append(l)
    return libs

def PkgConfigGetIncDirs(pkgname):
    """Returns a list of includes for the package, NOT prefixed by -I."""
    if (sys.platform == "win32" or not LocateBinary("pkg-config")):
        return []
    handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --cflags-only-I " + pkgname)
    result = handle.read().strip()
    handle.close()
    if len(result) == 0: return []
    libs = []
    for l in result.split(" "):
        libs.append(l.replace("-I", "").replace("\"", "").strip())
    return libs

def PkgConfigGetLibDirs(pkgname):
    """Returns a list of library paths for the package, NOT prefixed by -L."""
    if (sys.platform == "win32" or not LocateBinary("pkg-config")):
        return []
    handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --libs-only-L " + pkgname)
    result = handle.read().strip()
    handle.close()
    if len(result) == 0: return []
    libs = []
    for l in result.split(" "):
        libs.append(l.replace("-L", "").replace("\"", "").strip())
    return libs

def PkgConfigEnable(opt, pkgname):
    """Adds the libraries and includes to IncDirectory, LibName and LibDirectory."""
    for i in PkgConfigGetIncDirs(pkgname):
        IncDirectory(opt, i)
    for i in PkgConfigGetLibDirs(pkgname):
        LibDirectory(opt, i)
    for i in PkgConfigGetLibs(pkgname):
        LibName(opt, i)

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
    sdir = "sdks"
    if (sys.platform.startswith("win")):
        sdir += "/win"
        sdir += platform.architecture()[0][:2]
    elif (sys.platform.startswith("linux")):
        sdir += "/linux"
        sdir += platform.architecture()[0][:2]
    elif (sys.platform == "darwin"):
        sdir += "/macosx"
    sdir += "/" + sdkname
    
    # If it does not exist, try the old location.
    if (sdkkey and not os.path.isdir(sdir)):
        sdir = "sdks/" + sdir
        if (sys.platform.startswith("linux")):
            sdir += "-linux"
            sdir += platform.architecture()[0][:2]
        elif (sys.platform == "darwin"):
            sdir += "-osx"

    if (os.path.isdir(sdir)):
        SDK[sdkkey] = sdir
    
    return sdir

def SdkLocateDirectX():
    if (sys.platform != "win32"): return
    GetSdkDir("directx8", "DX8")
    GetSdkDir("directx9", "DX9")
    if ("DX9" not in SDK):
        ## Try to locate the key within the "new" March 2009 location in the registry (yecch):
        dir = GetRegistryKey("SOFTWARE\\Microsoft\\DirectX\\Microsoft DirectX SDK (March 2009)", "InstallPath")
        if (dir != 0):
            SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
    if ("DX9" not in SDK) or ("DX8" not in SDK):
        uninstaller = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
        for subdir in ListRegistryKeys(uninstaller):
            if (subdir[0]=="{"):
                dir = GetRegistryKey(uninstaller+"\\"+subdir, "InstallLocation")
                if (dir != 0):
                    if (("DX8" not in SDK) and
                        (os.path.isfile(dir+"\\Include\\d3d8.h")) and
                        (os.path.isfile(dir+"\\Include\\d3dx8.h")) and
                        (os.path.isfile(dir+"\\Lib\\d3d8.lib")) and
                        (os.path.isfile(dir+"\\Lib\\d3dx8.lib"))):
                        SDK["DX8"] = dir.replace("\\", "/").rstrip("/")
                    if (("DX9" not in SDK) and
                        (os.path.isfile(dir+"\\Include\\d3d9.h")) and
                        (os.path.isfile(dir+"\\Include\\d3dx9.h")) and
                        (os.path.isfile(dir+"\\Include\\dxsdkver.h")) and
                        (os.path.isfile(dir+"\\Lib\\x86\\d3d9.lib")) and
                        (os.path.isfile(dir+"\\Lib\\x86\\d3dx9.lib"))):
                        SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
    if ("DX9" in SDK):
        SDK["DIRECTCAM"] = SDK["DX9"]

def SdkLocateMaya():
    for (ver,key) in MAYAVERSIONINFO:
        if (PkgSkip(ver)==0 and ver not in SDK):
            GetSdkDir(ver.lower().replace("x",""), ver)
            if (not ver in SDK):
                if (sys.platform == "win32"):
                    for dev in ["Alias|Wavefront","Alias","Autodesk"]:
                        fullkey="SOFTWARE\\"+dev+"\\Maya\\"+key+"\\Setup\\InstallPath"
                        res = GetRegistryKey(fullkey, "MAYA_INSTALL_LOCATION")
                        if (res != 0):
                            res = res.replace("\\", "/").rstrip("/")
                            SDK[ver] = res
                elif (sys.platform == "darwin"):
                    ddir = "/Applications/Autodesk/maya"+key+"/Maya.app/Contents"
                    if (os.path.isdir(ddir)): SDK[ver] = ddir
                else:
                    if (platform.architecture()[0] == "64bit"):
                        ddir1 = "/usr/autodesk/maya"+key+"-x64"
                        ddir2 = "/usr/aw/maya"+key+"-x64"
                    else:
                        ddir1 = "/usr/autodesk/maya"+key
                        ddir2 = "/usr/aw/maya"+key
                    
                    if (os.path.isdir(ddir1)):   SDK[ver] = ddir1
                    elif (os.path.isdir(ddir2)): SDK[ver] = ddir2

def SdkLocateMax():
    if (sys.platform != "win32"): return
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

def SdkLocatePython():
    if (PkgSkip("PYTHON")==0):
        if (sys.platform == "win32"):
            SDK["PYTHON"] = "thirdparty/win-python"
            if (GetOptimize() <= 2):
                SDK["PYTHON"] += "-dbg"
            if (platform.architecture()[0] == "64bit" and os.path.isdir(SDK["PYTHON"] + "-x64")):
                SDK["PYTHON"] += "-x64"
            
            SDK["PYTHONEXEC"] = SDK["PYTHON"] + "/python"
            if (GetOptimize() <= 2): SDK["PYTHONEXEC"] += "_d.exe"
            else: SDK["PYTHONEXEC"] += ".exe"
            
            if (not os.path.isfile(SDK["PYTHONEXEC"])):
                exit("Could not find %s!" % SDK["PYTHONEXEC"])
            
            os.system(SDK["PYTHONEXEC"].replace("/", "\\") + " -V > "+OUTPUTDIR+"/tmp/pythonversion 2>&1")
            pv=ReadFile(OUTPUTDIR+"/tmp/pythonversion")
            if (pv.startswith("Python ")==0):
                exit("python -V did not produce the expected output")
            pv = pv[7:10]
            SDK["PYTHONVERSION"]="python"+pv

        elif (sys.platform == "darwin"):
            if "MACOSX" not in SDK: SdkLocateMacOSX()
            if (os.path.isdir("%s/System/Library/Frameworks/Python.framework" % SDK["MACOSX"])):
                pv = os.readlink("%s/System/Library/Frameworks/Python.framework/Versions/Current" % SDK["MACOSX"])
                SDK["PYTHON"] = SDK["MACOSX"] + "/System/Library/Frameworks/Python.framework/Headers"
                SDK["PYTHONVERSION"] = "python " +pv
                SDK["PYTHONEXEC"] = "/System/Library/Frameworks/Python.framework/Versions/Current/bin/python"
            else:
                exit("Could not find the python framework!")

        else:
            SDK["PYTHON"] = sysconfig.get_python_inc()
            SDK["PYTHONVERSION"] = "python" + sysconfig.get_python_version()
            SDK["PYTHONEXEC"] = sys.executable

def SdkLocateVisualStudio():
    if (sys.platform != "win32"): return
    vcdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VC7", "9.0")
    if (vcdir != 0) and (vcdir[-4:] == "\\VC\\"):
        vcdir = vcdir[:-3]
        SDK["VISUALSTUDIO"] = vcdir
    elif "VCINSTALLDIR" in os.environ:
        vcdir = os.environ["VCINSTALLDIR"]
        if (vcdir[-3:] == "\\VC"):
            vcdir = vcdir[:-2]
        elif (vcdir[-4:] == "\\VC\\"):
            vcdir = vcdir[:-3]
        SDK["VISUALSTUDIO"] = vcdir

def SdkLocateMSPlatform():
    if (sys.platform != "win32"): return
    platsdk = GetRegistryKey("SOFTWARE\\Microsoft\\MicrosoftSDK\\InstalledSDKs\\D2FF9F89-8AA2-4373-8A31-C838BF4DBBE1", "Install Dir")
    if (platsdk and not os.path.isdir(platsdk)): platsdk = 0
    if (platsdk == 0):
        platsdk = GetRegistryKey("SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v6.1","InstallationFolder")
        if (platsdk and not os.path.isdir(platsdk)): platsdk = 0
    
    if (platsdk == 0 and os.path.isdir(os.path.join(GetProgramFiles(), "Microsoft Platform SDK for Windows Server 2003 R2"))):
        if (platform.architecture()[0]!="64bit" or os.path.isdir(os.path.join(GetProgramFiles(), "Microsoft Platform SDK for Windows Server 2003 R2", "Lib", "AMD64"))):
            platsdk = os.path.join(GetProgramFiles(), "Microsoft Platform SDK for Windows Server 2003 R2")
            if (not os.path.isdir(platsdk)): platsdk = 0
    
    # Doesn't work with the Express versions, so we're checking for the "atlmfc" dir, which is not in the Express 
    if (platsdk == 0 and os.path.isdir(os.path.join(GetProgramFiles(), "Microsoft Visual Studio 9\\VC\\atlmfc"))
                     and os.path.isdir(os.path.join(GetProgramFiles(), "Microsoft Visual Studio 9\\VC\\PlatformSDK"))):
        platsdk = os.path.join(GetProgramFiles(), "Microsoft Visual Studio 9\\VC\\PlatformSDK")
        if (not os.path.isdir(platsdk)): platsdk = 0
    
    # This may not be the best idea but it does give a warning
    if (platsdk == 0):
        if ("WindowsSdkDir" in os.environ):
            WARNINGS.append("Windows SDK directory not found in registry, found in Environment variables instead")
            platsdk = os.environ["WindowsSdkDir"]
    if (platsdk != 0):
        if (not platsdk.endswith("\\")):
            platsdk += "\\"
        SDK["MSPLATFORM"] = platsdk

def SdkLocateMacOSX():
    if (sys.platform != "darwin"): return
    if (os.path.exists("/Developer/SDKs/MacOSX10.6.sdk")):
        SDK["MACOSX"] = "/Developer/SDKs/MacOSX10.6.sdk"
    elif (os.path.exists("/Developer/SDKs/MacOSX10.5.sdk")):
        SDK["MACOSX"] = "/Developer/SDKs/MacOSX10.5.sdk"
    elif (os.path.exists("/Developer/SDKs/MacOSX10.4u.sdk")):
        SDK["MACOSX"] = "/Developer/SDKs/MacOSX10.4u.sdk"
    elif (os.path.exists("/Developer/SDKs/MacOSX10.4.0.sdk")):
        SDK["MACOSX"] = "/Developer/SDKs/MacOSX10.4.0.sdk"
    else:
        exit("Could not find any MacOSX SDK")

########################################################################
##
## SDK Auto-Disables
##
## Disable packages whose SDKs could not be found.
##
########################################################################

def SdkAutoDisableDirectX():
    for ver in ["DX8","DX9","DIRECTCAM"]:
        if (PkgSkip(ver)==0):
            if (ver not in SDK):
                if (sys.platform.startswith("win")):
                    WARNINGS.append("I cannot locate SDK for "+ver)
                    WARNINGS.append("I have automatically added this command-line option: --no-"+ver.lower())
                PkgDisable(ver)
            else:
                WARNINGS.append("Using "+ver+" sdk: "+SDK[ver])

def SdkAutoDisableMaya():
    for (ver,key) in MAYAVERSIONINFO:
        if (ver not in SDK) and (PkgSkip(ver)==0):
            if (sys.platform == "win32"):
                WARNINGS.append("The registry does not appear to contain a pointer to the "+ver+" SDK.")
            else:
                WARNINGS.append("I cannot locate SDK for "+ver)
            WARNINGS.append("I have automatically added this command-line option: --no-"+ver.lower())
            PkgDisable(ver)

def SdkAutoDisableMax():
    for version,key1,key2,subdir in MAXVERSIONINFO:
        if (PkgSkip(version)==0) and ((version not in SDK) or (version+"CS" not in SDK)): 
            if (sys.platform.startswith("win")):
                if (version in SDK):
                    WARNINGS.append("Your copy of "+version+" does not include the character studio SDK")
                else: 
                    WARNINGS.append("The registry does not appear to contain a pointer to "+version)
                WARNINGS.append("I have automatically added this command-line option: --no-"+version.lower())
            PkgDisable(version)

########################################################################
##
## Visual Studio comes with a script called VSVARS32.BAT, which 
## you need to run before using visual studio command-line tools.
## The following python subroutine serves the same purpose.
##
########################################################################

def AddToPathEnv(path,add):
    if (path in os.environ):
        if (sys.platform.startswith("win")):
            os.environ[path] = add + ";" + os.environ[path]
        else:
            os.environ[path] = add + ":" + os.environ[path]
    else:
        os.environ[path] = add

def SetupVisualStudioEnviron():
    if ("VISUALSTUDIO" not in SDK):
        exit("Could not find Visual Studio install directory")
    if ("MSPLATFORM" not in SDK):
        exit("Could not find the Microsoft Platform SDK")
    os.environ["VCINSTALLDIR"] = SDK["VISUALSTUDIO"] + "VC"
    os.environ["WindowsSdkDir"] = SDK["MSPLATFORM"]
    suffix=""
    if (platform.architecture()[0]=="64bit"): suffix = "\\amd64"
    AddToPathEnv("PATH",    SDK["VISUALSTUDIO"] + "VC\\bin"+suffix)
    AddToPathEnv("PATH",    SDK["VISUALSTUDIO"] + "Common7\\IDE")
    AddToPathEnv("INCLUDE", SDK["VISUALSTUDIO"] + "VC\\include")
    AddToPathEnv("INCLUDE", SDK["VISUALSTUDIO"] + "VC\\atlmfc\\include")
    AddToPathEnv("LIB",     SDK["VISUALSTUDIO"] + "VC\\lib"+suffix)
    AddToPathEnv("INCLUDE", SDK["MSPLATFORM"] + "include")
    AddToPathEnv("INCLUDE", SDK["MSPLATFORM"] + "include\\atl")
    AddToPathEnv("INCLUDE", SDK["MSPLATFORM"] + "include\\mfc")
    if (platform.architecture()[0]=="32bit"):
        AddToPathEnv("LIB", SDK["MSPLATFORM"] + "lib")
    elif (os.path.isdir(SDK["MSPLATFORM"] + "lib\\x64")):
        AddToPathEnv("LIB", SDK["MSPLATFORM"] + "lib\\x64")
    elif (os.path.isdir(SDK["MSPLATFORM"] + "lib\\amd64")):
        AddToPathEnv("LIB", SDK["MSPLATFORM"] + "lib\\amd64")
    else:
        exit("Could not locate 64-bits libraries in Platform SDK!")

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
LIBNAMES = []
DEFSYMBOLS = []

def IncDirectory(opt, dir):
    INCDIRECTORIES.append((opt, dir))

def LibDirectory(opt, dir):
    LIBDIRECTORIES.append((opt, dir))

def LibName(opt, name):
    #check to see if the lib file actually exists for the thrid party library given
    #are we a thrid party library?
    if name.startswith("thirdparty"):
        #does this lib exists
        if not os.path.exists(name):
            PkgDisable(opt)
            WARNINGS.append(name + " not found.  Skipping Package " + opt)
            return
    LIBNAMES.append((opt, name))

def DefSymbol(opt, sym, val):
    DEFSYMBOLS.append((opt, sym, val))

########################################################################
#
# On Linux/OSX, to run panda, the dynamic linker needs to know how to
# find the shared libraries.  This subroutine verifies that the dynamic
# linker is properly configured.  If not, it sets it up on a temporary
# basis and issues a warning.
#
########################################################################

def CheckLinkerLibraryPath():
    if (sys.platform == "win32"): return
    builtlib = os.path.abspath(os.path.join(OUTPUTDIR,"lib"))
    dyldpath = []
    try:
        ldpath = []
        f = file("/etc/ld.so.conf","r")
        for line in f: ldpath.append(line.rstrip())
        f.close()
    except: ldpath = []
    
    # Get the current 
    if ("LD_LIBRARY_PATH" in os.environ):
        ldpath = ldpath + os.environ["LD_LIBRARY_PATH"].split(":")
    if (sys.platform == "darwin" and "DYLD_LIBRARY_PATH" in os.environ):
        dyldpath = os.environ["DYLD_LIBRARY_PATH"].split(":")
    
    # Remove any potential current Panda installation lib dirs
    for i in ldpath:
        if i.startswith("/usr/lib/panda"): ldpath.remove(i)
    for i in ldpath:
        if i.startswith("/usr/local/panda"): ldpath.remove(i)
    for i in dyldpath:
        if i.startswith("/Applications/Panda3D"): dyldpath.remove(i)
    
    # Add built/lib/ to (DY)LD_LIBRARY_PATH if it's not already there
    if (ldpath.count(builtlib)==0):
        if ("LD_LIBRARY_PATH" in os.environ):
            os.environ["LD_LIBRARY_PATH"] = builtlib + ":" + os.environ["LD_LIBRARY_PATH"]
        else:
            os.environ["LD_LIBRARY_PATH"] = builtlib
    if (sys.platform == "darwin" and dyldpath.count(builtlib)==0):
        if ("DYLD_LIBRARY_PATH" in os.environ):
            os.environ["DYLD_LIBRARY_PATH"] = builtlib + ":" + os.environ["DYLD_LIBRARY_PATH"]
        else:
            os.environ["DYLD_LIBRARY_PATH"] = builtlib
     
    # Workaround around compile issue on PCBSD
    if (platform.uname()[1]=="pcbsd"):
        os.environ["LD_LIBRARY_PATH"] += ":/usr/PCBSD/local/lib"

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
        WriteFile(dstfile,ReadFile(srcfile))
        JustBuilt([dstfile], [srcfile])

def CopyAllFiles(dstdir, srcdir, suffix=""):
    for x in GetDirectoryContents(srcdir, ["*"+suffix]):
        CopyFile(dstdir+x, srcdir+x)

def CopyAllHeaders(dir, skip=[]):
    for filename in GetDirectoryContents(dir, ["*.h", "*.I", "*.T"], skip):
        srcfile = dir + "/" + filename
        dstfile = OUTPUTDIR+"/include/" + filename
        if (NeedsBuild([dstfile],[srcfile])):
            WriteFile(dstfile,ReadFile(srcfile))
            JustBuilt([dstfile],[srcfile])

def CopyTree(dstdir,srcdir):
    if (os.path.isdir(dstdir)): return 0
    if (sys.platform == "win32"):
        cmd = 'xcopy /I/Y/E/Q "' + srcdir + '" "' + dstdir + '"'
    else:
        cmd = 'cp -R -f ' + srcdir + ' ' + dstdir
    oscmd(cmd)

########################################################################
##
## Parse PandaVersion.pp to extract the version number.
##
########################################################################

def ParsePandaVersion(fn):
    try:
        f = file(fn, "r")
        pattern = re.compile('^[ \t]*[#][ \t]*define[ \t]+PANDA_VERSION[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)')
        for line in f:
            match = pattern.match(line,0)
            if (match):
                version = match.group(1)+"."+match.group(2)+"."+match.group(3)
                break
        f.close()
    except: version="0.0.0"
    return version

########################################################################
##
## FindLocation
##
########################################################################

ORIG_EXT={}

def GetOrigExt(x):
    return ORIG_EXT[x]

def SetOrigExt(x, v):
    ORIG_EXT[x] = v

def CalcLocation(fn, ipath):
    if (fn.count("/")): return fn
    dllext = ""
    if (GetOptimize() <= 2): dllext = "_d"

    if (fn == "PandaModules.py"): return "pandac/" + fn
    if (fn.endswith(".cxx")): return CxxFindSource(fn, ipath)
    if (fn.endswith(".I")):   return CxxFindSource(fn, ipath)
    if (fn.endswith(".h")):   return CxxFindSource(fn, ipath)
    if (fn.endswith(".c")):   return CxxFindSource(fn, ipath)
    if (fn.endswith(".yxx")): return CxxFindSource(fn, ipath)
    if (fn.endswith(".lxx")): return CxxFindSource(fn, ipath)
    if (fn.endswith(".pdef")):return CxxFindSource(fn, ipath)
    if (sys.platform.startswith("win")):
        if (fn.endswith(".def")): return CxxFindSource(fn, ipath)
        if (fn.endswith(".rc")):  return CxxFindSource(fn, ipath)
        if (fn.endswith(".obj")): return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".res")): return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".dll")): return OUTPUTDIR+"/bin/"+fn[:-4]+dllext+".dll"
        if (fn.endswith(".pyd")): return OUTPUTDIR+"/bin/"+fn[:-4]+dllext+".pyd"
        if (fn.endswith(".mll")): return OUTPUTDIR+"/plugins/"+fn[:-4]+dllext+".mll"
        if (fn.endswith(".dlo")): return OUTPUTDIR+"/plugins/"+fn[:-4]+dllext+".dlo"
        if (fn.endswith(".dli")): return OUTPUTDIR+"/plugins/"+fn[:-4]+dllext+".dli"
        if (fn.endswith(".dle")): return OUTPUTDIR+"/plugins/"+fn[:-4]+dllext+".dle"
        if (fn.endswith(".exe")): return OUTPUTDIR+"/bin/"+fn
        if (fn.endswith(".lib")): return OUTPUTDIR+"/lib/"+fn[:-4]+dllext+".lib"
        if (fn.endswith(".ilb")): return OUTPUTDIR+"/tmp/"+fn[:-4]+dllext+".lib"
        if (fn.endswith(".dat")): return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".in")):  return OUTPUTDIR+"/pandac/input/"+fn
    elif (sys.platform == "darwin"):
        if (fn.endswith(".mm")):    return CxxFindSource(fn, ipath)
        if (fn.endswith(".r")):     return CxxFindSource(fn, ipath)
        if (fn.endswith(".plist")): return CxxFindSource(fn, ipath)
        if (fn.endswith(".obj")):   return OUTPUTDIR+"/tmp/"+fn[:-4]+".o"
        if (fn.endswith(".dll")):   return OUTPUTDIR+"/lib/"+fn[:-4]+".dylib"
        if (fn.endswith(".pyd")):   return OUTPUTDIR+"/lib/"+fn[:-4]+".dylib"
        if (fn.endswith(".mll")):   return OUTPUTDIR+"/plugins/"+fn
        if (fn.endswith(".exe")):   return OUTPUTDIR+"/bin/"+fn[:-4]
        if (fn.endswith(".lib")):   return OUTPUTDIR+"/lib/"+fn[:-4]+".a"
        if (fn.endswith(".ilb")):   return OUTPUTDIR+"/tmp/"+fn[:-4]+".a"
        if (fn.endswith(".dat")):   return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".rsrc")):  return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".plugin")):return OUTPUTDIR+"/plugins/"+fn
        if (fn.endswith(".in")):    return OUTPUTDIR+"/pandac/input/"+fn
    else:
        if (fn.endswith(".obj")): return OUTPUTDIR+"/tmp/"+fn[:-4]+".o"
        if (fn.endswith(".dll")): return OUTPUTDIR+"/lib/"+fn[:-4]+".so"
        if (fn.endswith(".pyd")): return OUTPUTDIR+"/lib/"+fn[:-4]+".so"
        if (fn.endswith(".mll")): return OUTPUTDIR+"/plugins/"+fn
        if (fn.endswith(".exe")): return OUTPUTDIR+"/bin/"+fn[:-4]
        if (fn.endswith(".lib")): return OUTPUTDIR+"/lib/"+fn[:-4]+".a"
        if (fn.endswith(".ilb")): return OUTPUTDIR+"/tmp/"+fn[:-4]+".a"
        if (fn.endswith(".dat")): return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".in")):  return OUTPUTDIR+"/pandac/input/"+fn
    return fn


def FindLocation(fn, ipath):
    loc = CalcLocation(fn, ipath)
    (base,ext) = os.path.splitext(fn)
    ORIG_EXT[loc] = ext
    return loc

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
########################################################################

class Target:
    pass

TARGET_LIST=[]
TARGET_TABLE={}

def TargetAdd(target, dummy=0, opts=0, input=0, dep=0, ipath=0):
    if (dummy != 0):
        exit("Syntax error in TargetAdd "+target)
    if (ipath == 0): ipath = opts
    if (ipath == 0): ipath = []
    if (type(input) == str): input = [input]
    if (type(dep) == str): dep = [dep]
    full = FindLocation(target,[OUTPUTDIR+"/include"])
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
    ipath = [OUTPUTDIR+"/tmp"] + GetListOption(ipath, "DIR:") + [OUTPUTDIR+"/include"]
    if (opts != 0):
        for x in opts:
            if (t.opts.count(x)==0):
                t.opts.append(x)
    if (input != 0):
        for x in input:
            fullinput = FindLocation(x, ipath)
            t.inputs.append(fullinput)
            t.deps[fullinput] = 1
            (base,suffix) = os.path.splitext(x)
            if (SUFFIX_INC.count(suffix)):
                for d in CxxCalcDependencies(fullinput, ipath, []):
                    t.deps[d] = 1
    if (dep != 0):                
        for x in dep:
            fulldep = FindLocation(x, ipath)
            t.deps[fulldep] = 1
    if (target.endswith(".in")):
        t.deps[FindLocation("interrogate.exe",[])] = 1
        t.deps[FindLocation("dtool_have_python.dat",[])] = 1

