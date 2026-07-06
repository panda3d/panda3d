#!/usr/bin/env python

# Author: Epihaius
# Date: 2019-06-21
#
# This demo showcases gradual, procedural generation of a "vine", a cylinder-like
# object that randomly grows and branches over time, along the sides of a box.

from panda3d.core import *
from direct.showbase.ShowBase import ShowBase
from src.gui import *
from src.nav import NavigationManager
from src.prim import *
import random


class Vine:

    def __init__(self):

        self.thickness = .05
        self.segments = 6
        segs = {"horizontal": self.segments, "vertical": self.segments // 2,
                "bottom_cap": 0}
        model_maker = SphereMaker(None, self.thickness, segs, bottom_clip=0.)
        node = model_maker.generate()
        self.root = NodePath(node)
        geom = node.modify_geom(0)
        vertex_data = geom.modify_vertex_data()
        mat = Mat4.scale_mat(1., 1., 3.)
        vertex_data.transform_vertices(mat)
        self.end_vertex_count = vertex_data.get_num_rows()
        self.end_tri_count = geom.get_primitive(0).get_num_vertices()
        self.growth = .05  # length of growth each step
        self.branch_delay = 10  # number of growth steps until next child branch sprouts
        self.branch_countdown = self.branch_delay
        main_branch = {
            "length": 0,
            "end_vertex_row": self.end_vertex_count,
            "end_tri_row": self.end_tri_count,
            "box_side": "back",
            "prev_box_side": "back",
            "axis_vec": Vec3(0., -1., 0.),
            "growth_vec": Vec3(0., 0., self.growth),
            "end_center": Point3(),
            "growth_hpr": Mat4(Mat4.ident_mat()),
            "child_side": -1.  # alternates side of branch the next child
                               # branch will sprout from
        }
        self.branches = [main_branch]
        self.max_branch_count = 15
        self.max_branch_length = 800
        self.normals = {
            "back": Vec3(0., -1., 0.),
            "front": Vec3(0., 1., 0.),
            "left": Vec3(-1., 0., 0.),
            "right": Vec3(1., 0., 0.),
            "bottom": Vec3(0., 0., -1.),
            "top": Vec3(0., 0., 1.)
        }

    def get_root(self):

        return self.root

    def grow(self):

        self.branch_countdown -= 1

        if self.branch_countdown == 0:
            for branch in self.branches[:]:
                if len(self.branches) < self.max_branch_count:
                    self.__add_branch(branch)
            self.branch_countdown = self.branch_delay

        for i, branch in enumerate(self.branches):
            self.__grow_branch(branch, i)

    def __add_branch(self, parent_branch):

        branch = {
            "length": 0,
            "box_side": parent_branch["box_side"],
            "prev_box_side": parent_branch["prev_box_side"],
            "growth_vec": Vec3(parent_branch["growth_vec"]),
            "axis_vec": Vec3(parent_branch["axis_vec"]),
            "end_center": Point3(parent_branch["end_center"]),
            "child_side": -1.
        }
        self.branches.append(branch)
        angle = 60. * parent_branch["child_side"]
        parent_branch["child_side"] *= -1.
        rot_mat = Mat4.rotate_mat(angle, branch["axis_vec"])
        mat = parent_branch["growth_hpr"] * rot_mat
        branch["growth_vec"] = rot_mat.xform_vec(branch["growth_vec"])
        branch["growth_hpr"] = mat

        segs = {"horizontal": self.segments, "vertical": self.segments // 2,
                "bottom_cap": 0}
        model_maker = SphereMaker(None, self.thickness, segs, bottom_clip=0.)
        node = model_maker.generate()
        end = NodePath(node)
        geom = node.modify_geom(0)
        vertex_data = geom.modify_vertex_data()
        pos = branch["end_center"]
        mat = Mat4.scale_mat(1., 1., 3.) * mat * Mat4.translate_mat(pos)
        vertex_data.transform_vertices(mat)
        data_array = vertex_data.modify_array(0)
        child_view = memoryview(data_array).cast("B").cast("f")

        tris_prim = geom.get_primitive(0)
        tris_array = tris_prim.get_vertices()
        child_tris_view = memoryview(tris_array).cast("B").cast("H")

        geom = self.root.node().modify_geom(0)
        vertex_data = geom.modify_vertex_data()
        old_vert_count = vertex_data.get_num_rows()
        new_count = old_vert_count + self.end_vertex_count
        branch["end_vertex_row"] = new_count
        vertex_data.set_num_rows(new_count)
        data_array = vertex_data.modify_array(0)
        parent_view = memoryview(data_array).cast("B").cast("f")
        parent_view[old_vert_count * 8:] = child_view

        tris_prim = geom.modify_primitive(0)
        old_count = tris_prim.get_num_vertices()
        new_count = old_count + self.end_tri_count
        branch["end_tri_row"] = new_count
        tris_array = tris_prim.modify_vertices()
        tris_array.set_num_rows(new_count)
        parent_tris_view = memoryview(tris_array).cast("B").cast("H")
        parent_tris_view[-self.end_tri_count:] = child_tris_view
        tris_prim.offset_vertices(old_vert_count, old_count, new_count)

    def __grow_branch(self, branch, branch_index):

        if branch["length"] == self.max_branch_length:
            return

        segments = self.segments
        box_side = branch["box_side"]
        prev_box_side = branch["prev_box_side"]
        old_box_side = box_side

        x, y, z = center_point = branch["end_center"]
        vx, vy, vz = axis_vec = branch["axis_vec"]
        x_min = -1. - self.thickness
        x_max = 1. + self.thickness
        y_min = 0.
        y_max = 2. + self.thickness * 2.
        z_min = -self.thickness
        z_max = 2. + self.thickness
        d_max = self.thickness * 5.
        dx = max(0., x_min - x)
        turn_back = dx >= d_max
        dxl = 0. if prev_box_side == "left" and dx < d_max else dx
        dx = max(0., x - x_max)
        turn_back = turn_back or dx >= d_max
        dxr = 0. if prev_box_side == "right" and dx < d_max else dx
        dy = max(0., y_min - y)
        turn_back = turn_back or dy >= d_max
        dyb = 0. if prev_box_side == "back" and dy < d_max else dy
        dy = max(0., y - y_max)
        turn_back = turn_back or dy >= d_max
        dyf = 0. if prev_box_side == "front" and dy < d_max else dy
        dz = max(0., z_min - z)
        turn_back = turn_back or dz >= d_max
        dzb = 0. if prev_box_side == "bottom" and dz < d_max else dz
        dz = max(0., z - z_max)
        turn_back = turn_back or dz >= d_max
        dzt = 0. if prev_box_side == "top" and dz < d_max else dz

        if box_side in ("back", "front"):
            if max(dxl, dxr, dzb, dzt) > 0.:
                if max(dxl, dxr) > max(dzb, dzt):
                    if dxl > 0.:
                        box_side = "left"
                    else:
                        box_side = "right"
                elif dzb > 0.:
                    box_side = "bottom"
                else:
                    box_side = "top"
        elif box_side in ("left", "right"):
            if max(dyb, dyf, dzb, dzt) > 0.:
                if max(dyb, dyf) > max(dzb, dzt):
                    if dyb > 0.:
                        box_side = "back"
                    else:
                        box_side = "front"
                elif dzb > 0.:
                    box_side = "bottom"
                else:
                    box_side = "top"
        elif box_side in ("bottom", "top"):
            if max(dxl, dxr, dyb, dyf) > 0.:
                if max(dxl, dxr) > max(dyb, dyf):
                    if dxl > 0.:
                        box_side = "left"
                    else:
                        box_side = "right"
                elif dyb > 0.:
                    box_side = "back"
                else:
                    box_side = "front"

        normal = self.normals[box_side]

        if box_side != old_box_side:
            branch["prev_box_side"] = old_box_side

        # check if the current rotation axis and the side normal are
        # significantly different
        if axis_vec.dot(normal) < .9999:
            new_axis_vec = (axis_vec * .5 + normal * .5).normalized()
            old_normal = self.normals[branch["prev_box_side"]]
            branch["axis_vec"] = new_axis_vec
            cross_vec = old_normal.cross(normal)
            if cross_vec.normalize():
                angle = axis_vec.signed_angle_deg(new_axis_vec, cross_vec)
                rot_mat = Mat4.rotate_mat(angle, cross_vec)
            else:
                angle = 10. - random.random() * 20.
                rot_mat = Mat4.rotate_mat(angle, axis_vec)
        else:
            angle = 10. - random.random() * 20.
            rot_mat = Mat4.rotate_mat(angle, axis_vec)

        branch["growth_vec"] = rot_mat.xform_vec(branch["growth_vec"])
        growth_vec_n = Vec3(branch["growth_vec"].normalized())

        if turn_back:

            new_growth_vec = growth_vec_n * .5 + (Point3(0., 1., 1.) - center_point) * .5
            new_growth_vec.normalize()
            cross_vec = new_growth_vec.cross(growth_vec_n)

            if cross_vec.normalize():
                angle = growth_vec_n.signed_angle_deg(new_growth_vec, cross_vec)
                rot_mat = rot_mat * Mat4.rotate_mat(angle, cross_vec)
                branch["growth_vec"] = new_growth_vec * self.growth

        elif abs(branch["growth_vec"].normalized().dot(normal)) > .0001:

            proj_vec = Plane(normal, Point3()).project(growth_vec_n).normalized()
            new_growth_vec = growth_vec_n * .8 + proj_vec * .2
            cross_vec = new_growth_vec.cross(growth_vec_n)

            if cross_vec.normalize():
                new_growth_vec.normalize()
                angle = growth_vec_n.signed_angle_deg(new_growth_vec, cross_vec)
                rot_mat = rot_mat * Mat4.rotate_mat(angle, cross_vec)
                branch["growth_vec"] = new_growth_vec * self.growth

        branch["box_side"] = box_side

        mat = Mat4.translate_mat(-center_point)
        mat = mat * rot_mat
        point = center_point + branch["growth_vec"]
        mat = mat * Mat4.translate_mat(point)
        branch["end_center"] += branch["growth_vec"]
        branch["growth_hpr"] *= rot_mat

        geom = self.root.node().modify_geom(0)
        vertex_data = geom.modify_vertex_data()
        old_count = vertex_data.get_num_rows()
        end_vertex_row = branch["end_vertex_row"]
        new_vert_count = old_count + segments + 1
        vertex_data.set_num_rows(new_vert_count)
        data_array = vertex_data.modify_array(0)
        memview = memoryview(data_array).cast("B").cast("f")
        start = (end_vertex_row - self.end_vertex_count) * 8
        memview[start + (segments + 1) * 8:] = memview[start:old_count * 8]

        end = end_vertex_row + segments + 1
        vertex_data.transform_vertices(mat, end - self.end_vertex_count, end)

        end_tri_row = branch["end_tri_row"]
        tris_prim = geom.modify_primitive(0)
        old_count = tris_prim.get_num_vertices()
        new_count = old_count + segments * 6
        start = end_tri_row - self.end_tri_count

        if new_count < 2 ** 16:
            # use the default index format of the GeomPrimitive (16-bit) if
            # the number of vertices allows it...
            index_format = "H"
        else:
            # ...or force the GeomPrimitive to accept more indices by setting
            # its index format to 32-bit
            index_format = "I"
            tris_prim.set_index_type(Geom.NT_uint32)

        tris_array = tris_prim.modify_vertices()
        tris_array.set_num_rows(old_count + segments * 6)
        memview = memoryview(tris_array).cast("B").cast(index_format)
        memview[start + segments * 6:] = memview[start:old_count]
        tris_prim.offset_vertices(segments + 1, end_tri_row - self.end_tri_count
                                  + segments * 6, new_count)
        branch["length"] += 1

        for branch in self.branches[branch_index:]:
            branch["end_vertex_row"] += segments + 1
            branch["end_tri_row"] += segments * 6


class Demo:

    def __init__(self):

        self.showbase = showbase = ShowBase()
        showbase.disable_mouse()

        gui = GUI(showbase)
        NavigationManager(showbase, gui_mouse_watcher=gui.mouse_watcher)

        # Build the GUI layout

        title = "Panda3D: Showcase - Growing vines procedurally"
        label = Label(gui.window, 10, title)
        borders = (10, 10, 50, 10)
        gui.add(label, expand=True, borders=borders)

        sizer = Sizer("vertical")
        borders = (10, 10, 10, 10)
        gui.add(sizer, borders=borders)
        borders = (0, 0, 10, 0)
        command = self.__toggle_box_display
        button = Button(gui.window, 10, "Toggle box display", command=command)
        sizer.add(button, expand=True, borders=borders)
        command = self.__toggle_vine_growth
        button = Button(gui.window, 10, "Toggle vine growth", command=command)
        sizer.add(button, expand=True, borders=borders)
        command = self.__regrow_vine
        button = Button(gui.window, 10, "Regrow vine", command=command)
        sizer.add(button, expand=True)

        gui.add((0, 0), proportion=1.)

        text = "Orbit camera: left mouse button; zoom: mouse wheel"
        label = Label(gui.window, 6, text, text_alignment="left")
        borders = (10, 10, 10, 10)
        gui.add(label, expand=True, borders=borders)

        gui.finalize()

        # set up a light source
        light_node = PointLight("point_light")
        light_node.set_color((1., 1., 1., 1.))
        self.light = showbase.camera.attach_new_node(light_node)
        self.light.set_pos(5., -10., 7.)
        showbase.render.set_light(self.light)

        # create a box for the vine to grow around
        model_maker = BoxMaker(None, 2., 2., 2.)
        self.box = box = showbase.render.attach_new_node(model_maker.generate())
        box.set_color(.9, .7, .5)

        # create vine
        vine = Vine()
        vine_root = vine.get_root()
        vine_root.reparent_to(showbase.render)
        vine_root.set_pos(0., -1.05, -1.)
        vine_root.set_color(.4, .8, .5)
        self.vine = vine
        self.growing_vine = False

        self.__toggle_vine_growth()

        showbase.run()

    def __toggle_box_display(self):

        if self.box.is_hidden():
            self.box.show()
        else:
            self.box.hide()

    def __grow_vine(self, task):

        self.vine.grow()

        return task.again

    def __toggle_vine_growth(self):

        self.growing_vine = not self.growing_vine

        if self.growing_vine:
            self.showbase.task_mgr.do_method_later(.05, self.__grow_vine, "grow_vine")
        else:
            self.showbase.task_mgr.remove("grow_vine")

    def __regrow_vine(self):

        self.showbase.task_mgr.remove("grow_vine")
        self.vine.get_root().remove_node()
        self.vine = vine = Vine()
        vine_root = vine.get_root()
        vine_root.reparent_to(self.showbase.render)
        vine_root.set_pos(0., -1.05, -1.)
        vine_root.set_color(.4, .8, .5)
        self.vine = vine
        self.showbase.task_mgr.do_method_later(.05, self.__grow_vine, "grow_vine")
        self.growing_vine = True


demo = Demo()
