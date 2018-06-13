/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeEgg.cxx
 * @author drose
 * @date 2000-06-20
 */

#include "loaderFileTypeEgg.h"
#include "load_egg_file.h"
#include "save_egg_file.h"

#include "eggData.h"

TypeHandle LoaderFileTypeEgg::_type_handle;

/**
 *
 */
LoaderFileTypeEgg::
LoaderFileTypeEgg() {
}

/**
 *
 */
std::string LoaderFileTypeEgg::
get_name() const {
  return "Egg";
}

/**
 *
 */
std::string LoaderFileTypeEgg::
get_extension() const {
  return "egg";
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz or .gz extension), false otherwise.
 */
bool LoaderFileTypeEgg::
supports_compressed() const {
  return true;
}

/**
 * Returns true if the file type can be used to load files, and load_file() is
 * supported.  Returns false if load_file() is unimplemented and will always
 * fail.
 */
bool LoaderFileTypeEgg::
supports_load() const {
  return true;
}

/**
 * Returns true if the file type can be used to save files, and save_file() is
 * supported.  Returns false if save_file() is unimplemented and will always
 * fail.
 */
bool LoaderFileTypeEgg::
supports_save() const {
  return true;
}

/**
 *
 */
PT(PandaNode) LoaderFileTypeEgg::
load_file(const Filename &path, const LoaderOptions &,
          BamCacheRecord *record) const {
  PT(PandaNode) result = load_egg_file(path, CS_default, record);
  return result;
}

/**
 *
 */
bool LoaderFileTypeEgg::
save_file(const Filename &path, const LoaderOptions &options,
          PandaNode *node) const {
  return save_egg_file(path, node);
}
