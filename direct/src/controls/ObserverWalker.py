"""
ObserverWalker.py is for avatars.

A walker control such as this one provides:
    - creation of the collision nodes
    - handling the keyboard and mouse input for avatar movement
    - moving the avatar

it does not:
    - play sounds
    - play animations

although it does send messeges that allow a listener to play sounds or
animations based on walker events.
"""

from direct.showbase.ShowBaseGlobal import *

from direct.directnotify import DirectNotifyGlobal
import NonPhysicsWalker

class ObserverWalker(NonPhysicsWalker.NonPhysicsWalker):
    notify = DirectNotifyGlobal.directNotify.newCategory("ObserverWalker")

    # Ghosts slide instead of jump:
    slideName = "jump"

    def initializeCollisions(self, collisionTraverser, avatarNodePath,
            avatarRadius = 1.4, floorOffset = 1.0, reach = 1.0):
        """
        Set up the avatar for collisions
        """
        NonPhysicsWalker.NonPhysicsWalker.initializeCollisions(
            self, collisionTraverser, avatarNodePath,
            avatarRadius, floorOffset, reach)
        self.cTrav.removeCollider(self.cRayNodePath)

    def enableAvatarControls(self):
        """
        Activate the arrow keys, etc.
        """
        assert(self.debugPrint("enableAvatarControls"))
        pass

    def disableAvatarControls(self):
        """
        Ignore the arrow keys, etc.
        """
        assert(self.debugPrint("disableAvatarControls"))
        pass
