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
        self.cleanedUp = 0

    def privInitialize(self, t):
        renderParent = None
        if self.worldRelative:
            renderParent = render
        self.particleEffect.start(self.parent, renderParent)
        self.state = CInterval.SStarted
        self.currT = t

    def privStep(self, t):
        if self.state == CInterval.SPaused:
            # Restarting from a pause.
            self.particleEffect.start(self.parent, renderParent)
        self.state = CInterval.SStarted
        self.currT = t

    def privFinalize(self):
        self.particleEffect.cleanup()
        self.cleanedUp = 1
        self.currT = self.getDuration()
        self.state = CInterval.SFinal

    def privInterrupt(self):
        self.particleEffect.cleanup()
        self.cleanedUp = 1
        self.state = CInterval.SPaused
