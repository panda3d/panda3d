from pandac.PandaModules import *
from MsgTypes import *
from direct.showbase import ShowBase # __builtin__.config
from direct.task.TaskManagerGlobal import * # taskMgr
from direct.directnotify import DirectNotifyGlobal
from ConnectionRepository import ConnectionRepository
from PyDatagram import PyDatagram
from PyDatagramIterator import PyDatagramIterator
from AstronDatabaseInterface import AstronDatabaseInterface
from NetMessenger import NetMessenger
import collections

# Helper functions for logging output:
def msgpack_length(dg, length, fix, maxfix, tag8, tag16, tag32):
    if length < maxfix:
        dg.addUint8(fix + length)
    elif tag8 is not None and length < 1<<8:
        dg.addUint8(tag8)
        dg.addUint8(length)
    elif tag16 is not None and length < 1<<16:
        dg.addUint8(tag16)
        dg.addBeUint16(length)
    elif tag32 is not None and length < 1<<32:
        dg.addUint8(tag32)
        dg.addBeUint32(length)
    else:
        raise ValueError('Value too big for MessagePack')

def msgpack_encode(dg, element):
    if element == None:
        dg.addUint8(0xc0)
    elif element is False:
        dg.addUint8(0xc2)
    elif element is True:
        dg.addUint8(0xc3)
    elif isinstance(element, (int, long)):
        if -32 <= element < 128:
            dg.addInt8(element)
        elif 128 <= element < 256:
            dg.addUint8(0xcc)
            dg.addUint8(element)
        elif 256 <= element < 65536:
            dg.addUint8(0xcd)
            dg.addBeUint16(element)
        elif 65536 <= element < (1<<32):
            dg.addUint8(0xce)
            dg.addBeUint32(element)
        elif (1<<32) <= element < (1<<64):
            dg.addUint8(0xcf)
            dg.addBeUint64(element)
        elif -128 <= element < -32:
            dg.addUint8(0xd0)
            dg.addInt8(element)
        elif -32768 <= element < -128:
            dg.addUint8(0xd1)
            dg.addBeInt16(element)
        elif -1<<31 <= element < -32768:
            dg.addUint8(0xd2)
            dg.addBeInt32(element)
        elif -1<<63 <= element < -1<<31:
            dg.addUint8(0xd3)
            dg.addBeInt64(element)
        else:
            raise ValueError('int out of range for msgpack: %d' % element)
    elif isinstance(element, dict):
        msgpack_length(dg, len(element), 0x80, 0x10, None, 0xde, 0xdf)
        for k,v in element.items():
            msgpack_encode(dg, k)
            msgpack_encode(dg, v)
    elif isinstance(element, list):
        msgpack_length(dg, len(element), 0x90, 0x10, None, 0xdc, 0xdd)
        for v in element:
            msgpack_encode(dg, v)
    elif isinstance(element, basestring):
        msgpack_length(dg, len(element), 0xa0, 0x20, 0xd9, 0xda, 0xdb)
        dg.appendData(element)
    elif isinstance(element, float):
        # Python does not distinguish between floats and doubles, so we send
        # everything as a double in MsgPack:
        dg.addUint8(0xcb)
        dg.addFloat64(element)
    else:
        raise TypeError('Encountered non-MsgPack-packable value: %r' % element)

class AstronInternalRepository(ConnectionRepository):
    """
    This class is part of Panda3D's new MMO networking framework.
    It interfaces with an Astron (https://github.com/Astron/Astron) server in
    order to manipulate objects in the Astron cluster. It does not require any
    specific "gateway" into the Astron network. Rather, it just connects directly
    to any Message Director. Hence, it is an "internal" repository.

    This class is suitable for constructing your own AI Servers and UberDOG servers
    using Panda3D. Objects with a "self.air" attribute are referring to an instance
    of this class.
    """
    notify = DirectNotifyGlobal.directNotify.newCategory("AstronInternalRepository")

    def __init__(self, baseChannel, serverId=None, dcFileNames = None,
                 dcSuffix = 'AI', connectMethod = None, threadedNet = None):
        if connectMethod is None:
            connectMethod = self.CM_HTTP
        ConnectionRepository.__init__(self, connectMethod, config, hasOwnerView = False, threadedNet = threadedNet)
        self.setClientDatagram(False)
        self.dcSuffix = dcSuffix
        if hasattr(self, 'setVerbose'):
            if self.config.GetBool('verbose-internalrepository'):
                self.setVerbose(1)

        # The State Server we are configured to use for creating objects.
        #If this is None, generating objects is not possible.
        self.serverId = self.config.GetInt('air-stateserver', 0) or None
        if serverId is not None:
            self.serverId = serverId

        maxChannels = self.config.GetInt('air-channel-allocation', 1000000)
        self.channelAllocator = UniqueIdAllocator(baseChannel, baseChannel+maxChannels-1)
        self._registeredChannels = set()

        self.__contextCounter = 0

        self.netMessenger = NetMessenger(self)

        self.dbInterface = AstronDatabaseInterface(self)
        self.__callbacks = {}

        self.ourChannel = self.allocateChannel()

        self.eventLogId = self.config.GetString('eventlog-id', 'AIR:%d' % self.ourChannel)
        self.eventSocket = None
        eventLogHost = self.config.GetString('eventlog-host', '')
        if eventLogHost:
            if ':' in eventLogHost:
                host, port = eventLogHost.split(':', 1)
                self.setEventLogHost(host, int(port))
            else:
                self.setEventLogHost(eventLogHost)

        self.readDCFile(dcFileNames)

    def getContext(self):
        self.__contextCounter = (self.__contextCounter + 1) & 0xFFFFFFFF
        return self.__contextCounter

    def allocateChannel(self):
        """
        Allocate an unused channel out of this AIR's configured channel space.

        This is also used to allocate IDs for DistributedObjects, since those
        occupy a channel.
        """

        return self.channelAllocator.allocate()

    def deallocateChannel(self, channel):
        """
        Return the previously-allocated channel back to the allocation pool.
        """

        self.channelAllocator.free(channel)

    def registerForChannel(self, channel):
        """
        Register for messages on a specific Message Director channel.

        If the channel is already open by this AIR, nothing will happen.
        """

        if channel in self._registeredChannels:
            return
        self._registeredChannels.add(channel)

        dg = PyDatagram()
        dg.addServerControlHeader(CONTROL_ADD_CHANNEL)
        dg.addChannel(channel)
        self.send(dg)

    def unregisterForChannel(self, channel):
        """
        Unregister a channel subscription on the Message Director. The Message
        Director will cease to relay messages to this AIR sent on the channel.
        """

        if channel not in self._registeredChannels:
            return
        self._registeredChannels.remove(channel)

        dg = PyDatagram()
        dg.addServerControlHeader(CONTROL_REMOVE_CHANNEL)
        dg.addChannel(channel)
        self.send(dg)

    def addPostRemove(self, dg):
        """
        Register a datagram with the Message Director that gets sent out if the
        connection is ever lost.

        This is useful for registering cleanup messages: If the Panda3D process
        ever crashes unexpectedly, the Message Director will detect the socket
        close and automatically process any post-remove datagrams.
        """

        dg2 = PyDatagram()
        dg2.addServerControlHeader(CONTROL_ADD_POST_REMOVE)
        dg2.addUint64(self.ourChannel)
        dg2.addString(dg.getMessage())
        self.send(dg2)

    def clearPostRemove(self):
        """
        Clear all datagrams registered with addPostRemove.

        This is useful if the Panda3D process is performing a clean exit. It may
        clear the "emergency clean-up" post-remove messages and perform a normal
        exit-time clean-up instead, depending on the specific design of the game.
        """

        dg = PyDatagram()
        dg.addServerControlHeader(CONTROL_CLEAR_POST_REMOVES)
        dg.addUint64(self.ourChannel)
        self.send(dg)

    def handleDatagram(self, di):
        msgType = self.getMsgType()

        if msgType in (STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED,
                       STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED_OTHER):
            self.handleObjEntry(di, msgType == STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED_OTHER)
        elif msgType in (STATESERVER_OBJECT_CHANGING_AI,
                         STATESERVER_OBJECT_DELETE_RAM):
            self.handleObjExit(di)
        elif msgType == STATESERVER_OBJECT_CHANGING_LOCATION:
            self.handleObjLocation(di)
        elif msgType in (DBSERVER_CREATE_OBJECT_RESP,
                         DBSERVER_OBJECT_GET_ALL_RESP,
                         DBSERVER_OBJECT_GET_FIELDS_RESP,
                         DBSERVER_OBJECT_GET_FIELD_RESP,
                         DBSERVER_OBJECT_SET_FIELD_IF_EQUALS_RESP,
                         DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP):
            self.dbInterface.handleDatagram(msgType, di)
        elif msgType == DBSS_OBJECT_GET_ACTIVATED_RESP:
            self.handleGetActivatedResp(di)
        elif msgType >= 20000:
            # These messages belong to the NetMessenger:
            self.netMessenger.handle(msgType, di)
        else:
            self.notify.warning('Received message with unknown MsgType=%d' % msgType)

    def handleObjLocation(self, di):
        doId = di.getUint32()
        parentId = di.getUint32()
        zoneId = di.getUint32()

        do = self.doId2do.get(doId)

        if not do:
            self.notify.warning('Received location for unknown doId=%d!' % (doId))
            return

        do.setLocation(parentId, zoneId)

    def handleObjEntry(self, di, other):
        doId = di.getUint32()
        parentId = di.getUint32()
        zoneId = di.getUint32()
        classId = di.getUint16()

        if classId not in self.dclassesByNumber:
            self.notify.warning('Received entry for unknown dclass=%d! (Object %d)' % (classId, doId))
            return

        if doId in self.doId2do:
            return # We already know about this object; ignore the entry.

        dclass = self.dclassesByNumber[classId]

        do = dclass.getClassDef()(self)
        do.dclass = dclass
        do.doId = doId
        # The DO came in off the server, so we do not unregister the channel when
        # it dies:
        do.doNotDeallocateChannel = True
        self.addDOToTables(do, location=(parentId, zoneId))

        # Now for generation:
        do.generate()
        if other:
            do.updateAllRequiredOtherFields(dclass, di)
        else:
            do.updateAllRequiredFields(dclass, di)

    def handleObjExit(self, di):
        doId = di.getUint32()

        if doId not in self.doId2do:
            self.notify.warning('Received AI exit for unknown object %d' % (doId))
            return

        do = self.doId2do[doId]
        self.removeDOFromTables(do)
        do.delete()
        do.sendDeleteEvent()

    def handleGetActivatedResp(self, di):
        ctx = di.getUint32()
        doId = di.getUint32()
        activated = di.getUint8()

        if ctx not in self.__callbacks:
            self.notify.warning('Received unexpected DBSS_OBJECT_GET_ACTIVATED_RESP (ctx: %d)' %ctx)
            return

        try:
            self.__callbacks[ctx](doId, activated)
        finally:
            del self.__callbacks[ctx]

    def getActivated(self, doId, callback):
        ctx = self.getContext()
        self.__callbacks[ctx] = callback

        dg = PyDatagram()
        dg.addServerHeader(doId, self.ourChannel, DBSS_OBJECT_GET_ACTIVATED)
        dg.addUint32(ctx)
        dg.addUint32(doId)
        self.send(dg)

    def sendUpdate(self, do, fieldName, args):
        """
        Send a field update for the given object.

        You should use do.sendUpdate(...) instead. This is not meant to be
        called directly unless you really know what you are doing.
        """

        self.sendUpdateToChannel(do, do.doId, fieldName, args)

    def sendUpdateToChannel(self, do, channelId, fieldName, args):
        """
        Send an object field update to a specific channel.

        This is useful for directing the update to a specific client or node,
        rather than at the State Server managing the object.

        You should use do.sendUpdateToChannel(...) instead. This is not meant
        to be called directly unless you really know what you are doing.
        """

        dclass = do.dclass
        field = dclass.getFieldByName(fieldName)
        dg = field.aiFormatUpdate(do.doId, channelId, self.ourChannel, args)
        self.send(dg)

    def sendActivate(self, doId, parentId, zoneId, dclass=None, fields=None):
        """
        Activate a DBSS object, given its doId, into the specified parentId/zoneId.

        If both dclass and fields are specified, an ACTIVATE_WITH_DEFAULTS_OTHER
        will be sent instead. In other words, the specified fields will be
        auto-applied during the activation.
        """

        fieldPacker = DCPacker()
        fieldCount = 0
        if dclass and fields:
            for k,v in fields.items():
                field = dclass.getFieldByName(k)
                if not field:
                    self.notify.error('Activation request for %s object contains '
                                      'invalid field named %s' % (dclass.getName(), k))

                fieldPacker.rawPackUint16(field.getNumber())
                fieldPacker.beginPack(field)
                field.packArgs(fieldPacker, v)
                fieldPacker.endPack()
                fieldCount += 1

            dg = PyDatagram()
            dg.addServerHeader(doId, self.ourChannel, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS)
            dg.addUint32(doId)
            dg.addUint32(0)
            dg.addUint32(0)
            self.send(dg)
            # DEFAULTS_OTHER isn't implemented yet, so we chase it with a SET_FIELDS
            dg = PyDatagram()
            dg.addServerHeader(doId, self.ourChannel, STATESERVER_OBJECT_SET_FIELDS)
            dg.addUint32(doId)
            dg.addUint16(fieldCount)
            dg.appendData(fieldPacker.getString())
            self.send(dg)
            # Now slide it into the zone we expect to see it in (so it
            # generates onto us with all of the fields in place)
            dg = PyDatagram()
            dg.addServerHeader(doId, self.ourChannel, STATESERVER_OBJECT_SET_LOCATION)
            dg.addUint32(parentId)
            dg.addUint32(zoneId)
            self.send(dg)
        else:
            dg = PyDatagram()
            dg.addServerHeader(doId, self.ourChannel, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS)
            dg.addUint32(doId)
            dg.addUint32(parentId)
            dg.addUint32(zoneId)
            self.send(dg)

    def sendSetLocation(self, do, parentId, zoneId):
        dg = PyDatagram()
        dg.addServerHeader(do.doId, self.ourChannel, STATESERVER_OBJECT_SET_LOCATION)
        dg.addUint32(parentId)
        dg.addUint32(zoneId)
        self.send(dg)

    def generateWithRequired(self, do, parentId, zoneId, optionalFields=[]):
        """
        Generate an object onto the State Server, choosing an ID from the pool.

        You should use do.generateWithRequired(...) instead. This is not meant
        to be called directly unless you really know what you are doing.
        """

        doId = self.allocateChannel()
        self.generateWithRequiredAndId(do, doId, parentId, zoneId, optionalFields)

    def generateWithRequiredAndId(self, do, doId, parentId, zoneId, optionalFields=[]):
        """
        Generate an object onto the State Server, specifying its ID and location.

        You should use do.generateWithRequiredAndId(...) instead. This is not
        meant to be called directly unless you really know what you are doing.
        """

        do.doId = doId
        self.addDOToTables(do, location=(parentId, zoneId))
        do.sendGenerateWithRequired(self, parentId, zoneId, optionalFields)

    def requestDelete(self, do):
        """
        Request the deletion of an object that already exists on the State Server.

        You should use do.requestDelete() instead. This is not meant to be
        called directly unless you really know what you are doing.
        """

        dg = PyDatagram()
        dg.addServerHeader(do.doId, self.ourChannel, STATESERVER_OBJECT_DELETE_RAM)
        dg.addUint32(do.doId)
        self.send(dg)

    def connect(self, host, port=7199):
        """
        Connect to a Message Director. The airConnected message is sent upon
        success.

        N.B. This overrides the base class's connect(). You cannot use the
        ConnectionRepository connect() parameters.
        """

        url = URLSpec()
        url.setServer(host)
        url.setPort(port)

        self.notify.info('Now connecting to %s:%s...' % (host, port))
        ConnectionRepository.connect(self, [url],
                                     successCallback=self.__connected,
                                     failureCallback=self.__connectFailed,
                                     failureArgs=[host, port])

    def __connected(self):
        self.notify.info('Connected successfully.')

        # Listen to our channel...
        self.registerForChannel(self.ourChannel)

        # If we're configured with a State Server, register a post-remove to
        # clean up whatever objects we own on this server should we unexpectedly
        # fall over and die.
        if self.serverId:
            dg = PyDatagram()
            dg.addServerHeader(self.serverId, self.ourChannel, STATESERVER_DELETE_AI_OBJECTS)
            dg.addChannel(self.ourChannel)
            self.addPostRemove(dg)

        messenger.send('airConnected')
        self.handleConnected()

    def __connectFailed(self, code, explanation, host, port):
        self.notify.warning('Failed to connect! (code=%s; %r)' % (code, explanation))

        # Try again...
        retryInterval = config.GetFloat('air-reconnect-delay', 5.0)
        taskMgr.doMethodLater(retryInterval, self.connect, 'Reconnect delay', extraArgs=[host, port])

    def handleConnected(self):
        """
        Subclasses should override this if they wish to handle the connection
        event.
        """

    def lostConnection(self):
        # This should be overridden by a subclass if unexpectedly losing connection
        # is okay.
        self.notify.error('Lost connection to gameserver!')

    def setEventLogHost(self, host, port=7197):
        """
        Set the target host for Event Logger messaging. This should be pointed
        at the UDP IP:port that hosts the cluster's running Event Logger.

        Providing a value of None or an empty string for 'host' will disable
        event logging.
        """

        if not host:
            self.eventSocket = None
            return

        address = SocketAddress()
        if not address.setHost(host, port):
            self.notify.warning('Invalid Event Log host specified: %s:%s' % (host, port))
            self.eventSocket = None
        else:
            self.eventSocket = SocketUDPOutgoing()
            self.eventSocket.InitToAddress(address)

    def writeServerEvent(self, logtype, *args, **kwargs):
        """
        Write an event to the central Event Logger, if one is configured.

        The purpose of the Event Logger is to keep a game-wide record of all
        interesting in-game events that take place. Therefore, this function
        should be used whenever such an interesting in-game event occurs.
        """

        if self.eventSocket is None:
            return # No event logger configured!

        log = collections.OrderedDict()
        log['type'] = logtype
        log['sender'] = self.eventLogId

        for i,v in enumerate(args):
            # +1 because the logtype was _0, so we start at _1
            log['_%d' % (i+1)] = v

        log.update(kwargs)

        dg = PyDatagram()
        msgpack_encode(dg, log)
        self.eventSocket.Send(dg.getMessage())

    def setAI(self, doId, aiChannel):
        """
        Sets the AI of the specified DistributedObjectAI to be the specified channel.
        Generally, you should not call this method, and instead call DistributedObjectAI.setAI.
        """

        dg = PyDatagram()
        dg.addServerHeader(doId, aiChannel, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(aiChannel)
        self.send(dg)

    def eject(self, clientChannel, reasonCode, reason):
        """
        Kicks the client residing at the specified clientChannel, using the specifed reasoning.
        """

        dg = PyDatagram()
        dg.addServerHeader(clientChannel, self.ourChannel, CLIENTAGENT_EJECT)
        dg.add_uint16(reasonCode)
        dg.addString(reason)
        self.send(dg)
        
    def setClientState(self, clientChannel, state):
        """
        Sets the state of the client on the CA.
        Useful for logging in and logging out, and for little else.
        """

        dg = PyDatagram()
        dg.addServerHeader(clientChannel, self.ourChannel, CLIENTAGENT_SET_STATE)
        dg.add_uint16(state)
        self.send(dg)

    def clientAddSessionObject(self, clientChannel, doId):
        """
        Declares the specified DistributedObject to be a "session object",
        meaning that it is destroyed when the client disconnects.
        Generally used for avatars owned by the client.
        """

        dg = PyDatagram()
        dg.addServerHeader(clientChannel, self.ourChannel, CLIENTAGENT_ADD_SESSION_OBJECT)
        dg.add_uint32(doId)
        self.send(dg)

    def clientAddInterest(self, clientChannel, interestId, parentId, zoneId):
        """
        Opens an interest on the behalf of the client. This, used in conjunction
        with add_interest: visible (or preferably, disabled altogether), will mitigate
        possible security risks.
        """

        dg = PyDatagram()
        dg.addServerHeader(clientChannel, self.ourChannel, CLIENTAGENT_ADD_INTEREST)
        dg.add_uint16(interestId)
        dg.add_uint32(parentId)
        dg.add_uint32(zoneId)
        self.send(dg)

    def setOwner(self, doId, newOwner):
        """
        Sets the owner of a DistributedObject. This will enable the new owner to send "ownsend" fields,
        and will generate an OwnerView.
        """

        dg = PyDatagram()
        dg.addServerHeader(doId, self.ourChannel, STATESERVER_OBJECT_SET_OWNER)
        dg.add_uint64(newOwner)
        self.send(dg)
