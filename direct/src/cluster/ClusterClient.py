"""ClusterClient: Master for mutlipiping or PC clusters.  """

from PandaModules import *
from ClusterMsgs import *
import DirectNotifyGlobal
import DirectObject
import Task

class ClusterConfigItem:
    def __init__(self, serverFunction, serverName, port):
        self.serverName = serverName
        self.serverFunction = serverFunction
        self.port = port
        # Camera Offset
        self.xyz = Vec3(0)
        self.hpr = Vec3(0)
        # Camera Frustum Data
        self.fFrustum = 0
        self.focalLength = None
        self.filmSize = None
        self.filmOffset = None
    def setCamOffset(self, xyz, hpr):
        self.xyz = xyz
        self.hpr = hpr
    def setCamFrustum(self, focalLength, filmSize, filmOffset):
        self.fFrustum = 1
        self.focalLength = focalLength
        self.filmSize = filmSize
        self.filmOffset = filmOffset

class DisplayConnection:
    def __init__(self,qcm,serverName,port,msgHandler):
        self.msgHandler = msgHandler
        gameServerTimeoutMs = base.config.GetInt(
            "game-server-timeout-ms", 20000)
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

    def sendCamFrustum(self,focalLength, filmSize, filmOffset):
        ClusterManager.notify.debug("send cam frustum...")
        ClusterManager.notify.debug(
            (("packet %d" % self.msgHandler.packetNumber) +
             (" fl, fs, fo=%0.3f, (%0.3f, %0.3f), (%0.3f, %0.3f)" %
              (focalLength, filmSize[0], filmSize[1],
               filmOffset[0], filmOffset[1])))
            )
        datagram = self.msgHandler.makeCamFrustumDatagram(
            focalLength, filmSize, filmOffset)
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
        while 1:
            (datagram, dgi,type) = self.msgHandler.blockingRead(self.qcr)
            if type == CLUSTER_SWAP_READY:
                break
            else:
                self.notify.warning('was expecting SWAP_READY, got %d' % type)

    # the following should only be called by a synchronized cluster manger
    def sendSwapNow(self):
        ClusterManager.notify.debug( 
            "display connect send swap now, packet %d" %
            self.msgHandler.packetNumber)
        datagram = self.msgHandler.makeSwapNowDatagram()
        self.cw.send(datagram, self.tcpConn)
        
    def sendCommandString(self, commandString):
        ClusterManager.notify.debug("send command string: %s" % commandString)
        datagram = self.msgHandler.makeCommandStringDatagram(commandString)
        self.cw.send(datagram, self.tcpConn)

    def sendExit(self):
        ClusterManager.notify.debug( 
            "display connect send exit, packet %d" %
            self.msgHandler.packetNumber)
        datagram = self.msgHandler.makeExitDatagram()
        self.cw.send(datagram, self.tcpConn)

class ClusterManager(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ClusterClient")
    MGR_NUM = 1000000

    def __init__(self, configList):
        self.qcm=QueuedConnectionManager()
        self.serverList = []
        self.msgHandler = MsgHandler(ClusterManager.MGR_NUM,self.notify)
        for serverConfig in configList:
            server = DisplayConnection(self.qcm,serverConfig.serverName,
                                         serverConfig.port,self.msgHandler)
            if server == None:
                self.notify.error( ('Could not open %s on %s port %d' %
                                    (serverConfig.serverFunction,
                                     serverConfig.serverName,
                                     serverConfig.port)) )
            else:
                server.sendCamOffset(serverConfig.xyz,serverConfig.hpr)
                if serverConfig.fFrustum:
                    server.sendCamFrustum(serverConfig.focalLength,
                                          serverConfig.filmSize,
                                          serverConfig.filmOffset)
                self.serverList.append(server)
        self.startMoveCamTask()

    def moveCamera(self, xyz, hpr):
        for server in self.serverList:
            server.sendMoveCam(xyz,hpr)

    def startMoveCamTask(self):
        taskMgr.add(self.moveCameraTask, "moveCamTask", 49)

    def moveCameraTask(self,task):
        self.moveCamera(
            direct.camera.getPos(render),
            direct.camera.getHpr(render))
        return Task.cont
        
    def clusterCommand(self, commandString):
        # Execute remotely
        for server in self.serverList:
            server.sendCommandString(commandString)
        # Execute locally
        exec( commandString, globals())

    def exit(self):
        # Execute remotely
        for server in self.serverList:
            server.sendExit()
        # Execute locally
        import sys
        sys.exit()

class ClusterManagerSync(ClusterManager):

    def __init__(self, configList):
        ClusterManager.__init__(self, configList)
        #I probably don't need this
        self.waitForSwap = 0
        self.ready = 0
        self.startSwapCoordinatorTask()

    def startSwapCoordinatorTask(self):
        taskMgr.add(self.swapCoordinator, "clientSwapCoordinator", 51)
        return None

    def swapCoordinator(self,task):
        self.ready = 1
        if self.waitForSwap:
            self.waitForSwap=0
            self.notify.debug(
                "START get swaps----------------------------------")
            for server in self.serverList:
                server.getSwapReady()
            self.notify.debug(
                "----------------START swap now--------------------")
            for server in self.serverList:
                server.sendSwapNow()
            self.notify.debug(
                "------------------------------START swap----------")
            base.win.swap()
            self.notify.debug(
                "------------------------------------------END swap")
        return Task.cont

    def moveCamera(self,xyz,hpr):
        if self.ready:
            self.notify.debug('moving synced camera')
            ClusterManager.moveCamera(self,xyz,hpr)
            self.waitForSwap=1
        





