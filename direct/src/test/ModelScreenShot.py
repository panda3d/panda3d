import direct
from pandac.PandaModules import loadPrcFileData

from direct.showbase.DirectObject import DirectObject
from direct.directbase.DirectStart import *
from pandac.PandaModules import *
import direct.gui.DirectGuiGlobals as DGG
from direct.gui.DirectGui import *
from direct.task import Task

from direct.directnotify import DirectNotifyGlobal
import math
from operator import *

import ModelScreenShotGlobals

class ModelScreenShot(DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ModelScreenShot")

    def __init__(self):

        # Grab a list of models to capture screenshots of from an array in
        # the globals file
        self.modelsToView = ModelScreenShotGlobals.models
        self.models = []

        # Attach all the models listed to render and save a pointer to them
        # in an array.  Then hide the model.
        for model in self.modelsToView:
            m = loader.loadModel(model)
            m.reparentTo(render)
            self.models.append(m)
            m.hide()

        # Set a nice farplane far, far away
        self.lens = base.camera.getChild(0).node().getLens()
        self.lens.setFar(10000)

        # Hide the cursor
        self.props = WindowProperties()
        self.props.setCursorHidden(0)
        base.win.requestProperties(self.props)

        # Method for getting the distance to an object from the camera
        def getDist(obj, lens):
            rad = obj.getBounds().getRadius()
            fov = lens.getFov()
            dist = rad / math.tan(deg2Rad(min(fov[0], fov[1]/2.0)))
            return dist

        # Determin the optimal camera position
        def getOptCamPos(obj, dist):
            cen = obj.getBounds().getCenter()
            camPos = VBase3(cen.getX(), -dist, cen.getZ())
            return camPos

        # Generate screenshots
        def generatePics():
            for model in self.models:
                model.show()
                base.camera.setPos(getOptCamPos(model, getDist(model, self.lens)))
                uFilename = model.getName().replace('.egg','.jpg')
                self.notify.info("screenshot %s   camera pos: %s" % (uFilename, base.camera.getPos()))
                base.graphicsEngine.renderFrame()
                base.screenshot(namePrefix = uFilename, defaultFilename = 0)
                model.hide()

        generatePics()
        
mss = ModelScreenShot()
run()
