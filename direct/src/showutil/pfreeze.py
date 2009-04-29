#! /usr/bin/env python

"""

This script can be used to produce a standalone executable from
arbitrary Python code.  You supply the name of the starting Python
file to import, and this script attempts to generate an executable
that will produce the same results as "python startfile.py".

This script is actually a wrapper around Panda's FreezeTool.py, which
is itself a tool to use Python's built-in "freeze" utility to compile
Python code into a standalone executable.  It also uses Python's
built-in modulefinder module, which it uses to find all of the modules
imported directly or indirectly by the original startfile.py.

Usage:

  pfreeze.py [opts] startfile

Options:

  -o output
     Specifies the name of the resulting executable file to produce.
     If this ends in ".mf", a multifile is written instead of a frozen
     binary.

  -x module[,module...]
     Specifies a comma-separated list of Python modules to exclude from
     the resulting file, even if they appear to be referenced.  You
     may also repeat the -x command for each module.

  -i module[,module...]
     Specifies a comma-separated list of Python modules to include in
     the resulting file, even if they do not appear to be referenced.
     You may also repeat the -i command for each module.

"""

import getopt
import sys
import os
from direct.showutil import FreezeTool

def usage(code, msg = ''):
    print >> sys.stderr, __doc__
    print >> sys.stderr, msg
    sys.exit(code)

if __name__ == '__main__':
    freezer = FreezeTool.Freezer()

    basename = None
    
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'o:i:x:h')
    except getopt.error, msg:
        usage(1, msg)

    for opt, arg in opts:
        if opt == '-o':
            basename = arg
        elif opt == '-i':
            for module in arg.split(','):
                freezer.addModule(module)
        elif opt == '-x':
            for module in arg.split(','):
                freezer.excludeModule(module)
        elif opt == '-h':
            usage(0)

    if not args:
        usage(0)

    if not basename:
        usage(1, 'You did not specify an output file.')

    if len(args) != 1:
        usage(1, 'Only one main file may be specified.')

    freezer.setMain(args[0])
    freezer.done()

    if basename.lower().endswith('.mf'):
        freezer.writeMultifile(basename)
    else:
        freezer.generateCode(basename)
