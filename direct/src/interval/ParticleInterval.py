"""ParticleInterval module: contains the ParticleInterval class"""

from pandac.PandaModules import *
from direct.directnotify.DirectNotifyGlobal import *
import Interval

from direct.particles import ParticleEffect

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
        self.fLoop = loop
        assert(duration > 0.0 or loop == 1)
        # Initialize superclass
        Interval.Interval.__init__(self, name, duration)

    def __del__(self):
        if self.particleEffect:
            self.particleEffect.cleanup()
            self.particleEffect = None

    def privInitialize(self, t):
        renderParent = None
        if self.worldRelative:
            renderParent = render
        if self.particleEffect:
            self.particleEffect.start(self.parent, renderParent)
        self.state = CInterval.SStarted
        self.currT = t

    def privStep(self, t):
        if self.state == CInterval.SPaused:
            # Restarting from a pause.
            self.privInitialize(t)
        else:
            self.state = CInterval.SStarted
            self.currT = t

    def privFinalize(self):
        if self.particleEffect:
            self.particleEffect.cleanup()
            self.particleEffect = None
        self.currT = self.getDuration()
        self.state = CInterval.SFinal

    def privInstant(self):
        if self.particleEffect:
            self.particleEffect.cleanup()
            self.particleEffect = None
        self.currT = self.getDuration()
        self.state = CInterval.SFinal

    def privInterrupt(self):
        self.particleEffect.disable()
        self.state = CInterval.SPaused
