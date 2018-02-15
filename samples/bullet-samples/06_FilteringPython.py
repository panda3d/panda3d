
from pandac.PandaModules import loadPrcFileData
loadPrcFileData('', 'bullet-filter-algorithm python-callback')
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

    self.accept('1', self.doSelect, [0,])
    self.accept('2', self.doSelect, [1,])
    self.accept('3', self.doSelect, [2,])
    self.accept('4', self.doSelect, [3,])
    self.accept('5', self.doSelect, [4,])
    self.accept('6', self.doSelect, [5,])

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

  def doSelect(self, i):
    self.boxNP = self.boxes[i]

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
    self.world.setPythonFilterCallback(self.filter)

    # Ground
    shape = BulletPlaneShape(Vec3(0, 0, 1), -1)
    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Ground'))
    np.node().addShape(shape)
    np.setPos(0, 0, -1)
    np.setPythonTag('foo', 2)
    self.world.attachRigidBody(np.node())

    # Box 1
    shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5))
    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Box-1'))
    np.node().setMass(1.0)
    np.node().addShape(shape)
    np.setPos(3, 0, 4)
    np.setPythonTag('foo', 0)
    self.world.attachRigidBody(np.node())
    self.boxNP = np

    # Box 2
    shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5))
    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Box-2'))
    np.node().setMass(1.0)
    np.node().addShape(shape)
    np.setPos(-3, 0, 4)
    np.setPythonTag('foo', -1)
    self.world.attachRigidBody(np.node())

  def filter(self, node1, node2):
    """
    A rather silly collision filtering algorithm. We assume every node
    has the Python tag 'foo' set, and that the value of this tag is integer.
    Then we add the values and if the result is greater than zero we want
    the nodes to collide.
    """
    x1 = node1.getPythonTag('foo')
    x2 = node2.getPythonTag('foo')
    return (x1 + x2 > 0)



game = Game()
run()

