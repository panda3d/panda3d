// Filename: configVariableSearchPath.cxx
// Created by:  drose (21Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "configVariableSearchPath.h"
#include "executionEnvironment.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableSearchPath::reload_search_path
//       Access: Private
//  Description: Recopies the config variable into the search path for
//               returning its value.
////////////////////////////////////////////////////////////////////
void ConfigVariableSearchPath::
reload_search_path() {
  nassertv(_core != (ConfigVariableCore *)NULL);
  mark_cache_valid(_local_modified);
  _cache.clear();

  _cache.append_path(_prefix);
  int num_unique_references = _core->get_num_unique_references();
  for (int i = 0; i < num_unique_references; i++) {
    const ConfigDeclaration *decl = _core->get_unique_reference(i);
    const ConfigPage *page = decl->get_page();

    Filename page_filename(page->get_name());
    Filename page_dirname = page_filename.get_dirname();
    ExecutionEnvironment::shadow_environment_variable("THIS_PRC_DIR", page_dirname.to_os_specific());
    string expanded = ExecutionEnvironment::expand_string(decl->get_string_value());
    ExecutionEnvironment::clear_shadow("THIS_PRC_DIR");
    if (!expanded.empty()) {
      _cache.append_directory(Filename::from_os_specific(expanded));
    }
  }
  _cache.append_path(_postfix);

  if (_cache.is_empty()) {
    // An empty search path implicitly has "." on it.
    _cache.append_directory(".");
  }
}
