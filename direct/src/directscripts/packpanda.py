#############################################################################
#
# packpanda - this is a tool that packages up a panda game into a
# convenient, easily-downloaded windows executable.  Packpanda runs on linux
# and windows - on linux, it builds .debs and .rpms, on windows it relies on
# NSIS, the nullsoft scriptable install system, to do the hard work.
#
# This is intentionally a very simplistic game-packer with very
# limited options.  The goal is simplicity, not feature richness.
# There are dozens of complex, powerful packaging tools already out
# there.  This one is for people who just want to do it quick and
# easy.
#
##############################################################################

import sys, os, getopt, string, shutil, py_compile, subprocess

OPTIONLIST = [
("dir",       1, "Name of directory containing game"),
("name",      1, "Human-readable name of the game"),
("version",   1, "Version number to add to game name"),
("rmdir",     2, "Delete all directories with given name"),
("rmext",     2, "Delete all files with given extension"),
("fast",      0, "Use fast compression instead of good compression"),
("bam",       0, "Generate BAM files, change default-model-extension to BAM"),
("pyc",       0, "Generate PYC files"),
]

def ParseFailure():
  print ""
  print "packpanda usage:"
  print ""
  for (opt, hasval, explanation) in OPTIONLIST:
    if (hasval):
      print "  --%-10s    %s"%(opt+" x", explanation)
    else:
      print "  --%-10s    %s"%(opt+"  ", explanation)
  sys.exit(1)

def ParseOptions(args):
  try:
    options = {}
    longopts = []
    for (opt, hasval, explanation) in OPTIONLIST:
      if (hasval==2):
        longopts.append(opt+"=")
        options[opt] = []
      elif (hasval==1):
        longopts.append(opt+"=")
        options[opt] = ""
      else:
        longopts.append(opt)
        options[opt] = 0
    opts, extras = getopt.getopt(args, "", longopts)
    for option, value in opts:
      for (opt, hasval, explanation) in OPTIONLIST:
        if (option == "--"+opt):
          if (hasval==2): options[opt].append(value)
          elif (hasval==1): options[opt] = value
          else: options[opt] = 1
    return options
  except: ParseFailure();

OPTIONS = ParseOptions(sys.argv[1:])

##############################################################################
#
# Locate the relevant trees.
#
##############################################################################

PANDA=None
for dir in sys.path:
    if (dir != "") and os.path.exists(os.path.join(dir,"direct")) and os.path.exists(os.path.join(dir,"pandac")):
        PANDA=os.path.abspath(dir)
if (PANDA is None):
  sys.exit("Cannot locate the panda root directory in the python path (cannot locate directory containing direct and pandac).")
print "PANDA located at "+PANDA

if (os.path.exists(os.path.join(PANDA,"..","makepanda","makepanda.py"))) and (sys.platform != "win32" or os.path.exists(os.path.join(PANDA,"..","thirdparty","win-nsis","makensis.exe"))):
  PSOURCE=os.path.abspath(os.path.join(PANDA,".."))
  if (sys.platform == "win32"):
    NSIS=os.path.abspath(os.path.join(PANDA,"..","thirdparty","win-nsis"))
else:
  PSOURCE=PANDA
  if (sys.platform == "win32"):
    NSIS=os.path.join(PANDA,"nsis")

##############################################################################
#
# Identify the main parts of the game: DIR, NAME, MAIN, ICON, BITMAP, etc
#
##############################################################################

VER=OPTIONS["version"]
DIR=OPTIONS["dir"]
if (DIR==""):
  print "You must specify the --dir option."
  ParseFailure()
DIR=os.path.abspath(DIR)
MYDIR=os.path.abspath(os.getcwd())
BASENAME=os.path.basename(DIR)
if (OPTIONS["name"] != ""):
  NAME=OPTIONS["name"]
else:
  NAME=BASENAME
SMDIRECTORY=NAME
if (VER!=""): SMDIRECTORY=SMDIRECTORY+" "+VER
PYTHONV="python"+sys.version[:3]
LICENSE=os.path.join(DIR, "license.txt")
OUTFILE=os.path.basename(DIR)
if (VER!=""): OUTFILE=OUTFILE+"-"+VER
if (sys.platform == "win32"):
  ICON=os.path.join(DIR, "icon.ico")
  BITMAP=os.path.join(DIR, "installer.bmp")
  OUTFILE=os.path.abspath(OUTFILE+".exe")
  INSTALLDIR='C:\\'+os.path.basename(DIR)
  if (VER!=""): INSTALLDIR=INSTALLDIR+"-"+VER
  COMPRESS="lzma"
  if (OPTIONS["fast"]): COMPRESS="zlib"
if (OPTIONS["pyc"]): MAIN="main.pyc"
else: MAIN="main.py"

def PrintFileStatus(label, file):
  if (os.path.exists(file)):
    print "%-15s: %s"%(label, file)
  else:
    print "%-15s: %s (MISSING)"%(label, file)

PrintFileStatus("Dir", DIR)
print "%-15s: %s"%("Name", NAME)
print "%-15s: %s"%("Start Menu", SMDIRECTORY)
PrintFileStatus("Main", os.path.join(DIR, MAIN))
if (sys.platform == "win32"):
  PrintFileStatus("Icon", ICON)
  PrintFileStatus("Bitmap", BITMAP)
PrintFileStatus("License", LICENSE)
print "%-15s: %s"%("Output", OUTFILE)
if (sys.platform == "win32"):
  print "%-15s: %s"%("Install Dir", INSTALLDIR)

if (os.path.isdir(DIR)==0):
  sys.exit("Difficulty reading "+DIR+". Cannot continue.")

if (os.path.isfile(os.path.join(DIR, "main.py"))==0):
  sys.exit("Difficulty reading main.py. Cannot continue.")

if (os.path.isfile(LICENSE)==0):
  LICENSE=os.path.join(PANDA,"LICENSE")

if (sys.platform == "win32") and (os.path.isfile(BITMAP)==0):
  BITMAP=os.path.join(NSIS,"Contrib","Graphics","Wizard","nsis.bmp")

if (sys.platform == "win32"):
  if (os.path.isfile(ICON)==0):
    PPICON="bin\\ppython.exe"
  else:
    PPICON="game\\icon.ico"

##############################################################################
#
# Copy the game to a temporary directory, so we can modify it safely.
#
##############################################################################

def limitedCopyTree(src, dst, rmdir):
    if (os.path.isdir(src)):
        if (rmdir.has_key(os.path.basename(src))):
            return
        if (not os.path.isdir(dst)): os.mkdir(dst)
        for x in os.listdir(src):
            limitedCopyTree(os.path.join(src,x), os.path.join(dst,x), rmdir)
    else:
        shutil.copyfile(src, dst)


TMPDIR=os.path.abspath("packpanda-TMP")
if (sys.platform == "win32"):
  TMPGAME=os.path.join(TMPDIR,"game")
  TMPETC=os.path.join(TMPDIR,"etc")
else:
  TMPGAME=os.path.join(TMPDIR,"usr","share","games",BASENAME,"game")
  TMPETC=os.path.join(TMPDIR,"usr","share","games",BASENAME,"etc")
print ""
print "Copying the game to "+TMPDIR+"..."
if (os.path.exists(TMPDIR)):
    try: shutil.rmtree(TMPDIR)
    except: sys.exit("Cannot delete "+TMPDIR)
try:
    os.mkdir(TMPDIR)
    rmdir = {}
    for x in OPTIONS["rmdir"]:
        rmdir[x] = 1
    if not os.path.isdir( TMPGAME ):
        os.makedirs(TMPGAME)
    limitedCopyTree(DIR, TMPGAME, rmdir)
    if not os.path.isdir( TMPETC ):
        os.makedirs(TMPETC)
    if sys.platform == "win32":
      limitedCopyTree(os.path.join(PANDA, "etc"), TMPETC, {})
    else:
      shutil.copyfile("/etc/Config.prc", os.path.join(TMPETC, "Config.prc"))
      shutil.copyfile("/etc/Confauto.prc", os.path.join(TMPETC, "Confauto.prc"))
except: sys.exit("Cannot copy game to "+TMPDIR)

##############################################################################
#
# If --bam requested, change default-model-extension .egg to bam.
#
##############################################################################

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

if OPTIONS["bam"]:
    CONF=ReadFile(os.path.join(TMPETC,"Confauto.prc"))
    CONF=CONF.replace("default-model-extension .egg","default-model-extension .bam")
    WriteFile(os.path.join(TMPETC,"Confauto.prc"), CONF)

##############################################################################
#
# Compile all py files, convert all egg files.
#
# We do this as a sanity check, even if the user
# hasn't requested that his files be compiled.
#
##############################################################################

if (sys.platform == "win32"):
  EGG2BAM=os.path.join(PANDA,"bin","egg2bam.exe")
else:
  EGG2BAM=os.path.join(PANDA,"bin","egg2bam")

def egg2bam(file,bam):
    present = os.path.exists(bam)
    if (present): bam = "packpanda-TMP.bam";
    cmd = 'egg2bam -noabs -ps rel -pd . "'+file+'" -o "'+bam+'"'
    print "Executing: "+cmd
    if (sys.platform == "win32"):
      res = os.spawnl(os.P_WAIT, EGG2BAM, cmd)
    else:
      res = os.system(cmd)
    if (res != 0): sys.exit("Problem in egg file: "+file)
    if (present) or (OPTIONS["bam"]==0):
        os.unlink(bam)

def py2pyc(file):
    print "Compiling python "+file
    pyc = file[:-3]+'.pyc'
    pyo = file[:-3]+'.pyo'
    if (os.path.exists(pyc)): os.unlink(pyc)
    if (os.path.exists(pyo)): os.unlink(pyo)
    try: py_compile.compile(file)
    except: sys.exit("Cannot compile "+file)
    if (OPTIONS["pyc"]==0):
        if (os.path.exists(pyc)):
            os.unlink(pyc)
        if (os.path.exists(pyo)):
            os.unlink(pyo)

def CompileFiles(file):
    if (os.path.isfile(file)):
        if (file.endswith(".egg")):
            egg2bam(file, file[:-4]+'.bam')
        elif (file.endswith(".egg.pz")):
            egg2bam(file, file[:-7]+'.bam')
        elif (file.endswith(".py")):
            py2pyc(file)
        else: pass
    elif (os.path.isdir(file)):
        for x in os.listdir(file):
            CompileFiles(os.path.join(file, x))

def DeleteFiles(file):
    base = string.lower(os.path.basename(file))
    if (os.path.isdir(file)):
        for pattern in OPTIONS["rmdir"]:
            if (string.lower(pattern) == base):
                print "Deleting "+file
                shutil.rmtree(file)
                return
        for x in os.listdir(file):
            DeleteFiles(os.path.join(file, x))
    else:
        for ext in OPTIONS["rmext"]:
            if (base[-(len(ext)+1):] == string.lower("."+ext)):
                print "Deleting "+file
                os.unlink(file)
                return

print ""
print "Compiling BAM and PYC files..."
os.chdir(TMPGAME)
CompileFiles(".")
DeleteFiles(".")

##############################################################################
#
# Now make the installer. Yay!
#
##############################################################################

INSTALLER_DEB_FILE="""
Package: BASENAME
Version: VERSION
Section: games
Priority: optional
Architecture: ARCH
Essential: no
Depends: PYTHONV
Provides: BASENAME
Description: NAME
Maintainer: Unknown
"""

INSTALLER_SPEC_FILE="""
Summary: NAME
Name: BASENAME
Version: VERSION
Release: 1
Group: Amusement/Games
License: See license file
BuildRoot: TMPDIR
BuildRequires: PYTHONV
%description
NAME
%files
%defattr(-,root,root)
/usr/bin/BASENAME
/usr/lib/games/BASENAME
/usr/share/games/BASENAME
"""

RUN_SCRIPT="""
#!/bin/sh
cd /usr/share/games/BASENAME/game
PYTHONPATH=/usr/lib/games/BASENAME:/usr/share/games/BASENAME
LD_LIBRARY_PATH=/usr/lib/games/BASENAME
PYTHONV MAIN
"""

if (sys.platform == "win32"):
    CMD="\""+NSIS+"\\makensis.exe\" /V2 "
    CMD=CMD+'/DCOMPRESSOR="'+COMPRESS+'" '
    CMD=CMD+'/DNAME="'+NAME+'" '
    CMD=CMD+'/DSMDIRECTORY="'+SMDIRECTORY+'" '
    CMD=CMD+'/DINSTALLDIR="'+INSTALLDIR+'" '
    CMD=CMD+'/DOUTFILE="'+OUTFILE+'" '
    CMD=CMD+'/DLICENSE="'+LICENSE+'" '
    CMD=CMD+'/DLANGUAGE="English" '
    CMD=CMD+'/DRUNTEXT="Play '+NAME+'" '
    CMD=CMD+'/DIBITMAP="'+BITMAP+'" '
    CMD=CMD+'/DUBITMAP="'+BITMAP+'" '
    CMD=CMD+'/DPANDA="'+PANDA+'" '
    CMD=CMD+'/DPANDACONF="'+TMPETC+'" '
    CMD=CMD+'/DPSOURCE="'+PSOURCE+'" '
    CMD=CMD+'/DPPGAME="'+TMPGAME+'" '
    CMD=CMD+'/DPPMAIN="'+MAIN+'" '
    CMD=CMD+'/DPPICON="'+PPICON+'" '
    CMD=CMD+'"'+PSOURCE+'\\direct\\directscripts\\packpanda.nsi"' 
    
    print ""
    print CMD
    print "packing..."
    subprocess.call(CMD)
else:
    os.chdir(MYDIR)
    os.system("mkdir -p %s/usr/bin" % TMPDIR)
    os.system("mkdir -p %s/usr/share/games/%s" % (TMPDIR, BASENAME))
    os.system("mkdir -p %s/usr/lib/games/%s" % (TMPDIR, BASENAME))
    os.system("cp --recursive %s/direct          %s/usr/share/games/%s/direct" % (PANDA, TMPDIR, BASENAME))
    os.system("cp --recursive %s/pandac          %s/usr/share/games/%s/pandac" % (PANDA, TMPDIR, BASENAME))
    os.system("cp --recursive %s/models          %s/usr/share/games/%s/models" % (PANDA, TMPDIR, BASENAME))
    os.system("cp --recursive %s/Pmw             %s/usr/share/games/%s/Pmw" % (PANDA, TMPDIR, BASENAME))
    os.system("cp %s                             %s/usr/share/games/%s/LICENSE" % (LICENSE, TMPDIR, BASENAME))
    os.system("cp --recursive /usr/lib/panda3d/* %s/usr/lib/games/%s/" % (TMPDIR, BASENAME))
    
    # Make the script to run the game
    txt = RUN_SCRIPT[1:].replace("BASENAME",BASENAME).replace("PYTHONV",PYTHONV).replace("MAIN",MAIN)
    WriteFile(TMPDIR+"/usr/bin/"+BASENAME, txt)
    os.system("chmod +x "+TMPDIR+"/usr/bin/"+BASENAME)
    
    if (os.path.exists("/usr/bin/rpmbuild")):
        os.system("rm -rf %s/DEBIAN" % TMPDIR)
        os.system("rpm -E '%_target_cpu' > packpanda-TMP.txt")
        ARCH=ReadFile("packpanda-TMP.txt").strip()
        os.remove("packpanda-TMP.txt")
        txt = INSTALLER_SPEC_FILE[1:].replace("VERSION",VER).replace("TMPDIR",TMPDIR)
        txt = txt.replace("BASENAME",BASENAME).replace("NAME",NAME).replace("PYTHONV",PYTHONV)
        WriteFile("packpanda-TMP.spec", txt)
        os.system("rpmbuild --define '_rpmdir "+TMPDIR+"' -bb packpanda-TMP.spec")
        os.system("mv "+ARCH+"/"+BASENAME+"-"+VER+"-1."+ARCH+".rpm .")
        os.rmdir(ARCH)
        os.remove("packpanda-TMP.spec")
    
    if (os.path.exists("/usr/bin/dpkg-deb")):
        os.system("dpkg --print-architecture > packpanda-TMP.txt")
        ARCH=ReadFile("packpanda-TMP.txt").strip()
        os.remove("packpanda-TMP.txt")
        txt = INSTALLER_DEB_FILE[1:].replace("VERSION",str(VER)).replace("PYTHONV",PYTHONV)
        txt = txt.replace("BASENAME",BASENAME).replace("NAME",NAME).replace("ARCH",ARCH)
        os.system("mkdir -p %s/DEBIAN" % TMPDIR)
        os.system("cd %s ; (find usr -type f -exec md5sum {} \;) >  DEBIAN/md5sums" % TMPDIR)
        WriteFile(TMPDIR+"/DEBIAN/control",txt)
        os.system("dpkg-deb -b "+TMPDIR+" "+BASENAME+"_"+VER+"_"+ARCH+".deb")
    
    if not(os.path.exists("/usr/bin/rpmbuild") or os.path.exists("/usr/bin/dpkg-deb")):
        exit("To build an installer, either rpmbuild or dpkg-deb must be present on your system!")

