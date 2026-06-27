#!/usr/bin/env python

# Tell Panda3D to use FMOD for better audio format support (must be BEFORE imports)
from panda3d.core import loadPrcFileData
loadPrcFileData("", "audio-library-name p3fmod_audio")
loadPrcFileData("", "notify-level-audio debug")  # Enable audio debug logging

from panda3d.core import MovieTexture, CardMaker, NodePath, TextNode, AudioSound, FilterProperties
from direct.showbase.DirectObject import DirectObject
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.ShowBase import ShowBase
import random


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

        # Initialize pause state
        self.is_paused = False

        # Track active filters
        self.active_filters = []

        self.title = addTitle("Panda3D: Tutorial - Media Player")
        self.inst1 = addInstructions(0.06, "P: Play/Pause")
        self.inst2 = addInstructions(0.12, "S: Stop and Rewind")
        self.inst3 = addInstructions(0.18, "M: Slow Motion / Normal Motion toggle")
        self.inst4 = addInstructions(0.24, "F: Add Random Audio Filter")
        self.inst5 = addInstructions(0.30, "C: Clear All Filters")

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

        self.sound = loader.loadSfx("PandaSneezes.mp3")
        # Synchronize the video to the sound.
        self.tex.synchronizeTo(self.sound)

        self.accept('p', self.playpause)
        self.accept('P', self.playpause)
        self.accept('s', self.stopsound)
        self.accept('S', self.stopsound)
        self.accept('m', self.fastforward)
        self.accept('M', self.fastforward)
        self.accept('f', self.add_random_filter)
        self.accept('F', self.add_random_filter)
        self.accept('c', self.clear_filters)
        self.accept('C', self.clear_filters)

    def stopsound(self):
        self.sound.stop()
        self.sound.setPlayRate(1.0)
        self.is_paused = False  # Reset pause state

    def fastforward(self):
        if self.sound.status() == AudioSound.PLAYING and not self.is_paused:
            # Toggle between normal and slow motion
            if self.sound.getPlayRate() == 1.0:
                self.sound.setPlayRate(0.5)
            else:
                self.sound.setPlayRate(1.0)

    def playpause(self):
        if self.is_paused:
            # Resume from pause by restoring play rate
            self.sound.setPlayRate(1.0)
            self.is_paused = False
        elif self.sound.status() == AudioSound.PLAYING:
            # Pause by setting play rate to 0 (keeps position, no visual glitch)
            self.sound.setPlayRate(0.0)
            self.is_paused = True
        else:
            # Start playing for the first time
            self.sound.play()
            self.is_paused = False

    def add_random_filter(self):
        """Add an echo DSP filter effect to the audio"""
        # Create FilterProperties and add echo filter
        # add_echo(drymix, wetmix, delay, decayratio)
        filter_props = FilterProperties()
        filter_props.add_echo(0.8, 0.5, 300.0, 0.5)

        # Apply to the audio manager
        if self.sfxManagerList:
            manager = self.sfxManagerList[0]
            manager.configure_filters(filter_props)
            self.active_filters.append('echo')
            print(f"Applied echo filter (total: {len(self.active_filters)})")

    def clear_filters(self):
        """Remove all DSP filters"""
        if self.sfxManagerList:
            manager = self.sfxManagerList[0]

            # Apply empty FilterProperties to clear all filters
            filter_props = FilterProperties()
            manager.configure_filters(filter_props)
            self.active_filters.clear()

            # Restart playback to flush FMOD's DSP buffers (which still contain
            # processed audio with the old effects) - do this even if paused
            if self.sound.status() == AudioSound.PLAYING:
                current_time = self.sound.getTime()
                was_paused = self.is_paused
                self.sound.stop()
                self.sound.setTime(current_time)
                self.sound.play()
                # Restore pause state if needed
                if was_paused:
                    self.sound.setPlayRate(0.0)

            print(f"Cleared all filters (total: {len(self.active_filters)})")

player = MediaPlayer("PandaSneezes.ogv")
player.run()
