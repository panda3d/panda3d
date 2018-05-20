
from direct.directnotify import DirectNotifyGlobal
from direct.distributed.PyDatagram import PyDatagram
from direct.showbase.Messenger import Messenger

import sys
if sys.version_info >= (3, 0):
    from pickle import dumps, loads
else:
    from cPickle import dumps, loads


class NetMessenger(Messenger):
    """
    This works very much like the Messenger class except that messages
    are sent over the network and (possibly) handled (accepted) on a
    remote machine (server).
    """
    notify = DirectNotifyGlobal.directNotify.newCategory('NetMessenger')

    def __init__(self, air, baseChannel=20000, baseMsgType=20000):
        """
        air is the AI Repository.
        baseChannel is the channel that the first message is sent on.
        baseMsgType is the MsgType of the same.
        """
        assert self.notify.debugCall()
        Messenger.__init__(self)
        self.air=air
        self.baseChannel = baseChannel
        self.baseMsgType = baseMsgType

        self.__message2type = {}
        self.__type2message = {}
        self.__message2channel = {}

    def clear(self):
        assert self.notify.debugCall()
        Messenger.clear(self)

    def register(self, code, message):
        assert self.notify.debugCall()
        channel = self.baseChannel + code
        msgType = self.baseMsgType + code

        if message in self.__message2type:
            self.notify.error('Tried to register message %s twice!' % message)
            return

        self.__message2type[message] = msgType
        self.__type2message[msgType] = message
        self.__message2channel[message] = channel

    def prepare(self, message, sentArgs=[]):
        """
        Prepare the datagram that would get sent in order to send this message
        to its designated channel.
        """
        assert self.notify.debugCall()

        # Make sure the message is registered:
        if message not in self.__message2type:
            self.notify.error('Tried to send unregistered message %s!' % message)
            return

        datagram = PyDatagram()
        # To:
        datagram.addUint8(1)
        datagram.addChannel(self.__message2channel[message])
        # From:
        datagram.addChannel(self.air.ourChannel)

        messageType=self.__message2type[message]
        datagram.addUint16(messageType)
        datagram.addString(str(dumps(sentArgs)))

        return datagram

    def accept(self, message, *args):
        if message not in self.__message2channel:
            self.notify.error('Tried to accept unregistered message %s!' % message)
            return

        anyAccepting = bool(self.whoAccepts(message))
        if not anyAccepting:
            self.air.registerForChannel(self.__message2channel[message])

        Messenger.accept(self, message, *args)

    def send(self, message, sentArgs=[]):
        """
        Send message to anything that's listening for it.
        """
        assert self.notify.debugCall()

        datagram = self.prepare(message, sentArgs)
        self.air.send(datagram)
        Messenger.send(self, message, sentArgs=sentArgs)

    def handle(self, msgType, di):
        """
        Send data from the net on the local netMessenger.
        """
        assert self.notify.debugCall()

        if msgType not in self.__type2message:
            self.notify.warning('Received unknown message: %d' % msgType)
            return

        message = self.__type2message[msgType]
        sentArgs=loads(di.getString())

        if type(sentArgs) != list:
            self.notify.warning('Received non-list item in %s message: %r' %
                                (message, sentArgs))
            return

        Messenger.send(self, message, sentArgs=sentArgs)
