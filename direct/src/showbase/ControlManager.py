
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
        assert(self.debugPrint("ControlManager()"))
        
        self.enableJumpCounter = 1
        
        self.swimControls=NonPhysicsWalker.NonPhysicsWalker()
        self.ghostControls=GhostWalker.GhostWalker()
        if __debug__:
            self.devControls=DevWalker.DevWalker()
        self.walkControls=GravityWalker.GravityWalker(
            gravity = -32.1740 * 2.0) # * 2.0 is a hack;
        
        # This is the non physics walker if you ever wanted to turn off phys
        # self.walkControls=NonPhysicsWalker.NonPhysicsWalker()
        
        self.currentControls = self.walkControls
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
        controls = self.controls.get(name)
        if controls is not None:
            print "Replacing controls:", name
            controls.delete()
        self.controls[name] = controls

    def use(self, name="basic"):
        controls = self.controls.get(name)
        if controls is not None:
            if controls is not self.currentControls:
                self.currentControls.disableAvatarControls()
                self.currentControls.setCollisionsActive(0)
                self.currentControls = controls
                self.currentControls.setCollisionsActive(1)
        else:
            print "Unkown controls:", name
    
    def setSpeeds_new(self, toonForwardSpeed, toonJumpForce,
            toonReverseSpeed, toonRotateSpeed):
        assert(self.debugPrint(
            "setSpeeds(toonForwardSpeed=%s, toonJumpForce=%s, toonReverseSpeed=%s, toonRotateSpeed=%s)"%(
            toonForwardSpeed, toonJumpForce,
            toonReverseSpeed, toonRotateSpeed)))
        for controls in self.controls.values():
            controls.setWalkSpeed(
                toonForwardSpeed,
                toonJumpForce,
                toonReverseSpeed,
                toonRotateSpeed)


    def useSwimControls(self):
        assert(self.debugPrint("useSwimControls()"))
        if self.currentControls is not self.swimControls:
            self.currentControls.disableAvatarControls()
            self.currentControls.setCollisionsActive(0)
            self.swimControls.setCollisionsActive(1)
            self.currentControls = self.swimControls
            if self.isEnabled:
                self.currentControls.enableAvatarControls()

    def useGhostControls(self):
        assert(self.debugPrint("useGhostControls()"))
        if self.currentControls is not self.ghostControls:
            self.currentControls.disableAvatarControls()
            self.currentControls.setCollisionsActive(0)
            self.ghostControls.setCollisionsActive(1)
            self.currentControls = self.ghostControls
            if self.isEnabled:
                self.currentControls.enableAvatarControls()

    def useWalkControls(self):
        assert(self.debugPrint("useWalkControls()"))
        if self.currentControls is not self.walkControls:
            self.currentControls.disableAvatarControls()
            self.currentControls.setCollisionsActive(0)
            self.walkControls.setCollisionsActive(1)
            self.currentControls = self.walkControls
            if self.isEnabled:
                self.currentControls.enableAvatarControls()

    if __debug__:
        def useDevControls(self):
            assert(self.debugPrint("useDevControls()"))
            if self.currentControls is not self.devControls:
                self.currentControls.disableAvatarControls()
                self.currentControls.setCollisionsActive(0)
                self.devControls.setCollisionsActive(1)
                self.currentControls = self.devControls
                if self.isEnabled:
                    self.currentControls.enableAvatarControls()

    def delete(self):
        self.disable()
        #self.monitorTask.remove()
    
    def setSpeeds(self, toonForwardSpeed, toonJumpForce,
            toonReverseSpeed, toonRotateSpeed):
        assert(self.debugPrint(
            "setSpeeds(toonForwardSpeed=%s, toonJumpForce=%s, toonReverseSpeed=%s, toonRotateSpeed=%s)"%(
            toonForwardSpeed, toonJumpForce,
            toonReverseSpeed, toonRotateSpeed)))
        self.swimControls.setWalkSpeed(
            toonForwardSpeed,
            toonJumpForce,
            toonReverseSpeed,
            toonRotateSpeed)
        self.ghostControls.setWalkSpeed(
            toonForwardSpeed,
            toonJumpForce,
            toonReverseSpeed,
            toonRotateSpeed)
        self.walkControls.setWalkSpeed(
            toonForwardSpeed,
            toonJumpForce,
            toonReverseSpeed,
            toonRotateSpeed)
        if __debug__:
            self.devControls.setWalkSpeed(
                toonForwardSpeed,
                toonJumpForce,
                toonReverseSpeed,
                toonRotateSpeed)
    
    def getSpeeds(self):
        return self.currentControls.getSpeeds()
    
    def initializeCollisions(self, cTrav,
            wallBitmask, floorBitmask, ghostBitmask, avatarRadius, floorOffset, reach = 4.0):
        assert(self.debugPrint("initializeCollisions()"))
        
        self.walkControls.initializeCollisions(cTrav, self.avatar,
                wallBitmask, floorBitmask, avatarRadius, floorOffset, reach)
        self.walkControls.setAirborneHeightFunc(self.avatar.getAirborneHeight)
        self.walkControls.disableAvatarControls()
        self.walkControls.setCollisionsActive(0)
        
        self.swimControls.initializeCollisions(cTrav, self.avatar,
                wallBitmask, floorBitmask, avatarRadius, floorOffset, reach)
        self.swimControls.setAirborneHeightFunc(self.avatar.getAirborneHeight)
        self.swimControls.disableAvatarControls()
        self.swimControls.setCollisionsActive(0)
        
        self.ghostControls.initializeCollisions(cTrav, self.avatar,
                ghostBitmask, floorBitmask, avatarRadius, floorOffset, reach)
        self.ghostControls.setAirborneHeightFunc(self.avatar.getAirborneHeight)
        self.ghostControls.disableAvatarControls()
        self.ghostControls.setCollisionsActive(0)
        
        if __debug__:
            self.devControls.initializeCollisions(cTrav, self.avatar,
                    wallBitmask, floorBitmask, avatarRadius, floorOffset, reach)
            self.devControls.setAirborneHeightFunc(self.avatar.getAirborneHeight)
            self.devControls.disableAvatarControls()
            self.devControls.setCollisionsActive(0)

        #self.walkControls.setCollisionsActive(1)
        #self.walkControls.enableAvatarControls()

    def deleteCollisions(self):
        assert(self.debugPrint("deleteCollisions()"))
        self.walkControls.deleteCollisions()
        self.swimControls.deleteCollisions()
        self.ghostControls.deleteCollisions()
        if __debug__:
            self.devControls.deleteCollisions()

    def collisionsOn(self):
        assert(self.debugPrint("collisionsOn()"))
        self.currentControls.setCollisionsActive(1)

    def collisionsOff(self):
        assert(self.debugPrint("collisionsOff()"))
        self.currentControls.setCollisionsActive(0)

    def placeOnFloor(self):
        self.currentControls.placeOnFloor()

    def enable(self):
        assert(self.debugPrint("enable()"))
        self.isEnabled = 1
        self.currentControls.enableAvatarControls()
    
    def disable(self):
        assert(self.debugPrint("disable()"))
        self.isEnabled = 0
        self.currentControls.disableAvatarControls()

    def enableAvatarJump(self):
        """
        Stop forcing the ctrl key to return 0's
        """
        self.enableJumpCounter+=1
        if self.enableJumpCounter:
            assert self.enableJumpCounter == 1
            self.enableJumpCounter = 1
            inputState.unforce("jump")

    def disableAvatarJump(self):
        """
        Force the ctrl key to return 0's
        """
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

    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(id(self))+' '+message)
