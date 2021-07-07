#!/usr/bin/env python

#from panda3d.core import load_prc_file_data
#load_prc_file_data('', 'bullet-enable-contact-events true')


import sys

from direct.showbase.ShowBase import ShowBase
from direct.showbase.InputStateGlobal import inputState

from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import LVector3
from panda3d.core import LPoint3
from panda3d.core import TransformState
from panda3d.core import BitMask32
from panda3d.core import NodePath

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletSphereShape
from panda3d.bullet import BulletRigidBodyNode


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
        if not self.box:
            return

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

        self.box.set_active(True)
        self.box.apply_central_force(force)
        self.box.apply_torque(torque)

    def process_contacts(self):
        if not self.box or not self.sphere:
            return

        result = self.world.contact_test_pair(self.box, self.sphere)
        #result = self.world.contact_test(self.box)

        #print '-->', result.get_num_contacts()
        for contact in result.get_contacts():
            cp = contact.get_manifold_point()
            node0 = contact.get_node0()
            node1 = contact.get_node1()
            print(node0.get_name(), node1.get_name(), cp.get_local_point_a())

            #print(contact.get_node0(), cp.get_position_world_on_a())
            #print(contact.get_idx0(), contact.get_idx1(), \
            #      contact.get_part_id0(), contact.get_part_id1())

            self.remove_node(node1)

    def remove_node(self, node):
        self.world.remove(node)
        if node == self.sphere: self.sphere = None
        if node == self.box: self.box = None
        np = NodePath(node)
        np.remove_node()

    def update(self, task):
        dt = globalClock.get_dt()
        self.process_input(dt)
        self.world.do_physics(dt, 10, 0.008)
        self.process_contacts()
        return task.cont

    def cleanup(self):
        self.world = None
        self.worldNP.remove_node()

    #def on_contact_added(self, node1, node2):
    #    print('contact added:', node1, node2)

    #def on_contact_destroyed(self, node1, node2):
    #    print('contact destroyed:', node1, node2)

    def setup(self):
        self.worldNP = render.attach_new_node('World')

        # World
        self.debugNP = self.worldNP.attach_new_node(BulletDebugNode('Debug'))
        self.debugNP.show()

        self.world = BulletWorld()
        self.world.set_gravity((0, 0, -9.81))
        self.world.set_debug_node(self.debugNP.node())

        # Plane
        shape = BulletPlaneShape((0, 0, 1), 0)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Ground'))
        np.node().add_shape(shape)
        np.set_pos(0, 0, -1)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        # Box
        shape = BulletBoxShape((0.5, 0.5, 0.5))

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Box'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.node().set_deactivation_enabled(False)
        np.set_pos(2, 0, 4)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        visualNP = loader.load_model('models/box.egg')
        visualNP.reparent_to(np)

        self.box = np.node()

        # Sphere
        shape = BulletSphereShape(0.6)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Sphere'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(-2, 0, 4)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        self.sphere = np.node()

        ## Enable contact reporting
        #self.accept('bullet-contact-added', self.on_contact_added)
        #self.accept('bullet-contact-destroyed', self.on_contact_destroyed)
        #self.box.notify_collisions(True)


game = Game()
game.run()
