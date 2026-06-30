# Author: Epihaius
# Date: 2019-06-21
#
# This module contains a class to generate cone primitives, as well as a
# function to create cones gradually over time.

from .base import *


class ConeMaker(ModelMaker):

    @property
    def bottom_center(self):
        return self._bottom_center

    @bottom_center.setter
    def bottom_center(self, pos):
        self._bottom_center = pos


    @property
    def top_center(self):
        return self._top_center

    @top_center.setter
    def top_center(self, pos):
        self._top_center = pos


    @property
    def bottom_radius(self):
        return self._bottom_radius

    @bottom_radius.setter
    def bottom_radius(self, radius):
        self._bottom_radius = radius


    @property
    def top_radius(self):
        return self._top_radius

    @top_radius.setter
    def top_radius(self, radius):
        self._top_radius = radius


    @property
    def smooth(self):
        return self._smooth

    @smooth.setter
    def smooth(self, smooth):
        self._smooth = smooth


    @property
    def slice(self):
        return self._slice

    @slice.setter
    def slice(self, angle):
        self._slice = angle


    @property
    def rotation(self):
        return self._rotation

    @rotation.setter
    def rotation(self, angle):
        self._rotation = angle


    @property
    def bottom_thickness(self):
        return self._bottom_thickness

    @bottom_thickness.setter
    def bottom_thickness(self, thickness):
        self._bottom_thickness = thickness


    @property
    def top_thickness(self):
        return self._top_thickness

    @top_thickness.setter
    def top_thickness(self, thickness):
        self._top_thickness = thickness

    def __init__(self, bottom_center=None, top_center=None, bottom_radius=1.,
                 top_radius=0., segments=None, smooth=True, slice=0.,
                 rotation=0., bottom_thickness=None, top_thickness=None,
                 inverted=False, vertex_color=None, has_uvs=True, tex_units=None,
                 tex_offset=None, tex_rotation=None, tex_scale=None):
                
        """
        This class generates cone model primitives with the given parameters:

            bottom_center (sequence or None):
                the position of the bottom center in object space;
                default is at the origin (the object-space (0., 0., 0.) point);
            top_center (sequence or None):
                the position of the top center in object space;
                default is at (0., 0., 1.);
            bottom_radius (float):
                the bottom radius of the cone; cannot be negative;
                default = 1.;
            top_radius (float):
                the top radius of the cone; cannot be negative;
                default = 0.;
            segments (dict of ints):
                the number of surface subdivisions:
                    "circular":
                        subdivisions of the mantle along a circular cross-section;
                        minimum = 3, default = 20;
                    "axial":
                        subdivisions of the mantle along the axis of rotation;
                        minimum = 1, default = 1;
                    "bottom_cap":
                        radial subdivisions of the bottom cap;
                        minimum = 0 (no cap), default = 1;
                    "top_cap":
                        radial subdivisions of the top cap;
                        minimum = 0 (no cap), default = 1;
                    "slice_caps_radial":
                        subdivisions of both slice caps, along the radius;
                        minimum = 0 (no caps), default = 1;
                    "slice_caps_axial":
                        subdivisions of both slice caps, along the axis of rotation;
                        minimum = 0 (no caps), default = 1;
            smooth (bool):
                whether the surface of the mantle should appear smooth or faceted;
                default is True;
            slice (float):
                the angle of the pie slice removed from the cone, in degrees;
                must be in [0., 360.] range; starts at the positive X-axis;
                default = 0. (no slice);
            rotation (float):
                the angle at which the cone is rotated about its local axis,
                in degrees;
                default = 0.;
            bottom_thickness (float or None):
                bottom radial offset of inner cone;
                creates an inner cone if smaller than bottom_radius;
                must be in [0., bottom_radius] range;
                default = None (no inner cone if top_thickness is also None);
            top_thickness (float or None):
                top radial offset of inner cone;
                creates an inner cone if smaller than top_radius;
                must be in [0., top_radius] range;
                default = None (no inner cone if bottom_thickness is also None).

        The parameters common to all primitive types are documented in the
        ModelMaker base class.

        The surfaces of this primitive type can be referred to in the relevant
        parameters and properties as follows:
            "main", "bottom_cap", "top_cap", "slice_start_cap", "slice_end_cap",
            "inner_main".

        Ranges of vertex indices of each surface can be retrieved through the
        vertex_ranges property, as a dict of (start_index, end_index) tuples
        (empty tuple if the surface was not created), with end_index not
        included in the range.

        """

        surface_ids = (
            "main", "bottom_cap", "top_cap", "slice_start_cap", "slice_end_cap",
            "inner_main"
        )

        ModelMaker.__init__(self, segments, inverted, vertex_color, has_uvs,
                            tex_units, tex_offset, tex_rotation, tex_scale,
                            surface_ids)

        self._bottom_center = bottom_center
        self._top_center = top_center
        self._bottom_radius = bottom_radius
        self._top_radius = top_radius
        self._smooth = smooth
        self._slice = slice
        self._rotation = rotation
        self._bottom_thickness = bottom_thickness
        self._top_thickness = top_thickness

    def reset(self):

        ModelMaker.reset(self)

        self._bottom_center = None
        self._top_center = None
        self._bottom_radius = 1.
        self._top_radius = 0.
        self._smooth = True
        self._slice = 0.
        self._rotation = 0.
        self._bottom_thickness = None
        self._top_thickness = None

    def __transform_vertices(self, vertex_data, axis_vec, bottom_center):

        mat = Mat4(Mat4.ident_mat())

        if self._rotation:
            mat *= Mat4.rotate_mat(self._rotation, Vec3.up())

        if axis_vec.normalize():

            cross_vec = axis_vec.cross(Vec3.up())
            ref_vec = cross_vec if cross_vec.normalize() else Vec3.right()
            angle = Vec3.up().signed_angle_deg(axis_vec, ref_vec)

            if angle:
                mat *= Mat4.rotate_mat(angle, ref_vec)

        x, y, z = bottom_center

        if x or y or z:
            mat *= Mat4.translate_mat(x, y, z)

        vertex_data.transform_vertices(mat)

    def generate(self):

        bottom_center = (0., 0., 0.) if self._bottom_center is None else self._bottom_center
        top_center = (0., 0., 1.) if self._top_center is None else self._top_center
        axis_vec = Point3(*top_center) - Point3(*bottom_center)
        height = axis_vec.length()
        bottom_radius = max(0., self._bottom_radius)
        top_radius = max(0., self._top_radius)

        if bottom_radius == top_radius == 0.:
            bottom_radius = .001

        segs = {} if self._segments is None else self._segments
        segs_c = max(3, segs.get("circular", 20))
        segs_a = max(1, segs.get("axial", 1))
        segs_bc = max(0, segs.get("bottom_cap", 1) if bottom_radius else 0)
        segs_tc = max(0, segs.get("top_cap", 1) if top_radius else 0)
        segs_sc_r = max(0, segs.get("slice_caps_radial", 1))
        segs_sc_a = max(0, segs.get("slice_caps_axial", 1))
        smooth = self._smooth
        slice = max(0., min(360., self._slice))
        slice_radians = pi * slice / 180.
        delta_angle = pi * ((360. - slice) / 180.) / segs_c
        delta_radius = top_radius - bottom_radius
        bottom_thickness = bottom_radius if self._bottom_thickness is None \
                           else max(0., min(bottom_radius, self._bottom_thickness))
        inner_bottom_radius = bottom_radius - bottom_thickness
        top_thickness = top_radius if self._top_thickness is None \
                           else max(0., min(top_radius, self._top_thickness))
        inner_top_radius = top_radius - top_thickness
        inverted = self._inverted
        has_uvs = self._has_uvs
        tex_units = self._tex_units
        tex_offset = self._tex_offset
        tex_rotation = self._tex_rotation
        tex_scale = self._tex_scale
        self._vert_ranges = vert_ranges = {
            "main": (), "bottom_cap": (), "top_cap": (), "slice_start_cap": (),
            "slice_end_cap": (), "inner_main": ()
        }
        stride = 8 if has_uvs else 6  # number of floats on each vertex data row
        values = array.array("f", [])
        indices = array.array("I", [])
        verts = []

        normal = (0., 0., 1. if inverted else -1.)

        if has_uvs:

            if tex_units and "bottom_cap" in tex_units:
                tex_size = tex_units["bottom_cap"]
            else:
                tex_size = None

            mat = self._get_tex_xform("bottom_cap")

        if segs_bc and not inner_bottom_radius:

            # Define the bottom cap triangle vertices

            u = v = .5

            if has_uvs and mat:
                u, v = mat.xform_point(Point2(u, v))

            vert = {
                "pos": (0., 0., 0.),
                "normal": normal,
                "uv": (u, v)
            }
            verts.append(vert)

            radius = bottom_radius / segs_bc

            for i in range(segs_c + 1):

                angle = delta_angle * i + (0. if inverted else slice_radians)
                c = cos(angle)
                s = sin(angle) * (-1. if inverted else 1.)
                x = radius * c
                y = radius * s

                if has_uvs:

                    u = .5 + .5 * c / segs_bc
                    v = .5 + .5 * s * (1. if inverted else -1.) / segs_bc

                    if tex_size:
                        u = (u - .5) * 2. * bottom_radius / tex_size[0] + .5
                        v = (v - .5) * 2. * bottom_radius / tex_size[1] + .5

                    if mat:
                        u, v = mat.xform_point(Point2(u, v))

                else:

                    u = v = 0.

                vert = {
                    "pos": (x, y, 0.),
                    "normal": normal,
                    "uv": (u, v)
                }
                verts.append(vert)

            # Define the vertex order of the bottom cap triangles

            for i in range(1, segs_c + 1):
                indices.extend((0, i + 1, i))

        if segs_bc and bottom_thickness:

            # Define the bottom cap quad vertices

            n = 0 if inner_bottom_radius else 1

            for i in range(n, segs_bc + 1 - n):

                radius = inner_bottom_radius + bottom_thickness * (i + n) / segs_bc

                for j in range(segs_c + 1):

                    angle = delta_angle * j + (0. if inverted else slice_radians)
                    c = cos(angle)
                    s = sin(angle) * (-1. if inverted else 1.)
                    x = radius * c
                    y = radius * s

                    if has_uvs:

                        r = radius / bottom_radius
                        u = .5 + .5 * c * r
                        v = .5 + .5 * s * (1. if inverted else -1.) * r

                        if tex_size:
                            u = (u - .5) * 2. * bottom_radius / tex_size[0] + .5
                            v = (v - .5) * 2. * bottom_radius / tex_size[1] + .5

                        if mat:
                            u, v = mat.xform_point(Point2(u, v))

                    else:

                        u = v = 0.

                    vert = {
                        "pos": (x, y, 0.),
                        "normal": normal,
                        "uv": (u, v)
                    }
                    verts.append(vert)

            # Define the vertex order of the bottom cap quads

            index_offset = segs_c + 1 if inner_bottom_radius else 1

            for i in range(0 if inner_bottom_radius else 1, segs_bc):
                for j in range(segs_c):
                    vi1 = index_offset + i * (segs_c + 1) + j
                    vi2 = vi1 - segs_c - 1
                    vi3 = vi2 + 1
                    vi4 = vi1 + 1
                    indices.extend((vi1, vi2, vi3))
                    indices.extend((vi1, vi3, vi4))

            vert_ranges["bottom_cap"] = (0, len(verts))

        vertex_count = len(verts)

        if has_uvs:

            if tex_units and "main" in tex_units:
                tex_size = tex_units["main"]
                max_radius = max(bottom_radius, top_radius)
                arc = (2 * pi - slice_radians) * max_radius
            else:
                tex_size = None

            mat = self._get_tex_xform("main")

        # Define the mantle quad vertices

        for i in range(segs_a + 1):

            radius = bottom_radius + delta_radius * i / segs_a
            z = height * i / segs_a

            if has_uvs:
                v = i / segs_a
                if tex_size:
                    v *= height / tex_size[1]
                v_start = v
            else:
                v = 0.

            for j in range(segs_c + 1):

                angle = delta_angle * j + (0. if inverted else slice_radians)
                x = radius * cos(angle)
                y = radius * sin(angle) * (-1. if inverted else 1.)

                if smooth:
                    normal = Vec3(x, y, -radius * delta_radius / height).normalized()
                    normal *= -1. if inverted else 1.

                if has_uvs:
                    u = j / segs_c
                    if tex_size:
                        u *= arc / tex_size[0]
                    if mat:
                        u, v = mat.xform_point(Point2(u, v_start))
                else:
                    u = 0.

                vert = {
                    "pos": (x, y, z),
                    "normal": normal if smooth else None,
                    "uv": (u, v)
                }
                verts.append(vert)

                if not smooth and 0 < j < segs_c:
                    verts.append(vert.copy())

        # Define the vertex order of the mantle quads

        n = segs_c + 1 if smooth else segs_c * 2
        f = 1 if smooth else 2

        for i in range(1, segs_a + 1):

            for j in range(0, segs_c * f, f):

                vi1 = vertex_count + i * n + j
                vi2 = vi1 - n
                vi3 = vi2 + 1
                vi4 = vi1 + 1
                indices.extend((vi1, vi2, vi4) if inverted else (vi1, vi2, vi3))
                indices.extend((vi2, vi3, vi4) if inverted else (vi1, vi3, vi4))

                if not smooth:
                    self._make_flat_shaded((vi1, vi2, vi3, vi4), verts)

        vert_ranges["main"] = (vertex_count, len(verts))

        vertex_count = len(verts)

        normal = (0., 0., -1. if inverted else 1.)

        if has_uvs:

            if tex_units and "top_cap" in tex_units:
                tex_size = tex_units["top_cap"]
            else:
                tex_size = None

            mat = self._get_tex_xform("top_cap")

        if segs_tc and not inner_top_radius:

            index_offset = vertex_count

            # Define the top cap triangle vertices

            u = v = .5

            if has_uvs and mat:
                u, v = mat.xform_point(Point2(u, v))

            vert = {
                "pos": (0., 0., height),
                "normal": normal,
                "uv": (u, v)
            }
            verts.append(vert)

            radius = top_radius / segs_tc

            for i in range(segs_c + 1):

                angle = delta_angle * i + (0. if inverted else slice_radians)
                c = cos(angle)
                s = sin(angle) * (-1. if inverted else 1.)
                x = radius * c
                y = radius * s

                if has_uvs:

                    u = .5 + .5 * c / segs_tc
                    v = .5 + .5 * s * (-1. if inverted else 1.) / segs_tc

                    if tex_size:
                        u = (u - .5) * 2. * top_radius / tex_size[0] + .5
                        v = (v - .5) * 2. * top_radius / tex_size[1] + .5

                    if mat:
                        u, v = mat.xform_point(Point2(u, v))

                else:

                    u = v = 0.

                vert = {
                    "pos": (x, y, height),
                    "normal": normal,
                    "uv": (u, v)
                }
                verts.append(vert)

            # Define the vertex order of the top cap triangles

            for i in range(vertex_count + 1, vertex_count + 1 + segs_c):
                indices.extend((vertex_count, i, i + 1))

        # Define the top cap quad vertices

        if segs_tc and top_thickness:

            n = 0 if inner_top_radius else 1

            for i in range(n, segs_tc + 1 - n):

                radius = inner_top_radius + top_thickness * (i + n) / segs_tc

                for j in range(segs_c + 1):

                    angle = delta_angle * j + (0. if inverted else slice_radians)
                    c = cos(angle)
                    s = sin(angle) * (-1. if inverted else 1.)
                    x = radius * c
                    y = radius * s

                    if has_uvs:

                        r = radius / top_radius
                        u = .5 + .5 * c * r
                        v = .5 + .5 * s * (-1. if inverted else 1.) * r

                        if tex_size:
                            u = (u - .5) * 2. * top_radius / tex_size[0] + .5
                            v = (v - .5) * 2. * top_radius / tex_size[1] + .5

                        if mat:
                            u, v = mat.xform_point(Point2(u, v))

                    else:

                        u = v = 0.

                    vert = {
                        "pos": (x, y, height),
                        "normal": normal,
                        "uv": (u, v)
                    }
                    verts.append(vert)

            # Define the vertex order of the top cap quads

            index_offset = vertex_count + (segs_c + 1 if inner_top_radius else 1)

            for i in range(0 if inner_top_radius else 1, segs_tc):
                for j in range(segs_c):
                    vi1 = index_offset + i * (segs_c + 1) + j
                    vi2 = vi1 - segs_c - 1
                    vi3 = vi2 + 1
                    vi4 = vi1 + 1
                    indices.extend((vi1, vi3, vi2))
                    indices.extend((vi1, vi4, vi3))

        vert_ranges["top_cap"] = (vertex_count, len(verts))

        if segs_sc_r and segs_sc_a and slice and (bottom_thickness or top_thickness):

            # Define the slice cap vertices

            index_offset = len(verts)
            max_radius = max(bottom_radius, top_radius)
            delta_inner_radius = inner_top_radius - inner_bottom_radius

            for cap_id in ("start", "end"):

                vertex_count = len(verts)

                if cap_id == "start":
                    normal = (0., -1. if inverted else 1., 0.)
                else:
                    angle = delta_angle * segs_c
                    c = cos(angle)
                    s = -sin(angle)
                    normal = Vec3(s, -c, 0.) * (-1. if inverted else 1.)

                if has_uvs:

                    cap_name = "slice_{}_cap".format(cap_id)
                    if tex_units and cap_name in tex_units:
                        tex_size = tex_units[cap_name]
                    else:
                        tex_size = None

                    mat = self._get_tex_xform(cap_name)

                for i in range(segs_sc_a + 1):

                    # Define the vertices of the slice cap quad

                    f = i / segs_sc_a
                    radius = bottom_radius + delta_radius * f
                    inner_radius = inner_bottom_radius + delta_inner_radius * f
                    z = height * f

                    if has_uvs:
                        v = f
                        if tex_size:
                            v *= height / tex_size[1]
                        v_start = v
                    else:
                        v = 0.

                    for j in range(segs_sc_r + 1):

                        r = inner_radius + (radius - inner_radius) * j / segs_sc_r

                        if cap_id == "start":
                            pos = (r, 0., z)
                        else:
                            pos = (r * c, r * s, z)

                        if has_uvs:

                            if cap_id == "start":
                                u = .5 + .5 * r / max_radius * (1. if inverted else -1.)
                            else:
                                u = .5 - .5 * r / max_radius * (1. if inverted else -1.)

                            if tex_size:
                                u = (u - .5) * 2. * max_radius / tex_size[0] + .5

                            if mat:
                                u, v = mat.xform_point(Point2(u, v_start))

                        else:

                            u = 0.

                        vert = {
                            "pos": pos,
                            "normal": normal,
                            "uv": (u, v)
                        }
                        verts.append(vert)

                for i in range(segs_sc_a):

                    # Define the vertex order of the slice cap quad

                    for j in range(segs_sc_r):

                        vi1 = index_offset + j
                        vi2 = vi1 + segs_sc_r + 1
                        vi3 = vi1 + 1
                        vi4 = vi2 + 1

                        if cap_id == "start":
                            indices.extend((vi1, vi3, vi2) if inverted else (vi1, vi2, vi3))
                            indices.extend((vi2, vi3, vi4) if inverted else (vi2, vi4, vi3))
                        else:
                            indices.extend((vi1, vi2, vi3) if inverted else (vi1, vi3, vi2))
                            indices.extend((vi2, vi4, vi3) if inverted else (vi2, vi3, vi4))

                    index_offset += segs_sc_r + 1

                index_offset += segs_sc_r + 1

                surface_name = "slice_{}_cap".format(cap_id)
                vert_ranges[surface_name] = (vertex_count, len(verts))

        for vert in verts:

            values.extend(vert["pos"])
            values.extend(vert["normal"])

            if has_uvs:
                values.extend(vert["uv"])

        # Create the geometry structures

        if inner_bottom_radius or inner_top_radius:

            # If a thickness is given, an inner cone needs to be created to close
            # the surface of the model; its parameters are derived from those of
            # the outer cone and adjusted to make both cones fit together.

            segs = {
                "circular": segs_c,
                "axial": segs_a,
                "bottom_cap": 0,
                "top_cap": 0,
                "slice_caps_radial": 0,
                "slice_caps_axial": 0
            }
            inner_tex_units = {} if tex_units else None

            if tex_units and "inner_main" in tex_units:
                inner_tex_units["main"] = tex_units["inner_main"]

            inner_tex_offset = {} if tex_offset else None

            if tex_offset and "inner_main" in tex_offset:
                inner_tex_offset["main"] = tex_offset["inner_main"]

            inner_tex_rot = {} if tex_rotation else None

            if tex_rotation and "inner_main" in tex_rotation:
                inner_tex_rot["main"] = tex_rotation["inner_main"]

            inner_tex_scale = {} if tex_scale else None

            if tex_scale and "inner_main" in tex_scale:
                inner_tex_scale["main"] = tex_scale["inner_main"]

            model_maker = ConeMaker(None, (0., 0., height), inner_bottom_radius,
                                    inner_top_radius, segs, smooth, slice,
                                    inverted=not inverted, has_uvs=has_uvs,
                                    tex_units=inner_tex_units,
                                    tex_offset=inner_tex_offset,
                                    tex_rotation=inner_tex_rot,
                                    tex_scale=inner_tex_scale)
            node = model_maker.generate()

            # Extend the geometry of the inner cone with the data of the outer cone

            geom = node.modify_geom(0)
            vertex_data = geom.modify_vertex_data()
            old_vert_count = vertex_data.get_num_rows()
            old_size = old_vert_count * stride
            vertex_data.set_num_rows(old_vert_count + len(verts))
            data_array = vertex_data.modify_array(0)
            memview = memoryview(data_array).cast("B").cast("f")
            memview[old_size:] = values
            self.__transform_vertices(vertex_data, axis_vec, bottom_center)

            if self._vertex_color:
                if has_uvs:
                    vertex_format = GeomVertexFormat.get_v3n3c4t2()
                else:
                    vertex_format = GeomVertexFormat.get_v3n3c4()
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

            inner_range = model_maker.vertex_ranges["main"]

            if inner_range:
                vert_ranges["inner_main"] = inner_range

            for surface_name in ("main", "bottom_cap", "top_cap",
                    "slice_start_cap", "slice_end_cap"):

                vert_range = vert_ranges[surface_name]

                if vert_range:
                    start, end = vert_range
                    start += old_vert_count
                    end += old_vert_count
                    vert_ranges[surface_name] = (start, end)

        else:

            if has_uvs:
                vertex_format = GeomVertexFormat.get_v3n3t2()
            else:
                vertex_format = GeomVertexFormat.get_v3n3()

            vertex_data = GeomVertexData("cone_data", vertex_format, Geom.UH_static)
            vertex_data.unclean_set_num_rows(len(verts))
            data_array = vertex_data.modify_array(0)
            memview = memoryview(data_array).cast("B").cast("f")
            memview[:] = values
            self.__transform_vertices(vertex_data, axis_vec, bottom_center)

            if self._vertex_color:
                if has_uvs:
                    vertex_format = GeomVertexFormat.get_v3n3c4t2()
                else:
                    vertex_format = GeomVertexFormat.get_v3n3c4()
                vertex_data = vertex_data.set_color(self._vertex_color)

            tris_prim = GeomTriangles(Geom.UH_static)

            if len(indices) < 2 ** 16:
                indices = array.array("H", indices)
            else:
                tris_prim.set_index_type(Geom.NT_uint32)

            tris_array = tris_prim.modify_vertices()
            tris_array.unclean_set_num_rows(len(indices))
            memview = memoryview(tris_array).cast("B").cast(indices.typecode)
            memview[:] = indices

            geom = Geom(vertex_data)
            geom.add_primitive(tris_prim)
            node = GeomNode("cone")
            node.add_geom(geom)

        return node


def build_cone(bottom_radius, top_radius, height, segments):
    """
    Gradually build up a cone, one polygon at a time.
    Please note that this is a generator function, whose main purpose is to
    show the creation of a cone over time. As such, it doesn't offer as many
    creation options as the ConeMaker class, which should be preferred
    for more serious work.

    """

    segs_c = segments["circular"]
    segs_a = segments["axial"]
    segs_cap = segments["caps"]
    delta_angle = 2. * pi / segs_c
    delta_radius = top_radius - bottom_radius
    coords = []
    vertex_count = 0
    model_initialized = False

    if segs_cap > 0:

        # Define the bottom cap triangle vertices

        normal = (0., 0., -1.)
        values = array.array("f", [])
        values.extend((0., 0., 0.))
        values.extend(normal)
        values.extend((.5, .5))
        coords.append((0., 0., 0.))

        radius = bottom_radius / segs_cap

        for i in range(segs_c):
            angle = delta_angle * i
            c = cos(angle)
            s = sin(angle)
            x = radius * c
            y = radius * s
            u = .5 + c * .5 / segs_cap
            v = .5 - s * .5 / segs_cap
            values.extend((x, y, 0.))
            values.extend(normal)
            values.extend((u, v))
            coords.append((x, y, 0.))

        vertex_count = segs_c + 1

        # Define the vertex order of the bottom cap triangles

        for i in range(1, segs_c):

            indices = array.array("H", [])
            indices.extend((0, i + 1, i))

            if model_initialized:
                add_poly(model, indices)
                yield [coords[k] for k in indices], "poly_points"
            else:
                model = init_model("cone", vertex_count, values, indices)
                model.set_z(-.5 * height)
                model_initialized = True
                yield [model, [coords[k] for k in indices]], "model"

        indices = array.array("H", [])
        indices.extend((0, 1, segs_c))
        add_poly(model, indices)
        yield [coords[k] for k in indices], "poly_points"

    for i in range(1, segs_cap):

        # Define the bottom cap quad vertices

        radius = bottom_radius * (i + 1) / segs_cap
        values = array.array("f", [])
        update_vertex_data = True

        for j in range(segs_c):
            angle = delta_angle * j
            c = cos(angle)
            s = sin(angle)
            x = radius * c
            y = radius * s
            u = .5 + c * .5 * (i + 1) / segs_cap
            v = .5 - s * .5 * (i + 1) / segs_cap
            values.extend((x, y, 0.))
            values.extend(normal)
            values.extend((u, v))
            coords.append((x, y, 0.))

        vertex_count += segs_c

        # Define the vertex order of the bottom cap quads

        for j in range(segs_c - 1):

            vi1 = 1 + i * segs_c + j
            vi2 = vi1 - segs_c
            vi3 = vi2 + 1
            vi4 = vi1 + 1
            indices = array.array("H", [])
            indices.extend((vi1, vi2, vi3))
            indices.extend((vi1, vi3, vi4))

            if update_vertex_data:
                add_poly(model, indices, vertex_count, values)
                update_vertex_data = False
            else:
                add_poly(model, indices)

            yield [coords[k] for k in (vi1, vi2, vi3, vi4)], "poly_points"

        vi1 = 1 + i * segs_c
        vi2 = vi1 - segs_c
        vi3 = vi2 + segs_c - 1
        vi4 = vi1 + segs_c - 1
        indices = array.array("H", [])
        indices.extend((vi1, vi3, vi2))
        indices.extend((vi1, vi4, vi3))
        add_poly(model, indices)
        yield [coords[k] for k in (vi1, vi2, vi3, vi4)], "poly_points"

    index_offset = vertex_count

    for i in range(segs_a + 1):

        # Define the mantle vertices

        radius = bottom_radius + delta_radius * i / segs_a
        z = height * i / segs_a
        v = i / segs_a

        if i != 1:
            values = array.array("f", [])

        update_vertex_data = True

        for j in range(segs_c + 1):
            angle = delta_angle * j
            x = radius * cos(angle)
            y = radius * sin(angle)
            normal = Vec3(x, y, -radius * delta_radius / height).normalized()
            u = j / segs_c
            values.extend((x, y, z))
            values.extend(normal)
            values.extend((u, v))
            coords.append((x, y, z))

        vertex_count += segs_c + 1

        # Define the vertex order of the mantle quads

        if i > 0:

            for j in range(segs_c):

                vi1 = index_offset + i * (segs_c + 1) + j
                vi2 = vi1 - segs_c - 1
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
                    model = init_model("cone", vertex_count, values, indices)
                    model.set_z(-.5 * height)
                    model_initialized = True
                    update_vertex_data = False
                    yield [model, [coords[k] for k in (vi1, vi2, vi3, vi4)]], "model"

    index_offset = vertex_count - 1

    # Define the top cap quad vertices

    normal = (0., 0., 1.)

    for i in range(segs_cap, 0, -1):

        radius = top_radius * i / segs_cap

        if i != segs_cap - 1:
            values = array.array("f", [])

        update_vertex_data = True

        for j in range(segs_c):
            angle = delta_angle * j
            c = cos(angle)
            s = sin(angle)
            x = radius * c
            y = radius * s
            u = .5 + c * .5 * i / segs_cap
            v = .5 + s * .5 * i / segs_cap
            values.extend((x, y, height))
            values.extend(normal)
            values.extend((u, v))
            coords.append((x, y, height))

        vertex_count += segs_c

        # Define the vertex order of the top cap quads

        if i < segs_cap:

            for j in range(segs_c - 1):

                vi1 = index_offset + 1 + (segs_cap - i) * segs_c + j
                vi2 = vi1 - segs_c
                vi3 = vi2 + 1
                vi4 = vi1 + 1
                indices = array.array("H", [])
                indices.extend((vi1, vi3, vi2))
                indices.extend((vi1, vi4, vi3))

                if update_vertex_data:
                    add_poly(model, indices, vertex_count, values)
                    update_vertex_data = False
                else:
                    add_poly(model, indices)

                yield [coords[k] for k in (vi1, vi2, vi3, vi4)], "poly_points"

            vi1 = index_offset + 1 + (segs_cap - i) * segs_c
            vi2 = vi1 - segs_c
            vi3 = vi2 + segs_c - 1
            vi4 = vi1 + segs_c - 1
            indices = array.array("H", [])
            indices.extend((vi1, vi2, vi3))
            indices.extend((vi1, vi3, vi4))
            add_poly(model, indices)
            yield [coords[k] for k in (vi1, vi2, vi3, vi4)], "poly_points"

    if segs_cap > 0:

        # Define the top cap center vertex

        if segs_cap > 1:
            values = array.array("f", [])

        values.extend((0., 0., height))
        values.extend(normal)
        values.extend((.5, .5))
        coords.append((0., 0., height))

        vertex_count += 1
        update_vertex_data = True
        index_offset += segs_c * (segs_cap - 1) + 1

        # Define the vertex order of the top cap triangles

        for i in range(index_offset + 1, index_offset + segs_c):

            indices = array.array("H", [])
            indices.extend((index_offset + segs_c, i - 1, i))

            if update_vertex_data:
                add_poly(model, indices, vertex_count, values)
                update_vertex_data = False
            else:
                add_poly(model, indices)

            yield [coords[k] for k in indices], "poly_points"

        indices = array.array("H", [])
        indices.extend((index_offset + segs_c, index_offset, index_offset + segs_c - 1))
        add_poly(model, indices)
        yield [coords[k] for k in indices], "poly_points"

    yield False
