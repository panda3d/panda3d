// Filename: loaderFileTypeAssimp.cxx
// Created by:  rdb (29Mar11)
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

#include "loaderFileTypeAssimp.h"
#include "config_assimp.h"
#include "assimpLoader.h"

TypeHandle LoaderFileTypeAssimp::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeAssimp::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileTypeAssimp::
LoaderFileTypeAssimp() : _loader(new AssimpLoader) {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeAssimp::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileTypeAssimp::
~LoaderFileTypeAssimp() {
  if (_loader != NULL) {
    delete _loader;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeAssimp::get_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeAssimp::
get_name() const {
  return "Assimp Importer";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeAssimp::get_extension
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeAssimp::
get_extension() const {
  return "";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeAssimp::get_additional_extensions
//       Access: Public, Virtual
//  Description: Returns a space-separated list of extension, in
//               addition to the one returned by get_extension(), that
//               are recognized by this converter.
////////////////////////////////////////////////////////////////////
string LoaderFileTypeAssimp::
get_additional_extensions() const {
  string exts;
  _loader->get_extensions(exts);
  return exts;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeAssimp::supports_compressed
//       Access: Published, Virtual
//  Description: Returns true if this file type can transparently load
//               compressed files (with a .pz extension), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool LoaderFileTypeAssimp::
supports_compressed() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeAssimp::load_file
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(PandaNode) LoaderFileTypeAssimp::
load_file(const Filename &path, const LoaderOptions &options,
          BamCacheRecord *record) const {

  assimp_cat.info()
    << "Reading " << path << "\n";

  if (!_loader->read(path)) {
    return NULL;
  }

  _loader->build_graph();
  return DCAST(PandaNode, _loader->_root);
}
