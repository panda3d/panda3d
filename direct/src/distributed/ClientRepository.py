"""ClientRepository module: contains the ClientRepository class"""

from pandac.PandaModules import *
from MsgTypes import *
from direct.task import Task
from direct.directnotify import DirectNotifyGlobal
import CRCache
import ConnectionRepository
from direct.showbase import PythonUtil
import ParentMgr
import RelatedObjectMgr
import time
from ClockDelta import *
from PyDatagram import PyDatagram
from PyDatagramIterator import PyDatagramIterator

class ClientRepository(ConnectionRepository.ConnectionRepository):
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepository")

    def __init__(self):
        ConnectionRepository.ConnectionRepository.__init__(self, base.config)
        self.setClientDatagram(1)

        self.recorder = base.recorder

        # Dict of {DistributedObject ids : DistributedObjects}
        self.doId2do = {}
        if wantOtpServer:
            # Dict of {parent DistributedObject id : {zoneIds : [child DistributedObject ids]}}
            self.__doHierarchy = {}
        self.readDCFile()
        self.cache=CRCache.CRCache()
        self.serverDelta = 0

        self.bootedIndex = None
        self.bootedText = None

        self.worldScale = render.attachNewNode("worldScale") # for grid zones.
        self.worldScale.setScale(base.config.GetFloat('world-scale', 100))
        self.priorWorldPos = None
        
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
        if wantOtpServer:
            # Top level Interest Manager
            self._interestIdAssign = 1
            self._interests = {}
        
    
    def abruptCleanup(self):
        """
        Call this method to clean up any pending hooks or tasks on
        distributed objects, but leave the ClientRepository in a sane
        state for creating more distributed objects.
        """
        self.relatedObjectMgr.abortAllRequests()

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

    def setWorldOffset(self, xOffset=0, yOffset=0):
        self.worldXOffset=xOffset
        self.worldYOffset=yOffset

    def getWorldPos(self, nodePath):
        pos = nodePath.getPos(self.worldScale)
        return (int(round(pos.getX())), int(round(pos.getY())))

    def sendWorldPos(self, x, y):
        # The server will need to know the world
        # offset of our current render node path
        # and adjust the x, y accordingly.  At one
        # point I considered adding the world offset
        # here, but that would just use extra bits.

        onScreenDebug.add("worldPos", "%-4d, %-4d"%(x, y))
        return #*#

        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_SET_WORLD_POS)
        # Add x
        datagram.addInt16(x)
        # Add y
        datagram.addSint16(y)
        # send the message
        self.send(datagram)

    def checkWorldPos(self, nodePath):
        worldPos = self.getWorldPos(nodePath)
        if self.priorWorldPos != worldPos:
            self.priorWorldPos = worldPos
            self.sendWorldPos(worldPos[0], worldPos[1])

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

    if wantOtpServer:
        def handleObjectLocation(self, di):
            # CLIENT_OBJECT_LOCATION        
            doId = di.getUint32()
            parentId = di.getUint32()
            zoneId = di.getUint32()
            obj = self.doId2do.get(doId)
            if (obj != None):
                self.notify.info("handleObjectLocation: doId: %s parentId: %s zoneId: %s" %
                                 (doId, parentId, zoneId))
                # Let the object finish the job
                obj.setLocation(parentId, zoneId)
            else:
                ClientRepository.notify.warning(
                    "handleObjectLocation: Asked to update non-existent obj: %s" % (doId))

        def storeObjectLocation(self, objId, parentId, zoneId):
            # Do not store null values
            if ((parentId is None) or (zoneId is None)):
                return

            # TODO: check current location
            obj = self.doId2do.get(objId)
            oldParentId, oldZoneId = obj.getLocation()

            # Case 1: Same parent, new zone
            if (oldParentId == parentId):
                parentZoneDict = self.__doHierarchy.get(parentId)
                # Remove this objId from the old zone list
                oldObjList = parentZoneDict.get(oldZoneId)
                oldObjList.remove(objId)
                # Add it to the new zone list
                objList = parentZoneDict.get(zoneId)
                if objList is None:
                    # No existing objList for this zone, let's make a new one
                    parentZoneDict[zoneId] = [objId]
                    return
                else:
                    # Just add this objId to the existing list
                    assert(objId not in objList)
                    objList.append(objId)
                    return 

            # Case 2: New parent, valid old parent
            # First delete the old location
            if ((oldParentId is not None) and (oldZoneId is not None)):
                self.deleteObjectLocation(objId, oldParentId, oldZoneId)
                # Do not return because we still need to add to the new location

            # Case 2: continued, already deleted from old location
            # Case 3: New parent - no old parent
            parentZoneDict = self.__doHierarchy.get(parentId)
            if parentZoneDict is None:
                # This parent is not here, just fill the whole entry in
                self.__doHierarchy[parentId] = {zoneId : [objId]}
            else:
                objList = parentZoneDict.get(zoneId)
                if objList is None:
                    # This parent has no objects in this zone before
                    # create a new entry for this zone and list this objId
                    parentZoneDict[zoneId] = [objId]
                else:
                    # Just add this objId to the existing list
                    objList.append(objId)

        def deleteObjectLocation(self, objId, parentId, zoneId):
            # Do not worry about null values
            if ((parentId is None) or (zoneId is None)):
                return
            parentZoneDict = self.__doHierarchy.get(parentId)
            assert(parentZoneDict is not None, "deleteObjectLocation: parentId: %s not found" % (parentId))
            objList = parentZoneDict.get(zoneId)
            assert(objList is not None, "deleteObjectLocation: zoneId: %s not found" % (zoneId))
            assert(objId in objList, "deleteObjectLocation: objId: %s not found" % (objId))
            if len(objList) == 1:
                # If this is the last obj in this zone, delete the entire entry
                del parentZoneDict[zoneId]
            else:
                # Just remove the object
                objList.remove(objId)
            

    def handleGenerateWithRequired(self, di):
        if wantOtpServer:        
            parentId = di.getUint32()
            zoneId = di.getUint32()
        # Get the class Id
        classId = di.getUint16()
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass                       
        dclass = self.dclassesByNumber[classId]
        dclass.startGenerate()
        # Create a new distributed object, and put it in the dictionary
        if wantOtpServer:
            distObj = self.generateWithRequiredFields(dclass, doId, di, parentId, zoneId)
        else:
            distObj = self.generateWithRequiredFields(dclass, doId, di)
        dclass.stopGenerate()

    def handleGenerateWithRequiredOther(self, di):
        if wantOtpServer:        
            parentId = di.getUint32()
            zoneId = di.getUint32()            
        # Get the class Id
        classId = di.getUint16()
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        dclass.startGenerate()
        # Create a new distributed object, and put it in the dictionary
        if wantOtpServer:
            distObj = self.generateWithRequiredOtherFields(dclass, doId, di, parentId, zoneId)
        else:
            distObj = self.generateWithRequiredOtherFields(dclass, doId, di)
        dclass.stopGenerate()

    def handleQuietZoneGenerateWithRequired(self, di):
        # Special handler for quiet zone generates -- we need to filter
        if wantOtpServer:        
            parentId = di.getUint32()
            zoneId = di.getUint32()                    
        # Get the class Id
        classId = di.getUint16()
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        dclass.startGenerate()
        # If the class is a neverDisable class (which implies uberzone) we
        # should go ahead and generate it even though we are in the quiet zone
        if not wantOtpServer:
            if dclass.getClassDef().neverDisable:
                # Create a new distributed object, and put it in the dictionary
                distObj = self.generateWithRequiredFields(dclass, doId, di)
        else:
            distObj = self.generateWithRequiredFields(dclass, doId, di, parentId, zoneId)
        dclass.stopGenerate()

    def handleQuietZoneGenerateWithRequiredOther(self, di):
        # Special handler for quiet zone generates -- we need to filter
        if wantOtpServer:        
            parentId = di.getUint32()
            zoneId = di.getUint32()                    
        # Get the class Id
        classId = di.getUint16()
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        # If the class is a neverDisable class (which implies uberzone) we
        # should go ahead and generate it even though we are in the quiet zone
        dclass.startGenerate()
        if not wantOtpServer:
            if dclass.getClassDef().neverDisable:
                # Create a new distributed object, and put it in the dictionary
                distObj = self.generateWithRequiredOtherFields(dclass, doId, di)
        else:
            distObj = self.generateWithRequiredOtherFields(dclass, doId, di, parentId, zoneId)
        dclass.stopGenerate()

    # wantOtpServer: remove the None defaults when we remove this config variable
    def generateWithRequiredFields(self, dclass, doId, di, parentId = None, zoneId = None):
        if self.doId2do.has_key(doId):
            # ...it is in our dictionary.
            # Just update it.
            distObj = self.doId2do[doId]
            assert(distObj.dclass == dclass)
            if wantOtpServer:
                distObj.setLocation(parentId, zoneId)
            distObj.generate()
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
        elif self.cache.contains(doId):
            # ...it is in the cache.
            # Pull it out of the cache:
            distObj = self.cache.retrieve(doId)
            assert(distObj.dclass == dclass)
            # put it in the dictionary:
            self.doId2do[doId] = distObj
            # and update it.
            if wantOtpServer:
                distObj.setLocation(parentId, zoneId)
            distObj.generate()
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
            if wantOtpServer:
                distObj.setLocation(parentId, zoneId)
            distObj.generateInit()  # Only called when constructed
            distObj.generate()
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
            if wantOtpServer:
                print "New DO:%s, dclass:%s"%(doId, dclass.getName())
        return distObj

    def generateGlobalObject(self , doId, dcname):
        # Look up the dclass
        dclass = self.dclassesByName[dcname]
        # Create a new distributed object, and put it in the dictionary
        #distObj = self.generateWithRequiredFields(dclass, doId, di)
        
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
        if wantOtpServer:
            # TODO: ROGER: where should we get parentId and zoneId?
            parentId = None
            zoneId = None
            distObj.setLocation(parentId, zoneId)
        distObj.generateInit()  # Only called when constructed
        distObj.generate()
        # updateRequiredFields calls announceGenerate
        return  distObj

    def generateWithRequiredOtherFields(self, dclass, doId, di, parentId = None, zoneId = None):
        if self.doId2do.has_key(doId):
            # ...it is in our dictionary.
            # Just update it.
            distObj = self.doId2do[doId]
            assert(distObj.dclass == dclass)
            if wantOtpServer:
                distObj.setLocation(parentId, zoneId)
            distObj.generate()
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        elif self.cache.contains(doId):
            # ...it is in the cache.
            # Pull it out of the cache:
            distObj = self.cache.retrieve(doId)
            assert(distObj.dclass == dclass)
            # put it in the dictionary:
            self.doId2do[doId] = distObj
            # and update it.
            if wantOtpServer:
                distObj.setLocation(parentId, zoneId)
            distObj.generate()
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
            if wantOtpServer:
                distObj.setLocation(parentId, zoneId)
            distObj.generateInit()  # Only called when constructed
            distObj.generate()
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        return distObj


    def handleDisable(self, di):
        # Get the DO Id
        doId = di.getUint32()
        # disable it.
        self.disableDoId(doId)

    def disableDoId(self, doId):
         # Make sure the object exists
        if self.doId2do.has_key(doId):
            # Look up the object
            distObj = self.doId2do[doId]
            # remove the object from the dictionary
            del(self.doId2do[doId])

            # Only cache the object if it is a "cacheable" type
            # object; this way we don't clutter up the caches with
            # trivial objects that don't benefit from caching.
            if distObj.getCacheable():
                self.cache.cache(distObj)
            else:
                distObj.deleteOrDelay()
        else:
            ClientRepository.notify.warning(
                "Disable failed. DistObj "
                + str(doId) +
                " is not in dictionary")

    def handleDelete(self, di):
        # Get the DO Id
        doId = di.getUint32()
        self.deleteObject(doId)

    def deleteObject(self, doId):
        """
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
        if self.doId2do.has_key(doId):
            # If it is in the dictionary, remove it.
            obj = self.doId2do[doId]
            # Remove it from the dictionary
            del(self.doId2do[doId])
            # Disable, announce, and delete the object itself...
            # unless delayDelete is on...
            obj.deleteOrDelay()
        elif self.cache.contains(doId):
            # If it is in the cache, remove it.
            self.cache.delete(doId)
        else:
            # Otherwise, ignore it
            ClientRepository.notify.warning(
                "Asked to delete non-existent DistObj " + str(doId))

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
        if (do != None):
            # Let the dclass finish the job
            do.dclass.receiveUpdate(do, di)
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

    def handleSystemMessage(self, di):
        # Got a system message from the server.
        message = di.getString()
        self.notify.info('Message from server: %s' % (message))
        return message

    def handleUnexpectedMsgType(self, msgType, di):        
        if msgType == CLIENT_CREATE_OBJECT_REQUIRED:
            self.handleGenerateWithRequired(di)
        elif msgType == CLIENT_CREATE_OBJECT_REQUIRED_OTHER:
            self.handleGenerateWithRequiredOther(di)
        elif msgType == CLIENT_OBJECT_UPDATE_FIELD:
            self.handleUpdateField(di)
        elif msgType == CLIENT_OBJECT_DISABLE_RESP:
            self.handleDisable(di)
        elif msgType == CLIENT_OBJECT_DELETE_RESP:
            self.handleDelete(di)
        elif msgType == CLIENT_GO_GET_LOST:
            self.handleGoGetLost(di)
        elif msgType == CLIENT_HEARTBEAT:
            self.handleServerHeartbeat(di)
        elif msgType == CLIENT_SYSTEM_MESSAGE:
            self.handleSystemMessage(di)
        elif wantOtpServer and msgType == CLIENT_CREATE_OBJECT_REQUIRED:
            self.handleGenerateWithRequired(di)
        elif wantOtpServer and msgType == CLIENT_CREATE_OBJECT_REQUIRED_OTHER:
            self.handleGenerateWithRequiredOther(di)
        elif wantOtpServer and msgType == CLIENT_DONE_SET_ZONE_RESP:
            self.handleSetZoneDone()                    
        elif  wantOtpServer and msgType == CLIENT_OBJECT_LOCATION:        
            self.handleObjectLocation(di)
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
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_SET_SHARD)
        # Add shard id
        datagram.addUint32(shardId)
        # send the message
        self.send(datagram)

    if wantOtpServer:
        def sendSetZoneMsg(self, zoneId, visibleZoneList=None, parent=None):
            datagram = PyDatagram()
            # Add message type
            datagram.addUint16(CLIENT_SET_ZONE)       
            # Add Parent
            if parent is not None:
                datagram.addUint32(zoneId)
            else:
                datagram.addUint32(base.localAvatar.defaultShard)                                    
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
    else:
        def sendSetZoneMsg(self, zoneId, visibleZoneList=None):
            datagram = PyDatagram()
            # Add message type
            datagram.addUint16(CLIENT_SET_ZONE)       
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

    def handleDatagram(self, di):
        if self.notify.getDebug():
            print "ClientRepository received datagram:"
            di.getDatagram().dumpHex(ostream)

        msgType = self.getMsgType()
        
        # watch for setZoneDones
        if msgType == CLIENT_DONE_SET_ZONE_RESP:
            self.handleSetZoneDone()

        if self.handler == None:
            self.handleUnexpectedMsgType(msgType, di)
        else:
            self.handler(msgType, di)
            
        # If we're processing a lot of datagrams within one frame, we
        # may forget to send heartbeats.  Keep them coming!
        self.considerHeartbeat()
    
    if wantOtpServer:
        # interest managment 
        def addInterest(self, parentId, zoneId, description):
            """
            Part of the new otp-server code.
            """
            self._interestIdAssign += 1
            self._interests[self._interestIdAssign] = description
            contextId = self._interestIdAssign
            self._sendAddInterest(contextId, parentId, zoneId)
            assert self.printInterests()
            return contextId
            
        def removeInterest(self,  contextId):        
            """
            Part of the new otp-server code.
            """
            answer = 0
            if  self._interests.has_key(contextId):
                self._sendRemoveInterest(contextId)
                del self._interests[contextId]
                answer = 1                                
            assert self.printInterests()            
            return answer

        def alterInterest(self, contextId, parentId, zoneId, description):        
            """
            Part of the new otp-server code.        
                Removes old and adds new.. 
            """
            answer = 0
            if  self._interests.has_key(contextId):
                self._interests[contextId] = description
                self._sendAlterInterest(contextId, parentId, zoneId)
                answer = 1
            assert self.printInterests()            
            return answer
            
        if __debug__:
            def printInterests(self):
                """
                Part of the new otp-server code.        
                """
                print "*********************** Interest Sets **************"
                for i in self._interests.keys():
                     print "Interest ID:%s, Description=%s"%(i, self._interests[i])
                print "****************************************************"
                return 1 # for assert()
        
        def _sendAddInterest(self, contextId, parentId, zoneId):
            """
            Part of the new otp-server code.

            contextId is a client-side created number that refers to
                    a set of interests.  The same contextId number doesn't
                    necessarily have any relationship to the same contextId
                    on another client.
            """
            datagram = PyDatagram()
            # Add message type
            datagram.addUint16(CLIENT_ADD_INTEREST)
            datagram.addUint16(contextId)
            datagram.addUint32(parentId)
            datagram.addUint32(zoneId)
            self.send(datagram)

        def _sendAlterInterest(self, contextId, parentId, zoneId):
            """
            Part of the new otp-server code.

            contextId is a client-side created number that refers to
                    a set of interests.  The same contextId number doesn't
                    necessarily have any relationship to the same contextId
                    on another client.
            """
            datagram = PyDatagram()
            # Add message type
            datagram.addUint16(CLIENT_ALTER_INTEREST)
            datagram.addUint16(contextId)
            datagram.addUint32(parentId)
            datagram.addUint32(zoneId)
            self.send(datagram)

        def _sendRemoveInterest(self, contextId):
            """
            Part of the new otp-server code.

            contextId is a client-side created number that refers to
                    a set of interests.  The same contextId number doesn't
                    necessarily have any relationship to the same contextId
                    on another client.
            """
            datagram = PyDatagram()
            # Add message type
            datagram.addUint16(CLIENT_REMOVE_INTEREST)
            datagram.addUint16(contextId)
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
        
    def sendUpdate(self, do, fieldName, args, sendToId = None):
        dg = do.dclass.clientFormatUpdate(fieldName, sendToId or do.doId, args)
        self.send(dg)

    def replaceMethod(self, oldMethod, newFunction):
        return 0

    def getAllOfType(self, type):
        # Returns a list of all DistributedObjects in the repository
        # of a particular type.
        result = []
        for obj in self.doId2do.values():
            if isinstance(obj, type):
                result.append(obj)
        return result
