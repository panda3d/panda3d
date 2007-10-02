"""ClusterMsgs module: Message types for Cluster rendering"""

# This module is intended to supply routines and dataformats common to
# both ClusterClient and ClusterServer.

from pandac.PandaModules import *
from direct.distributed.PyDatagram import PyDatagram
from direct.distributed.PyDatagramIterator import PyDatagramIterator
import time

#these are the types of messages that are currently supported.
CLUSTER_NONE                  = 0
CLUSTER_CAM_OFFSET            = 1
CLUSTER_CAM_FRUSTUM           = 2
CLUSTER_CAM_MOVEMENT          = 3
CLUSTER_SWAP_READY            = 4
CLUSTER_SWAP_NOW              = 5
CLUSTER_COMMAND_STRING        = 6
CLUSTER_SELECTED_MOVEMENT     = 7
CLUSTER_TIME_DATA             = 8
CLUSTER_NAMED_OBJECT_MOVEMENT = 9
CLUSTER_NAMED_MOVEMENT_DONE   = 10
CLUSTER_EXIT                  = 100

#Port number for cluster rendering
# DAEMON PORT IS PORT USED FOR STARTUP MESSAGE EXCHANGE
# CAN BE OVERRIDEN WITH cluster-daemon-client-port for client
# and cluster-daemon-server-port for server
CLUSTER_DAEMON_PORT = 8001
# THIS IS THE TCP PORT USED FOR EXCHANGE OF DATA ONCE STARTUP IS COMPLETE
CLUSTER_SERVER_PORT = 1970

# Precede command string with ! to tell server to execute command string
# NOTE: Had to stick with the import __builtin__ scheme, at startup,
# __builtins__ is a module, not a dictionary, like it is inside of a module
# Note, this startup string obviates the need to set any cluster related
# config variables in the client Configrc files
SERVER_STARTUP_STRING = (
    '!bash ppython -c ' +
    '"import __builtin__; ' +
    '__builtin__.clusterMode = \'server\';' +
    '__builtin__.clusterServerPort = %s;' +
    '__builtin__.clusterSyncFlag = %d;' +
    '__builtin__.clusterDaemonClient = \'%s\';' +
    '__builtin__.clusterDaemonPort = %d;'
    'from direct.directbase.DirectStart import *; run()"')

class ClusterMsgHandler:
    """ClusterMsgHandler: wrapper for PC clusters/multi-piping networking"""
    def __init__(self, packetStart, notify):
        # packetStart can be used to distinguish which ClusterMsgHandler
        # sends a given packet.
        self.packetNumber = packetStart
        self.notify = notify

    def nonBlockingRead(self, qcr):
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
            datagram = None
            dgi = None
            type = CLUSTER_NONE
        # Note, return datagram to keep a handle on the data
        return (datagram, dgi, type)

    def blockingRead(self, qcr):
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

    def readHeader(self, datagram):
        dgi = PyDatagramIterator(datagram)
        number = dgi.getUint32()
        type = dgi.getUint8()
        self.notify.debug("Packet %d type %d received" % (number, type))
        return (dgi, type)

    def makeCamOffsetDatagram(self, xyz, hpr):
        datagram = PyDatagram()
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
        self.notify.debug('new offset=%f %f %f  %f %f %f' % (x, y, z, h, p, r))
        return (x, y, z, h, p, r)

    def makeCamFrustumDatagram(self, focalLength, filmSize, filmOffset):
        datagram = PyDatagram()
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
        filmOffset  = (dgi.getFloat32(), dgi.getFloat32())
        self.notify.debug('fl, fs, fo=%f, (%f, %f), (%f, %f)' %
                          (focalLength, filmSize[0], filmSize[1],
                           filmOffset[0], filmOffset[1]))
        return (focalLength, filmSize, filmOffset)

    def makeCamMovementDatagram(self, xyz, hpr):
        datagram = PyDatagram()
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


    def makeNamedMovementDone(self):

        datagram = PyDatagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_NAMED_MOVEMENT_DONE)
        return datagram
            

    def makeNamedObjectMovementDatagram(self, xyz, hpr, scale, color, hidden, name):
        datagram = PyDatagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_NAMED_OBJECT_MOVEMENT)
        datagram.addString(name)
        datagram.addFloat32(xyz[0])
        datagram.addFloat32(xyz[1])
        datagram.addFloat32(xyz[2])
        datagram.addFloat32(hpr[0])
        datagram.addFloat32(hpr[1])
        datagram.addFloat32(hpr[2])
        datagram.addFloat32(scale[0])
        datagram.addFloat32(scale[1])
        datagram.addFloat32(scale[2])
        datagram.addFloat32(color[0])
        datagram.addFloat32(color[1])
        datagram.addFloat32(color[2])
        datagram.addFloat32(color[3])        
        datagram.addBool(hidden)
        return datagram    

    def parseCamMovementDatagram(self, dgi):
        x=dgi.getFloat32()
        y=dgi.getFloat32()
        z=dgi.getFloat32()
        h=dgi.getFloat32()
        p=dgi.getFloat32()
        r=dgi.getFloat32()
        self.notify.debug(('  new position=%f %f %f  %f %f %f' %
                           (x, y, z, h, p, r)))
        return (x, y, z, h, p, r)

    def parseNamedMovementDatagram(self, dgi):
        name = dgi.getString()
        x=dgi.getFloat32()
        y=dgi.getFloat32()
        z=dgi.getFloat32()
        h=dgi.getFloat32()
        p=dgi.getFloat32()
        r=dgi.getFloat32()
        sx = dgi.getFloat32()
        sy = dgi.getFloat32()
        sz = dgi.getFloat32()
        red = dgi.getFloat32()
        g = dgi.getFloat32()
        b = dgi.getFloat32()
        a = dgi.getFloat32()        
        hidden = dgi.getBool()
        return (name,x, y, z, h, p, r, sx, sy, sz, red, g, b, a, hidden)


    def makeSelectedMovementDatagram(self, xyz, hpr, scale):
        datagram = PyDatagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_SELECTED_MOVEMENT)
        datagram.addFloat32(xyz[0])
        datagram.addFloat32(xyz[1])
        datagram.addFloat32(xyz[2])
        datagram.addFloat32(hpr[0])
        datagram.addFloat32(hpr[1])
        datagram.addFloat32(hpr[2])
        datagram.addFloat32(scale[0])
        datagram.addFloat32(scale[1])
        datagram.addFloat32(scale[2])
        #datagram.addBool(hidden)
        return datagram

    def parseSelectedMovementDatagram(self, dgi):
        x=dgi.getFloat32()
        y=dgi.getFloat32()
        z=dgi.getFloat32()
        h=dgi.getFloat32()
        p=dgi.getFloat32()
        r=dgi.getFloat32()
        sx=dgi.getFloat32()
        sy=dgi.getFloat32()
        sz=dgi.getFloat32()
        self.notify.debug('  new position=%f %f %f  %f %f %f %f %f %f' %
                          (x, y, z, h, p, r, sx, sy, sz))
        return (x, y, z, h, p, r, sx, sy, sz)

    def makeCommandStringDatagram(self, commandString):
        datagram = PyDatagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_COMMAND_STRING)
        datagram.addString(commandString)
        return datagram

    def parseCommandStringDatagram(self, dgi):
        command = dgi.getString()
        return command

    def makeSwapNowDatagram(self):
        datagram = PyDatagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_SWAP_NOW)
        return datagram

    def makeSwapReadyDatagram(self):
        datagram = PyDatagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_SWAP_READY)
        return datagram

    def makeExitDatagram(self):
        datagram = PyDatagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_EXIT)
        return datagram

    def makeTimeDataDatagram(self, frameCount, frameTime, dt):
        datagram = PyDatagram()
        datagram.addUint32(self.packetNumber)
        self.packetNumber = self.packetNumber + 1
        datagram.addUint8(CLUSTER_TIME_DATA)
        datagram.addUint32(frameCount)
        datagram.addFloat32(frameTime)
        datagram.addFloat32(dt)
        return datagram

    def parseTimeDataDatagram(self, dgi):
        frameCount=dgi.getUint32()
        frameTime=dgi.getFloat32()
        dt=dgi.getFloat32()
        self.notify.debug('time data=%f %f' % (frameTime, dt))
        return (frameCount, frameTime, dt)









