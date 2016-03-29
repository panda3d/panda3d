"""AnimControlInterval module: contains the AnimControlInterval class"""

__all__ = ['AnimControlInterval']

from panda3d.core import *
from panda3d.direct import *
from direct.directnotify.DirectNotifyGlobal import *
from . import Interval
import math

class AnimControlInterval(Interval.Interval):

    # create AnimControlInterval DirectNotify category
    notify = directNotify.newCategory('AnimControlInterval')

    # Name counter
    animNum = 1
    # Class methods

    # Plays an animation.  The subrange of the animation
    # to be played may be specified via frames (startFrame up to and
    # including endFrame) or seconds (startTime up to and including
    # endTime).  If neither is specified, the default is the entire
    # range of the animation.

    # this class requires either an AnimControl, or an AnimControlCollection
    # (in which case, each anim control must be the same length)

    # The duration may be implicit or explicit.  If it is omitted, it
    # is taken to be endTime - startTime.  There's not much point in
    # specifying otherwise unless you also specify loop=1, which will
    # loop the animation over its frame range during the duration of
    # the interval.

    # Note: if loop == 0 and duration > anim duration then the
    # animation will play once and then hold its final pose for the
    # remainder of the interval.

    # loop = 1 implies a loop within the entire range of animation,
    # while constrainedLoop = 1 implies a loop within startFrame and
    # endFrame only.

    def __init__(self, controls, loop=0, constrainedLoop=0,
                 duration=None, startTime=None, endTime=None,
                 startFrame=None, endFrame=None,
                 playRate=1.0, name=None):
        # Generate unique id
        id = 'AnimControl-%d' % (AnimControlInterval.animNum)
        AnimControlInterval.animNum += 1
        # Record class specific variables

        if(isinstance(controls, AnimControlCollection)):
            self.controls = controls
            if(config.GetBool("strict-anim-ival",0)):
                checkSz = self.controls.getAnim(0).getNumFrames()
                for i in range(1,self.controls.getNumAnims()):
                    if(checkSz != self.controls.getAnim(i).getNumFrames()):
                        self.notify.error("anim controls don't have the same number of frames!")
        elif(isinstance(controls, AnimControl)):
            self.controls = AnimControlCollection()
            self.controls.storeAnim(controls,"")
        else:
            self.notify.error("invalid input control(s) for AnimControlInterval")

        self.loopAnim = loop
        self.constrainedLoop = constrainedLoop
        self.playRate = playRate

        # If no name specified, use id as name
        if (name == None):
            name = id

        self.frameRate = self.controls.getAnim(0).getFrameRate() * abs(playRate)
        # Compute start and end frames.
        if startFrame != None:
            self.startFrame = startFrame
        elif startTime != None:
            self.startFrame = startTime * self.frameRate
        else:
            self.startFrame = 0

        if endFrame != None:
            self.endFrame = endFrame
        elif endTime != None:
            self.endFrame = endTime * self.frameRate
        elif duration != None:
            if startTime == None:
                startTime = float(self.startFrame) / float(self.frameRate)
            endTime = startTime + duration
            self.endFrame = duration * self.frameRate
        else:
            # No end frame specified.  Choose the maximum of all
            # of the controls' numbers of frames.
            numFrames = self.controls.getAnim(0).getNumFrames()
            self.endFrame = numFrames - 1

        # Must we play the animation backwards?  We play backwards if
        # either (or both) of the following is true: the playRate is
        # negative, or endFrame is before startFrame.
        self.reverse = (playRate < 0)
        if self.endFrame < self.startFrame:
            self.reverse = 1
            t = self.endFrame
            self.endFrame = self.startFrame
            self.startFrame = t

        self.numFrames = self.endFrame - self.startFrame + 1

        # Compute duration if no duration specified
        self.implicitDuration = 0
        if duration == None:
            self.implicitDuration = 1
            duration = float(self.numFrames) / self.frameRate

        # Initialize superclass
        Interval.Interval.__init__(self, name, duration)

    def getCurrentFrame(self):
        """Calculate the current frame playing in this interval.

        returns a float value between startFrame and endFrame, inclusive
        returns None if there are any problems
        """
        retval = None
        if not self.isStopped():
            framesPlayed = self.numFrames * self.currT
            retval = self.startFrame + framesPlayed
        return retval

    def privStep(self, t):
        frameCount = t * self.frameRate
        if self.constrainedLoop:
            frameCount = frameCount % self.numFrames

        if self.reverse:
            absFrame = self.endFrame - frameCount
        else:
            absFrame = self.startFrame + frameCount

        # Calc integer frame number
        intFrame = int(math.floor(absFrame + 0.0001))

        # Pose anim

        # We use our pre-computed list of animControls for
        # efficiency's sake, rather than going through the relatively
        # expensive Actor interface every frame.

        # Each animControl might have a different number of frames.
        numFrames = self.controls.getAnim(0).getNumFrames()
        if self.loopAnim:
            frame = (intFrame % numFrames) + (absFrame - intFrame)
        else:
            frame = max(min(absFrame, numFrames - 1), 0)

        self.controls.poseAll(frame)

        self.state = CInterval.SStarted
        self.currT = t

    def privFinalize(self):
        if self.implicitDuration and not self.loopAnim:
            # As a special case, we ensure we end up posed to the last
            # frame of the animation if the original duration was
            # implicit.  This is necessary only to guard against
            # possible roundoff error in computing the final frame
            # from the duration.  We don't do this in the case of a
            # looping animation, however, because this would introduce
            # a hitch in the animation when it plays back-to-back with
            # the next cycle.
            if self.reverse:
                self.controls.poseAll(self.startFrame)
            else:
                self.controls.poseAll(self.endFrame)

        else:
            # Otherwise, the user-specified duration determines which
            # is our final frame.
            self.privStep(self.getDuration())

        self.state = CInterval.SFinal
        self.intervalDone()

