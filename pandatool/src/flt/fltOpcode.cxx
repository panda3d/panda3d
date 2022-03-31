/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltOpcode.cxx
 * @author drose
 * @date 2000-08-24
 */

#include "fltOpcode.h"

std::ostream &
operator << (std::ostream &out, FltOpcode opcode) {
  switch (opcode) {
  case FO_none:
    return out << "null opcode";

  case FO_header:
    return out << "header";

  case FO_group:
    return out << "group";

  case FO_OB_scale:
  case FO_OB_scale2:
  case FO_OB_scale3:
    return out << "(obsolete) scale";

  case FO_object:
    return out << "object";

  case FO_face:
    return out << "face";

  case FO_OB_vertex_i:
    return out << "(obsolete) vertex with ID";

  case FO_OB_short_vertex:
    return out << "(obsolete) short vertex";

  case FO_OB_vertex_c:
    return out << "(obsolete) vertex with color";

  case FO_OB_vertex_cn:
    return out << "(obsolete) vertex with color and normal";

  case FO_push:
    return out << "push";

  case FO_pop:
    return out << "pop";

  case FO_OB_translate:
  case FO_OB_translate2:
  case FO_OB_translate3:
    return out << "(obsolete) translate";

  case FO_OB_dof:
    return out << "(obsolete) degree-of-freedom";

  case FO_dof:
    return out << "degree-of-freedom";

  case FO_OB_instance_ref:
    return out << "(obsolete) instance reference";

  case FO_OB_instance:
    return out << "(obsolete) instance definition";

  case FO_push_face:
    return out << "push subface";

  case FO_pop_face:
    return out << "pop subface";

  case FO_push_extension:
    return out << "push extension";

  case FO_pop_extension:
    return out << "pop extension";

  case FO_continuation:
    return out << "continuation";

  case FO_comment:
    return out << "comment";

  case FO_color_palette:
    return out << "color palette";

  case FO_long_id:
    return out << "long ID";

  case FO_transform_matrix:
    return out << "transformation matrix";

  case FO_OB_rotate_point:
  case FO_OB_rotate_point2:
    return out << "(obsolete) rotate about point";

  case FO_OB_rotate_edge:
    return out << "(obsolete) rotate about edge";

  case FO_OB_nu_scale:
    return out << "(obsolete) non-uniform scale";

  case FO_OB_rotate_to_point:
    return out << "(obsolete) rotate to point";

  case FO_OB_put:
    return out << "(obsolete) put";

  case FO_OB_bounding_box:
    return out << "(obsolete) bounding box";

  case FO_vector:
    return out << "vector";

  case FO_multitexture:
    return out << "multitexture";

  case FO_uv_list:
    return out << "UV list";

  case FO_bsp:
    return out << "BSP";

  case FO_replicate:
    return out << "replicate";

  case FO_instance_ref:
    return out << "instance reference";

  case FO_instance:
    return out << "instance definition";

  case FO_external_ref:
    return out << "external reference";

  case FO_texture:
    return out << "texture";

  case FO_OB_eyepoint_palette:
    return out << "(obsolete) eyepoint palette";

  case FO_14_material_palette:
    return out << "v14 material palette";

  case FO_vertex_palette:
    return out << "vertex palette";

  case FO_vertex_c:
    return out << "vertex with color";

  case FO_vertex_cn:
    return out << "vertex with color and normal";

  case FO_vertex_cnu:
    return out << "vertex with color, normal, and uv";

  case FO_vertex_cu:
    return out << "vertex with color and uv";

  case FO_vertex_list:
    return out << "vertex list";

  case FO_lod:
    return out << "LOD";

  case FO_bounding_box:
    return out << "bounding box";

  case FO_rotate_about_edge:
    return out << "rotate about edge";

  case FO_translate:
    return out << "translate";

  case FO_scale:
    return out << "scale";

  case FO_rotate_about_point:
    return out << "rotate about point";

  case FO_rotate_and_scale:
    return out << "rotate and/or scale";

  case FO_put:
    return out << "put";

  case FO_eyepoint_palette:
    return out << "eyepoint palette";

  case FO_mesh:
    return out << "mesh";

  case FO_local_vertex_pool:
    return out << "local vertex pool";

  case FO_mesh_primitive:
    return out << "mesh primitive";

  case FO_road_segment:
    return out << "road segment";

  case FO_road_zone:
    return out << "road zone";

  case FO_morph_list:
    return out << "morph vertex list";

  case FO_behavior_palette:
    return out << "behavior palette";

  case FO_sound:
    return out << "sound";

  case FO_road_path:
    return out << "road path";

  case FO_sound_palette:
    return out << "sound palette";

  case FO_general_matrix:
    return out << "general matrix";

  case FO_text:
    return out << "text";

  case FO_switch:
    return out << "switch";

  case FO_line_style:
    return out << "line style";

  case FO_clip_region:
    return out << "clip region";

  case FO_light_source:
    return out << "light source";

  case FO_light_definition:
    return out << "light source definition";

  case FO_bounding_sphere:
    return out << "bounding sphere";

  case FO_bounding_cylinder:
    return out << "bounding cylinder";

  case FO_bv_center:
    return out << "bounding volume center";

  case FO_bv_orientation:
    return out << "bounding volume orientation";

  case FO_light_point:
    return out << "light point";

  case FO_texture_map_palette:
    return out << "texture mapping palette";

  case FO_15_material:
    return out << "material";

  case FO_name_table:
    return out << "name table";

  case FO_cat:
    return out << "continuously adaptive terrain";

  case FO_cat_data:
    return out << "CAT Data";

  case FO_push_attribute:
    return out << "push attribute";

  case FO_pop_attribute:
    return out << "pop attribute";

  case FO_adaptive_attribute:
    return out << "adaptive attribute";

  case FO_curve:
    return out << "curve";

  case FO_road_construction:
    return out << "road construction";

  default:
    return out << "unknown opcode " << (int)opcode;
  }
}
