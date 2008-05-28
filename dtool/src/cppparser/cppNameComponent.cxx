// Filename: cppNameComponent.cxx
// Created by:  drose (12Nov99)
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


#include "cppNameComponent.h"
#include "cppTemplateParameterList.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPNameComponent::
CPPNameComponent(const string &name) :
  _name(name)
{
  _templ = (CPPTemplateParameterList *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::Equivalence Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::Nonequivalence Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPNameComponent::
operator != (const CPPNameComponent &other) const {
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
string CPPNameComponent::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::get_name_with_templ
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::get_templ
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPTemplateParameterList *CPPNameComponent::
get_templ() const {
  return _templ;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPNameComponent::
empty() const {
  return _name.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::has_templ
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPNameComponent::
has_templ() const {
  return _templ != (CPPTemplateParameterList *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::is_tbd
//       Access: Public
//  Description: Returns true if the name component includes a
//               template parameter list that includes some
//               not-yet-defined type.
////////////////////////////////////////////////////////////////////
bool CPPNameComponent::
is_tbd() const {
  if (_templ != (CPPTemplateParameterList *)NULL) {
    return _templ->is_tbd();
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::set_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPNameComponent::
set_name(const string &name) {
  _name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::append_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPNameComponent::
append_name(const string &name) {
  _name += name;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::set_templ
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPNameComponent::
set_templ(CPPTemplateParameterList *templ) {
  _templ = templ;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNameComponent::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPNameComponent::
output(ostream &out) const {
  out << _name;
  if (_templ != NULL) {
    out << "< " << *_templ << " >";
  }
}
