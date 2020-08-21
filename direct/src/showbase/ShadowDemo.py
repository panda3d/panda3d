"""Create a cheesy shadow effect by rendering the view of an
object (e.g. the local avatar) from a special camera as seen from
above (as if from the sun), using a solid gray foreground and a
solid white background, and then multitexturing that view onto the
world.

This is meant primarily as a demonstration of multipass and
multitexture rendering techniques.  It's not a particularly great
way to do shadows.
"""

__all__ = ['ShadowCaster', 'avatarShadow', 'piratesAvatarShadow', 'arbitraryShadow']

from panda3d.core import *
from direct.task import Task

sc = None

class ShadowCaster:
    texXSize = 128
    texYSize = 128

    def __init__(self, lightPath, objectPath, filmX, filmY):
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
        self.tex.setWrapU(Texture.WMBorderColor)
        self.tex.setWrapV(Texture.WMBorderColor)

        # Set up a display region on this buffer, and create a camera.
        dr = self.buffer.makeDisplayRegion()
        self.camera = Camera('shadowCamera')
        self.cameraPath = self.lightPath.attachNewNode(self.camera)
        self.camera.setScene(self.objectPath)
        dr.setCamera(self.cameraPath)

        # Use a temporary NodePath to define the initial state for the
        # camera.  The initial state will render everything in a
        # flat-shaded gray, as if it were a shadow.
        initial = NodePath('initial')
        initial.setColor(0.6, 0.6, 0.6, 1, 1)
        initial.setTextureOff(2)
        initial.setLightOff(2)
        self.camera.setInitialState(initial.getState())

        # Use an orthographic lens for this camera instead of the
        # usual perspective lens.  An orthographic lens is better to
        # simulate sunlight, which is (almost) orthographic.  We set
        # the film size large enough to render a typical avatar (but
        # not so large that we lose detail in the texture).
        self.lens = OrthographicLens()
        self.lens.setFilmSize(filmX, filmY)
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
    # stash it so that when the game hides and shows it, it will still be gone
    base.localAvatar.dropShadow.stash()

    # Set up a new node to hold the "light": this is an abitrary point
    # somewhere above the avatar, looking down, as if from the sun.
    objectPath = base.localAvatar.getGeomNode()
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

    sc = ShadowCaster(lightPath, objectPath, 4, 6)

    # Naively, just apply the shadow to everything in the world.  It
    # would probably be better to use a little restraint.
    sc.setGround(render)

    return sc

def piratesAvatarShadow():
    a = avatarShadow()
    # Force the lod to be 0 at all times
    base.localAvatar.getGeomNode().getChild(0).node().forceSwitch(0)
    return a

def arbitraryShadow(node):
    # Turn off the existing drop shadow, if any
    if hasattr(node, "dropShadow"):
        # stash it so that when the game hides and shows it, it will still be gone
        node.dropShadow.stash()

    # Set up a new node to hold the "light": this is an abitrary point
    # somewhere above the node, looking down, as if from the sun.
    objectPath = node
    shadowCamera = objectPath.attachNewNode('shadowCamera')
    lightPath = shadowCamera.attachNewNode('lightPath')

    # We can change this position at will to change the angle of the
    # sun.
    lightPath.setPos(50, 0, 50)

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

    sc = ShadowCaster(lightPath, objectPath, 100, 100)

    # Naively, just apply the shadow to everything in the world.  It
    # would probably be better to use a little restraint.
    sc.setGround(render)

    return sc

##def testShadow():
##    a = piratesAvatarShadow()
##
##from direct.showbase.ShadowDemo import *
##from direct.interval.IntervalGlobal import *
##b = loader.loadModel('/i/beta/PotC/Maya/Pirates/scenes/models/sets/buildings/spanish_buildings/TavernIntExt/tavern_ext/bar.egg')
##bs = arbitraryShadow(b)
##s = loader.loadModel('smiley')
##s.reparentTo(bs.lightPath)
##b.reparentTo((base.localAvatar))
##a = AmbientLight('cloudAmbientHi')
##a.setColor(Vec4(0.9, 0.9, 0.9, 1.000))
##aNP = s.attachNewNode(a)
##b.setLight(aNP)
##d = DirectionalLight("chernabogDirectionalLight")
##d.setDirection(Vec3(0, 1, 0))
##d.setColor(Vec4(1))
###d.setColor(Vec4(0.9, 0.7, 0.7, 1.000))
##dNP = s.attachNewNode(d)
##b.setLight(dNP)
##
##ival = Sequence(LerpPosInterval(bs.lightPath, 0.0, Vec3(-200, 0, 50)),
##                LerpPosInterval(bs.lightPath, 10.0, Vec3(-200, 0, 200)),
##                LerpPosInterval(bs.lightPath, 10.0, Vec3(200, 0, 200)),
##                LerpPosInterval(bs.lightPath, 10.0, Vec3(200, 0, 50)),
##)
##ival.loop()



