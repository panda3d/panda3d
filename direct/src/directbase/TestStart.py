print 'TestStart: Starting up test environment.'

from pandac.PandaModules import *

from direct.showbase.PythonUtil import *
from direct.showbase import ShowBase
ShowBase.ShowBase()

# Put an axis in the world:
loader.loadModel("models/misc/xyzAxis").reparentTo(render)

if 0:
    # Hack:
    # Enable drive mode but turn it off, and reset the camera
    # This is here because ShowBase sets up a drive interface, this
    # can be removed if ShowBase is changed to not set that up.
    base.useDrive()
    base.disableMouse()
    if base.mouseInterface:
        base.mouseInterface.reparentTo(base.dataUnused)
    if base.mouse2cam:
        base.mouse2cam.reparentTo(base.dataUnused)
    # end of hack.

camera.setPosHpr(0, -10.0, 0, 0, 0, 0)
base.camLens.setFov(52.0)
base.camLens.setNearFar(1.0, 10000.0)

globalClock.setMaxDt(0.2)
base.enableParticles()

# Force the screen to update:
base.graphicsEngine.renderFrame()
