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
    wantAvatarPhysicsIndicator = base.config.GetBool('want-avatar-physics-indicator', 0)
    
    useLifter = 1
    useHeightRay = 0
    
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
        self.falling = 0
        self.needToDeltaPos = 0
        self.physVelocityIndicator=None
        self.avatarControlForwardSpeed=0
        self.avatarControlJumpForce=0
        self.avatarControlReverseSpeed=0
        self.avatarControlRotateSpeed=0
        self.getAirborneHeight=None
        self.__oldPosDelta=Vec3(0)
        self.__oldDt=0
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
        if not self.wantAvatarPhysicsIndicator:
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
        
        model = loader.loadModelCopy('phase_9/models/cogHQ/platform1')
        fakeId = id(self)
        self.platform = MovingPlatform.MovingPlatform()
        self.platform.setupCopyModel(fakeId, model, 'platformcollision')
        self.platformRoot = render.attachNewNode("GravityWalker-spawnTest-%s"%fakeId)
        self.platformRoot.setPos(toonbase.localToon, Vec3(0.0, 3.0, 1.0))
        self.platformRoot.setHpr(toonbase.localToon, Vec3.zero())
        self.platform.reparentTo(self.platformRoot)

        startPos = Vec3(0.0, -15.0, 0.0)
        endPos = Vec3(0.0, 15.0, 0.0)
        distance = Vec3(startPos-endPos).length()
        duration = distance/4
        self.moveIval = Sequence(
            WaitInterval(0.3),
            LerpPosInterval(self.platform, duration,
                            endPos, startPos=startPos,
                            name='platformOut%s' % fakeId,
                            fluid = 1),
            WaitInterval(0.3),
            LerpPosInterval(self.platform, duration,
                            startPos, startPos=endPos,
                            name='platformBack%s' % fakeId,
                            fluid = 1),
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

    def setupSphere(self, bitmask, avatarRadius):
        """
        Set up the collision sphere
        """
        # This is a sphere on the ground to detect barrier collisions
        self.avatarRadius = avatarRadius
        centerHeight = avatarRadius
        if self.useHeightRay:
            centerHeight *= 2.0
        self.cSphere = CollisionSphere(0.0, 0.0, centerHeight-0.01, avatarRadius)
        cSphereNode = CollisionNode('GW.cSphereNode')
        cSphereNode.addSolid(self.cSphere)
        self.cSphereNodePath = self.avatarNodePath.attachNewNode(cSphereNode)
        self.cSphereBitMask = bitmask

        cSphereNode.setFromCollideMask(bitmask)
        cSphereNode.setIntoCollideMask(BitMask32.allOff())

        # set up collision mechanism
        self.pusher = CollisionHandlerPusher()
        self.pusher.setInPattern("enter%in")
        self.pusher.setOutPattern("exit%in")

        self.pusher.addCollider(self.cSphereNodePath, self.avatarNodePath)

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
        self.setupSphere(wallBitmask|floorBitmask, avatarRadius)

        self.setCollisionsActive(1)

    def setAirborneHeightFunc(self, getAirborneHeight):
        self.getAirborneHeight = getAirborneHeight

    def setAvatarPhysicsIndicator(self, indicator):
        """
        indicator is a NodePath
        """
        assert(self.debugPrint("setAvatarPhysicsIndicator()"))
        self.cSphereNodePath.show()

    def deleteCollisions(self):
        assert(self.debugPrint("deleteCollisions()"))
        del self.cTrav

        if self.useHeightRay:
            del self.cRayQueue
            self.cRayNodePath.removeNode()
            del self.cRayNodePath

        del self.cSphere
        self.cSphereNodePath.removeNode()
        del self.cSphereNodePath

        del self.pusher
        del self.lifter

    def setCollisionsActive(self, active = 1):
        assert(self.debugPrint("collisionsActive(active=%s)"%(active,)))
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

    def getCollisionsActive(self):
        assert(self.debugPrint("getCollisionsActive() returning=%s"%(
            self.collisionsActive,)))
        return self.collisionsActive

    def oneTimeCollide(self):
        """
        Makes one quick collision pass for the avatar, for instance as
        a one-time straighten-things-up operation after collisions
        have been disabled.
        """
        assert(self.debugPrint("oneTimeCollide()"))
        tempCTrav = CollisionTraverser()
        tempCTrav.addCollider(self.cSphereNodePath, self.pusher)
        tempCTrav.addCollider(self.cRayNodePath, self.lifter)
        tempCTrav.traverse(render)
    
    def setMayJump(self, task):
        self.mayJump = 1
        return Task.done

    def startJumpDelay(self):
        assert(self.debugPrint("startJumpDelay()"))
        if self.jumpDelayTask:
            self.jumpDelayTask.remove()
        self.mayJump = 0
        self.jumpDelayTask=taskMgr.doMethodLater(
            0.5,
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
        slide = inputState.isSet("slide")
        jump = inputState.isSet("jump")
        pie = inputState.isSet("pie")
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

        # No moving allowed while throwing a pie.
        if pie:
            self.speed = 0
            self.slideSpeed = 0
            self.rotationSpeed = 0
            jump = 0

        if 1:
            onScreenDebug.add("airborneHeight", self.lifter.getAirborneHeight()) #*#
            onScreenDebug.add("falling", self.falling) #*#
            onScreenDebug.add("isOnGround", self.lifter.isOnGround()) #*#
            onScreenDebug.add("velocity", self.lifter.getVelocity()) #*#
            onScreenDebug.add("jump", jump) #*#
        if self.lifter.isOnGround():
            if self.falling:
                self.falling = 0
                #messenger.send("jumpHardLand")
                messenger.send("jumpLand")
                self.startJumpDelay()
            if jump and self.mayJump:
                # ...the jump button is down and we're close
                # enough to the ground to jump.
                self.lifter.addVelocity(self.avatarControlJumpForce)
                messenger.send("jumpStart")
                self.falling = 1
        else:
            self.falling = 1
        #    if self.lifter.getAirborneHeight() > 10000.0:
        #        assert(0)

        # Check to see if we're moving at all:
        if self.speed or self.slideSpeed or self.rotationSpeed:
            # How far did we move based on the amount of time elapsed?
            dt=min(ClockObject.getGlobalClock().getDt(), 0.1)
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
                self.avatarNodePath.setFluidPos(Point3(
                        self.avatarNodePath.getPos()+step))
            self.avatarNodePath.setH(self.avatarNodePath.getH()+rotation)
            messenger.send("avatarMoving")
        else:
            self.vel.set(0.0, 0.0, 0.0)
        return Task.cont
    
    def doDeltaPos(self):
        assert(self.debugPrint("doDeltaPos()"))
        self.needToDeltaPos = 1
    
    def setPriorParentVector(self):
        assert(self.debugPrint("doDeltaPos()"))
        
        print "self.__oldDt", self.__oldDt, "self.__oldPosDelta", self.__oldPosDelta
        if __debug__:
            onScreenDebug.add("__oldDt", "% 10.4f"%self.__oldDt)
            onScreenDebug.add("self.__oldPosDelta",
                              self.__oldPosDelta.pPrintValues())
        
        velocity = self.__oldPosDelta*(1/self.__oldDt)*4.0 # *4.0 is a hack
        assert(self.debugPrint("  __oldPosDelta=%s"%(self.__oldPosDelta,)))
        assert(self.debugPrint("  velocity=%s"%(velocity,)))
        self.priorParent.setVector(Vec3(velocity))
        if __debug__:
            if self.wantAvatarPhysicsIndicator:
                onScreenDebug.add("velocity", velocity.pPrintValues())

    def enableAvatarControls(self):
        """
        Activate the arrow keys, etc.
        """
        assert(self.debugPrint("enableAvatarControls()"))
        print id(self), "GW.enableAvatarControls()"
        assert self.collisionsActive

        if __debug__:
            self.accept("control-f3", self.spawnTest) #*#

        taskName = "AvatarControls%s"%(id(self),)
        # remove any old
        taskMgr.remove(taskName)
        # spawn the new task
        taskMgr.add(self.handleAvatarControls, taskName, 25)
        if self.physVelocityIndicator:
            taskMgr.add(self.avatarPhysicsIndicator, "AvatarControlsIndicator%s"%(id(self),), 35)

    def disableAvatarControls(self):
        """
        Ignore the arrow keys, etc.
        """
        assert(self.debugPrint("disableAvatarControls()"))
        print id(self), "GW.disableAvatarControls()"
        taskName = "AvatarControls%s"%(id(self),)
        taskMgr.remove(taskName)

        taskName = "AvatarControlsIndicator%s"%(id(self),)
        taskMgr.remove(taskName)

        if __debug__:
            self.ignore("control-f3") #*#

    def enableAvatarJump(self):
        """
        Stop forcing the jump key to return 0's
        """
        inputState.unforce("jump")

    def disableAvatarJump(self):
        """
        Force the jump key to return 0's
        """
        inputState.force("jump", 0)

    
    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(id(self))+' '+message)
