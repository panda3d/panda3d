/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderEnums.cxx
 * @author rdb
 * @date 2023-01-22
 */

#include "shaderEnums.h"

/**
 * Returns the stage as a string.
 */
std::string ShaderEnums::
format_stage(Stage stage) {
  switch (stage) {
  case Stage::vertex:
    return "vertex";
  case Stage::tess_control:
    return "tess_control";
  case Stage::tess_evaluation:
    return "tess_evaluation";
  case Stage::geometry:
    return "geometry";
  case Stage::fragment:
    return "fragment";
  case Stage::compute:
    return "compute";
  }

  return "**invalid**";
}

/**
 * Outputs the given capabilities mask.
 */
void ShaderEnums::
output_capabilities(std::ostream &out, int caps) {
  if (caps & C_basic_shader) {
    out << "basic_shader ";
  }
  if (caps & C_vertex_texture) {
    out << "vertex_texture ";
  }
  if (caps & C_point_coord) {
    out << "point_coord ";
  }
  if (caps & C_standard_derivatives) {
    out << "standard_derivatives ";
  }
  if (caps & C_shadow_samplers) {
    out << "shadow_samplers ";
  }
  if (caps & C_non_square_matrices) {
    out << "non_square_matrices ";
  }
  if (caps & C_texture_lod) {
    out << "texture_lod ";
  }
  if (caps & C_unified_model) {
    out << "unified_model ";
  }
  if (caps & C_noperspective_interpolation) {
    out << "noperspective_interpolation ";
  }
  if (caps & C_texture_integer) {
    out << "texture_integer ";
  }
  if (caps & C_texture_query_size) {
    out << "texture_query_size ";
  }
  if (caps & C_sampler_cube_shadow) {
    out << "sampler_cube_shadow ";
  }
  if (caps & C_vertex_id) {
    out << "vertex_id ";
  }
  if (caps & C_draw_buffers) {
    out << "draw_buffers ";
  }
  if (caps & C_clip_distance) {
    out << "clip_distance ";
  }
  if (caps & C_instance_id) {
    out << "instance_id ";
  }
  if (caps & C_texture_buffer) {
    out << "texture_buffer ";
  }
  if (caps & C_geometry_shader) {
    out << "geometry_shader ";
  }
  if (caps & C_primitive_id) {
    out << "primitive_id ";
  }
  if (caps & C_bit_encoding) {
    out << "bit_encoding ";
  }
  if (caps & C_texture_query_lod) {
    out << "texture_query_lod ";
  }
  if (caps & C_texture_gather_red) {
    out << "texture_gather_red ";
  }
  if (caps & C_texture_gather_any) {
    out << "texture_gather_any ";
  }
  if (caps & C_extended_arithmetic) {
    out << "extended_arithmetic ";
  }
  if (caps & C_double) {
    out << "double ";
  }
  if (caps & C_cube_map_array) {
    out << "cube_map_array ";
  }
  if (caps & C_geometry_shader_instancing) {
    out << "geometry_shader_instancing ";
  }
  if (caps & C_tessellation_shader) {
    out << "tessellation_shader ";
  }
  if (caps & C_sample_variables) {
    out << "sample_variables ";
  }
  if (caps & C_multisample_interpolation) {
    out << "multisample_interpolation ";
  }
  if (caps & C_atomic_counters) {
    out << "atomic_counters ";
  }
  if (caps & C_image_load_store) {
    out << "image_load_store ";
  }
  if (caps & C_image_atomic) {
    out << "image_atomic ";
  }
  if (caps & C_image_query_size) {
    out << "image_query_size ";
  }
  if (caps & C_texture_query_levels) {
    out << "texture_query_levels ";
  }
  if (caps & C_storage_buffer) {
    out << "storage_buffer ";
  }
  if (caps & C_compute_shader) {
    out << "compute_shader ";
  }
  if (caps & C_enhanced_layouts) {
    out << "enhanced_layouts ";
  }
  if (caps & C_cull_distance) {
    out << "cull_distance ";
  }
  if (caps & C_derivative_control) {
    out << "derivative_control ";
  }
  if (caps & C_texture_query_samples) {
    out << "texture_query_samples ";
  }
}
