# This file is executed when the Pmw package is imported.  It creates
# a lazy importer/dynamic loader for Pmw and replaces the Pmw module
# with it.  Even though the loader from the most recent installed
# version of Pmw is used, the user is able to specify which version of
# Pmw megawidgets to load by using the setversion() function of the
# loader.

# This is the only file in Pmw which is not part of a particular Pmw
# release.

import sys
import os
import re

def _hasLoader(dir):
    # Only accept Pmw_V_R_P with single digits, since ordering will
    # not work correctly with multiple digits (for example, Pmw_10_0
    # will be before Pmw_9_9).
    if re.search('^Pmw_[0-9]_[0-9](_[0-9])?$', dir) is not None:
        for suffix in ('.py', '.pyc', '.pyo'):
            path = os.path.join(_dir, dir, 'lib', 'PmwLoader' + suffix)
            if os.path.isfile(path):
                return 1
    return 0

# First get a list of all subdirectories containing versions of Pmw.
_dir = __path__[0]
_listdir = os.listdir(_dir)
_instdirs = filter(_hasLoader, _listdir)
_instdirs.sort()
_instdirs.reverse()

# Using the latest version import the dynamic loader.
_loader = 'Pmw.' + _instdirs[0] + '.lib.PmwLoader'
__import__(_loader)
_mod = sys.modules[_loader]

# Create the dynamic loader and install it into sys.modules.
sys.modules['_Pmw'] = sys.modules['Pmw']
sys.modules['Pmw'] = _mod.PmwLoader(_dir, _instdirs, _listdir)
