#!/usr/bin/env python
'''
Demonstrate usage of gamepads and other input devices

In this sample you can use a gamepad type device to control the camera and
show some messages on screen.  Using the left stick on the controler will
move the camera where the right stick will rotate the camera.
'''

from direct.showbase.ShowBase import ShowBase
from panda3d.core import TextNode, InputDevice, loadPrcFileData, Vec3
from panda3d.core import TextPropertiesManager
from direct.gui.OnscreenText import OnscreenText

loadPrcFileData("", """
    default-fov 60
    notify-level-device debug
""")

# Informational text in the bottom-left corner.  We use the special \5
# character to embed an image representing the gamepad buttons.
INFO_TEXT = """Move \5lstick\5 to strafe, \5rstick\5 to turn
Press \5ltrigger\5 and \5rtrigger\5 to go down/up
Press \5face_x\5 to reset camera"""


class App(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        # Print all events sent through the messenger
        #self.messenger.toggleVerbose()

        # Load the graphics for the gamepad buttons and register them, so that
        # we can embed them in our information text.
        graphics = loader.loadModel("models/xbone-icons.egg")
        mgr = TextPropertiesManager.getGlobalPtr()
        for name in ["face_a", "face_b", "face_x", "face_y", "ltrigger", "rtrigger", "lstick", "rstick"]:
            graphic = graphics.find("**/" + name)
            graphic.setScale(1.5)
            mgr.setGraphic(name, graphic)
            graphic.setZ(-0.5)

        # Show the informational text in the corner.
        self.lblInfo = OnscreenText(
            parent = self.a2dBottomLeft,
            pos = (0.1, 0.3),
            fg = (1, 1, 1, 1),
            bg = (0.2, 0.2, 0.2, 0.9),
            align = TextNode.A_left,
            text = INFO_TEXT)
        self.lblInfo.textNode.setCardAsMargin(0.5, 0.5, 0.5, 0.2)

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
        self.gamepad = None
        devices = self.devices.getDevices(InputDevice.DeviceClass.gamepad)
        if devices:
            self.connect(devices[0])

        # Accept device dis-/connection events
        self.accept("connect-device", self.connect)
        self.accept("disconnect-device", self.disconnect)

        self.accept("escape", exit)

        # Accept button events of the first connected gamepad
        self.accept("gamepad-back", exit)
        self.accept("gamepad-start", exit)
        self.accept("gamepad-face_x", self.reset)
        self.accept("gamepad-face_a", self.action, extraArgs=["face_a"])
        self.accept("gamepad-face_a-up", self.actionUp)
        self.accept("gamepad-face_b", self.action, extraArgs=["face_b"])
        self.accept("gamepad-face_b-up", self.actionUp)
        self.accept("gamepad-face_y", self.action, extraArgs=["face_y"])
        self.accept("gamepad-face_y-up", self.actionUp)

        self.environment = loader.loadModel("environment")
        self.environment.reparentTo(render)

        # Disable the default mouse-camera controls since we need to handle
        # our own camera controls.
        self.disableMouse()
        self.reset()

        self.taskMgr.add(self.moveTask, "movement update task")

    def connect(self, device):
        """Event handler that is called when a device is discovered."""

        # We're only interested if this is a gamepad and we don't have a
        # gamepad yet.
        if device.device_class == InputDevice.DeviceClass.gamepad and not self.gamepad:
            print("Found %s" % (device))
            self.gamepad = device

            # Enable this device to ShowBase so that we can receive events.
            # We set up the events with a prefix of "gamepad-".
            self.attachInputDevice(device, prefix="gamepad")

            # Hide the warning that we have no devices.
            self.lblWarning.hide()

    def disconnect(self, device):
        """Event handler that is called when a device is removed."""

        if self.gamepad != device:
            # We don't care since it's not our gamepad.
            return

        # Tell ShowBase that the device is no longer needed.
        print("Disconnected %s" % (device))
        self.detachInputDevice(device)
        self.gamepad = None

        # Do we have any other gamepads?  Attach the first other gamepad.
        devices = self.devices.getDevices(InputDevice.DeviceClass.gamepad)
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
        self.lblAction.text = "Pressed \5%s\5" % button
        self.lblAction.show()

    def actionUp(self):
        # Hide the label showing which button is pressed.
        self.lblAction.hide()

    def moveTask(self, task):
        dt = globalClock.getDt()

        if not self.gamepad:
            return task.cont

        strafe_speed = 85
        vert_speed = 50
        turn_speed = 100

        # If the left stick is pressed, we will go faster.
        lstick = self.gamepad.findButton("lstick")
        if lstick.pressed:
            strafe_speed *= 2.0

        # we will use the first found gamepad
        # Move the camera left/right
        strafe = Vec3(0)
        left_x = self.gamepad.findAxis(InputDevice.Axis.left_x)
        left_y = self.gamepad.findAxis(InputDevice.Axis.left_y)
        strafe.x = left_x.value
        strafe.y = left_y.value

        # Apply some deadzone, since the sticks don't center exactly at 0
        if strafe.lengthSquared() >= 0.01:
            self.camera.setPos(self.camera, strafe * strafe_speed * dt)

        # Use the triggers for the vertical position.
        trigger_l = self.gamepad.findAxis(InputDevice.Axis.left_trigger)
        trigger_r = self.gamepad.findAxis(InputDevice.Axis.right_trigger)
        lift = trigger_r.value - trigger_l.value
        self.camera.setZ(self.camera.getZ() + (lift * vert_speed * dt))

        # Move the camera forward/backward
        right_x = self.gamepad.findAxis(InputDevice.Axis.right_x)
        right_y = self.gamepad.findAxis(InputDevice.Axis.right_y)

        # Again, some deadzone
        if abs(right_x.value) >= 0.1 or abs(right_y.value) >= 0.1:
            self.camera.setH(self.camera, turn_speed * dt * -right_x.value)
            self.camera.setP(self.camera, turn_speed * dt * right_y.value)

            # Reset the roll so that the camera remains upright.
            self.camera.setR(0)

        return task.cont

app = App()
app.run()
