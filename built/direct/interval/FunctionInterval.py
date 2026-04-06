"""FunctionInterval module: contains the FunctionInterval class"""

from __future__ import annotations

__all__ = ['FunctionInterval', 'EventInterval', 'AcceptInterval', 'IgnoreInterval', 'ParentInterval', 'WrtParentInterval', 'PosInterval', 'HprInterval', 'ScaleInterval', 'PosHprInterval', 'HprScaleInterval', 'PosHprScaleInterval', 'Func', 'Wait']

from panda3d.direct import WaitInterval
from direct.showbase.MessengerGlobal import messenger
from direct.directnotify.DirectNotifyGlobal import directNotify
from . import Interval


class FunctionInterval(Interval.Interval):
    # Name counter
    functionIntervalNum = 1

    # Keep a list of function intervals currently in memory for
    # Control-C-Control-V redefining. These are just weakrefs so they
    # should not cause any leaks.
    if __debug__:
        import weakref
        FunctionIntervals: weakref.WeakKeyDictionary[FunctionInterval, int] = weakref.WeakKeyDictionary()

        @classmethod
        def replaceMethod(cls, oldFunction, newFunction):
            import types
            count = 0
            for ival in cls.FunctionIntervals:
                # print 'testing: ', ival.function, oldFunction
                # Note: you can only replace methods currently
                if isinstance(ival.function, types.MethodType):
                    if ival.function.__func__ == oldFunction:
                        # print 'found: ', ival.function, oldFunction
                        ival.function = types.MethodType(newFunction,
                                                         ival.function.__self__)
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
        if name is None:
            name = self.makeUniqueName(function)
        assert isinstance(name, str)

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
        func_name = getattr(func, '__name__', None)
        if func_name is None:
            func_name = str(func)
        name = 'Func-%s-%d' % (func_name, FunctionInterval.functionIntervalNum)
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
        if name is None:
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
        if name is None:
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
        if name is None:
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
        if name is None:
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
        if name is None:
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
        if name is None:
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
        if name is None:
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
        if name is None:
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
        if name is None:
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
        if name is None:
            name = ('PosHprScale-%d' %
                    PosHprScaleInterval.posHprScaleIntervalNum)
            PosHprScaleInterval.posHprScaleIntervalNum += 1
        # Create function interval
        FunctionInterval.__init__(self, posHprScaleFunc, name = name)




class Func(FunctionInterval):
    def __init__(self, *args, **kw):
        function = args[0]
        assert hasattr(function, '__call__')
        extraArgs = args[1:]
        kw['extraArgs'] = extraArgs
        FunctionInterval.__init__(self, function, **kw)

class Wait(WaitInterval):
    def __init__(self, duration):
        WaitInterval.__init__(self, duration)
