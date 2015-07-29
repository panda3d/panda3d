// Filename: cppExpression.cxx
// Created by:  drose (25Oct99)
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


#include "cppExpression.h"
#include "cppToken.h"
#include "cppIdentifier.h"
#include "cppType.h"
#include "cppSimpleType.h"
#include "cppPointerType.h"
#include "cppConstType.h"
#include "cppArrayType.h"
#include "cppPreprocessor.h"
#include "cppInstance.h"
#include "cppFunctionGroup.h"
#include "cppFunctionType.h"
#include "cppBison.h"
#include "pdtoa.h"

#include <assert.h>

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Result::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::Result::
Result() {
  _type = RT_error;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Result::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::Result::
Result(int value) {
  _type = RT_integer;
  _u._integer = value;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Result::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::Result::
Result(double value) {
  _type = RT_real;
  _u._real = value;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Result::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::Result::
Result(void *value) {
  _type = RT_pointer;
  _u._pointer = value;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Result::as_integer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int CPPExpression::Result::
as_integer() const {
  switch (_type) {
  case RT_integer:
    return _u._integer;

  case RT_real:
    return (int)_u._real;

  case RT_pointer:
    // We don't mind if this loses precision.
    return (int)reinterpret_cast<long>(_u._pointer);

  default:
    cerr << "Invalid type\n";
    assert(false);
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Result::as_real
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
double CPPExpression::Result::
as_real() const {
  switch (_type) {
  case RT_integer:
    return (double)_u._integer;

  case RT_real:
    return _u._real;

  case RT_pointer:
    // We don't mind if this loses precision.
    return (double)reinterpret_cast<long>(_u._pointer);

  default:
    cerr << "Invalid type\n";
    assert(false);
    return 0.0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Result::as_pointer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void *CPPExpression::Result::
as_pointer() const {
  switch (_type) {
  case RT_integer:
    return reinterpret_cast<void*>((long)_u._integer);

  case RT_real:
    return reinterpret_cast<void*>((long)_u._real);

  case RT_pointer:
    return _u._pointer;

  default:
    cerr << "Invalid type\n";
    assert(false);
    return (void *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Result::as_boolean
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPExpression::Result::
as_boolean() const {
  switch (_type) {
  case RT_integer:
    return (_u._integer != 0);

  case RT_real:
    return (_u._real != 0.0);

  case RT_pointer:
    return (_u._pointer != NULL);

  default:
    cerr << "Invalid type\n";
    assert(false);
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Result::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPExpression::Result::
output(ostream &out) const {
  switch (_type) {
  case RT_integer:
    out << _u._integer;
    break;

  case RT_real:
    out << _u._real;
    break;

  case RT_pointer:
    out << _u._pointer;
    break;

  case RT_error:
    out << "(error)";
    break;

  default:
    out << "(**invalid type**)\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::
CPPExpression(int value) :
  CPPDeclaration(CPPFile())
{
  _type = T_integer;
  _u._integer = value;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::
CPPExpression(double value) :
  CPPDeclaration(CPPFile())
{
  _type = T_real;
  _u._real = value;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::
CPPExpression(const string &value) :
  CPPDeclaration(CPPFile())
{
  _type = T_string;
  _str = value;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::
CPPExpression(CPPIdentifier *ident, CPPScope *current_scope,
              CPPScope *global_scope, CPPPreprocessor *error_sink) :
  CPPDeclaration(CPPFile())
{
  CPPDeclaration *decl =
    ident->find_symbol(current_scope, global_scope);

  if (decl != NULL) {
    CPPInstance *inst = decl->as_instance();
    if (inst != NULL) {
      _type = T_variable;
      _u._variable = inst;
      return;
    }
    CPPFunctionGroup *fgroup = decl->as_function_group();
    if (fgroup != NULL) {
      _type = T_function;
      _u._fgroup = fgroup;
      return;
    }
  }

  _type = T_unknown_ident;
  _u._ident = ident;
  _u._ident->_native_scope = current_scope;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::
CPPExpression(int unary_operator, CPPExpression *op1) :
  CPPDeclaration(CPPFile())
{
  _type = T_unary_operation;
  _u._op._operator = unary_operator;
  _u._op._op1 = op1;
  _u._op._op2 = NULL;
  _u._op._op3 = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::
CPPExpression(int binary_operator, CPPExpression *op1, CPPExpression *op2) :
  CPPDeclaration(CPPFile())
{
  _type = T_binary_operation;
  _u._op._operator = binary_operator;
  _u._op._op1 = op1;
  _u._op._op2 = op2;
  _u._op._op3 = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::
CPPExpression(int trinary_operator, CPPExpression *op1, CPPExpression *op2,
              CPPExpression *op3) :
  CPPDeclaration(CPPFile())
{
  _type = T_trinary_operation;
  _u._op._operator = trinary_operator;
  _u._op._op1 = op1;
  _u._op._op2 = op2;
  _u._op._op3 = op3;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::named typecast_op constructor
//       Access: Public, Static
//  Description: Creates an expression that represents a typecast
//               operation.
////////////////////////////////////////////////////////////////////
CPPExpression CPPExpression::
typecast_op(CPPType *type, CPPExpression *op1) {
  CPPExpression expr(0);
  expr._type = T_typecast;
  expr._u._typecast._to = type;
  expr._u._typecast._op1 = op1;
  return expr;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::named construct_op constructor
//       Access: Public, Static
//  Description: Creates an expression that represents a constructor
//               call.
////////////////////////////////////////////////////////////////////
CPPExpression CPPExpression::
construct_op(CPPType *type, CPPExpression *op1) {
  CPPExpression expr(0);
  if (op1 == NULL) {
    // A default constructor call--no parameters.
    expr._type = T_default_construct;
    expr._u._typecast._to = type;
    expr._u._typecast._op1 = NULL;
  } else {
    // A normal constructor call, with parameters.
    expr._type = T_construct;
    expr._u._typecast._to = type;
    expr._u._typecast._op1 = op1;
  }
  return expr;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::named new_op constructor
//       Access: Public, Static
//  Description: Creates an expression that represents a use of the
//               new operator.
////////////////////////////////////////////////////////////////////
CPPExpression CPPExpression::
new_op(CPPType *type, CPPExpression *op1) {
  CPPExpression expr(0);
  if (op1 == NULL) {
    // A default new operation--no parameters.
    expr._type = T_default_new;
    expr._u._typecast._to = type;
    expr._u._typecast._op1 = NULL;
  } else {
    // A normal new operation, with parameters.
    expr._type = T_new;
    expr._u._typecast._to = type;
    expr._u._typecast._op1 = op1;
  }
  return expr;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::named sizeof_func constructor
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression CPPExpression::
sizeof_func(CPPType *type) {
  CPPExpression expr(0);
  expr._type = T_sizeof;
  expr._u._typecast._to = type;
  expr._u._typecast._op1 = NULL;
  return expr;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::
~CPPExpression() {
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::evaluate
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression::Result CPPExpression::
evaluate() const {
  Result r1, r2;

  switch (_type) {
  case T_integer:
    return Result(_u._integer);

  case T_real:
    return Result(_u._real);

  case T_string:
    return Result();

  case T_variable:
    if (_u._variable->_type != NULL &&
        _u._variable->_initializer != NULL) {
      // A const variable.  Fetch its assigned value.
      CPPConstType *const_type = _u._variable->_type->as_const_type();
      if (const_type != NULL) {
        return _u._variable->_initializer->evaluate();
      }
    }
    return Result();

  case T_function:
    return Result();

  case T_unknown_ident:
    return Result();

  case T_typecast:
    assert(_u._typecast._op1 != NULL);
    r1 = _u._typecast._op1->evaluate();
    if (r1._type != RT_error) {
      CPPSimpleType *stype = _u._typecast._to->as_simple_type();
      if (stype != NULL) {
        if (stype->_type == CPPSimpleType::T_int) {
          return Result(r1.as_integer());
        } else if (stype->_type == CPPSimpleType::T_float ||
                   stype->_type == CPPSimpleType::T_double) {
          return Result(r1.as_real());
        }
      }
      if (_u._typecast._to->as_pointer_type()) {
        return Result(r1.as_pointer());
      }
    }
    return Result();

  case T_construct:
  case T_default_construct:
  case T_new:
  case T_default_new:
  case T_sizeof:
    return Result();

  case T_binary_operation:
    assert(_u._op._op2 != NULL);
    r2 = _u._op._op2->evaluate();

    // The operators && and || are special cases: these are
    // shirt-circuiting operators.  Thus, if we are using either of
    // these it might be acceptable for the second operand to be
    // invalid, since we might never evaluate it.

    // In all other cases, both operands must be valid in order for
    // the operation to be valid.
    if (r2._type == RT_error &&
        (_u._op._operator != OROR && _u._op._operator != ANDAND)) {
      return r2;
    }
    // Fall through


  case T_trinary_operation:
    // The trinary operator is also a short-circuiting operator: we
    // don't test the second or third operands until we need them.
    // The only critical one is the first operand.

    // Fall through

  case T_unary_operation:
    assert(_u._op._op1 != NULL);
    r1 = _u._op._op1->evaluate();
    if (r1._type == RT_error) {
      // Here's one more special case: if the first operand is
      // invalid, it really means we don't know how to evaluate it.
      // However, if the operator is ||, then it might not matter as
      // long as we can evaluate the second one *and* that comes out
      // to be true.
      if (_u._op._operator == OROR && r2._type == RT_integer &&
          r2.as_integer() != 0) {
        return r2;
      }

      // Ditto for the operator being && and the second one coming out
      // false.
      if (_u._op._operator == ANDAND && r2._type == RT_integer &&
          r2.as_integer() == 0) {
        return r2;
      }

      return r1;
    }

    switch (_u._op._operator) {
    case UNARY_NOT:
      return Result(!r1.as_integer());

    case UNARY_NEGATE:
      return Result(~r1.as_integer());

    case UNARY_MINUS:
      return (r1._type == RT_real) ? Result(-r1.as_real()) : Result(-r1.as_integer());

    case UNARY_STAR:
    case UNARY_REF:
      return Result();

    case '*':
      if (r1._type == RT_real || r2._type == RT_real) {
        return Result(r1.as_real() * r2.as_real());
      } else {
        return Result(r1.as_integer() * r2.as_integer());
      }

    case '/':
      if (r1._type == RT_real || r2._type == RT_real) {
        return Result(r1.as_real() / r2.as_real());
      } else {
        return Result(r1.as_integer() / r2.as_integer());
      }

    case '%':
      return Result(r1.as_integer() % r2.as_integer());

    case '+':
      if (r1._type == RT_real || r2._type == RT_real) {
        return Result(r1.as_real() + r2.as_real());
      } else {
        return Result(r1.as_integer() + r2.as_integer());
      }

    case '-':
      if (r1._type == RT_real || r2._type == RT_real) {
        return Result(r1.as_real() - r2.as_real());
      } else {
        return Result(r1.as_integer() - r2.as_integer());
      }

    case '|':
      return Result(r1.as_integer() | r2.as_integer());

    case '&':
      return Result(r1.as_integer() & r2.as_integer());

    case OROR:
      if (r1.as_integer()) {
        return r1;
      } else {
        return r2;
      }

    case ANDAND:
      if (r1.as_integer()) {
        return r2;
      } else {
        return r1;
      }

    case EQCOMPARE:
      if (r1._type == RT_real || r2._type == RT_real) {
        return Result(r1.as_real() == r2.as_real());
      } else {
        return Result(r1.as_integer() == r2.as_integer());
      }

    case NECOMPARE:
      if (r1._type == RT_real || r2._type == RT_real) {
        return Result(r1.as_real() != r2.as_real());
      } else {
        return Result(r1.as_integer() != r2.as_integer());
      }

    case LECOMPARE:
      if (r1._type == RT_real || r2._type == RT_real) {
        return Result(r1.as_real() <= r2.as_real());
      } else {
        return Result(r1.as_integer() <= r2.as_integer());
      }

    case GECOMPARE:
      if (r1._type == RT_real || r2._type == RT_real) {
        return Result(r1.as_real() >= r2.as_real());
      } else {
        return Result(r1.as_integer() >= r2.as_integer());
      }

    case '<':
      if (r1._type == RT_real || r2._type == RT_real) {
        return Result(r1.as_real() < r2.as_real());
      } else {
        return Result(r1.as_integer() < r2.as_integer());
      }

    case '>':
      if (r1._type == RT_real || r2._type == RT_real) {
        return Result(r1.as_real() > r2.as_real());
      } else {
        return Result(r1.as_integer() > r2.as_integer());
      }

    case LSHIFT:
      return Result(r1.as_integer() << r2.as_integer());

    case RSHIFT:
      return Result(r1.as_integer() >> r2.as_integer());

    case '?':
      return r1.as_integer() ?
        _u._op._op2->evaluate() : _u._op._op3->evaluate();

    case '.':
    case POINTSAT:
      return Result();

    case '[': // Array element reference
      return Result();

    case 'f': // Function evaluation
      return Result();

    case ',':
      return r2;

    default:
      cerr << "**unexpected operator**\n";
      abort();
    }

  default:
    cerr << "**invalid operand**\n";
    abort();
  }

  return Result();  // Compiler kludge; can't get here.
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::determine_type
//       Access: Public
//  Description: Returns the type of the expression, if it is known,
//               or NULL if the type cannot be determined.
////////////////////////////////////////////////////////////////////
CPPType *CPPExpression::
determine_type() const {
  CPPType *t1 = (CPPType *)NULL;
  CPPType *t2 = (CPPType *)NULL;

  CPPType *int_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_int));

  CPPType *bool_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_bool));

  CPPType *float_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_double));

  CPPType *char_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_char));

  CPPType *const_char_type =
    CPPType::new_type(new CPPConstType(char_type));

  CPPType *char_star_type =
    CPPType::new_type(new CPPPointerType(const_char_type));

  switch (_type) {
  case T_integer:
    return int_type;

  case T_real:
    return float_type;

  case T_string:
    return char_star_type;

  case T_variable:
    return _u._variable->_type;

  case T_function:
    if (_u._fgroup->get_return_type() == (CPPType *)NULL) {
      // There are multiple functions by this name that have different
      // return types.  We could attempt to differentiate them based
      // on the parameter list, but that's a lot of work.  Let's just
      // give up.
      return (CPPType *)NULL;
    }
    return _u._fgroup->_instances.front()->_type;

  case T_unknown_ident:
    return (CPPType *)NULL;

  case T_typecast:
  case T_construct:
  case T_default_construct:
    return _u._typecast._to;

  case T_new:
  case T_default_new:
    return CPPType::new_type(new CPPPointerType(_u._typecast._to));

  case T_sizeof:
    return int_type;

  case T_binary_operation:
  case T_trinary_operation:
    assert(_u._op._op2 != NULL);
    t2 = _u._op._op2->determine_type();
    // Fall through

  case T_unary_operation:
    assert(_u._op._op1 != NULL);
    t1 = _u._op._op1->determine_type();

    switch (_u._op._operator) {
    case UNARY_NOT:
      return bool_type;

    case UNARY_NEGATE:
      return int_type;

    case UNARY_MINUS:
      return t1;

    case UNARY_STAR:
    case '[': // Array element reference
      if (t1 != NULL) {
        if (t1->as_pointer_type()) {
          return t1->as_pointer_type()->_pointing_at;
        }
        if (t1->as_array_type()) {
          return t1->as_array_type()->_element_type;
        }
      }
      return NULL;

    case UNARY_REF:
      return t1;

    case '*':
    case '/':
    case '+':
    case '-':
      if (t1 == NULL) {
        return t2;
      } else if (t2 == NULL) {
        return t1;
      } else if (t1->as_pointer_type()) {
        if (t2->as_pointer_type()) {
          return int_type;
        }
        return t1;
      }
      return elevate_type(t1, t2);

    case '%':
    case '|':
    case '&':
    case LSHIFT:
    case RSHIFT:
      return int_type;

    case OROR:
    case ANDAND:
    case EQCOMPARE:
    case NECOMPARE:
    case LECOMPARE:
    case GECOMPARE:
    case '<':
    case '>':
      return bool_type;

    case '?':
      return t2;

    case '.':
    case POINTSAT:
      return NULL;

    case 'f': // Function evaluation
      if (t1 != NULL) {
        CPPFunctionType *ftype = t1->as_function_type();
        if (ftype != (CPPFunctionType *)NULL) {
          return ftype->_return_type;
        }
      }
      return NULL;

    case ',':
      return t2;

    default:
      cerr << "**unexpected operator**\n";
      abort();
    }

  default:
    cerr << "**invalid operand**\n";
    abort();
  }

  return NULL;  // Compiler kludge; can't get here.
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPExpression::
is_fully_specified() const {
  if (!CPPDeclaration::is_fully_specified()) {
    return false;
  }

  switch (_type) {
  case T_integer:
  case T_real:
  case T_string:
    return false;

  case T_variable:
    return _u._variable->is_fully_specified();

  case T_function:
    return _u._fgroup->is_fully_specified();

  case T_unknown_ident:
    return _u._ident->is_fully_specified();

  case T_typecast:
  case T_construct:
  case T_new:
    return (_u._typecast._to->is_fully_specified() &&
            _u._typecast._op1->is_fully_specified());

  case T_default_construct:
  case T_default_new:
  case T_sizeof:
    return _u._typecast._to->is_fully_specified();

  case T_trinary_operation:
    if (!_u._op._op3->is_fully_specified()) {
      return false;
    }
    // Fall through

  case T_binary_operation:
    if (!_u._op._op2->is_fully_specified()) {
      return false;
    }
    // Fall through

  case T_unary_operation:
    return _u._op._op1->is_fully_specified();

  default:
    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::substitute_decl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPExpression::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  CPPDeclaration *top =
    CPPDeclaration::substitute_decl(subst, current_scope, global_scope);
  if (top != this) {
    return top;
  }

  CPPExpression *rep = new CPPExpression(*this);
  bool any_changed = false;
  CPPDeclaration *decl;

  switch (_type) {
  case T_variable:
    decl = _u._variable->substitute_decl(subst, current_scope, global_scope);
    if (decl != rep->_u._variable) {
      if (decl->as_instance()) {
        // Replacing the variable reference with another variable reference.
        rep->_u._variable = decl->as_instance();
        any_changed = true;

      } else if (decl->as_expression()) {
        // Replacing the variable reference with an expression.
        delete rep;
        rep = decl->as_expression();
        any_changed = true;
      }
    }
    break;

  case T_unknown_ident:
    rep->_u._ident = _u._ident->substitute_decl(subst, current_scope, global_scope);
    any_changed = any_changed || (rep->_u._ident != _u._ident);

    // See if we can define it now.
    decl = rep->_u._ident->find_symbol(current_scope, global_scope, subst);
    if (decl != NULL) {
      CPPInstance *inst = decl->as_instance();
      if (inst != NULL) {
        rep->_type = T_variable;
        rep->_u._variable = inst;
        any_changed = true;

        decl = inst->substitute_decl(subst, current_scope, global_scope);
        if (decl != inst) {
          if (decl->as_instance()) {
            // Replacing the variable reference with another variable reference.
            rep->_u._variable = decl->as_instance();

          } else if (decl->as_expression()) {
            // Replacing the variable reference with an expression.
            delete rep;
            rep = decl->as_expression();
          }
        }
        break;
      }
      CPPFunctionGroup *fgroup = decl->as_function_group();
      if (fgroup != NULL) {
        rep->_type = T_function;
        rep->_u._fgroup = fgroup;
        any_changed = true;
      }
    }

    break;

  case T_typecast:
  case T_construct:
  case T_new:
    rep->_u._typecast._op1 =
      _u._typecast._op1->substitute_decl(subst, current_scope, global_scope)
      ->as_expression();
    any_changed = any_changed || (rep->_u._typecast._op1 != _u._typecast._op1);
    // fall through

  case T_default_construct:
  case T_default_new:
  case T_sizeof:
    rep->_u._typecast._to =
      _u._typecast._to->substitute_decl(subst, current_scope, global_scope)
      ->as_type();
    any_changed = any_changed || (rep->_u._typecast._to != _u._typecast._to);
    break;

  case T_trinary_operation:
    rep->_u._op._op3 =
      _u._op._op3->substitute_decl(subst, current_scope, global_scope)
      ->as_expression();
    any_changed = any_changed || (rep->_u._op._op3 != _u._op._op3);
    // fall through

  case T_binary_operation:
    rep->_u._op._op2 =
      _u._op._op2->substitute_decl(subst, current_scope, global_scope)
      ->as_expression();
    any_changed = any_changed || (rep->_u._op._op2 != _u._op._op2);
    // fall through

  case T_unary_operation:
    rep->_u._op._op1 =
      _u._op._op1->substitute_decl(subst, current_scope, global_scope)
      ->as_expression();
    any_changed = any_changed || (rep->_u._op._op1 != _u._op._op1);
    break;

  default:
    break;
  }

  if (!any_changed) {
    delete rep;
    rep = this;
  }
  subst.insert(SubstDecl::value_type(this, rep));
  return rep;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::is_tbd
//       Access: Public
//  Description: Returns true if any type within the expression list is
//               a CPPTBDType and thus isn't fully determined right
//               now.
////////////////////////////////////////////////////////////////////
bool CPPExpression::
is_tbd() const {
  switch (_type) {
  case T_variable:
    if (_u._variable->_type != NULL &&
        _u._variable->_initializer != NULL) {
      CPPConstType *const_type = _u._variable->_type->as_const_type();
      if (const_type != NULL) {
        return false;
      }
    }

    return true;

  case T_unknown_ident:
    return true;

  case T_typecast:
  case T_construct:
  case T_new:
  case T_default_construct:
  case T_default_new:
  case T_sizeof:
    return _u._typecast._to->is_tbd();

  case T_trinary_operation:
    if (_u._op._op3->is_tbd()) {
      return true;
    }
    // fall through

  case T_binary_operation:
    if (_u._op._op2->is_tbd()) {
      return true;
    }
    // fall through

  case T_unary_operation:
    if (_u._op._op1->is_tbd()) {
      return true;
    }
    return false;

  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPExpression::
output(ostream &out, int indent_level, CPPScope *scope, bool) const {
  switch (_type) {
  case T_integer:
    out << _u._integer;
    break;

  case T_real:
    {
      // We use our own dtoa implementation here because it guarantees
      // to never format the number as an integer.
      char buffer[32];
      pdtoa(_u._real, buffer);
      out << buffer;
    }
    break;

  case T_string:
    out << '"';
    {
      string::const_iterator si;
      for (si = _str.begin(); si != _str.end(); ++si) {
        switch (*si) {
        case '\n':
          out << "\\n";
          break;

        case '\t':
          out << "\\t";
          break;

        case '\r':
          out << "\\r";
          break;

        case '\a':
          out << "\\a";
          break;

        case '"':
          out << "\\\"";
          break;

        default:
          if (isprint(*si)) {
            out << *si;
          } else {
            out << '\\' << oct << setw(3) << setfill('0') << (int)(*si)
                << dec << setw(0);
          }
        }
      }
    }
    out << '"';
    break;

  case T_variable:
    // We can just refer to the variable by name, except if it's a
    // private constant, in which case we have to compute the value,
    // since we may have to use it in generated code.
    if (_u._variable->_type != NULL &&
        _u._variable->_initializer != NULL &&
        _u._variable->_vis > V_public) {
      // A const variable.  Fetch its assigned value.
      CPPConstType *const_type = _u._variable->_type->as_const_type();
      if (const_type != NULL) {
        _u._variable->_initializer->output(out, indent_level, scope, false);
        break;
      }
    }
    _u._variable->_ident->output(out, scope);
    break;

  case T_function:
    out << _u._fgroup->_name;
    break;

  case T_unknown_ident:
    _u._ident->output(out, scope);
    break;

  case T_typecast:
    out << "(";
    _u._typecast._to->output(out, indent_level, scope, false);
    out << ")(";
    _u._typecast._op1->output(out, indent_level, scope, false);
    out << ")";
    break;

  case T_construct:
    _u._typecast._to->output(out, indent_level, scope, false);
    out << "(";
    _u._typecast._op1->output(out, indent_level, scope, false);
    out << ")";
    break;

  case T_default_construct:
    _u._typecast._to->output(out, indent_level, scope, false);
    out << "()";
    break;

  case T_new:
    out << "(new ";
    _u._typecast._to->output(out, indent_level, scope, false);
    out << "(";
    _u._typecast._op1->output(out, indent_level, scope, false);
    out << "))";
    break;

  case T_default_new:
    out << "(new ";
    _u._typecast._to->output(out, indent_level, scope, false);
    out << "())";
    break;

  case T_sizeof:
    out << "sizeof(";
    _u._typecast._to->output(out, indent_level, scope, false);
    out << ")";
    break;

  case T_unary_operation:
    switch (_u._op._operator) {
    case UNARY_NOT:
      out << "(! ";
      _u._op._op1->output(out, indent_level, scope, false);
      out << ")";
      break;

    case UNARY_NEGATE:
      out << "(~ ";
      _u._op._op1->output(out, indent_level, scope, false);
      out << ")";
      break;

    case UNARY_MINUS:
      out << "(- ";
      _u._op._op1->output(out, indent_level, scope, false);
      out << ")";
      break;

    case UNARY_STAR:
      out << "(* ";
      _u._op._op1->output(out, indent_level, scope, false);
      out << ")";
      break;

    case UNARY_REF:
      out << "(& ";
      _u._op._op1->output(out, indent_level, scope, false);
      out << ")";
      break;

    case 'f': // Function evaluation, no parameters.
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << "())";
      break;

    default:
      out << "(" << (char)_u._op._operator << " ";
      _u._op._op1->output(out, indent_level, scope, false);
      out << ")";
      break;
    }
    break;

  case T_binary_operation:
    switch (_u._op._operator) {
    case OROR:
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << " || ";
      _u._op._op2->output(out, indent_level, scope, false);
      out << ")";
      break;

    case ANDAND:
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << " && ";
      _u._op._op2->output(out, indent_level, scope, false);
      out << ")";
      break;

    case EQCOMPARE:
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << " == ";
      _u._op._op2->output(out, indent_level, scope, false);
      out << ")";
      break;

    case NECOMPARE:
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << " != ";
      _u._op._op2->output(out, indent_level, scope, false);
      out << ")";
      break;

    case LECOMPARE:
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << " <= ";
      _u._op._op2->output(out, indent_level, scope, false);
      out << ")";
      break;

    case GECOMPARE:
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << " >= ";
      _u._op._op2->output(out, indent_level, scope, false);
      out << ")";
      break;

    case LSHIFT:
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << " << ";
      _u._op._op2->output(out, indent_level, scope, false);
      out << ")";
      break;

    case RSHIFT:
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << " >> ";
      _u._op._op2->output(out, indent_level, scope, false);
      out << ")";
      break;

    case '.':
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << ".";
      _u._op._op2->output(out, indent_level, scope, false);
      out << ")";
      break;

    case POINTSAT:
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << "->";
      _u._op._op2->output(out, indent_level, scope, false);
      out << ")";
      break;

    case '[': // Array element reference
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << "[";
      _u._op._op2->output(out, indent_level, scope, false);
      out << "])";
      break;

    case 'f': // Function evaluation
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << "(";
      _u._op._op2->output(out, indent_level, scope, false);
      out << "))";
      break;

    case ',': // Comma, no parens are used
      _u._op._op1->output(out, indent_level, scope, false);
      out << ", ";
      _u._op._op2->output(out, indent_level, scope, false);
      break;

    default:
      out << "(";
      _u._op._op1->output(out, indent_level, scope, false);
      out << " " << (char)_u._op._operator << " ";
      _u._op._op2->output(out, indent_level, scope, false);
      out << ")";
    }
    break;

  case T_trinary_operation:
    out << "(";
    _u._op._op1->output(out, indent_level, scope, false);
    out << " ? ";
    _u._op._op2->output(out, indent_level, scope, false);
    out << " : ";
    _u._op._op3->output(out, indent_level, scope, false);
    out << ")";
    break;

  default:
    out << "(** invalid operand type " << (int)_type << " **)";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPExpression::
get_subtype() const {
  return ST_expression;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::as_expression
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression *CPPExpression::
as_expression() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::elevate_type
//       Access: Public, Static
//  Description: Returns the most general of the two given types.
////////////////////////////////////////////////////////////////////
CPPType *CPPExpression::
elevate_type(CPPType *t1, CPPType *t2) {
  CPPSimpleType *st1 = t1->as_simple_type();
  CPPSimpleType *st2 = t2->as_simple_type();

  if (st1 == NULL || st2 == NULL) {
    // Nothing we can do about this.  Who knows?
    return NULL;
  }

  if (st1->_type == st2->_type) {
    // They have the same type, so return the one with the largest
    // flag bits.
    if (st1->_flags & CPPSimpleType::F_longlong) {
      return st1;
    } else if (st2->_flags & CPPSimpleType::F_longlong) {
      return st2;
    } else if (st1->_flags & CPPSimpleType::F_long) {
      return st1;
    } else if (st2->_flags & CPPSimpleType::F_long) {
      return st2;
    } else if (st1->_flags & CPPSimpleType::F_short) {
      return st2;
    } else if (st2->_flags & CPPSimpleType::F_short) {
      return st1;
    }
    return st1;
  }

  // They have different types.
  if (st1->_type == CPPSimpleType::T_float ||
      st1->_type == CPPSimpleType::T_double) {
    return st1;
  } else if (st2->_type == CPPSimpleType::T_float ||
             st2->_type == CPPSimpleType::T_double) {
    return st2;
  } else if (st1->_type == CPPSimpleType::T_int) {
    return st1;
  } else if (st2->_type == CPPSimpleType::T_int) {
    return st2;
  } else if (st1->_type == CPPSimpleType::T_bool) {
    return st1;
  } else if (st2->_type == CPPSimpleType::T_bool) {
    return st2;
  }
  return st1;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::is_equal
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration to determine whether this
//               expr is equivalent to another expr.
////////////////////////////////////////////////////////////////////
bool CPPExpression::
is_equal(const CPPDeclaration *other) const {
  const CPPExpression *ot = ((CPPDeclaration *)other)->as_expression();
  assert(ot != NULL);

  if (_type != ot->_type) {
    return false;
  }

  switch (_type) {
  case T_integer:
    return _u._integer == ot->_u._integer;

  case T_real:
    return _u._real == ot->_u._real;

  case T_string:
    return _str == ot->_str;

  case T_variable:
    return _u._variable == ot->_u._variable;

  case T_function:
    return _u._fgroup == ot->_u._fgroup;

  case T_unknown_ident:
    return *_u._ident == *ot->_u._ident;

  case T_typecast:
  case T_construct:
  case T_new:
    return _u._typecast._to == ot->_u._typecast._to &&
      *_u._typecast._op1 == *ot->_u._typecast._op1;

  case T_default_construct:
  case T_default_new:
  case T_sizeof:
    return _u._typecast._to == ot->_u._typecast._to;

  case T_unary_operation:
    return *_u._op._op1 == *ot->_u._op._op1;

  case T_binary_operation:
    return *_u._op._op1 == *ot->_u._op._op1 &&
      *_u._op._op2 == *ot->_u._op._op2;

  case T_trinary_operation:
    return *_u._op._op1 == *ot->_u._op._op1 &&
      *_u._op._op2 == *ot->_u._op._op2;

  default:
    cerr << "(** invalid operand type " << (int)_type << " **)";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpression::is_less
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration to determine whether this
//               expr should be ordered before another expr of the
//               same type, in an arbitrary but fixed ordering.
////////////////////////////////////////////////////////////////////
bool CPPExpression::
is_less(const CPPDeclaration *other) const {
  const CPPExpression *ot = ((CPPDeclaration *)other)->as_expression();
  assert(ot != NULL);

  if (_type != ot->_type) {
    return (int)_type < (int)ot->_type;
  }

  switch (_type) {
  case T_integer:
    return _u._integer < ot->_u._integer;

  case T_real:
    return _u._real < ot->_u._real;

  case T_string:
    return _str < ot->_str;

  case T_variable:
    return _u._variable < ot->_u._variable;

  case T_function:
    return *_u._fgroup < *ot->_u._fgroup;

  case T_unknown_ident:
    return *_u._ident < *ot->_u._ident;

  case T_typecast:
  case T_construct:
  case T_new:
    if (_u._typecast._to != ot->_u._typecast._to) {
      return _u._typecast._to < ot->_u._typecast._to;
    }
    return *_u._typecast._op1 < *ot->_u._typecast._op1;

  case T_default_construct:
  case T_default_new:
  case T_sizeof:
    return _u._typecast._to < ot->_u._typecast._to;

  case T_trinary_operation:
    if (*_u._op._op3 != *ot->_u._op._op3) {
      return *_u._op._op3 < *ot->_u._op._op3;
    }
    // Fall through

  case T_binary_operation:
    if (*_u._op._op2 != *ot->_u._op._op2) {
      return *_u._op._op2 < *ot->_u._op._op2;
    }
    // Fall through

  case T_unary_operation:
    return *_u._op._op1 < *ot->_u._op._op1;

  default:
    cerr << "(** invalid operand type " << (int)_type << " **)";
  }

  return false;
}
