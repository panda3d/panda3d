#!/usr/bin/env python

"""This script generates the panda3d-date.tar.gz tarball for a file
release of panda3d onto the SourceForge download site.

Options:

    -d cvsroot
        Specifies the CVSROOT string to use to tag and export the
        tree.  The default is $SFROOT if it is defined, or $CVSROOT
        otherwise.
 
    -r tag
        Specifies the tag to export from.  If this parameter is
        specified, the tree is not tagged again; otherwise, the
        current head of the CVS tree is tagged with the file version
        name.

    -m module
        Specifies the module to check out and build.  The default is 
        panda3d.
"""

import sys
import os
import os.path
import getopt
import time
import glob
import shutil

CVSROOT = os.getenv('SFROOT') or os.getenv('CVSROOT')
ORIGTAG = ''
MODULE  = 'panda3d'

def usage(code, msg = ''):
    print >> sys.stderr, __doc__
    print >> sys.stderr, msg
    sys.exit(code)

try:
    opts, args = getopt.getopt(sys.argv[1:], 'd:r:m:h')
except getopt.error, msg:
    usage(1, msg)

for opt, arg in opts:
    if opt == '-d':
        CVSROOT = arg
    elif opt == '-r':
        ORIGTAG = arg
    elif opt == '-m':
        MODULE = arg
    elif opt == '-h':
        usage(0)

if not CVSROOT:
    usage(1, 'CVSROOT must have a value.')

if not MODULE:
    usage(1, 'MODULE must have a value.')

basename = MODULE + '-' + time.strftime("%Y-%m-%d")
tarfile = basename + '.tar.gz'
zipfile = basename + '.zip'

if os.path.exists(basename):
    print basename, 'already exists in the local directory!'
    sys.exit(1)

if not ORIGTAG:
    # If we weren't given a starting tag, make one.
    tag = basename

    print 'Tagging sources.'
    cmd = 'cvs -f -d "%s" rtag -F -r HEAD "%s" "%s"' % (CVSROOT, tag, MODULE)
    os.system(cmd)
else:
    # Otherwise, we were given a starting tag, so use it.
    tag = ORIGTAG

print 'Checking out "%s" as "%s".' % (MODULE, basename)
cmd = 'cvs -z3 -f -d "%s" export -r "%s" -d "%s" "%s"' % (CVSROOT, tag,
                                                          basename, MODULE)
os.system(cmd)

# Move the contents of the doc module into the root directory where people
# will expect to see it.
docdir = basename + os.sep + 'doc'
if os.path.exists(docdir):
    files = glob.glob(docdir + os.sep + '*')
    for file in files:
        shutil.copy(file, basename)
        os.remove(file)
    os.rmdir(docdir)

# Generate the tarball.
cmd = 'tar cvzf "%s" "%s"' % (tarfile, basename)
os.system(cmd)

# Also generate a .zip file.
if os.path.exists(zipfile):
    os.remove(zipfile)
cmd = 'zip -9r "%s" "%s"' % (zipfile, basename)
os.system(cmd)

shutil.rmtree(basename)
