"""Undocumented Module"""

__all__ = ['ParticleInterval']

"""
Contains the ParticleInterval class
"""

from pandac.PandaModules import *
from direct.directnotify.DirectNotifyGlobal import directNotify
from Interval import Interval

from direct.particles import ParticleEffect

class ParticleInterval(Interval):
    """
    Use this interval when you want to have greater control over a
    ParticleEffect.  The interval does not register the effect with
    the global particle and physics managers, but it does call upon
    them to perform its stepping.  You should NOT call
    particleEffect.start() with an effect that is being controlled
    by a ParticleInterval.
    """
    # Name counter
    particleNum = 1
    # create ParticleInterval DirectNotify category
    notify = directNotify.newCategory('ParticleInterval')
    # Class methods
    def __init__(self,
                 particleEffect,
                 parent,
                 worldRelative = 1,
                 renderParent = None,
                 duration = 0.0,
                 softStopT = 0.0,
                 cleanup = False,
                 name = None):
        """
        particleEffect is a ParticleEffect
        parent is a NodePath: this is where the effect will be
                              parented in the scenegraph
        worldRelative is a boolean: this will override 'renderParent'
                                    with render
        renderParent is a NodePath: this is where the particles will
                                    be rendered in the scenegraph
        duration is a float: for the time
        softStopT is a float: no effect if 0.0,
                              a positive value will count from the
                              start of the interval,
                              a negative value will count from the
                              end of the interval
        cleanup is a boolean: if True the effect will be destroyed
                              and removed from the scenegraph upon
                              interval completion
                              set to False if planning on reusing
                              the interval
        name is a string: use this for unique intervals so that
                          they can be easily found in the taskMgr
        """
        
        # Generate unique name
        id = 'Particle-%d' % ParticleInterval.particleNum
        ParticleInterval.particleNum += 1
        if name == None:
            name = id
        # Record instance variables
        self.particleEffect = particleEffect 
        self.cleanup = cleanup

        if parent != None:
            self.particleEffect.reparentTo(parent)
        if worldRelative:
            renderParent = render
        if renderParent:
            for particles in self.particleEffect.getParticlesList():
                particles.setRenderParent(renderParent.node())

        self.__softStopped = False
        
        if softStopT == 0.0:
            self.softStopT = duration
        elif softStopT < 0.0:
            self.softStopT = duration+softStopT
        else:
            self.softStopT = softStopT
            
        # Initialize superclass
        Interval.__init__(self, name, duration)

    def __step(self,dt):
        if self.particleEffect:
            self.particleEffect.accelerate(dt,1,0.05)

    def __softStart(self):
        if self.particleEffect:
            self.particleEffect.softStart()
        self.__softStopped = False

    def __softStop(self):
        if self.particleEffect:
            self.particleEffect.softStop()
        self.__softStopped = True

    def privInitialize(self, t):
        if self.state != CInterval.SPaused:
            # Restarting from a hard stop or just interrupting the
            # current play
            self.__softStart()
            if self.particleEffect:
                self.particleEffect.clearToInitial()
            self.currT = 0

        if self.particleEffect:
            for forceGroup in self.particleEffect.getForceGroupList():
                forceGroup.enable()

        Interval.privInitialize(self,t)

    def privInstant(self):
        self.privInitialize(self.getDuration())
        self.privFinalize()
        
    def privStep(self, t):
        if self.state == CInterval.SPaused or t < self.currT:
            # Restarting from a pause.
            self.privInitialize(t)
        else:
            if not self.__softStopped and t > self.softStopT:
                self.__step(self.softStopT-self.currT)
                self.__softStop()
                self.__step(t-self.softStopT)
            else:
                self.__step(t-self.currT)
            Interval.privStep(self,t)

    def privFinalize(self):
        Interval.privFinalize(self)
        if self.cleanup and self.particleEffect:
            self.particleEffect.disable()
            self.particleEffect = None
            
