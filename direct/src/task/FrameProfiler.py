from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.fsm.StatePush import FunctionCall
from direct.showbase.PythonUtil import formatTimeExact, normalDistrib

class FrameProfiler:
    notify = directNotify.newCategory('FrameProfiler')

    # for precision, all times related to the profile/log schedule are stored as integers
    Minute = 60
    Hour = 60 * Minute
    Day = 24 * Hour

    def __init__(self):
        Hour = FrameProfiler.Hour
        # how long to wait between frame profiles
        self._period = 2 * FrameProfiler.Minute
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
        for t in self._logSchedule:
            assert isInteger(t)
            # make sure the period is evenly divisible into each element of the log schedule
            assert (t % self._period) == 0
        # make sure each element of the schedule is evenly divisible into each subsequent element
        for i in xrange(len(self._logSchedule)):
            e = self._logSchedule[i]
            for j in xrange(i, len(self._logSchedule)):
                assert (self._logSchedule[j] % e) == 0
        assert isInteger(self._period)
        self._enableFC = FunctionCall(self._setEnabled, taskMgr.getProfileFramesSV())

    def destroy(self):
        self._enableFC.set(False)
        self._enableFC.destroy()
        
    def _setEnabled(self, enabled):
        if enabled:
            self._startTime = globalClock.getFrameTime()
            self._profileCounter = 0
            self._jitter = None
            self._period2aggregateProfile = {}
            self._lastSession = None
            # don't profile process startup
            self._task = taskMgr.doMethodLater(self._period, self._startProfiling,
                                               'FrameProfilerStart-%s' % serialNum())
        else:
            self._task.remove()
            del self._task
            for session in self._period2aggregateProfile.itervalues:
                session.release()
            del self._period2aggregateProfile
            if self._lastSession:
                self._lastSession.release()
            del self._lastSession

    def _startProfiling(self, task):
        self._scheduleNextProfile()
        return task.done

    def _scheduleNextProfile(self):
        self._profileCounter += 1
        self._timeElapsed = self._profileCounter * self._period
        assert isInteger(self._timeElapsed)
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
            
        self._lastSession = taskMgr.getProfileSession('FrameProfile-%s' % serialNum())
        taskMgr.profileFrames(num=1, session=self._lastSession)

        delay = max(time - globalClock.getFrameTime(), 0.)
        self._task = taskMgr.doMethodLater(delay, self._frameProfileTask,
                                           'FrameProfiler-%s' % serialNum())

    def _frameProfileTask(self, task):
        if self._lastSession:
            p2ap = self._period2aggregateProfile
            # always add this profile to the first aggregated profile
            period = self._logSchedule[0]
            if period not in self._period2aggregateProfile:
                self._lastSession.setLines(500)
                p2ap[period] = self._lastSession.getReference()
            else:
                p2ap[period].aggregate(self._lastSession)
            # log profiles when it's time, and aggregate them upwards into the
            # next-larger profile
            for period in self._logSchedule:
                if (self._timeElapsed % period) == 0:
                    ap = p2ap[period]
                    self.notify.info('aggregate profile of sampled frames over last %s\n%s' %
                                     (formatTimeExact(period), ap.getResults()))
                    # aggregate this profile into the next larger profile
                    nextPeriod = period * 2
                    # make sure the next larger log period is in the schedule
                    if period == self._logSchedule[-1]:
                        self._logSchedule.append(nextPeriod)
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

            # always release the last-recorded profile
            self._lastSession.release()
            self._lastSession = None

        self._scheduleNextProfile()
        return task.done
