/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppExpression.cxx
 * @author drose
 * @date 1999-10-25
 */

#include "cppExpression.h"
#include "cppToken.h"
#include "cppIdentifier.h"
#include "cppType.h"
#include "cppSimpleType.h"
#include "cppPointerType.h"
#include "cppEnumType.h"
#include "cppConstType.h"
#include "cppArrayType.h"
#include "cppPreprocessor.h"
#include "cppInstance.h"
#include "cppFunctionGroup.h"
#include "cppFunctionType.h"
#include "cppClosureType.h"
#include "cppReferenceType.h"
#include "cppStructType.h"
#include "cppBison.h"
#include "pdtoa.h"

#include <assert.h>

using std::cerr;
using std::string;

/**
 *
 */
CPPExpression::Result::
Result() {
  _type = RT_error;
}

/**
 *
 */
CPPExpression::Result::
Result(int value) {
  _type = RT_integer;
  _u._integer = value;
}

/**
 *
 */
CPPExpression::Result::
Result(double value) {
  _type = RT_real;
  _u._real = value;
}

/**
 *
 */
CPPExpression::Result::
Result(void *value) {
  _type = RT_pointer;
  _u._pointer = value;
}


/**
 *
 */
int CPPExpression::Result::
as_integer() const {
  switch (_type) {
  case RT_integer:
    return _u._integer;

  case RT_real:
    return (int)_u._real;

  case RT_pointer:
    // We don't mind if this loses precision.
    return (int)(intptr_t)(_u._pointer);

  default:
    cerr << "Invalid type\n";
    assert(false);
    return 0;
  }
}

/**
 *
 */
double CPPExpression::Result::
as_real() const {
  switch (_type) {
  case RT_integer:
    return (double)_u._integer;

  case RT_real:
    return _u._real;

  case RT_pointer:
    // We don't mind if this loses precision.
    return (double)(uintptr_t)(_u._pointer);

  default:
    cerr << "Invalid type\n";
    assert(false);
    return 0.0;
  }
}

/**
 *
 */
void *CPPExpression::Result::
as_pointer() const {
  switch (_type) {
  case RT_integer:
    return (void *)(intptr_t)_u._integer;

  case RT_real:
    return (void *)(uintptr_t)_u._real;

  case RT_pointer:
    return _u._pointer;

  default:
    cerr << "Invalid type\n";
    assert(false);
    return nullptr;
  }
}

/**
 *
 */
bool CPPExpression::Result::
as_boolean() const {
  switch (_type) {
  case RT_integer:
    return (_u._integer != 0);

  case RT_real:
    return (_u._real != 0.0);

  case RT_pointer:
    return (_u._pointer != nullptr);

  default:
    cerr << "Invalid type\n";
    assert(false);
    return false;
  }
}

/**
 *
 */
void CPPExpression::Result::
output(std::ostream &out) const {
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

/**
 *
 */
CPPExpression::
CPPExpression(bool value) :
  CPPDeclaration(CPPFile())
{
  _type = T_boolean;
  _u._boolean = value;
}

/**
 *
 */
CPPExpression::
CPPExpression(unsigned long long value) :
  CPPDeclaration(CPPFile())
{
  _type = T_integer;
  _u._integer = value;
}

/**
 *
 */
CPPExpression::
CPPExpression(int value) :
  CPPDeclaration(CPPFile())
{
  _type = T_integer;
  _u._integer = value;
}

/**
 *
 */
CPPExpression::
CPPExpression(long double value) :
  CPPDeclaration(CPPFile())
{
  _type = T_real;
  _u._real = value;
}

/**
 *
 */
CPPExpression::
CPPExpression(const string &value) :
  CPPDeclaration(CPPFile())
{
  _type = T_string;
  _str = value;
}

/**
 *
 */
CPPExpression::
CPPExpression(CPPIdentifier *ident, CPPScope *current_scope,
              CPPScope *global_scope, CPPPreprocessor *error_sink) :
  CPPDeclaration(CPPFile())
{
  CPPDeclaration *decl =
    ident->find_symbol(current_scope, global_scope);

  if (decl != nullptr) {
    CPPInstance *inst = decl->as_instance();
    if (inst != nullptr) {
      _type = T_variable;
      _u._variable = inst;
      return;
    }
    /*CPPFunctionGroup *fgroup = decl->as_function_group();
    if (fgroup != nullptr) {
      _type = T_function;
      _u._fgroup = fgroup;
      return;
    }*/
  }

  _type = T_unknown_ident;
  _u._ident = ident;
  // _u._ident->_native_scope = current_scope;
}

/**
 *
 */
CPPExpression::
CPPExpression(int unary_operator, CPPExpression *op1) :
  CPPDeclaration(CPPFile())
{
  _type = T_unary_operation;
  _u._op._operator = unary_operator;
  _u._op._op1 = op1;
  _u._op._op2 = nullptr;
  _u._op._op3 = nullptr;
}

/**
 *
 */
CPPExpression::
CPPExpression(int binary_operator, CPPExpression *op1, CPPExpression *op2) :
  CPPDeclaration(CPPFile())
{
  _type = T_binary_operation;
  _u._op._operator = binary_operator;
  _u._op._op1 = op1;
  _u._op._op2 = op2;
  _u._op._op3 = nullptr;
}

/**
 *
 */
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

/**
 * Creates an expression that represents a typecast operation.
 */
CPPExpression CPPExpression::
typecast_op(CPPType *type, CPPExpression *op1, Type cast_type) {
  assert(cast_type >= T_typecast && cast_type <= T_reinterpret_cast);
  CPPExpression expr(0);
  expr._type = cast_type;
  expr._u._typecast._to = type;
  expr._u._typecast._op1 = op1;
  return expr;
}

/**
 * Creates an expression that represents a constructor call.
 */
CPPExpression CPPExpression::
construct_op(CPPType *type, CPPExpression *op1) {
  CPPExpression expr(0);
  if (op1 == nullptr) {
    // A default constructor call--no parameters.
    expr._type = T_default_construct;
    expr._u._typecast._to = type;
    expr._u._typecast._op1 = nullptr;
  } else {
    // A normal constructor call, with parameters.
    expr._type = T_construct;
    expr._u._typecast._to = type;
    expr._u._typecast._op1 = op1;
  }
  return expr;
}

/**
 * Creates an expression that represents an aggregate initialization.
 */
CPPExpression CPPExpression::
aggregate_init_op(CPPType *type, CPPExpression *op1) {
  CPPExpression expr(0);
  if (op1 == nullptr) {
    expr._type = T_empty_aggregate_init;
  } else {
    expr._type = T_aggregate_init;
  }
  expr._u._typecast._to = type;
  expr._u._typecast._op1 = op1;
  return expr;
}

/**
 * Creates an expression that represents a use of the new operator.
 */
CPPExpression CPPExpression::
new_op(CPPType *type, CPPExpression *op1) {
  CPPExpression expr(0);
  if (op1 == nullptr) {
    // A default new operation--no parameters.
    expr._type = T_default_new;
    expr._u._typecast._to = type;
    expr._u._typecast._op1 = nullptr;
  } else {
    // A normal new operation, with parameters.
    expr._type = T_new;
    expr._u._typecast._to = type;
    expr._u._typecast._op1 = op1;
  }
  return expr;
}

/**
 * Creates an expression that represents a use of the typeid operator.
 */
CPPExpression CPPExpression::
typeid_op(CPPType *type, CPPType *std_type_info) {
  CPPExpression expr(0);
  expr._type = T_typeid_type;
  expr._u._typeid._type = type;
  expr._u._typeid._std_type_info = std_type_info;
  return expr;
}

/**
 * Creates an expression that represents a use of the typeid operator.
 */
CPPExpression CPPExpression::
typeid_op(CPPExpression *op1, CPPType *std_type_info) {
  CPPExpression expr(0);
  expr._type = T_typeid_expr;
  expr._u._typeid._expr = op1;
  expr._u._typeid._std_type_info = std_type_info;
  return expr;
}

/**
 * Creates an expression that returns a particular type trait.
 */
CPPExpression CPPExpression::
type_trait(int trait, CPPType *type, CPPType *arg) {
  CPPExpression expr(0);
  expr._type = T_type_trait;
  expr._u._type_trait._trait = trait;
  expr._u._type_trait._type = type;
  expr._u._type_trait._arg = arg;
  return expr;
}

/**
 *
 */
CPPExpression CPPExpression::
sizeof_func(CPPType *type) {
  CPPExpression expr(0);
  expr._type = T_sizeof;
  expr._u._typecast._to = type;
  expr._u._typecast._op1 = nullptr;
  return expr;
}

/**
 *
 */
CPPExpression CPPExpression::
sizeof_ellipsis_func(CPPIdentifier *ident) {
  CPPExpression expr(0);
  expr._type = T_sizeof_ellipsis;
  expr._u._ident = ident;
  return expr;
}

/**
 *
 */
CPPExpression CPPExpression::
alignof_func(CPPType *type) {
  CPPExpression expr(0);
  expr._type = T_alignof;
  expr._u._typecast._to = type;
  expr._u._typecast._op1 = nullptr;
  return expr;
}

/**
 *
 */
CPPExpression CPPExpression::
lambda(CPPClosureType *type) {
  CPPExpression expr(0);
  expr._type = T_lambda;
  expr._u._closure_type = type;
  return expr;
}

/**
 *
 */
CPPExpression CPPExpression::
literal(unsigned long long value, CPPInstance *lit_op) {
  CPPExpression expr(0);
  expr._type = T_literal;
  expr._u._literal._value = new CPPExpression(value);
  expr._u._literal._operator = lit_op;
  return expr;
}

/**
 *
 */
CPPExpression CPPExpression::
literal(long double value, CPPInstance *lit_op) {
  CPPExpression expr(0);
  expr._type = T_literal;
  expr._u._literal._value = new CPPExpression(value);
  expr._u._literal._operator = lit_op;
  return expr;
}

/**
 *
 */
CPPExpression CPPExpression::
literal(CPPExpression *value, CPPInstance *lit_op) {
  CPPExpression expr(0);
  expr._type = T_literal;
  expr._u._literal._value = value;
  expr._u._literal._operator = lit_op;
  return expr;
}

/**
 *
 */
CPPExpression CPPExpression::
raw_literal(const string &raw, CPPInstance *lit_op) {
  CPPExpression expr(0);
  expr._type = T_raw_literal;
  expr._str = raw;
  expr._u._literal._value = nullptr;
  expr._u._literal._operator = lit_op;
  return expr;
}

/**
 *
 */
const CPPExpression &CPPExpression::
get_nullptr() {
  static CPPExpression expr(0);
  expr._type = T_nullptr;
  return expr;
}

/**
 *
 */
const CPPExpression &CPPExpression::
get_default() {
  static CPPExpression expr(0);
  expr._type = T_default;
  return expr;
}

/**
 *
 */
const CPPExpression &CPPExpression::
get_delete() {
  static CPPExpression expr(0);
  expr._type = T_delete;
  return expr;
}

/**
 *
 */
CPPExpression::Result CPPExpression::
evaluate() const {
  Result r1, r2;

  switch (_type) {
  case T_nullptr:
    return Result(nullptr);

  case T_boolean:
    return Result((int)_u._boolean);

  case T_integer:
    return Result((int)_u._integer);

  case T_real:
    return Result((double)_u._real);

  case T_string:
  case T_wstring:
  case T_u8string:
  case T_u16string:
  case T_u32string:
    return Result();

  case T_variable:
    if (_u._variable->_type != nullptr &&
        _u._variable->_initializer != nullptr) {
      // A constexpr variable, which is treated as const.
      if (_u._variable->_storage_class & CPPInstance::SC_constexpr) {
        return _u._variable->_initializer->evaluate();
      }
      // A const variable.  Fetch its assigned value.
      CPPConstType *const_type = _u._variable->_type->as_const_type();
      if (const_type != nullptr) {
        return _u._variable->_initializer->evaluate();
      }
    }
    return Result();

  case T_function:
    return Result();

  case T_unknown_ident:
    return Result();

  case T_typecast:
  case T_static_cast:
  case T_dynamic_cast:
  case T_const_cast:
  case T_reinterpret_cast:
    assert(_u._typecast._op1 != nullptr);
    r1 = _u._typecast._op1->evaluate();
    if (r1._type != RT_error) {
      CPPSimpleType *stype = _u._typecast._to->as_simple_type();
      if (stype != nullptr) {
        if (stype->_type == CPPSimpleType::T_bool) {
          return Result(r1.as_boolean());

        } else if (stype->_type == CPPSimpleType::T_int) {
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
  case T_aggregate_init:
  case T_empty_aggregate_init:
  case T_new:
  case T_default_new:
  case T_sizeof:
  case T_sizeof_ellipsis:
    return Result();

  case T_alignof:
    if (_u._typecast._to != nullptr) {
      // Check if the type is defined with an alignas.  TODO: this should
      // probably be moved to a virtual getter on CPPType.
      CPPExtensionType *etype = _u._typecast._to->as_extension_type();
      if (etype != nullptr && etype->_alignment != nullptr) {
        return etype->_alignment->evaluate();
      }
    }
    return Result();

  case T_binary_operation:
    assert(_u._op._op2 != nullptr);
    r2 = _u._op._op2->evaluate();

    // The operators && and || are special cases: these are shirt-circuiting
    // operators.  Thus, if we are using either of these it might be
    // acceptable for the second operand to be invalid, since we might never
    // evaluate it.

    // In all other cases, both operands must be valid in order for the
    // operation to be valid.
    if (r2._type == RT_error &&
        (_u._op._operator != OROR && _u._op._operator != ANDAND)) {
      return r2;
    }
    // Fall through


  case T_trinary_operation:
    // The trinary operator is also a short-circuiting operator: we don't test
    // the second or third operands until we need them.  The only critical one
    // is the first operand.

    // Fall through

  case T_unary_operation:
    assert(_u._op._op1 != nullptr);
    r1 = _u._op._op1->evaluate();
    if (r1._type == RT_error) {
      // Here's one more special case: if the first operand is invalid, it
      // really means we don't know how to evaluate it.  However, if the
      // operator is ||, then it might not matter as long as we can evaluate
      // the second one *and* that comes out to be true.
      if (_u._op._operator == OROR && r2._type == RT_integer &&
          r2.as_boolean()) {
        return r2;
      }

      // Ditto for the operator being && and the second one coming out false.
      if (_u._op._operator == ANDAND && r2._type == RT_integer &&
          !r2.as_boolean()) {
        return r2;
      }

      // Also for the operator being [] and the operand being a string.
      if (_u._op._operator == '[' && r2._type == RT_integer &&
          (_u._op._op1->_type == T_string ||
           _u._op._op1->_type == T_u8string)) {

        int index = (int)r2.as_integer();
        if ((size_t)index == _u._op._op1->_str.size()) {
          return Result(0);
        } else if (index >= 0 && (size_t)index < _u._op._op1->_str.size()) {
          return Result(_u._op._op1->_str[(size_t)index]);
        } else {
          cerr << "array index " << index << " out of bounds of string literal "
               << *_u._op._op1 << "\n";
        }
      }

      return r1;
    }

    switch (_u._op._operator) {
    case UNARY_NOT:
      return Result(!r1.as_boolean());

    case UNARY_NEGATE:
      return Result(~r1.as_integer());

    case UNARY_MINUS:
      return (r1._type == RT_real) ? Result(-r1.as_real()) : Result(-r1.as_integer());

    case UNARY_PLUS:
      return r1;

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
      if (r1.as_boolean()) {
        return r1;
      } else {
        return r2;
      }

    case ANDAND:
      if (r1.as_boolean()) {
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

  case T_literal:
  case T_raw_literal:
    return Result();

  case T_typeid_type:
  case T_typeid_expr:
    return Result();

  case T_type_trait:
    switch (_u._type_trait._trait) {
    case KW_HAS_VIRTUAL_DESTRUCTOR:
      {
        CPPStructType *struct_type = _u._type_trait._type->as_struct_type();
        return Result(struct_type != nullptr && struct_type->has_virtual_destructor());
      }

    case KW_IS_ABSTRACT:
      {
        CPPStructType *struct_type = _u._type_trait._type->as_struct_type();
        return Result(struct_type != nullptr && struct_type->is_abstract());
      }

    case KW_IS_BASE_OF:
      {
        CPPStructType *struct_type1 = _u._type_trait._type->as_struct_type();
        CPPStructType *struct_type2 = _u._type_trait._arg->as_struct_type();
        return Result(struct_type1 != nullptr && struct_type2 != nullptr && struct_type1->is_base_of(struct_type2));
      }

    case KW_IS_CLASS:
      {
        CPPExtensionType *ext_type = _u._type_trait._type->as_extension_type();
        return Result(ext_type != nullptr && (
          ext_type->_type == CPPExtensionType::T_class ||
          ext_type->_type == CPPExtensionType::T_struct));
      }

    case KW_IS_CONSTRUCTIBLE:
      if (_u._type_trait._arg == nullptr) {
        return Result(_u._type_trait._type->is_default_constructible());
      } else {
        return Result(_u._type_trait._type->is_constructible(_u._type_trait._arg));
      }

    case KW_IS_CONVERTIBLE_TO:
      assert(_u._type_trait._arg != nullptr);
      return Result(_u._type_trait._type->is_convertible_to(_u._type_trait._arg));

    case KW_IS_DESTRUCTIBLE:
      return Result(_u._type_trait._type->is_destructible());

    case KW_IS_EMPTY:
      {
        CPPStructType *struct_type = _u._type_trait._type->as_struct_type();
        return Result(struct_type != nullptr && struct_type->is_empty());
      }

    case KW_IS_ENUM:
      return Result(_u._type_trait._type->is_enum());

    case KW_IS_FINAL:
      {
        CPPStructType *struct_type = _u._type_trait._type->as_struct_type();
        return Result(struct_type != nullptr && struct_type->is_final());
      }

    case KW_IS_FUNDAMENTAL:
      return Result(_u._type_trait._type->is_fundamental());

    case KW_IS_POD:
      return Result(_u._type_trait._type->is_trivial() &&
                    _u._type_trait._type->is_standard_layout());

    case KW_IS_POLYMORPHIC:
      {
        CPPStructType *struct_type = _u._type_trait._type->as_struct_type();
        return Result(struct_type != nullptr && struct_type->is_polymorphic());
      }

    case KW_IS_STANDARD_LAYOUT:
      return Result(_u._type_trait._type->is_standard_layout());

    case KW_IS_TRIVIAL:
      return Result(_u._type_trait._type->is_trivial());

    case KW_IS_UNION:
      {
        CPPExtensionType *ext_type = _u._type_trait._type->as_extension_type();
        return Result(ext_type != nullptr &&
          ext_type->_type == CPPExtensionType::T_union);
      }

    default:
      cerr << "**unexpected type trait**\n";
      abort();
    }

  default:
    cerr << "**invalid operand**\n";
    abort();
  }

  return Result();  // Compiler kludge; can't get here.
}

/**
 * Returns the type of the expression, if it is known, or NULL if the type
 * cannot be determined.
 */
CPPType *CPPExpression::
determine_type() const {
  CPPType *t1 = nullptr;
  CPPType *t2 = nullptr;

  static CPPType *nullptr_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_nullptr));

  static CPPType *int_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_int));

  static CPPType *unsigned_long_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_int,
                                        CPPSimpleType::F_unsigned |
                                        CPPSimpleType::F_long));

  static CPPType *bool_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_bool));

  static CPPType *float_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_double));

  static CPPType *char_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_char));

  static CPPType *wchar_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_wchar_t));

  static CPPType *char16_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_char16_t));

  static CPPType *char32_type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_char32_t));

  static CPPType *char_str_type = CPPType::new_type(
    new CPPPointerType(CPPType::new_type(new CPPConstType(char_type))));

  static CPPType *wchar_str_type = CPPType::new_type(
    new CPPPointerType(CPPType::new_type(new CPPConstType(wchar_type))));

  static CPPType *char16_str_type = CPPType::new_type(
    new CPPPointerType(CPPType::new_type(new CPPConstType(char16_type))));

  static CPPType *char32_str_type = CPPType::new_type(
    new CPPPointerType(CPPType::new_type(new CPPConstType(char32_type))));

  switch (_type) {
  case T_nullptr:
    return nullptr_type;

  case T_boolean:
    return bool_type;

  case T_integer:
    return int_type;

  case T_real:
    return float_type;

  case T_string:
    return char_str_type;

  case T_wstring:
    return wchar_str_type;

  case T_u8string:
    return char_str_type;

  case T_u16string:
    return char16_str_type;

  case T_u32string:
    return char32_str_type;

  case T_variable:
    return _u._variable->_type;

  case T_function:
    if (_u._fgroup->get_return_type() == nullptr) {
      // There are multiple functions by this name that have different return
      // types.  We could attempt to differentiate them based on the parameter
      // list, but that's a lot of work.  Let's just give up.
      return nullptr;
    }
    return _u._fgroup->_instances.front()->_type;

  case T_unknown_ident:
    return nullptr;

  case T_typecast:
  case T_static_cast:
  case T_dynamic_cast:
  case T_const_cast:
  case T_reinterpret_cast:
  case T_construct:
  case T_default_construct:
  case T_aggregate_init:
  case T_empty_aggregate_init:
    return _u._typecast._to;

  case T_new:
  case T_default_new:
    return CPPType::new_type(new CPPPointerType(_u._typecast._to));

  case T_sizeof:
  case T_sizeof_ellipsis:
  case T_alignof:
    // Note: this should actually be size_t, but that is defined as a typedef
    // in parser-inc.  We could try to resolve it, but that's hacky.  Eh, it's
    // probably not worth the effort to get this right.
    return unsigned_long_type;

  case T_binary_operation:
  case T_trinary_operation:
    assert(_u._op._op2 != nullptr);
    t2 = _u._op._op2->determine_type();
    // Fall through

  case T_unary_operation:
    assert(_u._op._op1 != nullptr);
    t1 = _u._op._op1->determine_type();

    switch (_u._op._operator) {
    case UNARY_NOT:
      return bool_type;

    case UNARY_NEGATE:
      return int_type;

    case UNARY_MINUS:
    case UNARY_PLUS:
      if (t1 != nullptr) {
        switch (t1->get_subtype()) {
        case CPPDeclaration::ST_array:
          // Decay into pointer.
          return CPPType::new_type(new CPPPointerType(t1->as_array_type()->_element_type));

        case CPPDeclaration::ST_enum:
          // Convert into integral type.
          return t1->as_enum_type()->get_underlying_type();

        case CPPDeclaration::ST_simple:
          {
            CPPSimpleType *simple_type = t1->as_simple_type();
            if ((simple_type->_flags & CPPSimpleType::F_short) != 0 ||
                simple_type->_type == CPPSimpleType::T_bool ||
                simple_type->_type == CPPSimpleType::T_wchar_t ||
                simple_type->_type == CPPSimpleType::T_char16_t) {
              // Integer promotion.
              return int_type;
            }
          }
          // Fall through.
        default:
          return t1;
        }
      }
      return nullptr;

    case UNARY_STAR:
    case '[': // Array element reference
      if (t1 != nullptr) {
        if (t1->as_pointer_type()) {
          return t1->as_pointer_type()->_pointing_at;
        }
        if (t1->as_array_type()) {
          return t1->as_array_type()->_element_type;
        }
      }
      return nullptr;

    case UNARY_REF:
      return t1;

    case '*':
    case '/':
    case '+':
    case '-':
      if (t1 == nullptr) {
        return t2;
      } else if (t2 == nullptr) {
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
      return nullptr;

    case 'f': // Function evaluation
      if (t1 != nullptr) {
        // Easy case, function with only a single overload.
        CPPFunctionType *ftype = t1->as_function_type();
        if (ftype != nullptr) {
          return ftype->_return_type;
        }
      } else if (_u._op._op1->_type == T_function) {
        CPPFunctionGroup *fgroup = _u._op._op1->_u._fgroup;
        if (_u._op._op2 == nullptr) {
          // If we are passing no args, look for an overload that has takes no
          // args.
          for (auto it = fgroup->_instances.begin(); it != fgroup->_instances.end(); ++it) {
            CPPInstance *inst = *it;
            if (inst != nullptr && inst->_type != nullptr) {
              CPPFunctionType *type = inst->_type->as_function_type();
              if (type != nullptr && type->accepts_num_parameters(0)) {
                return type->_return_type;
              }
            }
          }
        } else {
          //TODO
        }
      }
      return nullptr;

    case ',':
      return t2;

    default:
      cerr << "**unexpected operator**\n";
      abort();
    }

  case T_literal:
  case T_raw_literal:
    if (_u._literal._operator != nullptr) {
      CPPType *type = _u._literal._operator->_type;

      CPPFunctionType *ftype = type->as_function_type();
      if (ftype != nullptr) {
        return ftype->_return_type;
      }
    }
    return nullptr;

  case T_typeid_type:
  case T_typeid_expr:
    return _u._typeid._std_type_info;

  case T_type_trait:
    return bool_type;

  case T_lambda:
    return _u._closure_type;

  default:
    cerr << "**invalid operand**\n";
    abort();
  }

  return nullptr;  // Compiler kludge; can't get here.
}

/**
 * Returns true if this is an lvalue expression.
 */
bool CPPExpression::
is_lvalue() const {
  switch (_type) {
  case T_variable:
  case T_function:
  case T_unknown_ident:
    return true;

  case T_typecast:
  case T_static_cast:
  case T_dynamic_cast:
  case T_const_cast:
  case T_reinterpret_cast:
    {
      CPPReferenceType *ref_type = _u._typecast._to->as_reference_type();
      return ref_type != nullptr && ref_type->_value_category == CPPReferenceType::VC_lvalue;
    }

  case T_unary_operation:
    if (_u._op._operator == 'f') {
      // A function returning an lvalue reference.
      CPPType *return_type = determine_type();
      if (return_type != nullptr) {
        CPPReferenceType *ref_type = return_type->as_reference_type();
        return ref_type != nullptr && ref_type->_value_category == CPPReferenceType::VC_lvalue;
      }
    }
    return _u._op._operator == PLUSPLUS
        || _u._op._operator == MINUSMINUS
        || _u._op._operator == '*';

  case T_binary_operation:
    if (_u._op._operator == ',') {
      CPPReferenceType *ref_type = _u._op._op2->as_reference_type();
      return ref_type != nullptr && ref_type->_value_category == CPPReferenceType::VC_lvalue;
    }
    return (_u._op._operator == POINTSAT || _u._op._operator == ',');

  case T_trinary_operation:
    return _u._op._op2->is_lvalue() && _u._op._op3->is_lvalue();

  case T_literal:
  case T_raw_literal:
    return true;

  default:
    break;
  }

  return false;
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
bool CPPExpression::
is_fully_specified() const {
  if (!CPPDeclaration::is_fully_specified()) {
    return false;
  }

  switch (_type) {
  case T_nullptr:
  case T_boolean:
  case T_integer:
  case T_real:
  case T_string:
  case T_wstring:
  case T_u8string:
  case T_u16string:
  case T_u32string:
    return false;

  case T_variable:
    return _u._variable->is_fully_specified();

  case T_function:
    return _u._fgroup->is_fully_specified();

  case T_unknown_ident:
    return _u._ident->is_fully_specified();

  case T_typecast:
  case T_static_cast:
  case T_dynamic_cast:
  case T_const_cast:
  case T_reinterpret_cast:
  case T_construct:
  case T_aggregate_init:
  case T_new:
    return (_u._typecast._to->is_fully_specified() &&
            _u._typecast._op1->is_fully_specified());

  case T_default_construct:
  case T_empty_aggregate_init:
  case T_default_new:
  case T_sizeof:
  case T_alignof:
    return _u._typecast._to->is_fully_specified();

  case T_sizeof_ellipsis:
    return _u._ident->is_fully_specified();

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

  case T_literal:
    return _u._literal._value->is_fully_specified() &&
      _u._literal._operator->is_fully_specified();

  case T_raw_literal:
    return _u._literal._value->is_fully_specified();

  case T_typeid_type:
    return _u._typeid._type->is_fully_specified();

  case T_typeid_expr:
    return _u._typeid._expr->is_fully_specified();

  case T_type_trait:
    return _u._type_trait._type->is_fully_specified();

  case T_lambda:
    return _u._closure_type->is_fully_specified();

  default:
    return true;
  }
}

/**
 *
 */
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
    if (decl != nullptr) {
      CPPInstance *inst = decl->as_instance();
      if (inst != nullptr) {
        rep->_type = T_variable;
        rep->_u._variable = inst;
        any_changed = true;

        decl = inst->substitute_decl(subst, current_scope, global_scope);
        if (decl != inst) {
          if (decl->as_instance()) {
            // Replacing the variable reference with another variable
            // reference.
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
      if (fgroup != nullptr) {
        rep->_type = T_function;
        rep->_u._fgroup = fgroup;
        any_changed = true;
      }
    }

    break;

  case T_typecast:
  case T_static_cast:
  case T_dynamic_cast:
  case T_const_cast:
  case T_reinterpret_cast:
  case T_construct:
  case T_aggregate_init:
  case T_new:
    rep->_u._typecast._op1 =
      _u._typecast._op1->substitute_decl(subst, current_scope, global_scope)
      ->as_expression();
    any_changed = any_changed || (rep->_u._typecast._op1 != _u._typecast._op1);
    // fall through

  case T_default_construct:
  case T_empty_aggregate_init:
  case T_default_new:
  case T_sizeof:
  case T_alignof:
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

  case T_typeid_type:
    rep->_u._typeid._type =
      _u._typeid._type->substitute_decl(subst, current_scope, global_scope)
      ->as_type();
    any_changed = any_changed || (rep->_u._typeid._type != _u._typeid._type);
    break;

  case T_typeid_expr:
    rep->_u._typeid._expr =
      _u._typeid._expr->substitute_decl(subst, current_scope, global_scope)
      ->as_expression();
    any_changed = any_changed || (rep->_u._typeid._expr != _u._typeid._expr);
    break;

  case T_type_trait:
    rep->_u._type_trait._type =
      _u._type_trait._type->substitute_decl(subst, current_scope, global_scope)
      ->as_type();
    any_changed = any_changed || (rep->_u._type_trait._type != _u._type_trait._type);
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


/**
 * Returns true if any type within the expression list is a CPPTBDType and
 * thus isn't fully determined right now.
 */
bool CPPExpression::
is_tbd() const {
  switch (_type) {
  case T_variable:
    if (_u._variable->_type != nullptr &&
        _u._variable->_initializer != nullptr) {
      if (_u._variable->_storage_class & CPPInstance::SC_constexpr) {
        return false;
      }
      CPPConstType *const_type = _u._variable->_type->as_const_type();
      if (const_type != nullptr) {
        return false;
      }
    }

    return true;

  case T_unknown_ident:
    return true;

  case T_typecast:
  case T_static_cast:
  case T_dynamic_cast:
  case T_const_cast:
  case T_reinterpret_cast:
  case T_construct:
  case T_aggregate_init:
  case T_empty_aggregate_init:
  case T_new:
  case T_default_construct:
  case T_default_new:
  case T_sizeof:
  case T_alignof:
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

  case T_typeid_type:
    return _u._typeid._type->is_tbd();

  case T_typeid_expr:
    return _u._typeid._expr->is_tbd();

  case T_type_trait:
    return _u._type_trait._type->is_tbd();

  case T_lambda:
    return _u._closure_type->is_tbd();

  default:
    return false;
  }
}

/**
 *
 */
void CPPExpression::
output(std::ostream &out, int indent_level, CPPScope *scope, bool) const {
  switch (_type) {
  case T_nullptr:
    out << "nullptr";
    break;

  case T_boolean:
    out << (_u._boolean ? "true" : "false");
    break;

  case T_integer:
    out << _u._integer;
    break;

  case T_real:
    {
      // We use our own dtoa implementation here because it guarantees to
      // never format the number as an integer.
      char buffer[32];
      pdtoa(_u._real, buffer);
      out << buffer;
    }
    break;

  case T_string:
  case T_wstring:
  case T_u8string:
  case T_u16string:
  case T_u32string:
    {
      switch (_type) {
      case T_wstring:
        out << 'L';
        break;
      case T_u8string:
        out << "u8";
        break;
      case T_u16string:
        out << "u";
        break;
      case T_u32string:
        out << "U";
        break;
      default:
        break;
      }
      // We don't really care about preserving the encoding for now.
      out << '"';
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

        case '\\':
          out << "\\\\";
          break;

        default:
          if (isprint(*si)) {
            out << *si;
          } else {
            out << '\\' << std::oct << std::setw(3) << std::setfill('0') << (int)(*si)
                << std::dec << std::setw(0);
          }
        }
      }
    }
    out << '"';
    break;

  case T_variable:
    // We can just refer to the variable by name, except if it's a private
    // constant, in which case we have to compute the value, since we may have
    // to use it in generated code.
    if (_u._variable->_type != nullptr &&
        _u._variable->_initializer != nullptr &&
        _u._variable->_vis > V_public) {
      // A constexpr or const variable.  Fetch its assigned value.
      CPPConstType *const_type = _u._variable->_type->as_const_type();
      if ((_u._variable->_storage_class & CPPInstance::SC_constexpr) != 0 ||
          const_type != nullptr) {
        _u._variable->_initializer->output(out, indent_level, scope, false);
        break;
      }
    }
    _u._variable->_ident->output(out, scope);
    break;

  case T_function:
    // Pick any instance; they all have the same name anyway.
    if (!_u._fgroup->_instances.empty() && _u._fgroup->_instances[0]->_ident != nullptr) {
      _u._fgroup->_instances[0]->_ident->output(out, scope);
    } else {
      out << _u._fgroup->_name;
    }
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

  case T_static_cast:
    out << "static_cast<";
    _u._typecast._to->output(out, indent_level, scope, false);
    out << ">(";
    _u._typecast._op1->output(out, indent_level, scope, false);
    out << ")";
    break;

  case T_dynamic_cast:
    out << "dynamic_cast<";
    _u._typecast._to->output(out, indent_level, scope, false);
    out << ">(";
    _u._typecast._op1->output(out, indent_level, scope, false);
    out << ")";
    break;

  case T_const_cast:
    out << "const_cast<";
    _u._typecast._to->output(out, indent_level, scope, false);
    out << ">(";
    _u._typecast._op1->output(out, indent_level, scope, false);
    out << ")";
    break;

  case T_reinterpret_cast:
    out << "reinterpret_cast<";
    _u._typecast._to->output(out, indent_level, scope, false);
    out << ">(";
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

  case T_aggregate_init:
    _u._typecast._to->output(out, indent_level, scope, false);
    out << "{";
    _u._typecast._op1->output(out, indent_level, scope, false);
    out << "}";
    break;

  case T_empty_aggregate_init:
    _u._typecast._to->output(out, indent_level, scope, false);
    out << "{}";
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

  case T_sizeof_ellipsis:
    out << "sizeof...(";
    _u._ident->output(out, scope);
    out << ")";
    break;

  case T_alignof:
    out << "alignof(";
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
      out << '-';
      _u._op._op1->output(out, indent_level, scope, false);
      break;

    case UNARY_PLUS:
      out << '+';
      _u._op._op1->output(out, indent_level, scope, false);
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
      _u._op._op1->output(out, indent_level, scope, false);
      out << "()";
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
      _u._op._op1->output(out, indent_level, scope, false);
      out << ".";
      _u._op._op2->output(out, indent_level, scope, false);
      break;

    case POINTSAT:
      _u._op._op1->output(out, indent_level, scope, false);
      out << "->";
      _u._op._op2->output(out, indent_level, scope, false);
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

  case T_literal:
    _u._literal._value->output(out, indent_level, scope, false);
    if (_u._literal._operator != nullptr) {
      string name = _u._literal._operator->get_simple_name();
      assert(name.substr(0, 12) == "operator \"\" ");
      out << name.substr(12);
    }
    break;

  case T_raw_literal:
    out << _str;
    if (_u._literal._operator != nullptr) {
      string name = _u._literal._operator->get_simple_name();
      assert(name.substr(0, 12) == "operator \"\" ");
      out << name.substr(12);
    }
    break;

  case T_typeid_type:
    out << "typeid(";
    _u._typeid._type->output(out, indent_level, scope, false);
    out << ")";
    break;

  case T_typeid_expr:
    out << "typeid(";
    _u._typeid._expr->output(out, indent_level, scope, false);
    out << ")";
    break;

  case T_default:
    out << "default";
    break;

  case T_delete:
    out << "delete";
    break;

  case T_type_trait:
    switch (_u._type_trait._trait) {
    case KW_HAS_VIRTUAL_DESTRUCTOR:
      out << "__has_virtual_destructor";
      break;
    case KW_IS_ABSTRACT:
      out << "__is_abstract";
      break;
    case KW_IS_BASE_OF:
      out << "__is_base_of";
      break;
    case KW_IS_CLASS:
      out << "__is_class";
      break;
    case KW_IS_CONSTRUCTIBLE:
      out << "__is_constructible";
      break;
    case KW_IS_CONVERTIBLE_TO:
      out << "__is_convertible_to";
      break;
    case KW_IS_DESTRUCTIBLE:
      out << "__is_destructible";
      break;
    case KW_IS_EMPTY:
      out << "__is_empty";
      break;
    case KW_IS_ENUM:
      out << "__is_enum";
      break;
    case KW_IS_FINAL:
      out << "__is_final";
      break;
    case KW_IS_FUNDAMENTAL:
      out << "__is_fundamental";
      break;
    case KW_IS_POD:
      out << "__is_pod";
      break;
    case KW_IS_POLYMORPHIC:
      out << "__is_polymorphic";
      break;
    case KW_IS_STANDARD_LAYOUT:
      out << "__is_standard_layout";
      break;
    case KW_IS_TRIVIAL:
      out << "__is_trivial";
      break;
    case KW_IS_UNION:
      out << "__is_union";
      break;
    default:
      out << (evaluate().as_boolean() ? "true" : "false");
      return;
    }
    out << '(';
    _u._type_trait._type->output(out, indent_level, scope, false);
    out << ')';
    break;

  case T_lambda:
    _u._closure_type->output(out, indent_level, scope, false);
    break;

  default:
    out << "(** invalid operand type " << (int)_type << " **)";
  }
}

/**
 *
 */
CPPDeclaration::SubType CPPExpression::
get_subtype() const {
  return ST_expression;
}

/**
 *
 */
CPPExpression *CPPExpression::
as_expression() {
  return this;
}

/**
 * Returns the most general of the two given types.
 */
CPPType *CPPExpression::
elevate_type(CPPType *t1, CPPType *t2) {
  CPPSimpleType *st1 = t1->as_simple_type();
  CPPSimpleType *st2 = t2->as_simple_type();

  if (st1 == nullptr || st2 == nullptr) {
    // Nothing we can do about this.  Who knows?
    return nullptr;
  }

  if (st1->_type == st2->_type) {
    // They have the same type, so return the one with the largest flag bits.
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

/**
 * Called by CPPDeclaration to determine whether this expr is equivalent to
 * another expr.
 */
bool CPPExpression::
is_equal(const CPPDeclaration *other) const {
  const CPPExpression *ot = ((CPPDeclaration *)other)->as_expression();
  assert(ot != nullptr);

  if (_type != ot->_type) {
    return false;
  }

  switch (_type) {
  case T_nullptr:
    return true;

  case T_boolean:
    return _u._boolean == ot->_u._boolean;

  case T_integer:
    return _u._integer == ot->_u._integer;

  case T_real:
    return _u._real == ot->_u._real;

  case T_string:
  case T_wstring:
  case T_u8string:
  case T_u16string:
  case T_u32string:
    return _str == ot->_str;

  case T_variable:
    return _u._variable == ot->_u._variable;

  case T_function:
    return _u._fgroup == ot->_u._fgroup;

  case T_unknown_ident:
  case T_sizeof_ellipsis:
    return *_u._ident == *ot->_u._ident;

  case T_typecast:
  case T_static_cast:
  case T_dynamic_cast:
  case T_const_cast:
  case T_reinterpret_cast:
  case T_construct:
  case T_aggregate_init:
  case T_new:
    return _u._typecast._to == ot->_u._typecast._to &&
      *_u._typecast._op1 == *ot->_u._typecast._op1;

  case T_default_construct:
  case T_empty_aggregate_init:
  case T_default_new:
  case T_sizeof:
  case T_alignof:
    return _u._typecast._to == ot->_u._typecast._to;

  case T_unary_operation:
    return *_u._op._op1 == *ot->_u._op._op1;

  case T_binary_operation:
    return *_u._op._op1 == *ot->_u._op._op1 &&
      *_u._op._op2 == *ot->_u._op._op2;

  case T_trinary_operation:
    return *_u._op._op1 == *ot->_u._op._op1 &&
      *_u._op._op2 == *ot->_u._op._op2;

  case T_literal:
    return *_u._literal._value == *ot->_u._literal._value &&
      _u._literal._operator == ot->_u._literal._operator;

  case T_raw_literal:
    return _str == ot->_str &&
      _u._literal._operator == ot->_u._literal._operator;

  case T_typeid_type:
    return _u._typeid._type == ot->_u._typeid._type;

  case T_typeid_expr:
    return _u._typeid._expr == ot->_u._typeid._expr;

  case T_type_trait:
    return _u._type_trait._trait == ot->_u._type_trait._trait &&
           _u._type_trait._type == ot->_u._type_trait._type;

  case T_lambda:
    return _u._closure_type == ot->_u._closure_type;

  default:
    cerr << "(** invalid operand type " << (int)_type << " **)";
  }

  return true;
}

/**
 * Called by CPPDeclaration to determine whether this expr should be ordered
 * before another expr of the same type, in an arbitrary but fixed ordering.
 */
bool CPPExpression::
is_less(const CPPDeclaration *other) const {
  const CPPExpression *ot = ((CPPDeclaration *)other)->as_expression();
  assert(ot != nullptr);

  if (_type != ot->_type) {
    return (int)_type < (int)ot->_type;
  }

  switch (_type) {
  case T_nullptr:
    return false;

  case T_boolean:
    return _u._boolean < ot->_u._boolean;

  case T_integer:
    return _u._integer < ot->_u._integer;

  case T_real:
    return _u._real < ot->_u._real;

  case T_string:
  case T_wstring:
  case T_u8string:
  case T_u16string:
  case T_u32string:
    return _str < ot->_str;

  case T_variable:
    return _u._variable < ot->_u._variable;

  case T_function:
    return *_u._fgroup < *ot->_u._fgroup;

  case T_unknown_ident:
  case T_sizeof_ellipsis:
    return *_u._ident < *ot->_u._ident;

  case T_typecast:
  case T_static_cast:
  case T_dynamic_cast:
  case T_const_cast:
  case T_reinterpret_cast:
  case T_construct:
  case T_aggregate_init:
  case T_new:
    if (_u._typecast._to != ot->_u._typecast._to) {
      return _u._typecast._to < ot->_u._typecast._to;
    }
    return *_u._typecast._op1 < *ot->_u._typecast._op1;

  case T_default_construct:
  case T_empty_aggregate_init:
  case T_default_new:
  case T_sizeof:
  case T_alignof:
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

  case T_literal:
    if (_u._literal._operator != ot->_u._literal._operator) {
      return _u._literal._operator < ot->_u._literal._operator;
    }
    return *_u._literal._value < *ot->_u._literal._value;

  case T_raw_literal:
    if (_u._literal._operator != ot->_u._literal._operator) {
      return _u._literal._operator < ot->_u._literal._operator;
    }
    return _str < ot->_str;

  case T_typeid_type:
    return _u._typeid._type < ot->_u._typeid._type;

  case T_typeid_expr:
    return *_u._typeid._expr < *ot->_u._typeid._expr;

  case T_type_trait:
    if (_u._type_trait._trait != ot->_u._type_trait._trait) {
      return _u._type_trait._trait < ot->_u._type_trait._trait;
    }
    return *_u._type_trait._type < *ot->_u._type_trait._type;

  case T_lambda:
    return _u._closure_type < ot->_u._closure_type;

  default:
    cerr << "(** invalid operand type " << (int)_type << " **)";
  }

  return false;
}
