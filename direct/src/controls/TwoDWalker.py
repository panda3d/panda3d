"""
TwoDWalker.py is for controling the avatars in a 2D Scroller game environment.
"""

from GravityWalker import *

class TwoDWalker(GravityWalker):
    """
    The TwoDWalker is primarily for a 2D Scroller game environment. Eg - Toon Blitz minigame.
    TODO: This class is still work in progress. 
    Currently Toon Blitz is using this only for jumping.
    Moving the Toon left to right is handled by toontown/src/minigame/TwoDDrive.py.
    I eventually want this class to control all the 2 D movements, possibly with a
    customizable input list.
    """
    notify = directNotify.newCategory("TwoDWalker")
    wantDebugIndicator = base.config.GetBool('want-avatar-physics-indicator', 0)
    wantFloorSphere = base.config.GetBool('want-floor-sphere', 0)
    earlyEventSphere = base.config.GetBool('early-event-sphere', 0)

    # special methods
    def __init__(self, gravity = -32.1740, standableGround=0.707,
            hardLandingForce=16.0):
        assert self.notify.debugStateCall(self)
        self.notify.debug('Constructing TwoDWalker')
        GravityWalker.__init__(self)
    
    def handleAvatarControls(self, task):
        """
        Check on the arrow keys and update the avatar.
        """                
        # get the button states:
        jump = inputState.isSet("forward")            
        if self.lifter.isOnGround():
            if self.isAirborne:
                self.isAirborne = 0
                assert self.debugPrint("isAirborne 0 due to isOnGround() true")
                impact = self.lifter.getImpactVelocity()
                messenger.send("jumpLand")
            assert self.isAirborne == 0
            self.priorParent = Vec3.zero()
        else:
            if self.isAirborne == 0:
                assert self.debugPrint("isAirborne 1 due to isOnGround() false")
            self.isAirborne = 1
            
        return Task.cont
    
    def jumpPressed(self):
        """This function should be called from TwoDDrive when the jump key is pressed."""
        if self.lifter.isOnGround():
            if self.isAirborne == 0:
                if self.mayJump:
                    # The jump button is down and we're close enough to the ground to jump.
                    self.lifter.addVelocity(self.avatarControlJumpForce)
                    messenger.send("jumpStart")
                    self.isAirborne = 1
                    assert self.debugPrint("isAirborne 1 due to jump")