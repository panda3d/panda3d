"""ClusterMsgs module: Message types for Cluster rendering"""

# This module is intended to supply routines and dataformats common to
# both ClusterClient and ClusterServer.

from PandaModules import *
import Datagram
import time

#these are the types of messages that are currently supported.
CLUSTER_NONE    = 0
CLUSTER_CAM_OFFSET = 1
CLUSTER_CAM_FRUSTUM = 2
CLUSTER_CAM_MOVEMENT = 3
CLUSTER_SWAP_READY = 4
CLUSTER_SWAP_NOW   = 5
CLUSTER_COMMAND_STRING = 6
CLUSTER_SELECTED_MOVEMENT = 7
CLUSTER_EXIT = 100

#Port number for cluster rendering
CLUSTER_PORT = 1970

class ClusterMsgHandler:
    """ClusterMsgHandler: wrapper for PC clusters/multi-piping networking"""
    def __init__(self,packetStart, notify):
        # packetStart can be used to distinguish which ClusterMsgHandler
        # sends a given packet.
        self.packetNumber = packetStart
        self.notify = notify

    def nonBlockingRead(self,qcr):
        """
        Return a datagram iterator and type if data is available on the
        queued connection reader
        """
        if qcr.dataAvailable():
            datagram = NetDatagram()
            if qcr.getData(datagram):
                (dgi, type) = self.readHeader(datagram)
            else:
                dgi = None
                type = CLUSTER_NONE
                self.notify.warning("getData returned false")
        else:
            dgi = None
            type = CLUSTER_NONE
        # Note, return datagram to keep a handle on the data
        return (datagram, dgi,type)

    def blockingRead(self,qcr):
        """
        Block until data is available on the queued connection reader.
        Returns a datagram iterator and type
        """
        while not qcr.dataAvailable():
            # The following may not be necessary.
            # I just wanted some
            # time given to the operating system while
            # busy waiting.
            time.sleep(0.002)
        # Data is available, create a datagram iterator
        datagram = NetDatagram()
        if qcr.getData(datagram):
            (dgi, type) = self.readHeader(datagram)
        else:
            (dgi, type) = (None, CLUSTER_NONE)
            self.notify.warning("getData returned false")
        # Note, return datagram to keep a handle on the data
        return (datagram, dgi, type)

    def readHeader(self,datagram):
        dgi = DatagramIterator(datagram)
        number = dgi.getUint32()
        type = dgi.getUint8()
        self.notify.debug("Packet %d type %d received" % (number,type))
        return (dgi,type)        

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

    def parseCamOffsetDatagram(self, dgi):
        x=dgi.getFloat32()
        y=dgi.getFloat32()
        z=dgi.getFloat32()
        h=dgi.getFloat32()
        p=dgi.getFloat32()
        r=dgi.getFloat32()
        self.notify.debug('new offset=%f %f %f  %f %f %f' % (x,y,z,h,p,r))
        return (x,y,z,h,p,r)

    def makeCamFrustumDatagram(self,focalLength, filmSize, filmOffset):
        datagram = Datagram.Datagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_CAM_FRUSTUM)
        datagram.addFloat32(focalLength)
        datagram.addFloat32(filmSize[0])
        datagram.addFloat32(filmSize[1])
        datagram.addFloat32(filmOffset[0])
        datagram.addFloat32(filmOffset[1])
        return datagram

    def parseCamFrustumDatagram(self, dgi):
        focalLength = dgi.getFloat32()
        filmSize    = (dgi.getFloat32(), dgi.getFloat32())
        filmOffset  = (dgi.getFloat32(),dgi.getFloat32())
        self.notify.debug('fl, fs, fo=%f, (%f, %f), (%f, %f)' %
                          (focalLength, filmSize[0], filmSize[1],
                           filmOffset[0], filmOffset[1]))
        return (focalLength, filmSize, filmOffset)

    def makeCamMovementDatagram(self,xyz,hpr):
        datagram = Datagram.Datagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_CAM_MOVEMENT)
        datagram.addFloat32(xyz[0])
        datagram.addFloat32(xyz[1])
        datagram.addFloat32(xyz[2])
        datagram.addFloat32(hpr[0])
        datagram.addFloat32(hpr[1])
        datagram.addFloat32(hpr[2])
        return datagram

    def parseCamMovementDatagram(self, dgi):
        x=dgi.getFloat32()
        y=dgi.getFloat32()
        z=dgi.getFloat32()
        h=dgi.getFloat32()
        p=dgi.getFloat32()
        r=dgi.getFloat32()
        self.notify.debug(('  new position=%f %f %f  %f %f %f' %
                           (x,y,z,h,p,r)))
        return (x,y,z,h,p,r)

    def makeSelectedMovementDatagram(self,xyz,hpr):
        datagram = Datagram.Datagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_SELECTED_MOVEMENT)
        datagram.addFloat32(xyz[0])
        datagram.addFloat32(xyz[1])
        datagram.addFloat32(xyz[2])
        datagram.addFloat32(hpr[0])
        datagram.addFloat32(hpr[1])
        datagram.addFloat32(hpr[2])
        return datagram

    def parseSelectedMovementDatagram(self, dgi):
        x=dgi.getFloat32()
        y=dgi.getFloat32()
        z=dgi.getFloat32()
        h=dgi.getFloat32()
        p=dgi.getFloat32()
        r=dgi.getFloat32()
        self.notify.debug('  new position=%f %f %f  %f %f %f' %
                          (x,y,z,h,p,r))
        return (x,y,z,h,p,r)

    def makeCommandStringDatagram(self, commandString):
        datagram = Datagram.Datagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_COMMAND_STRING)
        datagram.addString(commandString)
        return datagram

    def parseCommandStringDatagram(self, dgi):
        command = dgi.getString()
        return command

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

    def makeExitDatagram(self):
        datagram = Datagram.Datagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_EXIT)
        return datagram
        








