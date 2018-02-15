
from pandac.PandaModules import loadPrcFileData

loadPrcFileData("", "sync-video t")

#from pandac.PandaModules import loadPrcFileData
#loadPrcFileData('', 'load-display tinydisplay')

import sys
import time
import direct.directbase.DirectStart

from direct.actor.Actor import Actor
from direct.showbase.DirectObject import DirectObject
from direct.showbase.InputStateGlobal import inputState

from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import Vec3
from panda3d.core import Vec4
from panda3d.core import Point3
from panda3d.core import TransformState
from panda3d.core import BitMask32
from panda3d.core import Filename
from panda3d.core import PNMImage

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletCapsuleShape
from panda3d.bullet import BulletCharacterControllerNode
from panda3d.bullet import BulletHeightfieldShape
from panda3d.bullet import BulletTriangleMesh
from panda3d.bullet import BulletTriangleMeshShape
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

    self.accept('space', self.doJump)
    self.accept('c', self.doCrouch)

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

  def doJump(self):
    self.character.setMaxJumpHeight(5.0)
    self.character.setJumpSpeed(8.0)
    self.character.doJump()

  def doCrouch(self):
    self.crouching = not self.crouching
    sz = self.crouching and 0.6 or 1.0

    self.characterNP.setScale(Vec3(1, 1, sz))

    #self.character.getShape().setLocalScale(Vec3(1, 1, sz))
    #self.characterNP.setScale(Vec3(1, 1, sz) * 0.3048)
    #self.characterNP.setPos(0, 0, -1 * sz)

  # ____TASK___

  def processInput(self, dt):
    speed = Vec3(0, 0, 0)
    omega = 0.0

    if inputState.isSet('forward'): speed.setY( 2.0)
    if inputState.isSet('reverse'): speed.setY(-2.0)
    if inputState.isSet('left'):    speed.setX(-2.0)
    if inputState.isSet('right'):   speed.setX( 2.0)
    if inputState.isSet('turnLeft'):  omega =  120.0
    if inputState.isSet('turnRight'): omega = -120.0

    self.character.setAngularMovement(omega)
    self.character.setLinearMovement(speed, True)

  def update(self, task):
    dt = globalClock.getDt()

    self.processInput(dt)
    self.world.doPhysics(dt, 4, 1./240.)

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
    shape = BulletPlaneShape(Vec3(0, 0, 1), 0)

    #img = PNMImage(Filename('models/elevation2.png'))
    #shape = BulletHeightfieldShape(img, 1.0, ZUp)

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Ground'))
    np.node().addShape(shape)
    np.setPos(0, 0, -1)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    # Box
    shape = BulletBoxShape(Vec3(1.0, 3.0, 0.3))

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Box'))
    np.node().setMass(10.0)
    np.node().addShape(shape)
    np.setPos(3, 0, 4)
    np.setH(20.0)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    # Character
    self.crouching = False

    h = 1.75
    w = 0.4
    shape = BulletCapsuleShape(w, h - 2 * w, ZUp)

    self.character = BulletCharacterControllerNode(shape, 0.4, 'Player')
    self.character.setMass(1.0)
    self.characterNP = self.worldNP.attachNewNode(self.character)
    self.characterNP.setPos(-2, 0, 14)
    self.characterNP.setH(45)
    self.characterNP.setCollideMask(BitMask32.allOn())
    self.world.attachCharacter(self.character)

    self.actorNP = Actor('models/ralph/ralph.egg', {
                         'run' : 'models/ralph/ralph-run.egg',
                         'walk' : 'models/ralph/ralph-walk.egg',
                         'jump' : 'models/ralph/ralph-jump.egg'})
    self.actorNP.reparentTo(self.characterNP)
    self.actorNP.setScale(0.3048) # 1ft = 0.3048m
    self.actorNP.setH(180)
    self.actorNP.setPos(0, 0, -1)

game = Game()
run()

