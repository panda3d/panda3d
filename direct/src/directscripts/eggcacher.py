##############################################################################
#
# eggcacher
#
# EggCacher searches a directory for egg files, and loads
# them all into the model-cache.  This is used as part of the
# panda installation process.
#
##############################################################################

import os,sys,gc
from pandac.PandaModules import *

class EggCacher:
    def __init__(self, args):
        maindir = Filename.fromOsSpecific(os.getcwd()).getFullpath()
        ExecutionEnvironment.setEnvironmentVariable("MAIN_DIR", maindir)
        self.bamcache = BamCache.getGlobalPtr()
        self.pandaloader = PandaLoader()
        self.loaderopts = LoaderOptions()
        if (self.bamcache.getActive() == 0):
            print "The model cache is not currently active."
            print "You must set a model-cache-dir in your config file."
            sys.exit(1)
        self.parseArgs(args)
        files = self.scanPaths(self.paths)
        self.processFiles(files)

    def parseArgs(self, args):
        self.concise = 0
        self.pzkeep = 0
        while len(args):
            if (args[0]=="--concise"):
                self.concise = 1
                args = args[1:]
            elif (args[0]=="--pzkeep"):
                self.pzkeep = 1
                args = args[1:]
            else:
                break
        if (len(args) < 1):
            print "Usage: eggcacher options file-or-directory"
            sys.exit(1)
        self.paths = args

    def scanPath(self, eggs, path):
        if (os.path.exists(path)==0):
            print "No such file or directory: "+path
            return
        if (os.path.isdir(path)):
            for f in os.listdir(path):
                self.scanPath(eggs, os.path.join(path,f))
            return
        if (path.endswith(".egg")):
            size = os.path.getsize(path)
            eggs.append((path,size))
            return
        if (path.endswith(".egg.pz")):
            size = os.path.getsize(path)
            if (self.pzkeep): eggs.append((path,size))
            else: eggs.append((path[:-3],size))

    def scanPaths(self, paths):
        eggs = []
        for path in paths:
            abs = os.path.abspath(path)
            self.scanPath(eggs,path)
        return eggs

    def processFiles(self, files):
        total = 0
        for (path,size) in files:
            total += size
        progress = 0
        for (path,size) in files:
            fn = Filename.fromOsSpecific(path)
            cached = self.bamcache.lookup(fn,"bam")
            percent = (progress * 100) / total
            report = path
            if (self.concise): report = os.path.basename(report)
            print "Preprocessing Models %2d%% %s" % (percent, report)
            sys.stdout.flush()
            if (cached) and (cached.hasData()==0):
                self.pandaloader.loadSync(fn, self.loaderopts)
            gc.collect()
            ModelPool.releaseAllModels()
            TexturePool.releaseAllTextures()
            progress += size


cacher = EggCacher(sys.argv[1:])

