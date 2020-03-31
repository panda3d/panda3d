"""
NonPhysicsWalker.py is for avatars.

A walker control such as this one provides:

- creation of the collision nodes
- handling the keyboard and mouse input for avatar movement
- moving the avatar

it does not:

- play sounds
- play animations

although it does send messages that allow a listener to play sounds or
animations based on walker events.
"""

from direct.directnotify import DirectNotifyGlobal
from direct.showbase import DirectObject
from direct.controls.ControlManager import CollisionHandlerRayStart
from direct.showbase.InputStateGlobal import inputState
from direct.task.Task import Task
from panda3d.core import *

class NonPhysicsWalker(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("NonPhysicsWalker")
    wantDebugIndicator = ConfigVariableBool('want-avatar-physics-indicator', False)

    # Ghost mode overrides this:
    slideName = "slide-is-disabled"

    # special methods
    def __init__(self):
        DirectObject.DirectObject.__init__(self)
        self.worldVelocity = Vec3.zero()
        self.collisionsActive = 0
        self.speed=0.0
        self.rotationSpeed=0.0
        self.slideSpeed=0.0
        self.vel=Vec3(0.0, 0.0, 0.0)
        self.stopThisFrame = 0

    def setWalkSpeed(self, forward, jump, reverse, rotate):
        assert self.debugPrint("setWalkSpeed()")
        self.avatarControlForwardSpeed=forward
        #self.avatarControlJumpForce=jump
        self.avatarControlReverseSpeed=reverse
        self.avatarControlRotateSpeed=rotate

    def getSpeeds(self):
        #assert self.debugPrint("getSpeeds()")
        return (self.speed, self.rotationSpeed, self.slideSpeed)

    def setAvatar(self, avatar):
        self.avatar = avatar
        if avatar is not None:
            pass # setup the avatar

    def setAirborneHeightFunc(self, getAirborneHeight):
        self.getAirborneHeight = getAirborneHeight

    def setWallBitMask(self, bitMask):
        self.cSphereBitMask = bitMask

    def setFloorBitMask(self, bitMask):
        self.cRayBitMask = bitMask

    def swapFloorBitMask(self, oldMask, newMask):
        self.cRayBitMask = self.cRayBitMask &~ oldMask
        self.cRayBitMask |= newMask

        if self.cRayNodePath and not self.cRayNodePath.isEmpty():
            self.cRayNodePath.node().setFromCollideMask(self.cRayBitMask)

    def initializeCollisions(self, collisionTraverser, avatarNodePath,
            avatarRadius = 1.4, floorOffset = 1.0, reach = 1.0):
        """
        Set up the avatar for collisions
        """
        assert not avatarNodePath.isEmpty()

        self.cTrav = collisionTraverser
        self.avatarNodePath = avatarNodePath

        # Set up the collision sphere
        # This is a sphere on the ground to detect barrier collisions
        self.cSphere = CollisionSphere(0.0, 0.0, 0.0, avatarRadius)
        cSphereNode = CollisionNode('NPW.cSphereNode')
        cSphereNode.addSolid(self.cSphere)
        self.cSphereNodePath = avatarNodePath.attachNewNode(cSphereNode)

        cSphereNode.setFromCollideMask(self.cSphereBitMask)
        cSphereNode.setIntoCollideMask(BitMask32.allOff())

        # Set up the collison ray
        # This is a ray cast from your head down to detect floor polygons.
        # This ray start is arbitrarily high in the air.  Feel free to use
        # a higher or lower value depending on whether you want an avatar
        # that is outside of the world to step up to the floor when they
        # get under valid floor:
        self.cRay = CollisionRay(0.0, 0.0, CollisionHandlerRayStart, 0.0, 0.0, -1.0)
        cRayNode = CollisionNode('NPW.cRayNode')
        cRayNode.addSolid(self.cRay)
        self.cRayNodePath = avatarNodePath.attachNewNode(cRayNode)
        cRayNode.setFromCollideMask(self.cRayBitMask)
        cRayNode.setIntoCollideMask(BitMask32.allOff())

        # set up wall collision mechanism
        self.pusher = CollisionHandlerPusher()
        self.pusher.setInPattern("enter%in")
        self.pusher.setOutPattern("exit%in")

        # set up floor collision mechanism
        self.lifter = CollisionHandlerFloor()
        self.lifter.setInPattern("on-floor")
        self.lifter.setOutPattern("off-floor")
        self.lifter.setOffset(floorOffset)
        self.lifter.setReach(reach)

        # Limit our rate-of-fall with the lifter.
        # If this is too low, we actually "fall" off steep stairs
        # and float above them as we go down. I increased this
        # from 8.0 to 16.0 to prevent this
        self.lifter.setMaxVelocity(16.0)

        self.pusher.addCollider(self.cSphereNodePath, avatarNodePath)
        self.lifter.addCollider(self.cRayNodePath, avatarNodePath)

        # activate the collider with the traverser and pusher
        self.setCollisionsActive(1)

    def deleteCollisions(self):
        del self.cTrav

        del self.cSphere
        self.cSphereNodePath.removeNode()
        del self.cSphereNodePath

        del self.cRay
        self.cRayNodePath.removeNode()
        del self.cRayNodePath

        del self.pusher
        del self.lifter

    def setTag(self, key, value):
        self.cSphereNodePath.setTag(key, value)

    def setCollisionsActive(self, active = 1):
        assert self.debugPrint("setCollisionsActive(active%s)"%(active,))
        if self.collisionsActive != active:
            self.collisionsActive = active
            if active:
                self.cTrav.addCollider(self.cSphereNodePath, self.pusher)
                self.cTrav.addCollider(self.cRayNodePath, self.lifter)
            else:
                self.cTrav.removeCollider(self.cSphereNodePath)
                self.cTrav.removeCollider(self.cRayNodePath)

                # Now that we have disabled collisions, make one more pass
                # right now to ensure we aren't standing in a wall.
                self.oneTimeCollide()

    def placeOnFloor(self):
        """
        Make a reasonable effor to place the avatar on the ground.
        For example, this is useful when switching away from the
        current walker.
        """
        # With these on, getAirborneHeight is not returning the correct value so
        # when we open our book while swimming we pop down underneath the ground
        # self.oneTimeCollide()
        # self.avatarNodePath.setZ(self.avatarNodePath.getZ()-self.getAirborneHeight())
        # Since this is the non physics walker - wont they already be on the ground?
        return

    def oneTimeCollide(self):
        """
        Makes one quick collision pass for the avatar, for instance as
        a one-time straighten-things-up operation after collisions
        have been disabled.
        """
        tempCTrav = CollisionTraverser("oneTimeCollide")
        tempCTrav.addCollider(self.cSphereNodePath, self.pusher)
        tempCTrav.addCollider(self.cRayNodePath, self.lifter)
        tempCTrav.traverse(render)

    def addBlastForce(self, vector):
        pass

    def displayDebugInfo(self):
        """
        For debug use.
        """
        onScreenDebug.add("controls", "NonPhysicsWalker")

    def _calcSpeeds(self):
        # get the button states:
        forward = inputState.isSet("forward")
        reverse = inputState.isSet("reverse")
        turnLeft = inputState.isSet("turnLeft")
        turnRight = inputState.isSet("turnRight")
        slide = inputState.isSet(self.slideName) or 0
        #jump = inputState.isSet("jump")

        # Check for Auto-Run
        if base.localAvatar.getAutoRun():
            forward = 1
            reverse = 0

        # Determine what the speeds are based on the buttons:
        self.speed=(forward and self.avatarControlForwardSpeed or
                    reverse and -self.avatarControlReverseSpeed)
        # Should fSlide be renamed slideButton?
        self.slideSpeed=slide and ((reverse and turnLeft and -self.avatarControlReverseSpeed*(0.75)) or
                                   (reverse and turnRight and self.avatarControlReverseSpeed*(0.75)) or
                                   (turnLeft and -self.avatarControlForwardSpeed*(0.75)) or
                                   (turnRight and self.avatarControlForwardSpeed*(0.75)))
        self.rotationSpeed=not slide and (
                (turnLeft and self.avatarControlRotateSpeed) or
                (turnRight and -self.avatarControlRotateSpeed))

    def handleAvatarControls(self, task):
        """
        Check on the arrow keys and update the avatar.
        """
        if not self.lifter.hasContact():
            # hack fix for falling through the floor:
            messenger.send("walkerIsOutOfWorld", [self.avatarNodePath])

        self._calcSpeeds()

        if __debug__:
            debugRunning = inputState.isSet("debugRunning")
            if debugRunning:
                self.speed*=4.0
                self.slideSpeed*=4.0
                self.rotationSpeed*=1.25

        if self.wantDebugIndicator:
            self.displayDebugInfo()
        # How far did we move based on the amount of time elapsed?
        dt=ClockObject.getGlobalClock().getDt()
        # Check to see if we're moving at all:
        if self.speed or self.slideSpeed or self.rotationSpeed:
            if self.stopThisFrame:
                distance = 0.0
                slideDistance = 0.0
                rotation = 0.0
                self.stopThisFrame = 0
            else:
                distance = dt * self.speed
                slideDistance = dt * self.slideSpeed
                rotation = dt * self.rotationSpeed

            # Take a step in the direction of our previous heading.
            self.vel=Vec3(Vec3.forward() * distance +
                          Vec3.right() * slideDistance)
            if self.vel != Vec3.zero():
                # rotMat is the rotation matrix corresponding to
                # our previous heading.
                rotMat=Mat3.rotateMatNormaxis(self.avatarNodePath.getH(), Vec3.up())
                step=rotMat.xform(self.vel)
                self.avatarNodePath.setFluidPos(Point3(self.avatarNodePath.getPos()+step))
            self.avatarNodePath.setH(self.avatarNodePath.getH()+rotation)
            messenger.send("avatarMoving")
        else:
            self.vel.set(0.0, 0.0, 0.0)

        self.__oldPosDelta = self.avatarNodePath.getPosDelta(render)
        self.__oldDt = dt

        try:
            self.worldVelocity = self.__oldPosDelta*(1/self.__oldDt)
        except:
            # divide by zero
            self.worldVelocity = 0

        return Task.cont

    def doDeltaPos(self):
        assert self.debugPrint("doDeltaPos()")

    def reset(self):
        assert self.debugPrint("reset()")

    def getVelocity(self):
        return self.vel

    def enableAvatarControls(self):
        """
        Activate the arrow keys, etc.
        """
        assert self.debugPrint("enableAvatarControls")
        assert self.collisionsActive

        taskName = "AvatarControls-%s"%(id(self),)
        # remove any old
        taskMgr.remove(taskName)
        # spawn the new task
        taskMgr.add(self.handleAvatarControls, taskName)

    def disableAvatarControls(self):
        """
        Ignore the arrow keys, etc.
        """
        assert self.debugPrint("disableAvatarControls")
        taskName = "AvatarControls-%s"%(id(self),)
        taskMgr.remove(taskName)

    def flushEventHandlers(self):
        if hasattr(self, 'cTrav'):
            self.pusher.flush()
        self.lifter.flush() # not currently defined or needed

    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(id(self))+' '+message)
