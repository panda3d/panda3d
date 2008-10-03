from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.fsm.StatePush import FunctionCall
from direct.showbase.PythonUtil import Averager

class TaskTracker:
    # call it TaskProfiler to avoid confusion for the user
    notify = directNotify.newCategory("TaskProfiler")
    def __init__(self, namePattern):
        self._namePattern = namePattern
        self._durationAverager = Averager('%s-durationAverager' % namePattern)
        self._avgSession = None
        self._maxNonSpikeSession = None
    def destroy(self):
        self.flush()
        del self._namePattern
        del self._durationAverager
    def flush(self):
        self._durationAverager.reset()
        if self._avgSession:
            self._avgSession.release()
        if self._maxNonSpikeSession:
            self._maxNonSpikeSession.release()
        self._avgSession = None
        self._maxNonSpikeSession = None
    def getNamePattern(self, namePattern):
        return self._namePattern
    def addProfileSession(self, session, isSpike):
        duration = session.getWallClockDuration()
        self._durationAverager.addValue(duration)
        storeAvg = True
        storeMaxNS = True
        if self._avgSession is not None:
            avgDur = self.getAvgDuration()
            if abs(self._avgSession.getWallClockDuration() - avgDur) < abs(duration - avgDur):
                # current avg data is more average than this new sample, keep the data we've
                # already got stored
                storeAvg = False
        if isSpike:
            storeMaxNS = False
        else:
            if (self._maxNonSpikeSession is not None and
                self._maxNonSpikeSession.getWallClockDuration() > duration):
                storeMaxNS = False
        if storeAvg:
            if self._avgSession:
                self._avgSession.release()
            session.acquire()
            self._avgSession = session
        if storeMaxNS:
            if self._maxNonSpikeSession:
                self._maxNonSpikeSession.release()
            session.acquire()
            self._maxNonSpikeSession = session
    def getAvgDuration(self):
        return self._durationAverager.getAverage()
    def getNumDurationSamples(self):
        return self._durationAverager.getCount()
    def getAvgSession(self):
        # returns profile session for closest-to-average sample
        return self._avgSession
    def getMaxNonSpikeSession(self):
        # returns profile session for closest-to-average sample
        return self._maxNonSpikeSession
    def log(self):
        if self._avgSession:
            self.notify.info('task CPU profile (%s):\n'
                             '== AVERAGE (%s wall-clock seconds)\n%s\n'
                             '== LONGEST NON-SPIKE (%s wall-clock seconds)\n%s' % (
                self._namePattern,
                self._avgSession.getWallClockDuration(),
                self._avgSession.getResults(),
                self._maxNonSpikeSession.getWallClockDuration(),
                self._maxNonSpikeSession.getResults(),
                ))
        else:
            self.notify.info('task CPU profile (%s): no data collected' % self._namePattern)

class TaskProfiler:
    # this does intermittent profiling of tasks running on the system
    # if a task has a spike in execution time, the profile of the spike is logged
    notify = directNotify.newCategory("TaskProfiler")

    def __init__(self):
        self._enableFC = FunctionCall(self._setEnabled, taskMgr.getProfileTasksSV())
        # table of task name pattern to TaskTracker
        self._namePattern2tracker = {}
        self._task = None
        # number of samples required before spikes start getting identified
        self._minSamples = config.GetInt('profile-task-spike-min-samples', 30)
        # defines spike as longer than this multiple of avg task duration
        self._spikeThreshold = config.GetFloat('profile-task-spike-threshold', 10.)

    def destroy(self):
        if taskMgr.getProfileTasks():
            self._setEnabled(False)
        self._enableFC.destroy()
        for tracker in self._namePattern2tracker.itervalues():
            tracker.destroy()
        del self._namePattern2tracker
        del self._task

    def logProfiles(self, name=None):
        if name:
            name = name.lower()
        for namePattern, tracker in self._namePattern2tracker.iteritems():
            if (name and (name not in namePattern.lower())):
                continue
            tracker.log()

    def flush(self, name):
        if name:
            name = name.lower()
        # flush stored task profiles
        for namePattern, tracker in self._namePattern2tracker.iteritems():
            if (name and (name not in namePattern.lower())):
                continue
            tracker.flush()

    def _setEnabled(self, enabled):
        if enabled:
            self._taskName = 'profile-tasks-%s' % id(self)
            taskMgr.add(self._doProfileTasks, self._taskName, priority=-200)
        else:
            taskMgr.remove(self._taskName)
            del self._taskName
        
    def _doProfileTasks(self, task=None):
        # gather data from the previous frame
        # set up for the next frame
        if (self._task is not None) and taskMgr._hasProfiledDesignatedTask():
            session = taskMgr._getLastProfileSession()
            # if we couldn't profile, throw this result out
            if session.profileSucceeded():
                sessionDur = session.getWallClockDuration()
                namePattern = self._task.getNamePattern()
                if namePattern not in self._namePattern2tracker:
                    self._namePattern2tracker[namePattern] = TaskTracker(namePattern)
                tracker = self._namePattern2tracker[namePattern]
                isSpike = False
                # do we have enough samples?
                if tracker.getNumDurationSamples() > self._minSamples:
                    # was this a spike?
                    if sessionDur > (tracker.getAvgDuration() * self._spikeThreshold):
                        print 'sessionDur=%s' % sessionDur
                        print 'avgDur=%s' % tracker.getAvgDuration()
                        isSpike = True
                        avgSession = tracker.getAvgSession()
                        maxNSSession = tracker.getMaxNonSpikeSession()
                        self.notify.info('task CPU spike profile (%s):\n'
                                         '== AVERAGE (%s wall-clock seconds)\n%s\n'
                                         '== LONGEST NON-SPIKE (%s wall-clock seconds)\n%s\n'
                                         '== SPIKE (%s wall-clock seconds)\n%s' % (
                            namePattern,
                            avgSession.getWallClockDuration(), avgSession.getResults(),
                            maxNSSession.getWallClockDuration(), maxNSSession.getResults(),
                            sessionDur, session.getResults()))
                tracker.addProfileSession(session, isSpike)

        # set up the next task
        self._task = taskMgr._getRandomTask()
        taskMgr._setProfileTask(self._task)

        return task.cont
