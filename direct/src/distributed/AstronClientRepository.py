"""AstronClientRepository module: contains the AstronClientRepository class"""

from direct.directnotify import DirectNotifyGlobal
from ClientRepositoryBase import ClientRepositoryBase
from MsgTypesAstron import *
from direct.distributed.PyDatagram import PyDatagram
from pandac.PandaModules import STUint16, STUint32

class AstronClientRepository(ClientRepositoryBase):
    """
    The Astron implementation of a clients repository for
    communication with an Astron ClientAgent.
    
    This repo will emit events for:
    * CLIENT_HELLO_RESP
    * CLIENT_EJECT ( int error_code, string reason )
    * CLIENT_DONE_INTEREST_RESP" ( int context, int interest_id )
    * CLIENT_OBJECT_LEAVING ( int do_id )
    * CLIENT_ADD_INTEREST ( context, interest_id, parent_id, zone_id )
    * CLIENT_REMOVE_INTEREST ( context, interest_id )
    * LOST_CONNECTION ()
    """

    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepository")

    # This is required by DoCollectionManager, even though it's not
    # used by this implementation.
    GameGlobalsId = 0
        
    def __init__(self, *args, **kwargs):
        ClientRepositoryBase.__init__(self, *args, **kwargs)
        base.finalExitCallbacks.append(self.shutdown)
        # FIXME: Partial code from ClientRepository.py may be required here.

    #
    # Message Handling
    #
    
    def handleDatagram(self, di):
        msgType = self.getMsgType()
        self.handleMessageType(msgType, di)

    def handleMessageType(self, msgType, di):

        # These are the messages to a client that Astron specifies.

        if msgType == CLIENT_HELLO_RESP:
            messenger.send("CLIENT_HELLO_RESP", [])
        elif msgType == CLIENT_EJECT:
            error_code = di.get_uint16()
            reason = di.get_string()
            messenger.send("CLIENT_EJECT", [error_code, reason])
        elif msgType == CLIENT_ENTER_OBJECT_REQUIRED:
            self.handleEnterObjectRequired(di)
        elif msgType == CLIENT_ENTER_OBJECT_REQUIRED_OWNER:
            self.handleEnterObjectRequiredOwner(di)
        elif msgType == CLIENT_OBJECT_SET_FIELD:
            self.handleUpdateField(di)
        # FIXME: This is supposedly handled in cConnectionRepository; see CLIENT_OBJECT_SET_FIELD
        # elif msgType == CLIENT_OBJECT_SET_FIELDS:
        #     self.handleObjectSetFields(di)
        elif msgType == CLIENT_OBJECT_LEAVING:
            # FIXME: Doesn't this need to be handled in the repository?
            do_id = di.get_uint32()
            messenger.send("CLIENT_OBJECT_LEAVING", [do_id])
        elif msgType == CLIENT_OBJECT_LOCATION:
            # FIXME: What does this even do???
            self.handleObjectLocation(di)
        elif msgType == CLIENT_ADD_INTEREST:
            # FIXME: Doesn't this need to be handled in the repository?
            context = di.get_uint32()
            interest_id = di.get_uint16()
            parent_id = di.get_uint32()
            zone_id = di.get_uint32()
            messenger.send("CLIENT_ADD_INTEREST", [context, interest_id, parent_id, zone_id])
        elif msgType == CLIENT_ADD_INTEREST_MULTIPLE:
            # FIXME: Informative event here
            pass
        elif msgType == CLIENT_REMOVE_INTEREST:
            # FIXME: Doesn't this need to be handled in the repository?
            context = di.get_uint32()
            interest_id = di.get_uint16()
            messenger.send("CLIENT_REMOVE_INTEREST", [context, interest_id])
        elif msgType == CLIENT_DONE_INTEREST_RESP:
            context = di.get_uint32()
            interest_id = di.get_uint16 ()
            messenger.send("CLIENT_DONE_INTEREST_RESP", [context, interest_id])
        else:
            self.notify.error("Got unknown message type %d!" % (msgType,))

        self.considerHeartbeat()

    def handleEnterObjectRequired(self, di):
        do_id = di.getArg(STUint32)
        parent_id = di.getArg(STUint32)
        zone_id = di.getArg(STUint32)
        dclass_id = di.getArg(STUint16)
        dclass = self.dclassesByNumber[dclass_id]
        self.generateWithRequiredFields(dclass, do_id, di, parent_id, zone_id)

    def handleEnterObjectRequiredOwner(self, di):
        avatar_doId = di.getArg(STUint32)
        parentId = di.getArg(STUint32)
        zoneId = di.getArg(STUint32)
        dclass_id = di.getArg(STUint16)
        dclass = self.dclassesByNumber[dclass_id]
        self.generateWithRequiredFieldsOwner(dclass, avatar_doId, di)
    
    def generateWithRequiredFieldsOwner(self, dclass, doId, di):
        if doId in self.doId2ownerView:
            # ...it is in our dictionary.
            # Just update it.
            self.notify.error('duplicate owner generate for %s (%s)' % (
                doId, dclass.getName()))
            distObj = self.doId2ownerView[doId]
            assert distObj.dclass == dclass
            distObj.generate()
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
        elif self.cacheOwner.contains(doId):
            # ...it is in the cache.
            # Pull it out of the cache:
            distObj = self.cacheOwner.retrieve(doId)
            assert distObj.dclass == dclass
            # put it in the dictionary:
            self.doId2ownerView[doId] = distObj
            # and update it.
            distObj.generate()
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
        else:
            # ...it is not in the dictionary or the cache.
            # Construct a new one
            classDef = dclass.getOwnerClassDef()
            if classDef == None:
                self.notify.error("Could not create an undefined %s object. Have you created an owner view?" % (dclass.getName()))
            distObj = classDef(self)
            distObj.dclass = dclass
            # Assign it an Id
            distObj.doId = doId
            # Put the new do in the dictionary
            self.doId2ownerView[doId] = distObj
            # Update the required fields
            distObj.generateInit()  # Only called when constructed
            distObj.generate()
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
        return distObj

    def handleObjectLocation(self, di):
        # FIXME: Implement this!
        pass

    #
    # Sending messages
    #

    def sendUpdate(self, distObj, fieldName, args):
        """ Sends a normal update for a single field. """
        dg = distObj.dclass.clientFormatUpdate(
            fieldName, distObj.doId, args)
        self.send(dg)

    # FIXME: The version string should default to a .prc variable.
    def sendHello(self, version_string):
        dg = PyDatagram()
        dg.add_uint16(CLIENT_HELLO)
        dg.add_uint32(self.get_dc_file().get_hash())
        dg.add_string(version_string)
        self.send(dg)

    def sendHeartbeat(self):
        datagram = PyDatagram()
        datagram.addUint16(CLIENT_HEARTBEAT)
        self.send(datagram)

    def sendAddInterest(self, context, interest_id, parent_id, zone_id):
        dg = PyDatagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(context)
        dg.add_uint16(interest_id)
        dg.add_uint32(parent_id)
        dg.add_uint32(zone_id)
        self.send(dg)

    def sendAddInterestMultiple(self, context, interest_id, parent_id, zone_ids):
        dg = PyDatagram()
        dg.add_uint16(CLIENT_ADD_INTEREST_MULTIPLE)
        dg.add_uint32(context)
        dg.add_uint16(interest_id)
        dg.add_uint32(parent_id)
        dg.add_uint16(len(zone_ids))
        for zone_id in zone_ids:
            dg.add_uint32(zone_id)
        self.send(dg)

    def sendRemoveInterest(self, context, interest_id):
        dg = PyDatagram()
        dg.add_uint16(CLIENT_REMOVE_INTEREST)
        dg.add_uint32(context)
        dg.add_uint16(interest_id)
        self.send(dg)

    #
    # Other stuff
    #
    
    def lostConnection(self):
        messenger.send("LOST_CONNECTION")

