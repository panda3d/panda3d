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
    # run for one millisecond per frame by default
    DefTimeslice = .001

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
        self._highestPriority = Job.Priorities.Normal

    def destroy(self):
        taskMgr.remove(JobManager.TaskName)
        del self._pri2jobId2job

    def add(self, job):
        assert self.notify.debugCall()
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
        if pri > self._highestPriority:
            self._highestPriority = pri
        if len(self._jobId2pri) == 1:
            taskMgr.add(self._process, JobManager.TaskName)
        self.notify.debug('added job %s' % job.getJobName())
        
    def remove(self, job):
        assert self.notify.debugCall()
        jobId = job._getJobId()
        # look up the job's priority
        pri = self._jobId2pri.pop(jobId)
        # TODO: this removal is a linear search
        self._pri2jobIds[pri].remove(jobId)
        # remove the job from the main table
        del self._pri2jobId2job[pri][jobId]
        # clean up the job's generator, if any
        job._cleanupGenerator()
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
        self.notify.debug('removed job %s' % job.getJobName())

    def finish(self, job):
        # run this job, right now, until it finishes
        assert self.notify.debugCall()
        jobId = job._getJobId()
        # look up the job's priority
        pri = self._jobId2pri[jobId]
        # grab the job
        job = self._pri2jobId2job[pri][jobId]
        gen = job._getGenerator()
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
                # job is done.
                break

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
            assert self.notify.debugCall()
            # figure out how long we can run
            endT = globalClock.getRealTime() + (self._timeslice * .9)
            while True:
                # always process the highest priority first
                # TODO: give occasional timeslices to lower priorities to avoid starving
                # lower-priority jobs
                jobId2job = self._pri2jobId2job[self._highestPriority]
                # process jobs with equal priority in the order they came in
                jobId = self._pri2jobIds[self._highestPriority][0]
                job = jobId2job[jobId]
                gen = job._getGenerator()
                job.resume()
                while globalClock.getRealTime() < endT:
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
                        # highest-priority job is done.
                        # grab the next one if there's time left
                        break
                else:
                    # we've run out of time
                    assert self.notify.debug('out of time: %s, %s' % (endT, globalClock.getRealTime()))
                    job.suspend()
                    break
                
                if len(self._pri2jobId2job) == 0:
                    # there's nothing left to do, all the jobs are done!
                    break
        return task.cont

    def __repr__(self):
        s  =   '================================================='
        s += '\nJobManager: jobs, in descending order of priority'
        s += '\n================================================='
        pris = self._getSortedPriorities()
        pris.reverse()
        for pri in pris:
            jobId2job = self._pri2jobId2job[pri]
            # run through the jobs at this priority in the order that they will run
            for jobId in self._pri2jobIds[pri]:
                job = jobId2job[jobId]
                s += '\n%3d: %s' % (jobId, job.getJobName())
        s += '\n'
        return s
