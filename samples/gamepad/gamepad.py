#!/usr/bin/env python
'''
Demonstrate usage of gamepads and other input devices

In this sample you can use a gamepad type device to control the camera and
show some messages on screen.  Using the left stick on the controler will
move the camera where the right stick will rotate the camera.
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

        # Accept device dis-/connection events
        # NOTE: catching the events here will overwrite the accept in showbase, hence
        #       we need to forward the event in the functions we set here!
        self.accept("connect-device", self.connect)
        self.accept("disconnect-device", self.disconnect)

        self.accept("escape", exit)
        self.accept("gamepad0-start", exit)
        self.accept("flight_stick0-start", exit)

        # Accept button events of the first connected gamepad
        self.accept("gamepad0-action_a", self.doAction, extraArgs=[True, "Action"])
        self.accept("gamepad0-action_a-up", self.doAction, extraArgs=[False, "Release"])
        self.accept("gamepad0-action_b", self.doAction, extraArgs=[True, "Action 2"])
        self.accept("gamepad0-action_b-up", self.doAction, extraArgs=[False, "Release"])

        self.environment = loader.loadModel("environment")
        self.environment.reparentTo(render)

        # disable pandas default mouse-camera controls so we can handle the camera
        # movements by ourself
        self.disableMouse()

        # list of connected gamepad devices
        gamepads = base.devices.getDevices(InputDevice.DC_gamepad)

        # set the center position of the control sticks
        # NOTE: here we assume, that the wheel is centered when the application get started.
        #       In real world applications, you should notice the user and give him enough time
        #       to center the wheel until you store the center position of the controler!
        self.lxcenter = gamepads[0].findControl(InputDevice.C_left_x).state
        self.lycenter = gamepads[0].findControl(InputDevice.C_left_y).state
        self.rxcenter = gamepads[0].findControl(InputDevice.C_right_x).state
        self.rycenter = gamepads[0].findControl(InputDevice.C_right_y).state


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
        # check if we have gamepad devices connected
        if self.devices.get_devices(InputDevice.DC_gamepad):
            # we have at least one gamepad device
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

        gamepads = base.devices.getDevices(InputDevice.DC_gamepad)
        if len(gamepads) == 0:
            # savety check
            return task.cont

        # we will use the first found gamepad
        # Move the camera left/right
        left_x = gamepads[0].findControl(InputDevice.C_left_x)
        movementVec.setX(left_x.state - self.lxcenter)
        # Move the camera forward/backward
        left_y = gamepads[0].findControl(InputDevice.C_left_y)
        movementVec.setY(left_y.state - self.lycenter)
        # Control the cameras heading
        right_x = gamepads[0].findControl(InputDevice.C_right_x)
        base.camera.setH(base.camera, 100 * dt * (right_x.state - self.rxcenter))
        # Control the cameras pitch
        right_y = gamepads[0].findControl(InputDevice.C_right_y)
        base.camera.setP(base.camera, 100 * dt * (right_y.state - self.rycenter))

        # calculate movement
        base.camera.setX(base.camera, 100 * dt * movementVec.getX())
        base.camera.setY(base.camera, 100 * dt * movementVec.getY())

        return task.cont

app = App()
app.run()
