from PandaObject import *
from DirectObject import *

import Particles
import Forces

class ParticleEffect(NodePath):
    def __init__(self, name = 'ParticleEffect'):
	"""__init__(self)"""
	NodePath.__init__(self)
        self.assign(hidden.attachNewNode(name))
	self.name = name

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
	for p in self.particles:
	    p.enable()

    def disable(self):
	"""disable(self)"""
	for f in self.forces:
	    f.disable()
	for p in self.particles:
	    p.disable()
