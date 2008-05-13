##############################################################
#
# This module should be invoked by a shell-script that says:
#
#    python -c "import direct.ffi.jGenPyCode" <arguments>
#
# Before invoking python, the shell-script may need to set
# these environment variables, to make sure that everything
# can be located appropriately.
#
#    PYTHONPATH
#    PATH
#    LD_LIBRARY_PATH
#
##############################################################

import sys, os

##############################################################
#
# Locate the 'direct' tree and the 'pandac' tree.
#
##############################################################

DIRECT=None
PANDAC=None

for dir in sys.path:
    if (DIRECT is None):
        if (dir != "") and os.path.exists(os.path.join(dir,"direct")):
            DIRECT=os.path.join(dir,"direct")
    if (PANDAC is None):
        if (dir != "") and (os.path.exists(os.path.join(dir,"pandac"))):
            PANDAC=os.path.join(dir,"pandac")

if (DIRECT is None):
    sys.exit("Could not locate the 'direct' python modules")
if (PANDAC is None):
    sys.exit("Could not locate the 'pandac' python modules")

##############################################################
#
# Locate direct/src/extensions.
#
# It could be inside the direct tree.  It may be underneath
# a 'src' subdirectory.  Or, the direct tree may actually be
# a stub that points to the source tree.
#
##############################################################

EXTENSIONS=None

if (EXTENSIONS is None):
  if os.path.isdir(os.path.join(DIRECT,"src","extensions_native")):
    EXTENSIONS=os.path.join(DIRECT,"src","extensions_native")

if (EXTENSIONS is None):
  if os.path.isdir(os.path.join(DIRECT,"extensions_native")):
    EXTENSIONS=os.path.join(DIRECT,"extensions_native")

if (EXTENSIONS is None):
  if os.path.isdir(os.path.join(DIRECT,"..","..","direct","src","extensions_native")):
    EXTENSIONS=os.path.join(DIRECT,"..","..","direct","src","extensions_native")

if (EXTENSIONS is None):
  sys.exit("Could not locate direct/src/extensions_native")

##############################################################
#
# Call genpycode with default paths.
#
##############################################################

from direct.ffi import DoGenPyCode
from direct.ffi import FFIConstants
DoGenPyCode.outputCodeDir = PANDAC
DoGenPyCode.outputHTMLDir = os.path.join(PANDAC,"..","doc")
DoGenPyCode.directDir = DIRECT
DoGenPyCode.extensionsDir = EXTENSIONS
DoGenPyCode.interrogateLib = r'libdtoolconfig'
DoGenPyCode.codeLibs = ['libpandaexpress','libpanda','libpandaphysics','libpandafx','libp3direct','libpandaskel','libpandaegg','libpandaode']
DoGenPyCode.etcPath = [os.path.join(PANDAC,"input")]
DoGenPyCode.pythonSourcePath = [DIRECT]
DoGenPyCode.native = 1

#print "outputDir = ", DoGenPyCode.outputDir
#print "directDir = ", DoGenPyCode.directDir
#print "extensionsDir = ", DoGenPyCode.extensionsDir
#print "interrogateLib = ", DoGenPyCode.interrogateLib
#print "codeLibs = ", DoGenPyCode.codeLibs
#print "etcPath = ", DoGenPyCode.etcPath
#print "native = ", DoGenPyCode.native

DoGenPyCode.run()

