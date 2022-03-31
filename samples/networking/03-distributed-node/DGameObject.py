from direct.distributed.DistributedObject import DistributedObject

class DGameObject(DistributedObject):
    def __init__(self, cr):
        DistributedObject.__init__(self, cr)

    def sendGameData(self, data):
        """ Method that can be called from the clients with an sendUpdate call """
        print(data)

    def d_sendGameData(self):
        """ A method to send an update message to the server.  The d_ stands
        for distributed """
        # send the message to the server
        self.sendUpdate('sendGameData', [('ValueA', 123, 1.25)])
