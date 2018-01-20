
#from pandac.PandaModules import loadPrcFileData
#loadPrcFileData('', 'load-display tinydisplay')

import sys
import direct.directbase.DirectStart

from direct.showbase.DirectObject import DirectObject
from direct.interval.MetaInterval import Sequence
from direct.interval.MetaInterval import Parallel
from direct.interval.LerpInterval import LerpFunc
from direct.interval.FunctionInterval import Func
from direct.interval.FunctionInterval import Wait

from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import Material
from panda3d.core import Vec3
from panda3d.core import Vec4
from panda3d.core import Point3

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletGhostNode
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletHelper

class BallInMaze(DirectObject):

  def __init__(self):
    base.setBackgroundColor(0.1, 0.1, 0.8, 1)
    base.setFrameRateMeter(True)

    base.cam.setPosHpr(0, 0, 25, 0, -90, 0)
    base.disableMouse()

    # Input
    self.accept('escape', self.exitGame)
    self.accept('f1', self.toggleWireframe)
    self.accept('f2', self.toggleTexture)
    self.accept('f3', self.toggleDebug)
    self.accept('f5', self.doScreenshot)

    # Setup scene 1: World
    self.debugNP = render.attachNewNode(BulletDebugNode('Debug'))
    self.debugNP.node().showWireframe(True)
    self.debugNP.node().showConstraints(True)
    self.debugNP.node().showBoundingBoxes(True)
    self.debugNP.node().showNormals(True)
    self.debugNP.show()

    self.world = BulletWorld()
    self.world.setGravity(Vec3(0, 0, -9.81))
    self.world.setDebugNode(self.debugNP.node())

    # Setup scene 2: Ball
    visNP = loader.loadModel('models/ball.egg')
    visNP.clearModelNodes()

    bodyNPs = BulletHelper.fromCollisionSolids(visNP, True)
    self.ballNP = bodyNPs[0]
    self.ballNP.reparentTo(render)
    self.ballNP.node().setMass(1.0)
    self.ballNP.setPos(4, -4, 1)
    self.ballNP.node().setDeactivationEnabled(False)

    visNP.reparentTo(self.ballNP)

    # Setup scene 3: Maze
    visNP = loader.loadModel('models/maze.egg')
    visNP.clearModelNodes()
    visNP.reparentTo(render)

    self.holes = []
    self.maze = []
    self.mazeNP = visNP

    bodyNPs = BulletHelper.fromCollisionSolids(visNP, True);
    for bodyNP in bodyNPs:
      bodyNP.reparentTo(render)

      if isinstance(bodyNP.node(), BulletRigidBodyNode):
        bodyNP.node().setMass(0.0)
        bodyNP.node().setKinematic(True)
        self.maze.append(bodyNP)

      elif isinstance(bodyNP.node(), BulletGhostNode):
        self.holes.append(bodyNP)

    # Lighting and material for the ball
    ambientLight = AmbientLight('ambientLight')
    ambientLight.setColor(Vec4(0.55, 0.55, 0.55, 1))
    directionalLight = DirectionalLight('directionalLight')
    directionalLight.setDirection(Vec3(0, 0, -1))
    directionalLight.setColor(Vec4(0.375, 0.375, 0.375, 1))
    directionalLight.setSpecularColor(Vec4(1, 1, 1, 1))
    self.ballNP.setLight(render.attachNewNode(ambientLight))
    self.ballNP.setLight(render.attachNewNode(directionalLight))

    m = Material()
    m.setSpecular(Vec4(1,1,1,1))
    m.setShininess(96)
    self.ballNP.setMaterial(m, 1)

    # Startup
    self.startGame()

  def exitGame(self):
    sys.exit()

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

  def startGame(self):
    self.ballNP.setPos(4, -4, 1)
    self.ballNP.node().setLinearVelocity(Vec3(0, 0, 0))
    self.ballNP.node().setAngularVelocity(Vec3(0, 0, 0))

    # Mouse
    p = base.win.getProperties()
    base.win.movePointer(0, p.getXSize()/2, p.getYSize()/2)

    # Add bodies and ghosts
    self.world.attachRigidBody(self.ballNP.node())
    for bodyNP in self.maze:
      self.world.attachRigidBody(bodyNP.node())
    for ghostNP in self.holes:
      self.world.attachGhost(ghostNP.node())

    # Simulation task
    taskMgr.add(self.updateGame, 'updateGame')

  def stopGame(self):

    # Remove bodies and ghosts
    self.world.removeRigidBody(self.ballNP.node())
    for bodyNP in self.maze:
      self.world.removeRigidBody(bodyNP.node())
    for ghostNP in self.holes:
      self.world.removeGhost(ghostNP.node())

    # Simulation task
    taskMgr.remove('updateGame')

  def updateGame(self, task):
    dt = globalClock.getDt()

    # Get mouse position and tilt maze
    if base.mouseWatcherNode.hasMouse():
      mpos = base.mouseWatcherNode.getMouse()
      hpr = Vec3(0, mpos.getY() * -10, mpos.getX() * 10)

      # Maze visual node
      self.mazeNP.setHpr(hpr)

      # Maze body nodes
      for bodyNP in self.maze:
        bodyNP.setHpr(hpr)

    # Update simulation
    self.world.doPhysics(dt)

    # Check if ball is touching a hole
    for holeNP in self.holes:
      if holeNP.node().getNumOverlappingNodes() > 2:
        if (self.ballNP.node() in holeNP.node().getOverlappingNodes()):
          self.looseGame(holeNP)

    return task.cont

  def looseGame(self, holeNP):
    toPos = holeNP.node().getShapePos(0)
    self.stopGame()

    Sequence(
      Parallel(
      LerpFunc(self.ballNP.setX, fromData = self.ballNP.getX(),
               toData = toPos.getX(), duration = .1),
      LerpFunc(self.ballNP.setY, fromData = self.ballNP.getY(),
               toData = toPos.getY(), duration = .1),
      LerpFunc(self.ballNP.setZ, fromData = self.ballNP.getZ(),
               toData = self.ballNP.getZ() - .9, duration = .2)),
      Wait(1),
      Func(self.startGame)).start()

game = BallInMaze()
run()

