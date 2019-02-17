#!/usr/bin/env python

import sys

from direct.showbase.ShowBase import ShowBase
from direct.showbase.InputStateGlobal import inputState

from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import LVector3
from panda3d.core import LPoint3
from panda3d.core import TransformState
from panda3d.core import BitMask32
from panda3d.core import Filename
from panda3d.core import PNMImage
from panda3d.core import GeoMipTerrain

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletHeightfieldShape
from panda3d.bullet import ZUp


class Game(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        base.set_background_color(0.1, 0.1, 0.8, 1)
        base.set_frame_rate_meter(True)

        base.cam.set_pos(0, -20, 4)
        base.cam.look_at(0, 0, 0)

        # Light
        alight = AmbientLight('ambientLight')
        alight.set_color((0.5, 0.5, 0.5, 1))
        alightNP = render.attach_new_node(alight)

        dlight = DirectionalLight('directionalLight')
        dlight.set_direction((1, 1, -1))
        dlight.set_color((0.7, 0.7, 0.7, 1))
        dlightNP = render.attach_new_node(dlight)

        render.clear_light()
        render.set_light(alightNP)
        render.set_light(dlightNP)

        # Input
        self.accept('escape', self.do_exit)
        self.accept('r', self.do_reset)
        self.accept('f1', base.toggle_wireframe)
        self.accept('f2', base.toggle_texture)
        self.accept('f3', self.toggle_debug)
        self.accept('f5', self.do_screenshot)

        self.accept('x', self.toggle_heightfield)

        inputState.watchWithModifiers('forward', 'w')
        inputState.watchWithModifiers('left', 'a')
        inputState.watchWithModifiers('reverse', 's')
        inputState.watchWithModifiers('right', 'd')
        inputState.watchWithModifiers('turnLeft', 'q')
        inputState.watchWithModifiers('turnRight', 'e')

        # Task
        taskMgr.add(self.update, 'updateWorld')

        # Physics
        self.setup()

    def do_exit(self):
        self.cleanup()
        sys.exit(1)

    def do_reset(self):
        self.cleanup()
        self.setup()

    def toggle_debug(self):
        if self.debugNP.is_hidden():
          self.debugNP.show()
        else:
          self.debugNP.hide()

    def do_screenshot(self):
        base.screenshot('Bullet')

    def toggle_heightfield(self):
        self.hf.set_debug_enabled(not self.hf.is_debug_enabled())

    def process_input(self, dt):
        force = LVector3(0, 0, 0)
        torque = LVector3(0, 0, 0)

        if inputState.isSet('forward'): force.y = 1.0
        if inputState.isSet('reverse'): force.y = -1.0
        if inputState.isSet('left'): force.x = -1.0
        if inputState.isSet('right'): force.x = 1.0
        if inputState.isSet('turnLeft'): torque.z = 1.0
        if inputState.isSet('turnRight'): torque.z = -1.0

        force *= 30.0
        torque *= 10.0

        self.boxNP.node().set_active(True)
        self.boxNP.node().apply_central_force(force)
        self.boxNP.node().apply_torque(torque)

    def update(self, task):
        dt = globalClock.get_dt()
        self.process_input(dt)
        self.world.do_physics(dt, 10, 0.008)
        self.terrain.update()
        return task.cont

    def cleanup(self):
        self.world = None
        self.worldNP.remove_node()

    def setup(self):
        self.worldNP = render.attach_new_node('World')

        # World
        self.debugNP = self.worldNP.attach_new_node(BulletDebugNode('Debug'))
        self.debugNP.show()
        self.debugNP.node().show_normals(True)

        self.world = BulletWorld()
        self.world.set_gravity((0, 0, -9.81))
        self.world.set_debug_node(self.debugNP.node())

        # Box (dynamic)
        shape = BulletBoxShape((0.5, 0.5, 0.5))

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Box'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(0, 0, 4)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        self.boxNP = np # For applying force & torque


        visualNP = loader.load_model('models/box.egg')
        visualNP.clear_model_nodes()
        visualNP.reparent_to(self.boxNP)

        # Heightfield (static)
        height = 8.0

        img = PNMImage(Filename('models/elevation.png'))
        shape = BulletHeightfieldShape(img, height, ZUp)
        shape.set_use_diamond_subdivision(True)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Heightfield'))
        np.node().add_shape(shape)
        np.set_pos(0, 0, 0)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        self.hf = np.node() # To enable/disable debug visualisation

        self.terrain = GeoMipTerrain('terrain')
        self.terrain.set_heightfield(img)

        self.terrain.set_block_size(32)
        self.terrain.set_near(50)
        self.terrain.set_far(100)
        self.terrain.set_focal_point(base.camera)

        rootNP = self.terrain.get_root()
        rootNP.reparent_to(render)
        rootNP.set_sz(8.0)

        offset = img.get_x_size() / 2.0 - 0.5
        rootNP.set_pos(-offset, -offset, -height / 2.0)

        self.terrain.generate()


game = Game()
game.run()
