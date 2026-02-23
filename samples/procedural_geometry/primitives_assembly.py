#!/usr/bin/env python

# Author: Epihaius
# Date: 2019-06-21
#
# This demo showcases procedural generation over time of a variety of
# 3D primitive model types, such as boxes, spheres and cylinders.
# The actual geometry generation code can be found in the src/prim subpackage.

from panda3d.core import *
from direct.showbase.ShowBase import ShowBase
from direct.interval.IntervalGlobal import *
from src.gui import *
from src.nav import NavigationManager
from src.helpers import Laser
from src.prim import *
from src.textures import Textures


class Demo:

    def __init__(self):

        self.showbase = showbase = ShowBase()
        showbase.disable_mouse()

        gui = GUI(showbase)
        NavigationManager(showbase, Point3(0., 0., 2.), gui.mouse_watcher)

        # Build the GUI layout

        title = "Panda3D: Showcase - Assembling a primitive procedurally"
        label = Label(gui.window, 10, title)
        borders = (10, 10, 50, 10)
        gui.add(label, expand=True, borders=borders)

        sizer = Sizer("vertical")
        borders = (10, 10, 10, 10)
        gui.add(sizer, borders=borders)
        borders = (0, 0, 10, 0)
        command = lambda: self.__prepare_primitive_construction("box")
        button = Button(gui.window, 10, "Start box construction", command=command)
        sizer.add(button, expand=True, borders=borders)
        command = lambda: self.__prepare_primitive_construction("sphere")
        button = Button(gui.window, 10, "Start sphere construction", command=command)
        sizer.add(button, expand=True, borders=borders)
        command = lambda: self.__prepare_primitive_construction("cylinder")
        button = Button(gui.window, 10, "Start cylinder construction", command=command)
        sizer.add(button, expand=True, borders=borders)
        command = lambda: self.__prepare_primitive_construction("cone")
        button = Button(gui.window, 10, "Start cone construction", command=command)
        sizer.add(button, expand=True, borders=borders)
        command = lambda: self.__prepare_primitive_construction("torus")
        button = Button(gui.window, 10, "Start torus construction", command=command)
        sizer.add(button, expand=True)

        gui.add((0, 0), proportion=1.)

        sizer = Sizer("horizontal")
        borders = (10, 10, 10, 10)
        gui.add(sizer, alignment="right", borders=borders)
        command = self.__toggle_primitive_construction
        button = Button(gui.window, 10, "Toggle construction (<Space>)", command=command)
        sizer.add(button)
        borders = (10, 0, 0, 0)
        command = self.__start_model_disintegration
        button = Button(gui.window, 10, "Disintegrate model (<Del>)", command=command)
        sizer.add(button, borders=borders)

        text = "Orbit camera: left mouse button; zoom: mouse wheel"
        label = Label(gui.window, 6, text, text_alignment="left")
        borders = (10, 10, 10, 10)
        gui.add(label, expand=True, borders=borders)

        gui.finalize()

        # procedurally generate some textures
        Textures.create()
        tex = Textures.hull_tex

        # set up a light source
        light_node = PointLight("point_light")
        light_node.set_color((1., 1., 1., 1.))
        self.light = showbase.camera.attach_new_node(light_node)
        self.light.set_pos(5., -10., 7.)
        showbase.render.set_light(self.light)

        # create a construction platform using a cone without caps, so the user can
        # see the 3D primitive being constructed through the platform from below
        radius = 3.
        height = .2
        segments = {"circular": 40, "axial": 1, "bottom_cap": 0, "top_cap": 0}
        model_maker = ConeMaker((0., 0., -height), (0., 0., 0.), radius + .2, radius, segments)
        platform = showbase.render.attach_new_node(model_maker.generate())
        platform.set_color((.6, .6, .7, 1.))
        platform.set_texture(tex)

        # create cylindrical construction chamber
        radius = 3.
        height = 4.
        segments = {"circular": 40, "axial": 10}
        model_maker = CylinderMaker(None, (0., 0., height), radius, segments, inverted=True)
        self.chamber = chamber = platform.attach_new_node(model_maker.generate())

        # create laser
        self.laser = Laser()
        ring = self.laser.get_ring()
        ring.reparent_to(chamber)
        ring.set_z(4.)
        ring.set_color((1., 1., 1., 1.))

        # create dome for construction chamber
        radius = 3.
        segments = {"horizontal": 40, "vertical": 10, "bottom_cap": 0}
        model_maker = SphereMaker(None, radius, segments, bottom_clip=0., inverted=True)
        dome = chamber.attach_new_node(model_maker.generate())
        dome.set_z(4.)
        dome.set_sz(.5)

        # create a pivot node to attach the primitive to
        self.pivot = pivot = showbase.render.attach_new_node("pivot")
        pivot.set_z(2.)
        pivot.set_color((.4, .4, .4, 1.))
        pivot.set_texture(tex)
        # initialize primitive model and wireframe nodes
        self.prim = NodePath("primitive")
        self.prim_wire = NodePath("wireframe")
        # initialize generator functions to be called in a task
        self.gen = None
        self.gen_wire = None
        # initialize flags to indicate construction status
        self.new_construction_allowed = True
        self.disintegration_allowed = False
        self.construction_paused = False

        showbase.accept("space", self.__toggle_primitive_construction)
        showbase.accept("delete", self.__start_model_disintegration)

        showbase.run()

    def __aim_laser(self, coords):

        chamber = self.chamber
        prim_wire = self.prim_wire
        laser = self.laser
        ring = laser.get_ring()
        points = [Point3(*xyz) for xyz in coords]
        center = sum(points, Point3()) / len(points)
        center = chamber.get_relative_point(prim_wire, center)
        vec = Vec3(center)
        vec.z = 0.

        if vec.normalize():
            quat = Quat()
            look_at(quat, -vec, Vec3.up())
            h, p, r = quat.get_hpr()
            ring.set_h(h)

        pivot = laser.get_pivot()
        vec = center - pivot.get_pos(chamber)

        if vec.normalize():
            quat = Quat()
            look_at(quat, vec, Vec3.up())
            h, p, r = quat.get_hpr()
            pivot.set_p(p)
            beam = laser.get_beam()
            vec = center - beam.get_pos(chamber)
            beam.set_sy(vec.length())

    def __build_wire_model(self, task=None):

        r = next(self.gen_wire)

        if r is False:
            self.laser.stop()
            self.gen_wire = None
            return

        if r:
            value, val_type = r
            if val_type == "model":
                self.prim_wire, coords = value
                self.prim_wire.set_light_off()
                self.prim_wire.set_texture_off()
                self.prim_wire.set_color((1., 1., 0., 1.))
                self.prim_wire.set_render_mode_wireframe()
                self.prim_wire.set_render_mode_thickness(3)
                self.prim_wire.reparent_to(self.pivot)
                self.__aim_laser(coords)
            elif val_type == "poly_points":
                self.__aim_laser(value)

        if task:
            return task.again

    def __build_model(self, task):

        r = next(self.gen)

        if r is False:
            self.gen = None
            self.prim_wire.set_transparency(TransparencyAttrib.M_alpha)
            lerp = LerpColorInterval(self.prim_wire, 2., (0., 0., 0., 0.))
            lerp.start()
            return

        if r:
            value, val_type = r
            if val_type == "model":
                self.prim, _ = value
                self.prim.reparent_to(self.pivot)
                self.disintegration_allowed = True
                self.new_construction_allowed = True

        return task.again

    def __check_model_state(self, prim_type, task):

        if self.new_construction_allowed:
            self.__start_primitive_construction(prim_type)
            return

        return task.cont

    def __disintegrate_model(self, task):

        if not disintegrate_model(self.prim):
            self.prim.detach_node()
            self.prim_wire.detach_node()
            self.new_construction_allowed = True
            return

        return task.cont

    def __start_model_disintegration(self):

        if not self.disintegration_allowed:
            return False

        self.showbase.task_mgr.remove("build_model")
        self.showbase.task_mgr.remove("build_wire_model")
        self.gen = None
        self.gen_wire = None
        self.laser.stop()
        self.prim_wire.set_transparency(TransparencyAttrib.M_alpha)
        lerp = LerpColorInterval(self.prim_wire, .5, (0., 0., 0., 0.))
        lerp.start()
        self.showbase.task_mgr.add(self.__disintegrate_model, "disintegrate_model")
        self.construction_paused = False
        self.disintegration_allowed = False
        self.new_construction_allowed = False

        return True

    def __toggle_primitive_construction(self):

        if not self.new_construction_allowed:
            return

        if self.construction_paused:
            self.showbase.task_mgr.do_method_later(.25, self.__build_model,
                                                   "build_model")
            self.showbase.task_mgr.do_method_later(.25, self.__build_wire_model,
                                                   "build_wire_model")
            self.construction_paused = False
        elif self.gen_wire:
            self.showbase.task_mgr.remove("build_model")
            self.showbase.task_mgr.remove("build_wire_model")
            self.laser.stop()
            self.construction_paused = True

    def __prepare_primitive_construction(self, prim_type):

        if not self.new_construction_allowed:
            return

        if self.prim.has_parent() and not self.__start_model_disintegration():
            return

        self.showbase.task_mgr.remove("check_model_state")
        self.showbase.task_mgr.add(self.__check_model_state, "check_model_state",
                                   extraArgs=[prim_type], appendTask=True, sort=1)

    def __start_primitive_construction(self, prim_type):

        if prim_type == "box":
            width = 3.
            depth = 1.5
            height = 1.
            segments = {"width": 7, "depth": 4, "height": 5}
            self.gen = build_box(width, depth, height, segments)
            self.gen_wire = build_box(width, depth, height, segments)
        elif prim_type == "cylinder":
            radius = 1.5
            height = 2.
            segments = {"circular": 20, "axial": 3, "caps": 3}
            self.gen = build_cylinder(radius, height, segments)
            self.gen_wire = build_cylinder(radius, height, segments)
        elif prim_type == "cone":
            bottom_radius = 1.5
            top_radius = .5
            height = 2.
            segments = {"circular": 20, "axial": 3, "caps": 2}
            self.gen = build_cone(bottom_radius, top_radius, height, segments)
            self.gen_wire = build_cone(bottom_radius, top_radius, height, segments)
        elif prim_type == "sphere":
            radius = 1.5
            segments = 21
            self.gen = build_sphere(radius, segments)
            self.gen_wire = build_sphere(radius, segments)
        elif prim_type == "torus":
            ring_radius = 1.2
            section_radius = .5
            segments = {"ring": 24, "section": 12}
            self.gen = build_torus(ring_radius, section_radius, segments)
            self.gen_wire = build_torus(ring_radius, section_radius, segments)

        self.__build_wire_model()
        self.showbase.task_mgr.do_method_later(.25, self.__build_model,
                                               "build_model")
        self.showbase.task_mgr.do_method_later(.25, self.__build_wire_model,
                                               "build_wire_model")
        self.new_construction_allowed = False


demo = Demo()
