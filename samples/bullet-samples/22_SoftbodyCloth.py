
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
from panda3d.core import GeomVertexFormat
from panda3d.core import GeomVertexRewriter
from panda3d.core import InternalName
from panda3d.core import NurbsSurfaceEvaluator
from panda3d.core import SheetNode

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

    base.cam.setPos(0, -40, 10)
    base.cam.lookAt(0, 0, 0)

    # Light
    alight = AmbientLight('ambientLight')
    alight.setColor(Vec4(0.5, 0.5, 0.5, 1))
    alightNP = render.attachNewNode(alight)

    dlight = DirectionalLight('directionalLight')
    dlight.setDirection(Vec3(5, 0, -2))
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

    self.world.doPhysics(dt, 10, 0.004)

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

    # Box
    shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5) * 2.0)

    boxNP = self.worldNP.attachNewNode(BulletRigidBodyNode('Box'))
    boxNP.node().setMass(150.0)
    boxNP.node().addShape(shape)
    boxNP.setPos(0, 0, 2)
    boxNP.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(boxNP.node())

    visualNP = loader.loadModel('models/box.egg')
    visualNP.clearModelNodes()
    visualNP.setScale(2.0)
    visualNP.reparentTo(boxNP)

    # Soft body world information
    info = self.world.getWorldInfo()
    info.setAirDensity(1.2)
    info.setWaterDensity(0)
    info.setWaterOffset(0)
    info.setWaterNormal(Vec3(0, 0, 0))

    # Softbody
    nx = 31
    ny = 31

    p00 = Point3(-8, -8, 0)
    p10 = Point3( 8, -8, 0)
    p01 = Point3(-8,  8, 0)
    p11 = Point3( 8,  8, 0)
    bodyNode = BulletSoftBodyNode.makePatch(info, p00, p10, p01, p11, nx, ny, 1+2+4+8, True) 

    material = bodyNode.appendMaterial()
    material.setLinearStiffness(0.4)
    bodyNode.generateBendingConstraints(2, material);
    bodyNode.setTotalMass(50.0)
    bodyNode.getShape(0).setMargin(0.5)

    bodyNP = self.worldNP.attachNewNode(bodyNode)
    self.world.attachSoftBody(bodyNode)

    # Rendering with Geom:
    fmt = GeomVertexFormat.getV3n3t2()
    geom = BulletHelper.makeGeomFromFaces(bodyNode, fmt, True)
    bodyNode.linkGeom(geom)
    visNode = GeomNode('')
    visNode.addGeom(geom)
    visNP = bodyNP.attachNewNode(visNode)

    # Now we want to have a texture and texture coordinates.
    # The geom's format has already a column for texcoords, so we just need
    # to write texcoords using a GeomVertexRewriter.
    tex = loader.loadTexture('models/panda.jpg')
    visNP.setTexture(tex)
    BulletHelper.makeTexcoordsForPatch(geom, nx, ny)

    # Rendering with NURBS (please reduce nx & ny to 12):
    #surface = NurbsSurfaceEvaluator()
    #surface.reset(nx, ny)
    #bodyNode.linkSurface(surface)

    #visNode = SheetNode('')
    #visNode.setSurface(surface)
    #visNode.setNumUSubdiv(2)
    #visNode.setNumVSubdiv(2)
    #visNP = self.worldNP.attachNewNode(visNode)
    #visNP.setTwoSided(True)

  #def makeTexcoordsForPatch(self, geom, resx, resy):
  #  vdata = geom.modifyVertexData()
  #  texcoords = GeomVertexRewriter(vdata, InternalName.getTexcoord())

  #  i = 0
  #  while (not texcoords.isAtEnd()):
  #    ix, iy = divmod(i, resx)
  #    u = float(ix) / (resx - 1)
  #    v = float(iy) / (resy - 1)
  #    texcoords.setData2f(u, v)
  #    i += 1

game = Game()
run()

