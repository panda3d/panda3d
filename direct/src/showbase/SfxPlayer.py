
import math

class SfxPlayer:
    """
    Play sound effects, potentially localized.
    """

    UseInverseSquare = 1

    def __init__(self):
        # Distance at which sounds can no longer be heard
        # This was determined experimentally
        self.cutoffDistance = 120.0

        # cutoff for inverse square attenuation
        if SfxPlayer.UseInverseSquare:
            self.cutoffDistance = 300.0
        # volume attenuates according to the inverse square of the
        # distance from the source. volume = 1/(distance^2)
        # this is the volume at which a sound is nearly inaudible
        self.cutoffVolume = .02
        # this is the 'raw' distance at which the volume of a sound will
        # be equal to the cutoff volume
        rawCutoffDistance = math.sqrt(1./self.cutoffVolume)
        # this is a scale factor to convert distances so that a sound
        # located at self.cutoffDistance will have a volume
        # of self.cutoffVolume
        self.distanceScale = rawCutoffDistance / self.cutoffDistance

    def getLocalizedVolume(self, node):
        """
        Get the volume that a sound should be played at if it is
        localized at this node. We compute this wrt the camera
        """
        d = None
        if not node.isEmpty():
            d = node.getDistance(base.cam)
        if d == None or d > self.cutoffDistance:
            volume = 0
        else:
            if SfxPlayer.UseInverseSquare:
                sd = d*self.distanceScale
                volume = min(1, 1 / (sd*sd or 1))
                #print d, sd, volume
            else:
                volume = 1 - (d / (self.cutoffDistance or 1))
                #print d, volume
        return volume

    def playSfx(self, sfx, looping = 0, interrupt = 1, volume = None, time = 0.0, node=None):
        if sfx:
            # If we have either a node or a volume, we need to adjust the sfx
            # The volume passed in multiplies the distance base volume
            if node or (volume is not None):
                if node:
                    finalVolume = self.getLocalizedVolume(node)
                else:
                    finalVolume = 1
                if volume is not None:
                    finalVolume *= volume
                sfx.setVolume(finalVolume)

            # dont start over if it's already playing, unless "interrupt" was specified
            if interrupt or (sfx.status() != AudioSound.PLAYING):
                sfx.setTime(time)
                sfx.setLoop(looping)
                sfx.play()
        
