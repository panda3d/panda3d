"""Contains the SfxPlayer class, a thin utility class for playing sounds at
a particular location."""

__all__ = ['SfxPlayer']

import math
from panda3d.core import *

class SfxPlayer:
    """
    Play sound effects, potentially localized.
    """

    UseInverseSquare = 0

    def __init__(self):
        # volume attenuates according to the inverse square of the
        # distance from the source. volume = 1/(distance^2)
        # this is the volume at which a sound is nearly inaudible
        self.cutoffVolume = .02

        # cutoff for inverse square attenuation
        if SfxPlayer.UseInverseSquare:
            self.setCutoffDistance(300.0)
        else:
            # Distance at which sounds can no longer be heard
            # This was determined experimentally
            self.setCutoffDistance(120.0)

    def setCutoffDistance(self, d):
        self.cutoffDistance = d
        # this is the 'raw' distance at which the volume of a sound will
        # be equal to the cutoff volume
        rawCutoffDistance = math.sqrt(1./self.cutoffVolume)
        # this is a scale factor to convert distances so that a sound
        # located at self.cutoffDistance will have a volume
        # of self.cutoffVolume
        self.distanceScale = rawCutoffDistance / self.cutoffDistance

    def getCutoffDistance(self):
        """Return the curent cutoff distance."""
        return self.cutoffDistance

    def getLocalizedVolume(self, node, listenerNode = None, cutoff = None):
        """
        Get the volume that a sound should be played at if it is
        localized at this node. We compute this wrt the camera
        or to listenerNode.
        """
        d = None
        if not node.isEmpty():
            if listenerNode and not listenerNode.isEmpty():
                d = node.getDistance(listenerNode)
            else:
                d = node.getDistance(base.cam)
        if not cutoff:
            cutoff = self.cutoffDistance
        if d == None or d > cutoff:
            volume = 0
        else:
            if SfxPlayer.UseInverseSquare:
                sd = d*self.distanceScale
                volume = min(1, 1 / (sd*sd or 1))
                #print d, sd, volume
            else:
                volume = 1 - (d / (cutoff or 1))
                #print d, volume

        return volume

    def playSfx(
            self, sfx, looping = 0, interrupt = 1, volume = None,
            time = 0.0, node=None, listenerNode = None, cutoff = None):
        if sfx:
            self.setFinalVolume(sfx, node, volume, listenerNode, cutoff)

            # don't start over if it's already playing, unless
            # "interrupt" was specified
            if interrupt or (sfx.status() != AudioSound.PLAYING):
                sfx.setTime(time)
                sfx.setLoop(looping)
                sfx.play()

    def setFinalVolume(self, sfx, node, volume, listenerNode, cutoff = None):
        """Calculate the final volume based on all contributed factors."""
        # If we have either a node or a volume, we need to adjust the sfx
        # The volume passed in multiplies the distance base volume
        if node or (volume is not None):
            if node:
                finalVolume = self.getLocalizedVolume(node, listenerNode, cutoff)
            else:
                finalVolume = 1
            if volume is not None:
                finalVolume *= volume
            if node is not None:
                finalVolume *= node.getNetAudioVolume()
            sfx.setVolume(finalVolume)
