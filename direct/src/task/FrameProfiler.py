from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.fsm.StatePush import FunctionCall
from direct.showbase.PythonUtil import formatTimeExact, normalDistrib
from direct.task import Task

class FrameProfiler:
    notify = directNotify.newCategory('FrameProfiler')

    # because of precision requirements, all times related to the profile/log
    # schedule are stored as integers
    Minute = 60
    Hour = 60 * Minute
    Day = 24 * Hour

    def __init__(self):
        Hour = FrameProfiler.Hour
        # how long to wait between frame profiles
        self._period = 2 * FrameProfiler.Minute
        if config.GetBool('frequent-frame-profiles', 0):
            self._period = 1 * FrameProfiler.Minute
        # used to prevent profile from being taken exactly every 'period' seconds
        self._jitterMagnitude = self._period * .75
        # when to log output
        # each entry must be an integer multiple of all previous entries
        # as well as an integer multiple of the period
        self._logSchedule = [ 1 * FrameProfiler.Hour,
                              4 * FrameProfiler.Hour,
                             12 * FrameProfiler.Hour,
                              1 * FrameProfiler.Day,
                              ] # day schedule proceeds as 1, 2, 4, 8 days, etc.
        if config.GetBool('frequent-frame-profiles', 0):
            self._logSchedule = [ 1  * FrameProfiler.Minute,
                                  4  * FrameProfiler.Minute,
                                  12 * FrameProfiler.Minute,
                                  24 * FrameProfiler.Minute,
                                  ]
        for t in self._logSchedule:
            #assert isInteger(t)
            # make sure the period is evenly divisible into each element of the log schedule
            assert (t % self._period) == 0
        # make sure each element of the schedule is evenly divisible into each subsequent element
        for i in range(len(self._logSchedule)):
            e = self._logSchedule[i]
            for j in range(i, len(self._logSchedule)):
                assert (self._logSchedule[j] % e) == 0
        #assert isInteger(self._period)
        self._enableFC = FunctionCall(self._setEnabled, taskMgr.getProfileFramesSV())
        self._enableFC.pushCurrentState()

    def destroy(self):
        self._enableFC.set(False)
        self._enableFC.destroy()

    def _setEnabled(self, enabled):
        if enabled:
            self.notify.info('frame profiler started')
            self._startTime = globalClock.getFrameTime()
            self._profileCounter = 0
            self._jitter = None
            self._period2aggregateProfile = {}
            self._id2session = {}
            self._id2task = {}
            # don't profile process startup
            self._task = taskMgr.doMethodLater(self._period, self._scheduleNextProfileDoLater,
                                               'FrameProfilerStart-%s' % serialNum())
        else:
            self._task.remove()
            del self._task
            for session in self._period2aggregateProfile.values():
                session.release()
            del self._period2aggregateProfile
            for task in self._id2task.values():
                task.remove()
            del self._id2task
            for session in self._id2session.values():
                session.release()
            del self._id2session
            self.notify.info('frame profiler stopped')

    def _scheduleNextProfileDoLater(self, task):
        self._scheduleNextProfile()
        return task.done

    def _scheduleNextProfile(self):
        self._profileCounter += 1
        self._timeElapsed = self._profileCounter * self._period
        #assert isInteger(self._timeElapsed)
        time = self._startTime + self._timeElapsed

        # vary the actual delay between profiles by a random amount to prevent interaction
        # with periodic events
        jitter = self._jitter
        if jitter is None:
            jitter = normalDistrib(-self._jitterMagnitude, self._jitterMagnitude)
            time += jitter
        else:
            time -= jitter
            jitter = None
        self._jitter = jitter

        sessionId = serialNum()
        session = taskMgr.getProfileSession('FrameProfile-%s' % sessionId)
        self._id2session[sessionId] = session
        taskMgr.profileFrames(num=1, session=session, callback=Functor(
            self._analyzeResults, sessionId))

        # schedule the next profile
        delay = max(time - globalClock.getFrameTime(), 0.)
        self._task = taskMgr.doMethodLater(delay, self._scheduleNextProfileDoLater,
                                           'FrameProfiler-%s' % serialNum())

    def _analyzeResults(self, sessionId):
        # do the analysis in a task 1) to separate the processing from the profiled frame,
        # and 2) to get the processing to show up in a named task instead of in the taskMgr
        self._id2task[sessionId] = taskMgr.add(
            Functor(self._doAnalysis, sessionId), 'FrameProfilerAnalysis-%s' % sessionId)

    def _doAnalysis(self, sessionId, task):
        if hasattr(task, '_generator'):
            gen = task._generator
        else:
            gen = self._doAnalysisGen(sessionId)
            task._generator = gen
        result = next(gen)
        if result == Task.done:
            del task._generator
        return result

    def _doAnalysisGen(self, sessionId):
        # generator to limit max number of profile loggings per frame
        p2ap = self._period2aggregateProfile

        self._id2task.pop(sessionId)
        session = self._id2session.pop(sessionId)

        if session.profileSucceeded():
            # always add this profile to the first aggregated profile
            period = self._logSchedule[0]
            if period not in self._period2aggregateProfile:
                p2ap[period] = session.getReference()
            else:
                p2ap[period].aggregate(session)
        else:
            self.notify.warning('frame profile did not succeed')

        session.release()
        session = None

        counter = 0

        # log profiles when it's time, and aggregate them upwards into the
        # next-longer profile
        for pi in range(len(self._logSchedule)):
            period = self._logSchedule[pi]
            if (self._timeElapsed % period) == 0:
                if period in p2ap:
                    # delay until the next frame if we've already processed N profiles this frame
                    if counter >= 3:
                        counter = 0
                        yield Task.cont
                    self.notify.info('aggregate profile of sampled frames over last %s\n%s' %
                                     (formatTimeExact(period), p2ap[period].getResults()))
                    counter += 1
                    # aggregate this profile into the next larger profile
                    nextIndex = pi + 1
                    if nextIndex >= len(self._logSchedule):
                        # if we're adding a new period to the end of the log period table,
                        # set it to double the duration of the current longest period
                        nextPeriod = period * 2
                        self._logSchedule.append(nextPeriod)
                    else:
                        nextPeriod = self._logSchedule[nextIndex]
                    if nextPeriod not in p2ap:
                        p2ap[nextPeriod] = p2ap[period].getReference()
                    else:
                        p2ap[nextPeriod].aggregate(p2ap[period])
                    # this profile is now represented in the next larger profile
                    # throw it out
                    p2ap[period].release()
                    del p2ap[period]
            else:
                # current time is not divisible evenly into selected period, and all higher
                # periods are multiples of this one
                break

        yield Task.done
