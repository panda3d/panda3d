"""Interval module: contains the Interval class"""

__all__ = ['Interval']

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.DirectObject import DirectObject
from direct.task.Task import Task, TaskManager
from direct.task.TaskManagerGlobal import taskMgr
from panda3d.core import *
from panda3d.direct import *
from direct.extensions_native import CInterval_extensions
from direct.extensions_native import NodePath_extensions
import math

class Interval(DirectObject):
    """Interval class: Base class for timeline functionality"""

    # create Interval DirectNotify category
    notify = directNotify.newCategory("Interval")

    playbackCounter = 0

    # Class methods
    def __init__(self, name, duration, openEnded=1):
        self.name = name
        self.duration = max(duration, 0.0)
        self.state = CInterval.SInitial
        self.currT = 0.0
        self.doneEvent = None
        self.setTHooks = []
        self.__startT = 0
        self.__startTAtStart = 1
        self.__endT = duration
        self.__endTAtEnd = 1
        self.__playRate = 1.0
        self.__doLoop = 0
        self.__loopCount = 0

        self.pstats = None
        if __debug__ and TaskManager.taskTimerVerbose:
            self.pname = name.split('-', 1)[0]
            self.pstats = PStatCollector("App:Show code:ivalLoop:%s" % (self.pname))

        # Set true if the interval should be invoked if it was
        # completely skipped over during initialize or finalize, false
        # if it should be ignored in this case.
        self.openEnded = openEnded

    def getName(self):
        return self.name

    def getDuration(self):
        return self.duration

    def getOpenEnded(self):
        return self.openEnded

    def setLoop(self, loop=1):
        self.__doLoop = loop

    def getLoop(self):
        return self.__doLoop

    def getState(self):
        return self.state

    def isPaused(self):
        return self.getState() == CInterval.SPaused

    def isStopped(self):
        # Returns true if the interval has not been started, has already
        # played to its completion, or has been explicitly stopped via
        # finish().
        return (self.getState() == CInterval.SInitial or \
                self.getState() == CInterval.SFinal)

    def setT(self, t):
        # There doesn't seem to be any reason to clamp this, and it
        # breaks looping intervals.  The interval code should properly
        # handle t values outside the proper range.
        #t = min(max(t, 0.0), self.getDuration())

        state = self.getState()
        if state == CInterval.SInitial:
            self.privInitialize(t)
            if self.isPlaying():
                self.setupResume()
            else:
                self.privInterrupt()
        elif state == CInterval.SStarted:
            # Support modifying t while the interval is playing.  We
            # assume is_playing() will be true in this state.
            assert self.isPlaying()
            self.privInterrupt()
            self.privStep(t)
            self.setupResume()
        elif state == CInterval.SPaused:
            # Support modifying t while the interval is paused.  In
            # this case, we simply step to the new value of t; but
            # this will change the state to S_started, so we must then
            # change it back to S_paused by hand (because we're still
            # paused).
            self.privStep(t)
            self.privInterrupt()
        elif state == CInterval.SFinal:
            self.privReverseInitialize(t)
            if self.isPlaying():
                self.setupResume()
            else:
                self.privInterrupt()
        else:
            self.notify.error("Invalid state: %s" % (state))
        self.privPostEvent()

    def getT(self):
        return self.currT

    def start(self, startT = 0.0, endT = -1.0, playRate = 1.0):
        """ Starts the interval.  Returns an awaitable. """
        self.setupPlay(startT, endT, playRate, 0)
        return self.__spawnTask()

    def loop(self, startT = 0.0, endT = -1.0, playRate = 1.0):
        self.setupPlay(startT, endT, playRate, 1)
        self.__spawnTask()

    def pause(self):
        if self.getState() == CInterval.SStarted:
            self.privInterrupt()
        self.privPostEvent()
        self.__removeTask()
        return self.getT()

    def resume(self, startT = None):
        if startT != None:
            self.setT(startT)
        self.setupResume()
        if not self.isPlaying():
            self.__spawnTask()

    def resumeUntil(self, endT):
        duration = self.getDuration()

        if endT < 0 or endT >= duration:
            self.__endT = duration
            self.__endTAtEnd = 1
        else:
            self.__endT = endT
            self.__endTAtEnd = 0

        self.setupResume()
        if not self.isPlaying():
            self.__spawnTask()

    def finish(self):
        state = self.getState()
        if state == CInterval.SInitial:
            self.privInstant()
        elif state != CInterval.SFinal:
            self.privFinalize()
        self.privPostEvent()
        self.__removeTask()

    def clearToInitial(self):
        # This method resets the interval's internal state to the
        # initial state, abandoning any parts of the interval that
        # have not yet been called.  Calling it is like pausing the
        # interval and creating a new one in its place.
        self.pause()
        self.state = CInterval.SInitial
        self.currT = 0.0

    def isPlaying(self):
        return taskMgr.hasTaskNamed(self.getName() + '-play')

    def getPlayRate(self):
        """ Returns the play rate as set by the last call to start(),
        loop(), or setPlayRate(). """
        return self.__playRate

    def setPlayRate(self, playRate):
        """ Changes the play rate of the interval.  If the interval is
        already started, this changes its speed on-the-fly.  Note that
        since playRate is a parameter to start() and loop(), the next
        call to start() or loop() will reset this parameter. """

        if self.isPlaying():
            self.pause()
            self.__playRate = playRate
            self.resume()
        else:
            self.__playRate = playRate

    def setDoneEvent(self, event):
        self.doneEvent = event

    def getDoneEvent(self):
        return self.doneEvent

    def privDoEvent(self, t, event):
        if self.pstats:
            self.pstats.start()
        if event == CInterval.ETStep:
            self.privStep(t)
        elif event == CInterval.ETFinalize:
            self.privFinalize()
        elif event == CInterval.ETInterrupt:
            self.privInterrupt()
        elif event == CInterval.ETInstant:
            self.privInstant()
        elif event == CInterval.ETInitialize:
            self.privInitialize(t)
        elif event == CInterval.ETReverseFinalize:
            self.privReverseFinalize()
        elif event == CInterval.ETReverseInstant:
            self.privReverseInstant()
        elif event == CInterval.ETReverseInitialize:
            self.privReverseInitialize(t)
        else:
            self.notify.error('Invalid event type: %s' % (event))
        if self.pstats:
            self.pstats.stop()


    def privInitialize(self, t):
        # Subclasses may redefine this function
        self.state = CInterval.SStarted
        self.privStep(t)

    def privInstant(self):
        # Subclasses may redefine this function
        self.state = CInterval.SStarted
        self.privStep(self.getDuration())
        self.state = CInterval.SFinal
        self.intervalDone()

    def privStep(self, t):
        # Subclasses may redefine this function
        self.state = CInterval.SStarted
        self.currT = t

    def privFinalize(self):
        # Subclasses may redefine this function
        self.privStep(self.getDuration())
        self.state = CInterval.SFinal
        self.intervalDone()

    def privReverseInitialize(self, t):
        # Subclasses may redefine this function
        self.state = CInterval.SStarted
        self.privStep(t)

    def privReverseInstant(self):
        # Subclasses may redefine this function
        self.state = CInterval.SStarted
        self.privStep(0)
        self.state = CInterval.SInitial

    def privReverseFinalize(self):
        # Subclasses may redefine this function
        self.privStep(0)
        self.state = CInterval.SInitial

    def privInterrupt(self):
        # Subclasses may redefine this function
        self.state = CInterval.SPaused

    def intervalDone(self):
        # Subclasses should call this when the interval transitions to
        # its final state.
        if self.doneEvent:
            messenger.send(self.doneEvent)

    def setupPlay(self, startT, endT, playRate, doLoop):
        duration = self.getDuration()

        if startT <= 0:
            self.__startT = 0
            self.__startTAtStart = 1
        elif startT > duration:
            self.__startT = duration
            self.__startTAtStart = 0
        else:
            self.__startT = startT
            self.__startTAtStart = 0

        if endT < 0 or endT >= duration:
            self.__endT = duration
            self.__endTAtEnd = 1
        else:
            self.__endT = endT
            self.__endTAtEnd = 0

        self.__clockStart = ClockObject.getGlobalClock().getFrameTime()
        self.__playRate = playRate
        self.__doLoop = doLoop
        self.__loopCount = 0

    def setupResume(self):
        now = ClockObject.getGlobalClock().getFrameTime()
        if self.__playRate > 0:
            self.__clockStart = now - ((self.getT() - self.__startT) / self.__playRate)
        elif self.__playRate < 0:
            self.__clockStart = now - ((self.getT() - self.__endT) / self.__playRate)
        self.__loopCount = 0

    def stepPlay(self):
        now = ClockObject.getGlobalClock().getFrameTime()
        if self.__playRate >= 0:
            t = (now - self.__clockStart) * self.__playRate + self.__startT

            if self.__endTAtEnd:
                self.__endT = self.getDuration()

            if t < self.__endT:
                # In the middle of the interval, not a problem.
                if self.isStopped():
                    self.privInitialize(t)
                else:
                    self.privStep(t)

            else:
                # Past the ending point; time to finalize.
                if self.__endTAtEnd:
                    # Only finalize if the playback cycle includes the
                    # whole interval.
                    if self.isStopped():
                        if self.getOpenEnded() or self.__loopCount != 0:
                            self.privInstant()
                    else:
                        self.privFinalize()
                else:
                    if self.isStopped():
                        self.privInitialize(self.__endT)
                    else:
                        self.privStep(self.__endT)

                # Advance the clock for the next loop cycle.
                if self.__endT == self.__startT:
                    # If the interval has no length, we loop exactly once.
                    self.__loopCount += 1

                else:
                    # Otherwise, figure out how many loops we need to
                    # skip.
                    timePerLoop = (self.__endT - self.__startT) / self.__playRate
                    numLoops = math.floor((now - self.__clockStart) / timePerLoop)
                    self.__loopCount += numLoops
                    self.__clockStart += numLoops * timePerLoop

        else:
            # Playing backwards
            t = (now - self.__clockStart) * self.__playRate + self.__endT

            if t >= self.__startT:
                # In the middle of the interval, not a problem.
                if self.isStopped():
                    self.privInitialize(t)
                else:
                    self.privStep(t)
            else:
                # Past the ending point; time to finalize.
                if self.__startTAtStart:
                    # Only finalize if the playback cycle includes the
                    # whole interval.
                    if self.isStopped():
                        if self.getOpenEnded() or self.__loopCount != 0:
                            self.privReverseInstant()
                    else:
                        self.privReverseFinalize()
                else:
                    if self.isStopped():
                        self.privReverseInitialize(self.__startT)
                    else:
                        self.privStep(self.__startT)

                # Advance the clock for the next loop cycle.
                if self.__endT == self.__startT:
                    # If the interval has no length, we loop exactly once.
                    self.__loopCount += 1

                else:
                    # Otherwise, figure out how many loops we need to
                    # skip.
                    timePerLoop = (self.__endT - self.__startT) / -self.__playRate
                    numLoops = math.floor((now - self.__clockStart) / timePerLoop)
                    self.__loopCount += numLoops
                    self.__clockStart += numLoops * timePerLoop

        shouldContinue = (self.__loopCount == 0 or self.__doLoop)

        if (not shouldContinue and self.getState() == CInterval.SStarted):
            self.privInterrupt()

        return shouldContinue

    def __repr__(self, indent=0):
        space = ''
        for l in range(indent):
            space = space + ' '
        return (space + self.name + ' dur: %.2f' % self.duration)

    open_ended = property(getOpenEnded)
    stopped = property(isStopped)
    t = property(getT, setT)
    play_rate = property(getPlayRate, setPlayRate)
    done_event = property(getDoneEvent, setDoneEvent)

    # The rest of these methods are duplicates of functions defined
    # for the CInterval class via the file CInterval-extensions.py.

    def privPostEvent(self):
        # Call after calling any of the priv* methods to do any required
        # Python finishing steps.
        if self.pstats:
            self.pstats.start()
        t = self.getT()
        if hasattr(self, "setTHooks"):
            for func in self.setTHooks:
                func(t)
        if self.pstats:
            self.pstats.stop()

    def __spawnTask(self):
        # Spawn task
        self.__removeTask()
        taskName = self.getName() + '-play'
        task = Task(self.__playTask)
        task.interval = self
        taskMgr.add(task, taskName)
        return task

    def __removeTask(self):
        # Kill old task(s), including those from a similarly-named but
        # different interval.
        taskName = self.getName() + '-play'
        oldTasks = taskMgr.getTasksNamed(taskName)
        for task in oldTasks:
            if hasattr(task, "interval"):
                task.interval.privInterrupt()
                taskMgr.remove(task)

    def __playTask(self, task):
        again = self.stepPlay()
        self.privPostEvent()
        if again:
            return Task.cont
        else:
            return Task.done

    def popupControls(self, tl = None):
        """
        Popup control panel for interval.
        """
        # Don't use a regular import, to prevent ModuleFinder from picking
        # it up as a dependency when building a .p3d package.
        import importlib
        EntryScale = importlib.import_module('direct.tkwidgets.EntryScale')
        tkinter = importlib.import_module('tkinter')

        if tl == None:
            tl = tkinter.Toplevel()
            tl.title('Interval Controls')
        outerFrame = tkinter.Frame(tl)
        def entryScaleCommand(t, s=self):
            s.setT(t)
            s.pause()
        self.es = es = EntryScale.EntryScale(
            outerFrame, text = self.getName(),
            min = 0, max = math.floor(self.getDuration() * 100) / 100,
            command = entryScaleCommand)
        es.set(self.getT(), fCommand = 0)
        es.pack(expand = 1, fill = tkinter.X)
        bf = tkinter.Frame(outerFrame)
        # Jump to start and end
        def toStart(s=self, es=es):
            s.clearToInitial()
            es.set(0, fCommand = 0)
        def toEnd(s=self):
            s.pause()
            s.setT(s.getDuration())
            es.set(s.getDuration(), fCommand = 0)
            s.pause()
        jumpToStart = tkinter.Button(bf, text = '<<', command = toStart)
        # Stop/play buttons
        def doPlay(s=self, es=es):
            s.resume(es.get())

        stop = tkinter.Button(bf, text = 'Stop',
                      command = lambda s=self: s.pause())
        play = tkinter.Button(
            bf, text = 'Play',
            command = doPlay)
        jumpToEnd = tkinter.Button(bf, text = '>>', command = toEnd)
        jumpToStart.pack(side = tkinter.LEFT, expand = 1, fill = tkinter.X)
        play.pack(side = tkinter.LEFT, expand = 1, fill = tkinter.X)
        stop.pack(side = tkinter.LEFT, expand = 1, fill = tkinter.X)
        jumpToEnd.pack(side = tkinter.LEFT, expand = 1, fill = tkinter.X)
        bf.pack(expand = 1, fill = tkinter.X)
        outerFrame.pack(expand = 1, fill = tkinter.X)
        # Add function to update slider during setT calls
        def update(t, es=es):
            es.set(t, fCommand = 0)
        if not hasattr(self, "setTHooks"):
            self.setTHooks = []
        self.setTHooks.append(update)
        # Clear out function on destroy
        def onDestroy(e, s=self, u=update):
            if u in s.setTHooks:
                s.setTHooks.remove(u)
        tl.bind('<Destroy>', onDestroy)
