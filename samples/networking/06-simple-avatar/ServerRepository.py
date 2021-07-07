# all imports needed by the server
from direct.distributed.ServerRepository import ServerRepository
from panda3d.core import ConfigVariableInt

# the main server class
class GameServerRepository(ServerRepository):
    """The server repository class"""
    def __init__(self):
        """initialise the server class"""

        # get the port number from the configuration file
        # if it doesn't exist, we use 4400 as the default
        tcpPort = ConfigVariableInt('server-port', 4400).getValue()

        # list of all needed .dc files
        dcFileNames = ['../direct.dc', 'sample.dc']

        # initialise a threaded server on this machine with
        # the port number and the dc filenames
        ServerRepository.__init__(self, tcpPort, dcFileNames=dcFileNames, threadedNet=True)
