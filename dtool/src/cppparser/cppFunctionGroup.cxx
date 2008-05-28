// Filename: cppFunctionGroup.cxx
// Created by:  drose (11Nov99)
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


#include "cppFunctionGroup.h"
#include "cppFunctionType.h"
#include "cppInstance.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionGroup::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPFunctionGroup::
CPPFunctionGroup(const string &name) :
  CPPDeclaration(CPPFile()),
  _name(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionGroup::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPFunctionGroup::
~CPPFunctionGroup() {
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionGroup::get_return_type
//       Access: Public
//  Description: If all the functions that share this name have the
//               same return type, returns that type.  Otherwise, if
//               some functions have different return types, returns
//               NULL.
////////////////////////////////////////////////////////////////////
CPPType *CPPFunctionGroup::
get_return_type() const {
  CPPType *return_type = NULL;

  if (!_instances.empty()) {
    Instances::const_iterator ii = _instances.begin();
    return_type = (*ii)->_type->as_function_type()->_return_type;
    ++ii;
    while (ii != _instances.end()) {
      if ((*ii)->_type->as_function_type()->_return_type != return_type) {
        return (CPPType *)NULL;
      }
      ++ii;
    }
  }

  return return_type;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionGroup::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPFunctionGroup::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (!_instances.empty()) {
    Instances::const_iterator ii = _instances.begin();
    (*ii)->output(out, indent_level, scope, complete);
    ++ii;
    while (ii != _instances.end()) {
      out << ";\n";
      indent(out, indent_level);
      (*ii)->output(out, indent_level, scope, complete);
      ++ii;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionGroup::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPFunctionGroup::
get_subtype() const {
  return ST_function_group;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionGroup::as_function_group
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPFunctionGroup *CPPFunctionGroup::
as_function_group() {
  return this;
}
