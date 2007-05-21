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

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.controls.ControlManager import CollisionHandlerRayStart
from direct.showbase.InputStateGlobal import inputState
from direct.task.Task import Task
from pandac.PandaModules import *
import math

from PhysicsWalker import PhysicsWalker

class ShipPilot(PhysicsWalker):
    notify = directNotify.newCategory("ShipPilot")
    wantDebugIndicator = base.config.GetBool(
        'want-avatar-physics-indicator', 0)

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

    def setWalkSpeed(self, forward, jump, reverse, rotate):
        assert self.debugPrint("setWalkSpeed()")
        PhysicsWalker.setWalkSpeed(self, forward, 0, reverse, rotate)

    def setWallBitMask(self, bitMask):
        self.wallBitmask = bitMask

    def swapWallBitMask(self, oldMask, newMask):
        self.wallBitmask = self.wallBitmask &~ oldMask
        self.wallBitmask |= newMask

        if self.cSphereNodePath and not self.cSphereNodePath.isEmpty():
            self.cSphereNodePath.node().setFromCollideMask(self.wallBitmask)
        
    def setFloorBitMask(self, bitMask):
        self.floorBitmask = bitMask

    def setShip(self, ship):
        self.setAvatar(ship)
        
    def setAvatar(self, ship):
        if ship is None:
            base.controlForce.clearPhysicsObject()
            self.takedownPhysics()
            self.ship = None
            self.setCollisionsActive(0)
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
        self.pusher = PhysicsCollisionHandler()
        self.pusher.setInPattern("enter%in")
        self.pusher.setOutPattern("exit%in")

        cSphereNode = CollisionNode('SP.cSphereNode')

        # Front sphere:
        sRadius = abs((self.portPos - self.starboardPos)[0] / 2.0)
        cBowSphere = CollisionSphere(
            0.0, self.bowPos[1]+sRadius, 0.0, sRadius)
        cSphereNode.addSolid(cBowSphere)
        # cBowSphereNode = CollisionNode('SP.cBowSphereNode')
        # cBowSphereNode.addSolid(cBowSphere)
        # self.cBowSphereNodePath = self.ship.getInteractCollisionRoot().attachNewNode(
        #     cBowSphereNode)
        
        # self.pusher.addCollider(
        #     self.cBowSphereNodePath, self.shipNodePath)
        
        # Back sphere:
        cSternSphere = CollisionSphere(
            0.0, self.sternPos[1]-sRadius, 0.0, sRadius)
        cSternSphereNode = CollisionNode('SP.cSternSphereNode')
        cSphereNode.addSolid(cSternSphere)
        # cSternSphereNode.addSolid(cSternSphere)
        # self.cSternSphereNodePath = self.ship.getInteractCollisionRoot().attachNewNode(
        #     cSternSphereNode)

        # self.pusher.addCollider(
        #     self.cSternSphereNodePath, self.shipNodePath)

        
        self.cSphereNodePath = self.ship.getInteractCollisionRoot().attachNewNode(
            cSphereNode)
        self.pusher.addCollider(
            self.cSphereNodePath, self.shipNodePath)
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
                # self.cBowSphereNodePath.node().setFromCollideMask(self.wallBitmask | self.floorBitmask)
                # self.cBowSphereNodePath.node().setIntoCollideMask(BitMask32.allOff())
                # self.cSternSphereNodePath.node().setFromCollideMask(self.wallBitmask | self.floorBitmask)
                # self.cSternSphereNodePath.node().setIntoCollideMask(BitMask32.allOff())
                self.cSphereNodePath.node().setFromCollideMask(self.wallBitmask | self.floorBitmask)
                self.cSphereNodePath.node().setIntoCollideMask(BitMask32.allOff())
                # self.cTrav.addCollider(self.cBowSphereNodePath, self.pusher)
                # self.cTrav.addCollider(self.cSternSphereNodePath, self.pusher)
                self.cTrav.addCollider(self.cSphereNodePath, self.pusher)
                shipCollWall.stash()
            else:
                # self.cTrav.removeCollider(self.cBowSphereNodePath)
                # self.cTrav.removeCollider(self.cSternSphereNodePath)
                self.cTrav.removeCollider(self.cSphereNodePath)
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
        if hasattr(self, "cSphereNodePath"):
            self.cSphereNodePath.removeNode()
            del self.cSphereNodePath

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
        # self.cSternSphereNodePath.setTag(key, value)
        # self.cBowSphereNodePath.setTag(key, value)
        self.cSphereNodePath.setTag(key, value)

    def setAvatarPhysicsIndicator(self, indicator):
        """
        indicator is a NodePath
        """
        assert self.debugPrint("setAvatarPhysicsIndicator()")
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
        # assert self.debugPrint("handleAvatarControls(task=%s)"%(task,))
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

        # How far did we move based on the amount of time elapsed?
        dt = ClockObject.getGlobalClock().getDt()
        
        # this was causing the boat to get stuck moving forward or back
        if 0:
            if not hasattr(self, "sailsDeployed"):
                self.sailsDeployed = 0.0
            if forward and reverse:
                # Way anchor:
                self.__speed = 0.0
                physObject.setVelocity(Vec3.zero())
            elif forward:
                self.sailsDeployed += 0.25
                if self.sailsDeployed > 1.0:
                    self.sailsDeployed = 1.0
            elif reverse:
                self.sailsDeployed -= 0.25
                if self.sailsDeployed < -1.0:
                    self.sailsDeployed = -1.0
            self.__speed = self.ship.acceleration * self.sailsDeployed
        else:
            self.__speed=(forward and self.ship.acceleration) or \
                (reverse and -self.ship.reverseAcceleration)
            #self.__speed=(forward and min(dt*(self.__speed + self.ship.acceleration), self.ship.maxSpeed) or
            #        reverse and min(dt*(self.__speed - self.ship.reverseAcceleration), self.ship.maxReverseSpeed))
            
        avatarSlideSpeed=self.ship.acceleration*0.5
        #self.__slideSpeed=slide and (
        #        (turnLeft and -avatarSlideSpeed) or
        #        (turnRight and avatarSlideSpeed))
        self.__slideSpeed=(forward or reverse) and (
                (slideLeft and -avatarSlideSpeed) or
                (slideRight and avatarSlideSpeed))
        self.__rotationSpeed=not slide and (
                (turnLeft and self.ship.turnRate) or
                (turnRight and -self.ship.turnRate))
        
        # Enable debug turbo mode
        maxSpeed = self.ship.maxSpeed
        debugRunning = inputState.isSet("debugRunning")
        if debugRunning or base.localAvatar.getTurbo():
            self.__speed*=4.0
            self.__slideSpeed*=4.0
            self.__rotationSpeed*=1.25
            maxSpeed = self.ship.maxSpeed * 4.0
            
        self.__speed*=4.0
        self.__slideSpeed*=4.0
        self.__rotationSpeed*=1.25
        maxSpeed = self.ship.maxSpeed * 4.0
                
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
        if (physVelLen!=0.
                or self.__speed
                or self.__slideSpeed
                or self.__rotationSpeed):
            distance = dt * self.__speed
            goForward = True
            if (distance < 0):
                goForward = False
            slideDistance = dt * self.__slideSpeed
            rotation = dt * self.__rotationSpeed

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
                maxLen = self.ship.acceleration
            else:
                maxLen = self.ship.reverseAcceleration
            if newVector.length() > maxLen:
                newVector.normalize()
                newVector *= maxLen


            if __debug__:
                onScreenDebug.add(
                    "newVector", newVector)
                onScreenDebug.add(
                    "newVector length", newVector.length())
            base.controlForce.setVector(newVector)
            assert base.controlForce.getLocalVector() == newVector
            assert base.controlForce.getPhysicsObject()
            assert base.controlForce.getPhysicsObject() == physObject

            #momentum = self.momentumForce.getLocalVector()
            #momentum *= 0.9
            #self.momentumForce.setVector(momentum)

            # update hpr:
            o=physObject.getOrientation()
            r=LRotationf()
            r.setHpr(Vec3(rotation, 0.0, 0.0))
            physObject.setOrientation(o*r)

            # sync the change:
            self.actorNode.updateTransform()
            #assert self.shipNodePath.getH()==debugTempH-rotation
            messenger.send("avatarMoving")
        else:
            # even if there are no active inputs, we still might be moving
            assert physObject.getVelocity().length() == 0.
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
        ## physObject.setVelocity(speed)

        # print self.ship.getZ()
        # self.ship.setZ(0)
        
        #rotMat=Mat3.rotateMatNormaxis(self.shipNodePath.getH(), Vec3.up())
        #speed=rotMat.xform(speed)
        # The momentumForce makes it feel like we are sliding on ice -- Joe
        # f = Vec3(self.__vel)
        # f.normalize()
        # self.momentumForce.setVector(Vec3(f*(speed.length()*0.9)))


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
        assert self.debugPrint("enableAvatarControls()")
        assert self.collisionsActive

        if __debug__:
            #self.accept("control-f3", self.spawnTest) #*#
            self.accept("f3", self.reset) # for debugging only.

        taskName = "AvatarControls-%s"%(id(self),)
        # remove any old
        taskMgr.remove(taskName)
        # spawn the new task
        taskMgr.add(self.handleAvatarControls, taskName, 25)
        if self.physVelocityIndicator:
            taskMgr.add(
                self.avatarPhysicsIndicator,
                "AvatarControlsIndicator%s"%(id(self),), 35)
            
    def disableAvatarControls(self):
        """
        Ignore the arrow keys, etc.
        """
        assert self.debugPrint("disableAvatarControls()")
        taskName = "AvatarControls-%s"%(id(self),)
        taskMgr.remove(taskName)

        taskName = "AvatarControlsIndicator%s"%(id(self),)
        taskMgr.remove(taskName)

        if __debug__:
            self.ignore("control-f3") #*#
            self.ignore("f3")

    if __debug__:
        def setupAvatarPhysicsIndicator(self):
            if self.wantDebugIndicator:
                indicator=loader.loadModelCopy('phase_5/models/props/dagger')
                #self.walkControls.setAvatarPhysicsIndicator(indicator)

        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(id(self))+' '+message)
