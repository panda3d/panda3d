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

class PhysicsWalker(DirectObject.DirectObject):

    notify = DirectNotifyGlobal.directNotify.newCategory("PhysicsWalker")
    wantAvatarPhysicsIndicator = base.config.GetBool('want-avatar-physics-indicator', 1)

    # special methods
    def __init__(self, gravity = -32.1740, standableGround=0.707,
            hardLandingForce=16.0):
        assert(self.debugPrint("PhysicsWalker(gravity=%s, standableGround=%s)"%(
                gravity, standableGround)))
        DirectObject.DirectObject.__init__(self)
        self.__gravity=gravity
        self.__standableGround=standableGround
        self.__hardLandingForce=hardLandingForce

        self.physVelocityIndicator=None
        self.__old_contact=None
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
        #assert(self.debugPrint("handleAvatarControls(task=%s)"%(task,)))
        physObject=self.actorNode.getPhysicsObject()
        rotAvatarToPhys=Mat3.rotateMatNormaxis(-self.avatarNodePath.getH(), Vec3.up())
        #rotPhysToAvatar=Mat3.rotateMatNormaxis(self.avatarNodePath.getH(), Vec3.up())
        contact=self.actorNode.getContactVector()
        
        # hack fix for falling through the floor:
        if contact==Vec3.zero() and self.avatarNodePath.getZ()<-50.0:
            # reset:
            self.setPos(Vec3(0.0, 0.0, 20.0))

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

        if contact!=Vec3.zero():
            contactLength = contact.length()
            contact.normalize()
            angle=contact.dot(Vec3.up())
            if angle>self.__standableGround:
                # ...avatar is on standable ground.
                if self.__old_contact==Vec3.zero():
                    jumpTime = 0.0
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
        if contact!=self.__old_contact:
            # We must copy the vector to preserve it:
            self.__old_contact=Vec3(contact)
        self.phys.doPhysics(dt)
        # Check to see if we're moving at all:
        if self.__speed or self.__slideSpeed or self.__rotationSpeed:
            distance = dt * self.__speed
            slideDistance = dt * self.__slideSpeed
            rotation = dt * self.__rotationSpeed

            #debugTempH=self.avatarNodePath.getH()
            assert self.avatarNodePath.getPos()==physObject.getPosition()
            assert self.avatarNodePath.getHpr()==physObject.getOrientation().getHpr()

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

            assert self.avatarNodePath.getHpr()==physObject.getOrientation().getHpr()
            assert self.avatarNodePath.getPos()==physObject.getPosition()
            #assert self.avatarNodePath.getH()==debugTempH-rotation
            messenger.send("avatarMoving")
        else:
            self.__vel.set(0.0, 0.0, 0.0)
        # Clear the contact vector so we can tell if we contact something next frame:
        self.actorNode.setContactVector(Vec3.zero())
        # Set collision sphere node:
        v=physObject.getImplicitVelocity()
        v=rotAvatarToPhys.xform(v)
        self.cSphereNode.setVelocity(Vec3(v))
        return Task.cont
    
    def resetPhys(self):
        assert(self.debugPrint("resetPhys()"))
        self.actorNode.getPhysicsObject().resetPosition(self.avatarNodePath.getPos())
        self.actorNode.setContactVector(Vec3.zero())
        self.cSphereNode.setVelocity(Vec3(0.0))

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

        if 1:
            self.accept("f3", self.resetPhys) # for debugging only.

        taskName = "AvatarControls%s"%(id(self),)
        # remove any old
        taskMgr.remove(taskName)
        # spawn the new task
        taskMgr.add(self.handleAvatarControls, taskName)
        if self.physVelocityIndicator:
            taskMgr.add(self.avatarPhysicsIndicator, "AvatarControlsIndicator%s"%(id(self),), 47)

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
