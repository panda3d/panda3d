// Filename: configVariableBase.cxx
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

#include "configVariableBase.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableBase::Constructor
//       Access: Protected
//  Description: This constructor is only intended to be called from a
//               specialized ConfigVariableFoo derived class.
////////////////////////////////////////////////////////////////////
ConfigVariableBase::
ConfigVariableBase(const string &name, 
                   ConfigVariableBase::ValueType value_type,
                   const string &description, int flags) :
  _core(ConfigVariableManager::get_global_ptr()->make_variable(name))
{
  if (value_type != VT_undefined) {
    _core->set_value_type(value_type);
  }
  if (flags != 0) {
    _core->set_flags(flags);
  }
  if (!description.empty()) {
    _core->set_description(description);
  }
}
