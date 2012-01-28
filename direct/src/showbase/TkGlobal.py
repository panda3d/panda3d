""" This module is now vestigial.  """

import sys, Pmw

# This is required by the ihooks.py module used by Squeeze (used by
# pandaSqueezer.py) so that Pmw initializes properly
if '_Pmw' in sys.modules:
    sys.modules['_Pmw'].__name__ = '_Pmw'

def spawnWxLoop():
    base.spawnWxLoop()
