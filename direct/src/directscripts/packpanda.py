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

import sys,os,getopt,compileall

OPTIONLIST = [
("game",      1, "Name of directory containing game"),
("version",   1, "Version number to add to game name"),
("fast",      0, "Use fast compression instead of good compression"),
("bam",       0, "Convert all EGG files to BAM"),
("pyc",       0, "Convert all PY files to PYC"),
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
      if (hasval): longopts.append(opt+"=")
      else: longopts.append(opt)
      options[opt] = 0
      if (hasval): options[opt]=""
    opts, extras = getopt.getopt(args, "", longopts)
    for option,value in opts:
      for (opt, hasval, explanation) in OPTIONLIST:
        if (option == "--"+opt):
          if (hasval): options[opt] = value
          else: options[opt] = 1
    return options
  except: ParseFailure();

OPTIONS = ParseOptions(sys.argv[1:])

##############################################################################
#
# Identify the main parts of the game: GAME, NAME, MAIN, ICON, IMAGE
#
##############################################################################

VER=OPTIONS["version"]
GAME=OPTIONS["game"]
if (GAME==""):
  print "You must specify the --game option."
  ParseFailure()
GAME=os.path.abspath(GAME)
NAME=os.path.basename(GAME)
STARTMENU=os.path.basename(GAME)
if (VER!=""): STARTMENU=STARTMENU+" "+VER
MAIN=NAME+".py"
ICON=os.path.join(GAME,NAME+".ico")
IMAGE=os.path.join(GAME,NAME+".bmp")
OUTPUT=os.path.basename(GAME)
if (VER!=""): OUTPUT=OUTPUT+"-"+VER
OUTPUT=os.path.abspath(OUTPUT+".exe")
INSTALLTO='C:\\'+os.path.basename(GAME)
if (VER!=""): INSTALLTO=INSTALLTO+"-"+VER
COMPRESS="lzma"
if (OPTIONS["fast"]): COMPRESS="zlib"

def PrintFileStatus(label, file):
  if (os.path.exists(file)):
    print "%-15s: %s"%(label,file)
  else:
    print "%-15s: %s (MISSING)"%(label,file)
  
PrintFileStatus("Game",GAME)
print "%-15s: %s"%("Name",NAME)
print "%-15s: %s"%("Start Menu",STARTMENU)
PrintFileStatus("Main",os.path.join(GAME,MAIN))
PrintFileStatus("Icon",ICON)
PrintFileStatus("Image",IMAGE)
print "%-15s: %s"%("Output",OUTPUT)
print "%-15s: %s"%("Install To",INSTALLTO)

if (os.path.isdir(GAME)==0):
  sys.exit("Difficulty reading "+GAME+". Cannot continue.")

if (os.path.isfile(os.path.join(GAME,MAIN))==0):
  sys.exit("Difficulty reading "+MAIN+". Cannot continue.")

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
# Run NSIS. Yay!
#
##############################################################################

CMD=NSIS+"\\makensis.exe /V2 "
CMD=CMD+'/DCOMPRESSOR="'+COMPRESS+'" '
CMD=CMD+'/DFULLNAME="'+NAME+'" '
CMD=CMD+'/DSMDIRECTORY="'+STARTMENU+'" '
CMD=CMD+'/DINSTALLDIR="'+INSTALLTO+'" '
CMD=CMD+'/DPANDA="'+PANDA+'" '
CMD=CMD+'/DPSOURCE="'+PSOURCE+'" '
CMD=CMD+'/DOUTFILE="'+OUTPUT+'" '
CMD=CMD+'/DPPGAMEID="'+NAME+'" '
CMD=CMD+'/DPPGAMEPATH="'+GAME+'" '
CMD=CMD+'/DPPGAMEPY="'+MAIN+'" '
CMD=CMD+'"'+PSOURCE+'\\direct\\src\\directscripts\\packpanda.nsi"'

print ""
print CMD
print "packing..."
os.system(CMD)
