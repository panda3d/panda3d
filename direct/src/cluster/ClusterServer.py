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
        self.qcm = QueuedConnectionManager()
        self.qcl = QueuedConnectionListener(self.qcm, 0)
        self.qcr = QueuedConnectionReader(self.qcm, 0)
        self.cw = ConnectionWriter(self.qcm,0)
        port = base.config.GetInt("cluster-server-port",CLUSTER_PORT)
        self.tcpRendezvous = self.qcm.openTCPServerRendezvous(port, 1)
        print self.tcpRendezvous
        self.cameraGroup = cameraGroup
        self.camera = camera
        self.qcl.addConnection(self.tcpRendezvous)
        self.msgHandler = MsgHandler(ClusterServer.MSG_NUM,self.notify)
        self.startListenerPollTask()
        self.startReaderPollTask()
        self.posOffset = Vec3(0,0,0)
        self.hprOffset = Vec3(0,0,0)
        return None

    def startListenerPollTask(self):
        task = Task.Task(self.listenerPoll)
        taskMgr.add(task, "serverListenerPollTask")
        return None

    def listenerPoll(self, task):
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
        task = Task.Task(self.readerPollUntilEmpty,-10)
        taskMgr.add(task, "serverReaderPollTask")
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
                self.notify.warning("getData returned false")
        return availGetVal

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

    def handleDatagram(self, datagram):
        (type, dgi) = msgHandler.nonBlockingRead(self.qcr)
        if type==CLUSTER_CAM_OFFSET:
            self.handleCamOffset(dgi)
        elif type==CLUSTER_POS_UPDATE:
            self.handleCamMovement(dgi)
        elif type==CLUSTER_SWAP_READY:
            pass
        elif type==CLUSTER_SWAP_NOW:
            pass
        else:
            self.notify.warning("recieved unknown packet")
        return type
    
class ClusterServerSync(ClusterServer):

    def __init__(self,cameraGroup,camera):
        self.notify.info('starting ClusterServerSync')
        self.startReading = 0
        self.posRecieved = 0
        ClusterServer.__init__(self,cameraGroup,camera)
        self.startSwapCoordinator()
        return None

    def startListenerPollTask(self):
        task = Task.Task(self.listenerPoll,-2)
        taskMgr.add(task, "serverListenerPollTask")
        return None

    def listenerPoll(self, task):
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
                datagram = self.msgHandler.blockingRead(self.qcr)
                (type,dgi) = self.msgHandler.readHeader(datagram)
                if type==CLUSTER_CAM_OFFSET:
                    self.handleCamOffset(dgi)
                else:
                    self.notify.warning("Wanted cam offset, got something else")
                self.startReading = 1
                # now that we have the offset read, can start reading
            else:
                self.notify.warning("getNewConnection returned false")
        return Task.cont

    def startReaderPollTask(self):
        task = Task.Task(self.readPos,-1)
        taskMgr.add(task, "serverReadPosTask")
        return None

    def readPos(self, task):
        if self.startReading and self.qcr.isConnectionOk(self.lastConnection):
            datagram = self.msgHandler.blockingRead(self.qcr)
            (type,dgi) = self.msgHandler.readHeader(datagram)
            if type == CLUSTER_POS_UPDATE:
                self.posRecieved = 1
                self.handleCamMovement(dgi)
            elif type == CLUSTER_CAM_OFFSET:
                self.handleCamOffset(dgi)
            else:
                self.notify.warning('expected pos or orientation, instead got %d' % type)
        else:
            self.startReading = 0 # keep this 0 as long as connection not ok

        return Task.cont

    def sendSwapReady(self):
        self.notify.debug( ('send swap ready packet %d' %
                            self.msgHandler.packetNumber ) )
        datagram = self.msgHandler.makeSwapReadyDatagram()
        self.cw.send(datagram, self.lastConnection)

    def startSwapCoordinator(self):
        task = Task.Task(self.swapCoordinatorTask, 51)
        taskMgr.add(task, "serverSwapCoordinator")
        return None

    def swapCoordinatorTask(self, task):
        if self.posRecieved:
            self.posRecieved = 0
            localClock = ClockObject()
#            print "START send-------------------------------"
            t1 = localClock.getRealTime()
            self.sendSwapReady()
#            print "-----------START read--------------------"
            t2 = localClock.getRealTime()
            datagram = self.msgHandler.blockingRead(self.qcr)
            (type,dgi) = self.msgHandler.readHeader(datagram)
            if type == CLUSTER_SWAP_NOW:
                self.notify.debug('swapping')
#                print "---------------------START SWAP----------"
                t3 = localClock.getRealTime()
                base.win.swap()
                t4 = localClock.getRealTime()
#                print "---------------------------------END SWAP"
#                print "times=",t1,t2,t3,t4
#                print "deltas=",t2-t1,t3-t2,t4-t3
            else:
                self.notify.warning("did not get expected swap now")
        return Task.cont


    








