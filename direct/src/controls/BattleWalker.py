
from direct.showbase.ShowBaseGlobal import *
import GravityWalker

BattleStrafe = 1

def ToggleStrafe():
    global BattleStrafe
    BattleStrafe = not BattleStrafe

class BattleWalker(GravityWalker.GravityWalker):
    def __init__(self):
        GravityWalker.GravityWalker.__init__(self)

    def getSpeeds(self):
        #assert(self.debugPrint("getSpeeds()"))
        return (self.speed, self.rotationSpeed, self.slideSpeed)


    def handleAvatarControls(self, task):
        """
        Check on the arrow keys and update the avatar.
        """

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
        slide = 0 #hack -- was: inputState.isSet("slide")
        jump = inputState.isSet("jump")
        # Determine what the speeds are based on the buttons:
        self.speed=(forward and self.avatarControlForwardSpeed or 
                    reverse and -self.avatarControlReverseSpeed)
        if run and self.speed>0.0:
            self.speed*=2.0 #*#
        # Should fSlide be renamed slideButton?
        self.slideSpeed=.15*(turnLeft and -self.avatarControlForwardSpeed or 
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

        # Before we do anything with position or orientation, make the avatar
        # face it's target.  Only allow rMax degrees rotation per frame, so
        # we don't get an unnatural spinning effect
        oldH = self.avatarNodePath.getH()
        self.avatarNodePath.headsUp(targetNp)
        newH = self.avatarNodePath.getH()
        delH = newH-oldH
        rMax = 10
        if delH > rMax:
            self.avatarNodePath.setH(oldH+rMax)
            self.rotationSpeed=self.avatarControlRotateSpeed

        elif delH < -rMax:
            self.avatarNodePath.setH(oldH-rMax)
            self.rotationSpeed=-self.avatarControlRotateSpeed

        # Check to see if we're moving at all:
        self.moving = self.speed or self.slideSpeed or self.rotationSpeed or (self.priorParent!=Vec3.zero())
        if self.moving:
            distance = dt * self.speed
            slideDistance = dt * self.slideSpeed
            rotation = dt * self.rotationSpeed

            # Prevent avatar from getting too close to target
            d = self.avatarNodePath.getPos(targetNp)
            # TODO:  make min distance adjust for current weapon
            if (d[0]*d[0]+d[1]*d[1] < 6.0):
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
        if self.moving or jump:
            messenger.send("avatarMoving")
        return Task.cont
    
    
