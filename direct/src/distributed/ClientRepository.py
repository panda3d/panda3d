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
        
        self.doId2do={}
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

    def handleGenerateWithRequired(self, di):
        # Get the class Id
        classId = di.getUint16();
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        dclass.startGenerate()
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredFields(dclass, doId, di)
        dclass.stopGenerate()

    def handleGenerateWithRequiredOther(self, di):
        # Get the class Id
        classId = di.getUint16();
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        dclass.startGenerate()
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredOtherFields(dclass, doId, di)
        dclass.stopGenerate()

    def handleQuietZoneGenerateWithRequired(self, di):
        # Special handler for quiet zone generates -- we need to filter
        # Get the class Id
        classId = di.getUint16();
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        dclass.startGenerate()
        # If the class is a neverDisable class (which implies uberzone) we
        # should go ahead and generate it even though we are in the quiet zone
        if dclass.getClassDef().neverDisable:
            # Create a new distributed object, and put it in the dictionary
            distObj = self.generateWithRequiredFields(dclass, doId, di)
        dclass.stopGenerate()

    def handleQuietZoneGenerateWithRequiredOther(self, di):
        # Special handler for quiet zone generates -- we need to filter
        # Get the class Id
        classId = di.getUint16();
        # Get the DO Id
        doId = di.getUint32()
        # Look up the dclass
        dclass = self.dclassesByNumber[classId]
        # If the class is a neverDisable class (which implies uberzone) we
        # should go ahead and generate it even though we are in the quiet zone
        dclass.startGenerate()
        if dclass.getClassDef().neverDisable:
            # Create a new distributed object, and put it in the dictionary
            distObj = self.generateWithRequiredOtherFields(dclass, doId, di)
        dclass.stopGenerate()

    def generateWithRequiredFields(self, dclass, doId, di):
        if self.doId2do.has_key(doId):
            # ...it is in our dictionary.
            # Just update it.
            distObj = self.doId2do[doId]
            assert(distObj.dclass == dclass)
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
            distObj.generateInit()  # Only called when constructed
            distObj.generate()
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
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
        distObj.generateInit()  # Only called when constructed
        distObj.generate()
        # updateRequiredFields calls announceGenerate
        return  distObj       

    def generateWithRequiredOtherFields(self, dclass, doId, di):
        if self.doId2do.has_key(doId):
            # ...it is in our dictionary.
            # Just update it.
            distObj = self.doId2do[doId]
            assert(distObj.dclass == dclass)
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
        # If it is in the dictionary, remove it.
        if self.doId2do.has_key(doId):
            obj = self.doId2do[doId]
            # Remove it from the dictionary
            del(self.doId2do[doId])
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
        if msgType == CLIENT_GO_GET_LOST:
            self.handleGoGetLost(di)
        elif msgType == CLIENT_HEARTBEAT:
            self.handleServerHeartbeat(di)
        elif msgType == CLIENT_SYSTEM_MESSAGE:
            self.handleSystemMessage(di)
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

    def gridZoneCenter(self, x, y, zoneBase=0, resolution=500):
        """
        x is a float in the range 0.0 to 1.0
        y is a float in the range 0.0 to 1.0
        resolution is the number of cells on each axsis.
        """
        if x < 0.0 or x > 1.0 or y < 0.0 or y > 1.0:
            return None
        resolution=int(resolution)
        print "resolution", resolution,
        xCell=min(int(x*resolution), resolution-1)
        yCell=min(int(y*resolution), resolution-1)
        cell=yCell*resolution+xCell
        print "cell", cell,
        zone=zoneBase+cell
        print "zone", zone
        assert zone >= zoneBase and zone < zoneBase+resolution*resolution
        return zone

    def gridZoneList(self, x, y, zoneBase=0, zoneList=[], resolution=500):
        """
        x is a float in the range 0.0 to 1.0
        y is a float in the range 0.0 to 1.0
        resolution is the number of cells on each axsis.
        returns a list of zone ids.
        
        Create a box of cell numbers, while clipping
        to the edges of the set of cells.
        """
        if x < 0.0 or x > 1.0 or y < 0.0 or y > 1.0:
            return None
        resolution=int(resolution)
        print "resolution", resolution,
        xCell=min(int(x*resolution), resolution-1)
        yCell=min(int(y*resolution), resolution-1)
        cell=yCell*resolution+xCell
        print "cell", cell,
        zone=zoneBase+cell
        print "zone", zone

        zone=zone-2*resolution
        endZone=zone+5*resolution
        yCell=yCell-2
        while zone < endZone:
            if yCell >= 0 and yCell < resolution:
                if xCell > 1:
                    zoneList.append(zone-2)
                    zoneList.append(zone-1)
                elif xCell > 0:
                    zoneList.append(zone-1)
                r.append(zone)
                if xCell < resolution-2:
                    zoneList.append(zone+1)
                    zoneList.append(zone+2)
                elif xCell < resolution-1:
                    zoneList.append(zone+1)
            yCell+=1
            zone+=resolution
        return zoneList

    def gridZone(self, zoneId, pos):
        """
        zoneId is an integer.
        pos is a Vec3 with x,y,z float values.
        
        Figure out which zones in the multi-zone heirarchy
        the avatar is currently.  Use sendSetZoneMsg() to
        send the info to the server.
        
        So, one possibility is to use a 3x3 grid and have
        each cell be the movement distance in the load time
        plus the vision distance.
        
        Another possibility is to use a 5x5 grid and have
        each cell be the greater of the movement distance
        or the vision distance.
        
        Yet another possibility is to use a nxn grid inside
        of a mxm grid.  The nxn grid is used to add cells
        to the visible set, while the mxm grid is used to
        retain old cells a little longer.  This avoids 
        jitter (i.e. rapid generation and deletion of zones
        as the avatar runs down the line separating two cells).
        Also, the mxm grid is not neccessarily
        full (and is likely not to be full).  So, cell in
        the nxn grid are added and cells outside of the
        mxm grid are removed.
        
        When choosing a method, the generation (inlcluding
        loading and setup) time should be compared to the
        cost of having extra distributed objects.
        
        The third option optimizes for expensive generation,
        while the second option optimizes for epensive
        maintenance.
        
        o o o o o o o 
        o o o o o o o 
        o o[k k o]o o 
        o o|k a a|o o 
        o o[o a a]o o 
        o o o o o o o 
        o o o o o o o 
        """
        # The teirs are offset from each other to spread the
        # generates.
        width=2000.0
        height=2000.0
        teirBase=1000
        # The teirBase is a teir unto itself, all avatars in
        # in the given teir system are also in the main teir:
        r=[teirBase]
        teirBase+=1
        
        x=pos.getX()/width
        y=pos.getY()/height
        getGridZones(x, y, teirBase, r, 500)
        
        return r

        #*#
        # The teirs are offset from each other to spread the
        # generates.
        width=2000.0
        height=2000.0
        teirs=[20, 100, 500]
        teirOffsets=[0.33, 0.5, 0.0]
        teirBase=1000
        # The teirBase is a teir unto itself, all avatars in
        # in the given teir system are also in the main teir:
        r=[teirBase]
        teirBase+=1
        
        x=pos.getX()
        y=pos.getY()
        for i, offset in zip(teirs, teirOffsets):
            print "teirBase", teirBase,
            xCell=min(int((x-width/i*offset)/i), i-1)
            yCell=min(int((y-height/i*offset)/i), i-1)
            print "xCell", xCell, "yCell", yCell,
            cell=yCell*i+xCell
            print "cell", cell,
            zone=teirBase+cell
            print "zone", zone
            #for zone in range(teirBase+cell, teirBase+cell+5*i, i):
            zone=zone-2*i
            endZone=teirBase+cell+5*i
            yCell=yCell-2
            while zone < endZone:
                if yCell >= 0 and yCell < i:
                    if xCell > 1:
                        r.append(zone-2)
                        r.append(zone-1)
                    elif xCell > 0:
                        r.append(zone-1)
                    r.append(zone)
                    if xCell < i-2:
                        r.append(zone+1)
                        r.append(zone+2)
                    elif xCell < i-1:
                        r.append(zone+1)
                yCell+=1
                zone+=i
            print ""
            teirBase+=i*i
        print "teirBase", teirBase
        
        return r

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
