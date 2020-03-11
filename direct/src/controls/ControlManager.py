
from direct.showbase.InputStateGlobal import inputState
#from DirectGui import *
#from PythonUtil import *
#from IntervalGlobal import *

#from otp.avatar import Avatar
from direct.directnotify import DirectNotifyGlobal
#import GhostWalker
#import GravityWalker
#import NonPhysicsWalker
#import PhysicsWalker
#if __debug__:
#    import DevWalker
from direct.task import Task
from panda3d.core import ConfigVariableBool

# This is a hack, it may be better to use a line instead of a ray.
CollisionHandlerRayStart = 4000.0


class ControlManager:
    notify = DirectNotifyGlobal.directNotify.newCategory("ControlManager")
    wantWASD = ConfigVariableBool('want-WASD', False)

    def __init__(self, enable=True, passMessagesThrough = False):
        assert self.notify.debug("init control manager %s" % (passMessagesThrough))
        assert self.notify.debugCall(id(self))
        self.passMessagesThrough = passMessagesThrough
        self.inputStateTokens = []
        # Used to switch between strafe and turn. We will reset to whatever was last set.
        self.WASDTurnTokens = []
        self.__WASDTurn = True
        self.controls = {}
        self.currentControls = None
        self.currentControlsName = None
        self.isEnabled = 0
        if enable:
            self.enable()
        #self.monitorTask = taskMgr.add(self.monitor, "ControlManager-%s"%(id(self)), priority=-1)
        self.forceAvJumpToken = None



        if self.passMessagesThrough: # for not breaking toontown
            ist=self.inputStateTokens
            ist.append(inputState.watchWithModifiers("forward", "arrow_up", inputSource=inputState.ArrowKeys))
            ist.append(inputState.watchWithModifiers("reverse", "arrow_down", inputSource=inputState.ArrowKeys))
            ist.append(inputState.watchWithModifiers("turnLeft", "arrow_left", inputSource=inputState.ArrowKeys))
            ist.append(inputState.watchWithModifiers("turnRight", "arrow_right", inputSource=inputState.ArrowKeys))


    def __str__(self):
        return 'ControlManager: using \'%s\'' % self.currentControlsName

    def add(self, controls, name="basic"):
        """Add a control instance to the list of available control systems.

        Args:
            controls: an avatar control system.
            name (str): any key that you want to use to refer to the controls
                later (e.g. using the use(<name>) call).

        See also: :meth:`use()`.
        """
        assert self.notify.debugCall(id(self))
        assert controls is not None
        oldControls = self.controls.get(name)
        if oldControls is not None:
            assert self.notify.debug("Replacing controls: %s" % name)
            oldControls.disableAvatarControls()
            oldControls.setCollisionsActive(0)
            oldControls.delete()
        controls.disableAvatarControls()
        controls.setCollisionsActive(0)
        self.controls[name] = controls

    def get(self, name):
        return self.controls.get(name)

    def remove(self, name):
        """Remove a control instance from the list of available control
        systems.

        Args:
            name: any key that was used to refer to the controls when they were
                added (e.g. using the add(<controls>, <name>) call).

        See also: :meth:`add()`.
        """
        assert self.notify.debugCall(id(self))
        oldControls = self.controls.pop(name,None)
        if oldControls is not None:
            assert self.notify.debug("Removing controls: %s" % name)
            oldControls.disableAvatarControls()
            oldControls.setCollisionsActive(0)

    if __debug__:
        def lockControls(self):
            self.ignoreUse=True

        def unlockControls(self):
            if hasattr(self, "ignoreUse"):
                del self.ignoreUse

    def use(self, name, avatar):
        """
        name is a key (string) that was previously passed to add().

        Use a previously added control system.

        See also: :meth:`add()`.
        """
        assert self.notify.debugCall(id(self))
        if __debug__ and hasattr(self, "ignoreUse"):
            return
        controls = self.controls.get(name)

        if controls is not None:
            if controls is not self.currentControls:
                if self.currentControls is not None:
                    self.currentControls.disableAvatarControls()
                    self.currentControls.setCollisionsActive(0)
                    self.currentControls.setAvatar(None)
                self.currentControls = controls
                self.currentControlsName = name
                self.currentControls.setAvatar(avatar)
                self.currentControls.setCollisionsActive(1)
                if self.isEnabled:
                    self.currentControls.enableAvatarControls()
                messenger.send('use-%s-controls'%(name,), [avatar])
            #else:
            #    print "Controls are already", name
        else:
            assert self.notify.debug("Unkown controls: %s" % name)

    def setSpeeds(self, forwardSpeed, jumpForce,
            reverseSpeed, rotateSpeed, strafeLeft=0, strafeRight=0):
        assert self.notify.debugCall(id(self))
        for controls in self.controls.values():
            controls.setWalkSpeed(
                forwardSpeed, jumpForce, reverseSpeed, rotateSpeed)

    def delete(self):
        assert self.notify.debugCall(id(self))
        self.disable()
        for controls in self.controls.keys():
            self.remove(controls)
        del self.controls
        del self.currentControls

        for token in self.inputStateTokens:
            token.release()

        for token in self.WASDTurnTokens:
            token.release()
        self.WASDTurnTokens = []

        #self.monitorTask.remove()

    def getSpeeds(self):
        if self.currentControls:
            return self.currentControls.getSpeeds()
        return None

    def getIsAirborne(self):
        if self.currentControls:
            return self.currentControls.getIsAirborne()
        return False

    def setTag(self, key, value):
        assert self.notify.debugCall(id(self))
        for controls in self.controls.values():
            controls.setTag(key, value)

    def deleteCollisions(self):
        assert self.notify.debugCall(id(self))
        for controls in self.controls.values():
            controls.deleteCollisions()

    def collisionsOn(self):
        assert self.notify.debugCall(id(self))
        if self.currentControls:
            self.currentControls.setCollisionsActive(1)

    def collisionsOff(self):
        assert self.notify.debugCall(id(self))
        if self.currentControls:
            self.currentControls.setCollisionsActive(0)

    def placeOnFloor(self):
        assert self.notify.debugCall(id(self))
        if self.currentControls:
            self.currentControls.placeOnFloor()

    def enable(self):
        assert self.notify.debugCall(id(self))

        if self.isEnabled:
            assert self.notify.debug('already isEnabled')
            return

        self.isEnabled = 1

        # keep track of what we do on the inputState so we can undo it later on
        #self.inputStateTokens = []
        ist = self.inputStateTokens
        ist.append(inputState.watch("run", 'runningEvent', "running-on", "running-off"))

        ist.append(inputState.watchWithModifiers("forward", "arrow_up", inputSource=inputState.ArrowKeys))
        ist.append(inputState.watch("forward", "force-forward", "force-forward-stop"))

        ist.append(inputState.watchWithModifiers("reverse", "arrow_down", inputSource=inputState.ArrowKeys))
        ist.append(inputState.watchWithModifiers("reverse", "mouse4", inputSource=inputState.Mouse))

        if self.wantWASD:
            ist.append(inputState.watchWithModifiers("turnLeft", "arrow_left", inputSource=inputState.ArrowKeys))
            ist.append(inputState.watch("turnLeft", "mouse-look_left", "mouse-look_left-done"))
            ist.append(inputState.watch("turnLeft", "force-turnLeft", "force-turnLeft-stop"))

            ist.append(inputState.watchWithModifiers("turnRight", "arrow_right", inputSource=inputState.ArrowKeys))
            ist.append(inputState.watch("turnRight", "mouse-look_right", "mouse-look_right-done"))
            ist.append(inputState.watch("turnRight", "force-turnRight", "force-turnRight-stop"))

            ist.append(inputState.watchWithModifiers("forward", "w", inputSource=inputState.WASD))
            ist.append(inputState.watchWithModifiers("reverse", "s", inputSource=inputState.WASD))

            ist.append(inputState.watchWithModifiers("slideLeft", "q", inputSource=inputState.QE))
            ist.append(inputState.watchWithModifiers("slideRight", "e", inputSource=inputState.QE))

            self.setWASDTurn(self.__WASDTurn)
        else:
            ist.append(inputState.watchWithModifiers("turnLeft", "arrow_left", inputSource=inputState.ArrowKeys))
            ist.append(inputState.watch("turnLeft", "mouse-look_left", "mouse-look_left-done"))
            ist.append(inputState.watch("turnLeft", "force-turnLeft", "force-turnLeft-stop"))

            ist.append(inputState.watchWithModifiers("turnRight", "arrow_right", inputSource=inputState.ArrowKeys))
            ist.append(inputState.watch("turnRight", "mouse-look_right", "mouse-look_right-done"))
            ist.append(inputState.watch("turnRight", "force-turnRight", "force-turnRight-stop"))

        # Jump controls
        if self.wantWASD:
            ist.append(inputState.watchWithModifiers("jump", "space"))
        else:
            ist.append(inputState.watch("jump", "control", "control-up"))

        if self.currentControls:
            self.currentControls.enableAvatarControls()

    def disable(self):
        assert self.notify.debugCall(id(self))
        self.isEnabled = 0

        for token in self.inputStateTokens:
            token.release()
        self.inputStateTokens = []

        for token in self.WASDTurnTokens:
            token.release()
        self.WASDTurnTokens = []

        if self.currentControls:
            self.currentControls.disableAvatarControls()

        if self.passMessagesThrough: # for not breaking toontown
            ist=self.inputStateTokens
            ist.append(inputState.watchWithModifiers("forward", "arrow_up", inputSource=inputState.ArrowKeys))
            ist.append(inputState.watchWithModifiers("reverse", "arrow_down", inputSource=inputState.ArrowKeys))
            ist.append(inputState.watchWithModifiers("turnLeft", "arrow_left", inputSource=inputState.ArrowKeys))
            ist.append(inputState.watchWithModifiers("turnRight", "arrow_right", inputSource=inputState.ArrowKeys))

    def stop(self):
        self.disable()
        if self.currentControls:
            self.currentControls.setCollisionsActive(0)
            self.currentControls.setAvatar(None)
        self.currentControls = None

    def disableAvatarJump(self):
        """
        prevent
        """
        assert self.forceAvJumpToken is None
        self.forceAvJumpToken=inputState.force(
            "jump", 0, 'ControlManager.disableAvatarJump')

    def enableAvatarJump(self):
        """
        Stop forcing the ctrl key to return 0's
        """
        assert self.forceAvJumpToken is not None
        self.forceAvJumpToken.release()
        self.forceAvJumpToken = None

    def monitor(self, foo):
        #assert self.debugPrint("monitor()")
        #if 1:
        #    airborneHeight=self.avatar.getAirborneHeight()
        #    onScreenDebug.add("airborneHeight", "% 10.4f"%(airborneHeight,))
        if 0:
            onScreenDebug.add("InputState forward", "%d"%(inputState.isSet("forward")))
            onScreenDebug.add("InputState reverse", "%d"%(inputState.isSet("reverse")))
            onScreenDebug.add("InputState turnLeft", "%d"%(inputState.isSet("turnLeft")))
            onScreenDebug.add("InputState turnRight", "%d"%(inputState.isSet("turnRight")))
            onScreenDebug.add("InputState slideLeft", "%d"%(inputState.isSet("slideLeft")))
            onScreenDebug.add("InputState slideRight", "%d"%(inputState.isSet("slideRight")))
        return Task.cont

    def setWASDTurn(self, turn):
        self.__WASDTurn = turn

        if not self.isEnabled:
            return

        turnLeftWASDSet = inputState.isSet("turnLeft", inputSource=inputState.WASD)
        turnRightWASDSet = inputState.isSet("turnRight", inputSource=inputState.WASD)
        slideLeftWASDSet = inputState.isSet("slideLeft", inputSource=inputState.WASD)
        slideRightWASDSet = inputState.isSet("slideRight", inputSource=inputState.WASD)

        for token in self.WASDTurnTokens:
            token.release()

        if turn:
            self.WASDTurnTokens = (
                inputState.watchWithModifiers("turnLeft", "a", inputSource=inputState.WASD),
                inputState.watchWithModifiers("turnRight", "d", inputSource=inputState.WASD),
                )

            inputState.set("turnLeft", slideLeftWASDSet, inputSource=inputState.WASD)
            inputState.set("turnRight", slideRightWASDSet, inputSource=inputState.WASD)

            inputState.set("slideLeft", False, inputSource=inputState.WASD)
            inputState.set("slideRight", False, inputSource=inputState.WASD)

        else:
            self.WASDTurnTokens = (
                inputState.watchWithModifiers("slideLeft", "a", inputSource=inputState.WASD),
                inputState.watchWithModifiers("slideRight", "d", inputSource=inputState.WASD),
                )

            inputState.set("slideLeft", turnLeftWASDSet, inputSource=inputState.WASD)
            inputState.set("slideRight", turnRightWASDSet, inputSource=inputState.WASD)

            inputState.set("turnLeft", False, inputSource=inputState.WASD)
            inputState.set("turnRight", False, inputSource=inputState.WASD)

