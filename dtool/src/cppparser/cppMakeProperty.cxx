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
CPPMakeProperty(CPPIdentifier *ident,
                CPPFunctionGroup *getter, CPPFunctionGroup *setter,
                CPPScope *current_scope, const CPPFile &file) :
  CPPDeclaration(file),
  _ident(ident),
  _length_function(NULL),
  _has_function(NULL),
  _get_function(getter),
  _set_function(setter),
  _clear_function(NULL),
  _del_function(NULL)
{
  _ident->_native_scope = current_scope;
}

/**
 *
 */
CPPMakeProperty::
CPPMakeProperty(CPPIdentifier *ident,
                CPPFunctionGroup *hasser, CPPFunctionGroup *getter,
                CPPFunctionGroup *setter, CPPFunctionGroup *clearer,
                CPPScope *current_scope, const CPPFile &file) :
  CPPDeclaration(file),
  _ident(ident),
  _length_function(NULL),
  _has_function(hasser),
  _get_function(getter),
  _set_function(setter),
  _clear_function(clearer),
  _del_function(NULL)
{
  _ident->_native_scope = current_scope;
}

/**
 *
 */
string CPPMakeProperty::
get_simple_name() const {
  return _ident->get_simple_name();
}

/**
 *
 */
string CPPMakeProperty::
get_local_name(CPPScope *scope) const {
  return _ident->get_local_name(scope);
}

/**
 *
 */
string CPPMakeProperty::
get_fully_scoped_name() const {
  return _ident->get_fully_scoped_name();
}

/**
 *
 */
void CPPMakeProperty::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (_length_function != NULL) {
    out << "__make_seq_property";
  } else {
    out << "__make_property";
  }

  if (_has_function != NULL) {
    out.put('2');
  }

  out << "(" << _ident->get_local_name(scope);

  if (_length_function != NULL) {
    out << ", " << _length_function->_name;
  }

  if (_has_function != NULL) {
    out << ", " << _has_function->_name;
  }

  out << ", " << _get_function->_name;

  if (_set_function != NULL) {
    out << ", " << _set_function->_name;
  }

  if (_clear_function != NULL) {
    out << ", " << _clear_function->_name;
  }

  if (_del_function != NULL) {
    out << ", " << _del_function->_name;
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
