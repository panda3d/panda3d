from direct.showbase.DirectObject import DirectObject

if __debug__:
    from pandac.PandaModules import PStatCollector

class Job(DirectObject):
    # Base class for cpu-intensive or non-time-critical operations that
    # are run through the JobManager.

    # values to yield from your run() generator method
    Done = object()
    Continue = None # 'yield None' is acceptable in place of 'yield Job.Continue'
    Sleep = object() # yield any remaining time for this job until next frame

    # These priorities determine how many timeslices a job gets relative to other
    # jobs. A job with priority of 1000 will run 10 times more often than a job
    # with priority of 100.
    Priorities = ScratchPad(Min=1, Low=100, Normal=1000, High=10000)
    _SerialGen = SerialNumGen()
    
    def __init__(self, name):
        self._name = name
        self._generator = None
        self._id = Job._SerialGen.next()
        self._printing = False
        if __debug__:
            self._pstats = PStatCollector("App:Show code:jobManager:%s" % self._name)

    def destroy(self):
        del self._name
        del self._generator
        del self._printing

    def getFinishedEvent(self):
        return 'job-finished-%s' % self._id

    def run(self):
        # this is a generator
        # override and do your processing
        # yield Job.Continue when possible/reasonable
        # try not to run longer than the JobManager's timeslice between yields
        #
        # when done, yield Job.Done
        #
        raise "don't call down"

    def getPriority(self):
        # override if you want a different priority
        return Job.Priorities.Normal

    def printingBegin(self):
        self._printing = True
    def printingEnd(self):
        self._printing = False

    def resume(self):
        # called every time JobManager is going to start running this job
        if self._printing:
            print 'JOB:%s:RESUME' % self._name
    def suspend(self):
        # called when JobManager is going to stop running this job for a while
        if self._printing:
            print 'JOB:%s:SUSPEND' % self._name

    def finished(self):
        # called when the job finishes and has been removed from the JobManager
        pass

    def getJobName(self):
        return self._name
    def _getJobId(self):
        return self._id

    def _getGenerator(self):
        if self._generator is None:
            self._generator = self.run()
        return self._generator
    def _cleanupGenerator(self):
        if self._generator is not None:
            self._generator = None

if __debug__: # __dev__ not yet available at this point
    from direct.showbase.Job import Job
    class TestJob(Job):
        def __init__(self):
            Job.__init__(self, 'TestJob')
            self._counter = 0
            self._accum = 0
            self._finished = False

        def run(self):
            self.printingBegin()
            while True:
                while self._accum < 100:
                    self._accum += 1
                    print 'counter = %s, accum = %s' % (self._counter, self._accum)
                    yield None

                self._accum = 0
                self._counter += 1

                if self._counter >= 100:
                    print 'Job.Done'
                    self.printingEnd()
                    yield Job.Done
                else:
                    yield None

    def addTestJob():
        jobMgr.add(TestJob())
