
from direct.showbase.ShowBaseGlobal import *
import GravityWalker

class BattleWalker(GravityWalker.GravityWalker):
    def __init__(self):
        GravityWalker.GravityWalker.__init__(self)
        self.targetNp = None
        
    def setTarget(self, nodepath):
        # Set target that movement will be relative to
        self.targetNp = nodepath

    def getSpeeds(self):
        #assert(self.debugPrint("getSpeeds()"))
        return (self.speed+self.slideSpeed, self.rotationSpeed)


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
        if run and self.speed>0.0:
            self.speed*=2.0 #*#
        # Should fSlide be renamed slideButton?
        self.slideSpeed=(turnLeft and -self.avatarControlForwardSpeed or 
                         turnRight and self.avatarControlForwardSpeed)
        self.rotationSpeed=0
        
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

            # Before we do anything with position or orientation, make the avatar
            # face it's target
            self.avatarNodePath.headsUp(self.targetNp)

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
    
    
