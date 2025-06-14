print('TestStart: Starting up test environment.')

import direct.showbase.PythonUtil  # pylint: disable=unused-import

from direct.showbase import ShowBase
base = ShowBase.ShowBase()

# Put an axis in the world:
base.loader.loadModel("models/misc/xyzAxis").reparentTo(base.render)

base.camera.setPosHpr(0, -10.0, 0, 0, 0, 0)
base.camLens.setFov(52.0)
base.camLens.setNearFar(1.0, 10000.0)

base.clock.setMaxDt(0.2)
base.enableParticles()

# Force the screen to update:
base.graphicsEngine.renderFrame()
