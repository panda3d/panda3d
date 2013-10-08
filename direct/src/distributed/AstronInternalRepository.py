from pandac.PandaModules import *
from MsgTypes import *
from direct.directnotify import DirectNotifyGlobal
from ConnectionRepository import ConnectionRepository
from PyDatagram import PyDatagram
from PyDatagramIterator import PyDatagramIterator

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

    def __init__(self, baseChannel, dcFileNames = [], dcSuffix = 'AI',
                 connectMethod = None, threadedNet = None):
        if connectMethod is None:
            connectMethod = self.CM_HTTP
        ConnectionRepository.__init__(self, connectMethod, base.config, hasOwnerView = False, threadedNet = threadedNet)
        self.dcSuffix = dcSuffix
        if hasattr(self, 'setVerbose'):
            if self.config.GetBool('verbose-internalrepository'):
                self.setVerbose(1)

        maxChannels = self.config.GetInt('air-channel-allocation', 1000000)
        self.channelAllocator = UniqueIdAllocator(baseChannel, baseChannel+maxChannels-1)

        self.ourChannel = self.channelAllocator.allocate()

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

    def writeServerEvent(self, logtype, *args):
        """
        Write an event to the central Event Logger, if one is configured.

        The purpose of the Event Logger is to keep a game-wide record of all
        interesting in-game events that take place. Therefore, this function
        should be used whenever such an interesting in-game event occurs.
        """

        if self.eventSocket is None:
            return # No event logger configured!

        dg = PyDatagram()
        dg.addString(self.eventLogId)
        dg.addString(logtype)
        for arg in args:
            dg.addString(arg)
        self.eventSocket.Send(dg.getMessage())
