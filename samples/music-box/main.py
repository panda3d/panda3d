#!/usr/bin/env python

# Author: Shao Zhang, Phil Saltzman, and Elan Ruskin
# Last Updated: 2015-03-13
#
# This tutorial shows how to load, play, and manipulate sounds
# and sound intervals in a panda project.

from direct.showbase.ShowBase import ShowBase
from panda3d.core import NodePath, TextNode
from panda3d.core import PointLight, AmbientLight
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.interval.SoundInterval import SoundInterval
from direct.gui.DirectSlider import DirectSlider
from direct.gui.DirectButton import DirectButton
from direct.interval.MetaInterval import Parallel
from direct.interval.LerpInterval import LerpHprInterval
import sys

# Create an instance of ShowBase, which will open a window and set up a
# scene graph and camera.
base = ShowBase()

class MusicBox(DirectObject):
    def __init__(self):
        # Our standard title and instructions text
        self.title = OnscreenText(text="Panda3D: Tutorial - Music Box",
                                  parent=base.a2dBottomCenter,
                                  pos=(0, 0.08), scale=0.08,
                                  fg=(1, 1, 1, 1), shadow=(0, 0, 0, .5))
        self.escapeText = OnscreenText(text="ESC: Quit", parent=base.a2dTopLeft,
                                       fg=(1, 1, 1, 1), pos=(0.06, -0.1),
                                       align=TextNode.ALeft, scale=.05)

        # Set up the key input
        self.accept('escape', sys.exit)

        # Fix the camera position
        base.disableMouse()

        # Loading sounds is done in a similar way to loading other things
        # Loading the main music box song
        self.musicBoxSound = loader.loadMusic('music/musicbox.ogg')
        self.musicBoxSound.setVolume(.5)  # Volume is a percentage from 0 to 1
        # 0 means loop forever, 1 (default) means
        # play once. 2 or higher means play that many times
        self.musicBoxSound.setLoopCount(0)

        # Set up a simple light.
        self.plight = PointLight("light")
        self.plight.setColor((0.7, 0.7, 0.5, 1))
        light_path = base.render.attachNewNode(self.plight)
        light_path.setPos(0, 0, 20)
        base.render.setLight(light_path)

        alight = AmbientLight("ambient")
        alight.setColor((0.3, 0.3, 0.4, 1))
        base.render.setLight(base.render.attachNewNode(alight))

        # Enable per-pixel lighting
        base.render.setShaderAuto()

        # Sound objects do not have a pause function, just play and stop. So we will
        # Use this variable to keep track of where the sound is at when it was stoped
        # to impliment pausing
        self.musicTime = 0

        # Loading the open/close effect
        # loadSFX and loadMusic are identical. They are often used for organization
        #(loadMusic is used for background music, loadSfx is used for other effects)
        self.lidSfx = loader.loadSfx('music/openclose.ogg')
        # The open/close file has both effects in it. Fortunatly we can use intervals
        # to easily define parts of a sound file to play
        self.lidOpenSfx = SoundInterval(self.lidSfx, duration=2, startTime=0)
        self.lidCloseSfx = SoundInterval(self.lidSfx, startTime=5)

        # For this tutorial, it seemed appropriate to have on screen controls.
        # The following code creates them.
        # This is a label for a slider
        self.sliderText = OnscreenText("Volume", pos=(-0.1, 0.87), scale=.07,
                                       fg=(1, 1, 1, 1), shadow=(0, 0, 0, 1))
        # The slider itself. It calls self.setMusicBoxVolume when changed
        self.slider = DirectSlider(pos=(-0.1, 0, .75), scale=0.8, value=.50,
                                   command=self.setMusicBoxVolume)
        # A button that calls self.toggleMusicBox when pressed
        self.button = DirectButton(pos=(.9, 0, .75), text="Open",
                                   scale=.1, pad=(.2, .2),
                                   rolloverSound=None, clickSound=None,
                                   command=self.toggleMusicBox)

        # A variable to represent the state of the simulation. It starts closed
        self.boxOpen = False

        # Here we load and set up the music box. It was modeled in a complex way, so
        # setting it up will be complicated
        self.musicBox = loader.loadModel('models/MusicBox')
        self.musicBox.setPos(0, 60, -9)
        self.musicBox.reparentTo(render)
        # Just like the scene graph contains hierarchies of nodes, so can
        # models. You can get the NodePath for the node using the find
        # function, and then you can animate the model by moving its parts
        # To see the hierarchy of a model, use, the ls function
        # self.musicBox.ls() prints out the entire hierarchy of the model

        # Finding pieces of the model
        self.Lid = self.musicBox.find('**/lid')
        self.Panda = self.musicBox.find('**/turningthing')

        # This model was made with the hinge in the wrong place
        # this is here so we have something to turn
        self.HingeNode = self.musicBox.find(
            '**/box').attachNewNode('nHingeNode')
        self.HingeNode.setPos(.8659, 6.5, 5.4)
        # WRT - ie with respect to. Reparents the object without changing
        # its position, size, or orientation
        self.Lid.wrtReparentTo(self.HingeNode)
        self.HingeNode.setHpr(0, 90, 0)

        # This sets up an interval to play the close sound and actually close the box
        # at the same time.
        self.lidClose = Parallel(
            self.lidCloseSfx,
            LerpHprInterval(self.HingeNode, 2.0, (0, 90, 0), blendType='easeInOut'))

        # Same thing for opening the box
        self.lidOpen = Parallel(
            self.lidOpenSfx,
            LerpHprInterval(self.HingeNode, 2.0, (0, 0, 0), blendType='easeInOut'))

        # The interval for turning the panda
        self.PandaTurn = self.Panda.hprInterval(7, (360, 0, 0))
        # Do a quick loop and pause to set it as a looping interval so it can be
        # started with resume and loop properly
        self.PandaTurn.loop()
        self.PandaTurn.pause()

    def setMusicBoxVolume(self):
        # Simply reads the current value from the slider and sets it in the
        # sound
        newVol = self.slider.guiItem.getValue()
        self.musicBoxSound.setVolume(newVol)

    def toggleMusicBox(self):
        #if self.lidOpen.isPlaying() or self.lidClose.isPlaying():
        #    # It's currently already opening or closing
        #    return

        if self.boxOpen:
            self.lidOpen.pause()

            self.lidClose.start()  # Start the close box interval
            self.PandaTurn.pause()  # Pause the figurine turning
            # Save the current time of the music
            self.musicTime = self.musicBoxSound.getTime()
            self.musicBoxSound.stop()  # Stop the music
            self.button['text'] = "Open"  # Prepare to change button label
        else:
            self.lidClose.pause()

            self.lidOpen.start()  # Start the open box interval
            self.PandaTurn.resume()  # Resume the figuring turning
            # Reset the time of the music so it starts where it left off
            self.musicBoxSound.setTime(self.musicTime)
            self.musicBoxSound.play()  # Play the music
            self.button['text'] = "Close"  # Prepare to change button label

        self.button.setText()  # Actually change the button label
        # Set our state to opposite what it was
        self.boxOpen = not self.boxOpen
        #(closed to open or open to closed)

# and we can run!
mb = MusicBox()
base.run()
