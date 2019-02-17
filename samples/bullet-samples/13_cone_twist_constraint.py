#!/usr/bin/env python

import sys

from direct.showbase.ShowBase import ShowBase
from direct.showbase.InputStateGlobal import inputState

from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import LPoint3
from panda3d.core import TransformState
from panda3d.core import BitMask32

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletSphereShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletConeTwistConstraint
from panda3d.bullet import BulletDebugNode


class Game(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        base.set_background_color(0.1, 0.1, 0.8, 1)
        base.set_frame_rate_meter(True)

        base.cam.set_pos(0, -20, 5)
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

        self.accept('enter', self.do_shoot)

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

    def do_shoot(self):
        # Get from/to points from mouse click
        pMouse = base.mouseWatcherNode.get_mouse()
        pFrom = LPoint3()
        pTo = LPoint3()
        base.camLens.extrude(pMouse, pFrom, pTo)

        pFrom = render.get_relative_point(base.cam, pFrom)
        pTo = render.get_relative_point(base.cam, pTo)

        # Calculate initial velocity
        v = pTo - pFrom
        v.normalize()
        v *= 100.0

        # Create bullet
        shape = BulletSphereShape(0.3)
        body = BulletRigidBodyNode('Bullet')
        bodyNP = self.worldNP.attach_new_node(body)
        bodyNP.node().add_shape(shape)
        bodyNP.node().set_mass(1.0)
        bodyNP.node().set_linear_velocity(v)
        bodyNP.node().set_ccd_motion_threshold(1e-7);
        bodyNP.node().set_ccd_swept_sphere_radius(0.50);
        bodyNP.set_collide_mask(BitMask32.all_on())
        bodyNP.set_pos(pFrom)

        visNP = loader.load_model('models/ball.egg')
        visNP.set_scale(0.8)
        visNP.reparent_to(bodyNP)

        self.world.attach(bodyNP.node())

        # Remove the bullet again after 2 seconds
        taskMgr.do_method_later(2, self.do_remove, 'doRemove',
            extraArgs=[bodyNP], appendTask=True)

    def do_remove(self, bodyNP, task):
        self.world.remove(bodyNP.node())
        bodyNP.remove_node()
        return task.done

    def update(self, task):
        dt = globalClock.get_dt()
        self.world.do_physics(dt, 20, 1.0/180.0)
        return task.cont

    def cleanup(self):
        self.worldNP.remove_node()
        self.worldNP = None
        self.world = None

    def setup(self):
        self.worldNP = render.attach_new_node('World')

        # World
        self.debugNP = self.worldNP.attach_new_node(BulletDebugNode('Debug'))
        self.debugNP.show()
        self.debugNP.node().show_wireframe(True)
        self.debugNP.node().show_constraints(True)
        self.debugNP.node().show_bounding_boxes(False)
        self.debugNP.node().show_normals(False)

        self.world = BulletWorld()
        self.world.set_gravity((0, 0, -9.81))
        self.world.set_debug_node(self.debugNP.node())

        # Box A
        shape = BulletBoxShape((0.5, 0.5, 0.5))

        bodyA = BulletRigidBodyNode('Box A')
        bodyNP = self.worldNP.attach_new_node(bodyA)
        bodyNP.node().add_shape(shape)
        bodyNP.set_collide_mask(BitMask32.all_on())
        bodyNP.set_pos(-2, 0, 4)

        visNP = loader.load_model('models/box.egg')
        visNP.clear_model_nodes()
        visNP.reparent_to(bodyNP)

        self.world.attach(bodyA)

        # Box B
        shape = BulletBoxShape((0.5, 0.5, 0.5))

        bodyB = BulletRigidBodyNode('Box B')
        bodyNP = self.worldNP.attach_new_node(bodyB)
        bodyNP.node().add_shape(shape)
        bodyNP.node().set_mass(1.0)
        bodyNP.node().set_deactivation_enabled(False)
        bodyNP.set_collide_mask(BitMask32.all_on())
        bodyNP.set_pos(0, 0, 0)

        visNP = loader.load_model('models/box.egg')
        visNP.clear_model_nodes()
        visNP.reparent_to(bodyNP)

        self.world.attach(bodyB)

        # Cone
        frameA = TransformState.make_pos_hpr((0, 0, -2), (0, 0, 90))
        frameB = TransformState.make_pos_hpr((-5, 0, 0), (0, 0, 0))

        cone = BulletConeTwistConstraint(bodyA, bodyB, frameA, frameB)
        cone.set_debug_draw_size(2.0)
        cone.set_limit(30, 45, 170, softness=1.0, bias=0.3, relaxation=8.0)
        self.world.attach(cone)


game = Game()
game.run()
