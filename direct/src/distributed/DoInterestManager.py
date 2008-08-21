"""
The DoInterestManager keeps track of which parent/zones that we currently
have interest in.  When you want to "look" into a zone you add an interest
to that zone.  When you want to get rid of, or ignore, the objects in that
zone, remove interest in that zone.

p.s. A great deal of this code is just code moved from ClientRepository.py.
"""

from pandac.PandaModules import *
from MsgTypes import *
from direct.showbase.PythonUtil import *
from direct.showbase import DirectObject
from PyDatagram import PyDatagram
from direct.directnotify.DirectNotifyGlobal import directNotify
import types
from direct.showbase.PythonUtil import report

class InterestState:
    StateActive = 'Active'
    StatePendingDel = 'PendingDel'
    def __init__(self, desc, state, context, event, parentId, zoneIdList,
                 eventCounter, auto=False):
        self.desc = desc
        self.state = state
        self.context = context
        # We must be ready to keep track of multiple events. If somebody
        # requested an interest to be removed and we get a second request
        # for removal of the same interest before we get a response for the
        # first interest removal, we now have two parts of the codebase
        # waiting for a response on the removal of a single interest.
        self.events = []
        self.eventCounter = eventCounter
        if event:
            self.addEvent(event)
        self.parentId = parentId
        self.zoneIdList = zoneIdList
        self.auto = auto
    def addEvent(self, event):
        self.events.append(event)
        self.eventCounter.num += 1
    def getEvents(self):
        return list(self.events)
    def clearEvents(self):
        self.eventCounter.num -= len(self.events)
        assert self.eventCounter.num >= 0
        self.events = []
    def sendEvents(self):
        for event in self.events:
            messenger.send(event)
        self.clearEvents()
    def setDesc(self, desc):
        self.desc = desc
    def isPendingDelete(self):
        return self.state == InterestState.StatePendingDel
    def __repr__(self):
        return 'InterestState(desc=%s, state=%s, context=%s, event=%s, parentId=%s, zoneIdList=%s)' % (
            self.desc, self.state, self.context, self.events, self.parentId, self.zoneIdList)

class InterestHandle:
    """This class helps to ensure that valid handles get passed in to DoInterestManager funcs"""
    def __init__(self, id):
        self._id = id
    def asInt(self):
        return self._id
    def __eq__(self, other):
        if type(self) == type(other):
            return self._id == other._id
        return self._id == other
    def __repr__(self):
        return '%s(%s)' % (self.__class__.__name__, self._id)

# context value for interest changes that have no complete event
NO_CONTEXT = 0

class DoInterestManager(DirectObject.DirectObject):
    """
    Top level Interest Manager
    """
    notify = directNotify.newCategory("DoInterestManager")
    try:
        tempbase = base
    except:
        tempbase = simbase
    InterestDebug = tempbase.config.GetBool('interest-debug', False)
    del tempbase

    # 'handle' is a number that represents a single interest set that the
    # client has requested; the interest set may be modified
    _HandleSerialNum = 0
    # high bit is reserved for server interests
    _HandleMask = 0x7FFF

    # 'context' refers to a single request to change an interest set
    _ContextIdSerialNum = 100
    _ContextIdMask = 0x3FFFFFFF # avoid making Python create a long

    _interests = {}
    if __debug__:
        _debug_interestHistory = []
        _debug_maxDescriptionLen = 40

    _SerialGen = SerialNumGen()
    _SerialNum = serialNum()

    def __init__(self):
        assert DoInterestManager.notify.debugCall()
        DirectObject.DirectObject.__init__(self)
        self._addInterestEvent = uniqueName('DoInterestManager-Add')
        self._removeInterestEvent = uniqueName('DoInterestManager-Remove')
        self._noNewInterests = False
        self._completeDelayedCallback = None
        # keep track of request contexts that have not completed
        self._completeEventCount = ScratchPad(num=0)
        self._allInterestsCompleteCallbacks = []

    def __verbose(self):
        return self.InterestDebug or self.getVerbose()

    def _getAnonymousEvent(self, desc):
        return 'anonymous-%s-%s' % (desc, DoInterestManager._SerialGen.next())

    def setNoNewInterests(self, flag):
        self._noNewInterests = flag

    def noNewInterests(self):
        return self._noNewInterests

    def setAllInterestsCompleteCallback(self, callback):
        if ((self._completeEventCount.num == 0) and
            (self._completeDelayedCallback is None)):
            callback()
        else:
            self._allInterestsCompleteCallbacks.append(callback)

    def getAllInterestsCompleteEvent(self):
        return 'allInterestsComplete-%s' % DoInterestManager._SerialNum

    def resetInterestStateForConnectionLoss(self):
        DoInterestManager._interests.clear()
        self._completeEventCount = ScratchPad(num=0)
        if __debug__:
            self._addDebugInterestHistory("RESET", "", 0, 0, 0, [])

    def isValidInterestHandle(self, handle):
        # pass in a handle (or anything else) and this will return true if it is
        # still a valid interest handle
        if not isinstance(handle, InterestHandle):
            return False
        return DoInterestManager._interests.has_key(handle.asInt())

    def updateInterestDescription(self, handle, desc):
        iState = DoInterestManager._interests.get(handle.asInt())
        if iState:
            iState.setDesc(desc)

    def addInterest(self, parentId, zoneIdList, description, event=None):
        """
        Look into a (set of) zone(s).
        """
        assert DoInterestManager.notify.debugCall()
        handle = self._getNextHandle()
        # print 'base.cr.addInterest(',description,',',handle,'):',globalClock.getFrameCount()
        if self._noNewInterests:
            DoInterestManager.notify.warning(
                "addInterest: addingInterests on delete: %s" % (handle))
            return

        # make sure we've got parenting rules set in the DC
        if parentId not in (self.getGameDoId(),):
            parent = self.getDo(parentId)
            if not parent:
                DoInterestManager.notify.error(
                    'addInterest: attempting to add interest under unknown object %s' % parentId)
            else:
                if not parent.hasParentingRules():
                    DoInterestManager.notify.error(
                        'addInterest: no setParentingRules defined in the DC for object %s (%s)'
                        '' % (parentId, parent.__class__.__name__))

                    
        
        if event:
            contextId = self._getNextContextId()
        else:
            contextId = 0
            # event = self._getAnonymousEvent('addInterest')
            
        DoInterestManager._interests[handle] = InterestState(
            description, InterestState.StateActive, contextId, event, parentId, zoneIdList, self._completeEventCount)
        if self.__verbose():
            print 'CR::INTEREST.addInterest(handle=%s, parentId=%s, zoneIdList=%s, description=%s, event=%s)' % (
                handle, parentId, zoneIdList, description, event)
        self._sendAddInterest(handle, contextId, parentId, zoneIdList, description)
        if event:
            messenger.send(self._getAddInterestEvent(), [event])
        assert self.printInterestsIfDebug()
        return InterestHandle(handle)

    def addAutoInterest(self, parentId, zoneIdList, description):
        """
        Look into a (set of) zone(s).
        """
        assert DoInterestManager.notify.debugCall()
        handle = self._getNextHandle()
        if self._noNewInterests:
            DoInterestManager.notify.warning(
                "addInterest: addingInterests on delete: %s" % (handle))
            return

        # make sure we've got parenting rules set in the DC
        if parentId not in (self.getGameDoId(),):
            parent = self.getDo(parentId)
            if not parent:
                DoInterestManager.notify.error(
                    'addInterest: attempting to add interest under unknown object %s' % parentId)
            else:
                if not parent.hasParentingRules():
                    DoInterestManager.notify.error(
                        'addInterest: no setParentingRules defined in the DC for object %s (%s)'
                        '' % (parentId, parent.__class__.__name__))

        DoInterestManager._interests[handle] = InterestState(
            description, InterestState.StateActive, 0, None, parentId, zoneIdList, self._completeEventCount, True)
        if self.__verbose():
            print 'CR::INTEREST.addInterest(handle=%s, parentId=%s, zoneIdList=%s, description=%s)' % (
                handle, parentId, zoneIdList, description)
        assert self.printInterestsIfDebug()
        return InterestHandle(handle)

    def removeInterest(self, handle, event = None):
        """
        Stop looking in a (set of) zone(s)
        """
        # print 'base.cr.removeInterest(',handle,'):',globalClock.getFrameCount()

        assert DoInterestManager.notify.debugCall()
        assert isinstance(handle, InterestHandle)
        existed = False
        if not event:
            event = self._getAnonymousEvent('removeInterest')
        handle = handle.asInt()
        if DoInterestManager._interests.has_key(handle):
            existed = True
            intState = DoInterestManager._interests[handle]
            if event:
                messenger.send(self._getRemoveInterestEvent(),
                               [event, intState.parentId, intState.zoneIdList])
            if intState.isPendingDelete():
                self.notify.warning(
                    'removeInterest: interest %s already pending removal' %
                    handle)
                # this interest is already pending delete, so let's just tack this
                # callback onto the list
                if event is not None:
                    intState.addEvent(event)
            else:
                if len(intState.events) > 0:
                    # we're not pending a removal, but we have outstanding events?
                    # probably we are waiting for an add/alter complete.
                    # should we send those events now?
                    assert self.notify.warning('removeInterest: abandoning events: %s' %
                                               intState.events)
                    intState.clearEvents()
                intState.state = InterestState.StatePendingDel
                contextId = self._getNextContextId()
                intState.context = contextId
                if event:
                    intState.addEvent(event)
                self._sendRemoveInterest(handle, contextId)
                if not event:
                    self._considerRemoveInterest(handle)
                if self.__verbose():
                    print 'CR::INTEREST.removeInterest(handle=%s, event=%s)' % (
                        handle, event)
        else:
            DoInterestManager.notify.warning(
                "removeInterest: handle not found: %s" % (handle))
        assert self.printInterestsIfDebug()
        return existed
    
    def removeAutoInterest(self, handle):
        """
        Stop looking in a (set of) zone(s)
        """
        assert DoInterestManager.notify.debugCall()
        assert isinstance(handle, InterestHandle)
        existed = False
        handle = handle.asInt()
        if DoInterestManager._interests.has_key(handle):
            existed = True
            intState = DoInterestManager._interests[handle]
            if intState.isPendingDelete():
                self.notify.warning(
                    'removeInterest: interest %s already pending removal' %
                    handle)
                # this interest is already pending delete, so let's just tack this
                # callback onto the list
            else:
                if len(intState.events) > 0:
                    # we're not pending a removal, but we have outstanding events?
                    # probably we are waiting for an add/alter complete.
                    # should we send those events now?
                    self.notify.warning('removeInterest: abandoning events: %s' %
                                        intState.events)
                    intState.clearEvents()
                intState.state = InterestState.StatePendingDel
                self._considerRemoveInterest(handle)
                if self.__verbose():
                    print 'CR::INTEREST.removeAutoInterest(handle=%s)' % (handle)
        else:
            DoInterestManager.notify.warning(
                "removeInterest: handle not found: %s" % (handle))
        assert self.printInterestsIfDebug()
        return existed

    @report(types = ['args'], dConfigParam = 'want-guildmgr-report')
    def removeAIInterest(self, handle):
        """
        handle is NOT an InterestHandle.  It's just a bare integer representing an
        AI opened interest. We're making the client close down this interest since
        the AI has trouble removing interests(that its opened) when the avatar goes
        offline.  See GuildManager(UD) for how it's being used.
        """
        self._sendRemoveAIInterest(handle)

    def alterInterest(self, handle, parentId, zoneIdList, description=None,
                      event=None):
        """
        Removes old interests and adds new interests.

        Note that when an interest is changed, only the most recent
        change's event will be triggered. Previous events are abandoned.
        If this is a problem, consider opening multiple interests.
        """
        assert DoInterestManager.notify.debugCall()
        assert isinstance(handle, InterestHandle)
        #assert not self._noNewInterests
        handle = handle.asInt()
        if self._noNewInterests:
            DoInterestManager.notify.warning(
                "alterInterest: addingInterests on delete: %s" % (handle))
            return

        exists = False
        if event is None:
            event = self._getAnonymousEvent('alterInterest')
        if DoInterestManager._interests.has_key(handle):
            if description is not None:
                DoInterestManager._interests[handle].desc = description
            else:
                description = DoInterestManager._interests[handle].desc

            # are we overriding an existing change?
            if DoInterestManager._interests[handle].context != NO_CONTEXT:
                DoInterestManager._interests[handle].clearEvents()

            contextId = self._getNextContextId()
            DoInterestManager._interests[handle].context = contextId
            DoInterestManager._interests[handle].parentId = parentId            
            DoInterestManager._interests[handle].zoneIdList = zoneIdList
            DoInterestManager._interests[handle].addEvent(event)

            if self.__verbose():
                print 'CR::INTEREST.alterInterest(handle=%s, parentId=%s, zoneIdList=%s, description=%s, event=%s)' % (
                    handle, parentId, zoneIdList, description, event)
            self._sendAddInterest(handle, contextId, parentId, zoneIdList, description, action='modify')
            exists = True
            assert self.printInterestsIfDebug()
        else:
            DoInterestManager.notify.warning(
                "alterInterest: handle not found: %s" % (handle))
        return exists

    def openAutoInterests(self, obj):
        if hasattr(obj, '_autoInterestHandle'):
            # must be multiple inheritance
            self.notify.debug('openAutoInterests(%s): interests already open' % obj.__class__.__name__)
            return
        autoInterests = obj.getAutoInterests()
        obj._autoInterestHandle = None
        if not len(autoInterests):
            return
        obj._autoInterestHandle = self.addAutoInterest(obj.doId, autoInterests, '%s-autoInterest' % obj.__class__.__name__)
    def closeAutoInterests(self, obj):
        if not hasattr(obj, '_autoInterestHandle'):
            # must be multiple inheritance
            self.notify.debug('closeAutoInterests(%s): interests already closed' % obj)
            return
        if obj._autoInterestHandle is not None:
            self.removeAutoInterest(obj._autoInterestHandle)
        del obj._autoInterestHandle

    # events for InterestWatcher
    def _getAddInterestEvent(self):
        return self._addInterestEvent
    def _getRemoveInterestEvent(self):
        return self._removeInterestEvent

    def _getInterestState(self, handle):
        return DoInterestManager._interests[handle]

    def _getNextHandle(self):
        handle = DoInterestManager._HandleSerialNum
        while True:
            handle = (handle + 1) & DoInterestManager._HandleMask
            # skip handles that are already in use
            if handle not in DoInterestManager._interests:
                break
            DoInterestManager.notify.warning(
                'interest %s already in use' % handle)
        DoInterestManager._HandleSerialNum = handle
        return DoInterestManager._HandleSerialNum
    def _getNextContextId(self):
        contextId = DoInterestManager._ContextIdSerialNum
        while True:
            contextId = (contextId + 1) & DoInterestManager._ContextIdMask
            # skip over the 'no context' id
            if contextId != NO_CONTEXT:
                break
        DoInterestManager._ContextIdSerialNum = contextId
        return DoInterestManager._ContextIdSerialNum

    def _considerRemoveInterest(self, handle):
        """
        Consider whether we should cull the interest set.
        """
        assert DoInterestManager.notify.debugCall()
        
        if DoInterestManager._interests.has_key(handle):
            if DoInterestManager._interests[handle].isPendingDelete():
                # make sure there is no pending event for this interest
                if DoInterestManager._interests[handle].context == NO_CONTEXT:
                    assert len(DoInterestManager._interests[handle].events) == 0
                    del DoInterestManager._interests[handle]

    if __debug__:
        def printInterestsIfDebug(self):
            if DoInterestManager.notify.getDebug():
                self.printInterests()
            return 1 # for assert

        def _addDebugInterestHistory(self, action, description, handle,
                                     contextId, parentId, zoneIdList):
            if description is None:
                description = ''
            DoInterestManager._debug_interestHistory.append(
                (action, description, handle, contextId, parentId, zoneIdList))
            DoInterestManager._debug_maxDescriptionLen = max(
                DoInterestManager._debug_maxDescriptionLen, len(description))

        def printInterestHistory(self):
            print "***************** Interest History *************"
            format = '%9s %' + str(DoInterestManager._debug_maxDescriptionLen) + 's %6s %6s %9s %s'
            print format % (
                "Action", "Description", "Handle", "Context", "ParentId",
                "ZoneIdList")
            for i in DoInterestManager._debug_interestHistory:
                print format % tuple(i)
            print "Note: interests with a Context of 0 do not get" \
                " done/finished notices."
            
        def printInterestSets(self):
            print "******************* Interest Sets **************"
            format = '%6s %' + str(DoInterestManager._debug_maxDescriptionLen) + 's %11s %11s %8s %8s %8s'
            print format % (
                "Handle", "Description", 
                "ParentId", "ZoneIdList",
                "State", "Context",
                "Event")
            for id, state in DoInterestManager._interests.items():
                if len(state.events) == 0:
                    event = ''
                elif len(state.events) == 1:
                    event = state.events[0]
                else:
                    event = state.events
                print format % (id, state.desc,
                                state.parentId, state.zoneIdList,
                                state.state, state.context,
                                event)
            print "************************************************"

        def printInterests(self):
            self.printInterestHistory()
            self.printInterestSets()
            
    def _sendAddInterest(self, handle, contextId, parentId, zoneIdList, description,
                         action=None):
        """
        Part of the new otp-server code.

        handle is a client-side created number that refers to
                a set of interests.  The same handle number doesn't
                necessarily have any relationship to the same handle
                on another client.
        """
        assert DoInterestManager.notify.debugCall()
        if __debug__:
            if isinstance(zoneIdList, types.ListType):
                zoneIdList.sort()
            if action is None:
                action = 'add'
            self._addDebugInterestHistory(
                action, description, handle, contextId, parentId, zoneIdList)
        if parentId == 0:
            DoInterestManager.notify.error(
                'trying to set interest to invalid parent: %s' % parentId)
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_ADD_INTEREST)
        datagram.addUint16(handle)
        datagram.addUint32(contextId)
        datagram.addUint32(parentId)
        if isinstance(zoneIdList, types.ListType):
            vzl = list(zoneIdList)
            vzl.sort()
            uniqueElements(vzl)
            for zone in vzl:
                datagram.addUint32(zone)
        else:
           datagram.addUint32(zoneIdList)
        self.send(datagram)

    def _sendRemoveInterest(self, handle, contextId):
        """
        handle is a client-side created number that refers to
                a set of interests.  The same handle number doesn't
                necessarily have any relationship to the same handle
                on another client.
        """
        assert DoInterestManager.notify.debugCall()
        assert handle in DoInterestManager._interests
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_REMOVE_INTEREST)
        datagram.addUint16(handle)
        if contextId != 0:
            datagram.addUint32(contextId)
        self.send(datagram)
        if __debug__:
            state = DoInterestManager._interests[handle]
            self._addDebugInterestHistory(
                "remove", state.desc, handle, contextId,
                state.parentId, state.zoneIdList)

    def _sendRemoveAIInterest(self, handle):
        """
        handle is a bare int, NOT an InterestHandle.  Use this to
        close an AI opened interest.
        """
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_REMOVE_INTEREST)
        datagram.addUint16((1<<15) + handle)
        self.send(datagram)

    def cleanupWaitAllInterestsComplete(self):
        if self._completeDelayedCallback is not None:
            self._completeDelayedCallback.destroy()
            self._completeDelayedCallback = None

    def queueAllInterestsCompleteEvent(self, frames=5):
        # wait for N frames, if no new interests, send out all-done event
        # calling this is OK even if there are no pending interest completes
        def checkMoreInterests():
            # if there are new interests, cancel this delayed callback, another
            # will automatically be scheduled when all interests complete
            # print 'checkMoreInterests(',self._completeEventCount.num,'):',globalClock.getFrameCount()
            return self._completeEventCount.num > 0
        def sendEvent():
            messenger.send(self.getAllInterestsCompleteEvent())
            for callback in self._allInterestsCompleteCallbacks:
                callback()
            self._allInterestsCompleteCallbacks = []
        self.cleanupWaitAllInterestsComplete()
        self._completeDelayedCallback = FrameDelayedCall(
            'waitForAllInterestCompletes',
            callback=sendEvent,
            frames=frames,
            cancelFunc=checkMoreInterests)
        checkMoreInterests = None
        sendEvent = None

    def handleInterestDoneMessage(self, di):
        """
        This handles the interest done messages and may dispatch an event
        """
        assert DoInterestManager.notify.debugCall()
        handle = di.getUint16()
        contextId = di.getUint32()
        if self.__verbose():
            print 'CR::INTEREST.interestDone(handle=%s)' % handle
        DoInterestManager.notify.debug(
            "handleInterestDoneMessage--> Received handle %s, context %s" % (
            handle, contextId))
        if DoInterestManager._interests.has_key(handle):
            eventsToSend = []
            # if the context matches, send out the event
            if contextId == DoInterestManager._interests[handle].context:
                DoInterestManager._interests[handle].context = NO_CONTEXT
                # the event handlers may call back into the interest manager. Send out
                # the events after we're once again in a stable state.
                #DoInterestManager._interests[handle].sendEvents()
                eventsToSend = list(DoInterestManager._interests[handle].getEvents())
                DoInterestManager._interests[handle].clearEvents()
            else:
                DoInterestManager.notify.debug(
                    "handleInterestDoneMessage--> handle: %s: Expecting context %s, got %s" % (
                    handle, DoInterestManager._interests[handle].context, contextId))
            if __debug__:
                state = DoInterestManager._interests[handle]
                self._addDebugInterestHistory(
                    "finished", state.desc, handle, contextId, state.parentId,
                    state.zoneIdList)
            self._considerRemoveInterest(handle)
            for event in eventsToSend:
                messenger.send(event)
        else:
            DoInterestManager.notify.warning(
                "handleInterestDoneMessage: handle not found: %s" % (handle))
        # if there are no more outstanding interest-completes, send out global all-done event
        if self._completeEventCount.num == 0:
            self.queueAllInterestsCompleteEvent()
        assert self.printInterestsIfDebug()

if __debug__:
    import unittest

    class AsyncTestCase(unittest.TestCase):
        def setCompleted(self):
            self._async_completed = True
        def isCompleted(self):
            return getattr(self, '_async_completed', False)

    class AsyncTestSuite(unittest.TestSuite):
        pass

    class AsyncTestLoader(unittest.TestLoader):
        suiteClass = AsyncTestSuite

    class AsyncTextTestRunner(unittest.TextTestRunner):
        def run(self, testCase):
            result = self._makeResult()
            startTime = time.time()
            test(result)
            stopTime = time.time()
            timeTaken = stopTime - startTime
            result.printErrors()
            self.stream.writeln(result.separator2)
            run = result.testsRun
            self.stream.writeln("Ran %d test%s in %.3fs" %
                                (run, run != 1 and "s" or "", timeTaken))
            self.stream.writeln()
            if not result.wasSuccessful():
                self.stream.write("FAILED (")
                failed, errored = map(len, (result.failures, result.errors))
                if failed:
                    self.stream.write("failures=%d" % failed)
                if errored:
                    if failed: self.stream.write(", ")
                    self.stream.write("errors=%d" % errored)
                self.stream.writeln(")")
            else:
                self.stream.writeln("OK")
            return result

    class TestInterestAddRemove(AsyncTestCase, DirectObject.DirectObject):
        def testInterestAdd(self):
            event = uniqueName('InterestAdd')
            self.acceptOnce(event, self.gotInterestAddResponse)
            self.handle = base.cr.addInterest(base.cr.GameGlobalsId, 100, 'TestInterest', event=event)
        def gotInterestAddResponse(self):
            event = uniqueName('InterestRemove')
            self.acceptOnce(event, self.gotInterestRemoveResponse)
            base.cr.removeInterest(self.handle, event=event)
        def gotInterestRemoveResponse(self):
            self.setCompleted()

    def runTests():
        suite = unittest.makeSuite(TestInterestAddRemove)
        unittest.AsyncTextTestRunner(verbosity=2).run(suite)
