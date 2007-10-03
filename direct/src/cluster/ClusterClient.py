"""ClusterClient: Master for mutli-piping or PC clusters.  """

from pandac.PandaModules import *
from ClusterMsgs import *
from ClusterConfig import *
from direct.directnotify import DirectNotifyGlobal
from direct.showbase import DirectObject
from direct.task import Task
import os

class ClusterClient(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ClusterClient")
    MGR_NUM = 1000000

    def __init__(self, configList, clusterSyncFlag):
        # Set name so cluster __call__ function can be used in Intervals
        self.__name__ = 'cluster'
        # First start up servers using direct daemon
        # What is the name of the client machine?
        clusterClientDaemonHost = base.config.GetString(
            'cluster-client-daemon', 'None')
        if clusterClientDaemonHost == 'None':
            clusterClientDaemonHost = os.popen('uname -n').read()
            clusterClientDaemonHost = clusterClientDaemonHost.replace('\n', '')
        # What daemon port are we using to communicate between client/servers
        clusterClientDaemonPort = base.config.GetInt(
            'cluster-client-daemon-port', CLUSTER_DAEMON_PORT)
        # Create a daemon
        self.daemon = DirectD()
        # Start listening for the response
        self.daemon.listenTo(clusterClientDaemonPort)
        # Contact server daemons and start up remote server application
        for serverConfig in configList:
            # First kill existing application
            self.daemon.tellServer(serverConfig.serverName,
                                   serverConfig.serverDaemonPort,
                                   'ka')
            # Now start up new application
            serverCommand = (SERVER_STARTUP_STRING %
                             (serverConfig.serverMsgPort,
                              clusterSyncFlag,
                              clusterClientDaemonHost,
                              clusterClientDaemonPort))
            self.daemon.tellServer(serverConfig.serverName,
                                   serverConfig.serverDaemonPort,
                                   serverCommand)
        print 'Begin waitForServers'
        if not self.daemon.waitForServers(len(configList)):
            print 'Cluster Client, no response from servers'
        print 'End waitForServers'
        self.qcm=QueuedConnectionManager()
        self.serverList = []
        self.serverQueues = []
        self.msgHandler = ClusterMsgHandler(ClusterClient.MGR_NUM, self.notify)

        # A dictionary of objects that can be accessed by name
        self.objectMappings  = {}
        self.objectHasColor  = {}

        # a dictionary of name objects and the corresponding names of
        # objects they are to control on the server side
        self.controlMappings = {}
        self.controlOffsets  = {}
        self.taggedObjects   = {}
        self.controlPriorities = {}
        self.sortedControlMappings = []

        for serverConfig in configList:
            server = DisplayConnection(
                self.qcm, serverConfig.serverName,
                serverConfig.serverMsgPort, self.msgHandler)
            if server == None:
                self.notify.error('Could not open %s on %s port %d' %
                                  (serverConfig.serverConfigName,
                                   serverConfig.serverName,
                                   serverConfig.serverMsgPort))
            else:
                self.notify.debug('send cam pos')
                #server.sendMoveCam(Point3(0), Vec3(0))
                self.notify.debug('send cam offset')
                server.sendCamOffset(serverConfig.xyz, serverConfig.hpr)
                if serverConfig.fFrustum:
                    self.notify.debug('send cam frustum')
                    server.sendCamFrustum(serverConfig.focalLength,
                                          serverConfig.filmSize,
                                          serverConfig.filmOffset)
                self.serverList.append(server)
                self.serverQueues.append([])
        self.notify.debug('pre startTimeTask')
        self.startSynchronizeTimeTask()
        self.notify.debug('pre startMoveCam')
        self.startMoveCamTask()
        self.notify.debug('post startMoveCam')
        self.startMoveSelectedTask()


    def startReaderPollTask(self):
        """ Task to handle datagrams from server """
        # Run this task just after the listener poll task
        taskMgr.add(self._readerPollTask, "clientReaderPollTask", -39)

    def _readerPollTask(self, state):
        """ Non blocking task to read all available datagrams """

        for i in range(len(self.serverList)):
            server = self.serverList[i]
            datagrams = server.poll()
            for data in datagrams:
                self.handleDatagram(data[0],data[1],i)

        return Task.cont


    def startControlObjectTask(self):
        self.notify.debug("moving control objects")
        taskMgr.add(self.controlObjectTask,"controlObjectTask",50)

    def startSynchronizeTimeTask(self):
        self.notify.debug('broadcasting frame time')
        taskMgr.add(self.synchronizeTimeTask, "synchronizeTimeTask", -40)

    def synchronizeTimeTask(self, task):
        frameCount = globalClock.getFrameCount()
        frameTime = globalClock.getFrameTime()
        dt = globalClock.getDt()
        for server in self.serverList:
            server.sendTimeData(frameCount, frameTime, dt)
        return Task.cont

    def startMoveCamTask(self):
        self.notify.debug('adding move cam')
        taskMgr.add(self.moveCameraTask, "moveCamTask", 49)


    def controlObjectTask(self, task):
        for pair in self.sortedControlMappings:
            object     = pair[1]
            name       = self.controlMappings[object][0]
            serverList = self.controlMappings[object][1]
            if (self.objectMappings.has_key(object)):
                self.moveObject(self.objectMappings[object],name,serverList,
                                self.controlOffsets[object], self.objectHasColor[object])
        self.sendNamedMovementDone()
        return Task.cont

    def sendNamedMovementDone(self, serverList = None):

        if (serverList == None):
            serverList = range(len(self.serverList))
        
        for server in serverList:
            self.serverList[server].sendNamedMovementDone()



    def redoSortedPriorities(self):

        self.sortedControlMappings = []
        for key in self.controlMappings:
            self.sortedControlMappings.append([self.controlPriorities[key],
                                               key])

        self.sortedControlMappings.sort()
        
    def moveObject(self, nodePath, object, serverList, offset, hasColor = True):
        self.notify.debug('moving object '+object)
        xyz = nodePath.getPos(render) + offset
        hpr = nodePath.getHpr(render)
        scale = nodePath.getScale(render)
        hidden = nodePath.isHidden()
        if (hasColor):
            color = nodePath.getColor()
        else:
            color = [1,1,1,1]
        for server in serverList:
            self.serverList[server].sendMoveNamedObject(xyz,hpr,scale,color,hidden,object)


    def moveCameraTask(self, task):
        self.moveCamera(
            base.camera.getPos(render),
            base.camera.getHpr(render))
        return Task.cont

    def moveCamera(self, xyz, hpr):
        self.notify.debug('moving unsynced camera')
        for server in self.serverList:
            server.sendMoveCam(xyz, hpr)

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
                server.sendMoveSelected(xyz, hpr, scale)
        return Task.cont


    def addNamedObjectMapping(self,object,name,hasColor = True):
        if (not self.objectMappings.has_key(name)):
            self.objectMappings[name] = object
            self.objectHasColor[name] = hasColor
        else:
            self.notify.debug('attempt to add duplicate named object: '+name)

    def removeObjectMapping(self,name):
        if (self.objectMappings.has_key(name)):
            self.objectMappings.pop(name)


    def addControlMapping(self,objectName,controlledName, serverList = None,
                          offset = None, priority = 0):
        if (not self.controlMappings.has_key(objectName)):
            if (serverList == None):
                serverList = range(len(self.serverList))
            if (offset == None):
                offset = Vec3(0,0,0)
                
            self.controlMappings[objectName] = [controlledName,serverList]
            self.controlOffsets[objectName]  = offset
            self.controlPriorities[objectName] = priority
        else:
            oldList = self.controlMappings[objectName]
            mergedList = []
            for item in oldList:
                mergedList.append(item)
            for item in serverList:
                if (item not in mergedList):
                    mergedList.append(item)

        self.redoSortedPriorities()
            #self.notify.debug('attempt to add duplicate controlled object: '+name)

    def setControlMappingOffset(self,objectName,offset):
        if (self.controlMappings.has_key(objectName)):
            self.controlOffsets[objectName] = offset

    def removeControlMapping(self,name, serverList = None):
        if (self.controlMappings.has_key(name)):

            if (serverList == None):
                self.controlMappings.pop(name)
                self.controlPriorities.pop(name)                
            else:
                list = self.controlMappings[key][1]
                newList = []
                for server in list:
                    if (server not in serverList):
                        newList.append(server)
                self.controlMappings[key][1] = newList
                if (len(newList) == 0):
                    self.controlMappings.pop(name)
                    self.controlPriorities.pop(name)
        self.redoSortedPriorities()

        
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

    def getNodePathName(self, nodePath):
        import string
        pathString = `nodePath`
        index = string.find(pathString, '/')
        if index != -1:
            name = pathString[index+1:]
            return name
        else:
            return pathString


    def addObjectTag(self,object,selectFunction,deselectFunction,selectArgs,deselectArgs):
        newTag = {}
        newTag["selectFunction"] = selectFunction
        newTag["selectArgs"]     = selectArgs
        newTag["deselectFunction"] = deselectFunction
        newTag["deselectArgs"]     = deselectArgs        
        self.taggedObjects[object] = newTag


    def removeObjectTag(self,object):

        self.taggedObjects.pop(object)
    

    def selectNodePath(self, nodePath):
        name = self.getNodePathName(nodePath)
        if self.taggedObjects.has_key(name):
            taskMgr.remove("moveSelectedTask")
            tag = self.taggedObjects[name]
            function = tag["selectFunction"]
            args     = tag["selectArgs"]
            if (function != None):
                function(*args)
        else:
            self(self.getNodePathFindCmd(nodePath) + '.select()', 0)
        

    def deselectNodePath(self, nodePath):
        name = self.getNodePathName(nodePath)
        if self.taggedObjects.has_key(name):
            tag = self.taggedObjects[name]
            function = tag["deselectFunction"]
            args     = tag["deselectArgs"]        
            if (function != None):
                function(*args)
            self.startMoveSelectedTask()
        self(self.getNodePathFindCmd(nodePath) + '.deselect()', 0)

    def sendCamFrustum(self, focalLength, filmSize, filmOffset, indexList=[]):
        if indexList:
            serverList = map(lambda i: self.serverList[i], indexList)
        else:
            serverList = self.serverList
        for server in serverList:
            self.notify.debug('updating camera frustum')
            server.sendCamFrustum(focalLength, filmSize, filmOffset)

    def loadModel(self, nodePath):
        pass

    def __call__(self, commandString, fLocally = 1, serverList = []):
        # Execute remotely
        if serverList:
            # Passed in list of servers
            for serverNum in serverList:
                self.serverList[serverNum].sendCommandString(commandString)
        else:
            # All servers
            for server in self.serverList:
                server.sendCommandString(commandString)
        if fLocally:
            # Execute locally
            exec(commandString, __builtins__)


    def handleDatagram(self,dgi,type,server):
        if (type == CLUSTER_NONE):
            pass
        elif (type == CLUSTER_NAMED_OBJECT_MOVEMENT):
            self.serverQueues[server].append(self.msgHandler.parseNamedMovementDatagram(dgi))
            #self.handleNamedMovement(dgi)
        # when we recieve a 'named movement done' packet from a server we handle
        # all of its messages
        elif (type == CLUSTER_NAMED_MOVEMENT_DONE):
            self.handleMessageQueue(server)
        else:
            self.notify.warning("Received unsupported packet type:" % type)
        return type

    def handleMessageQueue(self,server):

        list = self.serverQueues[server]
        # handle all messages in the queue
        for data in list:
            #print dgi
            self.handleNamedMovement(data)

        # clear the queue
        self.serverQueues[server] = []
        

    def handleNamedMovement(self, data):
        """ Update cameraJig position to reflect latest position """
    
        (name,x, y, z, h, p, r, sx, sy, sz,red,g,b,a, hidden) = data 
        #print "name"
        #if (name == "camNode"):
        #    print x,y,z,h,p,r, sx, sy, sz,red,g,b,a, hidden
        if (self.objectMappings.has_key(name)):
            self.objectMappings[name].setPosHpr(render, x, y, z, h, p, r)
            self.objectMappings[name].setScale(render,sx,sy,sz)
            if (self.objectHasColor[name]):
                self.objectMappings[name].setColor(red,g,b,a)
            if (hidden):
                self.objectMappings[name].hide()
            else:
                self.objectMappings[name].show()
        else:
            self.notify.debug("recieved unknown named object command: "+name)


    def exit(self):
        # Execute remotely
        for server in self.serverList:
            server.sendExit()
        # Execute locally
        import sys
        sys.exit()


class ClusterClientSync(ClusterClient):
    def __init__(self, configList, clusterSyncFlag):
        ClusterClient.__init__(self, configList, clusterSyncFlag)
        #I probably don't need this
        self.waitForSwap = 0
        self.ready = 0
        print "creating synced client"
        self.startSwapCoordinatorTask()

    def startSwapCoordinatorTask(self):
        taskMgr.add(self.swapCoordinator, "clientSwapCoordinator", 51)

    def swapCoordinator(self, task):
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
            base.graphicsEngine.flipFrame()
            self.notify.debug(
                "------------------------------------------END swap")

        #print "syncing"
        return Task.cont

    def moveCamera(self, xyz, hpr):
        if self.ready:
            self.notify.debug('moving synced camera')
            ClusterClient.moveCamera(self, xyz, hpr)
            self.waitForSwap=1


class DisplayConnection:
    def __init__(self, qcm, serverName, port, msgHandler):
        self.msgHandler = msgHandler
        gameServerTimeoutMs = base.config.GetInt(
            "cluster-server-timeout-ms", 300000)
        # A giant 300 second timeout.
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



    def poll(self):
        """ Non blocking task to read all available datagrams """
        dataGrams = []
        while 1:
            (datagram, dgi, type) = self.msgHandler.nonBlockingRead(self.qcr)
            # Queue is empty, done for now
            if type is CLUSTER_NONE:
                break
            else:
                # Got a datagram, add it to the list
                dataGrams.append([dgi, type, datagram])

        return dataGrams




    def sendCamOffset(self, xyz, hpr):
        ClusterClient.notify.debug("send cam offset...")
        ClusterClient.notify.debug(("packet %d xyz, hpr=%f %f %f %f %f %f" %
             (self.msgHandler.packetNumber, xyz[0], xyz[1], xyz[2],
             hpr[0], hpr[1], hpr[2])))
        datagram = self.msgHandler.makeCamOffsetDatagram(xyz, hpr)
        self.cw.send(datagram, self.tcpConn)

    def sendCamFrustum(self, focalLength, filmSize, filmOffset):
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


    def sendNamedMovementDone(self):

        datagram = self.msgHandler.makeNamedMovementDone()
        self.cw.send(datagram, self.tcpConn)

    def sendMoveNamedObject(self, xyz, hpr, scale, color, hidden, name):
        ClusterClient.notify.debug("send named object move...")
        ClusterClient.notify.debug(("packet %d xyz, hpr=%f %f %f %f %f %f" %
             (self.msgHandler.packetNumber, xyz[0], xyz[1], xyz[2],
             hpr[0], hpr[1], hpr[2])))
        datagram = self.msgHandler.makeNamedObjectMovementDatagram(xyz,hpr,scale,
                                                                   color,hidden,
                                                                   name)
        self.cw.send(datagram, self.tcpConn)

    def sendMoveCam(self, xyz, hpr):
        ClusterClient.notify.debug("send cam move...")
        ClusterClient.notify.debug(("packet %d xyz, hpr=%f %f %f %f %f %f" %
             (self.msgHandler.packetNumber, xyz[0], xyz[1], xyz[2],
             hpr[0], hpr[1], hpr[2])))
        datagram = self.msgHandler.makeCamMovementDatagram(xyz, hpr)
        self.cw.send(datagram, self.tcpConn)

    def sendMoveSelected(self, xyz, hpr, scale):
        ClusterClient.notify.debug("send move selected...")
        ClusterClient.notify.debug(
            "packet %d xyz, hpr=%f %f %f %f %f %f %f %f %f" %
            (self.msgHandler.packetNumber,
             xyz[0], xyz[1], xyz[2],
             hpr[0], hpr[1], hpr[2],
             scale[0], scale[1], scale[2]))
        datagram = self.msgHandler.makeSelectedMovementDatagram(xyz, hpr, scale)
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

    def sendTimeData(self, frameCount, frameTime, dt):
        ClusterClient.notify.debug("send time data...")
        datagram = self.msgHandler.makeTimeDataDatagram(
            frameCount, frameTime, dt)
        self.cw.send(datagram, self.tcpConn)

class ClusterConfigItem:
    def __init__(self, serverConfigName, serverName,
                 serverDaemonPort, serverMsgPort):
        self.serverConfigName = serverConfigName
        self.serverName = serverName
        self.serverDaemonPort = serverDaemonPort
        self.serverMsgPort = serverMsgPort
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
            #lens.setInterocularDistance(pos[0])
            base.cam.setPos(pos)
            lens = base.cam.node().getLens()
            lens.setViewHpr(hpr)
            if fl is not None:
                lens.setFocalLength(fl)
            if fs is not None:
                lens.setFilmSize(fs[0], fs[1])
            if fo is not None:
                lens.setFilmOffset(fo[0], fo[1])
        else:
            serverConfigName = 'cluster-server-%s' % displayName
            serverName = base.config.GetString(serverConfigName, '')
            if serverName == '':
                base.notify.warning(
                    '%s undefined in Configrc: expected by %s display client.'%
                    (serverConfigName, clusterConfig))
                base.notify.warning('%s will not be used.' % serverConfigName)
            else:
                # Daemon port
                serverDaemonPortConfigName = (
                    'cluster-server-daemon-port-%s' % displayName)
                serverDaemonPort = base.config.GetInt(
                    serverDaemonPortConfigName,
                    CLUSTER_DAEMON_PORT)
                # TCP Server port
                serverMsgPortConfigName = (
                    'cluster-server-msg-port-%s' % displayName)
                serverMsgPort = base.config.GetInt(serverMsgPortConfigName,
                                                   CLUSTER_SERVER_PORT)
                cci = ClusterConfigItem(
                    serverConfigName,
                    serverName,
                    serverDaemonPort,
                    serverMsgPort)
                # Init cam offset
                cci.setCamOffset(pos, hpr)
                # Init frustum if specified
                if fl and fs and fo:
                    cci.setCamFrustum(fl, fs, fo)
                displayConfigs.append(cci)
    # Create Cluster Managers (opening connections to servers)
    # Are the servers going to be synced?
    if base.clusterSyncFlag:
        base.notify.warning('autoflip')
        base.graphicsEngine.setAutoFlip(0)
        base.notify.warning('ClusterClientSync')
        return ClusterClientSync(displayConfigs, base.clusterSyncFlag)
    else:
        return ClusterClient(displayConfigs, base.clusterSyncFlag)


class DummyClusterClient(DirectObject.DirectObject):
    """ Dummy class to handle command strings when not in cluster mode """
    notify = DirectNotifyGlobal.directNotify.newCategory("DummyClusterClient")
    def __init__(self):
        pass

    def __call__(self, commandString, fLocally = 1, serverList = None):
        if fLocally:
            # Execute locally
            exec(commandString, __builtins__)


