"""ClientRepository module: contains the ClientRepository class"""

from PandaModules import *
from TaskManagerGlobal import *
from MsgTypes import *
import Task
import DirectNotifyGlobal
import ClientDistClass
# The repository must import all known types of Distributed Objects
import DistributedObject
import DistributedToon
import DirectObject

class ClientRepository(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepository")

    def __init__(self, dcFileName, AIClientFlag=0):
        self.AIClientFlag=AIClientFlag
        self.number2cdc={}
        self.name2cdc={}
        self.doId2do={}
        self.doId2cdc={}
        self.parseDcFile(dcFileName)
        return None

    def parseDcFile(self, dcFileName):
        self.dcFile = DCFile()
        self.dcFile.read(dcFileName)
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
        self.tcpConn = self.qcm.openTCPClientConnection(
            serverName, serverPort, 1000)
        self.qcr=QueuedConnectionReader(self.qcm, 0)
        self.qcr.addConnection(self.tcpConn)
        self.cw=ConnectionWriter(self.qcm, 0)
        self.startReaderPollTask()

    def startReaderPollTask(self):
        task = Task.Task(self.readerPollUntilEmpty)
        taskMgr.spawnTaskNamed(task, "readerPollTask")
        return None

    def readerPollUntilEmpty(self, task):
        while self.readerPollOnce():
            pass
        return Task.cont

    def readerPollOnce(self):
        availGetVal = self.qcr.dataAvailable()
        if availGetVal:
            print "Client: Incoming message!"
            datagram = NetDatagram()
            readRetVal = self.qcr.getData(datagram)
            if readRetVal:
                self.handleDatagram(datagram)
            else:
                ClientRepository.notify.warning("getData returned false")
        return availGetVal

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
        distObj = self.generateWithRequiredFields(cdc,
                                                  eval(cdc.name + \
                                                       "." + \
                                                       cdc.name),
                                                  doId, di)
        return None

    def generateWithRequiredFields(self, cdc, constructor, doId, di):
        # Someday, this function will look in a cache of old distributed
        # objects to see if this object is in there, and pull it
        # out if necessary. For now, we'll just check to see if
        # it is in the standard dictionary, and ignore this update
        # if it is there. The right thing to do would be to update
        # all the required fields, but that will come later, too.

        if self.doId2do.has_key(doId):
            ClientRepository.notify.warning("doId: " +
                                            str(doId) +
                                            " was generated again")
            distObj = self.doId2do[doId]
            distObj.updateRequiredFields(cdc, di)
        else:
            # Construct a new one
            distObj = constructor(self)
            # Assign it an Id
            distObj.doId = doId
            # Update the required fields
            distObj.updateRequiredFields(cdc, di)
            # Put the new do in both dictionaries
            self.doId2do[doId] = distObj
            self.doId2cdc[doId] = cdc
            
        return distObj

    def handleUpdateField(self, di):
        # Get the DO Id
        doId = di.getArg(STUint32)
        print("Updating " + str(doId))
        # Find the DO
        assert(self.doId2do.has_key(doId))
        do = self.doId2do[doId]
        # Find the cdc
        assert(self.doId2cdc.has_key(doId))
        cdc = self.doId2cdc[doId]
        # Let the cdc finish the job
        cdc.updateField(do, di)
        
    def sendUpdate(self, do, fieldName, args):
        # Get the DO id
        doId = do.doId
        # Get the cdc
        assert(self.doId2cdc.has_key(doId))
        cdc = self.doId2cdc[doId]
        # Let the cdc finish the job
        cdc.sendUpdate(self, do, fieldName, args)

    def send(self, datagram):
        self.cw.send(datagram, self.tcpConn)
        return None
