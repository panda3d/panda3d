#!/usr/bin/env python

# Author: Epihaius
# Date: 2019-06-21
#
# This is a basic example of how to procedurally create a simple square.
# It not only shows how to use the available Panda3D classes and methods
# to turn vertex data into renderable geometry, but also visually helps
# to understand the importance of vertex winding order for backface culling.
#
# This example uses the GeomVertexWriter class to add the geometry data.

from panda3d.core import *
from direct.showbase.ShowBase import ShowBase
from direct.gui.DirectGui import *
from direct.interval.IntervalGlobal import *
from src.gui import *
from src.nav import NavigationManager
from src.helpers import create_arrow, create_round_arrow


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

        # use a predefined vertex array format that accommodates position, normal,
        # color and texture coordinate data
        vertex_format = GeomVertexFormat.get_v3n3c4t2()
        # create the vertex data table with the above format
        vertex_data = GeomVertexData("square", vertex_format, Geom.UH_static)

        # create the various vertex attribute writers to add the corresponding
        # data to the vertex data table, row by row
        pos_writer = GeomVertexWriter(vertex_data, "vertex")
        normal_writer = GeomVertexWriter(vertex_data, "normal")
        col_writer = GeomVertexWriter(vertex_data, "color")
        uv_writer = GeomVertexWriter(vertex_data, "texcoord")

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

        # add the values to the vertex table
        for vertex_values in square_values:
            pos_writer.add_data3(vertex_values["pos"])
            normal_writer.add_data3(normal)
            col_writer.add_data4(vertex_values["color"])
            uv_writer.add_data2(vertex_values["uv"])

        # create the triangle primitive; it defines which triangle is made up
        # of which vertices by using their indices in the vertex data table;
        # the order in which they are added determines which side of the
        # triangle will be rendered
        tris_prim = GeomTriangles(Geom.UH_static)
        # preallocating memory for the rows in the vertex index table can make
        # adding the indices faster
        tris_prim.reserve_num_vertices(6)
        # add the indices of the vertices in counter-clockwise winding order,
        # looking straight at the side that needs to be rendered
        tris_prim.add_vertices(0, 1, 2)
        tris_prim.add_vertices(1, 3, 2)

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
