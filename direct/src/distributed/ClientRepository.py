"""ClientRepository module: contains the ClientRepository class"""

from PandaModules import *
from TaskManagerGlobal import *
import Task
import DirectNotifyGlobal

class ClientRepository:
    defaultServerPort = 5150
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepository")

    def __init__(self, dcFileName):
        self.number2cdc={}
        self.name2cdc={}
        self.parseDcFile(dcFileName)
        return None

    def parseDcFile(dcFileName):
        dcFile = DCFile()
        dcFile.read(dcFileName)
        return self.parseDcClasses(dcFile)

    def parseDcClasses(dcFile):
        numClasses = dcFile.get_num_classes()
        for i in range(0, numClasses):
            # Create a clientDistClass from the dcClass
            clientDistClass = self.parseClass(dcFile.getClass())
            # List the cdc in the number and name dictionaries
            self.number2cdc[clientDistClass.getNumber()]=clientDistClass
            self.name2cdc[clientDistClass.getName()]=clientDistClass
        return None

    def parseClass(dcClass):
        

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
        print 'Got it!'
        return None

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

        
        
