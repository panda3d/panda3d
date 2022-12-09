"""Contains utility classes for debugging memory leaks."""

__all__ = ['FakeObject', '_createGarbage', 'GarbageReport', 'GarbageLogger']

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import fastRepr, AlphabetCounter, itype
from direct.showbase.Job import Job
from direct.showbase.JobManagerGlobal import jobMgr
from direct.showbase.MessengerGlobal import messenger
from panda3d.core import ConfigVariableBool
import gc
import sys

GarbageCycleCountAnnounceEvent = 'announceGarbageCycleDesc2num'

if sys.version_info >= (3, 0):
    xrange = range

class FakeObject:
    pass

class FakeDelObject:
    def __del__(self):
        pass

def _createGarbage(num=1):
    for i in xrange(num):
        a = FakeObject()
        b = FakeObject()
        a.other = b
        b.other = a
        a = FakeDelObject()
        b = FakeDelObject()
        a.other = b
        b.other = a

class GarbageReport(Job):
    """Detects leaked Python objects (via gc.collect()) and reports on garbage
    items, garbage-to-garbage references, and garbage cycles.
    If you just want to dump the report to the log, use GarbageLogger."""
    notify = directNotify.newCategory("GarbageReport")

    def __init__(self, name, log=True, verbose=False, fullReport=False, findCycles=True,
                 threaded=False, doneCallback=None, autoDestroy=False, priority=None,
                 safeMode=False, delOnly=False, collect=True):
        # if autoDestroy is True, GarbageReport will self-destroy after logging
        # if false, caller is responsible for calling destroy()
        # if threaded is True, processing will be performed over multiple frames
        # if collect is False, we assume that the caller just did a collect and the results
        # are still in gc.garbage
        Job.__init__(self, name)
        # stick the arguments onto a ScratchPad so we can delete them all at once
        self._args = ScratchPad(name=name, log=log, verbose=verbose, fullReport=fullReport,
                                findCycles=findCycles, doneCallback=doneCallback,
                                autoDestroy=autoDestroy, safeMode=safeMode, delOnly=delOnly,
                                collect=collect)
        if priority is not None:
            self.setPriority(priority)
        jobMgr.add(self)
        if not threaded:
            jobMgr.finish(self)

    def run(self):
        # do the garbage collection
        oldFlags = gc.get_debug()

        if self._args.delOnly:
            # do a collect without SAVEALL, to identify the instances that are involved in
            # cycles with instances that define __del__
            # cycles that do not involve any instances that define __del__ are cleaned up
            # automatically by Python, but they also appear in gc.garbage when SAVEALL is set
            gc.set_debug(0)
            if self._args.collect:
                gc.collect()
            garbageInstances = gc.garbage[:]
            del gc.garbage[:]
            # only yield if there's more time-consuming work to do,
            # if there's no garbage, give instant feedback
            if len(garbageInstances) > 0:
                yield None
            # don't repr the garbage list if we don't have to
            if self.notify.getDebug():
                self.notify.debug('garbageInstances == %s' % fastRepr(garbageInstances))

            self.numGarbageInstances = len(garbageInstances)
            # grab the ids of the garbage instances (objects with __del__)
            self.garbageInstanceIds = set()
            for i in xrange(len(garbageInstances)):
                self.garbageInstanceIds.add(id(garbageInstances[i]))
                if not (i % 20):
                    yield None
            # then release the list of instances so that it doesn't interfere with the gc.collect() below
            del garbageInstances
        else:
            self.garbageInstanceIds = set()

        # do a SAVEALL pass so that we have all of the objects involved in legitimate garbage cycles
        # without SAVEALL, gc.garbage only contains objects with __del__ methods
        gc.set_debug(gc.DEBUG_SAVEALL)
        if self._args.collect:
            gc.collect()
        self.garbage = gc.garbage[:]
        del gc.garbage[:]
        # only yield if there's more time-consuming work to do,
        # if there's no garbage, give instant feedback
        if len(self.garbage) > 0:
            yield None
        # don't repr the garbage list if we don't have to
        if self.notify.getDebug():
            self.notify.debug('self.garbage == %s' % fastRepr(self.garbage))
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

        self._id2garbageInfo = {}

        self.cycles = []
        self.cyclesBySyntax = []
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

        for i in xrange(self.numGarbage):
            if hasattr(self.garbage[i], '_garbageInfo') and callable(self.garbage[i]._garbageInfo):
                try:
                    info = self.garbage[i]._garbageInfo()
                except Exception as e:
                    info = str(e)
                self._id2garbageInfo[id(self.garbage[i])] = info
                yield None
            else:
                if not (i % 20):
                    yield None

        # find the cycles
        if self._args.findCycles and self.numGarbage > 0:
            if self._args.verbose:
                self.notify.info('calculating cycles...')
            for i in xrange(self.numGarbage):
                yield None
                for newCycles in self._getCycles(i, self.uniqueCycleSets):
                    yield None
                self.cycles.extend(newCycles)
                # create a representation of the cycle in human-readable form
                newCyclesBySyntax = []
                for cycle in newCycles:
                    cycleBySyntax = ''
                    objs = []
                    # leave off the last index, it's a repeat of the first index
                    for index in cycle[:-1]:
                        objs.append(self.garbage[index])
                        yield None
                    # make the list repeat so we can safely iterate off the end
                    numObjs = len(objs) - 1
                    objs.extend(objs)

                    # state variables for our loop below
                    numToSkip = 0
                    objAlreadyRepresented = False

                    # if cycle starts off with an instance dict, start with the instance instead
                    startIndex = 0
                    # + 1 to include a reference back to the first object
                    endIndex = numObjs + 1
                    if type(objs[0]) is dict and hasattr(objs[-1], '__dict__'):
                        startIndex -= 1
                        endIndex -= 1

                    for index in xrange(startIndex, endIndex):
                        if numToSkip:
                            numToSkip -= 1
                            continue
                        obj = objs[index]
                        if hasattr(obj, '__dict__'):
                            if not objAlreadyRepresented:
                                cycleBySyntax += '%s' % obj.__class__.__name__
                            cycleBySyntax += '.'
                            # skip past the instance dict and get the member obj
                            numToSkip += 1
                            member = objs[index+2]
                            for key, value in obj.__dict__.items():
                                if value is member:
                                    break
                                yield None
                            else:
                                key = '<unknown member name>'
                            cycleBySyntax += '%s' % key
                            objAlreadyRepresented = True
                        elif type(obj) is dict:
                            cycleBySyntax += '{'
                            # get object referred to by dict
                            val = objs[index+1]
                            for key, value in obj.items():
                                if value is val:
                                    break
                                yield None
                            else:
                                key = '<unknown key>'
                            cycleBySyntax += '%s}' % fastRepr(key)
                            objAlreadyRepresented = True
                        elif type(obj) in (tuple, list):
                            brackets = {
                                tuple: '()',
                                list: '[]',
                                }[type(obj)]
                            # get object being referenced by container
                            nextObj = objs[index+1]
                            cycleBySyntax += brackets[0]
                            for index in xrange(len(obj)):
                                if obj[index] is nextObj:
                                    index = str(index)
                                    break
                                yield None
                            else:
                                index = '<unknown index>'
                            cycleBySyntax += '%s%s' % (index, brackets[1])
                            objAlreadyRepresented = True
                        else:
                            cycleBySyntax += '%s --> ' % itype(obj)
                            objAlreadyRepresented = False
                    newCyclesBySyntax.append(cycleBySyntax)
                    yield None
                self.cyclesBySyntax.extend(newCyclesBySyntax)
                # if we're not doing a full report, add this cycle's IDs to the master set
                if not self._args.fullReport:
                    for cycle in newCycles:
                        yield None
                        self.cycleIds.update(set(cycle))

        self.numCycles = len(self.cycles)

        if self._args.findCycles:
            s = ['===== GarbageReport: \'%s\' (%s %s) =====' % (
                self._args.name, self.numCycles,
                ('cycle' if self.numCycles == 1 else 'cycles'))]
        else:
            s = ['===== GarbageReport: \'%s\' =====' % (
                self._args.name)]
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
                n = n // 10
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
                s.append('===== Garbage Cycles (Garbage Item Numbers) =====')
                ac = AlphabetCounter()
                for i in xrange(self.numCycles):
                    yield None
                    s.append('%s:%s' % (ac.next(), self.cycles[i]))

            if self._args.findCycles:
                s.append('===== Garbage Cycles (Python Syntax) =====')
                ac = AlphabetCounter()
                for i in xrange(len(self.cyclesBySyntax)):
                    yield None
                    s.append('%s:%s' % (ac.next(), self.cyclesBySyntax[i]))

            if len(self._id2garbageInfo):
                s.append('===== Garbage Custom Info =====')
                ac = AlphabetCounter()
                for i in xrange(len(self.cyclesBySyntax)):
                    yield None
                    counter = ac.next()
                    _id = id(self.garbage[i])
                    if _id in self._id2garbageInfo:
                        s.append('%s:%s' % (counter, self._id2garbageInfo[_id]))

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
            self.notify.info('===== Garbage Report Done =====')
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
        # don't get rid of these, we might need them
        #del self.numGarbage
        #del self.numCycles
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

    def getNumCycles(self):
        # if the job hasn't run yet, we don't have a numCycles yet
        return self.numCycles

    def getDesc2numDict(self):
        # dict of python-syntax leak -> number of that type of leak
        desc2num = {}
        for cycleBySyntax in self.cyclesBySyntax:
            desc2num.setdefault(cycleBySyntax, 0)
            desc2num[cycleBySyntax] += 1
        return desc2num

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
        # check if the root object is one of the garbage instances (has __del__)
        objId = id(self.garbage[rootId])
        numDelInstances = int(objId in self.garbageInstanceIds)
        stateStack.push(([rootId], rootId, numDelInstances, 0))
        while True:
            yield None
            if len(stateStack) == 0:
                break
            candidateCycle, curId, numDelInstances, resumeIndex = stateStack.pop()
            if self.notify.getDebug():
                if self._args.delOnly:
                    print('restart: %s root=%s cur=%s numDelInstances=%s resume=%s' % (
                        candidateCycle, rootId, curId, numDelInstances, resumeIndex))
                else:
                    print('restart: %s root=%s cur=%s resume=%s' % (
                        candidateCycle, rootId, curId, resumeIndex))
            for index in xrange(resumeIndex, len(self.referentsByNumber[curId])):
                yield None
                refId = self.referentsByNumber[curId][index]
                if self.notify.getDebug():
                    print('       : %s -> %s' % (curId, refId))
                if refId == rootId:
                    # we found a cycle! mark it down and move on to the next refId
                    normCandidateCycle = self._getNormalizedCycle(candidateCycle)
                    normCandidateCycleTuple = tuple(normCandidateCycle)
                    if not normCandidateCycleTuple in uniqueCycleSets:
                        # cycles with no instances that define __del__ will be
                        # cleaned up by Python
                        if (not self._args.delOnly) or numDelInstances >= 1:
                            if self.notify.getDebug():
                                print('  FOUND: ', normCandidateCycle + [normCandidateCycle[0],])
                            cycles.append(normCandidateCycle + [normCandidateCycle[0],])
                            uniqueCycleSets.add(normCandidateCycleTuple)
                elif refId in candidateCycle:
                    pass
                elif refId is not None:
                    # check if this object is one of the garbage instances (has __del__)
                    objId = id(self.garbage[refId])
                    numDelInstances += int(objId in self.garbageInstanceIds)
                    # this refId does not complete a cycle. Mark down
                    # where we are in this list of referents, then
                    # start looking through the referents of the new refId
                    stateStack.push((list(candidateCycle), curId, numDelInstances, index+1))
                    stateStack.push((list(candidateCycle) + [refId], refId, numDelInstances, 0))
                    break
        yield cycles

class GarbageLogger(GarbageReport):
    """If you just want to log the current garbage to the log file, make
    one of these. It automatically destroys itself after logging"""
    def __init__(self, name, *args, **kArgs):
        kArgs['log'] = True
        kArgs['autoDestroy'] = True
        GarbageReport.__init__(self, name, *args, **kArgs)

class _CFGLGlobals:
    # for checkForGarbageLeaks
    LastNumGarbage = 0
    LastNumCycles = 0

def checkForGarbageLeaks():
    gc.collect()
    numGarbage = len(gc.garbage)
    if numGarbage > 0 and ConfigVariableBool('auto-garbage-logging', False):
        if (numGarbage != _CFGLGlobals.LastNumGarbage):
            print("")
            gr = GarbageReport('found garbage', threaded=False, collect=False)
            print("")
            _CFGLGlobals.LastNumGarbage = numGarbage
            _CFGLGlobals.LastNumCycles = gr.getNumCycles()
            messenger.send(GarbageCycleCountAnnounceEvent, [gr.getDesc2numDict()])
            gr.destroy()
        notify = directNotify.newCategory("GarbageDetect")
        if ConfigVariableBool('allow-garbage-cycles', True):
            func = notify.warning
        else:
            func = notify.error
        func('%s garbage cycles found, see info above' % _CFGLGlobals.LastNumCycles)
    return numGarbage

def b_checkForGarbageLeaks(wantReply=False):
    if not __dev__:
        return 0
    # does a garbage collect on the client and the AI
    # returns number of client garbage leaks
    # logs leak info and terminates (if configured to do so)
    try:
        # if this is the client, tell the AI to check for leaks too
        base.cr.timeManager
    except:
        pass
    else:
        if base.cr.timeManager:
            base.cr.timeManager.d_checkForGarbageLeaks(wantReply=wantReply)
    return checkForGarbageLeaks()
