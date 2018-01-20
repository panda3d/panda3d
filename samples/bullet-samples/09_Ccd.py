
#from pandac.PandaModules import loadPrcFileData
#loadPrcFileData('', 'load-display tinydisplay')
#loadPrcFileData('', 'bullet-solver-iterations 20')

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

    base.cam.setPos(0, -40, 10)
    base.cam.lookAt(0, 0, 5)

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

    self.accept('1', self.doShoot, [True])
    self.accept('2', self.doShoot, [False])

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

  def doShoot(self, ccd):
    # Get from/to points from mouse click
    pMouse = base.mouseWatcherNode.getMouse()
    pFrom = Point3()
    pTo = Point3()
    base.camLens.extrude(pMouse, pFrom, pTo)

    pFrom = render.getRelativePoint(base.cam, pFrom)
    pTo = render.getRelativePoint(base.cam, pTo)

    # Calculate initial velocity
    v = pTo - pFrom
    v.normalize()
    v *= 10000.0

    # Create bullet
    shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5))
    body = BulletRigidBodyNode('Bullet')
    bodyNP = self.worldNP.attachNewNode(body)
    bodyNP.node().addShape(shape)
    bodyNP.node().setMass(2.0)
    bodyNP.node().setLinearVelocity(v)
    bodyNP.setPos(pFrom)
    bodyNP.setCollideMask(BitMask32.allOn())

    if ccd:
      bodyNP.node().setCcdMotionThreshold(1e-7);
      bodyNP.node().setCcdSweptSphereRadius(0.50);

    self.world.attachRigidBody(bodyNP.node())

    # Remove the bullet again after 1 second
    taskMgr.doMethodLater(1, self.doRemove, 'doRemove', 
      extraArgs=[bodyNP],
      appendTask=True)

  def doRemove(self, bulletNP, task):
    self.world.removeRigidBody(bulletNP.node())
    return task.done

  # ____TASK___

  def update(self, task):
    dt = globalClock.getDt()

    self.world.doPhysics(dt, 20, 1.0/180.0)

    return task.cont

  def cleanup(self):
    self.world = None
    self.worldNP.removeNode()
    self.worldNP = None

  def setup(self):
    self.worldNP = render.attachNewNode('World')

    # World
    self.debugNP = self.worldNP.attachNewNode(BulletDebugNode('Debug'))
    self.debugNP.show()

    self.world = BulletWorld()
    self.world.setGravity(Vec3(0, 0, -9.81))
    self.world.setDebugNode(self.debugNP.node())

    # Ground
    shape = BulletPlaneShape(Vec3(0, 0, 1), 0)
    body = BulletRigidBodyNode('Ground')
    bodyNP = self.worldNP.attachNewNode(body)
    bodyNP.node().addShape(shape)
    bodyNP.setPos(0, 0, -1)
    bodyNP.setCollideMask(BitMask32.allOn())
    self.world.attachRigidBody(bodyNP.node())

    # Some boxes
    shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5))

    for i in range(10):
      for j in range(10):
        x = i - 5.0
        y = 0.0
        z = j - 0.5

        body = BulletRigidBodyNode('Box-%i-%i' % (i, j))
        bodyNP = self.worldNP.attachNewNode(body)
        bodyNP.node().addShape(shape)
        bodyNP.node().setMass(1.0)
        bodyNP.node().setDeactivationEnabled(False)
        bodyNP.setPos(x, y, z)
        bodyNP.setCollideMask(BitMask32.allOn())

        self.world.attachRigidBody(bodyNP.node())

        visNP = loader.loadModel('models/box.egg')
        visNP.clearModelNodes()
        visNP.reparentTo(bodyNP)

game = Game()
run()

