"""ServerRepository module: contains the ServerRepository class"""

from ShowBaseGlobal import *
from ClusterMsgs import *
import DirectObject
import Datagram
#import DatagramIterator
#import NetDatagram
import __builtin__
import time
from PandaModules import *
from TaskManagerGlobal import *
from MsgTypes import *
import Task
import DirectNotifyGlobal

# Cam offset handling is a little sloppy.  The problem is that there is a
# single arc used for both movement of the camera, and the offset of the
# group.

# Also, I'm not sure multiple camera-group configurations are working for the
# cluster system.

class ClusterServer(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ClusterServer")
    MSG_NUM = 2000000

    def __init__(self,cameraGroup,camera):
        # Store information about the cluster's camera
        self.cameraGroup = cameraGroup
        self.camera = camera
        self.lens = camera.node().getLens()
        # Initialize camera offsets
        self.posOffset = Vec3(0,0,0)
        self.hprOffset = Vec3(0,0,0)
        # Create network layer objects
        self.lastConnection = None
        self.qcm = QueuedConnectionManager()
        self.qcl = QueuedConnectionListener(self.qcm, 0)
        self.qcr = QueuedConnectionReader(self.qcm, 0)
        self.cw = ConnectionWriter(self.qcm,0)
        port = base.config.GetInt("cluster-server-port",CLUSTER_PORT)
        self.tcpRendezvous = self.qcm.openTCPServerRendezvous(port, 1)
        self.qcl.addConnection(self.tcpRendezvous)
        self.msgHandler = MsgHandler(ClusterServer.MSG_NUM,self.notify)
        # Start cluster tasks
        self.startListenerPollTask()
        self.startReaderPollTask()

    def startListenerPollTask(self):
        taskMgr.add(self.listenerPollTask, "serverListenerPollTask",-40)

    def listenerPollTask(self, task):
        """ Task to listen for a new connection from the client """
        # Run this task after the dataloop
        if self.qcl.newConnectionAvailable():
            print "New connection is available"
            rendezvous = PointerToConnection()
            netAddress = NetAddress()
            newConnection = PointerToConnection()
            retVal = self.qcl.getNewConnection(rendezvous, netAddress,
                                               newConnection)
            if retVal:
                # Crazy dereferencing
                newConnection=newConnection.p()
                self.qcr.addConnection(newConnection)
                print "Got a connection!"
                self.lastConnection = newConnection
            else:
                self.notify.warning(
                    "getNewConnection returned false")
        return Task.cont

    def startReaderPollTask(self):
        """ Task to handle datagrams from client """
        # Run this task just after the listener poll task and dataloop
        taskMgr.add(self.readerPollTask, "serverReaderPollTask", -39)

    def readerPollTask(self):
        while self.qcr.dataAvailable():
            datagram = NetDatagram()
            readRetVal = self.qcr.getData(datagram)
            if readRetVal:
                self.handleDatagram(datagram)
            else:
                self.notify.warning("getData returned false")
        return Task.cont

    def handleDatagram(self, datagram):
        (type, dgi) = self.msgHandler.nonBlockingRead(self.qcr)
        if type==CLUSTER_CAM_OFFSET:
            self.handleCamOffset(dgi)
        elif type==CLUSTER_CAM_FRUSTUM:
            self.handleCamFrustum(dgi)
        elif type==CLUSTER_POS_UPDATE:
            self.handleCamMovement(dgi)
        elif type==CLUSTER_SWAP_READY:
            pass
        elif type==CLUSTER_SWAP_NOW:
            pass
        elif type==CLUSTER_COMMAND_STRING:
            self.handleCommandString(dgi)
        else:
            self.notify.warning("recieved unknown packet")
        return type
    
    def handleCamOffset(self,dgi):
        x=dgi.getFloat32()
        y=dgi.getFloat32()
        z=dgi.getFloat32()
        h=dgi.getFloat32()
        p=dgi.getFloat32()
        r=dgi.getFloat32()
        self.notify.debug(('  new offset=%f %f %f  %f %f %f' %
                           (x,y,z,h,p,r)))
        self.posOffset = Vec3(x,y,z)
        self.hprOffset = Vec3(h,p,r)
        
    def handleCamFrustum(self,dgi):
        focalLength=dgi.getFloat32()
        filmSize=(dgi.getFloat32(), dgi.getFloat32())
        filmOffset=(dgi.getFloat32(),dgi.getFloat32())
        self.notify.debug('  fl, fs, fo=%f, (%f, %f), (%f, %f)' %
                          (focalLength, filmSize[0], filmSize[1],
                           filmOffset[0], filmOffset[1]))
        self.lens.setFocalLength(focalLength)
        self.lens.setFilmSize(filmSize[0], filmSize[1])
        self.lens.setFilmOffset(filmOffset[0], filmOffset[1])

    def handleCamMovement(self,dgi):
        x=dgi.getFloat32()
        y=dgi.getFloat32()
        z=dgi.getFloat32()
        h=dgi.getFloat32()
        p=dgi.getFloat32()
        r=dgi.getFloat32()
        self.notify.debug(('  new position=%f %f %f  %f %f %f' %
                           (x,y,z,h,p,r)))
        finalX = x + self.posOffset[0]
        finalY = y + self.posOffset[1]
        finalZ = z + self.posOffset[2]
        finalH = h + self.hprOffset[0]
        finalP = p + self.hprOffset[1]
        finalR = r + self.hprOffset[2]
        self.cameraGroup.setPosHpr(render,finalX,finalY,finalZ,
                                   finalH,finalP,finalR)

    def handleCommandString(self, dgi):
        command = dgi.getString()
        exec( command, globals() )
        
class ClusterServerSync(ClusterServer):

    def __init__(self,cameraGroup,camera):
        self.notify.info('starting ClusterServerSync')
        self.posRecieved = 0
        ClusterServer.__init__(self,cameraGroup,camera)
        self.startSwapCoordinator()
        return None

    def readerPollTask(self, task):
        if self.lastConnection is None:
            pass
        elif self.qcr.isConnectionOk(self.lastConnection):
            # Process datagrams till you get a postion update
            type = CLUSTER_NOTHING
            while type != CLUSTER_POS_UPDATE:
                datagram = self.msgHandler.blockingRead(self.qcr)
                (type,dgi) = self.msgHandler.readHeader(datagram)
                if type == CLUSTER_POS_UPDATE:
                    # Move camera
                    self.handleCamMovement(dgi)
                    # Set flag for swap coordinator
                    self.posRecieved = 1
                elif type == CLUSTER_CAM_OFFSET:
                    # Update camera offset                    
                    self.handleCamOffset(dgi)
                elif type == CLUSTER_COMMAND_STRING:
                    # Got a command, execute it
                    self.handleCommandString(dgi)
        return Task.cont

    def sendSwapReady(self):
        self.notify.debug( ('send swap ready packet %d' %
                            self.msgHandler.packetNumber ) )
        datagram = self.msgHandler.makeSwapReadyDatagram()
        self.cw.send(datagram, self.lastConnection)

    def startSwapCoordinator(self):
        taskMgr.add(self.swapCoordinatorTask, "serverSwapCoordinator", 51)
        return None

    def swapCoordinatorTask(self, task):
        if self.posRecieved:
            self.posRecieved = 0
            self.sendSwapReady()
            datagram = self.msgHandler.blockingRead(self.qcr)
            (type,dgi) = self.msgHandler.readHeader(datagram)
            if type == CLUSTER_SWAP_NOW:
                self.notify.debug('swapping')
                base.win.swap()
            else:
                self.notify.warning("did not get expected swap now")
        return Task.cont


    








