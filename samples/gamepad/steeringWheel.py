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

loadPrcFileData("", """
    default-fov 60
    notify-level-device debug
""")

class App(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        # Print all events sent through the messenger
        #self.messenger.toggleVerbose()

        self.lblWarning = OnscreenText(
            text = "No devices found",
            fg=(1,0,0,1),
            scale = .25)

        self.lblAction = OnscreenText(
            text = "Action",
            fg=(1,1,1,1),
            scale = .15)
        self.lblAction.hide()

        # Is there a steering wheel connected?
        self.wheel = None
        devices = self.devices.getDevices(InputDevice.DeviceClass.steering_wheel)
        if devices:
            self.connect(devices[0])

        self.currentMoveSpeed = 0.0
        self.maxAccleration = 28.0
        self.deaccleration = 10.0
        self.deaclerationBreak = 37.0
        self.maxSpeed = 80.0

        # Accept device dis-/connection events
        self.accept("connect-device", self.connect)
        self.accept("disconnect-device", self.disconnect)

        self.accept("escape", exit)

        # Accept button events of the first connected steering wheel
        self.accept("steering_wheel0-face_a", self.action, extraArgs=["Action"])
        self.accept("steering_wheel0-face_a-up", self.actionUp)
        self.accept("steering_wheel0-hat_up", self.center_wheel)

        self.environment = loader.loadModel("environment")
        self.environment.reparentTo(render)

        # save the center position of the wheel
        # NOTE: here we assume, that the wheel is centered when the application get started.
        #       In real world applications, you should notice the user and give him enough time
        #       to center the wheel until you store the center position of the controler!
        self.wheelCenter = 0
        if self.wheel is not None:
            self.wheelCenter = self.wheel.findAxis(InputDevice.Axis.wheel).value

        # disable pandas default mouse-camera controls so we can handle the camera
        # movements by ourself
        self.disableMouse()
        self.reset()

        self.taskMgr.add(self.moveTask, "movement update task")

    def connect(self, device):
        """Event handler that is called when a device is discovered."""

        # We're only interested if this is a steering wheel and we don't have a
        # wheel yet.
        if device.device_class == InputDevice.DeviceClass.steering_wheel and not self.wheel:
            print("Found %s" % (device))
            self.wheel = device

            # Enable this device to ShowBase so that we can receive events.
            # We set up the events with a prefix of "steering_wheel0-".
            self.attachInputDevice(device, prefix="steering_wheel0")

            # Hide the warning that we have no devices.
            self.lblWarning.hide()

    def disconnect(self, device):
        """Event handler that is called when a device is removed."""

        if self.wheel != device:
            # We don't care since it's not our wheel.
            return

        # Tell ShowBase that the device is no longer needed.
        print("Disconnected %s" % (device))
        self.detachInputDevice(device)
        self.wheel = None

        # Do we have any steering wheels?  Attach the first other steering wheel.
        devices = self.devices.getDevices(InputDevice.DeviceClass.steering_wheel)
        if devices:
            self.connect(devices[0])
        else:
            # No devices.  Show the warning.
            self.lblWarning.show()

    def reset(self):
        """Reset the camera to the initial position."""
        self.camera.setPosHpr(0, -200, 2, 0, 0, 0)

    def action(self, button):
        # Just show which button has been pressed.
        self.lblAction.text = "Pressed %s" % button
        self.lblAction.show()

    def actionUp(self):
        # Hide the label showing which button is pressed.
        self.lblAction.hide()

    def center_wheel(self):
        """Reset the wheels center rotation to the current rotation of the wheel"""
        self.wheelCenter = self.wheel.findAxis(InputDevice.Axis.wheel).value

    def moveTask(self, task):
        dt = globalClock.getDt()
        movementVec = Vec3()

        if not self.wheel:
            return task.cont

        if self.currentMoveSpeed > 0:
            self.currentMoveSpeed -= dt * self.deaccleration
            if self.currentMoveSpeed < 0:
                self.currentMoveSpeed = 0

        # we will use the first found wheel
        # Acclerate
        accleratorPedal = self.wheel.findAxis(InputDevice.Axis.accelerator).value
        accleration = accleratorPedal * self.maxAccleration
        if self.currentMoveSpeed > accleratorPedal * self.maxSpeed:
            self.currentMoveSpeed -= dt * self.deaccleration
        self.currentMoveSpeed += dt * accleration

        # Break
        breakPedal = self.wheel.findAxis(InputDevice.Axis.brake).value
        deacleration = breakPedal * self.deaclerationBreak
        self.currentMoveSpeed -= dt * deacleration
        if self.currentMoveSpeed < 0:
            self.currentMoveSpeed = 0

        # Steering
        rotation = self.wheelCenter - self.wheel.findAxis(InputDevice.Axis.wheel).value
        base.camera.setH(base.camera, 100 * dt * rotation)

        # calculate movement
        base.camera.setY(base.camera, dt * self.currentMoveSpeed)

        return task.cont

app = App()
app.run()
