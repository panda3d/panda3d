/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcField.cxx
 * @author drose
 * @date 2000-10-11
 */

#include "dcField.h"
#include "dcFile.h"
#include "dcPacker.h"
#include "dcClass.h"
#include "hashGenerator.h"
#include "dcmsgtypes.h"

/**
 *
 */
DCField::
DCField() :
  _dclass(nullptr)
#ifdef WITHIN_PANDA
  ,
  _field_update_pcollector("DCField")
#endif
{
  _number = -1;
  _default_value_stale = true;
  _has_default_value = false;

  _bogus_field = false;

  _has_nested_fields = true;
  _num_nested_fields = 0;
  _pack_type = PT_field;

  _has_fixed_byte_size = true;
  _fixed_byte_size = 0;
  _has_fixed_structure = true;
}

/**
 *
 */
DCField::
DCField(const std::string &name, DCClass *dclass) :
  DCPackerInterface(name),
  _dclass(dclass)
#ifdef WITHIN_PANDA
  ,
  _field_update_pcollector(dclass->_class_update_pcollector, name)
#endif
{
  _number = -1;
  _has_default_value = false;
  _default_value_stale = true;

  _bogus_field = false;

  _has_nested_fields = true;
  _num_nested_fields = 0;
  _pack_type = PT_field;

  _has_fixed_byte_size = true;
  _fixed_byte_size = 0;
  _has_fixed_structure = true;
}

/**
 *
 */
DCField::
~DCField() {
}

/**
 *
 */
DCField *DCField::
as_field() {
  return this;
}

/**
 *
 */
const DCField *DCField::
as_field() const {
  return this;
}

/**
 * Returns the same field pointer converted to an atomic field pointer, if
 * this is in fact an atomic field; otherwise, returns NULL.
 */
DCAtomicField *DCField::
as_atomic_field() {
  return nullptr;
}

/**
 * Returns the same field pointer converted to an atomic field pointer, if
 * this is in fact an atomic field; otherwise, returns NULL.
 */
const DCAtomicField *DCField::
as_atomic_field() const {
  return nullptr;
}

/**
 * Returns the same field pointer converted to a molecular field pointer, if
 * this is in fact a molecular field; otherwise, returns NULL.
 */
DCMolecularField *DCField::
as_molecular_field() {
  return nullptr;
}

/**
 * Returns the same field pointer converted to a molecular field pointer, if
 * this is in fact a molecular field; otherwise, returns NULL.
 */
const DCMolecularField *DCField::
as_molecular_field() const {
  return nullptr;
}

/**
 *
 */
DCParameter *DCField::
as_parameter() {
  return nullptr;
}

/**
 *
 */
const DCParameter *DCField::
as_parameter() const {
  return nullptr;
}

/**
 * Given a blob that represents the packed data for this field, returns a
 * string formatting it for human consumption.  Returns empty string if there
 * is an error.
 */
std::string DCField::
format_data(const vector_uchar &packed_data, bool show_field_names) {
  DCPacker packer;
  packer.set_unpack_data(packed_data);
  packer.begin_unpack(this);
  std::string result = packer.unpack_and_format(show_field_names);
  if (!packer.end_unpack()) {
    return std::string();
  }
  return result;
}

/**
 * Given a human-formatted string (for instance, as returned by format_data(),
 * above) that represents the value of this field, parse the string and return
 * the corresponding packed data.  Returns empty string if there is an error.
 */
vector_uchar DCField::
parse_string(const std::string &formatted_string) {
  DCPacker packer;
  packer.begin_pack(this);
  if (!packer.parse_and_pack(formatted_string)) {
    // Parse error.
    return vector_uchar();
  }
  if (!packer.end_pack()) {
    // Data type mismatch.
    return vector_uchar();
  }

  return packer.get_bytes();
}

/**
 * Verifies that all of the packed values in the field data are within the
 * specified ranges and that there are no extra bytes on the end of the
 * record.  Returns true if all fields are valid, false otherwise.
 */
bool DCField::
validate_ranges(const vector_uchar &packed_data) const {
  DCPacker packer;
  packer.set_unpack_data(packed_data);
  packer.begin_unpack(this);
  packer.unpack_validate();
  if (!packer.end_unpack()) {
    return false;
  }

  return (packer.get_num_unpacked_bytes() == packed_data.size());
}

/**
 * Accumulates the properties of this field into the hash.
 */
void DCField::
generate_hash(HashGenerator &hashgen) const {
  // It shouldn't be necessary to explicitly add _number to the hash--this is
  // computed based on the relative position of this field with the other
  // fields, so adding it explicitly will be redundant.  However, the field
  // name is significant.
  hashgen.add_string(_name);

  // Actually, we add _number anyway, since we need to ensure the hash code
  // comes out different in the dc_multiple_inheritance case.
  if (dc_multiple_inheritance) {
    hashgen.add_int(_number);
  }
}

/**
 * Packs the field's specified default value (or a sensible default if no
 * value is specified) into the stream.  Returns true if the default value is
 * packed, false if the field doesn't know how to pack its default value.
 */
bool DCField::
pack_default_value(DCPackData &pack_data, bool &) const {
  // The default behavior is to pack the default value if we got it;
  // otherwise, to return false and let the packer visit our nested elements.
  if (!_default_value_stale) {
    pack_data.append_data((const char *)_default_value.data(), _default_value.size());
    return true;
  }

  return false;
}

/**
 * Sets the name of this field.
 */
void DCField::
set_name(const std::string &name) {
  DCPackerInterface::set_name(name);
  if (_dclass != nullptr) {
    _dclass->_dc_file->mark_inherited_fields_stale();
  }
}

/**
 * Recomputes the default value of the field by repacking it.
 */
void DCField::
refresh_default_value() {
  DCPacker packer;
  packer.begin_pack(this);
  packer.pack_default_value();
  if (!packer.end_pack()) {
    std::cerr << "Error while packing default value for " << get_name() << "\n";
  } else {
    const unsigned char *data = (const unsigned char *)packer.get_data();
    _default_value = vector_uchar(data, data + packer.get_length());
  }
  _default_value_stale = false;
}
