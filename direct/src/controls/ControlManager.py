
from direct.showbase.ShowBaseGlobal import *
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


class ControlManager:
    notify = DirectNotifyGlobal.directNotify.newCategory("ControlManager")
    wantAvatarPhysicsIndicator = base.config.GetBool('want-avatar-physics-indicator', 0)
    wantAvatarPhysicsDebug = base.config.GetBool('want-avatar-physics-debug', 0)
    wantWASD = base.config.GetBool('want-WASD', 0)

    def __init__(self):
        assert self.notify.debugCall(id(self))
        self.enableJumpCounter = 1
        self.controls = {}
        self.currentControls = None
        self.isEnabled = 1
        #self.monitorTask = taskMgr.add(self.monitor, "ControlManager-%s"%(id(self)), priority=-1)
        inputState.watch("run", "running-on", "running-off")
        
        inputState.watchWithModifiers("forward", "arrow_up")
        inputState.watch("forward", "force-forward", "force-forward-stop")
        
        inputState.watchWithModifiers("reverse", "arrow_down")
        inputState.watchWithModifiers("reverse", "mouse4")

        inputState.watchWithModifiers("turnLeft", "arrow_left")
        inputState.watch("turnLeft", "mouse-look_left", "mouse-look_left-done")
        inputState.watch("turnLeft", "force-turnLeft", "force-turnLeft-stop")

        inputState.watchWithModifiers("turnRight", "arrow_right")
        inputState.watch("turnRight", "mouse-look_right", "mouse-look_right-done")
        inputState.watch("turnRight", "force-turnRight", "force-turnRight-stop")

        if self.wantWASD:
            inputState.watchWithModifiers("forward", "w")
            inputState.watchWithModifiers("reverse", "s")
            inputState.watchWithModifiers("slideLeft", "a")
            inputState.watchWithModifiers("slideRight", "d")
            inputState.watchWithModifiers("turnLeft", "q")
            inputState.watchWithModifiers("turnRight", "e")

        # Jump controls
        if self.wantWASD:
            inputState.watchWithModifiers("jump", "space")
        else:
            inputState.watch("jump", "control", "control-up")



    def add(self, controls, name="basic"):
        """
        controls is an avatar control system.
        name is any key that you want to use to refer to the
            the controls later (e.g. using the use(<name>) call).
        
        Add a control instance to the list of available control systems.
        
        See also: use().
        """
        assert self.notify.debugCall(id(self))
        assert controls is not None
        oldControls = self.controls.get(name)
        if oldControls is not None:
            print "Replacing controls:", name
            oldControls.disableAvatarControls()
            oldControls.setCollisionsActive(0)
            oldControls.delete()
        controls.disableAvatarControls()
        controls.setCollisionsActive(0)
        self.controls[name] = controls

    def remove(self, name):
        """
        name is any key that was used to refer to the
            the controls when they were added (e.g. 
            using the add(<controls>, <name>) call).
        
        Remove a control instance from the list of available control systems.
        
        See also: add().
        """
        assert self.notify.debugCall(id(self))
        oldControls = self.controls.get(name)
        if oldControls is not None:
            print "Removing controls:", name
            oldControls.disableAvatarControls()
            oldControls.setCollisionsActive(0)
            oldControls.delete()
            del self.controls[name]

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
        
        See also: add().
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
                self.currentControls.setAvatar(avatar)
                self.currentControls.setCollisionsActive(1)
                if self.isEnabled:
                    self.currentControls.enableAvatarControls()
                messenger.send('use-%s-controls'%(name,), [avatar])
            #else:
            #    print "Controls are already", name
        else:
            print "Unkown controls:", name

    def setSpeeds(self, forwardSpeed, jumpForce,
            reverseSpeed, rotateSpeed):
        assert self.notify.debugCall(id(self))
        for controls in self.controls.values():
            controls.setWalkSpeed(
                forwardSpeed, jumpForce, reverseSpeed, rotateSpeed)

    def delete(self):
        assert self.notify.debugCall(id(self))
        self.disable()
        del self.controls
        del self.currentControls

        inputState.ignore("forward")
        inputState.ignore("reverse")
        inputState.ignore("turnLeft")
        inputState.ignore("turnRight")
        inputState.ignore("jump")
        inputState.ignore("run")

        if self.wantWASD:
            inputState.ignore("slideLeft")
            inputState.ignore("slideRight")
        
        #self.monitorTask.remove()
    
    def getSpeeds(self):
        return self.currentControls.getSpeeds()

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
        self.currentControls.setCollisionsActive(1)

    def collisionsOff(self):
        assert self.notify.debugCall(id(self))
        self.currentControls.setCollisionsActive(0)

    def placeOnFloor(self):
        assert self.notify.debugCall(id(self))
        self.currentControls.placeOnFloor()

    def enable(self):
        assert self.notify.debugCall(id(self))
        self.isEnabled = 1
        self.currentControls.enableAvatarControls()
    
    def disable(self):
        assert self.notify.debugCall(id(self))
        self.isEnabled = 0
        self.currentControls.disableAvatarControls()

    def enableAvatarJump(self):
        """
        Stop forcing the ctrl key to return 0's
        """
        assert self.notify.debugCall(id(self))
        self.enableJumpCounter+=1
        if self.enableJumpCounter:
            assert self.enableJumpCounter == 1
            self.enableJumpCounter = 1
            inputState.unforce("jump")

    def disableAvatarJump(self):
        """
        Force the ctrl key to return 0's
        """
        assert self.notify.debugCall(id(self))
        self.enableJumpCounter-=1
        if self.enableJumpCounter <= 0:
            inputState.force("jump", 0)
    
    def monitor(self, foo):
        #assert(self.debugPrint("monitor()"))
        #if 1:
        #    airborneHeight=self.avatar.getAirborneHeight()
        #    onScreenDebug.add("airborneHeight", "% 10.4f"%(airborneHeight,))
        if 0:
            onScreenDebug.add("InputState forward", "%d"%(inputState.isSet("forward")))
            onScreenDebug.add("InputState reverse", "%d"%(inputState.isSet("reverse")))
            onScreenDebug.add("InputState turnLeft", "%d"%(inputState.isSet("turnLeft")))
            onScreenDebug.add("InputState turnRight", "%d"%(inputState.isSet("turnRight")))
        return Task.cont
