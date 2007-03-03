"""FunctionInterval module: contains the FunctionInterval class"""

__all__ = ['FunctionInterval', 'EventInterval', 'AcceptInterval', 'IgnoreInterval', 'ParentInterval', 'WrtParentInterval', 'PosInterval', 'HprInterval', 'ScaleInterval', 'PosHprInterval', 'HprScaleInterval', 'PosHprScaleInterval', 'Func', 'Wait']

from pandac.PandaModules import *
from direct.showbase.MessengerGlobal import *
from direct.directnotify.DirectNotifyGlobal import directNotify
import Interval
import types


#############################################################
###                                                       ###
### See examples of function intervals in IntervalTest.py ###
###                                                       ###
#############################################################

class FunctionInterval(Interval.Interval):
    # Name counter
    functionIntervalNum = 1

    # Keep a list of function intervals currently in memory for
    # Control-C-Control-V redefining. These are just weakrefs so they
    # should not cause any leaks. 
    if __debug__:
        import weakref
        FunctionIntervals = weakref.WeakKeyDictionary()

        @classmethod
        def replaceMethod(self, oldFunction, newFunction):
            import new
            import types
            count = 0        
            for ival in self.FunctionIntervals:
                # print 'testing: ', ival.function, oldFunction
                # Note: you can only replace methods currently
                if type(ival.function) == types.MethodType:
                    if (ival.function.im_func == oldFunction):
                        # print 'found: ', ival.function, oldFunction
                        ival.function = new.instancemethod(newFunction,
                                                           ival.function.im_self,
                                                           ival.function.im_class)
                        count += 1
            return count

    # create FunctionInterval DirectNotify category
    notify = directNotify.newCategory('FunctionInterval')

    # Class methods
    def __init__(self, function, **kw):
        """__init__(function, name = None, openEnded = 1, extraArgs = [])
        """
        name = kw.pop('name', None)
        openEnded = kw.pop('openEnded', 1)
        extraArgs = kw.pop('extraArgs', [])

        # Record instance variables
        self.function = function
            
        # Create a unique name for the interval if necessary
        if (name == None):
            name = self.makeUniqueName(function)
        assert isinstance(name, types.StringType)
        # Record any arguments
        self.extraArgs = extraArgs
        self.kw = kw
        # Initialize superclass
        # Set openEnded true if privInitialize after end time cause interval
        # function to be called.  If false, privInitialize calls have no effect
        # Event, Accept, Ignore intervals default to openEnded = 0
        # Parent, Pos, Hpr, etc intervals default to openEnded = 1
        Interval.Interval.__init__(self, name, duration = 0.0, openEnded = openEnded)

        # For rebinding, let's remember this function interval on the class
        if __debug__:
            self.FunctionIntervals[self] = 1

    @staticmethod
    def makeUniqueName(func, suffix = ''):
        name = 'Func-%s-%d' % (func.__name__, FunctionInterval.functionIntervalNum)
        FunctionInterval.functionIntervalNum += 1
        if suffix:
            name = '%s-%s' % (name, str(suffix))
        return name

    def privInstant(self):
        # Evaluate the function
        self.function(*self.extraArgs, **self.kw)
        # Print debug information
        self.notify.debug(
            'updateFunc() - %s: executing Function' % self.name)


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
    def __init__(self, dirObj, event, function, name = None):
        """__init__(dirObj, event, function, name)
        """
        def acceptFunc(dirObj = dirObj, event = event, function = function):
            dirObj.accept(event, function)
        # Determine name
        if (name == None):
            name = 'Accept-' + event
        # Create function interval
        FunctionInterval.__init__(self, acceptFunc, name = name)

### FunctionInterval subclass for ignoring events ###
class IgnoreInterval(FunctionInterval):
    # Initialization
    def __init__(self, dirObj, event, name = None):
        """__init__(dirObj, event, name)
        """
        def ignoreFunc(dirObj = dirObj, event = event):
            dirObj.ignore(event)
        # Determine name
        if (name == None):
            name = 'Ignore-' + event
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
            name = ('WrtParentInterval-%d' %
                    WrtParentInterval.wrtParentIntervalNum)
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

class HprScaleInterval(FunctionInterval):
    # HprScaleInterval counter
    hprScaleIntervalNum = 1
    # Initialization
    def __init__(self, nodePath, hpr, scale, duration = 0.0,
                 name = None, other = None):
        """__init__(nodePath, hpr, scale, duration, other, name)
        """
        # Create function
        def hprScaleFunc(np=nodePath, hpr=hpr, scale=scale,
                            other = other):
            if other:
                np.setHprScale(other, hpr, scale)
            else:
                np.setHprScale(hpr, scale)
        # Determine name
        if (name == None):
            name = ('HprScale-%d' %
                    HprScaleInterval.hprScaleIntervalNum)
            HprScaleInterval.hprScaleIntervalNum += 1
        # Create function interval
        FunctionInterval.__init__(self, hprScaleFunc, name = name)

class PosHprScaleInterval(FunctionInterval):
    # PosHprScaleInterval counter
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




class Func(FunctionInterval):
    def __init__(self, *args, **kw):
        function = args[0]
        assert callable(function)
        extraArgs = args[1:]
        kw['extraArgs'] = extraArgs
        FunctionInterval.__init__(self, function, **kw)

class Wait(WaitInterval):
    def __init__(self, duration):
        WaitInterval.__init__(self, duration)

"""
SAMPLE CODE

from IntervalGlobal import *

i1 = Func(base.transitions.fadeOut)
i2 = Func(base.transitions.fadeIn)

def caughtIt():
    print 'Caught here-is-an-event'

class DummyAcceptor(DirectObject):
    pass

da = DummyAcceptor()
i3 = Func(da.accept, 'here-is-an-event', caughtIt)

i4 = Func(messenger.send, 'here-is-an-event')

i5 = Func(da.ignore, 'here-is-an-event')

# Using a function
def printDone():
    print 'done'

i6 = Func(printDone)

# Create track
t1 = Sequence([
    # Fade out
    (0.0, i1),
    # Fade in
    (2.0, i2),
    # Accept event
    (4.0, i3),
    # Throw it,
    (5.0, i4),
    # Ignore event
    (6.0, i5),
    # Throw event again and see if ignore worked
    (7.0, i4),
    # Print done
    (8.0, i6)], name = 'demo')

# Play track
t1.play()

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

i1 = Func(printStart)
# Just to take time
i2 = LerpPosInterval(camera, 2.0, Point3(0, 10, 5))
# This will be relative to end of camera move
i3 = FunctionInterval(printPreviousEnd)
# Just to take time
i4 = LerpPosInterval(camera, 2.0, Point3(0, 0, 5))
# This will be relative to the start of the camera move
i5 = FunctionInterval(printPreviousStart)
# This will be relative to track start
i6 = FunctionInterval(printTrackStart)
# Create the track, if you don't specify offset type in tuple it defaults to
# relative to TRACK_START (first entry below)
t2 = Track([(0.0, i1),                 # i1 start at t = 0, duration = 0.0
           (1.0, i2, TRACK_START),    # i2 start at t = 1, duration = 2.0
           (2.0, i3, PREVIOUS_END),   # i3 start at t = 5, duration = 0.0
           (1.0, i4, PREVIOUS_END),   # i4 start at t = 6, duration = 2.0
           (3.0, i5, PREVIOUS_START), # i5 start at t = 9, duration = 0.0
           (10.0, i6, TRACK_START)],  # i6 start at t = 10, duration = 0.0
          name = 'startTimeDemo')

t2.play()


smiley = loader.loadModel('models/misc/smiley')

from direct.actor import Actor
donald = Actor.Actor()
donald.loadModel("phase_6/models/char/donald-wheel-1000")
donald.loadAnims({"steer":"phase_6/models/char/donald-wheel-wheel"})
donald.reparentTo(render)


seq = Sequence(Func(donald.setPos, 0, 0, 0),
               donald.actorInterval('steer', duration=1.0),
               donald.posInterval(1, Point3(0, 0, 1)),
               Parallel(donald.actorInterval('steer', duration=1.0),
                        donald.posInterval(1, Point3(0, 0, 0)),
                        ),
               Wait(1.0),
               Func(base.toggleWireframe),
               Wait(1.0),
               Parallel(donald.actorInterval('steer', duration=1.0),
                        donald.posInterval(1, Point3(0, 0, -1)),
                        Sequence(donald.hprInterval(1, Vec3(180, 0, 0)),
                                 donald.hprInterval(1, Vec3(0, 0, 0)),
                                 ),
                        ),
               Func(base.toggleWireframe),
               Func(messenger.send, 'hello'),
               )



"""
