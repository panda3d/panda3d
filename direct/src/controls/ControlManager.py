
from direct.showbase.ShowBaseGlobal import *
#from DirectGui import *
#from PythonUtil import *
#from IntervalGlobal import *

from otp.avatar import Avatar
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

    def __init__(self):
        assert self.notify.debugCall(id(self))
        self.enableJumpCounter = 1
        self.controls = {}
        self.currentControls = None
        self.isEnabled = 1
        #self.monitorTask = taskMgr.add(self.monitor, "ControlManager-%s"%(id(self)), priority=-1)
        inputState.watch("forward", "arrow_up", "arrow_up-up")
        inputState.watch("forward", "control-arrow_up", "control-arrow_up-up")
        inputState.watch("forward", "alt-arrow_up", "alt-arrow_up-up")
        inputState.watch("forward", "shift-arrow_up", "shift-arrow_up-up")
        
        inputState.watch("reverse", "arrow_down", "arrow_down-up")
        inputState.watch("reverse", "control-arrow_down", "control-arrow_down-up")
        inputState.watch("reverse", "alt-arrow_down", "alt-arrow_down-up")
        inputState.watch("reverse", "shift-arrow_down", "shift-arrow_down-up")
        
        inputState.watch("turnLeft", "arrow_left", "arrow_left-up")
        inputState.watch("turnLeft", "control-arrow_left", "control-arrow_left-up")
        inputState.watch("turnLeft", "alt-arrow_left", "alt-arrow_left-up")
        inputState.watch("turnLeft", "shift-arrow_left", "shift-arrow_left-up")
        inputState.watch("turnLeft", "mouse-look_left", "mouse-look_left-done")
        
        inputState.watch("turnRight", "arrow_right", "arrow_right-up")
        inputState.watch("turnRight", "control-arrow_right", "control-arrow_right-up")
        inputState.watch("turnRight", "alt-arrow_right", "alt-arrow_right-up")
        inputState.watch("turnRight", "shift-arrow_right", "shift-arrow_right-up")
        inputState.watch("turnRight", "mouse-look_right", "mouse-look_right-done")
        
        inputState.watch("jump", "control", "control-up")
        inputState.watch("jump", "alt-control", "alt-control-up")
        inputState.watch("jump", "shift-control", "shift-control-up")
        
        inputState.watch("slideLeft", "home", "home-up")
        inputState.watch("slideRight", "end", "end-up")
        inputState.watch("levitateUp", "page_up", "page_up-up")
        inputState.watch("levitateDown", "page_down", "page_down-up")
        inputState.watch("run", "shift", "shift-up")
        
        # FYI, ghost mode uses jump for slide.
        #inputState.watch("slide", "slide-is-disabled", "slide-is-disabled")
        inputState.watch("slide", "mouse3", "mouse3-up")
        
        #inputState.watch("slideLeft", "shift-arrow_left", "shift-arrow_left-up")
        #inputState.watch("slideLeft", "control-arrow_left", "control-arrow_left-up")
        #inputState.watch("slideLeft", "alt-arrow_left", "alt-arrow_left-up")
        #inputState.watch("slideLeft", "shift-arrow_left", "shift-arrow_left-up")
        #inputState.watch("slideLeft", "slide-is-disabled", "slide-is-disabled")
        
        #inputState.watch("slideRight", "shift-arrow_right", "shift-arrow_right-up")
        #inputState.watch("slideRight", "control-arrow_right", "control-arrow_right-up")
        #inputState.watch("slideRight", "alt-arrow_right", "alt-arrow_right-up")
        #inputState.watch("slideRight", "shift-arrow_right", "shift-arrow_right-up")
        #inputState.watch("slideRight", "slide-is-disabled", "slide-is-disabled")

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

    def use(self, name, avatar):
        """
        name is a key (string) that was previously passed to add().
        
        Use a previously added control system.
        
        See also: add().
        """
        assert self.notify.debugCall(id(self))
        controls = self.controls.get(name)
        if controls is not None:
            if controls is not self.currentControls:
                if self.currentControls is not None:
                    self.currentControls.disableAvatarControls()
                    self.currentControls.setCollisionsActive(0)
                self.currentControls = controls
                self.currentControls.setAvatar(avatar)
                self.currentControls.setCollisionsActive(1)
                if self.isEnabled:
                    self.currentControls.enableAvatarControls()
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
        #self.monitorTask.remove()
    
    def getSpeeds(self):
        return self.currentControls.getSpeeds()

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
