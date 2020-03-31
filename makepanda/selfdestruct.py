#!/usr/bin/env python2

import os
import re
import sys
import shutil

# Ensure user WANTS th
if len(sys.argv) < 2 or sys.argv[1] != '--yes':
    print('======== MAKEPANDA SELF-DESTRUCT ========')
    print('This script will destroy every trace of')
    print('makepanda from the Panda3D directory that')
    print('contains it. This is for testing CMake\'s')
    print('dependencies and fully removing makepanda')
    print('should the latter buildsystem no longer')
    print('be desired.')
    print('')
    print('If you are sure, pass --yes')
    sys.exit(1)

# Some sanity-checks to make sure this script is in the right location:
scriptdir = os.path.abspath(os.path.dirname(__file__))
assert os.path.split(scriptdir)[1] == 'makepanda'
root = os.path.dirname(scriptdir)
assert os.path.isfile(os.path.join(root, 'LICENSE'))
assert os.path.isdir(os.path.join(root, 'pandatool'))

# Now we get to work! First, the makepanda directory isn't needed:
shutil.rmtree(os.path.join(root, 'makepanda'))

# Then we look under each of the separate project trees:
projects = ['contrib', 'direct', 'dtool', 'panda', 'pandatool']
for project in projects:
    # Remove non-CMakeLists.txt files from */metalibs/*/
    for path, dirs, files in os.walk(os.path.join(root, project, 'metalibs')):
        for filename in files:
            if filename.lower() != 'cmakelists.txt':
                os.unlink(os.path.join(path, filename))
        # Unlink the directory itself, if empty
        if not os.listdir(path):
            os.rmdir(path)

    for path, dirs, files in os.walk(os.path.join(root, project)):
        # Get rid of _composite#.cxx files
        for filename in files:
            if re.match(r'.*_composite[0-9]*\.(cxx|h|mm)', filename):
                os.unlink(os.path.join(path, filename))
