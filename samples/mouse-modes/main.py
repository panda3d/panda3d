#!/usr/bin/env python
'''
Demonstrate different mouse modes
'''

# from panda3d.core import loadPrcFileData
#
# loadPrcFileData("", "notify-level-x11display debug")
# loadPrcFileData("", "notify-level-windisplay debug")
#
# loadPrcFileData("", "load-display p3tinydisplay")

from panda3d.core import WindowProperties, TextNode
from direct.task.TaskManagerGlobal import taskMgr
from direct.gui.OnscreenText import OnscreenText
from direct.task import Task
from direct.showbase.ShowBase import ShowBase

import sys

class App(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        self.base = self
        self.setup()

    def genLabelText(self, text, i):
        text = OnscreenText(text = text, pos = (-1.3, .5-.05*i), fg=(0,1,0,1),
                      align = TextNode.ALeft, scale = .05)
        return text


    def setup(self):
        # Disable the camera trackball controls.
        self.disableMouse()

        # control mapping of mouse movement to box movement
        self.mouseMagnitude = 1

        self.rotateX, self.rotateY = 0, 0

        self.genLabelText("[0] Absolute mode, [1] Relative mode, [2] Confined mode", 0)

        self.base.accept('0', lambda: self.setMouseMode(WindowProperties.M_absolute))
        self.base.accept('1', lambda: self.setMouseMode(WindowProperties.M_relative))
        self.base.accept('2', lambda: self.setMouseMode(WindowProperties.M_confined))

        self.genLabelText("[C] Manually re-center mouse on each tick", 1)
        self.base.accept('C', lambda: self.toggleRecenter())
        self.base.accept('c', lambda: self.toggleRecenter())

        self.genLabelText("[S] Show mouse", 2)
        self.base.accept('S', lambda: self.toggleMouse())
        self.base.accept('s', lambda: self.toggleMouse())

        self.base.accept('escape', sys.exit, [0])

        self.mouseText = self.genLabelText("", 5)
        self.deltaText = self.genLabelText("", 6)
        self.positionText = self.genLabelText("", 8)

        self.lastMouseX, self.lastMouseY = None, None

        self.hideMouse = False

        self.setMouseMode(WindowProperties.M_absolute)
        self.manualRecenterMouse = True

        # make a box to move with the mouse
        self.model = self.loader.loadModel("box")
        self.model.reparentTo(self.render)

        self.cam.setPos(0, -5, 0)
        self.cam.lookAt(0, 0, 0)

        self.mouseTask = taskMgr.add(self.mouseTask, "Mouse Task")

    def setMouseMode(self, mode):
        print("Changing mode to %s" % mode)

        self.mouseMode = mode

        wp = WindowProperties()
        wp.setMouseMode(mode)
        self.base.win.requestProperties(wp)

        # these changes may require a tick to apply
        self.base.taskMgr.doMethodLater(0, self.resolveMouse, "Resolve mouse setting")

    def resolveMouse(self, t):
        wp = self.base.win.getProperties()

        actualMode = wp.getMouseMode()
        if self.mouseMode != actualMode:
            print("ACTUAL MOUSE MODE: %s" % actualMode)

        self.mouseMode = actualMode

        self.rotateX, self.rotateY = -.5, -.5
        self.lastMouseX, self.lastMouseY = None, None
        self.recenterMouse()

    def recenterMouse(self):
        self.base.win.movePointer(0,
              int(self.base.win.getProperties().getXSize() / 2),
              int(self.base.win.getProperties().getYSize() / 2))


    def toggleRecenter(self):
        print("Toggling re-center behavior")
        self.manualRecenterMouse = not self.manualRecenterMouse

    def toggleMouse(self):
        print("Toggling mouse visibility")

        self.hideMouse = not self.hideMouse

        wp = WindowProperties()
        wp.setCursorHidden(self.hideMouse)
        self.base.win.requestProperties(wp)

    def mouseTask(self, task):
        mw = self.base.mouseWatcherNode

        hasMouse = mw.hasMouse()
        if hasMouse:
            # get the window manager's idea of the mouse position
            x, y = mw.getMouseX(), mw.getMouseY()

            if self.lastMouseX is not None:
                # get the delta
                if self.manualRecenterMouse:
                    # when recentering, the position IS the delta
                    # since the center is reported as 0, 0
                    dx, dy = x, y
                else:
                    dx, dy = x - self.lastMouseX, y - self.lastMouseY
            else:
                # no data to compare with yet
                dx, dy = 0, 0

            self.lastMouseX, self.lastMouseY = x, y

        else:
            x, y, dx, dy = 0, 0, 0, 0

        if self.manualRecenterMouse:
            # move mouse back to center
            self.recenterMouse()
            self.lastMouseX, self.lastMouseY = 0, 0

        # scale position and delta to pixels for user
        w, h = self.win.getSize()

        self.mouseText.setText("Mode: {0}, Recenter: {1}  |  Mouse: {2}, {3}  |  hasMouse: {4}".format(
             self.mouseMode, self.manualRecenterMouse,
             int(x*w), int(y*h),
             hasMouse))
        self.deltaText.setText("Delta: {0}, {1}".format(
             int(dx*w), int(dy*h)))

        # rotate box by delta
        self.rotateX += dx * 10 * self.mouseMagnitude
        self.rotateY += dy * 10 * self.mouseMagnitude

        self.positionText.setText("Model rotation: {0}, {1}".format(
             int(self.rotateX*1000)/1000., int(self.rotateY*1000)/1000.))

        self.model.setH(self.rotateX)
        self.model.setP(self.rotateY)
        return Task.cont

app = App()
app.run()
