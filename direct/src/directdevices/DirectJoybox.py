""" Class used to create and control joybox device """
from PandaObject import *
from DirectDeviceManager import *
from DirectGeometry import CLAMP
import OnscreenText

JOY_MIN = -0.95
JOY_MAX = 0.95
JOY_RANGE = JOY_MAX - JOY_MIN
JOY_DEADBAND = 0.05
# BUTTONS
L_STICK = 0
L_UPPER = 1
L_LOWER = 2
R_STICK = 3
R_UPPER = 4
R_LOWER = 5
# ANALOGS
L_LEFT_RIGHT = 0
L_FWD_BACK = 1
L_TWIST = 2
L_SLIDE = 3
R_LEFT_RIGHT = 4
R_FWD_BACK = 5
R_TWIST = 6
R_SLIDE = 7

class DirectJoybox(PandaObject):
    joyboxCount = 0
    xyzScale = 1.0
    hprScale = 1.0
    def __init__(self, nodePath = direct.camera):
        # See if device manager has been initialized
        if direct.deviceManager == None:
            direct.deviceManager = DirectDeviceManager()
        # Set name
        self.name = 'Joybox-' + `DirectJoybox.joyboxCount`
        # Get buttons and analogs
        self.device = base.config.GetString('joybox-device', 'CerealBox')
        self.analogs = direct.deviceManager.createAnalogs(self.device)
        self.buttons = direct.deviceManager.createButtons(self.device)
        self.aList = [0,0,0,0,0,0,0,0]
        self.bList = [0,0,0,0,0,0,0,0]
        self.mapping = [0,1,2,4,5,6]
        self.modifier = [1,1,1,1,1,1]
        # Button registry
        self.addButtonEvents()        
        # Initialize time
        self.lastTime = globalClock.getTime()
        # Record node path
        self.nodePath = nodePath
        # Text object to display current mode
        self.readout = OnscreenText.OnscreenText( '', -0.9, -0.95 )
        # Pick initial mode
        self.updateFunc = self.joeFly
        # Spawn update task
        self.enable()
    
    def enable(self):
        taskMgr.spawnMethodNamed(self.updateTask, self.name + '-updateTask')
    
    def disable(self):
        taskMgr.removeTasksNamed(self.name + '-updateTask')

    def addButtonEvents(self):
        self.breg = ButtonRegistry.ptr()
        # MRM: Hard coded!
        for i in range(8):
            self.buttons.setButtonMap(
                i, self.breg.getButton(self.getEventName(i)))
        self.eventThrower = self.buttons.getNodePath().attachNewNode(
            ButtonThrower())
    
    def setNodePath(self, nodePath):
        self.nodePath = nodePath
    def getNodePath(self):
        return self.nodePath
    def getEventName(self, index):
        return self.name + '-button-' + `index`

    def updateTask(self, state):
        self.updateVals()
        self.updateFunc()
        return Task.cont
    
    def updateVals(self):
        # Update delta time
        cTime = globalClock.getTime()
        self.deltaTime = cTime - self.lastTime
        self.lastTime = cTime
        # Update analogs
        for i in range(len(self.analogs)):
            try:
                self.aList[i] = self.normalizeAnalogChannel(i)
            except IndexError:
                # That channel may not have been updated yet
                pass
        # Update buttons
        for i in range(len(self.buttons)):
            try:
                self.bList[i] = self.buttons[i]
            except IndexError:
                # That channel may not have been updated yet
                pass
    
    def normalizeAnalog(self, val, min = -1, max = -1):
        val = CLAMP(val, JOY_MIN, JOY_MAX)
        if abs(val) < JOY_DEADBAND:
            val = 0.0
        return ((max - min) * ((val - JOY_MIN) / JOY_RANGE)) + min
    
    def normalizeAnalogChannel(self, chan, min = -1, max = 1):
        if (chan == 2) | (chan == 6):
            return self.normalizeAnalog(self.analogs[chan] * 3.0, min, max)
        else:
            return self.normalizeAnalog(self.analogs[chan], min, max)

    def showMode(self, modeText):
        def hideText(state, s = self):
            s.readout.setText('')
            return Task.done
        taskMgr.removeTasksNamed(self.name + '-showMode')
        # Update display
        self.readout.setText(modeText)
        t = taskMgr.doMethodLater(3.0, hideText, self.name + '-showMode')
        t.uponDeath = hideText

    def setMode(self, func, name):
        self.disable()
        self.updateFunc = func
        self.showMode(name)
        self.enable()
        
    def joeMode(self):
        self.setMode(self.joeFly, 'Joe Mode')
    
    def joeFly(self):
        hprScale = (self.normalizeAnalogChannel(3, 0.1, 200) *
                    DirectJoybox.hprScale)
        posScale = (self.normalizeAnalogChannel(7, 0.1, 100) *
                    DirectJoybox.xyzScale)
        # XYZ
        x = self.aList[4]
        y = self.aList[5]
        if self.bList[L_STICK]:
            z = 0.0
        else:
            z = self.aList[L_FWD_BACK]
        pos = Vec3(x,y,z) * (posScale * self.deltaTime)
        # HPR
        h = -1 * self.aList[R_TWIST]
        if self.bList[L_STICK]:
            p = -1 * self.aList[L_FWD_BACK]
        else:
            p = 0.0
        r = 0.0
        hpr = Vec3(h,p,r) * (hprScale * self.deltaTime)
        # Move node path
        self.nodePath.setPosHpr(self.nodePath, pos, hpr)

    def joyboxFly(self):
        hprScale = (self.normalizeAnalogChannel(3, 0.1, 200) *
                    DirectJoybox.hprScale)
        posScale = (self.normalizeAnalogChannel(7, 0.1, 100) *
                    DirectJoybox.xyzScale)
        x = self.analogs[self.mapping[0]] * self.modifier[0]
        y = self.analogs[self.mapping[1]] * self.modifier[1]
        z = self.analogs[self.mapping[2]] * self.modifier[2]
        pos = Vec3(x,y,z) * (posScale * self.deltaTime)
        
        h = self.analogs[self.mapping[3]] * self.modifier[3]
        p = self.analogs[self.mapping[4]] * self.modifier[4]
        r = self.analogs[self.mapping[5]] * self.modifier[5]
        hpr = Vec3(h,p,r) * (hprScale * self.deltaTime)
        # Move node path
        self.nodePath.setPosHpr(self.nodePath, pos, hpr)

    def demoMode(self):
        self.mapping = [4,5,1,6,0,0]
        self.modifier = [1,1,1,-1,0,0]
        self.setMode(self.joyboxFly, 'Demo Mode')

    def driveMode(self):
        self.mapping = [0,5,1,4,1,0]
        self.modifier = [1,1,1,-1,0,0]
        self.setMode(self.joyboxFly, 'Drive Mode')

    def hprXyzMode(self):
        self.mapping = [4,5,6,2,1,0]
        self.modifier = [1,1,-1,-1,-1,1]
        self.setMode(self.joyboxFly, 'HprXyz Mode')

    def lookaroundMode(self):
        self.mapping = [0,0,0,4,5,0]
        self.modifier = [0,0,0,-1,-1,0]
        self.setMode(self.joyboxFly, 'Lookaround Mode')

    def walkthruMode(self):
        self.mapping = [4,5,2,6,1,0]
        self.modifier = [1,1,-1,-1,-1, 1]
        self.setMode(self.joyboxFly, 'Walkthru Mode')

def jbTest():
    jb = DirectJoybox()
    jb.joeMode()
    jb.accept(jb.getEventName(R_UPPER), jb.joeMode)
    direct.cameraControl.accept(jb.getEventName(L_UPPER),
                                direct.cameraControl.orbitUprightCam)
    return jb
