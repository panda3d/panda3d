##############################################################################
#
# eggcacher
#
# this tool searches a directory for egg files, and loads
# them all into the model-cache. 
#
##############################################################################

import os,sys
from pandac.PandaModules import *

MODELCACHE = ConfigVariableFilename("model-cache-dir", Filename()).getValue()

if (MODELCACHE.getValue().isEmpty()==0):
    print "The config variable 'model-cache-dir' is not set."
    print "You cannot cache egg files until you configure a cache dir."
    sys.exit(1)

