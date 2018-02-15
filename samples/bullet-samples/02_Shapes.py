
#from pandac.PandaModules import loadPrcFileData
#loadPrcFileData('', 'load-display tinydisplay')
#loadPrcFileData('', 'bullet-enable-contact-events true')

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
from panda3d.bullet import BulletSphereShape
from panda3d.bullet import BulletCapsuleShape
from panda3d.bullet import BulletCylinderShape
from panda3d.bullet import BulletConeShape
from panda3d.bullet import BulletConvexHullShape
from panda3d.bullet import BulletTriangleMesh
from panda3d.bullet import BulletTriangleMeshShape
from panda3d.bullet import BulletMultiSphereShape
from panda3d.bullet import XUp
from panda3d.bullet import YUp
from panda3d.bullet import ZUp

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

    #pairs = [(mf.getNode0().getName(),
    #          mf.getNode1().getName())
    #  for mf in self.world.getManifolds() if mf.getNumManifoldPoints() > 0]
    #print pairs

    return task.cont

  def cleanup(self):
    self.world = None
    self.worldNP.removeNode()

  #def doAdded(self, node1, node2):
  #  print 'added:', node1.getName(), node2.getName()

  #def doDestroyed(self, node1, node2):
  #  print 'destroyed:', node1.getName(), node2.getName()

  def setup(self):
    self.worldNP = render.attachNewNode('World')

    # World
    self.debugNP = self.worldNP.attachNewNode(BulletDebugNode('Debug'))
    self.debugNP.show()
    self.debugNP.node().showWireframe(True)
    self.debugNP.node().showConstraints(True)
    self.debugNP.node().showBoundingBoxes(False)
    self.debugNP.node().showNormals(True)

    self.world = BulletWorld()
    self.world.setGravity(Vec3(0, 0, -9.81))
    self.world.setDebugNode(self.debugNP.node())

    # Plane (static)
    shape = BulletPlaneShape(Vec3(0, 0, 1), 0)

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Ground'))
    np.node().addShape(shape)
    np.setPos(0, 0, -1)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    # Box (dynamic)
    shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5))

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Box'))
    np.node().setMass(1.0)
    np.node().addShape(shape)
    np.setPos(0, 0, 4)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    self.boxNP = np # For applying force & torque

    #np.node().notifyCollisions(True)
    #self.accept('bullet-contact-added', self.doAdded)
    #self.accept('bullet-contact-destroyed', self.doRemoved)

    # Sphere (dynamic)
    shape = BulletSphereShape(0.6)

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Sphere'))
    np.node().setMass(1.0)
    np.node().addShape(shape)
    np.setPos(-4, 0, 4)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    # Cone (dynamic)
    shape = BulletConeShape(0.6, 1.2, ZUp)

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Cone'))
    np.node().setMass(1.0)
    np.node().addShape(shape)
    np.setPos(4, 0, 4)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    # Capsule (dynamic)
    shape = BulletCapsuleShape(0.5, 1.0, ZUp)

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Capsule'))
    np.node().setMass(1.0)
    np.node().addShape(shape)
    np.setPos(0, 4, 4)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    # Cyliner (dynamic)
    shape = BulletCylinderShape(0.7, 1.5, ZUp)

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Cylinder'))
    np.node().setMass(1.0)
    np.node().addShape(shape)
    np.setPos(4, 4, 4)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    # Convex (dynamic)
    shape = BulletConvexHullShape()
    shape.addPoint(Point3(1, 1, 2))
    shape.addPoint(Point3(0, 0, 0))
    shape.addPoint(Point3(2, 0, 0))
    shape.addPoint(Point3(0, 2, 0))
    shape.addPoint(Point3(2, 2, 0))

    # Another way to create the convex hull shape:
    #shape = BulletConvexHullShape()
    #shape.addArray([
    #  Point3(1, 1, 2),
    #  Point3(0, 0, 0),
    #  Point3(2, 0, 0),
    #  Point3(0, 2, 0),
    #  Point3(2, 2, 0),
    #])

    # Yet another way to create the convex hull shape:
    #geom = loader.loadModel('models/box.egg')\
    #         .findAllMatches('**/+GeomNode')\
    #         .getPath(0)\
    #         .node()\
    #         .getGeom(0)
    #shape = BulletConvexHullShape()
    #shape.addGeom(geom)

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Convex'))
    np.node().setMass(1.0)
    np.node().addShape(shape)
    np.setPos(-4, 4, 4)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    # Mesh (static)
    p0 = Point3(-10, -10, 0)
    p1 = Point3(-10, 10, 0)
    p2 = Point3(10, -10, 0)
    p3 = Point3(10, 10, 0)
    mesh = BulletTriangleMesh()
    mesh.addTriangle(p0, p1, p2)
    mesh.addTriangle(p1, p2, p3)
    shape = BulletTriangleMeshShape(mesh, dynamic=False)

    # Another way to create the triangle mesh shape:
    #geom = loader.loadModel('models/box.egg')\
    #         .findAllMatches('**/+GeomNode')\
    #         .getPath(0)\
    #         .node()\
    #         .getGeom(0)
    #mesh = BulletTriangleMesh()
    #mesh.addGeom(geom)
    #shape = BulletTriangleMeshShape(mesh, dynamic=False)

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Mesh'))
    np.node().addShape(shape)
    np.setPos(0, 0, 0.1)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    # MutiSphere
    points = [Point3(-1, 0, 0), Point3(0, 0, 0), Point3(1, 0, 0)]
    radii = [.4, .8, .6]
    shape = BulletMultiSphereShape(points, radii)

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('MultiSphere'))
    np.node().setMass(1.0)
    np.node().addShape(shape)
    np.setPos(8, 0, 4)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

game = Game()
run()

