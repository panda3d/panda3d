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

        mask = BitMask32(0x0f)
        print('ground: ', mask)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Ground'))
        np.node().add_shape(shape)
        np.set_pos(0, 0, -1)
        np.set_collide_mask(mask)

        self.world.attach(np.node())

        ## Box 1
        #shape = BulletBoxShape((0.5, 0.5, 0.5))

        #mask = BitMask32.allOff()
        #print('box-1:  ', mask)

        #np = self.worldNP.attach_new_node(BulletRigidBodyNode('Box-1'))
        #np.node().set_mass(1.0)
        #np.node().add_shape(shape)
        #np.set_pos(-6, 0, 4)
        #np.set_collide_mask(mask)

        #self.world.attach(np.node())

        ## Box 2
        #shape = BulletBoxShape((0.5, 0.5, 0.5))

        #mask = BitMask32.bit(0)
        #print('box-2:  ', mask)

        #np = self.worldNP.attach_new_node(BulletRigidBodyNode('Box-2'))
        #np.node().set_mass(1.0)
        #np.node().add_shape(shape)
        #np.set_pos(-2, 0, 4)
        #np.set_collide_mask(mask)

        #self.world.attach(np.node())

        ## Box 3
        #shape = BulletBoxShape((0.5, 0.5, 0.5))

        #mask = BitMask32.bit(2)
        #print('box-3:  ', mask)

        #np = self.worldNP.attach_new_node(BulletRigidBodyNode('Box-3'))
        #np.node().set_mass(1.0)
        #np.node().add_shape(shape)
        #np.set_pos(2, 0, 4)
        #np.set_collide_mask(mask)

        #self.world.attach(np.node())

        # Box 4
        shape = BulletBoxShape((0.5, 0.5, 0.5))

        mask = BitMask32(0x3)
        print('box-4:  ', mask)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Box-4'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(6, 0, 4)
        np.set_collide_mask(mask)

        self.world.attach(np.node())

        self.boxNP = np

        visualNP = loader.load_model('models/box.egg')
        visualNP.reparent_to(self.boxNP)

        #self.boxNP.set_collide_mask(BitMask32.allOff())
        #self.world.remove(self.boxNP.node())
        #self.world.attach(self.boxNP.node())


game = Game()
game.run()
