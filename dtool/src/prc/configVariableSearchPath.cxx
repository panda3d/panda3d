/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableSearchPath.cxx
 * @author drose
 * @date 2004-10-21
 */

#include "configVariableSearchPath.h"
#include "executionEnvironment.h"

/**
 * Recopies the config variable into the search path for returning its value.
 */
void ConfigVariableSearchPath::
reload_search_path() {
  nassertv(_core != nullptr);
  mark_cache_valid(_local_modified);
  _cache.clear();

  _cache.append_path(_prefix);
  size_t num_unique_references = _core->get_num_unique_references();
  for (size_t i = 0; i < num_unique_references; i++) {
    const ConfigDeclaration *decl = _core->get_unique_reference(i);

    Filename fn = decl->get_filename_value();
    if (!fn.empty()) {
      _cache.append_directory(std::move(fn));
    }
  }

  if (_prefix.is_empty() && _postfix.is_empty() &&
      num_unique_references == 0) {
    // An empty search path implicitly has the default value.
    _cache = _default_value;
  }

  _cache.append_path(_postfix);
}
