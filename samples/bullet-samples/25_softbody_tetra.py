#!/usr/bin/env python

import sys

from direct.showbase.ShowBase import ShowBase
from direct.showbase.InputStateGlobal import inputState

from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import LPoint3
from panda3d.core import TransformState
from panda3d.core import BitMask32
from panda3d.core import GeomNode
from panda3d.core import GeomVertexFormat

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletHelper
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletTriangleMesh
from panda3d.bullet import BulletTriangleMeshShape
from panda3d.bullet import BulletSoftBodyNode
from panda3d.bullet import BulletSoftBodyConfig


class Game(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        base.set_background_color(0.1, 0.1, 0.8, 1)
        base.set_frame_rate_meter(True)

        base.cam.set_pos(0, -40, 10)
        base.cam.look_at(0, 0, 0)

        # Light
        alight = AmbientLight('ambientLight')
        alight.set_color((0.5, 0.5, 0.5, 1))
        alightNP = render.attach_new_node(alight)

        dlight = DirectionalLight('directionalLight')
        dlight.set_direction((5, 0, -2))
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

    def update(self, task):
        dt = globalClock.get_dt()
        self.world.do_physics(dt, 10, 0.008)
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
        p0 = LPoint3(-20, -20, 0)
        p1 = LPoint3(-20, 20, 0)
        p2 = LPoint3(20, -20, 0)
        p3 = LPoint3(20, 20, 0)
        mesh = BulletTriangleMesh()
        mesh.add_triangle(p0, p1, p2)
        mesh.add_triangle(p1, p2, p3)
        shape = BulletTriangleMeshShape(mesh, dynamic=False)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Mesh'))
        np.node().add_shape(shape)
        np.set_pos(0, 0, -4)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        # Soft body world information
        info = self.world.get_world_info()
        info.set_air_density(1.2)
        info.set_water_density(0)
        info.set_water_offset(0)
        info.set_water_normal((0, 0, 0))

        ## Softbody - From points/indices
        #import cube
        #points = [LPoint3(x,y,z) * 3 for x,y,z in cube.nodes]
        #indices = sum([list(x) for x in cube.elements], [])

        #node = BulletSoftBodyNode.make_tet_mesh(info, points, indices, True)
        #node.set_volume_mass(300);
        #node.get_shape(0).set_margin(0.01)
        #node.get_material(0).set_linear_stiffness(0.8)
        #node.get_cfg().set_positions_solver_iterations(1)
        #node.get_cfg().clear_all_collision_flags()
        #node.get_cfg().set_collision_flag(
            #BulletSoftBodyConfig.CF_cluster_soft_soft, True)
        #node.get_cfg().set_collision_flag(
            #BulletSoftBodyConfig.CF_cluster_rigid_soft, True)
        #node.generate_clusters(16)

        #softNP = self.worldNP.attach_new_node(node)
        #softNP.set_pos(0, 0, 8)
        #softNP.set_hpr(0, 0, 45)
        #self.world.attach(node)

        # Softbody - From tetgen data
        ele = open('models/cube/cube.1.ele', 'r').read()
        face = open('models/cube/cube.1.face', 'r').read()
        node = open('models/cube/cube.1.node', 'r').read()

        node = BulletSoftBodyNode.make_tet_mesh(info, ele, face, node)
        node.set_name('Tetra')
        node.set_volume_mass(300)
        node.get_shape(0).set_margin(0.01)
        node.get_material(0).set_linear_stiffness(0.1)
        node.get_cfg().set_positions_solver_iterations(1)
        node.get_cfg().clear_all_collision_flags()
        node.get_cfg().set_collision_flag(
            BulletSoftBodyConfig.CF_cluster_soft_soft, True)
        node.get_cfg().setCollisionFlag(
            BulletSoftBodyConfig.CF_cluster_rigid_soft, True)
        node.generate_clusters(6)

        softNP = self.worldNP.attach_new_node(node)
        softNP.set_pos(0, 0, 8)
        softNP.set_hpr(45, 0, 0)
        self.world.attach(node)

        # Option 1:
        visNP = loader.load_model('models/cube/cube.egg')
        visNP.reparent_to(softNP)
        geom = (visNP.findAllMatches('**/+GeomNode').
                getPath(0).node().modifyGeom(0))
        node.link_geom(geom)

        # Option 2:
        #geom = BulletHelper.make_geom_from_faces(node)
        #visNode = GeomNode('TetraVisual')
        #visNode.add_geom(geom)
        #visNP = softNP.attach_new_node(visNode)
        #node.link_geom(geom)


game = Game()
game.run()
