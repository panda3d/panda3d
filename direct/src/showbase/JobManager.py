from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.task.TaskManagerGlobal import taskMgr
from direct.showbase.Job import Job

class JobManager:
    """
    Similar to the taskMgr but designed for tasks that are CPU-intensive and/or
    not time-critical. Jobs run one at a time, in order of priority, in
    the timeslice that the JobManager is allotted each frame.
    """
    notify = directNotify.newCategory("JobManager")

    # there's one task for the JobManager, all jobs run in this task
    TaskName = 'jobManager'
    # run for 1/2 millisecond per frame by default
    DefTimeslice = (1./1000.) * .5

    def __init__(self, timeslice=None):
        if timeslice is None:
            timeslice = JobManager.DefTimeslice
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
        # this is the working copy of _jobId2timeslices that we use to count down how
        # many timeslices to give each job
        self._jobId2timeslicesLeft = {}
        # this is used to round-robin the jobs in _jobId2timeslicesLeft
        self._curJobIndex = 0
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
        if jobId in self._jobId2timeslicesLeft:
            del self._jobId2timeslicesLeft[jobId]
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
                result = gen.next()
            except StopIteration:
                # Job didn't yield Job.Done, it ran off the end and returned
                # treat it as if it returned Job.Done
                self.notify.warning('job %s never yielded Job.Done' % job)
                result = Job.Done
            if result is Job.Done:
                job.suspend()
                self.remove(job)
                job.finished()
                messenger.send(job.getFinishedEvent())
                # job is done.
                break
        if __debug__:
            job._pstats.stop()

    # how long should we run per frame?
    def getTimeslice(self):
        return self._timeslice
    def setTimeslice(self, timeslice):
        self._timeslice = timeslice

    def _getSortedPriorities(self):
        # returns all job priorities in ascending order
        priorities = self._pri2jobId2job.keys()
        priorities.sort()
        return priorities

    def _process(self, task=None):
        if len(self._pri2jobId2job):
            #assert self.notify.debugCall()
            # figure out how long we can run
            endT = globalClock.getRealTime() + (self._timeslice * .9)
            while True:
                # round-robin the jobs, dropping them as they run out of priority timeslices
                # until all timeslices are used
                if len(self._jobId2timeslicesLeft) == 0:
                    self._jobId2timeslicesLeft = dict(self._jobId2timeslices)
                self._curJobIndex = (self._curJobIndex + 1) % len(self._jobId2timeslicesLeft)
                jobId = self._jobId2timeslicesLeft.keys()[self._curJobIndex]
                # use up one of this job's timeslices
                self._jobId2timeslicesLeft[jobId] -= 1
                if self._jobId2timeslicesLeft[jobId] == 0:
                    del self._jobId2timeslicesLeft[jobId]
                pri = self._jobId2pri[jobId]
                job = self._pri2jobId2job[pri][jobId]
                gen = job._getGenerator()
                if __debug__:
                    job._pstats.start()
                job.resume()
                while globalClock.getRealTime() < endT:
                    try:
                        result = gen.next()
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
                        job.finished()
                        if __debug__:
                            job._pstats.stop()
                        messenger.send(job.getFinishedEvent())
                        # grab the next job if there's time left
                        break
                else:
                    # we've run out of time
                    #assert self.notify.debug('timeslice end: %s, %s' % (endT, globalClock.getRealTime()))
                    job.suspend()
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
                    s += '\n%4d: %s' % (jobId, job.getJobName())
        s += '\n'
        return s
