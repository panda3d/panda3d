
print('ThreeUpStart: Starting up environment.')

import direct.showbase.PythonUtil  # pylint: disable=unused-import

from direct.showbase import ThreeUpShow

base = ThreeUpShow.ThreeUpShow()

# Put an axis in the world:
base.loader.loadModel("models/misc/xyzAxis").reparentTo(base.render)

assert base.camera is not None
assert base.camLens is not None

base.camera.setPosHpr(0, -10.0, 0, 0, 0, 0)
base.camLens.setFov(52.0)
base.camLens.setNearFar(1.0, 10000.0)

base.clock.setMaxDt(0.2)
base.enableParticles()
base.addAngularIntegrator()

# Force the screen to update:
base.graphicsEngine.renderFrame()
