// Filename: configVariableList.cxx
// Created by:  drose (20Oct04)
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

#include "configVariableList.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
ConfigVariableList::
ConfigVariableList(const string &name, 
                   int trust_level, const string &description, 
                   const string &text) :
  _core(ConfigVariableManager::get_global_ptr()->make_variable(name))
{
  _core->set_value_type(ConfigVariableCore::VT_list);
  if (trust_level > -2) {
    _core->set_trust_level(trust_level);
  }
  if (!description.empty()) {
    _core->set_description(description);
  }
  if (!text.empty()) {
    _core->set_text(text);
  }

  // A list variable implicitly defines a default value of the empty
  // string.  This is just to prevent the core variable from
  // complaining should anyone ask for its solitary value.
  if (_core->get_default_value() == (ConfigDeclaration *)NULL) {
    _core->set_default_value("");
  }
  _core->set_used();
}
