#!/usr/bin/env python

# Author: Epihaius
# Date: 2019-06-21
#
# This is an advanced example of how to procedurally create and edit a cube.
# In particular, it shows how to add and remove parts of a model at runtime,
# using low-level geometry manipulation.

from panda3d.core import *
from direct.showbase.ShowBase import ShowBase
from src.gui import *
from src.nav import NavigationManager
from src.textures import Textures
import array
import struct


class Demo:

    def __init__(self):

        self.showbase = showbase = ShowBase()
        showbase.disable_mouse()

        gui = GUI(showbase)
        NavigationManager(showbase, gui_mouse_watcher=gui.mouse_watcher)

        # Build the GUI layout

        title = "Panda3D: Tutorial - Generating and editing a cube procedurally"
        label = Label(gui.window, 10, title)
        borders = (10, 10, 20, 10)
        gui.add(label, expand=True, borders=borders)

        sizer = Sizer("vertical")
        borders = (10, 10, 10, 10)
        gui.add(sizer, borders=borders)
        borders = (0, 0, 10, 0)
        get_command = lambda index: lambda: self.__toggle_cube_side(index)
        side_indices = {"left": 0, "right": 3, "back": 1,
                        "front": 4, "bottom": 2, "top": 5}

        for side_id in ("left", "right", "back", "front", "bottom", "top"):
            command = get_command(side_indices[side_id])
            text = "Add or remove {} side".format(side_id)
            button = Button(gui.window, 10, text, command=command)
            sizer.add(button, expand=True, borders=borders)

        gui.add((0, 0), proportion=1.)

        text = "Orbit camera: left mouse button; zoom: mouse wheel"
        label = Label(gui.window, 6, text, text_alignment="left")
        borders = (10, 10, 10, 10)
        gui.add(label, expand=True, borders=borders)

        gui.finalize()

        # procedurally generate some textures
        Textures.create()

        # set up a light source
        light_node = PointLight("point_light")
        light_node.set_color((1., 1., 1., 1.))
        self.light = showbase.camera.attach_new_node(light_node)
        self.light.set_pos(5., -10., 7.)
        showbase.render.set_light(self.light)

        # generate cube geometry
        self.cube = cube = self.__generate_cube()
        cube.reparent_to(showbase.render)
        cube.set_texture(Textures.cube_map)
        # normally, rendering both sides of a triangle should be avoided, but
        # it is used here to make it easier to see what remains of the cube
        # when removing its sides
        cube.set_two_sided(True)

        # keep track of which sides of the cube are added
        self.cube_sides = list(range(6))

        showbase.run()

    def __generate_cube(self):

        vertex_count = 0
        # left=purple, back=green, bottom=blue, right=red, front=yellow, top=white
        colors = ((1., 0., 1.), (0., 1., 0.), (0., 0., 1.),
                  (1., 0., 0.), (1., 1., 0.), (1., 1., 1.))
        values = array.array("f", [])
        indices = array.array("H", [])
        # use an offset along the U-axis to give each side of the cube different
        # texture coordinates, such that each side shows a different part of a
        # pre-generated texture (Textures.cube_map)
        u_offset = 0.

        for direction in (-1, 1):

            for i in range(3):

                normal = VBase3()
                normal[i] = direction
                r, g, b = colors[i if direction == -1 else i - 3]
                color = (r, g, b, 1.)

                for a, b in ((-1., -1.), (-1., 1.), (1., 1.), (1., -1.)):

                    pos = Point3()
                    pos[i] = direction
                    pos[(i + direction) % 3] = a
                    pos[(i + direction * 2) % 3] = b
                    u, v = [pos[j] for j in range(3) if j != i]
                    u *= (-1. if i == 1 else 1.) * direction
                    uv = (max(0., u) / 6. + u_offset, max(0., v))

                    values.extend(pos)
                    values.extend(normal)
                    values.extend(color)
                    values.extend(uv)

                u_offset += 1. / 6.
                vertex_count += 4

                indices.extend((vertex_count - 2, vertex_count - 3, vertex_count - 4))
                indices.extend((vertex_count - 4, vertex_count - 1, vertex_count - 2))

        array_format = GeomVertexArrayFormat()
        array_format.add_column(InternalName.make("vertex"), 3,
                                Geom.NT_float32, Geom.C_point)
        array_format.add_column(InternalName.make("normal"), 3,
                                Geom.NT_float32, Geom.C_normal)
        array_format.add_column(InternalName.make("color"), 4,
                                Geom.NT_float32, Geom.C_color)
        array_format.add_column(InternalName.make("texcoord"), 2,
                                Geom.NT_float32, Geom.C_texcoord)
        vertex_format = GeomVertexFormat.register_format(array_format)

        vertex_data = GeomVertexData("cube_data", vertex_format, Geom.UH_static)
        vertex_data.unclean_set_num_rows(vertex_count)
        data_array = vertex_data.modify_array(0)
        memview = memoryview(data_array).cast("B").cast("f")
        memview[:] = values

        tris_prim = GeomTriangles(Geom.UH_static)
        tris_array = tris_prim.modify_vertices()
        tris_array.unclean_set_num_rows(len(indices))
        memview = memoryview(tris_array).cast("B").cast("H")
        memview[:] = indices

        geom = Geom(vertex_data)
        geom.add_primitive(tris_prim)
        node = GeomNode("cube")
        node.add_geom(geom)
        cube = NodePath(node)

        return cube

    def __add_cube_side(self, cube, index):

        # left=purple, back=green, bottom=blue, right=red, front=yellow, top=white
        colors = ((1., 0., 1.), (0., 1., 0.), (0., 0., 1.),
                  (1., 0., 0.), (1., 1., 0.), (1., 1., 1.))
        values = array.array("f", [])
        indices = array.array("H", [])

        direction = -1 if index < 3 else 1
        i = index if index < 3 else index - 3
        normal = VBase3()
        normal[i] = direction
        r, g, b = colors[i if direction == -1 else i - 3]
        color = (r, g, b, 1.)

        for a, b in ((-1., -1.), (-1., 1.), (1., 1.), (1., -1.)):

            pos = Point3()
            pos[i] = direction
            pos[(i + direction) % 3] = a
            pos[(i + direction * 2) % 3] = b
            u, v = [pos[j] for j in range(3) if j != i]
            u *= (-1. if i == 1 else 1.) * direction
            uv = (max(0., u) / 6. + index / 6., max(0., v))

            values.extend(pos)
            values.extend(normal)
            values.extend(color)
            values.extend(uv)

        geom = cube.node().modify_geom(0)
        vertex_data = geom.modify_vertex_data()
        old_count = vertex_data.get_num_rows()

        indices.extend((old_count + 2, old_count + 1, old_count))
        indices.extend((old_count, old_count + 3, old_count + 2))

        vertex_data.set_num_rows(old_count + 4)
        data_array = vertex_data.modify_array(0)
        memview = memoryview(data_array).cast("B").cast("f")
        memview[old_count * 12:] = values  # each vertex data row contains 12 floats

        tris_prim = geom.modify_primitive(0)
        tris_array = tris_prim.modify_vertices()
        old_count = tris_array.get_num_rows()
        tris_array.set_num_rows(old_count + 6)  # 2 triangles, thus 6 vertex indices
        memview = memoryview(tris_array).cast("B").cast("H")
        memview[old_count:] = indices

    def __remove_cube_side(self, cube, index):

        geom = cube.node().modify_geom(0)
        vertex_data = geom.modify_vertex_data()
        old_count = vertex_data.get_num_rows()
        start = index * 4 * 12
        end = (old_count - 4) * 12
        data_array = vertex_data.modify_array(0)
        memview = memoryview(data_array).cast("B").cast("f")
        memview[start:-4 * 12] = memview[start + 4 * 12:]
        vertex_data.set_num_rows(old_count - 4)

        tris_prim = geom.modify_primitive(0)
        old_count = tris_prim.get_num_vertices()
        start = index * 6
        tris_prim.offset_vertices(-4, start + 6, old_count)
        tris_array = tris_prim.modify_vertices()
        memview = memoryview(tris_array).cast("B").cast("H")
        memview[start:-6] = memview[start + 6:]
        tris_array.set_num_rows(old_count - 6)

    def __toggle_cube_side(self, index):

        if index in self.cube_sides:
            self.__remove_cube_side(self.cube, self.cube_sides.index(index))
            self.cube_sides.remove(index)
        else:
            self.__add_cube_side(self.cube, index)
            self.cube_sides.append(index)


demo = Demo()
