"""ClusterMsgs module: Message types for Cluster rendering"""

CLUSTER_NOTHING    = -1
CLUSTER_CAM_OFFSET = 1
CLUSTER_POS_UPDATE = 2
CLUSTER_SWAP_READY = 3
CLUSTER_SWAP_NOW   = 4

#Port number for cluster rendering
CLUSTER_PORT = 1970

from ShowBaseGlobal import *
from PandaModules import *
from TaskManagerGlobal import *
import Task
import DirectNotifyGlobal
import Datagram
import time

class MsgHandler:
    """MsgHandler: wrapper for PC clusters/multi-piping networking"""
    def __init__(self,packetStart, notify):
        """packetStart can be used to distinguish which MsgHandler sends a
        given packet."""
        self.packetNumber = packetStart
        self.notify = notify

    def nonBlockingRead(self,qcr):
        availGetVal = qcr.dataAvailable()
        if availGetVal:
            datagram = NetDatagram()
            readRetVal = qcr.getData(datagram)
            if readRetVal:
                dgi = DatagramIterator(datagram)
                number = dgi.getUint32()
                type = dgi.getUint8()
                self.notify.debug( ("Packet %d type %d recieved" % (number,type)) )    
            else:
                self.notify.warning("getData returned false")
        else:
            type = CLUSTER_NOTHING
            dgi = None

        return (type,dgi)

    def readHeader(self,datagram):
        dgi = DatagramIterator(datagram)
        number = dgi.getUint32()
        type = dgi.getUint8()
        self.notify.info( ("Packet %d type %d recieved" % (number,type)) )
        return (type,dgi)        

    def blockingRead(self,qcr):
        availGetVal = 0
        while not availGetVal:
            availGetVal = qcr.dataAvailable()
            if not availGetVal:
                time.sleep(0.002)
                type = CLUSTER_NOTHING
        datagram = NetDatagram()
        readRetVal = qcr.getData(datagram)
        if not readRetVal:
            self.notify.warning("getData returned false")
                
        return datagram

    def makeCamOffsetDatagram(self,xyz,hpr):
        datagram = Datagram.Datagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_CAM_OFFSET)
        datagram.addFloat32(xyz[0])
        datagram.addFloat32(xyz[1])
        datagram.addFloat32(xyz[2])
        datagram.addFloat32(hpr[0])
        datagram.addFloat32(hpr[1])
        datagram.addFloat32(hpr[2])
        return datagram

    def makeMoveCamDatagram(self,xyz,hpr):
        datagram = Datagram.Datagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_POS_UPDATE)
        datagram.addFloat32(xyz[0])
        datagram.addFloat32(xyz[1])
        datagram.addFloat32(xyz[2])
        datagram.addFloat32(hpr[0])
        datagram.addFloat32(hpr[1])
        datagram.addFloat32(hpr[2])
        return datagram

    def makeSwapNowDatagram(self):
        datagram = Datagram.Datagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_SWAP_NOW)
        return datagram
         
    def makeSwapReadyDatagram(self):
        datagram = Datagram.Datagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_SWAP_READY)
        return datagram
        








