"""
DevWalker.py is for avatars.

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

from direct.showbase.InputStateGlobal import inputState
from direct.directnotify import DirectNotifyGlobal
from direct.showbase import DirectObject
from direct.task.Task import Task
from panda3d.core import *


class DevWalker(DirectObject.DirectObject):

    notify = DirectNotifyGlobal.directNotify.newCategory("DevWalker")
    wantDebugIndicator = ConfigVariableBool('want-avatar-physics-indicator', False)
    runMultiplier = ConfigVariableDouble('dev-run-multiplier', 4.0)

    # Ghost mode overrides this:
    slideName = "slide-is-disabled"

    # special methods
    def __init__(self):
        DirectObject.DirectObject.__init__(self)
        self.speed=0.0
        self.rotationSpeed=0.0
        self.slideSpeed=0.0
        self.vel=Vec3(0.0, 0.0, 0.0)

        self.task = None

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

    def setWallBitMask(self, bitMask):
        pass

    def setFloorBitMask(self, bitMask):
        pass

    def initializeCollisions(self, collisionTraverser, avatarNodePath,
            wallCollideMask, floorCollideMask,
            avatarRadius = 1.4, floorOffset = 1.0, reach = 1.0):
        assert not avatarNodePath.isEmpty()

        self.cTrav = collisionTraverser
        self.avatarNodePath = avatarNodePath

    def setAirborneHeightFunc(self, getAirborneHeight):
        pass

    def deleteCollisions(self):
        pass

    def setTag(self, key, value):
        pass

    def setCollisionsActive(self, active = 1):
        pass

    def placeOnFloor(self):
        pass

    def oneTimeCollide(self):
        pass

    def addBlastForce(self, vector):
        pass

    def displayDebugInfo(self):
        """
        For debug use.
        """
        onScreenDebug.add("w controls", "DevWalker")

    def handleAvatarControls(self, task):
        """
        Check on the arrow keys and update the avatar.
        """
        # get the button states:
        forward = inputState.isSet("forward")
        reverse = inputState.isSet("reverse")
        turnLeft = inputState.isSet("turnLeft")
        turnRight = inputState.isSet("turnRight")
        slideLeft = inputState.isSet("slideLeft")
        slideRight = inputState.isSet("slideRight")
        levitateUp = inputState.isSet("levitateUp")
        levitateDown = inputState.isSet("levitateDown")
        run = inputState.isSet("run") and self.runMultiplier.getValue() or 1.0

        # Check for Auto-Run
        if base.localAvatar.getAutoRun():
            forward = 1
            reverse = 0

        # Determine what the speeds are based on the buttons:
        self.speed=(
                (forward and self.avatarControlForwardSpeed or
                reverse and -self.avatarControlReverseSpeed))
        self.liftSpeed=(
                (levitateUp and self.avatarControlForwardSpeed or
                levitateDown and -self.avatarControlReverseSpeed))
        self.slideSpeed=(
                (slideLeft and -self.avatarControlForwardSpeed) or
                (slideRight and self.avatarControlForwardSpeed))
        self.rotationSpeed=(
                (turnLeft and self.avatarControlRotateSpeed) or
                (turnRight and -self.avatarControlRotateSpeed))

        if self.wantDebugIndicator:
            self.displayDebugInfo()

        # Check to see if we're moving at all:
        if self.speed or self.liftSpeed or self.slideSpeed or self.rotationSpeed:
            # How far did we move based on the amount of time elapsed?
            dt=ClockObject.getGlobalClock().getDt()
            distance = dt * self.speed * run
            lift = dt * self.liftSpeed * run
            slideDistance = dt * self.slideSpeed * run
            rotation = dt * self.rotationSpeed

            # Take a step in the direction of our previous heading.
            self.vel=Vec3(Vec3.forward() * distance +
                          Vec3.up() * lift +
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
        return Task.cont

    def enableAvatarControls(self):
        """
        Activate the arrow keys, etc.
        """
        assert self.debugPrint("enableAvatarControls")

        if self.task:
            # remove any old
            self.task.remove(self.task)
        # spawn the new task
        self.task = taskMgr.add(
            self.handleAvatarControls, "AvatarControls-dev-%s"%(id(self),))

    def disableAvatarControls(self):
        """
        Ignore the arrow keys, etc.
        """
        assert self.debugPrint("disableAvatarControls")
        if self.task:
            self.task.remove()
            self.task = None

    def flushEventHandlers(self):
        pass

    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(id(self))+' '+message)
