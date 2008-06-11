"""SoundInterval module: contains the SoundInterval class"""

__all__ = ['SoundInterval']

from pandac.PandaModules import *
from direct.directnotify.DirectNotifyGlobal import *
import Interval
import random

class SoundInterval(Interval.Interval):
    # Name counter
    soundNum = 1
    # create SoundInterval DirectNotify category
    notify = directNotify.newCategory('SoundInterval')
    # Class methods
    # Create a sound interval
    # If loop = 0, sound will play once, duration of the interval
    # equals the duration of the sound
    # If loop = 1, the sound will loop for the specified duration
    # If no duration is specified, sound will loop for the duration
    # of the sound, i.e. it will only play once.....usually, there
    # seems to be some timing in the audio such that the stop doesn't
    # kill the looping sound until the next time around if duration
    # of the interval equals duration of the sound
    # seamlessloop will let the audio system loop the sound rather
    # than explicitly restarting the sound every time around. This
    # prevents a skip in the sound at every repetition (the gap in
    # the sound is caused by the delay between the end of the sound
    # and the next taskMgr cycle). There still seems to be a skip
    # in Miles when looping MP3s. =(
    # RAU 03/01/07 add listenerNode in case we dont want to
    # use base.camera as the listener, node must not be None
    def __init__(self, sound, loop = 0, duration = 0.0, name = None,
                 volume = 1.0, startTime = 0.0, node=None,
                 seamlessLoop=True, listenerNode = None, cutOff = None):
        """__init__(sound, loop, name)
        """
        # Generate unique name
        id = 'Sound-%d' % SoundInterval.soundNum
        SoundInterval.soundNum += 1
        # Record instance variables
        self.sound = sound
        if sound:
            self.soundDuration = sound.length()
        else:
            self.soundDuration = 0
        self.fLoop = loop
        self.volume = volume
        self.startTime = startTime
        self.node = node
        self.listenerNode = listenerNode
        self.cutOff = cutOff
        self._seamlessLoop = seamlessLoop
        if self._seamlessLoop:
            self._fLoop = True
        self._soundPlaying = False
        # If no duration given use sound's duration as interval's duration
        if float(duration) == 0.0 and self.sound != None:
            duration = max(self.soundDuration - self.startTime, 0)
            #if (duration == 0):
            #    self.notify.warning('zero length duration!')
            
            # MPG - hack for Miles bug
            #duration += 1.5
            
            # DCR - hack for Miles bug - adding 1.5 seconds caused
            # problems for MG_neg_buzzer.wav
            
            # DCR - what this is all about: Miles is under-reporting the
            # length of MP3 files, and they're getting cut off too early.
            # This is a temporary hack. We could:
            # - hack Miles to fix its MP3 length calculation
            # - complain louder about this to RAD
            # - precompute MP3 durations and store them in a table

            # drose - ok, I've put in a lower-level workaround in the
            # MilesAudioManager.  This is no longer necessary up here,
            # where it pollutes SoundInterval for everyone.
            #duration += min(duration * 2.4, 1.5)
            
        # Generate unique name if necessary
        if (name == None):
            name = id
        # Initialize superclass
        Interval.Interval.__init__(self, name, duration)

    def privInitialize(self, t):
        # If it's within a 10th of a second of the start,
        # start at the beginning
        t1 = t + self.startTime
        if (t1 < 0.1):
            t1 = 0.0
        if (t1 < self.soundDuration) and not (self._seamlessLoop and self._soundPlaying):
            base.sfxPlayer.playSfx(
                self.sound, self.fLoop, 1, self.volume, t1, self.node,
                listenerNode = self.listenerNode, cutoff = self.cutOff)
            self._soundPlaying = True
        self.state = CInterval.SStarted
        self.currT = t

    def privStep(self, t):
        if self.state == CInterval.SPaused:
            # Restarting from a pause.
            t1 = t + self.startTime
            if t1 < self.soundDuration:
                base.sfxPlayer.playSfx(
                    self.sound, self.fLoop, 1, self.volume, t1, self.node,
                    listenerNode = self.listenerNode)
        if self.listenerNode and not self.listenerNode.isEmpty() and \
           self.node and not self.node.isEmpty():
            base.sfxPlayer.setFinalVolume(self.sound, self.node, self.volume,
                                          self.listenerNode, self.cutOff)
        
        self.state = CInterval.SStarted
        self.currT = t

    def finish(self, *args, **kArgs):
        self._inFinish = True
        Interval.Interval.finish(self, *args, **kArgs)
        del self._inFinish

    def privFinalize(self):
        # if we're just coming to the end of a seamlessloop, leave the sound alone,
        # let the audio subsystem loop it
        if (self._seamlessLoop and self._soundPlaying and self.getLoop()
            and not hasattr(self, '_inFinish')):
            base.sfxPlayer.setFinalVolume(self.sound, self.node, self.volume,
                                          self.listenerNode, self.cutOff)
            return
        elif self.sound != None:
            self.sound.stop()
            self._soundPlaying = False
        self.currT = self.getDuration()
        self.state = CInterval.SFinal

    def privInterrupt(self):
        if self.sound != None:
            self.sound.stop()
            self._soundPlaying = False
        self.state = CInterval.SPaused

    def loop(self, startT = 0.0, endT = -1.0, playRate = 1.0, stagger=False):
        self.fLoop = 1
        Interval.Interval.loop(self, startT, endT, playRate)
        if stagger:
            self.setT(random.random() * self.getDuration())
