
from direct.directnotify import DirectNotifyGlobal
from direct.distributed.PyDatagram import PyDatagram
from direct.showbase.Messenger import Messenger

import sys
if sys.version_info >= (3, 0):
    from pickle import dumps, loads
else:
    from cPickle import dumps, loads


# Messages do not need to be in the MESSAGE_TYPES list.
# This is just an optimization.  If the message is found
# in this list, it is reduced to an integer index and
# the message string is not sent.  Otherwise, the message
# string is sent in the datagram.
MESSAGE_TYPES=(
    "avatarOnline",
    "avatarOffline",
    "create",
    "needUberdogCreates",
    "transferDo",
)

# This is the reverse look up for the recipient of the
# datagram:
MESSAGE_STRINGS={}
for i in zip(MESSAGE_TYPES, range(1, len(MESSAGE_TYPES)+1)):
    MESSAGE_STRINGS[i[0]]=i[1]


class NetMessenger(Messenger):
    """
    This works very much like the Messenger class except that messages
    are sent over the network and (possibly) handled (accepted) on a
    remote machine (server).
    """
    notify = DirectNotifyGlobal.directNotify.newCategory('NetMessenger')

    def __init__(self, air, channels):
        """
        air is the AI Repository.
        channels is a list of channel IDs (uint32 values)
        """
        assert self.notify.debugCall()
        Messenger.__init__(self)
        self.air=air
        self.channels=channels
        for i in self.channels:
            self.air.registerForChannel(i)

    def clear(self):
        assert self.notify.debugCall()
        for i in self.channels:
            self.air.unRegisterChannel(i)
        del self.air
        del self.channels
        Messenger.clear(self)

    def send(self, message, sentArgs=[]):
        """
        Send message to All AI and Uber Dog servers.
        """
        assert self.notify.debugCall()
        datagram = PyDatagram()
        # To:
        datagram.addUint8(1)
        datagram.addChannel(self.channels[0])
        # From:
        datagram.addChannel(self.air.ourChannel)
        #if 1: # We send this just because the air expects it:
        #    # Add an 'A' for AI
        #    datagram.addUint8(ord('A'))

        messageType=MESSAGE_STRINGS.get(message, 0)
        datagram.addUint16(messageType)
        if messageType:
            datagram.addString(str(dumps(sentArgs)))
        else:
            datagram.addString(str(dumps((message, sentArgs))))
        self.air.send(datagram)

    def handle(self, pickleData):
        """
        Send pickleData from the net on the local netMessenger.
        The internal data in pickleData should have a tuple of
        (messageString, sendArgsList).
        """
        assert self.notify.debugCall()
        messageType=self.air.getMsgType()
        if messageType:
            message=MESSAGE_TYPES[messageType-1]
            sentArgs=loads(pickleData)
        else:
            (message, sentArgs) = loads(pickleData)
        Messenger.send(self, message, sentArgs=sentArgs)


