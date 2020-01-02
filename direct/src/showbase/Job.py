from direct.showbase.DirectObject import DirectObject

if __debug__:
    from panda3d.core import PStatCollector


class Job(DirectObject):
    """Base class for cpu-intensive or non-time-critical operations that
    are run through the :class:`.JobManager`.

    To use, subclass and override the `run()` method.
    """

    #: Yielded from the `run()` generator method when the job is done.
    Done = object()

    #: ``yield None`` is acceptable in place of ``yield Job.Continue``
    Continue = None

    #: Yield any remaining time for this job until next frame.
    Sleep = object()

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
        self._priority = Job.Priorities.Normal
        self._finished = False
        if __debug__:
            self._pstats = PStatCollector("App:Show code:jobManager:%s" % self._name)

    def destroy(self):
        del self._name
        del self._generator
        del self._printing

    def getFinishedEvent(self):
        return 'job-finished-%s' % self._id

    def run(self):
        """This should be overridden with a generator that does the
        needful processing.

        yield `Job.Continue` when possible/reasonable, and try not to run
        longer than the JobManager's timeslice between yields.

        When done, yield `Job.Done`.
        """
        raise NotImplementedError("don't call down")

    def getPriority(self):
        return self._priority
    def setPriority(self, priority):
        self._priority = priority

    def printingBegin(self):
        self._printing = True
    def printingEnd(self):
        self._printing = False

    def resume(self):
        """Called every time JobManager is going to start running this job."""
        #if self._printing:
        #    # we may be suspended/resumed multiple times per frame, that gets spammy
        #    # if we need to pick out the output of a job, put a prefix onto each line
        #    # of the output
        #    print('JOB:%s:RESUME' % self._name)

    def suspend(self):
        """Called when JobManager is going to stop running this job for a
        while.
        """

        #if self._printing:
        #    #print('JOB:%s:SUSPEND' % self._name)
        #    pass
        #    """

    def _setFinished(self):
        self._finished = True
        self.finished()
    def isFinished(self):
        return self._finished

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
                    print('counter = %s, accum = %s' % (self._counter, self._accum))
                    yield None

                self._accum = 0
                self._counter += 1

                if self._counter >= 100:
                    print('Job.Done')
                    self.printingEnd()
                    yield Job.Done
                else:
                    yield None

    def addTestJob():
        jobMgr.add(TestJob())
