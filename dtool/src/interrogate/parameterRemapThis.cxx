// Filename: parameterRemapThis.cxx
// Created by:  drose (02Aug00)
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

#include "parameterRemapThis.h"
#include "typeManager.h"

#include "cppType.h"
#include "cppSimpleType.h"
#include "cppPointerType.h"
#include "cppConstType.h"

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapThis::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ParameterRemapThis::
ParameterRemapThis(CPPType *type, bool is_const) :
  ParameterRemap(TypeManager::get_void_type())
{
  if (is_const) {
    _new_type = TypeManager::wrap_const_pointer(type);
  } else {
    _new_type = TypeManager::wrap_pointer(type);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapThis::pass_parameter
//       Access: Public, Virtual
//  Description: Outputs an expression that converts the indicated
//               variable from the new type to the original type, for
//               passing into the actual C++ function.
////////////////////////////////////////////////////////////////////
void ParameterRemapThis::
pass_parameter(ostream &out, const string &) {
  out << "**invalid**";
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapThis::get_return_expr
//       Access: Public, Virtual
//  Description: Returns an expression that evalutes to the
//               appropriate value type for returning from the
//               function, given an expression of the original type.
////////////////////////////////////////////////////////////////////
string ParameterRemapThis::
get_return_expr(const string &) {
  return "**invalid**";
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapThis::is_this
//       Access: Public, Virtual
//  Description: Returns true if this is the "this" parameter.
////////////////////////////////////////////////////////////////////
bool ParameterRemapThis::
is_this() {
  return true;
}
