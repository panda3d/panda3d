"""ClientRepository module: contains the ClientRepository class"""

from PandaModules import *
from TaskManagerGlobal import *
from MsgTypes import *
from ShowBaseGlobal import *
import Task
import DirectNotifyGlobal
import ClientDistClass
import CRCache
# The repository must import all known types of Distributed Objects
#import DistributedObject
#import DistributedToon
import DirectObject

class ClientRepository(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepository")

    TASK_PRIORITY = -30

    def __init__(self, dcFileName):
        self.number2cdc={}
        self.name2cdc={}
        self.doId2do={}
        self.doId2cdc={}
        self.parseDcFile(dcFileName)
        self.cache=CRCache.CRCache()
        self.serverDelta = 0
        
        # Set this to 'http' to establish a connection to the server
        # using the HTTPClient interface, which ultimately uses the
        # OpenSSL socket library (even though SSL is not involved).
        # This is not as robust a socket library as NSPR's, but the
        # HTTPClient interface does a good job of negotiating the
        # connection over an HTTP proxy if one is in use.

        # Set it to 'nspr' to use Panda's net interface
        # (e.g. QueuedConnectionManager, etc.) to establish the
        # connection, which ultimately uses the NSPR socket library.
        # This is a much better socket library, but it may be more
        # than you need for most applications; and the proxy support
        # is weak.

        # Set it to 'default' to use the HTTPClient interface if a
        # proxy is in place, but the NSPR interface if we don't have a
        # proxy.
        
        self.connectMethod = base.config.GetString('connect-method', 'default')
        self.connectHttp = None
        self.qcm = None

        self.bootedIndex = None
        self.bootedText = None

        self.tcpConn = None

        # Reader statistics
        self.rsDatagramCount = 0
        self.rsUpdateObjs = {}
        self.rsLastUpdate = 0
        self.rsDoReport = base.config.GetBool('reader-statistics', 1)
        self.rsUpdateInterval = base.config.GetDouble('reader-statistics-interval', 10)
        return None

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

    def parseDcFile(self, dcFileName):
        self.dcFile = DCFile()
        readResult = self.dcFile.read(dcFileName)
        if not readResult:
            self.notify.error("Could not read dcfile: %s" % dcFileName.cStr())
        self.hashVal = self.dcFile.getHash()
        return self.parseDcClasses(self.dcFile)

    def parseDcClasses(self, dcFile):
        numClasses = dcFile.getNumClasses()
        for i in range(0, numClasses):
            # Create a clientDistClass from the dcClass
            dcClass = dcFile.getClass(i)
            clientDistClass = ClientDistClass.ClientDistClass(dcClass)
            # List the cdc in the number and name dictionaries
            self.number2cdc[dcClass.getNumber()]=clientDistClass
            self.name2cdc[dcClass.getName()]=clientDistClass
        return None

    def connect(self, serverList,
                successCallback = None, successArgs = [],
                failureCallback = None, failureArgs = []):
        """
        Attempts to establish a connection to the server.  May return
        before the connection is established.  The two callbacks
        represent the two functions to call (and their arguments) on
        success or failure, respectively.  The failure callback also
        gets one additional parameter, which will be passed in first:
        the return status code giving reason for failure, if it is
        known.
        """

        if self.hasProxy:
            self.notify.info("Connecting to gameserver via proxy: %s" % (self.proxy.cStr()))
        else:
            self.notify.info("Connecting to gameserver directly (no proxy).");

        if self.connectMethod == 'http':
            self.connectHttp = 1
        elif self.connectMethod == 'nspr':
            self.connectHttp = 0
        else:
            self.connectHttp = (self.hasProxy or serverList[0].getScheme() == 'https')

        self.bootedIndex = None
        self.bootedText = None
        if self.connectHttp:
            # In the HTTP case, we can't just iterate through the list
            # of servers, because each server attempt requires
            # spawning a request and then coming back later to check
            # the success or failure.  Instead, we start the ball
            # rolling by calling the connect callback, which will call
            # itself repeatedly until we establish a connection (or
            # run out of servers).
            
            ch = self.http.makeChannel(0)
            self.httpConnectCallback(ch, serverList, 0,
                                     successCallback, successArgs,
                                     failureCallback, failureArgs)

        else:
            if self.qcm == None:
                self.qcm = QueuedConnectionManager()
                self.cw = ConnectionWriter(self.qcm, 0)
                self.qcr = QueuedConnectionReader(self.qcm, 0)
                minLag = config.GetFloat('min-lag', 0.)
                maxLag = config.GetFloat('max-lag', 0.)
                if minLag or maxLag:
                    self.qcr.startDelay(minLag, maxLag)

            # A big old 20 second timeout.
            gameServerTimeoutMs = base.config.GetInt("game-server-timeout-ms",
                                                     20000)

            # Try each of the servers in turn.
            for url in serverList:
                self.notify.info("Connecting to %s via NSPR interface." % (url.cStr()))
                self.tcpConn = self.qcm.openTCPClientConnection(
                    url.getServer(), url.getPort(),
                    gameServerTimeoutMs)

                if self.tcpConn:
                    self.tcpConn.setNoDelay(1)
                    self.qcr.addConnection(self.tcpConn)
                    self.startReaderPollTask()
                    if successCallback:
                        successCallback(*successArgs)
                    return

            # Failed to connect.
            if failureCallback:
                failureCallback(0, *failureArgs)

    def disconnect(self):
        """Closes the previously-established connection.
        """
        self.notify.info("Closing connection to server.")
        if self.tcpConn != None:
            if self.connectHttp:
                self.tcpConn.close()
            else: 
                self.qcm.closeConnection(self.tcpConn)
            self.tcpConn = None
        self.stopReaderPollTask()
                    
    def httpConnectCallback(self, ch, serverList, serverIndex,
                            successCallback, successArgs,
                            failureCallback, failureArgs):
        if ch.isConnectionReady():
            self.tcpConn = ch.getConnection()
            self.tcpConn.userManagesMemory = 1
            self.startReaderPollTask()
            if successCallback:
                successCallback(*successArgs)

        elif serverIndex < len(serverList):
            # No connection yet, but keep trying.
            
            url = serverList[serverIndex]
            self.notify.info("Connecting to %s via HTTP interface." % (url.cStr()))
            ch.beginConnectTo(DocumentSpec(url))
            ch.spawnTask(name = 'connect-to-server',
                         callback = self.httpConnectCallback,
                         extraArgs = [ch, serverList, serverIndex + 1,
                                      successCallback, successArgs,
                                      failureCallback, failureArgs])
        else:
            # No more servers to try; we have to give up now.
            if failureCallback:
                failureCallback(ch.getStatusCode(), *failureArgs)

    def startReaderPollTask(self):
        # Stop any tasks we are running now
        self.stopReaderPollTask()
        taskMgr.add(self.readerPollUntilEmpty, "readerPollTask",
                    priority=self.TASK_PRIORITY)
        return None

    def stopReaderPollTask(self):
        taskMgr.remove("readerPollTask")
        return None

    def readerPollUntilEmpty(self, task):
        while self.readerPollOnce():
            pass
        return Task.cont

    def readerPollOnce(self):
        # we simulate the network plug being pulled by setting tcpConn
        # to None; enforce that condition
        if not self.tcpConn:
            return 0

        # Make sure any recently-sent datagrams are flushed when the
        # time expires, if we're in collect-tcp mode.
        self.tcpConn.considerFlush()

        if self.rsDoReport:
            self.reportReaderStatistics()
        
        if self.connectHttp:
            datagram = Datagram()
            if self.tcpConn.receiveDatagram(datagram):
                if self.rsDoReport:
                    self.rsDatagramCount += 1
                self.handleDatagram(datagram)
                return 1

            # Unable to receive a datagram: did we lose the connection?
            if self.tcpConn.isClosed():
                self.tcpConn = None
                self.stopReaderPollTask()
                self.loginFSM.request("noConnection")
            return 0
        
        else:
            self.ensureValidConnection()
            if self.qcr.dataAvailable():
                datagram = NetDatagram()
                if self.qcr.getData(datagram):
                    if self.rsDoReport:
                        self.rsDatagramCount += 1
                    self.handleDatagram(datagram)
                    return 1
            return 0

    def ensureValidConnection(self):
        # Was the connection reset?
        if self.connectHttp:
            pass
        else:
            if self.qcm.resetConnectionAvailable():
                resetConnectionPointer = PointerToConnection()
                if self.qcm.getResetConnection(resetConnectionPointer):
                    resetConn = resetConnectionPointer.p()
                    self.qcm.closeConnection(resetConn)
                    # if we've simulated a network plug pull, restore the
                    # simulated plug
                    self.restoreNetworkPlug()
                    if self.tcpConn.this == resetConn.this:
                        self.tcpConn = None
                        self.stopReaderPollTask()
                        self.loginFSM.request("noConnection")
                    else:
                        self.notify.warning("Lost unknown connection.")
        return None

    def handleDatagram(self, datagram):
        # This class is meant to be pure virtual, and any classes that
        # inherit from it need to make their own handleDatagram method
        pass

    def reportReaderStatistics(self):
        now = globalClock.getRealTime()
        if now - self.rsLastUpdate < self.rsUpdateInterval:
            return

        self.rsLastUpdate = now
        self.notify.info("Received %s datagrams" % (self.rsDatagramCount))
        if self.rsUpdateObjs:
            self.notify.info("Updates: %s" % (self.rsUpdateObjs))

        self.rsDatagramCount = 0
        self.rsUpdateObjs = {}

    def handleGenerateWithRequired(self, di):
        # Get the class Id
        classId = di.getArg(STUint16);
        # Get the DO Id
        doId = di.getArg(STUint32)
        # Look up the cdc
        cdc = self.number2cdc[classId]
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredFields(cdc, doId, di)
        return None

    def handleGenerateWithRequiredOther(self, di):
        # Get the class Id
        classId = di.getArg(STUint16);
        # Get the DO Id
        doId = di.getArg(STUint32)
        # Look up the cdc
        cdc = self.number2cdc[classId]
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredOtherFields(cdc, doId, di)
        return None

    def handleQuietZoneGenerateWithRequired(self, di):
        # Special handler for quiet zone generates -- we need to filter
        # Get the class Id
        classId = di.getArg(STUint16);
        # Get the DO Id
        doId = di.getArg(STUint32)
        # Look up the cdc
        cdc = self.number2cdc[classId]
        # If the class is a neverDisable class (which implies uberzone) we
        # should go ahead and generate it even though we are in the quiet zone
        if cdc.constructor.neverDisable:
            # Create a new distributed object, and put it in the dictionary
            distObj = self.generateWithRequiredFields(cdc, doId, di)
        return None

    def handleQuietZoneGenerateWithRequiredOther(self, di):
        # Special handler for quiet zone generates -- we need to filter
        # Get the class Id
        classId = di.getArg(STUint16);
        # Get the DO Id
        doId = di.getArg(STUint32)
        # Look up the cdc
        cdc = self.number2cdc[classId]
        # If the class is a neverDisable class (which implies uberzone) we
        # should go ahead and generate it even though we are in the quiet zone
        if cdc.constructor.neverDisable:
            # Create a new distributed object, and put it in the dictionary
            distObj = self.generateWithRequiredOtherFields(cdc, doId, di)
        return None

    def generateWithRequiredFields(self, cdc, doId, di):
        # Is it in our dictionary? 
        if self.doId2do.has_key(doId):
            # If so, just update it.
            distObj = self.doId2do[doId]
            distObj.generate()
            distObj.updateRequiredFields(cdc, di)
            distObj.announceGenerate()

        # Is it in the cache? If so, pull it out, put it in the dictionaries,
        # and update it.
        elif self.cache.contains(doId):
            # If so, pull it out of the cache...
            distObj = self.cache.retrieve(doId)
            # put it in both dictionaries...
            self.doId2do[doId] = distObj
            self.doId2cdc[doId] = cdc
            # and update it.
            distObj.generate()
            distObj.updateRequiredFields(cdc, di)
            distObj.announceGenerate()

        # If it is not in the dictionary or the cache, then...
        else:
            # Construct a new one
            distObj = cdc.constructor(self)
            # Assign it an Id
            distObj.doId = doId
            # Put the new do in both dictionaries
            self.doId2do[doId] = distObj
            self.doId2cdc[doId] = cdc
            # Update the required fields
            distObj.generateInit()  # Only called when constructed
            distObj.generate()
            distObj.updateRequiredFields(cdc, di)
            distObj.announceGenerate()
            
        return distObj

    def generateWithRequiredOtherFields(self, cdc, doId, di):
        # Is it in our dictionary? 
        if self.doId2do.has_key(doId):
            # If so, just update it.
            distObj = self.doId2do[doId]
            distObj.generate()
            distObj.updateRequiredOtherFields(cdc, di)
            distObj.announceGenerate()

        # Is it in the cache? If so, pull it out, put it in the dictionaries,
        # and update it.
        elif self.cache.contains(doId):
            # If so, pull it out of the cache...
            distObj = self.cache.retrieve(doId)
            # put it in both dictionaries...
            self.doId2do[doId] = distObj
            self.doId2cdc[doId] = cdc
            # and update it.
            distObj.generate()
            distObj.updateRequiredOtherFields(cdc, di)
            distObj.announceGenerate()

        # If it is not in the dictionary or the cache, then...
        else:
            # Construct a new one
            distObj = cdc.constructor(self)
            # Assign it an Id
            distObj.doId = doId
            # Put the new do in both dictionaries
            self.doId2do[doId] = distObj
            self.doId2cdc[doId] = cdc
            # Update the required fields
            distObj.generateInit()  # Only called when constructed
            distObj.generate()
            distObj.updateRequiredOtherFields(cdc, di)
            distObj.announceGenerate()
            
        return distObj


    def handleDisable(self, di):
        # Get the DO Id
        doId = di.getArg(STUint32)
        # disable it.
        self.disableDoId(doId)
        return None

    def disableDoId(self, doId):
         # Make sure the object exists
        if self.doId2do.has_key(doId):
            # Look up the object
            distObj = self.doId2do[doId]
            # remove the object from both dictionaries
            del(self.doId2do[doId])
            del(self.doId2cdc[doId])
            assert(len(self.doId2do) == len(self.doId2cdc))

            # Only cache the object if it is a "cacheable" type
            # object; this way we don't clutter up the caches with
            # trivial objects that don't benefit from caching.
            if distObj.getCacheable():
                self.cache.cache(distObj)
            else:
                distObj.deleteOrDelay()
        else:
            ClientRepository.notify.warning("Disable failed. DistObj " +
                                            str(doId) +
                                            " is not in dictionary")
        return None

    def handleDelete(self, di):
        # Get the DO Id
        doId = di.getArg(STUint32)
        self.deleteObject(doId)

    def deleteObject(self, doId):
        """deleteObject(self, doId)

        Removes the object from the client's view of the world.  This
        should normally not be called except in the case of error
        recovery, since the server will normally be responsible for
        deleting and disabling objects as they go out of scope.

        After this is called, future updates by server on this object
        will be ignored (with a warning message).  The object will
        become valid again the next time the server sends a generate
        message for this doId.

        This is not a distributed message and does not delete the
        object on the server or on any other client.
        """
        # If it is in the dictionaries, remove it.
        if self.doId2do.has_key(doId):
            obj = self.doId2do[doId]
            # Remove it from the dictionaries
            del(self.doId2do[doId])
            del(self.doId2cdc[doId])
            # Sanity check the dictionaries
            assert(len(self.doId2do) == len(self.doId2cdc))
            # Disable, announce, and delete the object itself...
            # unless delayDelete is on...
            obj.deleteOrDelay()
        # If it is in the cache, remove it.
        elif self.cache.contains(doId):
            self.cache.delete(doId)
        # Otherwise, ignore it
        else:
            ClientRepository.notify.warning(
                "Asked to delete non-existent DistObj " + str(doId))
        return None

    def handleUpdateField(self, di):
        # Get the DO Id
        doId = di.getArg(STUint32)
        #print("Updating " + str(doId))
        if self.rsDoReport:
            self.rsUpdateObjs[doId] = self.rsUpdateObjs.get(doId, 0) + 1
        # Find the DO
            
        do = self.doId2do.get(doId)
        cdc = self.doId2cdc.get(doId)
        if (do != None and cdc != None):
            # Let the cdc finish the job
            cdc.updateField(do, di)
        else:
            ClientRepository.notify.warning(
                "Asked to update non-existent DistObj " + str(doId))
        return None

    def handleGoGetLost(self, di):
        # The server told us it's about to drop the connection on us.
        # Get ready!
        if (di.getRemainingSize() > 0):
            self.bootedIndex = di.getUint16()
            self.bootedText = di.getString()

            ClientRepository.notify.warning(
                "Server is booting us out (%d): %s" % (self.bootedIndex, self.bootedText))
        else:
            self.bootedIndex = None
            self.bootedText = None
            ClientRepository.notify.warning(
                "Server is booting us out with no explanation.")


    def handleServerHeartbeat(self, di):
        # Got a heartbeat message from the server.
        ClientRepository.notify.info("Server heartbeat.")
        

    def handleUnexpectedMsgType(self, msgType, di):
        if msgType == CLIENT_GO_GET_LOST:
            self.handleGoGetLost(di)
        elif msgType == CLIENT_HEARTBEAT:
            self.handleServerHeartbeat(di)
        else:
            currentLoginState = self.loginFSM.getCurrentState()
            if currentLoginState:
                currentLoginStateName = currentLoginState.getName()
            else:
                currentLoginStateName = "None"
            currentGameState = self.gameFSM.getCurrentState()
            if currentGameState:
                currentGameStateName = currentGameState.getName()
            else:
                currentGameStateName = "None"
            ClientRepository.notify.warning(
                "Ignoring unexpected message type: " +
                str(msgType) +
                " login state: " +
                currentLoginStateName +
                " game state: " +
                currentGameStateName)
        return None

    def sendSetShardMsg(self, shardId):
        datagram = Datagram()
        # Add message type
        datagram.addUint16(CLIENT_SET_SHARD)
        # Add shard id
        datagram.addUint32(shardId)
        # send the message
        self.send(datagram)
        return None

    def sendSetZoneMsg(self, zoneId):
        datagram = Datagram()
        # Add message type
        datagram.addUint16(CLIENT_SET_ZONE)
        # Add zone id
        datagram.addUint16(zoneId)

        # send the message
        self.send(datagram)
        return None
        
    def sendUpdate(self, do, fieldName, args, sendToId = None):
        # Get the DO id
        doId = do.doId
        # Get the cdc
        cdc = self.doId2cdc.get(doId, None)
        if cdc:
            # Let the cdc finish the job
            cdc.sendUpdate(self, do, fieldName, args, sendToId)

    def send(self, datagram):
        if self.notify.getDebug():
            print "ClientRepository sending datagram:"
            datagram.dumpHex(ostream)

        if not self.tcpConn:
            self.notify.warning("Unable to send message after connection is closed.")
            return

        if self.connectHttp:
            if not self.tcpConn.sendDatagram(datagram):
                self.notify.warning("Could not send datagram.")
        else:
            self.cw.send(datagram, self.tcpConn)
        return None

    def replaceMethod(self, oldMethod, newFunction):
        foundIt = 0
        import new
        # Iterate over the ClientDistClasses
        for cdc in self.number2cdc.values():
            # Iterate over the ClientDistUpdates
            for cdu in cdc.allCDU:
                method = cdu.func
                # See if this is a match
                if (method and (method.im_func == oldMethod)):
                    # Create a new unbound method out of this new function
                    newMethod = new.instancemethod(newFunction,
                                                   method.im_self,
                                                   method.im_class)
                    # Set the new method on the cdu
                    cdu.func = newMethod
                    foundIt = 1
        return foundIt

    # debugging funcs for simulating a network-plug-pull
    def pullNetworkPlug(self):
        self.restoreNetworkPlug()
        self.notify.warning('*** SIMULATING A NETWORK-PLUG-PULL ***')
        self.hijackedTcpConn = self.tcpConn
        self.tcpConn = None

    def networkPlugPulled(self):
        return hasattr(self, 'hijackedTcpConn')

    def restoreNetworkPlug(self):
        if self.networkPlugPulled():
            self.notify.info('*** RESTORING SIMULATED PULLED-NETWORK-PLUG ***')
            self.tcpConn = self.hijackedTcpConn
            del self.hijackedTcpConn

    def getAllOfType(self, type):
        # Returns a list of all DistributedObjects in the repository
        # of a particular type.
        result = []
        for obj in self.doId2do.values():
            if isinstance(obj, type):
                result.append(obj)
        return result
