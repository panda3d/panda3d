from PandaModules import *
import Task
import DirectNotifyGlobal
import DirectObject

class ConnectionRepository(DirectObject.DirectObject):
    """
    This is a base class for things that know how to establish a
    connection (and exchange datagrams) with a gameserver.  This
    includes ClientRepository and AIRepository.
    """
    
    notify = DirectNotifyGlobal.directNotify.newCategory("ConnectionRepository")
    taskPriority = -30


    def __init__(self, config):
        DirectObject.DirectObject.__init__(self)

        self.config = config
        
        # Set this to 'http' to establish a connection to the server
        # using the HTTPClient interface, which ultimately uses the
        # OpenSSL socket library (even though SSL is not involved).
        # This is not as robust a socket library as NSPR's, but the
        # HTTPClient interface does a good job of negotiating the
        # connection over an HTTP proxy if one is in use.

        # Set it to 'nspr' to use Panda's net interface
        # (e.g. QueuedConnectionManager, etc.) to establish the
        # connection, which ultimately uses the NSPR socket library.
        # This is a much better socket library, but it may be more
        # than you need for most applications; and the proxy support
        # is weak.

        # Set it to 'default' to use the HTTPClient interface if a
        # proxy is in place, but the NSPR interface if we don't have a
        # proxy.
        
        self.connectMethod = self.config.GetString('connect-method', 'default')
        self.connectHttp = None
        self.http = None
        self.qcm = None
        self.cw = None

        self.tcpConn = None

        # Reader statistics
        self.rsDatagramCount = 0
        self.rsUpdateObjs = {}
        self.rsLastUpdate = 0
        self.rsDoReport = self.config.GetBool('reader-statistics', 0)
        self.rsUpdateInterval = self.config.GetDouble('reader-statistics-interval', 10)


    def connect(self, serverList, allowProxy,
                successCallback = None, successArgs = [],
                failureCallback = None, failureArgs = []):
        """
        Attempts to establish a connection to the server.  May return
        before the connection is established.  The two callbacks
        represent the two functions to call (and their arguments) on
        success or failure, respectively.  The failure callback also
        gets one additional parameter, which will be passed in first:
        the return status code giving reason for failure, if it is
        known.
        """

        hasProxy = 0
        if allowProxy:
            if self.http == None:
                self.http = HTTPClient()
            proxies = self.http.getProxiesForUrl(serverList[0])
            hasProxy = (proxies != '')

        if hasProxy:
            self.notify.info("Connecting to gameserver via proxy: %s" % (proxies))
        else:
            self.notify.info("Connecting to gameserver directly (no proxy).");

        if self.connectMethod == 'http':
            self.connectHttp = 1
        elif self.connectMethod == 'nspr':
            self.connectHttp = 0
        else:
            self.connectHttp = (hasProxy or serverList[0].isSsl())

        self.bootedIndex = None
        self.bootedText = None
        if self.connectHttp:
            # In the HTTP case, we can't just iterate through the list
            # of servers, because each server attempt requires
            # spawning a request and then coming back later to check
            # the success or failure.  Instead, we start the ball
            # rolling by calling the connect callback, which will call
            # itself repeatedly until we establish a connection (or
            # run out of servers).

            if self.http == None:
                self.http = HTTPClient()

            ch = self.http.makeChannel(0)
            # Temporary try..except for old Pandas.
            try:
                ch.setAllowProxy(allowProxy)
            except:
                pass
            self.httpConnectCallback(ch, serverList, 0, hasProxy,
                                     successCallback, successArgs,
                                     failureCallback, failureArgs)

        else:
            if self.qcm == None:
                self.qcm = QueuedConnectionManager()

            if self.cw == None:
                self.cw = ConnectionWriter(self.qcm, 0)
                self.qcr = QueuedConnectionReader(self.qcm, 0)
                minLag = self.config.GetFloat('min-lag', 0.)
                maxLag = self.config.GetFloat('max-lag', 0.)
                if minLag or maxLag:
                    self.qcr.startDelay(minLag, maxLag)

            # A big old 20 second timeout.
            gameServerTimeoutMs = self.config.GetInt("game-server-timeout-ms",
                                                     20000)

            # Try each of the servers in turn.
            for url in serverList:
                self.notify.info("Connecting to %s via NSPR interface." % (url.cStr()))
                self.tcpConn = self.qcm.openTCPClientConnection(
                    url.getServer(), url.getPort(),
                    gameServerTimeoutMs)

                if self.tcpConn:
                    self.tcpConn.setNoDelay(1)
                    self.qcr.addConnection(self.tcpConn)
                    self.startReaderPollTask()
                    if successCallback:
                        successCallback(*successArgs)
                    return

            # Failed to connect.
            if failureCallback:
                failureCallback(hasProxy, 0, *failureArgs)

    def disconnect(self):
        """Closes the previously-established connection.
        """
        self.notify.info("Closing connection to server.")
        if self.tcpConn != None:
            if self.connectHttp:
                self.tcpConn.close()
            else: 
                self.qcm.closeConnection(self.tcpConn)
            self.tcpConn = None
        self.stopReaderPollTask()
                    
    def httpConnectCallback(self, ch, serverList, serverIndex, hasProxy,
                            successCallback, successArgs,
                            failureCallback, failureArgs):
        if ch.isConnectionReady():
            self.tcpConn = ch.getConnection()
            self.tcpConn.userManagesMemory = 1
            self.startReaderPollTask()
            if successCallback:
                successCallback(*successArgs)

        elif serverIndex < len(serverList):
            # No connection yet, but keep trying.
            
            url = serverList[serverIndex]
            self.notify.info("Connecting to %s via HTTP interface." % (url.cStr()))
            ch.beginConnectTo(DocumentSpec(url))
            ch.spawnTask(name = 'connect-to-server',
                         callback = self.httpConnectCallback,
                         extraArgs = [ch, serverList, serverIndex + 1,
                                      hasProxy,
                                      successCallback, successArgs,
                                      failureCallback, failureArgs])
        else:
            # No more servers to try; we have to give up now.
            if failureCallback:
                failureCallback(hasProxy, ch.getStatusCode(), *failureArgs)

    def startReaderPollTask(self):
        # Stop any tasks we are running now
        self.stopReaderPollTask()
        taskMgr.add(self.readerPollUntilEmpty, "readerPollTask",
                    priority = self.taskPriority)

    def stopReaderPollTask(self):
        taskMgr.remove("readerPollTask")

    def readerPollUntilEmpty(self, task):
        while self.readerPollOnce():
            pass
        return Task.cont

    def readerPollOnce(self):
        # we simulate the network plug being pulled by setting tcpConn
        # to None; enforce that condition
        if not self.tcpConn:
            return 0

        # Make sure any recently-sent datagrams are flushed when the
        # time expires, if we're in collect-tcp mode.
        self.tcpConn.considerFlush()

        if self.rsDoReport:
            self.reportReaderStatistics()
        
        if self.connectHttp:
            datagram = Datagram()
            if self.tcpConn.receiveDatagram(datagram):
                if self.rsDoReport:
                    self.rsDatagramCount += 1
                self.handleDatagram(datagram)
                return 1

            # Unable to receive a datagram: did we lose the connection?
            if self.tcpConn.isClosed():
                self.tcpConn = None
                self.stopReaderPollTask()
                self.lostConnection()
            return 0
        
        else:
            self.ensureValidConnection()
            if self.qcr.dataAvailable():
                datagram = NetDatagram()
                if self.qcr.getData(datagram):
                    if self.rsDoReport:
                        self.rsDatagramCount += 1
                    self.handleDatagram(datagram)
                    return 1
            return 0

    def ensureValidConnection(self):
        # Was the connection reset?
        if self.connectHttp:
            pass
        else:
            if self.qcm.resetConnectionAvailable():
                resetConnectionPointer = PointerToConnection()
                if self.qcm.getResetConnection(resetConnectionPointer):
                    resetConn = resetConnectionPointer.p()
                    self.qcm.closeConnection(resetConn)
                    # if we've simulated a network plug pull, restore the
                    # simulated plug
                    self.restoreNetworkPlug()
                    if self.tcpConn.this == resetConn.this:
                        self.tcpConn = None
                        self.stopReaderPollTask()
                        self.lostConnection()
                    else:
                        self.notify.warning("Lost unknown connection.")

    def lostConnection(self):
        # This should be overrided by a derived class to handle an
        # unexpectedly lost connection to the gameserver.
        self.notify.warning("Lost connection to gameserver.")

    def handleDatagram(self, datagram):
        # This class is meant to be pure virtual, and any classes that
        # inherit from it need to make their own handleDatagram method
        pass

    def reportReaderStatistics(self):
        now = globalClock.getRealTime()
        if now - self.rsLastUpdate < self.rsUpdateInterval:
            return

        self.rsLastUpdate = now
        self.notify.info("Received %s datagrams" % (self.rsDatagramCount))
        if self.rsUpdateObjs:
            self.notify.info("Updates: %s" % (self.rsUpdateObjs))

        self.rsDatagramCount = 0
        self.rsUpdateObjs = {}

    def send(self, datagram):
        if self.notify.getDebug():
            print "ConnectionRepository sending datagram:"
            datagram.dumpHex(ostream)

        if not self.tcpConn:
            self.notify.warning("Unable to send message after connection is closed.")
            return

        if self.connectHttp:
            if not self.tcpConn.sendDatagram(datagram):
                self.notify.warning("Could not send datagram.")
        else:
            self.cw.send(datagram, self.tcpConn)


    # debugging funcs for simulating a network-plug-pull
    def pullNetworkPlug(self):
        self.restoreNetworkPlug()
        self.notify.warning('*** SIMULATING A NETWORK-PLUG-PULL ***')
        self.hijackedTcpConn = self.tcpConn
        self.tcpConn = None

    def networkPlugPulled(self):
        return hasattr(self, 'hijackedTcpConn')

    def restoreNetworkPlug(self):
        if self.networkPlugPulled():
            self.notify.info('*** RESTORING SIMULATED PULLED-NETWORK-PLUG ***')
            self.tcpConn = self.hijackedTcpConn
            del self.hijackedTcpConn
