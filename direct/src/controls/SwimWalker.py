from direct.showbase.InputStateGlobal import inputState
from direct.directnotify import DirectNotifyGlobal
from direct.controls import NonPhysicsWalker

class SwimWalker(NonPhysicsWalker.NonPhysicsWalker):
    notify = DirectNotifyGlobal.directNotify.newCategory("SwimWalker")

    def _calcSpeeds(self):
        # get the button states:
        forward = inputState.isSet("forward")
        reverse = inputState.isSet("reverse")
        turnLeft = inputState.isSet("turnLeft") or inputState.isSet("slideLeft")
        turnRight = inputState.isSet("turnRight") or inputState.isSet("slideRight")

        # Check for Auto-Run
        if base.localAvatar.getAutoRun():
            forward = 1
            reverse = 0

        # Determine what the speeds are based on the buttons:
        self.speed=(forward and self.avatarControlForwardSpeed or
                    reverse and -self.avatarControlReverseSpeed)
        self.slideSpeed=0.
        self.rotationSpeed=(
            (turnLeft and self.avatarControlRotateSpeed) or
            (turnRight and -self.avatarControlRotateSpeed))
