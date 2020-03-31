"""
GravityWalker.py is for avatars.

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
from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase import DirectObject
from direct.controls.ControlManager import CollisionHandlerRayStart
from direct.showbase.InputStateGlobal import inputState
from direct.task.Task import Task
from panda3d.core import *
from direct.extensions_native import VBase3_extensions
from direct.extensions_native import VBase4_extensions
import math


class GravityWalker(DirectObject.DirectObject):
    notify = directNotify.newCategory("GravityWalker")
    wantDebugIndicator = ConfigVariableBool('want-avatar-physics-indicator', False)
    wantFloorSphere = ConfigVariableBool('want-floor-sphere', False)
    earlyEventSphere = ConfigVariableBool('early-event-sphere', False)

    DiagonalFactor = math.sqrt(2.) / 2.

    # special methods
    def __init__(self, gravity = 64.348, standableGround=0.707,
            hardLandingForce=16.0, legacyLifter=False):
        assert self.notify.debugStateCall(self)
        DirectObject.DirectObject.__init__(self)
        self.__gravity=gravity
        self.__standableGround=standableGround
        self.__hardLandingForce=hardLandingForce
        self._legacyLifter = legacyLifter

        self.mayJump = 1
        self.jumpDelayTask = None

        self.controlsTask = None
        self.indicatorTask = None

        self.falling = 0
        self.needToDeltaPos = 0
        self.physVelocityIndicator=None
        self.avatarControlForwardSpeed=0
        self.avatarControlJumpForce=0
        self.avatarControlReverseSpeed=0
        self.avatarControlRotateSpeed=0
        self.getAirborneHeight=None

        self.priorParent=Vec3(0)
        self.__oldPosDelta=Vec3(0)
        self.__oldDt=0

        self.moving=0
        self.speed=0.0
        self.rotationSpeed=0.0
        self.slideSpeed=0.0
        self.vel=Vec3(0.0)
        self.collisionsActive = 0

        self.isAirborne = 0
        self.highMark = 0

    def setWalkSpeed(self, forward, jump, reverse, rotate):
        assert self.notify.debugStateCall(self)
        self.avatarControlForwardSpeed=forward
        self.avatarControlJumpForce=jump
        self.avatarControlReverseSpeed=reverse
        self.avatarControlRotateSpeed=rotate

    def getSpeeds(self):
        #assert self.debugPrint("getSpeeds()")
        return (self.speed, self.rotationSpeed, self.slideSpeed)

    def getIsAirborne(self):
        return self.isAirborne

    def setAvatar(self, avatar):
        self.avatar = avatar
        if avatar is not None:
            pass # setup the avatar

    def setupRay(self, bitmask, floorOffset, reach):
        assert self.notify.debugStateCall(self)
        # This is a ray cast from your head down to detect floor polygons.
        # This ray start is arbitrarily high in the air.  Feel free to use
        # a higher or lower value depending on whether you want an avatar
        # that is outside of the world to step up to the floor when they
        # get under valid floor:
        self.cRay = CollisionRay(0.0, 0.0, CollisionHandlerRayStart, 0.0, 0.0, -1.0)
        cRayNode = CollisionNode('GW.cRayNode')
        cRayNode.addSolid(self.cRay)
        self.cRayNodePath = self.avatarNodePath.attachNewNode(cRayNode)
        cRayNode.setFromCollideMask(bitmask)
        cRayNode.setIntoCollideMask(BitMask32.allOff())

        # set up floor collision mechanism
        self.lifter = CollisionHandlerGravity()
        #self.lifter = CollisionHandlerHighestEvent()
        self.lifter.setLegacyMode(self._legacyLifter)
        self.lifter.setGravity(self.__gravity)
        self.lifter.addInPattern("enter%in")
        self.lifter.addAgainPattern("again%in")
        self.lifter.addOutPattern("exit%in")
        self.lifter.setOffset(floorOffset)
        self.lifter.setReach(reach)

        # Limit our rate-of-fall with the lifter.
        # If this is too low, we actually "fall" off steep stairs
        # and float above them as we go down. I increased this
        # from 8.0 to 16.0 to prevent this
        #self.lifter.setMaxVelocity(16.0)

        self.lifter.addCollider(self.cRayNodePath, self.avatarNodePath)

    def setupWallSphere(self, bitmask, avatarRadius):
        """
        Set up the collision sphere
        """
        assert self.notify.debugStateCall(self)
        # This is a sphere on the ground to detect collisions with
        # walls, but not the floor.
        self.avatarRadius = avatarRadius
        cSphere = CollisionSphere(0.0, 0.0, avatarRadius, avatarRadius)
        cSphereNode = CollisionNode('GW.cWallSphereNode')
        cSphereNode.addSolid(cSphere)
        cSphereNodePath = self.avatarNodePath.attachNewNode(cSphereNode)

        cSphereNode.setFromCollideMask(bitmask)
        cSphereNode.setIntoCollideMask(BitMask32.allOff())

        # set up collision mechanism
        if config.GetBool('want-fluid-pusher', 0):
            self.pusher = CollisionHandlerFluidPusher()
        else:
            self.pusher = CollisionHandlerPusher()
        self.pusher.addCollider(cSphereNodePath, self.avatarNodePath)
        self.cWallSphereNodePath = cSphereNodePath

    def setupEventSphere(self, bitmask, avatarRadius):
        """
        Set up the collision sphere
        """
        assert self.notify.debugStateCall(self)
        # This is a sphere a little larger than the wall sphere to
        # trigger events.
        self.avatarRadius = avatarRadius
        cSphere = CollisionSphere(0.0, 0.0, avatarRadius-0.1, avatarRadius*1.04)
        # Mark it intangible just to emphasize its non-physical purpose.
        cSphere.setTangible(0)
        cSphereNode = CollisionNode('GW.cEventSphereNode')
        cSphereNode.addSolid(cSphere)
        cSphereNodePath = self.avatarNodePath.attachNewNode(cSphereNode)

        cSphereNode.setFromCollideMask(bitmask)
        cSphereNode.setIntoCollideMask(BitMask32.allOff())

        # set up collision mechanism
        self.event = CollisionHandlerEvent()
        self.event.addInPattern("enter%in")
        self.event.addOutPattern("exit%in")
        self.cEventSphereNodePath = cSphereNodePath

    def setupFloorSphere(self, bitmask, avatarRadius):
        """
        Set up the collision sphere
        """
        assert self.notify.debugStateCall(self)
        # This is a tiny sphere concentric with the wallSphere to keep
        # us from slipping through floors.
        self.avatarRadius = avatarRadius
        cSphere = CollisionSphere(0.0, 0.0, avatarRadius, 0.01)
        cSphereNode = CollisionNode('GW.cFloorSphereNode')
        cSphereNode.addSolid(cSphere)
        cSphereNodePath = self.avatarNodePath.attachNewNode(cSphereNode)

        cSphereNode.setFromCollideMask(bitmask)
        cSphereNode.setIntoCollideMask(BitMask32.allOff())

        # set up collision mechanism
        self.pusherFloorhandler = CollisionHandlerPusher()
        self.pusherFloor.addCollider(cSphereNodePath, self.avatarNodePath)
        self.cFloorSphereNodePath = cSphereNodePath

    def setWallBitMask(self, bitMask):
        self.wallBitmask = bitMask

    def setFloorBitMask(self, bitMask):
        self.floorBitmask = bitMask

    def swapFloorBitMask(self, oldMask, newMask):
        self.floorBitmask = self.floorBitmask &~ oldMask
        self.floorBitmask |= newMask

        if self.cRayNodePath and not self.cRayNodePath.isEmpty():
            self.cRayNodePath.node().setFromCollideMask(self.floorBitmask)

    def setGravity(self, gravity):
        self.__gravity = gravity
        self.lifter.setGravity(self.__gravity)

    def getGravity(self, gravity):
        return self.__gravity

    def initializeCollisions(self, collisionTraverser, avatarNodePath,
            avatarRadius = 1.4, floorOffset = 1.0, reach = 1.0):
        """
        floorOffset is how high the avatar can reach.  I.e. if the avatar
            walks under a ledge that is <= floorOffset above the ground (a
            double floor situation), the avatar will step up on to the
            ledge (instantly).

        Set up the avatar collisions
        """
        assert self.notify.debugStateCall(self)

        assert not avatarNodePath.isEmpty()
        self.avatarNodePath = avatarNodePath

        self.cTrav = collisionTraverser

        self.setupRay(self.floorBitmask, floorOffset, reach)
        self.setupWallSphere(self.wallBitmask, avatarRadius)
        self.setupEventSphere(self.wallBitmask, avatarRadius)
        if self.wantFloorSphere:
            self.setupFloorSphere(self.floorBitmask, avatarRadius)

        self.setCollisionsActive(1)

    def setTag(self, key, value):
        self.cEventSphereNodePath.setTag(key, value)

    def setAirborneHeightFunc(self, unused_parameter):
        assert self.notify.debugStateCall(self)
        self.getAirborneHeight = self.lifter.getAirborneHeight

    def getAirborneHeight(self):
        assert self.notify.debugStateCall(self)
        self.lifter.getAirborneHeight()

    def setAvatarPhysicsIndicator(self, indicator):
        """
        indicator is a NodePath
        """
        assert self.notify.debugStateCall(self)
        self.cWallSphereNodePath.show()

    def deleteCollisions(self):
        assert self.notify.debugStateCall(self)
        del self.cTrav

        self.cWallSphereNodePath.removeNode()
        del self.cWallSphereNodePath
        if self.wantFloorSphere:
            self.cFloorSphereNodePath.removeNode()
            del self.cFloorSphereNodePath

        del self.pusher
        # del self.pusherFloor
        del self.event
        del self.lifter

        del self.getAirborneHeight

    def setCollisionsActive(self, active = 1):
        assert self.notify.debugStateCall(self)
        if self.collisionsActive != active:
            self.collisionsActive = active
            # Each time we change the collision geometry, make one
            # more pass to ensure we aren't standing in a wall.
            self.oneTimeCollide()
            # make sure we have a shadow traverser
            base.initShadowTrav()
            if active:
                if 1:
                    # Please let skyler or drose know if this is causing a problem
                    # This is a bit of a hack fix:
                    self.avatarNodePath.setP(0.0)
                    self.avatarNodePath.setR(0.0)
                self.cTrav.addCollider(self.cWallSphereNodePath, self.pusher)
                if self.wantFloorSphere:
                    self.cTrav.addCollider(self.cFloorSphereNodePath, self.pusherFloor)
                # Add the lifter to the shadow traverser, which runs after
                # our traverser. This prevents the "fall through wall and
                # off ledge" bug. The problem was that we couldn't control
                # which collided first, the wall pusher or the lifter, if
                # they're in the same collision traverser. If the lifter
                # collided first, we'd start falling before getting pushed
                # back behind the wall.
                base.shadowTrav.addCollider(self.cRayNodePath, self.lifter)

                if self.earlyEventSphere:
                    # If we want to trigger the events at the same
                    # time as we intersect walls (e.g. Toontown, for
                    # backward compatibility issues), add the event
                    # sphere to the main traverser.  This allows us to
                    # hit door triggers that are just slightly behind
                    # the door itself.
                    self.cTrav.addCollider(self.cEventSphereNodePath, self.event)
                else:
                    # Normally, we'd rather trigger the events after
                    # the pusher has had a chance to fix up our
                    # position, so we never trigger things that are
                    # behind other polygons.
                    base.shadowTrav.addCollider(self.cEventSphereNodePath, self.event)

            else:
                if hasattr(self, 'cTrav'):
                    self.cTrav.removeCollider(self.cWallSphereNodePath)
                    if self.wantFloorSphere:
                        self.cTrav.removeCollider(self.cFloorSphereNodePath)
                    self.cTrav.removeCollider(self.cEventSphereNodePath)
                base.shadowTrav.removeCollider(self.cEventSphereNodePath)
                base.shadowTrav.removeCollider(self.cRayNodePath)

    def getCollisionsActive(self):
        assert self.debugPrint("getCollisionsActive() returning=%s"%(
            self.collisionsActive,))
        return self.collisionsActive

    def placeOnFloor(self):
        """
        Make a reasonable effor to place the avatar on the ground.
        For example, this is useful when switching away from the
        current walker.
        """
        assert self.notify.debugStateCall(self)
        self.oneTimeCollide()
        self.avatarNodePath.setZ(self.avatarNodePath.getZ()-self.lifter.getAirborneHeight())

    def oneTimeCollide(self):
        """
        Makes one quick collision pass for the avatar, for instance as
        a one-time straighten-things-up operation after collisions
        have been disabled.
        """
        assert self.notify.debugStateCall(self)
        if not hasattr(self, 'cWallSphereNodePath'):
            return
        self.isAirborne = 0
        self.mayJump = 1
        tempCTrav = CollisionTraverser("oneTimeCollide")
        tempCTrav.addCollider(self.cWallSphereNodePath, self.pusher)
        if self.wantFloorSphere:
            tempCTrav.addCollider(self.cFloorSphereNodePath, self.event)
        tempCTrav.addCollider(self.cRayNodePath, self.lifter)
        tempCTrav.traverse(render)

    def setMayJump(self, task):
        """
        This function's use is internal to this class (maybe I'll add
        the __ someday).  Anyway, if you want to enable or disable
        jumping in a general way see the ControlManager (don't use this).
        """
        assert self.notify.debugStateCall(self)
        self.mayJump = 1
        return Task.done

    def startJumpDelay(self, delay):
        assert self.notify.debugStateCall(self)
        if self.jumpDelayTask:
            self.jumpDelayTask.remove()
        self.mayJump = 0
        self.jumpDelayTask=taskMgr.doMethodLater(
            delay,
            self.setMayJump,
            "jumpDelay-%s"%id(self))

    def addBlastForce(self, vector):
        self.lifter.addVelocity(vector.length())

    def displayDebugInfo(self):
        """
        For debug use.
        """
        onScreenDebug.add("w controls", "GravityWalker")

        onScreenDebug.add("w airborneHeight", self.lifter.getAirborneHeight())
        onScreenDebug.add("w falling", self.falling)
        onScreenDebug.add("w isOnGround", self.lifter.isOnGround())
        #onScreenDebug.add("w gravity", self.lifter.getGravity())
        #onScreenDebug.add("w jumpForce", self.avatarControlJumpForce)
        onScreenDebug.add("w contact normal", self.lifter.getContactNormal().pPrintValues())
        onScreenDebug.add("w mayJump", self.mayJump)
        onScreenDebug.add("w impact", self.lifter.getImpactVelocity())
        onScreenDebug.add("w velocity", self.lifter.getVelocity())
        onScreenDebug.add("w isAirborne", self.isAirborne)
        onScreenDebug.add("w hasContact", self.lifter.hasContact())

    def handleAvatarControls(self, task):
        """
        Check on the arrow keys and update the avatar.
        """
        # get the button states:
        run = inputState.isSet("run")
        forward = inputState.isSet("forward")
        reverse = inputState.isSet("reverse")
        turnLeft = inputState.isSet("turnLeft")
        turnRight = inputState.isSet("turnRight")
        slideLeft = inputState.isSet("slideLeft")
        slideRight = inputState.isSet("slideRight")
        jump = inputState.isSet("jump")

        # Check for Auto-Run
        if 'localAvatar' in __builtins__:
            if base.localAvatar and base.localAvatar.getAutoRun():
                forward = 1
                reverse = 0

        # Determine what the speeds are based on the buttons:
        self.speed=(forward and self.avatarControlForwardSpeed or
                    reverse and -self.avatarControlReverseSpeed)
        # Slide speed is a scaled down version of forward speed
        # Note: you can multiply a factor in here if you want slide to
        # be slower than normal walk/run. Let's try full speed.
        #self.slideSpeed=(slideLeft and -self.avatarControlForwardSpeed*0.75 or
        #                 slideRight and self.avatarControlForwardSpeed*0.75)
        self.slideSpeed=(reverse and slideLeft and -self.avatarControlReverseSpeed*0.75 or
                         reverse and slideRight and self.avatarControlReverseSpeed*0.75 or
                         slideLeft and -self.avatarControlForwardSpeed*0.75 or
                         slideRight and self.avatarControlForwardSpeed*0.75)
        self.rotationSpeed=not (slideLeft or slideRight) and (
                (turnLeft and self.avatarControlRotateSpeed) or
                (turnRight and -self.avatarControlRotateSpeed))

        if self.speed and self.slideSpeed:
            self.speed *= GravityWalker.DiagonalFactor
            self.slideSpeed *= GravityWalker.DiagonalFactor

        debugRunning = inputState.isSet("debugRunning")
        if(debugRunning):
            self.speed*=base.debugRunningMultiplier
            self.slideSpeed*=base.debugRunningMultiplier
            self.rotationSpeed*=1.25

        if self.needToDeltaPos:
            self.setPriorParentVector()
            self.needToDeltaPos = 0
        if self.wantDebugIndicator:
            self.displayDebugInfo()
        if self.lifter.isOnGround():
            if self.isAirborne:
                self.isAirborne = 0
                assert self.debugPrint("isAirborne 0 due to isOnGround() true")
                impact = self.lifter.getImpactVelocity()
                if impact < -30.0:
                    messenger.send("jumpHardLand")
                    self.startJumpDelay(0.3)
                else:
                    messenger.send("jumpLand")
                    if impact < -5.0:
                        self.startJumpDelay(0.2)
                    # else, ignore the little potholes.
            assert self.isAirborne == 0
            self.priorParent = Vec3.zero()
            if jump and self.mayJump:
                # The jump button is down and we're close
                # enough to the ground to jump.
                self.lifter.addVelocity(self.avatarControlJumpForce)
                messenger.send("jumpStart")
                self.isAirborne = 1
                assert self.debugPrint("isAirborne 1 due to jump")
        else:
            if self.isAirborne == 0:
                assert self.debugPrint("isAirborne 1 due to isOnGround() false")
            self.isAirborne = 1

        self.__oldPosDelta = self.avatarNodePath.getPosDelta(render)
        # How far did we move based on the amount of time elapsed?
        self.__oldDt = ClockObject.getGlobalClock().getDt()
        dt=self.__oldDt

        # Check to see if we're moving at all:
        self.moving = self.speed or self.slideSpeed or self.rotationSpeed or (self.priorParent!=Vec3.zero())
        if self.moving:
            distance = dt * self.speed
            slideDistance = dt * self.slideSpeed
            rotation = dt * self.rotationSpeed

            # Take a step in the direction of our previous heading.
            if distance or slideDistance or self.priorParent != Vec3.zero():
                # rotMat is the rotation matrix corresponding to
                # our previous heading.
                rotMat=Mat3.rotateMatNormaxis(self.avatarNodePath.getH(), Vec3.up())
                if self.isAirborne:
                    forward = Vec3.forward()
                else:
                    contact = self.lifter.getContactNormal()
                    forward = contact.cross(Vec3.right())
                    # Consider commenting out this normalize.  If you do so
                    # then going up and down slops is a touch slower and
                    # steeper terrain can cut the movement in half.  Without
                    # the normalize the movement is slowed by the cosine of
                    # the slope (i.e. it is multiplied by the sign as a
                    # side effect of the cross product above).
                    forward.normalize()
                self.vel=Vec3(forward * distance)
                if slideDistance:
                    if self.isAirborne:
                        right = Vec3.right()
                    else:
                        right = forward.cross(contact)
                        # See note above for forward.normalize()
                        right.normalize()
                    self.vel=Vec3(self.vel + (right * slideDistance))
                self.vel=Vec3(rotMat.xform(self.vel))
                step=self.vel + (self.priorParent * dt)
                self.avatarNodePath.setFluidPos(Point3(
                        self.avatarNodePath.getPos()+step))
            self.avatarNodePath.setH(self.avatarNodePath.getH()+rotation)
        else:
            self.vel.set(0.0, 0.0, 0.0)
        if self.moving or jump:
            messenger.send("avatarMoving")
        return Task.cont

    def doDeltaPos(self):
        assert self.notify.debugStateCall(self)
        self.needToDeltaPos = 1

    def setPriorParentVector(self):
        assert self.notify.debugStateCall(self)
        if __debug__:
            onScreenDebug.add("__oldDt", "% 10.4f"%self.__oldDt)
            onScreenDebug.add("self.__oldPosDelta",
                              self.__oldPosDelta.pPrintValues())
        # avoid divide by zero crash - grw
        if self.__oldDt == 0:
            velocity = 0
        else:
            velocity = self.__oldPosDelta*(1.0/self.__oldDt)
        self.priorParent = Vec3(velocity)
        if __debug__:
            if self.wantDebugIndicator:
                onScreenDebug.add("priorParent", self.priorParent.pPrintValues())

    def reset(self):
        assert self.notify.debugStateCall(self)
        self.lifter.setVelocity(0.0)
        self.priorParent=Vec3.zero()

    def getVelocity(self):
        return self.vel

    def enableAvatarControls(self):
        """
        Activate the arrow keys, etc.
        """
        assert self.notify.debugStateCall(self)
        assert self.collisionsActive

        #*#if __debug__:
        #*#    self.accept("control-f3", self.spawnTest) #*#

        # remove any old
        if self.controlsTask:
            self.controlsTask.remove()
        # spawn the new task
        taskName = "AvatarControls-%s"%(id(self),)
        self.controlsTask = taskMgr.add(self.handleAvatarControls, taskName, 25)

        self.isAirborne = 0
        self.mayJump = 1

        if self.physVelocityIndicator:
            if self.indicatorTask:
                self.indicatorTask.remove()
            self.indicatorTask = taskMgr.add(
                self.avatarPhysicsIndicator,
                "AvatarControlsIndicator-%s"%(id(self),), 35)

    def disableAvatarControls(self):
        """
        Ignore the arrow keys, etc.
        """
        assert self.notify.debugStateCall(self)
        if self.controlsTask:
            self.controlsTask.remove()
            self.controlsTask = None
        if self.indicatorTask:
            self.indicatorTask.remove()
            self.indicatorTask = None
        if self.jumpDelayTask:
            self.jumpDelayTask.remove()
            self.jumpDelayTask = None

        if __debug__:
            self.ignore("control-f3") #*#


    def flushEventHandlers(self):
        if hasattr(self, 'cTrav'):
            self.pusher.flush()
            if self.wantFloorSphere:
                self.floorPusher.flush()
            self.event.flush()
        self.lifter.flush() # not currently defined or needed

    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(id(self))+' '+message)

    # There are sometimes issues if the collision ray height is
    # so tall that it collides with multiple levels of floors.
    def setCollisionRayHeight(self, height):
        self.cRay.setOrigin(0.0, 0.0, height)
