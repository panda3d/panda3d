from PandaObject import *
from DirectObject import *
from ParticleManagerGlobal import *
from PhysicsManagerGlobal import *
from TaskManagerGlobal import *

import Particles
import ForceNode
import PhysicalNode
import ClockObject

globalClock = ClockObject.ClockObject.getGlobalClock()

class ParticleEffect(DirectObject):
    def __init__(self, name):
	"""__init__(self, name)"""
	
	self.name = name
	self.activated = 0
	self.particleDict = {}

	self.forceNode = ForceNode.ForceNode()
	self.forceNodePath = hidden.attachNewNode(self.forceNode)

	self.physicalNode = PhysicalNode.PhysicalNode()
	self.physicalNodePath = hidden.attachNewNode(self.physicalNode)

    def addLinearForce(self, force):
	"""addLinearForce(self, force)"""
	self.forceNode.addForce(force)
	physicsMgr.addLinearForce(force) 

    def removeLinearForce(self, force):
	"""removeLinearForce(self, force)"""
	physicsMgr.removeLinearForce(force)
	self.forceNode.removeForce(force)

    def addAngularForce(self, force):
	"""addAngularForce(self, force)"""
	self.forceNode.addForce(force)
	physicsMgr.addAngularForce(force)

    def removeAngularForce(self, force):
	"""removeAngularForce(self, force)"""
	physicsMgr.removeAngularForce(force)
	self.forceNode.removeForce(force)

    def addParticles(self, particles):
	"""addParticles(self, particles)"""
	self.particleDict[particles.name] = particles
	self.physicalNode.addPhysical(particles)
	physicsMgr.attachPhysical(particles)
	particleMgr.attachParticlesystem(particles)

    def removeParticles(self, particles):
	"""removeParticles(self, particles)"""
	particleMgr.removeParticlesystem(particles)	
	physicsMgr.removePhysical(particles)
	self.physicalNode.removePhysical(particles)
	self.particleDict[particles.name] = None

    def activate(self):
	"""activate(self)"""
	if (self.activated == 0):
	    self.activated = 1
	    self.forceNodePath.reparentTo(render)
	    self.physicalNodePath.reparentTo(render)
	    taskMgr.spawnTaskNamed(Task.Task(self.__update), 
					self.name + '-update')

    def deactivate(self):
	"""deactivate(self)"""
	if (self.activated == 1):
	    self.activated = 0
	    taskMgr.removeTasksNamed(self.name + '-update')
	    self.forceNodePath.reparentTo(hidden)
	    self.physicalNodePath.reparentTo(hidden)

    def __update(self, state):
	"""__update(self, state)"""
	dt = min(globalClock.getDt(), 0.1)
   	physicsMgr.doPhysics(dt)
	particleMgr.doParticles(dt)
	return Task.cont
