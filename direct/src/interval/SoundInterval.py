"""SoundInterval module: contains the SoundInterval class"""

from PandaModules import *
from Interval import *

class SoundInterval(Interval):
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
    def __init__(self, sound, loop = 0, duration = 0.0, name = None, volume = 1.0):
        """__init__(sound, loop, name)
        """
        # Generate unique name
        id = 'Sound-%d' % SoundInterval.soundNum
        SoundInterval.soundNum += 1
        # Record instance variables
        self.sound = sound
        self.loop = loop
        self.volume = volume
        # If no duration given use sound's duration as interval's duration
        if duration == 0.0 and self.sound != None:
            duration = self.sound.length()
            if (duration == 0):
                self.notify.warning('zero length duration!')
            # MPG - hack for Miles bug
            #duration += 1.5
            # DCR - hack for Miles bug - adding 1.5 seconds caused
            # problems for MG_neg_buzzer.wav
            duration += min(duration * 2.4, 1.5)
        # Generate unique name if necessary
        if (name == None):
            name = id
        # Initialize superclass
        Interval.__init__(self, name, duration)
        # Update stopEvent
        self.stopEvent = id + '_stopEvent'
        self.stopEventList = [self.stopEvent]

    def updateFunc(self, t, event = IVAL_NONE):
        """ updateFunc(t, event)
        Go to time t
        """
        # Update sound based on current time
        if (t >= self.getDuration()):
            # If end of sound reached or stop event received, stop sound
            if self.sound:
                self.sound.stop()
            self.ignore(self.stopEvent)
        elif (event == IVAL_INIT):
            # IVAL_INIT event, start new sound
            # If its within a 10th of a second of the start,
            # start at the beginning
            if (t < 0.1):
                t = 0.0
            if self.sound:
                # Start sound
                self.sound.setTime(t)
                self.sound.setLoop(self.loop)
                self.sound.setVolume(self.volume)
                self.sound.play()
                # Accept event to kill sound
                self.acceptOnce(self.stopEvent,
                        lambda s = self: s.sound.stop())
        # Print debug information
        self.notify.debug('updateFunc() - %s: t = %f' % (self.name, t))
            



