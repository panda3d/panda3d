from PandaObject import *
from DirectGeometry import *

import NodePath

class Mopath(PandaObject):

    def __init__(self):
	self.maxT = 0.0
	self.xyzNurbsCurve = None
	self.hprNurbsCurve = None
	self.tNurbsCurve = None

    def loadFile(self, filename):
	nodePath = loader.loadModel(filename)
        if nodePath:
            self.__extractCurves(nodePath)
	    if (self.xyzNurbsCurve != None):
		self.maxT = self.xyzNurbsCurve.getMaxT()
	    elif (self.hprNurbsCurve != None):
		self.maxT = self.hprNurbsCurve.getMaxT()
	    elif (self.tNurbsCurve != None):
		self.maxT = self.tNurbsCurve.getMaxT()
	    else:
		print 'Mopath: no valid curves in file: %s' % filename
	else:
	    print 'Mopath: no data in file: %s' % filename

    def __extractCurves(self, nodePath):
        node = nodePath.node()
        if isinstance(node, ParametricCurve):
            if node.getCurveType() == PCTXYZ:
                self.xyzNurbsCurve = node
            elif node.getCurveType() == PCTHPR:
                self.hprNurbsCurve = node
	    elif node.getCurveType() == PCTNONE:
		if (self.xyzNurbsCurve == None):
		    self.xyzNurbsCurve = node
		else:
		    print 'Mopath: got a PCT_NONE curve and an XYZ Curve!'
	    elif (node.getCurveType() == PCTT):
		self.tNurbsCurve = node
        else:
            # Iterate over children if any
            for child in nodePath.getChildrenAsList():
                self.__extractCurves(child)

    def goTo(self, node, time):
	if (self.xyzNurbsCurve == None) & (self.hprNurbsCurve == None):
	    print 'Mopath: Mopath has no curves'
	    return
	self.playbackTime = CLAMP(time, 0.0, self.maxT)
	if (self.xyzNurbsCurve != None):
	    pos = Point3(0)
	    self.xyzNurbsCurve.getPoint(self.playbackTime, pos)
	    node.setPos(pos)
	if (self.hprNurbsCurve != None):
	    hpr = Point3(0)
	    self.hprNurbsCurve.getPoint(self.playbackTime, hpr)
	    node.setHpr(hpr)
