from direct.distributed.DistributedObjectAI import DistributedObjectAI


class DistributedCameraAI(DistributedObjectAI):
    def __init__(self, air):
        DistributedObjectAI.__init__(self, air)
        self.parent = 0
        self.fixtures = []

    def getCamParent(self):
        return self.parent

    def getFixtures(self):
        return self.fixtures
