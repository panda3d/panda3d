"""Interval module: contains the Interval class"""

from DirectObject import *
from IntervalGlobal import *
from PandaModules import *
import Task

# Interval events
IVAL_NONE = 0
IVAL_INIT = 1
IVAL_DONE = 2

class Interval(DirectObject):
    """Interval class: Base class for timeline functionality"""

    # create Interval DirectNotify category
    notify = directNotify.newCategory("Interval")

    playbackCounter = 0

    clock = ClockObject.getGlobalClock()

    # Class methods
    def __init__(self, name, duration, openEnded=1, reverse=0):
        """__init__(name, duration, openEnded, reverse)
        """
        self.name = name
        self.duration = duration
        self.curr_t = 0.0
        self.prev_t = -1
        self.stopEventList = []
        self.setTHooks = []
        # Set true if interval responds to setT(t): t>duration
        self.fOpenEnded = openEnded
        self.reverse = reverse

    def getName(self):
        """ getName()
        """
        return self.name

    def getDuration(self):
        """ getDuration()
        """
        return self.duration

    def setfOpenEnded(self, openEnded):
        """ setfOpenEnded(openEnded)
        """
        self.fOpenEnded = openEnded

    def getfOpenEnded(self):
        """ getfOpenEnded()
        """
        return self.fOpenEnded

    def setT(self, t, event = IVAL_NONE):
        """ setT(t, event)
            Go to time t
        """
        # Update current time
        self.curr_t = t
        # Perform interval actions
        self.updateFunc(t, event)
        # Call setT Hook
        for func in self.setTHooks:
            func(t)
        # Record t for next time around
        self.prev_t = self.curr_t

    def getT(self):
        return self.curr_t

    def updateFunc(self, t, event = IVAL_NONE):
        """ updateFunc(t, event)
        """
        # Subclasses define this function
        pass

    def setTHook(self, t):
        # Used by control panel to update scale
        pass

    def setFinalT(self):
        """ setFinalT()
        """
        self.setT(self.getDuration(), event=IVAL_DONE)

    def play(self, t0=0.0, duration=0.0, scale=1.0):
        """ play(t0, duration)
        """
        # Make sure the start time is sensible.
        if t0 > self.duration:
            t0 = self.duration

        # Kill ongoing play task
        taskMgr.removeTasksNamed(self.name + '-play')
        # Start new one
        self.offset = t0
        self.startT = self.clock.getFrameTime()
        assert(scale > 0.0)
        self.scale = scale
        self.firstTime = 1
        if (duration == 0.0):
            # If no play duration specified, use duration of Interval
            self.endTime = self.duration
        else:
            # Otherwise use min of interval duration and offset + play duration
            self.endTime = min(self.duration, self.offset + duration)
        assert(t0 <= self.endTime)

        # Spawn task
        taskMgr.spawnMethodNamed(self.__playTask, self.name + '-play')

    def loop(self, t0=0.0, duration=0.0, scale=1.0):
        self.accept(self.name + '-loop', self.play,
                    extraArgs=[t0, duration, scale])
        self.play(t0, duration, scale)
        return

    def stop(self):
        """ stop()
        """
        # First send event to stop freerunning (e.g. sound and anim) intervals
        for stopEvent in self.stopEventList:
            messenger.send(stopEvent)
        # Kill task
        taskMgr.removeTasksNamed(self.name + '-play')
        # No more looping.
        self.ignore(self.name + '-loop')
        return self.curr_t

    def isPlaying(self):
        return taskMgr.hasTaskNamed(self.name + '-play')

    def __playTask(self, task):
        """ __playTask(task)
        """
        t = self.clock.getFrameTime()
        te = self.offset + ((t - self.startT) * self.scale)
        if (te < self.endTime):
            if (self.firstTime):
                # If first call, init intervals
                self.setT(te, event = IVAL_INIT)
                self.firstTime = 0
            else:
                self.setT(te)
            return Task.cont
        else:
            te = self.endTime
            if (self.firstTime):
                # If first call, init intervals
                self.setT(te, event = IVAL_INIT)
                self.firstTime = 0
            else:
                self.setT(te, IVAL_DONE)
            messenger.send(self.name + "-loop")
            return Task.done

    def __repr__(self, indent=0):
        """ __repr__(indent)
        """
        space = ''
        for l in range(indent):
            space = space + ' '
        return (space + self.name + ' dur: %.2f' % self.duration)

    def popupControls(self, tl = None):
        """ popupControls()
            Popup control panel for interval.
        """
        base.wantTk = 1
        import TkGlobal
        import fpformat
        import string
        # I moved this here because Toontown does not ship Tk
        from Tkinter import Toplevel, Frame, Button
        import Pmw
        import EntryScale
        if tl == None:
            tl = Toplevel()
            tl.title('Interval Controls')
        outerFrame = Frame(tl)
        self.es = es = EntryScale.EntryScale(
            outerFrame, text = self.getName(),
            min = 0, max = string.atof(fpformat.fix(self.duration, 2)),
            command = lambda t, s = self: s.setT(t))
        # So when you drag scale with mouse its like you started a playback
        def onPress(s=self,es=es):
            # Kill playback task
            taskMgr.removeTasksNamed(s.name + '-play')
            # INIT interval
            s.setT(es.get(), event = IVAL_INIT)
        es.onPress = onPress
        # To make sure you stop free running intervals
        es.onRelease = lambda s=self: s.stop()
        # To update scale and execute intervals with IVAL_INIT
        def onReturn(s = self, es = es):
            s.setT(es.get(), event = IVAL_INIT)
            s.stop()
        es.onReturnRelease = onReturn
        es.pack(expand = 1, fill = X)
        bf = Frame(outerFrame)
        # Jump to start and end
        def toStart(s=self, es=es):
            s.setT(0.0, event = IVAL_INIT)
            s.stop()
        def toEnd(s=self):
            s.setT(s.getDuration(), event = IVAL_INIT)
            s.stop()
        jumpToStart = Button(bf, text = '<<', command = toStart)
        # Stop/play buttons
        stop = Button(bf, text = 'Stop',
                      command = lambda s=self: s.stop())
        play = Button(
            bf, text = 'Play',
            command = lambda s=self, es=es: s.play(es.get()))
        jumpToEnd = Button(bf, text = '>>', command = toEnd)
        jumpToStart.pack(side = LEFT, expand = 1, fill = X)
        play.pack(side = LEFT, expand = 1, fill = X)
        stop.pack(side = LEFT, expand = 1, fill = X)
        jumpToEnd.pack(side = LEFT, expand = 1, fill = X)
        bf.pack(expand = 1, fill = X)
        outerFrame.pack(expand = 1, fill = X)
        # Add function to update slider during setT calls
        def update(t,es=es):
            es.set(t, fCommand = 0)
        self.setTHooks.append(update)
        # Clear out function on destroy
        def onDestroy(e, s=self, u=update):
            if u in s.setTHooks:
                s.setTHooks.remove(u)
        tl.bind('<Destroy>', onDestroy)

        
        
