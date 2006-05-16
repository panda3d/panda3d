from direct.directnotify import DirectNotifyGlobal
from direct.showbase import PythonUtil
from direct.showbase.TaskThreaded import TaskThreaded
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
    notify = DirectNotifyGlobal.directNotify.newCategory("GarbageReport")

    NotGarbage = 'NG'

    def __init__(self, name, log=True, verbose=False, fullReport=False, findCycles=True,
                 threaded=False, doneCallback=None):
        # if log is True, GarbageReport will self-destroy after logging
        # if false, caller is responsible for calling destroy()
        # if threaded is True, processing will be performed over multiple frames
        TaskThreaded.__init__(self, name, threaded)
        # stick the arguments onto a ScratchPad so we can access them from the thread
        # functions and delete them all at once
        self._args = ScratchPad()
        self._args.name = name
        self._args.log = log
        self._args.verbose = verbose
        self._args.fullReport = fullReport
        self._args.findCycles = findCycles
        self._args.doneCallback = doneCallback

        # do the garbage collection
        wasOn = PythonUtil.gcDebugOn()
        oldFlags = gc.get_debug()
        if not wasOn:
            gc.set_debug(gc.DEBUG_SAVEALL)
        gc.collect()
        self.notify.debug('gc.garbage == %s' % gc.garbage)
        self.garbage = list(gc.garbage)
        self.notify.debug('self.garbage == %s' % self.garbage)
        del gc.garbage[:]
        if not wasOn:
            gc.set_debug(oldFlags)

        self.numGarbage = len(self.garbage)

        if self._args.verbose:
            self.notify.info('found %s garbage items' % self.numGarbage)

        self.scheduleNext(self.T_getReferrers)
    def T_getReferrers(self):

        # grab the referrers (pointing to garbage)
        self.referrersByReference = {}
        self.referrersByNumber = {}
        if self._args.fullReport:
            if self._args.verbose:
                self.notify.info('getting referrers...')
            # we need referents to detect cycles, but we don't need referrers
            for i in xrange(self.numGarbage):
                byNum, byRef = self._getReferrers(self.garbage[i])
                self.referrersByNumber[i] = byNum
                self.referrersByReference[i] = byRef

        self.scheduleNext(self.T_getReferents)
    def T_getReferents(self):

        # grab the referents (pointed to by garbage)
        self.referentsByReference = {}
        self.referentsByNumber = {}
        if self._args.verbose:
            self.notify.info('getting referents...')
        for i in xrange(self.numGarbage):
            byNum, byRef = self._getReferents(self.garbage[i])
            self.referentsByNumber[i] = byNum
            self.referentsByReference[i] = byRef

        self.scheduleNext(self.T_getCycles)
    def T_getCycles(self):

        # find the cycles
        if self._args.findCycles and self.numGarbage > 0:
            if self._args.verbose:
                self.notify.info('detecting cycles...')
            self.cycles = self._getCycles()

        self.scheduleNext(self.T_createReport)
    def T_createReport(self):

        s = '\n===== GarbageReport: \'%s\' (%s items) =====' % (self._args.name, self.numGarbage)
        if self.numGarbage > 0:
            # log each individual item with a number in front of it
            s += '\n\n===== Garbage Items ====='
            digits = 0
            n = self.numGarbage
            while n > 0:
                digits += 1
                n /= 10
            format = '\n%0' + '%s' % digits + 'i:%s \t%s'
            for i in range(len(self.garbage)):
                s += format % (i, type(self.garbage[i]), self.garbage[i])

            if self._args.findCycles:
                format = '\n%s'
                s += '\n\n===== Cycles ====='
                for cycle in self.cycles:
                    s += format % cycle

            if self._args.fullReport:
                format = '\n%0' + '%s' % digits + 'i:%s'
                s += '\n\n===== Referrers By Number (what is referring to garbage item?) ====='
                for i in xrange(self.numGarbage):
                    s += format % (i, self.referrersByNumber[i])
                s += '\n\n===== Referents By Number (what is garbage item referring to?) ====='
                for i in xrange(self.numGarbage):
                    s += format % (i, self.referentsByNumber[i])
                s += '\n\n===== Referrers (what is referring to garbage item?) ====='
                for i in xrange(self.numGarbage):
                    s += format % (i, self.referrersByReference[i])
                s += '\n\n===== Referents (what is garbage item referring to?) ====='
                for i in xrange(self.numGarbage):
                    s += format % (i, self.referentsByReference[i])

        self._report = s

        self.scheduleNext(self.T_printReport)
    def T_printReport(self):

        if self._args.log:
            self.notify.info(self._report)

        self.scheduleNext(self.T_completed)
    def T_completed(self):

        if self._args.doneCallback:
            self._args.doneCallback(self)

    def destroy(self):
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

    def getNumItems(self):
        return self.numGarbage

    def getGarbage(self):
        return self.garbage

    def getReport(self):
        return self._report

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

    def _getCycles(self):
        assert self.notify.debugCall()
        # returns list of lists, sublists are garbage reference cycles
        cycles = []
        # sets of cycle members, to avoid duplicates
        cycleSets = []
        stateStack = Stack()
        for rootId in xrange(len(self.garbage)):
            assert len(stateStack) == 0
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
    def __init__(self, *args, **kArgs):
        kArgs['log'] = True
        GarbageReport.__init__(self, *args, **kArgs)
    def T_completed(self):
        GarbageReport.T_completed(self)
        self.destroy()
