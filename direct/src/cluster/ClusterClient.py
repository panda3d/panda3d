"""ClusterClient: Master for mutlipiping or PC clusters.  """

from PandaModules import *
from TaskManagerGlobal import *
from ShowBaseGlobal import *
import Task
import DirectNotifyGlobal
import Datagram
import DirectObject
from ClusterMsgs import *
import time

class ClusterConfigItem:
    def __init__(self, serverFunction, serverName, pos, hpr, port):
        self.serverName = serverName
        self.serverFunction = serverFunction
        self.xyz = pos
        self.hpr = hpr
        self.port = port

class DisplayConnection:
    def __init__(self,qcm,serverName,port,msgHandler):
        self.msgHandler = msgHandler
        gameServerTimeoutMs = base.config.GetInt("game-server-timeout-ms",
                                                 20000)
        # A big old 20 second timeout.
        self.tcpConn = qcm.openTCPClientConnection(
            serverName, port, gameServerTimeoutMs)
        # Test for bad connection
        if self.tcpConn == None:
            return None
        else:
            self.tcpConn.setNoDelay(1)
            self.qcr=QueuedConnectionReader(qcm, 0)
            self.qcr.addConnection(self.tcpConn)
            self.cw=ConnectionWriter(qcm, 0)

    def sendCamOffset(self,xyz,hpr):
        ClusterManager.notify.debug("send cam offset...")
        ClusterManager.notify.debug( ("packet %d xyz,hpr=%f %f %f %f %f %f" %
             (self.msgHandler.packetNumber,xyz[0],xyz[1],xyz[2],
             hpr[0],hpr[1],hpr[2])) )
        datagram = self.msgHandler.makeCamOffsetDatagram(xyz, hpr)
        self.cw.send(datagram, self.tcpConn)

    def sendMoveCam(self,xyz,hpr):
        ClusterManager.notify.debug("send cam move...")
        ClusterManager.notify.debug( ("packet %d xyz,hpr=%f %f %f %f %f %f" %
             (self.msgHandler.packetNumber,xyz[0],xyz[1],xyz[2],
             hpr[0],hpr[1],hpr[2])) )
        datagram = self.msgHandler.makeMoveCamDatagram(xyz, hpr)
        self.cw.send(datagram, self.tcpConn)

    # the following should only be called by a synchronized cluster manger
    def getSwapReady(self):
        datagram = self.msgHandler.blockingRead(self.qcr)
        (type,dgi) = self.msgHandler.readHeader(datagram)
        if type != CLUSTER_SWAP_READY:
            self.notify.warning( ('was expecting SWAP_READY, got %d' % type) )

    # the following should only be called by a synchronized cluster manger
    def sendSwapNow(self):
        ClusterManager.notify.debug( ("dispaly connect send swap now, packet %d" % self.msgHandler.packetNumber))
        datagram = self.msgHandler.makeSwapNowDatagram()
        self.cw.send(datagram, self.tcpConn)
        
class ClusterManager(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ClusterClient")
    MGR_NUM = 1000000

    def __init__(self, dispConfigs):
        self.qcm=QueuedConnectionManager()
        self.dispList = []
        self.msgHandler = MsgHandler(ClusterManager.MGR_NUM,self.notify)
        for dispConfig in dispConfigs:
            thisDisp = DisplayConnection(self.qcm,dispConfig.serverName,
                                         dispConfig.port,self.msgHandler)
            if thisDisp == None:
                self.notify.error( ('Could not open %s on %s port %d' %
                                    (dispConfig.serverFunction,
                                     dispConfig.serverName,
                                     dispConfig.port)) )
            else:
                self.dispList.append(thisDisp)
                self.dispList[len(self.dispList)-1].sendCamOffset(
                    dispConfig.xyz,dispConfig.hpr)
        self.startMoveCamTask()

    def moveCamera(self, xyz, hpr):
        for disp in self.dispList:
            disp.sendMoveCam(xyz,hpr)

    def startMoveCamTask(self):
        task = Task.Task(self.moveCameraTask,49)
        taskMgr.spawnTaskNamed(task, "moveCamTask")
        return None        

    def moveCameraTask(self,task):
        self.moveCamera(
            base.camera.getPos(render),
            base.camera.getHpr(render))
        return Task.cont
        

class ClusterManagerSync(ClusterManager):

    def __init__(self, dispConfigs):
        ClusterManager.__init__(self, dispConfigs)
        #I probably don't need this
        self.waitForSwap = 0
        self.ready = 0
        self.startSwapCoordinatorTask()

    def startSwapCoordinatorTask(self):
        task = Task.Task(self.swapCoordinator,51)
        taskMgr.spawnTaskNamed(task, "clientSwapCoordinator")
        return None

    def swapCoordinator(self,task):
        self.ready = 1
        if self.waitForSwap:
            self.waitForSwap=0
            self.notify.debug("START get swaps----------------------------------")
            localClock = ClockObject()
            t1 = localClock.getRealTime()
            for disp in self.dispList:
                disp.getSwapReady()
            self.notify.debug("----------------START swap now--------------------")
            t2 = localClock.getRealTime()
            for disp in self.dispList:
                disp.sendSwapNow()
            self.notify.debug("------------------------------START swap----------")
            t3 = localClock.getRealTime()
            base.win.swap()
            t4 = localClock.getRealTime()
            self.notify.debug("------------------------------------------END swap")
            self.notify.debug( ("times=%f %f %f %f" % (t1,t2,t3,t4)) )
            self.notify.debug( ("deltas=%f %f %f" % (t2-t1,t3-t2,t4-t3)) )
        return Task.cont

    def moveCamera(self,xyz,hpr):
        if self.ready:
            self.notify.debug('moving synced camera')
            ClusterManager.moveCamera(self,xyz,hpr)
            self.waitForSwap=1
        





