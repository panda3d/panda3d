"""ServerRepository module: contains the ServerRepository class"""

from PandaModules import *
from ClusterMsgs import *
from MsgTypes import *
import DirectNotifyGlobal
import DirectObject
import Task

# Also, I'm not sure multiple camera-group configurations are working for the
# cluster system.

class ClusterServer(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ClusterServer")
    MSG_NUM = 2000000

    def __init__(self,cameraJig,camera):
        # Store information about the cluster's camera
        self.cameraJig = cameraJig
        self.camera = camera
        self.lens = camera.node().getLens()
        self.lastConnection = None
        self.fPosReceived = 0
        # Create network layer objects
        self.qcm = QueuedConnectionManager()
        self.qcl = QueuedConnectionListener(self.qcm, 0)
        self.qcr = QueuedConnectionReader(self.qcm, 0)
        self.cw = ConnectionWriter(self.qcm,0)
        port = base.config.GetInt('cluster-server-port', CLUSTER_PORT)
        self.tcpRendezvous = self.qcm.openTCPServerRendezvous(port, 1)
        self.qcl.addConnection(self.tcpRendezvous)
        self.msgHandler = ClusterMsgHandler(ClusterServer.MSG_NUM, self.notify)
        # Start cluster tasks
        self.startListenerPollTask()
        self.startReaderPollTask()

    def startListenerPollTask(self):
        # Run this task near the start of frame, sometime after the dataloop
        taskMgr.add(self.listenerPollTask, "serverListenerPollTask",-40)

    def listenerPollTask(self, task):
        """ Task to listen for a new connection from the client """
        # Run this task after the dataloop
        if self.qcl.newConnectionAvailable():
            self.notify.info("New connection is available")
            rendezvous = PointerToConnection()
            netAddress = NetAddress()
            newConnection = PointerToConnection()
            if self.qcl.getNewConnection(rendezvous,netAddress,newConnection):
                # Crazy dereferencing
                newConnection=newConnection.p()
                self.qcr.addConnection(newConnection)
                self.lastConnection = newConnection
                self.notify.info("Got a connection!")
            else:
                self.notify.warning("getNewConnection returned false")
        return Task.cont

    def startReaderPollTask(self):
        """ Task to handle datagrams from client """
        # Run this task just after the listener poll task
        taskMgr.add(self.readerPollTask, "serverReaderPollTask", -39)

    def readerPollTask(self, state):
        """ Non blocking task to read all available datagrams """
        while 1:
            (datagram, dgi,type) = self.msgHandler.nonBlockingRead(self.qcr)
            # Queue is empty, done for now
            if type is CLUSTER_NONE:
                break
            else:
                # Got a datagram, handle it
                self.handleDatagram(dgi, type)
        return Task.cont

    def handleDatagram(self, dgi, type):
        """ Process a datagram depending upon type flag """
        if (type == CLUSTER_NONE):
            pass
        elif (type == CLUSTER_EXIT):
            import sys
            sys.exit()
        elif (type == CLUSTER_CAM_OFFSET):
            self.handleCamOffset(dgi)
        elif (type == CLUSTER_CAM_FRUSTUM):
            self.handleCamFrustum(dgi)
        elif (type == CLUSTER_CAM_MOVEMENT):
            self.handleCamMovement(dgi)
        elif (type == CLUSTER_COMMAND_STRING):
            self.handleCommandString(dgi)
        elif (type == CLUSTER_SWAP_READY):
            pass
        elif (type == CLUSTER_SWAP_NOW):
            self.notify.debug('swapping')
            base.win.swap()
        else:
            self.notify.warning("Received unknown packet type:" % type)
        return type

    # Server specific tasks
    def handleCamOffset(self,dgi):
        """ Set offset of camera from cameraJig """
        (x,y,z,h,p,r) = self.msgHandler.parseCamOffsetDatagram(dgi)
        self.lens.setIodOffset(x)
        self.lens.setViewHpr(h,p,r)
        
    def handleCamFrustum(self,dgi):
        """ Adjust camera frustum based on parameters sent by client """
        (fl,fs,fo) = self.msgHandler.parseCamFrustumDatagram(dgi)
        self.lens.setFocalLength(fl)
        self.lens.setFilmSize(fs[0], fs[1])
        self.lens.setFilmOffset(fo[0], fo[1])

    def handleCamMovement(self,dgi):
        """ Update cameraJig position to reflect latest position """
        (x,y,z,h,p,r) = self.msgHandler.parseCamMovementDatagram(dgi)
        self.cameraJig.setPosHpr(render,x,y,z,h,p,r)
        self.fPosReceived = 1

    def handleCommandString(self, dgi):
        """ Handle arbitrary command string from client """
        command = self.msgHandler.parseCommandStringDatagram(dgi)
        exec( command, globals() )
        
class ClusterServerSync(ClusterServer):

    def __init__(self,cameraJig,camera):
        self.notify.info('starting ClusterServerSync')
        ClusterServer.__init__(self,cameraJig,camera)
        self.startSwapCoordinator()

    def readerPollTask(self, task):
        if self.lastConnection is None:
            pass
        elif self.qcr.isConnectionOk(self.lastConnection):
            # Process datagrams till you get a postion update
            type = CLUSTER_NONE
            while type != CLUSTER_CAM_MOVEMENT:
                # Block until you get a new datagram
                (datagram,dgi,type) = self.msgHandler.blockingRead(self.qcr)
                # Process datagram
                self.handleDatagram(dgi,type)
        return Task.cont

    def sendSwapReady(self):
        self.notify.debug(
            'send swap ready packet %d' % self.msgHandler.packetNumber)
        datagram = self.msgHandler.makeSwapReadyDatagram()
        self.cw.send(datagram, self.lastConnection)

    def startSwapCoordinator(self):
        taskMgr.add(self.swapCoordinatorTask, "serverSwapCoordinator", 51)

    def swapCoordinatorTask(self, task):
        if self.fPosReceived:
            self.fPosReceived = 0
            # Alert client that this server is ready to swap
            self.sendSwapReady()
            # Wait for swap command (processing any intermediate datagrams)
            while 1:
                (datagram,dgi,type) = self.msgHandler.blockingRead(self.qcr)
                self.handleDatagram(dgi,type)
                if type == CLUSTER_SWAP_NOW:
                    break
        return Task.cont


    








