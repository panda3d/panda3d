""" This module implements genPyCode, which is itself a generated
script with a few default parameters filled in.  This module allows
the user to specify alternate parameters on the command line. """

import getopt
import sys
import os
import glob
import types
import time
from direct.ffi import FFIConstants

# Define a help string for the user
helpString ="""
genPyCode -h
genPyCode
genPyCode [opts] -i libdtoolconfig libcode1 libcode2 ...

This script generates Python wrappers to interface with the C++
libraries that have already been run through interrogate.  It is
necessary to run this script after building the Panda tools for the
first time, or after any major change in which some of the interface
may have changed.

The default options are baked into genPyCode by ppremake and need not
be specified.  However, it is possible to override these on the
command line if you need to fine-tune the behavior of genPyCode for
some reason.  Most often, the only needed change will be to add one or
more additional libraries to the list of libraries instrumented by
default.


Options:
  -h          print this message
  -v          verbose
  -d          generate HTML documentation too
  -C dir      directory to write output code
  -H dir      directory to write output HTML
  -x dir      directory to pull extension code from
  -i lib      interrogate library
  -e dir      directory to search for *.in files (may be repeated)
  -p dir      directory to search for Python source files (may be repeated)
  -r          remove the default library list; instrument only named libraries
  -O          no C++ comments or assertion statements
  -n          Don't use squeezeTool to squeeze the result into one .pyz file
  -s          Don't delete source files after squeezing

Any additional names listed on the command line are taken to be names
of libraries that are to be instrumented.

"""

HTMLHeader = """
<html>
<head>
<title>Panda3D documentation generated %s</title>
</head>
<body>
"""

HTMLFooter = """
</body>
</html>
"""

# Initialize variables
outputCodeDir = ''
outputHTMLDir = ''
directDir = ''
extensionsDir = ''
interrogateLib = ''
codeLibs = []
etcPath = []
pythonSourcePath = []
doSqueeze = True
deleteSourceAfterSqueeze = True
doHTML = False
native = False  # This is set by genPyCode.py

def doGetopts():
    global outputCodeDir
    global outputHTMLDir
    global extensionsDir
    global interrogateLib
    global codeLibs
    global doSqueeze
    global deleteSourceAfterSqueeze
    global doHTML
    global etcPath
    global pythonSourcePath

    # These options are allowed but are flagged as warnings (they are
    # deprecated with the new genPyCode script):

    # -g adds libgateway
    # -t adds libtoontown
    # -p adds libpirates
    # -o adds libopt

    FFIConstants.notify.setDebug(0)
    FFIConstants.notify.setInfo(0)

    # Extract the args the user passed in
    try:
        opts, pargs = getopt.getopt(sys.argv[1:], 'hvdOC:H:x:Ni:e:p:rns')
    except Exception, e:
        # User passed in a bad option, print the error and the help, then exit
        print e
        print helpString
        sys.exit()

    # Store the option values into our variables
    for opt in opts:
        flag, value = opt
        if (flag == '-h'):
            print helpString
            sys.exit()
        elif (flag == '-v'):
            if not FFIConstants.notify.getInfo():
                FFIConstants.notify.setInfo(1)
            else:
                FFIConstants.notify.setDebug(1)
        elif (flag == '-d'):
            doHTML = True
        elif (flag == '-C'):
            outputCodeDir = value
        elif (flag == '-H'):
            outputHTMLDir = value
        elif (flag == '-x'):
            extensionsDir = value
        elif (flag == '-i'):
            interrogateLib = value
        elif (flag == '-e'):
            etcPath.append(value)
        elif (flag == '-p'):
            pythonSourcePath.append(value)
        elif (flag == '-r'):
            codeLibs = []
        elif (flag == '-O'):
            FFIConstants.wantComments = 0
            FFIConstants.wantTypeChecking = 0
        elif (flag == '-n'):
            doSqueeze = False
        elif (flag == '-s'):
            deleteSourceAfterSqueeze = False
            
        else:
            FFIConstants.notify.error('illegal option: ' + flag)

    # Check for old, no-longer-used parameter:
    invalidParameters = [
        'linux', 'win-debug', 'win-release', 'win-publish',
        'install', 'release'
        ]
    if pargs and pargs[0] in invalidParameters:
        FFIConstants.notify.warning("parameter is deprecated: %s" % (pargs[0]))
        del pargs[0]

    # Store the program arguments into the codeLibs
    for arg in pargs:
        arg = arg.strip()
        if arg:
            codeLibs.append(arg)

    # Make sure each name appears on codeLibs exactly once.
    newLibs = []
    for codeLib in codeLibs:
        if codeLib not in newLibs:
            newLibs.append(codeLib)
    codeLibs = newLibs
        

def doErrorCheck():
    global outputCodeDir
    global outputHTMLDir
    global extensionsDir
    global interrogateLib
    global codeLibs
    global doSqueeze
    global etcPath

    # Now do some error checking and verbose output
    if (not interrogateLib):
        FFIConstants.notify.error('You must specify an interrogate library (-i lib)')
    else:
        FFIConstants.notify.debug('Setting interrogate library to: ' + interrogateLib)
        FFIConstants.InterrogateModuleName = interrogateLib

    if (not outputCodeDir):
        FFIConstants.notify.info('Setting output code directory to current directory')
        outputCodeDir = '.'
    elif (not os.path.exists(outputCodeDir)):
        FFIConstants.notify.info('Directory does not exist, creating: ' + outputCodeDir)
        os.mkdir(outputCodeDir)
        FFIConstants.notify.info('Setting output code directory to: ' + outputCodeDir)
    else:
        FFIConstants.notify.info('Setting output code directory to: ' + outputCodeDir)

    if doHTML:
        if (not outputHTMLDir):
            FFIConstants.notify.info('Setting output HTML directory to current directory')
            outputHTMLDir = '.'
        elif (not os.path.exists(outputHTMLDir)):
            FFIConstants.notify.info('Directory does not exist, creating: ' + outputHTMLDir)
            os.makedirs(outputHTMLDir)
            FFIConstants.notify.info('Setting output HTML directory to: ' + outputHTMLDir)
        else:
            FFIConstants.notify.info('Setting output HTML directory to: ' + outputHTMLDir)


    if (not extensionsDir):
        FFIConstants.notify.debug('Setting extensions directory to current directory')
        extensionsDir = '.'
    elif (not os.path.exists(extensionsDir)):
        FFIConstants.notify.error('Directory does not exist: ' + extensionsDir)
    else:
        FFIConstants.notify.debug('Setting extensions directory to: ' + extensionsDir)


    if (not codeLibs):
        FFIConstants.notify.error('You must specify one or more libraries to generate code from')
    else:
        FFIConstants.notify.debug('Generating code for: ' + `codeLibs`)
        FFIConstants.CodeModuleNameList = codeLibs

def generateNativeWrappers():
    from direct.extensions_native.extension_native_helpers import Dtool_PreloadDLL

    # Empty out the output directories of unnecessary crud from
    # previous runs before we begin.
    for file in os.listdir(outputCodeDir):
        pathname = os.path.join(outputCodeDir, file)
        if not os.path.isdir(pathname):
            os.unlink(pathname)

    # Generate __init__.py
    initFilename = os.path.join(outputCodeDir, '__init__.py')
    init = open(initFilename, 'w')

    # Generate PandaModules.py
    pandaModulesFilename = os.path.join(outputCodeDir, 'PandaModules.py')
    pandaModules = open(pandaModulesFilename, 'w')

    # Copy in any helper classes from the extensions_native directory
    extensionHelperFiles = ['extension_native_helpers.py']
    for name in extensionHelperFiles:
        inFilename = os.path.join(extensionsDir, name)
        outFilename = os.path.join(outputCodeDir, name)
        if os.path.exists(inFilename):
            inFile = open(inFilename, 'r')
            outFile = open(outFilename, 'w')
            outFile.write(inFile.read())

    # Generate a series of "libpandaModules.py" etc. files, one for
    # each named module.
    for moduleName in FFIConstants.CodeModuleNameList:
        print 'Importing code library: ' + moduleName
        Dtool_PreloadDLL(moduleName)
        exec('import %s as module' % moduleName)

        pandaModules.write('from %sModules import *\n' % (moduleName))

        moduleModulesFilename = os.path.join(outputCodeDir, '%sModules.py' % (moduleName))
        moduleModules = open(moduleModulesFilename, 'w')

        moduleModules.write('from extension_native_helpers import *\n')
        moduleModules.write('Dtool_PreloadDLL("%s")\n' % (moduleName))
        moduleModules.write('from %s import *\n\n' % (moduleName))

        # Now look for extensions
        for className, classDef in module.__dict__.items():
            if type(classDef) == types.TypeType:
                extensionFilename = os.path.join(extensionsDir, '%s_extensions.py' % (className))
                if os.path.exists(extensionFilename):
                    print '  Found extensions for class: %s' % (className)
                    extension = open(extensionFilename, 'r')
                    moduleModules.write(extension.read())
                    moduleModules.write('\n')
        

def run():
    global outputCodeDir
    global outputHTMLDir
    global directDir
    global extensionsDir
    global interrogateLib
    global codeLibs
    global doSqueeze
    global deleteSourceAfterSqueeze
    global etcPath
    global pythonSourcePath

    doGetopts()
    doErrorCheck()

    # Ok, now we can start generating code
    if native:
        generateNativeWrappers()

    else:
        from direct.ffi import FFIInterrogateDatabase
        db = FFIInterrogateDatabase.FFIInterrogateDatabase(etcPath = etcPath)
        db.generateCode(outputCodeDir, extensionsDir)

        if doSqueeze:
            db.squeezeGeneratedCode(outputCodeDir, deleteSourceAfterSqueeze)

    if doHTML:
        from direct.directscripts import gendocs
        from pandac.PandaModules import PandaSystem
        versionString = '%s %s' % (
            PandaSystem.getDistributor(), PandaSystem.getVersionString())

        gendocs.generate(versionString, etcPath, pythonSourcePath,
                         outputHTMLDir, HTMLHeader % time.asctime(),
                         HTMLFooter, '', '.html')
