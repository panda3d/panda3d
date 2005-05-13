""" This module implements genPyCode, which is itself a generated
script with a few default parameters filled in.  This module allows
the user to specify alternate parameters on the command line. """

import getopt
import sys
import os
import glob
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
  -d dir      directory to write output code
  -m dir      directory to write HTML manual (optional)
  -x dir      directory to pull extension code from
  -i lib      interrogate library
  -e dir      directory to search for *.in files (may be repeated)
  -r          remove the default library list; instrument only named libraries
  -O          no C++ comments or assertion statements
  -n          Don't use squeezeTool to squeeze the result into one .pyz file
  -s          Don't delete source files after squeezing

Any additional names listed on the command line are taken to be names
of libraries that are to be instrumented.

"""

# Initialize variables
outputDir = ''
directDir = ''
manualDir = ''
extensionsDir = ''
interrogateLib = ''
codeLibs = []
etcPath = []
doSqueeze = True
deleteSourceAfterSqueeze = True

def doGetopts():
    global outputDir
    global extensionsDir
    global interrogateLib
    global codeLibs
    global doSqueeze
    global deleteSourceAfterSqueeze
    global etcPath

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
        opts, pargs = getopt.getopt(sys.argv[1:], 'hvOd:x:i:e:rnsgtpo')
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
            outputDir = value
        elif (flag == '-m'):
            manualDir = value
        elif (flag == '-x'):
            extensionsDir = value
        elif (flag == '-i'):
            interrogateLib = value
        elif (flag == '-e'):
            etcPath.append(value)
        elif (flag == '-r'):
            codeLibs = []
        elif (flag == '-O'):
            FFIConstants.wantComments = 0
            FFIConstants.wantTypeChecking = 0
        elif (flag == '-n'):
            doSqueeze = False
        elif (flag == '-s'):
            deleteSourceAfterSqueeze = False
        elif (flag in ['-g', '-t', '-p', '-o']):
            FFIConstants.notify.warning("option is deprecated: %s" % (flag))
            
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
    codeLibs += pargs

    # Make sure each name appears on codeLibs exactly once.
    newLibs = []
    for codeLib in codeLibs:
        if codeLib not in newLibs:
            newLibs.append(codeLib)
    codeLibs = newLibs
        

def doErrorCheck():
    global outputDir
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

    if (not outputDir):
        FFIConstants.notify.info('Setting output directory to current directory')
        outputDir = '.'
    elif (not os.path.exists(outputDir)):
        FFIConstants.notify.info('Directory does not exist, creating: ' + outputDir)
        os.mkdir(outputDir)
        FFIConstants.notify.info('Setting output directory to: ' + outputDir)
    else:
        FFIConstants.notify.info('Setting output directory to: ' + outputDir)


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


def run():
    global outputDir
    global directDir
    global manualDir
    global extensionsDir
    global interrogateLib
    global codeLibs
    global doSqueeze
    global etcPath

    doGetopts()
    doErrorCheck()

    # Ok, now we can start generating code
    from direct.ffi import FFIInterrogateDatabase
    db = FFIInterrogateDatabase.FFIInterrogateDatabase(etcPath = etcPath)
    db.generateCode(outputDir, extensionsDir)

    if manualDir != '' and directDir != '':
        import epydoc.cli
        import direct.directbase.DirectStart
        cmd = ["epydoc","-n","Panda3D","-o",manualDir,"--docformat","panda","--ignore-param-mismatch",outputDir,directDir]
        sys.argv = cmd
        epydoc.cli.cli()

    if doSqueeze:
        db.squeezeGeneratedCode(outputDir,deleteSourceAfterSqueeze)
