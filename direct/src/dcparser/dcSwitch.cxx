/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcSwitch.cxx
 * @author drose
 * @date 2004-06-23
 */

#include "dcSwitch.h"
#include "dcField.h"
#include "dcParameter.h"
#include "hashGenerator.h"
#include "dcindent.h"
#include "dcPacker.h"

using std::ostream;
using std::string;

/**
 * The key_parameter must be recently allocated via new; it will be deleted
 * via delete when the switch destructs.
 */
DCSwitch::
DCSwitch(const string &name, DCField *key_parameter) :
  _name(name),
  _key_parameter(key_parameter)
{
  _default_case = nullptr;
  _fields_added = false;
}

/**
 *
 */
DCSwitch::
~DCSwitch() {
  nassertv(_key_parameter != nullptr);
  delete _key_parameter;

  Cases::iterator ci;
  for (ci = _cases.begin(); ci != _cases.end(); ++ci) {
    SwitchCase *dcase = (*ci);
    delete dcase;
  }

  CaseFields::iterator fi;
  for (fi = _case_fields.begin(); fi != _case_fields.end(); ++fi) {
    SwitchFields *fields = (*fi);
    delete fields;
  }

  Fields::iterator ni;
  for (ni = _nested_fields.begin(); ni != _nested_fields.end(); ++ni) {
    DCField *field = (*ni);
    delete field;
  }
}

/**
 *
 */
DCSwitch *DCSwitch::
as_switch() {
  return this;
}

/**
 *
 */
const DCSwitch *DCSwitch::
as_switch() const {
  return this;
}

/**
 * Returns the name of this switch.
 */
const string &DCSwitch::
get_name() const {
  return _name;
}

/**
 * Returns the key parameter on which the switch is based.  The value of this
 * parameter in the record determines which one of the several cases within
 * the switch will be used.
 */
DCField *DCSwitch::
get_key_parameter() const {
  return _key_parameter;
}

/**
 * Returns the number of different cases within the switch.  The legal values
 * for case_index range from 0 to get_num_cases() - 1.
 */
int DCSwitch::
get_num_cases() const {
  return _cases.size();
}

/**
 * Returns the index number of the case with the indicated packed value, or -1
 * if no case has this value.
 */
int DCSwitch::
get_case_by_value(const vector_uchar &case_value) const {
  CasesByValue::const_iterator vi;
  vi = _cases_by_value.find(case_value);
  if (vi != _cases_by_value.end()) {
    return (*vi).second;
  }

  return -1;
}

/**
 * Returns the DCPackerInterface that packs the nth case.
 */
DCPackerInterface *DCSwitch::
get_case(int n) const {
  nassertr(n >= 0 && n < (int)_cases.size(), nullptr);
  return _cases[n]->_fields;
}

/**
 * Returns the DCPackerInterface that packs the default case, or NULL if there
 * is no default case.
 */
DCPackerInterface *DCSwitch::
get_default_case() const {
  return _default_case;
}

/**
 * Returns the packed value associated with the indicated case.
 */
vector_uchar DCSwitch::
get_value(int case_index) const {
  nassertr(case_index >= 0 && case_index < (int)_cases.size(), vector_uchar());
  return _cases[case_index]->_value;
}

/**
 * Returns the number of fields in the indicated case.
 */
int DCSwitch::
get_num_fields(int case_index) const {
  nassertr(case_index >= 0 && case_index < (int)_cases.size(), 0);
  return _cases[case_index]->_fields->_fields.size();
}

/**
 * Returns the nth field in the indicated case.
 */
DCField *DCSwitch::
get_field(int case_index, int n) const {
  nassertr(case_index >= 0 && case_index < (int)_cases.size(), nullptr);
  nassertr(n >= 0 && n < (int)_cases[case_index]->_fields->_fields.size(), nullptr);
  return _cases[case_index]->_fields->_fields[n];
}

/**
 * Returns the field with the given name from the indicated case, or NULL if
 * no field has this name.
 */
DCField *DCSwitch::
get_field_by_name(int case_index, const string &name) const {
  nassertr(case_index >= 0 && case_index < (int)_cases.size(), nullptr);

  const FieldsByName &fields_by_name = _cases[case_index]->_fields->_fields_by_name;
  FieldsByName::const_iterator ni;
  ni = fields_by_name.find(name);
  if (ni != fields_by_name.end()) {
    return (*ni).second;
  }

  return nullptr;
}

/**
 * Returns true if it is valid to add a new field at this point (implying that
 * a case or default has been added already), or false if not.
 */
bool DCSwitch::
is_field_valid() const {
  return !_current_fields.empty();
}

/**
 * Adds a new case to the switch with the indicated value, and returns the new
 * case_index.  If the value has already been used for another case, returns
 * -1. This is normally called only by the parser.
 */
int DCSwitch::
add_case(const vector_uchar &value) {
  int case_index = (int)_cases.size();
  if (!_cases_by_value.insert(CasesByValue::value_type(value, case_index)).second) {
    add_invalid_case();
    return -1;
  }

  SwitchFields *fields = start_new_case();
  SwitchCase *dcase = new SwitchCase(value, fields);
  _cases.push_back(dcase);
  return case_index;
}

/**
 * Adds a new case to the switch that will never be matched.  This is only
 * used by the parser, to handle an error condition more gracefully without
 * bitching the parsing (which behaves differently according to whether a case
 * has been encountered or not).
 */
void DCSwitch::
add_invalid_case() {
  start_new_case();
}

/**
 * Adds a default case to the switch.  Returns true if the case is
 * successfully added, or false if it had already been added.  This is
 * normally called only by the parser.
 */
bool DCSwitch::
add_default() {
  if (_default_case != nullptr) {
    add_invalid_case();
    return false;
  }

  SwitchFields *fields = start_new_case();
  _default_case = fields;
  return true;
}

/**
 * Adds a field to the currently active cases (those that have been added via
 * add_case() or add_default(), since the last call to add_break()).  Returns
 * true if successful, false if the field duplicates a field already named
 * within this case.  It is an error to call this before calling add_case() or
 * add_default(). This is normally called only by the parser.
 */
bool DCSwitch::
add_field(DCField *field) {
  nassertr(!_current_fields.empty(), false);

  bool all_ok = true;

  CaseFields::iterator fi;
  for (fi = _current_fields.begin(); fi != _current_fields.end(); ++fi) {
    SwitchFields *fields = (*fi);
    if (!fields->add_field(field)) {
      all_ok = false;
    }
  }
  _nested_fields.push_back(field);
  _fields_added = true;

  return all_ok;
}

/**
 * Adds a break statement to the switch.  This closes the currently open cases
 * and prepares for a new, unrelated case.
 */
void DCSwitch::
add_break() {
  _current_fields.clear();
  _fields_added = false;
}

/**
 * Returns the DCPackerInterface that presents the alternative fields for the
 * case indicated by the given packed value string, or NULL if the value
 * string does not match one of the expected cases.
 */
const DCPackerInterface *DCSwitch::
apply_switch(const char *value_data, size_t length) const {
  CasesByValue::const_iterator vi;
  vi = _cases_by_value.find(vector_uchar((const unsigned char *)value_data,
                                         (const unsigned char *)value_data + length));
  if (vi != _cases_by_value.end()) {
    return _cases[(*vi).second]->_fields;
  }

  // Unexpected value--use the default.
  if (_default_case != nullptr) {
    return _default_case;
  }

  // No default.
  return nullptr;
}

/**
 * Write a string representation of this instance to <out>.
 */
void DCSwitch::
output(ostream &out, bool brief) const {
  output_instance(out, brief, "", "", "");
}

/**
 * Generates a parseable description of the object to the indicated output
 * stream.
 */
void DCSwitch::
write(ostream &out, bool brief, int indent_level) const {
  write_instance(out, brief, indent_level, "", "", "");
}

/**
 * Generates a parseable description of the object to the indicated output
 * stream.
 */
void DCSwitch::
output_instance(ostream &out, bool brief, const string &prename,
                const string &name, const string &postname) const {
  out << "switch";
  if (!_name.empty()) {
    out << " " << _name;
  }
  out << " (";
  _key_parameter->output(out, brief);
  out << ") {";

  const SwitchFields *last_fields = nullptr;

  Cases::const_iterator ci;
  for (ci = _cases.begin(); ci != _cases.end(); ++ci) {
    const SwitchCase *dcase = (*ci);
    if (dcase->_fields != last_fields && last_fields != nullptr) {
      last_fields->output(out, brief);
    }
    last_fields = dcase->_fields;
    out << "case " << _key_parameter->format_data(dcase->_value, false) << ": ";
  }

  if (_default_case != nullptr) {
    if (_default_case != last_fields && last_fields != nullptr) {
      last_fields->output(out, brief);
    }
    last_fields = _default_case;
    out << "default: ";
  }
  if (last_fields != nullptr) {
    last_fields->output(out, brief);
  }

  out << "}";
  if (!prename.empty() || !name.empty() || !postname.empty()) {
    out << " " << prename << name << postname;
  }
}

/**
 * Generates a parseable description of the object to the indicated output
 * stream.
 */
void DCSwitch::
write_instance(ostream &out, bool brief, int indent_level,
               const string &prename, const string &name,
               const string &postname) const {
  indent(out, indent_level)
    << "switch";
  if (!_name.empty()) {
    out << " " << _name;
  }
  out << " (";
  _key_parameter->output(out, brief);
  out << ") {\n";

  const SwitchFields *last_fields = nullptr;

  Cases::const_iterator ci;
  for (ci = _cases.begin(); ci != _cases.end(); ++ci) {
    const SwitchCase *dcase = (*ci);
    if (dcase->_fields != last_fields && last_fields != nullptr) {
      last_fields->write(out, brief, indent_level + 2);
    }
    last_fields = dcase->_fields;
    indent(out, indent_level)
      << "case " << _key_parameter->format_data(dcase->_value, false) << ":\n";
  }

  if (_default_case != nullptr) {
    if (_default_case != last_fields && last_fields != nullptr) {
      last_fields->write(out, brief, indent_level + 2);
    }
    last_fields = _default_case;
    indent(out, indent_level)
      << "default:\n";
  }
  if (last_fields != nullptr) {
    last_fields->write(out, brief, indent_level + 2);
  }

  indent(out, indent_level)
    << "}";
  if (!prename.empty() || !name.empty() || !postname.empty()) {
    out << " " << prename << name << postname;
  }
  out << ";\n";
}

/**
 * Accumulates the properties of this switch into the hash.
 */
void DCSwitch::
generate_hash(HashGenerator &hashgen) const {
  hashgen.add_string(_name);

  _key_parameter->generate_hash(hashgen);

  hashgen.add_int(_cases.size());
  Cases::const_iterator ci;
  for (ci = _cases.begin(); ci != _cases.end(); ++ci) {
    const SwitchCase *dcase = (*ci);
    hashgen.add_blob(dcase->_value);

    const SwitchFields *fields = dcase->_fields;
    hashgen.add_int(fields->_fields.size());
    Fields::const_iterator fi;
    for (fi = fields->_fields.begin(); fi != fields->_fields.end(); ++fi) {
      (*fi)->generate_hash(hashgen);
    }
  }

  if (_default_case != nullptr) {
    const SwitchFields *fields = _default_case;
    hashgen.add_int(fields->_fields.size());
    Fields::const_iterator fi;
    for (fi = fields->_fields.begin(); fi != fields->_fields.end(); ++fi) {
      (*fi)->generate_hash(hashgen);
    }
  }
}

/**
 * Packs the switchParameter's specified default value (or a sensible default
 * if no value is specified) into the stream.  Returns true if the default
 * value is packed, false if the switchParameter doesn't know how to pack its
 * default value.
 */
bool DCSwitch::
pack_default_value(DCPackData &pack_data, bool &pack_error) const {
  SwitchFields *fields = nullptr;
  DCPacker packer;
  packer.begin_pack(_key_parameter);
  if (!_cases.empty()) {
    // If we have any cases, the first case is always the default case,
    // regardless of the default value specified by the key parameter.  That's
    // just the easiest to code.
    packer.pack_literal_value(_cases[0]->_value);
    fields = _cases[0]->_fields;

  } else {
    // If we don't have any cases, just pack the key parameter's default.
    packer.pack_default_value();
    fields = _default_case;
  }

  if (!packer.end_pack()) {
    pack_error = true;
  }

  if (fields == nullptr) {
    pack_error = true;

  } else {
    // Then everything within the case gets its normal default.
    for (size_t i = 1; i < fields->_fields.size(); i++) {
      packer.begin_pack(fields->_fields[i]);
      packer.pack_default_value();
      if (!packer.end_pack()) {
        pack_error = true;
      }
    }
  }

  pack_data.append_data(packer.get_data(), packer.get_length());

  return true;
}

/**
 * Returns true if this switch matches the indicated other switch--that is,
 * the two switches are bitwise equivalent--false otherwise.  This is only
 * intended to be called internally from
 * DCSwitchParameter::do_check_match_switch_parameter().
 */
bool DCSwitch::
do_check_match_switch(const DCSwitch *other) const {
  if (!_key_parameter->check_match(other->_key_parameter)) {
    return false;
  }

  if (_cases.size() != other->_cases.size()) {
    return false;
  }

  Cases::const_iterator ci;
  for (ci = _cases.begin(); ci != _cases.end(); ++ci) {
    const SwitchCase *c1 = (*ci);
    CasesByValue::const_iterator vi;
    vi = other->_cases_by_value.find(c1->_value);
    if (vi == other->_cases_by_value.end()) {
      // No matching value.
      return false;
    }
    int c2_index = (*vi).second;
    nassertr(c2_index >= 0 && c2_index < (int)other->_cases.size(), false);
    const SwitchCase *c2 = other->_cases[c2_index];

    if (!c1->do_check_match_switch_case(c2)) {
      return false;
    }
  }

  return true;
}

/**
 * Creates a new field set for the new case, or shares the field set with the
 * previous case, as appropriate.  Returns the appropriate field set.
 */
DCSwitch::SwitchFields *DCSwitch::
start_new_case() {
  SwitchFields *fields = nullptr;

  if (_current_fields.empty() || _fields_added) {
    // If we have recently encountered a break (which removes all of the
    // current field sets) or if we have already added at least one field to
    // the previous case without an intervening break, then we can't share the
    // field set with the previous case.  Create a new one.
    fields = new SwitchFields(_name);
    fields->add_field(_key_parameter);

    _case_fields.push_back(fields);
    _current_fields.push_back(fields);

  } else {
    // Otherwise, we can share the field set with the previous case.
    fields = _current_fields.back();
  }

  _fields_added = false;

  return fields;
}


/**
 *
 */
DCSwitch::SwitchFields::
SwitchFields(const string &name) :
  DCPackerInterface(name)
{
  _has_nested_fields = true;
  _num_nested_fields = 0;
  _pack_type = PT_switch;

  _has_fixed_byte_size = true;
  _fixed_byte_size = 0;
  _has_fixed_structure = true;
  _has_range_limits = false;
  _has_default_value = false;
}

/**
 *
 */
DCSwitch::SwitchFields::
~SwitchFields() {
  // We don't delete any of the nested fields here, since they might be shared
  // by multiple SwitchFields objects.  Instead, we delete them in the
  // DCSwitch destructor.
}

/**
 * Returns the DCPackerInterface object that represents the nth nested field.
 * This may return NULL if there is no such field (but it shouldn't do this if
 * n is in the range 0 <= n < get_num_nested_fields()).
 */
DCPackerInterface *DCSwitch::SwitchFields::
get_nested_field(int n) const {
  nassertr(n >= 0 && n < (int)_fields.size(), nullptr);
  return _fields[n];
}

/**
 * Adds a field to this case.  Returns true if successful, false if the field
 * duplicates a field already named within this case.  This is normally called
 * only by the parser.
 */
bool DCSwitch::SwitchFields::
add_field(DCField *field) {
  if (!field->get_name().empty()) {
    bool inserted = _fields_by_name.insert
      (FieldsByName::value_type(field->get_name(), field)).second;

    if (!inserted) {
      return false;
    }
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
  if (!_has_range_limits) {
    _has_range_limits = field->has_range_limits();
  }
  if (!_has_default_value) {
    _has_default_value = field->has_default_value();
  }
  return true;
}

/**
 * Returns true if this case matches the indicated case, false otherwise.
 * This is only intended to be called internally from
 * DCSwitch::do_check_match_switch().
 */
bool DCSwitch::SwitchFields::
do_check_match_switch_case(const DCSwitch::SwitchFields *other) const {
  if (_fields.size() != other->_fields.size()) {
    return false;
  }
  for (size_t i = 0; i < _fields.size(); i++) {
    if (!_fields[i]->check_match(other->_fields[i])) {
      return false;
    }
  }

  return true;
}

/**
 *
 */
void DCSwitch::SwitchFields::
output(ostream &out, bool brief) const {
  Fields::const_iterator fi;
  if (!_fields.empty()) {
    fi = _fields.begin();
    ++fi;
    while (fi != _fields.end()) {
      (*fi)->output(out, brief);
      out << "; ";
      ++fi;
    }
  }
  out << "break; ";
}

/**
 *
 */
void DCSwitch::SwitchFields::
write(ostream &out, bool brief, int indent_level) const {
  Fields::const_iterator fi;
  if (!_fields.empty()) {
    fi = _fields.begin();
    ++fi;
    while (fi != _fields.end()) {
      (*fi)->write(out, brief, indent_level);
      ++fi;
    }
  }
  indent(out, indent_level)
    << "break;\n";
}

/**
 * Returns true if the other interface is bitwise the same as this one--that
 * is, a uint32 only matches a uint32, etc.  Names of components, and range
 * limits, are not compared.
 */
bool DCSwitch::SwitchFields::
do_check_match(const DCPackerInterface *) const {
  // This should never be called on a SwitchFields.
  nassertr(false, false);
  return false;
}

/**
 *
 */
DCSwitch::SwitchCase::
SwitchCase(const vector_uchar &value, DCSwitch::SwitchFields *fields) :
  _value(value),
  _fields(fields)
{
}

/**
 *
 */
DCSwitch::SwitchCase::
~SwitchCase() {
}

/**
 * Returns true if this case matches the indicated case, false otherwise.
 * This is only intended to be called internally from
 * DCSwitch::do_check_match_switch().
 */
bool DCSwitch::SwitchCase::
do_check_match_switch_case(const DCSwitch::SwitchCase *other) const {
  return _fields->do_check_match_switch_case(other->_fields);
}
