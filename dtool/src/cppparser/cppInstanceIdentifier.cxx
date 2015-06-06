// Filename: cppInstanceIdentifier.cxx
// Created by:  drose (21Oct99)
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
//     Function: CPPInstanceIdentifier::Modifier::named initializer_type constructor
//       Access: Public, Static
//  Description: This is used only for instance declarations that turn
//               out to be have a parameter list for an initializer.
////////////////////////////////////////////////////////////////////
CPPInstanceIdentifier::Modifier CPPInstanceIdentifier::Modifier::
initializer_type(CPPParameterList *params) {
  Modifier mod(IIT_initializer);
  mod._func_params = params;
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
//     Function: CPPInstanceIdentifier::add_func_modifier
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPInstanceIdentifier::
add_func_modifier(CPPParameterList *params, int flags) {
  // As a special hack, if we added a parameter list to an operator
  // function, check if the parameter list is empty.  If it is, this
  // is really a unary operator, so set the unary_op flag.  Operators
  // () and [] are never considered unary operators.
  if (_ident != NULL &&
      _ident->get_simple_name().substr(0, 9) == "operator ") {

    if (_ident->get_simple_name() != string("operator ()") &&
        _ident->get_simple_name() != string("operator []")) {
      if (params->_parameters.empty()) {
        flags |= CPPFunctionType::F_unary_op;
      }
    }

    flags |= CPPFunctionType::F_operator;
  }

  _modifiers.push_back(Modifier::func_type(params, flags));
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::add_scoped_pointer_modifier
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPInstanceIdentifier::
add_scoped_pointer_modifier(CPPIdentifier *scoping) {
  _modifiers.push_back(Modifier::scoped_pointer_type(scoping));
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::add_array_modifier
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPInstanceIdentifier::
add_array_modifier(CPPExpression *expr) {
  // Special case for operator new[] and delete[].  We're not really
  // adding an array modifier to them, but appending [] to the
  // identifier.  This is to work around a parser ambiguity.
  if (_ident != NULL && (_ident->get_simple_name() == "operator delete" ||
                         _ident->get_simple_name() == "operator new")) {

    _ident->_names.back().append_name("[]");
  } else {
    _modifiers.push_back(Modifier::array_type(expr));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::add_initializer_modifier
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPInstanceIdentifier::
add_initializer_modifier(CPPParameterList *params) {
  _modifiers.push_back(Modifier::initializer_type(params));
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstanceIdentifier::get_initializer
//       Access: Public
//  Description: Returns the initializer parameter list that was set
//               for this particular instance, e.g. if the instance
//               were:
//
//                  int foo(0);
//
//               this would return the parameter list (0).  Returns
//               NULL if the instance did not use a parameter list
//               initializer.
////////////////////////////////////////////////////////////////////
CPPParameterList *CPPInstanceIdentifier::
get_initializer() const {
  Modifiers::const_iterator mi;
  for (mi = _modifiers.begin(); mi != _modifiers.end(); ++mi) {
    const Modifier &mod = (*mi);
    if (mod._type == IIT_initializer) {
      return mod._func_params;
    }
  }

  return NULL;
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
    result = new CPPReferenceType(r_unroll_type(start_type, mi),
                                  CPPReferenceType::VC_lvalue);
    break;

  case IIT_rvalue_reference:
    result = new CPPReferenceType(r_unroll_type(start_type, mi),
                                  CPPReferenceType::VC_rvalue);
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

  case IIT_initializer:
    // In this case, we have parsed an instance declaration with a set
    // of initializers as a parameter list.  We lose the initializers
    // at this point, but the instance will put it back again.
    result = start_type;
    break;

  default:
    cerr << "Internal error--invalid CPPInstanceIdentifier\n";
    abort();
  }

  return CPPType::new_type(result);
}
