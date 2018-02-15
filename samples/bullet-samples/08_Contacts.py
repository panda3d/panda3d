
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
from panda3d.core import NodePath

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletSphereShape
from panda3d.bullet import BulletRigidBodyNode

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
    if not self.box:
      return

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

    self.box.setActive(True)
    self.box.applyCentralForce(force)
    self.box.applyTorque(torque)

  def processContacts(self):
    if not self.box or not self.sphere: 
      return

    result = self.world.contactTestPair(self.box, self.sphere)
    #result = self.world.contactTest(self.box)

    #print '-->', result.getNumContacts()
    for contact in result.getContacts():
      cp = contact.getManifoldPoint()
      node0 = contact.getNode0()
      node1 = contact.getNode1()
      print node0.getName(), node1.getName(), cp.getLocalPointA()

      #print contact.getNode0(), cp.getPositionWorldOnA()
      #print contact.getIdx0(), contact.getIdx1(), \
      #      contact.getPartId0(), contact.getPartId1()

      self.removeNode(node1)

  def removeNode(self, node):
      self.world.removeRigidBody(node)
      if node == self.sphere: self.sphere = None
      if node == self.box: self.box = None
      np = NodePath(node)
      np.removeNode()

  def update(self, task):
    dt = globalClock.getDt()

    self.processInput(dt)
    self.world.doPhysics(dt, 10, 0.008)
    self.processContacts()

    return task.cont

  def cleanup(self):
    self.world = None
    self.worldNP.removeNode()

  #def onContactAdded(self, node1, node2):
  #  print 'contact added:', node1, node2

  #def onContactDestroyed(self, node1, node2):
  #  print 'contact destroyed:', node1, node2

  def setup(self):
    self.worldNP = render.attachNewNode('World')

    # World
    self.debugNP = self.worldNP.attachNewNode(BulletDebugNode('Debug'))
    self.debugNP.show()

    self.world = BulletWorld()
    self.world.setGravity(Vec3(0, 0, -9.81))
    self.world.setDebugNode(self.debugNP.node())

    # Plane
    shape = BulletPlaneShape(Vec3(0, 0, 1), 0)

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Ground'))
    np.node().addShape(shape)
    np.setPos(0, 0, -1)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    # Box
    shape = BulletBoxShape(Vec3(0.5, 0.5, 0.5))

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Box'))
    np.node().setMass(1.0)
    np.node().addShape(shape)
    np.node().addShape(shape, TransformState.makePos(Point3(0, 1, 0)))
    np.node().setDeactivationEnabled(False)
    np.setPos(2, 0, 4)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    visualNP = loader.loadModel('models/box.egg')
    visualNP.reparentTo(np)

    self.box = np.node()

    # Sphere
    shape = BulletSphereShape(0.6)

    np = self.worldNP.attachNewNode(BulletRigidBodyNode('Sphere'))
    np.node().setMass(1.0)
    np.node().addShape(shape)
    np.node().addShape(shape, TransformState.makePos(Point3(0, 1, 0)))
    np.setPos(-2, 0, 4)
    np.setCollideMask(BitMask32.allOn())

    self.world.attachRigidBody(np.node())

    self.sphere = np.node()

    ## Enable contact reporting
    #self.accept('bullet-contact-added', self.onContactAdded)
    #self.accept('bullet-contact-destroyed', self.onContactDestroyed)
    #self.box.notifyCollisions(True)

game = Game()
run()

