/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppClosureType.cxx
 * @author rdb
 * @date 2017-01-14
 */

#include "cppClosureType.h"
#include "cppParameterList.h"
#include "cppExpression.h"

/**
 *
 */
CPPClosureType::
CPPClosureType(CaptureType default_capture) :
  CPPFunctionType(nullptr, nullptr, 0),
  _default_capture(default_capture) {
}

/**
 *
 */
CPPClosureType::
CPPClosureType(const CPPClosureType &copy) :
  CPPFunctionType(copy),
  _captures(copy._captures),
  _default_capture(copy._default_capture)
{
}

/**
 *
 */
void CPPClosureType::
operator = (const CPPClosureType &copy) {
  CPPFunctionType::operator = (copy);
  _captures = copy._captures;
  _default_capture = copy._default_capture;
}

/**
 * Adds a new capture to the beginning of the capture list.
 */
void CPPClosureType::
add_capture(std::string name, CaptureType type, CPPExpression *initializer) {
  if (type == CT_none) {
    if (name == "this") {
      type = CT_by_reference;
    } else {
      type = CT_by_value;
    }
  }

  Capture capture = {std::move(name), type, initializer};
  _captures.insert(_captures.begin(), std::move(capture));
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
bool CPPClosureType::
is_fully_specified() const {
  return CPPFunctionType::is_fully_specified();
}

/**
 * Returns true if the type is default-constructible.
 */
bool CPPClosureType::
is_default_constructible() const {
  return false;
}

/**
 * Returns true if the type is copy-constructible.
 */
bool CPPClosureType::
is_copy_constructible() const {
  return true;
}

/**
 * Returns true if the type is destructible.
 */
bool CPPClosureType::
is_destructible() const {
  return true;
}

/**
 *
 */
void CPPClosureType::
output(std::ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  out.put('[');

  bool have_capture = false;
  switch (_default_capture) {
  case CT_none:
    break;
  case CT_by_reference:
    out.put('&');
    have_capture = true;
    break;
  case CT_by_value:
    out.put('=');
    have_capture = true;
    break;
  }

  Captures::const_iterator it;
  for (it = _captures.begin(); it != _captures.end(); ++it) {
    const Capture &capture = *it;
    if (have_capture) {
      out << ", ";
    }
    if (capture._name == "this") {
      if (capture._type == CT_by_value) {
        out.put('*');
      }
    } else {
      if (capture._type == CT_by_reference) {
        out.put('&');
      }
    }
    out << capture._name;

    if (capture._initializer != nullptr) {
      out << " = " << *capture._initializer;
    }

    have_capture = true;
  }
  out.put(']');

  if (_parameters != nullptr) {
    out.put('(');
    _parameters->output(out, scope, true, -1);
    out.put(')');
  }

  if (_flags & F_noexcept) {
    out << " noexcept";
  }

  if (!_attributes.is_empty()) {
    out << " " << _attributes;
  }

  if (_return_type != nullptr) {
    out << " -> ";
    _return_type->output(out, indent_level, scope, false);
  }

  out << " {}";
}

/**
 *
 */
CPPDeclaration::SubType CPPClosureType::
get_subtype() const {
  return ST_closure;
}

/**
 *
 */
CPPClosureType *CPPClosureType::
as_closure_type() {
  return this;
}

/**
 * Called by CPPDeclaration() to determine whether this type is equivalent to
 * another type of the same type.
 */
bool CPPClosureType::
is_equal(const CPPDeclaration *other) const {
  return (this == other);
}


/**
 * Called by CPPDeclaration() to determine whether this type should be ordered
 * before another type of the same type, in an arbitrary but fixed ordering.
 */
bool CPPClosureType::
is_less(const CPPDeclaration *other) const {
  return (this < other);
}
