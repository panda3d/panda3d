##############################################################
#
# This module should be invoked by a shell-script that says:
#
#    python direct\\src\\ffi\\jGenPyCode.py <arguments>
#
# Before invoking python, the shell-script may need to set
# these environment variables, to make sure that everything
# can be located appropriately.
#
#    PYTHONPATH
#    PANDAROOT
#    PATH
#
##############################################################

import sys,os;

if (os.environ.has_key("PANDAROOT")==0):
  print "jGenPyCode was not invoked correctly"
  sys.exit(1)

pandaroot = os.environ["PANDAROOT"]
if (os.path.isdir(os.path.join(pandaroot,"direct","src"))):
  directsrc=os.path.join(pandaroot,"direct","src")
elif (os.path.isdir(os.path.join(os.path.dirname(pandaroot),"direct","src"))):
  directsrc=os.path.join(os.path.dirname(pandaroot),"direct","src")
else:
  print "jGenPyCode cannot locate the 'direct' tree"
  sys.exit(1)

from direct.ffi import DoGenPyCode
from direct.ffi import FFIConstants
DoGenPyCode.outputDir = os.path.join(pandaroot,"lib","pandac")
DoGenPyCode.extensionsDir = os.path.join(directsrc,"extensions")
DoGenPyCode.interrogateLib = r'libdtoolconfig'
DoGenPyCode.codeLibs = ['libpandaexpress','libpanda','libpandaphysics','libpandafx','libdirect']
DoGenPyCode.etcPath = [os.path.join(pandaroot,"etc")]

#print "outputDir = ",DoGenPyCode.outputDir
#print "extensionsDir = ",DoGenPyCode.extensionsDir
#print "interrogateLib = ",DoGenPyCode.interrogateLib
#print "codeLibs = ",DoGenPyCode.codeLibs
#print "etcPath = ",DoGenPyCode.etcPath

DoGenPyCode.run()

