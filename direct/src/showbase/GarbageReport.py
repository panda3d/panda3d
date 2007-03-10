"""Undocumented Module"""

__all__ = ['FakeObject', '_createGarbage', 'GarbageReport', 'GarbageLogger']

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import gcDebugOn, safeRepr, fastRepr
#from direct.showbase.TaskThreaded import TaskThreaded, TaskThread
from direct.showbase.Job import Job
import gc

class FakeObject:
    pass

def _createGarbage():
    a = FakeObject()
    b = FakeObject()
    a.other = b
    b.other = a

class GarbageReport(Job):
    """Detects leaked Python objects (via gc.collect()) and reports on garbage
    items, garbage-to-garbage references, and garbage cycles.
    If you just want to dump the report to the log, use GarbageLogger."""
    notify = directNotify.newCategory("GarbageReport")

    NotGarbage = 'NG'

    def __init__(self, name, log=True, verbose=False, fullReport=False, findCycles=True,
                 threaded=False, timeslice=None, doneCallback=None):
        # if log is True, GarbageReport will self-destroy after logging
        # if false, caller is responsible for calling destroy()
        # if threaded is True, processing will be performed over multiple frames
        Job.__init__(self, name)
        # stick the arguments onto a ScratchPad so we can access them from the thread
        # functions and delete them all at once
        self._args = ScratchPad(name=name, log=log, verbose=verbose, fullReport=fullReport,
                                findCycles=findCycles, doneCallback=doneCallback)
        self._printing = False
        jobMgr.add(self)

    def run(self):
        # do the garbage collection
        wasOn = gcDebugOn()
        oldFlags = gc.get_debug()
        if not wasOn:
            gc.set_debug(gc.DEBUG_SAVEALL)
        gc.collect()
        yield None
        self.notify.debug('gc.garbage == %s' % fastRepr(gc.garbage))
        yield None
        self.garbage = list(gc.garbage)
        self.notify.debug('self.garbage == %s' % self.garbage)
        del gc.garbage[:]
        if not wasOn:
            gc.set_debug(oldFlags)

        self.numGarbage = len(self.garbage)
        yield None

        if self._args.verbose:
            self.notify.info('found %s garbage items' % self.numGarbage)

        self.referrersByReference = {}
        self.referrersByNumber = {}

        self.referentsByReference = {}
        self.referentsByNumber = {}

        self.cycles = []
        self.cycleSets = []
        self.cycleIds = set()

        # grab the referrers (pointing to garbage)
        if self._args.fullReport and (self.numGarbage != 0):
            if self._args.verbose:
                self.notify.info('getting referrers...')
            for i in xrange(self.numGarbage):
                byNum, byRef = parent._getReferrers(self.garbage[i])
                self.referrersByNumber[i] = byNum
                self.referrersByReference[i] = byRef
                if (not (i & 0x0F)):
                    yield None

        # grab the referents (pointed to by garbage)
        if self.numGarbage > 0:
            if self._args.verbose:
                self.notify.info('getting referents...')
            for i in xrange(self.numGarbage):
                byNum, byRef = self._getReferents(self.garbage[i])
                self.referentsByNumber[i] = byNum
                self.referentsByReference[i] = byRef
                if (not (i & 0x0F)):
                    yield None

        # find the cycles
        if self._args.findCycles and self.numGarbage > 0:
            if self._args.verbose:
                self.notify.info('detecting cycles...')
            for i in xrange(self.numGarbage):
                newCycles = self._getCycles(i, self.cycleSets)
                self.cycles.extend(newCycles)
                # if we're not doing a full report, add this cycle's IDs to the master set
                if not self._args.fullReport:
                    for cycle in newCycles:
                        self.cycleIds.update(set(cycle))
                if (not (i & 0x0F)):
                    yield None

        s = ['===== GarbageReport: \'%s\' (%s items) =====' % (
            self._args.name, self.numGarbage)]
        if self.numGarbage > 0:
            # make a list of the ids we will actually be printing
            if self._args.fullReport:
                garbageIds = range(self.numGarbage)
            else:
                garbageIds = list(self.cycleIds)
                garbageIds.sort()
            numGarbage = len(garbageIds)

            # log each individual item with a number in front of it
            if not self._args.fullReport:
                abbrev = '(abbreviated) '
            else:
                abbrev = ''
            s.append('\n===== Garbage Items %s=====' % abbrev)
            digits = 0
            n = numGarbage
            while n > 0:
                digits += 1
                n /= 10
            digits = digits
            format = '%0' + '%s' % digits + 'i:%s \t%s'

            for i in xrange(numGarbage):
                id = garbageIds[i]
                objStr = safeRepr(self.garbage[id])
                maxLen = 5000
                if len(objStr) > maxLen:
                    snip = '<SNIP>'
                    objStr = '%s%s' % (objStr[:(maxLen-len(snip))], snip)
                s.append(format % (id, itype(self.garbage[id]), objStr))
                if (not (i & 0x7F)):
                    yield None

            if self._args.findCycles:
                s.append('\n===== Garbage Cycles =====')
                for i in xrange(len(self.cycles)):
                    s.append('%s' % self.cycles[i])
                    if (not (i & 0x7F)):
                        yield None

            if self._args.fullReport:
                format = '%0' + '%s' % digits + 'i:%s'
                s.append('\n===== Referrers By Number (what is referring to garbage item?) =====')
                for i in xrange(numGarbage):
                    s.append(format % (i, self.referrersByNumber[i]))
                    if (not (i & 0x7F)):
                        yield None
                s.append('\n===== Referents By Number (what is garbage item referring to?) =====')
                for i in xrange(numGarbage):
                    s.append(format % (i, self.referentsByNumber[i]))
                    if (not (i & 0x7F)):
                        yield None
                s.append('\n===== Referrers (what is referring to garbage item?) =====')
                for i in xrange(numGarbage):
                    s.append(format % (i, self.referrersByReference[i]))
                    if (not (i & 0x7F)):
                        yield None
                s.append('\n===== Referents (what is garbage item referring to?) =====')
                for i in xrange(numGarbage):
                    s.append(format % (i, self.referentsByReference[i]))
                    if (not (i & 0x7F)):
                        yield None

        self._report = s

        if self._args.log:
            self._printing = True
            for i in xrange(len(self._report)):
                print self._report[i]
                if (not (i & 0x3F)):
                    yield None
            self._printing = False

        if self._args.doneCallback:
            self._args.doneCallback(self)

        yield Job.Done

    def destroy(self):
        #print 'GarbageReport.destroy'
        del self._args
        del self.garbage
        del self.numGarbage
        del self.referrersByReference
        del self.referrersByNumber
        del self.referentsByReference
        del self.referentsByNumber
        if hasattr(self, 'cycles'):
            del self.cycles
        del self._report
        if hasattr(self, '_reportStr'):
            del self._reportStr
        Job.destroy(self)

    def suspend(self):
        if self._printing:
            self.notify.info('SUSPEND')
    def resume(self):
        if self._printing:
            self.notify.info('RESUME')

    def getNumItems(self):
        return self.numGarbage

    def getGarbage(self):
        return self.garbage

    def getReport(self):
        if not hasattr(self, '_reportStr'):
            self._reportStr = ''
            for str in self._report:
                self._reportStr += '\n' + str
        return self._reportStr

    def _getReferrers(self, obj):
        # referrers (pointing to garbage)
        # returns two lists, first by index into gc.garbage, second by
        # direct reference
        byRef = gc.get_referrers(obj)
        # look to see if each referrer is another garbage item
        byNum = []
        for referrer in byRef:
            try:
                num = self.garbage.index(referrer)
                byNum.append(num)
            except:
                #num = GarbageReport.NotGarbage
                pass
        return byNum, byRef

    def _getReferents(self, obj):
        # referents (pointed to by garbage)
        # returns two lists, first by index into gc.garbage, second by
        # direct reference
        byRef = gc.get_referents(obj)
        # look to see if each referent is another garbage item
        byNum = []
        for referent in byRef:
            try:
                num = self.garbage.index(referent)
                byNum.append(num)
            except:
                #num = GarbageReport.NotGarbage
                pass
        return byNum, byRef

    def _getCycles(self, index, cycleSets=None):
        # detect garbage cycles for a particular item of garbage
        assert self.notify.debugCall()
        # returns list of lists, sublists are garbage reference cycles
        cycles = []
        # sets of cycle members, to avoid duplicates
        if cycleSets is None:
            cycleSets = []
        stateStack = Stack()
        rootId = index
        stateStack.push(([rootId], rootId, 0))
        while True:
            if len(stateStack) == 0:
                break
            candidateCycle, curId, resumeIndex = stateStack.pop()
            if self.notify.getDebug():
                print 'restart: %s root=%s cur=%s resume=%s' % (
                    candidateCycle, rootId, curId, resumeIndex)
            for index in xrange(resumeIndex, len(self.referentsByNumber[curId])):
                refId = self.referentsByNumber[curId][index]
                if self.notify.getDebug():
                    print '       : %s -> %s' % (curId, refId)
                if refId == rootId:
                    # we found a cycle! mark it down and move on to the next refId
                    if not set(candidateCycle) in cycleSets:
                        if self.notify.getDebug():
                            print '  FOUND: ', list(candidateCycle) + [refId]
                        cycles.append(list(candidateCycle) + [refId])
                        cycleSets.append(set(candidateCycle))
                elif refId in candidateCycle:
                    pass
                else:
                    # this refId does not complete a cycle. Mark down
                    # where we are in this list of referents, then
                    # start looking through the referents of the new refId
                    stateStack.push((list(candidateCycle), curId, index+1))
                    stateStack.push((list(candidateCycle) + [refId], refId, 0))
                    break
        return cycles

class GarbageLogger(GarbageReport):
    """If you just want to log the current garbage to the log file, make
    one of these. It automatically destroys itself after logging"""
    def __init__(self, name, *args, **kArgs):
        kArgs['log'] = True
        GarbageReport.__init__(self, name, *args, **kArgs)
