"""
GravityWalker.py is for avatars.

A walker control such as this one provides:
    - creation of the collision nodes
    - handling the keyboard and mouse input for avatar movement
    - moving the avatar

it does not:
    - play sounds
    - play animations

although it does send messeges that allow a listener to play sounds or
animations based on walker events.
"""

from direct.showbase.ShowBaseGlobal import *

from direct.directnotify import DirectNotifyGlobal
from direct.showbase import DirectObject
#from pandac import PhysicsManager
import math


class GravityWalker(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("GravityWalker")
    wantDebugIndicator = base.config.GetBool('want-avatar-physics-indicator', 0)
    wantFloorSphere = base.config.GetBool('want-floor-sphere', 0)

    # special methods
    def __init__(self, gravity = -32.1740, standableGround=0.707,
            hardLandingForce=16.0):
        assert self.notify.debugStateCall(self)
        DirectObject.DirectObject.__init__(self)
        self.__gravity=gravity
        self.__standableGround=standableGround
        self.__hardLandingForce=hardLandingForce

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

    def delete(self):
        assert self.notify.debugStateCall(self)
        if self.doLaterTask is not None:
            self.doLaterTask.remove()
            del self.doLaterTask
        #DirectObject.DirectObject.delete(self)

    """
    def spawnTest(self):
        assert self.notify.debugStateCall(self)
        if not self.wantDebugIndicator:
            return
        from pandac.PandaModules import *
        from direct.interval.IntervalGlobal import *
        from toontown.coghq import MovingPlatform

        if hasattr(self, "platform"):
            # Remove the prior instantiation:
            self.moveIval.pause()
            del self.moveIval
            self.platform.destroy()
            del self.platform
            self.platform2.destroy()
            del self.platform2

        model = loader.loadModelCopy('phase_9/models/cogHQ/platform1')
        fakeId = id(self)
        self.platform = MovingPlatform.MovingPlatform()
        self.platform.setupCopyModel(fakeId, model, 'platformcollision')
        self.platformRoot = render.attachNewNode("GravityWalker-spawnTest-%s"%fakeId)
        self.platformRoot.setPos(base.localAvatar, Vec3(0.0, 0.0, 1.0))
        self.platformRoot.setHpr(base.localAvatar, Vec3.zero())
        self.platform.reparentTo(self.platformRoot)

        self.platform2 = MovingPlatform.MovingPlatform()
        self.platform2.setupCopyModel(1+fakeId, model, 'platformcollision')
        self.platform2Root = render.attachNewNode("GravityWalker-spawnTest2-%s"%fakeId)
        self.platform2Root.setPos(base.localAvatar, Vec3(-16.0, 30.0, 1.0))
        self.platform2Root.setHpr(base.localAvatar, Vec3.zero())
        self.platform2.reparentTo(self.platform2Root)

        duration = 5
        self.moveIval = Parallel(
                Sequence(
                    WaitInterval(0.3),
                    LerpPosInterval(self.platform, duration,
                                    Vec3(0.0, 30.0, 0.0),
                                    name='platformOut%s' % fakeId,
                                    fluid = 1),
                    WaitInterval(0.3),
                    LerpPosInterval(self.platform, duration,
                                    Vec3(0.0, 0.0, 0.0),
                                    name='platformBack%s' % fakeId,
                                    fluid = 1),
                    WaitInterval(0.3),
                    LerpPosInterval(self.platform, duration,
                                    Vec3(0.0, 0.0, 30.0),
                                    name='platformUp%s' % fakeId,
                                    fluid = 1),
                    WaitInterval(0.3),
                    LerpPosInterval(self.platform, duration,
                                    Vec3(0.0, 0.0, 0.0),
                                    name='platformDown%s' % fakeId,
                                    fluid = 1),
                ),
                Sequence(
                    WaitInterval(0.3),
                    LerpPosInterval(self.platform2, duration,
                                    Vec3(0.0, -30.0, 0.0),
                                    name='platform2Out%s' % fakeId,
                                    fluid = 1),
                    WaitInterval(0.3),
                    LerpPosInterval(self.platform2, duration,
                                    Vec3(0.0, 30.0, 30.0),
                                    name='platform2Back%s' % fakeId,
                                    fluid = 1),
                    WaitInterval(0.3),
                    LerpPosInterval(self.platform2, duration,
                                    Vec3(0.0, -30.0, 0.0),
                                    name='platform2Up%s' % fakeId,
                                    fluid = 1),
                    WaitInterval(0.3),
                    LerpPosInterval(self.platform2, duration,
                                    Vec3(0.0, 0.0, 0.0),
                                    name='platformDown%s' % fakeId,
                                    fluid = 1),
                ),
            name='platformIval%s' % fakeId,
            )
        self.moveIval.loop()
    """

    def setWalkSpeed(self, forward, jump, reverse, rotate):
        assert self.notify.debugStateCall(self)
        self.avatarControlForwardSpeed=forward
        self.avatarControlJumpForce=jump
        self.avatarControlReverseSpeed=reverse
        self.avatarControlRotateSpeed=rotate

    def getSpeeds(self):
        #assert(self.debugPrint("getSpeeds()"))
        return (self.speed, self.rotationSpeed)

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
        cRay = CollisionRay(0.0, 0.0, CollisionHandlerRayStart, 0.0, 0.0, -1.0)
        cRayNode = CollisionNode('GW.cRayNode')
        cRayNode.addSolid(cRay)
        self.cRayNodePath = self.avatarNodePath.attachNewNode(cRayNode)
        cRayNode.setFromCollideMask(bitmask)
        cRayNode.setIntoCollideMask(BitMask32.allOff())

        # set up floor collision mechanism
        self.lifter = CollisionHandlerGravity()
        self.lifter.setGravity(32.174 * 2.0)
        self.lifter.addInPattern("enter%in")
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
        handler = CollisionHandlerPusher()
        #handler.setInPattern("pusher_enter%in")
        #handler.setOutPattern("pusher_exit%in")

        handler.addCollider(cSphereNodePath, self.avatarNodePath)
        self.pusher = handler
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
        handler = CollisionHandlerEvent()
        handler.addInPattern("enter%in")
        handler.addOutPattern("exit%in")

        self.event = handler
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
        handler = CollisionHandlerPusher()
        #handler.setInPattern("pusherFloor_enter%in")
        #handler.setOutPattern("pusherFloor_exit%in")

        handler.addCollider(cSphereNodePath, self.avatarNodePath)
        self.pusherFloor = handler
        self.cFloorSphereNodePath = cSphereNodePath

    def setWallBitMask(self, bitMask):
        self.wallBitmask = bitMask

    def setFloorBitMask(self, bitMask):
        self.floorBitmask = bitMask

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

        self.setupRay(self.floorBitmask, floorOffset, reach )
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
            if active:
                if 1:
                    # Please let skyler or drose know if this is causing a problem
                    # This is a bit of a hack fix:
                    self.avatarNodePath.setP(0.0)
                    self.avatarNodePath.setR(0.0)
                self.cTrav.addCollider(self.cWallSphereNodePath, self.pusher)
                if self.wantFloorSphere:
                    self.cTrav.addCollider(self.cFloorSphereNodePath, self.pusherFloor)
                self.cTrav.addCollider(self.cEventSphereNodePath, self.event)
                self.cTrav.addCollider(self.cRayNodePath, self.lifter)
            else:
                self.cTrav.removeCollider(self.cWallSphereNodePath)
                if self.wantFloorSphere:
                    self.cTrav.removeCollider(self.cFloorSphereNodePath)
                self.cTrav.removeCollider(self.cEventSphereNodePath)
                self.cTrav.removeCollider(self.cRayNodePath)

    def getCollisionsActive(self):
        assert(self.debugPrint("getCollisionsActive() returning=%s"%(
            self.collisionsActive,)))
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
        slide = 0 #hack -- was: inputState.isSet("slide")
        jump = inputState.isSet("jump")
        # Determine what the speeds are based on the buttons:
        self.speed=(forward and self.avatarControlForwardSpeed or
                    reverse and -self.avatarControlReverseSpeed)
        #if run and self.speed>0.0:
        #    self.speed*=2.0 #*#
        # Should fSlide be renamed slideButton?
        self.slideSpeed=slide and (
                (turnLeft and -self.avatarControlForwardSpeed) or
                (turnRight and self.avatarControlForwardSpeed))
        self.rotationSpeed=not slide and (
                (turnLeft and self.avatarControlRotateSpeed) or
                (turnRight and -self.avatarControlRotateSpeed))

        if __debug__:
            debugRunning = inputState.isSet("debugRunning")
            if debugRunning:
                self.speed*=4.0
                self.slideSpeed*=4.0
                self.rotationSpeed*=1.25

        if self.needToDeltaPos:
            self.setPriorParentVector()
            self.needToDeltaPos = 0
        if self.wantDebugIndicator:
            self.displayDebugInfo()
        if self.lifter.isOnGround():
            if self.isAirborne:
                self.isAirborne = 0
                assert(self.debugPrint("isAirborne 0 due to isOnGround() true"))
                impact = self.lifter.getImpactVelocity()
                if impact < -30.0:
                    messenger.send("jumpHardLand")
                    self.startJumpDelay(0.3)
                else:
                    messenger.send("jumpLand")
                    if impact < -5.0:
                        self.startJumpDelay(0.2)
                    # else, ignore the little potholes.
            assert(self.isAirborne == 0)
            self.priorParent = Vec3.zero()
            if jump and self.mayJump:
                # ...the jump button is down and we're close
                # enough to the ground to jump.
                self.lifter.addVelocity(self.avatarControlJumpForce)
                messenger.send("jumpStart")
                self.isAirborne = 1
                assert(self.debugPrint("isAirborne 1 due to jump"))
        else:
            if self.isAirborne == 0:
                assert(self.debugPrint("isAirborne 1 due to isOnGround() false"))
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
            self.vel=Vec3(Vec3.forward() * distance +
                          Vec3.right() * slideDistance)
            if self.vel != Vec3.zero() or self.priorParent != Vec3.zero():
                if 1:
                    # rotMat is the rotation matrix corresponding to
                    # our previous heading.
                    rotMat=Mat3.rotateMatNormaxis(self.avatarNodePath.getH(), Vec3.up())
                    step=rotMat.xform(self.vel) + (self.priorParent * dt)
                    self.avatarNodePath.setFluidPos(Point3(
                            self.avatarNodePath.getPos()+step))
                if 0:
                    # rotMat is the rotation matrix corresponding to
                    # our previous heading.
                    rotMat=Mat3.rotateMatNormaxis(self.avatarNodePath.getH(), self.lifter.getContactNormal())
                    step=rotMat.xform(self.vel) + (self.priorParent * dt)
                    self.avatarNodePath.setFluidPos(Point3(
                            self.avatarNodePath.getPos()+step))
                if 0:
                    # rotMat is the rotation matrix corresponding to
                    # our previous heading.
                    rotMat=Mat3.rotateMatNormaxis(self.avatarNodePath.getH(), Vec3.up())
                    forward = Vec3(rotMat.xform(Vec3.forward()))
                    up      = Vec3(rotMat.xform(self.lifter.getContactNormal()))
                    rotMat2=Mat3()
                    headsUp(rotMat2, forward, up)
                    #rotMat2=Mat3.rotateMatNormaxis(0.0, )
                    step=rotMat2.xform(self.vel) + (self.priorParent * dt)
                    if 1:
                        onScreenDebug.add("a getH()", self.avatarNodePath.getH())
                        onScreenDebug.add("a forward", forward.pPrintValues())
                        onScreenDebug.add("a up", up.pPrintValues())
                        onScreenDebug.add("a Vec3.forward()", Vec3.forward().pPrintValues())
                        onScreenDebug.add("a Vec3.up()", Vec3.up().pPrintValues())
                        onScreenDebug.add("a Vec3.right()", Vec3.right().pPrintValues())
                        onScreenDebug.add("a contactNormal()", self.lifter.getContactNormal().pPrintValues())
                        onScreenDebug.add("a rotMat", rotMat.pPrintValues())
                        onScreenDebug.add("a rotMat2", rotMat2.pPrintValues())
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
        velocity = self.__oldPosDelta*(1.0/self.__oldDt)
        self.priorParent = Vec3(velocity)
        if __debug__:
            if self.wantDebugIndicator:
                onScreenDebug.add("priorParent", self.priorParent.pPrintValues())

    def reset(self):
        assert self.notify.debugStateCall(self)
        self.lifter.setVelocity(0.0)
        self.priorParent=Vec3.zero()

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


    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(id(self))+' '+message)
