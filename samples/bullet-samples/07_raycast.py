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

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletSphereShape
from panda3d.bullet import BulletConeShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletDebugNode
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

        self.boxNP.node().set_active(True)
        self.boxNP.node().apply_central_force(force)
        self.boxNP.node().apply_torque(torque)

    def update(self, task):
        dt = globalClock.get_dt()
        self.process_input(dt)
        self.world.do_physics(dt)
        self.raycast()
        return task.cont

    def raycast(self):
        pFrom = LPoint3(-4, 0, 0.5)
        pTo = LPoint3(4, 0, 0.5)
        #pTo = pFrom + LVector3(1, 0, 0) * 99999

        # Raycast for closest hit
        result = self.world.ray_test_closest(pFrom, pTo)
        print(result.has_hit(), \
        result.get_hit_fraction(), \
        result.get_node(), \
        result.get_hit_pos(), \
        result.get_hit_normal())

        # Raycast for all hits
        #result = self.world.ray_test_all(pFrom, pTo)
        #print result.has_hits(), \
        #      result.get_closest_hit_fraction(), \
        #      result.get_num_hits()
        #print [hit.get_hit_pos() for hit in result.get_hits()]

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
        shape = BulletPlaneShape((0, 0, 1), 0)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Ground'))
        np.node().add_shape(shape)
        np.set_pos(0, 0, 0)
        np.set_collide_mask(BitMask32(0x0f))

        self.world.attach(np.node())

        # Box
        shape = BulletBoxShape((0.5, 0.5, 0.5))

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Box'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(0, 0, 4)
        np.set_collide_mask(BitMask32(0x0f))

        self.world.attach(np.node())

        self.boxNP = np
        visualNP = loader.load_model('models/box.egg')
        visualNP.reparent_to(self.boxNP)

        # Sphere
        shape = BulletSphereShape(0.6)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Sphere'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(3, 0, 4)
        np.set_collide_mask(BitMask32(0x0f))

        self.world.attach(np.node())

        # Cone
        shape = BulletConeShape(0.6, 1.0)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Cone'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(6, 0, 4)
        np.set_collide_mask(BitMask32(0x0f))

        self.world.attach(np.node())


game = Game()
game.run()
