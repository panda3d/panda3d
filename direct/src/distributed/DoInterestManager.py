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
#from PyDatagramIterator import PyDatagramIterator
from direct.directnotify.DirectNotifyGlobal import directNotify

# indices into interest table entries
DESC   = 0
STATE  = 1
EVENTS = 2

STATE_ACTIVE = 'Active'
STATE_PENDING_DEL = 'PendingDel'

# scope value for interest changes that have no complete event
NO_SCOPE = 0

class DoInterestManager(DirectObject.DirectObject):
    """
    Top level Interest Manager
    """
    notify = directNotify.newCategory("DoInterestManager")

    # 'interestId' is a handle that represents a single interest set that the client has requested;
    #              the interest set may be modified
    _InterestIdSerialNum = 0
    _InterestIdMask = 0xFFFF

    # 'scope' refers to a single request to change an interest set
    _ScopeIdSerialNum = 100
    _ScopeIdMask = 0x3FFFFFFF # avoid making Python create a long

    _interests = {}
    if __debug__:
        _debug_currentInterests = []

    def __init__(self):
        assert DoInterestManager.notify.debugCall()
        DirectObject.DirectObject.__init__(self)

    def addInterest(self, parentId, zoneIdList, description, event=None):
        """
        Look into a (set of) zone(s).
        """
        assert DoInterestManager.notify.debugCall()
        interestId = self._getNextInterestId()
        DoInterestManager._interests[interestId] = [description, STATE_ACTIVE, {}]
        if event is not None:
            scopeId = self._getNextScopeId()
            self._setScopeCompleteEvent(interestId, scopeId, event)
        else:
            scopeId = NO_SCOPE
            
        self._sendAddInterest(interestId, scopeId, parentId, zoneIdList)
        assert self.printInterestsIfDebug()
        return interestId

    def removeInterest(self, interestId, event=None):
        """
        Stop looking in a (set of) zone(s)
        """
        assert DoInterestManager.notify.debugCall()
        existed = 0
        if DoInterestManager._interests.has_key(interestId):
            if event is not None:
                scopeId = self._getNextScopeId()
                self._setScopeCompleteEvent(interestId, scopeId, event)
                DoInterestManager._interests[interestId][STATE] = STATE_PENDING_DEL
            else:
                scopeId = NO_SCOPE
                # I don't think this is necessary, it'll be cleaned up in handleInterestDoneMessage
                #del DoInterestManager._interests[interestId]
            self._sendRemoveInterest(interestId, scopeId)
            existed = 1
        else:
            DoInterestManager.notify.warning("removeInterest: interestId not found: %s" % (interestId))
        assert self.printInterestsIfDebug()
        return existed

    def alterInterest(self, interestId, parentId, zoneIdList, description=None, event=None):
        """
        Removes old interests and adds new interests.
        """
        assert DoInterestManager.notify.debugCall()
        exists = 0
        if DoInterestManager._interests.has_key(interestId):
            if description is not None:
                DoInterestManager._interests[interestId][DESC] = description

            if event is not None:
                scopeId = self._getNextScopeId()
                self._setScopeCompleteEvent(interestId, scopeId, event)
            else:
                scopeId = NO_SCOPE
            
            self._sendAddInterest(interestId, scopeId, parentId, zoneIdList)
            exists = 1
            assert self.printInterestsIfDebug()
        else:
            DoInterestManager.notify.warning("alterInterest: interestId not found: %s" % (interestId))
        return exists

    def _getNextInterestId(self):
        DoInterestManager._InterestIdSerialNum = (DoInterestManager._InterestIdSerialNum + 1) & DoInterestManager._InterestIdMask
        return DoInterestManager._InterestIdSerialNum
    def _getNextScopeId(self):
        DoInterestManager._ScopeIdSerialNum = (DoInterestManager._ScopeIdSerialNum + 1) & DoInterestManager._ScopeIdMask
        # skip over the 'no scope' id
        while DoInterestManager._ScopeIdSerialNum == NO_SCOPE:
            DoInterestManager._ScopeIdSerialNum = (DoInterestManager._ScopeIdSerialNum + 1) & DoInterestManager._ScopeIdMask
        return DoInterestManager._ScopeIdSerialNum

    def _getScopeCompleteEventTable(self, interestId):
        return DoInterestManager._interests[interestId][EVENTS]

    def _setScopeCompleteEvent(self, interestId, scopeId, event):
        assert DoInterestManager.notify.debugCall()
        self._getScopeCompleteEventTable(interestId)[scopeId] = event

    def _getScopeCompleteEvent(self, interestId, scopeId):
        """
        returns an event for an interest.
        """
        return self._getScopeCompleteEventTable(interestId)[scopeId]

    def _removeScopeCompleteEvent(self, interestId, scopeId):
        """
        removes an event for an interest.
        """
        assert DoInterestManager.notify.debugCall()
        del self._getScopeCompleteEventTable(interestId)[scopeId]

    def _considerRemoveInterest(self, interestId):
        """
        Consider whether we should cull the interest set.
        """
        assert DoInterestManager.notify.debugCall()
        if DoInterestManager._interests.has_key(interestId):
            if DoInterestManager._interests[interestId][STATE] == STATE_PENDING_DEL:
                # make sure there are no pending events for this interest
                if len(self._getScopeCompleteEventTable(interestId)) == 0:
                    del DoInterestManager._interests[interestId]

    if __debug__:
        def printInterestsIfDebug(self):
            if DoInterestManager.notify.getDebug():
                self.printInterests()
            return 1 # for assert()

        def printInterests(self):
            print "*********************** Interest Sets **************"
            print "interestId: [Description, State, {scope:pendingCompleteEvent, ...}]"
            for id, data in DoInterestManager._interests.items():
                print '%s: %s' % (id, data)
            print "************************** History *****************"
            print "(InterestId, ScopeId, ParentId, ZoneIdList)"
            for i in DoInterestManager._debug_currentInterests:
                print i
            print "****************************************************"

    def _sendAddInterest(self, interestId, scopeId, parentId, zoneIdList):
        """
        Part of the new otp-server code.

        interestId is a client-side created number that refers to
                a set of interests.  The same interestId number doesn't
                necessarily have any relationship to the same interestId
                on another client.
        """
        assert DoInterestManager.notify.debugCall()
        if __debug__:
            if isinstance(zoneIdList, types.ListType):
                zoneIdList.sort()
            DoInterestManager._debug_currentInterests.append(
                (interestId, scopeId, parentId, zoneIdList))
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_ADD_INTEREST)
        datagram.addUint16(interestId)
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

    def _sendRemoveInterest(self, interestId, scopeId):
        """
        interestId is a client-side created number that refers to
                a set of interests.  The same interestId number doesn't
                necessarily have any relationship to the same interestId
                on another client.
        """
        assert DoInterestManager.notify.debugCall()
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_REMOVE_INTEREST)
        datagram.addUint16(interestId)
        if scopeId != 0:
            datagram.addUint32(scopeId)            
        self.send(datagram)

    def handleInterestDoneMessage(self, di):
        """
        This handles the interest done messages and may dispatch an event
        """
        assert DoInterestManager.notify.debugCall()
        interestId = di.getUint16()
        scopeId = di.getUint32()
        DoInterestManager.notify.debug(
            "handleInterestDoneMessage--> Received ID:%s Scope:%s"%(interestId, scopeId))
        # if there's a scope, send out its event
        if scopeId != NO_SCOPE:
            event = self._getScopeCompleteEvent(interestId, scopeId)
            self._removeScopeCompleteEvent(interestId, scopeId)
            DoInterestManager.notify.debug(
                "handleInterestDoneMessage--> Send Event : %s"%(event))
            messenger.send(event)
        self._considerRemoveInterest(interestId)

        assert self.printInterestsIfDebug()

