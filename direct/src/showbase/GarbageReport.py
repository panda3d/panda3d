"""Undocumented Module"""

__all__ = ['FakeObject', '_createGarbage', 'GarbageReport', 'GarbageLogger']

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import gcDebugOn, safeRepr, fastRepr
from direct.showbase.TaskThreaded import TaskThreaded, TaskThread
import gc

class FakeObject:
    pass

def _createGarbage():
    a = FakeObject()
    b = FakeObject()
    a.other = b
    b.other = a

class GarbageReport(TaskThreaded):
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
        TaskThreaded.__init__(self, name, threaded, timeslice=timeslice)
        # stick the arguments onto a ScratchPad so we can access them from the thread
        # functions and delete them all at once
        self._args = ScratchPad(name=name, log=log, verbose=verbose, fullReport=fullReport,
                                findCycles=findCycles, doneCallback=doneCallback)

        # do the garbage collection
        wasOn = gcDebugOn()
        oldFlags = gc.get_debug()
        if not wasOn:
            gc.set_debug(gc.DEBUG_SAVEALL)
        gc.collect()
        self.notify.debug('gc.garbage == %s' % fastRepr(gc.garbage))
        self.garbage = list(gc.garbage)
        self.notify.debug('self.garbage == %s' % self.garbage)
        del gc.garbage[:]
        if not wasOn:
            gc.set_debug(oldFlags)

        self.numGarbage = len(self.garbage)

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
        class GetReferrers(TaskThread):
            def setUp(self):
                if self.parent._args.fullReport and (self.parent.numGarbage != 0):
                    if self.parent._args.verbose:
                        self.parent.notify.info('getting referrers...')
                    self.index = 0
                else:
                    self.finished()
            def run(self):
                parent = self.parent
                for i in xrange(self.index, parent.numGarbage):
                    byNum, byRef = parent._getReferrers(parent.garbage[i])
                    parent.referrersByNumber[i] = byNum
                    parent.referrersByReference[i] = byRef
                    if (not (i & 0x0F)) and (not self.timeLeft()):
                        # we've run out of time, save the index
                        self.index = i+1
                        return
                self.finished()
            def done(self):
                self.parent.scheduleThread(self.parent.getReferents)

        # grab the referents (pointed to by garbage)
        class GetReferents(TaskThread):
            def setUp(self):
                if self.parent.numGarbage == 0:
                    self.finished()
                else:
                    if self.parent._args.verbose:
                        self.parent.notify.info('getting referents...')
                    self.index = 0
            def run(self):
                parent = self.parent
                for i in xrange(self.index, self.parent.numGarbage):
                    byNum, byRef = parent._getReferents(parent.garbage[i])
                    parent.referentsByNumber[i] = byNum
                    parent.referentsByReference[i] = byRef
                    if (not (i & 0x0F)) and (not self.timeLeft()):
                        # we've run out of time, save the index
                        self.index = i+1
                        return
                self.finished()
            def done(self):
                self.parent.scheduleThread(self.parent.getCycles)

        # find the cycles
        class GetCycles(TaskThread):
            def setUp(self):
                if self.parent._args.findCycles and self.parent.numGarbage > 0:
                    if self.parent._args.verbose:
                        self.parent.notify.info('detecting cycles...')
                    self.index = 0
                else:
                    self.finished()
            def run(self):
                for i in xrange(self.index, self.parent.numGarbage):
                    newCycles = self.parent._getCycles(i, self.parent.cycleSets)
                    self.parent.cycles.extend(newCycles)
                    # if we're not doing a full report, add this cycle's IDs to the master set
                    if not self.parent._args.fullReport:
                        for cycle in newCycles:
                            self.parent.cycleIds.update(set(cycle))
                    if (not (i & 0x0F)) and (not self.timeLeft()):
                        # we've run out of time, save the index
                        self.index = i+1
                        return
                self.finished()
            def done(self):
                self.parent.scheduleThread(self.parent.createReport)

        class CreateReport(TaskThread):
            def setUp(self):
                self.s = ['===== GarbageReport: \'%s\' (%s items) =====' % (
                    self.parent._args.name, self.parent.numGarbage)]
                if self.parent.numGarbage == 0:
                    self.finished()
                else:
                    self.curPhase = 0
                    self.index = 0
                    # make a list of the ids we will actually be printing
                    if self.parent._args.fullReport:
                        self.garbageIds = range(self.parent.numGarbage)
                    else:
                        self.garbageIds = list(self.parent.cycleIds)
                        self.garbageIds.sort()
                    self.numGarbage = len(self.garbageIds)
            def run(self):
                if self.curPhase == 0:
                    # log each individual item with a number in front of it
                    if self.index == 0:
                        if not self.parent._args.fullReport:
                            abbrev = '(abbreviated) '
                        else:
                            abbrev = ''
                        self.s.append('\n===== Garbage Items %s=====' % abbrev)
                        digits = 0
                        n = self.parent.numGarbage
                        while n > 0:
                            digits += 1
                            n /= 10
                        self.digits = digits
                        self.format = '%0' + '%s' % digits + 'i:%s \t%s'
                    for i in xrange(self.index, self.numGarbage):
                        id = self.garbageIds[i]
                        objStr = safeRepr(self.parent.garbage[id])
                        maxLen = 5000
                        if len(objStr) > maxLen:
                            snip = '<SNIP>'
                            objStr = '%s%s' % (objStr[:(maxLen-len(snip))], snip)
                        self.s.append(self.format % (id, itype(self.parent.garbage[id]), objStr))
                        if (not (i & 0x7F)) and (not self.timeLeft()):
                            # we've run out of time, save the index
                            self.index = i+1
                            return
                    self.curPhase = 1
                    self.index = 0
                if self.curPhase == 1:
                    if self.parent._args.findCycles:
                        if self.index == 0:
                            self.s.append('\n===== Cycles =====')
                        for i in xrange(self.index, len(self.parent.cycles)):
                            self.s.append('%s' % self.parent.cycles[i])
                            if (not (i & 0x7F)) and (not self.timeLeft()):
                                # we've run out of time, save the index
                                self.index = i+1
                                return
                    self.curPhase = 2
                    self.index = 0
                if self.parent._args.fullReport:
                    format = '%0' + '%s' % self.digits + 'i:%s'
                    if self.curPhase == 2:
                        if self.index == 0:
                            self.s.append('\n===== Referrers By Number (what is referring to garbage item?) =====')
                        for i in xrange(self.index, self.parent.numGarbage):
                            self.s.append(format % (i, self.parent.referrersByNumber[i]))
                            if (not (i & 0x7F)) and (not self.timeLeft()):
                                # we've run out of time, save the index
                                self.index = i+1
                                return
                        self.curPhase = 3
                        self.index = 0
                    if self.curPhase == 3:
                        if self.index == 0:
                            self.s.append('\n===== Referents By Number (what is garbage item referring to?) =====')
                        for i in xrange(self.index, self.parent.numGarbage):
                            self.s.append(format % (i, self.parent.referentsByNumber[i]))
                            if (not (i & 0x7F)) and (not self.timeLeft()):
                                # we've run out of time, save the index
                                self.index = i+1
                                return
                        self.curPhase = 4
                        self.index = 0
                    if self.curPhase == 4:
                        if self.index == 0:
                            self.s.append('\n===== Referrers (what is referring to garbage item?) =====')
                        for i in xrange(self.index, self.parent.numGarbage):
                            self.s.append(format % (i, self.parent.referrersByReference[i]))
                            if (not (i & 0x7F)) and (not self.timeLeft()):
                                # we've run out of time, save the index
                                self.index = i+1
                                return
                        self.curPhase = 5
                        self.index = 0
                    if self.curPhase == 5:
                        if self.index == 0:
                            self.s.append('\n===== Referents (what is garbage item referring to?) =====')
                        for i in xrange(self.index, self.parent.numGarbage):
                            self.s.append(format % (i, self.parent.referentsByReference[i]))
                            if (not (i & 0x7F)) and (not self.timeLeft()):
                                # we've run out of time, save the index
                                self.index = i+1
                                return
                self.finished()

            def done(self):
                self.parent._report = self.s
                self.parent.scheduleThread(self.parent.printReport)

        class PrintReport(TaskThread):
            def setUp(self):
                if not self.parent._args.log:
                    self.finished()
                else:
                    self.index = 0
            def run(self):
                if self.index > 0:
                    self.parent.notify.info('RESUME')
                for i in xrange(self.index, len(self.parent._report)):
                    print self.parent._report[i]
                    if (not (i & 0x3F)) and (not self.timeLeft()):
                        self.parent.notify.info('SUSPEND')
                        # we've run out of time, save the index
                        self.index = i+1
                        return
                self.finished()
            def done(self):
                if self.parent._args.doneCallback:
                    self.parent._args.doneCallback(self.parent)

        self.getReferrers = GetReferrers()
        self.getReferents = GetReferents()
        self.getCycles = GetCycles()
        self.createReport = CreateReport()
        self.printReport = PrintReport()

        self.scheduleThread(self.getReferrers)

    def destroy(self):
        print 'GarbageReport.destroy'
        del self.getReferrers
        del self.getReferents
        del self.getCycles
        del self.createReport
        del self.printReport
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
