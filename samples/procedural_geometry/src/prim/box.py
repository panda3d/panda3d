# Author: Epihaius
# Date: 2019-06-21
#
# This module contains a class to generate box primitives, as well as a
# function to create boxes gradually over time.

from .base import *


class BoxMaker(ModelMaker):

    @property
    def center(self):
        return self._center

    @center.setter
    def center(self, pos):
        self._center = pos


    @property
    def width(self):
        return self._width

    @width.setter
    def width(self, width):
        self._width = width


    @property
    def depth(self):
        return self._depth

    @depth.setter
    def depth(self, depth):
        self._depth = depth


    @property
    def height(self):
        return self._height

    @height.setter
    def height(self, height):
        self._height = height


    @property
    def open_sides(self):
        return self._open_sides

    @open_sides.setter
    def open_sides(self, side_ids):
        self._open_sides = side_ids


    @property
    def thickness(self):
        return self._thickness

    @thickness.setter
    def thickness(self, thickness):
        self._thickness = thickness

    def __init__(self, center=None, width=1., depth=1., height=1., segments=None,
                 open_sides=(), thickness=None, inverted=False, vertex_color=None,
                 has_uvs=True, tex_units=None, tex_offset=None, tex_rotation=None,
                 tex_scale=None):
        """
        This class generates box model primitives with the given parameters:

            center (sequence or None):
                the position of the center of the geometry in object space;
                default is at the origin (the object-space (0., 0., 0.) point);
            width (float):
                dimension along the X-axis; cannot be negative;
                default = 1.;
            depth (float):
                dimension along the Y-axis; cannot be negative;
                default = 1.;
            height (float):
                dimension along the Z-axis; cannot be negative;
                default = 1.;
            segments (dict of ints):
                the number of side subdivisions in each dimension:
                    "width": minimum = 1, default = 1;
                    "depth": minimum = 1, default = 1;
                    "height": minimum = 1, default = 1;
            open_sides (sequence of strings):
                any combination of "left", "right", "back", "front", "bottom", "top";
                sides of the box that should not be created;
                default = empty sequence (no open sides);
            thickness (float or None):
                offset of inner box sides (in conjunction with open_sides, can lead
                to a number of different shapes, other than an actual box);
                cannot be negative;
                default = None (no inner box).

        The parameters common to all primitive types are documented in the
        ModelMaker base class.

        The surfaces of this primitive type can be referred to in the relevant
        parameters and properties as follows:
            "left", "right", "back", "front", "bottom",
            "top", "inner_left", "inner_right", "inner_back",
            "inner_front", "inner_bottom", "inner_top".

        Ranges of vertex indices of each side can be retrieved through the
        vertex_ranges property, as a dict of (start_index, end_index) tuples
        (empty tuple if the side was not created), with end_index not
        included in the range.

        """

        surface_ids = (
            "left", "right", "back", "front", "bottom",
            "top", "inner_left", "inner_right", "inner_back",
            "inner_front", "inner_bottom", "inner_top"
        )

        ModelMaker.__init__(self, segments, inverted, vertex_color, has_uvs,
                            tex_units, tex_offset, tex_rotation, tex_scale,
                            surface_ids)

        self._center = center
        self._width = width
        self._depth = depth
        self._height = height
        self._open_sides = open_sides
        self._thickness = thickness
        self._side_ids = {
            "left": "-yz", "right": "yz", "back": "-zx",
            "front": "zx", "bottom": "-xy", "top": "xy"
        }
        self._side_names = {
            "-yz": "left", "yz": "right", "-zx": "back",
            "zx": "front", "-xy": "bottom", "xy": "top"
        }

    def reset(self):

        ModelMaker.reset(self)

        self._center = None
        self._width = 1.
        self._depth = 1.
        self._height = 1.
        self._open_sides = ()
        self._thickness = None

    def __define_quads(self, indices, index_offset, direction, segs1, segs2=1):

        for i in range(segs2):

            for j in range(segs1):

                vi1 = index_offset + i * (segs1 + 1) + j
                vi2 = vi1 + 1
                vi3 = vi2 + segs1
                vi4 = vi3 + 1

                if self._inverted:
                    inds = (vi1, vi4, vi2) if direction == 1 else (vi1, vi2, vi4)
                else:
                    inds = (vi1, vi2, vi4) if direction == 1 else (vi1, vi4, vi2)

                indices.extend(inds)

                if self._inverted:
                    inds = (vi1, vi3, vi4) if direction == 1 else (vi1, vi4, vi3)
                else:
                    inds = (vi1, vi4, vi3) if direction == 1 else (vi1, vi3, vi4)

                indices.extend(inds)

    def __define_vertices(self, values, dims, dim1, dim2, c1, c2, sign, plane_id,
                          direction, segs, pos, offset1, offset2, normal,
                          axis1_index, axis2_index, primary_index, indices):

        stride = 8 if self._has_uvs else 6  # number of floats on each vertex data row
        index_offset = len(values) // stride

        if primary_index == 1:
            t = ((dim2, c1, c2), (dim1, 0., 0.))
            index1 = axis1_index
            index2 = axis2_index
            offs1 = offset1
            offs2 = offset2
        else:
            t = ((dim1, 0., 0.), (dim2, c2, c1))
            index1 = axis2_index
            index2 = axis1_index
            offs1 = offset2
            offs2 = offset1

        if self._has_uvs:

            side_id = "-" if direction == -1 else ""
            side_id += plane_id
            side_name = self._side_names[side_id]
            tex_size = None

            if self._tex_units and side_name in self._tex_units:
                tex_w, tex_h = self._tex_units[side_name]
                tex_size = (tex_h, tex_w) if plane_id == "zx" else (tex_w, tex_h)

            mat = self._get_tex_xform(side_name)

        for dim, start1, start2 in t:

            if sign == "-":
                coord = start1 - dims[index1] * .5
            else:
                coord = dims[index1] * .5 - start1

            pos[index1] = coord + offs1

            a = coord / dims[index1] + .5

            for i in range(segs + 1):

                j = i / segs

                if sign == "-":
                    coord = start2 - dims[index2] * .5 + j * dim
                else:
                    coord = dims[index2] * .5 - j * dim - start2

                pos[index2] = coord + offs2

                values.extend(pos)
                values.extend(normal)

                if self._has_uvs:

                    b = coord / dims[index2] + .5

                    if primary_index == 1:
                        u = (-b if plane_id == "zx" else a) * direction
                    else:
                        u = (-a if plane_id == "zx" else b) * direction

                    if (direction > 0 if plane_id == "zx" else direction < 0):
                        u += 1.

                    if self._inverted:
                        u = 1. - u

                    if primary_index == 1:
                        v = a if plane_id == "zx" else b
                    else:
                        v = b if plane_id == "zx" else a

                    if tex_size:
                        if plane_id == "zx":
                            u *= dims[axis2_index] / tex_size[1]
                            v *= dims[axis1_index] / tex_size[0]
                        else:
                            u *= dims[axis1_index] / tex_size[0]
                            v *= dims[axis2_index] / tex_size[1]

                    if mat:
                        u, v = mat.xform_point(Point2(u, v))

                    values.extend((u, v))

        self.__define_quads(indices, index_offset, direction, segs)

    def generate(self):

        center = (0., 0., 0.) if self._center is None else self._center
        width = max(.001, self._width)
        depth = max(.001, self._depth)
        height = max(.001, self._height)
        dims = (width, depth, height)
        side_ids = self._side_ids
        side_names = self._side_names
        open_side_ids = [side_ids[name] for name in self._open_sides]
        thickness = self._thickness

        if thickness is not None:

            # Set limits for the given thickness and derive the dimensions and
            # corner coordinates of the inner box

            th = max(0., thickness)
            inner_corners = {}
            inner_dims = {}

            th_left = 0. if "-yz" in open_side_ids else min(width, th)
            th_right = 0. if "yz" in open_side_ids else min(width, th)

            if th_left + th_right > width:
                th_left = th_right = width * .5

            inner_corners["-x"] = th_left
            inner_corners["x"] = th_right
            inner_dims["x"] = width - th_left - th_right

            th_back = 0. if "-zx" in open_side_ids else min(depth, th)
            th_front = 0. if "zx" in open_side_ids else min(depth, th)

            if th_back + th_front > depth:
                th_back = th_front = depth * .5

            inner_corners["-y"] = th_back
            inner_corners["y"] = th_front
            inner_dims["y"] = depth - th_back - th_front

            th_bottom = 0. if "-xy" in open_side_ids else min(height, th)
            th_top = 0. if "xy" in open_side_ids else min(height, th)

            if th_bottom + th_top > height:
                th_bottom = th_top = height * .5

            inner_corners["-z"] = th_bottom
            inner_corners["z"] = th_top
            inner_dims["z"] = height - th_bottom - th_top

        segs = {} if self._segments is None else self._segments
        segs_w = max(1, segs.get("width", 1))
        segs_d = max(1, segs.get("depth", 1))
        segs_h = max(1, segs.get("height", 1))
        segs = {"x": segs_w, "y": segs_d, "z": segs_h}
        inverted = self._inverted
        has_uvs = self._has_uvs
        tex_units = self._tex_units
        tex_offset = self._tex_offset
        tex_rotation = self._tex_rotation
        tex_scale = self._tex_scale
        self._vert_ranges = vert_ranges = {
            "left": (), "right": (), "back": (), "front": (), "bottom": (),
            "top": (), "inner_left": (), "inner_right": (), "inner_back": (),
            "inner_front": (), "inner_bottom": (), "inner_top": ()
        }
        stride = 8 if has_uvs else 6  # number of floats on each vertex data row
        values = array.array("f", [])
        indices = array.array("I", [])
        pos = Point3()

        # Define the vertices for each side of the box

        for (axis1_id, axis2_id, axis3_id) in ("xyz", "zxy", "yzx"):

            plane_id = axis1_id + axis2_id
            axis1_index = "xyz".index(axis1_id)
            axis2_index = "xyz".index(axis2_id)
            axis3_index = "xyz".index(axis3_id)
            offset1 = center[axis1_index]
            offset2 = center[axis2_index]
            offset3 = center[axis3_index]
            segs1 = segs[axis1_id]
            segs2 = segs[axis2_id]
            segs3 = segs[axis3_id]

            for direction in (-1, 1):

                side_id = "-" if direction == -1 else ""
                side_id += plane_id

                normal = Vec3()
                normal[axis3_index] = direction * (-1. if inverted else 1.)

                if side_id in open_side_ids:

                    if thickness is not None:

                        pos[axis3_index] = .5 * dims[axis3_index] * direction + offset3
                        index_offset = len(values) // stride

                        for sign in ("-", ""):

                            c1 = inner_corners[sign + axis1_id]
                            c2 = inner_corners[sign + axis2_id]

                            if c1 > 0.:
                                dim1 = dims[axis2_index]
                                dim2 = inner_dims[axis2_id]
                                self.__define_vertices(values, dims, dim1, dim2, c1, c2,
                                                       sign, plane_id, direction, segs2, pos,
                                                       offset1, offset2, normal, axis1_index,
                                                       axis2_index, 1, indices)

                            if c2 > 0.:
                                dim1 = dims[axis1_index]
                                dim2 = inner_dims[axis1_id]
                                self.__define_vertices(values, dims, dim1, dim2, c1, c2,
                                                       sign, plane_id, direction, segs1, pos,
                                                       offset1, offset2, normal, axis1_index,
                                                       axis2_index, 2, indices)

                        side_name = side_names[side_id]
                        end = len(values) // stride
                        vert_ranges[side_name] = (index_offset, end)

                else:

                    pos[axis3_index] = .5 * dims[axis3_index] * direction + offset3
                    index_offset = len(values) // stride
                    side_name = side_names[side_id]

                    if has_uvs:

                        tex_size = None

                        if tex_units and side_name in tex_units:
                            tex_w, tex_h = tex_units[side_name]
                            tex_size = (tex_h, tex_w) if plane_id == "zx" else (tex_w, tex_h)

                        mat = self._get_tex_xform(side_name)

                    for i in range(segs2 + 1):

                        b = i / segs2
                        pos[axis2_index] = (-.5 + b) * dims[axis2_index] + offset2

                        for j in range(segs1 + 1):

                            a = j / segs1
                            pos[axis1_index] = (-.5 + a) * dims[axis1_index] + offset1

                            values.extend(pos)
                            values.extend(normal)

                            if has_uvs:

                                u = (-b if plane_id == "zx" else a) * direction
                                u += (1. if (direction > 0 if plane_id == "zx"
                                      else direction < 0) else 0.)

                                if inverted:
                                    u = 1. - u

                                v = a if plane_id == "zx" else b

                                if tex_size:
                                    if plane_id == "zx":
                                        u *= dims[axis2_index] / tex_size[1]
                                        v *= dims[axis1_index] / tex_size[0]
                                    else:
                                        u *= dims[axis1_index] / tex_size[0]
                                        v *= dims[axis2_index] / tex_size[1]

                                if mat:
                                    u, v = mat.xform_point(Point2(u, v))

                                values.extend((u, v))

                    self.__define_quads(indices, index_offset, direction, segs1, segs2)

                    end = len(values) // stride
                    vert_ranges[side_name] = (index_offset, end)

        # Create the geometry structures

        if thickness is not None:

            # If a thickness is given, an inner box needs to be created to close
            # the surface of the model; its parameters are derived from those of
            # the outer box and adjusted to make both boxes fit together.

            w = inner_dims["x"]
            d = inner_dims["y"]
            h = inner_dims["z"]
            x = (inner_corners["-x"] - inner_corners["x"]) * .5 + center[0]
            y = (inner_corners["-y"] - inner_corners["y"]) * .5 + center[1]
            z = (inner_corners["-z"] - inner_corners["z"]) * .5 + center[2]
            segs = {
                "width": segs_w,
                "depth": segs_d,
                "height": segs_h
            }
            inner_tex_units = {} if tex_units else None

            if tex_units:
                for side_name in self._side_ids:
                    if "inner_" + side_name in tex_units:
                        inner_tex_units[side_name] = tex_units["inner_" + side_name]

            inner_tex_offset = {} if tex_offset else None

            if tex_offset:
                for side_name in self._side_ids:
                    if "inner_" + side_name in tex_offset:
                        inner_tex_offset[side_name] = tex_offset["inner_" + side_name]

            inner_tex_rot = {} if tex_rotation else None

            if tex_rotation:
                for side_name in self._side_ids:
                    if "inner_" + side_name in tex_rotation:
                        inner_tex_rot[side_name] = tex_rotation["inner_" + side_name]

            inner_tex_scale = {} if tex_scale else None

            if tex_scale:
                for side_name in self._side_ids:
                    if "inner_" + side_name in tex_scale:
                        inner_tex_scale[side_name] = tex_scale["inner_" + side_name]

            model_maker = BoxMaker((x, y, z), w, d, h, segs, self._open_sides,
                                   inverted=not inverted, has_uvs=has_uvs,
                                   tex_units=inner_tex_units, tex_offset=inner_tex_offset,
                                   tex_rotation=inner_tex_rot, tex_scale=inner_tex_scale)
            node = model_maker.generate()

            # Extend the geometry of the inner box with the data of the outer box

            geom = node.modify_geom(0)
            vertex_data = geom.modify_vertex_data()
            old_vert_count = vertex_data.get_num_rows()
            old_size = old_vert_count * stride
            vertex_data.set_num_rows(old_vert_count + len(values) // stride)
            data_array = vertex_data.modify_array(0)
            memview = memoryview(data_array).cast("B").cast("f")
            memview[old_size:] = values

            if self._vertex_color:
                if has_uvs:
                    vertex_format = GeomVertexFormat.get_v3n3c4t2()
                else:
                    vertex_format = GeomVertexFormat.get_v3n3c4()
                vertex_data.set_format(vertex_format)
                vertex_data = vertex_data.set_color(self._vertex_color)
                geom.set_vertex_data(vertex_data)

            tris_prim = geom.modify_primitive(0)
            old_row_count = tris_prim.get_num_vertices()
            new_row_count = old_row_count + len(indices)

            if new_row_count < 2 ** 16:
                # make the array compatible with the default index format of the
                # GeomPrimitive (16-bit) if the number of vertices allows it...
                indices = array.array("H", indices)
            else:
                # ...or force the GeomPrimitive to accept more indices by setting
                # its index format to 32-bit
                tris_prim.set_index_type(Geom.NT_uint32)

            tris_array = tris_prim.modify_vertices()
            tris_array.set_num_rows(new_row_count)
            memview = memoryview(tris_array).cast("B").cast(indices.typecode)
            memview[old_row_count:] = indices
            tris_prim.offset_vertices(old_vert_count, old_row_count, new_row_count)

            inner_ranges = model_maker.vertex_ranges

            for side_name in ("left", "right", "back", "front", "bottom", "top"):

                inner_range = inner_ranges[side_name]

                if inner_range:
                    vert_ranges["inner_{}".format(side_name)] = inner_range

                vert_range = vert_ranges[side_name]

                if vert_range:
                    start, end = vert_range
                    start += old_vert_count
                    end += old_vert_count
                    vert_ranges[side_name] = (start, end)

        else:

            if has_uvs:
                vertex_format = GeomVertexFormat.get_v3n3t2()
            else:
                vertex_format = GeomVertexFormat.get_v3n3()

            vertex_data = GeomVertexData("box_data", vertex_format, Geom.UH_static)
            vertex_data.unclean_set_num_rows(len(values) // stride)
            data_array = vertex_data.modify_array(0)
            memview = memoryview(data_array).cast("B").cast("f")
            memview[:] = values

            if self._vertex_color:
                if has_uvs:
                    vertex_format = GeomVertexFormat.get_v3n3c4t2()
                else:
                    vertex_format = GeomVertexFormat.get_v3n3c4()
                vertex_data.set_format(vertex_format)
                vertex_data = vertex_data.set_color(self._vertex_color)

            tris_prim = GeomTriangles(Geom.UH_static)

            if len(indices) < 2 ** 16:
                indices = array.array("H", indices)
            else:
                tris_prim.set_index_type(Geom.NT_uint32)

            tris_prim = GeomTriangles(Geom.UH_static)
            tris_array = tris_prim.modify_vertices()
            tris_array.unclean_set_num_rows(len(indices))
            memview = memoryview(tris_array).cast("B").cast(indices.typecode)
            memview[:] = indices

            geom = Geom(vertex_data)
            geom.add_primitive(tris_prim)
            node = GeomNode("box_node")
            node.add_geom(geom)

        return node


def build_box(width=1., depth=1., height=1., segments=None):
    """
    Gradually build up a box, one polygon at a time.
    Please note that this is a generator function, whose main purpose is to
    show the creation of a box over time. As such, it doesn't offer as many
    creation options as the BoxMaker class, which should be preferred
    for more serious work.

    """

    width = max(.001, width)
    depth = max(.001, depth)
    height = max(.001, height)
    dims = (width, depth, height)
    segs = {} if segments is None else segments
    segs_w = max(1, segs.get("width", 1))
    segs_d = max(1, segs.get("depth", 1))
    segs_h = max(1, segs.get("height", 1))
    segs = {"x": segs_w, "y": segs_d, "z": segs_h}
    coords = []

    vertex_count = 0
    model_initialized = False

    # Define the vertices for each side of the box

    side_ids = ("-xyz", "+yzx", "+zxy", "-yzx", "-zxy", "+xyz")
    pos = Point3()

    for (sign, axis1_id, axis2_id, axis3_id) in side_ids:

        plane_id = axis1_id + axis2_id
        axis1_index = "xyz".index(axis1_id)
        axis2_index = "xyz".index(axis2_id)
        axis3_index = "xyz".index(axis3_id)
        segs1 = segs[axis1_id]
        segs2 = segs[axis2_id]
        segs3 = segs[axis3_id]

        direction = -1 if sign == "-" else 1
        side_id = sign + plane_id

        normal = Vec3()
        normal[axis3_index] = direction

        pos[axis3_index] = .5 * dims[axis3_index] * direction
        index_offset = len(coords)

        for i in range(segs2 + 1):

            b = i / segs2
            pos[axis2_index] = (-.5 + b) * dims[axis2_index]

            if i != 1:
                values = array.array("f", [])

            update_vertex_data = True

            for j in range(segs1 + 1):

                a = j / segs1
                pos[axis1_index] = (-.5 + a) * dims[axis1_index]

                u = (-b if plane_id == "zx" else a) * direction
                u += (1. if (direction > 0 if plane_id == "zx"
                      else direction < 0) else 0.)
                v = a if plane_id == "zx" else b

                values.extend(pos)
                values.extend(normal)
                values.extend((u, v))
                coords.append(tuple(pos))

            vertex_count += segs1 + 1

            # Define the vertex order of the quads

            if i > 0:

                for j in range(segs1):

                    vi1 = index_offset + i * (segs1 + 1) + j
                    vi2 = vi1 - segs1 - 1
                    vi3 = vi2 + 1
                    vi4 = vi1 + 1
                    indices = array.array("H", [])
                    indices.extend((vi1, vi2, vi3))
                    indices.extend((vi1, vi3, vi4))

                    if model_initialized:
                        if update_vertex_data:
                            add_poly(model, indices, vertex_count, values)
                            update_vertex_data = False
                        else:
                            add_poly(model, indices)
                        yield [coords[k] for k in (vi1, vi2, vi3, vi4)], "poly_points"
                    else:
                        model = init_model("box", vertex_count, values, indices)
                        model_initialized = True
                        update_vertex_data = False
                        yield [model, [coords[k] for k in (vi1, vi2, vi3, vi4)]], "model"

    yield False
