
"""
PirateSwimmer.py

Derived from NonPhysicsWalker.py.  Only difference is that it doesn't have a lifter.
This is set up so we can swim without a floor on the surface of the ocean, like Toontown
did.
"""
from direct.showbase.ShowBaseGlobal import *

from direct.directnotify import DirectNotifyGlobal
from direct.showbase import DirectObject

import NonPhysicsWalker

class PirateSwimmer(NonPhysicsWalker.NonPhysicsWalker):

    def __init__(self):
        NonPhysicsWalker.NonPhysicsWalker.__init__(self)

    def initializeCollisions(self, collisionTraverser, avatarNodePath,
            wallCollideMask, floorCollideMask,
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
        self.cSphereBitMask = wallCollideMask

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
        self.cRayBitMask = floorCollideMask
        cRayNode.setFromCollideMask(self.cRayBitMask)
        cRayNode.setIntoCollideMask(BitMask32.allOff())

        # set up wall collision mechanism
        self.pusher = CollisionHandlerPusher()
        self.pusher.setInPattern("enter%in")
        self.pusher.setOutPattern("exit%in")

        self.pusher.addCollider(self.cSphereNodePath, avatarNodePath)

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

        
    def setCollisionsActive(self, active = 1):
        assert(self.debugPrint("setCollisionsActive(active%s)"%(active,)))
        if self.collisionsActive != active:
            self.collisionsActive = active
            if active:
                self.cTrav.addCollider(self.cSphereNodePath, self.pusher)
            else:
                self.cTrav.removeCollider(self.cSphereNodePath)

                # Now that we have disabled collisions, make one more pass
                # right now to ensure we aren't standing in a wall.
                self.oneTimeCollide()

    def oneTimeCollide(self):
        """
        Makes one quick collision pass for the avatar, for instance as
        a one-time straighten-things-up operation after collisions
        have been disabled.
        """
        tempCTrav = CollisionTraverser("oneTimeCollide")
        tempCTrav.addCollider(self.cSphereNodePath, self.pusher)
        tempCTrav.traverse(render)

    def handleAvatarControls(self, task):
        """
        Check on the arrow keys and update the avatar.
        """
        # get the button states:
        forward = inputState.isSet("forward")
        reverse = inputState.isSet("reverse")
        turnLeft = inputState.isSet("turnLeft")
        turnRight = inputState.isSet("turnRight")
        slide = inputState.isSet(self.slideName) or 0
        #jump = inputState.isSet("jump")
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
        return Task.cont
