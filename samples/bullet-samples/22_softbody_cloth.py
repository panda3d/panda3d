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
from panda3d.core import GeomVertexRewriter
from panda3d.core import InternalName
from panda3d.core import NurbsSurfaceEvaluator
from panda3d.core import SheetNode

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
        self.world.do_physics(dt, 10, 0.004)
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

        # Box
        shape = BulletBoxShape((1.0, 1.0, 1.0))

        boxNP = self.worldNP.attach_new_node(BulletRigidBodyNode('Box'))
        boxNP.node().set_mass(150.0)
        boxNP.node().add_shape(shape)
        boxNP.set_pos(0, 0, 2)
        boxNP.set_collide_mask(BitMask32.all_on())

        self.world.attach(boxNP.node())

        visualNP = loader.load_model('models/box.egg')
        visualNP.clear_model_nodes()
        visualNP.set_scale(2.0)
        visualNP.reparent_to(boxNP)

        # Soft body world information
        info = self.world.get_world_info()
        info.set_air_density(1.2)
        info.set_water_density(0)
        info.set_water_offset(0)
        info.set_water_normal((0, 0, 0))

        # Softbody
        nx = 31
        ny = 31

        p00 = LPoint3(-8, -8, 0)
        p10 = LPoint3( 8, -8, 0)
        p01 = LPoint3(-8,  8, 0)
        p11 = LPoint3( 8,  8, 0)
        bodyNode = BulletSoftBodyNode.make_patch(
            info, p00, p10, p01, p11, nx, ny, 1+2+4+8, True)

        material = bodyNode.append_material()
        material.set_linear_stiffness(0.4)
        #material.set_angular_stiffness(2.)
        bodyNode.generate_bending_constraints(2, material);
        bodyNode.set_total_mass(50.0)
        bodyNode.get_shape(0).set_margin(0.5)

        bodyNP = self.worldNP.attach_new_node(bodyNode)
        self.world.attach(bodyNode)

        # Rendering with Geom:
        fmt = GeomVertexFormat.get_v3n3t2()
        geom = BulletHelper.make_geom_from_faces(bodyNode, fmt, True)
        bodyNode.link_geom(geom)
        visNode = GeomNode('')
        visNode.add_geom(geom)
        visNP = bodyNP.attach_new_node(visNode)

        # Now we want to have a texture and texture coordinates.
        # The geom's format has already a column for texcoords, so we just need
        # to write texcoords using a GeomVertexRewriter.
        tex = loader.load_texture('models/panda.jpg')
        visNP.set_texture(tex)
        BulletHelper.make_texcoords_for_patch(geom, nx, ny)

        # Rendering with NURBS (please reduce nx & ny to 12):
        #surface = NurbsSurfaceEvaluator()
        #surface.reset(nx, ny)
        #bodyNode.link_surface(surface)

        #visNode = SheetNode('')
        #visNode.set_surface(surface)
        #visNode.set_num_u_subdiv(2)
        #visNode.set_num_v_subdiv(2)
        #visNP = self.worldNP.attach_new_node(visNode)
        #visNP.setTwoSided(True)


game = Game()
game.run()
