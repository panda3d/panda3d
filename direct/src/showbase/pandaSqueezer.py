"""Undocumented Module"""

__all__ = []

import os
import sys
import getopt
import pandaSqueezeTool

# Assumption: We will be squeezing the files from the current directory or the -d directory.

if __name__ == "__main__":
    try:
        opts, pargs = getopt.getopt(sys.argv[1:], 'Od:')
    except Exception, e:
        # User passed in a bad option, print the error and the help, then exit
        print e
        print 'Usage: pass in -O for optimized'
        print '       pass in -d directory'
        sys.exit()

    fOptimized = 0
    # Store the option values into our variables
    for opt in opts:
        flag, value = opt
        if (flag == '-O'):
            fOptimized = 1
            print 'Squeezing pyo files'
        elif (flag == '-d'):
            os.chdir(value)

    def getSqueezeableFiles():
        fileList = os.listdir(".")
        newFileList = []
        if fOptimized:
            targetFileExtension = ".pyo"
        else:
            targetFileExtension = ".pyc"
        for i in fileList:
            base, ext = os.path.splitext(i)
            if (ext == ".py"):
                newFileList.append(i)
        return newFileList

    def squeezePandaFiles():
        l = getSqueezeableFiles()
        pandaSqueezeTool.squeeze("PandaModules", "PandaModulesUnsqueezed", l)

        # Clean up the source files now that they've been squeezed.  If
        # you don't like this behavior (e.g. if you want to inspect the
        # generated files), use genPyCode -n to avoid squeezing
        # altogether.
        for i in l:
            os.unlink(i)


    squeezePandaFiles()
