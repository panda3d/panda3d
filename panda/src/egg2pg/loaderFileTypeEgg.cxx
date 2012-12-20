// Filename: loaderFileTypeEgg.cxx
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

#include "loaderFileTypeEgg.h"
#include "load_egg_file.h"
#include "save_egg_file.h"

#include "eggData.h"

TypeHandle LoaderFileTypeEgg::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileTypeEgg::
LoaderFileTypeEgg() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::get_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeEgg::
get_name() const {
  return "Egg";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::get_extension
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeEgg::
get_extension() const {
  return "egg";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::supports_compressed
//       Access: Published, Virtual
//  Description: Returns true if this file type can transparently load
//               compressed files (with a .pz extension), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool LoaderFileTypeEgg::
supports_compressed() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::supports_load
//       Access: Published, Virtual
//  Description: Returns true if the file type can be used to load
//               files, and load_file() is supported.  Returns false
//               if load_file() is unimplemented and will always fail.
////////////////////////////////////////////////////////////////////
bool LoaderFileTypeEgg::
supports_load() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::supports_save
//       Access: Published, Virtual
//  Description: Returns true if the file type can be used to save
//               files, and save_file() is supported.  Returns false
//               if save_file() is unimplemented and will always fail.
////////////////////////////////////////////////////////////////////
bool LoaderFileTypeEgg::
supports_save() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::load_file
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(PandaNode) LoaderFileTypeEgg::
load_file(const Filename &path, const LoaderOptions &, 
          BamCacheRecord *record) const {
  PT(PandaNode) result = load_egg_file(path, CS_default, record);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::save_file
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool LoaderFileTypeEgg::
save_file(const Filename &path, const LoaderOptions &options,
          PandaNode *node) const {
  return save_egg_file(path, node);
}
