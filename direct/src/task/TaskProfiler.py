from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.fsm.StatePush import FunctionCall
from direct.showbase.PythonUtil import getProfileResultString, Averager

class TaskTracker:
    def __init__(self, namePattern):
        self._namePattern = namePattern
        self._durationAverager = Averager('%s-durationAverager' % namePattern)
        self._avgData = None
        self._avgDataDur = None
        self._maxNonSpikeData = None
        self._maxNonSpikeDataDur = None
    def destroy(self):
        del self._namePattern
        del self._durationAverager
    def flush(self):
        self._durationAverager.reset()
        self._avgData = None
        self._avgDataDur = None
        self._maxNonSpikeData = None
        self._maxNonSpikeDataDur = None
    def getNamePattern(self, namePattern):
        return self._namePattern
    def addDuration(self, duration, data, isSpike):
        self._durationAverager.addValue(duration)
        storeAvgData = True
        storeMaxNSData = True
        if self._avgDataDur is not None:
            avgDur = self.getAvgDuration()
            if abs(self._avgDataDur - avgDur) < abs(duration - avgDur):
                # current avg data is more average than this new sample, keep the data we've
                # already got stored
                storeAvgData = False
        if isSpike:
            storeMaxNSData = False
        else:
            if self._maxNonSpikeDataDur is not None and self._maxNonSpikeDataDur > duration:
                storeMaxNSData = False
        if storeAvgData:
            self._avgData = data
            self._avgDataDur = duration
        if storeMaxNSData:
            self._maxNonSpikeData = data
            self._maxNonSpikeDataDur = duration
    def getAvgDuration(self):
        return self._durationAverager.getAverage()
    def getNumDurationSamples(self):
        return self._durationAverager.getCount()
    def getAvgData(self):
        # returns duration, data for closest-to-average sample
        return self._avgDataDur, self._avgData
    def getMaxNonSpikeData(self):
        # returns duration, data for closest-to-average sample
        return self._maxNonSpikeDataDur, self._maxNonSpikeData

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
        # assign getProfileResultString to the taskMgr here, since Task.py can't import PythonUtil
        taskMgr._getProfileResultString = getProfileResultString

    def destroy(self):
        if taskMgr.getProfileTasks():
            self._setEnabled(False)
        self._enableFC.destroy()
        for tracker in self._namePattern2tracker.itervalues():
            tracker.destroy()
        del self._namePattern2tracker
        del self._task

    def flush(self):
        # flush stored task profiles
        for tracker in self._namePattern2tracker.itervalues():
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
        profileDt = taskMgr._getTaskProfileDt()
        if (self._task is not None) and (profileDt is not None):
            lastProfileResult = taskMgr._getLastProfileResultString()
            if lastProfileResult:
                namePattern = self._task.getNamePattern()
                if namePattern not in self._namePattern2tracker:
                    self._namePattern2tracker[namePattern] = TaskTracker(namePattern)
                tracker = self._namePattern2tracker[namePattern]
                isSpike = False
                # do we have enough samples?
                if tracker.getNumDurationSamples() > self._minSamples:
                    # was this a spike?
                    if profileDt > (tracker.getAvgDuration() * self._spikeThreshold):
                        isSpike = True
                        avgDur, avgResult = tracker.getAvgData()
                        maxNSDur, maxNSResult = tracker.getMaxNonSpikeData()
                        self.notify.info('task CPU spike profile (%s):\n'
                                         '== AVERAGE (%s wall-clock seconds)\n%s\n'
                                         '== LONGEST NON-SPIKE (%s wall-clock seconds)\n%s\n'
                                         '== SPIKE (%s wall-clock seconds)\n%s' % (
                            namePattern,
                            avgDur, avgResult,
                            maxNSDur, maxNSResult,
                            profileDt, lastProfileResult))
                tracker.addDuration(profileDt, lastProfileResult, isSpike)

        # set up the next task
        self._task = taskMgr._getRandomTask()
        taskMgr._setProfileTask(self._task)

        return task.cont
