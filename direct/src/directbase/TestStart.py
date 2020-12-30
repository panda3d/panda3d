print('TestStart: Starting up test environment.')

from panda3d.core import *

from direct.showbase.PythonUtil import *
from direct.showbase import ShowBase
base = ShowBase.ShowBase()

# Put an axis in the world:
base.loader.loadModel("models/misc/xyzAxis").reparentTo(render)

base.camera.setPosHpr(0, -10.0, 0, 0, 0, 0)
base.camLens.setFov(52.0)
base.camLens.setNearFar(1.0, 10000.0)

globalClock.setMaxDt(0.2)
base.enableParticles()

# Force the screen to update:
base.graphicsEngine.renderFrame()
