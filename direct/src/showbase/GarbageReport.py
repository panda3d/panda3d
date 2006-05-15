from direct.directnotify import DirectNotifyGlobal
from direct.showbase import PythonUtil
from direct.showbase.PythonUtil import POD
import gc

class FakeObject:
    pass

def _createGarbage():
    a = FakeObject()
    b = FakeObject()
    a.other = b
    b.other = a

class CycleSearchState(POD):
    DataSet = {
        'curId': None,
        'cycleByNumSoFar': [],
        'cycleByRefSoFar': [],
        'nonGarbageDepth': 0,
        }

class GarbageReport:
    notify = DirectNotifyGlobal.directNotify.newCategory("GarbageReport")

    NotGarbage = 'NG'

    def __init__(self, name=None, log=True, fullReport=False, findCycles=True):
        # if log is True, GarbageReport will self-destroy after logging
        # if false, caller is responsible for calling destroy()
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

        # grab the referrers (pointing to garbage)
        self.referrersByReference = {}
        self.referrersByNumber = {}
        for i in xrange(self.numGarbage):
            byNum, byRef = self._getReferrers(self.garbage[i])
            self.referrersByNumber[i] = byNum
            self.referrersByReference[i] = byRef

        # grab the referents (pointed to by garbage)
        self.referentsByReference = {}
        self.referentsByNumber = {}
        for i in xrange(self.numGarbage):
            byNum, byRef = self._getReferents(self.garbage[i])
            self.referentsByNumber[i] = byNum
            self.referentsByReference[i] = byRef

        """
        # find the cycles
        if findCycles:
            self.cycles = self._getCycles()
            """

        s = '\n===== GarbageReport: \'%s\' (%s items) =====' % (name, self.numGarbage)
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

            """
            if findCycles:
                format = '\n%s'
                s += '\n\n===== Cycles ====='
                for cycle in self.cycles:
                    s += format % cycle
                    """

            format = '\n%0' + '%s' % digits + 'i:%s'
            s += '\n\n===== Referrers By Number (what is referring to garbage item?) ====='
            for i in xrange(self.numGarbage):
                s += format % (i, self.referrersByNumber[i])
            s += '\n\n===== Referents By Number (what is garbage item referring to?) ====='
            for i in xrange(self.numGarbage):
                s += format % (i, self.referentsByNumber[i])

            if fullReport:
                s += '\n\n===== Referrers (what is referring to garbage item?) ====='
                for i in xrange(self.numGarbage):
                    s += format % (i, self.referrersByReference[i])
                s += '\n\n===== Referents (what is garbage item referring to?) ====='
                for i in xrange(self.numGarbage):
                    s += format % (i, self.referentsByReference[i])

        self._report = s
        if log:
            self.notify.info(self._report)
            self.destroy()

    def getNumItems(self):
        return self.numGarbage

    def getGarbage(self):
        return self.garbage

    def getReport(self):
        return self._report

    def destroy(self):
        del self.garbage
        del self.numGarbage
        del self.referrersByReference
        del self.referrersByNumber
        del self.referentsByReference
        del self.referentsByNumber
        if hasattr(self, 'cycles'):
            del self.cycles
        del self._report

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
        # dict of already-found cycles as sets so we can avoid duplicate cycle reports
        cycleSets = {}
        # set of garbage item IDs involved in already-discovered cycles
        visited = set()
        self.stateStack = Stack()
        for rootId in xrange(len(self.garbage)):
            cycleSoFar = []
            self.stateStack.push((rootId, rootId, cycles, cycleSoFar, cycleSets, visited))
            self._findCycles()
            self.stateStack.pop()
        assert len(self.stateStack) == 0
        return cycles

    def _findCycles(self):
        # rootId: id of first garbage item in potential cycle
        # curId: id of current garbage item under examination
        # cycles: list of complete cycles we have already found
        # cycleSoFar: list of IDs leading from rootId up to curId
        # cycleSets: dict of ID to list of cycle ID sets
        # visited: set of garbage item IDs from existing (already-found) cycles
        assert self.notify.debugCall()
        rootId, curId, cycles, cycleSoFar, cycleSets, visited = self.stateStack.top()
        cycleSoFar.append(curId)
        # make sure this is not a cycle that is already in the list of cycles
        if curId in cycleSets:
            cycleSet = set(cycleSoFar)
            cycleSet.add(curId)
            if cycleSet in cycleSets[curId]:
                return
        for refId in self.referentsByNumber[curId]:
            if refId == rootId:
                # we found a cycle!
                cycle = list(cycleSoFar) + [refId]
                cycleSet = set(cycle)
                # make sure this is not a duplicate of a previously-found cycle
                if cycleSet not in cycleSets.get(rootId, []):
                    cycles.append(cycle)
                    for id in cycle:
                        visited.add(id)
                        cycleSets.setdefault(id, [])
                        if cycleSet not in cycleSets[id]:
                            cycleSets[id].append(cycleSet)
            elif refId not in visited and refId not in cycleSoFar and refId != GarbageReport.NotGarbage:
                #print rootId, curId, refId, visited, cycles, cycleSoFar, cycleSets
                self.stateStack.push((rootId, refId, cycles, list(cycleSoFar), cycleSets, visited))
                self._findCycles()
                self.stateStack.pop()

"""
    def _getCycles(self):
        # returns list of lists, sublists are garbage reference cycles by number
        cycles = []
        for rootId in xrange(len(self.garbage)):
            curId = rootId
            stateStack = Stack()
            initialState = CycleSearchState(curId=curId, cycleByNumSoFar=[rootId],
                                            cycleByRefSoFar=[self.garbage[rootId]])
            stateStack.push(initialState)
            curState = stateStack.top()
            while not stateStack.isEmpty():
                for index in xrange(len(self.referentsByNumber[curId])):
                    refId = self.referentsByNumber[curId][index]
                    if refId is rootId:
                        # we found a cycle!
                        cyclesByNumber.append(curState.cycleByNumSoFar + [refId])
                        cyclesByReference.append(curState.cycleByRefSoFar + [

    def _getCycles(self):
        assert self.notify.debugCall()
        # returns list of lists, sublists are garbage reference cycles
        cycles = []
        # dict of already-found cycles as sets so we can avoid duplicate cycle reports
        cycleSets = {}
        # set of garbage item IDs involved in already-discovered cycles
        visited = set()
        for rootId in xrange(len(self.garbage)):
            for index in xrange(len(self.referentsByNumber[rootId])):
                refId = self.referentsByNumber[rootId][index]
                if refId is rootId:
                    # we found a cycle!
                    cycle = [rootId, rootId]
                    cycleSet = set(cycle)
                    # make sure this is not a duplicate of a previously-found cycle
                    if cycleSet not in cycleSets.get(rootId, []):
                        cycles.append(cycle)
                        for id in cycle:
                            visited.add(id)
                            cycleSets.setdefault(id, [])
                            if cycleSet not in cycleSets[id]:
                                cycleSets[id].append(cycleSet)
                elif refId is not GarbageReport.NotGarbage:
                    cycleSoFar = [rootId]
                    self._findCycles(rootId, refId, cycles, cycleSoFar, cycleSets, visited)
        return cycles
    """
