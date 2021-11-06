/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModule.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "shaderModule.h"

TypeHandle ShaderModule::_type_handle;

/**
 *
 */
ShaderModule::
ShaderModule(Stage stage) : _stage(stage), _used_caps(C_basic_shader) {
  switch (stage) {
  case Stage::tess_control:
  case Stage::tess_evaluation:
    _used_caps |= C_tessellation_shader;
    break;

  case Stage::geometry:
    _used_caps |= C_geometry_shader;
    break;

  case Stage::compute:
    _used_caps |= C_compute_shader;
    break;

  default:
    break;
  }
}

/**
 *
 */
ShaderModule::
~ShaderModule() {
}

/**
 * Adjusts any input bindings necessary to be able to link up with the given
 * previous stage.  Should return false to indicate that the link is not
 * possible.
 */
bool ShaderModule::
link_inputs(const ShaderModule *previous) {
  // By default we need to do nothing special to link it up, as long as the
  // modules have the same type.
  return get_stage() > previous->get_stage()
      && get_type() == previous->get_type();
}

/**
 * Remaps parameters with a given location to a given other location.  Locations
 * not included in the map remain untouched.
 */
void ShaderModule::
remap_parameter_locations(pmap<int, int> &locations) {
}

/**
 *
 */
void ShaderModule::
output(std::ostream &out) const {
  out << get_type() << " " << get_stage();
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderModule::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_uint8((int)_stage);
  dg.add_string(_source_filename);
  dg.add_uint64(_used_caps);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ShaderModule.
 */
void ShaderModule::
fillin(DatagramIterator &scan, BamReader *manager) {
  _stage = (Stage)scan.get_uint8();
  _source_filename = scan.get_string();
  _used_caps = (int)scan.get_uint64();
}

/**
 * Returns the stage as a string.
 */
std::string ShaderModule::
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
void ShaderModule::
output_capabilities(std::ostream &out, int caps) {
  if (caps & C_basic_shader) {
    out << "basic_shader ";
  }
  if (caps & C_vertex_texture) {
    out << "vertex_texture ";
  }
  if (caps & C_sampler_shadow) {
    out << "sampler_shadow ";
  }
  if (caps & C_invariant) {
    out << "invariant ";
  }
  if (caps & C_matrix_non_square) {
    out << "matrix_non_square ";
  }
  if (caps & C_integer) {
    out << "integer ";
  }
  if (caps & C_texture_lod) {
    out << "texture_lod ";
  }
  if (caps & C_texture_fetch) {
    out << "texture_fetch ";
  }
  if (caps & C_sampler_cube_shadow) {
    out << "sampler_cube_shadow ";
  }
  if (caps & C_vertex_id) {
    out << "vertex_id ";
  }
  if (caps & C_round_even) {
    out << "round_even ";
  }
  if (caps & C_instance_id) {
    out << "instance_id ";
  }
  if (caps & C_buffer_texture) {
    out << "buffer_texture ";
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
  if (caps & C_texture_gather) {
    out << "texture_gather ";
  }
  if (caps & C_double) {
    out << "double ";
  }
  if (caps & C_cube_map_array) {
    out << "cube_map_array ";
  }
  if (caps & C_tessellation_shader) {
    out << "tessellation_shader ";
  }
  if (caps & C_sample_variables) {
    out << "sample_variables ";
  }
  if (caps & C_extended_arithmetic) {
    out << "extended_arithmetic ";
  }
  if (caps & C_texture_query_lod) {
    out << "texture_query_lod ";
  }
  if (caps & C_image_load_store) {
    out << "image_load_store ";
  }
  if (caps & C_compute_shader) {
    out << "compute_shader ";
  }
  if (caps & C_texture_query_levels) {
    out << "texture_query_levels ";
  }
  if (caps & C_enhanced_layouts) {
    out << "enhanced_layouts ";
  }
  if (caps & C_derivative_control) {
    out << "derivative_control ";
  }
  if (caps & C_texture_query_samples) {
    out << "texture_query_samples ";
  }
}
