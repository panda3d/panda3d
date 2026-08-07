# Author: Epihaius
# Date: 2019-06-21
#
# This module contains a class to generate sphere primitives, as well as a
# function to create spheres gradually over time.

from .base import *


class SphereMaker(ModelMaker):

    @property
    def center(self):
        return self._center

    @center.setter
    def center(self, pos):
        self._center = pos


    @property
    def radius(self):
        return self._radius

    @radius.setter
    def radius(self, radius):
        self._radius = radius


    @property
    def smooth(self):
        return self._smooth

    @smooth.setter
    def smooth(self, smooth):
        self._smooth = smooth


    @property
    def bottom_clip(self):
        return self._bottom_clip

    @bottom_clip.setter
    def bottom_clip(self, clip_height):
        self._bottom_clip = clip_height


    @property
    def top_clip(self):
        return self._top_clip

    @top_clip.setter
    def top_clip(self, clip_height):
        self._top_clip = clip_height


    @property
    def slice(self):
        return self._slice

    @slice.setter
    def slice(self, angle):
        self._slice = angle


    @property
    def thickness(self):
        return self._thickness

    @thickness.setter
    def thickness(self, thickness):
        self._thickness = thickness

    def __init__(self, center=None, radius=1., segments=None,
                 smooth=True, bottom_clip=-1., top_clip=1., slice=0.,
                 thickness=None, inverted=False, vertex_color=None,
                 has_uvs=True, tex_units=None, tex_offset=None,
                 tex_rotation=None, tex_scale=None):
        """
        This class generates sphere model primitives with the given parameters:

            center (sequence or None):
                the position of the center of the geometry in object space;
                default is at the origin (the object-space (0., 0., 0.) point);
            radius (float):
                the radius of the sphere; cannot be negative;
                default = 1.;
            segments (dict of ints):
                the number of surface subdivisions:
                    "horizontal":
                        subdivisions along horizontal circles (or circle arcs, if
                        sliced);
                        minimum = 3, default = 20;
                    "vertical":
                        subdivisions along vertical semicircles (or shorter circle
                        arcs, if clipped);
                        minimum = 2, default = 10;
                    "bottom_cap":
                        radial subdivisions of the bottom clipping cap;
                        minimum = 0 (no cap), default = 1;
                    "top_cap":
                        radial subdivisions of the top clipping cap;
                        minimum = 0 (no cap), default = 1;
                    "slice_caps":
                        radial subdivisions of the slice caps;
                        minimum = 0 (no caps), default = 1;
            smooth (bool):
                whether the surface of the mantle should appear smooth or faceted;
                default is True;
            slice (float):
                the angle of the pie slice removed from the sphere, in degrees;
                must be in [0., 360.] range; starts at the positive X-axis;
                default = 0. (no slice);
            bottom_clip (float):
                relative height of the plane that cuts off a bottom part of the sphere;
                must be in [-1., 1.] range;
                default = -1. (no clipping);
            top_clip (float):
                relative height of the plane that cuts off a top part of the sphere;
                must be in [bottom_clip, 1.] range;
                default = 1. (no clipping);
            thickness (float or None):
                offset of inner sphere vertices; cannot be negative;
                default = None (no inner sphere).

        The parameters common to all primitive types are documented in the
        ModelMaker base class.

        The surfaces of this primitive type can be referred to in the relevant
        parameters and properties as follows:
            "main", "bottom_cap", "top_cap", "slice_start_cap", "slice_end_cap",
            "inner_main", "inner_bottom_cap", "inner_top_cap".

        Ranges of vertex indices of each surface can be retrieved through the
        vertex_ranges property, as a dict of (start_index, end_index) tuples
        (empty tuple if the surface was not created), with end_index not
        included in the range.

        """

        surface_ids = (
            "main", "bottom_cap", "top_cap", "slice_start_cap", "slice_end_cap",
            "inner_main", "inner_bottom_cap", "inner_top_cap"
        )

        ModelMaker.__init__(self, segments, inverted, vertex_color, has_uvs,
                            tex_units, tex_offset, tex_rotation, tex_scale,
                            surface_ids)

        self._center = center
        self._radius = radius
        self._smooth = smooth
        self._bottom_clip = bottom_clip
        self._top_clip = top_clip
        self._slice = slice
        self._thickness = thickness

    def reset(self):

        ModelMaker.reset(self)

        self._center = None
        self._radius = 1.
        self._smooth = True
        self._bottom_clip = -1.
        self._top_clip = 1.
        self._slice = 0.
        self._thickness = None

    def generate(self):

        center = (0., 0., 0.) if self._center is None else self._center
        radius = max(.001, self._radius)
        segs = {} if self._segments is None else self._segments
        segs_h = max(3, segs.get("horizontal", 20))
        segs_v = max(2, segs.get("vertical", 10))
        segs_bc = max(0, segs.get("bottom_cap", 1))
        segs_tc = max(0, segs.get("top_cap", 1))
        segs_sc = max(0, segs.get("slice_caps", 1))
        smooth = self._smooth
        bottom_clip = max(-1., min(1., self._bottom_clip))
        bottom_height = radius * bottom_clip
        top_clip = max(bottom_clip, min(1., self._top_clip))
        top_height = radius * top_clip
        slice = max(0., min(360., self._slice))
        slice_radians = pi * slice / 180.
        delta_angle_h = pi * ((360. - slice) / 180.) / segs_h
        bottom_angle = pi - acos(bottom_height / radius)
        top_angle = acos(top_height / radius)
        delta_angle_v = (pi - bottom_angle - top_angle) / segs_v
        thickness = radius if self._thickness is None else max(0., self._thickness)
        inner_radius = radius - thickness if (top_height
                       - bottom_height) * .5 > thickness else 0.

        if inner_radius:
            inner_bottom_height = bottom_height + thickness
            inner_bottom_clip = inner_bottom_height / inner_radius
            inner_bottom_angle = pi - acos(inner_bottom_height / inner_radius)
            inner_top_height = top_height - thickness
            inner_top_clip = inner_top_height / inner_radius
            inner_top_angle = acos(inner_top_height / inner_radius)
            inner_delta_angle_v = (pi - inner_bottom_angle - inner_top_angle) / segs_v

        inverted = self._inverted
        has_uvs = self._has_uvs
        tex_units = self._tex_units
        tex_offset = self._tex_offset
        tex_rotation = self._tex_rotation
        tex_scale = self._tex_scale
        self._vert_ranges = vert_ranges = {
            "main": (), "bottom_cap": (), "top_cap": (), "slice_start_cap": (),
            "slice_end_cap": (), "inner_main": (), "inner_bottom_cap": (),
            "inner_top_cap": ()
        }
        stride = 8 if has_uvs else 6  # number of floats on each vertex data row
        values = array.array("f", [])
        indices = array.array("I", [])
        verts = []
        index_offset = 0

        if has_uvs:

            if tex_units and "main" in tex_units:
                tex_size_main = tex_units["main"]
                arc = (2 * pi - slice_radians) * radius
            else:
                tex_size_main = None

            mat_main = self._get_tex_xform("main")

        if bottom_clip > -1.:

            z = bottom_height
            radius_h = sqrt(radius * radius - z * z)

            if has_uvs:

                if tex_units and "bottom_cap" in tex_units:
                    tex_size = tex_units["bottom_cap"]
                else:
                    tex_size = None

                mat = self._get_tex_xform("bottom_cap")

            if segs_bc and -radius < z < radius:

                # Define the bottom cap triangle vertices

                normal = (0., 0., 1. if inverted else -1.)
                u = v = .5

                if has_uvs and mat:
                    u, v = mat.xform_point(Point2(u, v))

                vert = {
                    "pos": (0., 0., z),
                    "normal": normal,
                    "uv": (u, v)
                }
                verts.append(vert)
                index_offset += segs_h + 2
                r = radius_h / segs_bc

                for i in range(segs_h + 1):

                    angle_h = delta_angle_h * i + (0. if inverted else slice_radians)
                    c = cos(angle_h)
                    s = sin(angle_h) * (-1. if inverted else 1.)
                    x = r * c
                    y = r * s

                    if has_uvs:

                        u = .5 + .5 * c / segs_bc
                        v = .5 + .5 * s * (1. if inverted else -1.) / segs_bc

                        if tex_size:
                            u = (u - .5) * 2. * radius_h / tex_size[0] + .5
                            v = (v - .5) * 2. * radius_h / tex_size[1] + .5

                        if mat:
                            u, v = mat.xform_point(Point2(u, v))

                    else:

                        u = v = 0.

                    vert = {
                        "pos": (x, y, z),
                        "normal": normal,
                        "uv": (u, v)
                    }
                    verts.append(vert)

                # Define the vertex order of the bottom cap triangles

                for i in range(1, segs_h + 1):
                    indices.extend((0, i + 1, i))

                # Define the bottom cap quad vertices

                for i in range(1, segs_bc):

                    r = radius_h * (i + 1) / segs_bc

                    for j in range(segs_h + 1):

                        angle_h = delta_angle_h * j + (0. if inverted else slice_radians)
                        c = cos(angle_h)
                        s = sin(angle_h) * (-1. if inverted else 1.)
                        x = r * c
                        y = r * s

                        if has_uvs:

                            r_ = (i + 1) / segs_bc
                            u = .5 + .5 * c * r_
                            v = .5 + .5 * s * (1. if inverted else -1.) * r_

                            if tex_size:
                                u = (u - .5) * 2. * radius_h / tex_size[0] + .5
                                v = (v - .5) * 2. * radius_h / tex_size[1] + .5

                            if mat:
                                u, v = mat.xform_point(Point2(u, v))

                        else:

                            u = v = 0.

                        vert = {
                            "pos": (x, y, z),
                            "normal": normal,
                            "uv": (u, v)
                        }
                        verts.append(vert)

                # Define the vertex order of the bottom cap quads

                for i in range(1, segs_bc):

                    for j in range(segs_h):
                        vi1 = index_offset + j
                        vi2 = vi1 - segs_h - 1
                        vi3 = vi2 + 1
                        vi4 = vi1 + 1
                        indices.extend((vi1, vi2, vi4) if inverted else (vi1, vi2, vi3))
                        indices.extend((vi2, vi3, vi4) if inverted else (vi1, vi3, vi4))

                    index_offset += segs_h + 1

                vert_ranges["bottom_cap"] = (0, len(verts))

            main_start_index = len(verts)

            # Define the bottom edge vertices

            if has_uvs:
                v = (pi - acos(z / radius)) / pi
                if tex_size_main:
                    v *= pi * radius / tex_size_main[1]
                v_start = v
            else:
                v = 0.

            for i in range(segs_h + 1):

                angle_h = delta_angle_h * i + (0. if inverted else slice_radians)
                x = radius_h * cos(angle_h)
                y = radius_h * sin(angle_h) * (-1. if inverted else 1.)

                if smooth:
                    normal = Vec3(x, y, z).normalized() * (-1. if inverted else 1.)

                if has_uvs:
                    u = i / segs_h
                    if tex_size_main:
                        u *= arc / tex_size_main[0]
                    if mat_main:
                        u, v = mat_main.xform_point(Point2(u, v_start))
                else:
                    u = 0.

                vert = {
                    "pos": (x, y, z),
                    "normal": normal if smooth else None,
                    "uv": (u, v)
                }
                verts.append(vert)

                if not smooth and 0 < i < segs_h:
                    verts.append(vert.copy())

        else:

            main_start_index = 0

            # Define the bottom pole triangle vertices

            if smooth:
                normal = (0., 0., 1. if inverted else -1.)

            for i in range(segs_h):

                if has_uvs:
                    u = i / segs_h
                    v = 0.
                    if tex_size_main:
                        u *= arc / tex_size_main[0]
                    if mat_main:
                        u, v = mat_main.xform_point(Point2(u, v))
                else:
                    u = v = 0.

                vert = {
                    "pos": (0., 0., -radius),
                    "normal": normal if smooth else None,
                    "uv": (u, v)
                }
                verts.append(vert)

        vertex_count = len(verts)
        angle_v = bottom_angle + delta_angle_v
        z = radius * -cos(angle_v)

        # Define the vertices along the bottom pole or cap

        radius_h = radius * sin(angle_v)

        if has_uvs:
            v = angle_v / pi
            if tex_size_main:
                v *= pi * radius / tex_size_main[1]
            v_start = v
        else:
            v = 0.

        for i in range(segs_h + 1):

            angle_h = delta_angle_h * i + (0. if inverted else slice_radians)
            x = radius_h * cos(angle_h)
            y = radius_h * sin(angle_h) * (-1. if inverted else 1.)

            if smooth:
                normal = Vec3(x, y, z).normalized() * (-1. if inverted else 1.)

            if has_uvs:
                u = i / segs_h
                if tex_size_main:
                    u *= arc / tex_size_main[0]
                if mat_main:
                    u, v = mat_main.xform_point(Point2(u, v_start))
            else:
                u = 0.

            vert = {
                "pos": (x, y, z),
                "normal": normal if smooth else None,
                "uv": (u, v)
            }
            verts.append(vert)

            if not smooth and 0 < i < segs_h:
                verts.append(vert.copy())

        # Define the vertex order of the polygons along the bottom pole or cap

        if bottom_clip > -1.:

            n = segs_h if smooth else segs_h * 2 - 1
            f = 1 if smooth else 2

            for i in range(0, segs_h * f, f):

                vi1 = i + index_offset
                vi2 = vi1 + 1
                vi3 = vi2 + n
                vi4 = vi3 + 1
                indices.extend((vi1, vi4, vi3) if inverted else (vi1, vi2, vi3))
                indices.extend((vi1, vi2, vi4) if inverted else (vi2, vi4, vi3))

                if not smooth:
                    self._make_flat_shaded((vi1, vi2, vi3, vi4), verts)

            if smooth:
                index_offset += segs_h + 1

        else:

            for i in range(segs_h):

                j = i + index_offset
                n = 0 if smooth else i
                inds = (j, j + segs_h + 1 + n, j + segs_h + n)
                indices.extend(inds)

                if not smooth:
                    self._make_flat_shaded(inds, verts)

            if smooth:
                index_offset += segs_h

        # Define the main quad vertices

        if not smooth:
            index_offset = len(verts)
            vert_index = len(verts) + segs_h + 1

        n = segs_h + 1 if smooth else segs_h * 2
        f = 1 if smooth else 2

        for i in range(1 if smooth else 0, segs_v - 1):

            angle_v = bottom_angle + delta_angle_v * (i + 1)
            z = radius * -cos(angle_v)
            radius_h = radius * sin(angle_v)

            if has_uvs:
                v = angle_v / pi
                if tex_size_main:
                    v *= pi * radius / tex_size_main[1]
                v_start = v
            else:
                v = 0.

            for j in range(segs_h + 1):

                angle_h = delta_angle_h * j + (0. if inverted else slice_radians)
                x = radius_h * cos(angle_h)
                y = radius_h * sin(angle_h) * (-1. if inverted else 1.)

                if smooth:
                    normal = Vec3(x, y, z).normalized() * (-1. if inverted else 1.)

                if has_uvs:
                    u = j / segs_h
                    if tex_size_main:
                        u *= arc / tex_size_main[0]
                    if mat_main:
                        u, v = mat_main.xform_point(Point2(u, v_start))
                else:
                    u = 0.

                vert = {
                    "pos": (x, y, z),
                    "normal": normal if smooth else None,
                    "uv": (u, v)
                }
                verts.append(vert)

                if not smooth and 0 < j < segs_h:
                    verts.append(vert.copy())

            # Define the vertex order of the main quads

            if i > 0:

                for j in range(0, segs_h * f, f):

                    vi1 = i * n + j + index_offset
                    vi2 = vi1 - n
                    vi3 = vi2 + 1
                    vi4 = vi1 + 1
                    indices.extend((vi1, vi2, vi4) if inverted else (vi1, vi2, vi3))
                    indices.extend((vi2, vi3, vi4) if inverted else (vi1, vi3, vi4))

                    if not smooth:
                        self._make_flat_shaded((vi1, vi2, vi3, vi4), verts)

            if not smooth and i > 0:
                # duplicate latest added vertices
                verts.extend(v.copy() for v in verts[-segs_h * 2:])
                index_offset += segs_h * 2

        if top_clip < 1.:

            # Define the top edge vertices

            z = top_height
            radius_h = sqrt(radius * radius - z * z)

            if has_uvs:
                v = (pi - acos(z / radius)) / pi
                if tex_size_main:
                    v *= pi * radius / tex_size_main[1]
                v_start = v
            else:
                v = 0.

            for i in range(segs_h + 1):

                angle_h = delta_angle_h * i + (0. if inverted else slice_radians)
                x = radius_h * cos(angle_h)
                y = radius_h * sin(angle_h) * (-1. if inverted else 1.)

                if smooth:
                    normal = Vec3(x, y, z).normalized() * (-1. if inverted else 1.)

                if has_uvs:
                    u = i / segs_h
                    if tex_size_main:
                        u *= arc / tex_size_main[0]
                    if mat_main:
                        u, v = mat_main.xform_point(Point2(u, v_start))
                else:
                    u = 0.

                vert = {
                    "pos": (x, y, z),
                    "normal": normal if smooth else None,
                    "uv": (u, v)
                }
                verts.append(vert)

                if not smooth and 0 < i < segs_h:
                    verts.append(vert.copy())

        else:

            # Define the top pole triangle vertices

            if smooth:
                normal = (0., 0., -1. if inverted else 1.)

            for i in range(segs_h):

                if has_uvs:
                    u = i / segs_h
                    v = 1.
                    if tex_size_main:
                        u *= arc / tex_size_main[0]
                        v *= pi * radius / tex_size_main[1]
                    if mat_main:
                        u, v = mat_main.xform_point(Point2(u, v))
                else:
                    u = v = 0.

                vert = {
                    "pos": (0., 0., radius),
                    "normal": normal if smooth else None,
                    "uv": (u, v)
                }
                verts.append(vert)

        index_offset = len(verts) - 1

        # Define the vertex order of the polygons along the top pole or cap

        if top_clip < 1.:

            n = segs_h if smooth else segs_h * 2 - 1
            f = 1 if smooth else 2
            index_offset -= (segs_h - 1) * f + n + 2

            for i in range(0, segs_h * f, f):

                vi1 = i + index_offset
                vi2 = vi1 + 1
                vi3 = vi2 + n
                vi4 = vi3 + 1
                indices.extend((vi1, vi2, vi4) if inverted else (vi1, vi2, vi3))
                indices.extend((vi1, vi4, vi3) if inverted else (vi2, vi4, vi3))

                if not smooth:
                    self._make_flat_shaded((vi1, vi2, vi3, vi4), verts)

            if smooth:
                index_offset += segs_h + 1

        else:

            # Define the vertex order of the top pole triangles

            for i in range(segs_h):

                j = index_offset - i
                n = 0 if smooth else i
                inds = (j, j - segs_h - 1 - n, j - segs_h - n)
                indices.extend(inds)

                if not smooth:
                    self._make_flat_shaded(inds, verts)

        vert_ranges["main"] = (main_start_index, len(verts))

        if top_clip < 1.:

            index_offset = len(verts)

            if segs_tc and -radius < z < radius:

                # Define the top cap triangle vertices

                top_cap_start_index = index_offset

                normal = (0., 0., -1. if inverted else 1.)

                if has_uvs:

                    if tex_units and "top_cap" in tex_units:
                        tex_size = tex_units["top_cap"]
                    else:
                        tex_size = None

                    mat = self._get_tex_xform("top_cap")

                u = v = .5

                if has_uvs and mat:
                    u, v = mat.xform_point(Point2(u, v))

                vert = {
                    "pos": (0., 0., z),
                    "normal": normal,
                    "uv": (u, v)
                }
                verts.append(vert)
                r = radius_h / segs_tc

                for i in range(segs_h + 1):

                    angle_h = delta_angle_h * i + (0. if inverted else slice_radians)
                    c = cos(angle_h)
                    s = sin(angle_h) * (-1. if inverted else 1.)
                    x = r * c
                    y = r * s

                    if has_uvs:

                        u = .5 + .5 * c / segs_tc
                        v = .5 + .5 * s * (-1. if inverted else 1.) / segs_tc

                        if tex_size:
                            u = (u - .5) * 2. * radius_h / tex_size[0] + .5
                            v = (v - .5) * 2. * radius_h / tex_size[1] + .5

                        if mat:
                            u, v = mat.xform_point(Point2(u, v))

                    else:

                        u = v = 0.

                    vert = {
                        "pos": (x, y, z),
                        "normal": normal,
                        "uv": (u, v)
                    }
                    verts.append(vert)

                # Define the vertex order of the top cap triangles

                for i in range(index_offset + 1, index_offset + segs_h + 1):
                    indices.extend((index_offset, i, i + 1))

                # Define the top cap quad vertices

                for i in range(1, segs_tc):

                    r = radius_h * (i + 1) / segs_tc

                    for j in range(segs_h + 1):

                        angle_h = delta_angle_h * j + (0. if inverted else slice_radians)
                        c = cos(angle_h)
                        s = sin(angle_h) * (-1. if inverted else 1.)
                        x = r * c
                        y = r * s

                        if has_uvs:

                            r_ = (i + 1) / segs_tc
                            u = .5 + .5 * c * r_
                            v = .5 + .5 * s * (-1. if inverted else 1.) * r_

                            if tex_size:
                                u = (u - .5) * 2. * radius_h / tex_size[0] + .5
                                v = (v - .5) * 2. * radius_h / tex_size[1] + .5

                            if mat:
                                u, v = mat.xform_point(Point2(u, v))

                        else:

                            u = v = 0.

                        vert = {
                            "pos": (x, y, z),
                            "normal": normal,
                            "uv": (u, v)
                        }
                        verts.append(vert)

                # Define the vertex order of the top cap quads

                index_offset += segs_h + 2

                for i in range(1, segs_tc):

                    for j in range(segs_h):
                        vi1 = index_offset + j
                        vi2 = vi1 - segs_h - 1
                        vi3 = vi2 + 1
                        vi4 = vi1 + 1
                        indices.extend((vi1, vi3, vi2) if inverted else (vi1, vi4, vi2))
                        indices.extend((vi1, vi4, vi3) if inverted else (vi4, vi3, vi2))

                    index_offset += segs_h + 1

                end = len(values) // stride
                vert_ranges["top_cap"] = (top_cap_start_index, len(verts))

        if segs_sc and slice and bottom_height < radius and top_height > -radius \
                and thickness:

            # Define the slice cap vertices

            for cap_id in ("start", "end"):

                index_offset = slice_cap_start_index = len(verts)

                if cap_id == "start":
                    normal = (0., -1. if inverted else 1., 0.)
                else:
                    angle_h = delta_angle_h * segs_h
                    c_h = cos(angle_h)
                    s_h = -sin(angle_h)
                    normal = Vec3(s_h, -c_h, 0.) * (-1. if inverted else 1.)

                seg_vecs = []

                if inner_radius:

                    inner_pos = []

                    if bottom_clip > -1.:
                        inner_pos.append(Point3(0., 0., inner_bottom_height))
                        seg_vecs.append(Vec3(0., 0., -thickness / segs_sc))

                    for i in range(segs_v + 1):

                        angle_v = bottom_angle + delta_angle_v * i
                        c = -cos(angle_v)
                        r = radius * sin(angle_v)
                        z = radius * c
                        i_angle_v = inner_bottom_angle + inner_delta_angle_v * i
                        i_c = -cos(i_angle_v)
                        i_r = inner_radius * sin(i_angle_v)
                        i_z = inner_radius * i_c

                        if cap_id == "start":
                            p = Point3(r, 0., z)
                            i_p = Point3(i_r, 0., i_z)
                        else:
                            p = Point3(r * c_h, r * s_h, z)
                            i_p = Point3(i_r * c_h, i_r * s_h, i_z)

                        inner_pos.append(i_p)
                        seg_vecs.append((p - i_p) / segs_sc)

                    if top_clip < 1.:
                        inner_pos.append(Point3(0., 0., inner_top_height))
                        seg_vecs.append(Vec3(0., 0., thickness / segs_sc))

                else:

                    z = (top_height + bottom_height) * .5
                    h = (top_height - bottom_height) * .5
                    inner_pos = Point3(0., 0., z)

                    if bottom_clip > -1.:
                        seg_vecs.append(Vec3(0., 0., -h / segs_sc))

                    for i in range(segs_v + 1):

                        angle_v = bottom_angle + delta_angle_v * i
                        c = -cos(angle_v)
                        r = radius * sin(angle_v)
                        z = radius * c

                        if cap_id == "start":
                            p = Point3(r, 0., z)
                        else:
                            p = Point3(r * c_h, r * s_h, z)

                        seg_vecs.append((p - inner_pos) / segs_sc)

                    if top_clip < 1.:
                        seg_vecs.append(Vec3(0., 0., h / segs_sc))

                if has_uvs:

                    cap_name = "slice_{}_cap".format(cap_id)

                    if tex_units and cap_name in tex_units:
                        tex_size = tex_units[cap_name]
                        u_f = 2 * radius / tex_size[0]
                        v_f = 2 * radius / tex_size[1]
                    else:
                        tex_size = None

                    mat = self._get_tex_xform(cap_name)

                if inner_radius:

                    # Define the lower inner central vertex of the slice cap

                    if bottom_clip > -1.:

                        pos = inner_pos[0]

                        if has_uvs:
                            u = .5
                            v = .5 + .5 * pos.z / radius
                            if tex_size:
                                v = (v - .5) * v_f + .5
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
                        index_offset += 1

                    # Define the main inner vertices of the slice cap quads

                    for i in range(segs_v + 1):

                        pos = inner_pos[i + (1 if bottom_clip > -1. else 0)]
                        vec = Vec3(pos)

                        if has_uvs:
                            if cap_id == "start":
                                u = .5 + .5 * pos.x / radius * (1. if inverted else -1.)
                            else:
                                vec[2] = 0.
                                u = .5 - .5 * vec.length() / radius * (1. if inverted else -1.)
                            v = .5 + .5 * pos.z / radius
                            if tex_size:
                                u = (u - .5) * u_f + .5
                                v = (v - .5) * v_f + .5
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

                    index_offset += segs_v + 1

                    # Define the upper inner central vertex of the slice cap

                    if top_clip < 1.:

                        pos = inner_pos[-1]

                        if has_uvs:
                            u = .5
                            v = .5 + .5 * pos.z / radius
                            if tex_size:
                                v = (v - .5) * v_f + .5
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
                        index_offset += 1

                else:

                    # Define the center vertex of the slice cap

                    if has_uvs:
                        u = .5
                        v = .5 + .5 * inner_pos.z / radius
                        if tex_size:
                            v = (v - .5) * v_f + .5
                        if mat:
                            u, v = mat.xform_point(Point2(u, v))
                    else:
                        u = v = 0.

                    vert = {
                        "pos": inner_pos,
                        "normal": normal,
                        "uv": (u, v)
                    }
                    verts.append(vert)

                for i in range(segs_sc):

                    # Define the lower central vertices of the slice cap

                    if bottom_clip > -1.:

                        i_p = inner_pos[0] if inner_radius else inner_pos
                        pos = i_p + seg_vecs[0] * (i + 1)

                        if has_uvs:
                            u = .5
                            v = .5 + .5 * pos.z / radius
                            if tex_size:
                                v = (v - .5) * v_f + .5
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

                    # Define the main vertices of the slice cap polygons

                    for j in range(segs_v + 1):

                        index = j + (1 if bottom_clip > -1. else 0)
                        i_p = inner_pos[index] if inner_radius else inner_pos
                        pos = i_p + seg_vecs[index] * (i + 1)
                        vec = Vec3(pos)

                        if has_uvs:
                            if cap_id == "start":
                                u = .5 + .5 * pos.x / radius * (1. if inverted else -1.)
                            else:
                                vec[2] = 0.
                                u = .5 - .5 * vec.length() / radius * (1. if inverted else -1.)
                            v = .5 + .5 * pos.z / radius
                            if tex_size:
                                u = (u - .5) * u_f + .5
                                v = (v - .5) * v_f + .5
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

                    # Define the upper central vertices of the slice cap

                    if top_clip < 1.:

                        i_p = inner_pos[-1] if inner_radius else inner_pos
                        pos = i_p + seg_vecs[-1] * (i + 1)

                        if has_uvs:
                            u = .5
                            v = .5 + .5 * pos.z / radius
                            if tex_size:
                                v = (v - .5) * v_f + .5
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

                    if i == 0 and not inner_radius:

                        # Define the vertex order of the slice cap triangles

                        n = 0 + (1 if bottom_clip > -1. else 0) + (1 if top_clip < 1. else 0)

                        for j in range(segs_v + n):

                            vi1 = index_offset
                            vi2 = vi1 + j + 1
                            vi3 = vi2 + 1

                            if cap_id == "start":
                                indices.extend((vi1, vi2, vi3) if inverted else (vi1, vi3, vi2))
                            else:
                                indices.extend((vi1, vi3, vi2) if inverted else (vi1, vi2, vi3))

                    else:

                        # Define the vertex order of the slice cap quads

                        n = 0 + (1 if bottom_clip > -1. else 0) + (1 if top_clip < 1. else 0)
                        n += 0 if inner_radius else 1

                        for j in range(0 if inner_radius else 1, segs_v + n):

                            vi1 = index_offset + j
                            vi2 = vi1 - segs_v - n - (1 if inner_radius else 0)
                            vi3 = vi2 + 1
                            vi4 = vi1 + 1

                            if cap_id == "start":
                                indices.extend((vi1, vi4, vi2) if inverted else (vi1, vi2, vi4))
                                indices.extend((vi2, vi4, vi3) if inverted else (vi2, vi3, vi4))
                            else:
                                indices.extend((vi1, vi2, vi4) if inverted else (vi1, vi4, vi2))
                                indices.extend((vi2, vi3, vi4) if inverted else (vi2, vi4, vi3))

                    n = 1 + (1 if bottom_clip > -1. else 0) + (1 if top_clip < 1. else 0)
                    index_offset += segs_v + n

                surface_name = "slice_{}_cap".format(cap_id)
                vert_ranges[surface_name] = (slice_cap_start_index, len(verts))

        for vert in verts:

            values.extend(vert["pos"])
            values.extend(vert["normal"])

            if has_uvs:
                values.extend(vert["uv"])

        # Create the geometry structures

        if inner_radius:

            # If a thickness is given, an inner sphere needs to be created to close
            # the surface of the model; its parameters are derived from those of
            # the outer sphere and adjusted to make both spheres fit together.

            segs = {
                "horizontal": segs_h,
                "vertical": segs_v,
                "bottom_cap": segs_bc,
                "top_cap": segs_tc,
                "slice_caps": 0
            }
            surface_names = ("main", "bottom_cap", "top_cap")
            inner_tex_units = {} if tex_units else None

            if tex_units:
                for surface_name in surface_names:
                    if "inner_" + surface_name in tex_units:
                        inner_tex_units[surface_name] = tex_units["inner_" + surface_name]

            inner_tex_offset = {} if tex_offset else None

            if tex_offset:
                for surface_name in surface_names:
                    if "inner_" + surface_name in tex_offset:
                        inner_tex_offset[surface_name] = tex_offset["inner_" + surface_name]

            inner_tex_rot = {} if tex_rotation else None

            if tex_rotation:
                for surface_name in surface_names:
                    if "inner_" + surface_name in tex_rotation:
                        inner_tex_rot[surface_name] = tex_rotation["inner_" + surface_name]

            inner_tex_scale = {} if tex_scale else None

            if tex_scale:
                for surface_name in surface_names:
                    if "inner_" + surface_name in tex_scale:
                        inner_tex_scale[surface_name] = tex_scale["inner_" + surface_name]

            model_maker = SphereMaker(None, inner_radius, segs, smooth,
                                      inner_bottom_clip, inner_top_clip,
                                      slice, inverted=not inverted,
                                      has_uvs=has_uvs, tex_units=inner_tex_units,
                                      tex_offset=inner_tex_offset,
                                      tex_rotation=inner_tex_rot,
                                      tex_scale=inner_tex_scale)
            node = model_maker.generate()

            # Extend the geometry of the inner sphere with the data of the outer sphere

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

            inner_ranges = model_maker.vertex_ranges

            for surface_name in ("main", "bottom_cap", "top_cap"):

                inner_range = inner_ranges[surface_name]

                if inner_range:
                    vert_ranges["inner_{}".format(surface_name)] = inner_range

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

            vertex_data = GeomVertexData("sphere_data", vertex_format, Geom.UH_static)
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

            tris_array = tris_prim.modify_vertices()
            tris_array.unclean_set_num_rows(len(indices))
            memview = memoryview(tris_array).cast("B").cast(indices.typecode)
            memview[:] = indices

            geom = Geom(vertex_data)
            geom.add_primitive(tris_prim)
            node = GeomNode("sphere")
            node.add_geom(geom)

        return node


def build_sphere(radius, segments):
    """
    Gradually build up a sphere, one polygon at a time.
    Please note that this is a generator function, whose main purpose is to
    show the creation of a sphere over time. As such, it doesn't offer as many
    creation options as the SphereMaker class, which should be preferred
    for more serious work.

    """

    delta_angle = 2 * pi / segments
    coords = []

    # Define the bottom pole vertices

    values = array.array("f", [])
    normal = (0., 0., -1.)

    for i in range(segments):
        u = i / segments
        values.extend((0., 0., -radius))
        values.extend(normal)
        values.extend((u, 0.))
        coords.append((0., 0., -radius))

    angle_v = 2. * pi / segments
    radius_h = radius * sin(angle_v)
    z = radius * -cos(angle_v)
    v = 2. / segments

    for i in range(segments + 1):
        angle = delta_angle * i
        c = cos(angle)
        s = sin(angle)
        x = radius_h * c
        y = radius_h * s
        normal = Vec3(x, y, z).normalized() 
        u = i / segments
        values.extend((x, y, z))
        values.extend(normal)
        values.extend((u, v))
        coords.append((x, y, z))

    vertex_count = 2 * segments + 1
    model_initialized = False

    # Define the vertex order of the bottom pole triangles

    for i in range(segments):

        indices = array.array("H", [])
        indices.extend((i, i + segments + 1, i + segments))

        if model_initialized:
            add_poly(model, indices)
            yield [coords[k] for k in indices], "poly_points"
        else:
            model = init_model("sphere", vertex_count, values, indices)
            model_initialized = True
            yield [model, [coords[k] for k in indices]], "model"

    # Define the quad vertices

    index_offset = segments

    for i in range(1, (segments - 2) // 2):

        angle_v = delta_angle * (i + 1)
        radius_h = radius * sin(angle_v)
        z = radius * -cos(angle_v)
        v = 2. * (i + 1) / segments
        values = array.array("f", [])
        update_vertex_data = True

        for j in range(segments + 1):
            angle_h = delta_angle * j
            x = radius_h * cos(angle_h)
            y = radius_h * sin(angle_h)
            normal = Vec3(x, y, z).normalized()
            u = j / segments
            values.extend((x, y, z))
            values.extend(normal)
            values.extend((u, v))
            coords.append((x, y, z))

        vertex_count += segments + 1

        # Define the vertex order of the quads

        for j in range(segments):

            vi1 = i * (segments + 1) + j + index_offset
            vi2 = vi1 - segments - 1
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

    # Define the top pole vertices

    values = array.array("f", [])
    normal = (0., 0., 1.)

    for i in range(segments):
        u = i / segments
        values.extend((0., 0., radius))
        values.extend(normal)
        values.extend((u, 1.))
        coords.append((0., 0., radius))

    index_offset = vertex_count - segments - 1
    vertex_count += segments
    update_vertex_data = True

    # Define the vertex order of the top pole triangles

    for i in range(index_offset, segments + index_offset):

        indices = array.array("H", [])
        indices.extend((i, i + 1, i + segments + 1))

        if update_vertex_data:
            add_poly(model, indices, vertex_count, values)
            update_vertex_data = False
        else:
            add_poly(model, indices)

        yield [coords[k] for k in indices], "poly_points"

    yield False
