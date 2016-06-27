from panda3d.core import *
from .ClusterMsgs import *
from direct.distributed.MsgTypes import *
from direct.directnotify import DirectNotifyGlobal
from direct.showbase import DirectObject
from direct.task import Task

# NOTE: This assumes the following variables are set via bootstrap command line
# arguments on server startup:
#     clusterServerPort
#     clusterSyncFlag
#     clusterDaemonClient
#     clusterDaemonPort
# Also, I'm not sure multiple camera-group configurations are working for the
# cluster system.

class ClusterServer(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ClusterServer")
    MSG_NUM = 2000000

    def __init__(self, cameraJig, camera):
        global clusterServerPort, clusterSyncFlag
        global clusterDaemonClient, clusterDaemonPort
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
        self.cw = ConnectionWriter(self.qcm, 0)
        try:
            port = clusterServerPort
        except NameError:
            port = CLUSTER_SERVER_PORT
        self.tcpRendezvous = self.qcm.openTCPServerRendezvous(port, 1)
        self.qcl.addConnection(self.tcpRendezvous)
        self.msgHandler = ClusterMsgHandler(ClusterServer.MSG_NUM, self.notify)
        # Start cluster tasks
        self.startListenerPollTask()
        self.startReaderPollTask()
        # If synchronized server, start swap coordinator too
        try:
            clusterSyncFlag
        except NameError:
            clusterSyncFlag = 0
        if clusterSyncFlag:
            self.startSwapCoordinator()
            base.graphicsEngine.setAutoFlip(0)
        # Set global clock mode to slave mode
        globalClock.setMode(ClockObject.MSlave)
        # Send verification of startup to client
        self.daemon = DirectD()

        self.objectMappings  = {}
        self.objectHasColor  = {}
        self.controlMappings = {}
        self.controlPriorities = {}
        self.controlOffsets  = {}
        self.messageQueue    = []
        self.sortedControlMappings   = []

        # These must be passed in as bootstrap arguments and stored in
        # the __builtins__ namespace
        try:
            clusterDaemonClient
        except NameError:
            clusterDaemonClient = 'localhost'
        try:
            clusterDaemonPort
        except NameError:
            clusterDaemonPort = CLUSTER_DAEMON_PORT
        self.daemon.serverReady(clusterDaemonClient, clusterDaemonPort)



    def startListenerPollTask(self):
        # Run this task near the start of frame, sometime after the dataLoop
        taskMgr.add(self.listenerPollTask, "serverListenerPollTask", -40)

    def listenerPollTask(self, task):
        """ Task to listen for a new connection from the client """
        # Run this task after the dataLoop
        if self.qcl.newConnectionAvailable():
            self.notify.info("New connection is available")
            rendezvous = PointerToConnection()
            netAddress = NetAddress()
            newConnection = PointerToConnection()
            if self.qcl.getNewConnection(rendezvous, netAddress, newConnection):
                # Crazy dereferencing
                newConnection=newConnection.p()
                self.qcr.addConnection(newConnection)
                self.lastConnection = newConnection
                self.notify.info("Got a connection!")
            else:
                self.notify.warning("getNewConnection returned false")
        return Task.cont


    def addNamedObjectMapping(self,object,name,hasColor = True,
                              priority = 0):
        if (name not in self.objectMappings):
            self.objectMappings[name] = object
            self.objectHasColor[name] = hasColor
        else:
            self.notify.debug('attempt to add duplicate named object: '+name)

    def removeObjectMapping(self,name):
        if (name in self.objectMappings):
            self.objectMappings.pop(name)


    def redoSortedPriorities(self):

        self.sortedControlMappings = []
        for key in self.objectMappings:
            self.sortedControlMappings.append([self.controlPriorities[key],
                                               key])

        self.sortedControlMappings.sort()


    def addControlMapping(self,objectName,controlledName, offset = None,
                          priority = 0):
        if (objectName not in self.controlMappings):
            self.controlMappings[objectName] = controlledName
            if (offset == None):
                offset = Vec3(0,0,0)
            self.controlOffsets[objectName]  = offset
            self.controlPriorities[objectName] = priority
            self.redoSortedPriorities()
        else:
            self.notify.debug('attempt to add duplicate controlled object: '+name)

    def setControlMappingOffset(self,objectName,offset):
        if (objectName in self.controlMappings):
            self.controlOffsets[objectName] = offset


    def removeControlMapping(self,name):
        if (name in self.controlMappings):
            self.controlMappings.pop(name)
            self.controlPriorities.pop(name)
        self.redoSortedPriorities()


    def startControlObjectTask(self):
        self.notify.debug("moving control objects")
        taskMgr.add(self.controlObjectTask,"controlObjectTask",50)

    def controlObjectTask(self, task):
        #print "running control object task"
        for pair in self.sortedControlPriorities:
            object = pair[1]
            name   = self.controlMappings[object]
            if (object in self.objectMappings):
                self.moveObject(self.objectMappings[object],name,self.controlOffsets[object],
                                self.objectHasColor[object])

        self.sendNamedMovementDone()
        return Task.cont


    def sendNamedMovementDone(self):

        self.notify.debug("named movement done")
        datagram = self.msgHandler.makeNamedMovementDone()
        self.cw.send(datagram,self.lastConnection)

    def moveObject(self, nodePath, object, offset, hasColor):
        self.notify.debug('moving object '+object)
        #print "moving object",object
        xyz = nodePath.getPos(render) + offset
        hpr = nodePath.getHpr(render)
        scale = nodePath.getScale(render)
        if (hasColor):
            color = nodePath.getColor()
        else:
            color = [1,1,1,1]
        hidden = nodePath.isHidden()
        datagram = self.msgHandler.makeNamedObjectMovementDatagram(xyz,hpr,scale,color,hidden,object)
        self.cw.send(datagram, self.lastConnection)

    def startReaderPollTask(self):
        """ Task to handle datagrams from client """
        # Run this task just after the listener poll task
        if clusterSyncFlag:
            # Sync version
            taskMgr.add(self._syncReaderPollTask, "serverReaderPollTask", -39)
        else:
            # Asynchronous version
            taskMgr.add(self._readerPollTask, "serverReaderPollTask", -39)

    def _readerPollTask(self, state):
        """ Non blocking task to read all available datagrams """
        while 1:
            (datagram, dgi, type) = self.msgHandler.nonBlockingRead(self.qcr)
            # Queue is empty, done for now
            if type is CLUSTER_NONE:
                break
            else:
                # Got a datagram, handle it
                self.handleDatagram(dgi, type)
        return Task.cont

    def _syncReaderPollTask(self, task):
        if self.lastConnection is None:
            pass
        elif self.qcr.isConnectionOk(self.lastConnection):
            # Process datagrams till you get a postion update
            type = CLUSTER_NONE
            while type != CLUSTER_CAM_MOVEMENT:
                # Block until you get a new datagram
                (datagram, dgi, type) = self.msgHandler.blockingRead(self.qcr)
                # Process datagram
                self.handleDatagram(dgi, type)
        return Task.cont

    def startSwapCoordinator(self):
        taskMgr.add(self.swapCoordinatorTask, "serverSwapCoordinator", 51)

    def swapCoordinatorTask(self, task):
        if self.fPosReceived:
            self.fPosReceived = 0
            # Alert client that this server is ready to swap
            self.sendSwapReady()
            # Wait for swap command (processing any intermediate datagrams)
            while 1:
                (datagram, dgi, type) = self.msgHandler.blockingRead(self.qcr)
                self.handleDatagram(dgi, type)
                if type == CLUSTER_SWAP_NOW:
                    break
        return Task.cont

    def sendSwapReady(self):
        self.notify.debug(
            'send swap ready packet %d' % self.msgHandler.packetNumber)
        datagram = self.msgHandler.makeSwapReadyDatagram()
        self.cw.send(datagram, self.lastConnection)

    def handleDatagram(self, dgi, type):
        """ Process a datagram depending upon type flag """
        if (type == CLUSTER_NONE):
            pass
        elif (type == CLUSTER_EXIT):
            print('GOT EXIT')
            import sys
            sys.exit()
        elif (type == CLUSTER_CAM_OFFSET):
            self.handleCamOffset(dgi)
        elif (type == CLUSTER_CAM_FRUSTUM):
            self.handleCamFrustum(dgi)
        elif (type == CLUSTER_CAM_MOVEMENT):
            self.handleCamMovement(dgi)
        elif (type == CLUSTER_SELECTED_MOVEMENT):
            self.handleSelectedMovement(dgi)
        elif (type == CLUSTER_COMMAND_STRING):
            self.handleCommandString(dgi)
        elif (type == CLUSTER_SWAP_READY):
            pass
        elif (type == CLUSTER_SWAP_NOW):
            self.notify.debug('swapping')
            base.graphicsEngine.flipFrame()
        elif (type == CLUSTER_TIME_DATA):
            self.notify.debug('time data')
            self.handleTimeData(dgi)
        elif (type == CLUSTER_NAMED_OBJECT_MOVEMENT):
            self.messageQueue.append(self.msgHandler.parseNamedMovementDatagram(dgi))
            #self.handleNamedMovement(dgi)
        elif (type == CLUSTER_NAMED_MOVEMENT_DONE):
            #print "got done",self.messageQueue
            #if (len(self.messageQueue) > 0):
            #    print self.messageQueue[0]
            #    print dir(self.messageQueue)
            self.handleMessageQueue()
        else:
            self.notify.warning("Received unknown packet type:" % type)
        return type

    # Server specific tasks
    def handleCamOffset(self, dgi):
        """ Set offset of camera from cameraJig """
        (x, y, z, h, p, r) = self.msgHandler.parseCamOffsetDatagram(dgi)
        self.camera.setPos(x,y,z)
        self.lens.setViewHpr(h, p, r)

    def handleCamFrustum(self, dgi):
        """ Adjust camera frustum based on parameters sent by client """
        (fl, fs, fo) = self.msgHandler.parseCamFrustumDatagram(dgi)
        self.lens.setFocalLength(fl)
        self.lens.setFilmSize(fs[0], fs[1])
        self.lens.setFilmOffset(fo[0], fo[1])

    def handleNamedMovement(self, data):
        """ Update cameraJig position to reflect latest position """
        (name,x, y, z, h, p, r,sx,sy,sz, red, g, b, a, hidden) = data
        if (name in self.objectMappings):
            self.objectMappings[name].setPosHpr(render, x, y, z, h, p, r)
            self.objectMappings[name].setScale(render,sx,sy,sz)
            self.objectMappings[name].setColor(red,g,b,a)
            if (hidden):
                self.objectMappings[name].hide()
            else:
                self.objectMappings[name].show()
        else:
            self.notify.debug("recieved unknown named object command: "+name)


    def handleMessageQueue(self):

        #print self.messageQueue
        for data in self.messageQueue:
            #print "in queue",dgi
            self.handleNamedMovement(data)

        self.messageQueue = []

    def handleCamMovement(self, dgi):
        """ Update cameraJig position to reflect latest position """
        (x, y, z, h, p, r) = self.msgHandler.parseCamMovementDatagram(dgi)
        self.cameraJig.setPosHpr(render, x, y, z, h, p, r)
        self.fPosReceived = 1

    def handleSelectedMovement(self, dgi):
        """ Update cameraJig position to reflect latest position """
        (x, y, z, h, p, r, sx, sy, sz) = self.msgHandler.parseSelectedMovementDatagram(
            dgi)
        if last:
            last.setPosHprScale(x, y, z, h, p, r, sx, sy, sz)

    def handleTimeData(self, dgi):
        """ Update cameraJig position to reflect latest position """
        (frameCount, frameTime, dt) = self.msgHandler.parseTimeDataDatagram(dgi)
        # Use frame time from client for both real and frame time
        globalClock.setFrameCount(frameCount)
        globalClock.setFrameTime(frameTime)
        globalClock.setDt(dt)

    def handleCommandString(self, dgi):
        """ Handle arbitrary command string from client """
        command = self.msgHandler.parseCommandStringDatagram(dgi)
        try:
            exec(command, __builtins__)
        except:
            pass


