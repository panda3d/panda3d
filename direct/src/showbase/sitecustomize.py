"""This is a VR Studio custom script to make sure sys.path is set up
properly from the ctattach tools.  It's not needed if you aren't using
ctattach, and its use is even somewhat deprecated for the VR Studio."""

import os
import sys

def deCygwinify(path):
    if os.name in ['nt'] and path[0] == '/':
        # On Windows, we may need to convert from a Cygwin-style path
        # to a native Windows path.

        # Check for a case like /i/ or /p/: this converts
        # to I:/ or P:/.

        dirs = path.split('/')
        if len(dirs) > 2 and len(dirs[1]) == 1:
            path = '%s:\%s' % (dirs[1], '\\'.join(dirs[2:]))

        else:
            # Otherwise, prepend $PANDA_ROOT and flip the slashes.
            pandaRoot = os.getenv('PANDA_ROOT')
            if pandaRoot:
                path = os.path.normpath(pandaRoot + path)

    return path

if os.getenv('CTPROJS'):
    tree = os.getenv('DIRECT')
    if not tree:
        raise StandardError,'CTPROJS is defined, but you are not attached to DIRECT!'
    tree = deCygwinify(tree)
    parent, base = os.path.split(tree)
    if parent not in sys.path:
        sys.path.append(parent)
    
    import direct.showbase.FindCtaPaths
