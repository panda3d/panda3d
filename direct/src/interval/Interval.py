"""Interval module: contains the Interval class"""

from DirectObject import *
import ClockObject
import Task

# Interval events
IVAL_NONE = 0
IVAL_INIT = 1
IVAL_STOP = 2

class Interval(DirectObject):
    """Interval class: Base class for timeline functionality"""

    # create Interval DirectNotify category
    notify = directNotify.newCategory("Interval")
    #notify.setDebug(1)

    # Class methods
    def __init__(self, name, duration, openEnded = 1):
        """__init__(name, duration)
        """
	self.name = name
	self.duration = duration
	self.clock = ClockObject.ClockObject.getGlobalClock()
	self.curr_t = 0.0
	self.prev_t = 0.0
        self.setTHooks = []
        # Set true if interval responds to setT(t): t>duration
        self.fOpenEnded = openEnded

    def __del__(self):
	print 'Interval destructing'

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
	self.setT(self.getDuration(), event=IVAL_NONE)

    def play(self, t0=0.0, duration=0.0, scale=1.0):
        """ play(t0, duration)
        """
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

    def stop(self):
        """ stop()
        """
        # Kill task
        taskMgr.removeTasksNamed(self.name + '-play')
        # And stop freerunning (e.g. sound and anim) intervals
        self.setT(self.curr_t, event = IVAL_STOP)
	return self.curr_t

    def __playTask(self, task):
        """ __playTask(task)
        """
        t = self.clock.getFrameTime()
        te = self.offset + ((t - self.startT) * self.scale)
        if (te < self.endTime):
            # If first call, init intervals
	    if (self.firstTime):
		self.setT(te, event = IVAL_INIT)
		self.firstTime = 0
	    else:
            	self.setT(te)
            return Task.cont
        else:
	    self.setT(self.endTime)
            return Task.done

    def __repr__(self, indent=0):
	""" __repr__(indent)
	"""
	space = ''
	for l in range(indent):
	    space = space + ' '
	return (space + self.name + ' dur: %.2f' % self.duration)

    def popupControls(self):
        """ popupControls()
            Popup control panel for interval.
        """
        import fpformat
        import string
        # I moved this here because Toontown does not ship Tk
        from Tkinter import *
        import Pmw
        import EntryScale
        tl = Toplevel()
        tl.title(self.getName() + ' Interval Controls')
        self.es = es = EntryScale.EntryScale(
            tl, text = 'Time',
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
        es.onReturnRelease = lambda s=self, es = es: s.setT(es.get(),
                                                            event = IVAL_INIT)
        es.pack(expand = 1, fill = X)
        f = Frame(tl)
        # Jump to start and end
        def toStart(s=self, es=es):
            s.stop()
            s.setT(0.0, event = IVAL_INIT)
        def toEnd(s=self):
            s.stop()
            s.setT(s.getDuration(), event = IVAL_INIT)
        jumpToStart = Button(tl, text = '<<', command = toStart)
        # Stop/play buttons
        stop = Button(tl, text = 'Stop',
                      command = lambda s=self: s.stop())
        play = Button(
            tl, text = 'Play',
            command = lambda s=self, es=es: s.play(es.get()))
        jumpToEnd = Button(tl, text = '>>', command = toEnd)
        jumpToStart.pack(side = LEFT, expand = 1, fill = X)
        play.pack(side = LEFT, expand = 1, fill = X)
        stop.pack(side = LEFT, expand = 1, fill = X)
        jumpToEnd.pack(side = LEFT, expand = 1, fill = X)
        f.pack(expand = 1, fill = X)
        # Add function to update slider during setT calls
        def update(t,es=es):
            es.set(t, fCommand = 0)
        self.setTHooks.append(update)
        # Clear out function on destroy
        def onDestroy(e, s=self, u=update):
            if u in s.setTHooks:
                s.setTHooks.remove(u)
        tl.bind('<Destroy>', onDestroy)

        
        
