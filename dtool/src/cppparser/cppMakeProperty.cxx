/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppMakeProperty.cxx
 * @author rdb
 * @date 2014-09-18
 */

#include "cppMakeProperty.h"
#include "cppFunctionGroup.h"

/**
 *
 */
CPPMakeProperty::
CPPMakeProperty(CPPIdentifier *ident, Type type,
                CPPScope *current_scope, const CPPFile &file) :
  CPPDeclaration(file),
  _ident(ident),
  _type(type),
  _length_function(nullptr),
  _has_function(nullptr),
  _get_function(nullptr),
  _set_function(nullptr),
  _clear_function(nullptr),
  _del_function(nullptr),
  _insert_function(nullptr),
  _get_key_function(nullptr)
{
  _ident->_native_scope = current_scope;
}

/**
 *
 */
std::string CPPMakeProperty::
get_simple_name() const {
  return _ident->get_simple_name();
}

/**
 *
 */
std::string CPPMakeProperty::
get_local_name(CPPScope *scope) const {
  return _ident->get_local_name(scope);
}

/**
 *
 */
std::string CPPMakeProperty::
get_fully_scoped_name() const {
  return _ident->get_fully_scoped_name();
}

/**
 *
 */
void CPPMakeProperty::
output(std::ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (_length_function != nullptr) {
    out << "__make_seq_property";
  } else {
    out << "__make_property";
  }

  if (_has_function != nullptr) {
    out.put('2');
  }

  out << "(" << _ident->get_local_name(scope);

  if (_length_function != nullptr) {
    out << ", " << _length_function->_name;
  }

  if (_has_function != nullptr) {
    out << ", " << _has_function->_name;
  }

  out << ", " << _get_function->_name;

  if (_set_function != nullptr) {
    out << ", " << _set_function->_name;
  }

  if (_clear_function != nullptr) {
    out << ", " << _clear_function->_name;
  }

  if (_del_function != nullptr) {
    out << ", " << _del_function->_name;
  }

  if (_insert_function != nullptr) {
    out << ", " << _insert_function->_name;
  }

  out << ");";
}

/**
 *
 */
CPPDeclaration::SubType CPPMakeProperty::
get_subtype() const {
  return ST_make_property;
}

/**
 *
 */
CPPMakeProperty *CPPMakeProperty::
as_make_property() {
  return this;
}
