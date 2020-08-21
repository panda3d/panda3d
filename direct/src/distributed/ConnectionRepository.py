from panda3d.core import *
from panda3d.direct import *
from direct.task import Task
from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.distributed.DoInterestManager import DoInterestManager
from direct.distributed.DoCollectionManager import DoCollectionManager
from direct.showbase import GarbageReport
from .PyDatagramIterator import PyDatagramIterator

import inspect
import gc

__all__ = ["ConnectionRepository", "GCTrigger"]

class ConnectionRepository(
        DoInterestManager, DoCollectionManager, CConnectionRepository):
    """
    This is a base class for things that know how to establish a
    connection (and exchange datagrams) with a gameserver.  This
    includes ClientRepository and AIRepository.
    """
    notify = directNotify.newCategory("ConnectionRepository")
    taskPriority = -30
    taskChain = None

    CM_HTTP=0
    CM_NET=1
    CM_NATIVE=2

    gcNotify = directNotify.newCategory("GarbageCollect")

    GarbageCollectTaskName = "allowGarbageCollect"
    GarbageThresholdTaskName = "adjustGarbageCollectThreshold"

    def __init__(self, connectMethod, config, hasOwnerView = False,
                 threadedNet = None):
        assert self.notify.debugCall()
        if threadedNet is None:
            # Default value.
            threadedNet = config.GetBool('threaded-net', False)

        # let the C connection repository know whether we're supporting
        # 'owner' views of distributed objects (i.e. 'receives ownrecv',
        # 'I own this object and have a separate view of it regardless of
        # where it currently is located')
        CConnectionRepository.__init__(self, hasOwnerView, threadedNet)
        self.setWantMessageBundling(config.GetBool('want-message-bundling', 1))
        # DoInterestManager.__init__ relies on CConnectionRepository being
        # initialized
        DoInterestManager.__init__(self)
        DoCollectionManager.__init__(self)
        self.setPythonRepository(self)

        # Create a unique ID number for each ConnectionRepository in
        # the world, helpful for sending messages specific to each one.
        self.uniqueId = hash(self)

        # Accept this hook so that we can respond to lost-connection
        # events in the main thread, instead of within the network
        # thread (if there is one).
        self.accept(self._getLostConnectionEvent(), self.lostConnection)

        self.config = config

        if self.config.GetBool('verbose-repository'):
            self.setVerbose(1)

        # Set this to 'http' to establish a connection to the server
        # using the HTTPClient interface, which ultimately uses the
        # OpenSSL socket library (even though SSL is not involved).
        # This is not as robust a socket library as NET's, but the
        # HTTPClient interface does a good job of negotiating the
        # connection over an HTTP proxy if one is in use.
        #
        # Set it to 'net' to use Panda's net interface
        # (e.g. QueuedConnectionManager, etc.) to establish the
        # connection.  This is a higher-level layer build on top of
        # the low-level "native net" library.  There is no support for
        # proxies.  This is a good, general choice.
        #
        # Set it to 'native' to use Panda's low-level native net
        # interface directly.  This is much faster than either http or
        # net for high-bandwidth (e.g. server) applications, but it
        # doesn't support the simulated delay via the start_delay()
        # call.
        #
        # Set it to 'default' to use an appropriate interface
        # according to the type of ConnectionRepository we are
        # creating.
        userConnectMethod = self.config.GetString('connect-method', 'default')
        if userConnectMethod == 'http':
            connectMethod = self.CM_HTTP
        elif userConnectMethod == 'net':
            connectMethod = self.CM_NET
        elif userConnectMethod == 'native':
            connectMethod = self.CM_NATIVE

        self.connectMethod = connectMethod
        if self.connectMethod == self.CM_HTTP:
            self.notify.info("Using connect method 'http'")
        elif self.connectMethod == self.CM_NET:
            self.notify.info("Using connect method 'net'")
        elif self.connectMethod == self.CM_NATIVE:
            self.notify.info("Using connect method 'native'")

        self.connectHttp = None
        self.http = None

        # This DatagramIterator is constructed once, and then re-used
        # each time we read a datagram.
        self.private__di = PyDatagramIterator()

        self.recorder = None
        self.readerPollTaskObj = None

        # This is the string that is appended to symbols read from the
        # DC file.  The AIRepository will redefine this to 'AI'.
        self.dcSuffix = ''

        self._serverAddress = ''

        if self.config.GetBool('gc-save-all', 1):
            # set gc to preserve every object involved in a cycle, even ones that
            # would normally be freed automatically during garbage collect
            # allows us to find and fix these cycles, reducing or eliminating the
            # need to run garbage collects
            # garbage collection CPU usage is O(n), n = number of Python objects
            gc.set_debug(gc.DEBUG_SAVEALL)

        if self.config.GetBool('want-garbage-collect-task', 1):
            # manual garbage-collect task
            taskMgr.add(self._garbageCollect, self.GarbageCollectTaskName, 200)
            # periodically increase gc threshold if there is no garbage
            taskMgr.doMethodLater(self.config.GetFloat('garbage-threshold-adjust-delay', 5 * 60.),
                                  self._adjustGcThreshold, self.GarbageThresholdTaskName)

        self._gcDefaultThreshold = gc.get_threshold()

    def _getLostConnectionEvent(self):
        return self.uniqueName('lostConnection')

    def _garbageCollect(self, task=None):
        # allow a collect
        # enable automatic garbage collection
        gc.enable()
        # creating an object with gc enabled causes garbage collection to trigger if appropriate
        gct = GCTrigger()
        # disable the automatic garbage collect during the rest of the frame
        gc.disable()
        return Task.cont

    def _adjustGcThreshold(self, task):
        # do an unconditional collect to make sure gc.garbage has a chance to be
        # populated before we start increasing the auto-collect threshold
        # don't distribute the leak check from the client to the AI, they both
        # do these garbage checks independently over time
        numGarbage = GarbageReport.checkForGarbageLeaks()
        if numGarbage == 0:
            self.gcNotify.debug('no garbage found, doubling gc threshold')
            a, b, c = gc.get_threshold()
            gc.set_threshold(min(a * 2, 1 << 30), b, c)

            task.delayTime = task.delayTime * 2
            retVal = Task.again

        else:
            self.gcNotify.warning('garbage found, reverting gc threshold')
            # the process is producing garbage, stick to the default collection threshold
            gc.set_threshold(*self._gcDefaultThreshold)
            retVal = Task.done

        return retVal

    def generateGlobalObject(self, doId, dcname, values=None):
        def applyFieldValues(distObj, dclass, values):
            for i in range(dclass.getNumInheritedFields()):
                field = dclass.getInheritedField(i)
                if field.asMolecularField() == None:
                    value = values.get(field.getName(), None)
                    if value is None and field.isRequired():
                        # Gee, this could be better.  What would really be
                        # nicer is to get value from field.getDefaultValue
                        # or similar, but that returns a binary string, not
                        # a python tuple, like the following does.  If you
                        # want to change something better, please go ahead.
                        packer = DCPacker()
                        packer.beginPack(field)
                        packer.packDefaultValue()
                        packer.endPack()

                        unpacker = DCPacker()
                        unpacker.setUnpackData(packer.getString())
                        unpacker.beginUnpack(field)
                        value = unpacker.unpackObject()
                        unpacker.endUnpack()
                    if value is not None:
                        function = getattr(distObj, field.getName())
                        if function is not None:
                            function(*value)
                        else:
                            self.notify.error("\n\n\nNot able to find %s.%s"%(
                                distObj.__class__.__name__, field.getName()))

        # Look up the dclass
        dclass = self.dclassesByName.get(dcname+self.dcSuffix)
        if dclass is None:
            #print "\n\n\nNeed to define", dcname+self.dcSuffix
            self.notify.warning("Need to define %s" % (dcname+self.dcSuffix))
            dclass = self.dclassesByName.get(dcname+'AI')
        if dclass is None:
            dclass = self.dclassesByName.get(dcname)
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
        if values is not None:
            applyFieldValues(distObj, dclass, values)
        distObj.announceGenerate()
        distObj.parentId = 0
        distObj.zoneId = 0
        # updateRequiredFields calls announceGenerate
        return  distObj

    def readDCFile(self, dcFileNames = None):
        """
        Reads in the dc files listed in dcFileNames, or if
        dcFileNames is None, reads in all of the dc files listed in
        the Config.prc file.
        """

        dcFile = self.getDcFile()
        dcFile.clear()
        self.dclassesByName = {}
        self.dclassesByNumber = {}
        self.hashVal = 0

        if isinstance(dcFileNames, str):
            # If we were given a single string, make it a list.
            dcFileNames = [dcFileNames]

        dcImports = {}
        if dcFileNames == None:
            readResult = dcFile.readAll()
            if not readResult:
                self.notify.error("Could not read dc file.")
        else:
            searchPath = getModelPath().getValue()
            for dcFileName in dcFileNames:
                pathname = Filename(dcFileName)
                vfs = VirtualFileSystem.getGlobalPtr()
                vfs.resolveFilename(pathname, searchPath)
                readResult = dcFile.read(pathname)
                if not readResult:
                    self.notify.error("Could not read dc file: %s" % (pathname))

        #if not dcFile.allObjectsValid():
        #    names = []
        #    for i in range(dcFile.getNumTypedefs()):
        #        td = dcFile.getTypedef(i)
        #        if td.isBogusTypedef():
        #            names.append(td.getName())
        #    nameList = ', '.join(names)
        #    self.notify.error("Undefined types in DC file: " + nameList)

        self.hashVal = dcFile.getHash()

        # Now import all of the modules required by the DC file.
        for n in range(dcFile.getNumImportModules()):
            moduleName = dcFile.getImportModule(n)[:]

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
                self.notify.debug("No class definition for %s." % (className))
            else:
                if inspect.ismodule(classDef):
                    if not hasattr(classDef, className):
                        self.notify.warning("Module %s does not define class %s." % (className, className))
                        continue
                    classDef = getattr(classDef, className)

                if not inspect.isclass(classDef):
                    self.notify.error("Symbol %s is not a class name." % (className))
                else:
                    dclass.setClassDef(classDef)

            self.dclassesByName[className] = dclass
            if number >= 0:
                self.dclassesByNumber[number] = dclass

        # Owner Views
        if self.hasOwnerView():
            ownerDcSuffix = self.dcSuffix + 'OV'
            # dict of class names (without 'OV') that have owner views
            ownerImportSymbols = {}

            # Now import all of the modules required by the DC file.
            for n in range(dcFile.getNumImportModules()):
                moduleName = dcFile.getImportModule(n)

                # Maybe the module name is represented as "moduleName/AI".
                suffix = moduleName.split('/')
                moduleName = suffix[0]
                suffix=suffix[1:]
                if ownerDcSuffix in suffix:
                    moduleName = moduleName + ownerDcSuffix

                importSymbols = []
                for i in range(dcFile.getNumImportSymbols(n)):
                    symbolName = dcFile.getImportSymbol(n, i)

                    # Check for the OV suffix
                    suffix = symbolName.split('/')
                    symbolName = suffix[0]
                    suffix=suffix[1:]
                    if ownerDcSuffix in suffix:
                        symbolName += ownerDcSuffix
                    importSymbols.append(symbolName)
                    ownerImportSymbols[symbolName] = None

                self.importModule(dcImports, moduleName, importSymbols)

            # Now get the class definition for the owner classes named
            # in the DC file.
            for i in range(dcFile.getNumClasses()):
                dclass = dcFile.getClass(i)
                if ((dclass.getName()+ownerDcSuffix) in ownerImportSymbols):
                    number = dclass.getNumber()
                    className = dclass.getName() + ownerDcSuffix

                    # Does the class have a definition defined in the newly
                    # imported namespace?
                    classDef = dcImports.get(className)
                    if classDef is None:
                        self.notify.error("No class definition for %s." % className)
                    else:
                        if inspect.ismodule(classDef):
                            if not hasattr(classDef, className):
                                self.notify.error("Module %s does not define class %s." % (className, className))
                            classDef = getattr(classDef, className)
                        dclass.setOwnerClassDef(classDef)
                        self.dclassesByName[className] = dclass

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
                    raise Exception('Symbol %s not defined in module %s.' % (symbolName, moduleName))
        else:
            # "import moduleName"

            # Copy the root module name into the dictionary.

            # Follow the dotted chain down to the actual module.
            components = moduleName.split('.')
            dcImports[components[0]] = module

    def getServerAddress(self):
        return self._serverAddress

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

        #Redefine the connection to http or net in the default case

        self.bootedIndex = None
        self.bootedText = None
        if self.connectMethod == self.CM_HTTP:
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
        elif self.connectMethod == self.CM_NET or (not hasattr(self,"connectNative")):
            # Try each of the servers in turn.
            for url in serverList:
                self.notify.info("Connecting to %s via NET interface." % (url))
                if self.tryConnectNet(url):
                    self.startReaderPollTask()
                    if successCallback:
                        successCallback(*successArgs)
                    return

            # Failed to connect.
            if failureCallback:
                failureCallback(0, '', *failureArgs)
        elif self.connectMethod == self.CM_NATIVE:
            for url in serverList:
                self.notify.info("Connecting to %s via Native interface." % (url))
                if self.connectNative(url):
                    self.startReaderPollTask()
                    if successCallback:
                        successCallback(*successArgs)
                    return

            # Failed to connect.
            if failureCallback:
                failureCallback(0, '', *failureArgs)
        else:
            print("uh oh, we aren't using one of the tri-state CM variables")
            failureCallback(0, '', *failureArgs)

    def disconnect(self):
        """
        Closes the previously-established connection.
        """
        self.notify.info("Closing connection to server.")
        self._serverAddress = ''
        CConnectionRepository.disconnect(self)
        self.stopReaderPollTask()

    def shutdown(self):
        self.ignoreAll()
        CConnectionRepository.shutdown(self)

    def httpConnectCallback(self, ch, serverList, serverIndex,
                            successCallback, successArgs,
                            failureCallback, failureArgs):
        if ch.isConnectionReady():
            self.setConnectionHttp(ch)
            self._serverAddress = serverList[serverIndex-1]
            self.notify.info("Successfully connected to %s." % (self._serverAddress))

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
            self.notify.info("Connecting to %s via HTTP interface." % (url))
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
        self.readerPollTaskObj = taskMgr.add(
            self.readerPollUntilEmpty, self.uniqueName("readerPollTask"),
            priority = self.taskPriority, taskChain = self.taskChain)

    def stopReaderPollTask(self):
        if self.readerPollTaskObj:
            taskMgr.remove(self.readerPollTaskObj)
            self.readerPollTaskObj = None
        self.ignore(CConnectionRepository.getOverflowEventName())

    def readerPollUntilEmpty(self, task):
        while self.readerPollOnce():
            pass
        return Task.cont

    def readerPollOnce(self):
        if self.checkDatagram():
            self.getDatagramIterator(self.private__di)
            self.handleDatagram(self.private__di)
            return 1

        # Unable to receive a datagram: did we lose the connection?
        if not self.isConnected():
            self.stopReaderPollTask()
            messenger.send(self.uniqueName('lostConnection'), taskChain = 'default')
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
        # Zero-length datagrams might freak out the server.  No point
        # in sending them, anyway.
        if datagram.getLength() > 0:
##             if self.notify.getDebug():
##                 print "ConnectionRepository sending datagram:"
##                 datagram.dumpHex(ostream)

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

    def uniqueName(self, idString):
        return ("%s-%s" % (idString, self.uniqueId))

class GCTrigger:
    # used to trigger garbage collection
    pass
