// Filename: cppInstanceIdentifier.cxx
// Created by:  drose (21Oct99)
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


#include "cppInstanceIdentifier.h"
#include "cppPointerType.h"
#include "cppReferenceType.h"
#include "cppArrayType.h"
#include "cppConstType.h"
#include "cppFunctionType.h"
#include "cppParameterList.h"
#include "cppIdentifier.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::Modifier::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstanceIdentifier::Modifier::
Modifier(CPPInstanceIdentifierType type) :
  _type(type)
{
  _func_params = NULL;
  _func_flags = 0;
  _scoping = NULL;
  _expr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::Modifier::named func_type constructor
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstanceIdentifier::Modifier CPPInstanceIdentifier::Modifier::
func_type(CPPParameterList *params, int flags) {
  Modifier mod(IIT_func);
  mod._func_params = params;
  mod._func_flags = flags;
  return mod;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::Modifier::named array_type constructor
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstanceIdentifier::Modifier CPPInstanceIdentifier::Modifier::
array_type(CPPExpression *expr) {
  Modifier mod(IIT_array);
  mod._expr = expr;
  return mod;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::Modifier::named scoped_pointer_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstanceIdentifier::Modifier CPPInstanceIdentifier::Modifier::
scoped_pointer_type(CPPIdentifier *scoping) {
  Modifier mod(IIT_scoped_pointer);
  mod._scoping = scoping;
  return mod;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstanceIdentifier::
CPPInstanceIdentifier(CPPIdentifier *ident) : _ident(ident) {
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::unroll_type
//       Access: Public
//  Description: Unrolls the list of type punctuation on either side
//               of the identifier to determine the actual type
//               represented by the identifier, given the indicated
//               starting type (that is, the type name written to the
//               left of the identifier).
////////////////////////////////////////////////////////////////////
CPPType *CPPInstanceIdentifier::
unroll_type(CPPType *start_type) {
  CPPType *result = r_unroll_type(start_type, _modifiers.begin());
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::add_modifier
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPInstanceIdentifier::
add_modifier(CPPInstanceIdentifierType type) {
  _modifiers.push_back(Modifier(type));
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::add_modifier
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPInstanceIdentifier::
add_func_modifier(CPPParameterList *params, int flags) {
  _modifiers.push_back(Modifier::func_type(params, flags));
}

void CPPInstanceIdentifier::
add_scoped_pointer_modifier(CPPIdentifier *scoping) {
  _modifiers.push_back(Modifier::scoped_pointer_type(scoping));
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::add_modifier
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPInstanceIdentifier::
add_array_modifier(CPPExpression *expr) {
  _modifiers.push_back(Modifier::array_type(expr));
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::get_scope
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPScope *CPPInstanceIdentifier::
get_scope(CPPScope *current_scope, CPPScope *global_scope,
          CPPPreprocessor *error_sink) const {
  if (_ident == NULL) {
    return current_scope;
  } else {
    return _ident->get_scope(current_scope, global_scope, error_sink);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::r_unroll_type
//       Access: Private
//  Description: The recursive implementation of unroll_type().
////////////////////////////////////////////////////////////////////
CPPType *CPPInstanceIdentifier::
r_unroll_type(CPPType *start_type,
              CPPInstanceIdentifier::Modifiers::const_iterator mi) {
  start_type = CPPType::new_type(start_type);

  if (mi == _modifiers.end()) {
    return start_type;
  }

  const Modifier &mod = (*mi);
  ++mi;

  CPPType *result = NULL;

  switch (mod._type) {
  case IIT_pointer:
    result = new CPPPointerType(r_unroll_type(start_type, mi));
    break;

  case IIT_reference:
    result = new CPPReferenceType(r_unroll_type(start_type, mi));
    break;

  case IIT_scoped_pointer:
    {
      CPPType *type = r_unroll_type(start_type, mi);
      CPPFunctionType *ftype = type->as_function_type();
      if (ftype != NULL) {
        ftype = new CPPFunctionType(*ftype);
        ftype->_class_owner = mod._scoping;
        ftype->_flags |= CPPFunctionType::F_method_pointer;
        type = ftype;
      }
      result = new CPPPointerType(type);
    }
    break;

  case IIT_array:
    result = new CPPArrayType(r_unroll_type(start_type, mi),
                              mod._expr);
    break;

  case IIT_const:
    result = new CPPConstType(r_unroll_type(start_type, mi));
    break;

  case IIT_paren:
    result = r_unroll_type(start_type, mi);
    break;

  case IIT_func:
    {
      CPPType *return_type = r_unroll_type(start_type, mi);
      result = new CPPFunctionType(return_type, mod._func_params,
                                   mod._func_flags);
    }
    break;

  default:
    cerr << "Internal error--invalid CPPInstanceIdentifier\n";
    abort();
  }

  return CPPType::new_type(result);
}
