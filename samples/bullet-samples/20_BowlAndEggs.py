
#from pandac.PandaModules import loadPrcFileData
#loadPrcFileData('', 'load-display tinydisplay')

import sys
import random
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
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletConvexHullShape
from panda3d.bullet import BulletTriangleMesh
from panda3d.bullet import BulletTriangleMeshShape
from panda3d.bullet import ZUp

class Game(DirectObject):

  def __init__(self):
    base.setBackgroundColor(0.1, 0.1, 0.8, 1)
    base.setFrameRateMeter(True)

    base.cam.setPos(0, -10, 5)
    base.cam.lookAt(0, 0, 0.2)

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

    inputState.watchWithModifiers('up', 'w')
    inputState.watchWithModifiers('left', 'a')
    inputState.watchWithModifiers('down', 's')
    inputState.watchWithModifiers('right', 'd')

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

    if inputState.isSet('up'): force.setY( 1.0)
    if inputState.isSet('down'): force.setY(-1.0)
    if inputState.isSet('left'):    force.setX(-1.0)
    if inputState.isSet('right'):   force.setX( 1.0)

    force *= 300.0

    self.bowlNP.node().setActive(True)
    self.bowlNP.node().applyCentralForce(force)

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
    self.debugNP.node().showWireframe(True)
    self.debugNP.node().showConstraints(True)
    self.debugNP.node().showBoundingBoxes(False)
    self.debugNP.node().showNormals(False)

    self.world = BulletWorld()
    self.world.setGravity(Vec3(0, 0, -9.81))
    self.world.setDebugNode(self.debugNP.node())

    # Ground
    shape = BulletPlaneShape(Vec3(0, 0, 1), 0)

    body = BulletRigidBodyNode('Ground')
    bodyNP = self.worldNP.attachNewNode(body)
    bodyNP.node().addShape(shape)
    bodyNP.setPos(0, 0, 0)
    bodyNP.setCollideMask(BitMask32.allOn())
    self.world.attachRigidBody(bodyNP.node())

    # Bowl
    visNP = loader.loadModel('models/bowl.egg')

    geom = visNP.findAllMatches('**/+GeomNode').getPath(0).node().getGeom(0)
    mesh = BulletTriangleMesh()
    mesh.addGeom(geom)
    shape = BulletTriangleMeshShape(mesh, dynamic=True)

    body = BulletRigidBodyNode('Bowl')
    bodyNP = self.worldNP.attachNewNode(body)
    bodyNP.node().addShape(shape)
    bodyNP.node().setMass(10.0)
    bodyNP.setPos(0, 0, 0)
    bodyNP.setCollideMask(BitMask32.allOn())
    self.world.attachRigidBody(bodyNP.node())

    visNP.reparentTo(bodyNP)

    self.bowlNP = bodyNP
    self.bowlNP.setScale(2)

    # Eggs
    self.eggNPs = []
    for i in range(5):
      x = random.gauss(0, 0.1)
      y = random.gauss(0, 0.1)
      z = random.gauss(0, 0.1) + 1
      h = random.random() * 360
      p = random.random() * 360
      r = random.random() * 360

      visNP = loader.loadModel('models/egg.egg')

      geom = visNP.findAllMatches('**/+GeomNode').getPath(0).node().getGeom(0)
      shape = BulletConvexHullShape()
      shape.addGeom(geom)

      body = BulletRigidBodyNode('Egg-%i' % i)
      bodyNP = self.worldNP.attachNewNode(body)
      bodyNP.node().setMass(1.0)
      bodyNP.node().addShape(shape)
      bodyNP.node().setDeactivationEnabled(False)
      bodyNP.setCollideMask(BitMask32.allOn())
      bodyNP.setPosHpr(x, y, z, h, p, r)
      #bodyNP.setScale(1.5)
      self.world.attachRigidBody(bodyNP.node())

      visNP.reparentTo(bodyNP)

      self.eggNPs.append(bodyNP)

game = Game()
run()

