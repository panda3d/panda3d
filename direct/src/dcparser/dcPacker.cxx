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
#include "dcParserDefs.h"
#include "dcLexerDefs.h"

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
DCPacker::
DCPacker() {
  _mode = M_idle;
  _unpack_data = NULL;
  _unpack_length = 0;
  _unpack_p = 0;
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
//     Function: DCPacker::begin_pack
//       Access: Published
//  Description: Begins a packing session.  The parameter is the DC
//               object that describes the packing format; it may be a
//               DCParameter or DCField.
////////////////////////////////////////////////////////////////////
void DCPacker::
begin_pack(const DCPackerInterface *root) {
  nassertv(_mode == M_idle);
  
  _mode = M_pack;
  _pack_error = false;
  _pack_data.clear();

  _stack.clear();
  _current_field = root;
  _current_parent = NULL;
  _current_field_index = 0;
  _num_nested_fields = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::end_pack
//       Access: Published, Virtual
//  Description: Finishes a packing session.
//
//               The return value is true on success, or false if
//               there has been some error during packing.
////////////////////////////////////////////////////////////////////
bool DCPacker::
end_pack() {
  nassertr(_mode == M_pack, false);
  
  _mode = M_idle;

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
//     Function: DCPacker::begin_unpack
//       Access: Published
//  Description: Begins an unpacking session.  Unlike the other
//               version of begin_unpack(), this version makes a copy
//               of the data string.
////////////////////////////////////////////////////////////////////
void DCPacker::
begin_unpack(const string &data, const DCPackerInterface *root) {
  _unpack_str = data;
  begin_unpack(_unpack_str.data(), _unpack_str.length(), root);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::begin_unpack
//       Access: Public
//  Description: Begins an unpacking session.  The data pointer is
//               used directly; the data buffer is not copied.
//               Therefore, you must not delete or modify the data
//               pointer until you call end_unpack().
////////////////////////////////////////////////////////////////////
void DCPacker::
begin_unpack(const char *data, size_t length,
             const DCPackerInterface *root) {
  nassertv(_mode == M_idle);
  
  _mode = M_unpack;
  _pack_error = false;
  _unpack_data = data;
  _unpack_length = length;
  _unpack_p = 0;

  _stack.clear();
  _current_field = root;
  _current_parent = NULL;
  _current_field_index = 0;
  _num_nested_fields = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::end_unpack
//       Access: Published
//  Description: Finishes the unpacking session.
//
//               The return value is true on success, or false if
//               there has been some error during unpacking.
////////////////////////////////////////////////////////////////////
bool DCPacker::
end_unpack() {
  nassertr(_mode == M_unpack, false);
  
  _mode = M_idle;

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
//               It is necessary to use push() / pop() only if
//               has_nested_fields() returns true.
////////////////////////////////////////////////////////////////////
void DCPacker::
push() {
  if (!has_nested_fields()) {
    _pack_error = true;

  } else {
    StackElement element;
    element._current_parent = _current_parent;
    element._current_field_index = _current_field_index;
    element._push_marker = _push_marker;
    _stack.push_back(element);
    _current_parent = _current_field;


    // Now deal with the length prefix that might or might not be
    // before a sequence of nested fields.
    int num_nested_fields = _current_parent->get_num_nested_fields();
    size_t length_bytes = _current_parent->get_num_length_bytes();
    
    if (_mode == M_pack) {
      // Reserve length_bytes for when we figure out what the length
      // is.
      _push_marker = _pack_data.get_length();
      _pack_data.append_junk(length_bytes);

    } else { // _mode == M_unpack
      // Read length_bytes to determine the end of this nested
      // sequence.
      _push_marker = 0;

      if (length_bytes != 0) {
        if (_unpack_p + length_bytes > _unpack_length) {
          _pack_error = true;

        } else {
          size_t length;
          if (length_bytes == 4) {
            length = DCPackerInterface::do_unpack_uint32
              (_unpack_data + _unpack_p);
            _unpack_p += 4;
            _push_marker = _unpack_p + length;
          } else {
            length = DCPackerInterface::do_unpack_uint16
              (_unpack_data + _unpack_p);
            _unpack_p += 2;
          }
          _push_marker = _unpack_p + length;
        
          // The explicit length trumps the number of nested fields
          // reported by get_num_nested_fields().
          if (length == 0) {
            num_nested_fields = 0;
          } else {
            num_nested_fields = _current_parent->calc_num_nested_fields(length);
          }
        }
      }
    }


    // Now point to the first field in the nested range.
    _num_nested_fields = num_nested_fields;
    _current_field_index = 0;

    if (_num_nested_fields >= 0 &&
        _current_field_index >= _num_nested_fields) {
      _current_field = NULL;
      
    } else {
      _current_field = _current_parent->get_nested_field(_current_field_index);
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
    // Oops, didn't pack or unpack enough values.
    _pack_error = true;

  } else if (_mode == M_unpack && _push_marker != 0 && 
             _unpack_p != _push_marker) {
    // Didn't unpack the right number of values.
    _pack_error = true;
  }

  if (_stack.empty()) {
    // Unbalanced pop().
    _pack_error = true;

  } else {
    if (_mode == M_pack) {
      size_t length_bytes = _current_parent->get_num_length_bytes();
      if (length_bytes != 0) {
        // Now go back and fill in the length of the array.
        size_t length = _pack_data.get_length() - _push_marker - length_bytes;
        if (length_bytes == 4) {
          DCPackerInterface::do_pack_uint32
            (_pack_data.get_rewrite_pointer(_push_marker, 4), length);
        } else {
          DCPackerInterface::do_pack_uint16
            (_pack_data.get_rewrite_pointer(_push_marker, 2), length);
        }
      }
    }

    _current_field = _current_parent;
    _current_parent = _stack.back()._current_parent;
    _current_field_index = _stack.back()._current_field_index;
    _push_marker = _stack.back()._push_marker;
    _num_nested_fields = (_current_parent == NULL) ? 0 : _current_parent->get_num_nested_fields();
    _stack.pop_back();
  }

  advance();
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCPacker::pack_object
//       Access: Published
//  Description: Packs the Python object of whatever type into the
//               packer.  Each numeric object and string object maps
//               to the corresponding pack_value() call; a tuple or
//               sequence maps to a push() followed by all of the
//               tuple's contents followed by a pop().
////////////////////////////////////////////////////////////////////
void DCPacker::
pack_object(PyObject *object) {
  nassertv(_mode == M_pack);
  PyObject *str = PyObject_Str(object);
  Py_DECREF(str);

  if (PyInt_Check(object)) {
    pack_int(PyInt_AS_LONG(object));

  } else if (PyFloat_Check(object)) {
    pack_double(PyFloat_AS_DOUBLE(object));

  } else if (PyLong_Check(object)) {
    pack_int64(PyLong_AsLongLong(object));

  } else if (PyString_Check(object) || PyUnicode_Check(object)) {
    char *buffer;
    int length;
    PyString_AsStringAndSize(object, &buffer, &length);
    if (buffer) {
      pack_string(string(buffer, length));
    }

  } else if (PySequence_Check(object)) {
    push();
    int size = PySequence_Size(object);
    for (int i = 0; i < size; i++) {
      PyObject *element = PySequence_GetItem(object, i);
      pack_object(element);
      Py_DECREF(element);
    }
    pop();
  }
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_object
//       Access: Published
//  Description: Unpacks a Python object of the appropriate type from
//               the stream for the current field.  This may be an
//               integer or a string for a simple field object; if the
//               current field represents a list of fields it will be
//               a tuple.
////////////////////////////////////////////////////////////////////
PyObject *DCPacker::
unpack_object() {
  PyObject *object = NULL;

  DCPackType pack_type = get_pack_type();

  switch (pack_type) {
  case PT_double:
    {
      double value = unpack_double();
      object = PyFloat_FromDouble(value);
    }
    break;
      
  case PT_int:
    {
      int value = unpack_int();
      object = PyInt_FromLong(value);
    }
    break;
      
  case PT_uint:
    {
      unsigned int value = unpack_uint();
      if (value & 0x80000000) {
        object = PyLong_FromUnsignedLong(value);
      } else {
        object = PyInt_FromLong(value);
      }
    }
    break;
      
  case PT_int64:
    {
      PN_int64 value = unpack_int64();
      object = PyLong_FromLongLong(value);
    }
    break;
      
  case PT_uint64:
    {
      PN_uint64 value = unpack_uint64();
      object = PyLong_FromUnsignedLongLong(value);
    }
    break;

  case PT_string:
    {
      string str = unpack_string();
      object = PyString_FromStringAndSize(str.data(), str.size());
    }
    break;

  default:
    {
      // First, build up a list from the nested objects.
      object = PyList_New(0);

      push();
      while (more_nested_fields()) {
        PyObject *element = unpack_object();
        PyList_Append(object, element);
        Py_DECREF(element);
      }
      pop();

      if (pack_type != PT_array) {
        // For these other kinds of objects, we'll convert the list
        // into a tuple.
        PyObject *tuple = PyList_AsTuple(object);
        Py_DECREF(object);
        object = tuple;
      }
    }
    break;
  }

  return object;
}
#endif  // HAVE_PYTHON


////////////////////////////////////////////////////////////////////
//     Function: DCPacker::parse_and_pack
//       Access: Published
//  Description: Parses an object's value according to the DC file
//               syntax (e.g. as a default value string) and packs it.
//               Returns true on success, false on a parse error.
////////////////////////////////////////////////////////////////////
bool DCPacker::
parse_and_pack(const string &formatted_object) {
  istringstream strm(formatted_object);
  return parse_and_pack(strm);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::parse_and_pack
//       Access: Published
//  Description: Parses an object's value according to the DC file
//               syntax (e.g. as a default value string) and packs it.
//               Returns true on success, false on a parse error.
////////////////////////////////////////////////////////////////////
bool DCPacker::
parse_and_pack(istream &in) {
  dc_init_parser_parameter_value(in, "parse_and_pack", *this);
  dcyyparse();
  dc_cleanup_parser();

  return (dc_error_count() == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_and_format
//       Access: Published
//  Description: Unpacks an object and formats its value into a syntax
//               suitable for parsing in the dc file (e.g. as a
//               default value), or as an input to parse_object.
////////////////////////////////////////////////////////////////////
string DCPacker::
unpack_and_format() {
  ostringstream strm;
  unpack_and_format(strm);
  return strm.str();
}

////////////////////////////////////////////////////////////////////
//     Function: DCPacker::unpack_and_format
//       Access: Published
//  Description: Unpacks an object and formats its value into a syntax
//               suitable for parsing in the dc file (e.g. as a
//               default value), or as an input to parse_object.
////////////////////////////////////////////////////////////////////
void DCPacker::
unpack_and_format(ostream &out) {
  DCPackType pack_type = get_pack_type();

  switch (pack_type) {
  case PT_invalid:
    out << "<invalid>";
    break;

  case PT_double:
    out << unpack_double();
    break;
      
  case PT_int:
    out << unpack_int();
    break;
      
  case PT_uint:
    out << unpack_uint();
    break;
      
  case PT_int64:
    out << unpack_int64();
    break;
      
  case PT_uint64:
    out << unpack_uint64();
    break;

  case PT_string:
    out << '"' << unpack_string() << '"';
    break;

  default:
    {
      switch (pack_type) {
      case PT_array:
        out << '[';
        break;

      case PT_field:
        out << '(';
        break;

      case PT_struct:
      default:
        out << '{';
        abort();
        break;
      }

      push();
      while (more_nested_fields()) {
        unpack_and_format(out);

        if (more_nested_fields()) {
          out << ", ";
        }
      }
      pop();

      switch (pack_type) {
      case PT_array:
        out << ']';
        break;

      case PT_field:
        out << ')';
        break;

      case PT_struct:
      default:
        out << '}';
        break;
      }
    }
    break;
  }
}
