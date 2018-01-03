"""This module serves as a container to hold the global ShowBase instance, as
an alternative to using the builtin scope.

Note that you cannot directly import `base` from this module since ShowBase
may not have been created yet; instead, ShowBase dynamically adds itself to
this module's scope when instantiated."""

__all__ = []

from .ShowBase import ShowBase, WindowControls
from direct.directnotify.DirectNotifyGlobal import directNotify, giveNotify
from panda3d.core import VirtualFileSystem, Notify, ClockObject, PandaSystem
from panda3d.core import ConfigPageManager, ConfigVariableManager
from panda3d.direct import get_config_showbase

config = get_config_showbase()

vfs = VirtualFileSystem.getGlobalPtr()
ostream = Notify.out()
globalClock = ClockObject.getGlobalClock()
cpMgr = ConfigPageManager.getGlobalPtr()
cvMgr = ConfigVariableManager.getGlobalPtr()
pandaSystem = PandaSystem.getGlobalPtr()

# Set direct notify categories now that we have config
directNotify.setDconfigLevels()

def inspect(anObject):
    # Don't use a regular import, to prevent ModuleFinder from picking
    # it up as a dependency when building a .p3d package.
    import importlib
    Inspector = importlib.import_module('direct.tkpanels.Inspector')
    return Inspector.inspect(anObject)

import sys
if sys.version_info >= (3, 0):
    import builtins
else:
    import __builtin__ as builtins
builtins.inspect = inspect

# this also appears in AIBaseGlobal
if (not __debug__) and __dev__:
    notify = directNotify.newCategory('ShowBaseGlobal')
    notify.error("You must set 'want-dev' to false in non-debug mode.")
    del notify
