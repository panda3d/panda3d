
from cPickle import dumps, loads

from direct.showbase.Messenger import Messenger


class NetMessenger(Messenger):
    """
    This works very much like the Messenger class except that messages
    are sent over the network and (possibly) handled (accepted) on a
    remote machine (server).
    """
    def __init__(self, air, channels=(4602, 4603, 4604)):
        self.air=air
        self.channels=channels
        Messenger.__init__(self)
        for i in channels:
            self.air.registerForChannel(i)

    def clear(self):
        for i in self.channels:
            self.air.unRegisterChannel(i)
        del self.air
        del self.channels
        Messenger.clear(self)

    def send(self, message, sentArgs=[]):
        """
        Send message to All AI and Uber Dog servers.
        """
        self.sendChannel(4602, message, sentArgs)

    def sendUD(self, message, sentArgs=[]):
        """
        Send message to the uber dog.
        """
        self.sendChannel(4603, message, sentArgs)

    def sendAI(self, message, sentArgs=[]):
        """
        Send message to the game AI servers (non-uber dog).
        """
        self.sendChannel(4604, message, sentArgs)

    def sendChannel(self, channel, message, sentArgs=[]):
        """
        Send message to a particular channel.
        """
        datagram = PyDatagram()
        # To:
        datagram.addChannel(channel)
        # From:
        datagram.addChannel(self.air.ourChannel)
        if 1: # We send this just because the air expects it:
            # Add an 'A' for AI
            datagram.addUint8(ord('A'))
            # Add the message type
            datagram.addUint16(0)
        datagram.addString(str(dumps((message, sentArgs))))
        self.air.send(datagram)

    def check(self, channel):
        """
        returns true if this instance wants this channel data.
        """
        return channel in self.channels:

    def handle(self, pickleData):
        """
        Send pickleData from the net on the local netMessenger.
        The internal data in pickleData should have a tuple of
        (messageString, sendArgsList).
        """
        (message, sentArgs) = loads(pickleData)
        Messenger.send(self, message, sentArgs=sentArgs)


