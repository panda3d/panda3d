"""ClientRepository module: contains the ClientRepository class"""

from PandaModules import *
from TaskManagerGlobal import *
from MsgTypes import *
from ShowBaseGlobal import *
import Task
import DirectNotifyGlobal
import ClientDistClass
import CRCache
import Datagram
# The repository must import all known types of Distributed Objects
#import DistributedObject
#import DistributedToon
import DirectObject

class ClientRepository(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepository")

    def __init__(self, dcFileName):
        self.number2cdc={}
        self.name2cdc={}
        self.doId2do={}
        self.doId2cdc={}
        self.parseDcFile(dcFileName)
        self.cache=CRCache.CRCache()
        return None

    def parseDcFile(self, dcFileName):
        self.dcFile = DCFile()
        fname = Filename(dcFileName)
        readResult = self.dcFile.read(fname)
        if not readResult:
            self.notify.error("Could not read dcfile: " + dcFileName)
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

    def connect(self, serverName, serverPort):
        self.qcm=QueuedConnectionManager()
        gameServerTimeoutMs = base.config.GetInt("game-server-timeout-ms",
                                                 20000)
        # A big old 20 second timeout.
        self.tcpConn = self.qcm.openTCPClientConnection(
            serverName, serverPort, gameServerTimeoutMs)
        # Test for bad connection
        if self.tcpConn == None:
            return None
        else:
            self.qcr=QueuedConnectionReader(self.qcm, 0)
            self.qcr.addConnection(self.tcpConn)
            self.cw=ConnectionWriter(self.qcm, 0)
            self.startReaderPollTask()
            return self.tcpConn

    def startReaderPollTask(self):
        task = Task.Task(self.readerPollUntilEmpty)
        taskMgr.spawnTaskNamed(task, "readerPollTask")
        return None

    def readerPollUntilEmpty(self, task):
        while self.readerPollOnce():
            pass
        return Task.cont

    def readerPollOnce(self):
        self.ensureValidConnection()
        availGetVal = self.qcr.dataAvailable()
        if availGetVal:
            #print "Client: Incoming message!"
            datagram = NetDatagram()
            readRetVal = self.qcr.getData(datagram)
            if readRetVal:
                self.handleDatagram(datagram)
            else:
                ClientRepository.notify.warning("getData returned false")
        return availGetVal

    def ensureValidConnection(self):
        # Was the connection reset?
        if self.qcm.resetConnectionAvailable():
            resetConnectionPointer = PointerToConnection()
            if self.qcm.getResetConnection(resetConnectionPointer):
                self.fsm.request("noConnection")
        return None

    def handleDatagram(self, datagram):
        # This class is meant to be pure virtual, and any classes that
        # inherit from it need to make their own handleDatagram method
        pass

    def handleGenerateWithRequired(self, di):
        # Get the class Id
        classId = di.getArg(STUint16);
        # Get the DO Id
        doId = di.getArg(STUint32)
        # Look up the cdc
        cdc = self.number2cdc[classId]
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredFields(cdc, doId, di)
        # Call "generate" for the dist obj
        #distObj.generate()
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
        # Call "generate" for the distObj
        #distObj.generate()
        return None

    def generateWithRequiredFields(self, cdc, doId, di):
        # Is it in our dictionary? 
        if self.doId2do.has_key(doId):
            # If so, just update it.
            distObj = self.doId2do[doId]
	    distObj.generate()
            distObj.updateRequiredFields(cdc, di)

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

        # If it is not in the dictionary or the cache, then...
        else:
            # Construct a new one
            distObj = cdc.constructor(self)
            # Assign it an Id
            distObj.doId = doId
            # Update the required fields
            distObj.generateInit()  # Only called when constructed
	    distObj.generate()
            distObj.updateRequiredFields(cdc, di)
            # Put the new do in both dictionaries
            self.doId2do[doId] = distObj
            self.doId2cdc[doId] = cdc
            
        return distObj

    def generateWithRequiredOtherFields(self, cdc, doId, di):
        # Is it in our dictionary? 
        if self.doId2do.has_key(doId):
            # If so, just update it.
            distObj = self.doId2do[doId]
	    distObj.generate()
            distObj.updateRequiredOtherFields(cdc, di)

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
            # Put the new do in both dictionaries
            # self.doId2do[doId] = distObj
            # self.doId2cdc[doId] = cdc
            
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
            # cache the object
            self.cache.cache(distObj)
        else:
            ClientRepository.notify.warning("Disable failed. DistObj " +
                                            str(doId) +
                                            " is not in dictionary")
        return None

    def handleDelete(self, di):
        # Get the DO Id
        doId = di.getArg(STUint32)
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
        # Find the DO
        assert(self.doId2do.has_key(doId))
        do = self.doId2do[doId]
        # Find the cdc
        assert(self.doId2cdc.has_key(doId))
        cdc = self.doId2cdc[doId]
        # Let the cdc finish the job
        cdc.updateField(do, di)
        return None

    def handleUnexpectedMsgType(self, msgType, di):
        ClientRepository.notify.warning(
            "Ignoring unexpected message type: " +
            str(msgType) +
            " in state: " +
            self.fsm.getCurrentState().getName())
        return None

    def sendSetShardMsg(self, shardId):
        datagram = Datagram.Datagram()
        # Add message type
        datagram.addUint16(CLIENT_SET_SHARD)
        # Add shard id
        datagram.addUint32(shardId)
        # send the message
        self.send(datagram)
        return None

    def sendSetZoneMsg(self, zoneId):
        datagram = Datagram.Datagram()
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
        assert(self.doId2cdc.has_key(doId))
        cdc = self.doId2cdc[doId]
        # Let the cdc finish the job
        cdc.sendUpdate(self, do, fieldName, args, sendToId)

    def send(self, datagram):
        #if self.notify.getDebug():
        #    print "ClientRepository sending datagram:"
        #    datagram.dumpHex(ostream)

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
