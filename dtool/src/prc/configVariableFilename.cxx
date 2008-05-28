// Filename: configVariableFilename.cxx
// Created by:  drose (22Nov04)
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

#include "configVariableFilename.h"
#include "executionEnvironment.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::reload_cache
//       Access: Private
//  Description: Recopies the config variable into the Filename for
//               returning its value.
////////////////////////////////////////////////////////////////////
void ConfigVariableFilename::
reload_cache() {
  nassertv(_core != (ConfigVariableCore *)NULL);
  mark_cache_valid(_local_modified);

  const ConfigDeclaration *decl = _core->get_declaration(0);
  const ConfigPage *page = decl->get_page();

  Filename page_filename(page->get_name());
  Filename page_dirname = page_filename.get_dirname();
  ExecutionEnvironment::shadow_environment_variable("THIS_PRC_DIR", page_dirname.to_os_specific());

  _cache = Filename::expand_from(decl->get_string_value());
  ExecutionEnvironment::clear_shadow("THIS_PRC_DIR");
}
