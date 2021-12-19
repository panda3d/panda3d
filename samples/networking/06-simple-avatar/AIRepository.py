from direct.distributed.ClientRepository import ClientRepository
from panda3d.core import URLSpec, ConfigVariableInt, ConfigVariableString

class AIRepository(ClientRepository):
    def __init__(self):
        """ The AI Repository usually lives on a server and is responsible for
        server side logic that will handle game objects """

        # List of all dc files that are of interest to this AI Repository
        dcFileNames = ['../direct.dc', 'sample.dc']

        # Initialize the repository.  We pass it the dc files and as this is an
        # AI repository the dcSuffix AI.  This will make sure any later calls to
        # createDistributedObject will use the correct version.
        # The connectMethod
        ClientRepository.__init__(
            self,
            dcFileNames = dcFileNames,
            dcSuffix = 'AI',
            threadedNet = True)

        # Set the same port as configured on the server to be able to connect
        # to it
        tcpPort = ConfigVariableInt('server-port', 4400).getValue()

        # Set the IP or hostname of the server we want to connect to
        hostname = ConfigVariableString('server-host', '127.0.0.1').getValue()

        # Build the URL from the server hostname and port. If your server
        # doesn't use http you should change it accordingly. Make sure to pass
        # the connectMethod to the  ClientRepository.__init__ call too.
        # Available connection methods are:
        # self.CM_HTTP, self.CM_NET and self.CM_NATIVE
        url = URLSpec('http://{}:{}'.format(hostname, tcpPort))

        # Attempt a connection to the server
        self.connect([url],
                     successCallback = self.connectSuccess,
                     failureCallback = self.connectFailure)

    def connectFailure(self, statusCode, statusString):
        """ something went wrong """
        print("Couldn't connect. Make sure to run server.py first!")
        raise(StandardError, statusString)

    def connectSuccess(self):
        """ Successfully connected.  But we still can't really do
        anything until we've got the doID range. """
        # The Client Repository will throw this event as soon as it has a doID
        # range and would be able to create distributed objects
        self.accept('createReady', self.gotCreateReady)

    def lostConnection(self):
        """ This should be overridden by a derived class to handle an
         unexpectedly lost connection to the gameserver. """
        exit()

    def gotCreateReady(self):
        """ Now we're ready to go! """

        # This method checks whether we actually have a valid doID range
        # to create distributed objects yet
        if not self.haveCreateAuthority():
            # Not ready yet.
            return

        # we are ready now, so ignore further createReady events
        self.ignore('createReady')

        # Create a Distributed Object by name.  This will look up the object in
        # the dc files passed to the repository earlier
        self.timeManager = self.createDistributedObject(
            className = 'TimeManagerAI', # The Name of the Class we want to initialize
            zoneId = 1) # The Zone this Object will live in

        print("AI Repository Ready")

    def deallocateChannel(self, doID):
        """ This method will be called whenever a client disconnects from the
        server.  The given doID is the ID of the client who left us. """
        print("Client left us: ", doID)
