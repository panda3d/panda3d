"""ClientRepository module: contains the ClientRepository class"""

from PandaModules import *
from MsgTypes import *
import Task
import DirectNotifyGlobal
import ClientDistClass
import CRCache
import ConnectionRepository
import PythonUtil
import ParentMgr
import time

class ClientRepository(ConnectionRepository.ConnectionRepository):
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepository")

    def __init__(self, dcFileName):
        ConnectionRepository.ConnectionRepository.__init__(self, base.config)
        
        self.number2cdc={}
        self.name2cdc={}
        self.doId2do={}
        self.doId2cdc={}
        self.parseDcFile(dcFileName)
        self.cache=CRCache.CRCache()
        self.serverDelta = 0

        self.bootedIndex = None
        self.bootedText = None

        # create a parentMgr to handle distributed reparents
        # this used to be 'token2nodePath'
        self.parentMgr = ParentMgr.ParentMgr()

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

    def handleGenerateWithRequired(self, di):
        # Get the class Id
        classId = di.getArg(STUint16);
        # Get the DO Id
        doId = di.getArg(STUint32)
        # Look up the cdc
        cdc = self.number2cdc[classId]
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredFields(cdc, doId, di)

    def handleGenerateWithRequiredOther(self, di):
        # Get the class Id
        classId = di.getArg(STUint16);
        # Get the DO Id
        doId = di.getArg(STUint32)
        # Look up the cdc
        cdc = self.number2cdc[classId]
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredOtherFields(cdc, doId, di)

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
            # updateRequiredFields calls announceGenerate

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
            # updateRequiredFields calls announceGenerate

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
            # updateRequiredFields calls announceGenerate
            
        return distObj

    def generateWithRequiredOtherFields(self, cdc, doId, di):
        # Is it in our dictionary? 
        if self.doId2do.has_key(doId):
            # If so, just update it.
            distObj = self.doId2do[doId]
            distObj.generate()
            distObj.updateRequiredOtherFields(cdc, di)
            # updateRequiredOtherFields calls announceGenerate

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
            # updateRequiredOtherFields calls announceGenerate

        # If it is not in the dictionary or the cache, then...
        else:
            # Construct a new one
            if cdc.constructor == None:
                self.notify.error("Could not create an undefined %s object." % (cdc.name))
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
            # updateRequiredOtherFields calls announceGenerate
            
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
        if base.config.GetBool('server-heartbeat-info', 1):
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

    def sendSetShardMsg(self, shardId):
        datagram = Datagram()
        # Add message type
        datagram.addUint16(CLIENT_SET_SHARD)
        # Add shard id
        datagram.addUint32(shardId)
        # send the message
        self.send(datagram)

    def sendSetZoneMsg(self, zoneId, visibleZoneList=None):
        datagram = Datagram()
        # Add message type
        datagram.addUint16(CLIENT_SET_ZONE)
        # Add zone id
        datagram.addUint32(zoneId)

        # if we have an explicit list of visible zones, add them
        if visibleZoneList is not None:
            for zone in visibleZoneList:
                datagram.addUint32(zone)

        # send the message
        self.send(datagram)
        
    def sendUpdate(self, do, fieldName, args, sendToId = None):
        # Get the DO id
        doId = do.doId
        # Get the cdc
        cdc = self.doId2cdc.get(doId, None)
        if cdc:
            # Let the cdc finish the job
            cdc.sendUpdate(self, do, fieldName, args, sendToId)

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

    def getAllOfType(self, type):
        # Returns a list of all DistributedObjects in the repository
        # of a particular type.
        result = []
        for obj in self.doId2do.values():
            if isinstance(obj, type):
                result.append(obj)
        return result
