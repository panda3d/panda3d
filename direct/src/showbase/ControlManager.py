
from ShowBaseGlobal import *
#from DirectGui import *
#from PythonUtil import *
#from IntervalGlobal import *

from otp.avatar import Avatar
if __debug__:
    import DevWalker
from direct.directnotify import DirectNotifyGlobal
import GhostWalker
import GravityWalker
import NonPhysicsWalker
import PhysicsWalker
from direct.task import Task


class ControlManager:
    notify = DirectNotifyGlobal.directNotify.newCategory("ControlManager")
    wantAvatarPhysicsIndicator = base.config.GetBool('want-avatar-physics-indicator', 0)
    wantAvatarPhysicsDebug = base.config.GetBool('want-avatar-physics-debug', 0)

    def __init__(self, avatar):
        self.avatar = avatar
        assert self.notify.debugCall(id(self))
        
        self.enableJumpCounter = 1
        self.controls = {}
        
        # This is the non physics walker if you ever wanted to turn off phys
        # self.walkControls=NonPhysicsWalker.NonPhysicsWalker()
        
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
        assert self.notify.debugCall(id(self))
        assert controls is not None
        oldControls = self.controls.get(name)
        if oldControls is not None:
            print "Replacing controls:", name
            oldControls.delete()
        self.controls[name] = controls

    def use(self, name="basic"):
        assert self.notify.debugCall(id(self))
        controls = self.controls.get(name)
        if controls is not None:
            if controls is not self.currentControls:
                self.currentControls.disableAvatarControls()
                self.currentControls.setCollisionsActive(0)
                self.currentControls = controls
                self.currentControls.setCollisionsActive(1)
                if self.isEnabled:
                    self.currentControls.enableAvatarControls()
            else:
                print "Controls are already", name
        else:
            print "Unkown controls:", name
    
    def setSpeeds(self, forwardSpeed, jumpForce,
            reverseSpeed, rotateSpeed):
        assert self.notify.debugCall(id(self))
        for controls in self.controls.values():
            controls.setWalkSpeed(
                forwardSpeed, jumpForce, reverseSpeed, rotateSpeed)


    #def useSwimControls(self):
    #    assert self.notify.debugCall(id(self))
    #    self.use("swim")

    #def useGhostControls(self):
    #    assert self.notify.debugCall(id(self))
    #    self.use("ghost")

    #def useWalkControls(self):
    #    assert self.notify.debugCall(id(self))
    #    self.use("walk")

    #if __debug__:
    #    def useDevControls(self):
    #        assert self.notify.debugCall(id(self))
    #        self.use("dev")

    def delete(self):
        assert self.notify.debugCall(id(self))
        self.disable()
        #self.monitorTask.remove()
    
    def getSpeeds(self):
        return self.currentControls.getSpeeds()
    
    def initializeCollisions(self, cTrav,
            wallBitmask, floorBitmask, ghostBitmask, avatarRadius, floorOffset, reach = 4.0):
        assert self.notify.debugCall(id(self))
        
        swimControls=NonPhysicsWalker.NonPhysicsWalker()
        ghostControls=GhostWalker.GhostWalker()
        if __debug__:
            devControls=DevWalker.DevWalker()
        walkControls=GravityWalker.GravityWalker(
            gravity = -32.1740 * 2.0) # * 2.0 is a hack;
        
        walkControls.initializeCollisions(cTrav, self.avatar,
                wallBitmask, floorBitmask, avatarRadius, floorOffset, reach)
        walkControls.setAirborneHeightFunc(self.avatar.getAirborneHeight)
        walkControls.disableAvatarControls()
        walkControls.setCollisionsActive(0)
        
        swimControls.initializeCollisions(cTrav, self.avatar,
                wallBitmask, floorBitmask, avatarRadius, floorOffset, reach)
        swimControls.setAirborneHeightFunc(self.avatar.getAirborneHeight)
        swimControls.disableAvatarControls()
        swimControls.setCollisionsActive(0)
        
        ghostControls.initializeCollisions(cTrav, self.avatar,
                ghostBitmask, floorBitmask, avatarRadius, floorOffset, reach)
        ghostControls.setAirborneHeightFunc(self.avatar.getAirborneHeight)
        ghostControls.disableAvatarControls()
        ghostControls.setCollisionsActive(0)
        
        if __debug__:
            devControls.initializeCollisions(cTrav, self.avatar,
                    wallBitmask, floorBitmask, avatarRadius, floorOffset, reach)
            devControls.setAirborneHeightFunc(self.avatar.getAirborneHeight)
            devControls.disableAvatarControls()
            devControls.setCollisionsActive(0)

        self.add(walkControls, "walk")
        self.add(swimControls, "swim")
        self.add(ghostControls, "ghost")
        self.add(devControls, "dev")

        self.currentControls=walkControls

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
        if 1:
            airborneHeight=self.avatar.getAirborneHeight()
            onScreenDebug.add("airborneHeight", "% 10.4f"%(airborneHeight,))
        if 0:
            onScreenDebug.add("InputState forward", "%d"%(inputState.isSet("forward")))
            onScreenDebug.add("InputState reverse", "%d"%(inputState.isSet("reverse")))
            onScreenDebug.add("InputState turnLeft", "%d"%(inputState.isSet("turnLeft")))
            onScreenDebug.add("InputState turnRight", "%d"%(inputState.isSet("turnRight")))
        return Task.cont
