/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcSwitchParameter.cxx
 * @author drose
 * @date 2004-06-18
 */

#include "dcSwitchParameter.h"
#include "dcSwitch.h"
#include "hashGenerator.h"

using std::string;

/**
 *
 */
DCSwitchParameter::
DCSwitchParameter(const DCSwitch *dswitch) :
  _dswitch(dswitch)
{
  set_name(dswitch->get_name());

  _has_fixed_byte_size = true;
  _fixed_byte_size = 0;
  _has_fixed_structure = false;

  // The DCSwitch presents just one nested field initially, which is the key
  // parameter.  When we pack or unpack that, the DCPacker calls
  // apply_switch(), which returns a new record that presents the remaining
  // nested fields.
  _has_nested_fields = true;
  _num_nested_fields = 1;

  _pack_type = PT_switch;

  DCField *key_parameter = dswitch->get_key_parameter();
  _has_fixed_byte_size = _has_fixed_byte_size && key_parameter->has_fixed_byte_size();
  _has_range_limits = _has_range_limits || key_parameter->has_range_limits();
  _has_default_value = _has_default_value || key_parameter->has_default_value();

  int num_cases = _dswitch->get_num_cases();
  if (num_cases > 0) {
    _fixed_byte_size = _dswitch->get_case(0)->get_fixed_byte_size();

    // Consider each case for fixed size, etc.
    for (int i = 0; i < num_cases; i++) {
      const DCSwitch::SwitchFields *fields =
        (const DCSwitch::SwitchFields *)_dswitch->get_case(i);

      if (!fields->has_fixed_byte_size() ||
          fields->get_fixed_byte_size() != _fixed_byte_size) {

        // Nope, we have a variable byte size.
        _has_fixed_byte_size = false;
      }

      _has_range_limits = _has_range_limits || fields->has_range_limits();
      _has_default_value = _has_default_value || fields->_has_default_value;
    }
  }

  // Also consider the default case, if there is one.
  const DCSwitch::SwitchFields *fields =
    (DCSwitch::SwitchFields *)_dswitch->get_default_case();
  if (fields != nullptr) {
    if (!fields->has_fixed_byte_size() ||
        fields->get_fixed_byte_size() != _fixed_byte_size) {
      _has_fixed_byte_size = false;
    }

    _has_range_limits = _has_range_limits || fields->has_range_limits();
    _has_default_value = _has_default_value || fields->_has_default_value;
  }
}

/**
 *
 */
DCSwitchParameter::
DCSwitchParameter(const DCSwitchParameter &copy) :
  DCParameter(copy),
  _dswitch(copy._dswitch)
{
}

/**
 *
 */
DCSwitchParameter *DCSwitchParameter::
as_switch_parameter() {
  return this;
}

/**
 *
 */
const DCSwitchParameter *DCSwitchParameter::
as_switch_parameter() const {
  return this;
}

/**
 *
 */
DCParameter *DCSwitchParameter::
make_copy() const {
  return new DCSwitchParameter(*this);
}

/**
 * Returns false if the type is an invalid type (e.g.  declared from an
 * undefined typedef), true if it is valid.
 */
bool DCSwitchParameter::
is_valid() const {
  return true; //_dswitch->is_valid();
}

/**
 * Returns the switch object this parameter represents.
 */
const DCSwitch *DCSwitchParameter::
get_switch() const {
  return _dswitch;
}

/**
 * Returns the DCPackerInterface object that represents the nth nested field.
 * This may return NULL if there is no such field (but it shouldn't do this if
 * n is in the range 0 <= n < get_num_nested_fields()).
 */
DCPackerInterface *DCSwitchParameter::
get_nested_field(int) const {
  return _dswitch->get_key_parameter();
}

/**
 * Returns the DCPackerInterface that presents the alternative fields for the
 * case indicated by the given packed value string, or NULL if the value
 * string does not match one of the expected cases.
 */
const DCPackerInterface *DCSwitchParameter::
apply_switch(const char *value_data, size_t length) const {
  return _dswitch->apply_switch(value_data, length);
}

/**
 * Formats the parameter in the C++-like dc syntax as a typename and
 * identifier.
 */
void DCSwitchParameter::
output_instance(std::ostream &out, bool brief, const string &prename,
                const string &name, const string &postname) const {
  if (get_typedef() != nullptr) {
    output_typedef_name(out, brief, prename, name, postname);

  } else {
    _dswitch->output_instance(out, brief, prename, name, postname);
  }
}

/**
 * Formats the parameter in the C++-like dc syntax as a typename and
 * identifier.
 */
void DCSwitchParameter::
write_instance(std::ostream &out, bool brief, int indent_level,
               const string &prename, const string &name,
               const string &postname) const {
  if (get_typedef() != nullptr) {
    write_typedef_name(out, brief, indent_level, prename, name, postname);

  } else {
    _dswitch->write_instance(out, brief, indent_level, prename, name, postname);
  }
}

/**
 * Accumulates the properties of this type into the hash.
 */
void DCSwitchParameter::
generate_hash(HashGenerator &hashgen) const {
  DCParameter::generate_hash(hashgen);
  _dswitch->generate_hash(hashgen);
}

/**
 * Packs the switchParameter's specified default value (or a sensible default
 * if no value is specified) into the stream.  Returns true if the default
 * value is packed, false if the switchParameter doesn't know how to pack its
 * default value.
 */
bool DCSwitchParameter::
pack_default_value(DCPackData &pack_data, bool &pack_error) const {
  if (has_default_value()) {
    return DCField::pack_default_value(pack_data, pack_error);
  }

  return _dswitch->pack_default_value(pack_data, pack_error);
}

/**
 * Returns true if the other interface is bitwise the same as this one--that
 * is, a uint32 only matches a uint32, etc.  Names of components, and range
 * limits, are not compared.
 */
bool DCSwitchParameter::
do_check_match(const DCPackerInterface *other) const {
  return other->do_check_match_switch_parameter(this);
}

/**
 * Returns true if this field matches the indicated switch parameter, false
 * otherwise.
 */
bool DCSwitchParameter::
do_check_match_switch_parameter(const DCSwitchParameter *other) const {
  return _dswitch->do_check_match_switch(other->_dswitch);
}
