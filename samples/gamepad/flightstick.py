#!/usr/bin/env python
'''
Demonstrate usage of flight stick

In this sample you can use a flight stick to control the camera and show some
messages on screen.  You can accelerate using the throttle.
'''

from direct.showbase.ShowBase import ShowBase
from panda3d.core import TextNode, InputDevice, loadPrcFileData, Vec3
from direct.gui.OnscreenText import OnscreenText

loadPrcFileData("", """
    default-fov 60
    notify-level-device debug
""")

STICK_DEAD_ZONE = 0.02
THROTTLE_DEAD_ZONE = 0.02


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

        # Is there a gamepad connected?
        self.flightStick = None
        devices = self.devices.getDevices(InputDevice.DeviceClass.flight_stick)
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
        self.accept("flight_stick0-start", exit)

        # Accept button events of the first connected flight stick
        self.accept("flight_stick0-trigger", self.action, extraArgs=["Trigger"])
        self.accept("flight_stick0-trigger-up", self.actionUp)

        self.environment = loader.loadModel("environment")
        self.environment.reparentTo(render)

        # disable pandas default mouse-camera controls so we can handle the camera
        # movements by ourself
        self.disableMouse()
        self.reset()

        self.taskMgr.add(self.moveTask, "movement update task")

    def connect(self, device):
        """Event handler that is called when a device is discovered."""

        # We're only interested if this is a flight stick and we don't have a
        # flight stick yet.
        if device.device_class == InputDevice.DeviceClass.flight_stick and not self.flightStick:
            print("Found %s" % (device))
            self.flightStick = device

            # Enable this device to ShowBase so that we can receive events.
            # We set up the events with a prefix of "flight_stick0-".
            self.attachInputDevice(device, prefix="flight_stick0")

            # Hide the warning that we have no devices.
            self.lblWarning.hide()

    def disconnect(self, device):
        """Event handler that is called when a device is removed."""

        if self.flightStick != device:
            # We don't care since it's not our gamepad.
            return

        # Tell ShowBase that the device is no longer needed.
        print("Disconnected %s" % (device))
        self.detachInputDevice(device)
        self.flightStick = None

        # Do we have any other gamepads?  Attach the first other gamepad.
        devices = self.devices.getDevices(InputDevice.DeviceClass.flight_stick)
        if devices:
            self.connect(devices[0])
        else:
            # No devices.  Show the warning.
            self.lblWarning.show()

    def reset(self):
        """Reset the camera to the initial position."""

        self.camera.setPosHpr(0, -200, 10, 0, 0, 0)

    def action(self, button):
        # Just show which button has been pressed.
        self.lblAction.text = "Pressed %s" % button
        self.lblAction.show()

    def actionUp(self):
        # Hide the label showing which button is pressed.
        self.lblAction.hide()

    def moveTask(self, task):
        dt = globalClock.getDt()

        if not self.flightStick:
            return task.cont

        if self.currentMoveSpeed > 0:
            self.currentMoveSpeed -= dt * self.deaccleration
            if self.currentMoveSpeed < 0:
                self.currentMoveSpeed = 0

        # Accelerate using the throttle.  Apply deadzone of 0.01.
        throttle = self.flightStick.findAxis(InputDevice.Axis.throttle).value
        if abs(throttle) < THROTTLE_DEAD_ZONE:
            throttle = 0
        accleration = throttle * self.maxAccleration
        if self.currentMoveSpeed > throttle * self.maxSpeed:
            self.currentMoveSpeed -= dt * self.deaccleration
        self.currentMoveSpeed += dt * accleration

        # Steering

        # Control the cameras yaw/Headding
        stick_yaw = self.flightStick.findAxis(InputDevice.Axis.yaw)
        if abs(stick_yaw.value) > STICK_DEAD_ZONE:
            base.camera.setH(base.camera, 100 * dt * stick_yaw.value)

        # Control the cameras pitch
        stick_y = self.flightStick.findAxis(InputDevice.Axis.pitch)
        if abs(stick_y.value) > STICK_DEAD_ZONE:
            base.camera.setP(base.camera, 100 * dt * stick_y.value)

        # Control the cameras roll
        stick_x = self.flightStick.findAxis(InputDevice.Axis.roll)
        if abs(stick_x.value) > STICK_DEAD_ZONE:
            base.camera.setR(base.camera, 100 * dt * stick_x.value)

        # calculate movement
        base.camera.setY(base.camera, dt * self.currentMoveSpeed)

        # Make sure camera does not go below the ground.
        if base.camera.getZ() < 1:
            base.camera.setZ(1)

        return task.cont

app = App()
app.run()
