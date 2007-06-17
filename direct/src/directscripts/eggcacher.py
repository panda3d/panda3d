##############################################################################
#
# eggcacher
#
# EggCacher searches a directory for egg files, and loads
# them all into the model-cache.  This is used as part of the
# panda installation process.
#
##############################################################################

import os,sys
from pandac.PandaModules import *

class EggCacher:
    def __init__(self):
        if (len(sys.argv) != 2):
            print "Usage: eggcacher <file-or-directory>"
            sys.exit(1)

        self.bamcache = BamCache.getGlobalPtr()
        self.pandaloader = PandaLoader()
        self.loaderopts = LoaderOptions()
        if (self.bamcache.getActive() == 0):
            print "The model cache is not currently active."
            print "You must set a model-cache-dir in your config file."
            sys.exit(1)

    def traversePath(self, path):
        if (os.path.exists(path)==0):
            print "No such file or directory: "+path
            return
        if (os.path.isdir(path)):
            for f in os.listdir(path):
                self.traversePath(os.path.join(path,f))
            return
        if (path.endswith(".egg")) or (path.endswith(".egg.pz")):
            fn = Filename.fromOsSpecific(path)
            cached = self.bamcache.lookup(fn,"bam")
            if (cached == None):
                print "Not cacheable: "+path
            elif (cached.hasData()):
                print "Already Cached: "+path
            else:
                print "Caching "+path
                self.pandaloader.loadSync(fn, self.loaderopts)
            ModelPool.releaseAllModels()

cacher = EggCacher()
for x in sys.argv[1:]:
    cacher.traversePath(os.path.abspath(x))


