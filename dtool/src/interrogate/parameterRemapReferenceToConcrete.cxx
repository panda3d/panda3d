// Filename: parameterRemapReferenceToConcrete.cxx
// Created by:  drose (04Aug00)
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

#include "parameterRemapReferenceToConcrete.h"
#include "typeManager.h"

#include "cppType.h"
#include "cppDeclaration.h"
#include "cppConstType.h"
#include "cppPointerType.h"
#include "cppReferenceType.h"

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapReferenceToConcrete::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ParameterRemapReferenceToConcrete::
ParameterRemapReferenceToConcrete(CPPType *orig_type) :
  ParameterRemap(orig_type)
{
  _new_type = TypeManager::unwrap_const_reference(orig_type);
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapReferenceToConcrete::pass_parameter
//       Access: Public, Virtual
//  Description: Outputs an expression that converts the indicated
//               variable from the new type to the original type, for
//               passing into the actual C++ function.
////////////////////////////////////////////////////////////////////
void ParameterRemapReferenceToConcrete::
pass_parameter(ostream &out, const string &variable_name) {
  out << variable_name;
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapReferenceToConcrete::get_return_expr
//       Access: Public, Virtual
//  Description: Returns an expression that evalutes to the
//               appropriate value type for returning from the
//               function, given an expression of the original type.
////////////////////////////////////////////////////////////////////
string ParameterRemapReferenceToConcrete::
get_return_expr(const string &expression) {
  return expression;
}

