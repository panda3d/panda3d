"""FunctionInterval module: contains the FunctionInterval class"""

from PandaModules import *
from Interval import *
from MessengerGlobal import *

class FunctionInterval(Interval):

    functionIntervalNum = 1
    
    # special methods
    def __init__(self, function, name = None):
        """__init__(function, name = None)
        """
	duration = 0.0
	self.prevt = 0.0
	self.function = function
        
	if (name == None):
	    name = 'FunctionInterval-%d' % FunctionInterval.functionIntervalNum
	    FunctionInterval.functionIntervalNum += 1

	Interval.__init__(self, name, duration)

    def setT(self, t, entry=0):
	""" setT(t, entry)
	    Go to time t
	"""
	if (t < 0):
	    self.prevt = t
	    return
	elif (t == 0) or (self.prevt < 0):
	    self.function()
	    self.prevt = 0.0

### FunctionInterval subclass for throwing events ###
class EventInterval(FunctionInterval):
    # Initialization
    def __init__(self, event, sentArgs=[]):
        """__init__(event, sentArgs)
        """
        def sendFunc(event = event, sentArgs = sentArgs):
            messenger.send(event, sentArgs)
        # Create function interval
	FunctionInterval.__init__(self, sendFunc, name = event)

### Function Interval subclass for adjusting scene graph hierarchy ###
class ParentInterval(FunctionInterval):
    # PosInterval counter
    parentIntervalNum = 1
    # Initialization
    def __init__(self, nodePath, parent, name = None):
        """__init__(nodePath, parent, name)
        """
        def reparentFunc(nodePath = nodePath, parent = parent):
            nodePath.reparentTo(parent)
        # Determine name
	if (name == None):
	    name = 'ParentInterval-%d' % ParentInterval.parentIntervalNum
	    ParentInterval.parentIntervalNum += 1
        # Create function interval
	FunctionInterval.__init__(self, reparentFunc, name = name)

### Function Interval subclasses for instantaneous pose changes ###
class PosInterval(FunctionInterval):
    # PosInterval counter
    posIntervalNum = 1
    # Initialization
    def __init__(self, nodePath, pos, duration = 0.0,
                 name = None, other = None):
        """__init__(nodePath, pos, duration, name)
        """
        # Create function
        def posFunc(np = nodePath, pos = pos, other = other):
            if other:
                np.setPos(other, pos)
            else:
                np.setPos(pos)
        # Determine name
	if (name == None):
	    name = 'PosInterval-%d' % PosInterval.posIntervalNum
	    PosInterval.posIntervalNum += 1
        # Create function interval
        FunctionInterval.__init__(self, posFunc, name = name)

class HprInterval(FunctionInterval):
    # HprInterval counter
    hprIntervalNum = 1
    # Initialization
    def __init__(self, nodePath, hpr, duration = 0.0,
                 name = None, other = None):
        """__init__(nodePath, hpr, duration, name)
        """
        # Create function
        def hprFunc(np = nodePath, hpr = hpr, other = other):
            if other:
                np.setHpr(other, hpr)
            else:
                np.setHpr(hpr)
        # Determine name
	if (name == None):
	    name = 'HprInterval-%d' % HprInterval.hprIntervalNum
	    HprInterval.hprIntervalNum += 1
        # Create function interval
        FunctionInterval.__init__(self, hprFunc, name = name)

class ScaleInterval(FunctionInterval):
    # ScaleInterval counter
    scaleIntervalNum = 1
    # Initialization
    def __init__(self, nodePath, scale, duration = 0.0,
                 name = None, other = None):
        """__init__(nodePath, scale, duration, name)
        """
        # Create function
        def scaleFunc(np = nodePath, scale = scale, other = other):
            if other:
                np.setScale(other, scale)
            else:
                np.setScale(scale)
        # Determine name
	if (name == None):
	    name = 'ScaleInterval-%d' % ScaleInterval.scaleIntervalNum
	    ScaleInterval.scaleIntervalNum += 1
        # Create function interval
        FunctionInterval.__init__(self, scaleFunc, name = name)

class PosHprInterval(FunctionInterval):
    # PosHprInterval counter
    posHprIntervalNum = 1
    # Initialization
    def __init__(self, nodePath, pos, hpr, duration = 0.0,
                 name = None, other = None):
        """__init__(nodePath, pos, hpr, duration, name)
        """
        # Create function
        def posHprFunc(np = nodePath, pos = pos, hpr = hpr, other = other):
            if other:
                np.setPosHpr(other, pos, hpr)
            else:
                np.setPosHpr(pos, hpr)
        # Determine name
	if (name == None):
	    name = 'PosHprInterval-%d' % PosHprInterval.posHprIntervalNum
	    PosHprInterval.posHprIntervalNum += 1
        # Create function interval
        FunctionInterval.__init__(self, posHprFunc, name = name)

class PosHprScaleInterval(FunctionInterval):
    # PosHprInterval counter
    posHprScaleIntervalNum = 1
    # Initialization
    def __init__(self, nodePath, pos, hpr, scale, duration = 0.0,
                 name = None, other = None):
        """__init__(nodePath, pos, hpr, scale, duration, other, name)
        """
        # Create function
        def posHprScaleFunc(np=nodePath, pos=pos, hpr=hpr, scale=scale,
                            other = other):
            if other:
                np.setPosHprScale(other, pos, hpr, scale)
            else:
                np.setPosHprScale(pos, hpr, scale)                
        # Determine name
	if (name == None):
	    name = ('PosHprScale-%d' %
                    PosHprScaleInterval.posHprScaleIntervalNum)
	    PosHprScaleInterval.posHprScaleIntervalNum += 1
        # Create function interval
        FunctionInterval.__init__(self, posHprScaleFunc, name = name)
