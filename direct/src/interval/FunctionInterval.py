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

### FunctionInterval subclass for accepting hooks ###
class AcceptInterval(FunctionInterval):
    # Initialization
    def __init__(self, dirObj, event, function, name):
        """__init__(dirObj, event, function, name)
        """
        def acceptFunc(dirObj = dirObj, event = event, function = function):
            print "accepting..."
            dirObj.accept(event, function)
        # Create function interval
	FunctionInterval.__init__(self, acceptFunc, name = name)

### FunctionInterval subclass for throwing events ###
class IgnoreInterval(FunctionInterval):
    # Initialization
    def __init__(self, dirObj, event, name):
        """__init__(dirObj, event, name)
        """
        def ignoreFunc(dirObj = dirObj, event = event):
            print "ignoring..."
            dirObj.ignore(event)
        # Create function interval
	FunctionInterval.__init__(self, ignoreFunc, name = name)

### Function Interval subclass for adjusting scene graph hierarchy ###
class ParentInterval(FunctionInterval):
    # ParentInterval counter
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

### Function Interval subclass for adjusting scene graph hierarchy ###
class WrtParentInterval(FunctionInterval):
    # WrtParentInterval counter
    wrtParentIntervalNum = 1
    # Initialization
    def __init__(self, nodePath, parent, name = None):
        """__init__(nodePath, parent, name)
        """
        def wrtReparentFunc(nodePath = nodePath, parent = parent):
            nodePath.wrtReparentTo(parent)
        # Determine name
	if (name == None):
	    name = 'WrtParentInterval-%d' % WrtParentInterval.wrtParentIntervalNum
	    WrtParentInterval.wrtParentIntervalNum += 1
        # Create function interval
	FunctionInterval.__init__(self, wrtReparentFunc, name = name)

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


"""
SAMPLE CODE

### Using lambdas and functions ###
# Using a lambda
i1 = FunctionInterval(lambda: base.transitions.fadeOut())
i2 = FunctionInterval(lambda: base.transitions.fadeIn())
# Using a function
def printHello():
    print 'hello'

i3 = FunctionInterval(printHello)
# Create track
t = Track([(0.0, i1), (2.0, i2), (4.0, i3)], name = 'demo')
# Play track
t.play()

### Specifying interval start times during track construction ###
# Interval start time can be specified relative to three different points:
# PREVIOUS_END
# PREVIOUS_START
# TRACK_START

startTime = 0.0
def printStart():
    global startTime
    startTime = globalClock.getFrameTime()
    print 'Start'

def printPreviousStart():
    global startTime
    currTime = globalClock.getFrameTime()
    print 'PREVIOUS_END %0.2f' % (currTime - startTime)

def printPreviousEnd():
    global startTime
    currTime = globalClock.getFrameTime()
    print 'PREVIOUS_END %0.2f' % (currTime - startTime)

def printTrackStart():
    global startTime
    currTime = globalClock.getFrameTime()
    print 'TRACK_START %0.2f' % (currTime - startTime)

i1 = FunctionInterval(printStart)
# Just to take time
i2 = LerpPosInterval(camera, 2.0, Point3(0,10,0))
# This will be relative to end of camera move
i3 = FunctionInterval(printPreviousEnd)
# Just to take time
i4 = LerpPosInterval(camera, 2.0, Point3(0,0,0))
# This will be relative to the start of the camera move
i5 = FunctionInterval(printPreviousStart)
# This will be relative to track start
i6 = FunctionInterval(printTrackStart)
# Create the track, if you don't specify offset type in tuple it defaults to
# relative to TRACK_START (first entry below)
t = Track([(0.0, i1),                 # i1 start at t = 0, duration = 0.0
           (1.0, i2, TRACK_START),    # i2 start at t = 1, duration = 2.0
           (2.0, i3, PREVIOUS_END),   # i3 start at t = 5, duration = 0.0
           (1.0, i4, PREVIOUS_END),   # i4 start at t = 6, duration = 2.0
           (3.0, i5, PREVIOUS_START), # i5 start at t = 9, duration = 0.0
           (10.0, i6, TRACK_START)],  # i6 start at t = 10, duration = 0.0
          name = 'startTimeDemo')
t.play()

"""
