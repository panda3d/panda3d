from PandaModules import *
from DirectNotifyGlobal import *
import Interval
import Task
import types

PREVIOUS_END = CMetaInterval.RSPreviousEnd
PREVIOUS_START = CMetaInterval.RSPreviousBegin
TRACK_START = CMetaInterval.RSLevelBegin

class MetaInterval(CMetaInterval):

    # This is a Python-C++ hybrid class.  MetaInterval is a Python
    # extension of the C++ class CMetaInterval, which adds some
    # Python-specific features (like list management).

    # This is the base class of Sequence, Parallel, and Track.

    notify = directNotify.newCategory("Interval")

    SequenceNum = 1
    def __init__(self, *ivals, **kw):
        name = None
        if len(ivals) == 2 and isinstance(ivals[1], types.StringType):
            # If the second parameter is a string, it's the name.
            name = ivals[1]
            ivals = ivals[0]
        else:
            # Otherwise, look for the name in the keyword params.
            if kw.has_key('name'):
                name = kw['name']
                del kw['name']

        if kw:
            self.notify.error("Unexpected keyword parameters: %s" % (kw.keys()))

        # We must allow the old style: Track([ ival0, ival1, ... ]) as
        # well as the new style: Track(ival0, ival1, ...)
        if len(ivals) == 1 and \
           (isinstance(ivals[0], types.TupleType) or \
            isinstance(ivals[0], types.ListType)):
            self.ivals = ivals[0]
        else:
            self.ivals = ivals
            
        self.__ivalsDirty = 1

        if name == None:
            name = '%s-%d' % (self.__class__.__name__, self.SequenceNum)
            MetaInterval.SequenceNum += 1

        CMetaInterval.__init__(self, name)

        self.pythonIvals = []


    # Functions to make the MetaInterval object act just like a Python
    # list of intervals:
    
    def append(self, ival):
        # Appends a single interval to the list so far.
        if isinstance(self.ivals, types.TupleType):
            self.ivals = list(self.ivals)
        self.ivals.append(ival)
        self.__ivalsDirty = 1

    def extend(self, ivals):
        # Appends a list of intervals to the list so far.
        self += ivals

    def __len__(self):
        return len(self.ivals)

    def __getitem__(self, index):
        return self.ivals[index]

    def __setitem__(self, index, value):
        if isinstance(self.ivals, types.TupleType):
            self.ivals = list(self.ivals)
        self.ivals[index] = value
        self.__ivalsDirty = 1

    def __delitem__(self, index):
        if isinstance(self.ivals, types.TupleType):
            self.ivals = list(self.ivals)
        del self.ivals[index]
        self.__ivalsDirty = 1

    def __getslice__(self, i, j):
        if isinstance(self.ivals, types.TupleType):
            self.ivals = list(self.ivals)
        return self.ivals[i : j]

    def __setslice__(self, i, j, s):
        if isinstance(self.ivals, types.TupleType):
            self.ivals = list(self.ivals)
        self.ivals[i : j] = s
        self.__ivalsDirty = 1

    def __delslice__(self, i, j):
        if isinstance(self.ivals, types.TupleType):
            self.ivals = list(self.ivals)
        del self.ivals[i : j]
        self.__ivalsDirty = 1

    def __iadd__(self, other):
        if isinstance(self.ivals, types.TupleType):
            self.ivals = list(self.ivals)
        if isinstance(other, MetaInterval):
            self.ivals += other.ivals
        else:
            self.ivals += list(other)
        self.__ivalsDirty = 1
        return self

    
    def addSequence(self, list, relTime, relTo):
        # Adds the given list of intervals to the MetaInterval to be
        # played one after the other.
        self.pushLevel(relTime, relTo)
        for ival in list:
            self.addInterval(ival, 0.0, PREVIOUS_END)
        self.popLevel()

    def addParallel(self, list, relTime, relTo):
        # Adds the given list of intervals to the MetaInterval to be
        # played simultaneously.
        self.pushLevel(relTime, relTo)
        for ival in list:
            self.addInterval(ival, 0.0, TRACK_START)
        self.popLevel()

    def addTrack(self, list, relTime, relTo):
        # Adds a "track list".  This is a list of tuples of the form:
        #
        #   ( <delay>, <Interval>,
        #       PREVIOUS_END | PREVIOUS_START | TRACK_START )
        #
        # where <delay> is a relative time, in seconds, for the
        # <Interval> to start, relative to either the end of the
        # previous interval (PREVIOUS_END), the start of the previous
        # interval (PREVIOUS_START) or the start of the track list
        # (TRACK_START).  If the relative code is omitted, the default
        # is TRACK_START.
        self.pushLevel(relTime, relTo)
        for tuple in list:
            if isinstance(tuple, Interval.Interval) or \
               isinstance(tuple, CInterval):
                # Actually, it's not a tuple, but just an interval.
                # In this case we fall back on the old default of
                # assuming a sequential list of intervals.  This is a
                # temporary feature for backward compatibility.
                self.addInterval(tuple, 0.0, PREVIOUS_END)

            elif isinstance(tuple, types.TupleType) or \
                 isinstance(tuple, types.ListType):
                relTime = tuple[0]
                ival = tuple[1]
                if len(tuple) >= 3:
                    relTo = tuple[2]
                else:
                    relTo = TRACK_START
                self.addInterval(ival, relTime, relTo)

            else:
                self.notify.error("Not a tuple in Track: %s" % (tuple,))
        self.popLevel()

    def addInterval(self, ival, relTime, relTo):
        # Adds the given interval to the MetaInterval.

        if isinstance(ival, CInterval):
            # It's a C++-style Interval, so add it directly.
            if getattr(ival, "inPython", 0):
                # Actually, it's been flagged to run in Python, even
                # though it's a C++ Interval.  It's probably got some
                # Python functors that must be invoked at runtime to
                # define some of its parameters.  Treat it as a Python
                # interval.
                index = len(self.pythonIvals)
                self.pythonIvals.append(ival)
                self.addExtIndex(index, ival.getName(), ival.getDuration(),
                                 ival.getOpenEnded(), relTime, relTo)

            elif isinstance(ival, MetaInterval):
                # It's another MetaInterval, so copy in its intervals
                # directly to this object.  We could just store the
                # MetaInterval itself, which would work, but we get a
                # performance advantage by flattening out the deeply
                # nested hierarchy into a linear list within the root
                # CMetaInterval object.
                ival.applyIvals(self, relTime, relTo)
            
            else:
                # Nope, a perfectly ordinary C++ interval.  Hooray!
                self.addCInterval(ival, relTime, relTo)

        elif isinstance(ival, Interval.Interval):
            # It's a Python-style Interval, so add it as an external.
            index = len(self.pythonIvals)
            self.pythonIvals.append(ival)
            self.addExtIndex(index, ival.getName(), ival.getDuration(),
                             ival.getOpenEnded(), relTime, relTo)

            # Once we have any Python intervals, we must handle this
            # interval from Python.
            self.inPython = 1

        else:
            self.notify.error("Not an Interval: %s" % (ival,))
            

    def __updateIvals(self):
        # The MetaInterval object does not create the C++ list of
        # Intervals immediately; rather, it stores a Python list of
        # Intervals that will be compiled into the C++ list the first
        # time it is needed.

        # This design allows us to avoid creation of the C++ list for
        # nested MetaInterval objects, instead copying all nested
        # MetaInterval hierarchy into the root CMetaInterval object,
        # for a performance benefit.

        # This function is called only on the root MetaInterval
        # object, when it is time to build the C++ list for itself.

        if self.__ivalsDirty:
            self.clearIntervals()
            self.applyIvals(self, 0, TRACK_START)
            self.__ivalsDirty = 0
        
    def clearIntervals(self):
        # This overrides the function defined at the C++ level to
        # reset the inPython flag.  Clearing out the intervals list
        # allows us to run entirely in C++ again, at least until a new
        # Python interval gets added.
        CMetaInterval.clearIntervals(self)
        self.inPython = 0
        
    def applyIvals(self, meta, relTime, relTo):
        # Add the intervals listed in this object to the given
        # MetaInterval object at the C++ level.  This will make the
        # other MetaInterval object ready to play the intervals.

        # This function should be overridden in a derived class to
        # change the intepretation of the intervals in this list.  In
        # the case of a MetaInterval directly, this is valid only if
        # the list has only zero or one intervals.

        if len(self.ivals) == 0:
            pass

        elif len(self.ivals) == 1:
            meta.addInterval(self.ivals[0], relTime, relTo)

        else:
            self.notify.error("Cannot build list from MetaInterval directly.")

    def __doPythonCallbacks(self):
        # This function invokes any Python-level Intervals that need
        # to be invoked at this point in time.  It must be called
        # after any call to setT() or setFinalT() or stepPlay(), or
        # some such; basically any function that might invoke an
        # interval.  The C++ base class will invoke whatever C++
        # intervals it can, and then indicate the Python intervals
        # that must be invoked through this interface.

        while (self.isEventReady()):
            index = self.getEventIndex()
            t = self.getEventT()
            eventType = self.getEventType()
            self.popEvent()
            
            ival = self.pythonIvals[index]
            ival.privDoEvent(t, eventType)
            ival.privPostEvent()

    def privDoEvent(self, t, event):
        # This function overrides the C++ function to initialize the
        # intervals first if necessary.
        self.__updateIvals()
        CMetaInterval.privDoEvent(self, t, event)

    def privPostEvent(self):
        self.__doPythonCallbacks()
        CMetaInterval.privPostEvent(self)

    def setIntervalStartTime(self, *args, **kw):
        # This function overrides from the parent level to force it to
        # update the interval list first, if necessary.

        self.__updateIvals()
        # Once we have monkeyed with the interval timings, we'd better
        # run the whole thing as a monolithic Python interval, since
        # we can't extract the ivals list back out and append them
        # into a parent MetaInterval.
        self.inPython = 1
        return CMetaInterval.setIntervalStartTime(self, *args, **kw)

    def getIntervalStartTime(self, *args, **kw):
        # This function overrides from the parent level to force it to
        # update the interval list first, if necessary.

        self.__updateIvals()
        return CMetaInterval.getIntervalStartTime(self, *args, **kw)
        
        
    def getDuration(self):
        # This function overrides from the parent level to force it to
        # update the interval list first, if necessary.

        self.__updateIvals()
        return CMetaInterval.getDuration(self)

    def start(self, *args, **kw):
        # This function overrides from the parent level to force it to
        # update the interval list first, if necessary.

        self.__updateIvals()
        return CMetaInterval.start(self, *args, **kw)

    def loop(self, *args, **kw):
        # This function overrides from the parent level to force it to
        # update the interval list first, if necessary.

        self.__updateIvals()
        return CMetaInterval.loop(self, *args, **kw)

    def __repr__(self, *args, **kw): 
        # This function overrides from the parent level to force it to
        # update the interval list first, if necessary.

        self.__updateIvals()
        return CMetaInterval.__repr__(self, *args, **kw)

    def __str__(self, *args, **kw): 
        # This function overrides from the parent level to force it to
        # update the interval list first, if necessary.

        self.__updateIvals()
        return CMetaInterval.__str__(self, *args, **kw)




class Sequence(MetaInterval):
    def applyIvals(self, meta, relTime, relTo):
        meta.addSequence(self.ivals, relTime, relTo)

class Parallel(MetaInterval):
    def applyIvals(self, meta, relTime, relTo):
        meta.addParallel(self.ivals, relTime, relTo)

class Track(MetaInterval):
    def applyIvals(self, meta, relTime, relTo):
        meta.addTrack(self.ivals, relTime, relTo)

# Temporary for backward compatibility.
class MultiTrack(MetaInterval):
    def applyIvals(self, meta, relTime, relTo):
        meta.addParallel(self.ivals, relTime, relTo)
