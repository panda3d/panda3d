from PandaObject import *
from DirectObject import *
from PhysicsManagerGlobal import *

import ForceNode

class ForceGroup(DirectObject):

    forceNum = 1

    def __init__(self, name = None):
	"""__init__(self)"""

	if (name == None):
	    self.name = 'ForceGroup-%d' % self.forceNum
	    self.forceNum = self.forceNum + 1
	else:
	    self.name = name

 	self.node = ForceNode.ForceNode(self.name)
	self.nodePath = hidden.attachNewNode(self.node)
        self.fEnabled = 0

    def enable(self):
	"""enable(self)"""
	for i in range(self.node.getNumForces()):
	    f = self.node.getForce(i)
	    if (f.isLinear() == 1):
		physicsMgr.addLinearForce(f)
	    else:
		physicsMgr.addAngularForce(f)
        self.fEnabled = 1

    def disable(self):
	"""disable(self)"""
	for i in range(self.node.getNumForces()):
	    f = self.node.getForce(i)
	    if (f.isLinear() == 1):
		physicsMgr.removeLinearForce(f)
	    else:
		physicsMgr.removeAngularForce(f)
        self.fEnabled = 0

    def isEnabled(self):
        return self.fEnabled

    def addForce(self, force):
	"""addForce(self, force)"""
	if (force.isLinear() == 0):
	    # Physics manager will need an angular integrator
	    base.addAngularIntegrator()
	self.node.addForce(force)
        if self.fEnabled:
	    if (force.isLinear() == 1):
		physicsMgr.addLinearForce(force)
	    else:
		physicsMgr.addAngularForce(force)

    def removeForce(self, force):
	"""removeForce(self, force)"""
	self.node.removeForce(force)

    # Get/set
    def getName(self):
        """getName(self)"""
        return self.name
    def getNode(self):
        """getNode(self)"""
        return self.node
    def getNodePath(self):
        """getNodePath(self)"""
        return self.nodePath

    # Utility functions 
    def __getitem__(self, index):
	"""__getItem__(self, index)"""
	return self.node.getForce(index)

    def __len__(self):
	"""__len__(self)"""
	return self.node.getNumForces()

    def asList(self):
	"""asList(self)"""
	l = []
	for i in range(self.node.getNumForces()):
	    l.append(self.node.getForce(i))
	return l

