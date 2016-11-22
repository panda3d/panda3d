#!/usr/bin/env python
'''
Demonstrate how a simple button mapping gui can be written
'''

from direct.showbase.ShowBase import ShowBase
from direct.gui.DirectGui import (
    DGG,
    DirectFrame,
    DirectButton,
    DirectLabel,
    OkCancelDialog,
    DirectScrolledFrame)
from panda3d.core import (
    VBase4,
    TextNode,
    Vec2,
    InputDevice,
    loadPrcFileData)

# Make sure the textures look crisp on every device that supports
# non-power-2 textures
loadPrcFileData("", "textures-auto-power-2 #t")

class App(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)

        self.setBackgroundColor(0, 0, 0)
        # make the font look nice at a big scale
        DGG.getDefaultFont().setPixelsPerUnit(100)

        # a dict of actions and button/axis events
        self.gamepadMapping = {
            "Move forward":"Left Stick Y",
            "Move backward":"Left Stick Y",
            "Move left":"Left Stick X",
            "Move right":"Left Stick X",
            "Jump":"a",
            "Action":"b",
            "Sprint":"x",
            "Map":"y",
            "action-1":"c",
            "action-2":"d",
            "action-3":"e",
            "action-4":"f",
            "action-5":"g",
            "action-6":"h",
            "action-7":"i",
            "action-8":"j",
            "action-9":"k",
            "action-10":"l",
            "action-11":"m",
        }
        # this will store the action that we want to remap
        self.actionToMap = ""
        # this will store the key/axis that we want to asign to an action
        self.newActionKey = ""
        # this will store the label that needs to be actualized in the list
        self.actualizeLabel = None

        # The geometry for our basic buttons
        maps = loader.loadModel("models/button_map")
        self.buttonGeom = (
            maps.find("**/ready"),
            maps.find("**/click"),
            maps.find("**/hover"),
            maps.find("**/disabled"))

        # Create the dialog that asks the user for input on a given
        # action to map a key to.
        DGG.setDefaultDialogGeom("models/dialog.png")
        # setup a dialog to ask for device input
        self.dlgInput = OkCancelDialog(
            dialogName="dlg_device_input",
            pos=(0, 0, 0.25),
            text="Hit desired key:",
            text_fg=VBase4(0.898, 0.839, 0.730, 1.0),
            text_shadow=VBase4(0, 0, 0, 0.75),
            text_shadowOffset=Vec2(0.05, 0.05),
            text_scale=0.05,
            text_align=TextNode.ACenter,
            fadeScreen=0.65,
            frameColor=VBase4(0.3, 0.3, 0.3, 1),
            button_geom=self.buttonGeom,
            button_scale=0.15,
            button_text_scale=0.35,
            button_text_align=TextNode.ALeft,
            button_text_fg=VBase4(0.898, 0.839, 0.730, 1.0),
            button_text_pos=Vec2(-0.9, -0.125),
            button_relief=1,
            button_pad=Vec2(0.01, 0.01),
            button_frameColor=VBase4(0, 0, 0, 0),
            button_frameSize=VBase4(-1.0, 1.0, -0.25, 0.25),
            button_pressEffect=False,
            command=self.closeDialog)
        self.dlgInput.setTransparency(True)
        self.dlgInput.configureDialog()
        scale = self.dlgInput["image_scale"]
        self.dlgInput["image_scale"] = (scale[0]/2.0, scale[1], scale[2]/2.0)
        self.dlgInput["text_pos"] = (self.dlgInput["text_pos"][0], self.dlgInput["text_pos"][1] + 0.06)
        self.dlgInput.hide()

        # create a sample title
        self.textscale = 0.1
        self.title = DirectLabel(
            scale=self.textscale,
            pos=(base.a2dLeft + 0.05, 0.0, base.a2dTop - (self.textscale + 0.05)),
            frameColor=VBase4(0, 0, 0, 0),
            text="Button Mapping",
            text_align=TextNode.ALeft,
            text_fg=VBase4(1, 1, 1, 1),
            text_shadow=VBase4(0, 0, 0, 0.75),
            text_shadowOffset=Vec2(0.05, 0.05))
        self.title.setTransparency(1)

        # Set up the list of actions that we can map keys to
        # create a frame that will create the scrollbars for us
        # Load the models for the scrollbar elements
        thumbMaps = loader.loadModel("models/thumb_map")
        thumbGeom = (
            thumbMaps.find("**/thumb_ready"),
            thumbMaps.find("**/thumb_click"),
            thumbMaps.find("**/thumb_hover"),
            thumbMaps.find("**/thumb_disabled"))
        incMaps = loader.loadModel("models/inc_map")
        incGeom = (
            incMaps.find("**/inc_ready"),
            incMaps.find("**/inc_click"),
            incMaps.find("**/inc_hover"),
            incMaps.find("**/inc_disabled"))
        decMaps = loader.loadModel("models/dec_map")
        decGeom = (
            decMaps.find("**/dec_ready"),
            decMaps.find("**/dec_click"),
            decMaps.find("**/dec_hover"),
            decMaps.find("**/dec_disabled"))
        # create the scrolled frame that will hold our list
        self.lstActionMap = DirectScrolledFrame(
            # make the frame occupy the whole window
            frameSize=VBase4(base.a2dLeft, base.a2dRight, 0.0, 1.55),
            # make the canvas as big as the frame
            canvasSize=VBase4(base.a2dLeft, base.a2dRight, 0.0, 0.0),
            # set the frames color to white
            frameColor=VBase4(0, 0, 0.25, 0.75),
            pos=(0, 0, -0.8),

            verticalScroll_scrollSize=0.2,
            verticalScroll_frameColor=VBase4(0.02, 0.02, 0.02, 1),

            verticalScroll_thumb_relief=1,
            verticalScroll_thumb_geom=thumbGeom,
            verticalScroll_thumb_pressEffect=False,
            verticalScroll_thumb_frameColor=VBase4(0, 0, 0, 0),

            verticalScroll_incButton_relief=1,
            verticalScroll_incButton_geom=incGeom,
            verticalScroll_incButton_pressEffect=False,
            verticalScroll_incButton_frameColor=VBase4(0, 0, 0, 0),

            verticalScroll_decButton_relief=1,
            verticalScroll_decButton_geom=decGeom,
            verticalScroll_decButton_pressEffect=False,
            verticalScroll_decButton_frameColor=VBase4(0, 0, 0, 0),)

        # creat the list items
        idx = 0
        self.listBGEven = base.loader.loadModel("models/list_item_even")
        self.listBGOdd = base.loader.loadModel("models/list_item_odd")
        for key, value in self.gamepadMapping.items():
            item = self.__makeListItem(key, key, value, idx)
            item.reparentTo(self.lstActionMap.getCanvas())
            idx += 1

        # recalculate the canvas size to set scrollbars if necesary
        self.lstActionMap["canvasSize"] = (
            base.a2dLeft+0.05, base.a2dRight-0.05,
            -(len(self.gamepadMapping.keys())*0.1), 0.09)
        self.lstActionMap.setCanvasSize()

    def closeDialog(self, result):
        self.dlgInput.hide()
        if result == DGG.DIALOG_OK:
            # map the event to the given action
            self.gamepadMapping[self.actionToMap] = self.newActionKey
            # actualize the label in the list that shows the current
            # event for the action
            self.actualizeLabel["text"] = self.newActionKey

        # cleanup
        self.dlgInput["text"] ="Hit desired key:"
        self.actionToMap = ""
        self.newActionKey = ""
        self.actualizeLabel = None
        for bt in base.buttonThrowers:
            bt.node().setButtonDownEvent("")
        for bt in base.deviceButtonThrowers:
            bt.node().setButtonDownEvent("")
        taskMgr.remove("checkControls")

    def changeMapping(self, action, label):
        # set the action that we want to map a new key to
        self.actionToMap = action
        # set the label that needs to be actualized
        self.actualizeLabel = label
        # show our dialog
        self.dlgInput.show()

        # catch all button events
        for bt in base.buttonThrowers:
            bt.node().setButtonDownEvent("keyListenEvent")
        for bt in base.deviceButtonThrowers:
            bt.node().setButtonDownEvent("deviceListenEvent")
        self.setKeyCalled = False
        self.accept("keyListenEvent", self.setKey)
        self.accept("deviceListenEvent", self.setDeviceKey)

        # As there are no events thrown for control changes, we set up
        # a task to check if the controls got moved
        # This list will help us for checking which controls got moved
        self.controlStates = {None:{}}
        # fill it with all available controls
        for device in base.devices.get_devices():
            for ctrl in device.controls:
                if device not in self.controlStates.keys():
                    self.controlStates.update({device: {ctrl.axis: ctrl.state}})
                else:
                    self.controlStates[device].update({ctrl.axis: ctrl.state})
        # start the task
        taskMgr.add(self.watchControls, "checkControls")

    def watchControls(self, task):
        # move through all devices and all it's controls
        for device in base.devices.get_devices():
            for ctrl in device.controls:
                # if a control got changed more than the given puffer zone
                if self.controlStates[device][ctrl.axis] + 0.2 < ctrl.state or \
                   self.controlStates[device][ctrl.axis] - 0.2 > ctrl.state:
                    # set the current state in the dict
                    self.controlStates[device][ctrl.axis] = ctrl.state
                    # check which axis got moved
                    if ctrl.axis == InputDevice.C_left_x:
                        self.setKey("Left Stick X")
                    elif ctrl.axis == InputDevice.C_left_y:
                        self.setKey("Left Stick Y")
                    elif ctrl.axis == InputDevice.C_left_trigger:
                        self.setKey("Left Trigger")
                    elif ctrl.axis == InputDevice.C_right_x:
                        self.setKey("Right Stick X")
                    elif ctrl.axis == InputDevice.C_right_y:
                        self.setKey("Right Stick Y")
                    elif ctrl.axis == InputDevice.C_right_trigger:
                        self.setKey("Right Trigger")
                    elif ctrl.axis == InputDevice.C_x:
                        self.setKey("X")
                    elif ctrl.axis == InputDevice.C_y:
                        self.setKey("Y")
                    elif ctrl.axis == InputDevice.C_trigger:
                        self.setKey("Trigger")
                    elif ctrl.axis == InputDevice.C_throttle:
                        self.setKey("Throttle")
                    elif ctrl.axis == InputDevice.C_rudder:
                        self.setKey("Rudder")
                    elif ctrl.axis == InputDevice.C_hat_x:
                        self.setKey("Hat X")
                    elif ctrl.axis == InputDevice.C_hat_y:
                        self.setKey("Hat Y")
                    elif ctrl.axis == InputDevice.C_wheel:
                        self.setKey("Wheel")
                    elif ctrl.axis == InputDevice.C_accelerator:
                        self.setKey("Acclerator")
                    elif ctrl.axis == InputDevice.C_brake:
                        self.setKey("Break")
        return task.cont

    def setKey(self, args):
        self.setKeyCalled = True
        if self.dlgInput.buttonList[0].guiItem.getState() == 1:
            # this occurs if the OK button was clicked. To prevent to
            # always set the mouse1 event whenever the OK button was
            # pressed, we instantly return from this function
            return
        self.dlgInput["text"] = "New event will be:\n\n" + args
        self.newActionKey = args

    def setDeviceKey(self, args):
        if not self.setKeyCalled:
            self.setKey(args)
        self.setKeyCalled = False

    def __makeListItem(self, itemName, action, event, index):
        def dummy(): pass
        if index % 2 == 0:
            bg = self.listBGEven
        else:
            bg = self.listBGOdd
        item = DirectFrame(
            text=itemName,
            geom=bg,
            geom_scale=(base.a2dRight-0.05, 1, 0.1),
            frameSize=VBase4(base.a2dLeft+0.05, base.a2dRight-0.05, -0.05, 0.05),
            frameColor=VBase4(1,0,0,0),
            text_align=TextNode.ALeft,
            text_scale=0.05,
            text_fg=VBase4(1,1,1,1),
            text_pos=(base.a2dLeft + 0.3, -0.015),
            text_shadow=VBase4(0, 0, 0, 0.35),
            text_shadowOffset=Vec2(-0.05, -0.05),
            pos=(0.05, 0, -(0.10 * index)))
        item.setTransparency(True)
        lbl = DirectLabel(
            text=event,
            text_fg=VBase4(1, 1, 1, 1),
            text_scale=0.05,
            text_pos=Vec2(0, -0.015),
            frameColor=VBase4(0, 0, 0, 0),
            )
        lbl.reparentTo(item)
        lbl.setTransparency(True)
        buttonScale = 0.15
        btn = DirectButton(
            text="Change",
            geom=self.buttonGeom,
            scale=buttonScale,
            text_scale=0.25,
            text_align=TextNode.ALeft,
            text_fg=VBase4(0.898, 0.839, 0.730, 1.0),
            text_pos=Vec2(-0.9, -0.085),
            relief=1,
            pad=Vec2(0.01, 0.01),
            frameColor=VBase4(0, 0, 0, 0),
            frameSize=VBase4(-1.0, 1.0, -0.25, 0.25),
            pos=(base.a2dRight-(0.898*buttonScale+0.3), 0, 0),
            pressEffect=False,
            command=self.changeMapping,
            extraArgs=[action, lbl])
        btn.setTransparency(True)
        btn.reparentTo(item)
        return item

app = App()
app.run()
