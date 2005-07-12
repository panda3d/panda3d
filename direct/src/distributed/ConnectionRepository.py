from pandac.PandaModules import *
from direct.task import Task
from direct.directnotify import DirectNotifyGlobal
from direct.distributed.DoInterestManager import DoInterestManager
from direct.distributed.DoCollectionManager import DoCollectionManager
from PyDatagram import PyDatagram
from PyDatagramIterator import PyDatagramIterator

import types
import imp

class ConnectionRepository(
        DoInterestManager, DoCollectionManager, CConnectionRepository):
    """
    This is a base class for things that know how to establish a
    connection (and exchange datagrams) with a gameserver.  This
    includes ClientRepository and AIRepository.
    """
    notify = DirectNotifyGlobal.directNotify.newCategory("ConnectionRepository")
    taskPriority = -30

    def __init__(self, config):
        assert self.notify.debugCall()
        DoInterestManager.__init__(self)
        DoCollectionManager.__init__(self)
        CConnectionRepository.__init__(self)
        self.setPythonRepository(self)

        self.config = config

        # Set this to 'http' to establish a connection to the server
        # using the HTTPClient interface, which ultimately uses the
        # OpenSSL socket library (even though SSL is not involved).
        # This is not as robust a socket library as NSPR's, but the
        # HTTPClient interface does a good job of negotiating the
        # connection over an HTTP proxy if one is in use.
        #
        # Set it to 'nspr' to use Panda's net interface
        # (e.g. QueuedConnectionManager, etc.) to establish the
        # connection, which ultimately uses the NSPR socket library.
        # This is a much better socket library, but it may be more
        # than you need for most applications; and there is no support
        # for proxies.
        #
        # Set it to 'default' to use the HTTPClient interface if a
        # proxy is in place, but the NSPR interface if we don't have a
        # proxy.
        self.connectMethod = self.config.GetString('connect-method', 'default')
        self.connectHttp = None
        self.http = None

        # This DatagramIterator is constructed once, and then re-used
        # each time we read a datagram.
        self.__di = PyDatagramIterator()

        self.recorder = None

        # This is the string that is appended to symbols read from the
        # DC file.  The AIRepository will redefine this to 'AI'.
        self.dcSuffix = ''

    if wantGlobalManagers:
        def generateGlobalObject(self, doId, dcname):
            # Look up the dclass
            dclass = self.dclassesByName[dcname+self.dcSuffix]
            # Create a new distributed object, and put it in the dictionary
            #distObj = self.generateWithRequiredFields(dclass, doId, di)

            # Construct a new one
            classDef = dclass.getClassDef()
            if classDef == None:
                self.notify.error("Could not create an undefined %s object."%(
                    dclass.getName()))
            distObj = classDef(self)
            distObj.dclass = dclass
            # Assign it an Id
            distObj.doId = doId
            # Put the new do in the dictionary
            self.doId2do[doId] = distObj
            # Update the required fields
            distObj.generateInit()  # Only called when constructed
            distObj.generate()
            ## if wantOtpServer:
                ## # TODO: ROGER: where should we get parentId and zoneId?
                ## parentId = None
                ## zoneId = None
                ## distObj.setLocation(parentId, zoneId)
            # updateRequiredFields calls announceGenerate
            return  distObj

        ## def generateGlobalObject(self, doId, dcname, parentId=None, zoneId=None):
            ## assert self.notify.debugStateCall(self)
            ## # Look up the dclass
            ## dclass = self.dclassesByName[dcname]
            ## # Construct a new one
            ## classDef = dclass.getClassDef()
            ## if classDef == None:
                ## self.notify.error("Could not create an undefined %s object."%(
                    ## dclass.getName()))
            ## distObj = classDef(self)
            ## distObj.dclass = dclass
            
            ## assert not hasattr(self, 'parentId')
            ## self.doId = doId
            ## self.parentId = parentId
            ## self.zoneId = zoneId
            ## # Put the new DO in the dictionaries
            ## self.air.addDOToTables(self, location=(parentId,zoneId))

            ## assert not hasattr(self, 'parentId')
            ## self.parentId = parentId
            ## self.zoneId = zoneId
            ## distObj.generateInit()  # Only called when constructed
            ## self.generate()
            ## return  distObj

    def readDCFile(self, dcFileNames = None):
        """
        Reads in the dc files listed in dcFileNames, or if
        dcFileNames is None, reads in all of the dc files listed in
        the Configrc file.
        """
        dcFile = self.getDcFile()
        dcFile.clear()
        self.dclassesByName = {}
        self.dclassesByNumber = {}
        self.hashVal = 0

        dcImports = {}
        if dcFileNames == None:
            readResult = dcFile.readAll()
            if not readResult:
                self.notify.error("Could not read dc file.")
        else:
            for dcFileName in dcFileNames:
                readResult = dcFile.read(Filename(dcFileName))
                if not readResult:
                    self.notify.error("Could not read dc file: %s" % (dcFileName))

        if not dcFile.allObjectsValid():
            names = []
            for i in range(dcFile.getNumTypedefs()):
                td = dcFile.getTypedef(i)
                if td.isBogusTypedef():
                    names.append(td.getName())
            nameList = ', '.join(names)
            self.notify.error("Undefined types in DC file: " + nameList)
            
        self.hashVal = dcFile.getHash()

        # Now import all of the modules required by the DC file.
        for n in range(dcFile.getNumImportModules()):
            moduleName = dcFile.getImportModule(n)

            # Maybe the module name is represented as "moduleName/AI".
            suffix = moduleName.split('/')
            moduleName = suffix[0]
            suffix=suffix[1:]
            if self.dcSuffix in suffix:
                moduleName += self.dcSuffix
            elif self.dcSuffix == 'UD' and 'AI' in suffix: #HACK:
                moduleName += 'AI'

            importSymbols = []
            for i in range(dcFile.getNumImportSymbols(n)):
                symbolName = dcFile.getImportSymbol(n, i)

                # Maybe the symbol name is represented as "symbolName/AI".
                suffix = symbolName.split('/')
                symbolName = suffix[0]
                suffix=suffix[1:]
                if self.dcSuffix in suffix:
                    symbolName += self.dcSuffix
                elif self.dcSuffix == 'UD' and 'AI' in suffix: #HACK:
                    symbolName += 'AI'

                importSymbols.append(symbolName)

            self.importModule(dcImports, moduleName, importSymbols)

        # Now get the class definition for the classes named in the DC
        # file.
        for i in range(dcFile.getNumClasses()):
            dclass = dcFile.getClass(i)
            number = dclass.getNumber()
            className = dclass.getName() + self.dcSuffix

            # Does the class have a definition defined in the newly
            # imported namespace?
            classDef = dcImports.get(className)
            if classDef is None and self.dcSuffix == 'UD': #HACK:
                className = dclass.getName() + 'AI'
                classDef = dcImports.get(className)

            # Also try it without the dcSuffix.
            if classDef == None:
                className = dclass.getName()
                classDef = dcImports.get(className)
            if classDef is None:
                self.notify.info("No class definition for %s." % (className))
            else:
                if type(classDef) == types.ModuleType:
                    if not hasattr(classDef, className):
                        self.notify.error("Module %s does not define class %s." % (className, className))
                    classDef = getattr(classDef, className)

                # rhh this seems to fail with new system not sure why ?
                #print "---classname " +className
                #print type(classDef)
                #print types.ClassType
                #if type(classDef) != types.ClassType:
                #    self.notify.error("Symbol %s is not a class name." % (className))
                #else:
                #    dclass.setClassDef(classDef)
                dclass.setClassDef(classDef)
            self.dclassesByName[className] = dclass
            if number >= 0:
                self.dclassesByNumber[number] = dclass

    def importModule(self, dcImports, moduleName, importSymbols):
        """
        Imports the indicated moduleName and all of its symbols
        into the current namespace.  This more-or-less reimplements
        the Python import command.
        """
        module = __import__(moduleName, globals(), locals(), importSymbols)

        if importSymbols:
            # "from moduleName import symbolName, symbolName, ..."
            # Copy just the named symbols into the dictionary.
            if importSymbols == ['*']:
                # "from moduleName import *"
                if hasattr(module, "__all__"):
                    importSymbols = module.__all__
                else:
                    importSymbols = module.__dict__.keys()

            for symbolName in importSymbols:
                if hasattr(module, symbolName):
                    dcImports[symbolName] = getattr(module, symbolName)
                else:
                    raise StandardError, 'Symbol %s not defined in module %s.' % (symbolName, moduleName)
        else:
            # "import moduleName"

            # Copy the root module name into the dictionary.

            # Follow the dotted chain down to the actual module.
            components = moduleName.split('.')
            dcImports[components[0]] = module

    def connect(self, serverList,
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

        ## if self.recorder and self.recorder.isPlaying():

        ##     # If we have a recorder and it's already in playback mode,
        ##     # don't actually attempt to connect to a gameserver since
        ##     # we don't need to.  Just let it play back the data.
        ##     self.notify.info("Not connecting to gameserver; using playback data instead.")

        ##     self.connectHttp = 1
        ##     self.tcpConn = SocketStreamRecorder()
        ##     self.recorder.addRecorder('gameserver', self.tcpConn)

        ##     self.startReaderPollTask()
        ##     if successCallback:
        ##         successCallback(*successArgs)
        ##     return

        hasProxy = 0
        if self.checkHttp():
            proxies = self.http.getProxiesForUrl(serverList[0])
            hasProxy = (proxies != 'DIRECT')

        if hasProxy:
            self.notify.info("Connecting to gameserver via proxy list: %s" % (proxies))
        else:
            self.notify.info("Connecting to gameserver directly (no proxy).")

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

            ch = self.http.makeChannel(0)
            self.httpConnectCallback(
                    ch, serverList, 0,
                    successCallback, successArgs,
                    failureCallback, failureArgs)
        else:
            # Try each of the servers in turn.
            for url in serverList:
                self.notify.info("Connecting to %s via NSPR interface." % (url.cStr()))
                if self.tryConnectNspr(url):
                    self.startReaderPollTask()
                    if successCallback:
                        successCallback(*successArgs)
                    return

            # Failed to connect.
            if failureCallback:
                failureCallback(0, '', *failureArgs)

    def disconnect(self):
        """
        Closes the previously-established connection.
        """
        self.notify.info("Closing connection to server.")
        CConnectionRepository.disconnect(self)
        self.stopReaderPollTask()

    def httpConnectCallback(self, ch, serverList, serverIndex,
                            successCallback, successArgs,
                            failureCallback, failureArgs):
        if ch.isConnectionReady():
            self.setConnectionHttp(ch)

            ## if self.recorder:
            ##     # If we have a recorder, we wrap the connect inside a
            ##     # SocketStreamRecorder, which will trap incoming data
            ##     # when the recorder is set to record mode.  (It will
            ##     # also play back data when the recorder is in playback
            ##     # mode, but in that case we never get this far in the
            ##     # code, since we just create an empty
            ##     # SocketStreamRecorder without actually connecting to
            ##     # the gameserver.)
            ##     stream = SocketStreamRecorder(self.tcpConn, 1)
            ##     self.recorder.addRecorder('gameserver', stream)

            ##     # In this case, we pass ownership of the original
            ##     # connection to the SocketStreamRecorder object.
            ##     self.tcpConn.userManagesMemory = 0
            ##     self.tcpConn = stream

            self.startReaderPollTask()
            if successCallback:
                successCallback(*successArgs)
        elif serverIndex < len(serverList):
            # No connection yet, but keep trying.

            url = serverList[serverIndex]
            self.notify.info("Connecting to %s via HTTP interface." % (url.cStr()))
            ch.preserveStatus()

            ch.beginConnectTo(DocumentSpec(url))
            ch.spawnTask(name = 'connect-to-server',
                         callback = self.httpConnectCallback,
                         extraArgs = [ch, serverList, serverIndex + 1,
                                      successCallback, successArgs,
                                      failureCallback, failureArgs])
        else:
            # No more servers to try; we have to give up now.
            if failureCallback:
                failureCallback(ch.getStatusCode(), ch.getStatusString(),
                                *failureArgs)

    def checkHttp(self):
        # Creates an HTTPClient, if possible, if we don't have one
        # already.  This might fail if the OpenSSL library isn't
        # available.  Returns the HTTPClient (also self.http), or None
        # if not set.

        if self.http == None:
            try:
                self.http = HTTPClient()
            except:
                pass

        return self.http

    def startReaderPollTask(self):
        # Stop any tasks we are running now
        self.stopReaderPollTask()
        self.accept(CConnectionRepository.getOverflowEventName(),
                    self.handleReaderOverflow)
        taskMgr.add(self.readerPollUntilEmpty, self.uniqueName("readerPollTask"),
                    priority = self.taskPriority)

    def stopReaderPollTask(self):
        taskMgr.remove(self.uniqueName("readerPollTask"))
        self.ignore(CConnectionRepository.getOverflowEventName())

    def readerPollUntilEmpty(self, task):
        while self.readerPollOnce():
            pass
        return Task.cont

    def readerPollOnce(self):
        if self.checkDatagram():
            self.getDatagramIterator(self.__di)
            self.handleDatagram(self.__di)
            return 1

        # Unable to receive a datagram: did we lose the connection?
        if not self.isConnected():
            self.stopReaderPollTask()
            self.lostConnection()
        return 0

    def handleReaderOverflow(self):
        # this is called if the incoming-datagram queue overflowed and
        # we lost some data. Override and handle if desired.
        pass

    def lostConnection(self):
        # This should be overrided by a derived class to handle an
        # unexpectedly lost connection to the gameserver.
        self.notify.warning("Lost connection to gameserver.")

    def handleDatagram(self, di):
        # This class is meant to be pure virtual, and any classes that
        # inherit from it need to make their own handleDatagram method
        pass

    def send(self, datagram):
        if self.notify.getDebug():
            print "ConnectionRepository sending datagram:"
            datagram.dumpHex(ostream)

        self.sendDatagram(datagram)



    # debugging funcs for simulating a network-plug-pull
    def pullNetworkPlug(self):
        self.notify.warning('*** SIMULATING A NETWORK-PLUG-PULL ***')
        self.setSimulatedDisconnect(1)

    def networkPlugPulled(self):
        return self.getSimulatedDisconnect()

    def restoreNetworkPlug(self):
        if self.networkPlugPulled():
            self.notify.info('*** RESTORING SIMULATED PULLED-NETWORK-PLUG ***')
            self.setSimulatedDisconnect(0)
