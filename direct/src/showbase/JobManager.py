from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.task.TaskManagerGlobal import taskMgr
from direct.showbase.Job import Job
from direct.showbase.PythonUtil import getBase
from direct.showbase.MessengerGlobal import messenger


class JobManager:
    """
    Similar to the taskMgr but designed for tasks that are CPU-intensive and/or
    not time-critical. Jobs run in a fixed timeslice that the JobManager is
    allotted each frame.
    """
    notify = directNotify.newCategory("JobManager")

    # there's one task for the JobManager, all jobs run in this task
    TaskName = 'jobManager'

    def __init__(self, timeslice=None):
        # how long do we run per frame
        self._timeslice = timeslice
        # store the jobs in these structures to allow fast lookup by various keys
        # priority -> jobId -> job
        self._pri2jobId2job = {}
        # priority -> chronological list of jobIds
        self._pri2jobIds = {}
        # jobId -> priority
        self._jobId2pri = {}
        # how many timeslices to give each job; this is used to efficiently implement
        # the relative job priorities
        self._jobId2timeslices = {}
        # how much time did the job use beyond the allotted timeslice, used to balance
        # out CPU usage
        self._jobId2overflowTime = {}
        self._useOverflowTime = None
        # this is a generator that we use to give high-priority jobs more timeslices,
        # it yields jobIds in a sequence that includes high-priority jobIds more often
        # than low-priority
        self._jobIdGenerator = None
        self._highestPriority = Job.Priorities.Normal

    def destroy(self):
        taskMgr.remove(JobManager.TaskName)
        del self._pri2jobId2job

    def add(self, job):
        pri = job.getPriority()
        jobId = job._getJobId()
        # store the job in the main table
        self._pri2jobId2job.setdefault(pri, {})
        self._pri2jobId2job[pri][jobId] = job
        # and also store a direct mapping from the job's ID to its priority
        self._jobId2pri[jobId] = pri
        # add the jobId onto the end of the list of jobIds for this priority
        self._pri2jobIds.setdefault(pri, [])
        self._pri2jobIds[pri].append(jobId)
        # record the job's relative timeslice count
        self._jobId2timeslices[jobId] = pri
        # init the overflow time tracking
        self._jobId2overflowTime[jobId] = 0.
        # reset the jobId round-robin
        self._jobIdGenerator = None
        if len(self._jobId2pri) == 1:
            taskMgr.add(self._process, JobManager.TaskName)
            self._highestPriority = pri
        elif pri > self._highestPriority:
            self._highestPriority = pri
        self.notify.debug('added job: %s' % job.getJobName())

    def remove(self, job):
        jobId = job._getJobId()
        # look up the job's priority
        pri = self._jobId2pri.pop(jobId)
        # TODO: this removal is a linear search
        self._pri2jobIds[pri].remove(jobId)
        # remove the job from the main table
        del self._pri2jobId2job[pri][jobId]
        # clean up the job's generator, if any
        job._cleanupGenerator()
        # remove the job's timeslice count
        self._jobId2timeslices.pop(jobId)
        # remove the overflow time
        self._jobId2overflowTime.pop(jobId)
        if len(self._pri2jobId2job[pri]) == 0:
            del self._pri2jobId2job[pri]
            if pri == self._highestPriority:
                if len(self._jobId2pri) > 0:
                    # calculate a new highest priority
                    # TODO: this is not very fast
                    priorities = self._getSortedPriorities()
                    self._highestPriority = priorities[-1]
                else:
                    taskMgr.remove(JobManager.TaskName)
                    self._highestPriority = 0
        self.notify.debug('removed job: %s' % job.getJobName())

    def finish(self, job):
        # run this job, right now, until it finishes
        assert self.notify.debugCall()
        jobId = job._getJobId()
        # look up the job's priority
        pri = self._jobId2pri[jobId]
        # grab the job
        job = self._pri2jobId2job[pri][jobId]
        gen = job._getGenerator()
        if __debug__:
            job._pstats.start()
        job.resume()
        while True:
            try:
                result = next(gen)
            except StopIteration:
                # Job didn't yield Job.Done, it ran off the end and returned
                # treat it as if it returned Job.Done
                self.notify.warning('job %s never yielded Job.Done' % job)
                result = Job.Done
            if result is Job.Done:
                job.suspend()
                self.remove(job)
                job._setFinished()
                messenger.send(job.getFinishedEvent())
                # job is done.
                break
        if __debug__:
            job._pstats.stop()

    # how long should we run per frame?
    @staticmethod
    def getDefaultTimeslice():
        # run for 1/2 millisecond per frame by default
        # config is in milliseconds, this func returns value in seconds
        return getBase().config.GetFloat('job-manager-timeslice-ms', .5) / 1000.
    def getTimeslice(self):
        if self._timeslice:
            return self._timeslice
        return self.getDefaultTimeslice()
    def setTimeslice(self, timeslice):
        self._timeslice = timeslice

    def _getSortedPriorities(self):
        # returns all job priorities in ascending order
        priorities = list(self._pri2jobId2job.keys())
        priorities.sort()
        return priorities

    def _process(self, task=None):
        if self._useOverflowTime is None:
            self._useOverflowTime = config.GetBool('job-use-overflow-time', 1)
        if len(self._pri2jobId2job):
            #assert self.notify.debugCall()
            # figure out how long we can run
            endT = globalClock.getRealTime() + (self.getTimeslice() * .9)
            while True:
                if self._jobIdGenerator is None:
                    # round-robin the jobs, giving high-priority jobs more timeslices
                    self._jobIdGenerator = flywheel(
                        list(self._jobId2timeslices.keys()),
                        countFunc = lambda jobId: self._jobId2timeslices[jobId])
                try:
                    # grab the next jobId in the sequence
                    jobId = next(self._jobIdGenerator)
                except StopIteration:
                    self._jobIdGenerator = None
                    continue
                # OK, we've selected a job to run
                pri = self._jobId2pri.get(jobId)
                if pri is None:
                    # this job is no longer present
                    continue
                # check if there's overflow time that we need to make up for
                if self._useOverflowTime:
                    overflowTime = self._jobId2overflowTime[jobId]
                    timeLeft = endT - globalClock.getRealTime()
                    if overflowTime >= timeLeft:
                        self._jobId2overflowTime[jobId] = max(0., overflowTime-timeLeft)
                        # don't run any more jobs this frame, this makes up
                        # for the extra overflow time that was used before
                        break
                job = self._pri2jobId2job[pri][jobId]
                gen = job._getGenerator()
                if __debug__:
                    job._pstats.start()
                job.resume()
                while globalClock.getRealTime() < endT:
                    try:
                        result = next(gen)
                    except StopIteration:
                        # Job didn't yield Job.Done, it ran off the end and returned
                        # treat it as if it returned Job.Done
                        self.notify.warning('job %s never yielded Job.Done' % job)
                        result = Job.Done

                    if result is Job.Sleep:
                        job.suspend()
                        if __debug__:
                            job._pstats.stop()
                        # grab the next job if there's time left
                        break
                    elif result is Job.Done:
                        job.suspend()
                        self.remove(job)
                        job._setFinished()
                        if __debug__:
                            job._pstats.stop()
                        messenger.send(job.getFinishedEvent())
                        # grab the next job if there's time left
                        break
                else:
                    # we've run out of time
                    #assert self.notify.debug('timeslice end: %s, %s' % (endT, globalClock.getRealTime()))
                    job.suspend()
                    overflowTime = globalClock.getRealTime() - endT
                    if overflowTime > self.getTimeslice():
                        self._jobId2overflowTime[jobId] += overflowTime
                    if __debug__:
                        job._pstats.stop()
                    break

                if len(self._pri2jobId2job) == 0:
                    # there's nothing left to do, all the jobs are done!
                    break
        return task.cont

    def __repr__(self):
        s  =   '======================================================='
        s += '\nJobManager: active jobs in descending order of priority'
        s += '\n======================================================='
        pris = self._getSortedPriorities()
        if len(pris) == 0:
            s += '\n    no jobs running'
        else:
            pris.reverse()
            for pri in pris:
                jobId2job = self._pri2jobId2job[pri]
                # run through the jobs at this priority in the order that they will run
                for jobId in self._pri2jobIds[pri]:
                    job = jobId2job[jobId]
                    s += '\n%5d: %s (jobId %s)' % (pri, job.getJobName(), jobId)
        s += '\n'
        return s
