from pandac.PandaModules import *
from MsgTypes import *
from direct.task import Task
from direct.directnotify import DirectNotifyGlobal
import CRCache
from direct.distributed.ConnectionRepository import ConnectionRepository
from direct.showbase import PythonUtil
import ParentMgr
import RelatedObjectMgr
import time
from ClockDelta import *
from PyDatagram import PyDatagram
from PyDatagramIterator import PyDatagramIterator

class ClientRepositoryBase(ConnectionRepository):
    """
    This maintains a client-side connection with a Panda server.

    This base class exists to collect the common code between
    ClientRepository, which is the CMU-provided, open-source version
    of the client repository code, and OTPClientRepository, which is
    the VR Studio's implementation of the same.
    """
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepositoryBase")

    def __init__(self, dcFileNames = None):
        self.dcSuffix=""
        ConnectionRepository.__init__(self, ConnectionRepository.CM_HTTP, base.config, hasOwnerView=True)
        if hasattr(self, 'setVerbose'):
            if self.config.GetBool('verbose-clientrepository'):
                self.setVerbose(1)

        self.context=100000
        self.setClientDatagram(1)

        self.recorder = base.recorder

        self.readDCFile(dcFileNames)
        self.cache=CRCache.CRCache()
        self.cacheOwner=CRCache.CRCache()
        self.serverDelta = 0

        self.bootedIndex = None
        self.bootedText = None

        # create a parentMgr to handle distributed reparents
        # this used to be 'token2nodePath'
        self.parentMgr = ParentMgr.ParentMgr()

        # The RelatedObjectMgr helps distributed objects find each
        # other.
        self.relatedObjectMgr = RelatedObjectMgr.RelatedObjectMgr(self)

        # Keep track of how recently we last sent a heartbeat message.
        # We want to keep these coming at heartbeatInterval seconds.
        self.heartbeatInterval = base.config.GetDouble('heartbeat-interval', 10)
        self.heartbeatStarted = 0
        self.lastHeartbeat = 0

    ## def queryObjectAll(self, doID, context=0):
        ## """
        ## Get a one-time snapshot look at the object.
        ## """
        ## assert self.notify.debugStateCall(self)
        ## # Create a message
        ## datagram = PyDatagram()
        ## datagram.addServerHeader(
            ## doID, localAvatar.getDoId(), 2020)
        ## # A context that can be used to index the response if needed
        ## datagram.addUint32(context)
        ## self.send(datagram)
        ## # Make sure the message gets there.
        ## self.flush()

    # Define uniqueName
    def uniqueName(self, desc):
        return desc

    def getTables(self, ownerView):
        if ownerView:
            return self.doId2ownerView, self.cacheOwner
        else:
            return self.doId2do, self.cache

    def _getMsgName(self, msgId):
        return makeList(MsgId2Names.get(msgId, 'UNKNOWN MESSAGE: %s' % msgId))[0]

    def sendDisconnect(self):
        if self.isConnected():
            # Tell the game server that we're going:
            datagram = PyDatagram()
            # Add message type
            datagram.addUint16(CLIENT_DISCONNECT)
            # Send the message
            self.send(datagram)
            self.notify.info("Sent disconnect message to server")
            self.disconnect()
        self.stopHeartbeat()

    def allocateContext(self):
        self.context+=1
        return self.context

    def setServerDelta(self, delta):
        """
        Indicates the approximate difference in seconds between the
        client's clock and the server's clock, in universal time (not
        including timezone shifts).  This is mainly useful for
        reporting synchronization information to the logs; don't
        depend on it for any precise timing requirements.

        Also see Notify.setServerDelta(), which also accounts for a
        timezone shift.
        """
        self.serverDelta = delta

    def getServerDelta(self):
        return self.serverDelta

    def getServerTimeOfDay(self):
        """
        Returns the current time of day (seconds elapsed since the
        1972 epoch) according to the server's clock.  This is in GMT,
        and hence is irrespective of timezones.

        The value is computed based on the client's clock and the
        known delta from the server's clock, which is not terribly
        precisely measured and may drift slightly after startup, but
        it should be accurate plus or minus a couple of seconds.
        """
        return time.time() + self.serverDelta

    def handleGenerateWithRequired(self, di):
        parentId = di.getUint32()
        zoneId = di.getUint32()
        assert parentId == self.GameGlobalsId or parentId in self.doId2do
        # Get the class Id
        classId = di.getUint16()
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        dclass.startGenerate()
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredFields(dclass, doId, di, parentId, zoneId)
        dclass.stopGenerate()

    def handleGenerateWithRequiredOther(self, di):
        parentId = di.getUint32()
        zoneId = di.getUint32()
        assert parentId == self.GameGlobalsId or parentId in self.doId2do
        # Get the class Id
        classId = di.getUint16()
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        dclass.startGenerate()
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredOtherFields(dclass, doId, di, parentId, zoneId)
        dclass.stopGenerate()

    def handleGenerateWithRequiredOtherOwner(self, di):
        # Get the class Id
        classId = di.getUint16()
        # Get the DO Id
        doId = di.getUint32()
        # parentId and zoneId are not relevant here
        parentId = di.getUint32()
        zoneId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        dclass.startGenerate()
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredOtherFieldsOwner(dclass, doId, di)
        dclass.stopGenerate()

    def handleQuietZoneGenerateWithRequired(self, di):
        # Special handler for quiet zone generates -- we need to filter
        parentId = di.getUint32()
        zoneId = di.getUint32()
        assert parentId in self.doId2do
        # Get the class Id
        classId = di.getUint16()
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        dclass.startGenerate()
        distObj = self.generateWithRequiredFields(dclass, doId, di, parentId, zoneId)
        dclass.stopGenerate()

    def handleQuietZoneGenerateWithRequiredOther(self, di):
        # Special handler for quiet zone generates -- we need to filter
        parentId = di.getUint32()
        zoneId = di.getUint32()
        assert parentId in self.doId2do
        # Get the class Id
        classId = di.getUint16()
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        dclass.startGenerate()
        distObj = self.generateWithRequiredOtherFields(dclass, doId, di, parentId, zoneId)
        dclass.stopGenerate()

    def generateWithRequiredFields(self, dclass, doId, di, parentId, zoneId):
        if self.doId2do.has_key(doId):
            # ...it is in our dictionary.
            # Just update it.
            distObj = self.doId2do[doId]
            assert distObj.dclass == dclass
            distObj.generate()
            distObj.setLocation(parentId, zoneId)
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
            # make sure we don't have a stale location
            distObj.parentId = None
            distObj.zoneId = None
            distObj.setLocation(parentId, zoneId)
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
        else:
            # ...it is not in the dictionary or the cache.
            # Construct a new one
            classDef = dclass.getClassDef()
            if classDef == None:
                self.notify.error("Could not create an undefined %s object." % (dclass.getName()))
            distObj = classDef(self)
            distObj.dclass = dclass
            # Assign it an Id
            distObj.doId = doId
            # Put the new do in the dictionary
            self.doId2do[doId] = distObj
            # Update the required fields
            distObj.generateInit()  # Only called when constructed
            distObj.generate()
            distObj.setLocation(parentId, zoneId)
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
            print "New DO:%s, dclass:%s"%(doId, dclass.getName())
        return distObj

    def generateWithRequiredOtherFields(self, dclass, doId, di,
                                        parentId = None, zoneId = None):
        if self.doId2do.has_key(doId):
            # ...it is in our dictionary.
            # Just update it.
            distObj = self.doId2do[doId]
            assert distObj.dclass == dclass
            distObj.generate()
            distObj.setLocation(parentId, zoneId)
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        elif self.cache.contains(doId):
            # ...it is in the cache.
            # Pull it out of the cache:
            distObj = self.cache.retrieve(doId)
            assert distObj.dclass == dclass
            # put it in the dictionary:
            self.doId2do[doId] = distObj
            # and update it.
            distObj.generate()
            # make sure we don't have a stale location
            distObj.parentId = None
            distObj.zoneId = None
            distObj.setLocation(parentId, zoneId)
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        else:
            # ...it is not in the dictionary or the cache.
            # Construct a new one
            classDef = dclass.getClassDef()
            if classDef == None:
                self.notify.error("Could not create an undefined %s object." % (dclass.getName()))
            distObj = classDef(self)
            distObj.dclass = dclass
            # Assign it an Id
            distObj.doId = doId
            # Put the new do in the dictionary
            self.doId2do[doId] = distObj
            # Update the required fields
            distObj.generateInit()  # Only called when constructed
            distObj.generate()
            distObj.setLocation(parentId, zoneId)
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        return distObj

    def generateWithRequiredOtherFieldsOwner(self, dclass, doId, di):
        if self.doId2ownerView.has_key(doId):
            # ...it is in our dictionary.
            # Just update it.
            distObj = self.doId2ownerView[doId]
            assert distObj.dclass == dclass
            distObj.generate()
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        elif self.cacheOwner.contains(doId):
            # ...it is in the cache.
            # Pull it out of the cache:
            distObj = self.cacheOwner.retrieve(doId)
            assert distObj.dclass == dclass
            # put it in the dictionary:
            self.doId2ownerView[doId] = distObj
            # and update it.
            distObj.generate()
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
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
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        return distObj


    def handleDisable(self, di, ownerView=False):
        # Get the DO Id
        doId = di.getUint32()
        # disable it.
        self.disableDoId(doId, ownerView)

    def disableDoId(self, doId, ownerView=False):
        table, cache = self.getTables(ownerView)
        # Make sure the object exists
        if table.has_key(doId):
            # Look up the object
            distObj = table[doId]
            # remove the object from the dictionary
            del table[doId]

            # Only cache the object if it is a "cacheable" type
            # object; this way we don't clutter up the caches with
            # trivial objects that don't benefit from caching.
            if distObj.getCacheable():
                cache.cache(distObj)
            else:
                distObj.deleteOrDelay()
        else:
            self._logFailedDisable(doId, ownerView)

    def _logFailedDisable(self, doId, ownerView):
        self.notify.warning(
            "Disable failed. DistObj "
            + str(doId) +
            " is not in dictionary, ownerView=%s" % ownerView)

    def handleDelete(self, di):
        # overridden by ToontownClientRepository
        assert 0

    def handleUpdateField(self, di):
        """
        This method is called when a CLIENT_OBJECT_UPDATE_FIELD
        message is received; it decodes the update, unpacks the
        arguments, and calls the corresponding method on the indicated
        DistributedObject.

        In fact, this method is exactly duplicated by the C++ method
        cConnectionRepository::handle_update_field(), which was
        written to optimize the message loop by handling all of the
        CLIENT_OBJECT_UPDATE_FIELD messages in C++.  That means that
        nowadays, this Python method will probably never be called,
        since UPDATE_FIELD messages will not even be passed to the
        Python message handlers.  But this method remains for
        documentation purposes, and also as a "just in case" handler
        in case we ever do come across a situation in the future in
        which python might handle the UPDATE_FIELD message.
        """
        # Get the DO Id
        doId = di.getUint32()
        #print("Updating " + str(doId))
        # Find the DO

        do = self.doId2do.get(doId)
        if do is not None:
            # Let the dclass finish the job
            do.dclass.receiveUpdate(do, di)
        else:
            self.notify.warning(
                "Asked to update non-existent DistObj " + str(doId))

    def handleGoGetLost(self, di):
        # The server told us it's about to drop the connection on us.
        # Get ready!
        if (di.getRemainingSize() > 0):
            self.bootedIndex = di.getUint16()
            self.bootedText = di.getString()

            self.notify.warning(
                "Server is booting us out (%d): %s" % (self.bootedIndex, self.bootedText))
        else:
            self.bootedIndex = None
            self.bootedText = None
            self.notify.warning(
                "Server is booting us out with no explanation.")

    def handleServerHeartbeat(self, di):
        # Got a heartbeat message from the server.
        if base.config.GetBool('server-heartbeat-info', 1):
            self.notify.info("Server heartbeat.")

    def handleSystemMessage(self, di):
        # Got a system message from the server.
        message = di.getString()
        self.notify.info('Message from server: %s' % (message))
        return message

    def getObjectsOfClass(self, objClass):
        """ returns dict of doId:object, containing all objects
        that inherit from 'class'. returned dict is safely mutable. """
        doDict = {}
        for doId, do in self.doId2do.items():
            if isinstance(do, objClass):
                doDict[doId] = do
        return doDict

    def getObjectsOfExactClass(self, objClass):
        """ returns dict of doId:object, containing all objects that
        are exactly of type 'class' (neglecting inheritance). returned
        dict is safely mutable. """
        doDict = {}
        for doId, do in self.doId2do.items():
            if do.__class__ == objClass:
                doDict[doId] = do
        return doDict


    def sendSetLocation(self, doId, parentId, zoneId):
        datagram = PyDatagram()
        datagram.addUint16(CLIENT_OBJECT_LOCATION)
        datagram.addUint32(doId)
        datagram.addUint32(parentId)
        datagram.addUint32(zoneId)
        self.send(datagram)

    def sendHeartbeat(self):
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_HEARTBEAT)
        # Send it!
        self.send(datagram)
        self.lastHeartbeat = globalClock.getRealTime()
        # This is important enough to consider flushing immediately
        # (particularly if we haven't run readerPollTask recently).
        self.considerFlush()

    def considerHeartbeat(self):
        """Send a heartbeat message if we haven't sent one recently."""
        if not self.heartbeatStarted:
            self.notify.debug("Heartbeats not started; not sending.")
            return

        elapsed = globalClock.getRealTime() - self.lastHeartbeat
        if elapsed < 0 or elapsed > self.heartbeatInterval:
            # It's time to send the heartbeat again (or maybe someone
            # reset the clock back).
            self.notify.info("Sending heartbeat mid-frame.")
            self.startHeartbeat()

    def stopHeartbeat(self):
        taskMgr.remove("heartBeat")
        self.heartbeatStarted = 0

    def startHeartbeat(self):
        self.stopHeartbeat()
        self.heartbeatStarted = 1
        self.sendHeartbeat()
        self.waitForNextHeartBeat()

    def sendHeartbeatTask(self, task):
        self.sendHeartbeat()
        self.waitForNextHeartBeat()
        return Task.done

    def waitForNextHeartBeat(self):
        taskMgr.doMethodLater(self.heartbeatInterval, self.sendHeartbeatTask,
                              "heartBeat")

    def replaceMethod(self, oldMethod, newFunction):
        return 0

    def getWorld(self, doId):
        # Get the world node for this object
        obj = self.doId2do[doId]

        worldNP = obj.getParent()
        while 1:
            nextNP = worldNP.getParent()
            if nextNP == render:
                break
            elif worldNP.isEmpty():
                return None
        return worldNP

    def isLive(self):
        if base.config.GetBool('force-live', 0):
            return True
        return not (__dev__ or launcher.isTestServer())

    def isLocalId(self, id):
        # By default, no ID's are local.  See also
        # ClientRepository.isLocalId().
        return 0
