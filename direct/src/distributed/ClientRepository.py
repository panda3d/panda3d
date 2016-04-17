"""ClientRepository module: contains the ClientRepository class"""

from .ClientRepositoryBase import ClientRepositoryBase
from direct.directnotify import DirectNotifyGlobal
from .MsgTypesCMU import *
from .PyDatagram import PyDatagram
from .PyDatagramIterator import PyDatagramIterator
from panda3d.core import UniqueIdAllocator


class ClientRepository(ClientRepositoryBase):
    """
    This is the open-source ClientRepository as provided by CMU.  It
    communicates with the ServerRepository in this same directory.

    If you are looking for the VR Studio's implementation of the
    client repository, look to OTPClientRepository (elsewhere).
    """
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepository")

    # This is required by DoCollectionManager, even though it's not
    # used by this implementation.
    GameGlobalsId = 0

    doNotDeallocateChannel = True

    def __init__(self, dcFileNames = None, dcSuffix = '', connectMethod = None,
                 threadedNet = None):
        ClientRepositoryBase.__init__(self, dcFileNames = dcFileNames, dcSuffix = dcSuffix, connectMethod = connectMethod, threadedNet = threadedNet)
        self.setHandleDatagramsInternally(False)

        base.finalExitCallbacks.append(self.shutdown)

        # The doId allocator.  The CMU LAN server may choose to
        # send us a block of doIds.  If it chooses to do so, then we
        # may create objects, using those doIds.
        self.doIdAllocator = None
        self.doIdBase = 0
        self.doIdLast = 0

        # The doIdBase of the client message currently being
        # processed.
        self.currentSenderId = None

        # Explicitly-requested interest zones.
        self.interestZones = []

    def handleSetDoIdrange(self, di):
        self.doIdBase = di.getUint32()
        self.doIdLast = self.doIdBase + di.getUint32()
        self.doIdAllocator = UniqueIdAllocator(self.doIdBase, self.doIdLast - 1)

        self.ourChannel = self.doIdBase

        self.createReady()

    def createReady(self):
        # Now that we've got a doId range, we can safely generate new
        # distributed objects.
        messenger.send('createReady', taskChain = 'default')
        messenger.send(self.uniqueName('createReady'), taskChain = 'default')

    def handleRequestGenerates(self, di):
        # When new clients join the zone of an object, they need to hear
        # about it, so we send out all of our information about objects in
        # that particular zone.

        zone = di.getUint32()
        for obj in self.doId2do.values():
            if obj.zoneId == zone:
                if (self.isLocalId(obj.doId)):
                    self.resendGenerate(obj)

    def resendGenerate(self, obj):
        """ Sends the generate message again for an already-generated
        object, presumably to inform any newly-arrived clients of this
        object's current state. """

        # get the list of "ram" fields that aren't
        # required.  These are fields whose values should
        # persist even if they haven't been received
        # lately, so we have to re-broadcast these values
        # in case the new client hasn't heard their latest
        # values.
        extraFields = []
        for i in range(obj.dclass.getNumInheritedFields()):
            field = obj.dclass.getInheritedField(i)
            if field.hasKeyword('broadcast') and field.hasKeyword('ram') and not field.hasKeyword('required'):
                if field.asMolecularField():
                    # It's a molecular field; this means
                    # we have to pack the components.
                    # Fortunately, we'll find those
                    # separately through the iteration, so
                    # we can ignore this field itself.
                    continue

                extraFields.append(field.getName())

        datagram = self.formatGenerate(obj, extraFields)
        self.send(datagram)

    def handleGenerate(self, di):
        self.currentSenderId = di.getUint32()
        zoneId = di.getUint32()
        classId = di.getUint16()
        doId = di.getUint32()

        # Look up the dclass
        dclass = self.dclassesByNumber[classId]

        distObj = self.doId2do.get(doId)
        if distObj and distObj.dclass == dclass:
            # We've already got this object.  Probably this is just a
            # repeat-generate, synthesized for the benefit of someone
            # else who just entered the zone.  Accept the new updates,
            # but don't make a formal generate.
            assert(self.notify.debug("performing generate-update for %s %s" % (dclass.getName(), doId)))
            dclass.receiveUpdateBroadcastRequired(distObj, di)
            dclass.receiveUpdateOther(distObj, di)
            return

        assert(self.notify.debug("performing generate for %s %s" % (dclass.getName(), doId)))
        dclass.startGenerate()
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredOtherFields(dclass, doId, di, 0, zoneId)
        dclass.stopGenerate()

    def allocateDoId(self):
        """ Returns a newly-allocated doId.  Call freeDoId() when the
        object has been deleted. """

        return self.doIdAllocator.allocate()

    def reserveDoId(self, doId):
        """ Removes the indicate doId from the available pool, as if
        it had been explicitly allocated.  You may pass it to
        freeDoId() later if you wish. """

        self.doIdAllocator.initialReserveId(doId)
        return doId

    def freeDoId(self, doId):
        """ Returns a doId back into the free pool for re-use. """

        assert self.isLocalId(doId)
        self.doIdAllocator.free(doId)

    def storeObjectLocation(self, object, parentId, zoneId):
        # The CMU implementation doesn't use the DoCollectionManager
        # much.
        object.parentId = parentId
        object.zoneId = zoneId

    def createDistributedObject(self, className = None, distObj = None,
                                zoneId = 0, optionalFields = None,
                                doId = None, reserveDoId = False):

        """ To create a DistributedObject, you must pass in either the
        name of the object's class, or an already-created instance of
        the class (or both).  If you pass in just a class name (to the
        className parameter), then a default instance of the object
        will be created, with whatever parameters the default
        constructor supplies.  Alternatively, if you wish to create
        some initial values different from the default, you can create
        the instance yourself and supply it to the distObj parameter,
        then that instance will be used instead.  (It should be a
        newly-created object, not one that has already been manifested
        on the network or previously passed through
        createDistributedObject.)  In either case, the new
        DistributedObject is returned from this method.

        This method will issue the appropriate network commands to
        make this object appear on all of the other clients.

        You should supply an initial zoneId in which to manifest the
        object.  The fields marked "required" or "ram" will be
        broadcast to all of the other clients; if you wish to
        broadcast additional field values at this time as well, pass a
        list of field names in the optionalFields parameters.

        Normally, doId is None, to mean allocate a new doId for the
        object.  If you wish to use a particular doId, pass it in
        here.  If you also pass reserveDoId = True, this doId will be
        reserved from the allocation pool using self.reserveDoId().
        You are responsible for ensuring this doId falls within the
        client's allowable doId range and has not already been
        assigned to another object.  """

        if not className:
            if not distObj:
                self.notify.error("Must specify either a className or a distObj.")
            className = distObj.__class__.__name__

        if doId is None:
            doId = self.allocateDoId()
        elif reserveDoId:
            self.reserveDoId(doId)

        dclass = self.dclassesByName.get(className)
        if not dclass:
            self.notify.error("Unknown distributed class: %s" % (distObj.__class__))
        classDef = dclass.getClassDef()
        if classDef == None:
            self.notify.error("Could not create an undefined %s object." % (
                dclass.getName()))

        if not distObj:
            distObj = classDef(self)
        if not isinstance(distObj, classDef):
            self.notify.error("Object %s is not an instance of %s" % (distObj.__class__.__name__, classDef.__name__))

        distObj.dclass = dclass
        distObj.doId = doId
        self.doId2do[doId] = distObj
        distObj.generateInit()
        distObj._retrieveCachedData()
        distObj.generate()
        distObj.setLocation(0, zoneId)
        distObj.announceGenerate()
        datagram = self.formatGenerate(distObj, optionalFields)
        self.send(datagram)
        return distObj

    def formatGenerate(self, distObj, extraFields):
        """ Returns a datagram formatted for sending the generate message for the indicated object. """
        return distObj.dclass.clientFormatGenerateCMU(distObj, distObj.doId, distObj.zoneId, extraFields)

    def sendDeleteMsg(self, doId):
        datagram = PyDatagram()
        datagram.addUint16(OBJECT_DELETE_CMU)
        datagram.addUint32(doId)
        self.send(datagram)

    def sendDisconnect(self):
        if self.isConnected():
            # Tell the game server that we're going:
            datagram = PyDatagram()
            # Add message type
            datagram.addUint16(CLIENT_DISCONNECT_CMU)
            # Send the message
            self.send(datagram)
            self.notify.info("Sent disconnect message to server")
            self.disconnect()
        self.stopHeartbeat()

    def setInterestZones(self, interestZoneIds):
        """ Changes the set of zones that this particular client is
        interested in hearing about. """

        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_SET_INTEREST_CMU)

        for zoneId in interestZoneIds:
            datagram.addUint32(zoneId)

        # send the message
        self.send(datagram)
        self.interestZones = interestZoneIds[:]

    def setObjectZone(self, distObj, zoneId):
        """ Moves the object into the indicated zone. """
        distObj.b_setLocation(0, zoneId)
        assert distObj.zoneId == zoneId

        # Tell all of the clients monitoring the new zone that we've
        # arrived.
        self.resendGenerate(distObj)

    def sendSetLocation(self, doId, parentId, zoneId):
        datagram = PyDatagram()
        datagram.addUint16(OBJECT_SET_ZONE_CMU)
        datagram.addUint32(doId)
        datagram.addUint32(zoneId)
        self.send(datagram)

    def sendHeartbeat(self):
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_HEARTBEAT_CMU)
        # Send it!
        self.send(datagram)
        self.lastHeartbeat = globalClock.getRealTime()
        # This is important enough to consider flushing immediately
        # (particularly if we haven't run readerPollTask recently).
        self.considerFlush()

    def isLocalId(self, doId):
        """ Returns true if this doId is one that we're the owner of,
        false otherwise. """

        return ((doId >= self.doIdBase) and (doId < self.doIdLast))

    def haveCreateAuthority(self):
        """ Returns true if this client has been assigned a range of
        doId's it may use to create objects, false otherwise. """

        return (self.doIdLast > self.doIdBase)

    def getAvatarIdFromSender(self):
        """ Returns the doIdBase of the client that originally sent
        the current update message.  This is only defined when
        processing an update message or a generate message. """
        return self.currentSenderId

    def handleDatagram(self, di):
        if self.notify.getDebug():
            print("ClientRepository received datagram:")
            di.getDatagram().dumpHex(ostream)

        msgType = self.getMsgType()
        self.currentSenderId = None

        # These are the sort of messages we may expect from the public
        # Panda server.

        if msgType == SET_DOID_RANGE_CMU:
            self.handleSetDoIdrange(di)
        elif msgType == OBJECT_GENERATE_CMU:
            self.handleGenerate(di)
        elif msgType == OBJECT_UPDATE_FIELD_CMU:
            self.handleUpdateField(di)
        elif msgType == OBJECT_DISABLE_CMU:
            self.handleDisable(di)
        elif msgType == OBJECT_DELETE_CMU:
            self.handleDelete(di)
        elif msgType == REQUEST_GENERATES_CMU:
            self.handleRequestGenerates(di)
        else:
            self.handleMessageType(msgType, di)

        # If we're processing a lot of datagrams within one frame, we
        # may forget to send heartbeats.  Keep them coming!
        self.considerHeartbeat()

    def handleMessageType(self, msgType, di):
        self.notify.error("unrecognized message type %s" % (msgType))

    def handleUpdateField(self, di):
        # The CMU update message starts with an additional field, not
        # present in the Disney update message: the doIdBase of the
        # original sender.  Extract that and call up to the parent.
        self.currentSenderId = di.getUint32()
        ClientRepositoryBase.handleUpdateField(self, di)

    def handleDisable(self, di):
        # Receives a list of doIds.
        while di.getRemainingSize() > 0:
            doId = di.getUint32()

            # We should never get a disable message for our own object.
            assert not self.isLocalId(doId)
            self.disableDoId(doId)

    def handleDelete(self, di):
        # Receives a single doId.
        doId = di.getUint32()
        self.deleteObject(doId)

    def deleteObject(self, doId):
        """
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

    def stopTrackRequestDeletedDO(self, *args):
        # No-op.  Not entirely sure what this does on the VR Studio side.
        pass

    def sendUpdate(self, distObj, fieldName, args):
        """ Sends a normal update for a single field. """
        dg = distObj.dclass.clientFormatUpdate(
            fieldName, distObj.doId, args)
        self.send(dg)

    def sendUpdateToChannel(self, distObj, channelId, fieldName, args):

        """ Sends a targeted update of a single field to a particular
        client.  The top 32 bits of channelId is ignored; the lower 32
        bits should be the client Id of the recipient (i.e. the
        client's doIdbase).  The field update will be sent to the
        indicated client only.  The field must be marked clsend or
        p2p, and may not be marked broadcast. """

        datagram = distObj.dclass.clientFormatUpdate(
            fieldName, distObj.doId, args)
        dgi = PyDatagramIterator(datagram)

        # Reformat the packed datagram to change the message type and
        # add the target id.
        dgi.getUint16()

        dg = PyDatagram()
        dg.addUint16(CLIENT_OBJECT_UPDATE_FIELD_TARGETED_CMU)
        dg.addUint32(channelId & 0xffffffff)
        dg.appendData(dgi.getRemainingBytes())

        self.send(dg)
