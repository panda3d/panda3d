


class SfxPlayer:
    """
    Play sound effects, potentially localized.
    """

    def __init__(self):
        # Distance at which sounds can no longer be heard
        # This was determined experimentally
        self.cutoffDistance = 120.0

    def getLocalizedVolume(self, node):
        """
        Get the volume that a sound should be played at if it is
        localized at this node. We compute this wrt the camera
        """
        import math
        d = node.getDistance(base.cam)
        if d > self.cutoffDistance:
            volume = 0
        else:
            volume = (1 - (d / self.cutoffDistance))
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
        
