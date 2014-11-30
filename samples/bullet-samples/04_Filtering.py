
#from pandac.PandaModules import loadPrcFileData
#loadPrcFileData('', 'load-display tinydisplay')

import sys
import direct.directbase.DirectStart

from direct.showbase.DirectObject import DirectObject
from direct.showbase.InputStateGlobal import inputState

from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import Vec3
from panda3d.core import Vec4
from panda3d.core import Point3
from panda3d.core import TransformState
from panda3d.core import BitMask32

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletDebugNode

class Game(DirectObject):

  def __init__(self):
    base.setBackgroundColor(0.1, 0.1, 0.8, 1)
    base.setFrameRateMeter(True)

    base.cam.setPos(0, -20, 4)
    base.cam.lookAt(0, 0, 0)

    # Light
    alight = AmbientLight('ambientLight')
    alight.setColor(Vec4(0.5, 0.5, 0.5, 1))
    alightNP = render.attachNewNode(alight)

    dlight = DirectionalLight('directionalLight')
    dlight.setDirection(Vec3(1, 1, -1))
    dlight.setColor(Vec4(0.7, 0.7, 0.7, 1))
    dlightNP = render.attachNewNode(dlight)

    render.clearLight()
    render.setLight(alightNP)
    render.setLight(dlightNP)

    # Input
    self.accept('escape', self.doExit)
    self.accept('r', self.doReset)
    self.accept('f1', self.toggleWireframe)
    self.accept('f2', self.toggleTexture)
    self.accept('f3', self.toggleDebug)
    self.accept('f5', self.doScreenshot)

    inputState.watchWithModifiers('forward', 'w')
    inputState.watchWithModifiers('left', 'a')
    inputState.watchWithModifiers('reverse', 's')
    inputState.watchWithModifiers('right', 'd')
    inputState.watchWithModifiers('turnLeft', 'q')
    inputState.watchWithModifiers('turnRight', 'e')

    # Task
    taskMgr.add(self.update, 'updateWorld')

    # Physics
    self.setup()

  # _____HANDLER_____

  def doExit(self):
    self.cleanup()
    sys.exit(1)

  def doReset(self):
    self.cleanup()
    self.setup()

  def toggleWireframe(self):
    base.toggleWireframe()

  def toggleTexture(self):
    base.toggleTexture()

  def toggleDebug(self):
    if self.debugNP.isHidden():
      self.debugNP.show()
    else:
      self.debugNP.hide()

  def doScreenshot(self):
    base.screenshot('Bullet')

  # ____TASK___

  def processInput(self, dt):
    force = Vec3(0, 0, 0)
    torque = Vec3(0, 0, 0)

    if inputState.isSet('forward'): force.setY( 1.0)
    if inputState.isSet('reverse'): force.setY(-1.0)
    if inputState.isSet('left'):    force.setX(-1.0)
    if inputState.isSet('right'):   force.setX( 1.0)
    if inputState.isSet('turnLeft'):  torque.setZ( 1.0)
    if inputState.isSet('turnRight'): torque.setZ(-1.0)

    force *= 30.0
    torque *= 10.0

    self.boxNP.node().setActive(True)
    self.boxNP.node().applyCentralForce(force)
    self.boxNP.node().applyTorque(torque)

  def update(self, task):
    dt = globalClock.getDt()

    self.processInput(dt)
    self.world.doPhysics(dt)

    return task.cont

  def cleanup(self):
    self.world = None
    self.worldNP.removeNode()

  def setup(self):
    self.worldNP = render.attachNewNode('World')

    # World
    self.debugNP = self.worldNP.attachNewNode(BulletDebugNode('Debug'))
    self.debugNP.show()

    self.world = BulletWorld()
    self.world.setGravity(Vec3(0, 0, -9.81))
    self.world.setDebugNode(self.debugNP.node())

    # Ground
    shape = BulletPlaneShape(Vec3(0, 0, 1), -1)

    mask = BitMask32(0x0f)
    print 'ground: ', mask

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Ground'))
    np.node().addShape(shape)
    np.setPos(0, 0, -1)
    np.setCollideMask(mask)

    self.world.attachRigidBody(np.node())

    ## Box 1
    #shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5))

    #mask = BitMask32.allOff()
    #print 'box-1:  ', mask

    #np = self.worldNP.attachNewNode(BulletRigidBodyNode('Box-1'))
    #np.node().setMass(1.0)
    #np.node().addShape(shape)
    #np.setPos(-6, 0, 4)
    #np.setCollideMask(mask)

    #self.world.attachRigidBody(np.node())

    ## Box 2
    #shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5))

    #mask = BitMask32.bit(0)
    #print 'box-2:  ', mask

    #np = self.worldNP.attachNewNode(BulletRigidBodyNode('Box-2'))
    #np.node().setMass(1.0)
    #np.node().addShape(shape)
    #np.setPos(-2, 0, 4)
    #np.setCollideMask(mask)

    #self.world.attachRigidBody(np.node())

    ## Box 3
    #shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5))

    #mask = BitMask32.bit(2)
    #print 'box-3:  ', mask

    #np = self.worldNP.attachNewNode(BulletRigidBodyNode('Box-3'))
    #np.node().setMass(1.0)
    #np.node().addShape(shape)
    #np.setPos(2, 0, 4)
    #np.setCollideMask(mask)

    #self.world.attachRigidBody(np.node())

    # Box 4
    shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5))

    mask = BitMask32(0x3)
    print 'box-4:  ', mask

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Box-4'))
    np.node().setMass(1.0)
    np.node().addShape(shape)
    np.setPos(6, 0, 4)
    np.setCollideMask(mask)

    self.world.attachRigidBody(np.node())

    self.boxNP = np

    visualNP = loader.loadModel('models/box.egg')
    visualNP.reparentTo(self.boxNP)

    #self.boxNP.setCollideMask(BitMask32.allOff())
    #self.world.removeRigidBody(self.boxNP.node())
    #self.world.attachRigidBody(self.boxNP.node())


game = Game()
run()

