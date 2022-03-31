
print('ThreeUpStart: Starting up environment.')

from panda3d.core import *

from direct.showbase.PythonUtil import *
from direct.showbase import ThreeUpShow

base = ThreeUpShow.ThreeUpShow()

# Put an axis in the world:
base.loader.loadModel("models/misc/xyzAxis").reparentTo(render)

base.camera.setPosHpr(0, -10.0, 0, 0, 0, 0)
base.camLens.setFov(52.0)
base.camLens.setNearFar(1.0, 10000.0)

base.clock.setMaxDt(0.2)
base.enableParticles()
base.addAngularIntegrator()

# Force the screen to update:
base.graphicsEngine.renderFrame()
