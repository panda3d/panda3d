// Filename: dcSwitch.cxx
// Created by:  drose (23Jun04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "dcSwitch.h"
#include "dcField.h"
#include "dcindent.h"

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::Constructor
//       Access: Public
//  Description: The switch_parameter must be recently allocated via
//               new; it will be deleted via delete when the switch
//               destructs.
////////////////////////////////////////////////////////////////////
DCSwitch::
DCSwitch(const string &name, DCParameter *switch_parameter) :
  DCField(name),
  _switch_parameter(switch_parameter)
{
  _has_fixed_byte_size = _switch_parameter->has_fixed_byte_size();
  _fixed_byte_size = _switch_parameter->get_fixed_byte_size();
  _has_fixed_structure = false;

  // The DCSwitch presents just one nested field initially, which is
  // the switch parameter.  When we pack or unpack that, the DCPacker
  // calls apply_switch(), which returns a new record that presents
  // the remaining nested fields.
  _has_nested_fields = true;
  _num_nested_fields = 1;

  _pack_type = PT_switch;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DCSwitch::
~DCSwitch() {
  delete _switch_parameter;

  Cases::iterator ci;
  for (ci = _cases.begin(); ci != _cases.end(); ++ci) {
    const SwitchCase *dcase = (*ci);
    delete dcase;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::as_switch
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCSwitch *DCSwitch::
as_switch() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::get_switch_parameter
//       Access: Published
//  Description: Returns the key parameter on which the switch is
//               based.  The value of this parameter in the record
//               determines which one of the several cases within the
//               switch will be used.
////////////////////////////////////////////////////////////////////
DCParameter *DCSwitch::
get_switch_parameter() const {
  return _switch_parameter;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::get_num_cases
//       Access: Published
//  Description: Returns the number of different cases within the
//               switch.  The legal values for case_index range from 0
//               to get_num_cases() - 1.
////////////////////////////////////////////////////////////////////
int DCSwitch::
get_num_cases() const {
  return _cases.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::get_case_by_value
//       Access: Published
//  Description: Returns the index number of the case with the
//               indicated packed value, or -1 if no case has this
//               value.
////////////////////////////////////////////////////////////////////
int DCSwitch::
get_case_by_value(const string &case_value) const {
  CasesByValue::const_iterator vi;
  vi = _cases_by_value.find(case_value);
  if (vi != _cases_by_value.end()) {
    return (*vi).second;
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::get_value
//       Access: Published
//  Description: Returns the packed value associated with the
//               indicated case.
////////////////////////////////////////////////////////////////////
string DCSwitch::
get_value(int case_index) const {
  nassertr(case_index >= 0 && case_index < (int)_cases.size(), string());
  return _cases[case_index]->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::get_num_fields
//       Access: Published
//  Description: Returns the number of fields in the indicated case.
////////////////////////////////////////////////////////////////////
int DCSwitch::
get_num_fields(int case_index) const {
  nassertr(case_index >= 0 && case_index < (int)_cases.size(), 0);
  return _cases[case_index]->_fields.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::get_num_fields
//       Access: Published
//  Description: Returns the nth field in the indicated case.
////////////////////////////////////////////////////////////////////
DCField *DCSwitch::
get_field(int case_index, int n) const {
  nassertr(case_index >= 0 && case_index < (int)_cases.size(), NULL);
  nassertr(n >= 0 && n < (int)_cases[case_index]->_fields.size(), NULL);
  return _cases[case_index]->_fields[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::get_field_by_name
//       Access: Published
//  Description: Returns the field with the given name from the
//               indicated case, or NULL if no field has this name.
////////////////////////////////////////////////////////////////////
DCField *DCSwitch::
get_field_by_name(int case_index, const string &name) const {
  nassertr(case_index >= 0 && case_index < (int)_cases.size(), NULL);

  const FieldsByName &fields_by_name = _cases[case_index]->_fields_by_name;
  FieldsByName::const_iterator ni;
  ni = fields_by_name.find(name);
  if (ni != fields_by_name.end()) {
    return (*ni).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::get_nested_field
//       Access: Public, Virtual
//  Description: Returns the DCPackerInterface object that represents
//               the nth nested field.  This may return NULL if there
//               is no such field (but it shouldn't do this if n is in
//               the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCSwitch::
get_nested_field(int) const {
  // The DCSwitch presents just one nested field initially, which is
  // the switch parameter.  When we pack or unpack that, the DCPacker
  // calls apply_switch(), which returns a new record that presents
  // the remaining nested fields.
  return _switch_parameter;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::add_case
//       Access: Public
//  Description: Adds a new case to the switch with the indicated
//               value, and returns the new case_index.  If the value
//               has already been used for another case, returns -1.
//               This is normally called only by the parser.
////////////////////////////////////////////////////////////////////
int DCSwitch::
add_case(const string &value) {
  int case_index = (int)_cases.size();
  if (!_cases_by_value.insert(CasesByValue::value_type(value, case_index)).second) {
    return -1;
  }

  SwitchCase *dcase = new SwitchCase(_name, value);
  dcase->add_field(_switch_parameter);
  _cases.push_back(dcase);
  return case_index;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::add_field
//       Access: Public
//  Description: Adds a field to the case most recently added via
//               add_case().  Returns true if successful, false if the
//               field duplicates a field already named within this
//               case.  It is an error to call this before calling
//               add_case().  This is normally called only by the
//               parser.
////////////////////////////////////////////////////////////////////
bool DCSwitch::
add_field(DCField *field) {
  nassertr(!_cases.empty(), false);

  if (!_cases.back()->add_field(field)) {
    return false;
  }

  // See if we now have a fixed byte size for the overall switch.
  // This will be true only if all of the individual cases have the
  // same fixed byte size.
  _fixed_byte_size = _cases.back()->get_fixed_byte_size();

  Cases::const_iterator ci;
  for (ci = _cases.begin(); ci != _cases.end(); ++ci) {
    const SwitchCase *dcase = (*ci);
    if (!dcase->has_fixed_byte_size() || 
        dcase->get_fixed_byte_size() != _fixed_byte_size) {

      // Nope, we have a variable byte size.
      _has_fixed_byte_size = false;
      return true;
    }
  }

  // Sonofagun, we do have a fixed byte size.
  _has_fixed_byte_size = true;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::apply_switch
//       Access: Public
//  Description: Returns the DCPackerInterface that presents the
//               alternative fields for the case indicated by the
//               given packed value string, or NULL if the value
//               string does not match one of the expected cases.
////////////////////////////////////////////////////////////////////
const DCPackerInterface *DCSwitch::
apply_switch(const char *value_data, size_t length) const {
  CasesByValue::const_iterator vi;
  vi = _cases_by_value.find(string(value_data, length));
  if (vi != _cases_by_value.end()) {
    return _cases[(*vi).second];
  }

  // Invalid value.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::write
//       Access: Public, Virtual
//  Description: Generates a parseable description of the object to
//               the indicated output stream.
////////////////////////////////////////////////////////////////////
void DCSwitch::
write(ostream &out, bool brief, int indent_level) const {
  indent(out, indent_level)
    << "switch";
  if (!_name.empty()) {
    out << " " << _name;
  }
  out << " (";
  _switch_parameter->output(out, brief);
  out << ") {\n";

  Cases::const_iterator ci;
  for (ci = _cases.begin(); ci != _cases.end(); ++ci) {
    const SwitchCase *dcase = (*ci);
    indent(out, indent_level)
      << "case " << _switch_parameter->format_data(dcase->_value) << ":\n";
    
    Fields::const_iterator fi;
    if (!dcase->_fields.empty()) {
      fi = dcase->_fields.begin();
      ++fi;
      while (fi != dcase->_fields.end()) {
        (*fi)->write(out, brief, indent_level + 2);
        ++fi;
      }
    }
  }
  indent(out, indent_level)
    << "};\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this switch into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCSwitch::
generate_hash(HashGenerator &hashgen) const {
  DCField::generate_hash(hashgen);

  _switch_parameter->generate_hash(hashgen);

  hashgen.add_int(_cases.size());
  Cases::const_iterator ci;
  for (ci = _cases.begin(); ci != _cases.end(); ++ci) {
    const SwitchCase *dcase = (*ci);
    hashgen.add_string(dcase->_value);

    hashgen.add_int(dcase->_fields.size());
    Fields::const_iterator fi;
    for (fi = dcase->_fields.begin(); fi != dcase->_fields.end(); ++fi) {
      (*fi)->generate_hash(hashgen);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::SwitchCase::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCSwitch::SwitchCase::
SwitchCase(const string &name, const string &value) :
  DCPackerInterface(name),
  _value(value)
{
  _has_nested_fields = true;
  _num_nested_fields = 0;
  _pack_type = PT_switch;

  _has_fixed_byte_size = true;
  _fixed_byte_size = 0;
  _has_fixed_structure = true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::SwitchCase::get_nested_field
//       Access: Public, Virtual
//  Description: Returns the DCPackerInterface object that represents
//               the nth nested field.  This may return NULL if there
//               is no such field (but it shouldn't do this if n is in
//               the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCSwitch::SwitchCase::
get_nested_field(int n) const {
  nassertr(n >= 0 && n < (int)_fields.size(), NULL);
  return _fields[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitch::SwitchCase::add_field
//       Access: Public
//  Description: Adds a field to this case.  Returns true if
//               successful, false if the field duplicates a field
//               already named within this case.  This is normally
//               called only by the parser.
////////////////////////////////////////////////////////////////////
bool DCSwitch::SwitchCase::
add_field(DCField *field) {
  bool inserted = _fields_by_name.insert
    (FieldsByName::value_type(field->get_name(), field)).second;

  if (!inserted) {
    return false;
  }

  _fields.push_back(field);

  _num_nested_fields = (int)_fields.size();

  // See if we still have a fixed byte size.
  if (_has_fixed_byte_size) {
    _has_fixed_byte_size = field->has_fixed_byte_size();
    _fixed_byte_size += field->get_fixed_byte_size();
  }
  if (_has_fixed_structure) {
    _has_fixed_structure = field->has_fixed_structure();
  }
  return true;
}
