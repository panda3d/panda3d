from PandaObject import *
from DirectObject import *

import Particles
import Forces

class ParticleEffect(NodePath):
    def __init__(self, nodePath):
	"""__init__(self, nodePath)"""
	NodePath.__init__(self)
	self.assign(nodePath)
	self.name = self.getName()

	self.forces = []

	self.particles = []
	self.addParticles(Particles.Particles())

    def addForces(self, forces):
	"""addForces(self, forces)"""
	forces.nodePath.reparentTo(self)
	self.forces.append(forces)

    def addParticles(self, part):
	"""addParticles(self, part)"""
	part.nodePath.reparentTo(self)
	self.particles.append(part)

    def enable(self):
	"""enable(self)"""
	for f in self.forces:
	    f.enable()
	for p = self.particles:
	    p.enable()

    def disable(self):
	"""disable(self)"""
	for f = self.forces:
	    f.disable()
	for p = self.particles:
	    p.disable()
