
"""Create a cheesy shadow effect by rendering the view of an
object (e.g. the local avatar) from a special camera as seen from
above (as if from the sun), using a solid gray foreground and a
solid white background, and then multitexturing that view onto the
world.

This is meant primarily as a demonstration of multipass and
multitexture rendering techniques.  It's not a particularly great
way to do shadows.
"""
    
from pandac.PandaModules import *
from direct.task import Task

sc = None

class ShadowCaster:
    texXSize = 256
    texYSize = 256
    
    def __init__(self, lightPath, objectPath):
        self.lightPath = lightPath
        self.objectPath = objectPath
        self.groundPath = None

        # Create an offscreen buffer to render the view of the avatar
        # into a texture.
        self.buffer = base.win.makeTextureBuffer(
            'shadowBuffer', self.texXSize, self.texYSize)

        # The background of this buffer--and the border of the
        # texture--is pure white.
        clearColor = VBase4(1, 1, 1, 1)
        self.buffer.setClearColor(clearColor)
        self.tex = self.buffer.getTexture()
        self.tex.setBorderColor(clearColor)
        self.tex.setWrapu(Texture.WMBorderColor)
        self.tex.setWrapv(Texture.WMBorderColor)

        # Set up a display region on this buffer, and create a camera.
        layer = self.buffer.getChannel(0).makeLayer()
        dr = layer.makeDisplayRegion()
        self.camera = Camera('shadowCamera')
        self.cameraPath = self.lightPath.attachNewNode(self.camera)
        self.camera.setScene(self.objectPath)
        dr.setCamera(self.cameraPath)

        # Use a temporary NodePath to define the initial state for the
        # camera.  The initial state will render everything in a
        # flat-shaded gray, as if it were a shadow.
        initial = NodePath('initial')
        initial.setColor(0.5, 0.5, 0.5, 1, 1)
        initial.setTextureOff(2)
        self.camera.setInitialState(initial.getState())

        # Use an orthographic lens for this camera instead of the
        # usual perspective lens.  An orthographic lens is better to
        # simulate sunlight, which is (almost) orthographic.  We set
        # the film size large enough to render a typical avatar (but
        # not so large that we lose detail in the texture).
        self.lens = OrthographicLens()
        self.lens.setFilmSize(4, 6)
        self.camera.setLens(self.lens)

        # Finally, we'll need a unique TextureStage to apply this
        # shadow texture to the world.
        self.stage = TextureStage('shadow')

        # Make sure the shadowing object doesn't get its own shadow
        # applied to it.
        self.objectPath.setTextureOff(self.stage)

    def setGround(self, groundPath):
        """ Specifies the part of the world that is to be considered
        the ground: this is the part onto which the rendered texture
        will be applied. """
        
        if self.groundPath:
            self.groundPath.clearProjectTexture(self.stage)

        self.groundPath = groundPath
        self.groundPath.projectTexture(self.stage, self.tex, self.cameraPath)

    def clear(self):
        """ Undoes the effect of the ShadowCaster. """
        if self.groundPath:
            self.groundPath.clearProjectTexture(self.stage)
            self.groundPath = None

        if self.lightPath:
            self.lightPath.detachNode()
            self.lightPath = None

        if self.cameraPath:
            self.cameraPath.detachNode()
            self.cameraPath = None
            self.camera = None
            self.lens = None

        if self.buffer:
            base.graphicsEngine.removeWindow(self.buffer)
            self.tex = None
            self.buffer = None

def avatarShadow():
    # Turn off the existing drop shadow.
    base.localAvatar.dropShadow.hide()

    # Set up a new node to hold the "light": this is an abitrary point
    # somewhere above the avatar, looking down, as if from the sun.
    objectPath = base.localAvatar
    shadowCamera = objectPath.attachNewNode('shadowCamera')
    lightPath = shadowCamera.attachNewNode('lightPath')

    # We can change this position at will to change the angle of the
    # sun.
    lightPath.setPos(5, 0, 7)

    # We need a task to keep the shadowCamera rotated in the same
    # direction relative to render (otherwise, the shadow seems to
    # rotate when you rotate your avatar, which is strange).  We can't
    # just use a compass effect, since that doesn't work on cameras.
    def shadowCameraRotate(task, shadowCamera = shadowCamera):
        shadowCamera.setHpr(render, 0, 0, 0)
        lightPath.lookAt(shadowCamera, 0, 0, 3)
        return Task.cont

    taskMgr.remove('shadowCamera')
    taskMgr.add(shadowCameraRotate, 'shadowCamera')    

    global sc
    if sc != None:
        sc.clear()
        
    sc = ShadowCaster(lightPath, objectPath)

    # Naively, just apply the shadow to everything in the world.  It
    # would probably be better to use a little restraint.
    sc.setGround(render)

    return sc
