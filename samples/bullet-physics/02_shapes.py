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

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletSphereShape
from panda3d.bullet import BulletCapsuleShape
from panda3d.bullet import BulletCylinderShape
from panda3d.bullet import BulletConeShape
from panda3d.bullet import BulletConvexHullShape
from panda3d.bullet import BulletTriangleMesh
from panda3d.bullet import BulletTriangleMeshShape
from panda3d.bullet import BulletMultiSphereShape
from panda3d.bullet import XUp
from panda3d.bullet import YUp
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
        #pairs = [(mf.getNode0().getName(),
        #          mf.getNode1().getName())
        #  for mf in self.world.getManifolds() if mf.getNumManifoldPoints() > 0]
        #print pairs
        return task.cont

    def cleanup(self):
        self.world = None
        self.worldNP.remove_node()

        #def do_added(self, node1, node2):
        #  print('added:', node1.getName(), node2.getName())

        #def do_destroyed(self, node1, node2):
        #  print('destroyed:', node1.getName(), node2.getName())

    def setup(self):
        self.worldNP = render.attach_new_node('World')

        # World
        self.debugNP = self.worldNP.attach_new_node(BulletDebugNode('Debug'))
        self.debugNP.show()
        self.debugNP.node().show_wireframe(True)
        self.debugNP.node().show_constraints(True)
        self.debugNP.node().show_bounding_boxes(False)
        self.debugNP.node().show_normals(True)

        self.world = BulletWorld()
        self.world.set_gravity((0, 0, -9.81))
        self.world.set_debug_node(self.debugNP.node())

        # Plane (static)
        shape = BulletPlaneShape((0, 0, 1), 0)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Ground'))
        np.node().add_shape(shape)
        np.set_pos(0, 0, -1)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        # Box (dynamic)
        shape = BulletBoxShape((0.5, 0.5, 0.5))

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Box'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(0, 0, 4)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        self.boxNP = np # For applying force & torque

        #np.node().notifyCollisions(True)
        #self.accept('bullet-contact-added', self.do_added)
        #self.accept('bullet-contact-destroyed', self.do_destroyed)

        # Sphere (dynamic)
        shape = BulletSphereShape(0.6)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Sphere'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(-4, 0, 4)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        # Cone (dynamic)
        shape = BulletConeShape(0.6, 1.2, ZUp)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Cone'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(4, 0, 4)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        # Capsule (dynamic)
        shape = BulletCapsuleShape(0.5, 1.0, ZUp)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Capsule'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(0, 4, 4)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        # Cyliner (dynamic)
        shape = BulletCylinderShape(0.7, 1.5, ZUp)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Cylinder'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(4, 4, 4)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        # Convex (dynamic)
        shape = BulletConvexHullShape()
        shape.add_point(LPoint3(1, 1, 2))
        shape.add_point(LPoint3(0, 0, 0))
        shape.add_point(LPoint3(2, 0, 0))
        shape.add_point(LPoint3(0, 2, 0))
        shape.add_point(LPoint3(2, 2, 0))

        # Another way to create the convex hull shape:
        #shape = BulletConvexHullShape()
        #shape.add_array([
        #  LPoint3(1, 1, 2),
        #  LPoint3(0, 0, 0),
        #  LPoint3(2, 0, 0),
        #  LPoint3(0, 2, 0),
        #  LPoint3(2, 2, 0),
        #])

        # Yet another way to create the convex hull shape:
        #geom = loader.load_model('models/box.egg')\
        #         .find_all_matches('**/+GeomNode')\
        #         .get_path(0)\
        #         .node()\
        #         .get_geom(0)
        #shape = BulletConvexHullShape()
        #shape.add_geom(geom)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Convex'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(-4, 4, 4)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        # Mesh (static)
        p0 = LPoint3(-10, -10, 0)
        p1 = LPoint3(-10, 10, 0)
        p2 = LPoint3(10, -10, 0)
        p3 = LPoint3(10, 10, 0)
        mesh = BulletTriangleMesh()
        mesh.add_triangle(p0, p1, p2)
        mesh.add_triangle(p1, p2, p3)
        shape = BulletTriangleMeshShape(mesh, dynamic=False)

        # Another way to create the triangle mesh shape:
        #geom = loader.load_model('models/box.egg')\
        #         .find_all_matches('**/+GeomNode')\
        #         .get_path(0)\
        #         .node()\
        #         .get_geom(0)
        #mesh = BulletTriangleMesh()
        #mesh.add_geom(geom)
        #shape = BulletTriangleMeshShape(mesh, dynamic=False)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Mesh'))
        np.node().add_shape(shape)
        np.set_pos(0, 0, 0.1)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        # MultiSphere
        points = [LPoint3(-1, 0, 0), LPoint3(0, 0, 0), LPoint3(1, 0, 0)]
        radii = [.4, .8, .6]
        shape = BulletMultiSphereShape(points, radii)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('MultiSphere'))
        np.node().set_mass(1.0)
        np.node().add_shape(shape)
        np.set_pos(8, 0, 4)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())


game = Game()
game.run()
