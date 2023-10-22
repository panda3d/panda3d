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
  // This may be called at static init time, so ensure it is constructed now.
  static ConfigVariableString assimp_disable_extensions
  ("assimp-disable-extensions", "gltf glb",
   PRC_DESC("A list of extensions (without preceding dot) that should not be "
            "loaded via the Assimp loader, even if Assimp supports these "
            "formats.  It is useful to set this for eg. gltf and glb files "
            "to prevent them from being accidentally loaded via the Assimp "
            "plug-in instead of via a superior plug-in like panda3d-gltf."));

  bool has_disabled_exts = !assimp_disable_extensions.empty();

  aiString aexts;
  aiGetExtensionList(&aexts);

  char *buffer = (char *)alloca(aexts.length + 2);
  char *p = buffer;

  // The format is like: *.mdc;*.mdl;*.mesh.xml;*.mot
  char *sub = strtok(aexts.data, ";");
  while (sub != nullptr) {
    bool enabled = true;
    if (has_disabled_exts) {
      for (size_t i = 0; i < assimp_disable_extensions.get_num_words(); ++i) {
        std::string disabled_ext = assimp_disable_extensions.get_word(i);
        if (strcmp(sub + 2, disabled_ext.c_str()) == 0) {
          enabled = false;
          break;
        }
      }
    }
    if (enabled) {
      *(p++) = ' ';
      size_t len = strlen(sub + 2);
      memcpy(p, sub + 2, len);
      p += len;
    }

    sub = strtok(nullptr, ";");
  }

  // Strip first space
  ++buffer;
  return std::string(buffer, p - buffer);
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
