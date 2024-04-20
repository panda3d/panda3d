/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppInstanceIdentifier.cxx
 * @author drose
 * @date 1999-10-21
 */

#include "cppInstanceIdentifier.h"
#include "cppPointerType.h"
#include "cppReferenceType.h"
#include "cppArrayType.h"
#include "cppConstType.h"
#include "cppFunctionType.h"
#include "cppSimpleType.h"
#include "cppParameterList.h"
#include "cppIdentifier.h"

/**
 *
 */
CPPInstanceIdentifier::Modifier::
Modifier(CPPInstanceIdentifierType type, CPPAttributeList attr) :
  _type(type),
  _func_params(nullptr),
  _func_flags(0),
  _scoping(nullptr),
  _expr(nullptr),
  _attributes(std::move(attr)) {
}

/**
 *
 */
CPPInstanceIdentifier::Modifier CPPInstanceIdentifier::Modifier::
func_type(CPPParameterList *params, int flags, CPPType *trailing_return_type,
          CPPAttributeList attr) {
  Modifier mod(IIT_func, std::move(attr));
  mod._func_params = params;
  mod._func_flags = flags;
  mod._trailing_return_type = trailing_return_type;
  return mod;
}

/**
 *
 */
CPPInstanceIdentifier::Modifier CPPInstanceIdentifier::Modifier::
array_type(CPPExpression *expr, CPPAttributeList attr) {
  Modifier mod(IIT_array, std::move(attr));
  mod._expr = expr;
  return mod;
}

/**
 *
 */
CPPInstanceIdentifier::Modifier CPPInstanceIdentifier::Modifier::
scoped_pointer_type(CPPIdentifier *scoping, CPPAttributeList attr) {
  Modifier mod(IIT_scoped_pointer, std::move(attr));
  mod._scoping = scoping;
  return mod;
}

/**
 * This is used only for instance declarations that turn out to be have a
 * parameter list for an initializer.
 */
CPPInstanceIdentifier::Modifier CPPInstanceIdentifier::Modifier::
initializer_type(CPPParameterList *params) {
  Modifier mod(IIT_initializer);
  mod._func_params = params;
  return mod;
}

/**
 *
 */
CPPInstanceIdentifier::
CPPInstanceIdentifier(CPPIdentifier *ident) :
  _ident(ident),
  _bit_width(nullptr),
  _packed(false) {
}

/**
 *
 */
CPPInstanceIdentifier::
CPPInstanceIdentifier(CPPIdentifier *ident, CPPAttributeList attributes) :
  _ident(ident),
  _attributes(std::move(attributes)),
  _bit_width(nullptr),
  _packed(false) {
}

/**
 * Unrolls the list of type punctuation on either side of the identifier to
 * determine the actual type represented by the identifier, given the
 * indicated starting type (that is, the type name written to the left of the
 * identifier).
 */
CPPType *CPPInstanceIdentifier::
unroll_type(CPPType *start_type) {
  CPPType *result = r_unroll_type(start_type, _modifiers.begin());
  return result;
}


/**
 *
 */
void CPPInstanceIdentifier::
add_modifier(CPPInstanceIdentifierType type, CPPAttributeList attr) {
  _modifiers.push_back(Modifier(type, std::move(attr)));
}

/**
 *
 */
void CPPInstanceIdentifier::
add_func_modifier(CPPParameterList *params, int flags,
                  CPPType *trailing_return_type, CPPAttributeList attr) {
  // As a special hack, if we added a parameter list to an operator function,
  // check if the parameter list is empty.  If it is, this is really a unary
  // operator, so set the unary_op flag.  Operators () and [] are never
  // considered unary operators.
  if (_ident != nullptr &&
      _ident->get_simple_name().substr(0, 9) == "operator ") {

    if (_ident->get_simple_name() != std::string("operator ()") &&
        _ident->get_simple_name() != std::string("operator []")) {
      if (params->_parameters.empty()) {
        flags |= CPPFunctionType::F_unary_op;
      }
    }

    flags |= CPPFunctionType::F_operator;
  }

  if (trailing_return_type != nullptr) {
    // Remember whether trailing return type notation was used.
    flags |= CPPFunctionType::F_trailing_return_type;
  }

  _modifiers.push_back(Modifier::func_type(params, flags, trailing_return_type, std::move(attr)));
}

/**
 *
 */
void CPPInstanceIdentifier::
add_scoped_pointer_modifier(CPPIdentifier *scoping, CPPAttributeList attr) {
  _modifiers.push_back(Modifier::scoped_pointer_type(scoping, std::move(attr)));
}

/**
 *
 */
void CPPInstanceIdentifier::
add_array_modifier(CPPExpression *expr, CPPAttributeList attr) {
  // Special case for operator new[] and delete[].  We're not really adding an
  // array modifier to them, but appending [] to the identifier.  This is to
  // work around a parser ambiguity.
  if (_ident != nullptr && (_ident->get_simple_name() == "operator delete" ||
                         _ident->get_simple_name() == "operator new")) {

    _ident->_names.back().append_name("[]");
  } else {
    _modifiers.push_back(Modifier::array_type(expr, std::move(attr)));
  }
}

/**
 *
 */
void CPPInstanceIdentifier::
add_initializer_modifier(CPPParameterList *params) {
  _modifiers.push_back(Modifier::initializer_type(params));
}

/**
 *
 */
void CPPInstanceIdentifier::
add_trailing_return_type(CPPType *type) {
  // This is an awkward hack.  Improve in the future.
  if (!_modifiers.empty()) {
    Modifier &mod = _modifiers.back();
    if (mod._type == IIT_func) {
      mod._trailing_return_type = type;
      mod._func_flags |= CPPFunctionType::F_trailing_return_type;
      return;
    }
  }
  std::cerr << "trailing return type can only be added to a function\n";
}

/**
 * Add attributes to the instance (not the type).
 */
void CPPInstanceIdentifier::
add_attributes(const CPPAttributeList &attributes) {
  _attributes.add_attributes_from(attributes);
}

/**
 * Returns the initializer parameter list that was set for this particular
 * instance, e.g.  if the instance were:
 *
 * int foo(0);
 *
 * this would return the parameter list (0).  Returns NULL if the instance did
 * not use a parameter list initializer.
 */
CPPParameterList *CPPInstanceIdentifier::
get_initializer() const {
  Modifiers::const_iterator mi;
  for (mi = _modifiers.begin(); mi != _modifiers.end(); ++mi) {
    const Modifier &mod = (*mi);
    if (mod._type == IIT_initializer) {
      return mod._func_params;
    }
  }

  return nullptr;
}

/**
 *
 */
CPPScope *CPPInstanceIdentifier::
get_scope(CPPScope *current_scope, CPPScope *global_scope,
          CPPPreprocessor *error_sink) const {
  if (_ident == nullptr) {
    return current_scope;
  } else {
    return _ident->get_scope(current_scope, global_scope, error_sink);
  }
}

/**
 * The recursive implementation of unroll_type().
 */
CPPType *CPPInstanceIdentifier::
r_unroll_type(CPPType *start_type,
              CPPInstanceIdentifier::Modifiers::const_iterator mi) {
  assert(start_type != nullptr);

  start_type = CPPType::new_type(start_type);

  if (mi == _modifiers.end()) {
    return start_type;
  }

  const Modifier &mod = (*mi);
  ++mi;

  CPPType *result = nullptr;

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
      if (ftype != nullptr) {
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

  case IIT_volatile:
  case IIT_restrict:
    // Just pass it through for now.
    result = r_unroll_type(start_type, mi);
    break;

  case IIT_paren:
    result = r_unroll_type(start_type, mi);
    break;

  case IIT_func:
    {
      CPPType *return_type = r_unroll_type(start_type, mi);
      if (mod._trailing_return_type != nullptr) {
        CPPSimpleType *simple_type = return_type->as_simple_type();
        if (simple_type != nullptr && simple_type->_type == CPPSimpleType::T_auto) {
          return_type = mod._trailing_return_type;
        } else {
          std::cerr << "function with trailing return type needs auto\n";
        }
      }
      result = new CPPFunctionType(return_type, mod._func_params,
                                   mod._func_flags);
    }
    break;

  case IIT_initializer:
    // In this case, we have parsed an instance declaration with a set of
    // initializers as a parameter list.  We lose the initializers at this
    // point, but the instance will put it back again.
    result = start_type;
    break;

  default:
    std::cerr << "Internal error--invalid CPPInstanceIdentifier\n";
    abort();
  }

  result->_attributes = mod._attributes;

  return CPPType::new_type(result);
}
