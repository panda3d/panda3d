"""ParticleInterval module: contains the ParticleInterval class"""

from PandaModules import *
import Interval

import ParticleEffect

class ParticleInterval(Interval.Interval):
    # Name counter
    particleNum = 1
    # create ParticleInterval DirectNotify category
    notify = directNotify.newCategory('ParticleInterval')
    # Class methods
    def __init__(self, particleEffect, parent, worldRelative=1, loop=0, 
                                                duration=0.0, name=None):
        """__init__(particleEffect, parent, worldRelative, loop, duration, name)
        """
        # Generate unique name
        id = 'Particle-%d' % ParticleInterval.particleNum
        ParticleInterval.particleNum += 1
        if (name == None):
            name = id
        # Record instance variables
        self.particleEffect = particleEffect 
        self.parent = parent
        self.worldRelative = worldRelative
        self.loop = loop
        assert(duration > 0.0 or loop == 1)
        # Initialize superclass
        Interval.Interval.__init__(self, name, duration)
        # Update stopEvent
        self.stopEvent = id + '_stopEvent'
        self.stopEventList = [self.stopEvent]
        self.cleanedUp = 0

    def updateFunc(self, t, event=Interval.IVAL_NONE):
        """ updateFunc(t, event)
        Go to time t
        """
        if (self.cleanedUp == 1):
            self.notify.warning('updateFunc() - already cleaned up!')
            return
        # Update particle effect based on current time
        if (t >= self.getDuration()):
            # If duration reached or stop event received, stop particle effect 
            self.particleEffect.cleanup()
            self.ignore(self.stopEvent)
            self.cleanedUp = 1
        elif (event == Interval.IVAL_INIT):
            # IVAL_INIT event, start new particle effect
            renderParent = None
            if self.worldRelative:
                renderParent = render
            self.particleEffect.start(self.parent, renderParent)
            # Accept event to kill particle effect 
            self.acceptOnce(self.stopEvent,
                        lambda s = self: 
                s.particleEffect.cleanup())
        # Print debug information
        assert(self.notify.debug('updateFunc() - %s: t = %f' % (self.name, t)))
            



