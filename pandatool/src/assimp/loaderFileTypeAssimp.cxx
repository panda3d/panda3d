/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeAssimp.cxx
 * @author rdb
 * @date 2011-03-29
 */

#include "loaderFileTypeAssimp.h"
#include "config_assimp.h"
#include "assimpLoader.h"

TypeHandle LoaderFileTypeAssimp::_type_handle;

/**
 *
 */
LoaderFileTypeAssimp::
LoaderFileTypeAssimp() : _loader(new AssimpLoader) {
}

/**
 *
 */
LoaderFileTypeAssimp::
~LoaderFileTypeAssimp() {
  if (_loader != nullptr) {
    delete _loader;
  }
}

/**
 *
 */
string LoaderFileTypeAssimp::
get_name() const {
  return "Assimp Importer";
}

/**
 *
 */
string LoaderFileTypeAssimp::
get_extension() const {
  return "";
}

/**
 * Returns a space-separated list of extension, in addition to the one
 * returned by get_extension(), that are recognized by this converter.
 */
string LoaderFileTypeAssimp::
get_additional_extensions() const {
  string exts;
  _loader->get_extensions(exts);
  return exts;
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz or .gz extension), false otherwise.
 */
bool LoaderFileTypeAssimp::
supports_compressed() const {
  return true;
}

/**
 *
 */
PT(PandaNode) LoaderFileTypeAssimp::
load_file(const Filename &path, const LoaderOptions &options,
          BamCacheRecord *record) const {

  assimp_cat.info()
    << "Reading " << path << "\n";

  if (!_loader->read(path)) {
    return nullptr;
  }

  _loader->build_graph();
  return DCAST(PandaNode, _loader->_root);
}
