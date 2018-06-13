/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeDae.cxx
 * @author rdb
 * @date 2009-08-23
 */

#include "loaderFileTypeDae.h"
#include "load_collada_file.h"

TypeHandle LoaderFileTypeDae::_type_handle;

/**
 *
 */
LoaderFileTypeDae::
LoaderFileTypeDae() {
}

/**
 *
 */
std::string LoaderFileTypeDae::
get_name() const {
#if PANDA_COLLADA_VERSION == 14
  return "COLLADA 1.4";
#elif PANDA_COLLADA_VERSION == 15
  return "COLLADA 1.5";
#else
  return "COLLADA";
#endif
}

/**
 *
 */
std::string LoaderFileTypeDae::
get_extension() const {
  return "dae";
}

/**
 * Returns a space-separated list of extension, in addition to the one
 * returned by get_extension(), that are recognized by this loader.
 */
std::string LoaderFileTypeDae::
get_additional_extensions() const {
  return "zae";
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz or .gz extension), false otherwise.
 */
bool LoaderFileTypeDae::
supports_compressed() const {
  return true;
}

/**
 *
 */
PT(PandaNode) LoaderFileTypeDae::
load_file(const Filename &path, const LoaderOptions &,
          BamCacheRecord *record) const {
  PT(PandaNode) result = load_collada_file(path, CS_default, record);
  return result;
}
