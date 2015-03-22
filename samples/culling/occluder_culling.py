#!/usr/bin/env python

"""
Author: Josh Enes
Last Updated: 2015-03-13

This is a demo of Panda's occluder-culling system. It demonstrates loading
occluder from an EGG file and adding them to a CullTraverser.
"""

# Load PRC data
from panda3d.core import loadPrcFileData
loadPrcFileData('', 'window-title Occluder Demo')
loadPrcFileData('', 'sync-video false')
loadPrcFileData('', 'show-frame-rate-meter true')
loadPrcFileData('', 'texture-minfilter linear-mipmap-linear')
#loadPrcFileData('', 'fake-view-frustum-cull true') # show culled nodes in red

# Import needed modules
import random
from direct.showbase.ShowBase import ShowBase
from direct.gui.OnscreenText import OnscreenText
from panda3d.core import PerspectiveLens, TextNode, \
TexGenAttrib, TextureStage, TransparencyAttrib, LPoint3, Texture


def add_instructions(pos, msg):
    """Function to put instructions on the screen."""
    return OnscreenText(text=msg, style=1, fg=(1, 1, 1, 1), shadow=(0, 0, 0, 1),
                        parent=base.a2dTopLeft, align=TextNode.ALeft,
                        pos=(0.08, -pos - 0.04), scale=.05)

def add_title(text):
    """Function to put title on the screen."""
    return OnscreenText(text=text, style=1, pos=(-0.1, 0.09), scale=.08,
                        parent=base.a2dBottomRight, align=TextNode.ARight,
                        fg=(1, 1, 1, 1), shadow=(0, 0, 0, 1))


class Game(ShowBase):
    """Sets up the game, camera, controls, and loads models."""
    def __init__(self):
        ShowBase.__init__(self)
        self.xray_mode = False
        self.show_model_bounds = False

        # Display instructions
        add_title("Panda3D Tutorial: Occluder Culling")
        add_instructions(0.06, "[Esc]: Quit")
        add_instructions(0.12, "[W]: Move Forward")
        add_instructions(0.18, "[A]: Move Left")
        add_instructions(0.24, "[S]: Move Right")
        add_instructions(0.30, "[D]: Move Back")
        add_instructions(0.36, "Arrow Keys: Look Around")
        add_instructions(0.42, "[F]: Toggle Wireframe")
        add_instructions(0.48, "[X]: Toggle X-Ray Mode")
        add_instructions(0.54, "[B]: Toggle Bounding Volumes")

        # Setup controls
        self.keys = {}
        for key in ['arrow_left', 'arrow_right', 'arrow_up', 'arrow_down',
                    'a', 'd', 'w', 's']:
            self.keys[key] = 0
            self.accept(key, self.push_key, [key, 1])
            self.accept('shift-%s' % key, self.push_key, [key, 1])
            self.accept('%s-up' % key, self.push_key, [key, 0])
        self.accept('f', self.toggleWireframe)
        self.accept('x', self.toggle_xray_mode)
        self.accept('b', self.toggle_model_bounds)
        self.accept('escape', __import__('sys').exit, [0])
        self.disableMouse()

        # Setup camera
        self.lens = PerspectiveLens()
        self.lens.setFov(60)
        self.lens.setNear(0.01)
        self.lens.setFar(1000.0)
        self.cam.node().setLens(self.lens)
        self.camera.setPos(-9, -0.5, 1)
        self.heading = -95.0
        self.pitch = 0.0

        # Load level geometry
        self.level_model = self.loader.loadModel('models/level')
        self.level_model.reparentTo(self.render)
        self.level_model.setTexGen(TextureStage.getDefault(),
                                   TexGenAttrib.MWorldPosition)
        self.level_model.setTexProjector(TextureStage.getDefault(),
                                         self.render, self.level_model)
        self.level_model.setTexScale(TextureStage.getDefault(), 4)
        tex = self.loader.load3DTexture('models/tex_#.png')
        self.level_model.setTexture(tex)

        # Load occluders
        occluder_model = self.loader.loadModel('models/occluders')
        occluder_nodepaths = occluder_model.findAllMatches('**/+OccluderNode')
        for occluder_nodepath in occluder_nodepaths:
            self.render.setOccluder(occluder_nodepath)
            occluder_nodepath.node().setDoubleSided(True)

        # Randomly spawn some models to test the occluders
        self.models = []
        box_model = self.loader.loadModel('box')

        for dummy in range(0, 500):
            pos = LPoint3((random.random() - 0.5) * 9,
                         (random.random() - 0.5) * 9,
                         random.random() * 8)
            box = box_model.copy_to(self.render)
            box.setScale(random.random() * 0.2 + 0.1)
            box.setPos(pos)
            box.setHpr(random.random() * 360,
                         random.random() * 360,
                         random.random() * 360)
            box.reparentTo(self.render)
            self.models.append(box)

        self.taskMgr.add(self.update, 'main loop')

    def push_key(self, key, value):
        """Stores a value associated with a key."""
        self.keys[key] = value

    def update(self, task):
        """Updates the camera based on the keyboard input."""
        delta = globalClock.getDt()
        move_x = delta * 3 * -self.keys['a'] + delta * 3 * self.keys['d']
        move_z = delta * 3 * self.keys['s'] + delta * 3 * -self.keys['w']
        self.camera.setPos(self.camera, move_x, -move_z, 0)
        self.heading += (delta * 90 * self.keys['arrow_left'] +
                         delta * 90 * -self.keys['arrow_right'])
        self.pitch += (delta * 90 * self.keys['arrow_up'] +
                       delta * 90 * -self.keys['arrow_down'])
        self.camera.setHpr(self.heading, self.pitch, 0)
        return task.cont

    def toggle_xray_mode(self):
        """Toggle X-ray mode on and off. This is useful for seeing the
        effectiveness of the occluder culling."""
        self.xray_mode = not self.xray_mode
        if self.xray_mode:
            self.level_model.setColorScale((1, 1, 1, 0.5))
            self.level_model.setTransparency(TransparencyAttrib.MDual)
        else:
            self.level_model.setColorScaleOff()
            self.level_model.setTransparency(TransparencyAttrib.MNone)

    def toggle_model_bounds(self):
        """Toggle bounding volumes on and off on the models."""
        self.show_model_bounds = not self.show_model_bounds
        if self.show_model_bounds:
            for model in self.models:
                model.showBounds()
        else:
            for model in self.models:
                model.hideBounds()

game = Game()
game.run()
