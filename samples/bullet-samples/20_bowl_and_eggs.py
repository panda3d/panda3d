#!/usr/bin/env python

import sys
import random

from direct.showbase.ShowBase import ShowBase
from direct.showbase.InputStateGlobal import inputState

from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import LVector3
from panda3d.core import TransformState
from panda3d.core import BitMask32

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletConvexHullShape
from panda3d.bullet import BulletTriangleMesh
from panda3d.bullet import BulletTriangleMeshShape
from panda3d.bullet import ZUp


class Game(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        base.set_background_color(0.1, 0.1, 0.8, 1)
        base.set_frame_rate_meter(True)

        base.cam.set_pos(0, -10, 5)
        base.cam.look_at(0, 0, 0.2)

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

        inputState.watchWithModifiers('up', 'w')
        inputState.watchWithModifiers('left', 'a')
        inputState.watchWithModifiers('down', 's')
        inputState.watchWithModifiers('right', 'd')

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

        if inputState.isSet('up'): force.y = 1.0
        if inputState.isSet('down'): force.y = -1.0
        if inputState.isSet('left'): force.x = -1.0
        if inputState.isSet('right'): force.x = 1.0

        force *= 300.0

        self.bowlNP.node().set_active(True)
        self.bowlNP.node().apply_central_force(force)

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
        self.debugNP.node().show_wireframe(True)
        self.debugNP.node().show_constraints(True)
        self.debugNP.node().show_bounding_boxes(False)
        self.debugNP.node().show_normals(False)

        self.world = BulletWorld()
        self.world.set_gravity((0, 0, -9.81))
        self.world.set_debug_node(self.debugNP.node())

        # Ground
        shape = BulletPlaneShape((0, 0, 1), 0)

        body = BulletRigidBodyNode('Ground')
        bodyNP = self.worldNP.attach_new_node(body)
        bodyNP.node().add_shape(shape)
        bodyNP.set_pos(0, 0, 0)
        bodyNP.set_collide_mask(BitMask32.all_on())
        self.world.attach(bodyNP.node())

        # Bowl
        visNP = loader.load_model('models/bowl.egg')

        geom = (visNP.findAllMatches('**/+GeomNode')
                .get_path(0).node().get_geom(0))
        mesh = BulletTriangleMesh()
        mesh.addGeom(geom)
        shape = BulletTriangleMeshShape(mesh, dynamic=True)

        body = BulletRigidBodyNode('Bowl')
        bodyNP = self.worldNP.attach_new_node(body)
        bodyNP.node().add_shape(shape)
        bodyNP.node().set_mass(10.0)
        bodyNP.set_pos(0, 0, 0)
        bodyNP.set_collide_mask(BitMask32.all_on())
        self.world.attach(bodyNP.node())

        visNP.reparent_to(bodyNP)

        self.bowlNP = bodyNP
        self.bowlNP.set_scale(2)

        # Eggs
        self.eggNPs = []
        for i in range(5):
            x = random.gauss(0, 0.1)
            y = random.gauss(0, 0.1)
            z = random.gauss(0, 0.1) + 1
            h = random.random() * 360
            p = random.random() * 360
            r = random.random() * 360

            visNP = loader.load_model('models/egg.egg')

            geom = (visNP.find_all_matches('**/+GeomNode')
                    .get_path(0).node().get_geom(0))
            shape = BulletConvexHullShape()
            shape.addGeom(geom)

            body = BulletRigidBodyNode('Egg-%i' % i)
            bodyNP = self.worldNP.attach_new_node(body)
            bodyNP.node().set_mass(1.0)
            bodyNP.node().add_shape(shape)
            bodyNP.node().set_deactivation_enabled(False)
            bodyNP.set_collide_mask(BitMask32.all_on())
            bodyNP.set_pos_hpr(x, y, z, h, p, r)
            #bodyNP.set_scale(1.5)
            self.world.attach(bodyNP.node())

            visNP.reparent_to(bodyNP)

            self.eggNPs.append(bodyNP)


game = Game()
game.run()
