#!/usr/bin/env python

# Author: Epihaius
# Date: 2019-06-21
#
# This is a medium-difficulty example of how to procedurally create a simple square.
# It not only shows how to use the available Panda3D classes and methods
# to turn vertex data into renderable geometry, but also visually helps
# to understand the importance of vertex winding order for backface culling.
#
# This example uses memoryviews to add the geometry data.

from panda3d.core import *
from direct.showbase.ShowBase import ShowBase
from direct.gui.DirectGui import *
from direct.interval.IntervalGlobal import *
from src.gui import *
from src.nav import NavigationManager
from src.helpers import create_arrow, create_round_arrow
import array
import struct


class Demo:

    def __init__(self):

        self.showbase = showbase = ShowBase()
        showbase.disable_mouse()

        gui = GUI(showbase)
        NavigationManager(showbase, gui_mouse_watcher=gui.mouse_watcher)

        # Build the GUI layout

        title = "Panda3D: Tutorial - Generating a square procedurally"
        label = Label(gui.window, 10, title)
        borders = (10, 10, 20, 10)
        gui.add(label, expand=True, borders=borders)

        sizer = Sizer("vertical")
        borders = (10, 10, 10, 10)
        gui.add(sizer, borders=borders)
        borders = (0, 0, 10, 0)
        command = self.__toggle_helpers
        text = "Toggle winding order display"
        button = Button(gui.window, 10, text, command=command)
        sizer.add(button, expand=True, borders=borders)
        command = self.__toggle_rotation
        text = "Toggle arrow rotation"
        button = Button(gui.window, 10, text, command=command)
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

        # generate square geometry
        square = self.__generate_square()
        # create a pivot node to attach the square and the helper objects to
        # (attaching the helper objects to the square would make them inherit
        # the render state of the square, which is not wanted)
        pivot = showbase.render.attach_new_node("pivot")
        square.reparent_to(pivot)
        # create a root node for the helper objects to easily toggle their display
        self.viz_root = viz_root = pivot.attach_new_node("viz_root")
        points = square.copy_to(viz_root)
        points.node().modify_geom(0).make_points_in_place()
        points.set_render_mode_thickness(10)
        points.set_color(1., 1., 0.)
        lines = square.copy_to(pivot)
        lines.node().modify_geom(0).make_lines_in_place()
        lines.set_render_mode_thickness(3)
        lines.set_color(1., 1., 1.)
        square.set_light(self.light)

        # create helper objects to show the correct winding order of the
        # vertices: counter-clockwise (indicated by the round arrow) when
        # looking straight at the side of the square that needs to be rendered
        # (i.e. in the direction of the other arrow)
        arrow = create_arrow()
        arrow.reparent_to(viz_root)
        arrow.set_scale(.1)
        arrow.set_hpr(0., -90., 0.)
        arrow.set_pos(0., -2., .5)
        arrow.set_color(1., 1., 0.)
        arrow = create_round_arrow()
        arrow.reparent_to(viz_root)
        arrow.set_scale(1.5)
        arrow.set_color(1., 1., 0.)
        arrow.set_render_mode_thickness(3)
        self.arrow_rotation = arrow.hprInterval(3., (0., 0., -360.))
        self.arrow_rotation.loop()

        # display info
        text = OnscreenText(text="This side is\ninvisible due to\nbackface culling",
                            fg=(0., 0., 0., 1.), scale=.27,
                            parent=pivot, align=TextNode.A_center)
        text.set_h(180.)
        text.set_pos(0., .01, .3)

        positions = ((-.3, -.01, -.5), (.3, -.01, .5))
        orders = ((0, 1, 2), (1, 3, 2))

        # display vertex index order
        for pos, order in zip(positions, orders):
            order_text = ", ".join(str(i) for i in order)
            text = OnscreenText(text="Index order:\n({})".format(order_text),
                                fg=(1., 1., 1., 1.), scale=.2,
                                parent=viz_root, align=TextNode.A_center)
            text.set_pos(pos)

        coords = ((-1.3, -1.3), (1.3, -1.3), (-1.3, 1.3), (1.3, 1.3))

        # display vertex indices
        for index, (x, z) in enumerate(coords):
            text = OnscreenText(text=str(index),
                                fg=(0., 0., 0., 1.), scale=.2,
                                parent=viz_root, align=TextNode.A_center)
            text.set_pos(x, 0., z)
            text.set_billboard_point_eye()

        showbase.run()

    def __generate_square(self):

        # for the sake of clarity, let's add the numerical data to a list of dicts;
        # the square is created in the XZ-plane, facing the negative Y-axis
        square_values = [
            {  # vertex 0: the lower-left-hand vertex
                "pos": (-1., 0., -1.),
                "color": (1., 0., 0., 1.),
                "uv": (0., 0.)
            },
            {  # vertex 1: the lower-right-hand vertex
                "pos": (1., 0., -1.),
                "color": (0., 1., 0., 1.),
                "uv": (1., 0.)
            },
            {  # vertex 2: the upper-left-hand vertex
                "pos": (-1., 0., 1.),
                "color": (0., 0., 1., 1.),
                "uv": (0., 1.)
            },
            {  # vertex 3: the upper-right-hand vertex
                "pos": (1., 0., 1.),
                "color": (1., 1., 1., 1.),
                "uv": (1., 1.)
            }
        ]
        # the normal is the same for all vertices; it points in the direction
        # of the negative Y-axis
        normal = (0., -1., 0.)

        # most of the time, all values added to a vertex data table are of type
        # float, so it is possible to add them to an array with format "f":
        # values = array.array("f", [])
        # however, in the general case it can happen that custom data columns
        # need to be added that are of a different type, e.g. int, and then an
        # array with format "B" (bytes) can be used, or simply a bytearray:
        values = bytearray()

        # define the triangles to be added to the GeomTriangles primitive;
        # add the indices of the vertices in counter-clockwise winding order,
        # looking straight at the side that needs to be rendered
        tri_vertex_indices = [
            0, 1, 2,  # the indices of the vertices defining the first triangle
            1, 3, 2   # the indices of the vertices defining the second triangle
        ]
        # the values added to a GeomPrimitive are indices and thus of type int:
        # (note that it is more efficient to use the "H" format, but this only
        # allows integers up to 65535; if higher values are needed, use the "I"
        # format)
        indices = array.array("H", tri_vertex_indices)

        # add the values to the array
        for index, vertex_values in enumerate(square_values):
            values.extend(struct.pack("3f", *vertex_values["pos"]))
            values.extend(struct.pack("3f", *normal))
#            values.extend(struct.pack("4B", *vertex_values["color"]))
            values.extend(struct.pack("4f", *vertex_values["color"]))
            values.extend(struct.pack("2f", *vertex_values["uv"]))
            values.extend(struct.pack("I", index))

        # instead of a predefined vertex array format, a custom format is used
        # that not only accommodates position, normal, color and texture
        # coordinate data, but also allows adding a unique index for each vertex,
        # e.g. for use with a shader
        array_format = GeomVertexArrayFormat()
        array_format.add_column(InternalName.make("vertex"), 3,
                                Geom.NT_float32, Geom.C_point)
        array_format.add_column(InternalName.make("normal"), 3,
                                Geom.NT_float32, Geom.C_normal)
#        array_format.add_column(InternalName.make("color"), 4,
#                                Geom.NT_uint8, Geom.C_color)
        array_format.add_column(InternalName.make("color"), 4,
                                Geom.NT_float32, Geom.C_color)
        array_format.add_column(InternalName.make("texcoord"), 2,
                                Geom.NT_float32, Geom.C_texcoord)
        # add a custom column (the name is arbitrarily chosen) for indices
        array_format.add_column(InternalName.make("index"), 1,
                                Geom.NT_int32, Geom.C_index)
        vertex_format = GeomVertexFormat()
        vertex_format.add_array(array_format)
        vertex_format = GeomVertexFormat.register_format(vertex_format)
        # create the vertex data table with the above format
        vertex_data = GeomVertexData("square", vertex_format, Geom.UH_static)

        # add the values to the vertex table using a memoryview;
        # since the size of a memoryview cannot change, the vertex data table
        # already needs to have the right amount of rows before creating
        # memoryviews from its array(s)
        vertex_data.unclean_set_num_rows(4)  # 4 vertices are needed
        # retrieve the data array, suitable for modification;
        # there is only a single data array in this example, so its index is zero
        data_array = vertex_data.modify_array(0)
        # if the values to be assigned to the memoryview slice were all of
        # type float, an additional cast to float would be necessary:
        # memview = memoryview(data_array).cast("B").cast("f")
        memview = memoryview(data_array).cast("B")
        memview[:] = values

        # create the triangle primitive; it defines which triangle is made up
        # of which vertices by using their indices in the vertex data table;
        # the order in which they are added determines which side of the
        # triangle will be rendered
        tris_prim = GeomTriangles(Geom.UH_static)
        # force the index type of the primitive to NT_uint32 if indices higher
        # than 65535 are needed (the default is NT_uint16):
        # tris_prim.set_index_type(Geom.NT_uint32)
        tris_array = tris_prim.modify_vertices()
        tris_array.unclean_set_num_rows(len(indices))
        # cast the memoryview to "I" instead of "H" if indices higher than 65535
        # are needed;
        # memview = memoryview(tris_array).cast("B").cast("I")
        memview = memoryview(tris_array).cast("B").cast("H")
        memview[:] = indices

        # create a Geom and add the primitive to it
        geom = Geom(vertex_data)
        geom.add_primitive(tris_prim)
        # finally, create a GeomNode, add the Geom to it and wrap it in a NodePath
        node = GeomNode("square")
        node.add_geom(geom)
        square = NodePath(node)

        return square

    def __toggle_helpers(self):

        if self.viz_root.is_hidden():
            self.viz_root.show()
        else:
            self.viz_root.hide()

    def __toggle_rotation(self):

        if self.arrow_rotation.is_playing():
            self.arrow_rotation.pause()
        else:
            self.arrow_rotation.resume()


demo = Demo()
