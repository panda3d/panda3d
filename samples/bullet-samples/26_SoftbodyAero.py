
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
from panda3d.core import GeomNode
from panda3d.core import GeomVertexFormat

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

    base.cam.setPos(0, -80, 40)
    base.cam.lookAt(0, 0, 10)

    # Light
    alight = AmbientLight('ambientLight')
    alight.setColor(Vec4(0.5, 0.5, 0.5, 1))
    alightNP = render.attachNewNode(alight)

    dlight = DirectionalLight('directionalLight')
    dlight.setDirection(Vec3(0, 0, -1))
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

  @staticmethod
  def Vec3Rand():
    x = 2 * random.random() - 1
    y = 2 * random.random() - 1
    z = 2 * random.random() - 1
    return Vec3(x, y, z)

  def setup(self):
    self.worldNP = render.attachNewNode('World')

    # World
    self.debugNP = self.worldNP.attachNewNode(BulletDebugNode('Debug'))
    self.debugNP.show()

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

    # Soft body world information
    info = self.world.getWorldInfo()
    info.setAirDensity(1.2)
    info.setWaterDensity(0)
    info.setWaterOffset(0)
    info.setWaterNormal(Vec3(0, 0, 0))

    # Softbody
    for i in range(50):
      p00 = Point3(-2, -2, 0)
      p10 = Point3( 2, -2, 0)
      p01 = Point3(-2,  2, 0)
      p11 = Point3( 2,  2, 0)
      node = BulletSoftBodyNode.makePatch(info, p00, p10, p01, p11, 6, 6, 0, True)
      node.generateBendingConstraints(2)
      node.getCfg().setLiftCoefficient(0.004)
      node.getCfg().setDynamicFrictionCoefficient(0.0003)
      node.getCfg().setAeroModel(BulletSoftBodyConfig.AMVertexTwoSided)
      node.setTotalMass(0.1)
      node.addForce(Vec3(0, 2, 0), 0)

      np = self.worldNP.attachNewNode(node)
      np.setPos(self.Vec3Rand() * 10 + Vec3(0, 0, 20))
      np.setHpr(self.Vec3Rand() * 16)
      self.world.attachSoftBody(node)

      fmt = GeomVertexFormat.getV3n3t2()
      geom = BulletHelper.makeGeomFromFaces(node, fmt, True)
      node.linkGeom(geom)
      nodeV = GeomNode('')
      nodeV.addGeom(geom)
      npV = np.attachNewNode(nodeV)

      #tex = loader.loadTexture('models/panda.jpg')
      #npV.setTexture(tex)
      #BulletHelper.makeTexcoordsForPatch(geom, 6, 6)

game = Game()
run()

