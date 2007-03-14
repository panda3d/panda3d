"""instantiate global ShowBase object"""

__all__ = []

from ShowBase import *

# Create the showbase instance
# This should be created by the game specific "start" file
#ShowBase()
# Instead of creating a show base, assert that one has already been created
assert base

# Set direct notify categories now that we have config
directNotify.setDconfigLevels()

def inspect(anObject):
    from direct.tkpanels import Inspector
    return Inspector.inspect(anObject)

import __builtin__
__builtin__.inspect = inspect
# this also appears in AIBaseGlobal
if (not __debug__) and __dev__:
    notify = directNotify.newCategory('ShowBaseGlobal')
    notify.error("You must set 'want-dev' to false in non-debug mode.")
