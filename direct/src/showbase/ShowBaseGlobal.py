"""This module serves as a container to hold the global
:class:`~.ShowBase.ShowBase` instance, as an alternative to using the builtin
scope.

Note that you cannot directly import `base` from this module since ShowBase
may not have been created yet; instead, ShowBase dynamically adds itself to
this module's scope when instantiated."""

__all__ = []

from .ShowBase import ShowBase, WindowControls
from direct.directnotify.DirectNotifyGlobal import directNotify, giveNotify
from panda3d.core import VirtualFileSystem, Notify, ClockObject, PandaSystem
from panda3d.core import ConfigPageManager, ConfigVariableManager
from panda3d.core import NodePath, PGTop
from . import DConfig as config

__dev__ = config.GetBool('want-dev', __debug__)

#: The global instance of the :class:`panda3d.core.VirtualFileSystem`.
vfs = VirtualFileSystem.getGlobalPtr()
ostream = Notify.out()
globalClock = ClockObject.getGlobalClock()
cpMgr = ConfigPageManager.getGlobalPtr()
cvMgr = ConfigVariableManager.getGlobalPtr()
pandaSystem = PandaSystem.getGlobalPtr()

# This is defined here so GUI elements can be instantiated before ShowBase.
render2d = NodePath("render2d")
aspect2d = render2d.attachNewNode(PGTop("aspect2d"))
hidden = NodePath("hidden")

# Set direct notify categories now that we have config
directNotify.setDconfigLevels()


def run():
    """Deprecated alias for :meth:`base.run() <.ShowBase.run>`."""
    assert ShowBase.notify.warning("run() is deprecated, use base.run() instead")
    base.run()


def inspect(anObject):
    """Opens up a :mod:`direct.tkpanels.Inspector` GUI panel for inspecting an
    object."""
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
    ShowBase.notify.error("You must set 'want-dev' to false in non-debug mode.")
