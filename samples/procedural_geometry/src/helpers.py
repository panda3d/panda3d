# Author: Epihaius
# Date: 2019-06-21
#
# This module contains helper objects for various purposes.

from panda3d.core import *
from direct.interval.IntervalGlobal import *
from src.prim import *
from array import array
from math import pi, sin, cos


def create_arrow():

    values = array("f", [])
    indices = array("H", [])
    delta_angle = pi / 1.5
    shaft_length = 10.
    head_radius = 2.  # shaft radius equals 1.
    head_length = 3.

    for i in range(3):
        angle = delta_angle * i
        x = cos(angle)
        y = sin(angle)
        # bottom shaft vertex
        values.extend((x, y, 0.))
        # top shaft vertex
        values.extend((x, y, shaft_length))
        # bottom head vertex
        values.extend((x * head_radius, y * head_radius, shaft_length))

    # top head vertex
    values.extend((0., 0., shaft_length + head_length))
    top_index = len(values) // 3 - 1

    indices.extend((0, 6, 3))

    for i in range(2):
        j = i * 3
        indices.extend((j, j + 3, j + 1))
        indices.extend((j + 3, j + 4, j + 1))
        indices.extend((j + 1, j + 4, j + 2))
        indices.extend((j + 4, j + 5, j + 2))
        indices.extend((j + 2, j + 5, top_index))

    indices.extend((6, 0, 7))
    indices.extend((0, 1, 7))
    indices.extend((7, 1, 8))
    indices.extend((1, 2, 8))
    indices.extend((8, 2, top_index))

    # use a predefined vertex array format that accommodates just position data
    vertex_format = GeomVertexFormat.get_v3()
    vertex_data = GeomVertexData("arrow", vertex_format, Geom.UH_static)
    pos_array = vertex_data.modify_array(0)
    pos_array.unclean_set_num_rows(len(values) // 3)
    memview = memoryview(pos_array).cast("B").cast("f")
    memview[:] = values

    tris_prim = GeomTriangles(Geom.UH_static)
    tris_array = tris_prim.modify_vertices()
    tris_array.unclean_set_num_rows(len(indices))
    memview = memoryview(tris_array).cast("B").cast("H")
    memview[:] = indices

    geom = Geom(vertex_data)
    geom.add_primitive(tris_prim)
    node = GeomNode("arrow")
    node.add_geom(geom)
    arrow = NodePath(node)

    return arrow


def create_round_arrow():

    values = array("f", [])
    indices = array("H", [])
    delta_angle = pi * .02

    # circle arc vertices
    for i in range(95):
        angle = delta_angle * i
        values.extend((cos(angle), 0., -sin(angle)))

    # arrow head vertices
    values.extend((.9, 0., -.1))
    values.extend((1.1, 0., -.1))

    for i in range(94):
        indices.extend((i, i + 1))

    # connect arrow head vertices to circle arc start vertex
    indices.extend((95, 0))
    indices.extend((96, 0))

    # use a predefined vertex array format that accommodates just position data
    vertex_format = GeomVertexFormat.get_v3()
    vertex_data = GeomVertexData("arrow", vertex_format, Geom.UH_static)
    pos_array = vertex_data.modify_array(0)
    pos_array.unclean_set_num_rows(len(values) // 3)
    memview = memoryview(pos_array).cast("B").cast("f")
    memview[:] = values

    lines_prim = GeomLines(Geom.UH_static)
    lines_array = lines_prim.modify_vertices()
    lines_array.unclean_set_num_rows(len(indices))
    memview = memoryview(lines_array).cast("B").cast("H")
    memview[:] = indices

    geom = Geom(vertex_data)
    geom.add_primitive(lines_prim)
    node = GeomNode("arrow")
    node.add_geom(geom)
    arrow = NodePath(node)

    return arrow


# A laser model is used to showcase the construction of a model primitive over time.
class Laser:

    def __init__(self):

        # create ring to attach the laser to; it will rotate the laser along
        # the surface it gets mounted to
        ring_radius = 2.6
        section_radius = .1
        segments = {"ring": 60, "section": 6}
        model_maker = TorusMaker(None, ring_radius, section_radius, segments)
        self.ring = ring = NodePath(model_maker.generate())
        radius = .08
        height = .5
        segments = {"circular": 10, "axial": 1, "bottom_cap": 0, "top_cap": 0}
        # use a CylinderMaker to generate a connector between the ring and the
        # pivot of the laser
        model_maker = CylinderMaker(None, (0., 0., -height), radius, segments)
        connector = ring.attach_new_node(model_maker.generate())
        connector.set_y(-ring_radius)
        # use the same CylinderMaker and change some of its properties to
        # generate the laser pivot
        height = .25
        model_maker.bottom_center = (height * .5, 0., 0.)
        model_maker.top_center = (-height * .5, 0., 0.)
        model_maker.radius = .25
        model_maker.segments = {"circular": 20, "axial": 1}
        self.pivot = pivot = connector.attach_new_node(model_maker.generate())
        pivot.set_pos(0., .1, -.5)
        pivot.set_p(-90.)
        # again use the same CylinderMaker to generate the main body of the laser
        height = .205
        model_maker.bottom_center = (height * .5, 0., 0.)
        model_maker.top_center = (-height * .5, 0., 0.)
        model_maker.radius = .2
        model_maker.segments = {"circular": 30, "axial": 1, "slice_caps_radial": 0}
        model_maker.slice = 180.
        model_maker.rotation = 180.
        body = pivot.attach_new_node(model_maker.generate())
        vertex_data = body.node().modify_geom(0).modify_vertex_data()
        # stretch the sliced half of the body cylinder by scaling its
        # vertices by 4 in the Y-direction
        mat = Mat4.scale_mat(1., 4., 1.)
        vertex_data.transform_vertices(mat)
        # reset all cylinder properties to their defaults and then set custom
        # values for some of them to generate the shaft for the laser beam
        model_maker.reset()
        model_maker.top_center = (0., 1., 0.)
        model_maker.radius = .075
        model_maker.segments = {"circular": 20, "axial": 1, "bottom_cap": 0}
        shaft = body.attach_new_node(model_maker.generate())
        # adjust the properties to generate the laser nozzle
        model_maker.top_center = (0., .1, 0.)
        model_maker.radius = .03
        model_maker.segments = {"circular": 16, "axial": 1, "bottom_cap": 0}
        nozzle = shaft.attach_new_node(model_maker.generate())
        nozzle.set_y(1.)
        # finally, adjust the properties to generate the laser beam itself
        model_maker.top_center = (0., 1., 0.)
        model_maker.radius = .02
        model_maker.segments = {"circular": 6, "axial": 1, "bottom_cap": 0,
                                "top_cap": 0}
        self.beam = beam = nozzle.attach_new_node(model_maker.generate())
        beam.set_sy(.01)
        beam.set_light_off()
        beam.set_texture_off()
        beam.set_color(.5, .75, 1.)

    def get_ring(self):

        return self.ring

    def get_pivot(self):

        return self.pivot

    def get_beam(self):

        return self.beam

    def stop(self):

        lerp = LerpHprInterval(self.pivot, .5, (0., -90., 0.))
        lerp.start()
        self.beam.set_sy(.01)
