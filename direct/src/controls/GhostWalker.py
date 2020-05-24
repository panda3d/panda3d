"""
GhostWalker.py is for avatars.

A walker control such as this one provides:

- creation of the collision nodes
- handling the keyboard and mouse input for avatar movement
- moving the avatar

it does not:

- play sounds
- play animations

although it does send messages that allow a listener to play sounds or
animations based on walker events.
"""

from direct.directnotify import DirectNotifyGlobal
from . import NonPhysicsWalker

class GhostWalker(NonPhysicsWalker.NonPhysicsWalker):

    notify = DirectNotifyGlobal.directNotify.newCategory("GhostWalker")

    # Ghosts slide instead of jump:
    slideName = "jump"
