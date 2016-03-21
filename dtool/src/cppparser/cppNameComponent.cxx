/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppNameComponent.cxx
 * @author drose
 * @date 1999-11-12
 */

#include "cppNameComponent.h"
#include "cppTemplateParameterList.h"

/**
 *
 */
CPPNameComponent::
CPPNameComponent(const string &name) :
  _name(name)
{
  _templ = (CPPTemplateParameterList *)NULL;
}

/**
 *
 */
bool CPPNameComponent::
operator == (const CPPNameComponent &other) const {
  if (_name != other._name) {
    return false;
  }
  if (_templ == NULL && other._templ == NULL) {
    return true;
  }
  if (_templ == NULL || other._templ == NULL) {
    return false;
  }
  if (*_templ != *other._templ) {
    return false;
  }

  return true;
}

/**
 *
 */
bool CPPNameComponent::
operator != (const CPPNameComponent &other) const {
  return !(*this == other);
}

/**
 *
 */
bool CPPNameComponent::
operator < (const CPPNameComponent &other) const {
  if (_name != other._name) {
    return _name < other._name;
  }
  if (_templ == NULL && other._templ == NULL) {
    return false;
  }
  if (_templ == NULL || other._templ == NULL) {
    return _templ < other._templ;
  }
  return (*_templ) < (*other._templ);
}

/**
 *
 */
string CPPNameComponent::
get_name() const {
  return _name;
}

/**
 *
 */
string CPPNameComponent::
get_name_with_templ(CPPScope *scope) const {
  ostringstream strm;
  strm << _name;
  if (_templ != NULL) {
    strm << "< ";
    _templ->output(strm, scope);
    strm << " >";
  }
  return strm.str();
}

/**
 *
 */
CPPTemplateParameterList *CPPNameComponent::
get_templ() const {
  return _templ;
}

/**
 *
 */
bool CPPNameComponent::
empty() const {
  return _name.empty();
}

/**
 *
 */
bool CPPNameComponent::
has_templ() const {
  return _templ != (CPPTemplateParameterList *)NULL;
}

/**
 * Returns true if the name component includes a template parameter list that
 * includes some not-yet-defined type.
 */
bool CPPNameComponent::
is_tbd() const {
  if (_templ != (CPPTemplateParameterList *)NULL) {
    return _templ->is_tbd();
  }
  return false;
}

/**
 *
 */
void CPPNameComponent::
set_name(const string &name) {
  _name = name;
}

/**
 *
 */
void CPPNameComponent::
append_name(const string &name) {
  _name += name;
}

/**
 *
 */
void CPPNameComponent::
set_templ(CPPTemplateParameterList *templ) {
  _templ = templ;
}

/**
 *
 */
void CPPNameComponent::
output(ostream &out) const {
  out << _name;
  if (_templ != NULL) {
    out << "< " << *_templ << " >";
  }
}
