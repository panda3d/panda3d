"""ClientRepository module: contains the ClientRepository class"""

from ClientRepositoryBase import *

class ClientRepository(ClientRepositoryBase):
    """
    This is the open-source ClientRepository as provided by CMU.  It
    communicates with the ServerRepository in this same directory.

    If you are looking for the VR Studio's implementation of the
    client repository, look to OTPClientRepository (elsewhere).
    """
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepository")

    def __init__(self, dcFileNames = None):
        ClientRepositoryBase.__init__(self, dcFileNames = dcFileNames)

        # The DOID allocator.  The CMU LAN server may choose to
        # send us a block of DOIDs.  If it chooses to do so, then we
        # may create objects, using those DOIDs.
        self.DOIDbase = 0
        self.DOIDnext = 0
        self.DOIDlast = 0

    def handleSetDOIDrange(self, di):
        self.DOIDbase = di.getUint32()
        self.DOIDlast = self.DOIDbase + di.getUint32()
        self.DOIDnext = self.DOIDbase

    def handleRequestGenerates(self, di):
        # When new clients join the zone of an object, they need to hear
        # about it, so we send out all of our information about objects in
        # that particular zone.

        assert self.DOIDnext < self.DOIDlast
        zone = di.getUint32()
        for obj in self.doId2do.values():
            if obj.zone == zone:
                id = obj.doId
                if (self.isLocalId(id)):
                    self.send(obj.dclass.clientFormatGenerate(obj, id, zone, []))

    def createWithRequired(self, className, zoneId = 0, optionalFields=None):
        if self.DOIDnext >= self.DOIDlast:
            self.notify.error(
                "Cannot allocate a distributed object ID: all IDs used up.")
            return None
        id = self.DOIDnext
        self.DOIDnext = self.DOIDnext + 1
        dclass = self.dclassesByName[className]
        classDef = dclass.getClassDef()
        if classDef == None:
            self.notify.error("Could not create an undefined %s object." % (
                dclass.getName()))
        obj = classDef(self)
        obj.dclass = dclass
        obj.zone = zoneId
        obj.doId = id
        self.doId2do[id] = obj
        obj.generateInit()
        obj._retrieveCachedData()
        obj.generate()
        obj.announceGenerate()
        datagram = dclass.clientFormatGenerate(obj, id, zoneId, optionalFields)
        self.send(datagram)
        return obj

    def sendDisableMsg(self, doId):
        datagram = PyDatagram()
        datagram.addUint16(CLIENT_OBJECT_DISABLE)
        datagram.addUint32(doId)
        self.send(datagram)

    def sendDeleteMsg(self, doId):
        datagram = PyDatagram()
        datagram.addUint16(CLIENT_OBJECT_DELETE)
        datagram.addUint32(doId)
        self.send(datagram)

    def sendRemoveZoneMsg(self, zoneId, visibleZoneList=None):
        datagram = PyDatagram()
        datagram.addUint16(CLIENT_REMOVE_ZONE)
        datagram.addUint32(zoneId)

        # if we have an explicit list of visible zones, add them
        if visibleZoneList is not None:
            vzl = list(visibleZoneList)
            vzl.sort()
            assert PythonUtil.uniqueElements(vzl)
            for zone in vzl:
                datagram.addUint32(zone)

        # send the message
        self.send(datagram)

    def sendUpdateZone(self, obj, zoneId):
        id = obj.doId
        assert self.isLocalId(id)
        self.sendDeleteMsg(id, 1)
        obj.zone = zoneId
        self.send(obj.dclass.clientFormatGenerate(obj, id, zoneId, []))

    def sendSetZoneMsg(self, zoneId, visibleZoneList=None):
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_SET_ZONE_CMU)
        # Add zone id
        datagram.addUint32(zoneId)

        # if we have an explicit list of visible zones, add them
        if visibleZoneList is not None:
            vzl = list(visibleZoneList)
            vzl.sort()
            assert PythonUtil.uniqueElements(vzl)
            for zone in vzl:
                datagram.addUint32(zone)

        # send the message
        self.send(datagram)

    def isLocalId(self, id):
        return ((id >= self.DOIDbase) and (id < self.DOIDlast))

    def haveCreateAuthority(self):
        return (self.DOIDlast > self.DOIDnext)

    def handleDatagram(self, di):
        if self.notify.getDebug():
            print "ClientRepository received datagram:"
            di.getDatagram().dumpHex(ostream)

        msgType = self.getMsgType()

        # These are the sort of messages we may expect from the public
        # Panda server.

        if msgType == CLIENT_SET_DOID_RANGE:
            self.handleSetDOIDrange(di)
        elif msgType == CLIENT_CREATE_OBJECT_REQUIRED_RESP:
            self.handleGenerateWithRequired(di)
        elif msgType == CLIENT_CREATE_OBJECT_REQUIRED_OTHER_RESP:
            self.handleGenerateWithRequiredOther(di)
        elif msgType == CLIENT_OBJECT_UPDATE_FIELD_RESP:
            self.handleUpdateField(di)
        elif msgType == CLIENT_OBJECT_DELETE_RESP:
            self.handleDelete(di)
        elif msgType == CLIENT_OBJECT_DISABLE_RESP:
            self.handleDisable(di)
        elif msgType == CLIENT_REQUEST_GENERATES:
            self.handleRequestGenerates(di)
        else:
            self.handleMessageType(msgType, di)

        # If we're processing a lot of datagrams within one frame, we
        # may forget to send heartbeats.  Keep them coming!
        self.considerHeartbeat()

    def handleMessageType(self, msgType, di):
        self.notify.error("unrecognized message")

    def handleGenerateWithRequired(self, di):
        # Get the class Id
        classId = di.getUint16()
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        dclass.startGenerate()
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredFields(dclass, doId, di)
        dclass.stopGenerate()

    def generateWithRequiredFields(self, dclass, doId, di):
        if self.doId2do.has_key(doId):
            # ...it is in our dictionary.
            # Just update it.
            distObj = self.doId2do[doId]
            assert distObj.dclass == dclass
            distObj.generate()
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
        elif self.cache.contains(doId):
            # ...it is in the cache.
            # Pull it out of the cache:
            distObj = self.cache.retrieve(doId)
            assert distObj.dclass == dclass
            # put it in the dictionary:
            self.doId2do[doId] = distObj
            # and update it.
            distObj.generate()
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
        else:
            # ...it is not in the dictionary or the cache.
            # Construct a new one
            classDef = dclass.getClassDef()
            if classDef == None:
                self.notify.error("Could not create an undefined %s object." % (
                    dclass.getName()))
            distObj = classDef(self)
            distObj.dclass = dclass
            # Assign it an Id
            distObj.doId = doId
            # Put the new do in the dictionary
            self.doId2do[doId] = distObj
            # Update the required fields
            distObj.generateInit()  # Only called when constructed
            distObj._retrieveCachedData()
            distObj.generate()
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
        return distObj
