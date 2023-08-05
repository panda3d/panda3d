/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppAttributeList.cxx
 * @author rdb
 * @date 2022-10-23
 */

#include "cppAttributeList.h"
#include "cppExpression.h"
#include "cppIdentifier.h"
#include "cppType.h"

/**
 * Returns true if no attributes have been defined.
 */
bool CPPAttributeList::
is_empty() const {
  return _attributes.empty() && _alignas == nullptr;
}

/**
 * Returns true if the attribute list has an attribute with the given name.
 */
bool CPPAttributeList::
has_attribute(const std::string &name) const {
  for (const Attribute &attr : _attributes) {
    if (attr._ident->get_fully_scoped_name() == name) {
      return true;
    }
  }
  return false;
}

/**
 *
 */
bool CPPAttributeList::
operator == (const CPPAttributeList &other) const {
  if (_attributes.size() != other._attributes.size()) {
    return false;
  }
  if ((_alignas != nullptr) != (other._alignas != nullptr)) {
    return false;
  }
  if (_alignas != nullptr && *_alignas != *other._alignas) {
    return false;
  }
  for (size_t i = 0; i < _attributes.size(); ++i) {
    if (_attributes[i]._ident != other._attributes[i]._ident) {
      return false;
    }
    if (_attributes[i]._reason != other._attributes[i]._reason) {
      return false;
    }
  }
  return true;
}

/**
 *
 */
bool CPPAttributeList::
operator != (const CPPAttributeList &other) const {
  return !(*this == other);
}

/**
 *
 */
bool CPPAttributeList::
operator < (const CPPAttributeList &other) const {
  if (_attributes.size() != other._attributes.size()) {
    return _attributes.size() < other._attributes.size();
  }
  if ((_alignas != nullptr) != (other._alignas != nullptr)) {
    return _alignas == nullptr;
  }
  if (_alignas != nullptr && *_alignas != *other._alignas) {
    return *_alignas < *other._alignas;
  }
  for (size_t i = 0; i < _attributes.size(); ++i) {
    if (_attributes[i]._ident != other._attributes[i]._ident) {
      return _attributes[i]._ident < other._attributes[i]._ident;
    }
    if (_attributes[i]._reason != other._attributes[i]._reason) {
      return _attributes[i]._reason < other._attributes[i]._reason;
    }
  }
  return false;
}

/**
 * Adds an attribute.
 */
void CPPAttributeList::
add_attribute(CPPIdentifier *ident) {
  _attributes.push_back({ident});
}

/**
 * Adds an attribute.
 */
void CPPAttributeList::
add_attribute(CPPIdentifier *ident, std::string reason) {
  _attributes.push_back({ident, std::move(reason)});
}

/**
 * Adds an alignas specifier.
 */
void CPPAttributeList::
add_alignas(int size) {
  if (_alignas == nullptr || size >= _alignas->evaluate().as_integer()) {
    _alignas = new CPPExpression(size);
  }
}

/**
 * Adds an alignas specifier.
 */
void CPPAttributeList::
add_alignas(CPPType *type) {
  CPPExpression expr = CPPExpression::alignof_func(type);
  if (_alignas == nullptr || expr.evaluate().as_integer() > _alignas->evaluate().as_integer()) {
    _alignas = new CPPExpression(expr);
  }
}

/**
 * Adds an alignas specifier.
 */
void CPPAttributeList::
add_alignas(CPPExpression *expr) {
  if (_alignas == nullptr || expr->evaluate().as_integer() > _alignas->evaluate().as_integer()) {
    _alignas = expr;
  }
}

/**
 * Merges the other list into this one.
 */
void CPPAttributeList::
add_attributes_from(const CPPAttributeList &other) {
  for (const Attribute &attr : other._attributes) {
    _attributes.push_back(attr);
  }

  if (other._alignas != nullptr) {
    add_alignas(other._alignas);
  }
}

/**
 *
 */
void CPPAttributeList::
output(std::ostream &out, CPPScope *scope) const {
  Attributes::const_iterator it = _attributes.begin();
  if (it != _attributes.end()) {
    out << "[[";
    (*it)._ident->output(out, scope);
    if (!(*it)._reason.empty()) {
      out << "(" << (*it)._reason << ")";
    }

    for (++it; it != _attributes.end(); ++it) {
      out << ", ";
      (*it)._ident->output(out, scope);
      if (!(*it)._reason.empty()) {
        out << "(" << (*it)._reason << ")";
      }
    }

    out << "]]";

    if (_alignas != nullptr) {
      out << " ";
    }
  }

  if (_alignas != nullptr) {
    out << "alignas(";
    if (_alignas->_type == CPPExpression::T_alignof) {
      _alignas->_u._typecast._to->output(out, 0, scope, false);
    } else {
      _alignas->output(out, 0, scope, false);
    }
    out << ")";
  }
}
