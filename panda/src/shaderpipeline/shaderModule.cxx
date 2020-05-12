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
ShaderModule(Stage stage) : _stage(stage) {
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
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ShaderModule.
 */
void ShaderModule::
fillin(DatagramIterator &scan, BamReader *manager) {
  _stage = (Stage)scan.get_uint8();
  _source_filename = scan.get_string();
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
