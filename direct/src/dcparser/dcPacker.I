// Filename: dcPacker.I
// Created by:  drose (15Jun04)
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


////////////////////////////////////////////////////////////////////
//     Function: DCPacker::has_nested_fields
//       Access: Public, Virtual
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
  return (_current_field != (DCPackerInterface *)NULL);
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
    if (!_current_field->pack_double(_pack_data, value)) {
      _pack_error = true;
    }
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
    if (!_current_field->pack_int(_pack_data, value)) {
      _pack_error = true;
    }
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
    if (!_current_field->pack_uint(_pack_data, value)) {
      _pack_error = true;
    }
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
    if (!_current_field->pack_int64(_pack_data, value)) {
      _pack_error = true;
    }
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
    if (!_current_field->pack_uint64(_pack_data, value)) {
      _pack_error = true;
    }
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
    if (!_current_field->pack_string(_pack_data, value)) {
      _pack_error = true;
    }
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
    if (!_current_field->unpack_double(_unpack_data, _unpack_length, _unpack_p, value)) {
      _pack_error = true;
    }
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
    if (!_current_field->unpack_int(_unpack_data, _unpack_length, _unpack_p, value)) {
      _pack_error = true;
    }
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
    if (!_current_field->unpack_uint(_unpack_data, _unpack_length, _unpack_p, value)) {
      _pack_error = true;
    }
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
    if (!_current_field->unpack_int64(_unpack_data, _unpack_length, _unpack_p, value)) {
      _pack_error = true;
    }
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
    if (!_current_field->unpack_uint64(_unpack_data, _unpack_length, _unpack_p, value)) {
      _pack_error = true;
    }
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
    if (!_current_field->unpack_string(_unpack_data, _unpack_length, _unpack_p, value)) {
      _pack_error = true;
    }
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
//     Function: DCPacker::had_pack_error
//       Access: Published
//  Description: Returns true if there has been an error in packing
//               since the most recent call to begin().  It is
//               most useful to call this after end() to validate
//               the entire packing run.
////////////////////////////////////////////////////////////////////
INLINE bool DCPacker::
had_pack_error() const {
  return _pack_error;
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
//     Function: DCPacker::get_data
//       Access: Public
//  Description: Returns the beginning of the data buffer.  The buffer
//               is not null-terminated, but see also get_string().
//
//               This may be used in conjunction with get_length() to
//               copy all of the bytes out of the buffer.
////////////////////////////////////////////////////////////////////
INLINE const char *DCPacker::
get_data() const {
  return _pack_data.get_data();
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

  } else if (_mode == M_unpack && _push_marker != 0 &&
             _unpack_p >= _push_marker) {
    // Done with all the fields on this parent.  The caller must now
    // call pop().
    _current_field = NULL;

  } else {
    // We have another field to advance to.
    _current_field = _current_parent->get_nested_field(_current_field_index);
  }
}
