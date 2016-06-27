
from direct.showbase.InputStateGlobal import inputState
from direct.task.Task import Task
from panda3d.core import *
from . import GravityWalker

BattleStrafe = 0

def ToggleStrafe():
    global BattleStrafe
    BattleStrafe = not BattleStrafe

def SetStrafe(status):
    global BattleStrafe
    BattleStrafe = status

class BattleWalker(GravityWalker.GravityWalker):
    def __init__(self):
        GravityWalker.GravityWalker.__init__(self)
        self.slideSpeed = 0
        self.advanceSpeed = 0

    def getSpeeds(self):
        return (self.speed, self.rotationSpeed, self.slideSpeed, self.advanceSpeed)

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
        if base.localAvatar.getAutoRun():
            forward = 1
            reverse = 0

        # Determine what the speeds are based on the buttons:
        self.speed=(forward and self.avatarControlForwardSpeed or
                    reverse and -self.avatarControlReverseSpeed)
        # Slide speed is a scaled down version of forward speed
        self.slideSpeed=(slideLeft and -self.avatarControlForwardSpeed or
                         slideRight and self.avatarControlForwardSpeed) * 0.5
        self.rotationSpeed=not (slideLeft or slideRight) and (
                (turnLeft and self.avatarControlRotateSpeed) or
                (turnRight and -self.avatarControlRotateSpeed))

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

    if 0:
        def handleAvatarControls(self, task):
            # If targetNp is not available, revert back to GravityWalker.handleAvatarControls.
            # This situation occurs when the target dies, but we aren't switched out of
            # battle walker control mode.

            targetNp = self.avatarNodePath.currentTarget
            if not BattleStrafe or targetNp == None or targetNp.isEmpty():
                return GravityWalker.GravityWalker.handleAvatarControls(self, task)

            # get the button states:
            run = inputState.isSet("run")
            forward = inputState.isSet("forward")
            reverse = inputState.isSet("reverse")
            turnLeft = inputState.isSet("turnLeft")
            turnRight = inputState.isSet("turnRight")
            slide = inputState.isSet("slide")
            jump = inputState.isSet("jump")
            # Determine what the speeds are based on the buttons:
            self.advanceSpeed=(forward and self.avatarControlForwardSpeed or
                               reverse and -self.avatarControlReverseSpeed)
            if run and self.advanceSpeed>0.0:
                self.advanceSpeed*=2.0 #*#
            # Should fSlide be renamed slideButton?
            self.slideSpeed=.15*(turnLeft and -self.avatarControlForwardSpeed or
                                 turnRight and self.avatarControlForwardSpeed)
            print('slideSpeed: %s' % self.slideSpeed)
            self.rotationSpeed=0
            self.speed=0

            debugRunning = inputState.isSet("debugRunning")
            if debugRunning:
                self.advanceSpeed*=4.0
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

            # Before we do anything with position or orientation, make the avatar
            # face it's target.  Only allow rMax degrees rotation per frame, so
            # we don't get an unnatural spinning effect
            curH = self.avatarNodePath.getH()
            self.avatarNodePath.headsUp(targetNp)
            newH = self.avatarNodePath.getH()
            delH = reduceAngle(newH-curH)
            rMax = 10
            if delH < -rMax:
                self.avatarNodePath.setH(curH-rMax)
                self.rotationSpeed=-self.avatarControlRotateSpeed
            elif delH > rMax:
                self.avatarNodePath.setH(curH+rMax)
                self.rotationSpeed=self.avatarControlRotateSpeed

            # Check to see if we're moving at all:
            self.moving = self.speed or self.slideSpeed or self.rotationSpeed or (self.priorParent!=Vec3.zero())
            if self.moving:
                distance = dt * self.speed
                slideDistance = dt * self.slideSpeed
                print('slideDistance: %s' % slideDistance)
                rotation = dt * self.rotationSpeed

                # Take a step in the direction of our previous heading.
                self.vel=Vec3(Vec3.forward() * distance +
                              Vec3.right() * slideDistance)
                if self.vel != Vec3.zero() or self.priorParent != Vec3.zero():
                    if 1:
                        # rotMat is the rotation matrix corresponding to
                        # our previous heading.
                        rotMat=Mat3.rotateMatNormaxis(self.avatarNodePath.getH(), Vec3.up())
                        step=(self.priorParent * dt) + rotMat.xform(self.vel)
                        self.avatarNodePath.setFluidPos(Point3(
                                self.avatarNodePath.getPos()+step))
                self.avatarNodePath.setH(self.avatarNodePath.getH()+rotation)
            else:
                self.vel.set(0.0, 0.0, 0.0)

            """
            # Check to see if we're moving at all:
            self.moving = self.advanceSpeed or self.slideSpeed or self.rotationSpeed or (self.priorParent!=Vec3.zero())
            if self.moving:
                distance = dt * self.advanceSpeed
                slideDistance = dt * self.slideSpeed
                rotation = dt * self.rotationSpeed

                # Prevent avatar from getting too close to target
                d = self.avatarNodePath.getPos(targetNp)
                # TODO:  make min distance adjust for current weapon
                if (d[0]*d[0]+d[1]*d[1] < 6.0 and distance > 0):
                    # move the avatar sideways instead of forward
                    slideDistance += .2
                    distance = 0

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
            """
            if self.moving or jump:
                messenger.send("avatarMoving")
            return Task.cont


