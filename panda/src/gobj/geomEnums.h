/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomEnums.h
 * @author drose
 * @date 2005-04-14
 */

#ifndef GEOMENUMS_H
#define GEOMENUMS_H

#include "pandabase.h"

/**
 * This class exists just to provide scoping for the various enumerated types
 * used by Geom, GeomVertexData, GeomVertexArrayData, GeomPrimitive, and other
 * related classes.
 */
class EXPCL_PANDA_GOBJ GeomEnums {
PUBLISHED:

  // The usage hint describes to the rendering backend how often the data in
  // question will be modified andor rendered.  It allows the backend to make
  // appropriate choices about what part of memory the data should be stored
  // in.

  // The hint is provided as a performance optimization only, and does not
  // constrain actual usage; although it may be an important optimization.

  enum UsageHint {
    // The following are intentionally ordered from most dynamic to most
    // static.  In general, if usage_a < usage_b, then usage_a is more dynamic
    // than usage_b.

    // UH_client: don't attempt to upload the data; always keep it on the
    // client.
    UH_client,

    // UH_stream: the data will be created once, used to render a few times,
    // and then discarded.  This should be used for short-lived temporary
    // objects.
    UH_stream,

    // UH_dynamic: the data will be repeatedly modified and re-rendered.  This
    // is for data that will be modified at runtime, such as animated or soft-
    // skinned vertices.
    UH_dynamic,

    // UH_static: the data will be created once, and used to render many
    // times, without modification.  This is the most common case, since
    // typically vertex data is not directly animated (this is not related to
    // scene graph animation, e.g.  from adjusting transforms on a node).
    UH_static,

    // UH_unspecified: the usage is unspecified.  This is intended as a "don't
    // care" option for abstract objects; it should not be applied to any
    // actual geometry to be rendered.  You take your chances if a geom
    // actually gets into the scene graph with this set.
    UH_unspecified,
  };

  // This type specifies a number of bits that are used to represent the
  // rendering requirements of a particular Geom, as well as the rendering
  // capabilities of the GSG.  The difference between the two indicates
  // whether the Geom needs to be munged for the GSG.
  enum GeomRendering {
    // If there are indexed points.
    GR_indexed_point        = 0x00001,

    // If there is indexed geometry of any other type.
    GR_indexed_other        = 0x10000,

    // The union of all of the indexed attributes.
    GR_indexed_bits         = 0x10001,

    // If there are any points at all.
    GR_point                = 0x00002,

    // If the points are all the same size, other than 1 pixel.
    GR_point_uniform_size   = 0x00004,

    // If the points have a per-vertex size designation.
    GR_per_point_size       = 0x00008,

    // If the points' size is specified in camera units rather than screen
    // pixels.
    GR_point_perspective    = 0x00010,

    // If the points have a non-square aspect ratio.
    GR_point_aspect_ratio   = 0x00020,

    // If the points are under a scale transform, uniform or non-uniform.
    GR_point_scale          = 0x00040,

    // If the points are rotated off the orthonormal axis.
    GR_point_rotate         = 0x00080,

    // If the points require texture coordinates interpolated across their
    // face, to render textures as sprites.
    GR_point_sprite         = 0x00100,

    // If there is a texture matrix applied to the sprite's generated texture
    // coordinates.
    GR_point_sprite_tex_matrix = 0x00200,

    // The union of all the above point attributes, except GR_indexed_point.
    GR_point_bits           = 0x003fe,

    // If there are any of these composite types.
    GR_triangle_strip       = 0x00400,
    GR_triangle_fan         = 0x00800,
    GR_line_strip           = 0x01000,

    // The union of all of the above composite types.
    GR_composite_bits       = 0x01c00,

    // If strip-cut indices are used to restart a composite primitive.
    GR_strip_cut_index      = 0x20000,

    // If the shade model requires a particular vertex for flat shading.
    GR_flat_first_vertex    = 0x02000,
    GR_flat_last_vertex     = 0x04000,

    // The union of the above shade model types.
    GR_shade_model_bits     = 0x06000,

    // If a particular non-fill polygon mode is used.
    GR_render_mode_wireframe= 0x40000,
    GR_render_mode_point    = 0x80000,

    // The primitive has adjacency information.
    GR_adjacency            = 0x100000,
  };

  // The shade model specifies whether the per-vertex colors and normals
  // indexed by a given primitive truly represent per-vertex colors and
  // normals, or whether they actually represent per-triangle flat-shaded
  // colors and normals.
  enum ShadeModel {
    // SM_uniform: all vertices across all faces have the same colors and
    // normals.  It doesn't really matter which ShadeModelAttrib mode is used
    // to render this primitive.
    SM_uniform,

    // SM_smooth: vertices within a single face have different colorsnormals
    // that should be smoothed across the face.  This primitive should be
    // rendered with SmoothModelAttrib::M_smooth.
    SM_smooth,

    // SM_flat_(first,last)_vertex: each face within the primitive might have
    // a different colornormal than the other faces, but across a particular
    // face there is only one colornormal.  Each face's colornormal is taken
    // from the (first, last) vertex of the face.  This primitive should be
    // rendered with SmoothModelAttrib::M_flat.
    SM_flat_first_vertex,
    SM_flat_last_vertex,
  };

  // The primitive type represents the core primitive type of a particular
  // GeomPrimitive.  It's used for determining what kind of antialiasing
  // should be enabled.
  enum PrimitiveType {
    PT_none,
    PT_polygons,
    PT_lines,
    PT_points,
    PT_patches
  };

  // The numeric type determines what physical representation is used to
  // encode a numeric value within the vertex data.
  enum NumericType {
    NT_uint8,        // An integer 0..255
    NT_uint16,       // An integer 0..65535
    NT_uint32,       // An integer 0..4294967295
    NT_packed_dcba,  // DirectX style, four byte values packed in a uint32
    NT_packed_dabc,  // DirectX packed color order (ARGB)
    NT_float32,      // A single-precision float
    NT_float64,      // A double-precision float
    NT_stdfloat,     // Either single- or double-precision, according to vertices-float64.
    NT_int8,         // An integer -128..127
    NT_int16,        // An integer -32768..32767
    NT_int32,        // An integer -2147483648..2147483647
    NT_packed_ufloat,// Three 10/11-bit float components packed in a uint32
  };

  // The contents determine the semantic meaning of a numeric value within the
  // vertex data.  This is also used to determine what automatic transforms
  // might be applied to the various columns.
  enum Contents {
    C_other,        // Arbitrary meaning, leave it alone
    C_point,        // A point in 3-space or 4-space
    C_clip_point,   // A point pre-transformed into clip coordinates
    C_vector,       // A surface tangent or binormal (see C_normal for normals)
    C_texcoord,     // A texture coordinate
    C_color,        // 3- or 4-component color, ordered R, G, B, [A]
    C_index,        // An index value into some other table
    C_morph_delta,  // A delta from some base value, defining a blend shape

    // A transformation matrix.  This is typically three or four columns, but
    // we pretend it's only one for convenience.
    C_matrix,

    // A special version of C_vector that should be used for normal vectors,
    // which are scaled differently from other vectors.
    C_normal,
  };

  // The type of animation data that is represented by a particular
  // GeomVertexFormat.
  enum AnimationType {
    AT_none,     // No vertex animation.
    AT_panda,    // Vertex animation calculated on the CPU by Panda.
    AT_hardware, // Hardware-accelerated animation on the graphics card.
  };
};

EXPCL_PANDA_GOBJ std::ostream &operator << (std::ostream &out, GeomEnums::UsageHint usage_hint);
EXPCL_PANDA_GOBJ std::istream &operator >> (std::istream &in, GeomEnums::UsageHint &usage_hint);
EXPCL_PANDA_GOBJ std::ostream &operator << (std::ostream &out, GeomEnums::NumericType numeric_type);
EXPCL_PANDA_GOBJ std::ostream &operator << (std::ostream &out, GeomEnums::Contents contents);

#endif
