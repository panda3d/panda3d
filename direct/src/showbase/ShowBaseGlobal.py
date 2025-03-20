"""This module serves as a container to hold the global
:class:`~.ShowBase.ShowBase` instance, as an alternative to using the builtin
scope.

Many of the variables contained in this module are also automatically written
to the :mod:`builtins` module when ShowBase is instantiated, making them
available to any Python code.  Importing them from this module instead can make
it easier to see where these variables are coming from.

Note that you cannot directly import :data:`~builtins.base` from this module
since ShowBase may not have been created yet; instead, ShowBase dynamically
adds itself to this module's scope when instantiated."""

__all__ = ()

from .ShowBase import ShowBase, WindowControls # pylint: disable=unused-import
from direct.directnotify.DirectNotifyGlobal import directNotify, giveNotify # pylint: disable=unused-import
from panda3d.core import VirtualFileSystem, Notify, ClockObject, PandaSystem
from panda3d.core import ConfigPageManager, ConfigVariableManager, ConfigVariableBool
from panda3d.core import NodePath, PGTop
from . import DConfig as config # pylint: disable=unused-import
from .Loader import Loader
import warnings

__dev__: bool = ConfigVariableBool('want-dev', __debug__).value

base: ShowBase

#: The global instance of the :ref:`virtual-file-system`, as obtained using
#: :meth:`panda3d.core.VirtualFileSystem.getGlobalPtr()`.
vfs = VirtualFileSystem.getGlobalPtr()

#: The default Panda3D output stream for notifications and logging, as
#: obtained using :meth:`panda3d.core.Notify.out()`.
ostream = Notify.out()

#: The clock object used by default for rendering and animation, obtained using
#: :meth:`panda3d.core.ClockObject.getGlobalClock()`.
#: @deprecated Use `base.clock` instead.
globalClock = ClockObject.getGlobalClock()

#: See :meth:`panda3d.core.ConfigPageManager.getGlobalPtr()`.
cpMgr = ConfigPageManager.getGlobalPtr()

#: See :meth:`panda3d.core.ConfigVariableManager.getGlobalPtr()`.
cvMgr = ConfigVariableManager.getGlobalPtr()

#: See :meth:`panda3d.core.PandaSystem.getGlobalPtr()`.
pandaSystem = PandaSystem.getGlobalPtr()

#: The root of the 2-D scene graph.  The coordinate system of this node runs
#: from -1 to 1, with the X axis running from left to right and the Z axis from
#: bottom to top.
render2d = NodePath("render2d")

#: The root of the 2-D scene graph used for GUI rendering.  Unlike render2d,
#: which may result in elements being stretched in windows that do not have a
#: square aspect ratio, this node is scaled automatically to ensure that nodes
#: parented to it do not appear stretched.
aspect2d = render2d.attachNewNode(PGTop("aspect2d"))

#: A dummy scene graph that is not being rendered by anything.
hidden = NodePath("hidden")

#: The global Loader instance for models, textures, etc.
loader = Loader()

# Set direct notify categories now that we have config
directNotify.setDconfigLevels()


def run():
    """Deprecated alias for :meth:`base.run() <.ShowBase.run>`."""
    if __debug__:
        warnings.warn("run() is deprecated, use base.run() instead", DeprecationWarning, stacklevel=2)
    base.run()


def inspect(anObject):
    """Opens up a :mod:`direct.tkpanels.Inspector` GUI panel for inspecting an
    object."""
    # Don't use a regular import, to prevent ModuleFinder from picking
    # it up as a dependency when building a .p3d package.
    import importlib
    Inspector = importlib.import_module('direct.tkpanels.Inspector')
    return Inspector.inspect(anObject)


import builtins
builtins.inspect = inspect  # type: ignore[attr-defined]

# this also appears in AIBaseGlobal
if (not __debug__) and __dev__:
    ShowBase.notify.error("You must set 'want-dev' to false in non-debug mode.")
