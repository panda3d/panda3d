"""
Contains the ParticleInterval class
"""

from pandac.PandaModules import *
from direct.directnotify.DirectNotifyGlobal import directNotify
from Interval import Interval

from direct.particles import ParticleEffect

class ParticleInterval(Interval):
    # Name counter
    particleNum = 1
    # create ParticleInterval DirectNotify category
    notify = directNotify.newCategory('ParticleInterval')
    # Class methods
    def __init__(self,
                 particleEffect,
                 parent,
                 worldRelative=1,
                 renderParent = None,
                 duration=0.0,
                 softStopT = 0.0,
                 cleanup = False,
                 name=None):
        """
        particleEffect is a ParticleEffect
        parent is a NodePath
        worldRelative is a boolean
        renderParent is a NodePath
        duration is a float for the time
        softStopT is a float
        cleanup is a boolean
        name is a string
        """
        
        # Generate unique name
        id = 'Particle-%d' % ParticleInterval.particleNum
        ParticleInterval.particleNum += 1
        if name == None:
            name = id
        # Record instance variables
        self.particleEffect = particleEffect 
        self.softStopT = softStopT
        self.cleanup = cleanup
        
        if parent != None:
            self.particleEffect.reparentTo(parent)
        if worldRelative:
            self.particleEffect.setRenderParent(render.node())
        
        # Initialize superclass
        Interval.__init__(self, name, duration)

    def __step(self,dt):
        if self.particleEffect:
            self.particleEffect.accelerate(dt,1,0.05)

    def start(self, *args, **kwargs):
        self.state = CInterval.SInitial
        if self.particleEffect:
            self.particleEffect.softStart()
        Interval.start(self, *args, **kwargs)        

    def privInitialize(self, t):
        if self.state == CInterval.SInitial:
            if self.particleEffect:
                self.particleEffect.clearToInitial()
            self.currT = 0

        if self.particleEffect:
            for f in self.particleEffect.forceGroupDict.values():
                f.enable()

        self.state = CInterval.SStarted        
        self.__step(t-self.currT)
        self.currT = t

    def privStep(self, t):
        if self.state == CInterval.SPaused:
            # Restarting from a pause.
            self.privInitialize(t)
        else:
            self.state = CInterval.SStarted
            self.__step(t-self.currT)
            self.currT = t

        if self.currT > (self.getDuration() - self.softStopT):
            if self.particleEffect:
                self.particleEffect.softStop()

    def privFinalize(self):
        Interval.privFinalize(self)
        if self.cleanup and self.particleEffect:
            self.particleEffect.cleanup()
            self.particleEffect = None
            
