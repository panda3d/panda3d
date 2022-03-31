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

#include <assimp/cimport.h>

using std::string;

TypeHandle LoaderFileTypeAssimp::_type_handle;

/**
 *
 */
LoaderFileTypeAssimp::
LoaderFileTypeAssimp() {
}

/**
 *
 */
LoaderFileTypeAssimp::
~LoaderFileTypeAssimp() {
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
  aiString aexts;
  aiGetExtensionList(&aexts);

  // The format is like: *.mdc;*.mdl;*.mesh.xml;*.mot
  std::string ext;
  char *sub = strtok(aexts.data, ";");
  while (sub != nullptr) {
    ext += sub + 2;
    sub = strtok(nullptr, ";");

    if (sub != nullptr) {
      ext += ' ';
    }
  }

  return ext;
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

  AssimpLoader loader;
  loader.local_object();

  if (!loader.read(path)) {
    return nullptr;
  }

  loader.build_graph();
  return DCAST(PandaNode, loader._root);
}
