#!/usr/bin/env python
'''
Demonstrate usage of steering wheels

In this sample you can use a wheel type device to control the camera and
show some messages on screen.  You can acclerate forward using the
accleration pedal and slow down using the break pedal.
'''

from direct.showbase.ShowBase import ShowBase
from panda3d.core import TextNode, InputDevice, loadPrcFileData, Vec3
from direct.gui.OnscreenText import OnscreenText

loadPrcFileData("", "notify-level-device debug")

class App(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        # print all events sent through the messenger
        self.messenger.toggleVerbose()

        self.lblWarning = OnscreenText(
            text = "No devices found",
            fg=(1,0,0,1),
            scale = .25)
        self.lblWarning.hide()

        self.lblAction = OnscreenText(
            text = "Action",
            fg=(1,1,1,1),
            scale = .15)
        self.lblAction.hide()

        self.checkDevices()

        self.currentMoveSpeed = 0.0
        self.maxAccleration = 28.0
        self.deaccleration = 10.0
        self.deaclerationBreak = 37.0
        self.maxSpeed = 80.0

        # Accept device dis-/connection events
        # NOTE: catching the events here will overwrite the accept in showbase, hence
        #       we need to forward the event in the functions we set here!
        self.accept("connect-device", self.connect)
        self.accept("disconnect-device", self.disconnect)

        self.accept("escape", exit)
        self.accept("flight_stick0-start", exit)

        # Accept button events of the first connected steering wheel
        self.accept("steering_wheel0-action_a", self.doAction, extraArgs=[True, "Action"])
        self.accept("steering_wheel0-action_a-up", self.doAction, extraArgs=[False, "Release"])

        self.environment = loader.loadModel("environment")
        self.environment.reparentTo(render)

        # save the center position of the wheel
        # NOTE: here we assume, that the wheel is centered when the application get started.
        #       In real world applications, you should notice the user and give him enough time
        #       to center the wheel until you store the center position of the controler!
        self.wheelCenter = 0
        wheels = base.devices.getDevices(InputDevice.DC_steering_wheel)
        if len(wheels) > 0:
            self.wheelCenter = wheels[0].findControl(InputDevice.C_wheel).state

        # disable pandas default mouse-camera controls so we can handle the camera
        # movements by ourself
        self.disableMouse()
        base.camera.setZ(2)

        self.taskMgr.add(self.moveTask, "movement update task")

    def connect(self, device):
        # we need to forward the event to the connectDevice function of showbase
        self.connectDevice(device)
        # Now we can check for ourself
        self.checkDevices()

    def disconnect(self, device):
        # we need to forward the event to the disconnectDevice function of showbase
        self.disconnectDevice(device)
        # Now we can check for ourself
        self.checkDevices()

    def checkDevices(self):
        # check if we have wheel devices connected
        if self.devices.get_devices(InputDevice.DC_steering_wheel):
            # we have at least one steering wheel device
            self.lblWarning.hide()
        else:
            # no devices connected
            self.lblWarning.show()

    def doAction(self, showText, text):
        if showText and self.lblAction.isHidden():
            self.lblAction.show()
        else:
            self.lblAction.hide()

    def moveTask(self, task):
        dt = globalClock.getDt()
        movementVec = Vec3()

        wheels = base.devices.getDevices(InputDevice.DC_steering_wheel)
        if len(wheels) == 0:
            # savety check
            return task.cont

        if self.currentMoveSpeed > 0:
            self.currentMoveSpeed -= dt * self.deaccleration
            if self.currentMoveSpeed < 0:
                self.currentMoveSpeed = 0

        # we will use the first found wheel
        # Acclerate
        accleration = wheels[0].findControl(InputDevice.C_accelerator).state * self.maxAccleration
        if self.currentMoveSpeed > wheels[0].findControl(InputDevice.C_accelerator).state * self.maxSpeed:
            self.currentMoveSpeed -= dt * self.deaccleration
        self.currentMoveSpeed += dt * accleration

        # Break
        deacleration = wheels[0].findControl(InputDevice.C_brake).state * self.deaclerationBreak
        self.currentMoveSpeed -= dt * deacleration
        if self.currentMoveSpeed < 0:
            self.currentMoveSpeed = 0

        # Steering
        rotation = self.wheelCenter - wheels[0].findControl(InputDevice.C_wheel).state
        base.camera.setH(base.camera, 100 * dt * rotation)

        # calculate movement
        base.camera.setY(base.camera, dt * self.currentMoveSpeed)

        return task.cont

app = App()
app.run()
