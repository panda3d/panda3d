/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModuleGlsl.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "shaderModuleGlsl.h"
#include "string_utils.h"

TypeHandle ShaderModuleGlsl::_type_handle;

ShaderModuleGlsl::
ShaderModuleGlsl(Stage stage, std::string source, int version) :
  ShaderModule(stage),
  _code(std::move(source)),
  _version(0)
{
  if (version >= 130) { // also matches ESSL 3.00+
    _used_caps |= C_unified_model;
  }

  // Strip trailing whitespace.
  while (!_code.empty() && isspace(_code[_code.size() - 1])) {
    _code.resize(_code.size() - 1);
  }

  // Except add back a newline at the end, which is needed by Intel drivers.
  _code += "\n";
}

ShaderModuleGlsl::
~ShaderModuleGlsl() {
}

/**
 * Required to implement CopyOnWriteObject.
 */
PT(CopyOnWriteObject) ShaderModuleGlsl::
make_cow_copy() {
  return new ShaderModuleGlsl(*this);
}

std::string ShaderModuleGlsl::
get_ir() const {
  return this->_code;
}

/**
 * Lists the given filename as having been included by this shader.  A unique
 * number identifying the file is returned that can later be passed to
 * get_filename_from_index.
 */
int ShaderModuleGlsl::
add_included_file(Filename fn) {
  int fileno = _fileno_offset + _included_files.size();
  _included_files.push_back(std::move(fn));
  return fileno;
}

/**
 * Returns the filename of the included shader with the given source file
 * index (as recorded in the #line statement in r_preprocess_source).  We use
 * this to associate error messages with included files.
 */
Filename ShaderModuleGlsl::
get_filename_from_index(int index) const {
  if (index == 0) {
    Filename fn = get_source_filename();
    if (!fn.empty()) {
      return fn;
    }
  }
  else if (index >= _fileno_offset && (index - _fileno_offset) < (int)_included_files.size()) {
    return _included_files[(size_t)index - _fileno_offset];
  }

  // Must be a mistake.  Quietly put back the integer.
  std::string str = format_string(index);
  return Filename(str);
}

/**
 * Tells the BamReader how to create objects of type ShaderModuleGlsl.
 */
void ShaderModuleGlsl::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderModuleGlsl::
write_datagram(BamWriter *manager, Datagram &dg) {
  ShaderModule::write_datagram(manager, dg);

  dg.add_int16(_version);
  dg.add_string32(_code);

  dg.add_uint16(_included_files.size());
  for (const Filename &fn : _included_files) {
    dg.add_string(fn);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderModule is encountered in the Bam file.  It should create the
 * ShaderModule and extract its information from the file.
 */
TypedWritable *ShaderModuleGlsl::
make_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  Stage stage = (Stage)scan.get_uint8();
  ShaderModuleGlsl *module = new ShaderModuleGlsl(stage, std::string(), 0);
  module->fillin(scan, manager);

  return module;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ShaderModuleGlsl.
 */
void ShaderModuleGlsl::
fillin(DatagramIterator &scan, BamReader *manager) {
  _source_filename = scan.get_string();
  _used_caps = (int)scan.get_uint64();

  _version = scan.get_int16();
  _code = scan.get_string32();

  size_t num_included_files = scan.get_uint16();
  _included_files.resize(num_included_files);
  for (size_t i = 0; i < num_included_files; ++i) {
    _included_files[i] = scan.get_string();
  }
}
