// Filename: configVariable.cxx
// Created by:  drose (18Oct04)
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

#include "configVariable.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::Constructor
//       Access: Protected
//  Description: This constructor is only intended to be called from a
//               specialized ConfigVariableFoo derived class.
////////////////////////////////////////////////////////////////////
ConfigVariable::
ConfigVariable(const string &name, ConfigVariableCore::ValueType value_type,
               int trust_level, const string &description, 
               const string &text) :
  _core(ConfigVariableManager::get_global_ptr()->make_variable(name))
{
  _core->set_value_type(value_type);
  if (trust_level > -2) {
    _core->set_trust_level(trust_level);
  }
  if (!description.empty()) {
    _core->set_description(description);
  }
  if (!text.empty()) {
    _core->set_text(text);
  }
}
