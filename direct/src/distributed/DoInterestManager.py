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

class InterestState:
    StateActive = 'Active'
    StatePendingDel = 'PendingDel'
    def __init__(self, desc, state, scope, event, parentId, zoneIdList):
        self.desc = desc
        self.state = state
        self.scope = scope
        # We must be ready to keep track of multiple events. If somebody
        # requested an interest to be removed and we get a second request
        # for removal of the same interest before we get a response for the
        # first interest removal, we now have two parts of the codebase
        # waiting for a response on the removal of a single interest.
        self.events = [event]
        self.parentId = parentId
        self.zoneIdList = zoneIdList
    def addEvent(self, event):
        self.events.append(event)
    def getEvents(self):
        return list(self.events)
    def clearEvents(self):
        self.events = []
    def sendEvents(self):
        for event in self.events:
            messenger.send(event)
        self.clearEvents()
    def isPendingDelete(self):
        return self.state == InterestState.StatePendingDel
    def __repr__(self):
        return 'InterestState(desc=%s, state=%s, scope=%s, event=%s, parentId=%s, zoneIdList=%s)' % (
            self.desc, self.state, self.scope, self.events, self.parentId, self.zoneIdList)

# scope value for interest changes that have no complete event
NO_SCOPE = 0

class DoInterestManager(DirectObject.DirectObject):
    """
    Top level Interest Manager
    """
    notify = directNotify.newCategory("DoInterestManager")
    try:
        tempbase = base
    except:
        tempbase = simbase
    InterestDebug = tempbase.config.GetBool('interest-debug', True)
    del tempbase

    # 'handle' is a number that represents a single interest set that the
    # client has requested; the interest set may be modified
    _HandleSerialNum = 0
    # high bit is reserved for server interests
    _HandleMask = 0x7FFF

    # 'scope' refers to a single request to change an interest set
    _ScopeIdSerialNum = 100
    _ScopeIdMask = 0x3FFFFFFF # avoid making Python create a long

    _interests = {}
    if __debug__:
        _debug_interestHistory = []
        _debug_maxDescriptionLen = 20

    def __init__(self):
        assert DoInterestManager.notify.debugCall()
        DirectObject.DirectObject.__init__(self)

    def addInterest(self, parentId, zoneIdList, description, event=None):
        """
        Look into a (set of) zone(s).
        """
        assert DoInterestManager.notify.debugCall()
        handle = self._getNextHandle()
        scopeId = self._getNextScopeId()
        DoInterestManager._interests[handle] = InterestState(
            description, InterestState.StateActive, scopeId, event, parentId, zoneIdList)
        if self.InterestDebug:
            print 'INTEREST DEBUG: addInterest(): handle=%s, parent=%s, zoneIds=%s, description=%s, event=%s' % (
                handle, parentId, zoneIdList, description, event)
        self._sendAddInterest(handle, scopeId, parentId, zoneIdList, description)
        assert self.printInterestsIfDebug()
        return handle

    def removeInterest(self, handle, event=None):
        """
        Stop looking in a (set of) zone(s)
        """
        assert DoInterestManager.notify.debugCall()
        existed = False
        if DoInterestManager._interests.has_key(handle):
            existed = True
            intState = DoInterestManager._interests[handle]
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
                    self.notify.warning('removeInterest: abandoning events: %s' %
                                        intState.events)
                    intState.clearEvents()
                scopeId = self._getNextScopeId()
                intState.state = InterestState.StatePendingDel
                intState.scope = scopeId
                if event is not None:
                    intState.addEvent(event)
                if self.InterestDebug:
                    print 'INTEREST DEBUG: removeInterest(): handle=%s, event=%s' % (
                        handle, event)
                self._sendRemoveInterest(handle, scopeId)
                if event is None:
                    self._considerRemoveInterest(handle)
        else:
            DoInterestManager.notify.warning(
                "removeInterest: handle not found: %s" % (handle))
        assert self.printInterestsIfDebug()
        return existed

    def alterInterest(self, handle, parentId, zoneIdList, description=None,
                      event=None):
        """
        Removes old interests and adds new interests.

        Note that when an interest is changed, only the most recent
        change's event will be triggered. Previous events are abandoned.
        If this is a problem, consider opening multiple interests.
        """
        assert DoInterestManager.notify.debugCall()
        exists = False
        if DoInterestManager._interests.has_key(handle):
            if description is not None:
                DoInterestManager._interests[handle].desc = description
            else:
                description = DoInterestManager._interests[handle].desc

            scopeId = self._getNextScopeId()
            DoInterestManager._interests[handle].scope = scopeId
            DoInterestManager._interests[handle].addEvent(event)

            if self.InterestDebug:
                print 'INTEREST DEBUG: alterInterest(): handle=%s, parent=%s, zoneIds=%s, description=%s, event=%s' % (
                    handle, parentId, zoneIdList, description, event)
            self._sendAddInterest(handle, scopeId, parentId, zoneIdList, description, action='modify')
            exists = True
            assert self.printInterestsIfDebug()
        else:
            DoInterestManager.notify.warning(
                "alterInterest: handle not found: %s" % (handle))
        return exists

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
    def _getNextScopeId(self):
        scopeId = DoInterestManager._ScopeIdSerialNum
        while True:
            scopeId = (scopeId + 1) & DoInterestManager._ScopeIdMask
            # skip over the 'no scope' id
            if scopeId != NO_SCOPE:
                break
        DoInterestManager._ScopeIdSerialNum = scopeId
        return DoInterestManager._ScopeIdSerialNum

    def _considerRemoveInterest(self, handle):
        """
        Consider whether we should cull the interest set.
        """
        assert DoInterestManager.notify.debugCall()
        if DoInterestManager._interests.has_key(handle):
            if DoInterestManager._interests[handle].isPendingDelete():
                # make sure there is no pending event for this interest
                if DoInterestManager._interests[handle].scope == NO_SCOPE:
                    del DoInterestManager._interests[handle]

    if __debug__:
        def printInterestsIfDebug(self):
            if DoInterestManager.notify.getDebug():
                self.printInterests()
            return 1 # for assert

        def _addDebugInterestHistory(self, action, description, handle,
                                     scopeId, parentId, zoneIdList):
            if description is None:
                description = ''
            DoInterestManager._debug_interestHistory.append(
                (action, description, handle, scopeId, parentId, zoneIdList))
            DoInterestManager._debug_maxDescriptionLen = max(
                DoInterestManager._debug_maxDescriptionLen, len(description))

        def printInterests(self):
            print "***************** Interest History *************"
            format = '%9s %' + str(DoInterestManager._debug_maxDescriptionLen) + 's %6s %6s %9s %s'
            print format % (
                "Action", "Description", "Handle", "Scope", "ParentId",
                "ZoneIdList")
            for i in DoInterestManager._debug_interestHistory:
                print format % tuple(i)
            print "Note: interests with a Scope of 0 do not get" \
                " done/finished notices."
            print "******************* Interest Sets **************"
            format = '%6s %' + str(DoInterestManager._debug_maxDescriptionLen) + 's %10s %5s %9s %9s %10s'
            print format % (
                "Handle", "Description", "State", "Scope", 
                "ParentId", "ZoneIdList", "Event")
            for id, state in DoInterestManager._interests.items():
                if len(state.events) == 0:
                    event = ''
                elif len(state.events) == 1:
                    event = state.events[0]
                else:
                    event = state.events
                print format % (id, state.desc, state.state, state.scope,
                                state.parentId, state.zoneIdList, event)
            print "************************************************"

    def _sendAddInterest(self, handle, scopeId, parentId, zoneIdList, description,
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
                action, description, handle, scopeId, parentId, zoneIdList)
        if parentId == 0:
            DoInterestManager.notify.error(
                'trying to set interest to invalid parent: %s' % parentId)
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_ADD_INTEREST)
        datagram.addUint16(handle)
        datagram.addUint32(scopeId)
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

    def _sendRemoveInterest(self, handle, scopeId):
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
        if scopeId != 0:
            datagram.addUint32(scopeId)
        self.send(datagram)
        if __debug__:
            state = DoInterestManager._interests[handle]
            self._addDebugInterestHistory(
                "remove", state.desc, handle, scopeId,
                state.parentId, state.zoneIdList)

    def handleInterestDoneMessage(self, di):
        """
        This handles the interest done messages and may dispatch an event
        """
        assert DoInterestManager.notify.debugCall()
        handle = di.getUint16()
        scopeId = di.getUint32()
        if self.InterestDebug:
            print 'INTEREST DEBUG: interestDone(): handle=%s' % handle
        DoInterestManager.notify.debug(
            "handleInterestDoneMessage--> Received handle %s, scope %s" % (
            handle, scopeId))
        eventsToSend = []
        # if the scope matches, send out the event
        if scopeId == DoInterestManager._interests[handle].scope:
            DoInterestManager._interests[handle].scope = NO_SCOPE
            # the event handlers may call back into the interest manager. Send out
            # the events after we're once again in a stable state.
            #DoInterestManager._interests[handle].sendEvents()
            eventsToSend = list(DoInterestManager._interests[handle].getEvents())
        else:
            DoInterestManager.notify.warning(
                "handleInterestDoneMessage--> handle: %s: Expecting scope %s, got %s" % (
                handle, DoInterestManager._interests[handle].scope, scopeId))
        if __debug__:
            state = DoInterestManager._interests[handle]
            self._addDebugInterestHistory(
                "finished", state.desc, handle, scopeId, state.parentId,
                state.zoneIdList)
        self._considerRemoveInterest(handle)
        for event in eventsToSend:
            messenger.send(event)
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
