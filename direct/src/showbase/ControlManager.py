
from ShowBaseGlobal import *
#from DirectGui import *
#from PythonUtil import *
#from IntervalGlobal import *

import Avatar
import DirectNotifyGlobal
import GhostWalker
import GravityWalker
import NonPhysicsWalker
import PhysicsWalker
import Task


class ControlManager:
    notify = DirectNotifyGlobal.directNotify.newCategory("ControlManager")
    wantAvatarPhysics = base.config.GetBool('want-avatar-physics', 0)
    wantAvatarPhysicsIndicator = base.config.GetBool('want-avatar-physics-indicator', 0)
    wantAvatarPhysicsDebug = base.config.GetBool('want-avatar-physics-debug', 0)

    def __init__(self, avatar):
        self.avatar = avatar
        assert(self.debugPrint("ControlManager()"))
        self.swimControls=GravityWalker.GravityWalker(
                    gravity = -32.1740 * 2.0)
        #self.swimControls=NonPhysicsWalker.NonPhysicsWalker()
        self.ghostControls=GhostWalker.GhostWalker()
        if self.wantAvatarPhysics:
            self.walkControls=GravityWalker.GravityWalker(
                    gravity = -32.1740 * 2.0) # * 2.0 is a hack;
            #self.walkControls=NonPhysicsWalker.NonPhysicsWalker()
            #self.walkControls=PhysicsWalker.PhysicsWalker(
            #        gravity = -32.1740 * 2.0) # * 2.0 is a hack;
        else:
            self.walkControls=NonPhysicsWalker.NonPhysicsWalker()
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
        
        inputState.watch("turnRight", "arrow_right", "arrow_right-up")
        inputState.watch("turnRight", "control-arrow_right", "control-arrow_right-up")
        inputState.watch("turnRight", "alt-arrow_right", "alt-arrow_right-up")
        inputState.watch("turnRight", "shift-arrow_right", "shift-arrow_right-up")
        
        inputState.watch("jump", "control", "control-up")
        inputState.watch("jump", "alt-control", "alt-control-up")
        inputState.watch("jump", "shift-control", "shift-control-up")
        
        # FYI, ghost mode uses jump for slide.
        inputState.watch("slide", "slide-is-disabled", "slide-is-disabled")
        
        #inputState.watch("slideLeft", "shift-arrow_left", "shift-arrow_left-up")
        #inputState.watch("slideLeft", "control-arrow_left", "control-arrow_left-up")
        #inputState.watch("slideLeft", "alt-arrow_left", "alt-arrow_left-up")
        #inputState.watch("slideLeft", "shift-arrow_left", "shift-arrow_left-up")
        inputState.watch("slideLeft", "slide-is-disabled", "slide-is-disabled")
        
        #inputState.watch("slideRight", "shift-arrow_right", "shift-arrow_right-up")
        #inputState.watch("slideRight", "control-arrow_right", "control-arrow_right-up")
        #inputState.watch("slideRight", "alt-arrow_right", "alt-arrow_right-up")
        #inputState.watch("slideRight", "shift-arrow_right", "shift-arrow_right-up")
        inputState.watch("slideRight", "slide-is-disabled", "slide-is-disabled")

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
    
    def getSpeeds(self):
        return self.currentControls.getSpeeds()
    
    def initializeCollisions(self, cTrav,
            wallBitmask, floorBitmask, ghostBitMask, avatarRadius, floorOffset):
        assert(self.debugPrint("initializeCollisions()"))
        
        self.walkControls.initializeCollisions(cTrav, self.avatar,
                wallBitmask, floorBitmask, avatarRadius, floorOffset)
        self.walkControls.setAirborneHeightFunc(self.avatar.getAirborneHeight)
        self.walkControls.disableAvatarControls()
        self.walkControls.setCollisionsActive(0)
        
        self.swimControls.initializeCollisions(cTrav, self.avatar,
                wallBitmask, floorBitmask, avatarRadius, floorOffset)
        self.swimControls.setAirborneHeightFunc(self.avatar.getAirborneHeight)
        self.swimControls.disableAvatarControls()
        self.swimControls.setCollisionsActive(0)
        
        self.ghostControls.initializeCollisions(cTrav, self.avatar,
                ghostBitMask, floorBitmask, avatarRadius, floorOffset)
        self.ghostControls.setAirborneHeightFunc(self.avatar.getAirborneHeight)
        self.ghostControls.disableAvatarControls()
        self.ghostControls.setCollisionsActive(0)

        self.walkControls.setCollisionsActive(1)
        self.walkControls.enableAvatarControls()

    def deleteCollisions(self):
        assert(self.debugPrint("deleteCollisions()"))
        self.walkControls.deleteCollisions()
        self.swimControls.deleteCollisions()
        self.ghostControls.deleteCollisions()

    def collisionsOn(self):
        assert(self.debugPrint("collisionsOn()"))
        self.currentControls.setCollisionsActive(1)

    def collisionsOff(self):
        assert(self.debugPrint("collisionsOff()"))
        self.currentControls.setCollisionsActive(0)

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
        inputState.unforce("jump")

    def disableAvatarJump(self):
        """
        Force the ctrl key to return 0's
        """
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
