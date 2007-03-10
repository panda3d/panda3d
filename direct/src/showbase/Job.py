class Job:
    # Base class for cpu-intensive or non-time-critical operations that
    # are run through the JobManager.

    # values to yield from your run() generator method
    Done = object()
    Continue = None # 'yield None' is acceptable in place of 'yield Job.Continue'

    Priorities = ScratchPad(Low=-100, Normal=0, High=100)
    _SerialGen = SerialNumGen()
    
    def __init__(self, name):
        self._name = name
        self._generator = None
        self._id = Job._SerialGen.next()

    def destroy(self):
        del self._name
        del self._generator

    def run(self):
        # override and do your processing
        # yield Job.Continue when possible/reasonable
        # try not to run longer than the JobManager's timeslice between yields
        # when done, yield Job.Done
        raise "don't call down"

    def getPriority(self):
        # override if you want a different priority
        # you can use numbers other than those in Job.Priorities
        return Job.Priorities.Normal

    def suspend(self):
        # called when JobManager is going to stop running this job for a while
        # most jobs don't need to override this
        pass
    def resume(self):
        # called when JobManager is going to start running this job again
        # most jobs don't need to override this
        pass

    def _getJobId(self):
        return self._id

    def _getGenerator(self):
        if self._generator is None:
            self._generator = self.run()
        return self._generator

if __debug__: # __dev__ not yet available at this point
    from direct.showbase.Job import Job
    class TestJob(Job):
        def __init__(self):
            Job.__init__(self, 'TestJob')
            self._counter = 0
            self._accum = 0
            self._finished = False

        def run(self):
            while True:
                while self._accum < 100:
                    self._accum += 1
                    print 'counter = %s, accum = %s' % (self._counter, self._accum)
                    yield None

                self._accum = 0
                self._counter += 1

                if self._counter >= 100:
                    print 'Job.Done'
                    yield Job.Done
                else:
                    yield None

    def addTestJob():
        jobMgr.add(TestJob())
