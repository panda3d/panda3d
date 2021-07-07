#!/usr/bin/env python

from panda3d.core import load_prc_file_data
load_prc_file_data('', 'bullet-filter-algorithm groups-mask')

import sys

from direct.showbase.ShowBase import ShowBase
from direct.showbase.InputStateGlobal import inputState

from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import LVector3
from panda3d.core import LPoint3
from panda3d.core import TransformState
from panda3d.core import BitMask32

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletDebugNode


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

        self.accept('1', self.do_select, [0,])
        self.accept('2', self.do_select, [1,])
        self.accept('3', self.do_select, [2,])
        self.accept('4', self.do_select, [3,])
        self.accept('5', self.do_select, [4,])
        self.accept('6', self.do_select, [5,])

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

    def do_select(self, i):
        self.boxNP = self.boxes[i]

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
        self.world.do_physics(dt)
        return task.cont

    def cleanup(self):
        self.world = None
        self.worldNP.remove_node()

    def setup(self):
        self.worldNP = render.attach_new_node('World')

        # World
        self.debugNP = self.worldNP.attach_new_node(BulletDebugNode('Debug'))
        self.debugNP.show()

        self.world = BulletWorld()
        self.world.set_gravity((0, 0, -9.81))
        self.world.set_debug_node(self.debugNP.node())

        # Ground
        shape = BulletPlaneShape((0, 0, 1), -1)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Ground'))
        np.node().add_shape(shape)
        np.set_pos(0, 0, -1)
        np.set_collide_mask(BitMask32.bit(0))

        self.world.attach(np.node())

        # Boxes
        self.boxes = [None,]*6

        for i in range(6):
            shape = BulletBoxShape((0.5, 0.5, 0.5))

            np = self.worldNP.attach_new_node(BulletRigidBodyNode('Box-1'))
            np.node().set_mass(1.0)
            np.node().add_shape(shape)

            self.world.attach(np.node())

            self.boxes[i] = np

            visualNP = loader.load_model('models/box.egg')
            visualNP.reparent_to(np)

        self.boxes[0].set_pos(-3, -3, 0)
        self.boxes[1].set_pos( 0, -3, 0)
        self.boxes[2].set_pos( 3, -3, 0)
        self.boxes[3].set_pos(-3,  3, 0)
        self.boxes[4].set_pos( 0,  3, 0)
        self.boxes[5].set_pos( 3,  3, 0)

        self.boxes[0].set_collide_mask(BitMask32.bit(1))
        self.boxes[1].set_collide_mask(BitMask32.bit(2))
        self.boxes[2].set_collide_mask(BitMask32.bit(3))
        self.boxes[3].set_collide_mask(BitMask32.bit(1))
        self.boxes[4].set_collide_mask(BitMask32.bit(2))
        self.boxes[5].set_collide_mask(BitMask32.bit(3))

        self.boxNP = self.boxes[0]

        self.world.set_group_collision_flag(0, 1, True)
        self.world.set_group_collision_flag(0, 2, True)
        self.world.set_group_collision_flag(0, 3, True)

        self.world.set_group_collision_flag(1, 1, False)
        self.world.set_group_collision_flag(1, 2, True)


game = Game()
game.run()
