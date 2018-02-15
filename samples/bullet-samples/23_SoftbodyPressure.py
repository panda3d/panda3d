
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
from panda3d.core import GeomNode

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletHelper
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletTriangleMesh
from panda3d.bullet import BulletTriangleMeshShape
from panda3d.bullet import BulletSoftBodyNode
from panda3d.bullet import BulletSoftBodyConfig

class Game(DirectObject):

  def __init__(self):
    base.setBackgroundColor(0.1, 0.1, 0.8, 1)
    base.setFrameRateMeter(True)

    base.cam.setPos(0, -60, 20)
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

  def update(self, task):
    dt = globalClock.getDt()

    self.world.doPhysics(dt, 10, 0.008)

    return task.cont

  def cleanup(self):
    self.world = None
    self.worldNP.removeNode()

  def setup(self):
    self.worldNP = render.attachNewNode('World')

    # World
    self.debugNP = self.worldNP.attachNewNode(BulletDebugNode('Debug'))
    self.debugNP.show()

    #self.debugNP.showTightBounds()
    #self.debugNP.showBounds()

    self.world = BulletWorld()
    self.world.setGravity(Vec3(0, 0, -9.81))
    self.world.setDebugNode(self.debugNP.node())

    # Ground
    p0 = Point3(-20, -20, 0)
    p1 = Point3(-20, 20, 0)
    p2 = Point3(20, -20, 0)
    p3 = Point3(20, 20, 0)
    mesh = BulletTriangleMesh()
    mesh.addTriangle(p0, p1, p2)
    mesh.addTriangle(p1, p2, p3)
    shape = BulletTriangleMeshShape(mesh, dynamic=False)

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Mesh'))
    np.node().addShape(shape)
    np.setPos(0, 0, -2)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    # Stair
    origin = Point3(0, 0, 0)
    size = Vec3(2, 10, 1)
    shape = BulletBoxShape(size * 0.5)
    for i in range(10):
      pos = origin + size * i
      pos.setY(0)

      np = self.worldNP.attachNewNode(BulletRigidBodyNode('Stair%i' % i))
      np.node().addShape(shape)
      np.setPos(pos)
      np.setCollideMask(BitMask32.allOn())

      npV = loader.loadModel('models/box.egg')
      npV.reparentTo(np)
      npV.setScale(size)

      self.world.attachRigidBody(np.node())

    # Soft body world information
    info = self.world.getWorldInfo()
    info.setAirDensity(1.2)
    info.setWaterDensity(0)
    info.setWaterOffset(0)
    info.setWaterNormal(Vec3(0, 0, 0))

    # Softbody
    center = Point3(0, 0, 0)
    radius = Vec3(1, 1, 1) * 1.5
    node = BulletSoftBodyNode.makeEllipsoid(info, center, radius, 128)
    node.setName('Ellipsoid')
    node.getMaterial(0).setLinearStiffness(0.1)
    node.getCfg().setDynamicFrictionCoefficient(1)
    node.getCfg().setDampingCoefficient(0.001)
    node.getCfg().setPressureCoefficient(1500)
    node.setTotalMass(30, True)
    node.setPose(True, False)

    np = self.worldNP.attachNewNode(node)
    np.setPos(15, 0, 12)
    #np.setH(90.0)
    #np.showBounds()
    #np.showTightBounds()
    self.world.attachSoftBody(np.node())

    geom = BulletHelper.makeGeomFromFaces(node)
    node.linkGeom(geom)
    nodeV = GeomNode('EllipsoidVisual')
    nodeV.addGeom(geom)
    npV = np.attachNewNode(nodeV)

    #npV.showBounds()

game = Game()
run()

