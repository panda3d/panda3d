/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppEnumType.cxx
 * @author drose
 * @date 1999-10-25
 */

#include "cppEnumType.h"
#include "cppTypedefType.h"
#include "cppExpression.h"
#include "cppSimpleType.h"
#include "cppConstType.h"
#include "cppScope.h"
#include "cppParser.h"
#include "cppIdentifier.h"
#include "indent.h"

/**
 * Creates an untyped enum.
 */
CPPEnumType::
CPPEnumType(Type type, CPPIdentifier *ident, CPPScope *current_scope,
            CPPScope *scope, const CPPFile &file) :
  CPPExtensionType(type, ident, current_scope, file),
  _scope(scope),
  _element_type(nullptr),
  _last_value(nullptr)
{
  _parent_scope = (type == T_enum) ? current_scope : scope;

  if (ident != nullptr) {
    ident->_native_scope = current_scope;
  }
}

/**
 * Creates a typed enum.
 */
CPPEnumType::
CPPEnumType(Type type, CPPIdentifier *ident, CPPType *element_type,
            CPPScope *current_scope, CPPScope *scope, const CPPFile &file) :
  CPPExtensionType(type, ident, current_scope, file),
  _scope(scope),
  _element_type(element_type),
  _last_value(nullptr)
{
  _parent_scope = (type == T_enum) ? current_scope : scope;
  if (ident != nullptr) {
    ident->_native_scope = current_scope;
  }
}

/**
 * Returns true if this is a scoped enum.
 */
bool CPPEnumType::
is_scoped() const {
  return (_type != T_enum);
}

/**
 * Returns the integral type used to store enum values.
 */
CPPType *CPPEnumType::
get_underlying_type() {
  if (_element_type == nullptr) {
    // This enum is untyped.  Use a suitable default, ie.  'int'. In the
    // future, we might want to check whether it fits in an int.
    static CPPType *default_element_type = nullptr;
    if (default_element_type == nullptr) {
      default_element_type =
        CPPType::new_type(new CPPConstType(new CPPSimpleType(CPPSimpleType::T_int, 0)));
    }

    return default_element_type;
  } else {
    // This enum has an explicit type, so use that.
    return CPPType::new_type(new CPPConstType(_element_type));
  }
}

/**
 *
 */
CPPInstance *CPPEnumType::
add_element(const std::string &name, CPPExpression *value, CPPPreprocessor *preprocessor, const cppyyltype &pos) {
  CPPIdentifier *ident = new CPPIdentifier(name);
  ident->_native_scope = _parent_scope;

  CPPInstance *inst;
  if (_type == T_enum) {
    // Weakly typed enum.
    inst = new CPPInstance(get_underlying_type(), ident);
  } else {
    // C++11-style strongly typed enum.
    inst = new CPPInstance(this, ident);
  }
  inst->_storage_class |= CPPInstance::SC_constexpr;
  _elements.push_back(inst);

  if (value == nullptr) {
    if (_last_value == nullptr) {
      // This is the first value, and should therefore be 0.
      static CPPExpression *const zero = new CPPExpression(0);
      value = zero;

    } else if (_last_value->_type == CPPExpression::T_integer) {
      value = new CPPExpression(_last_value->_u._integer + 1);

    } else {
      // We may not be able to determine the value just yet.  No problem;
      // we'll just define it as another expression.
      static CPPExpression *const one = new CPPExpression(1);
      value = new CPPExpression('+', _last_value, one);
    }
  }
  inst->_initializer = value;
  _last_value = value;

  if (preprocessor != nullptr) {
    // Same-line comment?
    CPPCommentBlock *comment =
      preprocessor->get_comment_on(pos.first_line, pos.file);

    if (comment == nullptr) {
      // Nope.  Check for a comment before this line.
      comment =
        preprocessor->get_comment_before(pos.first_line, pos.file);

      if (comment != nullptr) {
        // This is a bit of a hack, but it prevents us from picking up a same-
        // line comment from the previous line.
        if (comment->_line_number != pos.first_line - 1 ||
            comment->_col_number <= pos.first_column) {

          inst->_leading_comment = comment;
        }
      }
    } else {
      inst->_leading_comment = comment;
    }
  }

  // Add the value to the enum scope (as per C++11), assuming it's not anonymous.
  if (_scope != nullptr) {
    _scope->add_enum_value(inst);
  }

  // Now add it to the containing scope as well if it's not an "enum class".
  if (!is_scoped() && _parent_scope != nullptr) {
    _parent_scope->add_enum_value(inst);
  }

  return inst;
}

/**
 * Returns true if the type has not yet been fully specified, false if it has.
 */
bool CPPEnumType::
is_incomplete() const {
  return false;
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
bool CPPEnumType::
is_fully_specified() const {
  if (!CPPDeclaration::is_fully_specified()) {
    return false;
  }

  if (_ident != nullptr && !_ident->is_fully_specified()) {
    return false;
  }

  if (_element_type != nullptr && !_element_type->is_fully_specified()) {
    return false;
  }

  Elements::const_iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    if (!(*ei)->is_fully_specified()) {
      return false;
    }
  }

  return true;
}

/**
 *
 */
CPPDeclaration *CPPEnumType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    return (*si).second;
  }

  CPPEnumType *rep = new CPPEnumType(*this);

  if (_ident != nullptr) {
    rep->_ident =
      _ident->substitute_decl(subst, current_scope, global_scope);
  }

  if (_element_type != nullptr) {
    rep->_element_type =
      _element_type->substitute_decl(subst, current_scope, global_scope)
      ->as_type();
  }

  bool any_changed = false;

  for (size_t i = 0; i < _elements.size(); ++i) {
    // We don't just do substitute_decl on the instance, which could lead to
    // an infinite recursion.
    CPPInstance *element = _elements[i];
    CPPExpression *value = element->_initializer->
      substitute_decl(subst, current_scope, global_scope)->as_expression();

    if (is_scoped()) {
      // For a strong enum, we consider the elements to be of this type.
      if (value != element->_initializer) {
        rep->_elements[i] = new CPPInstance(rep, element->_ident);
        rep->_elements[i]->_initializer = value;
        any_changed = true;
      }
    } else if (value != element->_initializer ||
               rep->get_underlying_type() != get_underlying_type()) {
      // In an unscoped enum, the elements are integers.
      rep->_elements[i] = new CPPInstance(rep->get_underlying_type(), element->_ident);
      rep->_elements[i]->_initializer = value;
      any_changed = true;
    }
  }

  if (rep->_ident == _ident &&
      rep->_element_type == _element_type &&
      !any_changed) {
    delete rep;
    rep = this;
  }
  rep = CPPType::new_type(rep)->as_enum_type();
  subst.insert(SubstDecl::value_type(this, rep));

  return rep;
}

/**
 *
 */
void CPPEnumType::
output(std::ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (!complete && _ident != nullptr) {
    // If we have a name, use it.
    if (cppparser_output_class_keyword) {
      out << _type << " ";
    }
    out << _ident->get_local_name(scope);

  } else {
    out << _type;
    if (_ident != nullptr) {
      out << " " << _ident->get_local_name(scope);
    }
    if (_element_type != nullptr) {
      out << " : " << _element_type->get_local_name(scope);
    }

    out << " {\n";
    Elements::const_iterator ei;
    for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
      indent(out, indent_level + 2) << (*ei)->get_local_name();
      if ((*ei)->_initializer != nullptr) {
        out << " = " << *(*ei)->_initializer;
      }
      out << ",\n";
    }
    indent(out, indent_level) << "}";
  }
}

/**
 *
 */
CPPDeclaration::SubType CPPEnumType::
get_subtype() const {
  return ST_enum;
}

/**
 *
 */
CPPEnumType *CPPEnumType::
as_enum_type() {
  return this;
}
