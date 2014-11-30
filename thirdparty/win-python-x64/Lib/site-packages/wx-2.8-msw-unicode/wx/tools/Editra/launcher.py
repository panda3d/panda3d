#!/usr/bin/env python

# This module provides an importable module that can do the same
# things as the Editra script, namely, set up the sys.path and
# sys.modules for Editra and then start Editra running.  This is done
# by using execfile() to execute the code in the Editra script as if
# it was in this module, and then the importer of this module can call
# the main() function defined there.

import os
launcher = os.path.join(os.path.dirname(__file__), 'Editra')
execfile(launcher)

