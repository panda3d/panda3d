// Filename: dcSwitchParameter.cxx
// Created by:  drose (18Jun04)
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

#include "dcSwitchParameter.h"
#include "dcSwitch.h"
#include "hashGenerator.h"

////////////////////////////////////////////////////////////////////
//     Function: DCSwitchParameter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCSwitchParameter::
DCSwitchParameter(DCSwitch *dswitch) :
  _dswitch(dswitch)
{
  set_name(dswitch->get_name());

  _has_fixed_byte_size = false;
  _has_fixed_structure = false;

  // The DCSwitch presents just one nested field initially, which is
  // the key parameter.  When we pack or unpack that, the DCPacker
  // calls apply_switch(), which returns a new record that presents
  // the remaining nested fields.
  _has_nested_fields = true;
  _num_nested_fields = 1;

  _pack_type = PT_switch;

  DCParameter *key_parameter = dswitch->get_key_parameter();
  if (key_parameter->has_fixed_byte_size()) {
    // See if we have a fixed byte size for the overall switch.  This
    // will be true only if all of the individual cases have the same
    // fixed byte size.
    int num_cases = _dswitch->get_num_cases();
    _has_fixed_byte_size = true;
    _fixed_byte_size = 0;

    if (num_cases > 0) {
      _fixed_byte_size = _dswitch->get_case(0)->get_fixed_byte_size();

      for (int i = 0; i < num_cases && _has_fixed_byte_size; i++) {
        const DCPackerInterface *dcase = _dswitch->get_case(i);
        if (!dcase->has_fixed_byte_size() || 
            dcase->get_fixed_byte_size() != _fixed_byte_size) {
          
          // Nope, we have a variable byte size.
          _has_fixed_byte_size = false;
        }
      }
    }

    _fixed_byte_size += key_parameter->get_fixed_byte_size();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitchParameter::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCSwitchParameter::
DCSwitchParameter(const DCSwitchParameter &copy) :
  DCParameter(copy),
  _dswitch(copy._dswitch)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitchParameter::as_switch_parameter
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCSwitchParameter *DCSwitchParameter::
as_switch_parameter() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitchParameter::make_copy
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCParameter *DCSwitchParameter::
make_copy() const {
  return new DCSwitchParameter(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitchParameter::is_valid
//       Access: Published, Virtual
//  Description: Returns false if the type is an invalid type
//               (e.g. declared from an undefined typedef), true if
//               it is valid.
////////////////////////////////////////////////////////////////////
bool DCSwitchParameter::
is_valid() const {
  return true; //_dswitch->is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitchParameter::get_switch
//       Access: Published
//  Description: Returns the switch object this parameter represents.
////////////////////////////////////////////////////////////////////
DCSwitch *DCSwitchParameter::
get_switch() const {
  return _dswitch;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitchParameter::get_nested_field
//       Access: Public, Virtual
//  Description: Returns the DCPackerInterface object that represents
//               the nth nested field.  This may return NULL if there
//               is no such field (but it shouldn't do this if n is in
//               the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCSwitchParameter::
get_nested_field(int) const {
  return _dswitch->get_key_parameter();
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitchParameter::apply_switch
//       Access: Public
//  Description: Returns the DCPackerInterface that presents the
//               alternative fields for the case indicated by the
//               given packed value string, or NULL if the value
//               string does not match one of the expected cases.
////////////////////////////////////////////////////////////////////
const DCPackerInterface *DCSwitchParameter::
apply_switch(const char *value_data, size_t length) const {
  return _dswitch->apply_switch(value_data, length);
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitchParameter::output_instance
//       Access: Public, Virtual
//  Description: Formats the parameter in the C++-like dc syntax as a
//               typename and identifier.
////////////////////////////////////////////////////////////////////
void DCSwitchParameter::
output_instance(ostream &out, bool brief, const string &prename, 
                const string &name, const string &postname) const {
  if (get_typedef() != (DCTypedef *)NULL) {
    output_typedef_name(out, brief, prename, name, postname);

  } else {
    _dswitch->output_instance(out, brief, prename, name, postname);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitchParameter::write_instance
//       Access: Public, Virtual
//  Description: Formats the parameter in the C++-like dc syntax as a
//               typename and identifier.
////////////////////////////////////////////////////////////////////
void DCSwitchParameter::
write_instance(ostream &out, bool brief, int indent_level,
               const string &prename, const string &name, 
               const string &postname) const {
  if (get_typedef() != (DCTypedef *)NULL) {
    write_typedef_name(out, brief, indent_level, prename, name, postname);

  } else {
    _dswitch->write_instance(out, brief, indent_level, prename, name, postname);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSwitchParameter::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this type into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCSwitchParameter::
generate_hash(HashGenerator &hashgen) const {
  DCParameter::generate_hash(hashgen);
  _dswitch->generate_hash(hashgen);
}
