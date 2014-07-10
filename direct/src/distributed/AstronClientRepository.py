"""AstronClientRepository module: contains the AstronClientRepository class"""

from direct.directnotify import DirectNotifyGlobal
from ClientRepositoryBase import ClientRepositoryBase
from MsgTypes import *
from direct.distributed.PyDatagram import PyDatagram
from pandac.PandaModules import STUint16, STUint32

class AstronClientRepository(ClientRepositoryBase):
    """
    The Astron implementation of a clients repository for
    communication with an Astron ClientAgent.
    
    This repo will emit events for:
    * CLIENT_HELLO_RESP
    * CLIENT_EJECT ( error_code, reason )
    * CLIENT_OBJECT_LEAVING ( do_id )
    * CLIENT_ADD_INTEREST ( context, interest_id, parent_id, zone_id )
    * CLIENT_ADD_INTEREST_MULTIPLE ( icontext, interest_id, parent_id, [zone_ids] )
    * CLIENT_REMOVE_INTEREST ( context, interest_id )
    * CLIENT_DONE_INTEREST_RESP ( context, interest_id )
    * LOST_CONNECTION ()
    """

    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepository")

    # This is required by DoCollectionManager, even though it's not
    # used by this implementation.
    GameGlobalsId = 0
    
    def __init__(self, *args, **kwargs):
        ClientRepositoryBase.__init__(self, *args, **kwargs)
        base.finalExitCallbacks.append(self.shutdown)
        self.message_handlers = {CLIENT_HELLO_RESP: self.handleHelloResp,
                                 CLIENT_EJECT: self.handleEject,
                                 CLIENT_ENTER_OBJECT_REQUIRED: self.handleEnterObjectRequired,
                                 CLIENT_ENTER_OBJECT_REQUIRED_OWNER: self.handleEnterObjectRequiredOwner,
                                 CLIENT_OBJECT_SET_FIELD: self.handleUpdateField,
                                 CLIENT_OBJECT_SET_FIELDS: self.handleUpdateFields,
                                 CLIENT_OBJECT_LEAVING: self.handleObjectLeaving,
                                 CLIENT_OBJECT_LOCATION: self.handleObjectLocation,
                                 CLIENT_ADD_INTEREST: self.handleAddInterest,
                                 CLIENT_ADD_INTEREST_MULTIPLE: self.handleAddInterestMultiple,
                                 CLIENT_REMOVE_INTEREST: self.handleRemoveInterest,
                                 CLIENT_DONE_INTEREST_RESP: self.handleInterestDoneMessage,
                                 }
       
    #
    # Message Handling
    #
    
    def handleDatagram(self, di):
        msgType = self.getMsgType()
    #    self.handleMessageType(msgType, di)
    #
    #def handleMessageType(self, msgType, di):
        if msgType in self.message_handlers:
            self.message_handlers[msgType](di)
        else:
            self.notify.error("Got unknown message type %d!" % (msgType,))

        self.considerHeartbeat()

    def handleHelloResp(self, di):
        messenger.send("CLIENT_HELLO_RESP", []) 

    def handleEject(self, di):
        error_code = di.get_uint16()
        reason = di.get_string()
        messenger.send("CLIENT_EJECT", [error_code, reason])
    
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

    def handleUpdateFields(self, di):
        # Can't test this without the server actually sending it.
        self.notify.error("CLIENT_OBJECT_SET_FIELDS not implemented!")
        # # Here's some tentative code and notes:
        # do_id = di.getUint32()
        # field_count = di.getUint16()
        # for i in range(0, field_count):
        #     field_id = di.getUint16()
        #     field = self.get_dc_file().get_field_by_index(field_id)
        #     # print(type(field))
        #     # print(field)
        #     # FIXME: Get field type, unpack value, create and send message.
        #     # value = di.get?()
        #     # Assemble new message

    def handleObjectLeaving(self, di):
        do_id = di.get_uint32()
        dist_obj = self.doId2do.get(do_id)
        dist_obj.delete()
        self.deleteObject(do_id)
        messenger.send("CLIENT_OBJECT_LEAVING", [do_id])

    def handleAddInterest(self, di):
        context = di.get_uint32()
        interest_id = di.get_uint16()
        parent_id = di.get_uint32()
        zone_id = di.get_uint32()
        messenger.send("CLIENT_ADD_INTEREST", [context, interest_id, parent_id, zone_id])

    def handleAddInterestMultiple(self, di):
        context = di.get_uint32()
        interest_id = di.get_uint16()
        parent_id = di.get_uint32()
        zone_ids = [di.get_uint32() for i in range(0, di.get_uint16())]
        messenger.send("CLIENT_ADD_INTEREST_MULTIPLE", [context, interest_id, parent_id, zone_ids])

    def handleRemoveInterest(self, di):
        context = di.get_uint32()
        interest_id = di.get_uint16()
        messenger.send("CLIENT_REMOVE_INTEREST", [context, interest_id])
    
    def deleteObject(self, doId):
        """
        implementation copied from ClientRepository.py
        
        Removes the object from the client's view of the world.  This
        should normally not be called directly except in the case of
        error recovery, since the server will normally be responsible
        for deleting and disabling objects as they go out of scope.

        After this is called, future updates by server on this object
        will be ignored (with a warning message).  The object will
        become valid again the next time the server sends a generate
        message for this doId.

        This is not a distributed message and does not delete the
        object on the server or on any other client.
        """
        if doId in self.doId2do:
            # If it is in the dictionary, remove it.
            obj = self.doId2do[doId]
            # Remove it from the dictionary
            del self.doId2do[doId]
            # Disable, announce, and delete the object itself...
            # unless delayDelete is on...
            obj.deleteOrDelay()
            if self.isLocalId(doId):
                self.freeDoId(doId)
        elif self.cache.contains(doId):
            # If it is in the cache, remove it.
            self.cache.delete(doId)
            if self.isLocalId(doId):
                self.freeDoId(doId)
        else:
            # Otherwise, ignore it
            self.notify.warning(
                "Asked to delete non-existent DistObj " + str(doId))

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

    def disconnect(self):
        """
        This implicitly deletes all objects from the repository.
        """
        for do_id in self.doId2do.keys():
            self.deleteObject(do_id)
        ClientRepositoryBase.disconnect(self)
