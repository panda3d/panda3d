# all imports needed by the engine itself
from direct.showbase.ShowBase import ShowBase

# initialize the engine
base = ShowBase()

# initialize the client

from direct.distributed.ClientRepository import ClientRepository
from panda3d.core import URLSpec, ConfigVariableInt, ConfigVariableString
from direct.gui.OnscreenText import OnscreenText
from panda3d.core import TextNode


base.accept("escape", exit)

# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(0, 0, 0, 1), shadow=(1, 1, 1, 1),
                        parent=base.a2dTopLeft, align=TextNode.ALeft,
                        pos=(0.08, -pos - 0.04), scale=.06)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, pos=(-0.1, 0.09), scale=.08,
                        parent=base.a2dBottomRight, align=TextNode.ARight,
                        fg=(1, 1, 1, 1), shadow=(0, 0, 0, 1))

title = addTitle("Panda3D: Tutorial - Distributed Network (NOT CONNECTED)")
inst1 = addInstructions(0.06, "esc: Close the client")
inst2 = addInstructions(0.12, "See console output")

def setConnectedMessage():
    title["text"] = "Panda3D: Tutorial - Distributed Network (CONNECTED)"

base.accept("client-ready", setConnectedMessage)

#
# CLIENT
#

class GameClientRepository(ClientRepository):

    def __init__(self):
        dcFileNames = ['../direct.dc']

        # a distributed object of our game.
        self.distributedObject = None
        self.aiDGameObect = None

        ClientRepository.__init__(
            self,
            dcFileNames = dcFileNames,
            threadedNet = True)

        # Set the same port as configured on the server to be able to connect
        # to it
        tcpPort = ConfigVariableInt('server-port', 4400).getValue()

        # Set the IP or hostname of the server we want to connect to
        hostname = ConfigVariableString('server-host', '127.0.0.1').getValue()

        # Build the URL from the server hostname and port. If your server
        # uses another protocol then http you should change it accordingly.
        # Make sure to pass the connectMethod to the  ClientRepository.__init__
        # call too.  Available connection methods are:
        # self.CM_HTTP, self.CM_NET and self.CM_NATIVE
        self.url = URLSpec('http://{}:{}'.format(hostname, tcpPort))

        # Attempt a connection to the server
        self.connect([self.url],
                     successCallback = self.connectSuccess,
                     failureCallback = self.connectFailure)

    def lostConnection(self):
        """ This should be overridden by a derived class to handle an
        unexpectedly lost connection to the gameserver. """
        # Handle the disconnection from the server.  This can be a reconnect,
        # simply exiting the application or anything else.
        exit()

    def connectFailure(self, statusCode, statusString):
        """ Something went wrong """
        # we could create a reconnect task to try and connect again.
        exit()

    def connectSuccess(self):
        """ Successfully connected.  But we still can't really do
        anything until we've got the doID range. """

        # Make sure we have interest in the by the AIRepository defined
        # TimeManager zone, so we always see it even if we switch to
        # another zone.
        self.setInterestZones([1])

        # We must wait for the TimeManager to be fully created and
        # synced before we can enter another zone and wait for the
        # game object.  The uniqueName is important that we get the
        # correct, our sync message from the TimeManager and not
        # accidentaly a message from another client
        self.acceptOnce(self.uniqueName('gotTimeSync'), self.syncReady)

    def syncReady(self):
        """ Now we've got the TimeManager manifested, and we're in
        sync with the server time.  Now we can enter the world.  Check
        to see if we've received our doIdBase yet. """

        # This method checks whether we actually have a valid doID range
        # to create distributed objects yet
        if self.haveCreateAuthority():
            # we already have one
            self.gotCreateReady()
        else:
            # Not yet, keep waiting a bit longer.
            self.accept(self.uniqueName('createReady'), self.gotCreateReady)

    def gotCreateReady(self):
        """ Ready to enter the world.  Expand our interest to include
        any other zones """

        # This method checks whether we actually have a valid doID range
        # to create distributed objects yet
        if not self.haveCreateAuthority():
            # Not ready yet.
            return

        # we are ready now, so ignore further createReady events
        self.ignore(self.uniqueName('createReady'))

        print("Client Ready")
        base.messenger.send("client-ready")

# Start the client
client = GameClientRepository()

base.run()
