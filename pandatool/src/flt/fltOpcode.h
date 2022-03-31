/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltOpcode.h
 * @author drose
 * @date 2000-08-24
 */

#ifndef FLTOPCODE_H
#define FLTOPCODE_H

#include "pandatoolbase.h"

// Known opcodes, as of the latest version of flt.
enum FltOpcode {
  FO_none                = 0,
  FO_header              = 1,
  FO_group               = 2,
  FO_OB_scale            = 3,    // obsolete
  FO_object              = 4,
  FO_face                = 5,
  FO_OB_vertex_i         = 6,    // obsolete
  FO_OB_short_vertex     = 7,    // obsolete
  FO_OB_vertex_c         = 8,    // obsolete
  FO_OB_vertex_cn        = 9,    // obsolete
  FO_push                = 10,
  FO_pop                 = 11,
  FO_OB_translate        = 12,   // obsolete
  FO_OB_dof              = 13,   // obsolete
  FO_dof                 = 14,
  FO_OB_instance_ref     = 16,   // obsolete
  FO_OB_instance         = 17,   // obsolete
  FO_push_face           = 19,
  FO_pop_face            = 20,
  FO_push_extension      = 21,
  FO_pop_extension       = 22,
  FO_continuation        = 23,

  FO_comment             = 31,
  FO_color_palette       = 32,
  FO_long_id             = 33,
  FO_OB_translate2       = 40,   // obsolete
  FO_OB_rotate_point     = 41,   // obsolete
  FO_OB_rotate_edge      = 42,   // obsolete
  FO_OB_scale2           = 43,   // obsolete
  FO_OB_translate3       = 44,   // obsolete
  FO_OB_nu_scale         = 45,   // obsolete
  FO_OB_rotate_point2    = 46,   // obsolete
  FO_OB_rotate_to_point  = 47,   // obsolete
  FO_OB_put              = 48,   // obsolete
  FO_transform_matrix    = 49,
  FO_vector              = 50,
  FO_OB_bounding_box     = 51,   // obsolete
  FO_multitexture        = 52,
  FO_uv_list             = 53,
  FO_bsp                 = 55,
  FO_replicate           = 60,
  FO_instance_ref        = 61,
  FO_instance            = 62,
  FO_external_ref        = 63,
  FO_texture             = 64,
  FO_OB_eyepoint_palette = 65,   // obsolete
  FO_14_material_palette = 66,
  FO_vertex_palette      = 67,
  FO_vertex_c            = 68,
  FO_vertex_cn           = 69,
  FO_vertex_cnu          = 70,
  FO_vertex_cu           = 71,
  FO_vertex_list         = 72,
  FO_lod                 = 73,
  FO_bounding_box        = 74,
  FO_rotate_about_edge   = 76,
  FO_OB_scale3           = 77,   // obsolete
  FO_translate           = 78,
  FO_scale               = 79,
  FO_rotate_about_point  = 80,
  FO_rotate_and_scale    = 81,
  FO_put                 = 82,
  FO_eyepoint_palette    = 83,
  FO_mesh                = 84,
  FO_local_vertex_pool   = 85,
  FO_mesh_primitive      = 86,
  FO_road_segment        = 87,
  FO_road_zone           = 88,
  FO_morph_list          = 89,
  FO_behavior_palette    = 90,
  FO_sound               = 91,
  FO_road_path           = 92,
  FO_sound_palette       = 93,
  FO_general_matrix      = 94,
  FO_text                = 95,
  FO_switch              = 96,
  FO_line_style          = 97,
  FO_clip_region         = 98,
  FO_extension           = 100,
  FO_light_source        = 101,
  FO_light_definition    = 102,
  FO_bounding_sphere     = 105,
  FO_bounding_cylinder   = 106,
  FO_bv_center           = 108,
  FO_bv_orientation      = 109,
  FO_light_point         = 111,
  FO_texture_map_palette = 112,
  FO_15_material         = 113,
  FO_name_table          = 114,
  FO_cat                 = 115,
  FO_cat_data            = 116,
  FO_push_attribute      = 122,
  FO_pop_attribute       = 123,
  FO_adaptive_attribute  = 125,
  FO_curve               = 126,
  FO_road_construction   = 127
};

std::ostream &operator << (std::ostream &out, FltOpcode opcode);

#endif
