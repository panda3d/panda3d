// Filename: loaderFileType.cxx
// Created by:  drose (20Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "loaderFileType.h"
#include "loaderOptions.h"
#include "config_pgraph.h"


TypeHandle LoaderFileType::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileType::
LoaderFileType() {
  // Derived LoaderFileType classes that return a different result
  // based on the setting of certain LoaderOptions flags (like
  // LF_convert_anim) should set those bits in the following bitmask,
  // so that we will not inadvertently cache a model without
  // respecting these flags.
  _no_cache_flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileType::
~LoaderFileType() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::get_additional_extensions
//       Access: Published, Virtual
//  Description: Returns a space-separated list of extension, in
//               addition to the one returned by get_extension(), that
//               are recognized by this loader.
////////////////////////////////////////////////////////////////////
string LoaderFileType::
get_additional_extensions() const {
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::supports_compressed
//       Access: Published, Virtual
//  Description: Returns true if this file type can transparently load
//               compressed files (with a .pz extension), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool LoaderFileType::
supports_compressed() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::get_allow_disk_cache
//       Access: Published, Virtual
//  Description: Returns true if the loader flags allow retrieving the
//               model from the on-disk bam cache (if it is enabled),
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LoaderFileType::
get_allow_disk_cache(const LoaderOptions &options) const {
  return (options.get_flags() & (LoaderOptions::LF_no_disk_cache | _no_cache_flags)) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::get_allow_ram_cache
//       Access: Published, Virtual
//  Description: Returns true if the loader flags allow retrieving the
//               model from the in-memory ModelPool cache, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool LoaderFileType::
get_allow_ram_cache(const LoaderOptions &options) const {
  return (options.get_flags() & (LoaderOptions::LF_no_ram_cache | _no_cache_flags)) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::supports_load
//       Access: Published, Virtual
//  Description: Returns true if the file type can be used to load
//               files, and load_file() is supported.  Returns false
//               if load_file() is unimplemented and will always fail.
////////////////////////////////////////////////////////////////////
bool LoaderFileType::
supports_load() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::supports_save
//       Access: Published, Virtual
//  Description: Returns true if the file type can be used to save
//               files, and save_file() is supported.  Returns false
//               if save_file() is unimplemented and will always fail.
////////////////////////////////////////////////////////////////////
bool LoaderFileType::
supports_save() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::load_file
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(PandaNode) LoaderFileType::
load_file(const Filename &path, const LoaderOptions &options,
          BamCacheRecord *record) const {
  loader_cat.error()
    << get_type() << " cannot read PandaNode objects.\n";
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::save_file
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool LoaderFileType::
save_file(const Filename &path, const LoaderOptions &options,
          PandaNode *node) const {
  loader_cat.error()
    << get_type() << " cannot save PandaNode objects.\n";
  return false;
}
