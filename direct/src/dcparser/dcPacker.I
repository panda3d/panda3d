// Filename: dcPacker.I
// Created by:  drose (15Jun04)
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


////////////////////////////////////////////////////////////////////
//     Function: DCPacker::clear_data
//       Access: Published
//  Description: Empties the data in the pack buffer and unpack
//               buffer.  This should be called between calls to
//               begin_pack(), unless you want to concatenate all of
//               the pack results together.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
clear_data() {
  _pack_data.clear();

  if (_owns_unpack_data) {
    delete[] _unpack_data;
    _owns_unpack_data = false;
  }
  _unpack_data = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::has_nested_fields
//       Access: Published
//  Description: Returns true if the current field has any nested
//               fields (and thus expects a push() .. pop()
//               interface), or false otherwise.  If this returns
//               true, get_num_nested_fields() may be called to
//               determine how many nested fields are expected.
////////////////////////////////////////////////////////////////////
INLINE bool DCPacker::
has_nested_fields() const {
  if (_current_field == NULL) {
    return false;
  } else {
    return _current_field->has_nested_fields();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_num_nested_fields
//       Access: Published
//  Description: Returns the number of nested fields associated with
//               the current field, if has_nested_fields() returned
//               true.
//
//               The return value may be -1 to indicate that a
//               variable number of nested fields are accepted by this
//               field type (e.g. a variable-length array).
//
//               Note that this method is unreliable to determine how
//               many fields you must traverse before you can call
//               pop(), since particularly in the presence of a
//               DCSwitch, it may change during traversal.  Use
//               more_nested_fields() instead.
////////////////////////////////////////////////////////////////////
INLINE int DCPacker::
get_num_nested_fields() const {
  return _num_nested_fields;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::more_nested_fields
//       Access: Published
//  Description: Returns true if there are more nested fields to pack
//               or unpack in the current push sequence, false if it
//               is time to call pop().
////////////////////////////////////////////////////////////////////
INLINE bool DCPacker::
more_nested_fields() const {
  return (_current_field != (DCPackerInterface *)NULL && !_pack_error);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_current_parent
//       Access: Published
//  Description: Returns the field that we left in our last call to
//               push(): the owner of the current level of fields.
//               This may be NULL at the beginning of the pack
//               operation.
////////////////////////////////////////////////////////////////////
INLINE const DCPackerInterface *DCPacker::
get_current_parent() const {
  return _current_parent;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_current_field
//       Access: Published
//  Description: Returns the field that will be referenced by the next
//               call to pack_*() or unpack_*().  This will be NULL if
//               we have unpacked (or packed) all fields, or if it is
//               time to call pop().
////////////////////////////////////////////////////////////////////
INLINE const DCPackerInterface *DCPacker::
get_current_field() const {
  return _current_field;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_last_switch
//       Access: Published
//  Description: Returns a pointer to the last DCSwitch instance that
//               we have passed by and selected one case of during the
//               pack/unpack process.  Each time we encounter a new
//               DCSwitch and select a case, this will change state.
//
//               This may be used to detect when a DCSwitch has been
//               selected.  At the moment this changes state,
//               get_current_parent() will contain the particular
//               SwitchCase that was selected by the switch.
////////////////////////////////////////////////////////////////////
INLINE const DCSwitchParameter *DCPacker::
get_last_switch() const {
  return _last_switch;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_pack_type
//       Access: Published
//  Description: Returns the type of value expected by the current
//               field.  See the enumerated type definition at the top
//               of DCPackerInterface.h.  If this returns one of
//               PT_double, PT_int, PT_int64, or PT_string, then you
//               should call the corresponding pack_double(),
//               pack_int() function (or unpack_double(),
//               unpack_int(), etc.) to transfer data.  Otherwise, you
//               should call push() and begin packing or unpacking the
//               nested fields.
////////////////////////////////////////////////////////////////////
INLINE DCPackType DCPacker::
get_pack_type() const {
  if (_current_field == NULL) {
    return PT_invalid;
  } else {
    return _current_field->get_pack_type();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_current_field_name
//       Access: Published
//  Description: Returns the name of the current field, if it has a
//               name, or the empty string if the field does not have
//               a name or there is no current field.
////////////////////////////////////////////////////////////////////
INLINE string DCPacker::
get_current_field_name() const {
  if (_current_field == NULL) {
    return string();
  } else {
    return _current_field->get_name();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::pack_double
//       Access: Published
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
pack_double(double value) {
  nassertv(_mode == M_pack || _mode == M_repack);
  if (_current_field == NULL) {
    _pack_error = true;
  } else {
    _current_field->pack_double(_pack_data, value, _pack_error, _range_error);
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::pack_int
//       Access: Published
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
pack_int(int value) {
  nassertv(_mode == M_pack || _mode == M_repack);
  if (_current_field == NULL) {
    _pack_error = true;
  } else {
    _current_field->pack_int(_pack_data, value, _pack_error, _range_error);
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::pack_uint
//       Access: Published
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
pack_uint(unsigned int value) {
  nassertv(_mode == M_pack || _mode == M_repack);
  if (_current_field == NULL) {
    _pack_error = true;
  } else {
    _current_field->pack_uint(_pack_data, value, _pack_error, _range_error);
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::pack_int64
//       Access: Published
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
pack_int64(PN_int64 value) {
  nassertv(_mode == M_pack || _mode == M_repack);
  if (_current_field == NULL) {
    _pack_error = true;
  } else {
    _current_field->pack_int64(_pack_data, value, _pack_error, _range_error);
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::pack_uint64
//       Access: Published
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
pack_uint64(PN_uint64 value) {
  nassertv(_mode == M_pack || _mode == M_repack);
  if (_current_field == NULL) {
    _pack_error = true;
  } else {
    _current_field->pack_uint64(_pack_data, value, _pack_error, _range_error);
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::pack_string
//       Access: Published
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
pack_string(const string &value) {
  nassertv(_mode == M_pack || _mode == M_repack);
  if (_current_field == NULL) {
    _pack_error = true;
  } else {
    _current_field->pack_string(_pack_data, value, _pack_error, _range_error);
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::pack_literal_value
//       Access: Published
//  Description: Adds the indicated string value into the stream,
//               representing a single pre-packed field element, or a
//               whole group of field elements at once.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
pack_literal_value(const string &value) {
  nassertv(_mode == M_pack || _mode == M_repack);
  if (_current_field == NULL) {
    _pack_error = true;
  } else {
    _pack_data.append_data(value.data(), value.length());
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_double
//       Access: Published
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE double DCPacker::
unpack_double() {
  double value = 0.0;
  nassertr(_mode == M_unpack, value);
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    _current_field->unpack_double(_unpack_data, _unpack_length, _unpack_p, 
                                  value, _pack_error, _range_error);
    advance();
  }

  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_int
//       Access: Published
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE int DCPacker::
unpack_int() {
  int value = 0;
  nassertr(_mode == M_unpack, value);
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    _current_field->unpack_int(_unpack_data, _unpack_length, _unpack_p,
                               value, _pack_error, _range_error);
    advance();
  }

  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_uint
//       Access: Published
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE unsigned int DCPacker::
unpack_uint() {
  unsigned int value = 0;
  nassertr(_mode == M_unpack, value);
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    _current_field->unpack_uint(_unpack_data, _unpack_length, _unpack_p,
                                value, _pack_error, _range_error);
    advance();
  }

  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_int64
//       Access: Published
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE PN_int64 DCPacker::
unpack_int64() {
  PN_int64 value = 0;
  nassertr(_mode == M_unpack, value);
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    _current_field->unpack_int64(_unpack_data, _unpack_length, _unpack_p,
                                 value, _pack_error, _range_error);
    advance();
  }

  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_uint64
//       Access: Published
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE PN_uint64 DCPacker::
unpack_uint64() {
  PN_uint64 value = 0;
  nassertr(_mode == M_unpack, value);
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    _current_field->unpack_uint64(_unpack_data, _unpack_length, _unpack_p,
                                  value, _pack_error, _range_error);
    advance();
  }

  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_string
//       Access: Published
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE string DCPacker::
unpack_string() {
  string value;
  nassertr(_mode == M_unpack, value);
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    _current_field->unpack_string(_unpack_data, _unpack_length, _unpack_p,
                                  value, _pack_error, _range_error);
    advance();
  }

  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_literal_value
//       Access: Published
//  Description: Returns the literal string that represents the packed
//               value of the current field, and advances the field
//               pointer.
////////////////////////////////////////////////////////////////////
INLINE string DCPacker::
unpack_literal_value() {
  size_t start = _unpack_p;
  unpack_skip();
  nassertr(_unpack_p >= start, string());
  return string(_unpack_data + start, _unpack_p - start);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_double
//       Access: Public
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
unpack_double(double &value) {
  nassertv(_mode == M_unpack);
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    _current_field->unpack_double(_unpack_data, _unpack_length, _unpack_p, 
                                  value, _pack_error, _range_error);
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_int
//       Access: Public
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
unpack_int(int &value) {
  nassertv(_mode == M_unpack);
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    _current_field->unpack_int(_unpack_data, _unpack_length, _unpack_p,
                               value, _pack_error, _range_error);
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_uint
//       Access: Public
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
unpack_uint(unsigned int &value) {
  nassertv(_mode == M_unpack);
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    _current_field->unpack_uint(_unpack_data, _unpack_length, _unpack_p,
                                value, _pack_error, _range_error);
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_int64
//       Access: Public
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
unpack_int64(PN_int64 &value) {
  nassertv(_mode == M_unpack);
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    _current_field->unpack_int64(_unpack_data, _unpack_length, _unpack_p,
                                 value, _pack_error, _range_error);
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_uint64
//       Access: Public
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
unpack_uint64(PN_uint64 &value) {
  nassertv(_mode == M_unpack);
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    _current_field->unpack_uint64(_unpack_data, _unpack_length, _unpack_p,
                                  value, _pack_error, _range_error);
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_string
//       Access: Public
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
unpack_string(string &value) {
  nassertv(_mode == M_unpack);
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    _current_field->unpack_string(_unpack_data, _unpack_length, _unpack_p,
                                  value, _pack_error, _range_error);
    advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_literal_value
//       Access: Public
//  Description: Returns the literal string that represents the packed
//               value of the current field, and advances the field
//               pointer.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
unpack_literal_value(string &value) {
  size_t start = _unpack_p;
  unpack_skip();
  nassertv(_unpack_p >= start);
  value.assign(_unpack_data + start, _unpack_p - start);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::had_parse_error
//       Access: Published
//  Description: Returns true if there has been an parse error
//               since the most recent call to begin(); this can only
//               happen if you call parse_and_pack().
////////////////////////////////////////////////////////////////////
INLINE bool DCPacker::
had_parse_error() const {
  return _parse_error;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::had_pack_error
//       Access: Published
//  Description: Returns true if there has been an packing error
//               since the most recent call to begin(); in particular,
//               this may be called after end() has returned false to
//               determine the nature of the failure.
//
//               A return value of true indicates there was a push/pop
//               mismatch, or the push/pop structure did not match the
//               data structure, or there were the wrong number of
//               elements in a nested push/pop structure, or on unpack
//               that the data stream was truncated.
////////////////////////////////////////////////////////////////////
INLINE bool DCPacker::
had_pack_error() const {
  return _pack_error;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::had_range_error
//       Access: Published
//  Description: Returns true if there has been an range validation
//               error since the most recent call to begin(); in
//               particular, this may be called after end() has
//               returned false to determine the nature of the
//               failure.
//
//               A return value of true indicates a value that was
//               packed or unpacked did not fit within the specified
//               legal range for a parameter, or within the limits of
//               the field size.
////////////////////////////////////////////////////////////////////
INLINE bool DCPacker::
had_range_error() const {
  return _range_error;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::had_error
//       Access: Published
//  Description: Returns true if there has been any error (either a
//               pack error or a range error) since the most recent
//               call to begin().  If this returns true, then the
//               matching call to end() will indicate an error
//               (false).
////////////////////////////////////////////////////////////////////
INLINE bool DCPacker::
had_error() const {
  return _range_error || _pack_error || _parse_error;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_num_unpacked_bytes
//       Access: Published
//  Description: Returns the number of bytes that have been unpacked
//               so far, or after unpack_end(), the total number of
//               bytes that were unpacked at all.  This can be used to
//               validate that all of the bytes in the buffer were
//               actually unpacked (which is not otherwise considered
//               an error).
////////////////////////////////////////////////////////////////////
INLINE size_t DCPacker::
get_num_unpacked_bytes() const {
  return _unpack_p;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_length
//       Access: Published
//  Description: Returns the current length of the buffer.  This is
//               the number of useful bytes stored in the buffer, not
//               the amount of memory it takes up.
////////////////////////////////////////////////////////////////////
INLINE size_t DCPacker::
get_length() const {
  return _pack_data.get_length();
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_string
//       Access: Published
//  Description: Returns the packed data buffer as a string.  Also see
//               get_data().
////////////////////////////////////////////////////////////////////
INLINE string DCPacker::
get_string() const {
  return _pack_data.get_string();
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_unpack_length
//       Access: Published
//  Description: Returns the total number of bytes in the unpack data
//               buffer.  This is the buffer used when unpacking; it
//               is separate from the pack data returned by
//               get_length(), which is filled during packing.
////////////////////////////////////////////////////////////////////
INLINE size_t DCPacker::
get_unpack_length() const {
  return _unpack_length;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_unpack_string
//       Access: Published
//  Description: Returns the unpack data buffer, as a string.
//               This is the buffer used when unpacking; it is
//               separate from the pack data returned by get_string(),
//               which is filled during packing.  Also see
//               get_unpack_data().
////////////////////////////////////////////////////////////////////
INLINE string DCPacker::
get_unpack_string() const {
  return string(_unpack_data, _unpack_length);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_string
//       Access: Published
//  Description: Copies the packed data into the indicated string.
//               Also see get_data().
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
get_string(string &data) const {
  data.assign(_pack_data.get_data(), _pack_data.get_length());
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_data
//       Access: Public
//  Description: Returns the beginning of the data buffer.  The buffer
//               is not null-terminated, but see also get_string().
//
//               This may be used in conjunction with get_length() to
//               copy all of the bytes out of the buffer.  Also see
//               take_data() to get the packed data without a copy
//               operation.
////////////////////////////////////////////////////////////////////
INLINE const char *DCPacker::
get_data() const {
  return _pack_data.get_data();
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::take_data
//       Access: Public
//  Description: Returns the pointer to the beginning of the data
//               buffer, and transfers ownership of the buffer to the
//               caller.  The caller is now responsible for ultimately
//               freeing the returned pointer with delete[], if it is
//               non-NULL.  This may (or may not) return NULL if the
//               buffer is empty.
//
//               This also empties the DCPackData structure, and sets
//               its length to zero (so you should call get_length()
//               before calling this method).
////////////////////////////////////////////////////////////////////
INLINE char *DCPacker::
take_data() {
  return _pack_data.take_data();
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::append_data
//       Access: Public
//  Description: Adds the indicated bytes to the end of the data.
//               This may only be called between packing sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
append_data(const char *buffer, size_t size) {
  nassertv(_mode == M_idle);
  _pack_data.append_data(buffer, size);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_write_pointer
//       Access: Public
//  Description: Adds the indicated number of bytes to the end of the
//               data without initializing them, and returns a pointer
//               to the beginning of the new data.  This may only be
//               called between packing sessions.
////////////////////////////////////////////////////////////////////
INLINE char *DCPacker::
get_write_pointer(size_t size) {
  nassertr(_mode == M_idle, NULL);
  return _pack_data.get_write_pointer(size);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::get_unpack_data
//       Access: Public
//  Description: Returns a read pointer to the unpack data buffer.
//               This is the buffer used when unpacking; it is
//               separate from the pack data returned by get_data(),
//               which is filled during packing.
////////////////////////////////////////////////////////////////////
INLINE const char *DCPacker::
get_unpack_data() const {
  return _unpack_data;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::StackElement::get_num_stack_elements_ever_allocated
//       Access: Published, Static
//  Description: Returns the number of DCPacker::StackElement pointers
//               ever simultaneously allocated; these are now either
//               in active use or have been recycled into the deleted
//               DCPacker::StackElement pool to be used again.
////////////////////////////////////////////////////////////////////
INLINE int DCPacker::
get_num_stack_elements_ever_allocated() {
  return StackElement::_num_ever_allocated;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_pack_int8
//       Access: Published
//  Description: Packs the data into the buffer between packing
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_pack_int8(int value) {
  nassertv(_mode == M_idle);
  DCPackerInterface::do_pack_int8(_pack_data.get_write_pointer(1), value);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_pack_int16
//       Access: Published
//  Description: Packs the data into the buffer between packing
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_pack_int16(int value) {
  nassertv(_mode == M_idle);
  DCPackerInterface::do_pack_int16(_pack_data.get_write_pointer(2), value);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_pack_int32
//       Access: Published
//  Description: Packs the data into the buffer between packing
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_pack_int32(int value) {
  nassertv(_mode == M_idle);
  DCPackerInterface::do_pack_int32(_pack_data.get_write_pointer(4), value);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_pack_int64
//       Access: Published
//  Description: Packs the data into the buffer between packing
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_pack_int64(PN_int64 value) {
  nassertv(_mode == M_idle);
  DCPackerInterface::do_pack_int64(_pack_data.get_write_pointer(8), value);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_pack_uint8
//       Access: Published
//  Description: Packs the data into the buffer between packing
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_pack_uint8(unsigned int value) {
  nassertv(_mode == M_idle);
  DCPackerInterface::do_pack_uint8(_pack_data.get_write_pointer(1), value);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_pack_uint16
//       Access: Published
//  Description: Packs the data into the buffer between packing
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_pack_uint16(unsigned int value) {
  nassertv(_mode == M_idle);
  DCPackerInterface::do_pack_uint16(_pack_data.get_write_pointer(2), value);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_pack_uint32
//       Access: Published
//  Description: Packs the data into the buffer between packing
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_pack_uint32(unsigned int value) {
  nassertv(_mode == M_idle);
  DCPackerInterface::do_pack_uint32(_pack_data.get_write_pointer(4), value);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_pack_uint64
//       Access: Published
//  Description: Packs the data into the buffer between packing
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_pack_uint64(PN_uint64 value) {
  nassertv(_mode == M_idle);
  DCPackerInterface::do_pack_uint64(_pack_data.get_write_pointer(8), value);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_pack_float64
//       Access: Published
//  Description: Packs the data into the buffer between packing
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_pack_float64(double value) {
  nassertv(_mode == M_idle);
  DCPackerInterface::do_pack_float64(_pack_data.get_write_pointer(8), value);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_pack_string
//       Access: Published
//  Description: Packs the data into the buffer between packing
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_pack_string(const string &value) {
  nassertv(_mode == M_idle);
  DCPackerInterface::do_pack_uint16(_pack_data.get_write_pointer(2), value.length());
  _pack_data.append_data(value.data(), value.length());
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_int8
//       Access: Published
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE int DCPacker::
raw_unpack_int8() {
  int value = 0;
  raw_unpack_int8(value);
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_int16
//       Access: Published
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE int DCPacker::
raw_unpack_int16() {
  int value = 0;
  raw_unpack_int16(value);
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_int32
//       Access: Published
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE int DCPacker::
raw_unpack_int32() {
  int value = 0;
  raw_unpack_int32(value);
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_int64
//       Access: Published
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE PN_int64 DCPacker::
raw_unpack_int64() {
  PN_int64 value = 0;
  raw_unpack_int64(value);
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_int8
//       Access: Public
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_unpack_int8(int &value) {
  nassertv(_mode == M_idle && _unpack_data != NULL);
  if (_unpack_p + 1 > _unpack_length) {
    _pack_error = true;
    return;
  }
  value = DCPackerInterface::do_unpack_int8(_unpack_data + _unpack_p);
  _unpack_p++;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_int16
//       Access: Public
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_unpack_int16(int &value) {
  nassertv(_mode == M_idle && _unpack_data != NULL);
  if (_unpack_p + 2 > _unpack_length) {
    _pack_error = true;
    return;
  }
  value = DCPackerInterface::do_unpack_int16(_unpack_data + _unpack_p);
  _unpack_p += 2;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_int32
//       Access: Public
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_unpack_int32(int &value) {
  nassertv(_mode == M_idle && _unpack_data != NULL);
  if (_unpack_p + 4 > _unpack_length) {
    _pack_error = true;
    return;
  }
  value = DCPackerInterface::do_unpack_int32(_unpack_data + _unpack_p);
  _unpack_p += 4;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_uint8
//       Access: Published
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE unsigned int DCPacker::
raw_unpack_uint8() {
  unsigned int value = 0;
  raw_unpack_uint8(value);
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_uint16
//       Access: Published
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE unsigned int DCPacker::
raw_unpack_uint16() {
  unsigned int value = 0;
  raw_unpack_uint16(value);
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_uint32
//       Access: Published
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE unsigned int DCPacker::
raw_unpack_uint32() {
  unsigned int value = 0;
  raw_unpack_uint32(value);
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_uint64
//       Access: Published
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE PN_uint64 DCPacker::
raw_unpack_uint64() {
  PN_uint64 value = 0;
  raw_unpack_uint64(value);
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_float64
//       Access: Published
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE double DCPacker::
raw_unpack_float64() {
  double value = 0;
  raw_unpack_float64(value);
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_string
//       Access: Published
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE string DCPacker::
raw_unpack_string() {
  string value;
  raw_unpack_string(value);
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_int64
//       Access: Public
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_unpack_int64(PN_int64 &value) {
  nassertv(_mode == M_idle && _unpack_data != NULL);
  if (_unpack_p + 8 > _unpack_length) {
    _pack_error = true;
    return;
  }
  value = DCPackerInterface::do_unpack_int64(_unpack_data + _unpack_p);
  _unpack_p += 8;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_uint8
//       Access: Public
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_unpack_uint8(unsigned int &value) {
  nassertv(_mode == M_idle && _unpack_data != NULL);
  if (_unpack_p + 1 > _unpack_length) {
    _pack_error = true;
    return;
  }
  value = DCPackerInterface::do_unpack_uint8(_unpack_data + _unpack_p);
  _unpack_p++;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_uint16
//       Access: Public
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_unpack_uint16(unsigned int &value) {
  nassertv(_mode == M_idle && _unpack_data != NULL);
  if (_unpack_p + 2 > _unpack_length) {
    _pack_error = true;
    return;
  }
  value = DCPackerInterface::do_unpack_uint16(_unpack_data + _unpack_p);
  _unpack_p += 2;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_uint32
//       Access: Public
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_unpack_uint32(unsigned int &value) {
  nassertv(_mode == M_idle && _unpack_data != NULL);
  if (_unpack_p + 4 > _unpack_length) {
    _pack_error = true;
    return;
  }
  value = DCPackerInterface::do_unpack_uint32(_unpack_data + _unpack_p);
  _unpack_p += 4;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_uint64
//       Access: Public
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_unpack_uint64(PN_uint64 &value) {
  nassertv(_mode == M_idle && _unpack_data != NULL);
  if (_unpack_p + 8 > _unpack_length) {
    _pack_error = true;
    return;
  }
  value = DCPackerInterface::do_unpack_uint64(_unpack_data + _unpack_p);
  _unpack_p += 8;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_float64
//       Access: Public
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_unpack_float64(double &value) {
  nassertv(_mode == M_idle && _unpack_data != NULL);
  if (_unpack_p + 8 > _unpack_length) {
    _pack_error = true;
    return;
  }
  value = DCPackerInterface::do_unpack_float64(_unpack_data + _unpack_p);
  _unpack_p += 8;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::raw_unpack_string
//       Access: Public
//  Description: Unpacks the data from the buffer between unpacking
//               sessions.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
raw_unpack_string(string &value) {
  nassertv(_mode == M_idle && _unpack_data != NULL);
  unsigned int string_length = raw_unpack_uint16();

  if (_unpack_p + string_length > _unpack_length) {
    _pack_error = true;
    return;
  }

  value.assign(_unpack_data + _unpack_p, string_length);
  _unpack_p += string_length;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::advance
//       Access: Private
//  Description: Advances to the next field after a call to
//               pack_value() or pop().
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::
advance() {
  _current_field_index++;
  if (_num_nested_fields >= 0 &&
      _current_field_index >= _num_nested_fields) {
    // Done with all the fields on this parent.  The caller must now
    // call pop().
    _current_field = NULL;

    // But if the parent is a switch record, we make a special case so
    // we can get the alternate fields.
    if (_current_parent != (DCPackerInterface *)NULL) {
      const DCSwitchParameter *switch_parameter = ((DCPackerInterface *)_current_parent)->as_switch_parameter();
      if (switch_parameter != (DCSwitchParameter *)NULL) {
        handle_switch(switch_parameter);
      }
    }

  } else if (_pop_marker != 0 && _unpack_p >= _pop_marker) {
    // Done with all the fields on this parent.  The caller must now
    // call pop().
    _current_field = NULL;

  } else {
    // We have another field to advance to.
    _current_field = _current_parent->get_nested_field(_current_field_index);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::StackElement::operator new
//       Access: Public
//  Description: Allocates the memory for a new DCPacker::StackElement.
//               This is specialized here to provide for fast
//               allocation of these things.
////////////////////////////////////////////////////////////////////
INLINE void *DCPacker::StackElement::
operator new(size_t size) {
  if (_deleted_chain != (DCPacker::StackElement *)NULL) {
    StackElement *obj = _deleted_chain;
    _deleted_chain = _deleted_chain->_next;
    return obj;
  }
#ifndef NDEBUG
  _num_ever_allocated++;
#endif  // NDEBUG
  return ::operator new(size);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::StackElement::operator delete
//       Access: Public
//  Description: Frees the memory for a deleted DCPacker::StackElement.
//               This is specialized here to provide for fast
//               allocation of these things.
////////////////////////////////////////////////////////////////////
INLINE void DCPacker::StackElement::
operator delete(void *ptr) {
  StackElement *obj = (StackElement *)ptr;
  obj->_next = _deleted_chain;
  _deleted_chain = obj;
}
