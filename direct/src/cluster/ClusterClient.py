"""ClusterClient: Master for mutli-piping or PC clusters.  """

from PandaModules import *
from ClusterMsgs import *
from ClusterConfig import *
import DirectNotifyGlobal
import DirectObject
import Task

class ClusterClient(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ClusterClient")
    MGR_NUM = 1000000

    def __init__(self, configList):
        self.qcm=QueuedConnectionManager()
        self.serverList = []
        self.msgHandler = ClusterMsgHandler(ClusterClient.MGR_NUM, self.notify)
        for serverConfig in configList:
            server = DisplayConnection(self.qcm,serverConfig.serverName,
                                         serverConfig.port,self.msgHandler)
            if server == None:
                self.notify.error('Could not open %s on %s port %d' %
                                  (serverConfig.serverConfigName,
                                   serverConfig.serverName,
                                   serverConfig.port))
            else:
                server.sendCamOffset(serverConfig.xyz,serverConfig.hpr)
                if serverConfig.fFrustum:
                    server.sendCamFrustum(serverConfig.focalLength,
                                          serverConfig.filmSize,
                                          serverConfig.filmOffset)
                self.serverList.append(server)
        self.startMoveCamTask()
        self.startMoveSelectedTask()

    def startMoveCamTask(self):
        taskMgr.add(self.moveCameraTask, "moveCamTask", 49)

    def moveCameraTask(self,task):
        self.moveCamera(
            base.camera.getPos(render),
            base.camera.getHpr(render))
        return Task.cont

    def moveCamera(self, xyz, hpr):
        self.notify.debug('moving unsynced camera')
        for server in self.serverList:
            server.sendMoveCam(xyz,hpr)

    def startMoveSelectedTask(self):
        taskMgr.add(self.moveSelectedTask, "moveSelectedTask", 48)

    def moveSelectedTask(self, state):
        # Update cluster if current display is a cluster client
        if (last is not None):
            self.notify.debug('moving selected node path')
            xyz = Point3(0)
            hpr = VBase3(0)
            scale = VBase3(1)
            decomposeMatrix(last.getMat(), scale, hpr, xyz)
            for server in self.serverList:
                server.sendMoveSelected(xyz,hpr)
        return Task.cont

    def getNodePathFindCmd(self, nodePath):
        import string
        pathString = `nodePath`
        index = string.find(pathString, '/')
        if index != -1:
            rootName = pathString[:index]
            searchString = pathString[index+1:]
            return rootName + ('.find("%s")' % searchString)
        else:
            return rootName

    def selectNodePath(self, nodePath):
        self.cmd(self.getNodePathFindCmd(nodePath) + '.select()', 0)

    def deselectNodePath(self, nodePath):
        self.cmd(self.getNodePathFindCmd(nodePath) + '.deselect()', 0)

    def loadModel(self, nodePath):
        pass

    def cmd(self, commandString, fLocally = 1):
        # Execute remotely
        for server in self.serverList:
            server.sendCommandString(commandString)
        if fLocally:
            # Execute locally
            exec( commandString, globals() )

    def exit(self):
        # Execute remotely
        for server in self.serverList:
            server.sendExit()
        # Execute locally
        import sys
        sys.exit()

class ClusterClientSync(ClusterClient):

    def __init__(self, configList):
        ClusterClient.__init__(self, configList)
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
            ClusterClient.moveCamera(self,xyz,hpr)
            self.waitForSwap=1
        
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
        ClusterClient.notify.debug("send cam offset...")
        ClusterClient.notify.debug( ("packet %d xyz,hpr=%f %f %f %f %f %f" %
             (self.msgHandler.packetNumber,xyz[0],xyz[1],xyz[2],
             hpr[0],hpr[1],hpr[2])) )
        datagram = self.msgHandler.makeCamOffsetDatagram(xyz, hpr)
        self.cw.send(datagram, self.tcpConn)

    def sendCamFrustum(self,focalLength, filmSize, filmOffset):
        ClusterClient.notify.info("send cam frustum...")
        ClusterClient.notify.info(
            (("packet %d" % self.msgHandler.packetNumber) +
             (" fl, fs, fo=%0.3f, (%0.3f, %0.3f), (%0.3f, %0.3f)" %
              (focalLength, filmSize[0], filmSize[1],
               filmOffset[0], filmOffset[1])))
            )
        datagram = self.msgHandler.makeCamFrustumDatagram(
            focalLength, filmSize, filmOffset)
        self.cw.send(datagram, self.tcpConn)

    def sendMoveCam(self,xyz,hpr):
        ClusterClient.notify.debug("send cam move...")
        ClusterClient.notify.debug( ("packet %d xyz,hpr=%f %f %f %f %f %f" %
             (self.msgHandler.packetNumber,xyz[0],xyz[1],xyz[2],
             hpr[0],hpr[1],hpr[2])) )
        datagram = self.msgHandler.makeCamMovementDatagram(xyz, hpr)
        self.cw.send(datagram, self.tcpConn)

    def sendMoveSelected(self,xyz,hpr):
        ClusterClient.notify.debug("send move selected...")
        ClusterClient.notify.debug("packet %d xyz,hpr=%f %f %f %f %f %f" %
                                   (self.msgHandler.packetNumber,
                                    xyz[0],xyz[1],xyz[2],
                                    hpr[0],hpr[1],hpr[2]))
        datagram = self.msgHandler.makeSelectedMovementDatagram(xyz, hpr)
        self.cw.send(datagram, self.tcpConn)

    # the following should only be called by a synchronized cluster manger
    def getSwapReady(self):
        while 1:
            (datagram, dgi, type) = self.msgHandler.blockingRead(self.qcr)
            if type == CLUSTER_SWAP_READY:
                break
            else:
                self.notify.warning('was expecting SWAP_READY, got %d' % type)

    # the following should only be called by a synchronized cluster manger
    def sendSwapNow(self):
        ClusterClient.notify.debug( 
            "display connect send swap now, packet %d" %
            self.msgHandler.packetNumber)
        datagram = self.msgHandler.makeSwapNowDatagram()
        self.cw.send(datagram, self.tcpConn)
        
    def sendCommandString(self, commandString):
        ClusterClient.notify.debug("send command string: %s" % commandString)
        datagram = self.msgHandler.makeCommandStringDatagram(commandString)
        self.cw.send(datagram, self.tcpConn)

    def sendExit(self):
        ClusterClient.notify.debug( 
            "display connect send exit, packet %d" %
            self.msgHandler.packetNumber)
        datagram = self.msgHandler.makeExitDatagram()
        self.cw.send(datagram, self.tcpConn)

class ClusterConfigItem:
    def __init__(self, serverConfigName, serverName, port):
        self.serverConfigName = serverConfigName
        self.serverName = serverName
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

def createClusterClient():
    # setup camera offsets based on cluster-config
    clusterConfig = base.config.GetString('cluster-config', 'single-server')
    # No cluster config specified!
    if not ClientConfigs.has_key(clusterConfig):
        base.notify.warning(
            'createClusterClient: %s cluster-config is undefined.' %
            clusterConfig)
        return None
    # Get display config for each server in the cluster
    displayConfigs = []
    configList = ClientConfigs[clusterConfig]
    numConfigs = len(configList)
    for i in range(numConfigs):
        configData = configList[i]
        displayName = configData.get('display name', ('display%d' % i))
        displayMode = configData.get('display mode', 'server')
        # Init Cam Offset
        pos = configData.get('pos', Vec3(0))
        hpr = configData.get('hpr', Vec3(0))
        # Init Frustum if specified
        fl = configData.get('focal length', None)
        fs = configData.get('film size', None)
        fo = configData.get('film offset', None)
        if displayMode == 'client':
            base.camera.setPosHpr(pos,hpr)
            lens = base.cam.node().getLens()
            lens.setFocalLength(fl)
            lens.setFilmSize(fs[0], fs[1])
            lens.setFilmOffset(fo[0], fo[1])
        else:
            serverConfigName = 'cluster-server-%s' % displayName
            serverString = base.config.GetString(serverConfigName, '')
            if serverString == '':
                base.notify.warning(
                    '%s undefined in Configrc: expected by %s display client.'%
                    (serverConfigName,clusterConfig))
                base.notify.warning('%s will not be used.' % serverConfigName)
            else:
                serverInfo = string.split(serverString)
                serverName = serverInfo[0]
                if len(serverInfo) > 1:
                    port = int(serverInfo[1])
                else:
                    # Use default port
                    port = CLUSTER_PORT
                cci = ClusterConfigItem(
                    serverConfigName,
                    serverName,
                    port)
                # Init cam offset
                cci.setCamOffset(pos, hpr)
                # Init frustum if specified
                if fl and fs and fo:
                    cci.setCamFrustum(fl, fs, fo)
                displayConfigs.append(cci)
    # Create Cluster Managers (opening connections to servers)
    # Are the servers going to be synced?
    if base.config.GetBool('cluster-sync', 0):
        base.win.setSync(1)
        return ClusterClientSync(displayConfigs)
    else:
            return ClusterClient(displayConfigs)
    
    
