"""
Contains the TestInterval class
"""

__all__ = ['TestInterval']

from panda3d.core import *
from panda3d.direct import *
from direct.directnotify.DirectNotifyGlobal import directNotify
from .Interval import Interval


class TestInterval(Interval):
    # Name counter
    particleNum = 1
    # create ParticleInterval DirectNotify category
    notify = directNotify.newCategory('TestInterval')
    # Class methods
    def __init__(self,
                 particleEffect,
                 duration=0.0,
                 parent = None,
                 renderParent = None,
                 name=None):
        """
        particleEffect is ??
        parent is ??
        worldRelative is a boolean
        loop is a boolean
        duration is a float for the time
        name is ??
        """
        # Generate unique name
        id = 'Particle-%d' % TestInterval.particleNum
        TestInterval.particleNum += 1
        if name == None:
            name = id
        # Record instance variables
        self.particleEffect = particleEffect
        self.parent = parent
        self.renderParent = renderParent

        Interval.__init__(self, name, duration)

    def __del__(self):
        pass

    def __step(self,dt):
        self.particleEffect.accelerate(dt,1,0.05)

    def start(self,*args,**kwargs):
        self.particleEffect.clearToInitial()
        self.currT = 0
        Interval.start(self,*args,**kwargs)

    def privInitialize(self, t):
        if self.parent != None:
            self.particleEffect.reparentTo(self.parent)
        if self.renderParent != None:
            self.setRenderParent(self.renderParent.node())

        self.state = CInterval.SStarted
        #self.particleEffect.enable()
        """
        if (self.particleEffect.renderParent != None):
            for p in self.particleEffect.particlesDict.values():
                p.setRenderParent(self.particleEffect.renderParent.node())
        """
        for f in self.particleEffect.forceGroupDict.values():
            f.enable()
        """
        for p in self.particleEffect.particlesDict.values():
            p.enable()
        self.particleEffect.fEnabled = 1
        """
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

    def privFinalize(self):
        self.__step(self.getDuration()-self.currT)
        self.currT = self.getDuration()

        self.state = CInterval.SFinal

    def privInstant(self):
        """
        Full jump from Initial state to Final State
        """
        self.__step(self.getDuration()-self.currT)
        self.currT = self.getDuration()

        self.state = CInterval.SFinal

    def privInterrupt(self):
        if not self.isStopped():
            self.state = CInterval.SPaused
