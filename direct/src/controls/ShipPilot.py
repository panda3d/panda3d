"""
ShipPilot.py is for avatars pilotting ships (or more accurately, a ship
as the avatar).

A control such as this one provides:
    - creation of the collision nodes
    - handling the keyboard and mouse input for avatar movement
    - moving the avatar

it does not:
    - play sounds
    - play animations

although it does send messeges that allow a listener to play sounds or
animations based on control events.
"""

from pandac.PandaModules import *
from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.InputStateGlobal import inputState
from direct.task.Task import Task
from PhysicsWalker import PhysicsWalker
import math

class ShipPilot(PhysicsWalker):
    notify = directNotify.newCategory("ShipPilot")
    wantDebugIndicator = base.config.GetBool(
        'want-avatar-physics-indicator', 0)
    
    MAX_STRAIGHT_SAIL_BONUS = 1.25
    STRAIGHT_SAIL_BONUS_TIME = 10.0
    
    # special methods
    def __init__(self,
                 gravity = -32.1740,
                 standableGround=0.707,
                 hardLandingForce=16.0):
        assert self.debugPrint(
            "PhysicsWalker(gravity=%s, standableGround=%s)"%(
            gravity, standableGround))
        PhysicsWalker.__init__(
            self, gravity, standableGround, hardLandingForce)
        
        self.__speed=0.0
        self.__rotationSpeed=0.0
        self.__slideSpeed=0.0
        self.__vel=Vec3(0.0)
        self.currentTurning = 0.0
        
        self.ship = None
        self.pusher = None

        # Keeps track of the ship sailing in a straight heading
        # for long periods of time.  We slowly up the ship's max
        # acceleration as this increases.
        self.straightHeading = 0

        self.cNodePath = None
        
    def setWalkSpeed(self, forward, jump, reverse, rotate):
        assert self.debugPrint("setWalkSpeed()")
        PhysicsWalker.setWalkSpeed(self, forward, 0, reverse, rotate)

    def setWallBitMask(self, bitMask):
        self.wallBitmask = bitMask

    def adjustWallBitMask(self, oldMask, newMask):
        # this will clear any bits set in oldMask and set any bits
        # set in newMask
        self.wallBitmask = self.wallBitmask &~ oldMask
        self.wallBitmask |= newMask

        if self.cNodePath and not self.cNodePath.isEmpty():
            self.cNodePath.node().setFromCollideMask(self.wallBitmask)
        
    def setFloorBitMask(self, bitMask):
        self.floorBitmask = bitMask

    def setShip(self, ship):
        self.setAvatar(ship)
        
    def setAvatar(self, ship):
        if ship is None:
            # only clear out the global controlForce's physics object if it hasn't
            # been changed since we set it up for our boat
            # this could still break if we have more than one ShipPilot referencing
            # a single boat
            if (self.ship is not None and
                base.controlForce.getPhysicsObject() == self.ship.node().getPhysicsObject()):
                base.controlForce.clearPhysicsObject()
                base.controlForce.setVector(Vec3(0))
            self.takedownPhysics()
            self.setCollisionsActive(0)
            self.ship = ship
        else:
            base.controlForce.setPhysicsObject(ship.node().getPhysicsObject())
            self.ship = ship
            self.setupPhysics(ship)
            self.setCollisionsActive(1)

    def initializeCollisions(self, collisionTraverser, cRootNodePath,
                             bow, stern, starboard, port):
        """
        Set up the avatar collisions.  All we do here is assign
        the traverser.  All the ship specific stuff happens in
        setAvatar() and its helper functions.
        """
        assert self.debugPrint("initializeCollisions()")
        self.cTrav = collisionTraverser
        self.cRootNodePath = cRootNodePath
        self.bowPos = bow.getPos(cRootNodePath)
        self.sternPos = stern.getPos(cRootNodePath)
        self.starboardPos = starboard.getPos(cRootNodePath)
        self.portPos = port.getPos(cRootNodePath)

    def setupCollisions(self):
        if self.pusher:
            return
        """
        Set up the collision sphere
        """
        # This is a sphere on the ground to detect barrier collisions

        # set up collision mechanism
        self.pusher = CollisionHandlerPusher()
        self.pusher.setInPattern("enter%in")
        self.pusher.setOutPattern("exit%in")

        sRadius = abs((self.portPos - self.starboardPos)[0] / 2.0)

        cNode = CollisionNode('SP.cNode')

        # Front sphere:
        frontPos = self.bowPos[1]+sRadius
        rearPos = self.sternPos[1]-sRadius
        cBowSphere = CollisionSphere(
            0.0, frontPos, 0.0, sRadius)

        # Back sphere:
        cSternSphere = CollisionSphere(
            0.0, rearPos, 0.0, sRadius)

        # Mid sphere:  create to fill in the gap between front and back spheres,
        # minimum radius it can be is the radius of the front and rear sphere, but
        # allow it to get bigger to completely fill the middle
        midSphereRadius = max(sRadius,((rearPos - frontPos) - (sRadius * 2))/2)
        cMidSphere = CollisionSphere(
            0.0, frontPos + ((rearPos - frontPos) / 2), 0.0, midSphereRadius)

        # Scaled sphere
        # jbutler: abandoned using the stretched sphere since there are problems
        # with pushing against a scaled sphere
#        cObj = CollisionSphere(
#            0.0, (self.sternPos[1]+self.bowPos[1])/2.0, 0.0, sRadius)

        ## Combine them all
        cNode.addSolid(cBowSphere)
        cNode.addSolid(cMidSphere)
        cNode.addSolid(cSternSphere)
        #cNode.addSolid(cObj)
        shipIColRoot = self.ship.getInteractCollisionRoot()
        self.cNodePath = shipIColRoot.attachNewNode(cNode)
        shipLen = abs(self.sternPos[1]-self.bowPos[1])
        self.cNodePath.setScale(1,1,1)
        #self.cNodePath.setScale(1,(shipLen/2.0)/sRadius,1)
        self.pusher.addCollider(
            self.cNodePath, self.shipNodePath)
        self.pusher.setHorizontal(True)
        # hide other things on my ship that these spheres might collide
        # with and which I dont need anyways...
        shipCollWall = self.shipNodePath.find("**/collision_hull")
        if not shipCollWall.isEmpty():
            shipCollWall.stash()

    def setCollisionsActive(self, active = 1):
        if active:
            self.setupCollisions()

        assert self.debugPrint("collisionsActive(active=%s)"%(active,))
        if self.collisionsActive != active:
            self.collisionsActive = active
            shipCollWall = self.shipNodePath.find("**/collision_hull;+s")
            if active:
                self.cNodePath.node().setFromCollideMask(self.wallBitmask)
                self.cNodePath.node().setIntoCollideMask(BitMask32.allOff())
                self.cTrav.addCollider(self.cNodePath, self.pusher)
                shipCollWall.stash()
            else:
                self.cTrav.removeCollider(self.cNodePath)
                shipCollWall.unstash()
                # Now that we have disabled collisions, make one more pass
                # right now to ensure we aren't standing in a wall.
                self.oneTimeCollide()

    def deleteCollisions(self):
        assert self.debugPrint("deleteCollisions()")
        del self.cTrav

        """
        if hasattr(self, "cBowSphereNodePath"):
            self.cBowSphereNodePath.removeNode()
            del self.cBowSphereNodePath

        if hasattr(self, "cSternSphereNodePath"):
            self.cSternSphereNodePath.removeNode()
            del self.cSternSphereNodePath
        """
        if hasattr(self, "cNodePath"):
            self.cNodePath.removeNode()
            self.cNodePath = None

            del self.pusher

    def setupPhysics(self, shipNodePath):
        assert self.debugPrint("setupPhysics()")
        if shipNodePath is None:
            return
        assert not shipNodePath.isEmpty()

        self.takedownPhysics()

        # these are used in collision setup and handleControls
        self.shipNodePath = shipNodePath
        self.actorNode = shipNodePath.node()
        
    def takedownPhysics(self):
        assert self.debugPrint("takedownPhysics()")

    def setTag(self, key, value):
        self.cNodePath.setTag(key, value)

    def setAvatarPhysicsIndicator(self, indicator):
        """
        indicator is a NodePath
        """
        assert self.debugPrint("setAvatarPhysicsIndicator()")
        self.cNodePath.show()
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
            indicatorNode.setPos(self.shipNodePath, 0.0, 0.0, 6.0)
            indicatorNode.setColor(0.0, 0.0, 1.0, 1.0)
            change.reparentTo(indicatorNode)

            self.physVelocityIndicator=indicatorNode
            # Contact Node:
            contactIndicatorNode=render.attachNewNode("physContactIndicator")
            contactIndicatorNode.setScale(0.25)
            contactIndicatorNode.setP(90.0)
            contactIndicatorNode.setPos(self.shipNodePath, 0.0, 0.0, 5.0)
            contactIndicatorNode.setColor(1.0, 0.0, 0.0, 1.0)
            indicator.instanceTo(contactIndicatorNode)
            self.physContactIndicator=contactIndicatorNode
        else:
            print "failed load of physics indicator"

    def avatarPhysicsIndicator(self, task):
        #assert self.debugPrint("avatarPhysicsIndicator()")
        # Velocity:
        self.physVelocityIndicator.setPos(self.shipNodePath, 0.0, 0.0, 6.0)
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
            self.physContactIndicator.setPos(self.shipNodePath, 0.0, 0.0, 5.0)
            #contact=self.actorNode.getContactVector()
            point=Point3(contact+self.physContactIndicator.getPos())
            self.physContactIndicator.lookAt(point)
        return Task.cont

    def displayDebugInfo(self):
        """
        For debug use.
        """
        onScreenDebug.add("w controls", "ShipPilot")

        onScreenDebug.add("w ship", self.ship)
        onScreenDebug.add("w isAirborne", self.isAirborne)
        onScreenDebug.add("posDelta1",
            self.shipNodePath.getPosDelta(render).pPrintValues())

        physObject=self.actorNode.getPhysicsObject()
        if 0:
            onScreenDebug.add("w posDelta3",
                render.getRelativeVector(
                    self.shipNodePath,
                    self.shipNodePath.getPosDelta(render)).pPrintValues())
        if 0:
            onScreenDebug.add("w priorParent",
                self.priorParent.getLocalVector().pPrintValues())

            onScreenDebug.add("w physObject pos",
                physObject.getPosition().pPrintValues())
            onScreenDebug.add("w physObject hpr",
                physObject.getOrientation().getHpr().pPrintValues())
            onScreenDebug.add("w physObject orien",
                physObject.getOrientation().pPrintValues())
        if 1:
            physObject = physObject.getVelocity()
            onScreenDebug.add("w physObject vec",
                physObject.pPrintValues())
            onScreenDebug.add("w physObject len",
                "% 10.4f"%physObject.length())

            onScreenDebug.add("orientation",
                self.actorNode.getPhysicsObject().getOrientation().pPrintValues())

        if 0:
            momentumForce = self.momentumForce.getLocalVector()
            onScreenDebug.add("w momentumForce vec",
                momentumForce.pPrintValues())
            onScreenDebug.add("w momentumForce len",
                "% 10.4f"%momentumForce.length())

        if 0:
            onScreenDebug.add("posDelta4",
                self.priorParentNp.getRelativeVector(
                    render,
                    self.shipNodePath.getPosDelta(render)).pPrintValues())
        if 0:
            onScreenDebug.add("w priorParent",
                self.priorParent.getLocalVector().pPrintValues())
        if 0:
            onScreenDebug.add("w priorParent po",
                self.priorParent.getVector(physObject).pPrintValues())

        if 1:
            onScreenDebug.add("w contact",
                self.actorNode.getContactVector().pPrintValues())

    def handleAvatarControls(self, task):
        """
        Check on the arrow keys and update the "avatar" (ship).
        """
        if __debug__:
            if self.wantDebugIndicator:
                onScreenDebug.append("localAvatar pos = %s\n"%(
                    base.localAvatar.getPos().pPrintValues(),))
                onScreenDebug.append("localAvatar hpr = %s\n"%(
                    base.localAvatar.getHpr().pPrintValues(),))
        assert self.debugPrint("handleAvatarControls(task=%s)"%(task,))
        physObject = self.actorNode.getPhysicsObject()
        contact = self.actorNode.getContactVector()
        # get the button states:
        forward = inputState.isSet("forward")
        reverse = inputState.isSet("reverse")
        turnLeft = inputState.isSet("slideLeft") or inputState.isSet("turnLeft")
        turnRight = inputState.isSet("slideRight") or inputState.isSet("turnRight")
        slide = inputState.isSet("slide")
        slideLeft = 0
        slideRight = 0
        jump = inputState.isSet("jump")
        # Determine what the speeds are based on the buttons:

        # Check for Auto-Sailing
        if self.ship.getIsAutoSailing():
            forward = 1
            reverse = 0
        else:
            forward = 0
            
        # How far did we move based on the amount of time elapsed?
        dt = ClockObject.getGlobalClock().getDt()
        
        if reverse or turnLeft or turnRight or not forward:
            # Reset Straight Sailing Bonus
            self.straightHeading = 0
        else:
            # Add in the Straight Sailing Time
            self.straightHeading += dt
            
        # Straight Sailing Acceleration Bonus
        straightSailBonus = 0.0
        #if self.straightHeading > self.STRAIGHT_SAIL_BONUS_TIME * 0.333:
        #    straightSailBonus = (self.straightHeading - (self.STRAIGHT_SAIL_BONUS_TIME * 0.333)) / self.STRAIGHT_SAIL_BONUS_TIME * 0.666
        if self.straightHeading > (self.STRAIGHT_SAIL_BONUS_TIME * 0.5):
            straightSailBonus = (self.straightHeading - (self.STRAIGHT_SAIL_BONUS_TIME * 0.5)) / self.STRAIGHT_SAIL_BONUS_TIME * 0.5
        straightSailBonus = min(self.MAX_STRAIGHT_SAIL_BONUS, straightSailBonus * self.MAX_STRAIGHT_SAIL_BONUS)
        straightSailBonus += 1.0
        
        self.__speed=(forward and self.ship.acceleration * straightSailBonus) or \
                      (reverse and -self.ship.reverseAcceleration)
            
        avatarSlideSpeed=self.ship.acceleration *0.5 * straightSailBonus
        #self.__slideSpeed=slide and (
        #        (turnLeft and -avatarSlideSpeed) or
        #        (turnRight and avatarSlideSpeed))
        self.__slideSpeed=(forward or reverse) and (
                (slideLeft and -avatarSlideSpeed) or
                (slideRight and avatarSlideSpeed))
        self.__rotationSpeed=not slide and (
                (turnLeft and self.ship.turnRate) or
                (turnRight and -self.ship.turnRate))
        
        # Add in Straight Sailing Multiplier
        self.__speed *= straightSailBonus
        self.__slideSpeed *= straightSailBonus
        maxSpeed = self.ship.maxSpeed * straightSailBonus
        
        # Enable debug turbo modec
        debugRunning = inputState.isSet("debugRunning")
        if(debugRunning):
            self.__speed*=base.debugRunningMultiplier
            self.__slideSpeed*=base.debugRunningMultiplier
            self.__rotationSpeed*=1.25
            maxSpeed = self.ship.maxSpeed * base.debugRunningMultiplier

        #*#
        self.currentTurning += self.__rotationSpeed
        if self.currentTurning > self.ship.maxTurn:
            self.currentTurning = self.ship.maxTurn
        elif self.currentTurning < -self.ship.maxTurn:
            self.currentTurning = -self.ship.maxTurn
        if turnLeft or turnRight:
            mult = .9
        elif forward or reverse:
            mult = .82
        else:
            mult = .8
        self.currentTurning *= mult
        if self.currentTurning < 0.001 and self.currentTurning > -0.001:
            self.currentTurning = 0.0
        self.__rotationSpeed = self.currentTurning

        if self.wantDebugIndicator:
            self.displayDebugInfo()

        if self.needToDeltaPos:
            self.setPriorParentVector()
            self.needToDeltaPos = 0

        #------------------------------
        #debugTempH=self.shipNodePath.getH()
        if __debug__:
            q1=self.shipNodePath.getQuat()
            q2=physObject.getOrientation()
            q1.normalize()
            q2.normalize()
            assert q1.isSameDirection(q2) or (q1.getHpr() == q2.getHpr())
        assert self.shipNodePath.getPos().almostEqual(
            physObject.getPosition(), 0.0001)
        #------------------------------

        # Check to see if we're moving at all:
        physVel = physObject.getVelocity()
        physVelLen = physVel.length()
        if (not physVel.almostEqual(Vec3(0),0.1)
            or self.__speed
            or self.__slideSpeed
            or self.__rotationSpeed):
            # don't factor in dt, the physics system will do that
            distance = self.__speed #dt * self.__speed
            goForward = True
            if (distance < 0):
                goForward = False
            slideDistance = self.__slideSpeed
            rotation = self.__rotationSpeed
            if debugRunning:
                rotation *= 4

            # update pos:
            # Take a step in the direction of our previous heading.
            self.__vel=Vec3(
                Vec3.forward() * distance +
                Vec3.right() * slideDistance)

            # rotMat is the rotation matrix corresponding to
            # our previous heading.
            rotMat=Mat3.rotateMatNormaxis(
                self.shipNodePath.getH(), Vec3.up())
            step=rotMat.xform(self.__vel)

            #newVector = self.acForce.getLocalVector()+Vec3(step)
            newVector = Vec3(step)
            #newVector=Vec3(rotMat.xform(newVector))
            #maxLen = maxSpeed
            if (goForward):
                maxLen = self.ship.acceleration * straightSailBonus
            else:
                maxLen = self.ship.reverseAcceleration

            if newVector.length() > maxLen and \
               not (debugRunning or base.localAvatar.getTurbo()):
                newVector.normalize()
                newVector *= maxLen
                
            if __debug__:
                onScreenDebug.add(
                    "newVector", newVector)
                onScreenDebug.add(
                    "newVector length", newVector.length())

            base.controlForce.setVector(newVector)

            assert base.controlForce.getLocalVector() == newVector,'1'
            assert base.controlForce.getPhysicsObject(),'2'
            assert base.controlForce.getPhysicsObject() == physObject,'3'

            #momentum = self.momentumForce.getLocalVector()
            #momentum *= 0.9
            #self.momentumForce.setVector(momentum)

            # update hpr:
            o=physObject.getOrientation()
            r=LRotationf()
            # factor in dt since we're directly setting the rotation here
            r.setHpr(Vec3(rotation * dt, 0.0, 0.0))
            physObject.setOrientation(o*r)

            # sync the change:
            self.actorNode.updateTransform()
            #assert self.shipNodePath.getH()==debugTempH-rotation
            messenger.send("avatarMoving")
        else:
            # even if there are no active inputs, we still might be moving
            assert physObject.getVelocity().almostEqual(Vec3(0),0.1)
            #base.controlForce.setVector(Vec3.zero())
            goForward = True


        #*#
        speed = physVel
        if (goForward):
            if physVelLen > maxSpeed:
                speed.normalize()
                speed *= maxSpeed
        else:
            if physVelLen > self.ship.maxReverseSpeed:
                speed.normalize()
                speed *= self.ship.maxReverseSpeed

        #speed *= 1.0 - dt * 0.05

        # modify based on sail damage
        speed *= self.ship.Sp
        speed /= self.ship.maxSp

        if __debug__:
            q1=self.shipNodePath.getQuat()
            q2=physObject.getOrientation()
            q1.normalize()
            q2.normalize()
            assert q1.isSameDirection(q2) or q1.getHpr() == q2.getHpr()
        assert self.shipNodePath.getPos().almostEqual(
            physObject.getPosition(), 0.0001)

        # Clear the contact vector so we can
        # tell if we contact something next frame
        self.actorNode.setContactVector(Vec3.zero())

        oldPosDelta = self.shipNodePath.getPosDelta(render)
        oldDt = dt
        assert hasattr(self.ship, 'worldVelocity')
        if oldDt:
            self.ship.worldVelocity = oldPosDelta*(1/oldDt)
        if self.wantDebugIndicator:
            onScreenDebug.add("w __oldPosDelta vec",
                oldPosDelta.pPrintValues())
            onScreenDebug.add("w __oldPosDelta len",
                "% 10.4f"%oldPosDelta.length())
            onScreenDebug.add("w __oldDt",
                "% 10.4f"%oldDt)
            onScreenDebug.add("w worldVelocity vec",
                self.ship.worldVelocity.pPrintValues())
            onScreenDebug.add("w worldVelocity len",
                "% 10.4f"%self.ship.worldVelocity.length())

        # if hasattr(self.ship, 'sailBillow'):
        #     self.ship.sailBillow = self.sailsDeployed

        if hasattr(self.ship, 'currentTurning'):
            self.ship.currentTurning = self.currentTurning

        return Task.cont

    def getVelocity(self):
        return self.__vel

    def enableAvatarControls(self):
        """
        Activate the arrow keys, etc.
        """
        assert self.debugPrint("enableShipControls()")

        self.setCollisionsActive(1)
        
        if __debug__:
            #self.accept("control-f3", self.spawnTest) #*#
            self.accept("f3", self.reset) # for debugging only.

        taskName = "ShipControls-%s"%(id(self),)
        # remove any old
        taskMgr.remove(taskName)
        # spawn the new task
        taskMgr.add(self.handleAvatarControls, taskName, 25)
        if self.physVelocityIndicator:
            taskMgr.add(
                self.avatarPhysicsIndicator,
                "ShipControlsIndicator%s"%(id(self),), 35)
            
    def disableAvatarControls(self):
        """
        Ignore the arrow keys, etc.
        """
        base.controlForce.setVector(Vec3(0))
            
        assert self.debugPrint("disableShipControls()")
        taskName = "ShipControls-%s"%(id(self),)
        taskMgr.remove(taskName)
        
        taskName = "ShipControlsIndicator%s"%(id(self),)
        taskMgr.remove(taskName)

        if self.ship:
            self.ship.worldVelocity = Vec3(0)
        
        if __debug__:
            self.ignore("control-f3") #*#
            self.ignore("f3")

    if __debug__:
        def setupAvatarPhysicsIndicator(self):
            if self.wantDebugIndicator:
                indicator=loader.loadModel('phase_5/models/props/dagger')
                #self.walkControls.setAvatarPhysicsIndicator(indicator)

        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(id(self))+' '+message)
