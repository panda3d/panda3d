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

def filedate(path) :
  global FileDateCache
  if (FileDateCache.has_key(path)):
    return(FileDateCache[path]);
  try : date = os.path.getmtime(path)
  except : date = 0;
  FileDateCache[path] = date;
  return(date);

def updatefiledate(path) :
  global FileDateCache
  try : date = os.path.getmtime(path)
  except : date = 0;
  FileDateCache[path] = date;

def youngest(files):
  if (type(files) == str):
    source = filedate(files);
    if (source==0): sys.exit("Error: source file not readable: "+files);
    return(source);
  result = 0;
  for sfile in files:
    source = youngest(sfile);
    if (source > result): result = source;
  return(result);

def debug_older(file,others):
    print [file, others]
    y=youngest(others)
    fd=filedate(file)
    print "youngest", y
    print "filedate", fd
    print "is older", fd<y
    return fd<y

def older(file,others):
  return (filedate(file)<youngest(others));

def xpaths(prefix,base,suffix):
  if (type(base) == str):
    return(prefix + base + suffix);
  result = [];
  for x in base:
    result.append(xpaths(prefix,x,suffix));
  return(result);

if (sys.platform == "win32"):

  import _winreg;
  def GetRegistryKey(path, subkey):
    k1=0
    key=0
    try:
      key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, path, 0, _winreg.KEY_READ);
      k1, k2 = _winreg.QueryValueEx(key, subkey)
    except: pass;
    if (key!=0): _winreg.CloseKey(key);
    return k1;

  def backslashify(exp):
    return exp
    if 0:
        if (type(exp) == str):
          return(string.replace(exp,"/","\\"))
        result = []
        for x in exp: result.append(backslashify(x))
        return(result)

else:

  def backslashify(exp):
    return(exp)

if 0:
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
      os.spawnl(os.P_WAIT, exe, cmd)
    else: os.system(cmd)
    if (cd != "."):
      os.chdir(base)

if 1:
  def getExecutablePath(cmd):
    for i in os.getenv("PATH").split(os.pathsep):
      if os.path.isfile(os.path.join(i, cmd)):
        return os.path.join(i, cmd)
    return cmd

  # This version gives us more control of how the executable is called:
  def oscmd(cmd):
    global VERBOSE
    if VERBOSE >= 1:
      print cmd;
    sys.stdout.flush();
    cmdLine = cmd.split()
    cmd = getExecutablePath(cmdLine[0])
    exitCode = os.spawnv(os.P_WAIT, cmd, cmdLine)
    if exitCode:
      sys.exit("Failed: \"%s\" returned exit code (%s)"%(cmd, exitCode))
else:
  from distutils.spawn import spawn
  # This version seems more "standard" and may be updated
  # without us needing to do it:
  def oscmd(cmd):
    # pring the cmd ourselves rather than using verbose=1
    # on the spawn so that we can flush stdout:
    global VERBOSE
    if VERBOSE >= 1:
      print cmd;
    sys.stdout.flush();
    cmdLine = cmd.split()
    spawn(cmdLine)

def oscdcmd(cd, cmd):
  global VERBOSE
  if VERBOSE >= 1:
    print "cd", cd;
  base=os.getcwd()
  os.chdir(cd)
  oscmd(cmd)
  if VERBOSE >= 1:
    print "cd", base;
  os.chdir(base)

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
  return(building)

def ReadFile(wfile):
  try:
    srchandle = open(wfile, "rb")
    data = srchandle.read()
    srchandle.close()
    return(data)
  except: sys.exit("Cannot read "+wfile)

def WriteFile(wfile,data):
  try:
    dsthandle = open(wfile, "wb")
    dsthandle.write(data)
    dsthandle.close()
  except: sys.exit("Cannot write "+wfile)

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
COMPILER=COMPILERS[0]
OPTIMIZE="3"
INSTALLER=0
COMPLETE=0
THIRDPARTY=""
VERSION1=0
VERSION2=0
VERSION3=0
COMPRESSOR="zlib"

PACKAGES=["ZLIB","PNG","JPEG","TIFF","VRPN","FMOD","NVIDIACG","HELIX","NSPR",
          "SSL","FREETYPE","FFTW","MILES","MAYA5","MAYA6","MAX5","MAX6","MAX7"]
OMIT=PACKAGES[:]
WARNINGS=[]

DirectXSDK=None
VERBOSE=0
##########################################################################################
#
# Read the default version number out of dtool/PandaVersion.pp
#
##########################################################################################

try:
  f = file("dtool/PandaVersion.pp","r")
  pattern = re.compile('^[ \t]*[#][ \t]*define[ \t]+PANDA_VERSION[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)')
  for line in f:
    match = pattern.match(line,0);
    if (match):
      VERSION1 = int(match.group(1))
      VERSION2 = int(match.group(2))
      VERSION3 = int(match.group(3))
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

# Variable                         Windows                   Unix

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
("DEFAULT_PRC_DIR",                '"<auto>"',               '"<auto>"'),
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
("HAVE_MILES",                     'UNDEF',                  'UNDEF'),
("HAVE_SSL",                       'UNDEF',                  'UNDEF'),
("HAVE_NET",                       'UNDEF',                  'UNDEF'),
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
    OPENSSL   Open Secure Socket Layer
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
  print "  --compiler X      (currently, compiler can only be MSVC7,LINUXA)"
  print "  --optimize X      (optimization level can be 1,2,3,4)"
  print "  --thirdparty X    (directory containing third-party software)"
  print "  --complete        (copy models, samples, direct into the build)"
  print "  --installer       (build an executable installer)"
  print "  --v1 X            (set the major version number)"
  print "  --v2 X            (set the minor version number)"
  print "  --v3 X            (set the sequence version number)"
  print "  --lzma            (use lzma compression when building installer)"
  print ""
  for pkg in PACKAGES:
    print "  --use-"+pkg.lower()+"         "[len(pkg.lower()):]+"  --no-"+pkg.lower()+"         "[len(pkg.lower()):]+"(enable/disable use of "+pkg+")"
  print ""
  print "  --nothing         (disable every third-party lib)"
  print "  --everything      (enable every third-party lib)"
  print "  --default         (use default options for everything not specified)"
  print "  --vrdefault       (use default options for the vr studio)"
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
  global COMPILER,OPTIMIZE,OMIT,THIRDPARTY,INSTALLER,COPYEXTRAS,VERSION1,VERSION2,VERSION3,COMPRESSOR
  global DirectXSDK,VERBOSE
  longopts = [
    "help","package-info","compiler=","directx-sdk=","thirdparty=",
    "optimize=","everything","nothing","installer","quiet","verbose",
    "complete","default","v1=","v2=","v3=","lzma"]
  anything = 0
  for pkg in PACKAGES: longopts.append("no-"+pkg.lower())
  for pkg in PACKAGES: longopts.append("use-"+pkg.lower())
  try:
    opts, extras = getopt.getopt(args, "", longopts)
    for option,value in opts:
      if (option=="--help"): raise "usage"
      if (option=="--package-info"): raise "package-info"
      if (option=="--compiler"): COMPILER=value
      if (option=="--directx-sdk"): DirectXSDK=value
      if (option=="--thirdparty"): THIRDPARTY=value
      if (option=="--optimize"): OPTIMIZE=value
      if (option=="--quiet"): VERBOSE-=1
      if (option=="--verbose"): VERBOSE+=1
      if (option=="--installer"): INSTALLER=1
      if (option=="--complete"): COMPLETE=1
      if (option=="--everything"): OMIT=[]
      if (option=="--nothing"): OMIT=PACKAGES[:]
      if (option=="--v1"): VERSION1=int(value)
      if (option=="--v2"): VERSION2=int(value)
      if (option=="--v3"): VERSION3=int(value)
      if (option=="--lzma"): COMPRESSOR="lzma"
      for pkg in PACKAGES:
        if (option=="--use-"+pkg.lower()):
          if (OMIT.count(pkg)): OMIT.delete(pkg)
      for pkg in PACKAGES:
        if (option=="--no-"+pkg.lower()):
          if (OMIT.count(pkg)==0): OMIT.append(pkg)
      anything = 1
  except "package-info": packageInfo()
  except: usage(0)
  if (anything==0): usage(0)
  if   (OPTIMIZE=="1"): OPTIMIZE=1
  elif (OPTIMIZE=="2"): OPTIMIZE=2
  elif (OPTIMIZE=="3"): OPTIMIZE=3
  elif (OPTIMIZE=="4"): OPTIMIZE=4
  else: usage("Invalid setting for OPTIMIZE");
  if (COMPILERS.count(COMPILER)==0): usage("Invalid setting for COMPILER: "+COMPILER);

parseopts(sys.argv[1:])

########################################################################
#
# Locate the root of the panda tree
#
########################################################################

PANDASOURCE=os.path.dirname(os.path.dirname(os.path.abspath(sys.argv[0])))
print "PANDASOURCE:", PANDASOURCE

if ((os.path.exists(os.path.join(PANDASOURCE,"makepanda/makepanda.py"))==0) or
    (os.path.exists(os.path.join(PANDASOURCE,"makepanda/makepanda.sln"))==0) or
    (os.path.exists(os.path.join(PANDASOURCE,"dtool","src","dtoolbase","dtoolbase.h"))==0) or
    (os.path.exists(os.path.join(PANDASOURCE,"panda","src","pandabase","pandabase.h"))==0)):
  sys.exit("I am unable to locate the root of the panda source tree.")
  os.chdir(PANDASOURCE)

########################################################################
##
## Locate the Directory containing the precompiled third-party software,
## if it wasn't specified on the command-line.
##
########################################################################

if (THIRDPARTY == ""):
  if (COMPILER == "MSVC7"): THIRDPARTY="thirdparty\\win-libs-vc7\\"
  if (COMPILER == "LINUXA"): THIRDPARTY="thirdparty/linux-libs-a/"
STDTHIRDPARTY = THIRDPARTY.replace("\\","/")

########################################################################
##
## Locate the DirectX SDK
##
########################################################################

if sys.platform == "win32" and DirectXSDK is None:
  dxdir = GetRegistryKey("SOFTWARE\\Microsoft\\DirectX SDK", "DX9SDK Samples Path")
  if (dxdir != 0): DirectXSDK = os.path.dirname(dxdir)
  else:
    dxdir = GetRegistryKey("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment","DXSDK_DIR")
    if (dxdir != 0): DirectXSDK=dxdir
    else:
      dxdir = GetRegistryKey("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment","DXSDKROOT")
      if dxdir != 0:
        if dxdir[-2:]=="/.":
          DirectXSDK=dxdir[:-1]
        else:
          DirectXSDK=dxdir
      else:
        sys.exit("The registry does not appear to contain a pointer to the DirectX 9.0 SDK.");
  DirectXSDK=DirectXSDK.replace("\\", "/")

########################################################################
##
## Locate the Python SDK (unix only)
##
########################################################################

if sys.platform == "win32":
  if 0: # Needs testing:
    if   (os.path.isdir("C:/Python22")): PythonSDK = "C:/Python22"
    elif (os.path.isdir("C:/Python23")): PythonSDK = "C:/Python23"
    elif (os.path.isdir("C:/Python24")): PythonSDK = "C:/Python24"
    elif (os.path.isdir("C:/Python25")): PythonSDK = "C:/Python25"
    else: sys.exit("Cannot find the python SDK")
else:
  if   (os.path.isdir("/usr/include/python2.2")): PythonSDK = "/usr/include/python2.2"
  elif (os.path.isdir("/usr/include/python2.3")): PythonSDK = "/usr/include/python2.3"
  elif (os.path.isdir("/usr/include/python2.4")): PythonSDK = "/usr/include/python2.4"
  elif (os.path.isdir("/usr/include/python2.5")): PythonSDK = "/usr/include/python2.5"
  else: sys.exit("Cannot find the python SDK")

########################################################################
##
## Locate the Maya 5.0 SDK
##
########################################################################

if (OMIT.count("MAYA5")==0):
  if (sys.platform == "win32"):
    Maya5SDK = GetRegistryKey("SOFTWARE\\Alias|Wavefront\\Maya\\5.0\\Setup\\InstallPath", "MAYA_INSTALL_LOCATION")
    if (Maya5SDK == 0):
      WARNINGS.append("The registry does not appear to contain a pointer to the Maya 5 SDK.")
      WARNINGS.append("I have automatically added this command-line option: --no-maya5")
      OMIT.append("MAYA5")
  else:
    WARNINGS.append("MAYA5 not yet supported under linux")
    WARNINGS.append("I have automatically added this command-line option: --no-maya5")
    OMIT.append("MAYA5")

########################################################################
##
## Locate the Maya 6.0 SDK
##
########################################################################

if (OMIT.count("MAYA6")==0):
  if (sys.platform == "win32"):
    Maya6SDK = GetRegistryKey("SOFTWARE\\Alias|Wavefront\\Maya\\6.0\\Setup\\InstallPath", "MAYA_INSTALL_LOCATION")
    if (Maya6SDK == 0):
      WARNINGS.append("The registry does not appear to contain a pointer to the Maya 6 SDK.")
      WARNINGS.append("I have automatically added this command-line option: --no-maya6")
      OMIT.append("MAYA6")
  else:
    WARNINGS.append("MAYA6 not yet supported under linux")
    WARNINGS.append("I have automatically added this command-line option: --no-maya6")
    OMIT.append("MAYA6")

########################################################################
##
## Locate the 3D Studio Max and Character Studio SDKs
##
########################################################################

MAXVERSIONS = [("MAX5", "SOFTWARE\\Autodesk\\3DSMAX\\5.0\\MAX-1:409", "uninstallpath", "Cstudio\\Sdk"),
               ("MAX6", "SOFTWARE\\Autodesk\\3DSMAX\\6.0",            "installdir",    "maxsdk\\cssdk\\include"),
               ("MAX7", "SOFTWARE\\Autodesk\\3DSMAX\\7.0",            "Installdir",    "maxsdk\\include\\CS")]
MAXSDK = {}
MAXSDKCS = {}
for version,key1,key2,subdir in MAXVERSIONS:
  if (OMIT.count(version)==0):
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
          MAXSDK[version] = top + "maxsdk\\"
          MAXSDKCS[version] = top + subdir
    else:
      WARNINGS.append(version+" not yet supported under linux")
      WARNINGS.append("I have automatically added this command-line option: --no-"+version.lower())
      OMIT.append(version)

########################################################################
##
## Locate Visual Studio 7.0 or 7.1
##
## The visual studio compiler doesn't work unless you set up a
## couple of environment variables to point at the compiler.
##
########################################################################

if (COMPILER == "MSVC7"):
  vcdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\7.1", "InstallDir");
  if ((vcdir == 0) or (vcdir[-13:] != "\\Common7\\IDE\\")):
    vcdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\7.0", "InstallDir");
    if ((vcdir == 0) or (vcdir[-13:] != "\\Common7\\IDE\\")):
      sys.exit("The registry does not appear to contain a pointer to the Visual Studio 7 install directory");
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
# Disable Miles (until we implement support for it)
#
##########################################################################################

if (OMIT.count("MILES")==0):
  WARNINGS.append("Miles audio not yet supported by makepanda")
  WARNINGS.append("I have automatically added this command-line option: --no-miles")
  OMIT.append("MILES")

##########################################################################################
#
# Enable or Disable runtime debugging mechanisms based on optimize level.
#
##########################################################################################

DTOOLCONFIG["HAVE_NET"] = DTOOLCONFIG["HAVE_NSPR"]

if (OPTIMIZE <= 3):
  if (DTOOLCONFIG["HAVE_NET"] != 'UNDEF'):
    DTOOLCONFIG["DO_PSTATS"] = '1'

if (OPTIMIZE <= 3):
  DTOOLCONFIG["DO_COLLISION_RECORDING"] = '1'

if (OPTIMIZE <= 2):
  DTOOLCONFIG["TRACK_IN_INTERPRETER"] = '1'

if (OPTIMIZE <= 3):
  DTOOLCONFIG["DO_MEMORY_USAGE"] = '1'

if (OPTIMIZE <= 1):
  DTOOLCONFIG["DO_PIPELINING"] = '1'

if (OPTIMIZE <= 3):
  DTOOLCONFIG["NOTIFY_DEBUG"] = '1'

##########################################################################################
#
# See if we're using SSL, and if so, which version.
#
##########################################################################################


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
      else                 : tomit = tomit + x + " "
    print "Makepanda: Compiler:",COMPILER
    print "Makepanda: Optimize:",OPTIMIZE
    print "Makepanda: Keep Pkg:",tkeep
    print "Makepanda: Omit Pkg:",tomit
    print "Makepanda: Thirdparty dir:",STDTHIRDPARTY
    print "Makepanda: DirectX SDK dir:",DirectXSDK
    print "Makepanda: Verbose vs. Quiet Level:",VERBOSE
    print "Makepanda: Build installer:",INSTALLER,COMPRESSOR
    print "Makepanda: Version ID: "+str(VERSION1)+"."+str(VERSION2)+"."+str(VERSION3)
    for x in warnings: print "Makepanda: "+x
    print "-------------------------------------------------------------------"
    print ""
    sys.stdout.flush()

printStatus("Makepanda Initial Status Report", WARNINGS)

########################################################################
##
## PkgSelected(package-list,package)
##
## This function returns true if the package-list contains the
## package, AND the OMIT list does not contain the package.
##
########################################################################

def PkgSelected(pkglist, pkg):
  if (pkglist.count(pkg)==0): return(0);
  if (OMIT.count(pkg)): return(0);
  return(1);

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

try: icache = open("makepanda-icache",'rb')
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
  except: sys.exit("Cannot open source file \""+path+"\" for reading.");
  include = []
  for line in sfile:
    match = CxxIncludeRegex.match(line,0);
    if (match):
      incname = match.group(1)
      include.append(incname)
  sfile.close();
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
    full = srcdir + incfile;
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
  if (ignore.count(srcfile)): return([]);
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
  result = dep.keys();
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
    rfile = open(dest, 'rb');
    contents = rfile.read(-1);
    rfile.close();
  except: contents=0;
  if (contents != desiredcontents):
    print "Regenerating file: "+dest
    sys.stdout.flush()
    WriteFile(dest,desiredcontents)

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
    dstfile = dstdir + fn;
  if (older(dstfile,srcfile)):
    global VERBOSE
    if VERBOSE >= 1:
      print "Copying "+srcfile+" -> "+dstfile+"..."
    WriteFile(dstfile,ReadFile(srcfile))
    updatefiledate(dstfile);
  ALLTARGETS.append(dstfile)

########################################################################
##
## CopyAllFiles
##
## Copy the contents of an entire directory into the build tree.
##
########################################################################

def CopyAllFiles(dstdir,srcdir):
  files = os.listdir(srcdir)
  for x in files:
    if (os.path.isfile(srcdir+x)):
      CopyFile(dstdir+x, srcdir+x)

########################################################################
##
## CopyTree
##
## Copy a directory into the build tree.
##
########################################################################

def CopyTree(dstdir,srcdir):
  if (os.path.isdir(dstdir)): return(0);
  if (COMPILER=="MSVC7"): cmd = "xcopy.exe /I/Y/E/Q \""+srcdir+"\" \""+dstdir+"\""
  if (COMPILER=="LINUXA"): cmd = "cp --recursive --force "+srcdir+" "+dstdir
  oscmd(cmd)
  updatefiledate(dstdir)

########################################################################
##
## CompileBison
##
## Generate a CXX file from a source YXX file.
##
########################################################################

def CompileBison(pre,dstc,dsth,src):
  (base, fn) = os.path.split(src)
  dstc=base+"/"+dstc
  dsth=base+"/"+dsth
  if (older(dstc,src) or older(dsth,src)):
    CopyFile("built/tmp/", src)
    if (COMPILER=="MSVC7"):
      CopyFile("built/tmp/", STDTHIRDPARTY+"win-util/bison.simple")
      bisonFullPath=os.path.abspath(STDTHIRDPARTY+"win-util/bison.exe")
      oscdcmd("built/tmp", bisonFullPath+" -y -d -p " + pre + " " + fn)
      osmove("built/tmp/y_tab.c", dstc)
      osmove("built/tmp/y_tab.h", dsth)
    if (COMPILER=="LINUXA"):
      oscdcmd("built/tmp", "bison -y -d -p "+pre+" "+fn)
      osmove("built/tmp/y.tab.c", dstc)
      osmove("built/tmp/y.tab.h", dsth)
    updatefiledate(dstc);
    updatefiledate(dsth);

########################################################################
##
## CompileFlex
##
## Generate a CXX file from a source LXX file.
##
########################################################################

def CompileFlex(pre,dst,src,dashi):
  last = src.rfind("/")
  fn = src[last+1:]
  dst = "built/tmp/"+dst
  if (older(dst,src)):
    CopyFile("built/tmp/", src)
    if (COMPILER=="MSVC7"):
      flexFullPath=os.path.abspath(STDTHIRDPARTY+"win-util/flex.exe")
      if (dashi): oscdcmd("built/tmp", flexFullPath+" -i -P" + pre + " -olex.yy.c " + fn)
      else      : oscdcmd("built/tmp", flexFullPath+"    -P" + pre + " -olex.yy.c " + fn)
      replaceInFile('built/tmp/lex.yy.c', dst, '#include <unistd.h>', '')
      #WriteFile(wdst, ReadFile("built\\tmp\\lex.yy.c").replace("#include <unistd.h>",""))
    if (COMPILER=="LINUXA"):
      if (dashi): oscdcmd("built/tmp", "flex -i -P" + pre + " -olex.yy.c " + fn)
      else      : oscdcmd("built/tmp", "flex    -P" + pre + " -olex.yy.c " + fn)
      oscmd('cp built/tmp/lex.yy.c '+dst)
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
    print "\nStaring compile in \"%s\":\n"%(path,)
  priorIPath=path

def CompileC(obj=0,src=0,ipath=[],opts=[]):
  if ((obj==0)|(src==0)): sys.exit("syntax error in CompileC directive");
  ipath = ["built/tmp"] + ipath + ["built/include"]
  fullsrc = CxxFindSource(src, ipath)
  if (fullsrc == 0): sys.exit("Cannot find source file "+src)
  dep = CxxCalcDependencies(fullsrc, ipath, [])

  if (COMPILER=="MSVC7"):
    wobj = "built/tmp/"+obj
    if (older(wobj, dep)):
      global VERBOSE
      if VERBOSE >= 0:
        checkIfNewDir(ipath[1])
      cmd = 'cl.exe /Fo"' + wobj + '" /nologo /c';
      cmd = cmd + " /I\"built/python/include\""
      if (opts.count("DXSDK")): cmd = cmd + ' /I"' + DirectXSDK + '/include"'
      if (opts.count("MAYA5")): cmd = cmd + ' /I"' + Maya5SDK + 'include"'
      if (opts.count("MAYA6")): cmd = cmd + ' /I"' + Maya6SDK + 'include"'
      for max in ["MAX5","MAX6","MAX7"]:
        if (PkgSelected(opts,max)):
          cmd = cmd + ' /I"' + MAXSDK[max] + 'include" /I"' + MAXSDKCS[max] + '" /D' + max
      for pkg in PACKAGES:
        if (pkg != "MAYA5") and (pkg != "MAYA6") and PkgSelected(opts,pkg):
          cmd = cmd + ' /I"' + THIRDPARTY + pkg.lower() + "/include" + '"'
      for x in ipath: cmd = cmd + " /I \"" + x + "\"";
      if (opts.count('NOFLOATWARN')): cmd = cmd + ' /wd4244 /wd4305'
      if (opts.count("WITHINPANDA")): cmd = cmd + ' /DWITHIN_PANDA'
      if (OPTIMIZE==1): cmd = cmd + " /D_DEBUG /Zc:forScope /MDd /Zi /RTCs /GS "
      if (OPTIMIZE==2): cmd = cmd + " /D_DEBUG /Zc:forScope /MDd /Zi /RTCs /GS "
      if (OPTIMIZE==3): cmd = cmd + " /Zc:forScope /MD /O2 /Ob2 /G6 /Zi /DFORCE_INLINING "
      if (OPTIMIZE==4): cmd = cmd + " /Zc:forScope /MD /O2 /Ob2 /G6 /GL /Zi /DFORCE_INLINING /DNDEBUG "
      cmd = cmd + " /Fd\"" + wobj[:-4] + ".pdb\"";
      building = buildingwhat(opts)
      if (building): cmd = cmd + " /DBUILDING_"+building
      cmd = cmd + " /EHsc /Zm300 /DWIN32_VC /DWIN32 /W3 \"" + fullsrc + "\""
      oscmd(cmd)
      updatefiledate(wobj)

  if (COMPILER=="LINUXA"):
    wobj = "built/tmp/" + obj[:-4] + ".o"
    if (older(wobj, dep)):
      global VERBOSE
      if VERBOSE >= 0:
        checkIfNewDir(ipath[1])
      if (src[-2:]==".c"): cmd = "gcc -c -o "+wobj
      else:                cmd = "g++ -ftemplate-depth-30 -c -o "+wobj
      cmd = cmd + ' -I"' + PythonSDK + '"'
      if (PkgSelected(opts,"VRPN")):     cmd = cmd + ' -I"' + THIRDPARTY + 'vrpn/include"'
      if (PkgSelected(opts,"FFTW")):     cmd = cmd + ' -I"' + THIRDPARTY + 'fftw/include"'
      if (PkgSelected(opts,"FMOD")):     cmd = cmd + ' -I"' + THIRDPARTY + 'fmod/include"'
      if (PkgSelected(opts,"NVIDIACG")): cmd = cmd + ' -I"' + THIRDPARTY + 'nvidiacg/include"'
      if (PkgSelected(opts,"NSPR")):     cmd = cmd + ' -I"' + THIRDPARTY + 'nspr/include"'
      if (PkgSelected(opts,"FREETYPE")): cmd = cmd + ' -I/usr/include/freetype2'
      for x in ipath: cmd = cmd + ' -I"' + x + '"'
      if (opts.count("WITHINPANDA")): cmd = cmd + ' -DWITHIN_PANDA'
      if (OPTIMIZE==1): cmd = cmd + " -g"
      if (OPTIMIZE==2): cmd = cmd + " -O1"
      if (OPTIMIZE==3): cmd = cmd + " -O2"
      if (OPTIMIZE==4): cmd = cmd + " -O2"
      building = buildingwhat(opts)
      if (building): cmd = cmd + " -DBUILDING_" + building
      cmd = cmd + " " + fullsrc
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
  if ((obj==0)|(src==0)): sys.exit("syntax error in CompileRES directive");
  fullsrc = CxxFindSource(src, ipath)
  if (fullsrc == 0): sys.exit("Cannot find source file "+src)
  obj = "built/tmp/"+obj
  wdep = CxxCalcDependencies(fullsrc, ipath, [])

  if (COMPILER=="MSVC7"):
    if (older(obj, wdep)):
      cmd = 'rc.exe /d "NDEBUG" /l 0x409'
      for x in ipath: cmd = cmd + " /I " + x;
      cmd = cmd + ' /fo"' + obj + '"'
      cmd = cmd + ' "'+ fullsrc + '"'
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
    sys.exit("syntax error in Interrogate directive");
  ipath = ["built/tmp"] + ipath + ["built/include"]
  outd = "built/etc/"+outd
  outc = "built/tmp/"+outc
  paths = xpaths(src+"/",files,"")
  dep = CxxCalcDependenciesAll(paths, ipath)
  dotdots = ""
  for i in range(0,src.count("/")+1): dotdots = dotdots + "../"
  ALLIN.append(outd)
  building = 0;
  for x in opts:
    if (x[:9]=="BUILDING_"): building = x[9:]
  if (older(outc, dep) or older(outd, dep)):
    if (COMPILER=="MSVC7"):
      cmd = dotdots + "built/bin/interrogate.exe"
      cmd = cmd + ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -longlong __int64 -D_X86_ -DWIN32_VC -D_WIN32'
      cmd = cmd + ' -D"_declspec(param)=" -D_near -D_far -D__near -D__far -D__stdcall'
      if (OPTIMIZE==1): cmd = cmd + ' '
      if (OPTIMIZE==2): cmd = cmd + ' '
      if (OPTIMIZE==3): cmd = cmd + ' -DFORCE_INLINING'
      if (OPTIMIZE==4): cmd = cmd + ' -DFORCE_INLINING'
      cmd = cmd + ' -S"' + dotdots + 'built/include/parser-inc"'
      cmd = cmd + ' -I"' + dotdots + 'built/python/include"'
    if (COMPILER=="LINUXA"):
      cmd = dotdots + "built/bin/interrogate"
      cmd = cmd + ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -D__i386__ -D__const=const'
      if (OPTIMIZE==1): cmd = cmd + ' '
      if (OPTIMIZE==2): cmd = cmd + ' '
      if (OPTIMIZE==3): cmd = cmd + ' '
      if (OPTIMIZE==4): cmd = cmd + ' '
      cmd = cmd + ' -S"' + dotdots + 'built/include/parser-inc" -S"/usr/include"'
      cmd = cmd + ' -I"' + dotdots + 'built/python/include"'
    cmd = cmd + " -oc "+dotdots+outc+" -od "+dotdots+outd
    cmd = cmd + ' -fnames -string -refcount -assert -python'
    for x in ipath: cmd = cmd + ' -I"' + dotdots + x + '"'
    if (building): cmd = cmd + " -DBUILDING_"+building
    if (opts.count("WITHINPANDA")): cmd = cmd + " -DWITHIN_PANDA"
    for pkg in PACKAGES:
      if (PkgSelected(opts,pkg)):
        cmd = cmd + ' -I"' + dotdots + STDTHIRDPARTY + pkg.lower() + "/include" + '"'
    cmd = cmd + ' -module "' + module + '" -library "' + library + '"'
    if ((COMPILER=="MSVC7") and opts.count("DXSDK")): cmd = cmd + ' -I"' + DirectXSDK + '/include"'
    if ((COMPILER=="MSVC7") and opts.count("MAYA5")): cmd = cmd + ' -I"' + Maya5SDK + 'include"'
    if ((COMPILER=="MSVC7") and opts.count("MAYA6")): cmd = cmd + ' -I"' + Maya6SDK + 'include"'
    for x in files: cmd = cmd + ' ' + x
    oscdcmd(src, cmd)
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
    sys.exit("syntax error in InterrogateModule directive");
  woutc = backslashify("built/tmp/"+outc)
  wfiles = backslashify(xpaths("built/etc/",files,""))
  if (older(woutc, wfiles)):
    if (COMPILER=="MSVC7"):
        cmd = "built\\bin\\interrogate_module.exe "
    if (COMPILER=="LINUXA"):
        cmd = "built/bin/interrogate_module "
    cmd = cmd + " -oc " + woutc + ' -module "' + module + '" -library "' + library + '" -python '
    for x in wfiles: cmd = cmd + ' ' + x
    oscmd(cmd)
    updatefiledate(woutc);

########################################################################
##
## CompileLIB
##
## Generate a LIB file from a bunch of OBJ files.
##
########################################################################

def CompileLIB(lib=0, obj=[], opts=[]):
  if (lib==0): sys.exit("syntax error in CompileLIB directive");

  if (COMPILER=="MSVC7"):
    wlib = "built\\lib\\" + lib
    wobj = xpaths("built\\tmp\\",obj,"")
    ALLTARGETS.append(wlib)
    if (older(wlib, wobj)):
      cmd = 'lib.exe /nologo /OUT:'+wlib;
      if (OPTIMIZE==4): cmd = cmd + " /LTCG "
      for x in wobj: cmd=cmd+" "+x;
      oscmd(cmd)
      updatefiledate(wlib);

  if (COMPILER=="LINUXA"):
    wlib = "built/lib/" + lib[:-4] + ".a"
    wobj = []
    for x in obj: wobj.append("built/tmp/" + x[:-4] + ".o")
    if (older(wlib, wobj)):
      cmd = "ar cru " + wlib
      for x in wobj: cmd=cmd+" "+x;
      oscmd(cmd)
      updatefiledate(wlib);

########################################################################
##
## CompileLink
##
## Generate a DLL or EXE file from a bunch of OBJ and LIB files.
##
########################################################################

def CompileLink(dll=0, obj=[], opts=[], xdep=[]):
  if (dll==0): sys.exit("Syntax error in CompileLink directive");

  if (COMPILER=="MSVC7"):
    ALLTARGETS.append("built/bin/"+dll)
    wdll = backslashify("built/bin/"+dll)
    wlib = backslashify("built/lib/"+dll[:-4]+".lib")
    wobj = []
    for x in obj:
      suffix = x[-4:]
      if   (suffix==".obj"): wobj.append("built\\tmp\\"+x)
      elif (suffix==".dll"): wobj.append("built\\lib\\"+x[:-4]+".lib")
      elif (suffix==".lib"): wobj.append("built\\lib\\"+x)
      elif (suffix==".res"): wobj.append("built\\tmp\\"+x)
      else: sys.exit("unknown suffix in object list.")
    if (older(wdll, wobj+backslashify(xdep))):
      cmd = 'link.exe /nologo /NODEFAULTLIB:LIBCI.LIB'
      if (dll[-4:-1]==".dl"): cmd = cmd + " /DLL"
      if (OPTIMIZE==1): cmd = cmd + " /DEBUG /NODEFAULTLIB:MSVCRT.LIB "
      if (OPTIMIZE==2): cmd = cmd + " /DEBUG /NODEFAULTLIB:MSVCRT.LIB "
      if (OPTIMIZE==3): cmd = cmd + " /DEBUG /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF "
      if (OPTIMIZE==4): cmd = cmd + " /DEBUG /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF /LTCG "
      cmd = cmd + " /MAP /MAPINFO:EXPORTS /MAPINFO:LINES /fixed:no /incremental:no /stack:4194304 "
      if (opts.count("NOLIBCI")): cmd = cmd + " /NODEFAULTLIB:LIBCI.LIB ";
      if (PkgSelected(opts,"MAX5") or PkgSelected(opts,"MAX6")
          or PkgSelected(opts,"MAX7")):
        cmd = cmd + ' /DEF:".\\pandatool\\src\\maxegg\\MaxEgg.def" '
      cmd = cmd + " /OUT:" + wdll + " /IMPLIB:" + wlib + " /MAP:NUL"
      cmd = cmd + " /LIBPATH:built\\python\\libs "
      for x in wobj: cmd = cmd + " " + x
      if (opts.count("D3D8") or opts.count("D3D9") or opts.count("DXDRAW") or opts.count("DXSOUND") or opts.count("DXGUID")):
        cmd = cmd + ' /LIBPATH:"' + DirectXSDK + 'lib/x86"'
        cmd = cmd + ' /LIBPATH:"' + DirectXSDK + 'lib"'
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
      if (PkgSelected(opts,"ZLIB")):     cmd = cmd + " " + THIRDPARTY + 'zlib/lib/libz.lib'
      if (PkgSelected(opts,"PNG")):      cmd = cmd + " " + THIRDPARTY + 'png/lib/libpng.lib'
      if (PkgSelected(opts,"JPEG")):     cmd = cmd + " " + THIRDPARTY + 'jpeg/lib/libjpeg.lib'
      if (PkgSelected(opts,"TIFF")):     cmd = cmd + " " + THIRDPARTY + 'tiff/lib/libtiff.lib'
      if (PkgSelected(opts,"VRPN")):     cmd = cmd + " " + THIRDPARTY + 'vrpn/lib/vrpn.lib'
      if (PkgSelected(opts,"VRPN")):     cmd = cmd + " " + THIRDPARTY + 'vrpn/lib/quat.lib'
      if (PkgSelected(opts,"FMOD")):     cmd = cmd + " " + THIRDPARTY + 'fmod/lib/fmod.lib'
      if (PkgSelected(opts,"MILES")):    cmd = cmd + " " + THIRDPARTY + 'miles/lib/mss32.lib'
      if (PkgSelected(opts,"NVIDIACG")):
        if (opts.count("CGGL")): cmd = cmd + " " + THIRDPARTY + 'nvidiacg/lib/cgGL.lib'
        cmd = cmd + " " + THIRDPARTY + 'nvidiacg/lib/cg.lib'
      if (PkgSelected(opts,"HELIX")):    cmd = cmd + " " + THIRDPARTY + 'helix/lib/runtlib.lib'
      if (PkgSelected(opts,"HELIX")):    cmd = cmd + " " + THIRDPARTY + 'helix/lib/syslib.lib'
      if (PkgSelected(opts,"HELIX")):    cmd = cmd + " " + THIRDPARTY + 'helix/lib/contlib.lib'
      if (PkgSelected(opts,"HELIX")):    cmd = cmd + " " + THIRDPARTY + 'helix/lib/debuglib.lib'
      if (PkgSelected(opts,"HELIX")):    cmd = cmd + " " + THIRDPARTY + 'helix/lib/utillib.lib'
      if (PkgSelected(opts,"HELIX")):    cmd = cmd + " " + THIRDPARTY + 'helix/lib/stlport_vc7.lib'
      if (PkgSelected(opts,"NSPR")):     cmd = cmd + " " + THIRDPARTY + 'nspr/lib/libnspr4.lib'
      if (PkgSelected(opts,"OPENSSL")):  cmd = cmd + " " + THIRDPARTY + 'openssl/lib/ssleay32.lib'
      if (PkgSelected(opts,"OPENSSL")):  cmd = cmd + " " + THIRDPARTY + 'openssl/lib/libeay32.lib'
      if (PkgSelected(opts,"FREETYPE")): cmd = cmd + " " + THIRDPARTY + 'freetype/lib/libfreetype.lib'
      if (PkgSelected(opts,"FFTW")):     cmd = cmd + " " + THIRDPARTY + 'fftw/lib/rfftw.lib'
      if (PkgSelected(opts,"FFTW")):     cmd = cmd + " " + THIRDPARTY + 'fftw/lib/fftw.lib'
      if (PkgSelected(opts,"MAYA5")):    cmd = cmd + ' "' + Maya5SDK +  'lib/Foundation.lib"'
      if (PkgSelected(opts,"MAYA5")):    cmd = cmd + ' "' + Maya5SDK +  'lib/OpenMaya.lib"'
      if (PkgSelected(opts,"MAYA5")):    cmd = cmd + ' "' + Maya5SDK +  'lib/OpenMayaAnim.lib"'
      if (PkgSelected(opts,"MAYA6")):    cmd = cmd + ' "' + Maya6SDK +  'lib/Foundation.lib"'
      if (PkgSelected(opts,"MAYA6")):    cmd = cmd + ' "' + Maya6SDK +  'lib/OpenMaya.lib"'
      if (PkgSelected(opts,"MAYA6")):    cmd = cmd + ' "' + Maya6SDK +  'lib/OpenMayaAnim.lib"'
      for max in ["MAX5","MAX6","MAX7"]:
        if PkgSelected(opts,max):
          cmd = cmd + ' "' + MAXSDK[max] +  'lib\\core.lib"'
          cmd = cmd + ' "' + MAXSDK[max] +  'lib\\mesh.lib"'
          cmd = cmd + ' "' + MAXSDK[max] +  'lib\\maxutil.lib"'
          cmd = cmd + ' "' + MAXSDK[max] +  'lib\\paramblk2.lib"'
      if 1:
        oscmd(cmd)
      else:
        WriteFile('built\\tmp\\linkcontrol',cmd)
        print "link.exe "+cmd
        oscmd("link.exe @built\\tmp\\linkcontrol")
      updatefiledate(wdll);
      if ((OPTIMIZE == 1) and (dll[-4:]==".dll")):
        CopyFile(dll[:-4]+"_d.dll", dll);

  if (COMPILER=="LINUXA"):
    ALLTARGETS.append("built/lib/"+dll[:-4]+".so")
    if (dll[-4:]==".exe"): wdll = "built/bin/"+dll[:-4]
    else: wdll = "built/lib/"+dll[:-4]+".so"
    wobj = []
    for x in obj:
      suffix = x[-4:]
      if   (suffix==".obj"): wobj.append("built/tmp/"+x[:-4]+".o")
      elif (suffix==".dll"): wobj.append("built/lib/"+x[:-4]+".so")
      elif (suffix==".lib"): wobj.append("built/lib/"+x[:-4]+".a")
      else: sys.exit("unknown suffix in object list.")
    if (older(wdll, wobj+xdep)):
      if (dll[-4:]==".exe"): cmd = "g++ -o " + wdll + " -Lbuilt/lib"
      else:                  cmd = "g++ -shared -o " + wdll + " -Lbuilt/lib"
      for x in obj:
        suffix = x[-4:]
        if   (suffix==".obj"): cmd = cmd + " built/tmp/"+x[:-4]+".o"
        elif (suffix==".dll"): cmd = cmd + " -l" + x[3:-4]
        elif (suffix==".lib"): cmd = cmd + " built/lib/"+x[:-4]+".a"
      if (PkgSelected(opts,"FMOD")):     cmd = cmd + ' -L"' + THIRDPARTY + 'fmod/lib" -lfmod-3.74'
      if (PkgSelected(opts,"NVIDIACG")):
        cmd = cmd + ' -L"' + THIRDPARTY + 'nvidiacg/lib" '
        if (opts.count("CGGL")): cmd = cmd + " -lCgGL"
        cmd = cmd + " -lCg"
      if (PkgSelected(opts,"NSPR")):     cmd = cmd + ' -L"' + THIRDPARTY + 'nspr/lib" -lpandanspr4'
      if (PkgSelected(opts,"ZLIB")):     cmd = cmd + " -lz"
      if (PkgSelected(opts,"PNG")):      cmd = cmd + " -lpng"
      if (PkgSelected(opts,"JPEG")):     cmd = cmd + " -ljpeg"
      if (PkgSelected(opts,"TIFF")):     cmd = cmd + " -ltiff"
      if (PkgSelected(opts,"SSL")):      cmd = cmd + " -lssl"
      if (PkgSelected(opts,"FREETYPE")): cmd = cmd + " -lfreetype"
      if (PkgSelected(opts,"VRPN")):     cmd = cmd + ' -L"' + THIRDPARTY + 'vrpn/lib" -lvrpn -lquat'
      if (PkgSelected(opts,"FFTW")):     cmd = cmd + ' -L"' + THIRDPARTY + 'fftw/lib" -lrfftw -lfftw'
      if (opts.count("GLUT")):           cmd = cmd + " -lGL -lGLU"
      oscmd(cmd)
      updatefiledate(wdll);

##########################################################################################
#
# If the 'make depend' process discovers an 'include'
# directive that includes one of the following files,
# the specified file is not added as a dependency,
# nor is it traversed.
#
##########################################################################################

CxxIgnoreHeader["Python.h"] = 1;
CxxIgnoreHeader["Python/Python.h"] = 1;
CxxIgnoreHeader["alloc.h"] = 1;
CxxIgnoreHeader["ctype.h"] = 1;
CxxIgnoreHeader["stdlib.h"] = 1;
CxxIgnoreHeader["ipc_thread.h"] = 1;
CxxIgnoreHeader["platform/symbian/symbian_print.h"] = 1;
CxxIgnoreHeader["hxtypes.h"] = 1;
CxxIgnoreHeader["hxcom.h"] = 1;
CxxIgnoreHeader["hxiids.h"] = 1;
CxxIgnoreHeader["hxpiids.h"] = 1;
CxxIgnoreHeader["dsound.h"] = 1;
CxxIgnoreHeader["hlxosstr.h"] = 1;
CxxIgnoreHeader["ddraw.h"] = 1;
CxxIgnoreHeader["mss.h"] = 1;
CxxIgnoreHeader["MacSocket.h"] = 1;
CxxIgnoreHeader["textureTransition.h"] = 1;
CxxIgnoreHeader["transformTransition.h"] = 1;
CxxIgnoreHeader["billboardTransition.h"] = 1;
CxxIgnoreHeader["transformTransition.h"] = 1;
CxxIgnoreHeader["transparencyTransition.h"] = 1;
CxxIgnoreHeader["allTransitionsWrapper.h"] = 1;
CxxIgnoreHeader["allTransitionsWrapper.h"] = 1;
CxxIgnoreHeader["namedNode.h"] = 1;
CxxIgnoreHeader["renderRelation.h"] = 1;
CxxIgnoreHeader["renderTraverser.h"] = 1;
CxxIgnoreHeader["get_rel_pos.h"] = 1;
CxxIgnoreHeader["Max.h"] = 1;
CxxIgnoreHeader["iparamb2.h"] = 1;
CxxIgnoreHeader["iparamm2.h"] = 1;
CxxIgnoreHeader["istdplug.h"] = 1;
CxxIgnoreHeader["iskin.h"] = 1;
CxxIgnoreHeader["stdmat.h"] = 1;
CxxIgnoreHeader["phyexp.h"] = 1;
CxxIgnoreHeader["bipexp.h"] = 1;
CxxIgnoreHeader["windows.h"] = 1;
CxxIgnoreHeader["windef.h"] = 1;
CxxIgnoreHeader["modstack.h"] = 1;
CxxIgnoreHeader["afxres.h"] = 1;


##########################################################################################
#
# Create the directory tree
#
##########################################################################################

MakeDirectory("built")
MakeDirectory("built/bin")
MakeDirectory("built/lib")
MakeDirectory("built/etc")
MakeDirectory("built/include")
MakeDirectory("built/include/parser-inc")
MakeDirectory("built/include/parser-inc/openssl")
MakeDirectory("built/include/parser-inc/Cg")
MakeDirectory("built/include/openssl")
MakeDirectory("built/direct")
MakeDirectory("built/tmp")

##########################################################################################
#
# Generate pandaVersion.h
#
##########################################################################################

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
conf = conf.replace("NVERSION",str(VERSION1*1000000+VERSION2*1000+VERSION3))

ConditionalWriteFile('built/include/pandaVersion.h',conf);

conf="""
# include "dtoolbase.h"
EXPCL_DTOOL int panda_version_VERSION1_VERSION2_VERSION3 = 0;
"""

conf = conf.replace("VERSION1",str(VERSION1))
conf = conf.replace("VERSION2",str(VERSION2))
conf = conf.replace("VERSION3",str(VERSION3))
conf = conf.replace("NVERSION",str(VERSION1*1000000+VERSION2*1000+VERSION3))

ConditionalWriteFile('built/include/checkPandaVersion.cxx',conf);

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
conf = conf.replace("NVERSION",str(VERSION1*1000000+VERSION2*1000+VERSION3))

ConditionalWriteFile('built/include/checkPandaVersion.h',conf);

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
ConditionalWriteFile('built/direct/__init__.py', DIRECTINIT)

##########################################################################################
#
# Generate dtool_have_xxx.dat
#
##########################################################################################

for x in PACKAGES:
  if (OMIT.count(x)): ConditionalWriteFile('built/tmp/dtool_have_'+x.lower()+'.dat',"0\n")
  else:               ConditionalWriteFile('built/tmp/dtool_have_'+x.lower()+'.dat',"1\n")

##########################################################################################
#
# Generate dtool_config.h
#
##########################################################################################

conf = "/* dtool_config.h.  Generated automatically by makepanda.py */\n"
for key,win,unix in DTOOLDEFAULTS:
  val = DTOOLCONFIG[key]
  if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
  else               : conf = conf + "#define " + key + " " + val + "\n"
ConditionalWriteFile('built/include/dtool_config.h',conf);

##########################################################################################
#
# Copy the config file into the build
#
##########################################################################################

CopyFile('built/', 'Config.prc')

##########################################################################################
#
# Copy the precompiled binaries and DLLs into the build.
#
##########################################################################################

for pkg in PACKAGES:
  if (OMIT.count(pkg)==0):
    if (sys.platform == "win32"):
      if (os.path.exists(backslashify(STDTHIRDPARTY + pkg.lower() + "/bin"))):
        CopyAllFiles("built/bin/", STDTHIRDPARTY + pkg.lower() + "/bin/")
    else:
      if (os.path.exists(STDTHIRDPARTY + pkg.lower() + "/lib")):
        CopyAllFiles("built/lib/", STDTHIRDPARTY + pkg.lower() + "/lib/")

if (os.path.exists(backslashify(STDTHIRDPARTY + "extras/bin"))):
  CopyAllFiles("built/bin/", STDTHIRDPARTY + "extras/bin/")
if (sys.platform == "win32"):
  CopyTree('built/python', STDTHIRDPARTY+'win-python')
  CopyFile('built/bin/', STDTHIRDPARTY+'win-python/python22.dll')

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
# Copy header files to the built/include directory.
#
# Are we just copying *ALL* headers into the include directory?
# If so, let's automate this.
#
########################################################################

ConditionalWriteFile('built/include/ctl3d.h', '/* dummy file to make MAX happy */')

CopyAllFiles('built/include/parser-inc/','dtool/src/parser-inc/')
CopyAllFiles('built/include/parser-inc/openssl/','dtool/src/parser-inc/')
CopyFile('built/include/parser-inc/Cg/','dtool/src/parser-inc/cg.h')
CopyFile('built/include/parser-inc/Cg/','dtool/src/parser-inc/cgGL.h')

CopyFile('built/include/','dtool/src/dtoolbase/cmath.I')
CopyFile('built/include/','dtool/src/dtoolbase/cmath.h')
CopyFile('built/include/','dtool/src/dtoolbase/dallocator.T')
CopyFile('built/include/','dtool/src/dtoolbase/dallocator.h')
CopyFile('built/include/','dtool/src/dtoolbase/dtoolbase.h')
CopyFile('built/include/','dtool/src/dtoolbase/dtoolbase_cc.h')
CopyFile('built/include/','dtool/src/dtoolbase/dtoolsymbols.h')
CopyFile('built/include/','dtool/src/dtoolbase/fakestringstream.h')
CopyFile('built/include/','dtool/src/dtoolbase/nearly_zero.h')
CopyFile('built/include/','dtool/src/dtoolbase/stl_compares.I')
CopyFile('built/include/','dtool/src/dtoolbase/stl_compares.h')
CopyFile('built/include/','dtool/src/dtoolbase/pallocator.T')
CopyFile('built/include/','dtool/src/dtoolbase/pallocator.h')
CopyFile('built/include/','dtool/src/dtoolbase/pdeque.h')
CopyFile('built/include/','dtool/src/dtoolbase/plist.h')
CopyFile('built/include/','dtool/src/dtoolbase/pmap.h')
CopyFile('built/include/','dtool/src/dtoolbase/pset.h')
CopyFile('built/include/','dtool/src/dtoolbase/pvector.h')
CopyFile('built/include/','dtool/src/dtoolutil/executionEnvironment.I')
CopyFile('built/include/','dtool/src/dtoolutil/executionEnvironment.h')
CopyFile('built/include/','dtool/src/dtoolutil/filename.I')
CopyFile('built/include/','dtool/src/dtoolutil/filename.h')
CopyFile('built/include/','dtool/src/dtoolutil/load_dso.h')
CopyFile('built/include/','dtool/src/dtoolutil/dSearchPath.I')
CopyFile('built/include/','dtool/src/dtoolutil/dSearchPath.h')
CopyFile('built/include/','dtool/src/dtoolutil/pfstream.h')
CopyFile('built/include/','dtool/src/dtoolutil/pfstream.I')
CopyFile('built/include/','dtool/src/dtoolutil/vector_string.h')
CopyFile('built/include/','dtool/src/dtoolutil/gnu_getopt.h')
CopyFile('built/include/','dtool/src/dtoolutil/pfstreamBuf.h')
CopyFile('built/include/','dtool/src/dtoolutil/vector_src.cxx')
CopyFile('built/include/','dtool/src/dtoolutil/vector_src.h')
CopyFile('built/include/','dtool/src/dtoolutil/pandaSystem.h')
CopyFile('built/include/','dtool/src/prc/config_prc.h')
CopyFile('built/include/','dtool/src/prc/configDeclaration.I')
CopyFile('built/include/','dtool/src/prc/configDeclaration.h')
CopyFile('built/include/','dtool/src/prc/configFlags.I')
CopyFile('built/include/','dtool/src/prc/configFlags.h')
CopyFile('built/include/','dtool/src/prc/configPage.I')
CopyFile('built/include/','dtool/src/prc/configPage.h')
CopyFile('built/include/','dtool/src/prc/configPageManager.I')
CopyFile('built/include/','dtool/src/prc/configPageManager.h')
CopyFile('built/include/','dtool/src/prc/configVariable.I')
CopyFile('built/include/','dtool/src/prc/configVariable.h')
CopyFile('built/include/','dtool/src/prc/configVariableBase.I')
CopyFile('built/include/','dtool/src/prc/configVariableBase.h')
CopyFile('built/include/','dtool/src/prc/configVariableBool.I')
CopyFile('built/include/','dtool/src/prc/configVariableBool.h')
CopyFile('built/include/','dtool/src/prc/configVariableCore.I')
CopyFile('built/include/','dtool/src/prc/configVariableCore.h')
CopyFile('built/include/','dtool/src/prc/configVariableDouble.I')
CopyFile('built/include/','dtool/src/prc/configVariableDouble.h')
CopyFile('built/include/','dtool/src/prc/configVariableEnum.I')
CopyFile('built/include/','dtool/src/prc/configVariableEnum.h')
CopyFile('built/include/','dtool/src/prc/configVariableFilename.I')
CopyFile('built/include/','dtool/src/prc/configVariableFilename.h')
CopyFile('built/include/','dtool/src/prc/configVariableInt.I')
CopyFile('built/include/','dtool/src/prc/configVariableInt.h')
CopyFile('built/include/','dtool/src/prc/configVariableList.I')
CopyFile('built/include/','dtool/src/prc/configVariableList.h')
CopyFile('built/include/','dtool/src/prc/configVariableManager.I')
CopyFile('built/include/','dtool/src/prc/configVariableManager.h')
CopyFile('built/include/','dtool/src/prc/configVariableSearchPath.I')
CopyFile('built/include/','dtool/src/prc/configVariableSearchPath.h')
CopyFile('built/include/','dtool/src/prc/configVariableString.I')
CopyFile('built/include/','dtool/src/prc/configVariableString.h')
CopyFile('built/include/','dtool/src/prc/globPattern.I')
CopyFile('built/include/','dtool/src/prc/globPattern.h')
CopyFile('built/include/','dtool/src/prc/notify.I')
CopyFile('built/include/','dtool/src/prc/notify.h')
CopyFile('built/include/','dtool/src/prc/notifyCategory.I')
CopyFile('built/include/','dtool/src/prc/notifyCategory.h')
CopyFile('built/include/','dtool/src/prc/notifyCategoryProxy.I')
CopyFile('built/include/','dtool/src/prc/notifyCategoryProxy.h')
CopyFile('built/include/','dtool/src/prc/notifySeverity.h')
CopyFile('built/include/','dtool/src/prc/prcKeyRegistry.I')
CopyFile('built/include/','dtool/src/prc/prcKeyRegistry.h')
CopyFile('built/include/','dtool/src/dconfig/configTable.I')
CopyFile('built/include/','dtool/src/dconfig/configTable.h')
CopyFile('built/include/','dtool/src/dconfig/config_dconfig.h')
CopyFile('built/include/','dtool/src/dconfig/config_setup.h')
CopyFile('built/include/','dtool/src/dconfig/dconfig.I')
CopyFile('built/include/','dtool/src/dconfig/dconfig.h')
CopyFile('built/include/','dtool/src/dconfig/serialization.I')
CopyFile('built/include/','dtool/src/dconfig/serialization.h')
CopyFile('built/include/','dtool/src/dconfig/symbolEnt.I')
CopyFile('built/include/','dtool/src/dconfig/symbolEnt.h')
CopyFile('built/include/','dtool/src/interrogatedb/interrogate_interface.h')
CopyFile('built/include/','dtool/src/interrogatedb/interrogate_request.h')
CopyFile('built/include/','dtool/src/interrogatedb/vector_int.h')
CopyFile('built/include/','dtool/src/interrogatedb/config_interrogatedb.h')
CopyFile('built/include/','dtool/src/pystub/pystub.h')
CopyFile('built/include/','dtool/src/prckeys/signPrcFile_src.cxx')
CopyFile('built/include/','panda/src/pandabase/pandabase.h')
CopyFile('built/include/','panda/src/pandabase/pandasymbols.h')
CopyFile('built/include/','panda/src/express/atomicAdjustDummyImpl.h')
CopyFile('built/include/','panda/src/express/atomicAdjustDummyImpl.I')
CopyFile('built/include/','panda/src/express/atomicAdjust.h')
CopyFile('built/include/','panda/src/express/atomicAdjust.I')
CopyFile('built/include/','panda/src/express/atomicAdjustImpl.h')
CopyFile('built/include/','panda/src/express/atomicAdjustNsprImpl.h')
CopyFile('built/include/','panda/src/express/atomicAdjustNsprImpl.I')
CopyFile('built/include/','panda/src/express/bigEndian.h')
CopyFile('built/include/','panda/src/express/buffer.I')
CopyFile('built/include/','panda/src/express/buffer.h')
CopyFile('built/include/','panda/src/express/checksumHashGenerator.I')
CopyFile('built/include/','panda/src/express/checksumHashGenerator.h')
CopyFile('built/include/','panda/src/express/circBuffer.I')
CopyFile('built/include/','panda/src/express/circBuffer.h')
CopyFile('built/include/','panda/src/express/clockObject.I')
CopyFile('built/include/','panda/src/express/clockObject.h')
CopyFile('built/include/','panda/src/express/conditionVarDummyImpl.h')
CopyFile('built/include/','panda/src/express/conditionVarDummyImpl.I')
CopyFile('built/include/','panda/src/express/conditionVar.h')
CopyFile('built/include/','panda/src/express/conditionVar.I')
CopyFile('built/include/','panda/src/express/conditionVarImpl.h')
CopyFile('built/include/','panda/src/express/conditionVarNsprImpl.h')
CopyFile('built/include/','panda/src/express/conditionVarNsprImpl.I')
CopyFile('built/include/','panda/src/express/config_express.h')
CopyFile('built/include/','panda/src/express/datagram.I')
CopyFile('built/include/','panda/src/express/datagram.h')
CopyFile('built/include/','panda/src/express/datagramGenerator.I')
CopyFile('built/include/','panda/src/express/datagramGenerator.h')
CopyFile('built/include/','panda/src/express/datagramIterator.I')
CopyFile('built/include/','panda/src/express/datagramIterator.h')
CopyFile('built/include/','panda/src/express/datagramSink.I')
CopyFile('built/include/','panda/src/express/datagramSink.h')
CopyFile('built/include/','panda/src/express/dcast.T')
CopyFile('built/include/','panda/src/express/dcast.h')
CopyFile('built/include/','panda/src/express/encryptStreamBuf.h')
CopyFile('built/include/','panda/src/express/encryptStreamBuf.I')
CopyFile('built/include/','panda/src/express/encryptStream.h')
CopyFile('built/include/','panda/src/express/encryptStream.I')
CopyFile('built/include/','panda/src/express/error_utils.h')
CopyFile('built/include/','panda/src/express/hashGeneratorBase.I')
CopyFile('built/include/','panda/src/express/hashGeneratorBase.h')
CopyFile('built/include/','panda/src/express/hashVal.I')
CopyFile('built/include/','panda/src/express/hashVal.h')
CopyFile('built/include/','panda/src/express/indent.I')
CopyFile('built/include/','panda/src/express/indent.h')
CopyFile('built/include/','panda/src/express/indirectLess.I')
CopyFile('built/include/','panda/src/express/indirectLess.h')
CopyFile('built/include/','panda/src/express/littleEndian.h')
CopyFile('built/include/','panda/src/express/memoryInfo.I')
CopyFile('built/include/','panda/src/express/memoryInfo.h')
CopyFile('built/include/','panda/src/express/memoryUsage.I')
CopyFile('built/include/','panda/src/express/memoryUsage.h')
CopyFile('built/include/','panda/src/express/memoryUsagePointerCounts.I')
CopyFile('built/include/','panda/src/express/memoryUsagePointerCounts.h')
CopyFile('built/include/','panda/src/express/memoryUsagePointers.I')
CopyFile('built/include/','panda/src/express/memoryUsagePointers.h')
CopyFile('built/include/','panda/src/express/multifile.I')
CopyFile('built/include/','panda/src/express/multifile.h')
CopyFile('built/include/','panda/src/express/mutexDummyImpl.h')
CopyFile('built/include/','panda/src/express/mutexDummyImpl.I')
CopyFile('built/include/','panda/src/express/pmutex.h')
CopyFile('built/include/','panda/src/express/mutexHolder.h')
CopyFile('built/include/','panda/src/express/mutexHolder.I')
CopyFile('built/include/','panda/src/express/pmutex.I')
CopyFile('built/include/','panda/src/express/mutexImpl.h')
CopyFile('built/include/','panda/src/express/mutexNsprImpl.h')
CopyFile('built/include/','panda/src/express/mutexNsprImpl.I')
CopyFile('built/include/','panda/src/express/namable.I')
CopyFile('built/include/','panda/src/express/namable.h')
CopyFile('built/include/','panda/src/express/nativeNumericData.I')
CopyFile('built/include/','panda/src/express/nativeNumericData.h')
CopyFile('built/include/','panda/src/express/numeric_types.h')
CopyFile('built/include/','panda/src/express/ordered_vector.h')
CopyFile('built/include/','panda/src/express/ordered_vector.I')
CopyFile('built/include/','panda/src/express/ordered_vector.T')
CopyFile('built/include/','panda/src/express/password_hash.h')
CopyFile('built/include/','panda/src/express/patchfile.I')
CopyFile('built/include/','panda/src/express/patchfile.h')
CopyFile('built/include/','panda/src/express/pointerTo.I')
CopyFile('built/include/','panda/src/express/pointerTo.h')
CopyFile('built/include/','panda/src/express/pointerToArray.I')
CopyFile('built/include/','panda/src/express/pointerToArray.h')
CopyFile('built/include/','panda/src/express/pointerToBase.I')
CopyFile('built/include/','panda/src/express/pointerToBase.h')
CopyFile('built/include/','panda/src/express/pointerToVoid.I')
CopyFile('built/include/','panda/src/express/pointerToVoid.h')
CopyFile('built/include/','panda/src/express/profileTimer.I')
CopyFile('built/include/','panda/src/express/profileTimer.h')
CopyFile('built/include/','panda/src/express/pta_uchar.h')
CopyFile('built/include/','panda/src/express/ramfile.I')
CopyFile('built/include/','panda/src/express/ramfile.h')
CopyFile('built/include/','panda/src/express/referenceCount.I')
CopyFile('built/include/','panda/src/express/referenceCount.h')
CopyFile('built/include/','panda/src/express/register_type.I')
CopyFile('built/include/','panda/src/express/register_type.h')
CopyFile('built/include/','panda/src/express/reversedNumericData.I')
CopyFile('built/include/','panda/src/express/reversedNumericData.h')
CopyFile('built/include/','panda/src/express/selectThreadImpl.h')
CopyFile('built/include/','panda/src/express/streamReader.I')
CopyFile('built/include/','panda/src/express/streamReader.h')
CopyFile('built/include/','panda/src/express/streamWriter.I')
CopyFile('built/include/','panda/src/express/streamWriter.h')
CopyFile('built/include/','panda/src/express/stringDecoder.h')
CopyFile('built/include/','panda/src/express/stringDecoder.I')
CopyFile('built/include/','panda/src/express/subStream.I')
CopyFile('built/include/','panda/src/express/subStream.h')
CopyFile('built/include/','panda/src/express/subStreamBuf.h')
CopyFile('built/include/','panda/src/express/textEncoder.h')
CopyFile('built/include/','panda/src/express/textEncoder.I')
CopyFile('built/include/','panda/src/express/threadDummyImpl.h')
CopyFile('built/include/','panda/src/express/threadDummyImpl.I')
CopyFile('built/include/','panda/src/express/thread.h')
CopyFile('built/include/','panda/src/express/thread.I')
CopyFile('built/include/','panda/src/express/threadImpl.h')
CopyFile('built/include/','panda/src/express/threadNsprImpl.h')
CopyFile('built/include/','panda/src/express/threadNsprImpl.I')
CopyFile('built/include/','panda/src/express/threadPriority.h')
CopyFile('built/include/','panda/src/express/tokenBoard.I')
CopyFile('built/include/','panda/src/express/tokenBoard.h')
CopyFile('built/include/','panda/src/express/trueClock.I')
CopyFile('built/include/','panda/src/express/trueClock.h')
CopyFile('built/include/','panda/src/express/typeHandle.I')
CopyFile('built/include/','panda/src/express/typeHandle.h')
CopyFile('built/include/','panda/src/express/typedObject.I')
CopyFile('built/include/','panda/src/express/typedObject.h')
CopyFile('built/include/','panda/src/express/typedReferenceCount.I')
CopyFile('built/include/','panda/src/express/typedReferenceCount.h')
CopyFile('built/include/','panda/src/express/typedef.h')
CopyFile('built/include/','panda/src/express/typeRegistry.I')
CopyFile('built/include/','panda/src/express/typeRegistry.h')
CopyFile('built/include/','panda/src/express/typeRegistryNode.I')
CopyFile('built/include/','panda/src/express/typeRegistryNode.h')
CopyFile('built/include/','panda/src/express/unicodeLatinMap.h')
CopyFile('built/include/','panda/src/express/vector_uchar.h')
CopyFile('built/include/','panda/src/express/virtualFileComposite.h')
CopyFile('built/include/','panda/src/express/virtualFileComposite.I')
CopyFile('built/include/','panda/src/express/virtualFile.h')
CopyFile('built/include/','panda/src/express/virtualFile.I')
CopyFile('built/include/','panda/src/express/virtualFileList.I')
CopyFile('built/include/','panda/src/express/virtualFileList.h')
CopyFile('built/include/','panda/src/express/virtualFileMount.h')
CopyFile('built/include/','panda/src/express/virtualFileMount.I')
CopyFile('built/include/','panda/src/express/virtualFileMountMultifile.h')
CopyFile('built/include/','panda/src/express/virtualFileMountMultifile.I')
CopyFile('built/include/','panda/src/express/virtualFileMountSystem.h')
CopyFile('built/include/','panda/src/express/virtualFileMountSystem.I')
CopyFile('built/include/','panda/src/express/virtualFileSimple.h')
CopyFile('built/include/','panda/src/express/virtualFileSimple.I')
CopyFile('built/include/','panda/src/express/virtualFileSystem.h')
CopyFile('built/include/','panda/src/express/virtualFileSystem.I')
CopyFile('built/include/','panda/src/express/weakPointerTo.I')
CopyFile('built/include/','panda/src/express/weakPointerTo.h')
CopyFile('built/include/','panda/src/express/weakPointerToBase.I')
CopyFile('built/include/','panda/src/express/weakPointerToBase.h')
CopyFile('built/include/','panda/src/express/weakPointerToVoid.I')
CopyFile('built/include/','panda/src/express/weakPointerToVoid.h')
CopyFile('built/include/','panda/src/express/weakReferenceList.I')
CopyFile('built/include/','panda/src/express/weakReferenceList.h')
CopyFile('built/include/','panda/src/express/windowsRegistry.h')
CopyFile('built/include/','panda/src/express/zStream.I')
CopyFile('built/include/','panda/src/express/zStream.h')
CopyFile('built/include/','panda/src/express/zStreamBuf.h')
CopyFile('built/include/','panda/src/downloader/asyncUtility.h')
CopyFile('built/include/','panda/src/downloader/asyncUtility.I')
CopyFile('built/include/','panda/src/downloader/bioPtr.I')
CopyFile('built/include/','panda/src/downloader/bioPtr.h')
CopyFile('built/include/','panda/src/downloader/bioStreamPtr.I')
CopyFile('built/include/','panda/src/downloader/bioStreamPtr.h')
CopyFile('built/include/','panda/src/downloader/bioStream.I')
CopyFile('built/include/','panda/src/downloader/bioStream.h')
CopyFile('built/include/','panda/src/downloader/bioStreamBuf.h')
CopyFile('built/include/','panda/src/downloader/chunkedStream.I')
CopyFile('built/include/','panda/src/downloader/chunkedStream.h')
CopyFile('built/include/','panda/src/downloader/chunkedStreamBuf.h')
CopyFile('built/include/','panda/src/downloader/config_downloader.h')
CopyFile('built/include/','panda/src/downloader/decompressor.h')
CopyFile('built/include/','panda/src/downloader/decompressor.I')
CopyFile('built/include/','panda/src/downloader/documentSpec.h')
CopyFile('built/include/','panda/src/downloader/documentSpec.I')
CopyFile('built/include/','panda/src/downloader/download_utils.h')
CopyFile('built/include/','panda/src/downloader/downloadDb.h')
CopyFile('built/include/','panda/src/downloader/downloadDb.I')
CopyFile('built/include/','panda/src/downloader/extractor.h')
CopyFile('built/include/','panda/src/downloader/httpAuthorization.I')
CopyFile('built/include/','panda/src/downloader/httpAuthorization.h')
CopyFile('built/include/','panda/src/downloader/httpBasicAuthorization.I')
CopyFile('built/include/','panda/src/downloader/httpBasicAuthorization.h')
CopyFile('built/include/','panda/src/downloader/httpChannel.I')
CopyFile('built/include/','panda/src/downloader/httpChannel.h')
CopyFile('built/include/','panda/src/downloader/httpClient.I')
CopyFile('built/include/','panda/src/downloader/httpClient.h')
CopyFile('built/include/','panda/src/downloader/httpCookie.I')
CopyFile('built/include/','panda/src/downloader/httpCookie.h')
CopyFile('built/include/','panda/src/downloader/httpDate.I')
CopyFile('built/include/','panda/src/downloader/httpDate.h')
CopyFile('built/include/','panda/src/downloader/httpDigestAuthorization.I')
CopyFile('built/include/','panda/src/downloader/httpDigestAuthorization.h')
CopyFile('built/include/','panda/src/downloader/httpEntityTag.I')
CopyFile('built/include/','panda/src/downloader/httpEntityTag.h')
CopyFile('built/include/','panda/src/downloader/httpEnum.h')
CopyFile('built/include/','panda/src/downloader/identityStream.I')
CopyFile('built/include/','panda/src/downloader/identityStream.h')
CopyFile('built/include/','panda/src/downloader/identityStreamBuf.h')
CopyFile('built/include/','panda/src/downloader/multiplexStream.I')
CopyFile('built/include/','panda/src/downloader/multiplexStream.h')
CopyFile('built/include/','panda/src/downloader/multiplexStreamBuf.I')
CopyFile('built/include/','panda/src/downloader/multiplexStreamBuf.h')
CopyFile('built/include/','panda/src/downloader/patcher.h')
CopyFile('built/include/','panda/src/downloader/patcher.I')
CopyFile('built/include/','panda/src/downloader/socketStream.h')
CopyFile('built/include/','panda/src/downloader/socketStream.I')
CopyFile('built/include/','panda/src/downloader/ssl_utils.h')
CopyFile('built/include/','panda/src/downloader/urlSpec.h')
CopyFile('built/include/','panda/src/downloader/urlSpec.I')
CopyFile('built/include/','panda/src/putil/bam.h')
CopyFile('built/include/','panda/src/putil/bamReader.I')
CopyFile('built/include/','panda/src/putil/bamReader.h')
CopyFile('built/include/','panda/src/putil/bamReaderParam.I')
CopyFile('built/include/','panda/src/putil/bamReaderParam.h')
CopyFile('built/include/','panda/src/putil/bamWriter.I')
CopyFile('built/include/','panda/src/putil/bamWriter.h')
CopyFile('built/include/','panda/src/putil/bitMask.I')
CopyFile('built/include/','panda/src/putil/bitMask.h')
CopyFile('built/include/','panda/src/putil/buttonHandle.I')
CopyFile('built/include/','panda/src/putil/buttonHandle.h')
CopyFile('built/include/','panda/src/putil/buttonRegistry.I')
CopyFile('built/include/','panda/src/putil/buttonRegistry.h')
CopyFile('built/include/','panda/src/putil/collideMask.h')
CopyFile('built/include/','panda/src/putil/portalMask.h')
CopyFile('built/include/','panda/src/putil/compareTo.I')
CopyFile('built/include/','panda/src/putil/compareTo.h')
CopyFile('built/include/','panda/src/putil/config_util.h')
CopyFile('built/include/','panda/src/putil/configurable.h')
CopyFile('built/include/','panda/src/putil/factory.I')
CopyFile('built/include/','panda/src/putil/factory.h')
CopyFile('built/include/','panda/src/putil/cachedTypedWritableReferenceCount.h')
CopyFile('built/include/','panda/src/putil/cachedTypedWritableReferenceCount.I')
CopyFile('built/include/','panda/src/putil/cycleData.h')
CopyFile('built/include/','panda/src/putil/cycleData.I')
CopyFile('built/include/','panda/src/putil/cycleDataReader.h')
CopyFile('built/include/','panda/src/putil/cycleDataReader.I')
CopyFile('built/include/','panda/src/putil/cycleDataWriter.h')
CopyFile('built/include/','panda/src/putil/cycleDataWriter.I')
CopyFile('built/include/','panda/src/putil/datagramInputFile.I')
CopyFile('built/include/','panda/src/putil/datagramInputFile.h')
CopyFile('built/include/','panda/src/putil/datagramOutputFile.I')
CopyFile('built/include/','panda/src/putil/datagramOutputFile.h')
CopyFile('built/include/','panda/src/putil/drawMask.h')
CopyFile('built/include/','panda/src/putil/factoryBase.I')
CopyFile('built/include/','panda/src/putil/factoryBase.h')
CopyFile('built/include/','panda/src/putil/factoryParam.I')
CopyFile('built/include/','panda/src/putil/factoryParam.h')
CopyFile('built/include/','panda/src/putil/factoryParams.I')
CopyFile('built/include/','panda/src/putil/factoryParams.h')
CopyFile('built/include/','panda/src/putil/firstOfPairCompare.I')
CopyFile('built/include/','panda/src/putil/firstOfPairCompare.h')
CopyFile('built/include/','panda/src/putil/firstOfPairLess.I')
CopyFile('built/include/','panda/src/putil/firstOfPairLess.h')
CopyFile('built/include/','panda/src/putil/globalPointerRegistry.I')
CopyFile('built/include/','panda/src/putil/globalPointerRegistry.h')
CopyFile('built/include/','panda/src/putil/indirectCompareNames.I')
CopyFile('built/include/','panda/src/putil/indirectCompareNames.h')
CopyFile('built/include/','panda/src/putil/indirectCompareTo.I')
CopyFile('built/include/','panda/src/putil/indirectCompareTo.h')
CopyFile('built/include/','panda/src/putil/ioPtaDatagramFloat.h')
CopyFile('built/include/','panda/src/putil/ioPtaDatagramInt.h')
CopyFile('built/include/','panda/src/putil/ioPtaDatagramShort.h')
CopyFile('built/include/','panda/src/putil/iterator_types.h')
CopyFile('built/include/','panda/src/putil/keyboardButton.h')
CopyFile('built/include/','panda/src/putil/lineStream.I')
CopyFile('built/include/','panda/src/putil/lineStream.h')
CopyFile('built/include/','panda/src/putil/lineStreamBuf.I')
CopyFile('built/include/','panda/src/putil/lineStreamBuf.h')
CopyFile('built/include/','panda/src/putil/load_prc_file.h')
CopyFile('built/include/','panda/src/putil/modifierButtons.I')
CopyFile('built/include/','panda/src/putil/modifierButtons.h')
CopyFile('built/include/','panda/src/putil/mouseButton.h')
CopyFile('built/include/','panda/src/putil/mouseData.I')
CopyFile('built/include/','panda/src/putil/mouseData.h')
CopyFile('built/include/','panda/src/putil/nameUniquifier.I')
CopyFile('built/include/','panda/src/putil/nameUniquifier.h')
CopyFile('built/include/','panda/src/putil/pipeline.h')
CopyFile('built/include/','panda/src/putil/pipeline.I')
CopyFile('built/include/','panda/src/putil/pipelineCycler.h')
CopyFile('built/include/','panda/src/putil/pipelineCycler.I')
CopyFile('built/include/','panda/src/putil/pipelineCyclerBase.h')
CopyFile('built/include/','panda/src/putil/pipelineCyclerBase.I')
CopyFile('built/include/','panda/src/putil/pta_double.h')
CopyFile('built/include/','panda/src/putil/pta_float.h')
CopyFile('built/include/','panda/src/putil/pta_int.h')
CopyFile('built/include/','panda/src/putil/pta_ushort.h')
CopyFile('built/include/','panda/src/putil/string_utils.I')
CopyFile('built/include/','panda/src/putil/string_utils.h')
CopyFile('built/include/','panda/src/putil/timedCycle.I')
CopyFile('built/include/','panda/src/putil/timedCycle.h')
CopyFile('built/include/','panda/src/putil/typedWritable.I')
CopyFile('built/include/','panda/src/putil/typedWritable.h')
CopyFile('built/include/','panda/src/putil/typedWritableReferenceCount.I')
CopyFile('built/include/','panda/src/putil/typedWritableReferenceCount.h')
CopyFile('built/include/','panda/src/putil/updateSeq.I')
CopyFile('built/include/','panda/src/putil/updateSeq.h')
CopyFile('built/include/','panda/src/putil/uniqueIdAllocator.h')
CopyFile('built/include/','panda/src/putil/vector_double.h')
CopyFile('built/include/','panda/src/putil/vector_float.h')
CopyFile('built/include/','panda/src/putil/vector_typedWritable.h')
CopyFile('built/include/','panda/src/putil/vector_ushort.h')
CopyFile('built/include/','panda/src/putil/vector_writable.h')
CopyFile('built/include/','panda/src/putil/writableConfigurable.h')
CopyFile('built/include/','panda/src/putil/writableParam.I')
CopyFile('built/include/','panda/src/putil/writableParam.h')
CopyFile('built/include/','panda/src/audio/config_audio.h')
CopyFile('built/include/','panda/src/audio/audio.h')
CopyFile('built/include/','panda/src/audio/audioManager.h')
CopyFile('built/include/','panda/src/audio/audioSound.h')
CopyFile('built/include/','panda/src/audio/nullAudioManager.h')
CopyFile('built/include/','panda/src/audio/nullAudioSound.h')
CopyFile('built/include/','panda/src/event/buttonEvent.I')
CopyFile('built/include/','panda/src/event/buttonEvent.h')
CopyFile('built/include/','panda/src/event/buttonEventList.I')
CopyFile('built/include/','panda/src/event/buttonEventList.h')
CopyFile('built/include/','panda/src/event/event.I')
CopyFile('built/include/','panda/src/event/event.h')
CopyFile('built/include/','panda/src/event/eventHandler.h')
CopyFile('built/include/','panda/src/event/eventHandler.I')
CopyFile('built/include/','panda/src/event/eventParameter.I')
CopyFile('built/include/','panda/src/event/eventParameter.h')
CopyFile('built/include/','panda/src/event/eventQueue.I')
CopyFile('built/include/','panda/src/event/eventQueue.h')
CopyFile('built/include/','panda/src/event/eventReceiver.h')
CopyFile('built/include/','panda/src/event/pt_Event.h')
CopyFile('built/include/','panda/src/event/throw_event.I')
CopyFile('built/include/','panda/src/event/throw_event.h')
CopyFile('built/include/','panda/src/linmath/compose_matrix.h')
CopyFile('built/include/','panda/src/linmath/compose_matrix_src.I')
CopyFile('built/include/','panda/src/linmath/compose_matrix_src.h')
CopyFile('built/include/','panda/src/linmath/config_linmath.h')
CopyFile('built/include/','panda/src/linmath/coordinateSystem.h')
CopyFile('built/include/','panda/src/linmath/dbl2fltnames.h')
CopyFile('built/include/','panda/src/linmath/dblnames.h')
CopyFile('built/include/','panda/src/linmath/deg_2_rad.h')
CopyFile('built/include/','panda/src/linmath/flt2dblnames.h')
CopyFile('built/include/','panda/src/linmath/fltnames.h')
CopyFile('built/include/','panda/src/linmath/ioPtaDatagramLinMath.I')
CopyFile('built/include/','panda/src/linmath/ioPtaDatagramLinMath.h')
CopyFile('built/include/','panda/src/linmath/lcast_to.h')
CopyFile('built/include/','panda/src/linmath/lcast_to_src.I')
CopyFile('built/include/','panda/src/linmath/lcast_to_src.h')
CopyFile('built/include/','panda/src/linmath/lmat_ops.h')
CopyFile('built/include/','panda/src/linmath/lmat_ops_src.I')
CopyFile('built/include/','panda/src/linmath/lmat_ops_src.h')
CopyFile('built/include/','panda/src/linmath/lmatrix.h')
CopyFile('built/include/','panda/src/linmath/lmatrix3.h')
CopyFile('built/include/','panda/src/linmath/lmatrix3_src.I')
CopyFile('built/include/','panda/src/linmath/lmatrix3_src.h')
CopyFile('built/include/','panda/src/linmath/lmatrix4.h')
CopyFile('built/include/','panda/src/linmath/lmatrix4_src.I')
CopyFile('built/include/','panda/src/linmath/lmatrix4_src.h')
CopyFile('built/include/','panda/src/linmath/lorientation.h')
CopyFile('built/include/','panda/src/linmath/lorientation_src.I')
CopyFile('built/include/','panda/src/linmath/lorientation_src.h')
CopyFile('built/include/','panda/src/linmath/lpoint2.h')
CopyFile('built/include/','panda/src/linmath/lpoint2_src.I')
CopyFile('built/include/','panda/src/linmath/lpoint2_src.h')
CopyFile('built/include/','panda/src/linmath/lpoint3.h')
CopyFile('built/include/','panda/src/linmath/lpoint3_src.I')
CopyFile('built/include/','panda/src/linmath/lpoint3_src.h')
CopyFile('built/include/','panda/src/linmath/lpoint4.h')
CopyFile('built/include/','panda/src/linmath/lpoint4_src.I')
CopyFile('built/include/','panda/src/linmath/lpoint4_src.h')
CopyFile('built/include/','panda/src/linmath/lquaternion.h')
CopyFile('built/include/','panda/src/linmath/lquaternion_src.I')
CopyFile('built/include/','panda/src/linmath/lquaternion_src.h')
CopyFile('built/include/','panda/src/linmath/lrotation.h')
CopyFile('built/include/','panda/src/linmath/lrotation_src.I')
CopyFile('built/include/','panda/src/linmath/lrotation_src.h')
CopyFile('built/include/','panda/src/linmath/luse.I')
CopyFile('built/include/','panda/src/linmath/luse.h')
CopyFile('built/include/','panda/src/linmath/lvec2_ops.h')
CopyFile('built/include/','panda/src/linmath/lvec2_ops_src.I')
CopyFile('built/include/','panda/src/linmath/lvec2_ops_src.h')
CopyFile('built/include/','panda/src/linmath/lvec3_ops.h')
CopyFile('built/include/','panda/src/linmath/lvec3_ops_src.I')
CopyFile('built/include/','panda/src/linmath/lvec3_ops_src.h')
CopyFile('built/include/','panda/src/linmath/lvec4_ops.h')
CopyFile('built/include/','panda/src/linmath/lvec4_ops_src.I')
CopyFile('built/include/','panda/src/linmath/lvec4_ops_src.h')
CopyFile('built/include/','panda/src/linmath/lvecBase2.h')
CopyFile('built/include/','panda/src/linmath/lvecBase2_src.I')
CopyFile('built/include/','panda/src/linmath/lvecBase2_src.h')
CopyFile('built/include/','panda/src/linmath/lvecBase3.h')
CopyFile('built/include/','panda/src/linmath/lvecBase3_src.I')
CopyFile('built/include/','panda/src/linmath/lvecBase3_src.h')
CopyFile('built/include/','panda/src/linmath/lvecBase4.h')
CopyFile('built/include/','panda/src/linmath/lvecBase4_src.I')
CopyFile('built/include/','panda/src/linmath/lvecBase4_src.h')
CopyFile('built/include/','panda/src/linmath/lvector2.h')
CopyFile('built/include/','panda/src/linmath/lvector2_src.I')
CopyFile('built/include/','panda/src/linmath/lvector2_src.h')
CopyFile('built/include/','panda/src/linmath/lvector3.h')
CopyFile('built/include/','panda/src/linmath/lvector3_src.I')
CopyFile('built/include/','panda/src/linmath/lvector3_src.h')
CopyFile('built/include/','panda/src/linmath/lvector4.h')
CopyFile('built/include/','panda/src/linmath/lvector4_src.I')
CopyFile('built/include/','panda/src/linmath/lvector4_src.h')
CopyFile('built/include/','panda/src/linmath/mathNumbers.h')
CopyFile('built/include/','panda/src/linmath/mathNumbers.I')
CopyFile('built/include/','panda/src/linmath/pta_Colorf.h')
CopyFile('built/include/','panda/src/linmath/pta_Normalf.h')
CopyFile('built/include/','panda/src/linmath/pta_TexCoordf.h')
CopyFile('built/include/','panda/src/linmath/pta_Vertexf.h')
CopyFile('built/include/','panda/src/linmath/vector_Colorf.h')
CopyFile('built/include/','panda/src/linmath/vector_LPoint2f.h')
CopyFile('built/include/','panda/src/linmath/vector_LVecBase3f.h')
CopyFile('built/include/','panda/src/linmath/vector_Normalf.h')
CopyFile('built/include/','panda/src/linmath/vector_TexCoordf.h')
CopyFile('built/include/','panda/src/linmath/vector_Vertexf.h')
CopyFile('built/include/','panda/src/mathutil/boundingHexahedron.I')
CopyFile('built/include/','panda/src/mathutil/boundingHexahedron.h')
CopyFile('built/include/','panda/src/mathutil/boundingLine.I')
CopyFile('built/include/','panda/src/mathutil/boundingLine.h')
CopyFile('built/include/','panda/src/mathutil/boundingSphere.I')
CopyFile('built/include/','panda/src/mathutil/boundingSphere.h')
CopyFile('built/include/','panda/src/mathutil/boundingVolume.I')
CopyFile('built/include/','panda/src/mathutil/boundingVolume.h')
CopyFile('built/include/','panda/src/mathutil/config_mathutil.h')
CopyFile('built/include/','panda/src/mathutil/fftCompressor.h')
CopyFile('built/include/','panda/src/mathutil/finiteBoundingVolume.h')
CopyFile('built/include/','panda/src/mathutil/frustum.h')
CopyFile('built/include/','panda/src/mathutil/frustum_src.I')
CopyFile('built/include/','panda/src/mathutil/frustum_src.h')
CopyFile('built/include/','panda/src/mathutil/geometricBoundingVolume.I')
CopyFile('built/include/','panda/src/mathutil/geometricBoundingVolume.h')
CopyFile('built/include/','panda/src/mathutil/look_at.h')
CopyFile('built/include/','panda/src/mathutil/look_at_src.I')
CopyFile('built/include/','panda/src/mathutil/look_at_src.h')
CopyFile('built/include/','panda/src/mathutil/linmath_events.h')
CopyFile('built/include/','panda/src/mathutil/mathHelpers.I')
CopyFile('built/include/','panda/src/mathutil/mathHelpers.h')
CopyFile('built/include/','panda/src/mathutil/omniBoundingVolume.I')
CopyFile('built/include/','panda/src/mathutil/omniBoundingVolume.h')
CopyFile('built/include/','panda/src/mathutil/plane.h')
CopyFile('built/include/','panda/src/mathutil/plane_src.I')
CopyFile('built/include/','panda/src/mathutil/plane_src.h')
CopyFile('built/include/','panda/src/mathutil/rotate_to.h')
CopyFile('built/include/','panda/src/gsgbase/graphicsStateGuardianBase.h')
CopyFile('built/include/','panda/src/pnmimage/config_pnmimage.h')
CopyFile('built/include/','panda/src/pnmimage/pnmFileType.h')
CopyFile('built/include/','panda/src/pnmimage/pnmFileTypeRegistry.h')
CopyFile('built/include/','panda/src/pnmimage/pnmImage.I')
CopyFile('built/include/','panda/src/pnmimage/pnmImage.h')
CopyFile('built/include/','panda/src/pnmimage/pnmImageHeader.I')
CopyFile('built/include/','panda/src/pnmimage/pnmImageHeader.h')
CopyFile('built/include/','panda/src/pnmimage/pnmReader.I')
CopyFile('built/include/','panda/src/pnmimage/pnmReader.h')
CopyFile('built/include/','panda/src/pnmimage/pnmWriter.I')
CopyFile('built/include/','panda/src/pnmimage/pnmWriter.h')
CopyFile('built/include/','panda/src/pnmimage/pnmimage_base.h')
CopyFile('built/include/','panda/src/pnmimagetypes/sgi.h')
CopyFile('built/include/','panda/src/net/config_net.h')
CopyFile('built/include/','panda/src/net/connection.h')
CopyFile('built/include/','panda/src/net/connectionListener.h')
CopyFile('built/include/','panda/src/net/connectionManager.h')
CopyFile('built/include/','panda/src/net/connectionReader.h')
CopyFile('built/include/','panda/src/net/connectionWriter.h')
CopyFile('built/include/','panda/src/net/datagramQueue.h')
CopyFile('built/include/','panda/src/net/datagramTCPHeader.I')
CopyFile('built/include/','panda/src/net/datagramTCPHeader.h')
CopyFile('built/include/','panda/src/net/datagramUDPHeader.I')
CopyFile('built/include/','panda/src/net/datagramUDPHeader.h')
CopyFile('built/include/','panda/src/net/netAddress.h')
CopyFile('built/include/','panda/src/net/netDatagram.I')
CopyFile('built/include/','panda/src/net/netDatagram.h')
CopyFile('built/include/','panda/src/net/pprerror.h')
CopyFile('built/include/','panda/src/net/queuedConnectionListener.I')
CopyFile('built/include/','panda/src/net/queuedConnectionListener.h')
CopyFile('built/include/','panda/src/net/queuedConnectionManager.h')
CopyFile('built/include/','panda/src/net/queuedConnectionReader.h')
CopyFile('built/include/','panda/src/net/queuedReturn.I')
CopyFile('built/include/','panda/src/net/queuedReturn.h')
CopyFile('built/include/','panda/src/net/recentConnectionReader.h')
CopyFile('built/include/','panda/src/pstatclient/config_pstats.h')
CopyFile('built/include/','panda/src/pstatclient/pStatClient.I')
CopyFile('built/include/','panda/src/pstatclient/pStatClient.h')
CopyFile('built/include/','panda/src/pstatclient/pStatClientImpl.I')
CopyFile('built/include/','panda/src/pstatclient/pStatClientImpl.h')
CopyFile('built/include/','panda/src/pstatclient/pStatClientVersion.I')
CopyFile('built/include/','panda/src/pstatclient/pStatClientVersion.h')
CopyFile('built/include/','panda/src/pstatclient/pStatClientControlMessage.h')
CopyFile('built/include/','panda/src/pstatclient/pStatCollector.I')
CopyFile('built/include/','panda/src/pstatclient/pStatCollector.h')
CopyFile('built/include/','panda/src/pstatclient/pStatCollectorDef.h')
CopyFile('built/include/','panda/src/pstatclient/pStatFrameData.I')
CopyFile('built/include/','panda/src/pstatclient/pStatFrameData.h')
CopyFile('built/include/','panda/src/pstatclient/pStatProperties.h')
CopyFile('built/include/','panda/src/pstatclient/pStatServerControlMessage.h')
CopyFile('built/include/','panda/src/pstatclient/pStatThread.I')
CopyFile('built/include/','panda/src/pstatclient/pStatThread.h')
CopyFile('built/include/','panda/src/pstatclient/pStatTimer.I')
CopyFile('built/include/','panda/src/pstatclient/pStatTimer.h')
CopyFile('built/include/','panda/src/gobj/boundedObject.I')
CopyFile('built/include/','panda/src/gobj/boundedObject.h')
CopyFile('built/include/','panda/src/gobj/config_gobj.h')
CopyFile('built/include/','panda/src/gobj/drawable.h')
CopyFile('built/include/','panda/src/gobj/geom.I')
CopyFile('built/include/','panda/src/gobj/geom.h')
CopyFile('built/include/','panda/src/gobj/textureContext.I')
CopyFile('built/include/','panda/src/gobj/textureContext.h')
CopyFile('built/include/','panda/src/gobj/geomLine.h')
CopyFile('built/include/','panda/src/gobj/geomLinestrip.h')
CopyFile('built/include/','panda/src/gobj/geomPoint.h')
CopyFile('built/include/','panda/src/gobj/geomPolygon.h')
CopyFile('built/include/','panda/src/gobj/geomQuad.h')
CopyFile('built/include/','panda/src/gobj/geomSphere.h')
CopyFile('built/include/','panda/src/gobj/geomSprite.I')
CopyFile('built/include/','panda/src/gobj/geomSprite.h')
CopyFile('built/include/','panda/src/gobj/geomTri.h')
CopyFile('built/include/','panda/src/gobj/geomTrifan.h')
CopyFile('built/include/','panda/src/gobj/geomTristrip.h')
CopyFile('built/include/','panda/src/gobj/geomprimitives.h')
CopyFile('built/include/','panda/src/gobj/imageBuffer.I')
CopyFile('built/include/','panda/src/gobj/imageBuffer.h')
CopyFile('built/include/','panda/src/gobj/material.I')
CopyFile('built/include/','panda/src/gobj/material.h')
CopyFile('built/include/','panda/src/gobj/materialPool.I')
CopyFile('built/include/','panda/src/gobj/materialPool.h')
CopyFile('built/include/','panda/src/gobj/matrixLens.I')
CopyFile('built/include/','panda/src/gobj/matrixLens.h')
CopyFile('built/include/','panda/src/gobj/orthographicLens.I')
CopyFile('built/include/','panda/src/gobj/orthographicLens.h')
CopyFile('built/include/','panda/src/gobj/perspectiveLens.I')
CopyFile('built/include/','panda/src/gobj/perspectiveLens.h')
CopyFile('built/include/','panda/src/gobj/pixelBuffer.I')
CopyFile('built/include/','panda/src/gobj/pixelBuffer.h')
CopyFile('built/include/','panda/src/gobj/preparedGraphicsObjects.I')
CopyFile('built/include/','panda/src/gobj/preparedGraphicsObjects.h')
CopyFile('built/include/','panda/src/gobj/lens.h')
CopyFile('built/include/','panda/src/gobj/lens.I')
CopyFile('built/include/','panda/src/gobj/savedContext.I')
CopyFile('built/include/','panda/src/gobj/savedContext.h')
CopyFile('built/include/','panda/src/gobj/texture.I')
CopyFile('built/include/','panda/src/gobj/texture.h')
CopyFile('built/include/','panda/src/gobj/texturePool.I')
CopyFile('built/include/','panda/src/gobj/texturePool.h')
CopyFile('built/include/','panda/src/gobj/texCoordName.I')
CopyFile('built/include/','panda/src/gobj/texCoordName.h')
CopyFile('built/include/','panda/src/gobj/textureStage.I')
CopyFile('built/include/','panda/src/gobj/textureStage.h')
CopyFile('built/include/','panda/src/lerp/lerp.h')
CopyFile('built/include/','panda/src/lerp/lerpblend.h')
CopyFile('built/include/','panda/src/lerp/lerpfunctor.h')
CopyFile('built/include/','panda/src/pgraph/accumulatedAttribs.I')
CopyFile('built/include/','panda/src/pgraph/accumulatedAttribs.h')
CopyFile('built/include/','panda/src/pgraph/alphaTestAttrib.I')
CopyFile('built/include/','panda/src/pgraph/alphaTestAttrib.h')
CopyFile('built/include/','panda/src/pgraph/antialiasAttrib.I')
CopyFile('built/include/','panda/src/pgraph/antialiasAttrib.h')
CopyFile('built/include/','panda/src/pgraph/ambientLight.I')
CopyFile('built/include/','panda/src/pgraph/ambientLight.h')
CopyFile('built/include/','panda/src/pgraph/auxSceneData.I')
CopyFile('built/include/','panda/src/pgraph/auxSceneData.h')
CopyFile('built/include/','panda/src/pgraph/bamFile.I')
CopyFile('built/include/','panda/src/pgraph/bamFile.h')
CopyFile('built/include/','panda/src/pgraph/billboardEffect.I')
CopyFile('built/include/','panda/src/pgraph/billboardEffect.h')
CopyFile('built/include/','panda/src/pgraph/binCullHandler.I')
CopyFile('built/include/','panda/src/pgraph/binCullHandler.h')
CopyFile('built/include/','panda/src/pgraph/camera.I')
CopyFile('built/include/','panda/src/pgraph/camera.h')
CopyFile('built/include/','panda/src/pgraph/clipPlaneAttrib.I')
CopyFile('built/include/','panda/src/pgraph/clipPlaneAttrib.h')
CopyFile('built/include/','panda/src/pgraph/colorAttrib.I')
CopyFile('built/include/','panda/src/pgraph/colorAttrib.h')
CopyFile('built/include/','panda/src/pgraph/colorBlendAttrib.I')
CopyFile('built/include/','panda/src/pgraph/colorBlendAttrib.h')
CopyFile('built/include/','panda/src/pgraph/colorScaleAttrib.I')
CopyFile('built/include/','panda/src/pgraph/colorScaleAttrib.h')
CopyFile('built/include/','panda/src/pgraph/colorWriteAttrib.I')
CopyFile('built/include/','panda/src/pgraph/colorWriteAttrib.h')
CopyFile('built/include/','panda/src/pgraph/compassEffect.I')
CopyFile('built/include/','panda/src/pgraph/compassEffect.h')
CopyFile('built/include/','panda/src/pgraph/config_pgraph.h')
CopyFile('built/include/','panda/src/pgraph/cullBin.I')
CopyFile('built/include/','panda/src/pgraph/cullBin.h')
CopyFile('built/include/','panda/src/pgraph/cullBinAttrib.I')
CopyFile('built/include/','panda/src/pgraph/cullBinAttrib.h')
CopyFile('built/include/','panda/src/pgraph/cullBinBackToFront.I')
CopyFile('built/include/','panda/src/pgraph/cullBinBackToFront.h')
CopyFile('built/include/','panda/src/pgraph/cullBinFixed.I')
CopyFile('built/include/','panda/src/pgraph/cullBinFixed.h')
CopyFile('built/include/','panda/src/pgraph/cullBinFrontToBack.I')
CopyFile('built/include/','panda/src/pgraph/cullBinFrontToBack.h')
CopyFile('built/include/','panda/src/pgraph/cullBinManager.I')
CopyFile('built/include/','panda/src/pgraph/cullBinManager.h')
CopyFile('built/include/','panda/src/pgraph/cullBinUnsorted.I')
CopyFile('built/include/','panda/src/pgraph/cullBinUnsorted.h')
CopyFile('built/include/','panda/src/pgraph/cullFaceAttrib.I')
CopyFile('built/include/','panda/src/pgraph/cullFaceAttrib.h')
CopyFile('built/include/','panda/src/pgraph/cullHandler.I')
CopyFile('built/include/','panda/src/pgraph/cullHandler.h')
CopyFile('built/include/','panda/src/pgraph/cullResult.I')
CopyFile('built/include/','panda/src/pgraph/cullResult.h')
CopyFile('built/include/','panda/src/pgraph/cullTraverser.I')
CopyFile('built/include/','panda/src/pgraph/cullTraverser.h')
CopyFile('built/include/','panda/src/pgraph/cullTraverserData.I')
CopyFile('built/include/','panda/src/pgraph/cullTraverserData.h')
CopyFile('built/include/','panda/src/pgraph/cullableObject.I')
CopyFile('built/include/','panda/src/pgraph/cullableObject.h')
CopyFile('built/include/','panda/src/pgraph/decalEffect.I')
CopyFile('built/include/','panda/src/pgraph/decalEffect.h')
CopyFile('built/include/','panda/src/pgraph/depthOffsetAttrib.I')
CopyFile('built/include/','panda/src/pgraph/depthOffsetAttrib.h')
CopyFile('built/include/','panda/src/pgraph/depthTestAttrib.I')
CopyFile('built/include/','panda/src/pgraph/depthTestAttrib.h')
CopyFile('built/include/','panda/src/pgraph/depthWriteAttrib.I')
CopyFile('built/include/','panda/src/pgraph/depthWriteAttrib.h')
CopyFile('built/include/','panda/src/pgraph/directionalLight.I')
CopyFile('built/include/','panda/src/pgraph/directionalLight.h')
CopyFile('built/include/','panda/src/pgraph/drawCullHandler.I')
CopyFile('built/include/','panda/src/pgraph/drawCullHandler.h')
CopyFile('built/include/','panda/src/pgraph/fadeLodNode.I')
CopyFile('built/include/','panda/src/pgraph/fadeLodNode.h')
CopyFile('built/include/','panda/src/pgraph/fadeLodNodeData.h')
CopyFile('built/include/','panda/src/pgraph/fog.I')
CopyFile('built/include/','panda/src/pgraph/fog.h')
CopyFile('built/include/','panda/src/pgraph/fogAttrib.I')
CopyFile('built/include/','panda/src/pgraph/fogAttrib.h')
CopyFile('built/include/','panda/src/pgraph/geomNode.I')
CopyFile('built/include/','panda/src/pgraph/geomNode.h')
CopyFile('built/include/','panda/src/pgraph/geomTransformer.I')
CopyFile('built/include/','panda/src/pgraph/geomTransformer.h')
CopyFile('built/include/','panda/src/pgraph/lensNode.I')
CopyFile('built/include/','panda/src/pgraph/lensNode.h')
CopyFile('built/include/','panda/src/pgraph/light.I')
CopyFile('built/include/','panda/src/pgraph/light.h')
CopyFile('built/include/','panda/src/pgraph/lightAttrib.I')
CopyFile('built/include/','panda/src/pgraph/lightAttrib.h')
CopyFile('built/include/','panda/src/pgraph/lightLensNode.I')
CopyFile('built/include/','panda/src/pgraph/lightLensNode.h')
CopyFile('built/include/','panda/src/pgraph/lightNode.I')
CopyFile('built/include/','panda/src/pgraph/lightNode.h')
CopyFile('built/include/','panda/src/pgraph/loader.I')
CopyFile('built/include/','panda/src/pgraph/loader.h')
CopyFile('built/include/','panda/src/pgraph/loaderFileType.h')
CopyFile('built/include/','panda/src/pgraph/loaderFileTypeBam.h')
CopyFile('built/include/','panda/src/pgraph/loaderFileTypeRegistry.h')
CopyFile('built/include/','panda/src/pgraph/lodNode.I')
CopyFile('built/include/','panda/src/pgraph/lodNode.h')
CopyFile('built/include/','panda/src/pgraph/materialAttrib.I')
CopyFile('built/include/','panda/src/pgraph/materialAttrib.h')
CopyFile('built/include/','panda/src/pgraph/modelNode.I')
CopyFile('built/include/','panda/src/pgraph/modelNode.h')
CopyFile('built/include/','panda/src/pgraph/modelPool.I')
CopyFile('built/include/','panda/src/pgraph/modelPool.h')
CopyFile('built/include/','panda/src/pgraph/modelRoot.I')
CopyFile('built/include/','panda/src/pgraph/modelRoot.h')
CopyFile('built/include/','panda/src/pgraph/nodePath.I')
CopyFile('built/include/','panda/src/pgraph/nodePath.h')
CopyFile('built/include/','panda/src/pgraph/nodePathCollection.I')
CopyFile('built/include/','panda/src/pgraph/nodePathCollection.h')
CopyFile('built/include/','panda/src/pgraph/nodePathComponent.I')
CopyFile('built/include/','panda/src/pgraph/nodePathComponent.h')
CopyFile('built/include/','panda/src/pgraph/nodePathLerps.h')
CopyFile('built/include/','panda/src/pgraph/pandaNode.I')
CopyFile('built/include/','panda/src/pgraph/pandaNode.h')
CopyFile('built/include/','panda/src/pgraph/planeNode.I')
CopyFile('built/include/','panda/src/pgraph/planeNode.h')
CopyFile('built/include/','panda/src/pgraph/pointLight.I')
CopyFile('built/include/','panda/src/pgraph/pointLight.h')
CopyFile('built/include/','panda/src/pgraph/polylightNode.I')
CopyFile('built/include/','panda/src/pgraph/polylightNode.h')
CopyFile('built/include/','panda/src/pgraph/polylightEffect.I')
CopyFile('built/include/','panda/src/pgraph/polylightEffect.h')
CopyFile('built/include/','panda/src/pgraph/portalNode.I')
CopyFile('built/include/','panda/src/pgraph/portalNode.h')
CopyFile('built/include/','panda/src/pgraph/portalClipper.I')
CopyFile('built/include/','panda/src/pgraph/portalClipper.h')
CopyFile('built/include/','panda/src/pgraph/renderAttrib.I')
CopyFile('built/include/','panda/src/pgraph/renderAttrib.h')
CopyFile('built/include/','panda/src/pgraph/renderEffect.I')
CopyFile('built/include/','panda/src/pgraph/renderEffect.h')
CopyFile('built/include/','panda/src/pgraph/renderEffects.I')
CopyFile('built/include/','panda/src/pgraph/renderEffects.h')
CopyFile('built/include/','panda/src/pgraph/renderModeAttrib.I')
CopyFile('built/include/','panda/src/pgraph/renderModeAttrib.h')
CopyFile('built/include/','panda/src/pgraph/renderState.I')
CopyFile('built/include/','panda/src/pgraph/renderState.h')
CopyFile('built/include/','panda/src/pgraph/rescaleNormalAttrib.I')
CopyFile('built/include/','panda/src/pgraph/rescaleNormalAttrib.h')
CopyFile('built/include/','panda/src/pgraph/sceneGraphAnalyzer.h')
CopyFile('built/include/','panda/src/pgraph/sceneGraphReducer.I')
CopyFile('built/include/','panda/src/pgraph/sceneGraphReducer.h')
CopyFile('built/include/','panda/src/pgraph/sceneSetup.I')
CopyFile('built/include/','panda/src/pgraph/sceneSetup.h')
CopyFile('built/include/','panda/src/pgraph/selectiveChildNode.I')
CopyFile('built/include/','panda/src/pgraph/selectiveChildNode.h')
CopyFile('built/include/','panda/src/pgraph/sequenceNode.I')
CopyFile('built/include/','panda/src/pgraph/sequenceNode.h')
CopyFile('built/include/','panda/src/pgraph/showBoundsEffect.I')
CopyFile('built/include/','panda/src/pgraph/showBoundsEffect.h')
CopyFile('built/include/','panda/src/pgraph/spotlight.I')
CopyFile('built/include/','panda/src/pgraph/spotlight.h')
CopyFile('built/include/','panda/src/pgraph/switchNode.I')
CopyFile('built/include/','panda/src/pgraph/switchNode.h')
CopyFile('built/include/','panda/src/pgraph/texMatrixAttrib.I')
CopyFile('built/include/','panda/src/pgraph/texMatrixAttrib.h')
CopyFile('built/include/','panda/src/pgraph/texProjectorEffect.I')
CopyFile('built/include/','panda/src/pgraph/texProjectorEffect.h')
CopyFile('built/include/','panda/src/pgraph/textureApplyAttrib.I')
CopyFile('built/include/','panda/src/pgraph/textureApplyAttrib.h')
CopyFile('built/include/','panda/src/pgraph/textureAttrib.I')
CopyFile('built/include/','panda/src/pgraph/textureAttrib.h')
CopyFile('built/include/','panda/src/pgraph/texGenAttrib.I')
CopyFile('built/include/','panda/src/pgraph/texGenAttrib.h')
CopyFile('built/include/','panda/src/pgraph/textureCollection.I')
CopyFile('built/include/','panda/src/pgraph/textureCollection.h')
CopyFile('built/include/','panda/src/pgraph/textureStageCollection.I')
CopyFile('built/include/','panda/src/pgraph/textureStageCollection.h')
CopyFile('built/include/','panda/src/pgraph/transformState.I')
CopyFile('built/include/','panda/src/pgraph/transformState.h')
CopyFile('built/include/','panda/src/pgraph/transparencyAttrib.I')
CopyFile('built/include/','panda/src/pgraph/transparencyAttrib.h')
CopyFile('built/include/','panda/src/pgraph/weakNodePath.I')
CopyFile('built/include/','panda/src/pgraph/weakNodePath.h')
CopyFile('built/include/','panda/src/pgraph/workingNodePath.I')
CopyFile('built/include/','panda/src/pgraph/workingNodePath.h')
CopyFile('built/include/','panda/src/chan/animBundle.I')
CopyFile('built/include/','panda/src/chan/animBundle.h')
CopyFile('built/include/','panda/src/chan/animBundleNode.I')
CopyFile('built/include/','panda/src/chan/animBundleNode.h')
CopyFile('built/include/','panda/src/chan/animChannel.I')
CopyFile('built/include/','panda/src/chan/animChannel.h')
CopyFile('built/include/','panda/src/chan/animChannelBase.I')
CopyFile('built/include/','panda/src/chan/animChannelBase.h')
CopyFile('built/include/','panda/src/chan/animChannelFixed.I')
CopyFile('built/include/','panda/src/chan/animChannelFixed.h')
CopyFile('built/include/','panda/src/chan/animChannelMatrixDynamic.I')
CopyFile('built/include/','panda/src/chan/animChannelMatrixDynamic.h')
CopyFile('built/include/','panda/src/chan/animChannelMatrixXfmTable.I')
CopyFile('built/include/','panda/src/chan/animChannelMatrixXfmTable.h')
CopyFile('built/include/','panda/src/chan/animChannelScalarDynamic.I')
CopyFile('built/include/','panda/src/chan/animChannelScalarDynamic.h')
CopyFile('built/include/','panda/src/chan/animChannelScalarTable.I')
CopyFile('built/include/','panda/src/chan/animChannelScalarTable.h')
CopyFile('built/include/','panda/src/chan/animControl.I')
CopyFile('built/include/','panda/src/chan/animControl.h')
CopyFile('built/include/','panda/src/chan/animControlCollection.I')
CopyFile('built/include/','panda/src/chan/animControlCollection.h')
CopyFile('built/include/','panda/src/chan/animGroup.I')
CopyFile('built/include/','panda/src/chan/animGroup.h')
CopyFile('built/include/','panda/src/chan/auto_bind.h')
CopyFile('built/include/','panda/src/chan/config_chan.h')
CopyFile('built/include/','panda/src/chan/movingPart.I')
CopyFile('built/include/','panda/src/chan/movingPart.h')
CopyFile('built/include/','panda/src/chan/movingPartBase.I')
CopyFile('built/include/','panda/src/chan/movingPartBase.h')
CopyFile('built/include/','panda/src/chan/movingPartMatrix.I')
CopyFile('built/include/','panda/src/chan/movingPartMatrix.h')
CopyFile('built/include/','panda/src/chan/movingPartScalar.I')
CopyFile('built/include/','panda/src/chan/movingPartScalar.h')
CopyFile('built/include/','panda/src/chan/partBundle.I')
CopyFile('built/include/','panda/src/chan/partBundle.h')
CopyFile('built/include/','panda/src/chan/partBundleNode.I')
CopyFile('built/include/','panda/src/chan/partBundleNode.h')
CopyFile('built/include/','panda/src/chan/partGroup.I')
CopyFile('built/include/','panda/src/chan/partGroup.h')
CopyFile('built/include/','panda/src/chan/vector_PartGroupStar.h')
CopyFile('built/include/','panda/src/char/character.I')
CopyFile('built/include/','panda/src/char/character.h')
CopyFile('built/include/','panda/src/char/characterJoint.h')
CopyFile('built/include/','panda/src/char/characterJointBundle.I')
CopyFile('built/include/','panda/src/char/characterJointBundle.h')
CopyFile('built/include/','panda/src/char/characterSlider.h')
CopyFile('built/include/','panda/src/char/computedVertices.I')
CopyFile('built/include/','panda/src/char/computedVertices.h')
CopyFile('built/include/','panda/src/char/computedVerticesMorph.I')
CopyFile('built/include/','panda/src/char/computedVerticesMorph.h')
CopyFile('built/include/','panda/src/char/config_char.h')
CopyFile('built/include/','panda/src/char/dynamicVertices.h')
CopyFile('built/include/','panda/src/dgraph/config_dgraph.h')
CopyFile('built/include/','panda/src/dgraph/dataGraphTraverser.I')
CopyFile('built/include/','panda/src/dgraph/dataGraphTraverser.h')
CopyFile('built/include/','panda/src/dgraph/dataNode.I')
CopyFile('built/include/','panda/src/dgraph/dataNode.h')
CopyFile('built/include/','panda/src/dgraph/dataNodeTransmit.I')
CopyFile('built/include/','panda/src/dgraph/dataNodeTransmit.h')
CopyFile('built/include/','panda/src/display/config_display.h')
CopyFile('built/include/','panda/src/display/drawableRegion.I')
CopyFile('built/include/','panda/src/display/drawableRegion.h')
CopyFile('built/include/','panda/src/display/displayRegion.I')
CopyFile('built/include/','panda/src/display/displayRegion.h')
CopyFile('built/include/','panda/src/display/displayRegionStack.I')
CopyFile('built/include/','panda/src/display/displayRegionStack.h')
CopyFile('built/include/','panda/src/display/frameBufferProperties.I')
CopyFile('built/include/','panda/src/display/frameBufferProperties.h')
CopyFile('built/include/','panda/src/display/frameBufferStack.I')
CopyFile('built/include/','panda/src/display/frameBufferStack.h')
CopyFile('built/include/','panda/src/display/graphicsEngine.I')
CopyFile('built/include/','panda/src/display/graphicsEngine.h')
CopyFile('built/include/','panda/src/display/graphicsOutput.I')
CopyFile('built/include/','panda/src/display/graphicsOutput.h')
CopyFile('built/include/','panda/src/display/graphicsBuffer.I')
CopyFile('built/include/','panda/src/display/graphicsBuffer.h')
CopyFile('built/include/','panda/src/display/graphicsPipe.I')
CopyFile('built/include/','panda/src/display/graphicsPipe.h')
CopyFile('built/include/','panda/src/display/graphicsPipeSelection.I')
CopyFile('built/include/','panda/src/display/graphicsPipeSelection.h')
CopyFile('built/include/','panda/src/display/graphicsStateGuardian.I')
CopyFile('built/include/','panda/src/display/graphicsStateGuardian.h')
CopyFile('built/include/','panda/src/display/graphicsWindow.I')
CopyFile('built/include/','panda/src/display/graphicsWindow.h')
CopyFile('built/include/','panda/src/display/graphicsThreadingModel.I')
CopyFile('built/include/','panda/src/display/graphicsThreadingModel.h')
CopyFile('built/include/','panda/src/display/graphicsWindowInputDevice.I')
CopyFile('built/include/','panda/src/display/graphicsWindowInputDevice.h')
CopyFile('built/include/','panda/src/display/graphicsDevice.I')
CopyFile('built/include/','panda/src/display/graphicsDevice.h')
CopyFile('built/include/','panda/src/display/parasiteBuffer.I')
CopyFile('built/include/','panda/src/display/parasiteBuffer.h')
CopyFile('built/include/','panda/src/display/windowProperties.I')
CopyFile('built/include/','panda/src/display/windowProperties.h')
CopyFile('built/include/','panda/src/display/lensStack.I')
CopyFile('built/include/','panda/src/display/lensStack.h')
CopyFile('built/include/','panda/src/display/renderBuffer.h')
CopyFile('built/include/','panda/src/display/savedFrameBuffer.I')
CopyFile('built/include/','panda/src/display/savedFrameBuffer.h')
CopyFile('built/include/','panda/src/device/analogNode.I')
CopyFile('built/include/','panda/src/device/analogNode.h')
CopyFile('built/include/','panda/src/device/buttonNode.I')
CopyFile('built/include/','panda/src/device/buttonNode.h')
CopyFile('built/include/','panda/src/device/clientAnalogDevice.I')
CopyFile('built/include/','panda/src/device/clientAnalogDevice.h')
CopyFile('built/include/','panda/src/device/clientBase.I')
CopyFile('built/include/','panda/src/device/clientBase.h')
CopyFile('built/include/','panda/src/device/clientButtonDevice.I')
CopyFile('built/include/','panda/src/device/clientButtonDevice.h')
CopyFile('built/include/','panda/src/device/clientDevice.I')
CopyFile('built/include/','panda/src/device/clientDevice.h')
CopyFile('built/include/','panda/src/device/clientDialDevice.I')
CopyFile('built/include/','panda/src/device/clientDialDevice.h')
CopyFile('built/include/','panda/src/device/clientTrackerDevice.I')
CopyFile('built/include/','panda/src/device/clientTrackerDevice.h')
CopyFile('built/include/','panda/src/device/config_device.h')
CopyFile('built/include/','panda/src/device/mouseAndKeyboard.h')
CopyFile('built/include/','panda/src/device/dialNode.I')
CopyFile('built/include/','panda/src/device/dialNode.h')
CopyFile('built/include/','panda/src/device/trackerData.I')
CopyFile('built/include/','panda/src/device/trackerData.h')
CopyFile('built/include/','panda/src/device/trackerNode.I')
CopyFile('built/include/','panda/src/device/trackerNode.h')
CopyFile('built/include/','panda/src/device/virtualMouse.h')
CopyFile('built/include/','panda/src/tform/buttonThrower.I')
CopyFile('built/include/','panda/src/tform/buttonThrower.h')
CopyFile('built/include/','panda/src/tform/driveInterface.I')
CopyFile('built/include/','panda/src/tform/driveInterface.h')
CopyFile('built/include/','panda/src/tform/mouseInterfaceNode.I')
CopyFile('built/include/','panda/src/tform/mouseInterfaceNode.h')
CopyFile('built/include/','panda/src/tform/mouseWatcher.I')
CopyFile('built/include/','panda/src/tform/mouseWatcher.h')
CopyFile('built/include/','panda/src/tform/mouseWatcherGroup.h')
CopyFile('built/include/','panda/src/tform/mouseWatcherParameter.I')
CopyFile('built/include/','panda/src/tform/mouseWatcherParameter.h')
CopyFile('built/include/','panda/src/tform/mouseWatcherRegion.I')
CopyFile('built/include/','panda/src/tform/mouseWatcherRegion.h')
CopyFile('built/include/','panda/src/tform/trackball.h')
CopyFile('built/include/','panda/src/tform/transform2sg.h')
CopyFile('built/include/','panda/src/collide/collisionEntry.I')
CopyFile('built/include/','panda/src/collide/collisionEntry.h')
CopyFile('built/include/','panda/src/collide/collisionHandler.h')
CopyFile('built/include/','panda/src/collide/collisionHandlerEvent.I')
CopyFile('built/include/','panda/src/collide/collisionHandlerEvent.h')
CopyFile('built/include/','panda/src/collide/collisionHandlerFloor.I')
CopyFile('built/include/','panda/src/collide/collisionHandlerFloor.h')
CopyFile('built/include/','panda/src/collide/collisionHandlerGravity.I')
CopyFile('built/include/','panda/src/collide/collisionHandlerGravity.h')
CopyFile('built/include/','panda/src/collide/collisionHandlerPhysical.I')
CopyFile('built/include/','panda/src/collide/collisionHandlerPhysical.h')
CopyFile('built/include/','panda/src/collide/collisionHandlerPusher.I')
CopyFile('built/include/','panda/src/collide/collisionHandlerPusher.h')
CopyFile('built/include/','panda/src/collide/collisionHandlerQueue.h')
CopyFile('built/include/','panda/src/collide/collisionInvSphere.I')
CopyFile('built/include/','panda/src/collide/collisionInvSphere.h')
CopyFile('built/include/','panda/src/collide/collisionLevelState.I')
CopyFile('built/include/','panda/src/collide/collisionLevelState.h')
CopyFile('built/include/','panda/src/collide/collisionLine.I')
CopyFile('built/include/','panda/src/collide/collisionLine.h')
CopyFile('built/include/','panda/src/collide/collisionNode.I')
CopyFile('built/include/','panda/src/collide/collisionNode.h')
CopyFile('built/include/','panda/src/collide/collisionPlane.I')
CopyFile('built/include/','panda/src/collide/collisionPlane.h')
CopyFile('built/include/','panda/src/collide/collisionPolygon.I')
CopyFile('built/include/','panda/src/collide/collisionPolygon.h')
CopyFile('built/include/','panda/src/collide/collisionRay.I')
CopyFile('built/include/','panda/src/collide/collisionRay.h')
CopyFile('built/include/','panda/src/collide/collisionRecorder.I')
CopyFile('built/include/','panda/src/collide/collisionRecorder.h')
CopyFile('built/include/','panda/src/collide/collisionSegment.I')
CopyFile('built/include/','panda/src/collide/collisionSegment.h')
CopyFile('built/include/','panda/src/collide/collisionSolid.I')
CopyFile('built/include/','panda/src/collide/collisionSolid.h')
CopyFile('built/include/','panda/src/collide/collisionSphere.I')
CopyFile('built/include/','panda/src/collide/collisionSphere.h')
CopyFile('built/include/','panda/src/collide/collisionTraverser.I')
CopyFile('built/include/','panda/src/collide/collisionTraverser.h')
CopyFile('built/include/','panda/src/collide/collisionTube.I')
CopyFile('built/include/','panda/src/collide/collisionTube.h')
CopyFile('built/include/','panda/src/collide/collisionVisualizer.I')
CopyFile('built/include/','panda/src/collide/collisionVisualizer.h')
CopyFile('built/include/','panda/src/collide/config_collide.h')
CopyFile('built/include/','panda/src/pnmtext/config_pnmtext.h')
CopyFile('built/include/','panda/src/pnmtext/freetypeFont.h')
CopyFile('built/include/','panda/src/pnmtext/freetypeFont.I')
CopyFile('built/include/','panda/src/pnmtext/pnmTextGlyph.h')
CopyFile('built/include/','panda/src/pnmtext/pnmTextGlyph.I')
CopyFile('built/include/','panda/src/pnmtext/pnmTextMaker.h')
CopyFile('built/include/','panda/src/pnmtext/pnmTextMaker.I')
CopyFile('built/include/','panda/src/text/config_text.h')
CopyFile('built/include/','panda/src/text/dynamicTextFont.I')
CopyFile('built/include/','panda/src/text/dynamicTextFont.h')
CopyFile('built/include/','panda/src/text/dynamicTextGlyph.I')
CopyFile('built/include/','panda/src/text/dynamicTextGlyph.h')
CopyFile('built/include/','panda/src/text/dynamicTextPage.I')
CopyFile('built/include/','panda/src/text/dynamicTextPage.h')
CopyFile('built/include/','panda/src/text/fontPool.I')
CopyFile('built/include/','panda/src/text/fontPool.h')
CopyFile('built/include/','panda/src/text/geomTextGlyph.I')
CopyFile('built/include/','panda/src/text/geomTextGlyph.h')
CopyFile('built/include/','panda/src/text/staticTextFont.I')
CopyFile('built/include/','panda/src/text/staticTextFont.h')
CopyFile('built/include/','panda/src/text/textAssembler.I')
CopyFile('built/include/','panda/src/text/textAssembler.h')
CopyFile('built/include/','panda/src/text/textFont.I')
CopyFile('built/include/','panda/src/text/textFont.h')
CopyFile('built/include/','panda/src/text/textGlyph.I')
CopyFile('built/include/','panda/src/text/textGlyph.h')
CopyFile('built/include/','panda/src/text/textNode.I')
CopyFile('built/include/','panda/src/text/textNode.h')
CopyFile('built/include/','panda/src/text/textProperties.I')
CopyFile('built/include/','panda/src/text/textProperties.h')
CopyFile('built/include/','panda/src/text/textPropertiesManager.I')
CopyFile('built/include/','panda/src/text/textPropertiesManager.h')
CopyFile('built/include/','panda/src/grutil/cardMaker.I')
CopyFile('built/include/','panda/src/grutil/cardMaker.h')
CopyFile('built/include/','panda/src/grutil/frameRateMeter.I')
CopyFile('built/include/','panda/src/grutil/frameRateMeter.h')
CopyFile('built/include/','panda/src/grutil/lineSegs.I')
CopyFile('built/include/','panda/src/grutil/lineSegs.h')
CopyFile('built/include/','panda/src/grutil/multitexReducer.I')
CopyFile('built/include/','panda/src/grutil/multitexReducer.h')
CopyFile('built/include/','panda/src/gsgmisc/geomIssuer.I')
CopyFile('built/include/','panda/src/gsgmisc/geomIssuer.h')
if OMIT.count("HELIX")==0:
  CopyFile('built/include/','panda/src/helix/config_helix.h')
  CopyFile('built/include/','panda/src/helix/HelixClient.h')
CopyFile('built/include/','panda/src/parametrics/classicNurbsCurve.I')
CopyFile('built/include/','panda/src/parametrics/classicNurbsCurve.h')
CopyFile('built/include/','panda/src/parametrics/config_parametrics.h')
CopyFile('built/include/','panda/src/parametrics/cubicCurveseg.h')
CopyFile('built/include/','panda/src/parametrics/parametricCurveDrawer.I')
CopyFile('built/include/','panda/src/parametrics/parametricCurveDrawer.h')
CopyFile('built/include/','panda/src/parametrics/curveFitter.I')
CopyFile('built/include/','panda/src/parametrics/curveFitter.h')
CopyFile('built/include/','panda/src/parametrics/hermiteCurve.h')
CopyFile('built/include/','panda/src/parametrics/nurbsCurve.h')
CopyFile('built/include/','panda/src/parametrics/nurbsCurveDrawer.I')
CopyFile('built/include/','panda/src/parametrics/nurbsCurveDrawer.h')
CopyFile('built/include/','panda/src/parametrics/nurbsCurveEvaluator.I')
CopyFile('built/include/','panda/src/parametrics/nurbsCurveEvaluator.h')
CopyFile('built/include/','panda/src/parametrics/nurbsCurveInterface.I')
CopyFile('built/include/','panda/src/parametrics/nurbsCurveInterface.h')
CopyFile('built/include/','panda/src/parametrics/nurbsCurveResult.I')
CopyFile('built/include/','panda/src/parametrics/nurbsCurveResult.h')
CopyFile('built/include/','panda/src/parametrics/nurbsBasisVector.I')
CopyFile('built/include/','panda/src/parametrics/nurbsBasisVector.h')
CopyFile('built/include/','panda/src/parametrics/nurbsSurfaceEvaluator.I')
CopyFile('built/include/','panda/src/parametrics/nurbsSurfaceEvaluator.h')
CopyFile('built/include/','panda/src/parametrics/nurbsSurfaceResult.I')
CopyFile('built/include/','panda/src/parametrics/nurbsSurfaceResult.h')
CopyFile('built/include/','panda/src/parametrics/nurbsVertex.h')
CopyFile('built/include/','panda/src/parametrics/nurbsVertex.I')
CopyFile('built/include/','panda/src/parametrics/nurbsPPCurve.h')
CopyFile('built/include/','panda/src/parametrics/parametricCurve.h')
CopyFile('built/include/','panda/src/parametrics/parametricCurveCollection.I')
CopyFile('built/include/','panda/src/parametrics/parametricCurveCollection.h')
CopyFile('built/include/','panda/src/parametrics/piecewiseCurve.h')
CopyFile('built/include/','panda/src/parametrics/ropeNode.I')
CopyFile('built/include/','panda/src/parametrics/ropeNode.h')
CopyFile('built/include/','panda/src/parametrics/sheetNode.I')
CopyFile('built/include/','panda/src/parametrics/sheetNode.h')
CopyFile('built/include/','panda/src/pgui/pgButton.I')
CopyFile('built/include/','panda/src/pgui/pgButton.h')
CopyFile('built/include/','panda/src/pgui/pgSliderButton.I')
CopyFile('built/include/','panda/src/pgui/pgSliderButton.h')
CopyFile('built/include/','panda/src/pgui/pgCullTraverser.I')
CopyFile('built/include/','panda/src/pgui/pgCullTraverser.h')
CopyFile('built/include/','panda/src/pgui/pgEntry.I')
CopyFile('built/include/','panda/src/pgui/pgEntry.h')
CopyFile('built/include/','panda/src/pgui/pgMouseWatcherGroup.I')
CopyFile('built/include/','panda/src/pgui/pgMouseWatcherGroup.h')
CopyFile('built/include/','panda/src/pgui/pgMouseWatcherParameter.I')
CopyFile('built/include/','panda/src/pgui/pgMouseWatcherParameter.h')
CopyFile('built/include/','panda/src/pgui/pgFrameStyle.I')
CopyFile('built/include/','panda/src/pgui/pgFrameStyle.h')
CopyFile('built/include/','panda/src/pgui/pgItem.I')
CopyFile('built/include/','panda/src/pgui/pgItem.h')
CopyFile('built/include/','panda/src/pgui/pgMouseWatcherBackground.h')
CopyFile('built/include/','panda/src/pgui/pgMouseWatcherRegion.I')
CopyFile('built/include/','panda/src/pgui/pgMouseWatcherRegion.h')
CopyFile('built/include/','panda/src/pgui/pgTop.I')
CopyFile('built/include/','panda/src/pgui/pgTop.h')
CopyFile('built/include/','panda/src/pgui/pgWaitBar.I')
CopyFile('built/include/','panda/src/pgui/pgWaitBar.h')
CopyFile('built/include/','panda/src/pgui/pgSliderBar.I')
CopyFile('built/include/','panda/src/pgui/pgSliderBar.h')
CopyFile('built/include/','panda/src/pnmimagetypes/config_pnmimagetypes.h')
CopyFile('built/include/','panda/src/recorder/mouseRecorder.h')
CopyFile('built/include/','panda/src/recorder/recorderBase.h')
CopyFile('built/include/','panda/src/recorder/recorderBase.I')
CopyFile('built/include/','panda/src/recorder/recorderController.h')
CopyFile('built/include/','panda/src/recorder/recorderController.I')
CopyFile('built/include/','panda/src/recorder/recorderFrame.h')
CopyFile('built/include/','panda/src/recorder/recorderFrame.I')
CopyFile('built/include/','panda/src/recorder/recorderHeader.h')
CopyFile('built/include/','panda/src/recorder/recorderHeader.I')
CopyFile('built/include/','panda/src/recorder/recorderTable.h')
CopyFile('built/include/','panda/src/recorder/recorderTable.I')
CopyFile('built/include/','panda/src/recorder/socketStreamRecorder.h')
CopyFile('built/include/','panda/src/recorder/socketStreamRecorder.I')
CopyFile('built/include/','panda/src/vrpn/config_vrpn.h')
CopyFile('built/include/','panda/src/vrpn/vrpnClient.I')
CopyFile('built/include/','panda/src/vrpn/vrpnClient.h')
CopyFile('built/include/','panda/metalibs/panda/panda.h')
CopyFile('built/include/','panda/src/builder/builder.I')
CopyFile('built/include/','panda/src/builder/builder.h')
CopyFile('built/include/','panda/src/builder/builder_compare.I')
CopyFile('built/include/','panda/src/builder/builder_compare.h')
CopyFile('built/include/','panda/src/builder/builderAttrib.I')
CopyFile('built/include/','panda/src/builder/builderAttrib.h')
CopyFile('built/include/','panda/src/builder/builderAttribTempl.I')
CopyFile('built/include/','panda/src/builder/builderAttribTempl.h')
CopyFile('built/include/','panda/src/builder/builderBucket.I')
CopyFile('built/include/','panda/src/builder/builderBucket.h')
CopyFile('built/include/','panda/src/builder/builderBucketNode.I')
CopyFile('built/include/','panda/src/builder/builderBucketNode.h')
CopyFile('built/include/','panda/src/builder/builderNormalVisualizer.I')
CopyFile('built/include/','panda/src/builder/builderNormalVisualizer.h')
CopyFile('built/include/','panda/src/builder/builderPrim.I')
CopyFile('built/include/','panda/src/builder/builderPrim.h')
CopyFile('built/include/','panda/src/builder/builderPrimTempl.I')
CopyFile('built/include/','panda/src/builder/builderPrimTempl.h')
CopyFile('built/include/','panda/src/builder/builderProperties.h')
CopyFile('built/include/','panda/src/builder/builderTypes.h')
CopyFile('built/include/','panda/src/builder/builderVertex.I')
CopyFile('built/include/','panda/src/builder/builderVertex.h')
CopyFile('built/include/','panda/src/builder/builderVertexTempl.I')
CopyFile('built/include/','panda/src/builder/builderVertexTempl.h')
CopyFile('built/include/','panda/src/builder/config_builder.h')
CopyFile('built/include/','panda/src/windisplay/config_windisplay.h')
CopyFile('built/include/','panda/src/windisplay/winGraphicsPipe.I')
CopyFile('built/include/','panda/src/windisplay/winGraphicsPipe.h')
CopyFile('built/include/','panda/src/windisplay/winGraphicsWindow.I')
CopyFile('built/include/','panda/src/windisplay/winGraphicsWindow.h')
CopyFile('built/include/','panda/src/dxgsg7/config_dxgsg7.h')
CopyFile('built/include/','panda/src/dxgsg7/dxGraphicsStateGuardian7.I')
CopyFile('built/include/','panda/src/dxgsg7/dxGraphicsStateGuardian7.h')
CopyFile('built/include/','panda/src/dxgsg7/dxTextureContext7.h')
CopyFile('built/include/','panda/src/dxgsg7/dxgsg7base.h')
CopyFile('built/include/','panda/src/dxgsg8/dxgsg8base.h')
CopyFile('built/include/','panda/src/dxgsg8/config_dxgsg8.h')
CopyFile('built/include/','panda/src/dxgsg8/dxGraphicsStateGuardian8.I')
CopyFile('built/include/','panda/src/dxgsg8/dxGraphicsStateGuardian8.h')
CopyFile('built/include/','panda/src/dxgsg8/dxTextureContext8.h')
CopyFile('built/include/','panda/src/dxgsg8/d3dfont8.h')
CopyFile('built/include/','panda/src/dxgsg8/dxGraphicsDevice8.h')
CopyFile('built/include/','panda/src/dxgsg9/dxgsg9base.h')
CopyFile('built/include/','panda/src/dxgsg9/config_dxgsg9.h')
CopyFile('built/include/','panda/src/dxgsg9/dxGraphicsStateGuardian9.I')
CopyFile('built/include/','panda/src/dxgsg9/dxGraphicsStateGuardian9.h')
CopyFile('built/include/','panda/src/dxgsg9/dxTextureContext9.h')
CopyFile('built/include/','panda/src/dxgsg9/d3dfont9.h')
CopyFile('built/include/','panda/src/dxgsg9/dxGraphicsDevice9.h')
CopyFile('built/include/','panda/src/effects/config_effects.h')
CopyFile('built/include/','panda/src/effects/cgShader.I')
CopyFile('built/include/','panda/src/effects/cgShader.h')
CopyFile('built/include/','panda/src/effects/cgShaderAttrib.I')
CopyFile('built/include/','panda/src/effects/cgShaderAttrib.h')
CopyFile('built/include/','panda/src/effects/cgShaderContext.I')
CopyFile('built/include/','panda/src/effects/cgShaderContext.h')
CopyFile('built/include/','panda/src/effects/lensFlareNode.I')
CopyFile('built/include/','panda/src/effects/lensFlareNode.h')
CopyFile('built/include/','panda/src/egg/eggAnimData.I')
CopyFile('built/include/','panda/src/egg/eggAnimData.h')
CopyFile('built/include/','panda/src/egg/eggAttributes.I')
CopyFile('built/include/','panda/src/egg/eggAttributes.h')
CopyFile('built/include/','panda/src/egg/eggBin.h')
CopyFile('built/include/','panda/src/egg/eggBinMaker.h')
CopyFile('built/include/','panda/src/egg/eggComment.I')
CopyFile('built/include/','panda/src/egg/eggComment.h')
CopyFile('built/include/','panda/src/egg/eggCoordinateSystem.I')
CopyFile('built/include/','panda/src/egg/eggCoordinateSystem.h')
CopyFile('built/include/','panda/src/egg/eggCurve.I')
CopyFile('built/include/','panda/src/egg/eggCurve.h')
CopyFile('built/include/','panda/src/egg/eggData.I')
CopyFile('built/include/','panda/src/egg/eggData.h')
CopyFile('built/include/','panda/src/egg/eggExternalReference.I')
CopyFile('built/include/','panda/src/egg/eggExternalReference.h')
CopyFile('built/include/','panda/src/egg/eggFilenameNode.I')
CopyFile('built/include/','panda/src/egg/eggFilenameNode.h')
CopyFile('built/include/','panda/src/egg/eggGroup.I')
CopyFile('built/include/','panda/src/egg/eggGroup.h')
CopyFile('built/include/','panda/src/egg/eggGroupNode.I')
CopyFile('built/include/','panda/src/egg/eggGroupNode.h')
CopyFile('built/include/','panda/src/egg/eggGroupUniquifier.h')
CopyFile('built/include/','panda/src/egg/eggLine.I')
CopyFile('built/include/','panda/src/egg/eggLine.h')
CopyFile('built/include/','panda/src/egg/eggMaterial.I')
CopyFile('built/include/','panda/src/egg/eggMaterial.h')
CopyFile('built/include/','panda/src/egg/eggMaterialCollection.I')
CopyFile('built/include/','panda/src/egg/eggMaterialCollection.h')
CopyFile('built/include/','panda/src/egg/eggMorph.I')
CopyFile('built/include/','panda/src/egg/eggMorph.h')
CopyFile('built/include/','panda/src/egg/eggMorphList.I')
CopyFile('built/include/','panda/src/egg/eggMorphList.h')
CopyFile('built/include/','panda/src/egg/eggNamedObject.I')
CopyFile('built/include/','panda/src/egg/eggNamedObject.h')
CopyFile('built/include/','panda/src/egg/eggNameUniquifier.h')
CopyFile('built/include/','panda/src/egg/eggNode.I')
CopyFile('built/include/','panda/src/egg/eggNode.h')
CopyFile('built/include/','panda/src/egg/eggNurbsCurve.I')
CopyFile('built/include/','panda/src/egg/eggNurbsCurve.h')
CopyFile('built/include/','panda/src/egg/eggNurbsSurface.I')
CopyFile('built/include/','panda/src/egg/eggNurbsSurface.h')
CopyFile('built/include/','panda/src/egg/eggObject.I')
CopyFile('built/include/','panda/src/egg/eggObject.h')
CopyFile('built/include/','panda/src/egg/eggParameters.h')
CopyFile('built/include/','panda/src/egg/eggPoint.I')
CopyFile('built/include/','panda/src/egg/eggPoint.h')
CopyFile('built/include/','panda/src/egg/eggPolygon.I')
CopyFile('built/include/','panda/src/egg/eggPolygon.h')
CopyFile('built/include/','panda/src/egg/eggPolysetMaker.h')
CopyFile('built/include/','panda/src/egg/eggPoolUniquifier.h')
CopyFile('built/include/','panda/src/egg/eggPrimitive.I')
CopyFile('built/include/','panda/src/egg/eggPrimitive.h')
CopyFile('built/include/','panda/src/egg/eggRenderMode.I')
CopyFile('built/include/','panda/src/egg/eggRenderMode.h')
CopyFile('built/include/','panda/src/egg/eggSAnimData.I')
CopyFile('built/include/','panda/src/egg/eggSAnimData.h')
CopyFile('built/include/','panda/src/egg/eggSurface.I')
CopyFile('built/include/','panda/src/egg/eggSurface.h')
CopyFile('built/include/','panda/src/egg/eggSwitchCondition.h')
CopyFile('built/include/','panda/src/egg/eggTable.I')
CopyFile('built/include/','panda/src/egg/eggTable.h')
CopyFile('built/include/','panda/src/egg/eggTexture.I')
CopyFile('built/include/','panda/src/egg/eggTexture.h')
CopyFile('built/include/','panda/src/egg/eggTextureCollection.I')
CopyFile('built/include/','panda/src/egg/eggTextureCollection.h')
CopyFile('built/include/','panda/src/egg/eggTransform3d.I')
CopyFile('built/include/','panda/src/egg/eggTransform3d.h')
CopyFile('built/include/','panda/src/egg/eggUserData.I')
CopyFile('built/include/','panda/src/egg/eggUserData.h')
CopyFile('built/include/','panda/src/egg/eggUtilities.I')
CopyFile('built/include/','panda/src/egg/eggUtilities.h')
CopyFile('built/include/','panda/src/egg/eggVertex.I')
CopyFile('built/include/','panda/src/egg/eggVertex.h')
CopyFile('built/include/','panda/src/egg/eggVertexPool.I')
CopyFile('built/include/','panda/src/egg/eggVertexPool.h')
CopyFile('built/include/','panda/src/egg/eggVertexUV.I')
CopyFile('built/include/','panda/src/egg/eggVertexUV.h')
CopyFile('built/include/','panda/src/egg/eggXfmAnimData.I')
CopyFile('built/include/','panda/src/egg/eggXfmAnimData.h')
CopyFile('built/include/','panda/src/egg/eggXfmSAnim.I')
CopyFile('built/include/','panda/src/egg/eggXfmSAnim.h')
CopyFile('built/include/','panda/src/egg/pt_EggMaterial.h')
CopyFile('built/include/','panda/src/egg/vector_PT_EggMaterial.h')
CopyFile('built/include/','panda/src/egg/pt_EggTexture.h')
CopyFile('built/include/','panda/src/egg/vector_PT_EggTexture.h')
CopyFile('built/include/','panda/src/egg/pt_EggVertex.h')
CopyFile('built/include/','panda/src/egg/vector_PT_EggVertex.h')
CopyFile('built/include/','panda/src/egg2pg/egg_parametrics.h')
CopyFile('built/include/','panda/src/egg2pg/load_egg_file.h')
CopyFile('built/include/','panda/src/egg2pg/config_egg2pg.h')
CopyFile('built/include/','panda/src/framework/pandaFramework.I')
CopyFile('built/include/','panda/src/framework/pandaFramework.h')
CopyFile('built/include/','panda/src/framework/windowFramework.I')
CopyFile('built/include/','panda/src/framework/windowFramework.h')
CopyFile('built/include/','panda/src/glstuff/glext.h')
CopyFile('built/include/','panda/src/glstuff/glmisc_src.cxx')
CopyFile('built/include/','panda/src/glstuff/glmisc_src.h')
CopyFile('built/include/','panda/src/glstuff/glstuff_src.cxx')
CopyFile('built/include/','panda/src/glstuff/glstuff_src.h')
CopyFile('built/include/','panda/src/glstuff/glstuff_undef_src.h')
CopyFile('built/include/','panda/src/glstuff/glGeomContext_src.cxx')
CopyFile('built/include/','panda/src/glstuff/glGeomContext_src.I')
CopyFile('built/include/','panda/src/glstuff/glGeomContext_src.h')
CopyFile('built/include/','panda/src/glstuff/glGraphicsStateGuardian_src.cxx')
CopyFile('built/include/','panda/src/glstuff/glGraphicsStateGuardian_src.I')
CopyFile('built/include/','panda/src/glstuff/glGraphicsStateGuardian_src.h')
CopyFile('built/include/','panda/src/glstuff/glSavedFrameBuffer_src.cxx')
CopyFile('built/include/','panda/src/glstuff/glSavedFrameBuffer_src.I')
CopyFile('built/include/','panda/src/glstuff/glSavedFrameBuffer_src.h')
CopyFile('built/include/','panda/src/glstuff/glTextureContext_src.cxx')
CopyFile('built/include/','panda/src/glstuff/glTextureContext_src.I')
CopyFile('built/include/','panda/src/glstuff/glTextureContext_src.h')
CopyFile('built/include/','panda/src/glstuff/glCgShaderContext_src.cxx')
CopyFile('built/include/','panda/src/glstuff/glCgShaderContext_src.h')
CopyFile('built/include/','panda/src/glstuff/glCgShaderContext_src.I')
CopyFile('built/include/','panda/src/glgsg/config_glgsg.h')
CopyFile('built/include/','panda/src/glgsg/glgsg.h')
CopyFile('built/include/','panda/metalibs/pandaegg/pandaegg.h')
CopyFile('built/include/','panda/src/wgldisplay/config_wgldisplay.h')
CopyFile('built/include/','panda/src/wgldisplay/wglGraphicsBuffer.I')
CopyFile('built/include/','panda/src/wgldisplay/wglGraphicsBuffer.h')
CopyFile('built/include/','panda/src/wgldisplay/wglGraphicsPipe.I')
CopyFile('built/include/','panda/src/wgldisplay/wglGraphicsPipe.h')
CopyFile('built/include/','panda/src/wgldisplay/wglGraphicsStateGuardian.I')
CopyFile('built/include/','panda/src/wgldisplay/wglGraphicsStateGuardian.h')
CopyFile('built/include/','panda/src/wgldisplay/wglGraphicsWindow.I')
CopyFile('built/include/','panda/src/wgldisplay/wglGraphicsWindow.h')
CopyFile('built/include/','panda/src/wgldisplay/wglext.h')
CopyFile('built/include/','panda/metalibs/pandagl/pandagl.h')
CopyFile('built/include/','panda/src/physics/actorNode.I')
CopyFile('built/include/','panda/src/physics/actorNode.h')
CopyFile('built/include/','panda/src/physics/angularEulerIntegrator.h')
CopyFile('built/include/','panda/src/physics/angularForce.h')
CopyFile('built/include/','panda/src/physics/angularIntegrator.h')
CopyFile('built/include/','panda/src/physics/angularVectorForce.I')
CopyFile('built/include/','panda/src/physics/angularVectorForce.h')
CopyFile('built/include/','panda/src/physics/baseForce.I')
CopyFile('built/include/','panda/src/physics/baseForce.h')
CopyFile('built/include/','panda/src/physics/baseIntegrator.I')
CopyFile('built/include/','panda/src/physics/baseIntegrator.h')
CopyFile('built/include/','panda/src/physics/config_physics.h')
CopyFile('built/include/','panda/src/physics/forceNode.I')
CopyFile('built/include/','panda/src/physics/forceNode.h')
CopyFile('built/include/','panda/src/physics/forces.h')
CopyFile('built/include/','panda/src/physics/linearCylinderVortexForce.I')
CopyFile('built/include/','panda/src/physics/linearCylinderVortexForce.h')
CopyFile('built/include/','panda/src/physics/linearDistanceForce.I')
CopyFile('built/include/','panda/src/physics/linearDistanceForce.h')
CopyFile('built/include/','panda/src/physics/linearEulerIntegrator.h')
CopyFile('built/include/','panda/src/physics/linearForce.I')
CopyFile('built/include/','panda/src/physics/linearForce.h')
CopyFile('built/include/','panda/src/physics/linearFrictionForce.I')
CopyFile('built/include/','panda/src/physics/linearFrictionForce.h')
CopyFile('built/include/','panda/src/physics/linearIntegrator.h')
CopyFile('built/include/','panda/src/physics/linearJitterForce.h')
CopyFile('built/include/','panda/src/physics/linearNoiseForce.I')
CopyFile('built/include/','panda/src/physics/linearNoiseForce.h')
CopyFile('built/include/','panda/src/physics/linearRandomForce.I')
CopyFile('built/include/','panda/src/physics/linearRandomForce.h')
CopyFile('built/include/','panda/src/physics/linearSinkForce.h')
CopyFile('built/include/','panda/src/physics/linearSourceForce.h')
CopyFile('built/include/','panda/src/physics/linearUserDefinedForce.I')
CopyFile('built/include/','panda/src/physics/linearUserDefinedForce.h')
CopyFile('built/include/','panda/src/physics/linearVectorForce.I')
CopyFile('built/include/','panda/src/physics/linearVectorForce.h')
CopyFile('built/include/','panda/src/physics/physical.I')
CopyFile('built/include/','panda/src/physics/physical.h')
CopyFile('built/include/','panda/src/physics/physicalNode.I')
CopyFile('built/include/','panda/src/physics/physicalNode.h')
CopyFile('built/include/','panda/src/physics/physicsCollisionHandler.I')
CopyFile('built/include/','panda/src/physics/physicsCollisionHandler.h')
CopyFile('built/include/','panda/src/physics/physicsManager.I')
CopyFile('built/include/','panda/src/physics/physicsManager.h')
CopyFile('built/include/','panda/src/physics/physicsObject.I')
CopyFile('built/include/','panda/src/physics/physicsObject.h')
CopyFile('built/include/','panda/src/particlesystem/baseParticle.I')
CopyFile('built/include/','panda/src/particlesystem/baseParticle.h')
CopyFile('built/include/','panda/src/particlesystem/baseParticleEmitter.I')
CopyFile('built/include/','panda/src/particlesystem/baseParticleEmitter.h')
CopyFile('built/include/','panda/src/particlesystem/baseParticleFactory.I')
CopyFile('built/include/','panda/src/particlesystem/baseParticleFactory.h')
CopyFile('built/include/','panda/src/particlesystem/baseParticleRenderer.I')
CopyFile('built/include/','panda/src/particlesystem/baseParticleRenderer.h')
CopyFile('built/include/','panda/src/particlesystem/boxEmitter.I')
CopyFile('built/include/','panda/src/particlesystem/boxEmitter.h')
CopyFile('built/include/','panda/src/particlesystem/config_particlesystem.h')
CopyFile('built/include/','panda/src/particlesystem/discEmitter.I')
CopyFile('built/include/','panda/src/particlesystem/discEmitter.h')
CopyFile('built/include/','panda/src/particlesystem/emitters.h')
CopyFile('built/include/','panda/src/particlesystem/geomParticleRenderer.I')
CopyFile('built/include/','panda/src/particlesystem/geomParticleRenderer.h')
CopyFile('built/include/','panda/src/particlesystem/lineEmitter.I')
CopyFile('built/include/','panda/src/particlesystem/lineEmitter.h')
CopyFile('built/include/','panda/src/particlesystem/lineParticleRenderer.I')
CopyFile('built/include/','panda/src/particlesystem/lineParticleRenderer.h')
CopyFile('built/include/','panda/src/particlesystem/particleSystem.I')
CopyFile('built/include/','panda/src/particlesystem/particleSystem.h')
CopyFile('built/include/','panda/src/particlesystem/particleSystemManager.I')
CopyFile('built/include/','panda/src/particlesystem/particleSystemManager.h')
CopyFile('built/include/','panda/src/particlesystem/particlefactories.h')
CopyFile('built/include/','panda/src/particlesystem/particles.h')
CopyFile('built/include/','panda/src/particlesystem/pointEmitter.I')
CopyFile('built/include/','panda/src/particlesystem/pointEmitter.h')
CopyFile('built/include/','panda/src/particlesystem/pointParticle.h')
CopyFile('built/include/','panda/src/particlesystem/pointParticleFactory.h')
CopyFile('built/include/','panda/src/particlesystem/pointParticleRenderer.I')
CopyFile('built/include/','panda/src/particlesystem/pointParticleRenderer.h')
CopyFile('built/include/','panda/src/particlesystem/rectangleEmitter.I')
CopyFile('built/include/','panda/src/particlesystem/rectangleEmitter.h')
CopyFile('built/include/','panda/src/particlesystem/ringEmitter.I')
CopyFile('built/include/','panda/src/particlesystem/ringEmitter.h')
CopyFile('built/include/','panda/src/particlesystem/sparkleParticleRenderer.I')
CopyFile('built/include/','panda/src/particlesystem/sparkleParticleRenderer.h')
CopyFile('built/include/','panda/src/particlesystem/sphereSurfaceEmitter.I')
CopyFile('built/include/','panda/src/particlesystem/sphereSurfaceEmitter.h')
CopyFile('built/include/','panda/src/particlesystem/sphereVolumeEmitter.I')
CopyFile('built/include/','panda/src/particlesystem/sphereVolumeEmitter.h')
CopyFile('built/include/','panda/src/particlesystem/spriteParticleRenderer.I')
CopyFile('built/include/','panda/src/particlesystem/spriteParticleRenderer.h')
CopyFile('built/include/','panda/src/particlesystem/tangentRingEmitter.I')
CopyFile('built/include/','panda/src/particlesystem/tangentRingEmitter.h')
CopyFile('built/include/','panda/src/particlesystem/zSpinParticle.I')
CopyFile('built/include/','panda/src/particlesystem/zSpinParticle.h')
CopyFile('built/include/','panda/src/particlesystem/zSpinParticleFactory.I')
CopyFile('built/include/','panda/src/particlesystem/zSpinParticleFactory.h')
CopyFile('built/include/','panda/src/particlesystem/particleCommonFuncs.h')
CopyFile('built/include/','panda/metalibs/pandaphysics/pandaphysics.h')
CopyFile('built/include/','direct/src/directbase/directbase.h')
CopyFile('built/include/','direct/src/directbase/directsymbols.h')
CopyFile('built/include/','direct/src/deadrec/smoothMover.h')
CopyFile('built/include/','direct/src/deadrec/smoothMover.I')
CopyFile('built/include/','direct/src/interval/config_interval.h')
CopyFile('built/include/','direct/src/interval/cInterval.I')
CopyFile('built/include/','direct/src/interval/cInterval.h')
CopyFile('built/include/','direct/src/interval/cIntervalManager.I')
CopyFile('built/include/','direct/src/interval/cIntervalManager.h')
CopyFile('built/include/','direct/src/interval/cLerpInterval.I')
CopyFile('built/include/','direct/src/interval/cLerpInterval.h')
CopyFile('built/include/','direct/src/interval/cLerpNodePathInterval.I')
CopyFile('built/include/','direct/src/interval/cLerpNodePathInterval.h')
CopyFile('built/include/','direct/src/interval/cLerpAnimEffectInterval.I')
CopyFile('built/include/','direct/src/interval/cLerpAnimEffectInterval.h')
CopyFile('built/include/','direct/src/interval/cMetaInterval.I')
CopyFile('built/include/','direct/src/interval/cMetaInterval.h')
CopyFile('built/include/','direct/src/interval/hideInterval.I')
CopyFile('built/include/','direct/src/interval/hideInterval.h')
CopyFile('built/include/','direct/src/interval/showInterval.I')
CopyFile('built/include/','direct/src/interval/showInterval.h')
CopyFile('built/include/','direct/src/interval/waitInterval.I')
CopyFile('built/include/','direct/src/interval/waitInterval.h')
CopyFile('built/include/','pandatool/src/pandatoolbase/animationConvert.h')
CopyFile('built/include/','pandatool/src/pandatoolbase/config_pandatoolbase.h')
CopyFile('built/include/','pandatool/src/pandatoolbase/distanceUnit.h')
CopyFile('built/include/','pandatool/src/pandatoolbase/pandatoolbase.h')
CopyFile('built/include/','pandatool/src/pandatoolbase/pandatoolsymbols.h')
CopyFile('built/include/','pandatool/src/pandatoolbase/pathReplace.I')
CopyFile('built/include/','pandatool/src/pandatoolbase/pathReplace.h')
CopyFile('built/include/','pandatool/src/pandatoolbase/pathStore.h')
CopyFile('built/include/','pandatool/src/converter/somethingToEggConverter.I')
CopyFile('built/include/','pandatool/src/converter/somethingToEggConverter.h')
CopyFile('built/include/','pandatool/src/progbase/programBase.I')
CopyFile('built/include/','pandatool/src/progbase/programBase.h')
CopyFile('built/include/','pandatool/src/progbase/withOutputFile.I')
CopyFile('built/include/','pandatool/src/progbase/withOutputFile.h')
CopyFile('built/include/','pandatool/src/progbase/wordWrapStream.h')
CopyFile('built/include/','pandatool/src/progbase/wordWrapStreamBuf.I')
CopyFile('built/include/','pandatool/src/progbase/wordWrapStreamBuf.h')
CopyFile('built/include/','pandatool/src/eggbase/eggBase.h')
CopyFile('built/include/','pandatool/src/eggbase/eggConverter.h')
CopyFile('built/include/','pandatool/src/eggbase/eggFilter.h')
CopyFile('built/include/','pandatool/src/eggbase/eggMakeSomething.h')
CopyFile('built/include/','pandatool/src/eggbase/eggMultiBase.h')
CopyFile('built/include/','pandatool/src/eggbase/eggMultiFilter.h')
CopyFile('built/include/','pandatool/src/eggbase/eggReader.h')
CopyFile('built/include/','pandatool/src/eggbase/eggSingleBase.h')
CopyFile('built/include/','pandatool/src/eggbase/eggToSomething.h')
CopyFile('built/include/','pandatool/src/eggbase/eggWriter.h')
CopyFile('built/include/','pandatool/src/eggbase/somethingToEgg.h')
CopyFile('built/include/','pandatool/src/cvscopy/cvsCopy.h')
CopyFile('built/include/','pandatool/src/dxf/dxfFile.h')
CopyFile('built/include/','pandatool/src/dxf/dxfLayer.h')
CopyFile('built/include/','pandatool/src/dxf/dxfLayerMap.h')
CopyFile('built/include/','pandatool/src/dxf/dxfVertex.h')
CopyFile('built/include/','pandatool/src/dxfegg/dxfToEggConverter.h')
CopyFile('built/include/','pandatool/src/dxfegg/dxfToEggLayer.h')
CopyFile('built/include/','pandatool/src/eggcharbase/eggBackPointer.h')
CopyFile('built/include/','pandatool/src/eggcharbase/eggCharacterCollection.I')
CopyFile('built/include/','pandatool/src/eggcharbase/eggCharacterCollection.h')
CopyFile('built/include/','pandatool/src/eggcharbase/eggCharacterData.I')
CopyFile('built/include/','pandatool/src/eggcharbase/eggCharacterData.h')
CopyFile('built/include/','pandatool/src/eggcharbase/eggCharacterFilter.h')
CopyFile('built/include/','pandatool/src/eggcharbase/eggComponentData.I')
CopyFile('built/include/','pandatool/src/eggcharbase/eggComponentData.h')
CopyFile('built/include/','pandatool/src/eggcharbase/eggJointData.h')
CopyFile('built/include/','pandatool/src/eggcharbase/eggJointData.I')
CopyFile('built/include/','pandatool/src/eggcharbase/eggJointPointer.h')
CopyFile('built/include/','pandatool/src/eggcharbase/eggJointPointer.I')
CopyFile('built/include/','pandatool/src/eggcharbase/eggJointNodePointer.h')
CopyFile('built/include/','pandatool/src/eggcharbase/eggMatrixTablePointer.h')
CopyFile('built/include/','pandatool/src/eggcharbase/eggScalarTablePointer.h')
CopyFile('built/include/','pandatool/src/eggcharbase/eggSliderData.I')
CopyFile('built/include/','pandatool/src/eggcharbase/eggSliderData.h')
CopyFile('built/include/','pandatool/src/eggcharbase/eggVertexPointer.h')
CopyFile('built/include/','pandatool/src/flt/fltBead.h')
CopyFile('built/include/','pandatool/src/flt/fltBeadID.h')
CopyFile('built/include/','pandatool/src/flt/fltCurve.I')
CopyFile('built/include/','pandatool/src/flt/fltCurve.h')
CopyFile('built/include/','pandatool/src/flt/fltError.h')
CopyFile('built/include/','pandatool/src/flt/fltExternalReference.h')
CopyFile('built/include/','pandatool/src/flt/fltEyepoint.h')
CopyFile('built/include/','pandatool/src/flt/fltFace.I')
CopyFile('built/include/','pandatool/src/flt/fltFace.h')
CopyFile('built/include/','pandatool/src/flt/fltGeometry.I')
CopyFile('built/include/','pandatool/src/flt/fltGeometry.h')
CopyFile('built/include/','pandatool/src/flt/fltGroup.h')
CopyFile('built/include/','pandatool/src/flt/fltHeader.h')
CopyFile('built/include/','pandatool/src/flt/fltInstanceDefinition.h')
CopyFile('built/include/','pandatool/src/flt/fltInstanceRef.h')
CopyFile('built/include/','pandatool/src/flt/fltLOD.h')
CopyFile('built/include/','pandatool/src/flt/fltLightSourceDefinition.h')
CopyFile('built/include/','pandatool/src/flt/fltLocalVertexPool.I')
CopyFile('built/include/','pandatool/src/flt/fltLocalVertexPool.h')
CopyFile('built/include/','pandatool/src/flt/fltMaterial.h')
CopyFile('built/include/','pandatool/src/flt/fltMesh.I')
CopyFile('built/include/','pandatool/src/flt/fltMesh.h')
CopyFile('built/include/','pandatool/src/flt/fltMeshPrimitive.I')
CopyFile('built/include/','pandatool/src/flt/fltMeshPrimitive.h')
CopyFile('built/include/','pandatool/src/flt/fltObject.h')
CopyFile('built/include/','pandatool/src/flt/fltOpcode.h')
CopyFile('built/include/','pandatool/src/flt/fltPackedColor.I')
CopyFile('built/include/','pandatool/src/flt/fltPackedColor.h')
CopyFile('built/include/','pandatool/src/flt/fltRecord.I')
CopyFile('built/include/','pandatool/src/flt/fltRecord.h')
CopyFile('built/include/','pandatool/src/flt/fltRecordReader.h')
CopyFile('built/include/','pandatool/src/flt/fltRecordWriter.h')
CopyFile('built/include/','pandatool/src/flt/fltTexture.h')
CopyFile('built/include/','pandatool/src/flt/fltTrackplane.h')
CopyFile('built/include/','pandatool/src/flt/fltTransformGeneralMatrix.h')
CopyFile('built/include/','pandatool/src/flt/fltTransformPut.h')
CopyFile('built/include/','pandatool/src/flt/fltTransformRecord.h')
CopyFile('built/include/','pandatool/src/flt/fltTransformRotateAboutEdge.h')
CopyFile('built/include/','pandatool/src/flt/fltTransformRotateAboutPoint.h')
CopyFile('built/include/','pandatool/src/flt/fltTransformRotateScale.h')
CopyFile('built/include/','pandatool/src/flt/fltTransformScale.h')
CopyFile('built/include/','pandatool/src/flt/fltTransformTranslate.h')
CopyFile('built/include/','pandatool/src/flt/fltUnsupportedRecord.h')
CopyFile('built/include/','pandatool/src/flt/fltVectorRecord.h')
CopyFile('built/include/','pandatool/src/flt/fltVertex.I')
CopyFile('built/include/','pandatool/src/flt/fltVertex.h')
CopyFile('built/include/','pandatool/src/flt/fltVertexList.h')
CopyFile('built/include/','pandatool/src/fltegg/fltToEggConverter.I')
CopyFile('built/include/','pandatool/src/fltegg/fltToEggConverter.h')
CopyFile('built/include/','pandatool/src/fltegg/fltToEggLevelState.I')
CopyFile('built/include/','pandatool/src/fltegg/fltToEggLevelState.h')
CopyFile('built/include/','pandatool/src/imagebase/imageBase.h')
CopyFile('built/include/','pandatool/src/imagebase/imageFilter.h')
CopyFile('built/include/','pandatool/src/imagebase/imageReader.h')
CopyFile('built/include/','pandatool/src/imagebase/imageWriter.I')
CopyFile('built/include/','pandatool/src/imagebase/imageWriter.h')
CopyFile('built/include/','pandatool/src/lwo/iffChunk.I')
CopyFile('built/include/','pandatool/src/lwo/iffChunk.h')
CopyFile('built/include/','pandatool/src/lwo/iffGenericChunk.I')
CopyFile('built/include/','pandatool/src/lwo/iffGenericChunk.h')
CopyFile('built/include/','pandatool/src/lwo/iffId.I')
CopyFile('built/include/','pandatool/src/lwo/iffId.h')
CopyFile('built/include/','pandatool/src/lwo/iffInputFile.I')
CopyFile('built/include/','pandatool/src/lwo/iffInputFile.h')
CopyFile('built/include/','pandatool/src/lwo/lwoBoundingBox.h')
CopyFile('built/include/','pandatool/src/lwo/lwoChunk.h')
CopyFile('built/include/','pandatool/src/lwo/lwoClip.h')
CopyFile('built/include/','pandatool/src/lwo/lwoDiscontinuousVertexMap.h')
CopyFile('built/include/','pandatool/src/lwo/lwoGroupChunk.h')
CopyFile('built/include/','pandatool/src/lwo/lwoHeader.I')
CopyFile('built/include/','pandatool/src/lwo/lwoHeader.h')
CopyFile('built/include/','pandatool/src/lwo/lwoInputFile.I')
CopyFile('built/include/','pandatool/src/lwo/lwoInputFile.h')
CopyFile('built/include/','pandatool/src/lwo/lwoLayer.h')
CopyFile('built/include/','pandatool/src/lwo/lwoPoints.h')
CopyFile('built/include/','pandatool/src/lwo/lwoPolygons.h')
CopyFile('built/include/','pandatool/src/lwo/lwoPolygonTags.h')
CopyFile('built/include/','pandatool/src/lwo/lwoTags.h')
CopyFile('built/include/','pandatool/src/lwo/lwoStillImage.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurface.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlock.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockAxis.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockChannel.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockCoordSys.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockEnabled.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockImage.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockOpacity.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockProjection.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockHeader.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockRefObj.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockRepeat.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockTMap.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockTransform.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockVMapName.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceBlockWrap.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceColor.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceParameter.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceSidedness.h')
CopyFile('built/include/','pandatool/src/lwo/lwoSurfaceSmoothingAngle.h')
CopyFile('built/include/','pandatool/src/lwo/lwoVertexMap.h')
CopyFile('built/include/','pandatool/src/lwoegg/lwoToEggConverter.I')
CopyFile('built/include/','pandatool/src/lwoegg/lwoToEggConverter.h')
CopyFile('built/include/','pandatool/src/vrmlegg/indexedFaceSet.h')
CopyFile('built/include/','pandatool/src/vrmlegg/vrmlAppearance.h')
CopyFile('built/include/','pandatool/src/vrmlegg/vrmlToEggConverter.h')
CopyFile('built/include/','pandatool/src/ptloader/config_ptloader.h')
CopyFile('built/include/','pandatool/src/ptloader/loaderFileTypePandatool.h')
CopyFile('built/include/','pandatool/src/pstatserver/pStatClientData.h')
CopyFile('built/include/','pandatool/src/pstatserver/pStatGraph.I')
CopyFile('built/include/','pandatool/src/pstatserver/pStatGraph.h')
CopyFile('built/include/','pandatool/src/pstatserver/pStatListener.h')
CopyFile('built/include/','pandatool/src/pstatserver/pStatMonitor.I')
CopyFile('built/include/','pandatool/src/pstatserver/pStatMonitor.h')
CopyFile('built/include/','pandatool/src/pstatserver/pStatPianoRoll.I')
CopyFile('built/include/','pandatool/src/pstatserver/pStatPianoRoll.h')
CopyFile('built/include/','pandatool/src/pstatserver/pStatReader.h')
CopyFile('built/include/','pandatool/src/pstatserver/pStatServer.h')
CopyFile('built/include/','pandatool/src/pstatserver/pStatStripChart.I')
CopyFile('built/include/','pandatool/src/pstatserver/pStatStripChart.h')
CopyFile('built/include/','pandatool/src/pstatserver/pStatThreadData.I')
CopyFile('built/include/','pandatool/src/pstatserver/pStatThreadData.h')
CopyFile('built/include/','pandatool/src/pstatserver/pStatView.I')
CopyFile('built/include/','pandatool/src/pstatserver/pStatView.h')
CopyFile('built/include/','pandatool/src/pstatserver/pStatViewLevel.I')
CopyFile('built/include/','pandatool/src/pstatserver/pStatViewLevel.h')
CopyFile('built/include/','pandaapp/src/pandaappbase/pandaappbase.h')
CopyFile('built/include/','pandaapp/src/pandaappbase/pandaappsymbols.h')
CopyFile('built/include/','pandaapp/src/stitchbase/config_stitch.h')
CopyFile('built/include/','pandaapp/src/stitchbase/fadeImagePool.I')
CopyFile('built/include/','pandaapp/src/stitchbase/fadeImagePool.h')
CopyFile('built/include/','pandaapp/src/stitchbase/fixedPoint.h')
CopyFile('built/include/','pandaapp/src/stitchbase/layeredImage.h')
CopyFile('built/include/','pandaapp/src/stitchbase/morphGrid.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchCommand.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchCommandReader.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchCylindricalLens.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchFile.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchFisheyeLens.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchImage.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchImageCommandOutput.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchImageOutputter.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchImageRasterizer.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchLens.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchLexerDefs.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchPSphereLens.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchParserDefs.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchPerspectiveLens.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchPoint.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitcher.h')
CopyFile('built/include/','pandaapp/src/stitchbase/triangle.h')
CopyFile('built/include/','pandaapp/src/stitchbase/triangleRasterizer.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchCylindricalScreen.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchFlatScreen.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchMultiScreen.h')
CopyFile('built/include/','pandaapp/src/stitchbase/stitchScreen.h')

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
CompileLIB(lib='libcppParser.lib', obj=[
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
             'libcppParser.lib',
             'libpystub.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

CompileC(ipath=IPATH, opts=OPTS, src='interrogate_module.cxx', obj='interrogate_module_interrogate_module.obj')
CompileLink(opts=['ADVAPI', 'NSPR', 'SSL'], dll='interrogate_module.exe', obj=[
             'interrogate_module_interrogate_module.obj',
             'libcppParser.lib',
             'libpystub.dll',
             'libdtoolconfig.dll',
             'libdtool.dll',
])

CompileC(ipath=IPATH, opts=OPTS, src='parse_file.cxx', obj='parse_file_parse_file.obj')
CompileLink(opts=['ADVAPI', 'NSPR', 'SSL'], dll='parse_file.exe', obj=[
             'parse_file_parse_file.obj',
             'libcppParser.lib',
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

IPATH=['panda/src/net']
OPTS=['BUILDING_PANDA', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='net_composite1.cxx', obj='net_composite1.obj')
CompileC(ipath=IPATH, opts=OPTS, src='net_composite2.cxx', obj='net_composite2.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libnet.in', outc='libnet_igate.cxx',
            src='panda/src/net',  module='panda', library='libnet', files=[
            'config_net.h', 'connection.h', 'connectionListener.h', 'connectionManager.h', 'connectionReader.h', 'connectionWriter.h', 'datagramQueue.h', 'datagramTCPHeader.h', 'datagramUDPHeader.h', 'netAddress.h', 'netDatagram.h', 'pprerror.h', 'queuedConnectionListener.h', 'queuedConnectionManager.h', 'queuedConnectionReader.h', 'recentConnectionReader.h', 'queuedReturn.h', 'net_composite1.cxx', 'net_composite2.cxx'])
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
  CompileLIB(lib='libhelix.lib', obj=[
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

IPATH=['panda/src/vrpn']
OPTS=['BUILDING_PANDA', 'NSPR', 'VRPN']
CompileC(ipath=IPATH, opts=OPTS, src='config_vrpn.cxx', obj='pvrpn_config_vrpn.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrpnClient.cxx', obj='pvrpn_vrpnClient.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrpnAnalog.cxx', obj='pvrpn_vrpnAnalog.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrpnAnalogDevice.cxx', obj='pvrpn_vrpnAnalogDevice.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrpnButton.cxx', obj='pvrpn_vrpnButton.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrpnButtonDevice.cxx', obj='pvrpn_vrpnButtonDevice.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrpnDial.cxx', obj='pvrpn_vrpnDial.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrpnDialDevice.cxx', obj='pvrpn_vrpnDialDevice.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrpnTracker.cxx', obj='pvrpn_vrpnTracker.obj')
CompileC(ipath=IPATH, opts=OPTS, src='vrpnTrackerDevice.cxx', obj='pvrpn_vrpnTrackerDevice.obj')
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
OPTS=['BUILDING_PANDA', 'ZLIB', 'NSPR', 'VRPN', 'JPEG', 'TIFF', 'FREETYPE']
INFILES=['librecorder.in', 'libpgraph.in', 'libpvrpn.in', 'libgrutil.in', 'libchan.in', 'libpstatclient.in',
         'libchar.in', 'libcollide.in', 'libdevice.in', 'libdgraph.in', 'libdisplay.in', 'libevent.in',
         'libgobj.in', 'libgsgbase.in', 'liblinmath.in', 'libmathutil.in', 'libnet.in', 'libparametrics.in',
         'libpnmimage.in', 'libtext.in', 'libtform.in', 'liblerp.in', 'libputil.in', 'libaudio.in',
         'libpgui.in']
OBJFILES=['panda_panda.obj', 'libpanda_module.obj', 'recorder_composite1.obj',
          'recorder_composite2.obj', 'librecorder_igate.obj',
          'pgraph_nodePath.obj', 'pgraph_composite1.obj', 'pgraph_composite2.obj', 'libpgraph_igate.obj',
          'pvrpn_config_vrpn.obj', 'pvrpn_vrpnClient.obj', 'pvrpn_vrpnAnalog.obj',
          'pvrpn_vrpnAnalogDevice.obj', 'pvrpn_vrpnButton.obj', 'pvrpn_vrpnButtonDevice.obj',
          'pvrpn_vrpnDial.obj', 'pvrpn_vrpnDialDevice.obj', 'pvrpn_vrpnTracker.obj',
          'pvrpn_vrpnTrackerDevice.obj', 'libpvrpn_igate.obj',
          'grutil_multitexReducer.obj', 'grutil_composite1.obj', 'libgrutil_igate.obj',
          'chan_composite1.obj', 'chan_composite2.obj', 'libchan_igate.obj', 'pstatclient_composite1.obj',
          'pstatclient_composite2.obj', 'libpstatclient_igate.obj', 'char_composite1.obj',
          'char_composite2.obj', 'libchar_igate.obj', 'collide_composite1.obj', 'collide_composite2.obj',
          'libcollide_igate.obj', 'device_composite1.obj', 'device_composite2.obj', 'libdevice_igate.obj',
          'dgraph_composite1.obj', 'dgraph_composite2.obj', 'libdgraph_igate.obj', 'display_composite1.obj',
          'display_composite2.obj', 'libdisplay_igate.obj', 'event_composite1.obj', 'libevent_igate.obj',
          'gobj_composite1.obj', 'gobj_composite2.obj', 'libgobj_igate.obj', 'gsgbase_composite1.obj',
          'libgsgbase_igate.obj', 'gsgmisc_geomIssuer.obj', 'linmath_composite1.obj',
          'linmath_composite2.obj', 'liblinmath_igate.obj', 'mathutil_composite1.obj', 'mathutil_composite2.obj',
          'libmathutil_igate.obj', 'net_composite1.obj', 'net_composite2.obj',
          'libnet_igate.obj', 'parametrics_composite1.obj', 'parametrics_composite2.obj', 'libparametrics_igate.obj',
          'pnmimagetypes_pnmFileTypePNG.obj', 'pnmimagetypes_pnmFileTypeTIFF.obj', 'pnmimagetypes_composite1.obj',
          'pnmimagetypes_composite2.obj', 'pnmimage_composite1.obj', 'pnmimage_composite2.obj', 'libpnmimage_igate.obj',
          'pnmtext_config_pnmtext.obj', 'pnmtext_freetypeFont.obj', 'pnmtext_pnmTextGlyph.obj', 'pnmtext_pnmTextMaker.obj',
          'text_composite1.obj', 'text_composite2.obj', 'libtext_igate.obj', 'tform_composite1.obj', 'tform_composite2.obj',
          'libtform_igate.obj', 'lerp_composite1.obj', 'liblerp_igate.obj', 'putil_composite1.obj', 'putil_composite2.obj',
          'libputil_igate.obj', 'audio_composite1.obj', 'libaudio_igate.obj', 'pgui_composite1.obj', 'pgui_composite2.obj',
          'libpgui_igate.obj', 'pandabase_pandabase.obj', 'libpandaexpress.dll', 'libdtoolconfig.dll', 'libdtool.dll']
LINKOPTS=['ADVAPI', 'WINSOCK2', 'WINUSER', 'WINMM', 'VRPN', 'NSPR', 'ZLIB', 'JPEG', 'PNG', 'TIFF', 'FFTW', 'FREETYPE']
LINKXDEP=[]
if OMIT.count("HELIX")==0:
  OPTS.append('HELIX')
  OBJFILES.append("libhelix.lib")
  INFILES.append("libhelix.in")
  LINKOPTS.append('HELIX')
  LINKXDEP.append('built/tmp/dtool_have_helix.dat')
InterrogateModule(outc='libpanda_module.cxx', module='panda', library='libpanda', files=INFILES)
CompileC(ipath=IPATH, opts=OPTS, src='panda.cxx', obj='panda_panda.obj')
CompileC(ipath=IPATH, opts=OPTS, src='libpanda_module.cxx', obj='libpanda_module.obj')
CompileLink(opts=['ADVAPI', 'WINSOCK2', 'WINUSER', 'WINMM', 'HELIX', 'VRPN', 'NSPR',
                  'ZLIB', 'JPEG', 'PNG', 'TIFF', 'FFTW', 'FREETYPE'],
            xdep=['built/tmp/dtool_have_helix.dat'],  dll='libpanda.dll', obj=OBJFILES)

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
  CompileC(ipath=IPATH, opts=OPTS, src='config_glxdisplay.cxx',        obj='glxdisplay_config_glxdisplay.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='glxGraphicsBuffer.cxx',        obj='glxdisplay_glxGraphicsBuffer.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='glxGraphicsPipe.cxx',          obj='glxdisplay_glxGraphicsPipe.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='glxGraphicsStateGuardian.cxx', obj='glxdisplay_glxGraphicsStateGuardian.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='glxGraphicsWindow.cxx',        obj='glxdisplay_glxGraphicsWindow.obj')
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
    'glxdisplay_config_glxdisplay.obj',
    'glxdisplay_glxGraphicsBuffer.obj',
    'glxdisplay_glxGraphicsPipe.obj',
    'glxdisplay_glxGraphicsStateGuardian.obj',
    'glxdisplay_glxGraphicsWindow.obj',
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
            files=['dcAtomicField.h', 'dcClass.h', 'dcDeclaration.h', 'dcField.h', 'dcFile.h', 'dcLexerDefs.h', 'dcMolecularField.h', 'dcParserDefs.h', 'dcSubatomicType.h', 'dcPackData.h', 'dcPacker.h', 'dcPackerCatalog.h', 'dcPackerInterface.h', 'dcParameter.h', 'dcClassParameter.h', 'dcArrayParameter.h', 'dcSimpleParameter.h', 'dcSwitchParameter.h', 'dcNumericRange.h', 'dcSwitch.h', 'dcTypedef.h', 'dcPython.h', 'dcbase.h', 'dcindent.h', 'hashGenerator.h', 'primeNumberGenerator.h', 'dcparser_composite1.cxx', 'dcparser_composite2.cxx'])
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
            files=['config_distributed.cxx', 'config_distributed.h', 'cConnectionRepository.cxx', 'cConnectionRepository.h', 'cDistributedSmoothNodeBase.cxx', 'cDistributedSmoothNodeBase.h'])
CompileC(ipath=IPATH, opts=OPTS, src='libdistributed_igate.cxx', obj='libdistributed_igate.obj')

#
# DIRECTORY: direct/src/interval/
#

IPATH=['direct/src/interval']
OPTS=['BUILDING_DIRECT', 'NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='config_interval.cxx', obj='interval_config_interval.obj')
CompileC(ipath=IPATH, opts=OPTS, src='cInterval.cxx', obj='interval_cInterval.obj')
CompileC(ipath=IPATH, opts=OPTS, src='cIntervalManager.cxx', obj='interval_cIntervalManager.obj')
CompileC(ipath=IPATH, opts=OPTS, src='cLerpInterval.cxx', obj='interval_cLerpInterval.obj')
CompileC(ipath=IPATH, opts=OPTS, src='cLerpNodePathInterval.cxx', obj='interval_cLerpNodePathInterval.obj')
CompileC(ipath=IPATH, opts=OPTS, src='cLerpAnimEffectInterval.cxx', obj='interval_cLerpAnimEffectInterval.obj')
CompileC(ipath=IPATH, opts=OPTS, src='cMetaInterval.cxx', obj='interval_cMetaInterval.obj')
CompileC(ipath=IPATH, opts=OPTS, src='hideInterval.cxx', obj='interval_hideInterval.obj')
CompileC(ipath=IPATH, opts=OPTS, src='showInterval.cxx', obj='interval_showInterval.obj')
CompileC(ipath=IPATH, opts=OPTS, src='waitInterval.cxx', obj='interval_waitInterval.obj')
Interrogate(ipath=IPATH, opts=OPTS, outd='libinterval.in', outc='libinterval_igate.cxx',
            src='direct/src/interval',  module='direct', library='libinterval',
            files=['config_interval.cxx', 'config_interval.h', 'cInterval.cxx', 'cInterval.h', 'cIntervalManager.cxx', 'cIntervalManager.h', 'cLerpInterval.cxx', 'cLerpInterval.h', 'cLerpNodePathInterval.cxx', 'cLerpNodePathInterval.h', 'cLerpAnimEffectInterval.cxx', 'cLerpAnimEffectInterval.h', 'cMetaInterval.cxx', 'cMetaInterval.h', 'hideInterval.cxx', 'hideInterval.h', 'showInterval.cxx', 'showInterval.h', 'waitInterval.cxx', 'waitInterval.h', 'lerp_helpers.h'])
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
             'interval_config_interval.obj',
             'interval_cInterval.obj',
             'interval_cIntervalManager.obj',
             'interval_cLerpInterval.obj',
             'interval_cLerpNodePathInterval.obj',
             'interval_cLerpAnimEffectInterval.obj',
             'interval_cMetaInterval.obj',
             'interval_hideInterval.obj',
             'interval_showInterval.obj',
             'interval_waitInterval.obj',
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
CompileC(ipath=IPATH, opts=OPTS, src='animationConvert.cxx', obj='pandatoolbase_animationConvert.obj')
CompileC(ipath=IPATH, opts=OPTS, src='config_pandatoolbase.cxx', obj='pandatoolbase_config_pandatoolbase.obj')
CompileC(ipath=IPATH, opts=OPTS, src='distanceUnit.cxx', obj='pandatoolbase_distanceUnit.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pandatoolbase.cxx', obj='pandatoolbase_pandatoolbase.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pathReplace.cxx', obj='pandatoolbase_pathReplace.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pathStore.cxx', obj='pandatoolbase_pathStore.obj')
CompileLIB(lib='libpandatoolbase.lib', obj=[
             'pandatoolbase_animationConvert.obj',
             'pandatoolbase_config_pandatoolbase.obj',
             'pandatoolbase_distanceUnit.obj',
             'pandatoolbase_pandatoolbase.obj',
             'pandatoolbase_pathReplace.obj',
             'pandatoolbase_pathStore.obj',
])

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
CompileC(ipath=IPATH, opts=OPTS, src='dxfFile.cxx', obj='dxf_dxfFile.obj')
CompileC(ipath=IPATH, opts=OPTS, src='dxfLayer.cxx', obj='dxf_dxfLayer.obj')
CompileC(ipath=IPATH, opts=OPTS, src='dxfLayerMap.cxx', obj='dxf_dxfLayerMap.obj')
CompileC(ipath=IPATH, opts=OPTS, src='dxfVertex.cxx', obj='dxf_dxfVertex.obj')
CompileLIB(lib='libdxf.lib', obj=[
             'dxf_dxfFile.obj',
             'dxf_dxfLayer.obj',
             'dxf_dxfLayerMap.obj',
             'dxf_dxfVertex.obj',
])

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
    CompileC(ipath=IPATH, opts=OPTS, src='config_maya.cxx',        obj='maya'+VER+'_config_maya.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='mayaApi.cxx',            obj='maya'+VER+'_mayaApi.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='mayaShader.cxx',         obj='maya'+VER+'_mayaShader.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='mayaShaderColorDef.cxx', obj='maya'+VER+'_mayaShaderColorDef.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='mayaShaders.cxx',        obj='maya'+VER+'_mayaShaders.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='maya_funcs.cxx',         obj='maya'+VER+'_funcs.obj')
    CompileLIB(lib='libmaya'+VER+'.lib', obj=[
                 'maya'+VER+'_config_maya.obj',
                 'maya'+VER+'_mayaApi.obj',
                 'maya'+VER+'_mayaShader.obj',
                 'maya'+VER+'_mayaShaderColorDef.obj',
                 'maya'+VER+'_mayaShaders.obj',
                 'maya'+VER+'_funcs.obj',
    ])

#
# DIRECTORY: pandatool/src/mayaegg/
#

for VER in ["5","6"]:
  if (OMIT.count("MAYA"+VER)==0):
    IPATH=['pandatool/src/mayaegg', 'pandatool/src/maya']
    OPTS=['MAYA'+VER, 'NSPR']
    CompileC(ipath=IPATH, opts=OPTS, src='config_mayaegg.cxx',       obj='mayaegg'+VER+'_config_mayaegg.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='mayaEggGroupUserData.cxx', obj='mayaegg'+VER+'_mayaEggGroupUserData.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='mayaBlendDesc.cxx',        obj='mayaegg'+VER+'_mayaBlendDesc.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='mayaNodeDesc.cxx',         obj='mayaegg'+VER+'_mayaNodeDesc.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='mayaNodeTree.cxx',         obj='mayaegg'+VER+'_mayaNodeTree.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='mayaToEggConverter.cxx',   obj='mayaegg'+VER+'_mayaToEggConverter.obj')
    CompileLIB(lib='libmayaegg'+VER+'.lib', obj=[
                 'mayaegg'+VER+'_config_mayaegg.obj',
                 'mayaegg'+VER+'_mayaEggGroupUserData.obj',
                 'mayaegg'+VER+'_mayaBlendDesc.obj',
                 'mayaegg'+VER+'_mayaNodeDesc.obj',
                 'mayaegg'+VER+'_mayaNodeTree.obj',
                 'mayaegg'+VER+'_mayaToEggConverter.obj'
    ])

#
# DIRECTORY: pandatool/src/maxegg/
#

for VER in ["5", "6", "7"]:
  if (OMIT.count("MAX"+VER)==0):
    IPATH=['pandatool/src/maxegg']
    OPTS=['MAX'+VER, 'NSPR', "WINCOMCTL", "WINUSER"]
    CompileC(ipath=IPATH, opts=OPTS, src='DllEntry.cpp',         obj='maxegg'+VER+'_DllEntry.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='Logger.cpp',           obj='maxegg'+VER+'_Logger.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='MaxEgg.cpp',           obj='maxegg'+VER+'_MaxEgg.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='maxNodeDesc.cxx',      obj='maxegg'+VER+'_maxNodeDesc.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='MaxNodeTree.cxx',      obj='maxegg'+VER+'_MaxNodeTree.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='MaxToEgg.cpp',         obj='maxegg'+VER+'_MaxToEgg.obj')
    CompileC(ipath=IPATH, opts=OPTS, src='MaxToEggConverter.cxx',obj='maxegg'+VER+'_MaxToEggConverter.obj')
    CompileRES(ipath=IPATH, opts=OPTS, src='MaxEgg.rc',          obj='maxegg'+VER+'_MaxEgg.res')

    CompileLink(opts = OPTS, dll='maxegg'+VER+'.dle', obj=[
                'maxegg'+VER+'_DllEntry.obj',
                'maxegg'+VER+'_Logger.obj',
                'maxegg'+VER+'_MaxEgg.obj',
                'maxegg'+VER+'_maxNodeDesc.obj',
                'maxegg'+VER+'_MaxNodeTree.obj',
                'maxegg'+VER+'_MaxToEgg.obj',
                'maxegg'+VER+'_MaxToEggConverter.obj',
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
    CompileLink(dll='libmayapview'+VER+'.dll',                 opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
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
    CompileLink(dll='libmayasavepview.dll',                 opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
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
    CompileLink(dll='mayacopy'+VER+'.exe',                 opts=['ADVAPI', 'NSPR', 'MAYA'+VER], obj=[
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

IPATH=['pandatool/src/pstatserver']
OPTS=['NSPR']
CompileC(ipath=IPATH, opts=OPTS, src='pStatClientData.cxx', obj='pstatserver_pStatClientData.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pStatGraph.cxx', obj='pstatserver_pStatGraph.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pStatListener.cxx', obj='pstatserver_pStatListener.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pStatMonitor.cxx', obj='pstatserver_pStatMonitor.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pStatPianoRoll.cxx', obj='pstatserver_pStatPianoRoll.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pStatReader.cxx', obj='pstatserver_pStatReader.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pStatServer.cxx', obj='pstatserver_pStatServer.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pStatStripChart.cxx', obj='pstatserver_pStatStripChart.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pStatThreadData.cxx', obj='pstatserver_pStatThreadData.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pStatView.cxx', obj='pstatserver_pStatView.obj')
CompileC(ipath=IPATH, opts=OPTS, src='pStatViewLevel.cxx', obj='pstatserver_pStatViewLevel.obj')
CompileLIB(lib='libpstatserver.lib', obj=[
             'pstatserver_pStatClientData.obj',
             'pstatserver_pStatGraph.obj',
             'pstatserver_pStatListener.obj',
             'pstatserver_pStatMonitor.obj',
             'pstatserver_pStatPianoRoll.obj',
             'pstatserver_pStatReader.obj',
             'pstatserver_pStatServer.obj',
             'pstatserver_pStatStripChart.obj',
             'pstatserver_pStatThreadData.obj',
             'pstatserver_pStatView.obj',
             'pstatserver_pStatViewLevel.obj',
])

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

if (sys.platform == "win32"):
  IPATH=['pandatool/src/win-stats']
  OPTS=['NSPR']
  CompileC(ipath=IPATH, opts=OPTS, src='winStats.cxx', obj='pstats_winStats.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='winStatsChartMenu.cxx', obj='pstats_winStatsChartMenu.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='winStatsGraph.cxx', obj='pstats_winStatsGraph.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='winStatsLabel.cxx', obj='pstats_winStatsLabel.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='winStatsLabelStack.cxx', obj='pstats_winStatsLabelStack.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='winStatsMonitor.cxx', obj='pstats_winStatsMonitor.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='winStatsPianoRoll.cxx', obj='pstats_winStatsPianoRoll.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='winStatsServer.cxx', obj='pstats_winStatsServer.obj')
  CompileC(ipath=IPATH, opts=OPTS, src='winStatsStripChart.cxx', obj='pstats_winStatsStripChart.obj')
  CompileLink(opts=['WINSOCK', 'WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM', 'NSPR'],
              dll='pstats.exe', obj=[
              'pstats_winStats.obj',
              'pstats_winStatsChartMenu.obj',
              'pstats_winStatsGraph.obj',
              'pstats_winStatsLabel.obj',
              'pstats_winStatsLabelStack.obj',
              'pstats_winStatsMonitor.obj',
              'pstats_winStatsPianoRoll.obj',
              'pstats_winStatsServer.obj',
              'pstats_winStatsStripChart.obj',
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
CompileC(ipath=IPATH, opts=OPTS, src='config_stitch.cxx', obj='stitchbase_config_stitch.obj')
CompileC(ipath=IPATH, opts=OPTS, src='fadeImagePool.cxx', obj='stitchbase_fadeImagePool.obj')
CompileC(ipath=IPATH, opts=OPTS, src='layeredImage.cxx', obj='stitchbase_layeredImage.obj')
CompileC(ipath=IPATH, opts=OPTS, src='morphGrid.cxx', obj='stitchbase_morphGrid.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchCommand.cxx', obj='stitchbase_stitchCommand.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchCommandReader.cxx', obj='stitchbase_stitchCommandReader.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchCylindricalLens.cxx', obj='stitchbase_stitchCylindricalLens.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchFile.cxx', obj='stitchbase_stitchFile.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchFisheyeLens.cxx', obj='stitchbase_stitchFisheyeLens.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchImage.cxx', obj='stitchbase_stitchImage.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchImageCommandOutput.cxx', obj='stitchbase_stitchImageCommandOutput.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchImageOutputter.cxx', obj='stitchbase_stitchImageOutputter.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchImageRasterizer.cxx', obj='stitchbase_stitchImageRasterizer.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchLens.cxx', obj='stitchbase_stitchLens.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchPSphereLens.cxx', obj='stitchbase_stitchPSphereLens.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchPerspectiveLens.cxx', obj='stitchbase_stitchPerspectiveLens.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchPoint.cxx', obj='stitchbase_stitchPoint.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitcher.cxx', obj='stitchbase_stitcher.obj')
CompileC(ipath=IPATH, opts=OPTS, src='triangle.cxx', obj='stitchbase_triangle.obj')
CompileC(ipath=IPATH, opts=OPTS, src='triangleRasterizer.cxx', obj='stitchbase_triangleRasterizer.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchCylindricalScreen.cxx', obj='stitchbase_stitchCylindricalScreen.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchFlatScreen.cxx', obj='stitchbase_stitchFlatScreen.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchMultiScreen.cxx', obj='stitchbase_stitchMultiScreen.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchScreen.cxx', obj='stitchbase_stitchScreen.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchParser.cxx', obj='stitchbase_stitchParser.obj')
CompileC(ipath=IPATH, opts=OPTS, src='stitchLexer.cxx', obj='stitchbase_stitchLexer.obj')
CompileLIB(lib='libstitchbase.lib', obj=[
             'stitchbase_config_stitch.obj',
             'stitchbase_fadeImagePool.obj',
             'stitchbase_layeredImage.obj',
             'stitchbase_morphGrid.obj',
             'stitchbase_stitchCommand.obj',
             'stitchbase_stitchCommandReader.obj',
             'stitchbase_stitchCylindricalLens.obj',
             'stitchbase_stitchFile.obj',
             'stitchbase_stitchFisheyeLens.obj',
             'stitchbase_stitchImage.obj',
             'stitchbase_stitchImageCommandOutput.obj',
             'stitchbase_stitchImageOutputter.obj',
             'stitchbase_stitchImageRasterizer.obj',
             'stitchbase_stitchLens.obj',
             'stitchbase_stitchPSphereLens.obj',
             'stitchbase_stitchPerspectiveLens.obj',
             'stitchbase_stitchPoint.obj',
             'stitchbase_stitcher.obj',
             'stitchbase_triangle.obj',
             'stitchbase_triangleRasterizer.obj',
             'stitchbase_stitchCylindricalScreen.obj',
             'stitchbase_stitchFlatScreen.obj',
             'stitchbase_stitchMultiScreen.obj',
             'stitchbase_stitchScreen.obj',
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
# Run genpycode
#
##########################################################################################

if (older('built/lib/pandac/PandaModules.pyz',xpaths("built/etc/",ALLIN,""))):
  ALLTARGETS.append('built/lib/pandac/PandaModules.pyz')
  if (sys.platform=="win32"):
    oscmd(backslashify("built/bin/genpycode.exe"))
  else:
    oscmd(backslashify("built/bin/genpycode"))
  updatefiledate('built/lib/pandac/PandaModules.pyz')

########################################################################
##
## Save the CXX include-cache for next time.
##
########################################################################

try: icache = open("makepanda-icache",'wb')
except: icache = 0
if (icache!=0):
  cPickle.dump(CxxIncludeCache, icache, 1)
  icache.close()

##########################################################################################
#
# 'Complete' mode.
#
# Copies the samples, models, and direct into the build. Note that
# this isn't usually what you want.  It is usually better to let the
# compiled panda load this stuff directly from the source tree.
# The only time you really want to do this is if you plan to move
# the build somewhere and leave the source tree behind.
#
##########################################################################################

if (COMPLETE):
  CopyFile('built/', 'InstallerNotes')
  CopyFile('built/', 'LICENSE')
  CopyFile('built/', 'README')
  CopyTree('built/samples', 'samples')
  CopyTree('built/models', 'models')
  CopyTree('built/direct/src', 'direct/src')
  CopyTree('built/SceneEditor', 'SceneEditor')

##########################################################################################
#
# The Installer
#
##########################################################################################

if (INSTALLER):
  if (sys.platform == "win32"):
    if (older('panda3d-install.exe', ALLTARGETS)):
      VERSION = str(VERSION1)+"-"+str(VERSION2)+"-"+str(VERSION3)
      print("Building installer. This can take up to an hour.")
      if (COMPRESSOR != "lzma"): print("Note: you are using zlib, which is faster, but lzma gives better compression.")
      if (os.path.exists("panda3d-"+VERSION+".exe")):
        os.remove("panda3d-"+VERSION+".exe")
      oscmd("thirdparty\\win-nsis\\makensis.exe /V2 /DCOMPRESSOR="+COMPRESSOR+" /DVERSION="+VERSION+" thirdparty\\win-nsis\\panda.nsi")
      os.rename("panda3d-install-TMP.exe", "panda3d-"+VERSION+".exe")
  else:
    # Do an rpmbuild or something like that.
    pass

##########################################################################################
#
# Print final status report.
#
##########################################################################################

printStatus("Makepanda Final Status Report", WARNINGS)

