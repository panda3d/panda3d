"""instantiate global ShowBase object"""

__all__ = []

from .ShowBase import *

# Create the showbase instance
# This should be created by the game specific "start" file
#ShowBase()
# Instead of creating a show base, assert that one has already been created
assert base

# Set direct notify categories now that we have config
directNotify.setDconfigLevels()

def inspect(anObject):
    # Don't use a regular import, to prevent ModuleFinder from picking
    # it up as a dependency when building a .p3d package.
    import importlib
    Inspector = importlib.import_module('direct.tkpanels.Inspector')
    return Inspector.inspect(anObject)

if sys.version_info >= (3, 0):
    import builtins
else:
    import __builtin__ as builtins
builtins.inspect = inspect

# this also appears in AIBaseGlobal
if (not __debug__) and __dev__:
    notify = directNotify.newCategory('ShowBaseGlobal')
    notify.error("You must set 'want-dev' to false in non-debug mode.")
