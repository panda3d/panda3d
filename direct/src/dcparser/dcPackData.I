// Filename: dcPackData.I
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
//     Function: DCPackData::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE DCPackData::
DCPackData() {
  _buffer = NULL;
  _allocated_size = 0;
  _used_length = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackData::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE DCPackData::
~DCPackData() {
  if (_buffer != (const char *)NULL) {
    delete[] _buffer;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackData::clear
//       Access: Published
//  Description: Empties the contents of the data (without necessarily
//               freeing its allocated memory).
////////////////////////////////////////////////////////////////////
INLINE void DCPackData::
clear() {
  _used_length = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackData::append_data
//       Access: Published
//  Description: Adds the indicated bytes to the end of the data.
////////////////////////////////////////////////////////////////////
INLINE void DCPackData::
append_data(const char *buffer, size_t size) {
  set_used_length(_used_length + size);
  memcpy(_buffer + _used_length - size, buffer, size);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackData::get_write_pointer
//       Access: Published
//  Description: Adds the indicated number of bytes to the end of the
//               data without initializing them, and returns a pointer
//               to the beginning of the new data.
////////////////////////////////////////////////////////////////////
INLINE char *DCPackData::
get_write_pointer(size_t size) {
  set_used_length(_used_length + size);
  return _buffer + _used_length - size;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackData::append_junk
//       Access: Published
//  Description: Adds some uninitialized bytes to the end of the data.
////////////////////////////////////////////////////////////////////
INLINE void DCPackData::
append_junk(size_t size) {
  set_used_length(_used_length + size);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackData::rewrite_data
//       Access: Published
//  Description: Changes the data at the indicated position to the
//               given value.  It is an error if there are not at
//               least position + size bytes in the data.
////////////////////////////////////////////////////////////////////
INLINE void DCPackData::
rewrite_data(size_t position, const char *buffer, size_t size) {
  nassertv(position + size <= _used_length);
  memcpy(_buffer + position, buffer, size);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackData::get_rewrite_pointer
//       Access: Published
//  Description: Returns a pointer into the middle of the data at the
//               indicated point.
////////////////////////////////////////////////////////////////////
INLINE char *DCPackData::
get_rewrite_pointer(size_t position, size_t size) {
  nassertr(position + size <= _used_length, NULL);
  return _buffer + position;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackData::get_string
//       Access: Published
//  Description: Returns the data buffer as a string.  Also see
//               get_data().
////////////////////////////////////////////////////////////////////
INLINE string DCPackData::
get_string() const {
  return string(_buffer, _used_length);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackData::get_length
//       Access: Published
//  Description: Returns the current length of the buffer.  This is
//               the number of useful bytes stored in the buffer, not
//               the amount of memory it takes up.
////////////////////////////////////////////////////////////////////
INLINE size_t DCPackData::
get_length() const {
  return _used_length;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackData::get_data
//       Access: Public
//  Description: Returns the beginning of the data buffer.  The buffer
//               is not null-terminated, but see also get_string().
//
//               This may be used in conjunction with get_length() to
//               copy all of the bytes out of the buffer.
////////////////////////////////////////////////////////////////////
INLINE const char *DCPackData::
get_data() const {
  return _buffer;
}
