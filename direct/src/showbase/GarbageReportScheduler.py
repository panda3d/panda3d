from direct.showbase.GarbageReport import GarbageReport

class GarbageReportScheduler:
    # runs a garbage report every once in a while and logs the results
    def __init__(self, waitBetween=None, waitScale=None):
        # waitBetween is in seconds
        # waitScale is a multiplier for the waitBetween every time around
        if waitBetween is None:
            waitBetween = 30*60
        if waitScale is None:
            waitScale = 1.5
        self._waitBetween = waitBetween
        self._waitScale = waitScale
        self._taskName = 'startScheduledGarbageReport-%s' % serialNum()
        self._garbageReport = None
        self._scheduleNextGarbageReport()

    def getTaskName(self):
        return self._taskName

    def _scheduleNextGarbageReport(self, garbageReport=None):
        if garbageReport:
            # this report finished, wait a bit then start another
            assert garbageReport is self._garbageReport
            # garbagereport will clean itself up
            self._garbageReport = None
        # run another garbagereport after a delay
        taskMgr.doMethodLater(self._waitBetween,
                              self._runGarbageReport,
                              self._taskName)
        # and increase the delay every time around
        self._waitBetween = self._waitBetween * self._waitScale
    def _runGarbageReport(self, task):
        # run a garbage report and schedule the next one after this one finishes
        # give this job 3 times as many timeslices as normal-priority jobs
        self._garbageReport = GarbageReport('ScheduledGarbageReport', threaded=True,
                                            doneCallback=self._scheduleNextGarbageReport,
                                            autoDestroy=True,
                                            priority=GarbageReport.Priorities.Normal * 3)
        return task.done
