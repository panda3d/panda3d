
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
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletSphereShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletGenericConstraint
from panda3d.bullet import BulletDebugNode

class Game(DirectObject):

  def __init__(self):
    base.setBackgroundColor(0.1, 0.1, 0.8, 1)
    base.setFrameRateMeter(True)

    base.cam.setPos(0, -20, 5)
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

    self.accept('enter', self.doShoot)

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

  def doShoot(self):
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
    v *= 100.0

    # Create bullet
    shape = BulletSphereShape(0.3)
    body = BulletRigidBodyNode('Bullet')
    bodyNP = self.worldNP.attachNewNode(body)
    bodyNP.node().addShape(shape)
    bodyNP.node().setMass(1.0)
    bodyNP.node().setLinearVelocity(v)
    bodyNP.node().setCcdMotionThreshold(1e-7);
    bodyNP.node().setCcdSweptSphereRadius(0.50);
    bodyNP.setCollideMask(BitMask32.allOn())
    bodyNP.setPos(pFrom)

    visNP = loader.loadModel('models/ball.egg')
    visNP.setScale(0.8)
    visNP.reparentTo(bodyNP)

    self.world.attachRigidBody(bodyNP.node())

    # Remove the bullet again after 2 seconds
    taskMgr.doMethodLater(2, self.doRemove, 'doRemove',
      extraArgs=[bodyNP],
      appendTask=True)

  def doRemove(self, bodyNP, task):
    self.world.removeRigidBody(bodyNP.node())
    bodyNP.removeNode()
    return task.done

  # ____TASK___

  def update(self, task):
    dt = globalClock.getDt()
    self.world.doPhysics(dt, 20, 1.0/180.0)
    return task.cont

  def cleanup(self):
    self.worldNP.removeNode()
    self.worldNP = None
    self.world = None

  def setup(self):
    self.worldNP = render.attachNewNode('World')

    # World
    self.debugNP = self.worldNP.attachNewNode(BulletDebugNode('Debug'))
    self.debugNP.show()
    self.debugNP.node().showWireframe(True)
    self.debugNP.node().showConstraints(True)
    self.debugNP.node().showBoundingBoxes(False)
    self.debugNP.node().showNormals(False)

    self.world = BulletWorld()
    self.world.setGravity(Vec3(0, 0, -9.81))
    self.world.setDebugNode(self.debugNP.node())

    # Box A
    shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5))

    bodyA = BulletRigidBodyNode('Box A')
    bodyNP = self.worldNP.attachNewNode(bodyA)
    bodyNP.node().addShape(shape)
    bodyNP.setCollideMask(BitMask32.allOn())
    bodyNP.setPos(-4, 0, 4)

    visNP = loader.loadModel('models/box.egg')
    visNP.clearModelNodes()
    visNP.reparentTo(bodyNP)

    self.world.attachRigidBody(bodyA)

    # Box B
    shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5))

    bodyB = BulletRigidBodyNode('Box B')
    bodyNP = self.worldNP.attachNewNode(bodyB)
    bodyNP.node().addShape(shape)
    bodyNP.node().setMass(1.0)
    bodyNP.node().setDeactivationEnabled(False)
    bodyNP.setCollideMask(BitMask32.allOn())
    bodyNP.setPos(1, 0, 0)

    visNP = loader.loadModel('models/box.egg')
    visNP.clearModelNodes()
    visNP.reparentTo(bodyNP)

    self.world.attachRigidBody(bodyB)

    # Generic
    frameA = TransformState.makePosHpr(Point3(4, 0, 0), Vec3(0, 0, 0))
    frameB = TransformState.makePosHpr(Point3(2, 0, 0), Vec3(0, 0, 0))

    generic = BulletGenericConstraint(bodyA, bodyB, frameA, frameB, True)
    generic.setDebugDrawSize(2.0)
    generic.setLinearLimit(0, 0, 0)
    generic.setLinearLimit(1, 0, 3)
    generic.setLinearLimit(2, 0, 1)
    generic.setAngularLimit(0, -60, 60)
    generic.setAngularLimit(1, -30, 30)
    generic.setAngularLimit(2, -120, 120)

    self.world.attachConstraint(generic)

game = Game()
run()

