#!/usr/bin/env python

"""
This sample is incomplete; do not expect to gain much insight from this.
"""


import sys
from direct.showbase.ShowBase import ShowBase
from direct.showbase.InputStateGlobal import inputState

from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import LVector3
from panda3d.core import TransformState
from panda3d.core import BitMask32

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletGhostNode
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

        self.boxNP.node().apply_central_force(force)
        self.boxNP.node().apply_torque(torque)

    def update(self, task):
        dt = globalClock.get_dt()
        self.process_input(dt)
        self.world.do_physics(dt)
        #self.world.do_physics(dt, 20, 1.0/180.0)
        return task.cont

    def cleanup(self):
        self.world.remove(self.groundNP.node())
        self.world.remove(self.boxNP.node())
        self.world = None

        self.debugNP = None
        self.groundNP = None
        self.boxNP = None

        self.worldNP.remove_node()

    def setup(self):
        self.worldNP = render.attach_new_node('World')

        # World
        self.debugNP = self.worldNP.attach_new_node(BulletDebugNode('Debug'))
        self.debugNP.show()
        self.debugNP.node().show_wireframe(True)
        self.debugNP.node().show_constraints(True)
        self.debugNP.node().show_bounding_boxes(False)
        self.debugNP.node().show_normals(True)

        #self.debugNP.show_tight_bounds()
        #self.debugNP.show_bounds()

        self.world = BulletWorld()
        self.world.set_gravity((0, 0, -9.81))
        self.world.set_debug_node(self.debugNP.node())

        # Ground (static)
        shape = BulletPlaneShape((0, 0, 1), 1)

        self.groundNP = self.worldNP.attach_new_node(BulletRigidBodyNode('Ground'))
        self.groundNP.node().add_shape(shape)
        self.groundNP.set_pos(0, 0, -2)
        self.groundNP.set_collide_mask(BitMask32.all_on())

        self.world.attach(self.groundNP.node())

        # Box (dynamic)
        shape = BulletBoxShape((0.5, 0.5, 0.5))

        self.boxNP = self.worldNP.attach_new_node(BulletRigidBodyNode('Box'))
        self.boxNP.node().set_mass(1.0)
        self.boxNP.node().add_shape(shape)
        self.boxNP.set_pos(0, 0, 2)
        self.boxNP.set_collide_mask(BitMask32.all_on())
        self.boxNP.node().set_deactivation_enabled(False)

        self.world.attach(self.boxNP.node())

        visualNP = loader.load_model('models/box.egg')
        visualNP.clear_model_nodes()
        visualNP.reparent_to(self.boxNP)

        # TODO:
        # self.ghostNP = self.worldNP.attach_new_node(BulletGhostNode('ghost'))
        # self.ghostNP.reparent_to(self.boxNP)


game = Game()
game.run()
