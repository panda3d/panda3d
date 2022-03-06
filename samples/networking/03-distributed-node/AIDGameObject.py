from direct.distributed.DistributedObject import DistributedObject

class AIDGameObject(DistributedObject):
    """ This class is a DirectObject which will be created and managed by the
    AI Repository. """

    def __init__(self, cr):
        DistributedObject.__init__(self, cr)

    def announceGenerate(self):
        """ The AI has created this object, so we send it's distributed object ID
        over to the client.  That way the client can actually grab the object
        and use it to communicate with the AI.  Alternatively store it in the
        Client Repository in self.cr """
        base.messenger.send(self.cr.uniqueName('AIDGameObjectGenerated'), [self.doId])
        # call the base class method
        DistributedObject.announceGenerate(self)

    def d_requestDataFromAI(self):
        """ Request some data from the AI and passing it some data from us. """
        data = ("Some Data", 1, -1.25)
        print("Sending game data:", data)
        self.sendUpdate('messageRoundtripToAI', [data])

    def messageRoundtripToClient(self, data):
        """ Here we expect the answer from the AI from a previous
        messageRoundtripToAI call """
        print("Got Data:", data)
        print("Roundtrip message complete")
