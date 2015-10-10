#!/usr/bin/env python

from panda3d.core import *
# Tell Panda3D to use OpenAL, not FMOD
loadPrcFileData("", "audio-library-name p3openal_audio")

from direct.showbase.DirectObject import DirectObject
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.ShowBase import ShowBase

# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(0, 0, 0, 1), shadow=(1, 1, 1, 1),
                        parent=base.a2dTopLeft, align=TextNode.ALeft,
                        pos=(0.08, -pos - 0.04), scale=.06)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, pos=(-0.1, 0.09), scale=.08,
                        parent=base.a2dBottomRight, align=TextNode.ARight,
                        fg=(1, 1, 1, 1), shadow=(0, 0, 0, 1))


class MediaPlayer(ShowBase):

    def __init__(self, media_file):
        # Initialize the ShowBase class from which we inherit, which will
        # create a window and set up everything we need for rendering into it.
        ShowBase.__init__(self)

        self.title = addTitle("Panda3D: Tutorial - Media Player")
        self.inst1 = addInstructions(0.06, "P: Play/Pause")
        self.inst2 = addInstructions(0.12, "S: Stop and Rewind")
        self.inst3 = addInstructions(0.18,
            "M: Slow Motion / Normal Motion toggle")

        # Load the texture. We could use loader.loadTexture for this,
        # but we want to make sure we get a MovieTexture, since it
        # implements synchronizeTo.
        self.tex = MovieTexture("name")
        success = self.tex.read(media_file)
        assert success, "Failed to load video!"

        # Set up a fullscreen card to set the video texture on.
        cm = CardMaker("My Fullscreen Card")
        cm.setFrameFullscreenQuad()

        # Tell the CardMaker to create texture coordinates that take into
        # account the padding region of the texture.
        cm.setUvRange(self.tex)

        # Now place the card in the scene graph and apply the texture to it.
        card = NodePath(cm.generate())
        card.reparentTo(self.render2d)
        card.setTexture(self.tex)

        self.sound = loader.loadSfx(media_file)
        # Synchronize the video to the sound.
        self.tex.synchronizeTo(self.sound)

        self.accept('p', self.playpause)
        self.accept('P', self.playpause)
        self.accept('s', self.stopsound)
        self.accept('S', self.stopsound)
        self.accept('m', self.fastforward)
        self.accept('M', self.fastforward)

    def stopsound(self):
        self.sound.stop()
        self.sound.setPlayRate(1.0)

    def fastforward(self):
        if self.sound.status() == AudioSound.PLAYING:
            t = self.sound.getTime()
            self.sound.stop()
            if self.sound.getPlayRate() == 1.0:
                self.sound.setPlayRate(0.5)
            else:
                self.sound.setPlayRate(1.0)
            self.sound.setTime(t)
            self.sound.play()

    def playpause(self):
        if self.sound.status() == AudioSound.PLAYING:
            t = self.sound.getTime()
            self.sound.stop()
            self.sound.setTime(t)
        else:
            self.sound.play()

player = MediaPlayer("PandaSneezes.ogv")
player.run()
