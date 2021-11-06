from direct.distributed.ClientRepository import ClientRepository
from panda3d.core import URLSpec, ConfigVariableInt, ConfigVariableString
from DistributedSmoothActor import DistributedSmoothActor

class GameClientRepository(ClientRepository):

    def __init__(self):
        dcFileNames = ['../direct.dc', 'sample.dc']

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

        self.join()

        print("Client Ready")

    def join(self):
        """ Join a game/room/whatever """
        # set our intersted zones to let the client see all distributed obects
        # in those zones
        self.setInterestZones([1, 2])

        base.messenger.send('client-joined')
        print("Joined")
