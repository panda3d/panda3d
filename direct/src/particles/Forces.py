from PandaObject import *
from DirectObject import *
from PhysicsManagerGlobal import *

import ForceNode

class Forces(DirectObject):

    forceNum = 1

    def __init__(self, name = None):
	"""__init__(self)"""

	if (name == None):
	    self.name = 'Forces-%d' % self.forceNum
	    self.forceNum = self.forceNum + 1
	else:
	    self.name = name

 	self.node = ForceNode.ForceNode(self.name)
	self.nodePath = hidden.attachNewNode(self.node)

    def addForce(self, force):
	"""addForce(self, force)"""
	self.node.addForce(force)

    def removeForce(self, force):
	"""removeForce(self, force)"""
	self.node.removeForce(force)

    def enable(self):
	"""enable(self)"""
	for i in self.node.getNumForces():
	    f = self.node.getForce(i)
	    if (f.isLinear() == 1):
		physicsManager.addLinearForce(f)
	    else:
		physicsManager.addAngularForce(f)

    def disable(self):
	"""disable(self)"""
	for i in self.node.getNumForces():
	    f = self.node.getForce(i)
	    if (f.isLinear() == 1):
		physicsManager.removeLinearForce(f)
	    else:
		physicsManager.removeAngularForce(f)

    def __getItem__(self, index):
	"""__getItem__(self, index)"""
	return self.node.getForce(index)

    def __len__(self):
	"""__len__(self)"""
	return self.node.getNumForces()

    def __asList__(self):
	"""__asList__(self)"""
	l = []
	for i in self.node.getNumForces():
	    l.append(self.node.getForce(i))
	return l
