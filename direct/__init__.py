"""This file defines the path to the Python files within this package.
There are two cases:

(1) This is a source tree being run interactively by a developer, in
    which case the Python files are found in package/src/*/*.py.  This
    case also breaks down into two sub-cases: (1a) we are using the
    ctattach tools, in which case we should look for the files in the
    actual source directory according to the ctattach variables, or
    (1b) we are not using the ctattach tools, in which case the files
    are right where we expect them to be.

(2) This is an installed tree being run by an end-user, in which case
    the Python files are found in package/*/*.py.  In this case, this
    file doesn't really need to be installed; an empty __init__.py
    file to define the package would serve just as well.  But the file
    is crafted so that it will do no harm if it is installed.
"""

package = 'DIRECT'

import os

if os.getenv('CTPROJS'):
    # Ok, this is case (1a): we are using the ctattach tools, are
    # therefore will expect to find the source files in
    # $(package)/src/*/*.py.  Completely replace the search path with
    # this path.
    tree = os.getenv(package)

    if not tree:
        raise StandardError, 'CTPROJS is defined, but $%s is not defined!' % (package)
    __path__[0] = os.path.join(tree, 'src')

else:
    # We are not using the ctattach tools.
    srcDir = os.path.join(__path__[0], 'src')
    if os.path.isdir(srcDir):
        # The source directory exists; therefore, we are in case (1b).
        __path__[0] = srcDir

    else:
        # The source directory does not exist, so we must be in case
        # (2).  Leave well enough alone.
        pass
    
