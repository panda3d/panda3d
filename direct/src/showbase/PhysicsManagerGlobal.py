"""PhysicsManagerGlobal module: contains the global physics manager"""

__all__ = ['physicsMgr']

from panda3d.physics import PhysicsManager

physicsMgr = PhysicsManager()
