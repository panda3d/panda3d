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
 * Takes a vector of parameter names and returns a vectort of parameter ids.
 */
pvector<uint32_t> ShaderModule::
get_parameter_ids_from_names(const pvector<const InternalName *> &names) const {
  pvector<uint32_t> ids(names.size(), 0u);

  for (size_t i = 0; i < names.size(); ++i) {
    const InternalName *name = names[i];
    if (name != nullptr) {
      int index = find_parameter(name);
      if (index >= 0) {
        ids[i] = _parameters[index].id;
      }
    }
  }

  return ids;
}

/**
 * Links the stage with the given previous stage, by matching up its inputs with
 * the outputs of the previous stage.  Rather than reassigning the locations
 * directly, this method just returns the location remappings that need to be
 * made, or returns false if the stages cannot be linked.
 */
bool ShaderModule::
link_inputs(const ShaderModule *previous, pmap<int, int> &remap) const {
  // By default we need to do nothing special to link it up, as long as the
  // modules have the same type.
  return get_stage() > previous->get_stage()
      && get_type() == previous->get_type();
}

/**
 * Remaps inputs with a given location to a given other location.  Locations
 * not included in the map remain untouched.
 */
void ShaderModule::
remap_input_locations(const pmap<int, int> &locations) {
}

/**
 * Remaps parameters with a given location to a given other location.  Locations
 * not included in the map remain untouched.
 */
void ShaderModule::
remap_parameter_locations(const pmap<int, int> &locations) {
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
