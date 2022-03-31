from direct.distributed.DistributedObjectAI import DistributedObjectAI

class AIDGameObjectAI(DistributedObjectAI):
    def __init__(self, aiRepository):
        DistributedObjectAI.__init__(self, aiRepository)

    def messageRoundtripToAI(self, data):
        """ The client sent us some data to process.  So work with it and send
        changed data back to the requesting client """
        requesterId = self.air.getAvatarIdFromSender()
        print("Got client data:", data, "from client with ID", requesterId)

        # do something with the data
        aiChangedData = (
            data[0] + " from the AI",
            data[1] + 1,
            data[2])

        print("Sending modified game data back:", aiChangedData)
        self.d_messageRoundtripToClient(aiChangedData, requesterId)

    def d_messageRoundtripToClient(self, data, requesterId):
        """ Send the given data to the requesting client """
        print("Send message to back to:", requesterId)
        self.sendUpdateToAvatarId(requesterId, 'messageRoundtripToClient', [data])
