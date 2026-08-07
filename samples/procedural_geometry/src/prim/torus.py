# Author: Epihaius
# Date: 2019-06-21
#
# This module contains a class to generate torus primitives, as well as a
# function to create torii gradually over time.

from .base import *


class TorusMaker(ModelMaker):

    @property
    def center(self):
        return self._center

    @center.setter
    def center(self, pos):
        self._center = pos


    @property
    def ring_radius(self):
        return self._ring_radius

    @ring_radius.setter
    def ring_radius(self, radius):
        self._ring_radius = radius


    @property
    def section_radius(self):
        return self._section_radius

    @section_radius.setter
    def section_radius(self, radius):
        self._section_radius = radius


    @property
    def smooth_ring(self):
        return self._smooth_ring

    @smooth_ring.setter
    def smooth_ring(self, smooth):
        self._smooth_ring = smooth


    @property
    def smooth_section(self):
        return self._smooth_section

    @smooth_section.setter
    def smooth_section(self, smooth):
        self._smooth_section = smooth


    @property
    def ring_slice(self):
        return self._ring_slice

    @ring_slice.setter
    def ring_slice(self, angle):
        self._ring_slice = angle


    @property
    def section_slice(self):
        return self._section_slice

    @section_slice.setter
    def section_slice(self, angle):
        self._section_slice = angle


    @property
    def rotation(self):
        return self._rotation

    @rotation.setter
    def rotation(self, angle):
        self._rotation = angle


    @property
    def twist(self):
        return self._twist

    @twist.setter
    def twist(self, angle):
        self._twist = angle


    @property
    def thickness(self):
        return self._thickness

    @thickness.setter
    def thickness(self, thickness):
        self._thickness = thickness

    def __init__(self, center=None, ring_radius=2., section_radius=1.,
                 segments=None, smooth_ring=True, smooth_section=True,
                 ring_slice=0., section_slice=0., rotation=0., twist=0.,
                 thickness=None, inverted=False, vertex_color=None,
                 has_uvs=True, tex_units=None, tex_offset=None,
                 tex_rotation=None, tex_scale=None):
        """
        This class generates torus model primitives with the given parameters:

            center (sequence or None):
                the position of the center of the geometry in object space;
                default is at the origin (the object-space (0., 0., 0.) point);
            ring_radius (float):
                the radius of the ring; cannot be negative;
                default = 2.;
            section_radius (float):
                the radius of the cross-sections perpendicular to the ring;
                cannot be negative;
                default = 1.;
            segments (dict of ints):
                the number of surface subdivisions:
                    "ring":
                        subdivisions along the ring;
                        minimum = 3, default = 20;
                    "section":
                        subdivisions along the cross-sections;
                        minimum = 3, default = 10;
                    "ring_slice_start_cap":
                        radial subdivisions of the cap at the start of the
                        ring slice;
                        minimum = 0 (no cap), default = 1;
                    "ring_slice_end_cap":
                        radial subdivisions of the cap at the end of the
                        ring slice;
                        minimum = 0 (no cap), default = 1;
                    "section_slice_start_cap":
                        radial subdivisions of the cap at the start of the
                        section slice;
                        minimum = 0 (no cap), default = 1;
                    "section_slice_end_cap":
                        radial subdivisions of the cap at the end of the
                        section slice;
                        minimum = 0 (no cap), default = 1;
            smooth_ring (bool):
                whether the surface of the mantle along the ring should appear
                smooth or faceted;
                default is True;
            smooth_section (bool):
                whether the surface of the mantle along the cross-sections should
                appear smooth or faceted;
                default is True;
            ring_slice (float):
                the angle of the ring pie slice removed from the torus, in degrees;
                must be in [0., 360.] range; starts at the positive X-axis;
                default = 0. (no slice);
            section_slice (float):
                the angle of the section pie slice removed from the torus, in degrees;
                must be in [0., 360.] range; starts at the positive X-axis;
                default = 0. (no slice);
            rotation (float):
                the angle at which the cross-sections are rotated about the ring,
                in degrees;
                default = 0.;
            twist (float):
                like rotation, but for the last cross-section relative to the first
                cross-section;
                default = 0.;
            thickness (float or None):
                radial offset of inner torus;
                results in a curved tube with an inner section radius equal to
                section_radius minus thickness;
                must be in [0., section_radius] range;
                default = None (no inner torus).

        The parameters common to all primitive types are documented in the
        ModelMaker base class.

        The surfaces of this primitive type can be referred to in the relevant
        parameters and properties as follows:
            "main", "ring_slice_start_cap", "ring_slice_end_cap",
            "section_slice_start_cap", "section_slice_end_cap", "inner_main".

        Ranges of vertex indices of each surface can be retrieved through the
        vertex_ranges property, as a dict of (start_index, end_index) tuples
        (empty tuple if the surface was not created), with end_index not
        included in the range.

        """

        surface_ids = (
            "main", "ring_slice_start_cap", "ring_slice_end_cap",
            "section_slice_start_cap", "section_slice_end_cap", "inner_main"
        )

        ModelMaker.__init__(self, segments, inverted, vertex_color, has_uvs,
                            tex_units, tex_offset, tex_rotation, tex_scale,
                            surface_ids)

        self._center = center
        self._ring_radius = ring_radius
        self._section_radius = section_radius
        self._smooth_ring = smooth_ring
        self._smooth_section = smooth_section
        self._ring_slice = ring_slice
        self._section_slice = section_slice
        self._rotation = rotation
        self._twist = twist
        self._thickness = thickness

    def reset(self):

        ModelMaker.reset(self)

        self._center = None
        self._ring_radius = 2.
        self._section_radius = 1.
        self._smooth_ring = True
        self._smooth_section = True
        self._ring_slice = 0.
        self._section_slice = 0.
        self._rotation = 0.
        self._twist = 0.
        self._thickness = None

    def __add_cap_data(self, segs, point, vec, points, uvs, has_uvs, u_start,
                       mirror_v, tex_size, radius, ring_arc, section_arc, mat):

        for i in range(segs + 1):

            points.append(point + vec * i / segs)

            if has_uvs:

                u = u_start
                v = .5 * vec.length() / radius * i / segs

                if mirror_v:
                    v = 1. - v

                if tex_size:
                    u *= ring_arc / tex_size[0]
                    v *= section_arc / tex_size[1]
                if mat:
                    u, v = mat.xform_point(Point2(u, v))

            else:

                u = v = 0.

            uvs.append((u, v))

    def generate(self):

        center = (0., 0., 0.) if self._center is None else self._center
        ring_radius = max(0., self._ring_radius)
        section_radius = max(.001, self._section_radius)
        segs = {} if self._segments is None else self._segments
        segs_r = max(3, segs.get("ring", 20))
        segs_s = max(3, segs.get("section", 10))
        segs_rssc = max(0, segs.get("ring_slice_start_cap", 1))
        segs_rsec = max(0, segs.get("ring_slice_end_cap", 1))
        segs_sssc = max(0, segs.get("section_slice_start_cap", 1))
        segs_ssec = max(0, segs.get("section_slice_end_cap", 1))
        smooth_ring = self._smooth_ring
        smooth_section = self._smooth_section
        smooth = smooth_ring or smooth_section
        ring_slice = max(0., min(360., self._ring_slice))
        ring_slice_radians = pi * ring_slice / 180.
        section_slice = max(0., min(360., self._section_slice))
        section_slice_radians = pi * section_slice / 180.
        rotation = self._rotation
        twist = self._twist
        rot = pi * rotation / 180.
        inverted = self._inverted
        tw = pi * twist / (180. * segs_r) * (-1 if inverted else 1.)
        twist_angle = -tw * segs_r if inverted else 0.
        delta_angle_h = pi * ((360. - ring_slice) / 180.) / segs_r
        delta_angle_v = pi * ((360. - section_slice) / 180.) / segs_s
        thickness = section_radius if self._thickness is None \
                    else max(0., min(section_radius, self._thickness))
        inner_radius = section_radius - thickness
        has_uvs = self._has_uvs
        tex_units = self._tex_units
        tex_offset = self._tex_offset
        tex_rotation = self._tex_rotation
        tex_scale = self._tex_scale
        self._vert_ranges = vert_ranges = {
            "main": (), "ring_slice_start_cap": (), "ring_slice_end_cap": (),
            "section_slice_start_cap": (), "section_slice_end_cap": (), "inner_main": ()
        }
        stride = 8 if has_uvs else 6  # number of floats on each vertex data row
        values = array.array("f", [])
        indices = array.array("I", [])
        verts = []
        index_offset = 0

        # Define the main quad vertices

        if section_slice and (segs_sssc or segs_ssec) and thickness:
            # initialize cap data, to be gathered while defining the vertices
            # of the main quads, as those along the cap borders share position
            # and uvs
            cap_data = {"start": [], "end": []}
            cap_normal = Vec3.down() if inverted else Vec3.up()
            x = delta_angle_h * ring_radius * (-1. if inverted else 1.)
            y = tw * section_radius
            angle = atan2(y, x)
            quat = Quat()
            quat.set_from_axis_angle_rad(angle, Vec3.right())
            cap_normal = quat.xform(cap_normal)

        if has_uvs:

            ring_arc = (2 * pi - ring_slice_radians) * ring_radius
            section_arc = (2 * pi - section_slice_radians) * section_radius

            if tex_units and "main" in tex_units:
                tex_size = tex_units["main"]
            else:
                tex_size = None

            if tex_units and "section_slice_start_cap" in tex_units:
                tex_size_sssc = tex_units["section_slice_start_cap"]
            else:
                tex_size_sssc = None

            if tex_units and "section_slice_end_cap" in tex_units:
                tex_size_ssec = tex_units["section_slice_end_cap"]
            else:
                tex_size_ssec = None

            mat = self._get_tex_xform("main")
            mat_sssc = self._get_tex_xform("section_slice_start_cap")
            mat_ssec = self._get_tex_xform("section_slice_end_cap")

        for i in range(segs_r + 1):

            angle_h = delta_angle_h * i + (0. if inverted else ring_slice_radians)
            c = cos(angle_h)
            s = sin(angle_h) * (-1. if inverted else 1.)

            if has_uvs:
                u = u_sc = i / segs_r
                if tex_size:
                    u *= ring_arc / tex_size[0]
                u_start = u
            else:
                u = u_sc = 0.

            if section_slice and (segs_sssc or segs_ssec) and thickness:
                quat_h = Quat()
                quat_h.set_from_axis_angle_rad(angle_h, Vec3.down() if inverted else Vec3.up())

            for j in range(segs_s + 1):

                angle_v = rot + twist_angle + delta_angle_v * j + section_slice_radians
                r = ring_radius - section_radius * cos(angle_v)
                x = r * c
                y = r * s 
                z = section_radius * sin(angle_v)
                nx = x - ring_radius * c
                ny = y - ring_radius * s

                if smooth:
                    normal = Vec3(nx, ny, z).normalized() * (-1. if inverted else 1.)

                if has_uvs:
                    v = 1. - j / segs_s
                    if tex_size:
                        v *= section_arc / tex_size[1]
                    if mat:
                        u, v = mat.xform_point(Point2(u_start, v))
                else:
                    v = 0.

                vert = {
                    "pos": (x, y, z),
                    "normal": normal if smooth else None,
                    "uv": (u, v)
                }
                verts.append(vert)

                if j in (0, segs_s) and section_slice and (segs_sssc or segs_ssec) and thickness:

                    quat_v = Quat()
                    quat_v.set_from_axis_angle_rad(angle_v, Vec3.forward())
                    n_vec = (quat_v * quat_h).xform(cap_normal)
                    p1 = Point3(x, y, z)
                    x = ring_radius * c
                    y = ring_radius * s
                    p2 = Point3(x, y, 0.)
                    r_vec = (p2 - p1).normalized() * thickness
                    points = []
                    uvs = []

                    if j == 0:
                        self.__add_cap_data(segs_sssc, p1, r_vec, points, uvs, has_uvs,
                                            u_sc, False, tex_size_sssc, section_radius,
                                            ring_arc, section_arc, mat_sssc)

                        n_vec *= 1. if inverted else -1.
                        cap_data["start"].append((points, uvs, n_vec))

                    elif j == segs_s:
                        self.__add_cap_data(segs_ssec, p1, r_vec, points, uvs, has_uvs,
                                            u_sc, True, tex_size_ssec, section_radius,
                                            ring_arc, section_arc, mat_ssec)

                        n_vec *= -1. if inverted else 1.
                        cap_data["end"].append((points, uvs, n_vec))

                if not smooth_section and 0 < j < segs_s:
                    verts.append(vert.copy())

            twist_angle += tw

            if not smooth_ring and 0 < i < segs_r:
                # duplicate latest added cross-section vertices
                n = segs_s + 1 if smooth_section else segs_s * 2
                verts.extend(v.copy() for v in verts[-n:])

        # Define the vertex order of the main quads

        n = segs_s + 1 if smooth_section else segs_s * 2
        f = 1 if smooth_section else 2

        for i in range(1, segs_r + 1):

            for j in range(0, segs_s * f, f):

                vi1 = i * n + j + index_offset
                vi2 = vi1 - n
                vi3 = vi2 + 1
                vi4 = vi1 + 1
                indices.extend((vi1, vi2, vi4) if inverted else (vi1, vi2, vi3))
                indices.extend((vi2, vi3, vi4) if inverted else (vi1, vi3, vi4))

                if not smooth:
                    self._make_flat_shaded((vi1, vi2, vi3, vi4), verts)
                elif not smooth_ring:
                    self._average_normals(vi1, vi2, verts)
                    self._average_normals(vi3, vi4, verts)
                elif not smooth_section:
                    self._average_normals(vi1, vi4, verts)
                    self._average_normals(vi2, vi3, verts)

            if not smooth_ring:
                index_offset += segs_s + 1 if smooth_section else segs_s * 2

        vert_ranges["main"] = (0, len(verts))

        if ring_slice and thickness:

            # Define the ring slice cap vertices

            for cap_id in ("start", "end"):

                segs_sc = segs_rssc if cap_id == "start" else segs_rsec

                if segs_sc:

                    vertex_count = len(verts)
                    twist_angle = tw * segs_r if cap_id == "start" else 0.
                    twist_angle *= -1. if inverted else 1.

                    if cap_id == "start":
                        normal = (0., -1. if inverted else 1., 0.)
                    else:
                        angle_h = delta_angle_h * segs_r
                        c_h = cos(angle_h)
                        s_h = -sin(angle_h)
                        normal = Vec3(s_h, -c_h, 0.) * (-1. if inverted else 1.)

                    if has_uvs:
                        cap_name = "ring_slice_{}_cap".format(cap_id)
                        if tex_units and cap_name in tex_units:
                            tex_size = tex_units[cap_name]
                        else:
                            tex_size = None
                        mat = self._get_tex_xform(cap_name)

                    if not inner_radius:

                        # Define the ring slice cap triangle vertices

                        index_offset = vertex_count

                        if cap_id == "start":
                            pos = (ring_radius, 0., 0.)
                        else:
                            pos = (ring_radius * c_h, ring_radius * s_h, 0.)

                        u = v = .5

                        if has_uvs and mat:
                            u, v = mat.xform_point(Point2(u, v))

                        vert = {
                            "pos": pos,
                            "normal": normal,
                            "uv": (u, v)
                        }
                        verts.append(vert)

                        radius = section_radius / segs_sc

                        for i in range(segs_s + 1):

                            angle_v = rot + twist_angle + delta_angle_v * i + section_slice_radians
                            c = cos(angle_v)
                            s = sin(angle_v)
                            r_ = radius * c
                            r = ring_radius - r_
                            z = radius * s

                            if cap_id == "start":
                                pos = (r, 0., z)
                            else:
                                pos = (r * c_h, r * s_h, z)

                            if has_uvs:

                                if cap_id == "start":
                                    u = .5 + .5 * r_ / section_radius * (-1. if inverted else 1.)
                                else:
                                    u = .5 - .5 * r_ / section_radius * (-1. if inverted else 1.)

                                v = .5 + .5 * s / segs_sc

                                if tex_size:
                                    u = (u - .5) * 2. * section_radius / tex_size[0] + .5
                                    v = (v - .5) * 2. * section_radius / tex_size[1] + .5

                                if mat:
                                    u, v = mat.xform_point(Point2(u, v))

                            else:

                                u = v = 0.

                            vert = {
                                "pos": pos,
                                "normal": normal,
                                "uv": (u, v)
                            }
                            verts.append(vert)

                        # Define the vertex order of the ring slice cap triangles

                        for i in range(index_offset + 1, index_offset + 1 + segs_s):
                            if cap_id == "start":
                                if inverted:
                                    indices.extend((index_offset, i + 1, i))
                                else:
                                    indices.extend((index_offset, i, i + 1))
                            else:
                                if inverted:
                                    indices.extend((index_offset, i, i + 1))
                                else:
                                    indices.extend((index_offset, i + 1, i))

                    # Define the vertices of the ring slice cap quads

                    n = 0 if inner_radius else 1

                    for i in range(n, segs_sc + 1 - n):

                        radius = inner_radius + thickness * (i + n) / segs_sc

                        for j in range(segs_s + 1):

                            angle_v = rot + twist_angle + delta_angle_v * j + section_slice_radians
                            c = cos(angle_v)
                            s = sin(angle_v)
                            r_ = radius * c
                            r = ring_radius - r_
                            z = radius * s

                            if cap_id == "start":
                                pos = (r, 0., z)
                            else:
                                pos = (r * c_h, r * s_h, z)

                            if has_uvs:

                                if cap_id == "start":
                                    u = .5 + .5 * r_ / section_radius * (-1. if inverted else 1.)
                                else:
                                    u = .5 - .5 * r_ / section_radius * (-1. if inverted else 1.)

                                v = .5 + .5 * z / section_radius

                                if tex_size:
                                    u = (u - .5) * 2. * section_radius / tex_size[0] + .5
                                    v = (v - .5) * 2. * section_radius / tex_size[1] + .5

                                if mat:
                                    u, v = mat.xform_point(Point2(u, v))

                            else:

                                u = v = 0.

                            vert = {
                                "pos": pos,
                                "normal": normal,
                                "uv": (u, v)
                            }
                            verts.append(vert)

                    # Define the vertex order of the ring slice cap quads

                    index_offset = vertex_count + (segs_s + 1 if inner_radius else 1)

                    for i in range(0 if inner_radius else 1, segs_sc):
                        for j in range(segs_s):
                            vi1 = index_offset + i * (segs_s + 1) + j
                            vi2 = vi1 - segs_s - 1
                            vi3 = vi2 + 1
                            vi4 = vi1 + 1
                            if inverted:
                                indices.extend((vi1, vi2, vi3) if cap_id == "start" else (vi1, vi3, vi2))
                                indices.extend((vi1, vi3, vi4) if cap_id == "start" else (vi1, vi4, vi3))
                            else:
                                indices.extend((vi1, vi3, vi2) if cap_id == "start" else (vi1, vi2, vi3))
                                indices.extend((vi1, vi4, vi3) if cap_id == "start" else (vi1, vi3, vi4))

                    surface_id = "ring_slice_{}_cap".format(cap_id)
                    vert_ranges[surface_id] = (vertex_count, len(verts))

        if section_slice and thickness:

            # Define the section slice cap vertices

            for cap_id in ("start", "end"):

                segs_sc = segs_sssc if cap_id == "start" else segs_ssec

                if segs_sc:

                    vertex_count = len(verts)

                    for points, uvs, normal in cap_data[cap_id]:
                        for point, uv in zip(points, uvs):
                            vert = {
                                "pos": point,
                                "normal": normal,
                                "uv": uv
                            }
                            verts.append(vert)

                    # Define the vertex order of the section slice cap quads

                    index_offset = vertex_count

                    for i in range(segs_r):
                        for j in range(segs_sc):
                            vi1 = index_offset + j
                            vi2 = vi1 + segs_sc + 1
                            vi3 = vi2 + 1
                            vi4 = vi1 + 1
                            indices.extend((vi1, vi2, vi3) if cap_id == "start" else (vi1, vi3, vi2))
                            indices.extend((vi1, vi3, vi4) if cap_id == "start" else (vi1, vi4, vi3))
                        index_offset += segs_sc + 1

                    surface_id = "section_slice_{}_cap".format(cap_id)
                    vert_ranges[surface_id] = (vertex_count, len(verts))

        for vert in verts:

            values.extend(vert["pos"])
            values.extend(vert["normal"])

            if has_uvs:
                values.extend(vert["uv"])

        # Create the geometry structures

        if inner_radius:

            # If a thickness is given, an inner torus needs to be created to close
            # the surface of the model; its parameters are derived from those of
            # the outer torus and adjusted to make both torii fit together.

            segs = {
                "ring": segs_r,
                "section": segs_s,
                "ring_slice_start_cap": 0,
                "ring_slice_end_cap": 0,
                "section_slice_start_cap": 0,
                "section_slice_end_cap": 0
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

            model_maker = TorusMaker(None, ring_radius, inner_radius, segs,
                                     smooth_ring, smooth_section, ring_slice,
                                     section_slice, rotation, twist,
                                     inverted=not inverted, has_uvs=has_uvs,
                                     tex_units=inner_tex_units,
                                     tex_offset=inner_tex_offset,
                                     tex_rotation=inner_tex_rot,
                                     tex_scale=inner_tex_scale)
            node = model_maker.generate()

            # Extend the geometry of the inner torus with the data of the outer torus

            geom = node.modify_geom(0)
            vertex_data = geom.modify_vertex_data()
            old_vert_count = vertex_data.get_num_rows()
            old_size = old_vert_count * stride
            vertex_data.set_num_rows(old_vert_count + len(verts))
            data_array = vertex_data.modify_array(0)
            memview = memoryview(data_array).cast("B").cast("f")
            memview[old_size:] = values

            x, y, z = center

            if x or y or z:
                vertex_data.transform_vertices(Mat4.translate_mat(x, y, z))

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

            for surface_name in ("main", "ring_slice_start_cap", "ring_slice_end_cap",
                    "section_slice_start_cap", "section_slice_end_cap"):

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

            vertex_data = GeomVertexData("torus_data", vertex_format, Geom.UH_static)
            vertex_data.unclean_set_num_rows(len(verts))
            data_array = vertex_data.modify_array(0)
            memview = memoryview(data_array).cast("B").cast("f")
            memview[:] = values

            x, y, z = center

            if x or y or z:
                vertex_data.transform_vertices(Mat4.translate_mat(x, y, z))

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

            tris_prim = GeomTriangles(Geom.UH_static)
            tris_array = tris_prim.modify_vertices()
            tris_array.unclean_set_num_rows(len(indices))
            memview = memoryview(tris_array).cast("B").cast(indices.typecode)
            memview[:] = indices

            geom = Geom(vertex_data)
            geom.add_primitive(tris_prim)
            node = GeomNode("torus")
            node.add_geom(geom)

        return node


def build_torus(ring_radius, section_radius, segments):
    """
    Gradually build up a torus, one polygon at a time.
    Please note that this is a generator function, whose main purpose is to
    show the creation of a torus over time. As such, it doesn't offer as many
    creation options as the TorusMaker class, which should be preferred
    for more serious work.

    """

    segs_r = segments["ring"]
    segs_s = segments["section"]
    delta_angle_h = 2. * pi / segs_r
    delta_angle_v = 2. * pi / segs_s
    coords = []

    vertex_count = 0
    model_initialized = False

    # Define the vertices

    for i in range(segs_r + 1):

        angle_h = delta_angle_h * i
        u = i / segs_r

        if i != 1:
            values = array.array("f", [])

        update_vertex_data = True

        for j in range(segs_s + 1):
            angle_v = delta_angle_v * j
            r = ring_radius - section_radius * cos(angle_v)
            c = cos(angle_h)
            s = sin(angle_h)
            x = r * c
            y = r * s
            z = section_radius * sin(angle_v)
            nx = x - ring_radius * c
            ny = y - ring_radius * s
            normal = Vec3(nx, ny, z).normalized()
            v = 1. - j / segs_s
            values.extend((x, y, z))
            values.extend(normal)
            values.extend((u, v))
            coords.append((x, y, z))

        vertex_count += segs_s + 1

        # Define the vertex order of the quads

        if i > 0:

            for j in range(segs_s):

                vi1 = i * (segs_s + 1) + j
                vi2 = vi1 - segs_s - 1
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
                    model = init_model("torus", vertex_count, values, indices)
                    model_initialized = True
                    update_vertex_data = False
                    yield [model, [coords[k] for k in (vi1, vi2, vi3, vi4)]], "model"

    yield False
