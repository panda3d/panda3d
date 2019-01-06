/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcPackerInterface.cxx
 * @author drose
 * @date 2004-06-15
 */

#include "dcPackerInterface.h"
#include "dcPackerCatalog.h"
#include "dcField.h"
#include "dcParserDefs.h"
#include "dcLexerDefs.h"

using std::string;

/**
 *
 */
DCPackerInterface::
DCPackerInterface(const string &name) :
  _name(name)
{
  _has_fixed_byte_size = false;
  _fixed_byte_size = 0;
  _has_fixed_structure = false;
  _has_range_limits = false;
  _num_length_bytes = 0;
  _has_nested_fields = false;
  _num_nested_fields = -1;
  _pack_type = PT_invalid;
  _catalog = nullptr;
}

/**
 *
 */
DCPackerInterface::
DCPackerInterface(const DCPackerInterface &copy) :
  _name(copy._name),
  _has_fixed_byte_size(copy._has_fixed_byte_size),
  _fixed_byte_size(copy._fixed_byte_size),
  _has_fixed_structure(copy._has_fixed_structure),
  _has_range_limits(copy._has_range_limits),
  _num_length_bytes(copy._num_length_bytes),
  _has_nested_fields(copy._has_nested_fields),
  _num_nested_fields(copy._num_nested_fields),
  _pack_type(copy._pack_type)
{
  _catalog = nullptr;
}

/**
 *
 */
DCPackerInterface::
~DCPackerInterface() {
  if (_catalog != nullptr) {
    delete _catalog;
  }
}

/**
 * Returns the index number to be passed to a future call to DCPacker::seek()
 * to seek directly to the named field without having to look up the field
 * name in a table later, or -1 if the named field cannot be found.
 *
 * If the named field is nested within a switch or some similar dynamic
 * structure that reveals different fields based on the contents of the data,
 * this mechanism cannot be used to pre-fetch the field index number--you must
 * seek for the field by name.
 */
int DCPackerInterface::
find_seek_index(const string &name) const {
  return get_catalog()->find_entry_by_name(name);
}

/**
 *
 */
DCField *DCPackerInterface::
as_field() {
  return nullptr;
}

/**
 *
 */
const DCField *DCPackerInterface::
as_field() const {
  return nullptr;
}

/**
 *
 */
DCSwitchParameter *DCPackerInterface::
as_switch_parameter() {
  return nullptr;
}

/**
 *
 */
const DCSwitchParameter *DCPackerInterface::
as_switch_parameter() const {
  return nullptr;
}

/**
 *
 */
DCClassParameter *DCPackerInterface::
as_class_parameter() {
  return nullptr;
}

/**
 *
 */
const DCClassParameter *DCPackerInterface::
as_class_parameter() const {
  return nullptr;
}

/**
 * Returns true if this interface is bitwise the same as the interface
 * described with the indicated formatted string, e.g.  "(uint8, uint8,
 * int16)", or false otherwise.
 *
 * If DCFile is not NULL, it specifies the DCFile that was previously loaded,
 * from which some predefined structs and typedefs may be referenced in the
 * description string.
 */
bool DCPackerInterface::
check_match(const string &description, DCFile *dcfile) const {
  bool match = false;

  std::istringstream strm(description);
  dc_init_parser_parameter_description(strm, "check_match", dcfile);
  dcyyparse();
  dc_cleanup_parser();

  DCField *field = dc_get_parameter_description();
  if (field != nullptr) {
    match = check_match(field);
    delete field;
  }

  if (dc_error_count() == 0) {
    return match;
  }

  // Parse error: no match is allowed.
  return false;
}

/**
 * Sets the name of this field.
 */
void DCPackerInterface::
set_name(const string &name) {
  _name = name;
}

/**
 * This flavor of get_num_nested_fields is used during unpacking.  It returns
 * the number of nested fields to expect, given a certain length in bytes (as
 * read from the _num_length_bytes stored in the stream on the push).  This
 * will only be called if _num_length_bytes is nonzero.
 */
int DCPackerInterface::
calc_num_nested_fields(size_t) const {
  return 0;
}

/**
 * Returns the DCPackerInterface object that represents the nth nested field.
 * This may return NULL if there is no such field (but it shouldn't do this if
 * n is in the range 0 <= n < get_num_nested_fields()).
 */
DCPackerInterface *DCPackerInterface::
get_nested_field(int) const {
  return nullptr;
}

/**
 * After a number of fields have been packed via push() .. pack_*() .. pop(),
 * this is called to confirm that the number of nested fields that were added
 * is valid for this type.  This is primarily useful for array types with
 * dynamic ranges that can't validate the number of fields any other way.
 */
bool DCPackerInterface::
validate_num_nested_fields(int) const {
  return true;
}

/**
 * Packs the indicated numeric or string value into the stream.
 */
void DCPackerInterface::
pack_double(DCPackData &, double, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Packs the indicated numeric or string value into the stream.
 */
void DCPackerInterface::
pack_int(DCPackData &, int, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Packs the indicated numeric or string value into the stream.
 */
void DCPackerInterface::
pack_uint(DCPackData &, unsigned int, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Packs the indicated numeric or string value into the stream.
 */
void DCPackerInterface::
pack_int64(DCPackData &, int64_t, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Packs the indicated numeric or string value into the stream.
 */
void DCPackerInterface::
pack_uint64(DCPackData &, uint64_t, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Packs the indicated numeric or string value into the stream.
 */
void DCPackerInterface::
pack_string(DCPackData &, const string &, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Packs the indicated numeric or string value into the stream.
 */
void DCPackerInterface::
pack_blob(DCPackData &, const vector_uchar &, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Packs the field's specified default value (or a sensible default if no
 * value is specified) into the stream.  Returns true if the default value is
 * packed, false if the field doesn't know how to pack its default value.
 */
bool DCPackerInterface::
pack_default_value(DCPackData &, bool &) const {
  return false;
}

/**
 * Unpacks the current numeric or string value from the stream.
 */
void DCPackerInterface::
unpack_double(const char *, size_t, size_t &, double &, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Unpacks the current numeric or string value from the stream.
 */
void DCPackerInterface::
unpack_int(const char *, size_t, size_t &, int &, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Unpacks the current numeric or string value from the stream.
 */
void DCPackerInterface::
unpack_uint(const char *, size_t, size_t &, unsigned int &, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Unpacks the current numeric or string value from the stream.
 */
void DCPackerInterface::
unpack_int64(const char *, size_t, size_t &, int64_t &, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Unpacks the current numeric or string value from the stream.
 */
void DCPackerInterface::
unpack_uint64(const char *, size_t, size_t &, uint64_t &, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Unpacks the current numeric or string value from the stream.
 */
void DCPackerInterface::
unpack_string(const char *, size_t, size_t &, string &, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Unpacks the current numeric or string value from the stream.
 */
void DCPackerInterface::
unpack_blob(const char *, size_t, size_t &, vector_uchar &, bool &pack_error, bool &) const {
  pack_error = true;
}

/**
 * Internally unpacks the current numeric or string value and validates it
 * against the type range limits, but does not return the value.  Returns true
 * on success, false on failure (e.g.  we don't know how to validate this
 * field).
 */
bool DCPackerInterface::
unpack_validate(const char *data, size_t length, size_t &p,
                bool &pack_error, bool &) const {
  if (!_has_range_limits) {
    return unpack_skip(data, length, p, pack_error);
  }
  return false;
}

/**
 * Increments p to the end of the current field without actually unpacking any
 * data or performing any range validation.  Returns true on success, false on
 * failure (e.g.  we don't know how to skip this field).
 */
bool DCPackerInterface::
unpack_skip(const char *data, size_t length, size_t &p,
            bool &pack_error) const {
  if (_has_fixed_byte_size) {
    // If this field has a fixed byte size, it's easy to skip.
    p += _fixed_byte_size;
    if (p > length) {
      pack_error = true;
    }
    return true;
  }

  if (_has_nested_fields && _num_length_bytes != 0) {
    // If we have a length prefix, use that for skipping.
    if (p + _num_length_bytes > length) {
      pack_error = true;

    } else {
      if (_num_length_bytes == 4) {
        size_t this_length = do_unpack_uint32(data + p);
        p += this_length + 4;
      } else {
        size_t this_length = do_unpack_uint16(data + p);
        p += this_length + 2;
      }
      if (p > length) {
        pack_error = true;
      }
    }
    return true;
  }

  // Otherwise, we don't know how to skip this field (presumably it can be
  // skipped by skipping over its nested fields individually).
  return false;
}

/**
 * Returns the DCPackerCatalog associated with this field, listing all of the
 * nested fields by name.
 */
const DCPackerCatalog *DCPackerInterface::
get_catalog() const {
  if (_catalog == nullptr) {
    ((DCPackerInterface *)this)->make_catalog();
  }
  return _catalog;
}

/**
 * Returns true if this field matches the indicated simple parameter, false
 * otherwise.
 */
bool DCPackerInterface::
do_check_match_simple_parameter(const DCSimpleParameter *) const {
  return false;
}

/**
 * Returns true if this field matches the indicated class parameter, false
 * otherwise.
 */
bool DCPackerInterface::
do_check_match_class_parameter(const DCClassParameter *) const {
  return false;
}

/**
 * Returns true if this field matches the indicated switch parameter, false
 * otherwise.
 */
bool DCPackerInterface::
do_check_match_switch_parameter(const DCSwitchParameter *) const {
  return false;
}

/**
 * Returns true if this field matches the indicated array parameter, false
 * otherwise.
 */
bool DCPackerInterface::
do_check_match_array_parameter(const DCArrayParameter *) const {
  return false;
}

/**
 * Returns true if this field matches the indicated atomic field, false
 * otherwise.
 */
bool DCPackerInterface::
do_check_match_atomic_field(const DCAtomicField *) const {
  return false;
}

/**
 * Returns true if this field matches the indicated molecular field, false
 * otherwise.
 */
bool DCPackerInterface::
do_check_match_molecular_field(const DCMolecularField *) const {
  return false;
}

/**
 * Called internally to create a new DCPackerCatalog object.
 */
void DCPackerInterface::
make_catalog() {
  nassertv(_catalog == nullptr);
  _catalog = new DCPackerCatalog(this);

  _catalog->r_fill_catalog("", this, nullptr, 0);
}
