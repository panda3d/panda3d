#! /usr/bin/env python

"""

This tool will invoke the AppRunner to execute a packaged p3d
application.  It requires that that the current Panda3D and Python
versions match the version expected by the application.

Normally, you do not need to use this tool; instead, use the provided
standalone panda3d executable to invoke any p3d application.  Using
panda3d will guarantee that the correct versions of Panda3D and Python
are used to run the application.  However, there may be occasions when
it is useful to use this tool to run the application with the current
build instead of with its advertised version requirements.

Usage:

  runp3d.py app.p3d [args]

The command-line arguments following the application name are passed
into the application unchanged.

See pack3d.p3d for an application that generates these p3d files.

"""

import sys
import getopt
from .AppRunner import AppRunner, ArgumentError
from direct.task.TaskManagerGlobal import taskMgr
from panda3d.core import Filename

def parseSysArgs():
    """ Handles sys.argv, if there are any local arguments, and
    returns a new argv suitable for passing into the
    application. """

    # We prefix a "+" sign, following the GNU convention, to tell
    # getopt not to parse options following the first non-option
    # parameter.
    opts, args = getopt.getopt(sys.argv[1:], '+h')

    for option, value in opts:
        if option == '-h':
            print(__doc__)
            sys.exit(1)

    if not args or not args[0]:
        raise ArgumentError("No Panda app specified.  Use:\nrunp3d.py app.p3d")

    arg0 = args[0]
    p3dFilename = Filename.fromOsSpecific(arg0)
    if p3dFilename.exists():
        p3dFilename.makeAbsolute()
        arg0 = p3dFilename.toOsSpecific()

    return [arg0] + args[1:]

def runPackedApp(pathname):
    runner = AppRunner()
    runner.gotWindow = True
    try:
        runner.setP3DFilename(pathname, tokens = [], argv = [],
                              instanceId = 0, interactiveConsole = False)
    except ArgumentError as e:
        print(e.args[0])
        sys.exit(1)

if __name__ == '__main__':
    runner = AppRunner()
    runner.gotWindow = True
    try:
        argv = parseSysArgs()
        runner.setP3DFilename(argv[0], tokens = [], argv = argv,
                              instanceId = 0, interactiveConsole = False)
    except ArgumentError as e:
        print(e.args[0])
        sys.exit(1)
    taskMgr.run()
