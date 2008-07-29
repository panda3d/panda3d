"""Undocumented Module"""

__all__ = ['FakeObject', '_createGarbage', 'GarbageReport', 'GarbageLogger']

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import gcDebugOn, safeRepr, fastRepr, printListEnumGen, printNumberedTypesGen
from direct.showbase.Job import Job
import gc

class FakeObject:
    pass

def _createGarbage(num=1):
    for i in xrange(num):
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
                 threaded=False, doneCallback=None, autoDestroy=False, priority=None,
                 safeMode=False):
        # if autoDestroy is True, GarbageReport will self-destroy after logging
        # if false, caller is responsible for calling destroy()
        # if threaded is True, processing will be performed over multiple frames
        Job.__init__(self, name)
        # stick the arguments onto a ScratchPad so we can delete them all at once
        self._args = ScratchPad(name=name, log=log, verbose=verbose, fullReport=fullReport,
                                findCycles=findCycles, doneCallback=doneCallback,
                                autoDestroy=autoDestroy, safeMode=safeMode)
        if priority is not None:
            self.setPriority(priority)
        jobMgr.add(self)
        if not threaded:
            jobMgr.finish(self)

    def run(self):
        # do the garbage collection
        wasOn = gcDebugOn()
        oldFlags = gc.get_debug()
        if not wasOn:
            gc.set_debug(gc.DEBUG_SAVEALL)
        gc.collect()
        self.garbage = list(gc.garbage)
        # only yield if there's more time-consuming work to do,
        # if there's no garbage, give instant feedback
        if len(self.garbage) > 0:
            yield None
        # don't repr the garbage list if we don't have to
        if self.notify.getDebug():
            self.notify.debug('self.garbage == %s' % fastRepr(self.garbage))
        del gc.garbage[:]
        if not wasOn:
            gc.set_debug(oldFlags)

        self.numGarbage = len(self.garbage)
        # only yield if there's more time-consuming work to do,
        # if there's no garbage, give instant feedback
        if self.numGarbage > 0:
            yield None

        if self._args.verbose:
            self.notify.info('found %s garbage items' % self.numGarbage)

        """ spammy
        # print the types of the garbage first, in case the repr of an object
        # causes a crash
        if self.numGarbage > 0:
            self.notify.info('TYPES ONLY (this is only needed if a crash occurs before GarbageReport finishes):')
            for result in printNumberedTypesGen(self.garbage):
                yield None
                """

        # Py obj id -> garbage list index
        self._id2index = {}

        self.referrersByReference = {}
        self.referrersByNumber = {}

        self.referentsByReference = {}
        self.referentsByNumber = {}

        self.cycles = []
        self.uniqueCycleSets = set()
        self.cycleIds = set()

        # make the id->index table to speed up the next steps
        for i in xrange(self.numGarbage):
            self._id2index[id(self.garbage[i])] = i
            if not (i % 20):
                yield None

        # grab the referrers (pointing to garbage)
        if self._args.fullReport and (self.numGarbage != 0):
            if self._args.verbose:
                self.notify.info('getting referrers...')
            for i in xrange(self.numGarbage):
                yield None
                for result in self._getReferrers(self.garbage[i]):
                    yield None
                byNum, byRef = result
                self.referrersByNumber[i] = byNum
                self.referrersByReference[i] = byRef

        # grab the referents (pointed to by garbage)
        if self.numGarbage > 0:
            if self._args.verbose:
                self.notify.info('getting referents...')
            for i in xrange(self.numGarbage):
                yield None
                for result in self._getReferents(self.garbage[i]):
                    yield None
                byNum, byRef = result                    
                self.referentsByNumber[i] = byNum
                self.referentsByReference[i] = byRef

        # find the cycles
        if self._args.findCycles and self.numGarbage > 0:
            if self._args.verbose:
                self.notify.info('detecting cycles...')
            for i in xrange(self.numGarbage):
                yield None
                for newCycles in self._getCycles(i, self.uniqueCycleSets):
                    yield None
                self.cycles.extend(newCycles)
                # if we're not doing a full report, add this cycle's IDs to the master set
                if not self._args.fullReport:
                    for cycle in newCycles:
                        yield None
                        self.cycleIds.update(set(cycle))

        if self._args.findCycles:
            s = ['===== GarbageReport: \'%s\' (%s items, %s cycles) =====' % (
                self._args.name, self.numGarbage, len(self.cycles))]
        else:
            s = ['===== GarbageReport: \'%s\' (%s items) =====' % (
                self._args.name, self.numGarbage)]
        if self.numGarbage > 0:
            # make a list of the ids we will actually be printing
            if self._args.fullReport:
                garbageIndices = range(self.numGarbage)
            else:
                garbageIndices = list(self.cycleIds)
                garbageIndices.sort()
            numGarbage = len(garbageIndices)

            # log each individual item with a number in front of it
            if not self._args.fullReport:
                abbrev = '(abbreviated) '
            else:
                abbrev = ''
            s.append('===== Garbage Items %s=====' % abbrev)
            digits = 0
            n = numGarbage
            while n > 0:
                yield None
                digits += 1
                n /= 10
            digits = digits
            format = '%0' + '%s' % digits + 'i:%s \t%s'

            for i in xrange(numGarbage):
                yield None
                idx = garbageIndices[i]
                if self._args.safeMode:
                    # in safe mode, don't try to repr any of the objects
                    objStr = repr(itype(self.garbage[idx]))
                else:
                    objStr = fastRepr(self.garbage[idx])
                maxLen = 5000
                if len(objStr) > maxLen:
                    snip = '<SNIP>'
                    objStr = '%s%s' % (objStr[:(maxLen-len(snip))], snip)
                s.append(format % (idx, itype(self.garbage[idx]), objStr))

            # also log the types of the objects
            s.append('===== Garbage Item Types %s=====' % abbrev)
            for i in xrange(numGarbage):
                yield None
                idx = garbageIndices[i]
                objStr = str(deeptype(self.garbage[idx]))
                maxLen = 5000
                if len(objStr) > maxLen:
                    snip = '<SNIP>'
                    objStr = '%s%s' % (objStr[:(maxLen-len(snip))], snip)
                s.append(format % (idx, itype(self.garbage[idx]), objStr))

            if self._args.findCycles:
                s.append('===== Garbage Cycles =====')
                for i in xrange(len(self.cycles)):
                    yield None
                    s.append('%s' % self.cycles[i])

            if self._args.fullReport:
                format = '%0' + '%s' % digits + 'i:%s'
                s.append('===== Referrers By Number (what is referring to garbage item?) =====')
                for i in xrange(numGarbage):
                    yield None
                    s.append(format % (i, self.referrersByNumber[i]))
                s.append('===== Referents By Number (what is garbage item referring to?) =====')
                for i in xrange(numGarbage):
                    yield None
                    s.append(format % (i, self.referentsByNumber[i]))
                s.append('===== Referrers (what is referring to garbage item?) =====')
                for i in xrange(numGarbage):
                    yield None
                    s.append(format % (i, self.referrersByReference[i]))
                s.append('===== Referents (what is garbage item referring to?) =====')
                for i in xrange(numGarbage):
                    yield None
                    s.append(format % (i, self.referentsByReference[i]))

        self._report = s

        if self._args.log:
            self.printingBegin()
            for i in xrange(len(self._report)):
                if self.numGarbage > 0:
                    yield None
                self.notify.info(self._report[i])
            self.printingEnd()

        yield Job.Done

    def finished(self):
        if self._args.doneCallback:
            self._args.doneCallback(self)
        if self._args.autoDestroy:
            self.destroy()

    def destroy(self):
        #print 'GarbageReport.destroy'
        del self._args
        del self.garbage
        # don't get rid of this, we might need it
        #del self.numGarbage
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

    def getNumItems(self):
        # if the job hasn't run yet, we don't have a numGarbage yet
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
        yield None
        byRef = gc.get_referrers(obj)
        yield None
        # look to see if each referrer is another garbage item
        byNum = []
        for i in xrange(len(byRef)):
            if not (i % 20):
                yield None
            referrer = byRef[i]
            num = self._id2index.get(id(referrer), None)
            byNum.append(num)
        yield byNum, byRef

    def _getReferents(self, obj):
        # referents (pointed to by garbage)
        # returns two lists, first by index into gc.garbage, second by
        # direct reference
        yield None
        byRef = gc.get_referents(obj)
        yield None
        # look to see if each referent is another garbage item
        byNum = []
        for i in xrange(len(byRef)):
            if not (i % 20):
                yield None
            referent = byRef[i]
            num = self._id2index.get(id(referent), None)
            byNum.append(num)
        yield byNum, byRef

    def _getNormalizedCycle(self, cycle):
        # returns a representation of a cycle (list of indices) that will be
        # reliably derived from a unique cycle regardless of ordering
        # this lets us detect duplicate cycles that appear different because of
        # which element appears first
        if len(cycle) == 0:
            return cycle
        min = 1<<30
        minIndex = None
        for i in xrange(len(cycle)):
            elem = cycle[i]
            if elem < min:
                min = elem
                minIndex = i
        return cycle[minIndex:] + cycle[:minIndex]

    def _getCycles(self, index, uniqueCycleSets=None):
        # detect garbage cycles for a particular item of garbage
        assert self.notify.debugCall()
        # returns list of lists, sublists are garbage reference cycles
        cycles = []
        # this lets us eliminate duplicate cycles
        if uniqueCycleSets is None:
            uniqueCycleSets = set()
        stateStack = Stack()
        rootId = index
        stateStack.push(([rootId], rootId, 0))
        while True:
            yield None
            if len(stateStack) == 0:
                break
            candidateCycle, curId, resumeIndex = stateStack.pop()
            if self.notify.getDebug():
                print 'restart: %s root=%s cur=%s resume=%s' % (
                    candidateCycle, rootId, curId, resumeIndex)
            for index in xrange(resumeIndex, len(self.referentsByNumber[curId])):
                yield None
                refId = self.referentsByNumber[curId][index]
                if self.notify.getDebug():
                    print '       : %s -> %s' % (curId, refId)
                if refId == rootId:
                    # we found a cycle! mark it down and move on to the next refId
                    normCandidateCycle = self._getNormalizedCycle(candidateCycle)
                    normCandidateCycleTuple = tuple(normCandidateCycle)
                    if not normCandidateCycleTuple in uniqueCycleSets:
                        if self.notify.getDebug():
                            print '  FOUND: ', normCandidateCycle + [normCandidateCycle[0],]
                        cycles.append(normCandidateCycle + [normCandidateCycle[0],])
                        uniqueCycleSets.add(normCandidateCycleTuple)
                elif refId in candidateCycle:
                    pass
                elif refId is not None:
                    # this refId does not complete a cycle. Mark down
                    # where we are in this list of referents, then
                    # start looking through the referents of the new refId
                    stateStack.push((list(candidateCycle), curId, index+1))
                    stateStack.push((list(candidateCycle) + [refId], refId, 0))
                    break
        yield cycles

class GarbageLogger(GarbageReport):
    """If you just want to log the current garbage to the log file, make
    one of these. It automatically destroys itself after logging"""
    def __init__(self, name, *args, **kArgs):
        kArgs['log'] = True
        kArgs['autoDestroy'] = True
        GarbageReport.__init__(self, name, *args, **kArgs)
