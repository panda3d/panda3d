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

from ShowBaseGlobal import *

import DirectNotifyGlobal
import DirectObject
import PhysicsManager
import math


class GravityWalker(DirectObject.DirectObject):

    notify = DirectNotifyGlobal.directNotify.newCategory("GravityWalker")
    wantDebugIndicator = base.config.GetBool('want-avatar-physics-indicator', 0)
    
    # special methods
    def __init__(self, gravity = -32.1740, standableGround=0.707,
            hardLandingForce=16.0):
        assert(self.debugPrint("GravityWalker(gravity=%s, standableGround=%s)"%(
                gravity, standableGround)))
        DirectObject.DirectObject.__init__(self)
        self.__gravity=gravity
        self.__standableGround=standableGround
        self.__hardLandingForce=hardLandingForce
        
        self.mayJump = 1
        self.jumpDelayTask = None

        self.controlsTask = None
        self.fixCliffTask = None
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
        if self.doLaterTask is not None:
            self.doLaterTask.remove()
            del self.doLaterTask
        #DirectObject.DirectObject.delete(self)
    
    def spawnTest(self):
        assert(self.debugPrint("\n\nspawnTest()\n"))
        if not self.wantDebugIndicator:
            return
        from PandaModules import *
        from IntervalGlobal import *
        import MovingPlatform
        
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
        self.platformRoot.setPos(toonbase.localToon, Vec3(0.0, 0.0, 1.0))
        self.platformRoot.setHpr(toonbase.localToon, Vec3.zero())
        self.platform.reparentTo(self.platformRoot)

        self.platform2 = MovingPlatform.MovingPlatform()
        self.platform2.setupCopyModel(1+fakeId, model, 'platformcollision')
        self.platform2Root = render.attachNewNode("GravityWalker-spawnTest2-%s"%fakeId)
        self.platform2Root.setPos(toonbase.localToon, Vec3(-16.0, 30.0, 1.0))
        self.platform2Root.setHpr(toonbase.localToon, Vec3.zero())
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

    def setWalkSpeed(self, forward, jump, reverse, rotate):
        assert(self.debugPrint("setWalkSpeed()"))
        self.avatarControlForwardSpeed=forward
        self.avatarControlJumpForce=jump
        self.avatarControlReverseSpeed=reverse
        self.avatarControlRotateSpeed=rotate

    def getSpeeds(self):
        #assert(self.debugPrint("getSpeeds()"))
        return (self.speed, self.rotationSpeed)

    def setupRay(self, floorBitmask, floorOffset):
        # This is a ray cast from your head down to detect floor polygons
        # A toon is about 4.0 feet high, so start it there
        cRay = CollisionRay(0.0, 0.0, 4.0, 0.0, 0.0, -1.0)
        cRayNode = CollisionNode('GW.cRayNode')
        cRayNode.addSolid(cRay)
        self.cRayNodePath = self.avatarNodePath.attachNewNode(cRayNode)
        cRayNode.setFromCollideMask(floorBitmask)
        cRayNode.setIntoCollideMask(BitMask32.allOff())

        # set up floor collision mechanism
        self.lifter = CollisionHandlerGravity()
        self.lifter.setGravity(32.174 * 2.0)
        self.lifter.setInPattern("enterRay-%in")
        self.lifter.setOutPattern("exitRay-%in")
        self.lifter.setOffset(floorOffset)

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
        # This is a sphere on the ground to detect barrier collisions
        self.avatarRadius = avatarRadius
        self.cSphere = CollisionSphere(0.0, 0.0, avatarRadius, avatarRadius)
        cSphereNode = CollisionNode('GW.cWallSphereNode')
        cSphereNode.addSolid(self.cSphere)
        cSphereNodePath = self.avatarNodePath.attachNewNode(cSphereNode)

        cSphereNode.setFromCollideMask(bitmask)
        cSphereNode.setIntoCollideMask(BitMask32.allOff())

        # set up collision mechanism
        handler = CollisionHandlerPusher()
        handler.setInPattern("pusher_enter%in")
        handler.setOutPattern("pusher_exit%in")

        handler.addCollider(cSphereNodePath, self.avatarNodePath)
        self.pusher = handler
        self.cWallSphereNodePath = cSphereNodePath

    def setupEventSphere(self, bitmask, avatarRadius):
        """
        Set up the collision sphere
        """
        # This is a sphere on the ground to detect barrier collisions
        self.avatarRadius = avatarRadius
        self.cSphere = CollisionSphere(0.0, 0.0, avatarRadius-0.1, avatarRadius*1.04)
        cSphereNode = CollisionNode('GW.cEventSphereNode')
        cSphereNode.addSolid(self.cSphere)
        cSphereNodePath = self.avatarNodePath.attachNewNode(cSphereNode)

        cSphereNode.setFromCollideMask(bitmask)
        cSphereNode.setIntoCollideMask(BitMask32.allOff())

        # set up collision mechanism
        handler = CollisionHandlerEvent()
        handler.setInPattern("enter%in")
        handler.setOutPattern("exit%in")

        self.event = handler
        self.cEventSphereNodePath = cSphereNodePath

    def setupFloorSphere(self, bitmask, avatarRadius):
        """
        Set up the collision sphere
        """
        # This is a sphere on the ground to detect barrier collisions
        self.avatarRadius = avatarRadius
        self.cSphere = CollisionSphere(0.0, 0.0, avatarRadius, 0.01)
        cSphereNode = CollisionNode('GW.cFloorSphereNode')
        cSphereNode.addSolid(self.cSphere)
        cSphereNodePath = self.avatarNodePath.attachNewNode(cSphereNode)

        cSphereNode.setFromCollideMask(bitmask)
        cSphereNode.setIntoCollideMask(BitMask32.allOff())

        # set up collision mechanism
        handler = CollisionHandlerPusher()
        handler.setInPattern("pusherFloor_enter%in")
        handler.setOutPattern("pusherFloor_exit%in")

        handler.addCollider(cSphereNodePath, self.avatarNodePath)
        self.pusherFloor = handler
        self.cFloorSphereNodePath = cSphereNodePath

    def initializeCollisions(self, collisionTraverser, avatarNodePath, 
            wallBitmask, floorBitmask, 
            avatarRadius = 1.4, floorOffset = 1.0):
        """
        Set up the avatar collisions
        """
        assert(self.debugPrint("initializeCollisions()"))
        
        assert not avatarNodePath.isEmpty()
        self.avatarNodePath = avatarNodePath
        
        self.cTrav = collisionTraverser
        self.floorOffset = 0.0

        self.setupRay(floorBitmask, self.floorOffset)
        self.setupWallSphere(wallBitmask, avatarRadius)
        self.setupEventSphere(wallBitmask|floorBitmask, avatarRadius)
        # self.setupFloorSphere(floorBitmask, avatarRadius)

        self.setCollisionsActive(1)

    def setAirborneHeightFunc(self, unused_parameter):
        self.getAirborneHeight = self.lifter.getAirborneHeight

    def getAirborneHeight(self):
        self.lifter.getAirborneHeight()

    def setAvatarPhysicsIndicator(self, indicator):
        """
        indicator is a NodePath
        """
        assert(self.debugPrint("setAvatarPhysicsIndicator()"))
        self.cWallSphereNodePath.show()

    def deleteCollisions(self):
        assert(self.debugPrint("deleteCollisions()"))
        del self.cTrav

        del self.cSphere
        self.cWallSphereNodePath.removeNode()
        del self.cWallSphereNodePath
        # self.cFloorSphereNodePath.removeNode()
        # del self.cFloorSphereNodePath

        del self.pusher
        del self.pusherFloor
        del self.event
        del self.lifter
        
        del self.getAirborneHeight

    def setCollisionsActive(self, active = 1):
        assert(self.debugPrint("collisionsActive(active=%s)"%(active,)))
        if self.collisionsActive != active:
            self.collisionsActive = active
            # Each time we change the collision geometry, make one 
            # more pass to ensure we aren't standing in a wall.
            self.oneTimeCollide()
            if active:
                self.cTrav.addCollider(self.cWallSphereNodePath, self.pusher)
                # self.cTrav.addCollider(self.cFloorSphereNodePath, self.pusherFloor)
                self.cTrav.addCollider(self.cEventSphereNodePath, self.event)
                self.cTrav.addCollider(self.cRayNodePath, self.lifter)
            else:
                self.cTrav.removeCollider(self.cWallSphereNodePath)
                # self.cTrav.removeCollider(self.cFloorSphereNodePath)
                self.cTrav.removeCollider(self.cEventSphereNodePath)
                self.cTrav.removeCollider(self.cRayNodePath)

    def getCollisionsActive(self):
        assert(self.debugPrint("getCollisionsActive() returning=%s"%(
            self.collisionsActive,)))
        return self.collisionsActive

    def FixCliff(self, task):
        """
        People are still making polygons that are marked
        as floor, but are nearly vertical.  This ray is
        a hack to help deal with the cliff.
        """
        if (self.collisionsActive
                and self.moving
                and self.lifter.isInOuterSpace()):
            temp = self.cRayNodePath.getZ()
            self.cRayNodePath.setZ(14.0)
            self.oneTimeCollide()
            self.cRayNodePath.setZ(temp)
        return Task.cont
    
    def placeOnFloor(self):
        """
        Make a reasonable effor to place the avatar on the ground.
        For example, this is useful when switching away from the 
        current walker.
        """
        self.oneTimeCollide()
        self.avatarNodePath.setZ(self.avatarNodePath.getZ()-self.lifter.getAirborneHeight())

    def oneTimeCollide(self):
        """
        Makes one quick collision pass for the avatar, for instance as
        a one-time straighten-things-up operation after collisions
        have been disabled.
        """
        assert(self.debugPrint("oneTimeCollide()"))
        tempCTrav = CollisionTraverser()
        tempCTrav.addCollider(self.cWallSphereNodePath, self.pusher)
        # tempCTrav.addCollider(self.cFloorSphereNodePath, self.event)
        tempCTrav.addCollider(self.cRayNodePath, self.lifter)
        tempCTrav.traverse(render)

    def setMayJump(self, task):
        """
        This function's use is internal to this class (maybe I'll add
        the __ someday).  Anyway, if you want to enable or disable
        jumping in a general way see the ControlManager (don't use this).
        """
        self.mayJump = 1
        return Task.done

    def startJumpDelay(self, delay):
        assert(self.debugPrint("startJumpDelay(delay=%s)"%(delay,)))
        if self.jumpDelayTask:
            self.jumpDelayTask.remove()
        self.mayJump = 0
        self.jumpDelayTask=taskMgr.doMethodLater(
            delay,
            self.setMayJump,
            "jumpDelay-%s"%id(self))

    def handleAvatarControls(self, task):
        """
        Check on the arrow keys and update the avatar.
        """
        # get the button states:
        forward = inputState.isSet("forward")
        reverse = inputState.isSet("reverse")
        turnLeft = inputState.isSet("turnLeft")
        turnRight = inputState.isSet("turnRight")
        slide = 0 #hack -- was: inputState.isSet("slide")
        jump = inputState.isSet("jump")
        # Determine what the speeds are based on the buttons:
        self.speed=(forward and self.avatarControlForwardSpeed or 
                    reverse and -self.avatarControlReverseSpeed)
        # Should fSlide be renamed slideButton?
        self.slideSpeed=slide and (
                (turnLeft and -self.avatarControlForwardSpeed) or 
                (turnRight and self.avatarControlForwardSpeed))
        self.rotationSpeed=not slide and (
                (turnLeft and self.avatarControlRotateSpeed) or
                (turnRight and -self.avatarControlRotateSpeed))

        if self.needToDeltaPos:
            self.setPriorParentVector()
            self.needToDeltaPos = 0
        if self.wantDebugIndicator:
            onScreenDebug.add("airborneHeight", self.lifter.getAirborneHeight()) #*#
            onScreenDebug.add("falling", self.falling) #*#
            onScreenDebug.add("isOnGround", self.lifter.isOnGround()) #*#

            onScreenDebug.add("gravity", self.lifter.getGravity()) #*#
            onScreenDebug.add("jumpForce", self.avatarControlJumpForce) #*#
            onScreenDebug.add("mayJump", self.mayJump) #*#
            onScreenDebug.add("impact", self.lifter.getImpactVelocity()) #*#

            onScreenDebug.add("velocity", self.lifter.getVelocity()) #*#
            onScreenDebug.add("isAirborne", self.isAirborne) #*#
            onScreenDebug.add("jump", jump) #*#

            onScreenDebug.add("inOuterSpace", self.lifter.isInOuterSpace()) #*#
        if self.lifter.isOnGround():
            if self.isAirborne:
                self.isAirborne = 0
                self.priorParent = Vec3(0)
                impact = self.lifter.getImpactVelocity()
                if impact < -30.0:
                    messenger.send("jumpHardLand")
                    self.startJumpDelay(0.3)
                else:
                    messenger.send("jumpLand")
                    if impact < -5.0:
                        self.startJumpDelay(0.2)
                    # else, ignore the little potholes.
            if jump and self.mayJump:
                # ...the jump button is down and we're close
                # enough to the ground to jump.
                self.lifter.addVelocity(self.avatarControlJumpForce)
                messenger.send("jumpStart")
                self.isAirborne = 1
        else:
            self.isAirborne = 1
        #    if self.lifter.getAirborneHeight() > 10000.0:
        #        assert(0)

        self.__oldPosDelta = self.avatarNodePath.getPosDelta(render)
        # How far did we move based on the amount of time elapsed?
        self.__oldDt = ClockObject.getGlobalClock().getDt()
        dt=min(self.__oldDt, 0.1)

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
                # rotMat is the rotation matrix corresponding to
                # our previous heading.
                rotMat=Mat3.rotateMatNormaxis(self.avatarNodePath.getH(), Vec3.up())
                step=rotMat.xform(self.vel) + (self.priorParent * dt)
                self.avatarNodePath.setFluidPos(Point3(
                        self.avatarNodePath.getPos()+step))
            self.avatarNodePath.setH(self.avatarNodePath.getH()+rotation)
        else:
            self.vel.set(0.0, 0.0, 0.0)
        if self.moving or jump:
            messenger.send("avatarMoving")
        return Task.cont
    
    def doDeltaPos(self):
        assert(self.debugPrint("doDeltaPos()"))
        self.needToDeltaPos = 1
    
    def setPriorParentVector(self):
        assert(self.debugPrint("setPriorParentVector()"))
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
        assert(self.debugPrint("reset()"))
        self.lifter.setVelocity(0.0)
        self.priorParent=Vec3(0.0)

    def enableAvatarControls(self):
        """
        Activate the arrow keys, etc.
        """
        assert(self.debugPrint("enableAvatarControls()"))
        print id(self), "GW.enableAvatarControls()"
        assert self.collisionsActive

        if __debug__:
            self.accept("control-f3", self.spawnTest) #*#

        # remove any old
        if self.controlsTask:
            self.controlsTask.remove()
        # spawn the new task
        taskName = "AvatarControls%s"%(id(self),)
        self.controlsTask = taskMgr.add(self.handleAvatarControls, taskName, 25)

        # remove any old
        if self.fixCliffTask:
            self.fixCliffTask.remove()
        # spawn the new task
        taskName = "AvatarControls-FixCliff%s"%(id(self),)
        self.fixCliffTask = taskMgr.add(self.FixCliff, taskName, 31)

        if self.physVelocityIndicator:
            if self.indicatorTask:
                self.indicatorTask.remove()
            self.indicatorTask = taskMgr.add(
                self.avatarPhysicsIndicator,
                "AvatarControlsIndicator%s"%(id(self),), 35)

    def disableAvatarControls(self):
        """
        Ignore the arrow keys, etc.
        """
        assert(self.debugPrint("disableAvatarControls()"))
        print id(self), "GW.disableAvatarControls()"
        if self.controlsTask:
            self.controlsTask.remove()
            self.controlsTask = None
        if self.fixCliffTask:
            self.fixCliffTask.remove()
            self.fixCliffTask = None
        if self.indicatorTask:
            self.indicatorTask.remove()
            self.indicatorTask = None

        if __debug__:
            self.ignore("control-f3") #*#

    
    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(id(self))+' '+message)
