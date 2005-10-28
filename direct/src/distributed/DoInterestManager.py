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

class DoInterestManager(DirectObject.DirectObject):
    """
    Top level Interest Manager
    """
    #if __debug__:      is this production ready?
    from direct.directnotify.DirectNotifyGlobal import directNotify
    notify = directNotify.newCategory("DoInterestManager")

    _interestIdAssign = 1
    _interestIdScopes = 100
    _interests = {}
    if __debug__:
        _debug_currentInterests = []

    def __init__(self):
        assert DoInterestManager.notify.debugCall()
        DirectObject.DirectObject.__init__(self)

    def addInterest(self, parentId, zoneIdList, description, event=None):
        """
        Look into a zone.
        """
        assert DoInterestManager.notify.debugCall()
        DoInterestManager._interestIdAssign += 1
        contextId = DoInterestManager._interestIdAssign
        scopeId = 0
        if event is not None:
            DoInterestManager._interestIdScopes += 1
            scopeId = DoInterestManager._interestIdScopes
            
        DoInterestManager._interests[contextId] = [description, scopeId, event, "Active"]
        self._sendAddInterest(contextId, scopeId, parentId, zoneIdList)
        assert self.printInterestsIfDebug()
        return contextId

    def removeInterest(self,  contextId, event=None):
        """
        Stop looking in a zone
        """
        assert DoInterestManager.notify.debugCall()
        answer = 0
        if  DoInterestManager._interests.has_key(contextId):
            if event is not None:
                DoInterestManager._interests[contextId][3] = "PendingDel"
                DoInterestManager._interests[contextId][2] = event
                DoInterestManager._interestIdScopes  += 1
                DoInterestManager._interests[contextId][1] = DoInterestManager._interestIdScopes
                self._sendRemoveInterest(contextId,DoInterestManager._interestIdScopes)
            else:
                DoInterestManager._interests[contextId][2] = None
                DoInterestManager._interests[contextId][1] = 0
                self._sendRemoveInterest(contextId,0)
                del DoInterestManager._interests[contextId]
            answer = 1
        else:
            DoInterestManager.notify.warning("removeInterest: contextId not found: %s" % (contextId))
        assert self.printInterestsIfDebug()
        return answer

    def alterInterest(self, contextId, parentId, zoneIdList, description=None, event=None):
        """
        Removes old interests and adds new interests.
        """
        assert DoInterestManager.notify.debugCall()
        answer = 0
        if  DoInterestManager._interests.has_key(contextId):
            if description is not None:
                DoInterestManager._interests[contextId][0] = description

            if event is not None:
                DoInterestManager._interestIdScopes  += 1
                DoInterestManager._interests[contextId][1] = DoInterestManager._interestIdScopes
            else:
                DoInterestManager._interests[contextId][1] = 0
            
            DoInterestManager._interests[contextId][2] = event
            self._sendAddInterest(contextId,DoInterestManager._interests[contextId][1], parentId, zoneIdList)
            answer = 1
            assert self.printInterestsIfDebug()
        else:
            DoInterestManager.notify.warning("alterInterest: contextId not found: %s" % (contextId))
        return answer


    def getInterestScopeId(self, contextId):
        """
        Part of the new otp-server code.
             Return a ScopeId Id for an Interest
        """
        assert DoInterestManager.notify.debugCall()
        answer = 0
        if  DoInterestManager._interests.has_key(contextId):
            answer = DoInterestManager._interests[contextId][1]
        else:
            DoInterestManager.notify.warning("GetInterestScopeID: contextId not found: %s" % (contextId))
        return answer


    def getInterestScopeEvent(self, contextId):
        """
        returns an event for an interest.
        """
        assert DoInterestManager.notify.debugCall()
        answer = None
        if  DoInterestManager._interests.has_key(contextId):
            answer = DoInterestManager._interests[contextId][2]
        else:
            DoInterestManager.notify.warning("GetInterestScopeEvent: contextId not found: %s" % (contextId))
        return answer

    def _ponderRemoveFlaggedInterest(self, handle):
        """
        Consider whether we should cull the interest set.
        """
        assert DoInterestManager.notify.debugCall()
        if  DoInterestManager._interests.has_key(handle):
                if DoInterestManager._interests[handle][3] == "PendingDel":
                    del DoInterestManager._interests[handle]

    if __debug__:
        def printInterestsIfDebug(self):
            if DoInterestManager.notify.getDebug():
                self.printInterests()
            return 1 # for assert()

        def printInterests(self):
            print "*********************** Interest Sets **************"
            print "(Description, Scope, Event, Mode)"
            for i in DoInterestManager._interests.values():
                print i
            print "****************************************************"
            print "(ContextId, ScopeId, ParentId, ZoneIdList)"
            for i in DoInterestManager._debug_currentInterests:
                print i
            print "****************************************************"

    def _sendAddInterest(self, contextId, scopeId, parentId, zoneIdList):
        """
        Part of the new otp-server code.

        contextId is a client-side created number that refers to
                a set of interests.  The same contextId number doesn't
                necessarily have any relationship to the same contextId
                on another client.
        """
        assert DoInterestManager.notify.debugCall()
        if __debug__:
            DoInterestManager._debug_currentInterests.append(
                (contextId, scopeId, parentId, zoneIdList))
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_ADD_INTEREST)
        datagram.addUint16(contextId)
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

    def _sendRemoveInterest(self, contextId, scopeId):
        """
        contextId is a client-side created number that refers to
                a set of interests.  The same contextId number doesn't
                necessarily have any relationship to the same contextId
                on another client.
        """
        assert DoInterestManager.notify.debugCall()
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_REMOVE_INTEREST)
        datagram.addUint16(contextId)
        if scopeId != 0:
            datagram.addUint32(scopeId)            
        self.send(datagram)

    def handleInterestDoneMessage(self, di):
        """
        This handles the interest done messages and may dispatch a
        action based on the ID, Context
        """
        assert DoInterestManager.notify.debugCall()
        interestId = di.getUint16()
        scope = di.getUint32()
        expect_scope = self.getInterestScopeId(interestId)
        DoInterestManager.notify.debug(
            "handleInterestDoneMessage--> Received ID:%s Scope:%s"%(interestId, scope))
        if expect_scope == scope:
            DoInterestManager.notify.debug(
                "handleInterestDoneMessage--> Scope Match:%s Scope:%s"
                %(interestId, scope))
            event = self.getInterestScopeEvent(interestId)
            if event is not None:
                DoInterestManager.notify.debug(
                    "handleInterestDoneMessage--> Send Event : %s"%(event))
                messenger.send(event)
            else:
                DoInterestManager.notify.debug("handleInterestDoneMessage--> No Event ")
            self._ponderRemoveFlaggedInterest(interestId)
        else:
            DoInterestManager.notify.debug(
                "handleInterestDoneMessage--> Scope MisMatch :%s :%s"
                %(expect_scope,scope))

        assert self.printInterestsIfDebug()

