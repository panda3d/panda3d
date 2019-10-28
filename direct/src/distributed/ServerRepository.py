"""ServerRepository module: contains the ServerRepository class"""

from panda3d.core import *
from panda3d.direct import *
from direct.distributed.MsgTypesCMU import *
from direct.task import Task
from direct.directnotify import DirectNotifyGlobal
from direct.distributed.PyDatagram import PyDatagram

import inspect


class ServerRepository:

    """ This maintains the server-side connection with a Panda server.
    It is only for use with the Panda LAN server provided by CMU."""

    notify = DirectNotifyGlobal.directNotify.newCategory("ServerRepository")

    class Client:
        """ This internal class keeps track of the data associated
        with each connected client. """
        def __init__(self, connection, netAddress, doIdBase):
            # The connection used to communicate with the client.
            self.connection = connection

            # The net address to the client, including IP address.
            # Used for reporting purposes only.
            self.netAddress = netAddress

            # The first doId in the range assigned to the client.
            # This also serves as a unique numeric ID for this client.
            # (It is sometimes called "avatarId" in some update
            # messages, even though the client is not required to use
            # this particular number as an avatar ID.)
            self.doIdBase = doIdBase

            # The set of zoneIds that the client explicitly has
            # interest in.  The client will receive updates for all
            # distributed objects appearing in one of these zones.
            # (The client will also receive updates for all zones in
            # which any one of the distributed obejcts that it has
            # created still exist.)
            self.explicitInterestZoneIds = set()

            # The set of interest zones sent to the client at the last
            # update.  This is the actual set of zones the client is
            # informed of.  Changing the explicitInterestZoneIds,
            # above, creating or deleting objects in different zones,
            # or moving objects between zones, might influence this
            # set.
            self.currentInterestZoneIds = set()

            # A dictionary of doId -> Object, for distributed objects
            # currently in existence that were created by the client.
            self.objectsByDoId = {}

            # A dictionary of zoneId -> set([Object]), listing the
            # distributed objects assigned to each zone, of the
            # objects created by this client.
            self.objectsByZoneId = {}

    class Object:
        """ This internal class keeps track of the data associated
        with each extent distributed object. """
        def __init__(self, doId, zoneId, dclass):
            # The object's distributed ID.
            self.doId = doId

            # The object's current zone.  Each object is associated
            # with only one zone.
            self.zoneId = zoneId

            # The object's class type.
            self.dclass = dclass

            # Note that the server does not store any other data about
            # the distributed objects; in particular, it doesn't
            # record its current fields.  That is left to the clients.


    def __init__(self, tcpPort, serverAddress = None,
                 udpPort = None, dcFileNames = None,
                 threadedNet = None):
        if threadedNet is None:
            # Default value.
            threadedNet = config.GetBool('threaded-net', False)

        # Set up networking interfaces.
        numThreads = 0
        if threadedNet:
            numThreads = 1
        self.qcm = QueuedConnectionManager()
        self.qcl = QueuedConnectionListener(self.qcm, numThreads)
        self.qcr = QueuedConnectionReader(self.qcm, numThreads)
        self.cw = ConnectionWriter(self.qcm, numThreads)

        taskMgr.setupTaskChain('flushTask')
        if threadedNet:
            taskMgr.setupTaskChain('flushTask', numThreads = 1,
                                   threadPriority = TPLow, frameSync = True)

        self.tcpRendezvous = self.qcm.openTCPServerRendezvous(
            serverAddress or '', tcpPort, 10)
        self.qcl.addConnection(self.tcpRendezvous)
        taskMgr.add(self.listenerPoll, "serverListenerPollTask")
        taskMgr.add(self.readerPollUntilEmpty, "serverReaderPollTask")
        taskMgr.add(self.clientHardDisconnectTask, "clientHardDisconnect")

        # A set of clients that have recently been written to and may
        # need to be flushed.
        self.needsFlush = set()

        collectTcpInterval = ConfigVariableDouble('collect-tcp-interval').getValue()
        taskMgr.doMethodLater(collectTcpInterval, self.flushTask, 'flushTask',
                              taskChain = 'flushTask')

        # A dictionary of connection -> Client object, tracking all of
        # the clients we currently have connected.
        self.clientsByConnection = {}

        # A similar dictionary of doIdBase -> Client object, indexing
        # by the client's doIdBase number instead.
        self.clientsByDoIdBase = {}

        # A dictionary of zoneId -> set([Client]), listing the clients
        # that have an interest in each zoneId.
        self.zonesToClients = {}

        # A dictionary of zoneId -> set([Object]), listing the
        # distributed objects assigned to each zone, globally.
        self.objectsByZoneId = {}

        # The number of doId's to assign to each client.  Must remain
        # constant during server lifetime.
        self.doIdRange = base.config.GetInt('server-doid-range', 1000000)

        # An allocator object that assigns the next doIdBase to each
        # client.
        self.idAllocator = UniqueIdAllocator(0, 0xffffffff // self.doIdRange)

        self.dcFile = DCFile()
        self.dcSuffix = ''
        self.readDCFile(dcFileNames)

    def flushTask(self, task):
        """ This task is run periodically to flush any connections
        that might need it.  It's only necessary in cases where
        collect-tcp is set true (if this is false, messages are sent
        immediately and do not require periodic flushing). """

        flush = self.needsFlush
        self.needsFlush = set()
        for client in flush:
            client.connection.flush()

        return Task.again

    def setTcpHeaderSize(self, headerSize):
        """Sets the header size of TCP packets.  At the present, legal
        values for this are 0, 2, or 4; this specifies the number of
        bytes to use encode the datagram length at the start of each
        TCP datagram.  Sender and receiver must independently agree on
        this."""
        self.qcr.setTcpHeaderSize(headerSize)
        self.cw.setTcpHeaderSize(headerSize)

    def getTcpHeaderSize(self):
        """Returns the current setting of TCP header size. See
        setTcpHeaderSize(). """
        return self.qcr.getTcpHeaderSize()


    def importModule(self, dcImports, moduleName, importSymbols):
        """ Imports the indicated moduleName and all of its symbols
        into the current namespace.  This more-or-less reimplements
        the Python import command. """

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

    def readDCFile(self, dcFileNames = None):
        """
        Reads in the dc files listed in dcFileNames, or if
        dcFileNames is None, reads in all of the dc files listed in
        the Configrc file.
        """
        dcFile = self.dcFile
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
            searchPath = getModelPath().getValue()
            for dcFileName in dcFileNames:
                pathname = Filename(dcFileName)
                vfs.resolveFilename(pathname, searchPath)
                readResult = dcFile.read(pathname)
                if not readResult:
                    self.notify.error("Could not read dc file: %s" % (pathname))

        self.hashVal = dcFile.getHash()

        # Now import all of the modules required by the DC file.
        for n in range(dcFile.getNumImportModules()):
            moduleName = dcFile.getImportModule(n)

            # Maybe the module name is represented as "moduleName/AI".
            suffix = moduleName.split('/')
            moduleName = suffix[0]
            if self.dcSuffix and self.dcSuffix in suffix[1:]:
                moduleName += self.dcSuffix

            importSymbols = []
            for i in range(dcFile.getNumImportSymbols(n)):
                symbolName = dcFile.getImportSymbol(n, i)

                # Maybe the symbol name is represented as "symbolName/AI".
                suffix = symbolName.split('/')
                symbolName = suffix[0]
                if self.dcSuffix and self.dcSuffix in suffix[1:]:
                    symbolName += self.dcSuffix

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

            # Also try it without the dcSuffix.
            if classDef == None:
                className = dclass.getName()
                classDef = dcImports.get(className)

            if classDef == None:
                self.notify.debug("No class definition for %s." % (className))
            else:
                if inspect.ismodule(classDef):
                    if not hasattr(classDef, className):
                        self.notify.error("Module %s does not define class %s." % (className, className))
                    classDef = getattr(classDef, className)

                if not inspect.isclass(classDef):
                    self.notify.error("Symbol %s is not a class name." % (className))
                else:
                    dclass.setClassDef(classDef)

            self.dclassesByName[className] = dclass
            if number >= 0:
                self.dclassesByNumber[number] = dclass


# listens for new clients

    def listenerPoll(self, task):
        if self.qcl.newConnectionAvailable():
            rendezvous = PointerToConnection()
            netAddress = NetAddress()
            newConnection = PointerToConnection()
            retVal = self.qcl.getNewConnection(rendezvous, netAddress,
                                               newConnection)
            if not retVal:
                return Task.cont

            # Crazy dereferencing
            newConnection = newConnection.p()

            #  Add clients information to dictionary
            id = self.idAllocator.allocate()
            doIdBase = id * self.doIdRange + 1

            self.notify.info(
                "Got client %s from %s" % (doIdBase, netAddress))

            client = self.Client(newConnection, netAddress, doIdBase)
            self.clientsByConnection[client.connection] = client
            self.clientsByDoIdBase[client.doIdBase] = client

            # Now we can start listening to that new connection.
            self.qcr.addConnection(newConnection)

            self.lastConnection = newConnection
            self.sendDoIdRange(client)

        return Task.cont

    def readerPollUntilEmpty(self, task):
        """ continuously polls for new messages on the server """
        while self.readerPollOnce():
            pass
        return Task.cont

    def readerPollOnce(self):
        """ checks for available messages to the server """

        availGetVal = self.qcr.dataAvailable()
        if availGetVal:
            datagram = NetDatagram()
            readRetVal = self.qcr.getData(datagram)
            if readRetVal:
                # need to send to message processing unit
                self.handleDatagram(datagram)
        return availGetVal

    def handleDatagram(self, datagram):
        """ switching station for messages """

        client = self.clientsByConnection.get(datagram.getConnection())

        if not client:
            # This shouldn't be possible, though it appears to happen
            # sometimes?
            self.notify.warning(
                "Ignoring datagram from unknown connection %s" % (datagram.getConnection()))
            return

        if self.notify.getDebug():
            self.notify.debug(
                "ServerRepository received datagram from %s:" % (client.doIdBase))
            #datagram.dumpHex(ostream)

        dgi = DatagramIterator(datagram)

        type = dgi.getUint16()

        if type == CLIENT_DISCONNECT_CMU:
            self.handleClientDisconnect(client)
        elif type == CLIENT_SET_INTEREST_CMU:
            self.handleClientSetInterest(client, dgi)
        elif type == CLIENT_OBJECT_GENERATE_CMU:
            self.handleClientCreateObject(datagram, dgi)
        elif type == CLIENT_OBJECT_UPDATE_FIELD:
            self.handleClientObjectUpdateField(datagram, dgi)
        elif type == CLIENT_OBJECT_UPDATE_FIELD_TARGETED_CMU:
            self.handleClientObjectUpdateField(datagram, dgi, targeted = True)
        elif type == OBJECT_DELETE_CMU:
            self.handleClientDeleteObject(datagram, dgi.getUint32())
        elif type == OBJECT_SET_ZONE_CMU:
            self.handleClientObjectSetZone(datagram, dgi)
        else:
            self.handleMessageType(type, dgi)

    def handleMessageType(self, msgType, di):
        self.notify.warning("unrecognized message type %s" % (msgType))

    def handleClientCreateObject(self, datagram, dgi):
        """ client wants to create an object, so we store appropriate
        data, and then pass message along to corresponding zones """

        connection = datagram.getConnection()
        zoneId  = dgi.getUint32()
        classId = dgi.getUint16()
        doId    = dgi.getUint32()

        client = self.clientsByConnection[connection]

        if self.getDoIdBase(doId) != client.doIdBase:
            self.notify.warning(
                "Ignoring attempt to create invalid doId %s from client %s" % (doId, client.doIdBase))
            return

        dclass = self.dclassesByNumber[classId]

        object = client.objectsByDoId.get(doId)
        if object:
            # This doId is already in use; thus, this message is
            # really just an update.
            if object.dclass != dclass:
                self.notify.warning(
                    "Ignoring attempt to change object %s from %s to %s by client %s" % (
                    doId, object.dclass.getName(), dclass.getName(), client.doIdBase))
                return
            self.setObjectZone(client, object, zoneId)
        else:
            if self.notify.getDebug():
                self.notify.debug(
                    "Creating object %s of type %s by client %s" % (
                    doId, dclass.getName(), client.doIdBase))

            object = self.Object(doId, zoneId, dclass)
            client.objectsByDoId[doId] = object
            client.objectsByZoneId.setdefault(zoneId, set()).add(object)
            self.objectsByZoneId.setdefault(zoneId, set()).add(object)

            self.updateClientInterestZones(client)


        # Rebuild the new datagram that we'll send on.  We shim in the
        # doIdBase of the owner.
        dg = PyDatagram()
        dg.addUint16(OBJECT_GENERATE_CMU)
        dg.addUint32(client.doIdBase)
        dg.addUint32(zoneId)
        dg.addUint16(classId)
        dg.addUint32(doId)
        dg.appendData(dgi.getRemainingBytes())

        self.sendToZoneExcept(zoneId, dg, [client])

    def handleClientObjectUpdateField(self, datagram, dgi, targeted = False):
        """ Received an update request from a client. """
        connection = datagram.getConnection()
        client = self.clientsByConnection[connection]

        if targeted:
            targetId = dgi.getUint32()
        doId = dgi.getUint32()
        fieldId = dgi.getUint16()

        doIdBase = self.getDoIdBase(doId)
        owner = self.clientsByDoIdBase.get(doIdBase)
        object = owner and owner.objectsByDoId.get(doId)
        if not object:
            self.notify.warning(
                "Ignoring update for unknown object %s from client %s" % (
                doId, client.doIdBase))
            return

        dcfield = object.dclass.getFieldByIndex(fieldId)
        if dcfield == None:
            self.notify.warning(
                "Ignoring update for field %s on object %s from client %s; no such field for class %s." % (
                fieldId, doId, client.doIdBase, object.dclass.getName()))

        if client != owner:
            # This message was not sent by the object's owner.
            if not dcfield.hasKeyword('clsend') and not dcfield.hasKeyword('p2p'):
                self.notify.warning(
                    "Ignoring update for %s.%s on object %s from client %s: not owner" % (
                    object.dclass.getName(), dcfield.getName(), doId, client.doIdBase))
                return

        # We reformat the message slightly to insert the sender's
        # doIdBase.
        dg = PyDatagram()
        dg.addUint16(OBJECT_UPDATE_FIELD_CMU)
        dg.addUint32(client.doIdBase)
        dg.addUint32(doId)
        dg.addUint16(fieldId)
        dg.appendData(dgi.getRemainingBytes())

        if targeted:
            # A targeted update: only to the indicated client.
            target = self.clientsByDoIdBase.get(targetId)
            if not target:
                self.notify.warning(
                    "Ignoring targeted update to %s for %s.%s on object %s from client %s: target not known" % (
                    targetId,
                    dclass.getName(), dcfield.getName(), doId, client.doIdBase))
                return
            self.cw.send(dg, target.connection)
            self.needsFlush.add(target)

        elif dcfield.hasKeyword('p2p'):
            # p2p: to object owner only
            self.cw.send(dg, owner.connection)
            self.needsFlush.add(owner)

        elif dcfield.hasKeyword('broadcast'):
            # Broadcast: to everyone except orig sender
            self.sendToZoneExcept(object.zoneId, dg, [client])

        elif dcfield.hasKeyword('reflect'):
            # Reflect: broadcast to everyone including orig sender
            self.sendToZoneExcept(object.zoneId, dg, [])

        else:
            self.notify.warning(
                "Message is not broadcast or p2p")

    def getDoIdBase(self, doId):
        """ Given a doId, return the corresponding doIdBase.  This
        will be the owner of the object (clients may only create
        object doId's within their assigned range). """

        return int(doId / self.doIdRange) * self.doIdRange + 1

    def handleClientDeleteObject(self, datagram, doId):
        """ client deletes an object, let everyone who has interest in
        the object's zone know about it. """

        connection = datagram.getConnection()
        client = self.clientsByConnection[connection]
        object = client.objectsByDoId.get(doId)
        if not object:
            self.notify.warning(
                "Ignoring update for unknown object %s from client %s" % (
                doId, client.doIdBase))
            return

        self.sendToZoneExcept(object.zoneId, datagram, [])

        self.objectsByZoneId[object.zoneId].remove(object)
        if not self.objectsByZoneId[object.zoneId]:
            del self.objectsByZoneId[object.zoneId]
        client.objectsByZoneId[object.zoneId].remove(object)
        if not client.objectsByZoneId[object.zoneId]:
            del client.objectsByZoneId[object.zoneId]
        del client.objectsByDoId[doId]

        self.updateClientInterestZones(client)

    def handleClientObjectSetZone(self, datagram, dgi):
        """ The client is telling us the object is changing to a new
        zone. """
        doId = dgi.getUint32()
        zoneId = dgi.getUint32()

        connection = datagram.getConnection()
        client = self.clientsByConnection[connection]
        object = client.objectsByDoId.get(doId)
        if not object:
            # Don't know this object.
            self.notify.warning("Ignoring object location for %s: unknown" % (doId))
            return

        self.setObjectZone(client, object, zoneId)

    def setObjectZone(self, owner, object, zoneId):
        if object.zoneId == zoneId:
            # No change.
            return

        oldZoneId = object.zoneId
        self.objectsByZoneId[object.zoneId].remove(object)
        if not self.objectsByZoneId[object.zoneId]:
            del self.objectsByZoneId[object.zoneId]
        owner.objectsByZoneId[object.zoneId].remove(object)
        if not owner.objectsByZoneId[object.zoneId]:
            del owner.objectsByZoneId[object.zoneId]

        object.zoneId = zoneId
        self.objectsByZoneId.setdefault(zoneId, set()).add(object)
        owner.objectsByZoneId.setdefault(zoneId, set()).add(object)

        self.updateClientInterestZones(owner)

        # Any clients that are listening to oldZoneId but not zoneId
        # should receive a disable message: this object has just gone
        # out of scope for you.
        datagram = PyDatagram()
        datagram.addUint16(OBJECT_DISABLE_CMU)
        datagram.addUint32(object.doId)
        for client in self.zonesToClients[oldZoneId]:
            if client != owner:
                if zoneId not in client.currentInterestZoneIds:
                    self.cw.send(datagram, client.connection)
                    self.needsFlush.add(client)

        # The client is now responsible for sending a generate for the
        # object that just switched zones, to inform the clients that
        # are listening to the new zoneId but not the old zoneId.

    def sendDoIdRange(self, client):
        """ sends the client the range of doid's that the client can
        use """

        datagram = NetDatagram()
        datagram.addUint16(SET_DOID_RANGE_CMU)
        datagram.addUint32(client.doIdBase)
        datagram.addUint32(self.doIdRange)

        self.cw.send(datagram, client.connection)
        self.needsFlush.add(client)

    # a client disconnected from us, we need to update our data, also
    # tell other clients to remove the disconnected clients objects
    def handleClientDisconnect(self, client):
        for zoneId in client.currentInterestZoneIds:
            if len(self.zonesToClients[zoneId]) == 1:
                del self.zonesToClients[zoneId]
            else:
                self.zonesToClients[zoneId].remove(client)

        for object in client.objectsByDoId.values():
            #create and send delete message
            datagram = NetDatagram()
            datagram.addUint16(OBJECT_DELETE_CMU)
            datagram.addUint32(object.doId)
            self.sendToZoneExcept(object.zoneId, datagram, [])
            self.objectsByZoneId[object.zoneId].remove(object)
            if not self.objectsByZoneId[object.zoneId]:
                del self.objectsByZoneId[object.zoneId]

        client.objectsByDoId = {}
        client.objectsByZoneId = {}

        del self.clientsByConnection[client.connection]
        del self.clientsByDoIdBase[client.doIdBase]

        id = client.doIdBase / self.doIdRange
        self.idAllocator.free(id)

        self.qcr.removeConnection(client.connection)
        self.qcm.closeConnection(client.connection)


    def handleClientSetInterest(self, client, dgi):
        """ The client is specifying a particular set of zones it is
        interested in. """

        zoneIds = set()
        while dgi.getRemainingSize() > 0:
            zoneId = dgi.getUint32()
            zoneIds.add(zoneId)

        client.explicitInterestZoneIds = zoneIds
        self.updateClientInterestZones(client)

    def updateClientInterestZones(self, client):
        """ Something about the client has caused its set of interest
        zones to potentially change.  Recompute them. """

        origZoneIds = client.currentInterestZoneIds
        newZoneIds = client.explicitInterestZoneIds | set(client.objectsByZoneId.keys())
        if origZoneIds == newZoneIds:
            # No change.
            return

        client.currentInterestZoneIds = newZoneIds
        addedZoneIds = newZoneIds - origZoneIds
        removedZoneIds = origZoneIds - newZoneIds

        for zoneId in addedZoneIds:
            self.zonesToClients.setdefault(zoneId, set()).add(client)

            # The client is opening interest in this zone. Need to get
            # all of the data from clients who may have objects in
            # this zone
            datagram = NetDatagram()
            datagram.addUint16(REQUEST_GENERATES_CMU)
            datagram.addUint32(zoneId)
            self.sendToZoneExcept(zoneId, datagram, [client])

        datagram = PyDatagram()
        datagram.addUint16(OBJECT_DISABLE_CMU)
        for zoneId in removedZoneIds:
            self.zonesToClients[zoneId].remove(client)

            # The client is abandoning interest in this zone.  Any
            # objects in this zone should be disabled for the client.
            for object in self.objectsByZoneId.get(zoneId, []):
                datagram.addUint32(object.doId)
        self.cw.send(datagram, client.connection)

        self.needsFlush.add(client)


    def clientHardDisconnectTask(self, task):
        """ client did not tell us he was leaving but we lost connection to
        him, so we need to update our data and tell others """
        for client in self.clientsByConnection.values():
            if not self.qcr.isConnectionOk(client.connection):
                self.handleClientDisconnect(client)
        return Task.cont

    def sendToZoneExcept(self, zoneId, datagram, exceptionList):
        """sends a message to everyone who has interest in the
        indicated zone, except for the clients on exceptionList."""

        if self.notify.getDebug():
            self.notify.debug(
                "ServerRepository sending to all in zone %s except %s:" % (zoneId, [c.doIdBase for c in exceptionList]))
            #datagram.dumpHex(ostream)

        for client in self.zonesToClients.get(zoneId, []):
            if client not in exceptionList:
                if self.notify.getDebug():
                    self.notify.debug(
                        "  -> %s" % (client.doIdBase))
                self.cw.send(datagram, client.connection)
                self.needsFlush.add(client)

    def sendToAllExcept(self, datagram, exceptionList):
        """ sends a message to all connected clients, except for
        clients on exceptionList. """

        if self.notify.getDebug():
            self.notify.debug(
                "ServerRepository sending to all except %s:" % ([c.doIdBase for c in exceptionList],))
            #datagram.dumpHex(ostream)

        for client in self.clientsByConnection.values():
            if client not in exceptionList:
                if self.notify.getDebug():
                    self.notify.debug(
                        "  -> %s" % (client.doIdBase))
                self.cw.send(datagram, client.connection)
                self.needsFlush.add(client)
