from PandaObject import *
from DirectObject import *

import Particles
import Forces

class ParticleEffect(NodePath):
    def __init__(self, name = 'ParticleEffect'):
	"""__init__()"""
	NodePath.__init__(self)
        self.assign(hidden.attachNewNode(name))
        # Record particle effect name
	self.name = name
        # Dictionary of particles and forces
	self.particlesDict = {}
	self.forcesDict = {}
        # The effect's particle system
	self.addParticles(Particles.Particles())

    def enable(self):
	"""enable()"""
	for f in self.forcesDict.values():
	    f.enable()
	for p in self.particlesDict.value():
	    p.enable()

    def disable(self):
	"""disable()"""
	for f in self.forcesDict.values():
	    f.disable()
	for p in self.particlesDict.values():
	    p.disable()

    def addForces(self, forces):
	"""addForces(forces)"""
	forces.nodePath.reparentTo(self)
	self.forcesDict[forces.getName()] = forces

    def addParticles(self, particles):
	"""addParticles(particles)"""
	particles.nodePath.reparentTo(self)
	self.particlesDict[particles.getName()] = particles

    def getParticles(self):
        """getParticles()"""
        return self.particlesDict.values()
    
    def getParticlesNamed(self, name):
        """getParticlesNamed(name)"""
        return self.particlesDict.get(name, None)

    def getParticlesDict(self):
        """getParticlesDict()"""
        return self.particlesDict

    def getForces(self):
        """getForces()"""
        return self.forcesDict.values()

    def getForcesNamed(self, name):
        """getForcesNamed(name)"""
        return self.forcesDict.get(name, None)

    def getForcesDict(self):
        """getForces()"""
        return self.forcesDict

