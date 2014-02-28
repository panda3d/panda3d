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
    
    #def handleDatagram(self, di):
        #msgType = self.getMsgType()

    def handleMessageType(self, msgType, di):

        # These are the messages to a client that Astron specifies.

        if msgType == CLIENT_HELLO_RESP:
            print("Hello back!")
            messenger.send("CLIENT_HELLO_RESP", [])
        elif msgType == CLIENT_EJECT:
            error_code = di.get_uint16()
            reason = di.get_string()
            messenger.send("CLIENT_EJECT", [error_code, reason])
        elif msgType == CLIENT_ENTER_OBJECT_REQUIRED:
            self.handleEnterObjectRequired(di)
        elif msgType == CLIENT_ENTER_OBJECT_REQUIRED_OWNER:
            self.handleEnterObjectRequiredOwner(di)
        # FIXME: These are supposedly handled in cConnectionRepository
        #elif msgType == CLIENT_OBJECT_SET_FIELD:
        #    self.handleObjectSetField(di)
        #elif msgType == CLIENT_OBJECT_SET_FIELDS:
        #    self.handleObjectSetFields(di)
        elif msgType == CLIENT_OBJECT_LEAVING:
            self.handleObjectLeaving(di)
        elif msgType == CLIENT_OBJECT_LOCATION:
            self.handleObjectLocation(di)
        elif msgType == CLIENT_ADD_INTEREST:
            self.handleAddInterest(di)
        elif msgType == CLIENT_ADD_INTEREST_MULTIPLE:
            self.handleAddInterestMultiple(di)
        elif msgType == CLIENT_REMOVE_INTEREST:
            self.handleRemoveInterest(di)
        else:
            print("Passing unknown message type "+str(msgType))
            ClientRepositoryBase.handleMessageType(self, msgType, di)

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

    def handleObjectSetField(self, di):
        do_id = di.getArg(STUint32)
        field_id = di.getArg(STUint16)
        # FIXME: Is this really the best way?
        do = self.doId2do[do_id]
        # FIXME: Figure out field types and unpack them, or use
        # some code that auto-unpacks them.
        field_name = self.get_dc_file().get_field_by_index(field_id).get_name()
        # FIXME: You actually need to figure out the fields types and unpack them!
        self.sendUpdate(field_name, [fieldArguments])

    def handleObjectSetFields(self, di):
        # FIXME: Implement this!
        pass

    def handleObjectLeaving(self, di):
        # FIXME: Implement this!
        pass

    def handleObjectLocation(self, di):
        # FIXME: Implement this!
        pass

    def handleAddInterest(self, di):
        # FIXME: Implement this!
        pass

    def handleAddInterestMultiple(self, di):
        # FIXME: Implement this!
        pass

    def handleRemoveInterest(self, di):
        # FIXME: Implement this!
        pass

    #
    # Sending messages
    #

    # FIXME: The version string should default to a .prc variable.
    def sendHello(self, version_string):
        dg = PyDatagram()
        dg.add_uint16(CLIENT_HELLO)
        dg.add_uint32(self.get_dc_file().get_hash())
        dg.add_string(version_string)
        self.send(dg)
        # FIXME: Remove debug print
        print("Hello!")

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

