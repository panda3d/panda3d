# Author: Epihaius
# Date: 2019-06-21
#
# This module contains a class to procedurally create button widgets.

from panda3d.core import *
from .widget import Widget
from .font import Font
from ..prim import *
import struct


class Button(Widget):

    _model = None
    _corner_indices = None
    _font = Font("fonts/Arimo-Bold.ttf", 14., 18, 14, 22)

    @classmethod
    def __create_geometry(cls):

        # The buttons created by this class look like part of a box with
        # rounded edges.
        # It is built by first creating 2 standard primitives: a sliced
        # hemisphere and a sliced cylinder. Then, copies of these are made for
        # the different corners, resp. sides, and all of their geometry is
        # merged into a single GeomPrimitive.
        # While this is not an optimal way to create the desired shape, it
        # does make for an interesting exercise on geometry manipulation.

        # create the lower right hand corner as a sliced hemisphere
        segments = {"horizontal": 10, "vertical": 10,
                    "bottom_cap": 0, "slice_caps": 0}
        model_maker = SphereMaker(None, 1., segments, bottom_clip=0., slice=270.,
                                  has_uvs=False)
        corner_lr = model_maker.generate()
        vertex_data = corner_lr.modify_geom(0).modify_vertex_data()
        # the border geometry (corner sphere and edge cylinder, below) is
        # created without UV's, but the central rectangle of the button will
        # need them, so a new vertex format with a texcoord column is used,
        # which initializes the UV's to (0., 0.), and that's just fine for the
        # border geometry
        vertex_data.set_format(GeomVertexFormat.get_v3n3t2())
        cls._model = NodePath(corner_lr)
        # create the remaining corners as copies of the first one (their
        # vertices will be rotated when merging their geometry)
        corner_ur = corner_lr.make_copy()
        corner_ul = corner_lr.make_copy()
        corner_ll = corner_lr.make_copy()
        corners = (corner_ur, corner_ul, corner_ll)
        # create the bottom edge as a sliced cylinder
        segments = {"circular": 10, "bottom_cap": 0, "top_cap": 0,
                    "slice_caps_radial": 0}
        model_maker = CylinderMaker((1., 0., 0.), (0., 0., 0.), 1.,
                                    segments, slice=270., has_uvs=False)
        edge_b = model_maker.generate()
        geom = edge_b.modify_geom(0)
        vertex_data = geom.modify_vertex_data()
        vertex_data.set_format(GeomVertexFormat.get_v3n3t2())
        edge_vert_count = vertex_data.get_num_rows()
        # the height of the cylinder should be zero, but creating it as such
        # causes problems, so instead its bottom vertices will be translated
        # so they coincide with the top vertices
        mat = Mat4.translate_mat(-1., 0., 0.)
        vertex_data.transform_vertices(mat, 0, edge_vert_count // 2)
        edge_size = vertex_data.get_array(0).data_size_bytes
        tris_prim = geom.get_primitive(0)
        edge_row_count = tris_prim.get_num_vertices()
        edge_tris_size = tris_prim.get_vertices().data_size_bytes
        # create the remaining edges as copies of the first one (their
        # vertices will be rotated when merging their geometry)
        edge_r = edge_b.make_copy()
        edge_t = edge_b.make_copy()
        edge_l = edge_b.make_copy()
        edges = (edge_b, edge_r, edge_t, edge_l)
        s1 = SparseArray()
        s2 = SparseArray()
        s3 = SparseArray()
        s4 = SparseArray()
        index_arrays = (s1, s2, s3, s4)
        # keep track of the vertex indices associated with each corner,
        # so it will be possible to translate specific corner vertices
        # in order to stretch the button
        cls._corner_indices = {"lr": s1, "ur": s2, "ul": s3, "ll": s4}

        # Merge the geometry of all corner spheres into one single node

        main_geom = corner_lr.modify_geom(0)
        vertex_data = main_geom.modify_vertex_data()
        old_vert_count = vertex_data.get_num_rows()
        vertex_data.set_num_rows(old_vert_count * 4)
        data_array = vertex_data.modify_array(0)
        old_size = data_array.data_size_bytes // 4
        to_view = memoryview(data_array).cast("B")
        main_tris_prim = main_geom.modify_primitive(0)
        old_row_count = main_tris_prim.get_num_vertices()
        tris_array = main_tris_prim.modify_vertices()
        old_tris_size = tris_array.data_size_bytes
        tris_array.set_num_rows(old_row_count * 4)
        to_tris_view = memoryview(tris_array).cast("B")
        s1.set_range(0, old_vert_count)

        for i, (node, s) in enumerate(zip(corners, index_arrays[1:])):

            j = i + 1
            s.set_range(old_vert_count * j, old_vert_count)

            geom = node.modify_geom(0)
            vertex_data = geom.modify_vertex_data()
            mat = Mat4.rotate_mat(90. * j, Vec3().up())
            # rotate the vertices of the corner so it ends up with the
            # correct orientation
            vertex_data.transform_vertices(mat)
            data_array = vertex_data.get_array(0)
            from_view = memoryview(data_array).cast("B")
            to_view[old_size * j:old_size * (j + 1)] = from_view

            tris_prim = geom.get_primitive(0)
            tris_array = tris_prim.get_vertices()
            from_view = memoryview(tris_array).cast("B")
            to_tris_view[old_tris_size * j:old_tris_size * (j + 1)] = from_view
            main_tris_prim.offset_vertices(old_vert_count * j, old_row_count * j,
                                           old_row_count * (j + 1))

        # Merge the edge geometry with the corner geometry

        vertex_data = main_geom.modify_vertex_data()
        old_vert_count = vertex_data.get_num_rows()
        data_array = vertex_data.get_array(0)
        old_size = data_array.data_size_bytes
        # provide additional vertex rows, not just for the edges but also for
        # the central rectangle (an extra 4 rows)
        vertex_data.set_num_rows(old_vert_count + (edge_vert_count + 1) * 4)
        data_array = vertex_data.modify_array(0)
        to_view = memoryview(data_array).cast("B")
        old_row_count = main_tris_prim.get_num_vertices()
        tris_array = main_tris_prim.modify_vertices()
        old_tris_size = tris_array.data_size_bytes
        # provide additional vertex index rows, not just for the edges but also
        # for the central rectangle (an extra 6 rows)
        tris_array.set_num_rows(old_row_count + edge_row_count * 4 + 6)
        to_tris_view = memoryview(tris_array).cast("B")

        for i, (node, s) in enumerate(zip(edges, index_arrays)):

            count = edge_vert_count
            size = count // 2
            s.set_range(old_vert_count + count * i, size)

            if i == 3:
                s.set_range(old_vert_count + size, size)
            else:
                s.set_range(old_vert_count + count * (i + 1) + size, size)

            geom = node.modify_geom(0)
            vertex_data = geom.modify_vertex_data()
            mat = Mat4.rotate_mat(90. * i, Vec3().up())
            # rotate the vertices of the edge so it ends up with the
            # correct orientation
            vertex_data.transform_vertices(mat)
            data_array = vertex_data.get_array(0)
            from_view = memoryview(data_array).cast("B")
            to_view[old_size + edge_size * i:old_size + edge_size * (i + 1)] = from_view

            tris_prim = geom.get_primitive(0)
            tris_array = tris_prim.get_vertices()
            from_view = memoryview(tris_array).cast("B")
            to_tris_view[old_tris_size + edge_tris_size * i:old_tris_size
                + edge_tris_size * (i + 1)] = from_view
            main_tris_prim.offset_vertices(old_vert_count + edge_vert_count * i,
                                           old_row_count + edge_row_count * i,
                                           old_row_count + edge_row_count * (i + 1))

        vertex_data = main_geom.modify_vertex_data()

        # Define the central rectangle vertices

        values = bytearray()
        indices = bytearray()

        for (u, v) in ((1., 0.), (1., 1.), (0., 1.), (0., 0.)):
            values.extend(struct.pack("3f", 0., 0., 1.))  # initial position
            values.extend(struct.pack("3f", 0., 0., 1.))  # normal
            values.extend(struct.pack("2f", u, v))        # texture coordinates

        # Define the vertex order of the central rectangle

        vi1 = old_vert_count + edge_vert_count * 4
        vi2 = vi1 + 1
        vi3 = vi2 + 1
        vi4 = vi3 + 1
        indices.extend(struct.pack("3H", vi1, vi2, vi3))
        indices.extend(struct.pack("3H", vi1, vi3, vi4))

        # Add the central vertices to the already existing geometry

        data_array = vertex_data.modify_array(0)
        to_view = memoryview(data_array).cast("B")
        to_view[old_size + edge_size * 4:] = values
        to_tris_view[old_tris_size + edge_tris_size * 4:] = indices

        # Add the central vertex indices to the corner index arrays

        s1.set_bit(vi1)
        s2.set_bit(vi2)
        s3.set_bit(vi3)
        s4.set_bit(vi4)

        # apply one final rotation to the vertices so the button faces the
        # positive Y-axis
        mat = Mat4.rotate_mat(90., Vec3().right())
        vertex_data.transform_vertices(mat)

    def __init__(self, parent, border, text="", text_alignment="center",
                 command=None, stretch_dir="horizontal"):

        # while the border parameter refers to the thickness of the rounded
        # borders, the borders variable below adds some space around the text
        # by increasing that value for each of the button sides
        borders = (border + 5, border + 5, border + 2, border + 2)

        Widget.__init__(self, "button", parent, borders, stretch_dir=stretch_dir)

        if not self._model:
            self.__create_geometry()

        self.border = border
        self.model = self._model.copy_to(parent.get_gui_root())
        geom = self.model.node().modify_geom(0)
        vertex_data = geom.modify_vertex_data()
        # give the corners the desired border size
        mat = Mat4.scale_mat(border)
        vertex_data.transform_vertices(mat)

        width, height = self.get_size()
        w = width - border
        h = height - border
        corner_pos = ((w, -h), (w, -border), (border, -border), (border, -h))
        corner_ids = ("lr", "ur", "ul", "ll")

        for (x, z), corner_id in zip(corner_pos, corner_ids):
            mat = Mat4.translate_mat(x, 0., z)
            s = self._corner_indices[corner_id]
            # move the corner to its intended position
            vertex_data.transform_vertices(mat, s)

        color = (0., 0., 0., 1.)
        self._text_image = txt_img = self._font.create_image(text, color)
        self._tex = Texture("")
        self.model.set_texture(self._tex)
        self._text_alignment = text_alignment
        self.set_size((txt_img.get_x_size(), txt_img.get_y_size()),
                      includes_borders=False, is_min=True)

        self._is_pressed = False
        self._has_mouse = False
        self._command = command if command else lambda: None

    def destroy(self):

        Widget.destroy(self)

        self._command = lambda: None

    def set_size(self, size, includes_borders=True, is_min=False):

        old_width, old_height = self.get_size()
        width, height = Widget.set_size(self, size, includes_borders, is_min)
        w_ = width - self.border * 2
        h_ = height - self.border * 2

        image = PNMImage(w_, h_)
        image.fill(1., 1., 1.)
        txt_img = self._text_image
        w = txt_img.get_x_size()
        h = txt_img.get_y_size()
        l, r, b, t = (d - self.border for d in self.get_inner_borders())

        if self._text_alignment == "center":
            x = (l + w_ - r - w) // 2
        elif self._text_alignment == "right":
            x = w_ - r - w
        else:
            x = l

        y = (h_ - h) // 2

        image.blend_sub_image(txt_img, x, y, 0, 0)
        self._tex.load(image)

        w = width - old_width
        h = height - old_height
        corner_pos = ((w, -h), (w, 0.), (0., -h))
        geom = self.model.node().modify_geom(0)
        vertex_data = geom.modify_vertex_data()
        corner_ids = ("lr", "ur", "ll")

        for (x, z), corner_id in zip(corner_pos, corner_ids):
            mat = Mat4.translate_mat(x, 0., z)
            s = self._corner_indices[corner_id]
            # move the corner to its new position
            vertex_data.transform_vertices(mat, s)

        return width, height

    def on_enter(self):

        self._has_mouse = True
        self.model.set_light(self.get_hi_lights()[0])

    def on_leave(self):

        if not self._has_mouse:
            return False

        self._is_pressed = False
        self._has_mouse = False
        self.model.set_color_off()

        for hi_light in self.get_hi_lights():
            self.model.set_light_off(hi_light)

        return True

    def on_left_down(self):

        self._is_pressed = True
        self.model.set_color(1., 1., .8, 1.)
        hi_lights = self.get_hi_lights()
        self.model.set_light_off(hi_lights[0])

        for hi_light in hi_lights[1:]:
            self.model.set_light(hi_light)

    def on_left_up(self):

        if self._is_pressed:

            self._is_pressed = False
            self.model.set_color_off()
            hi_lights = self.get_hi_lights()
            self.model.set_light(hi_lights[0])

            for hi_light in hi_lights[1:]:
                self.model.set_light_off(hi_light)

            self._command()
