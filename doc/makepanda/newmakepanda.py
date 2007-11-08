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

import sys,os,time,stat,string,re,getopt,cPickle,fnmatch,threading,Queue,signal,shutil

from makepandacore import *

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
VC80CRTVERSION=""
OPTIMIZE="3"
INSTALLER=0
GENMAN=0
VERBOSE=1
COMPRESSOR="zlib"
THREADCOUNT=0
DXVERSIONS=["8","9"]
MAYAVERSIONS=["6","65","7","8","85"]
MAXVERSIONS=["6","7","8","9"]

PkgListSet([
  "PYTHON","ZLIB","PNG","JPEG","TIFF","VRPN","FMOD","FMODEX",
  "OPENAL","NVIDIACG","OPENSSL","FREETYPE","FFTW","MILES",
  "ARTOOLKIT","DIRECTCAM","MAYA6","MAYA65","MAYA7","MAYA8",
  "MAYA85","MAX6","MAX7","MAX8","MAX9","FFMPEG","PANDATOOL",
  "PANDAAPP","DX8","DX9"
])

CheckPandaSourceTree()

VERSION=ParsePandaVersion("dtool/PandaVersion.pp")

LoadDependencyCache()

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
    print "  --optimize X      (optimization level can be 1,2,3,4)"
    print "  --installer       (build an installer)"
    print "  --version         (set the panda version number)"
    print "  --lzma            (use lzma compression when building installer)"
    print "  --threads N       (use the multithreaded build system. see manual)"
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
    exit("")

def parseopts(args):
    global OPTIMIZE,INSTALLER,GENMAN
    global VERSION,COMPRESSOR,VERBOSE,THREADCOUNT
    longopts = [
        "help",
        "optimize=","everything","nothing","installer",
        "version=","lzma","no-python","threads="]
    anything = 0
    for pkg in PkgListGet(): longopts.append("no-"+pkg.lower())
    for pkg in PkgListGet(): longopts.append("use-"+pkg.lower())
    try:
        opts, extras = getopt.getopt(args, "", longopts)
        for option,value in opts:
            if (option=="--help"): raise "usage"
            elif (option=="--optimize"): OPTIMIZE=value
            elif (option=="--installer"): INSTALLER=1
            elif (option=="--genman"): GENMAN=1
            elif (option=="--everything"): PkgEnableAll()
            elif (option=="--nothing"): PkgDisableAll()
            elif (option=="--threads"): THREADCOUNT=int(value)
            elif (option=="--version"):
                VERSION=value
                if (len(VERSION.split(".")) != 3): raise "usage"
            elif (option=="--lzma"): COMPRESSOR="lzma"
            else:
                for pkg in PkgListGet():
                    if (option=="--use-"+pkg.lower()):
                        PkgEnable(pkg)
                        break
                for pkg in PkgListGet():
                    if (option=="--no-"+pkg.lower()):
                        PkgDisable(pkg)
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
##
## Locate various SDKs.
##
########################################################################

SdkLocateDirectX()
SdkLocateMaya()
SdkLocateMax()
SdkLocatePython()
SdkLocateVisualStudio()
SdkLocateMSPlatform()

SdkAutoDisableDirectX()
SdkAutoDisableMaya()
SdkAutoDisableMax()

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
    THIRDPARTYLIBS="thirdparty/win-libs-vc8/"
    VC80CRTVERSION = GetVC80CRTVersion(THIRDPARTYLIBS+"extras/bin/Microsoft.VC80.CRT.manifest")
else:
    CheckLinkerLibraryPath()
    COMPILER="LINUX"
    THIRDPARTYLIBS="thirdparty/linux-libs-a/"
    VC80CRTVERSION = 0

##########################################################################################
#
# Disable packages that are currently broken or not supported.
#
##########################################################################################

if (PkgSkip("MILES")==0):
    WARNINGS.append("makepanda currently does not support miles sound system")
    WARNINGS.append("I have automatically added this command-line option: --no-miles")
    PkgDisable("MILES")

if (sys.platform != "win32"):
    if (PkgSkip("DX8")==0): PkgDisable("DX8")
    if (PkgSkip("DX9")==0): PkgDisable("DX9")

if (sys.platform == "win32"):
     os.environ["BISON_SIMPLE"] = "thirdparty/win-util/bison.simple"

if (INSTALLER) and (PkgSkip("PYTHON")):
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
        for x in PkgListGet():
            if (PkgSkip(x)==0): tkeep = tkeep + x + " "
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
## CompileCxx
##
########################################################################

def CompileCxx(obj,src,opts):
    ipath = ["built/tmp"] + GetListOption(opts, "DIR:") + ["built/include"]
    if (COMPILER=="MSVC"):
        cmd = "cl /wd4996 /Fo" + obj + " /nologo /c "
        if (PkgSkip("PYTHON")==0): cmd = cmd + " /Ithirdparty/win-python/include"
        for ver in DXVERSIONS:
            if (PkgSelected(opts,"DX"+ver)):
                cmd = cmd + ' /I"' + SDK["DX"+ver] + '/include"'
        for ver in MAYAVERSIONS:
            if (PkgSelected(opts,"MAYA"+ver)):
                cmd = cmd + ' /I"' + SDK["MAYA"+ver] + '/include"'
                cmd = cmd + " /DMAYAVERSION=" + ver
        for ver in MAXVERSIONS:
            if (PkgSelected(opts,"MAX"+ver)):
                cmd = cmd + ' /I"' + SDK["MAX"+ver] + '/include" /I"' + SDK["MAX"+ver+"CS"] + '" /DMAX' + ver
        for pkg in PkgListGet():
            if (pkg[:4] != "MAYA") and (pkg[:3]!="MAX") and (pkg[:2]!="DX") and PkgSelected(opts,pkg):
                cmd = cmd + " /I" + THIRDPARTYLIBS + pkg.lower() + "/include"
        for x in ipath: cmd = cmd + " /I" + x
        if (opts.count('NOFLOATWARN')): cmd = cmd + ' /wd4244 /wd4305'
        if (opts.count("WITHINPANDA")): cmd = cmd + ' /DWITHIN_PANDA'
        if (opts.count("MSFORSCOPE")): cmd = cmd + ' /Zc:forScope-'
        optlevel = GetOptimizeOption(opts,OPTIMIZE)
        if (optlevel==1): cmd = cmd + " /MD /Zi /RTCs /GS"
        if (optlevel==2): cmd = cmd + " /MD /Zi "
        if (optlevel==3): cmd = cmd + " /MD /Zi /O2 /Ob2 /DFORCE_INLINING "
        if (optlevel==4): cmd = cmd + " /MD /Zi /Ox /Ob2 /DFORCE_INLINING /DNDEBUG /GL "
        cmd = cmd + " /Fd" + obj[:-4] + ".pdb"
        building = GetValueOption(opts, "BUILDING:")
        if (building): cmd = cmd + " /DBUILDING_" + building
        cmd = cmd + " /EHsc /Zm300 /DWIN32_VC /DWIN32 /W3 " + src
        oscmd(cmd)
    if (COMPILER=="LINUX"):
        if (src.endswith(".c")): cmd = 'gcc -fPIC -c -o ' + obj
        else:                    cmd = 'g++ -ftemplate-depth-30 -fPIC -c -o ' + obj
        if (PkgSkip("PYTHON")==0): cmd = cmd + ' -I"' + SDK["PYTHON"] + '"'
        if (PkgSelected(opts,"VRPN")):     cmd = cmd + ' -I' + THIRDPARTYLIBS + 'vrpn/include'
        if (PkgSelected(opts,"FFTW")):     cmd = cmd + ' -I' + THIRDPARTYLIBS + 'fftw/include'
        if (PkgSelected(opts,"FMOD")):     cmd = cmd + ' -I' + THIRDPARTYLIBS + 'fmod/include'
        if (PkgSelected(opts,"FMODEX")):   cmd = cmd + ' -I' + THIRDPARTYLIBS + 'fmodex/include'
        if (PkgSelected(opts,"OPENAL")):   cmd = cmd + ' -I' + THIRDPARTYLIBS + 'openal/include'
        if (PkgSelected(opts,"NVIDIACG")): cmd = cmd + ' -I' + THIRDPARTYLIBS + 'nvidiacg/include'
        if (PkgSelected(opts,"FFMPEG")):   cmd = cmd + ' -I' + THIRDPARTYLIBS + 'ffmpeg/include'
        if (PkgSelected(opts,"FREETYPE")): cmd = cmd + ' -I/usr/include/freetype2'
        for x in ipath: cmd = cmd + ' -I' + x
        if (opts.count("WITHINPANDA")): cmd = cmd + ' -DWITHIN_PANDA'
        optlevel = GetOptimizeOption(opts,OPTIMIZE)
        if (optlevel==1): cmd = cmd + " -g"
        if (optlevel==2): cmd = cmd + " -O1"
        if (optlevel==3): cmd = cmd + " -O2"
        if (optlevel==4): cmd = cmd + " -O2 -DNDEBUG"
        building = GetValueOption(opts, "BUILDING:")
        if (building): cmd = cmd + " -DBUILDING_" + building
        cmd = cmd + ' ' + src
        oscmd(cmd)

########################################################################
##
## CompileBison
##
########################################################################

def CompileBison(wobj, wsrc, opts):
    ifile = os.path.basename(wsrc)
    wdsth = "built/include/" + ifile[:-4] + ".h"
    wdstc = "built/tmp/" + ifile + ".cxx"
    pre = GetValueOption(opts, "BISONPREFIX_")
    if (COMPILER == "MSVC"):
        oscmd('thirdparty/win-util/bison -y -d -obuilt/tmp/'+ifile+'.c -p '+pre+' '+wsrc)
        CopyFile(wdstc, "built/tmp/"+ifile+".c")
        CopyFile(wdsth, "built/tmp/"+ifile+".h")
    if (COMPILER == "LINUX"):
        oscmd("bison -y -d -obuilt/tmp/"+ifile+".c -p "+pre+" "+wsrc)
        CopyFile(wdstc, "built/tmp/"+ifile+".c")
        CopyFile(wdsth, "built/tmp/"+ifile+".h")
    CompileCxx(wobj,wdstc,opts)

########################################################################
##
## CompileFlex
##
########################################################################

def CompileFlex(wobj,wsrc,opts):
    ifile = os.path.basename(wsrc)
    wdst = "built/tmp/"+ifile+".cxx"
    pre = GetValueOption(opts, "BISONPREFIX_")
    dashi = opts.count("FLEXDASHI")
    if (COMPILER == "MSVC"):
        if (dashi): oscmd("thirdparty/win-util/flex -i -P" + pre + " -o"+wdst+" "+wsrc)
        else:       oscmd("thirdparty/win-util/flex    -P" + pre + " -o"+wdst+" "+wsrc)
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
    wobj = "built/tmp/"+(os.path.basename(woutd)[:-3])+"_igate.obj"
    srcdir = GetValueOption(opts, "SRCDIR:")
    module = GetValueOption(opts, "IMOD:")
    library = GetValueOption(opts, "ILIB:")
    woutc = wobj[:-4]+".cxx"
    ipath = ["built/tmp"] + GetListOption(opts, "DIR:") + ["built/include"]
    if (PkgSkip("PYTHON")):
        WriteFile(woutc,"")
        WriteFile(woutd,"")
        CompileCxx(wobj,woutc,opts)
        ConditionalWriteFile(woutd,"")
        return
    if (COMPILER=="MSVC"):
        cmd = "built/bin/interrogate -srcdir "+srcdir+" -I"+srcdir
        cmd = cmd + ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -longlong __int64 -D_X86_ -DWIN32_VC -D_WIN32'
        cmd = cmd + ' -D"_declspec(param)=" -D_near -D_far -D__near -D__far -D__stdcall'
        optlevel=GetOptimizeOption(opts,OPTIMIZE)
        if (optlevel==1): cmd = cmd + ' '
        if (optlevel==2): cmd = cmd + ' '
        if (optlevel==3): cmd = cmd + ' -DFORCE_INLINING'
        if (optlevel==4): cmd = cmd + ' -DNDEBUG -DFORCE_INLINING'
        cmd = cmd + ' -oc ' + woutc + ' -od ' + woutd
        cmd = cmd + ' -fnames -string -refcount -assert -python-native'
        cmd = cmd + ' -Sbuilt/include/parser-inc'
        for x in ipath: cmd = cmd + ' -I' + x
        cmd = cmd + ' -Sthirdparty/win-python/include'
        for ver in DXVERSIONS:
            if ((COMPILER=="MSVC") and PkgSelected(opts,"DX"+ver)):
                cmd = cmd + ' -S"' + SDK["DX"+ver] + '/include"'
        for ver in MAYAVERSIONS:
            if ((COMPILER=="MSVC") and PkgSelected(opts,"MAYA"+ver)):
                cmd = cmd + ' -S"' + SDK["MAYA"+ver] + '/include"'
        for pkg in PkgListGet():
            if (PkgSelected(opts,pkg)):
                cmd = cmd + " -S" + THIRDPARTYLIBS + pkg.lower() + "/include"
        building = GetValueOption(opts, "BUILDING:")
        if (building): cmd = cmd + " -DBUILDING_"+building
        if (opts.count("WITHINPANDA")): cmd = cmd + " -DWITHIN_PANDA"
        cmd = cmd + ' -module ' + module + ' -library ' + library
        for x in wsrc: cmd = cmd + ' ' + os.path.basename(x)
        oscmd(cmd)
        CompileCxx(wobj,woutc,opts)
        return
    if (COMPILER=="LINUX"):
        cmd = 'built/bin/interrogate -srcdir '+srcdir
        cmd = cmd + ' -DCPPPARSER -D__STDC__=1 -D__cplusplus -D__i386__ -D__const=const'
        optlevel = GetOptimizeOption(opts,OPTIMIZE)
        if (optlevel==1): cmd = cmd + ' '
        if (optlevel==2): cmd = cmd + ' '
        if (optlevel==3): cmd = cmd + ' '
        if (optlevel==4): cmd = cmd + ' -DNDEBUG '
        cmd = cmd + ' -oc ' + woutc + ' -od ' + woutd
        cmd = cmd + ' -fnames -string -refcount -assert -python-native'
        cmd = cmd + ' -Sbuilt/include/parser-inc'
        for x in ipath: cmd = cmd + ' -I' + x
        cmd = cmd + ' -S'+SDK["PYTHON"]
        for pkg in PkgListGet():
            if (PkgSelected(opts,pkg)):
                cmd = cmd + " -S" + THIRDPARTYLIBS + pkg.lower() + "/include"
        cmd = cmd + ' -S/usr/include'
        building = GetValueOption(opts, "BUILDING:")
        if (building): cmd = cmd + " -DBUILDING_"+building
        if (opts.count("WITHINPANDA")): cmd = cmd + " -DWITHIN_PANDA"
        cmd = cmd + ' -module ' + module + ' -library ' + library
        for ver in MAYAVERSIONS:
            if (PkgSelected(opts, "MAYA"+ver)):
                cmd = cmd + ' -I"' + SDK["MAYA"+ver] + '/include"'
        for x in wsrc: cmd = cmd + ' ' + os.path.basename(x)
        oscmd(cmd)
        CompileCxx(wobj,woutc,opts)
        return

########################################################################
##
## CompileImod
##
########################################################################

def CompileImod(wobj, wsrc, opts):
    woutc = wobj[:-4]+".cxx"
    module = GetValueOption(opts, "IMOD:")
    library = GetValueOption(opts, "ILIB:")
    if (PkgSkip("PYTHON")):
        WriteFile(woutc,"")
        CompileCxx(wobj,woutc,opts)
        return
    if (COMPILER=="MSVC"):
        cmd = 'built/bin/interrogate_module '
        cmd = cmd + ' -oc ' + woutc + ' -module ' + module + ' -library ' + library + ' -python-native '
        for x in wsrc: cmd = cmd + ' ' + x
        oscmd(cmd)
        CompileCxx(wobj,woutc,opts)
        return
    if (COMPILER=="LINUX"):
        cmd = 'built/bin/interrogate_module '
        cmd = cmd + ' -oc ' + woutc + ' -module ' + module + ' -library ' + library + ' -python-native '
        for x in wsrc: cmd = cmd + ' ' + x
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
        cmd = 'link /lib /nologo /OUT:' + lib
        for x in obj: cmd = cmd + ' ' + x
        oscmd(cmd)
    if (COMPILER=="LINUX"):
        cmd = 'ar cru ' + lib
        for x in obj: cmd=cmd + ' ' + x
        oscmd(cmd)

########################################################################
##
## CompileLink
##
########################################################################

def CompileLink(dll, obj, opts):
    if (COMPILER=="MSVC"):
        cmd = 'link /nologo /NOD:MFC80.LIB /NOD:LIBCI.LIB /NOD:MSVCRTD.LIB /DEBUG '
        cmd = cmd + " /nod:libc /nod:libcmtd /nod:atlthunk"
        if (dll.endswith(".exe")==0): cmd = cmd + " /DLL"
        optlevel = GetOptimizeOption(opts,OPTIMIZE)
        if (optlevel==1): cmd = cmd + " /MAP /MAPINFO:EXPORTS"
        if (optlevel==2): cmd = cmd + " /MAP:NUL "
        if (optlevel==3): cmd = cmd + " /MAP:NUL "
        if (optlevel==4): cmd = cmd + " /MAP:NUL /LTCG "
        cmd = cmd + " /FIXED:NO /OPT:REF /STACK:4194304 /INCREMENTAL:NO "
        cmd = cmd + ' /OUT:' + dll
        if (dll.endswith(".dll")):
            cmd = cmd + ' /IMPLIB:built/lib/'+dll[10:-4]+".lib"
        if (PkgSkip("PYTHON")==0): cmd = cmd + ' /LIBPATH:thirdparty/win-python/libs '
        for x in obj:
            if (x.endswith(".dll")):
                cmd = cmd + ' built/lib/' + x[10:-4] + ".lib"
            elif (x.endswith(".lib")):
                dname = x[:-4]+".dll"
                if (os.path.exists("built/bin/" + x[10:-4] + ".dll")):
                    exit("Error: in makepanda, specify "+dname+", not "+x)
                cmd = cmd + ' ' + x
            elif (x.endswith(".def")):
                cmd = cmd + ' /DEF:"' + x + '"'
            elif (x.endswith(".dat")):
                pass
            else: cmd = cmd + ' ' + x
        if (dll[-4:]==".exe"): cmd = cmd + ' panda/src/configfiles/pandaIcon.obj'
        for ver in DXVERSIONS:
            if (PkgSelected(opts,"DX"+ver)):
                cmd = cmd + ' /LIBPATH:"' + SDK["DX"+ver] + '/lib/x86"'
                cmd = cmd + ' /LIBPATH:"' + SDK["DX"+ver] + '/lib"'
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
        if (opts.count("WINSHELL")):    cmd = cmd + " shell32.lib"
        if (opts.count("WINGDI")):      cmd = cmd + " gdi32.lib"
        if (opts.count("ADVAPI")):      cmd = cmd + " advapi32.lib"
        if (opts.count("GLUT")):        cmd = cmd + " opengl32.lib glu32.lib"
        if (PkgSelected(opts,"DIRECTCAM")): cmd = cmd + " strmiids.lib quartz.lib odbc32.lib odbccp32.lib"
        if (PkgSelected(opts,"PNG")):       cmd = cmd + ' ' + THIRDPARTYLIBS + 'png/lib/libpandapng.lib'
        if (PkgSelected(opts,"JPEG")):      cmd = cmd + ' ' + THIRDPARTYLIBS + 'jpeg/lib/libpandajpeg.lib'
        if (PkgSelected(opts,"TIFF")):      cmd = cmd + ' ' + THIRDPARTYLIBS + 'tiff/lib/libpandatiff.lib'
        if (PkgSelected(opts,"ZLIB")):      cmd = cmd + ' ' + THIRDPARTYLIBS + 'zlib/lib/libpandazlib1.lib'
        if (PkgSelected(opts,"VRPN")):
            cmd = cmd + ' ' + THIRDPARTYLIBS + 'vrpn/lib/vrpn.lib'
            cmd = cmd + ' ' + THIRDPARTYLIBS + 'vrpn/lib/quat.lib'
        if (PkgSelected(opts,"FMOD")):
            cmd = cmd + ' ' + THIRDPARTYLIBS + 'fmod/lib/fmod.lib'
        if (PkgSelected(opts,"FMODEX")):
            cmd = cmd + ' ' + THIRDPARTYLIBS + 'fmodex/lib/fmodex_vc.lib'
        if (PkgSelected(opts,"OPENAL")):
            cmd = cmd + ' ' + THIRDPARTYLIBS + 'openal/lib/pandaopenal32.lib'
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
        if (PkgSelected(opts,"ARTOOLKIT")):
            cmd = cmd + ' ' + THIRDPARTYLIBS + 'artoolkit/lib/libAR.lib'
        for ver in MAYAVERSIONS:
            if (PkgSelected(opts,"MAYA"+ver)):
                cmd = cmd + ' "' + SDK["MAYA"+ver] +  '/lib/Foundation.lib"'
                cmd = cmd + ' "' + SDK["MAYA"+ver] +  '/lib/OpenMaya.lib"'
                cmd = cmd + ' "' + SDK["MAYA"+ver] +  '/lib/OpenMayaAnim.lib"'
                cmd = cmd + ' "' + SDK["MAYA"+ver] +  '/lib/OpenMayaUI.lib"'
        for ver in MAXVERSIONS:
            if (PkgSelected(opts,"MAX"+ver)):
                cmd = cmd + ' "' + SDK["MAX"+ver] +  '/lib/core.lib"'
                cmd = cmd + ' "' + SDK["MAX"+ver] +  '/lib/edmodel.lib"'
                cmd = cmd + ' "' + SDK["MAX"+ver] +  '/lib/gfx.lib"'
                cmd = cmd + ' "' + SDK["MAX"+ver] +  '/lib/geom.lib"'
                cmd = cmd + ' "' + SDK["MAX"+ver] +  '/lib/mesh.lib"'
                cmd = cmd + ' "' + SDK["MAX"+ver] +  '/lib/maxutil.lib"'
                cmd = cmd + ' "' + SDK["MAX"+ver] +  '/lib/paramblk2.lib"'
        oscmd(cmd)
        SetVC80CRTVersion(dll+".manifest", VC80CRTVERSION)
        mtcmd = 'mt -manifest ' + dll + '.manifest -outputresource:' + dll
        if (dll.endswith(".exe")==0): mtcmd = mtcmd + ';2'
        else:                          mtcmd = mtcmd + ';1'
        oscmd(mtcmd)
    if (COMPILER=="LINUX"):
        if (dll.endswith(".exe")): cmd = 'g++ -o ' + wdll + ' -Lbuilt/lib -L/usr/X11R6/lib'
        else:                      cmd = 'g++ -shared -o ' + wdll + ' -Lbuilt/lib -L/usr/X11R6/lib'
        for x in obj:
            suffix = x[-4:]
            if   (suffix==".obj"): cmd = cmd + ' built/tmp/' + x[:-4] + '.o'
            elif (suffix==".dll"): cmd = cmd + ' -l' + x[3:-4]
            elif (suffix==".lib"): cmd = cmd + ' built/lib/' + x[:-4] + '.a'
            elif (suffix==".ilb"): cmd = cmd + ' built/tmp/' + x[:-4] + '.a'
            elif (suffix==".dat"): pass
        #if (PkgSelected(opts,"FMOD")):     cmd = cmd + ' -L' + THIRDPARTYLIBS + 'fmod/lib -lfmod'
        if (PkgSelected(opts,"FMODEX")):   cmd = cmd + ' -L' + THIRDPARTYLIBS + 'fmodex/lib -lfmodex'
        #if (PkgSelected(opts,"OPENAL")):   cmd = cmd + ' -L' + THIRDPARTYLIBS + 'openal/lib -lopenal'
        if (PkgSelected(opts,"NVIDIACG")):
            cmd = cmd + ' -Lthirdparty/nvidiacg/lib '
            if (opts.count("CGGL")):  cmd = cmd + " -lCgGL"
            cmd = cmd + " -lCg "
        if (PkgSelected(opts,"FFMPEG")):   cmd = cmd + ' -L' + THIRDPARTYLIBS + 'ffmpeg/lib -lavformat -lavcodec -lavformat -lavutil'
        if (PkgSelected(opts,"OPENAL")):   cmd = cmd + ' -L' + THIRDPARTYLIBS + 'openal/lib -lpandaopenal'
        if (PkgSelected(opts,"ZLIB")):     cmd = cmd + " -lz"
        if (PkgSelected(opts,"PNG")):      cmd = cmd + " -lpng"
        if (PkgSelected(opts,"JPEG")):     cmd = cmd + " -ljpeg"
        if (PkgSelected(opts,"TIFF")):     cmd = cmd + " -ltiff"
        if (PkgSelected(opts,"OPENSSL")):  cmd = cmd + " -lssl"
        if (PkgSelected(opts,"FREETYPE")): cmd = cmd + " -lfreetype"
        if (PkgSelected(opts,"VRPN")):     cmd = cmd + ' -L' + THIRDPARTYLIBS + 'vrpn/lib -lvrpn -lquat'
        if (PkgSelected(opts,"FFTW")):     cmd = cmd + ' -L' + THIRDPARTYLIBS + 'fftw/lib -lrfftw -lfftw'
        if (opts.count("GLUT")):           cmd = cmd + " -lGL -lGLU"
        cmd = cmd + " -lpthread -ldl"
        oscmd(cmd)

##########################################################################################
#
# CompileEggPZ
#
##########################################################################################

def CompileEggPZ(eggpz, src, opts):
    if (src.endswith(".egg")):
        CopyFile(eggpz[:-3], src)
    elif (src.endswith(".flt")):
        oscmd("built/bin/flt2egg -ps keep -o " + eggpz[:-3] + " " + src)
    oscmd("built/bin/pzip " + eggpz[:-3])

##########################################################################################
#
# CompileAnything
#
##########################################################################################

def CompileAnything(target, origsuffix, inputs, opts):
    if (opts.count("DEPENDENCYONLY")):
        return
    if (len(inputs)==0):
        exit("No input files for target "+target)
    infile = inputs[0]
    if SUFFIX_LIB.count(origsuffix):
        return CompileLib(target, inputs, opts)
    elif SUFFIX_DLL.count(origsuffix):
        return CompileLink(target, inputs, opts)
    elif (origsuffix==".in"):
        return CompileIgate(target, inputs, opts)
    elif (origsuffix==".pz"):
        return CompileEggPZ(target, infile, opts)
    elif (origsuffix==".obj"):
        if (infile.endswith(".cxx") or infile.endswith(".c")):
            return CompileCxx(target, infile, opts)
        elif (infile.endswith(".yxx")):
            return CompileBison(target, infile, opts)
        elif (infile.endswith(".lxx")):
            return CompileFlex(target, infile, opts)
        elif (infile.endswith(".in")):
            return CompileImod(target, inputs, opts)
    exit("Don't know how to compile: "+target)

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
MakeDirectory("built/modelcache")
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
MakeDirectory("built/direct")
MakeDirectory("built/pandac")
MakeDirectory("built/pandac/input")

##########################################################################################
#
# Generate dtool_config.h, prc_parameters.h, and dtool_have_xxx.dat
#
##########################################################################################

DTOOL_CONFIG=[
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
    ("HAVE_GETOPT_H",                  'UNDEF',                  '1'),
    ("HAVE_LINUX_INPUT_H",             'UNDEF',                  '1'),
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
    ("HAVE_FMOD",                      'UNDEF',                  'UNDEF'),
    ("HAVE_FMODEX",                    'UNDEF',                  'UNDEF'),
    ("HAVE_OPENAL",                    'UNDEF',                  'UNDEF'),
    ("HAVE_NVIDIACG",                  'UNDEF',                  'UNDEF'),
    ("HAVE_FREETYPE",                  'UNDEF',                  'UNDEF'),
    ("HAVE_FFTW",                      'UNDEF',                  'UNDEF'),
    ("HAVE_OPENSSL",                   'UNDEF',                  'UNDEF'),
    ("HAVE_NET",                       'UNDEF',                  'UNDEF'),
    ("HAVE_CG",                        'UNDEF',                  'UNDEF'),
    ("HAVE_CGGL",                      'UNDEF',                  'UNDEF'),
    ("HAVE_CGDX9",                     'UNDEF',                  'UNDEF'),
    ("HAVE_FFMPEG",                    'UNDEF',                  'UNDEF'),
    ("HAVE_ARTOOLKIT",                 'UNDEF',                  'UNDEF'),
    ("HAVE_DIRECTCAM",                 'UNDEF',                  'UNDEF'),
    ("PRC_SAVE_DESCRIPTIONS",          '1',                      '1'),
]

PRC_PARAMETERS=[
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
]

def WriteConfigSettings():
    dtool_config={}
    prc_parameters={}

    if (sys.platform == "win32"):
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
        if (PkgSkip(x)==0):
            if (dtool_config.has_key("HAVE_"+x)):
                dtool_config["HAVE_"+x] = '1'
    
    dtool_config["HAVE_NET"] = '1'
    
    if (PkgSkip("NVIDIACG")==0):
        dtool_config["HAVE_CG"] = '1'
        dtool_config["HAVE_CGGL"] = '1'
        dtool_config["HAVE_CGDX9"] = '1'
    
    if (OPTIMIZE <= 3):
        if (dtool_config["HAVE_NET"] != 'UNDEF'):
            dtool_config["DO_PSTATS"] = '1'
    
    if (OPTIMIZE <= 3):
        dtool_config["DO_COLLISION_RECORDING"] = '1'
    
    #if (OPTIMIZE <= 2):
    #    dtool_config["TRACK_IN_INTERPRETER"] = '1'
    
    if (OPTIMIZE <= 3):
        dtool_config["DO_MEMORY_USAGE"] = '1'
    
    #if (OPTIMIZE <= 1):
    #    dtool_config["DO_PIPELINING"] = '1'
    
    if (OPTIMIZE <= 3):
        dtool_config["NOTIFY_DEBUG"] = '1'

    conf = "/* prc_parameters.h.  Generated automatically by makepanda.py */\n"
    for key in prc_parameters.keys():
        if ((key == "DEFAULT_PRC_DIR") or (key[:4]=="PRC_")):
            val = prc_parameters[key]
            if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
            else:                conf = conf + "#define " + key + " " + val + "\n"
            del prc_parameters[key]
    ConditionalWriteFile('built/include/prc_parameters.h', conf)

    conf = "/* dtool_config.h.  Generated automatically by makepanda.py */\n"
    for key in dtool_config.keys():
        val = dtool_config[key]
        if (val == 'UNDEF'): conf = conf + "#undef " + key + "\n"
        else:                conf = conf + "#define " + key + " " + val + "\n"
        del dtool_config[key]
    ConditionalWriteFile('built/include/dtool_config.h', conf)

    for x in PkgListGet():
        if (PkgSkip(x)): ConditionalWriteFile('built/tmp/dtool_have_'+x.lower()+'.dat',"0\n")
        else:            ConditionalWriteFile('built/tmp/dtool_have_'+x.lower()+'.dat',"1\n")

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
    if (PkgSkip("PYTHON")==0):
        ConditionalWriteFile("built/tmp/pythonversion", os.path.basename(SDK["PYTHON"]))
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

if (PkgSkip("PYTHON")==0):
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

ConditionalWriteFile("built/etc/Config.prc", configprc)
ConditionalWriteFile("built/etc/Confauto.prc", confautoprc)

##########################################################################################
#
# Copy the precompiled binaries and DLLs into the build.
#
##########################################################################################

for pkg in PkgListGet():
    if (PkgSkip(pkg)==0):
        if (COMPILER == "MSVC"):
            if (os.path.exists(THIRDPARTYLIBS+pkg.lower()+"/bin")):
                CopyAllFiles("built/bin/",THIRDPARTYLIBS+pkg.lower()+"/bin/")
        if (COMPILER == "LINUX"):
            if (os.path.exists(THIRDPARTYLIBS+pkg.lower()+"/lib")):
                CopyAllFiles("built/lib/",THIRDPARTYLIBS+pkg.lower()+"/lib/")
if (COMPILER=="MSVC"):
    CopyAllFiles("built/bin/", THIRDPARTYLIBS+"extras"+"/bin/")
if (sys.platform == "win32"):
    if (PkgSkip("PYTHON")==0):
        CopyFile('built/bin/python24.dll', 'thirdparty/win-python/python24.dll')
        CopyTree('built/python', 'thirdparty/win-python')
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
if (PkgSkip("PYTHON")==0):
    CopyTree('built/Pmw',         'thirdparty/Pmw')
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
CopyAllHeaders('dtool/src/cppparser')
CopyAllHeaders('dtool/src/prc')
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
CopyFile('built/include/','dtool/src/dtoolutil/vector_src.cxx')

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
    'libp3pystub.dll',
] + COMMON_PANDA_LIBS

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

#
# DIRECTORY: dtool/metalibs/dtool/
#

OPTS=['DIR:dtool/metalibs/dtool', 'BUILDING:DTOOL']
TargetAdd('dtool_dtool.obj', opts=OPTS, input='dtool.cxx')
TargetAdd('libp3dtool.dll', input='dtool_dtool.obj')
TargetAdd('libp3dtool.dll', input='dtoolutil_gnu_getopt.obj')
TargetAdd('libp3dtool.dll', input='dtoolutil_gnu_getopt1.obj')
TargetAdd('libp3dtool.dll', input='dtoolutil_composite.obj')
TargetAdd('libp3dtool.dll', input='dtoolbase_composite1.obj')
TargetAdd('libp3dtool.dll', input='dtoolbase_composite2.obj')
TargetAdd('libp3dtool.dll', input='dtoolbase_indent.obj')
TargetAdd('libp3dtool.dll', input='dtoolbase_lookup3.obj')
TargetAdd('libp3dtool.dll', opts=['ADVAPI','WINSHELL'])

#
# DIRECTORY: dtool/src/cppparser/
#

OPTS=['DIR:dtool/src/cppparser', 'BISONPREFIX_cppyy']
CreateFile("built/include/cppBison.h")
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

OPTS=['DIR:dtool/src/interrogate', 'DIR:dtool/src/cppparser', 'DIR:dtool/src/interrogatedb']
TargetAdd('interrogate_composite.obj', opts=OPTS, input='interrogate_composite.cxx')
TargetAdd('interrogate.exe', input='interrogate_composite.obj')
TargetAdd('interrogate.exe', input='libcppParser.ilb')
TargetAdd('interrogate.exe', input=COMMON_DTOOL_LIBS)
TargetAdd('interrogate.exe', opts=['ADVAPI',  'OPENSSL'])

TargetAdd('interrogate_module_interrogate_module.obj', opts=OPTS, input='interrogate_module.cxx')
TargetAdd('interrogate_module.exe', input='interrogate_module_interrogate_module.obj')
TargetAdd('interrogate_module.exe', input='libcppParser.ilb')
TargetAdd('interrogate_module.exe', input=COMMON_DTOOL_LIBS)
TargetAdd('interrogate_module.exe', opts=['ADVAPI',  'OPENSSL'])

TargetAdd('parse_file_parse_file.obj', opts=OPTS, input='parse_file.cxx')
TargetAdd('parse_file.exe', input='parse_file_parse_file.obj')
TargetAdd('parse_file.exe', input='libcppParser.ilb')
TargetAdd('parse_file.exe', input=COMMON_DTOOL_LIBS)
TargetAdd('parse_file.exe', opts=['ADVAPI',  'OPENSSL'])

#
# DIRECTORY: dtool/src/prckeys/
#

if (PkgSkip("OPENSSL")==0):
    OPTS=['DIR:dtool/src/prckeys', 'OPENSSL']
    TargetAdd('make-prc-key_makePrcKey.obj', opts=OPTS, input='makePrcKey.cxx')
    TargetAdd('make-prc-key.exe', input='make-prc-key_makePrcKey.obj')
    TargetAdd('make-prc-key.exe', input=COMMON_DTOOL_LIBS)
    TargetAdd('make-prc-key.exe', opts=['ADVAPI',  'OPENSSL'])

#
# DIRECTORY: dtool/src/test_interrogate/
#

OPTS=['DIR:dtool/src/test_interrogate']
TargetAdd('test_interrogate_test_interrogate.obj', opts=OPTS, input='test_interrogate.cxx')
TargetAdd('test_interrogate.exe', input='test_interrogate_test_interrogate.obj')
TargetAdd('test_interrogate.exe', input=COMMON_DTOOL_LIBS)
TargetAdd('test_interrogate.exe', opts=['ADVAPI',  'OPENSSL'])

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

OPTS=['DIR:panda/src/pipeline', 'BUILDING:PANDA']
TargetAdd('pipeline_composite.obj', opts=OPTS, input='pipeline_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/pipeline', ["*.h", "*_composite.cxx"])
TargetAdd('libpipeline.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libpipeline.in', opts=['IMOD:panda', 'ILIB:libpipeline', 'SRCDIR:panda/src/pipeline'])
TargetAdd('libpipeline_igate.obj', input='libpipeline.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/putil/
#

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

OPTS=['DIR:panda/src/audio', 'BUILDING:PANDA']
TargetAdd('audio_composite.obj', opts=OPTS, input='audio_composite.cxx')
IGATEFILES=["audio.h"]
TargetAdd('libaudio.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libaudio.in', opts=['IMOD:panda', 'ILIB:libaudio', 'SRCDIR:panda/src/audio'])
TargetAdd('libaudio_igate.obj', input='libaudio.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/event/
#

OPTS=['DIR:panda/src/event', 'BUILDING:PANDA']
TargetAdd('event_composite.obj', opts=OPTS, input='event_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/event', ["*.h", "*_composite.cxx"])
TargetAdd('libevent.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libevent.in', opts=['IMOD:panda', 'ILIB:libevent', 'SRCDIR:panda/src/event'])
TargetAdd('libevent_igate.obj', input='libevent.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/linmath/
#

OPTS=['DIR:panda/src/linmath', 'BUILDING:PANDA']
TargetAdd('linmath_composite.obj', opts=OPTS, input='linmath_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/linmath', ["*.h", "*_composite.cxx"])
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

OPTS=['DIR:panda/src/mathutil', 'BUILDING:PANDA', 'FFTW']
TargetAdd('mathutil_composite.obj', opts=OPTS, input='mathutil_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/mathutil', ["*.h", "*_composite.cxx"])
TargetAdd('libmathutil.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libmathutil.in', opts=['IMOD:panda', 'ILIB:libmathutil', 'SRCDIR:panda/src/mathutil'])
TargetAdd('libmathutil_igate.obj', input='libmathutil.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/gsgbase/
#

OPTS=['DIR:panda/src/gsgbase', 'BUILDING:PANDA']
TargetAdd('gsgbase_composite.obj', opts=OPTS, input='gsgbase_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/gsgbase', ["*.h", "*_composite.cxx"])
TargetAdd('libgsgbase.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libgsgbase.in', opts=['IMOD:panda', 'ILIB:libgsgbase', 'SRCDIR:panda/src/gsgbase'])
TargetAdd('libgsgbase_igate.obj', input='libgsgbase.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pnmimage/
#

OPTS=['DIR:panda/src/pnmimage', 'BUILDING:PANDA',  'ZLIB']
TargetAdd('pnmimage_composite.obj', opts=OPTS, input='pnmimage_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/pnmimage', ["*.h", "*_composite.cxx"])
TargetAdd('libpnmimage.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libpnmimage.in', opts=['IMOD:panda', 'ILIB:libpnmimage', 'SRCDIR:panda/src/pnmimage'])
TargetAdd('libpnmimage_igate.obj', input='libpnmimage.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/nativenet/
#
 
OPTS=['DIR:panda/src/nativenet', 'OPENSSL', 'BUILDING:PANDA']
TargetAdd('nativenet_composite.obj', opts=OPTS, input='nativenet_composite1.cxx')
IGATEFILES=GetDirectoryContents('panda/src/nativenet', ["*.h", "*_composite.cxx"])
TargetAdd('libnativenet.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libnativenet.in', opts=['IMOD:panda', 'ILIB:libnativenet', 'SRCDIR:panda/src/nativenet'])
TargetAdd('libnativenet_igate.obj', input='libnativenet.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/net/
#

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

OPTS=['DIR:panda/src/pstatclient', 'BUILDING:PANDA']
TargetAdd('pstatclient_composite.obj', opts=OPTS, input='pstatclient_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/pstatclient', ["*.h", "*_composite.cxx"])
TargetAdd('libpstatclient.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libpstatclient.in', opts=['IMOD:panda', 'ILIB:libpstatclient', 'SRCDIR:panda/src/pstatclient'])
TargetAdd('libpstatclient_igate.obj', input='libpstatclient.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/gobj/
#

OPTS=['DIR:panda/src/gobj', 'BUILDING:PANDA',  'NVIDIACG', 'ZLIB']
TargetAdd('gobj_composite1.obj', opts=OPTS, input='gobj_composite1.cxx')
TargetAdd('gobj_composite2.obj', opts=OPTS, input='gobj_composite2.cxx')
IGATEFILES=GetDirectoryContents('panda/src/gobj', ["*.h", "*_composite.cxx"])
TargetAdd('libgobj.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libgobj.in', opts=['IMOD:panda', 'ILIB:libgobj', 'SRCDIR:panda/src/gobj'])
TargetAdd('libgobj_igate.obj', input='libgobj.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/lerp/
#

OPTS=['DIR:panda/src/lerp', 'BUILDING:PANDA']
TargetAdd('lerp_composite.obj', opts=OPTS, input='lerp_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/lerp', ["*.h", "*_composite.cxx"])
IGATEFILES.remove("lerpchans.h")
TargetAdd('liblerp.in', opts=OPTS, input=IGATEFILES)
TargetAdd('liblerp.in', opts=['IMOD:panda', 'ILIB:liblerp', 'SRCDIR:panda/src/lerp'])
TargetAdd('liblerp_igate.obj', input='liblerp.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pgraph/
#

OPTS=['DIR:panda/src/pgraph', 'BUILDING:PANDA']
TargetAdd('pgraph_nodePath.obj', opts=OPTS, input='nodePath.cxx')
TargetAdd('pgraph_composite1.obj', opts=OPTS, input='pgraph_composite1.cxx')
TargetAdd('pgraph_composite2.obj', opts=OPTS, input='pgraph_composite2.cxx')
TargetAdd('pgraph_composite3.obj', opts=OPTS, input='pgraph_composite3.cxx')
TargetAdd('pgraph_composite4.obj', opts=OPTS, input='pgraph_composite4.cxx')
IGATEFILES=GetDirectoryContents('panda/src/pgraph', ["*.h", "*_composite.cxx"])
# IGATEFILES.remove("antialiasAttrib.h")
TargetAdd('libpgraph.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libpgraph.in', opts=['IMOD:panda', 'ILIB:libpgraph', 'SRCDIR:panda/src/pgraph'])
TargetAdd('libpgraph_igate.obj', input='libpgraph.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/cull/
#

OPTS=['DIR:panda/src/cull', 'BUILDING:PANDA']
TargetAdd('cull_composite.obj', opts=OPTS, input='cull_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/cull', ["*.h", "*_composite.cxx"])
TargetAdd('libcull.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libcull.in', opts=['IMOD:panda', 'ILIB:libcull', 'SRCDIR:panda/src/cull'])
TargetAdd('libcull_igate.obj', input='libcull.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/chan/
#

OPTS=['DIR:panda/src/chan', 'BUILDING:PANDA']
TargetAdd('chan_composite.obj', opts=OPTS, input='chan_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/chan', ["*.h", "*_composite.cxx"])
IGATEFILES.remove('movingPart.h')
IGATEFILES.remove('animChannelFixed.h')
TargetAdd('libchan.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libchan.in', opts=['IMOD:panda', 'ILIB:libchan', 'SRCDIR:panda/src/chan'])
TargetAdd('libchan_igate.obj', input='libchan.in', opts=["DEPENDENCYONLY"])


# DIRECTORY: panda/src/char/
#

OPTS=['DIR:panda/src/char', 'BUILDING:PANDA']
TargetAdd('char_composite.obj', opts=OPTS, input='char_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/char', ["*.h", "*_composite.cxx"])
TargetAdd('libchar.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libchar.in', opts=['IMOD:panda', 'ILIB:libchar', 'SRCDIR:panda/src/char'])
TargetAdd('libchar_igate.obj', input='libchar.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/dgraph/
#

OPTS=['DIR:panda/src/dgraph', 'BUILDING:PANDA']
TargetAdd('dgraph_composite.obj', opts=OPTS, input='dgraph_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/dgraph', ["*.h", "*_composite.cxx"])
TargetAdd('libdgraph.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libdgraph.in', opts=['IMOD:panda', 'ILIB:libdgraph', 'SRCDIR:panda/src/dgraph'])
TargetAdd('libdgraph_igate.obj', input='libdgraph.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/display/
#

OPTS=['DIR:panda/src/display', 'BUILDING:PANDA']
TargetAdd('display_composite.obj', opts=OPTS, input='display_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/display', ["*.h", "*_composite.cxx"])
IGATEFILES.remove("renderBuffer.h")
TargetAdd('libdisplay.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libdisplay.in', opts=['IMOD:panda', 'ILIB:libdisplay', 'SRCDIR:panda/src/display'])
TargetAdd('libdisplay_igate.obj', input='libdisplay.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/device/
#

OPTS=['DIR:panda/src/device', 'BUILDING:PANDA']
TargetAdd('device_composite.obj', opts=OPTS, input='device_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/device', ["*.h", "*_composite.cxx"])
TargetAdd('libdevice.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libdevice.in', opts=['IMOD:panda', 'ILIB:libdevice', 'SRCDIR:panda/src/device'])
TargetAdd('libdevice_igate.obj', input='libdevice.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pnmtext/
#

if (PkgSkip("FREETYPE")==0):
    OPTS=['DIR:panda/src/pnmtext', 'BUILDING:PANDA',  'FREETYPE']
    TargetAdd('pnmtext_composite.obj', opts=OPTS, input='pnmtext_composite.cxx')
    IGATEFILES=GetDirectoryContents('panda/src/pnmtext', ["*.h", "*_composite.cxx"])
    TargetAdd('libpnmtext.in', opts=OPTS, input=IGATEFILES)
    TargetAdd('libpnmtext.in', opts=['IMOD:panda', 'ILIB:libpnmtext', 'SRCDIR:panda/src/pnmtext'])
    TargetAdd('libpnmtext_igate.obj', input='libpnmtext.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/text/
#

OPTS=['DIR:panda/src/text', 'BUILDING:PANDA', 'ZLIB',  'FREETYPE']
TargetAdd('text_composite.obj', opts=OPTS, input='text_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/text', ["*.h", "*_composite.cxx"])
TargetAdd('libtext.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libtext.in', opts=['IMOD:panda', 'ILIB:libtext', 'SRCDIR:panda/src/text'])
TargetAdd('libtext_igate.obj', input='libtext.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/movies/
#

OPTS=['DIR:panda/src/movies', 'BUILDING:PANDA', 'FFMPEG', 'DX9', 'DIRECTCAM']
TargetAdd('movies_composite1.obj', opts=OPTS, input='movies_composite1.cxx')
IGATEFILES=GetDirectoryContents('panda/src/movies', ["*.h", "*_composite.cxx"])
TargetAdd('libmovies.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libmovies.in', opts=['IMOD:panda', 'ILIB:libmovies', 'SRCDIR:panda/src/movies'])
TargetAdd('libmovies_igate.obj', input='libmovies.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/grutil/
#

OPTS=['DIR:panda/src/grutil', 'BUILDING:PANDA',  'FFMPEG', 'ARTOOLKIT']
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

OPTS=['DIR:panda/src/tform', 'BUILDING:PANDA']
TargetAdd('tform_composite.obj', opts=OPTS, input='tform_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/tform', ["*.h", "*_composite.cxx"])
TargetAdd('libtform.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libtform.in', opts=['IMOD:panda', 'ILIB:libtform', 'SRCDIR:panda/src/tform'])
TargetAdd('libtform_igate.obj', input='libtform.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/collide/
#

OPTS=['DIR:panda/src/collide', 'BUILDING:PANDA']
TargetAdd('collide_composite.obj', opts=OPTS, input='collide_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/collide', ["*.h", "*_composite.cxx"])
TargetAdd('libcollide.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libcollide.in', opts=['IMOD:panda', 'ILIB:libcollide', 'SRCDIR:panda/src/collide'])
TargetAdd('libcollide_igate.obj', input='libcollide.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/parametrics/
#

OPTS=['DIR:panda/src/parametrics', 'BUILDING:PANDA']
TargetAdd('parametrics_composite.obj', opts=OPTS, input='parametrics_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/parametrics', ["*.h", "*_composite.cxx"])
TargetAdd('libparametrics.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libparametrics.in', opts=['IMOD:panda', 'ILIB:libparametrics', 'SRCDIR:panda/src/parametrics'])
TargetAdd('libparametrics_igate.obj', input='libparametrics.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pgui/
#

OPTS=['DIR:panda/src/pgui', 'BUILDING:PANDA']
TargetAdd('pgui_composite.obj', opts=OPTS, input='pgui_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/pgui', ["*.h", "*_composite.cxx"])
TargetAdd('libpgui.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libpgui.in', opts=['IMOD:panda', 'ILIB:libpgui', 'SRCDIR:panda/src/pgui'])
TargetAdd('libpgui_igate.obj', input='libpgui.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/pnmimagetypes/
#

OPTS=['DIR:panda/src/pnmimagetypes', 'DIR:panda/src/pnmimage', 'BUILDING:PANDA', 'PNG', 'ZLIB', 'JPEG', 'ZLIB',  'JPEG', 'TIFF']
TargetAdd('pnmimagetypes_composite.obj', opts=OPTS, input='pnmimagetypes_composite.cxx')

#
# DIRECTORY: panda/src/recorder/
#

OPTS=['DIR:panda/src/recorder', 'BUILDING:PANDA']
TargetAdd('recorder_composite.obj', opts=OPTS, input='recorder_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/recorder', ["*.h", "*_composite.cxx"])
TargetAdd('librecorder.in', opts=OPTS, input=IGATEFILES)
TargetAdd('librecorder.in', opts=['IMOD:panda', 'ILIB:librecorder', 'SRCDIR:panda/src/recorder'])
TargetAdd('librecorder_igate.obj', input='librecorder.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/vrpn/
#

if (PkgSkip("VRPN")==0):
    OPTS=['DIR:panda/src/vrpn', 'BUILDING:PANDA',  'VRPN']
    TargetAdd('vrpn_composite.obj', opts=OPTS, input='vrpn_composite.cxx')
    IGATEFILES=GetDirectoryContents('panda/src/vrpn', ["*.h", "*_composite.cxx"])
    TargetAdd('libvrpn.in', opts=OPTS, input=IGATEFILES)
    TargetAdd('libvrpn.in', opts=['IMOD:panda', 'ILIB:libvrpn', 'SRCDIR:panda/src/vrpn'])
    TargetAdd('libvrpn_igate.obj', input='libvrpn.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/panda/
#

OPTS=['DIR:panda/metalibs/panda', 'BUILDING:PANDA', 'VRPN', 'JPEG', 'PNG', 'TIFF', 'ZLIB',
      'NVIDIACG', 'OPENSSL', 'FREETYPE', 'FFTW', 'ADVAPI', 'WINSOCK2', 'WINUSER', 'WINMM',
      'FFMPEG', 'DIRECTCAM', 'ARTOOLKIT']

TargetAdd('panda_panda.obj', opts=OPTS, input='panda.cxx')

TargetAdd('libpanda_module.obj', input='librecorder.in')
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

TargetAdd('libpanda.dll', input='panda_panda.obj')
TargetAdd('libpanda.dll', input='libpanda_module.obj')
TargetAdd('libpanda.dll', input='recorder_composite.obj')
TargetAdd('libpanda.dll', input='librecorder_igate.obj')
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
# DIRECTORY: panda/src/skel
#

OPTS=['DIR:panda/src/skel', 'BUILDING:PANDASKEL', 'ADVAPI']
TargetAdd('skel_composite.obj', opts=OPTS, input='skel_composite.cxx')
IGATEFILES=GetDirectoryContents("panda/src/skel", ["*.h", "*_composite.cxx"])
TargetAdd('libskel.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libskel.in', opts=['IMOD:pandaskel', 'ILIB:libskel', 'SRCDIR:panda/src/skel'])
TargetAdd('libskel_igate.obj', input='libskel.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/skel
#

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

OPTS=['DIR:panda/src/distort', 'BUILDING:PANDAFX']
TargetAdd('distort_composite.obj', opts=OPTS, input='distort_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/distort', ["*.h", "*_composite.cxx"])
TargetAdd('libdistort.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libdistort.in', opts=['IMOD:pandafx', 'ILIB:libdistort', 'SRCDIR:panda/src/distort'])
TargetAdd('libdistort_igate.obj', input='libdistort.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/effects/
#

OPTS=['DIR:panda/src/effects', 'BUILDING:PANDAFX',  'NVIDIACG']
TargetAdd('effects_composite.obj', opts=OPTS, input='effects_composite.cxx')
IGATEFILES=GetDirectoryContents('panda/src/effects', ["*.h", "*_composite.cxx"])
TargetAdd('libeffects.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libeffects.in', opts=['IMOD:pandafx', 'ILIB:libeffects', 'SRCDIR:panda/src/effects'])
TargetAdd('libeffects_igate.obj', input='libeffects.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/metalibs/pandafx/
#

OPTS=['DIR:panda/metalibs/pandafx', 'DIR:panda/src/distort', 'BUILDING:PANDAFX',  'NVIDIACG']
TargetAdd('pandafx_pandafx.obj', opts=OPTS, input='pandafx.cxx')

TargetAdd('libpandafx_module.obj', input='libdistort.in')
TargetAdd('libpandafx_module.obj', input='libeffects.in')
TargetAdd('libpandafx_module.obj', opts=OPTS)
TargetAdd('libpandafx_module.obj', opts=['IMOD:pandfx', 'ILIB:libpandafx'])

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

if PkgSkip("FMODEX") == 0:
  OPTS=['DIR:panda/src/audiotraits', 'BUILDING:FMOD_AUDIO',  'FMODEX']
  TargetAdd('fmod_audio_fmod_audio_composite.obj', opts=OPTS, input='fmod_audio_composite.cxx')
  TargetAdd('libp3fmod_audio.dll', input='fmod_audio_fmod_audio_composite.obj')
  TargetAdd('libp3fmod_audio.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3fmod_audio.dll', opts=['ADVAPI', 'WINUSER', 'WINMM', 'FMODEX'])

if PkgSkip("OPENAL") == 0:
  OPTS=['DIR:panda/src/audiotraits', 'BUILDING:OPENAL_AUDIO',  'OPENAL']
  TargetAdd('openal_audio_openal_audio_composite.obj', opts=OPTS, input='openal_audio_composite.cxx')
  TargetAdd('libp3openal_audio.dll', input='openal_audio_openal_audio_composite.obj')
  TargetAdd('libp3openal_audio.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3openal_audio.dll', opts=['ADVAPI', 'WINUSER', 'WINMM', 'OPENAL'])

if PkgSkip("MILES") == 0:
  OPTS=['DIR:panda/src/audiotraits', 'BUILDING:MILES_AUDIO',  'MILES']
  TargetAdd('miles_audio_miles_audio_composite.obj', opts=OPTS, input='miles_audio_composite.cxx')
  TargetAdd('libp3miles_audio.dll', input='miles_audio_miles_audio_composite.obj')
  TargetAdd('libp3miles_audio.dll', input=COMMON_PANDA_LIBS)
  TargetAdd('libp3miles_audio.dll', opts=['ADVAPI', 'WINUSER', 'WINMM', 'MILES'])

#
# DIRECTORY: panda/src/downloadertools/
#

if PkgSkip("OPENSSL")==0:
    OPTS=['DIR:panda/src/downloadertools', 'OPENSSL', 'ZLIB', 'ADVAPI']

    TargetAdd('apply_patch_apply_patch.obj', opts=OPTS, input='apply_patch.cxx')
    TargetAdd('apply_patch.exe', input=['apply_patch_apply_patch.obj']+COMMON_PANDA_LIBS)
    TargetAdd('apply_patch.exe', opts=OPTS)

    TargetAdd('build_patch_build_patch.obj', opts=OPTS, input='build_patch.cxx')
    TargetAdd('build_patch.exe', input=['build_patch_build_patch.obj']+COMMON_PANDA_LIBS)
    TargetAdd('build_patch.exe', opts=OPTS)

    TargetAdd('check_adler_check_adler.obj', opts=OPTS, input='check_adler.cxx')
    TargetAdd('check_adler.exe', input=['check_adler_check_adler.obj']+COMMON_PANDA_LIBS)
    TargetAdd('check_adler.exe', opts=OPTS)

    TargetAdd('check_crc_check_crc.obj', opts=OPTS, input='check_crc.cxx')
    TargetAdd('check_crc.exe', input=['check_crc_check_crc.obj']+COMMON_PANDA_LIBS)
    TargetAdd('check_crc.exe', opts=OPTS)

    TargetAdd('check_md5_check_md5.obj', opts=OPTS, input='check_md5.cxx')
    TargetAdd('check_md5.exe', input=['check_md5_check_md5.obj']+COMMON_PANDA_LIBS)
    TargetAdd('check_md5.exe', opts=OPTS)

    TargetAdd('pdecrypt_pdecrypt.obj', opts=OPTS, input='pdecrypt.cxx')
    TargetAdd('pdecrypt.exe', input=['pdecrypt_pdecrypt.obj']+COMMON_PANDA_LIBS)
    TargetAdd('pdecrypt.exe', opts=OPTS)

    TargetAdd('pencrypt_pencrypt.obj', opts=OPTS, input='pencrypt.cxx')
    TargetAdd('pencrypt.exe', input=['pencrypt_pencrypt.obj']+COMMON_PANDA_LIBS)
    TargetAdd('pencrypt.exe', opts=OPTS)

    TargetAdd('show_ddb_show_ddb.obj', opts=OPTS, input='show_ddb.cxx')
    TargetAdd('show_ddb.exe', input=['show_ddb_show_ddb.obj']+COMMON_PANDA_LIBS)
    TargetAdd('show_ddb.exe', opts=OPTS)

#
# DIRECTORY: panda/src/downloadertools/
#

OPTS=['DIR:panda/src/downloadertools', 'ZLIB', 'ADVAPI']

TargetAdd('multify_multify.obj', opts=OPTS, input='multify.cxx')
TargetAdd('multify.exe', input=['multify_multify.obj']+COMMON_PANDA_LIBS)
TargetAdd('multify.exe', opts=OPTS)

TargetAdd('pzip_pzip.obj', opts=OPTS, input='pzip.cxx')
TargetAdd('pzip.exe', input=['pzip_pzip.obj']+COMMON_PANDA_LIBS)
TargetAdd('pzip.exe', opts=OPTS)

TargetAdd('punzip_punzip.obj', opts=OPTS, input='punzip.cxx')
TargetAdd('punzip.exe', input=['punzip_punzip.obj']+COMMON_PANDA_LIBS)
TargetAdd('punzip.exe', opts=OPTS)

#
# DIRECTORY: panda/src/windisplay/
#

if (sys.platform == "win32"):
    OPTS=['DIR:panda/src/windisplay', 'BUILDING:PANDAWIN']
    TargetAdd('windisplay_composite.obj', opts=OPTS, input='windisplay_composite.cxx')
    TargetAdd('windisplay_windetectdx8.obj', opts=OPTS + ["DX8"], input='winDetectDx8.cxx')
    TargetAdd('windisplay_windetectdx9.obj', opts=OPTS + ["DX9"], input='winDetectDx9.cxx')
    TargetAdd('libp3windisplay.dll', input='windisplay_composite.obj')
    TargetAdd('libp3windisplay.dll', input='windisplay_windetectdx8.obj')
    TargetAdd('libp3windisplay.dll', input='windisplay_windetectdx9.obj')
    TargetAdd('libp3windisplay.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libp3windisplay.dll', opts=['WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM'])


#
# DIRECTORY: panda/metalibs/pandadx8/
#

if PkgSkip("DX8")==0:
    OPTS=['DIR:panda/src/dxgsg8', 'DIR:panda/metalibs/pandadx8', 'BUILDING:PANDADX', 'DX8']
    TargetAdd('dxgsg8_dxGraphicsStateGuardian8.obj', opts=OPTS, input='dxGraphicsStateGuardian8.cxx')
    TargetAdd('dxgsg8_composite.obj', opts=OPTS, input='dxgsg8_composite.cxx')
    TargetAdd('pandadx8_pandadx8.obj', opts=OPTS, input='pandadx8.cxx')
    TargetAdd('libpandadx8.dll', input='pandadx8_pandadx8.obj')
    TargetAdd('libpandadx8.dll', input='dxgsg8_dxGraphicsStateGuardian8.obj')
    TargetAdd('libpandadx8.dll', input='dxgsg8_composite.obj')
    TargetAdd('libpandadx8.dll', input='libp3windisplay.dll')
    TargetAdd('libpandadx8.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libpandadx8.dll', opts=['ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DX8'])

#
# DIRECTORY: panda/metalibs/pandadx9/
#

if PkgSkip("DX9")==0:
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
    TargetAdd('libpandadx9.dll', opts=['ADVAPI', 'WINGDI', 'WINKERNEL', 'WINUSER', 'WINMM', 'DX9',  'NVIDIACG', 'CGDX9'])

#
# DIRECTORY: panda/src/egg/
#

OPTS=['DIR:panda/src/egg', 'BUILDING:PANDAEGG',  'ZLIB', 'BISONPREFIX_eggyy', 'FLEXDASHI']
CreateFile("built/include/parser.h")
TargetAdd('egg_parser.obj', opts=OPTS, input='parser.yxx')
TargetAdd('parser.h', input='egg_parser.obj', opts=['DEPENDENCYONLY'])
TargetAdd('egg_lexer.obj', opts=OPTS, input='lexer.lxx')
TargetAdd('egg_composite1.obj', opts=OPTS, input='egg_composite1.cxx')
TargetAdd('egg_composite2.obj', opts=OPTS, input='egg_composite2.cxx')
IGATEFILES=GetDirectoryContents('panda/src/egg', ["*.h", "*_composite.cxx"])
TargetAdd('libegg.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libegg.in', opts=['IMOD:pandaegg', 'ILIB:libegg', 'SRCDIR:panda/src/egg'])
TargetAdd('libegg_igate.obj', input='libegg.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/egg2pg/
#

OPTS=['DIR:panda/src/egg2pg', 'BUILDING:PANDAEGG']
TargetAdd('egg2pg_composite.obj', opts=OPTS, input='egg2pg_composite.cxx')
IGATEFILES=['load_egg_file.h']
TargetAdd('libegg2pg.in', opts=OPTS, input=IGATEFILES)
TargetAdd('libegg2pg.in', opts=['IMOD:pandaegg', 'ILIB:libegg2pg', 'SRCDIR:panda/src/egg2pg'])
TargetAdd('libegg2pg_igate.obj', input='libegg2pg.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: panda/src/framework/
#

OPTS=['DIR:panda/src/framework', 'BUILDING:FRAMEWORK']
TargetAdd('framework_composite.obj', opts=OPTS, input='framework_composite.cxx')
TargetAdd('libp3framework.dll', input='framework_composite.obj')
TargetAdd('libp3framework.dll', input=COMMON_PANDA_LIBS)
TargetAdd('libp3framework.dll', opts=['ADVAPI'])

#
# DIRECTORY: panda/src/glstuff/
#

OPTS=['DIR:panda/src/glstuff',  'NVIDIACG', 'CGGL']
TargetAdd('glstuff_glpure.obj', opts=OPTS, input='glpure.cxx')
TargetAdd('libp3glstuff.dll', input='glstuff_glpure.obj')
TargetAdd('libp3glstuff.dll', input='libpandafx.dll')
TargetAdd('libp3glstuff.dll', input=COMMON_PANDA_LIBS)
TargetAdd('libp3glstuff.dll', opts=['ADVAPI', 'GLUT',  'NVIDIACG', 'CGGL'])

#
# DIRECTORY: panda/src/glgsg/
#

OPTS=['DIR:panda/src/glgsg', 'DIR:panda/src/glstuff', 'DIR:panda/src/gobj', 'BUILDING:PANDAGL',  'NVIDIACG']
TargetAdd('glgsg_config_glgsg.obj', opts=OPTS, input='config_glgsg.cxx')
TargetAdd('glgsg_glgsg.obj', opts=OPTS, input='glgsg.cxx')

#
# DIRECTORY: panda/metalibs/pandaegg/
#

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
# DIRECTORY: panda/metalibs/panda/
#

OPTS=['DIR:panda/metalibs/panda', 'BUILDING:PANDA',  'FFTW', 'PNG', 'JPEG', 'TIFF', 'ZLIB', 'OPENSSL', 'ADVAPI', 'WINSOCK2', 'WINUSER', 'WINMM']
TargetAdd('libpandastripped.dll', input='pipeline_composite.obj')
TargetAdd('libpandastripped.dll', input='event_composite.obj')
TargetAdd('libpandastripped.dll', input='net_composite.obj')
TargetAdd('libpandastripped.dll', input='nativenet_composite.obj')
TargetAdd('libpandastripped.dll', input='pstatclient_composite.obj')
TargetAdd('libpandastripped.dll', input='linmath_composite.obj')
TargetAdd('libpandastripped.dll', input='mathutil_composite.obj')
TargetAdd('libpandastripped.dll', input='putil_composite1.obj')
TargetAdd('libpandastripped.dll', input='putil_composite2.obj')
TargetAdd('libpandastripped.dll', input='pnmimagetypes_composite.obj')
TargetAdd('libpandastripped.dll', input='pnmimage_composite.obj')
TargetAdd('libpandastripped.dll', input='pandabase_pandabase.obj')
TargetAdd('libpandastripped.dll', input='libpandaexpress.dll')
TargetAdd('libpandastripped.dll', input='libp3dtoolconfig.dll')
TargetAdd('libpandastripped.dll', input='libp3dtool.dll')
TargetAdd('libpandastripped.dll', input='dtool_have_vrpn.dat')
TargetAdd('libpandastripped.dll', input='dtool_have_freetype.dat')
TargetAdd('libpandastripped.dll', opts=OPTS)

#
# DIRECTORY: panda/metalibs/pandaegg/
#

OPTS=['DIR:panda/metalibs/pandaegg', 'DIR:panda/src/egg', 'BUILDING:PANDAEGG']
TargetAdd('pandaegg_pandaeggnopg.obj', opts=OPTS, input='pandaeggnopg.cxx')
TargetAdd('libpandaeggstripped.dll', input='pandaegg_pandaeggnopg.obj')
TargetAdd('libpandaeggstripped.dll', input='egg_composite1.obj')
TargetAdd('libpandaeggstripped.dll', input='egg_composite2.obj')
TargetAdd('libpandaeggstripped.dll', input='egg_parser.obj')
TargetAdd('libpandaeggstripped.dll', input='egg_lexer.obj')
TargetAdd('libpandaeggstripped.dll', input='libpandastripped.dll')
TargetAdd('libpandaeggstripped.dll', input='libpandaexpress.dll')
TargetAdd('libpandaeggstripped.dll', input='libp3dtoolconfig.dll')
TargetAdd('libpandaeggstripped.dll', input='libp3dtool.dll')
TargetAdd('libpandaeggstripped.dll', opts=['ADVAPI'])

#
# DIRECTORY: panda/src/mesadisplay/
#

if (sys.platform != "win32"):
    OPTS=['DIR:panda/src/mesadisplay', 'DIR:panda/src/glstuff', 'BUILDING:PANDAGLUT', 'NVIDIACG', 'GLUT']
    TargetAdd('mesadisplay_composite.obj', opts=OPTS, input='mesadisplay_composite.cxx')
    OPTS=['DIR:panda/metalibs/pandagl', 'BUILDING:PANDAGLUT', 'NVIDIACG', 'GLUT']
    TargetAdd('libpandamesa.dll', input='mesadisplay_composite.obj')
    TargetAdd('libpandamesa.dll', input='libp3glstuff.dll')
    TargetAdd('libpandamesa.dll', input='libpandafx.dll')
    TargetAdd('libpandamesa.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libpandamesa.dll', opts=['GLUT'])

#
# DIRECTORY: panda/src/glxdisplay/
#

if (sys.platform != "win32"):
    OPTS=['DIR:panda/src/glxdisplay', 'BUILDING:PANDAGLUT',  'GLUT', 'NVIDIACG', 'CGGL']
    TargetAdd('glxdisplay_composite.obj', opts=OPTS, input='glxdisplay_composite.cxx')
    OPTS=['DIR:panda/metalibs/pandagl', 'BUILDING:PANDAGLUT',  'GLUT', 'NVIDIACG', 'CGGL']
    TargetAdd('pandagl_pandagl.obj', opts=OPTS, input='pandagl.cxx')
    TargetAdd('libpandagl.dll', input='pandagl_pandagl.obj')
    TargetAdd('libpandagl.dll', input='glgsg_config_glgsg.obj')
    TargetAdd('libpandagl.dll', input='glgsg_glgsg.obj')
    TargetAdd('libpandagl.dll', input='glxdisplay_composite.obj')
    TargetAdd('libpandagl.dll', input='libp3glstuff.dll')
    TargetAdd('libpandagl.dll', input='libpandafx.dll')
    TargetAdd('libpandagl.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libpandagl.dll', opts=['GLUT', 'NVIDIACG', 'CGGL'])

#
# DIRECTORY: panda/src/wgldisplay/
#

if (sys.platform == "win32"):
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
    TargetAdd('libpandagl.dll', opts=['WINGDI', 'GLUT', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM',  'NVIDIACG', 'CGGL'])

#
# DIRECTORY: panda/src/physics/
#

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
# DIRECTORY: panda/src/testbed/
#

OPTS=['DIR:panda/src/testbed']
TargetAdd('pview_pview.obj', opts=OPTS, input='pview.cxx')
TargetAdd('pview.exe', input='pview_pview.obj')
TargetAdd('pview.exe', input='libp3framework.dll')
TargetAdd('pview.exe', input='libpandafx.dll')
TargetAdd('pview.exe', input=COMMON_PANDA_LIBS)
TargetAdd('pview.exe', opts=['ADVAPI'])

#
# DIRECTORY: direct/src/directbase/
#

if (PkgSkip("PYTHON")==0):
    OPTS=['DIR:direct/src/directbase']

    TargetAdd('genpycode.obj', opts=OPTS+['BUILDING:GENPYCODE'], input='ppython.cxx')
    TargetAdd('genpycode.exe', input='genpycode.obj')
    TargetAdd('genpycode.exe', opts=['WINUSER'])

    TargetAdd('packpanda.obj', opts=OPTS+['BUILDING:PACKPANDA'], input='ppython.cxx')
    TargetAdd('packpanda.exe', input='packpanda.obj')
    TargetAdd('packpanda.exe', opts=['WINUSER'])

    TargetAdd('eggcacher.obj', opts=OPTS+['BUILDING:EGGCACHER'], input='ppython.cxx')
    TargetAdd('eggcacher.exe', input='eggcacher.obj')
    TargetAdd('eggcacher.exe', opts=['WINUSER'])

    OPTS=['DIR:direct/src/directbase', 'BUILDING:DIRECT']

    TargetAdd('directbase_directbase.obj', opts=OPTS, input='directbase.cxx')

#
# DIRECTORY: direct/src/dcparser/
#

if (PkgSkip("PYTHON")==0):
    OPTS=['DIR:direct/src/dcparser', 'WITHINPANDA', 'BUILDING:DIRECT', 'BISONPREFIX_dcyy']
    CreateFile("built/include/dcParser.h")
    TargetAdd('dcparser_dcParser.obj', opts=OPTS, input='dcParser.yxx')
    TargetAdd('dcParser.h', input='egg_parser.obj', opts=['DEPENDENCYONLY'])
    TargetAdd('dcparser_dcLexer.obj', opts=OPTS, input='dcLexer.lxx')
    TargetAdd('dcparser_composite.obj', opts=OPTS, input='dcparser_composite.cxx')
    IGATEFILES=GetDirectoryContents('direct/src/dcparser', ["*.h", "*_composite.cxx"])
    IGATEFILES.remove('dcmsgtypes.h')
    TargetAdd('libdcparser.in', opts=OPTS, input=IGATEFILES)
    TargetAdd('libdcparser.in', opts=['IMOD:p3direct', 'ILIB:libdcparser', 'SRCDIR:direct/src/dcparser'])
    TargetAdd('libdcparser_igate.obj', input='libdcparser.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/src/deadrec/
#

if (PkgSkip("PYTHON")==0):
    OPTS=['DIR:direct/src/deadrec', 'BUILDING:DIRECT']
    TargetAdd('deadrec_composite.obj', opts=OPTS, input='deadrec_composite.cxx')
    IGATEFILES=GetDirectoryContents('direct/src/deadrec', ["*.h", "*_composite.cxx"])
    TargetAdd('libdeadrec.in', opts=OPTS, input=IGATEFILES)
    TargetAdd('libdeadrec.in', opts=['IMOD:p3direct', 'ILIB:libdeadrec', 'SRCDIR:direct/src/deadrec'])
    TargetAdd('libdeadrec_igate.obj', input='libdeadrec.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/src/distributed/
#

if (PkgSkip("PYTHON")==0):
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

if (PkgSkip("PYTHON")==0):
    OPTS=['DIR:direct/src/interval', 'BUILDING:DIRECT']
    TargetAdd('interval_composite.obj', opts=OPTS, input='interval_composite.cxx')
    IGATEFILES=GetDirectoryContents('direct/src/interval', ["*.h", "*_composite.cxx"])
    TargetAdd('libinterval.in', opts=OPTS, input=IGATEFILES)
    TargetAdd('libinterval.in', opts=['IMOD:p3direct', 'ILIB:libinterval', 'SRCDIR:direct/src/interval'])
    TargetAdd('libinterval_igate.obj', input='libinterval.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/src/showbase/
#

if (PkgSkip("PYTHON")==0):
    OPTS=['DIR:direct/src/showbase', 'BUILDING:DIRECT']
    TargetAdd('showbase_showBase.obj', opts=OPTS, input='showBase.cxx')
    IGATEFILES=GetDirectoryContents('direct/src/showbase', ["*.h", "showBase.cxx"])
    TargetAdd('libshowbase.in', opts=OPTS, input=IGATEFILES)
    TargetAdd('libshowbase.in', opts=['IMOD:p3direct', 'ILIB:libshowbase', 'SRCDIR:direct/src/showbase'])
    TargetAdd('libshowbase_igate.obj', input='libshowbase.in', opts=["DEPENDENCYONLY"])

#
# DIRECTORY: direct/metalibs/direct/
#

if (PkgSkip("PYTHON")==0):
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
    TargetAdd('libp3direct.dll', opts=['ADVAPI',  'OPENSSL'])

#
# DIRECTORY: direct/src/dcparse/
#

if (PkgSkip("PYTHON")==0):
    OPTS=['DIR:direct/src/dcparse', 'DIR:direct/src/dcparser', 'WITHINPANDA', 'ADVAPI']
    TargetAdd('dcparse_dcparse.obj', opts=OPTS, input='dcparse.cxx')
    TargetAdd('p3dcparse.exe', input='dcparse_dcparse.obj')
    TargetAdd('p3dcparse.exe', input='libp3direct.dll')
    TargetAdd('p3dcparse.exe', input='libpandaexpress.dll')
    TargetAdd('p3dcparse.exe', input=COMMON_DTOOL_LIBS)
    TargetAdd('p3dcparse.exe', opts=['ADVAPI'])

#
# DIRECTORY: direct/src/heapq/
#

if (PkgSkip("PYTHON")==0):
    OPTS=['DIR:direct/src/heapq']
    TargetAdd('heapq_heapq.obj', opts=OPTS, input='heapq.cxx')
    TargetAdd('libp3heapq.dll', input='heapq_heapq.obj')
    TargetAdd('libp3heapq.dll', input='libpandaexpress.dll')
    TargetAdd('libp3heapq.dll', input=COMMON_DTOOL_LIBS)
    TargetAdd('libp3heapq.dll', opts=['ADVAPI'])

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
    TargetAdd('bam-info.exe', input='libp3pystub.dll')
    TargetAdd('bam-info.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('bam-info.exe', opts=['ADVAPI',  'FFTW'])

    TargetAdd('bam2egg_bamToEgg.obj', opts=OPTS, input='bamToEgg.cxx')
    TargetAdd('bam2egg.exe', input='bam2egg_bamToEgg.obj')
    TargetAdd('bam2egg.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('bam2egg.exe', opts=['ADVAPI',  'FFTW'])

    TargetAdd('egg2bam_eggToBam.obj', opts=OPTS, input='eggToBam.cxx')
    TargetAdd('egg2bam.exe', input='egg2bam_eggToBam.obj')
    TargetAdd('egg2bam.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('egg2bam.exe', opts=['ADVAPI',  'FFTW'])
    
#
# DIRECTORY: pandatool/src/cvscopy/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/cvscopy']
    TargetAdd('cvscopy_composite1.obj', opts=OPTS, input='cvscopy_composite1.cxx')
    TargetAdd('libcvscopy.lib', input='cvscopy_composite1.obj')

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
    TargetAdd('dxf-points.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('dxf-points.exe', input='libp3pystub.dll')
    TargetAdd('dxf-points.exe', opts=['ADVAPI',  'FFTW'])

    TargetAdd('dxf2egg_dxfToEgg.obj', opts=OPTS, input='dxfToEgg.cxx')
    TargetAdd('dxf2egg.exe', input='dxf2egg_dxfToEgg.obj')
    TargetAdd('dxf2egg.exe', input='libdxfegg.lib')
    TargetAdd('dxf2egg.exe', input='libdxf.lib')
    TargetAdd('dxf2egg.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('dxf2egg.exe', opts=['ADVAPI',  'FFTW'])

    TargetAdd('egg2dxf_eggToDXF.obj', opts=OPTS, input='eggToDXF.cxx')
    TargetAdd('egg2dxf_eggToDXFLayer.obj', opts=OPTS, input='eggToDXFLayer.cxx')
    TargetAdd('egg2dxf.exe', input='egg2dxf_eggToDXF.obj')
    TargetAdd('egg2dxf.exe', input='egg2dxf_eggToDXFLayer.obj')
    TargetAdd('egg2dxf.exe', input='libdxf.lib')
    TargetAdd('egg2dxf.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('egg2dxf.exe', opts=['ADVAPI',  'FFTW'])

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
    TargetAdd('egg-mkfont.exe', input=COMMON_EGG2X_LIBS)
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
    TargetAdd('egg-optchar.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('egg-optchar.exe', opts=['ADVAPI', 'FREETYPE'])

#
# DIRECTORY: pandatool/src/egg-palettize/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/egg-palettize', 'DIR:pandatool/src/palettizer']
    TargetAdd('egg-palettize_eggPalettize.obj', opts=OPTS, input='eggPalettize.cxx')
    TargetAdd('egg-palettize.exe', input='egg-palettize_eggPalettize.obj')
    TargetAdd('egg-palettize.exe', input='libpalettizer.lib')
    TargetAdd('egg-palettize.exe', input=COMMON_EGG2X_LIBS)
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
    TargetAdd('egg-qtess.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('egg-qtess.exe', opts=['ADVAPI'])
    
#
# DIRECTORY: pandatool/src/eggprogs/
#

if (PkgSkip("PANDATOOL")==0):
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
    TargetAdd('egg-topstrip.exe', input='libeggcharbase.lib')
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
    TargetAdd('egg-retarget-anim.exe', input='libeggcharbase.lib')
    TargetAdd('egg-retarget-anim.exe', input=COMMON_EGG2X_LIBS)
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
    TargetAdd('egg2flt.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('egg2flt.exe', opts=['ADVAPI'])

    TargetAdd('flt-info_fltInfo.obj', opts=OPTS, input='fltInfo.cxx')
    TargetAdd('flt-info.exe', input='flt-info_fltInfo.obj')
    TargetAdd('flt-info.exe', input='libflt.lib')
    TargetAdd('flt-info.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('flt-info.exe', opts=['ADVAPI'])

    TargetAdd('flt-trans_fltTrans.obj', opts=OPTS, input='fltTrans.cxx')
    TargetAdd('flt-trans.exe', input='flt-trans_fltTrans.obj')
    TargetAdd('flt-trans.exe', input='libflt.lib')
    TargetAdd('flt-trans.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('flt-trans.exe', opts=['ADVAPI'])

    TargetAdd('flt2egg_fltToEgg.obj', opts=OPTS, input='fltToEgg.cxx')
    TargetAdd('flt2egg.exe', input='flt2egg_fltToEgg.obj')
    TargetAdd('flt2egg.exe', input='libflt.lib')
    TargetAdd('flt2egg.exe', input='libfltegg.lib')
    TargetAdd('flt2egg.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('flt2egg.exe', opts=['ADVAPI'])

    TargetAdd('fltcopy_fltCopy.obj', opts=OPTS, input='fltCopy.cxx')
    TargetAdd('fltcopy.exe', input='fltcopy_fltCopy.obj')
    TargetAdd('fltcopy.exe', input='libcvscopy.lib')
    TargetAdd('fltcopy.exe', input='libflt.lib')
    TargetAdd('fltcopy.exe', input=COMMON_EGG2X_LIBS)
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
    TargetAdd('lwo2egg.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('lwo2egg.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/maya/
#

for VER in MAYAVERSIONS:
  if (PkgSkip("MAYA"+VER)==0) and (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/maya', 'MAYA'+VER]
    TargetAdd('maya'+VER+'_composite1.obj', opts=OPTS, input='maya_composite1.cxx')
    TargetAdd('libmaya'+VER+'.lib', input='maya'+VER+'_composite1.obj')

#
# DIRECTORY: pandatool/src/mayaegg/
#

for VER in MAYAVERSIONS:
  if (PkgSkip("MAYA"+VER)==0) and (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/mayaegg', 'DIR:pandatool/src/maya', 'MAYA'+VER]
    TargetAdd('mayaegg'+VER+'_loader.obj', opts=OPTS, input='mayaEggLoader.cxx')
    TargetAdd('mayaegg'+VER+'_composite1.obj', opts=OPTS, input='mayaegg_composite1.cxx')
    TargetAdd('libmayaegg'+VER+'.lib', input='mayaegg'+VER+'_composite1.obj')

#
# DIRECTORY: pandatool/src/maxegg/
#

for VER in MAXVERSIONS:
  if (PkgSkip("MAX"+VER)==0) and (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/maxegg', 'MAX'+VER,  "WINCOMCTL", "WINCOMDLG", "WINUSER", "MSFORSCOPE"]
    CopyFile("built/tmp/maxEgg.obj", "pandatool/src/maxegg/maxEgg.obj")
    TargetAdd('maxegg'+VER+'_loader.obj', opts=OPTS, input='maxEggLoader.cxx')
    TargetAdd('maxegg'+VER+'_composite1.obj', opts=OPTS, input='maxegg_composite1.cxx')
    TargetAdd('maxegg'+VER+'.dlo', input='maxegg'+VER+'_composite1.obj')
    TargetAdd('maxegg'+VER+'.dlo', input='maxEgg.obj')
    TargetAdd('maxegg'+VER+'.dlo', input='maxEgg.def', ipath=OPTS)
    TargetAdd('maxegg'+VER+'.dlo', input=COMMON_EGG2X_LIBS)
    TargetAdd('maxegg'+VER+'.dlo', opts=OPTS)

#
# DIRECTORY: pandatool/src/maxprogs/
#

for VER in MAXVERSIONS:
  if (PkgSkip("MAX"+VER)==0) and (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/maxprogs', 'MAX'+VER,  "WINCOMCTL", "WINCOMDLG", "WINUSER", "MSFORSCOPE"]
    CopyFile("built/tmp/maxImportRes.obj", "pandatool/src/maxprogs/maxImportRes.obj")
    TargetAdd('maxprogs'+VER+'_maxeggimport.obj', opts=OPTS, input='maxEggImport.cxx')
    TargetAdd('maxeggimport'+VER+'.dle', input='maxegg'+VER+'_loader.obj')
    TargetAdd('maxeggimport'+VER+'.dle', input='maxprogs'+VER+'_maxeggimport.obj')
    TargetAdd('maxeggimport'+VER+'.dle', input='maxImportRes.obj')
    TargetAdd('maxeggimport'+VER+'.dle', input='libpandaeggstripped.dll')
    TargetAdd('maxeggimport'+VER+'.dle', input='libpandastripped.dll')
    TargetAdd('maxeggimport'+VER+'.dle', input='libpandaexpress.dll')
    TargetAdd('maxeggimport'+VER+'.dle', input='maxEggImport.def', ipath=OPTS)
    TargetAdd('maxeggimport'+VER+'.dle', input=COMMON_DTOOL_LIBS)
    TargetAdd('maxeggimport'+VER+'.dle', opts=OPTS)

#
# DIRECTORY: pandatool/src/vrml/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/vrml', 'ZLIB', 'BISONPREFIX_vrmlyy']
    CreateFile("built/include/vrmlParser.h")
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
    OPTS=['DIR:pandatool/src/xfile', 'ZLIB', 'BISONPREFIX_xyy']
    CreateFile("built/include/xParser.h")
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
    OPTS=['DIR:pandatool/src/ptloader', 'DIR:pandatool/src/flt', 'DIR:pandatool/src/lwo', 'DIR:pandatool/src/xfile', 'DIR:pandatool/src/xfileegg', 'BUILDING:PTLOADER']
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
    TargetAdd('libp3ptloader.dll', input='libvrmlegg.lib')
    TargetAdd('libp3ptloader.dll', input='libpvrml.lib')
    TargetAdd('libp3ptloader.dll', input='libxfileegg.lib')
    TargetAdd('libp3ptloader.dll', input='libxfile.lib')
    TargetAdd('libp3ptloader.dll', input='libeggbase.lib')
    TargetAdd('libp3ptloader.dll', input='libprogbase.lib')
    TargetAdd('libp3ptloader.dll', input='libconverter.lib')
    TargetAdd('libp3ptloader.dll', input='libpandatoolbase.lib')
    TargetAdd('libp3ptloader.dll', input='libpandaegg.dll')
    TargetAdd('libp3ptloader.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libp3ptloader.dll', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/mayaprogs/
#

for VER in MAYAVERSIONS:
  if (PkgSkip('MAYA'+VER)==0) and (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/mayaprogs', 'DIR:pandatool/src/maya', 'DIR:pandatool/src/mayaegg', 'DIR:pandatool/src/cvscopy', 'BUILDING:MISC', 'MAYA'+VER]
    TargetAdd('mayaeggimport'+VER+'_mayaeggimport.obj', opts=OPTS, input='mayaEggImport.cxx')
    TargetAdd('mayaeggimport'+VER+'.mll', input='mayaegg'+VER+'_loader.obj')
    TargetAdd('mayaeggimport'+VER+'.mll', input='mayaeggimport'+VER+'_mayaeggimport.obj')
    TargetAdd('mayaeggimport'+VER+'.mll', input='libpandaegg.dll')
    TargetAdd('mayaeggimport'+VER+'.mll', input=COMMON_PANDA_LIBS)
    TargetAdd('mayaeggimport'+VER+'.mll', input='libp3pystub.dll')
    TargetAdd('mayaeggimport'+VER+'.mll', opts=['ADVAPI', 'MAYA'+VER])

    TargetAdd('mayaloader'+VER+'_config_mayaloader.obj', opts=OPTS, input='config_mayaloader.cxx')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='mayaloader'+VER+'_config_mayaloader.obj')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libmayaegg'+VER+'.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libp3ptloader.dll')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libmaya'+VER+'.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libfltegg.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libflt.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='liblwoegg.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='liblwo.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libdxfegg.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libdxf.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libvrmlegg.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libpvrml.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libxfileegg.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libxfile.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libeggbase.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libprogbase.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libconverter.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libpandatoolbase.lib')
    TargetAdd('libp3mayaloader'+VER+'.dll', input='libpandaegg.dll')
    TargetAdd('libp3mayaloader'+VER+'.dll', input=COMMON_PANDA_LIBS)
    TargetAdd('libp3mayaloader'+VER+'.dll', opts=['ADVAPI', 'MAYA'+VER])

    TargetAdd('mayapview'+VER+'_mayaPview.obj', opts=OPTS, input='mayaPview.cxx')
    TargetAdd('libmayapview'+VER+'.mll', input='mayapview'+VER+'_mayaPview.obj')
    TargetAdd('libmayapview'+VER+'.mll', input='libmayaegg'+VER+'.lib')
    TargetAdd('libmayapview'+VER+'.mll', input='libmaya'+VER+'.lib')
    TargetAdd('libmayapview'+VER+'.mll', input='libp3framework.dll')
    TargetAdd('libmayapview'+VER+'.mll', input=COMMON_EGG2X_LIBS)
    TargetAdd('libmayapview'+VER+'.mll', opts=['ADVAPI', 'MAYA'+VER])

    TargetAdd('maya2egg'+VER+'_mayaToEgg.obj', opts=OPTS, input='mayaToEgg.cxx')
    TargetAdd('maya2egg'+VER+'-wrapped.exe', input='maya2egg'+VER+'_mayaToEgg.obj')
    TargetAdd('maya2egg'+VER+'-wrapped.exe', input='libmayaegg'+VER+'.lib')
    TargetAdd('maya2egg'+VER+'-wrapped.exe', input='libmaya'+VER+'.lib')
    TargetAdd('maya2egg'+VER+'-wrapped.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('maya2egg'+VER+'-wrapped.exe', opts=['ADVAPI', 'MAYA'+VER])

    TargetAdd('mayacopy'+VER+'_mayaCopy.obj', opts=OPTS, input='mayaCopy.cxx')
    TargetAdd('mayacopy'+VER+'-wrapped.exe', input='mayacopy'+VER+'_mayaCopy.obj')
    TargetAdd('mayacopy'+VER+'-wrapped.exe', input='libcvscopy.lib')
    TargetAdd('mayacopy'+VER+'-wrapped.exe', input='libmaya'+VER+'.lib')
    TargetAdd('mayacopy'+VER+'-wrapped.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('mayacopy'+VER+'-wrapped.exe', opts=['ADVAPI', 'MAYA'+VER])

    TargetAdd('mayasavepview'+VER+'_mayaSavePview.obj', opts=OPTS, input='mayaSavePview.cxx')
    TargetAdd('libmayasavepview'+VER+'.mll', input='mayasavepview'+VER+'_mayaSavePview.obj')
    TargetAdd('libmayasavepview'+VER+'.mll', opts=['ADVAPI',  'MAYA'+VER])

    TargetAdd('mayaWrapper'+VER+'.obj', opts=OPTS, input='mayaWrapper.cxx')

    TargetAdd('maya2egg'+VER+'.exe', input='mayaWrapper'+VER+'.obj')
    TargetAdd('maya2egg'+VER+'.exe', opts=['ADVAPI'])

    TargetAdd('mayacopy'+VER+'.exe', input='mayaWrapper'+VER+'.obj')
    TargetAdd('mayacopy'+VER+'.exe', opts=['ADVAPI'])



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
    OPTS=['DIR:pandatool/src/softprogs']
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
    TargetAdd('vrml2egg.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('vrml2egg.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandatool/src/win-stats/
#

if (PkgSkip("PANDATOOL")==0) and (sys.platform == "win32"):
    OPTS=['DIR:pandatool/src/win-stats']
    TargetAdd('pstats_composite1.obj', opts=OPTS, input='winstats_composite1.cxx')
    TargetAdd('pstats.exe', input='pstats_composite1.obj')
    TargetAdd('pstats.exe', input='libpstatserver.lib')
    TargetAdd('pstats.exe', input='libprogbase.lib')
    TargetAdd('pstats.exe', input='libpandatoolbase.lib')
    TargetAdd('pstats.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('pstats.exe', input='libp3pystub.dll')
    TargetAdd('pstats.exe', opts=['WINSOCK', 'WINIMM', 'WINGDI', 'WINKERNEL', 'WINOLDNAMES', 'WINUSER', 'WINMM'])

#
# DIRECTORY: pandatool/src/xfileprogs/
#

if (PkgSkip("PANDATOOL")==0):
    OPTS=['DIR:pandatool/src/xfileprogs', 'DIR:pandatool/src/xfile', 'DIR:pandatool/src/xfileegg']
    TargetAdd('egg2x_eggToX.obj', opts=OPTS, input='eggToX.cxx')
    TargetAdd('egg2x.exe', input='egg2x_eggToX.obj')
    TargetAdd('egg2x.exe', input='libxfileegg.lib')
    TargetAdd('egg2x.exe', input='libxfile.lib')
    TargetAdd('egg2x.exe', input=COMMON_EGG2X_LIBS)
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
    TargetAdd('x2egg.exe', input=COMMON_EGG2X_LIBS)
    TargetAdd('x2egg.exe', opts=['ADVAPI'])

#
# DIRECTORY: pandaapp/src/pandaappbase/
#

if (PkgSkip("PANDAAPP")==0):
    OPTS=['DIR:pandaapp/src/pandaappbase']
    TargetAdd('pandaappbase_pandaappbase.obj', opts=OPTS, input='pandaappbase.cxx')
    TargetAdd('libpandaappbase.lib', input='pandaappbase_pandaappbase.obj')

#
# DIRECTORY: pandaapp/src/httpbackup/
#

if (PkgSkip("OPENSSL")==0) and (PkgSkip("PANDAAPP")==0):
    OPTS=['DIR:pandaapp/src/httpbackup', 'DIR:pandaapp/src/pandaappbase', 'OPENSSL']
    TargetAdd('httpbackup_backupCatalog.obj', opts=OPTS, input='backupCatalog.cxx')
    TargetAdd('httpbackup_httpBackup.obj', opts=OPTS, input='httpBackup.cxx')
    TargetAdd('httpbackup.exe', input='httpbackup_backupCatalog.obj')
    TargetAdd('httpbackup.exe', input='httpbackup_httpBackup.obj')
    TargetAdd('httpbackup.exe', input='libpandaappbase.lib')
    TargetAdd('httpbackup.exe', input='libprogbase.lib')
    TargetAdd('httpbackup.exe', input='libpandatoolbase.lib')
    TargetAdd('httpbackup.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('httpbackup.exe', input='libp3pystub.dll')
    TargetAdd('httpbackup.exe', opts=['ADVAPI', 'OPENSSL'])

#
# DIRECTORY: pandaapp/src/indexify/
#

if (PkgSkip("FREETYPE")==0) and (PkgSkip("PANDAAPP")==0):
    OPTS=['DIR:pandaapp/src/indexify', 'FREETYPE']
    TargetAdd('font-samples_default_font.obj', opts=OPTS, input='default_font.cxx')
    TargetAdd('font-samples_fontSamples.obj', opts=OPTS, input='fontSamples.cxx')
    TargetAdd('font-samples.exe', input='font-samples_default_font.obj')
    TargetAdd('font-samples.exe', input='font-samples_fontSamples.obj')
    TargetAdd('font-samples.exe', input='libprogbase.lib')
    TargetAdd('font-samples.exe', input='libpandatoolbase.lib')
    TargetAdd('font-samples.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('font-samples.exe', input='libp3pystub.dll')
    TargetAdd('font-samples.exe', opts=['ADVAPI', 'FREETYPE'])

    TargetAdd('indexify_default_index_icons.obj', opts=OPTS, input='default_index_icons.cxx')
    TargetAdd('indexify_default_font.obj', opts=OPTS, input='default_font.cxx')
    TargetAdd('indexify_indexImage.obj', opts=OPTS, input='indexImage.cxx')
    TargetAdd('indexify_indexParameters.obj', opts=OPTS, input='indexParameters.cxx')
    TargetAdd('indexify_indexify.obj', opts=OPTS, input='indexify.cxx')
    TargetAdd('indexify_photo.obj', opts=OPTS, input='photo.cxx')
    TargetAdd('indexify_rollDirectory.obj', opts=OPTS, input='rollDirectory.cxx')

    TargetAdd('indexify.exe', input='indexify_default_index_icons.obj')
    TargetAdd('indexify.exe', input='indexify_default_font.obj')
    TargetAdd('indexify.exe', input='indexify_indexImage.obj')
    TargetAdd('indexify.exe', input='indexify_indexParameters.obj')
    TargetAdd('indexify.exe', input='indexify_indexify.obj')
    TargetAdd('indexify.exe', input='indexify_photo.obj')
    TargetAdd('indexify.exe', input='indexify_rollDirectory.obj')
    TargetAdd('indexify.exe', input='libprogbase.lib')
    TargetAdd('indexify.exe', input='libpandatoolbase.lib')
    TargetAdd('indexify.exe', input=COMMON_PANDA_LIBS)
    TargetAdd('indexify.exe', input='libp3pystub.dll')
    TargetAdd('indexify.exe', opts=['ADVAPI', 'FREETYPE'])

#
# Generate the models directory and samples directory
#

if (PkgSkip("PANDATOOL")==0):

    for model in GetDirectoryContents("dmodels/src/misc", ["*.egg", "*.flt"]):
        eggpz = model[:-4] + ".egg.pz"
        TargetAdd("built/models/misc/"+eggpz, input="dmodels/src/misc/"+model)

    for model in GetDirectoryContents("dmodels/src/gui", ["*.egg", "*.flt"]):
        eggpz = model[:-4] + ".egg.pz"
        TargetAdd("built/models/gui/"+eggpz, input="dmodels/src/gui/"+model)

    for model in GetDirectoryContents("models", ["*.egg", "*.flt"]):
        eggpz = model[:-4] + ".egg.pz"
        TargetAdd("built/models/"+eggpz, input="models/"+model)

    CopyAllFiles("built/models/audio/sfx/",  "dmodels/src/audio/sfx/", ".wav")
    CopyAllFiles("built/models/icons/",      "dmodels/src/icons/",     ".gif")
    
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

DEPENDENCYQUEUE=[]

for target in TARGET_LIST:
    ext = target.ext
    name = target.name
    inputs = target.inputs
    opts = target.opts
    deps = target.deps
    DEPENDENCYQUEUE.append([CompileAnything, [name, ext, inputs, opts], [name], deps, []])

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
        if (pending.has_key(x)):
            return 0
    altsources = task[4]
    for x in altsources:
        if (pending.has_key(x)):
            return 0
    return 1

def ParallelMake(tasklist):
    # create the communication queues.
    donequeue=Queue.Queue()
    taskqueue=Queue.Queue()
    # build up a table listing all the pending targets
    pending = {}
    for task in tasklist:
        for target in task[2]:
            pending[target] = 1
    # create the workers
    for slave in range(THREADCOUNT):
        th = threading.Thread(target=BuildWorker, args=[taskqueue,donequeue])
        th.setDaemon(1)
        th.start()
    # feed tasks to the workers.
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
    # kill the workers.
    for slave in range(THREADCOUNT):
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
# Run genpycode
#
##########################################################################################

if (PkgSkip("PYTHON")==0):
    inputs = []
    for x in GetDirectoryContents("built/pandac/input", ["*.in"]):
        inputs.append("built/pandac/input/" + x)
    if (NeedsBuild(['built/pandac/PandaModules.py'],inputs)):
        if (GENMAN): oscmd("built/bin/genpycode -d")
        else       : oscmd("built/bin/genpycode")
        JustBuilt(['built/pandac/PandaModules.py'],inputs)

##########################################################################################
#
# The Installers
#
# Under windows, we can build an 'exe' package using NSIS
# Under linux, we can build a 'deb' package or an 'rpm' package.
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
    cmd=cmd+'/DRUNTEXT="Visit the Panda Manual" '
    cmd=cmd+'/DIBITMAP="panda-install.bmp" '
    cmd=cmd+'/DUBITMAP="panda-uninstall.bmp" '
    cmd=cmd+'/DPANDA="'+panda+'" '
    cmd=cmd+'/DPANDACONF="'+panda+'\\etc" '
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
BuildRoot: PANDASOURCE/linuxroot
%description
The Panda3D engine.
%post
/sbin/ldconfig
%postun
/sbin/ldconfig
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
    PYTHONV=os.path.basename(SDK["PYTHON"])
    if (os.path.isdir("linuxroot")): oscmd("chmod -R 755 linuxroot")
    if (os.path.exists("/usr/bin/dpkg-deb")):
        oscmd("dpkg --print-architecture > built/tmp/architecture.txt")
    if (os.path.exists("/usr/bin/rpmbuild")):
        oscmd("rpm -E '%_target_cpu' > built/tmp/architecture.txt")
    ARCH=ReadFile("built/tmp/architecture.txt").strip()
    oscmd("rm -rf linuxroot data.tar.gz control.tar.gz panda3d.spec "+ARCH)
    oscmd("mkdir -p linuxroot/usr/bin")
    oscmd("mkdir -p linuxroot/usr/include")
    oscmd("mkdir -p linuxroot/usr/share/panda3d")
    oscmd("mkdir -p linuxroot/usr/lib/panda3d")
    oscmd("mkdir -p linuxroot/usr/lib/"+PYTHONV+"/lib-dynload")
    oscmd("mkdir -p linuxroot/usr/lib/"+PYTHONV+"/site-packages")
    oscmd("mkdir -p linuxroot/etc/ld.so.conf.d")
    oscmd("sed -e 's@model-cache-@# model-cache-@' -e 's@$THIS_PRC_DIR/[.][.]@/usr/share/panda3d@' < built/etc/Config.prc > linuxroot/etc/Config.prc")
    oscmd("cp built/etc/Confauto.prc  linuxroot/etc/Confauto.prc")
    oscmd("cp --recursive built/include linuxroot/usr/include/panda3d")
    oscmd("cp --recursive direct        linuxroot/usr/share/panda3d/direct")
    oscmd("cp --recursive built/pandac  linuxroot/usr/share/panda3d/pandac")
    oscmd("cp --recursive built/Pmw     linuxroot/usr/share/panda3d/Pmw")
    oscmd("cp built/direct/__init__.py  linuxroot/usr/share/panda3d/direct/__init__.py")
    oscmd("cp --recursive built/models  linuxroot/usr/share/panda3d/models")
    oscmd("cp --recursive samples       linuxroot/usr/share/panda3d/samples")
    oscmd("cp doc/LICENSE               linuxroot/usr/share/panda3d/LICENSE")
    oscmd("cp doc/LICENSE               linuxroot/usr/include/panda3d/LICENSE")
    oscmd("cp doc/ReleaseNotes          linuxroot/usr/share/panda3d/ReleaseNotes")
    oscmd("echo '/usr/lib/panda3d'   >  linuxroot/etc/ld.so.conf.d/panda3d.conf")
    oscmd("echo '/usr/share/panda3d' >  linuxroot/usr/lib/"+PYTHONV+"/site-packages/panda3d.pth")
    oscmd("echo '/usr/lib/panda3d'   >> linuxroot/usr/lib/"+PYTHONV+"/site-packages/panda3d.pth")
    oscmd("cp built/bin/*               linuxroot/usr/bin/")
    for base in os.listdir("built/lib"):
        oscmd("cp built/lib/"+base+" linuxroot/usr/lib/panda3d/"+base)
    for base in os.listdir("linuxroot/usr/share/panda3d/direct/src"):
        if ((base != "extensions") and (base != "extensions_native")):
            compileall.compile_dir("linuxroot/usr/share/panda3d/direct/src/"+base)
    compileall.compile_dir("linuxroot/usr/share/panda3d/Pmw")
    DeleteCVS("linuxroot")
    oscmd("chmod -R 555 linuxroot/usr/share/panda3d")

    if (os.path.exists("/usr/bin/dpkg-deb")):
        txt = INSTALLER_DEB_FILE[1:].replace("VERSION",str(VERSION)).replace("PYTHONV",PYTHONV)
        oscmd("mkdir -p linuxroot/DEBIAN")
        oscmd("cd linuxroot ; (find usr -type f -exec md5sum {} \;) >  DEBIAN/md5sums")
        oscmd("cd linuxroot ; (find etc -type f -exec md5sum {} \;) >> DEBIAN/md5sums")
        WriteFile("linuxroot/DEBIAN/conffiles","/etc/Config.prc\n")
        WriteFile("linuxroot/DEBIAN/control",txt)
	WriteFile("linuxroot/DEBIAN/postinst","#!/bin/sh\necho running ldconfig\nldconfig\n")
	oscmd("chmod 755 linuxroot/DEBIAN/postinst")
	oscmd("cp linuxroot/DEBIAN/postinst linuxroot/DEBIAN/postrm")
        oscmd("dpkg-deb -b linuxroot panda3d_"+VERSION+"_"+ARCH+".deb")
        oscmd("chmod -R 755 linuxroot")

    if (os.path.exists("/usr/bin/rpmbuild")):
        pandasource = os.path.abspath(os.getcwd())
        txt = INSTALLER_SPEC_FILE[1:].replace("VERSION",VERSION).replace("PANDASOURCE",pandasource)
        WriteFile("panda3d.spec", txt)
        oscmd("rpmbuild --define '_rpmdir "+pandasource+"' -bb panda3d.spec")
        oscmd("mv "+ARCH+"/panda3d-"+VERSION+"-1."+ARCH+".rpm .")

#    oscmd("chmod -R 755 linuxroot")
#    oscmd("rm -rf linuxroot data.tar.gz control.tar.gz panda3d.spec "+ARCH)
    
        
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

printStatus("Makepanda Final Status Report", WARNINGS)

