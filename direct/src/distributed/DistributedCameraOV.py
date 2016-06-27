from panda3d.core import *
from panda3d.direct import *
from direct.distributed.DistributedObjectOV import DistributedObjectOV

class DistributedCameraOV(DistributedObjectOV):
    def __init__(self, cr):
        DistributedObjectOV.__init__(self, cr)
        self.parent = 0
        self.fixtures = []
        self.accept('refresh-fixture', self.refreshFixture)

    def delete(self):
        self.ignore('escape')
        self.ignore('refresh-fixture')
        DistributedObjectOV.delete(self)

    def getObject(self):
        return self.cr.getDo(self.getDoId())

    def setCamParent(self, doId):
        self.parent = doId

    def setFixtures(self, fixtures):
        self.fixtures = fixtures

    def storeToFile(self, name):
        f = file('cameras-%s.txt' % name, 'w')
        f.writelines(self.getObject().pack())
        f.close()

    def unpackFixture(self, data):
        data = data.strip().replace('Camera','')
        pos,hpr,fov = eval(data)
        return pos,hpr,fov

    def loadFromFile(self, name):
        self.b_setFixtures([])
        f = file('cameras-%s.txt' % name, 'r');
        for line in f.readlines():
            pos,hpr,fov = self.unpackFixture(line)
            self.addFixture([pos[0],pos[1],pos[2],
                             hpr[0],hpr[1],hpr[2],
                             fov[0],fov[1],
                             'Standby'])
        f.close()

    def refreshFixture(self, id, data):
        pos,hpr,fov = self.unpackFixture(data)
        fixture = self.fixtures[id]
        fixture = [pos[0],pos[1],pos[2],
                   hpr[0],hpr[1],hpr[2],
                   fov[0],fov[1],
                   fixture[8]]

        # distributed only
        self.d_setFixtures(self.fixtures)

    def b_setFixtures(self, fixtures):
        self.getObject().setFixtures(fixtures)
        self.setFixtures(fixtures)
        self.d_setFixtures(fixtures)

    def d_setFixtures(self, fixtures):
        self.sendUpdate('setFixtures', [fixtures])

    def addFixture(self, fixture, index = None):
        if index is not None:
            self.fixtures.insert(index, fixture)
        else:
            self.fixtures.append(fixture)
        self.b_setFixtures(self.fixtures)
        return self.fixtures.index(fixture)

    def blinkFixture(self, index):
        if index < len(self.fixtures):
            fixture = self.fixtures[index]
            fixture[6] = 'Blinking'
            self.b_setFixtures(self.fixtures)

    def standbyFixture(self, index):
        if index < len(self.fixtures):
            fixture = self.fixtures[index]
            fixture[6] = 'Standby'
            self.b_setFixtures(self.fixtures)

    def testFixture(self, index):
        if index < len(self.fixtures):
            self.getObject().testFixture(index)

    def removeFixture(self, index):
        self.fixtures.pop(index)
        self.b_setFixtures(self.fixtures)

    def saveFixture(self, index = None):
        """
        Position the camera with ~oobe, then call this to save its telemetry.
        """
        parent = self.getObject().getCamParent()
        pos = base.cam.getPos(parent)
        hpr = base.cam.getHpr(parent)
        return self.addFixture([pos[0], pos[1], pos[2],
                                hpr[0], hpr[1], hpr[2],
                                'Standby'],
                               index)

    def startRecording(self):
        self.accept('escape', self.stopRecording)
        for fixture in self.fixtures:
            fixture[6] = 'Recording'
        self.b_setFixtures(self.fixtures)

    def stopRecording(self):
        self.ignore('escape')
        for fixture in self.fixtures:
            fixture[6] = 'Standby'
        self.b_setFixtures(self.fixtures)
