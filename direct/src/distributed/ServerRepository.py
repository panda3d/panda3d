"""ServerRepository module: contains the ServerRepository class"""

from pandac.PandaModules import *
#from TaskManagerGlobal import *
from direct.distributed.MsgTypes import *
from direct.task import Task
from direct.directnotify import DirectNotifyGlobal
from direct.distributed.PyDatagram import PyDatagram
from direct.distributed.PyDatagramIterator import PyDatagramIterator
import time
import types

class ServerRepository:

    """ This maintains the server-side connection with a Panda server.
    It is only for use with the Panda LAN server provided by CMU."""

    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepository")

    def __init__(self, tcpPort, udpPort, dcFileNames = None):
        self.qcm = QueuedConnectionManager()
        self.qcl = QueuedConnectionListener(self.qcm, 0)
        self.qcr = QueuedConnectionReader(self.qcm, 0)
        self.cw = ConnectionWriter(self.qcm, 0)
        self.tcpRendezvous = self.qcm.openTCPServerRendezvous(tcpPort, 10)
        print self.tcpRendezvous
        self.qcl.addConnection(self.tcpRendezvous)
        taskMgr.add(self.listenerPoll, "serverListenerPollTask")
        taskMgr.add(self.readerPollUntilEmpty, "serverReaderPollTask")
        taskMgr.add(self.clientHardDisconnectTask, "clientHardDisconnect")
        self.ClientIP = {}
        self.ClientZones = {}
        self.ClientDOIDbase = {}
        self.ClientObjects = {}
        self.DOIDnext = 1
        self.DOIDrange = 1000000
        self.DOIDtoClient = {}
        self.DOIDtoZones = {}
        self.DOIDtoDClass = {}
        self.ZonesToClients = {}
        self.ZonetoDOIDs = {}
        self.dcFile = DCFile()
        self.dcSuffix = ''
        self.readDCFile(dcFileNames)

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
                    raise StandardError, 'Symbol %s not defined in module %s.' % (symbolName, moduleName)

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
            for dcFileName in dcFileNames:
                readResult = dcFile.read(Filename(dcFileName))
                if not readResult:
                    self.notify.error("Could not read dc file: %s" % (dcFileName))

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
                self.notify.info("No class definition for %s." % (className))
            else:
                if type(classDef) == types.ModuleType:
                    if not hasattr(classDef, className):
                        self.notify.error("Module %s does not define class %s." % (className, className))
                    classDef = getattr(classDef, className)

                if type(classDef) != types.ClassType and type(classDef) != types.TypeType:
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
            if retVal:
                # Crazy dereferencing
                newConnection=newConnection.p()
                self.qcr.addConnection(newConnection)
                #  Add clients infomation to dictionary
                self.ClientIP[newConnection] = netAddress.getIpString()
                self.ClientZones[newConnection] = []
                self.ClientObjects[newConnection] = []
                self.lastConnection = newConnection
                self.sendDOIDrange(self.lastConnection)
            else:
                self.notify.warning(
                    "getNewConnection returned false")
        return Task.cont

# continuously polls for new messages on the server

    def readerPollUntilEmpty(self, task):
        while self.readerPollOnce():
            pass
        return Task.cont

# checks for available messages to the server

    def readerPollOnce(self):
        availGetVal = self.qcr.dataAvailable()
        if availGetVal:
            datagram = NetDatagram()
            readRetVal = self.qcr.getData(datagram)
            if readRetVal:
                # need to send to message processing unit
                self.handleDatagram(datagram)
            else:
                self.notify.warning("getData returned false")
        return availGetVal

# switching station for messages

    def handleDatagram(self, datagram):
        dgi = DatagramIterator(datagram)
        type = dgi.getUint16()

        if type == CLIENT_DISCONNECT:
            self.handleClientDisconnect(datagram.getConnection())
        elif type == CLIENT_SET_ZONE_CMU:
            self.handleSetZone(dgi, datagram.getConnection())
        elif type == CLIENT_REMOVE_ZONE:
            self.handleRemoveZone(dgi, datagram.getConnection())
        elif type == CLIENT_CREATE_OBJECT_REQUIRED:
            self.handleClientCreateObjectRequired(datagram, dgi)
        elif type == CLIENT_OBJECT_UPDATE_FIELD:
            self.handleClientUpdateField(datagram, dgi)
        elif type == CLIENT_OBJECT_DELETE:
            self.handleClientDeleteObject(datagram, dgi.getUint32())
        elif type == CLIENT_OBJECT_DISABLE:
            self.handleClientDisable(datagram, dgi.getUint32())
        else:
            self.handleMessageType(type, dgi)

    def handleMessageType(self, msgType, di):
        self.notify.error("unrecognized message")

# client wants to create an object, so we store appropriate data,
# and then pass message along to corresponding zones

    def handleClientCreateObjectRequired(self, datagram, dgi):
        connection = datagram.getConnection()
        # no need to create a new message, just forward the received
        # message as it has the same msg type number
        zone    = dgi.getUint32()
        classid = dgi.getUint16()
        doid    = dgi.getUint32()
        rest    = dgi.getRemainingBytes()
        datagram = NetDatagram()
        datagram.addUint16(CLIENT_CREATE_OBJECT_REQUIRED)
        datagram.addUint16(classid)
        datagram.addUint32(doid)
        datagram.appendData(rest)
        dclass = self.dclassesByNumber[classid]
        if self.ClientObjects[connection].count(doid) == 0:
            self.ClientObjects[connection].append(doid)
        self.DOIDtoZones[doid] = zone
        self.DOIDtoDClass[doid] = dclass
        if zone in self.ZonetoDOIDs:
            if self.ZonetoDOIDs[zone].count(doid)==0:
                self.ZonetoDOIDs[zone].append(doid)
        else:
            self.ZonetoDOIDs[zone] = [doid]
        self.sendToZoneExcept(zone, datagram, connection)


    # client wants to update an object, forward message along
    # to corresponding zone

    def handleClientUpdateField(self, datagram, dgi):
        connection = datagram.getConnection()
        doid = dgi.getUint32()
        fieldid = dgi.getUint16()
        dclass = self.DOIDtoDClass[doid]
        dcfield = dclass.getFieldByIndex(fieldid)
        if dcfield == None:
          self.notify.error(
              "Received update for field %s on object %s; no such field for class %s." % (
              fieldid, doid, dclass.getName()))
          return
        if (dcfield.hasKeyword('broadcast')):

          if (dcfield.hasKeyword('p2p')):
            self.sendToZoneExcept(self.DOIDtoZones[doid], datagram, 0)
          else:
            self.sendToZoneExcept(self.DOIDtoZones[doid], datagram, connection)
        elif (dcfield.hasKeyword('p2p')):
          doidbase = (doid / self.DOIDrange) * self.DOIDrange
          self.cw.send(datagram, self.DOIDtoClient[doidbase])
        else:
          self.notify.warning(
                "Message is not broadcast, p2p, or broadcast+p2p")

    # client disables an object, let everyone know who is in
    # that zone know about it

    def handleClientDisable(self, datagram, doid):
        # now send disable message to all clients that need to know
        if doid in self.DOIDtoZones:
            self.sendToZoneExcept(self.DOIDtoZones[doid], datagram, 0)

    # client deletes an object, let everyone who is in zone with
    # object know about it

    def handleClientDeleteObject(self, datagram, doid):
        if doid in self.DOIDtoZones:
            self.sendToZoneExcept(self.DOIDtoZones[doid], datagram, 0)
            self.ClientObjects[datagram.getConnection()].remove(doid)
            self.ZonetoDOIDs[self.DOIDtoZones[doid]].remove(doid)
            del self.DOIDtoZones[doid]
            del self.DOIDtoDClass[doid]

    def sendAvatarGenerate(self):
        datagram = PyDatagram()
        # Message type is 1
        datagram.addUint16(ALL_OBJECT_GENERATE_WITH_REQUIRED)
        # Avatar class type is 2
        datagram.addUint8(2)
        # A sample id
        datagram.addUint32(10)
        # The only required field is the zone field
        datagram.addUint32(999)
        self.cw.send(datagram, self.lastConnection)

    #  sends the client the range of doid's that the client can use

    def sendDOIDrange(self, connection):
        # reuse DOID assignments if we can
        id = self.DOIDnext + self.DOIDrange
        self.DOIDnext = self.DOIDnext + self.DOIDrange
        self.DOIDtoClient[id] = connection
        self.ClientDOIDbase[connection] = id
        datagram = NetDatagram()
        datagram.addUint16(CLIENT_SET_DOID_RANGE)
        datagram.addUint32(id)
        datagram.addUint32(self.DOIDrange)
        print "Sending DOID range: ", id, self.DOIDrange
        self.cw.send(datagram, connection)

    # a client disconnected from us, we need to update our data, also tell other clients to remove
    # the disconnected clients objects
    def handleClientDisconnect(self, connection):
        if (self.ClientIP.has_key(connection)):
            del self.DOIDtoClient[self.ClientDOIDbase[connection]]
            for zone in self.ClientZones[connection]:
                if len(self.ZonesToClients[zone]) == 1:
                    del self.ZonesToClients[zone]
                else:
                    self.ZonesToClients[zone].remove(connection)
            for obj in self.ClientObjects[connection]:
                #create and send delete message
                datagram = NetDatagram()
                datagram.addUint16(CLIENT_OBJECT_DELETE_RESP)
                datagram.addUint32(obj)
                self.sendToZoneExcept(self.DOIDtoZones[obj], datagram, 0)
                self.ZonetoDOIDs[self.DOIDtoZones[obj]].remove(obj)
                del self.DOIDtoZones[obj]
                del self.DOIDtoDClass[obj]
            del self.ClientIP[connection]
            del self.ClientZones[connection]
            del self.ClientDOIDbase[connection]
            del self.ClientObjects[connection]

    #  client told us its zone(s), store information
    def handleSetZone(self, dgi, connection):
        while dgi.getRemainingSize() > 0:
            ZoneID = dgi.getUint32()
            if self.ClientZones[connection].count(ZoneID) == 0:
                self.ClientZones[connection].append(ZoneID)
            if ZoneID in self.ZonesToClients:
                if self.ZonesToClients[ZoneID].count(connection) == 0:
                    self.ZonesToClients[ZoneID].append(connection)
            else:
                self.ZonesToClients[ZoneID] = [connection]

            # We have a new member, need to get all of the data from clients who may have objects in this zone
            datagram = NetDatagram()
            datagram.addUint16(CLIENT_REQUEST_GENERATES)
            datagram.addUint32(ZoneID)
            self.sendToAll(datagram)
            print "SENDING REQUEST GENERATES (", ZoneID, ") TO ALL"

    # client has moved zones, need to update them
    def handleRemoveZone(self, dgi, connection):
        while dgi.getRemainingSize() > 0:
            ZoneID = dgi.getUint32()
            if self.ClientZones[connection].count(ZoneID) == 1:
                self.ClientZones[connection].remove(ZoneID)
            if ZoneID in self.ZonesToClients:
                if self.ZonesToClients[ZoneID].count(connection) == 1:
                    self.ZonesToClients[ZoneID].remove(connection)
            for i in self.ZonetoDOIDs[ZoneID]:
                datagram = NetDatagram()
                datagram.addUint16(CLIENT_OBJECT_DELETE)
                datagram.addUint32(i)
                self.cw.send(datagram, connection)

    # client did not tell us he was leaving but we lost connection to him, so we need to update our data and tell others

    def clientHardDisconnectTask(self, task):
        for i in self.ClientIP.keys():
            if not self.qcr.isConnectionOk(i):
                self.handleClientDisconnect(i)
        return Task.cont

    # sends a message to everyone who is in the zone
    def sendToZoneExcept(self, ZoneID, datagram, connection):
        if ZoneID in self.ZonesToClients:
            for conn in self.ZonesToClients[ZoneID]:
                if (conn != connection):
                    self.cw.send(datagram, conn)

    # sends a message to all connected clients
    def sendToAll(self, datagram):
        for client in self.ClientIP.keys():
            self.cw.send(datagram, client)
