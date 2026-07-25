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
#include "bamCacheRecord.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "virtualFile.h"
#include "virtualFileSystem.h"

TypeHandle ShaderModule::_type_handle;

/**
 *
 */
ShaderModule::
ShaderModule(Stage stage) : _stage(stage), _used_caps(C_basic_shader) {
  switch (stage) {
  case Stage::TESS_CONTROL:
  case Stage::TESS_EVALUATION:
    _used_caps |= C_tessellation_shader;
    break;

  case Stage::GEOMETRY:
    _used_caps |= C_geometry_shader;
    break;

  case Stage::COMPUTE:
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
 * Adds the given file to the list of files that the compilation of this
 * module depended on, such as included files.  This is used by
 * check_dependents_modified() to detect if any of them have changed since
 * the module was compiled.
 */
void ShaderModule::
add_dependent_file(const VirtualFile *file) {
  DependentFile df;
  df._pathname = file->get_filename();
  df._timestamp = file->get_timestamp();
  df._size = file->get_file_size();
  _dependent_files.push_back(std::move(df));
}

/**
 * Returns true if any of the files that the compilation of this module
 * depended on have been modified since the module was compiled.
 */
bool ShaderModule::
check_dependents_modified() const {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  for (const DependentFile &df : _dependent_files) {
    PT(VirtualFile) file = vfs->get_file(df._pathname);
    if (file == nullptr) {
      // File no longer exists.
      if (df._timestamp != 0) {
        return true;
      }
    } else {
      if (file->get_timestamp() != df._timestamp ||
          file->get_file_size() != df._size) {
        return true;
      }
    }
  }
  return false;
}

/**
 * Adds all of the files that the compilation of this module depended on as
 * dependent files of the given cache record.
 */
void ShaderModule::
record_dependent_files(BamCacheRecord *record) const {
  for (const DependentFile &df : _dependent_files) {
    record->add_dependent_file(df._pathname);
  }
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
  dg.add_string(_source_entry_point);
  dg.add_uint64(_used_caps);

  dg.add_uint16(_dependent_files.size());
  for (const DependentFile &df : _dependent_files) {
    dg.add_string(df._pathname);
    dg.add_uint32(df._timestamp);
    dg.add_uint64(df._size);
  }
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ShaderModule.
 */
void ShaderModule::
fillin(DatagramIterator &scan, BamReader *manager) {
  // Note that the stage is not read here; it has already been consumed by
  // the make_from_bam function of the derived class, which needed it to
  // construct the module.
  _source_filename = scan.get_string();
  _source_entry_point = scan.get_string();
  _used_caps = scan.get_uint64();

  _dependent_files.clear();
  size_t num_dependent_files = scan.get_uint16();
  _dependent_files.resize(num_dependent_files);
  for (size_t i = 0; i < num_dependent_files; ++i) {
    DependentFile &df = _dependent_files[i];
    df._pathname = scan.get_string();
    df._timestamp = scan.get_uint32();
    df._size = scan.get_uint64();
  }
}
