import os
import sys
import getopt
import pandaSqueezeTool

# Assumption: We will be squeezing the files from the current directory.

try:
    opts, pargs = getopt.getopt(sys.argv[1:], 'O')
except Exception, e:
    # User passed in a bad option, print the error and the help, then exit
    print e
    print 'Usage: pass in -O for optimized'
    sys.exit()

fOptimized = 0
# Store the option values into our variables
for opt in opts:
    flag, value = opt
    if (flag == '-O'):
        fOptimized = 1
        print 'Squeezing pyo files'

def getSqueezeableFiles():
    fileList = os.listdir(".")
    newFileList = []
    if fOptimized:
        targetFileExtension = ".pyo"
    else:
        targetFileExtension = ".pyc"
    for i in fileList:
        base,ext = os.path.splitext(i)
        if (ext == ".py"):
            newFileList.append(i)
    return newFileList

def squeezePandaFiles():
    l = getSqueezeableFiles()
    pandaSqueezeTool.squeeze("PandaModules", "PandaModulesUnsqueezed", l)

    # Clean up the source files now that they've been squeezed.
    for i in l:
        os.unlink(i)

squeezePandaFiles()
