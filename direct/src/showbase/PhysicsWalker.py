"""
PhysicsWalker.py is for avatars.

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

#import LineStream


class PhysicsWalker(DirectObject.DirectObject):

    notify = DirectNotifyGlobal.directNotify.newCategory("PhysicsWalker")
    wantAvatarPhysicsIndicator = base.config.GetBool('want-avatar-physics-indicator', 0)

    # special methods
    def __init__(self, gravity = -32.1740, standableGround=0.707,
            hardLandingForce=16.0):
        assert(self.debugPrint("PhysicsWalker(gravity=%s, standableGround=%s)"%(
                gravity, standableGround)))
        DirectObject.DirectObject.__init__(self)
        self.__gravity=gravity
        self.__standableGround=standableGround
        self.__hardLandingForce=hardLandingForce
        
        self.needToDeltaPos = 0
        self.physVelocityIndicator=None
        self.__oldAirborneHeight=None
        self.getAirborneHeight=None
        self.__oldContact=None
        self.__oldPosDelta=Vec3(0)
        self.__oldDt=0
        self.__forwardButton=0
        self.__reverseButton=0
        self.__jumpButton=0
        self.__leftButton=0
        self.__rightButton=0
        self.__speed=0.0
        self.__rotationSpeed=0.0
        self.__slideSpeed=0.0
        self.__vel=Vec3(0.0)
        self.__slideButton = 0

    def spawnTest(self):
        assert(self.debugPrint("\n\nspawnTest()\n"))
        from PandaModules import *
        from IntervalGlobal import *
        import MovingPlatform
        
        if hasattr(self, "platform"):
            # Remove the prior instantiation:
            self.moveIval.pause()
            del self.moveIval
            self.platform.destroy()
            del self.platform
        
        model = loader.loadModelCopy('phase_7/models/cogHQ/platform1')
        fakeId = id(self)
        self.platform = MovingPlatform.MovingPlatform()
        self.platform.setupCopyModel(fakeId, model, 'platformcollision')
        self.platformRoot = render.attachNewNode("physicsWalker-spawnTest-%s"%fakeId)
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
        return (self.__speed, self.__rotationSpeed)

    def initializeCollisions(self, collisionTraverser, avatarNodePath, 
            wallBitmask, floorBitmask, 
            avatarRadius = 1.4, floorOffset = 1.0):
        """
        Set up the avatar collisions
        """
        assert(self.debugPrint("initializeCollisions()"))
        
        assert not avatarNodePath.isEmpty()
        
        self.cTrav = collisionTraverser

        # Set up the collision sphere
        # This is a sphere on the ground to detect barrier collisions
        self.cSphere = CollisionSphere(0.0, 0.0, avatarRadius, avatarRadius)
        self.cSphereNode = CollisionNode('cSphereNode')
        self.cSphereNode.addSolid(self.cSphere)
        self.cSphereNodePath = avatarNodePath.attachNewNode(self.cSphereNode)
        self.cSphereBitMask = wallBitmask|floorBitmask

        self.cSphereNode.setFromCollideMask(self.cSphereBitMask)
        self.cSphereNode.setIntoCollideMask(BitMask32.allOff())

        # set up collision mechanism
        self.pusher = PhysicsCollisionHandler()
        self.pusher.setInPattern("enter%in")
        self.pusher.setOutPattern("exit%in")

        # Connect to Physics Manager:
        self.actorNode=ActorNode("physicsActor")
        self.actorNode.getPhysicsObject().setOriented(1)
        self.actorNode.getPhysical(0).setViscosity(0.1)
        physicsActor=NodePath(self.actorNode)
        avatarNodePath.reparentTo(physicsActor)
        avatarNodePath.assign(physicsActor)
        self.phys=PhysicsManager.PhysicsManager()

        fn=ForceNode("gravity")
        fnp=NodePath(fn)
        #fnp.reparentTo(physicsActor)
        fnp.reparentTo(render)
        gravity=LinearVectorForce(0.0, 0.0, self.__gravity)
        fn.addForce(gravity)
        self.phys.addLinearForce(gravity)
        self.gravity = gravity

        fn=ForceNode("priorParent")
        fnp=NodePath(fn)
        fnp.reparentTo(render)
        priorParent=LinearVectorForce(0.0, 0.0, 0.0)
        fn.addForce(priorParent)
        self.phys.addLinearForce(priorParent)
        self.priorParentNp = fnp
        self.priorParent = priorParent

        fn=ForceNode("viscosity")
        fnp=NodePath(fn)
        #fnp.reparentTo(physicsActor)
        fnp.reparentTo(render)
        self.avatarViscosity=LinearFrictionForce(0.0, 1.0, 0)
        #self.avatarViscosity.setCoef(0.9)
        fn.addForce(self.avatarViscosity)
        self.phys.addLinearForce(self.avatarViscosity)

        self.phys.attachLinearIntegrator(LinearEulerIntegrator())
        self.phys.attachPhysicalnode(physicsActor.node())

        self.acForce=LinearVectorForce(0.0, 0.0, 0.0)
        fn=ForceNode("avatarControls")
        fnp=NodePath(fn)
        fnp.reparentTo(render)
        fn.addForce(self.acForce)
        self.phys.addLinearForce(self.acForce)
        #self.phys.removeLinearForce(self.acForce)
        #fnp.remove()

        # activate the collider with the traverser and pusher
        self.collisionsOn()
        self.pusher.addColliderNode(self.cSphereNode, avatarNodePath.node())
        
        self.avatarNodePath = avatarNodePath

    def setAirborneHeightFunc(self, getAirborneHeight):
        self.getAirborneHeight = getAirborneHeight

    def setAvatarPhysicsIndicator(self, indicator):
        """
        indicator is a NodePath
        """
        assert(self.debugPrint("setAvatarPhysicsIndicator()"))
        self.cSphereNodePath.show()
        if indicator:
            # Indicator Node:
            change=render.attachNewNode("change")
            #change.setPos(Vec3(1.0, 1.0, 1.0))
            #change.setHpr(0.0, 0.0, 0.0)
            change.setScale(0.1)
            #change.setColor(Vec4(1.0, 1.0, 1.0, 1.0))
            indicator.reparentTo(change)

            indicatorNode=render.attachNewNode("physVelocityIndicator")
            #indicatorNode.setScale(0.1)
            #indicatorNode.setP(90.0)
            indicatorNode.setPos(self.avatarNodePath, 0.0, 0.0, 6.0)
            indicatorNode.setColor(0.0, 0.0, 1.0, 1.0)
            change.reparentTo(indicatorNode)

            self.physVelocityIndicator=indicatorNode
            # Contact Node:
            contactIndicatorNode=render.attachNewNode("physContactIndicator")
            contactIndicatorNode.setScale(0.25)
            contactIndicatorNode.setP(90.0)
            contactIndicatorNode.setPos(self.avatarNodePath, 0.0, 0.0, 5.0)
            contactIndicatorNode.setColor(1.0, 0.0, 0.0, 1.0)
            indicator.instanceTo(contactIndicatorNode)
            self.physContactIndicator=contactIndicatorNode
        else:
            print "failed load of physics indicator"

    def avatarPhysicsIndicator(self, task):
        #assert(self.debugPrint("avatarPhysicsIndicator()"))
        # Velosity:
        self.physVelocityIndicator.setPos(self.avatarNodePath, 0.0, 0.0, 6.0)
        physObject=self.actorNode.getPhysicsObject()
        a=physObject.getVelocity()
        self.physVelocityIndicator.setScale(math.sqrt(a.length()))
        a+=self.physVelocityIndicator.getPos()
        self.physVelocityIndicator.lookAt(Point3(a))
        # Contact:
        contact=self.actorNode.getContactVector()
        if contact==Vec3.zero():
            self.physContactIndicator.hide()
        else:
            self.physContactIndicator.show()
            self.physContactIndicator.setPos(self.avatarNodePath, 0.0, 0.0, 5.0)
            #contact=self.actorNode.getContactVector()
            point=Point3(contact+self.physContactIndicator.getPos())
            self.physContactIndicator.lookAt(point)
        return Task.cont

    def deleteCollisions(self):
        assert(self.debugPrint("deleteCollisions()"))
        del self.cTrav

        del self.cSphere
        del self.cSphereNode
        self.cSphereNodePath.removeNode()
        del self.cSphereNodePath

        del self.pusher

    def collisionsOff(self):
        assert(self.debugPrint("collisionsOff()"))
        self.cTrav.removeCollider(self.cSphereNode)
        # Now that we have disabled collisions, make one more pass
        # right now to ensure we aren't standing in a wall.
        self.oneTimeCollide()

    def collisionsOn(self):
        assert(self.debugPrint("collisionsOn()"))
        self.cTrav.addCollider(self.cSphereNode, self.pusher)

    def oneTimeCollide(self):
        """
        Makes one quick collision pass for the avatar, for instance as
        a one-time straighten-things-up operation after collisions
        have been disabled.
        """
        assert(self.debugPrint("oneTimeCollide()"))
        tempCTrav = CollisionTraverser()
        tempCTrav.addCollider(self.cSphereNode, self.pusher)
        tempCTrav.traverse(render)

    def handleAvatarControls(self, task):
        """
        Check on the arrow keys and update the avatar.
        """
        if self.wantAvatarPhysicsIndicator:
            onScreenDebug.append("localToon pos = %s\n"%(toonbase.localToon.getPos().pPrintValues(),))
            onScreenDebug.append("localToon hpr = %s\n"%(toonbase.localToon.getHpr().pPrintValues(),))
        #assert(self.debugPrint("handleAvatarControls(task=%s)"%(task,)))
        physObject=self.actorNode.getPhysicsObject()
        #rotAvatarToPhys=Mat3.rotateMatNormaxis(-self.avatarNodePath.getH(), Vec3.up())
        #rotPhysToAvatar=Mat3.rotateMatNormaxis(self.avatarNodePath.getH(), Vec3.up())
        contact=self.actorNode.getContactVector()
        
        # hack fix for falling through the floor:
        if contact==Vec3.zero() and self.avatarNodePath.getZ()<-50.0:
            # reset:
            # DCR: don't reset X and Y; allow player to move
            self.avatarNodePath.setZ(50.0)
            self.resetPhys()

        # Determine what the speeds are based on the buttons:
        self.__speed=(self.__forwardButton and self.avatarControlForwardSpeed or 
                self.__reverseButton and -self.avatarControlReverseSpeed)
        self.__slideSpeed=self.__slideButton and (
                (self.__leftButton and -self.avatarControlForwardSpeed) or 
                (self.__rightButton and self.avatarControlForwardSpeed))
        self.__rotationSpeed=not self.__slideButton and (
                (self.__leftButton and self.avatarControlRotateSpeed) or
                (self.__rightButton and -self.avatarControlRotateSpeed))
        # How far did we move based on the amount of time elapsed?
        dt=min(ClockObject.getGlobalClock().getDt(), 0.1)

        if self.needToDeltaPos:
            self.setPriorParentVector()
            self.needToDeltaPos = 0
        #self.__oldPosDelta = render.getRelativeVector(
        #    self.avatarNodePath,
        #    self.avatarNodePath.getPosDelta(render))
        #self.__oldPosDelta = self.avatarNodePath.getRelativeVector(
        #    render,
        #    self.avatarNodePath.getPosDelta(render))
        self.__oldPosDelta = self.avatarNodePath.getPosDelta(render)
        self.__oldDt = dt
        #posDelta = self.avatarNodePath.getPosDelta(render)
        #if posDelta==Vec3.zero():
        #    self.priorParent.setVector(self.__oldPosDelta)
        #else:
        #    self.priorParent.setVector(Vec3.zero())
        #    # We must copy the vector to preserve it:
        #    self.__oldPosDelta=Vec3(posDelta)
        if 1 and __debug__:
            if self.wantAvatarPhysicsIndicator:
                onScreenDebug.add("posDelta1",
                    self.avatarNodePath.getPosDelta(render).pPrintValues())
                
                # is same as posDelta1:
                #onScreenDebug.add("posDelta2",
                #    self.avatarNodePath.getPosDelta(self.priorParentNp).pPrintValues())
                
                # is always zero:
                #onScreenDebug.add("posDelta2.5",
                #    self.avatarNodePath.getPosDelta(self.avatarNodePath).pPrintValues())
                
                if 0:
                    onScreenDebug.add("posDelta3",
                        render.getRelativeVector(
                            self.avatarNodePath,
                            self.avatarNodePath.getPosDelta(render)).pPrintValues())
                
                if 1:
                    onScreenDebug.add("gravity",
                        self.gravity.getLocalVector().pPrintValues())
                    onScreenDebug.add("priorParent",
                        self.priorParent.getLocalVector().pPrintValues())
                    onScreenDebug.add("avatarViscosity",
                        "% 10.4f"%(self.avatarViscosity.getCoef(),))
                    
                    onScreenDebug.add("physObject pos",
                        physObject.getPosition().pPrintValues())
                    onScreenDebug.add("physObject hpr",
                        physObject.getOrientation().getHpr().pPrintValues())
                    onScreenDebug.add("physObject orien",
                        physObject.getOrientation().pPrintValues())

                if 0:
                    onScreenDebug.add("posDelta4", 
                        self.priorParentNp.getRelativeVector(
                            render,
                            self.avatarNodePath.getPosDelta(render)).pPrintValues())

                if 0:
                    onScreenDebug.add("priorParent",
                        self.priorParent.getLocalVector().pPrintValues())

                if 0:
                    onScreenDebug.add("priorParent po",
                        self.priorParent.getVector(physObject).pPrintValues())

                if 0:
                    onScreenDebug.add("__posDelta",
                        self.__oldPosDelta.pPrintValues())

                if 1:
                    onScreenDebug.add("airborneHeight", "% 10.4f"%(
                        self.getAirborneHeight(),))
        airborneHeight=self.getAirborneHeight()
        #if airborneHeight < 0.1: #contact!=Vec3.zero():
        if contact!=Vec3.zero():
            contactLength = contact.length()
            contact.normalize()
            angle=contact.dot(Vec3.up())
            if angle>self.__standableGround:
                # ...avatar is on standable ground.
                if self.__oldAirborneHeight > 0.1: #self.__oldContact==Vec3.zero():
                    # ...avatar was airborne.
                    if contactLength>self.__hardLandingForce:
                        messenger.send("jumpHardLand")
                    else:
                        messenger.send("jumpLand")
                if self.__jumpButton:
                    self.__jumpButton=0
                    messenger.send("jumpStart")
                    jump=Vec3(contact+Vec3.up())
                    #jump=Vec3(rotAvatarToPhys.xform(jump))
                    jump.normalize()
                    jump*=self.avatarControlJumpForce
                    physObject.addImpulse(Vec3(jump))
        if contact!=self.__oldContact:
            # We must copy the vector to preserve it:
            self.__oldContact=Vec3(contact)
        self.__oldAirborneHeight=airborneHeight
        self.phys.doPhysics(dt)
        # Check to see if we're moving at all:
        if self.__speed or self.__slideSpeed or self.__rotationSpeed:
            distance = dt * self.__speed
            slideDistance = dt * self.__slideSpeed
            rotation = dt * self.__rotationSpeed

            #debugTempH=self.avatarNodePath.getH()
            assert self.avatarNodePath.getHpr().almostEqual(physObject.getOrientation().getHpr(), 0.0001)
            assert self.avatarNodePath.getPos().almostEqual(physObject.getPosition(), 0.0001)

            # update pos:
            # Take a step in the direction of our previous heading.
            self.__vel=Vec3(
                Vec3.forward() * distance + 
                Vec3.right() * slideDistance)
            
            # rotMat is the rotation matrix corresponding to
            # our previous heading.
            rotMat=Mat3.rotateMatNormaxis(self.avatarNodePath.getH(), Vec3.up())
            step=rotMat.xform(self.__vel)
            physObject.setPosition(Point3(
                physObject.getPosition()+step))

            # update hpr:
            o=physObject.getOrientation()
            r=LOrientationf()
            r.setHpr(Vec3(rotation, 0.0, 0.0))
            physObject.setOrientation(o*r)

            # sync the change:
            self.actorNode.updateTransform()

            assert self.avatarNodePath.getHpr().almostEqual(physObject.getOrientation().getHpr(), 0.0001)
            assert self.avatarNodePath.getPos().almostEqual(physObject.getPosition(), 0.0001)
            #assert self.avatarNodePath.getH()==debugTempH-rotation
            messenger.send("avatarMoving")
        else:
            self.__vel.set(0.0, 0.0, 0.0)
        # Clear the contact vector so we can tell if we contact something next frame:
        self.actorNode.setContactVector(Vec3.zero())
        return Task.cont

    def handleAvatarControls_wip(self, task):
        """
        Check on the arrow keys and update the avatar.
        """
        #assert(self.debugPrint("handleAvatarControls(task=%s)"%(task,)))
        physObject=self.actorNode.getPhysicsObject()
        #rotAvatarToPhys=Mat3.rotateMatNormaxis(-self.avatarNodePath.getH(), Vec3.up())
        #rotPhysToAvatar=Mat3.rotateMatNormaxis(self.avatarNodePath.getH(), Vec3.up())
        contact=self.actorNode.getContactVector()
        
        # hack fix for falling through the floor:
        if contact==Vec3.zero() and self.avatarNodePath.getZ()<-50.0:
            # reset:
            self.avatarNodePath.setPos(Vec3(0.0, 0.0, 20.0))

        # Determine what the speeds are based on the buttons:
        self.__speed=(self.__forwardButton and self.avatarControlForwardSpeed or 
                    self.__reverseButton and -self.avatarControlReverseSpeed)
        self.__slideSpeed=self.__slideButton and (
                (self.__leftButton and -self.avatarControlForwardSpeed) or 
                (self.__rightButton and self.avatarControlForwardSpeed))
        self.__rotationSpeed=not self.__slideButton and (
                (self.__leftButton and self.avatarControlRotateSpeed) or
                (self.__rightButton and -self.avatarControlRotateSpeed))
        # How far did we move based on the amount of time elapsed?
        dt=min(ClockObject.getGlobalClock().getDt(), 0.1)

        doPhysics=1
        if not contact.almostEqual(Vec3.zero()):
            contactLength = contact.length()
            contact.normalize()
            angle=contact.dot(Vec3.up())
            if angle>self.__standableGround:
                # ...avatar is on standable ground.
                #print "standableGround"
                if self.__oldContact==Vec3.zero():
                    if contactLength>self.__hardLandingForce:
                        # ...avatar was airborne.
                        messenger.send("jumpHardLand")
                    else:
                        messenger.send("jumpLand")
                if self.__jumpButton:
                    self.__jumpButton=0
                    messenger.send("jumpStart")
                    jump=Vec3(contact+Vec3.up())
                    #jump=Vec3(rotAvatarToPhys.xform(jump))
                    jump.normalize()
                    jump*=self.avatarControlJumpForce
                    physObject.addImpulse(Vec3(jump))
                else:
                    physObject.setVelocity(Vec3(0.0))
                    self.__vel.set(0.0, 0.0, 0.0)
                    doPhysics=0
        if contact!=self.__oldContact:
            # We must copy the vector to preserve it:
            self.__oldContact=Vec3(contact)
        #print "doPhysics", doPhysics
        #print "contact", contact
        if doPhysics:
            self.phys.doPhysics(dt)
        # Check to see if we're moving at all:
        if self.__speed or self.__slideSpeed or self.__rotationSpeed:
            distance = dt * self.__speed
            slideDistance = dt * self.__slideSpeed
            rotation = dt * self.__rotationSpeed

            #debugTempH=self.avatarNodePath.getH()
            assert self.avatarNodePath.getHpr().almostEqual(physObject.getOrientation().getHpr(), 0.0001)
            assert self.avatarNodePath.getPos().almostEqual(physObject.getPosition(), 0.0001)

            # update pos:
            # Take a step in the direction of our previous heading.
            self.__vel=Vec3(Vec3.forward() * distance + 
                          Vec3.right() * slideDistance)
            # rotMat is the rotation matrix corresponding to
            # our previous heading.
            rotMat=Mat3.rotateMatNormaxis(self.avatarNodePath.getH(), Vec3.up())
            step=rotMat.xform(self.__vel)
            physObject.setPosition(Point3(
                physObject.getPosition()+step))
            # update hpr:
            o=physObject.getOrientation()
            r=LOrientationf()
            r.setHpr(Vec3(rotation, 0.0, 0.0))
            physObject.setOrientation(o*r)
            # sync the change:
            self.actorNode.updateTransform()

            assert self.avatarNodePath.getHpr().almostEqual(physObject.getOrientation().getHpr(), 0.0001)
            assert self.avatarNodePath.getPos().almostEqual(physObject.getPosition(), 0.0001)
            #assert self.avatarNodePath.getH()==debugTempH-rotation
            messenger.send("avatarMoving")
        else:
            self.__vel.set(0.0, 0.0, 0.0)
        # Clear the contact vector so we can tell if we contact something next frame:
        self.actorNode.setContactVector(Vec3.zero())
        return Task.cont
    
    def doDeltaPos(self):
        assert(self.debugPrint("doDeltaPos()"))
        self.needToDeltaPos = 1
    
    def setPriorParentVector(self):
        assert(self.debugPrint("doDeltaPos()"))
        print "self.__oldDt", self.__oldDt, "self.__oldPosDelta", self.__oldPosDelta
        velocity = self.__oldPosDelta*(1/self.__oldDt)
        assert(self.debugPrint("  __oldPosDelta=%s"%(self.__oldPosDelta,)))
        assert(self.debugPrint("  velocity=%s"%(velocity,)))
        self.priorParent.setVector(Vec3(velocity))
        if self.wantAvatarPhysicsIndicator:
            onScreenDebug.add("velocity", velocity.pPrintValues())
    
    def resetPhys(self):
        assert(self.debugPrint("resetPhys()"))
        self.actorNode.getPhysicsObject().resetPosition(self.avatarNodePath.getPos())
        self.priorParent.setVector(Vec3.zero())
        self.actorNode.setContactVector(Vec3.zero())

    def enableAvatarControls(self):
        """
        Activate the arrow keys, etc.
        """
        assert(self.debugPrint("enableAvatarControls()"))
        self.accept("control", self.moveJump, [1])
        self.accept("control-up", self.moveJump, [0])
        self.accept("control-arrow_left", self.moveJumpLeft, [1])
        self.accept("control-arrow_left-up", self.moveJumpLeft, [0])
        self.accept("control-arrow_right", self.moveJumpRight, [1])
        self.accept("control-arrow_right-up", self.moveJumpRight, [0])
        self.accept("control-arrow_up", self.moveJumpForward, [1])
        self.accept("control-arrow_up-up", self.moveJumpForward, [0])
        self.accept("control-arrow_down", self.moveJumpInReverse, [1])
        self.accept("control-arrow_down-up", self.moveJumpInReverse, [0])
        
        self.accept("arrow_left", self.moveTurnLeft, [1])
        self.accept("arrow_left-up", self.moveTurnLeft, [0])
        self.accept("arrow_right", self.moveTurnRight, [1])
        self.accept("arrow_right-up", self.moveTurnRight, [0])
        self.accept("arrow_up", self.moveForward, [1])
        self.accept("arrow_up-up", self.moveForward, [0])
        self.accept("arrow_down", self.moveInReverse, [1])
        self.accept("arrow_down-up", self.moveInReverse, [0])

        if __debug__:
            self.accept("control-f3", self.spawnTest) #*#
            self.accept("f3", self.resetPhys) # for debugging only.

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
        taskName = "AvatarControls%s"%(id(self),)
        taskMgr.remove(taskName)

        taskName = "AvatarControlsIndicator%s"%(id(self),)
        taskMgr.remove(taskName)

        self.ignore("control")
        self.ignore("control-up")
        self.ignore("control-arrow_left")
        self.ignore("control-arrow_left-up")
        self.ignore("control-arrow_right")
        self.ignore("control-arrow_right-up")
        self.ignore("control-arrow_up")
        self.ignore("control-arrow_up-up")
        self.ignore("control-arrow_down")
        self.ignore("control-arrow_down-up")

        self.ignore("arrow_left")
        self.ignore("arrow_left-up")
        self.ignore("arrow_right")
        self.ignore("arrow_right-up")
        self.ignore("arrow_up")
        self.ignore("arrow_up-up")
        self.ignore("arrow_down")
        self.ignore("arrow_down-up")


        if __debug__:
            self.ignore("control-f3") #*#
            self.ignore("f3")

        # reset state
        self.moveTurnLeft(0)
        self.moveTurnRight(0)
        self.moveForward(0)
        self.moveInReverse(0)
        self.moveJumpLeft(0)
        self.moveJumpRight(0)
        self.moveJumpForward(0)
        self.moveJumpInReverse(0)
        self.moveJump(0)
        self.moveSlide(0)

    def moveTurnLeft(self, isButtonDown):
        assert(self.debugPrint("moveTurnLeft(isButtonDown=%s)"%(isButtonDown,)))
        self.__leftButton=isButtonDown

    def moveTurnRight(self, isButtonDown):
        assert(self.debugPrint("moveTurnRight(isButtonDown=%s)"%(isButtonDown,)))
        self.__rightButton=isButtonDown

    def moveForward(self, isButtonDown):
        assert(self.debugPrint("moveForward(isButtonDown=%s)"%(isButtonDown,)))
        self.__forwardButton=isButtonDown

    def moveInReverse(self, isButtonDown):
        assert(self.debugPrint("moveInReverse(isButtonDown=%s)"%(isButtonDown,)))
        self.__reverseButton=isButtonDown

    def moveJumpLeft(self, isButtonDown):
        assert(self.debugPrint("moveJumpLeft(isButtonDown=%s)"%(isButtonDown,)))
        self.__jumpButton=isButtonDown
        self.__leftButton=isButtonDown

    def moveJumpRight(self, isButtonDown):
        assert(self.debugPrint("moveJumpRight(isButtonDown=%s)"%(isButtonDown,)))
        self.__jumpButton=isButtonDown
        self.__rightButton=isButtonDown

    def moveJumpForward(self, isButtonDown):
        assert(self.debugPrint("moveJumpForward(isButtonDown=%s)"%(isButtonDown,)))
        self.__jumpButton=isButtonDown
        self.__forwardButton=isButtonDown

    def moveJumpInReverse(self, isButtonDown):
        assert(self.debugPrint("moveJumpInReverse(isButtonDown=%s)"%(isButtonDown,)))
        self.__jumpButton=isButtonDown
        self.__reverseButton=isButtonDown

    def moveJump(self, isButtonDown):
        assert(self.debugPrint("moveJump()"))
        self.__jumpButton=isButtonDown

    def moveSlide(self, isButtonDown):
        assert(self.debugPrint("moveSlide(isButtonDown=%s)"%(isButtonDown,)))
        self.__slideButton=isButtonDown
    
    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(id(self))+' '+message)
