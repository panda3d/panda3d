// Filename: dcPacker.cxx
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

#include "dcPacker.h"

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
DCPacker::
DCPacker() {
  _current_field = NULL;
  _current_parent = NULL;
  _current_field_index = 0;
  _num_nested_fields = 0;
  _pack_error = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
DCPacker::
~DCPacker() {
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::begin
//       Access: Published
//  Description: Begins a packing session.  The parameter is the DC
//               object that describes the packing format; it may be a
//               DCType or DCField.
////////////////////////////////////////////////////////////////////
void DCPacker::
begin(DCPackerInterface *root) {
  // If this assertion fails, we didn't match begin() up with end().
  nassertv(_stack.empty() && 
           _current_field == NULL &&
           _current_parent == NULL);
  
  _pack_error = false;
  _pack_data.clear();

  _stack.clear();
  _current_field = root;
  _current_parent = NULL;
  _current_field_index = 0;
  _num_nested_fields = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::end
//       Access: Published, Virtual
//  Description: Finishes a packing session.
//
//               The return value is true on success, or false if
//               there has been some error during packing.
////////////////////////////////////////////////////////////////////
bool DCPacker::
end() {
  if (!_stack.empty() || _current_field != NULL || _current_parent != NULL) {
    _pack_error = true;
    _stack.clear();
    _current_field = NULL;
    _current_parent = NULL;
    _current_field_index = 0;
    _num_nested_fields = 0;
  }

  return !_pack_error;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::push
//       Access: Published
//  Description: Marks the beginning of a nested series of fields.
//
//               This must be called before filling the elements of an
//               array or the individual fields in a structure field.
//               It must also be balanced by a matching pop().
//
//               It is necessary to use push() / pop() if and only if
//               get_num_nested_fields() returns nonzero.
////////////////////////////////////////////////////////////////////
void DCPacker::
push() {
  if (_current_field == NULL) {
    _pack_error = true;

  } else {
    int num_nested_fields = _current_field->get_num_nested_fields();
    if (num_nested_fields == 0) {
      _pack_error = true;

    } else {
      StackElement element;
      element._current_parent = _current_parent;
      element._current_field_index = _current_field_index;
      element._push_start = _push_start;
      _stack.push_back(element);

      _current_parent = _current_field;
      _current_field_index = 0;
      _current_field = _current_parent->get_nested_field(0);
      _num_nested_fields = num_nested_fields;

      // Reserve length_bytes for when we figure out what the length
      // is.
      _push_start = _pack_data.get_length();
      size_t length_bytes = _current_parent->get_length_bytes();
      _pack_data.append_junk(length_bytes);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::pop
//       Access: Published
//  Description: Marks the end of a nested series of fields.
//
//               This must be called to match a previous push() only
//               after all the expected number of nested fields have
//               been packed.  It is an error to call it too early, or
//               too late.
////////////////////////////////////////////////////////////////////
void DCPacker::
pop() {
  if (_current_field != NULL && _num_nested_fields >= 0) {
    // Oops, didn't pack enough values.
    _pack_error = true;
  }

  if (_stack.empty()) {
    // Unbalanced pop().
    _pack_error = true;

  } else {
    size_t length_bytes = _current_parent->get_length_bytes();
    if (length_bytes != 0) {
      // Now go back and fill in the length of the array.
      char buffer[4];
      size_t length = _pack_data.get_length() - _push_start - length_bytes;
      if (length_bytes == 4) {
        buffer[0] = (char)(length & 0xff);
        buffer[1] = (char)((length >> 8) & 0xff);
        buffer[2] = (char)((length >> 16) & 0xff);
        buffer[3] = (char)((length >> 24) & 0xff);
        _pack_data.rewrite_data(_push_start, buffer, 4);
      } else {
        buffer[0] = (char)(length & 0xff);
        buffer[1] = (char)((length >> 8) & 0xff);
        _pack_data.rewrite_data(_push_start, buffer, 2);
      }
    }

    _current_field = _current_parent;
    _current_parent = _stack.back()._current_parent;
    _current_field_index = _stack.back()._current_field_index;
    _push_start = _stack.back()._push_start;
    _num_nested_fields = (_current_parent == NULL) ? 0 : _current_parent->get_num_nested_fields();
    _stack.pop_back();
  }

  advance();
}
