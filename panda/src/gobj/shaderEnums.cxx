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
output_capabilities(std::ostream &out, uint64_t caps) {
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
  if (caps & C_dynamic_indexing) {
    out << "dynamic_indexing ";
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

/**
 *
 */
int ShaderEnums::
get_matrix_deps(StateMatrix inp) {
  int dep = D_none;
  if (inp == SM_model_to_view ||
      inp == SM_view_to_model ||
      inp == SM_model_to_apiview ||
      inp == SM_apiview_to_model) {
    dep |= D_transform & ~D_view_transform;
  }
  if (inp == SM_view_to_world ||
      inp == SM_world_to_view ||
      inp == SM_apiview_to_world ||
      inp == SM_world_to_apiview ||
      inp == SM_view_x_to_view ||
      inp == SM_view_to_view_x ||
      inp == SM_apiview_x_to_view ||
      inp == SM_view_to_apiview_x ||
      inp == SM_clip_x_to_view ||
      inp == SM_view_to_clip_x ||
      inp == SM_apiclip_x_to_view ||
      inp == SM_view_to_apiclip_x) {
    dep |= D_view_transform;
  }
  if (inp == SM_mat_constant_x ||
      inp == SM_vec_constant_x ||
      inp == SM_view_x_to_view ||
      inp == SM_view_to_view_x ||
      inp == SM_apiview_x_to_view ||
      inp == SM_view_to_apiview_x ||
      inp == SM_clip_x_to_view ||
      inp == SM_view_to_clip_x ||
      inp == SM_apiclip_x_to_view ||
      inp == SM_view_to_apiclip_x) {
    dep |= D_shader_inputs;

    if (inp == SM_mat_constant_x ||
        inp == SM_view_x_to_view ||
        inp == SM_view_to_view_x ||
        inp == SM_apiview_x_to_view ||
        inp == SM_view_to_apiview_x ||
        inp == SM_clip_x_to_view ||
        inp == SM_view_to_clip_x ||
        inp == SM_apiclip_x_to_view ||
        inp == SM_view_to_apiclip_x ||
        inp == SM_world_to_apiclip_light_i ||
        inp == SM_point_attenuation) {
      // We can't track changes to these yet, so we have to assume that they
      // are modified every frame.
      dep |= D_frame;
    }
  }
  if (inp == SM_clipplane_x) {
    dep |= D_clip_planes;
  }
  if (inp == SM_clip_to_view ||
      inp == SM_view_to_clip ||
      inp == SM_apiclip_to_view ||
      inp == SM_view_to_apiclip ||
      inp == SM_apiview_to_apiclip ||
      inp == SM_apiclip_to_apiview ||
      inp == SM_point_attenuation) {
    dep |= D_projection;
  }
  if (inp == SM_point_attenuation) {
    dep |= D_scene;
  }
  if (inp == SM_world_to_apiclip_light_i) {
    dep |= D_light;
  }
  return dep;
}
