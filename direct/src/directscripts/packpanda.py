##############################################################################
#
# packpanda - this is a tool that packages up a panda game into a
# convenient, easily-downloaded windows executable.  Packpanda relies on
# NSIS, the netscape scriptable install system, to do the hard work.
#
# This is intentionally a very simplistic game-packer with very
# limited options.  The goal is simplicity, not feature richness.
# There are dozens of complex, powerful packaging tools already out
# there.  This one is for people who just want to do it quick and
# easy.
#
##############################################################################

import sys,os,getopt,string,shutil,py_compile

OPTIONLIST = [
("game",      1, "Name of directory containing game"),
("version",   1, "Version number to add to game name"),
("rmdir",     2, "Delete all directories with given name"),
("rmext",     2, "Delete all files with given extension"),
("fast",      0, "Use fast compression instead of good compression"),
("bam",       0, "Generate BAM files"),
("pyc",       0, "Generate PYC files"),
]

def ParseFailure():
  print ""
  print "packpanda usage:"
  print ""
  for (opt, hasval, explanation) in OPTIONLIST:
    if (hasval):
      print "  --%-10s    %s"%(opt+" x",explanation)
    else:
      print "  --%-10s    %s"%(opt+"  ",explanation)
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
    for option,value in opts:
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
    if os.path.exists(os.path.join(dir,"direct")) and os.path.exists(os.path.join(dir,"pandac")) and os.path.exists(os.path.join(dir,"python")):
        PANDA=os.path.abspath(dir)
if (PANDA is None):
  sys.exit("Cannot locate the panda root directory in the python path (cannot locate directory containing direct and pandac).")

if (os.path.exists(os.path.join("PANDA","..","makepanda","makepanda.py"))) and (os.path.exists(os.path.join("PANDA","..","thirdparty","win-nsis","makensis.exe"))):
  PSOURCE=os.path.abspath(os.path.join(PANDA,".."))
  NSIS=os.path.abspath(os.path.join(PANDA,"..","thirdparty","win-nsis"))
else:
  PSOURCE=PANDA
  NSIS=os.path.join(PANDA,"nsis")

##############################################################################
#
# Identify the main parts of the game: GAME, NAME, MAIN, ICON, BITMAP, etc
#
##############################################################################

VER=OPTIONS["version"]
GAME=OPTIONS["game"]
if (GAME==""):
  print "You must specify the --game option."
  ParseFailure()
GAME=os.path.abspath(GAME)
NAME=os.path.basename(GAME)
SMDIRECTORY=os.path.basename(GAME)
if (VER!=""): SMDIRECTORY=SMDIRECTORY+" "+VER
ICON=os.path.join(GAME,NAME+".ico")
BITMAP=os.path.join(GAME,NAME+".bmp")
LICENSE=os.path.join(GAME,"LICENSE.TXT")
OUTFILE=os.path.basename(GAME)
if (VER!=""): OUTFILE=OUTFILE+"-"+VER
OUTFILE=os.path.abspath(OUTFILE+".exe")
INSTALLDIR='C:\\'+os.path.basename(GAME)
if (VER!=""): INSTALLDIR=INSTALLDIR+"-"+VER
COMPRESS="lzma"
if (OPTIONS["fast"]): COMPRESS="zlib"
if (OPTIONS["pyc"]): MAIN=NAME+".pyc"
else: MAIN=NAME+".py"

def PrintFileStatus(label, file):
  if (os.path.exists(file)):
    print "%-15s: %s"%(label,file)
  else:
    print "%-15s: %s (MISSING)"%(label,file)
  
PrintFileStatus("Game",GAME)
print "%-15s: %s"%("Name",NAME)
print "%-15s: %s"%("Start Menu",SMDIRECTORY)
PrintFileStatus("Main",os.path.join(GAME,MAIN))
PrintFileStatus("Icon",ICON)
PrintFileStatus("Bitmap",BITMAP)
PrintFileStatus("License",LICENSE)
print "%-15s: %s"%("Output",OUTFILE)
print "%-15s: %s"%("Install Dir",INSTALLDIR)

if (os.path.isdir(GAME)==0):
  sys.exit("Difficulty reading "+GAME+". Cannot continue.")

if (os.path.isfile(os.path.join(GAME,NAME+".py"))==0):
  sys.exit("Difficulty reading "+NAME+".py. Cannot continue.")

if (os.path.isfile(LICENSE)==0):
  LICENSE=os.path.join(PANDA,"LICENSE")

if (os.path.isfile(BITMAP)==0):
  BITMAP=os.path.join(NSIS,"Contrib","Modern UI","Graphics","Wizard","nsis.bmp")

##############################################################################
#
# Copy the game to a temporary directory, so we can modify it safely.
#
##############################################################################

TMPDIR=os.path.abspath("packpanda-TMP")
print ""
print "Copying the game to "+TMPDIR+"..."
if (os.path.exists(TMPDIR)):
   try: shutil.rmtree(TMPDIR)
   except: sys.exit("Cannot delete "+TMPDIR)
try: shutil.copytree(GAME, TMPDIR)
except: sys.exit("Cannot copy game to "+TMPDIR)

##############################################################################
#
# Compile all py files, convert all egg files.
#
# We do this as a sanity check, even if the user
# hasn't requested that his files be compiled.
#
##############################################################################

EGG2BAM=os.path.join(PANDA,"bin","egg2bam.exe")

def egg2bam(file):
    bam = file[:-4]+'.bam'
    present = os.path.exists(bam)
    if (present): bam = "packpanda-TMP.bam";
    cmd = 'egg2bam -noabs -ps rel -pd . "'+file+'" -o "'+bam+'"'
    print "Executing: "+cmd
    res = os.spawnl(os.P_WAIT, EGG2BAM, cmd)
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
        if (string.lower(file[-4:])==".egg"):
            egg2bam(file)
        elif (string.lower(file[-3:])==".py"):
            py2pyc(file)
        else: pass
    elif (os.path.isdir(file)):
        for x in os.listdir(file):
            CompileFiles(os.path.join(file,x))

def DeleteFiles(file):
    base = string.lower(os.path.basename(file))
    if (os.path.isdir(file)):
        for pattern in OPTIONS["rmdir"]:
            if (string.lower(pattern) == base):
                print "Deleting "+file
                shutil.rmtree(file)
                return
        for x in os.listdir(file):
            DeleteFiles(os.path.join(file,x))
    else:
        for ext in OPTIONS["rmext"]:
            if (base[-(len(ext)+1):] == string.lower("."+ext)):
                print "Deleting "+file
                os.unlink(file)
                return

print ""
print "Compiling BAM and PYC files..."
os.chdir(TMPDIR)
CompileFiles(".")
DeleteFiles(".")

##############################################################################
#
# Run NSIS. Yay!
#
##############################################################################

CMD=NSIS+"\\makensis.exe /V2 "
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
CMD=CMD+'/DPSOURCE="'+PSOURCE+'" '
CMD=CMD+'/DPPGAME="'+TMPDIR+'" '
CMD=CMD+'/DPPMAIN="'+MAIN+'" '
CMD=CMD+'"'+PSOURCE+'\\direct\\src\\directscripts\\packpanda.nsi"'

print ""
print CMD
print "packing..."
os.system(CMD)


