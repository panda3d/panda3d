"""ClientRepository module: contains the ClientRepository class"""

from PandaModules import *
from TaskManagerGlobal import *
import Task
import DirectNotifyGlobal
import ClientDistClass

class ClientRepository:
    defaultServerPort = 5150
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

    def connect(self, serverName="localhost",
                serverPort=defaultServerPort):
        self.qcm=QueuedConnectionManager()
        self.tcpConn = self.qcm.openTCPClientConnection(
            serverName, serverPort, 1000)
        self.qcr=QueuedConnectionReader(self.qcm, 0)
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
            datagram = NetDatagram()
            readRetVal = self.qcr.getData(datagram)
            if readRetVal:
                self.handleDatagram(datagram)
            else:
                ClientRepository.notify.warning("getData returned false")
        return availGetVal

    def handleDatagram(datagram):
        di = DatagramIterator(datagram)
        msgType = di.getArg(ST_uint16)

        if msgType == ALL_OBJECT_GENERATE_WITH_REQUIRED:
            self.handleGenerateWithRequired(di)
        elif msgType == ALL_OBJECT_UPDATE_FIELD:
            self.handleUpdateField(di)
        else:
            ClientRepository.notify.warning("We don't handle type: "
                                            + str(msgType))
        return None

    def handleGenerateWithRequired(di):
        # Get the class Id
        classId = di.getArg(ST_uint8);
        # Get the DO Id
        doId = di.getArg(ST_uint32)
        # Get the echo context
        echoContext = di.getArg(ST_uint32);
        # Look up the cdc
        cdc = self.number2cdc[classId]
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredFields(cdc, doId, di)
        return None

    def generateWithRequiredFields(cdc, doId, di):
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
            distObj = self.doId2do(doId)
        else:
            distObj = \
                    eval(cdc.name).generateWithRequiredFields(doId, di)
            # Put the new do in both dictionaries
            self.doId2do[doId] = distObj
            self.doId2cdc[doId] = cdc
            
        return distObj

    def handleUpdateField(di):
        # Get the DO Id
        doId = di.getArg(ST_uint32)
        # Find the DO
        assert(self.doId2do.has_key(doId))
        do = self.doId2do(doId)
        # Find the cdc
        assert(self.doId2cdc.has_key(doId))
        cdc = self.doId2cdc[doId]
        # Let the cdc finish the job
        cdc.updateField(do, di)
        
    def sendLoginMsg(self):
        datagram = Datagram()
        # Add message type
        datagram.addUint16(1)
        # Add swid
        datagram.addString("1234567890123456789012345678901234")
        # Add IP Address
        datagram.addUint32(0)
        # Add UDP port
        datagram.addUint16(5150)
        # Send the message
        self.cw.send(datagram, self.tcpConn)

    def sendUpdate(self, do, fieldName, args):
        # Get the DO id
        doId = do.doId
        # Get the cdc
        assert(self.doId2cdc.has_key(doId))
        cdc = self.doId2cdc[doId]
        # Let the cdc finish the job
        cdc.sendUpdate(do, fieldName, args)
        
