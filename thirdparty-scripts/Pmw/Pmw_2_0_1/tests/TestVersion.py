# Set the version of Pmw to use for the tests based on the directory
# name.

import imp
import os
import string
import Pmw

file = imp.find_module(__name__)[1]
if not os.path.isabs(file):
    file = os.path.join(os.getcwd(), file)
file = os.path.normpath(file)

dir = os.path.dirname(file)
dir = os.path.dirname(dir)
dir = os.path.basename(dir)

version = str.replace(dir[4:], '_', '.')
Pmw.setversion(version)
