"""
This module defines the various "meta intervals", which execute other
intervals either in parallel or in a specified sequential order.
"""

__all__ = ['MetaInterval', 'Sequence', 'Parallel', 'ParallelEndTogether', 'Track']

from panda3d.core import *
from panda3d.direct import *
from direct.directnotify.DirectNotifyGlobal import *
from .IntervalManager import ivalMgr
from . import Interval
from direct.task.Task import TaskManager
#if __debug__:
#    import direct.showbase.PythonUtil as PythonUtil

PREVIOUS_END = CMetaInterval.RSPreviousEnd
PREVIOUS_START = CMetaInterval.RSPreviousBegin
TRACK_START = CMetaInterval.RSLevelBegin

class MetaInterval(CMetaInterval):
    # This is a Python-C++ hybrid class.  MetaInterval is a Python
    # extension of the C++ class CMetaInterval, which adds some
    # Python-specific features (like list management).

    # This is the base class of Sequence, Parallel, and Track.

    notify = directNotify.newCategory("MetaInterval")

    SequenceNum = 1
    def __init__(self, *ivals, **kw):
        #if __debug__:
        #    self.debugInitTraceback = PythonUtil.StackTrace(
        #        "create interval", 1, 10)

        name = None
        #if len(ivals) == 2 and isinstance(ivals[1], str):
        #    # If the second parameter is a string, it's the name.
        #    name = ivals[1]
        #    ivals = ivals[0]
        #else:

        # Look for the name in the keyword params.
        if 'name' in kw:
            name = kw['name']
            del kw['name']

        # If the keyword "autoPause" or "autoFinish" is defined to
        # non-zero, it means the interval may be automatically paused
        # or finished when CIntervalManager::interrupt() is called.
        # This is generally called only on a catastrophic situation
        # (for instance, the connection to the server being lost) when
        # we have to exit right away; these keywords indicate
        # intervals that might not be cleaned up by their owners.

        autoPause = 0
        autoFinish = 0
        if 'autoPause' in kw:
            autoPause = kw['autoPause']
            del kw['autoPause']
        if 'autoFinish' in kw:
            autoFinish = kw['autoFinish']
            del kw['autoFinish']

        # A duration keyword specifies the duration the interval will
        # appear to have for the purposes of computing the start time
        # for subsequent intervals in a sequence or track.
        self.phonyDuration = -1
        if 'duration' in kw:
            self.phonyDuration = kw['duration']
            del kw['duration']

        if kw:
            self.notify.error("Unexpected keyword parameters: %s" % (list(kw.keys())))

        # We must allow the old style: Track([ival0, ival1, ...]) as
        # well as the new style: Track(ival0, ival1, ...)

        # Note: this breaks in the case of a Track with one tuple:
        # Track((0, ival0),).  We could go through some effort to fix
        # this case, but for now I prefer just to document it as a
        # bug, since it will go away when we eventually remove support
        # for the old interface.
        #if len(ivals) == 1 and \
        #   (isinstance(ivals[0], tuple) or \
        #    isinstance(ivals[0], list)):
        #    self.ivals = ivals[0]
        #else:

        self.ivals = ivals

        self.__ivalsDirty = 1

        if name == None:
            name = self.__class__.__name__ + '-%d'

        if '%' in name:
            name = name % (self.SequenceNum)
            MetaInterval.SequenceNum += 1

        CMetaInterval.__init__(self, name)
        self.__manager = ivalMgr
        self.setAutoPause(autoPause)
        self.setAutoFinish(autoFinish)

        self.pstats = None
        if __debug__ and TaskManager.taskTimerVerbose:
            self.pname = name.split('-', 1)[0]
            self.pstats = PStatCollector("App:Show code:ivalLoop:%s" % (self.pname))

        self.pythonIvals = []

        # If we are running in debug mode, we validate the intervals
        # in the list right away.  There's no good reason to do this,
        # except that it makes it easier for the programmer to detect
        # when a MetaInterval is misdefined at creation time.
        assert self.validateComponents(self.ivals)



    # Functions to make the MetaInterval object act just like a Python
    # list of intervals:

    def append(self, ival):
        # Appends a single interval to the list so far.
        if isinstance(self.ivals, tuple):
            self.ivals = list(self.ivals)
        self.ivals.append(ival)
        self.__ivalsDirty = 1
        assert self.validateComponent(ival)

    def extend(self, ivals):
        # Appends a list of intervals to the list so far.
        self += ivals

    def count(self, ival):
        # Returns the number of occurrences of the indicated interval.
        return self.ivals.count(ival)

    def index(self, ival):
        # Returns the position of the indicated interval within the list.
        return self.ivals.index(ival)

    def insert(self, index, ival):
        # Inserts the given interval into the middle of the list.
        if isinstance(self.ivals, tuple):
            self.ivals = list(self.ivals)
        self.ivals.insert(index, ival)
        self.__ivalsDirty = 1
        assert self.validateComponent(ival)

    def pop(self, index = None):
        # Returns element index (or the last element) and removes it
        # from the list.
        if isinstance(self.ivals, tuple):
            self.ivals = list(self.ivals)
        self.__ivalsDirty = 1
        if index == None:
            return self.ivals.pop()
        else:
            return self.ivals.pop(index)

    def remove(self, ival):
        # Removes the indicated interval from the list.
        if isinstance(self.ivals, tuple):
            self.ivals = list(self.ivals)
        self.ivals.remove(ival)
        self.__ivalsDirty = 1

    def reverse(self):
        # Reverses the order of the intervals.
        if isinstance(self.ivals, tuple):
            self.ivals = list(self.ivals)
        self.ivals.reverse()
        self.__ivalsDirty = 1

    def sort(self, cmpfunc = None):
        # Sorts the intervals. (?)
        if isinstance(self.ivals, tuple):
            self.ivals = list(self.ivals)
        self.__ivalsDirty = 1
        if cmpfunc == None:
            self.ivals.sort()
        else:
            self.ivals.sort(cmpfunc)

    def __len__(self):
        return len(self.ivals)

    def __getitem__(self, index):
        return self.ivals[index]

    def __setitem__(self, index, value):
        if isinstance(self.ivals, tuple):
            self.ivals = list(self.ivals)
        self.ivals[index] = value
        self.__ivalsDirty = 1
        assert self.validateComponent(value)

    def __delitem__(self, index):
        if isinstance(self.ivals, tuple):
            self.ivals = list(self.ivals)
        del self.ivals[index]
        self.__ivalsDirty = 1

    def __getslice__(self, i, j):
        if isinstance(self.ivals, tuple):
            self.ivals = list(self.ivals)
        return self.__class__(self.ivals[i: j])

    def __setslice__(self, i, j, s):
        if isinstance(self.ivals, tuple):
            self.ivals = list(self.ivals)
        self.ivals[i: j] = s
        self.__ivalsDirty = 1
        assert self.validateComponents(s)

    def __delslice__(self, i, j):
        if isinstance(self.ivals, tuple):
            self.ivals = list(self.ivals)
        del self.ivals[i: j]
        self.__ivalsDirty = 1

    def __iadd__(self, other):
        if isinstance(self.ivals, tuple):
            self.ivals = list(self.ivals)
        if isinstance(other, MetaInterval):
            assert self.__class__ == other.__class__
            ivals = other.ivals
        else:
            ivals = list(other)
        self.ivals += ivals
        self.__ivalsDirty = 1
        assert self.validateComponents(ivals)
        return self

    def __add__(self, other):
        copy = self[:]
        copy += other
        return copy

    # Functions to define sequence, parallel, and track behaviors:

    def addSequence(self, list, name, relTime, relTo, duration):
        # Adds the given list of intervals to the MetaInterval to be
        # played one after the other.
        self.pushLevel(name, relTime, relTo)
        for ival in list:
            self.addInterval(ival, 0.0, PREVIOUS_END)
        self.popLevel(duration)

    def addParallel(self, list, name, relTime, relTo, duration):
        # Adds the given list of intervals to the MetaInterval to be
        # played simultaneously; all will start at the same time.
        self.pushLevel(name, relTime, relTo)
        for ival in list:
            self.addInterval(ival, 0.0, TRACK_START)
        self.popLevel(duration)

    def addParallelEndTogether(self, list, name, relTime, relTo, duration):
        # Adds the given list of intervals to the MetaInterval to be
        # played simultaneously; all will end at the same time, but
        # the longest interval will be started first to achieve this.

        maxDuration = 0
        for ival in list:
            maxDuration = max(maxDuration, ival.getDuration())

        self.pushLevel(name, relTime, relTo)
        for ival in list:
            self.addInterval(ival, maxDuration - ival.getDuration(), TRACK_START)
        self.popLevel(duration)

    def addTrack(self, trackList, name, relTime, relTo, duration):
        # Adds a "track list".  This is a list of tuples of the form:
        #
        #   (<delay>, <Interval>,
        #       PREVIOUS_END | PREVIOUS_START | TRACK_START)
        #
        # where <delay> is a relative time, in seconds, for the
        # <Interval> to start, relative to either the end of the
        # previous interval (PREVIOUS_END), the start of the previous
        # interval (PREVIOUS_START) or the start of the track list
        # (TRACK_START).  If the relative code is omitted, the default
        # is TRACK_START.
        self.pushLevel(name, relTime, relTo)
        for tupleObj in trackList:
            if isinstance(tupleObj, tuple) or \
               isinstance(tupleObj, list):
                relTime = tupleObj[0]
                ival = tupleObj[1]
                if len(tupleObj) >= 3:
                    relTo = tupleObj[2]
                else:
                    relTo = TRACK_START
                self.addInterval(ival, relTime, relTo)

            else:
                self.notify.error("Not a tuple in Track: %s" % (tupleObj,))
        self.popLevel(duration)

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
            if self.pstats:
                ival.pstats = PStatCollector(self.pstats, ival.pname)
            self.addExtIndex(index, ival.getName(), ival.getDuration(),
                             ival.getOpenEnded(), relTime, relTo)

        else:
            self.notify.error("Not an Interval: %s" % (ival,))

    # Functions to support automatic playback of MetaIntervals along
    # with all of their associated Python callbacks:

    def setManager(self, manager):
        self.__manager = manager
        CMetaInterval.setManager(self, manager)

    def getManager(self):
        return self.__manager

    def setT(self, t):
        self.__updateIvals()
        CMetaInterval.setT(self, t)

    def start(self, startT = 0.0, endT = -1.0, playRate = 1.0):
        self.__updateIvals()
        self.setupPlay(startT, endT, playRate, 0)
        self.__manager.addInterval(self)

    def loop(self, startT = 0.0, endT = -1.0, playRate = 1.0):
        self.__updateIvals()
        self.setupPlay(startT, endT, playRate, 1)
        self.__manager.addInterval(self)

    def pause(self):
        if self.getState() == CInterval.SStarted:
            self.privInterrupt()
        self.__manager.removeInterval(self)
        self.privPostEvent()
        return self.getT()

    def resume(self, startT = None):
        self.__updateIvals()
        if startT != None:
            self.setT(startT)
        self.setupResume()
        self.__manager.addInterval(self)

    def resumeUntil(self, endT):
        self.__updateIvals()
        self.setupResumeUntil(endT)
        self.__manager.addInterval(self)

    def finish(self):
        self.__updateIvals()
        state = self.getState()
        if state == CInterval.SInitial:
            self.privInstant()
        elif state != CInterval.SFinal:
            self.privFinalize()
        self.__manager.removeInterval(self)
        self.privPostEvent()

    def clearToInitial(self):
        # This is overloaded at the Python level to properly call
        # pause() at the Python level, then upcall to finish the job
        # at the C++ level.
        self.pause()
        CMetaInterval.clearToInitial(self)

    # Internal functions:

    def validateComponent(self, component):
        # This is called only in debug mode to verify that the
        # indicated component added to the MetaInterval is appropriate
        # to this type of MetaInterval.  In most cases except Track,
        # this is the same as asking that the component is itself an
        # Interval.
        return isinstance(component, CInterval) or \
               isinstance(component, Interval.Interval)

    def validateComponents(self, components):
        # This is called only in debug mode to verify that all the
        # components on the indicated list are appropriate to this
        # type of MetaInterval.
        for component in components:
            if not self.validateComponent(component):
                return 0
        return 1

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

    def setPlayRate(self, playRate):
        """ Changes the play rate of the interval.  If the interval is
        already started, this changes its speed on-the-fly.  Note that
        since playRate is a parameter to start() and loop(), the next
        call to start() or loop() will reset this parameter. """

        if self.isPlaying():
            self.pause()
            CMetaInterval.setPlayRate(self, playRate)
            self.resume()
        else:
            CMetaInterval.setPlayRate(self, playRate)

    def __doPythonCallbacks(self):
        # This function invokes any Python-level Intervals that need
        # to be invoked at this point in time.  It must be called
        # after any call to setT() or setFinalT() or stepPlay(), or
        # some such; basically any function that might invoke an
        # interval.  The C++ base class will invoke whatever C++
        # intervals it can, and then indicate the Python intervals
        # that must be invoked through this interface.

        ival = None
        try:
            while (self.isEventReady()):
                index = self.getEventIndex()
                t = self.getEventT()
                eventType = self.getEventType()
                self.popEvent()

                ival = self.pythonIvals[index]
                ival.privDoEvent(t, eventType)
                ival.privPostEvent()
                ival = None
        except:
            if ival != None:
                print("Exception occurred while processing %s of %s:" % (ival.getName(), self.getName()))
            else:
                print("Exception occurred while processing %s:" % (self.getName()))
            print(self)
            raise

    def privDoEvent(self, t, event):
        # This function overrides the C++ function to initialize the
        # intervals first if necessary.
        if self.pstats:
            self.pstats.start()
        self.__updateIvals()
        CMetaInterval.privDoEvent(self, t, event)
        if self.pstats:
            self.pstats.stop()

    def privPostEvent(self):
        if self.pstats:
            self.pstats.start()
        self.__doPythonCallbacks()
        CMetaInterval.privPostEvent(self)
        if self.pstats:
            self.pstats.stop()

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


    def timeline(self, out = None):
        # This function overrides from the parent level to force it to
        # update the interval list first, if necessary.

        self.__updateIvals()
        if out == None:
            out = ostream
        CMetaInterval.timeline(self, out)

    add_sequence = addSequence
    add_parallel = addParallel
    add_parallel_end_together = addParallelEndTogether
    add_track = addTrack
    add_interval = addInterval
    set_manager = setManager
    get_manager = getManager
    set_t = setT
    resume_until = resumeUntil
    clear_to_initial = clearToInitial
    clear_intervals = clearIntervals
    set_play_rate = setPlayRate
    priv_do_event = privDoEvent
    priv_post_event = privPostEvent
    set_interval_start_time = setIntervalStartTime
    get_interval_start_time = getIntervalStartTime
    get_duration = getDuration


class Sequence(MetaInterval):
    def applyIvals(self, meta, relTime, relTo):
        meta.addSequence(self.ivals, self.getName(),
                         relTime, relTo, self.phonyDuration)
class Parallel(MetaInterval):
    def applyIvals(self, meta, relTime, relTo):
        meta.addParallel(self.ivals, self.getName(),
                         relTime, relTo, self.phonyDuration)

class ParallelEndTogether(MetaInterval):
    def applyIvals(self, meta, relTime, relTo):
        meta.addParallelEndTogether(self.ivals, self.getName(),
                         relTime, relTo, self.phonyDuration)

class Track(MetaInterval):
    def applyIvals(self, meta, relTime, relTo):
        meta.addTrack(self.ivals, self.getName(),
                      relTime, relTo, self.phonyDuration)

    def validateComponent(self, tupleObj):
        # This is called only in debug mode to verify that the
        # indicated component added to the MetaInterval is appropriate
        # to this type of MetaInterval.  In most cases except Track,
        # this is the same as asking that the component is itself an
        # Interval.

        if not (isinstance(tupleObj, tuple) or \
                isinstance(tupleObj, list)):
            # It's not a tuple.
            return 0

        relTime = tupleObj[0]
        ival = tupleObj[1]
        if len(tupleObj) >= 3:
            relTo = tupleObj[2]
        else:
            relTo = TRACK_START

        if not (isinstance(relTime, float) or \
                isinstance(relTime, int)):
            # First parameter is not a number.
            return 0
        if not MetaInterval.validateComponent(self, ival):
            # Second parameter is not an interval.
            return 0
        if relTo != PREVIOUS_END and \
           relTo != PREVIOUS_START and \
           relTo != TRACK_START:
            # Third parameter is an invalid value.
            return 0

        # Looks good.
        return 1
