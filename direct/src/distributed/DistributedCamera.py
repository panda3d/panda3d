from pandac.PandaModules import *
from direct.fsm.FSM import FSM 
from direct.interval.IntervalGlobal import *
from direct.distributed.DistributedObject import DistributedObject

class Fixture(NodePath, FSM):
    def __init__(self, id, parent, pos, hpr):
        NodePath.__init__(self, 'cam-%s' % id)
        FSM.__init__(self, '%s-fsm' % self.getName())
        self.id = id

        model = loader.loadModel('models/misc/camera', okMissing = True)
        model.reparentTo(self)

        self.reparentTo(parent)
        self.setPos(*pos)
        self.setHpr(*hpr)
        self.setLightOff(100)
        self.hide()

        self.scaleIval = None

        self.recordingInProgress = False
        pass

    def __str__(self):
        return 'Fixture(%d, \'%s\', %s, %s)' % (self.id, self.state, self.getPos(), self.getHpr())

    def pack(self):
        return 'Camera(%s, %s)' % (self.getPos(), self.getHpr())

    def setId(self, id):
        self.id = id
        pass
    
    def setRecordingInProgress(self, inProgress):
        self.recordingInProgress = inProgress
        if self.recordingInProgress and \
           base.config.GetInt('camera-id', -1) >= 0:
            self.hide()
            pass
        else:
            self.show()
            pass
        pass
        
    def show(self):
        if base.config.GetBool('aware-of-cameras',0) and \
           not self.recordingInProgress:
            NodePath.show(self)
            pass
        pass

    def getScaleIval(self):
        if not self.scaleIval:
            self.scaleIval = Sequence(LerpScaleInterval(self.getChild(0), 0.25, 2, startScale = 1, blendType = 'easeInOut'),
                                      LerpScaleInterval(self.getChild(0), 0.25, 1, startScale = 2, blendType = 'easeInOut'))
            pass
        return self.scaleIval
    
    def setState(self, state):
        self.request(state)
        pass

    def defaultFilter(self, request, args):
        if request == self.getCurrentOrNextState():
            return None
        return FSM.defaultFilter(self, request, args)

    def exitOff(self):
        self.accept('recordingInProgress', self.setRecordingInProgress)
        pass

    def enterOff(self):
        self.ignore('recordingInProgress')

        if self.scaleIval:
            self.scaleIval.finish()
            self.scaleIval = None
            pass

        self.hide()
        pass
        
    def enterStandby(self):
        self.show()
        if self.id == base.config.GetInt('camera-id', -1):
            self.setColorScale(3,0,0,1)
        else:
            self.setColorScale(3,3,0,1)
            pass
        pass
    
    def enterBlinking(self):
        self.show()
        self.setColorScale(0,3,0,1)
        self.getScaleIval().loop()
        pass        

    def exitBlinking(self):
        if self.scaleIval:
            self.scaleIval.finish()
            pass
        pass

    def enterRecording(self):
        if base.config.GetInt('camera-id', -1) == self.id:
            self.demand('Using')
            pass
        else:
            self.show()
            self.setColorScale(3,0,0,1)
            self.getScaleIval().loop()
            pass
        pass

    def exitRecording(self):
        if self.scaleIval:
            self.scaleIval.finish()
            pass
        pass

    def enterUsing(self):
        localAvatar.b_setGameState('Camera')
        camera.setPosHpr(0,0,0,0,0,0)
        camera.reparentTo(self)
        self.hide()
        pass
    
    def exitUsing(self):
        localAvatar.b_setGameState('LandRoam')
        self.show()
        pass
    

class DistributedCamera(DistributedObject):
    def __init__(self, cr):
        DistributedObject.__init__(self, cr)
        self.parent = None
        self.fixtures = {}
        self.cameraId = base.config.GetInt('camera-id',0)

        pass

    def __getitem__(self, index):
        return self.fixtures.get(index)

    def __str__(self):
        out = ''
        for fixture in self.fixtures.itervalues():
            out = '%s\n%s' % (out, fixture)
        return out[1:]

    def pack(self):
        out = ''
        for fixture in self.fixtures.itervalues():
            out = '%s\n%s' % (out, fixture.pack())
        return out[1:]

    def disable(self):
        self.ignore('escape')
        
        self.parent = None
        
        for fixture in self.fixtures.itervalues():
            fixture.cleanup()
            fixture.detachNode()
            pass
        self.fixtures = {}
        
        DistributedObject.disable(self)

    def getOV(self):
        return self.cr.doId2ownerView.get(self.getDoId())

    def setCamParent(self, doId):
        if doId != self.parent:
            if not doId:
                self.parent = render
            else:
                self.parent = self.cr.getDo(doId)
                pass

            for fix in self.fixtures.itervalues():
                fix.reparentTo(self.parent)
                pass
            pass
        pass

    def getCamParent(self):
        return self.parent
    
    def setFixtures(self, fixtures):

        for x in range(len(fixtures), len(self.fixtures)):
            fixture = self.fixtures.pop(x)
            fixture.cleanup()
            fixture.detachNode()
            pass

        recordingInProgress = False
        for x,fixture in enumerate(fixtures):
            fix = self.fixtures.get(x)
            if not fix:
                fix = Fixture(x, self.parent, fixture[:3], fixture[3:6])
                self.fixtures[x] = fix
                pass

            posHpr = fixture[:6]
            state = fixture[6]
            fix.setId(x)
            fix.setPosHpr(*posHpr)
            fix.setState(state)
            recordingInProgress = recordingInProgress or state == 'Recording'
            pass
        
        messenger.send('recordingInProgress', [recordingInProgress])

    def testFixture(self, index):
        fixture = self.fixtures.get(index)
        if fixture:
            fixture.request('Using')
            self.accept('escape', self.stopTesting, [index])
            pass
        pass

    def stopTesting(self, index):
        fixture = self.fixtures.get(index)
        if fixture:
            self.ignore('escape')
            fixture.request('Standby')
            localAvatar.b_setGameState('LandRoam')
        pass
    
