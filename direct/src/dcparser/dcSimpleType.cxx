// Filename: dcSimpleType.cxx
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

#include "dcSimpleType.h"
#include "dcPackData.h"
#include "hashGenerator.h"

DCSimpleType::NestedFieldMap DCSimpleType::_nested_field_map;
DCSimpleType::Uint32Uint8Type *DCSimpleType::_uint32uint8_type = NULL;

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCSimpleType::
DCSimpleType(DCSubatomicType type, int divisor) :
  _type(type),
  _divisor(divisor)
{
  // Check for one of the built-in array types.  For these types, we
  // must present a packing interface that has a variable number of
  // nested fields of the appropriate type.
  switch (_type) {
  case ST_int8array:
    _nested_type = ST_int8;
    _is_array = true;
    break;

  case ST_int16array:
    _nested_type = ST_int16;
    _is_array = true;
    break;

  case ST_int32array:
    _nested_type = ST_int32;
    _is_array = true;
    break;

  case ST_uint8array:
    _nested_type = ST_uint8;
    _is_array = true;
    break;

  case ST_uint16array:
    _nested_type = ST_uint16;
    _is_array = true;
    break;

  case ST_uint32array:
    _nested_type = ST_uint32;
    _is_array = true;
    break;

  case ST_uint32uint8array:
    _nested_type = ST_invalid;
    _is_array = true;
    break;

  case ST_blob:
  case ST_blob32:
  case ST_string: 
    // For these types, we will present an array interface as an array
    // of uint8, but we will also accept a set_value() with a string
    // parameter.
    _nested_type = ST_uint8;
    _is_array = true;
    break;

  default:
    _nested_type = ST_invalid;
    _is_array = false;
  }

  if (_nested_type != ST_invalid) {
    _nested_field = create_nested_field(_nested_type, _divisor);

  } else if (_type == ST_uint32uint8array) {
    // This one is a special case.  We must create a special nested
    // type that accepts a uint32 followed by a uint8 for each
    // element.
    _nested_field = create_uint32uint8_type();

  } else {
    _nested_field = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::as_simple_type
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCSimpleType *DCSimpleType::
as_simple_type() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::make_copy
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCType *DCSimpleType::
make_copy() const {
  return new DCSimpleType(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::get_type
//       Access: Published
//  Description: Returns the particular subatomic type represented by
//               this instance.
////////////////////////////////////////////////////////////////////
DCSubatomicType DCSimpleType::
get_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::get_divisor
//       Access: Published
//  Description: Returns the divisor associated with this type.  This
//               is 1 by default, but if this is other than one it
//               represents the scale to apply when packing and
//               unpacking numeric values (to store fixed-point values
//               in an integer field).  It is only meaningful for
//               numeric-type fields.
////////////////////////////////////////////////////////////////////
int DCSimpleType::
get_divisor() const {
  return _divisor;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::set_divisor
//       Access: Published
//  Description: See get_divisor().
////////////////////////////////////////////////////////////////////
void DCSimpleType::
set_divisor(int divisor) {
  _divisor = divisor;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::has_nested_fields
//       Access: Public, Virtual
//  Description: Returns true if this field type has any nested fields
//               (and thus expects a push() .. pop() interface to the
//               DCPacker), or false otherwise.  If this returns true,
//               get_num_nested_fields() may be called to determine
//               how many nested fields are expected.
////////////////////////////////////////////////////////////////////
bool DCSimpleType::
has_nested_fields() const {
  return _is_array;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::get_num_nested_fields
//       Access: Public, Virtual
//  Description: Returns the number of nested fields required by this
//               field type.  These may be array elements or structure
//               elements.  The return value may be -1 to indicate the
//               number of nested fields is variable.
////////////////////////////////////////////////////////////////////
int DCSimpleType::
get_num_nested_fields() const {
  return _is_array ? -1 : 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::get_nested_field
//       Access: Public, Virtual
//  Description: Returns the DCPackerInterface object that represents
//               the nth nested field.  This may return NULL if there
//               is no such field (but it shouldn't do this if n is in
//               the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCSimpleType::
get_nested_field(int n) const {
  return _nested_field;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::get_length_bytes
//       Access: Public, Virtual
//  Description: If has_nested_fields() returns true, this should
//               return either 0, 2, or 4, indicating the number of
//               bytes this field's data should be prefixed with to
//               record its length.  This is respected by push() and
//               pop().
////////////////////////////////////////////////////////////////////
size_t DCSimpleType::
get_length_bytes() const {
  return _type == ST_blob32 ? 4 : 2;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::get_pack_type
//       Access: Public, Virtual
//  Description: Returns the type of value expected by this field, or
//               ST_invalid if this field cannot accept simple value
//               types.
////////////////////////////////////////////////////////////////////
DCSubatomicType DCSimpleType::
get_pack_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::pack_double
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCSimpleType::
pack_double(DCPackData &pack_data, double value) const {
  double real_value = value * _divisor;
  int int_value = (int)floor(real_value + 0.5);

  char buffer[8];

  switch (_type) {
  case ST_int8:
  case ST_uint8:
    buffer[0] = (char)(int_value & 0xff);
    pack_data.append_data(buffer, 1);
    break;

  case ST_int16:
  case ST_uint16:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    pack_data.append_data(buffer, 2);
    break;

  case ST_int32:
  case ST_uint32:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    buffer[2] = (char)((int_value >> 16) & 0xff);
    buffer[3] = (char)((int_value >> 24) & 0xff);
    pack_data.append_data(buffer, 4);
    break;

  case ST_int64:
  case ST_uint64:
    {
      PN_int64 int64_value = (PN_int64)floor(real_value + 0.5);
      buffer[0] = (char)(int64_value & 0xff);
      buffer[1] = (char)((int64_value >> 8) & 0xff);
      buffer[2] = (char)((int64_value >> 16) & 0xff);
      buffer[3] = (char)((int64_value >> 24) & 0xff);
      buffer[4] = (char)((int64_value >> 32) & 0xff);
      buffer[5] = (char)((int64_value >> 40) & 0xff);
      buffer[6] = (char)((int64_value >> 48) & 0xff);
      buffer[7] = (char)((int64_value >> 56) & 0xff);
      pack_data.append_data(buffer, 8);
    }
    break;

  case ST_float64:
#ifdef WORDS_BIGENDIAN
    {
      // Reverse the byte ordering for big-endian machines.
      char *p = (char *)real_value;
      for (size_t i = 0; i < 8; i++) {
        buffer[i] = p[7 - i];
      }
    }
#else
    memcpy(buffer, &real_value, 8);
#endif
    pack_data.append_data(buffer, 8);
    break;

  default:
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::pack_int
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCSimpleType::
pack_int(DCPackData &pack_data, int value) const {
  int int_value = value * _divisor;

  char buffer[8];

  switch (_type) {
  case ST_int8:
  case ST_uint8:
    buffer[0] = (char)(int_value & 0xff);
    pack_data.append_data(buffer, 1);
    break;

  case ST_int16:
  case ST_uint16:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    pack_data.append_data(buffer, 2);
    break;

  case ST_int32:
  case ST_uint32:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    buffer[2] = (char)((int_value >> 16) & 0xff);
    buffer[3] = (char)((int_value >> 24) & 0xff);
    pack_data.append_data(buffer, 4);
    break;

  case ST_int64:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    buffer[2] = (char)((int_value >> 16) & 0xff);
    buffer[3] = (char)((int_value >> 24) & 0xff);
    if ((int_value & 0x80000000) != 0) {
      buffer[4] = buffer[5] = buffer[6] = buffer[7] = (char)0xff;
    } else {
      buffer[4] = buffer[5] = buffer[6] = buffer[7] = (char)0;
    }
    pack_data.append_data(buffer, 8);
    break;

  case ST_uint64:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    buffer[2] = (char)((int_value >> 16) & 0xff);
    buffer[3] = (char)((int_value >> 24) & 0xff);
    buffer[4] = buffer[5] = buffer[6] = buffer[7] = (char)0;
    pack_data.append_data(buffer, 8);
    break;

  case ST_float64:
#ifdef WORDS_BIGENDIAN
    {
      // Reverse the byte ordering for big-endian machines.
      double real_value = int_value;
      char *p = (char *)real_value;
      for (size_t i = 0; i < 8; i++) {
        buffer[i] = p[7 - i];
      }
    }
#else
    {
      double real_value = int_value;
      memcpy(buffer, &real_value, 8);
    }
#endif
    pack_data.append_data(buffer, 8);
    break;

  default:
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::pack_int64
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCSimpleType::
pack_int64(DCPackData &pack_data, PN_int64 value) const {
  PN_int64 int_value = value * _divisor;

  char buffer[8];

  switch (_type) {
  case ST_int8:
  case ST_uint8:
    buffer[0] = (char)(int_value & 0xff);
    pack_data.append_data(buffer, 1);
    break;

  case ST_int16:
  case ST_uint16:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    pack_data.append_data(buffer, 2);
    break;

  case ST_int32:
  case ST_uint32:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    buffer[2] = (char)((int_value >> 16) & 0xff);
    buffer[3] = (char)((int_value >> 24) & 0xff);
    pack_data.append_data(buffer, 4);
    break;

  case ST_int64:
  case ST_uint64:
    {
      buffer[0] = (char)(int_value & 0xff);
      buffer[1] = (char)((int_value >> 8) & 0xff);
      buffer[2] = (char)((int_value >> 16) & 0xff);
      buffer[3] = (char)((int_value >> 24) & 0xff);
      buffer[4] = (char)((int_value >> 32) & 0xff);
      buffer[5] = (char)((int_value >> 40) & 0xff);
      buffer[6] = (char)((int_value >> 48) & 0xff);
      buffer[7] = (char)((int_value >> 56) & 0xff);
      pack_data.append_data(buffer, 8);
    }
    break;

  case ST_float64:
#ifdef WORDS_BIGENDIAN
    {
      // Reverse the byte ordering for big-endian machines.
      double real_value = int_value;
      char *p = (char *)real_value;
      for (size_t i = 0; i < 8; i++) {
        buffer[i] = p[7 - i];
      }
    }
#else
    {
      double real_value = int_value;
      memcpy(buffer, &real_value, 8);
    }
#endif
    pack_data.append_data(buffer, 8);
    break;

  default:
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::pack_string
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCSimpleType::
pack_string(DCPackData &pack_data, const string &value) const {
  char buffer[4];

  switch (_type) {
  case ST_string:
  case ST_blob:
    {
      int length = value.length();
      buffer[0] = (char)(length & 0xff);
      buffer[1] = (char)((length >> 8) & 0xff);
      pack_data.append_data(buffer, 2);
      pack_data.append_data(value.data(), length);
    }
    break;

  case ST_blob32:
    {
      int length = value.length();
      buffer[0] = (char)(length & 0xff);
      buffer[1] = (char)((length >> 8) & 0xff);
      buffer[2] = (char)((length >> 16) & 0xff);
      buffer[3] = (char)((length >> 24) & 0xff);
      pack_data.append_data(buffer, 4);
      pack_data.append_data(value.data(), length);
    }
    break;

  default:
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DCSimpleType::
output(ostream &out, const string &parameter_name, bool brief) const {
  out << _type;
  if (_divisor != 1) {
    out << " / " << _divisor;
  }
  if (!brief && !parameter_name.empty()) {
    out << " " << parameter_name;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this type into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCSimpleType::
generate_hash(HashGenerator &hashgen) const {
  DCType::generate_hash(hashgen);

  hashgen.add_int(_type);
  hashgen.add_int(_divisor);
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::create_nested_field
//       Access: Private, Static
//  Description: Creates the one instance of the DCSimpleType
//               corresponding to this combination of type and divisor
//               if it is not already created.
////////////////////////////////////////////////////////////////////
DCSimpleType *DCSimpleType::
create_nested_field(DCSubatomicType type, int divisor) {
  DivisorMap divisor_map = _nested_field_map[type];
  DivisorMap::iterator di;
  di = divisor_map.find(divisor);
  if (di != divisor_map.end()) {
    return (*di).second;
  }

  DCSimpleType *nested_field = new DCSimpleType(type, divisor);
  divisor_map[divisor] = nested_field;
  return nested_field;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::create_uint32uint8_type
//       Access: Private, Static
//  Description: Creates the one instance of the Uint32Uint8Type
//               object if it is not already created.
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCSimpleType::
create_uint32uint8_type() {
  if (_uint32uint8_type == NULL) {
    _uint32uint8_type = new Uint32Uint8Type;
  }
  return _uint32uint8_type;
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::pack_arg
//       Access: Public, Virtual
//  Description: Packs the Python object into the datagram, appending
//               to the end of the datagram.
////////////////////////////////////////////////////////////////////
void DCSimpleType::
pack_arg(Datagram &datagram, PyObject *item) const {
  do_pack_arg(datagram, item, _type);
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::unpack_arg
//       Access: Public, Virtual
//  Description: Unpacks a Python object from the datagram, beginning
//               at the current point in the interator, and returns a
//               new reference, or NULL if there was not enough data
//               in the datagram.
////////////////////////////////////////////////////////////////////
PyObject *DCSimpleType::
unpack_arg(DatagramIterator &iterator) const {
  return do_unpack_arg(iterator, _type);
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::do_pack_arg
//       Access: Private
//  Description: Packs the Python object into the datagram, appending
//               to the end of the datagram.
////////////////////////////////////////////////////////////////////
void DCSimpleType::
do_pack_arg(Datagram &datagram, PyObject *item, DCSubatomicType type) const {
  char *str;
  int size;

  // Check for an array type.  These are handled recursively.
  DCSubatomicType array_subtype;
  int num_bytes = 0;
  switch (type) {
  case ST_int16array:
    array_subtype = ST_int16;
    num_bytes = 2;
    break;

  case ST_int32array:
    array_subtype = ST_int32;
    num_bytes = 4;
    break;

  case ST_uint16array:
    array_subtype = ST_uint16;
    num_bytes = 2;
    break;

  case ST_uint32array:
    array_subtype = ST_uint32;
    num_bytes = 4;
    break;

  case ST_int8array:
    array_subtype = ST_int8;
    num_bytes = 1;
    break;

  case ST_uint8array:
    array_subtype = ST_uint8;
    num_bytes = 1;
    break;

  case ST_uint32uint8array:
    array_subtype = ST_uint32;
    num_bytes = 5;
    break;

  default:
    array_subtype = ST_invalid;
  }

  if (array_subtype != ST_invalid) {
    int size = PySequence_Size(item);
    datagram.add_uint16(size * num_bytes);
    if (type == ST_uint32uint8array) {
      // This one is a special case: an array of tuples.
      for (int i = 0; i < size; i++) {
        PyObject *tuple = PySequence_GetItem(item, i);
        do_pack_arg(datagram, PyTuple_GetItem(tuple, 0), ST_uint32);
        do_pack_arg(datagram, PyTuple_GetItem(tuple, 1), ST_uint8);
        Py_DECREF(tuple);
      }
    } else {
      for (int i = 0; i < size; i++) {
        PyObject *element = PySequence_GetItem(item, i);
        do_pack_arg(datagram, element, array_subtype);
        Py_DECREF(element);
      }
    }

    return;
  }

  if (_divisor == 1) {
    switch (type) {
    case ST_int8:
      datagram.add_int8(PyInt_AsLong(item));
      break;

    case ST_int16:
      datagram.add_int16(PyInt_AsLong(item));
      break;

    case ST_int32:
      datagram.add_int32(PyInt_AsLong(item));
      break;

    case ST_int64:
      datagram.add_int64(PyLong_AsLongLong(item));
      break;

    case ST_uint8:
      datagram.add_uint8(PyInt_AsLong(item));
      break;

    case ST_uint16:
      datagram.add_uint16(PyInt_AsLong(item));
      break;

    case ST_uint32:
      datagram.add_uint32(PyInt_AsLong(item));
      break;

    case ST_uint64:
      datagram.add_uint64(PyLong_AsUnsignedLongLong(item));
      break;

    case ST_float64:
      datagram.add_float64(PyFloat_AsDouble(item));
      break;

    case ST_string:
    case ST_blob:
      PyString_AsStringAndSize(item, &str, &size);
      datagram.add_string(string(str, size));
      break;
      
    case ST_blob32:
      PyString_AsStringAndSize(item, &str, &size);
      datagram.add_string32(string(str, size));
      break;

    default:
      break;
    }

  } else {
    switch (type) {
    case ST_int8:
      datagram.add_int8((PN_int8)floor(PyFloat_AsDouble(item) * _divisor + 0.5));
      break;

    case ST_int16:
      datagram.add_int16((PN_int16)floor(PyFloat_AsDouble(item) * _divisor + 0.5));
      break;

    case ST_int32:
      datagram.add_int32((PN_int32)floor(PyFloat_AsDouble(item) * _divisor + 0.5));
      break;

    case ST_int64:
      datagram.add_int64((PN_int64)floor(PyFloat_AsDouble(item) * _divisor + 0.5));
      break;

    case ST_uint8:
      datagram.add_uint8((PN_uint8)floor(PyFloat_AsDouble(item) * _divisor + 0.5));
      break;

    case ST_uint16:
      datagram.add_uint16((PN_uint16)floor(PyFloat_AsDouble(item) * _divisor + 0.5));
      break;

    case ST_uint32:
      datagram.add_uint32((PN_uint32)floor(PyFloat_AsDouble(item) * _divisor + 0.5));
      break;

    case ST_uint64:
      datagram.add_uint64((PN_uint64)floor(PyFloat_AsDouble(item) * _divisor + 0.5));
      break;

    case ST_float64:
      datagram.add_float64(PyFloat_AsDouble(item) * _divisor);
      break;

    case ST_string:
    case ST_blob:
      PyString_AsStringAndSize(item, &str, &size);
      datagram.add_string(string(str, size));
      break;
      
    case ST_blob32:
      PyString_AsStringAndSize(item, &str, &size);
      datagram.add_string32(string(str, size));
      break;

    default:
      break;
    }
  }
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::do_unpack_arg
//       Access: Private
//  Description: Unpacks a Python object from the datagram, beginning
//               at the current point in the interator, and returns a
//               new reference, or NULL if there was not enough data
//               in the datagram.
////////////////////////////////////////////////////////////////////
PyObject *DCSimpleType::
do_unpack_arg(DatagramIterator &iterator, DCSubatomicType type) const {
  string str;

  // Check for an array type.  These are handled recursively.
  DCSubatomicType array_subtype;
  int num_bytes = 0;
  switch (type) {
  case ST_int16array:
    array_subtype = ST_int16;
    num_bytes = 2;
    break;

  case ST_int32array:
    array_subtype = ST_int32;
    num_bytes = 4;
    break;

  case ST_uint16array:
    array_subtype = ST_uint16;
    num_bytes = 2;
    break;

  case ST_uint32array:
    array_subtype = ST_uint32;
    num_bytes = 4;
    break;

  case ST_int8array:
    array_subtype = ST_int8;
    num_bytes = 1;
    break;

  case ST_uint8array:
    array_subtype = ST_uint8;
    num_bytes = 1;
    break;

  case ST_uint32uint8array:
    array_subtype = ST_uint32;
    num_bytes = 5;
    break;

  default:
    array_subtype = ST_invalid;
  }

  if (array_subtype != ST_invalid) {
    int size_bytes = iterator.get_uint16();
    int size = size_bytes / num_bytes;
    nassertr(size * num_bytes == size_bytes, NULL);

    PyObject *list = PyList_New(size);
    if (type == ST_uint32uint8array) {
      // This one is a special case: an array of tuples.
      for (int i = 0; i < size; i++) {
        PyObject *a = do_unpack_arg(iterator, ST_uint32);
        PyObject *b = do_unpack_arg(iterator, ST_uint8);
        PyObject *tuple = PyTuple_New(2);
        PyTuple_SET_ITEM(tuple, 0, a);
        PyTuple_SET_ITEM(tuple, 1, b);
        PyList_SET_ITEM(list, i, tuple);
      }
    } else {
      for (int i = 0; i < size; i++) {
        PyObject *element = do_unpack_arg(iterator, array_subtype);
        PyList_SET_ITEM(list, i, element);
      }
    }

    return list;
  }

  if (_divisor == 1) {
    switch (type) {
    case ST_int8:
      return PyInt_FromLong(iterator.get_int8());

    case ST_int16:
      return PyInt_FromLong(iterator.get_int16());

    case ST_int32:
      return PyInt_FromLong(iterator.get_int32());

    case ST_int64:
      return PyLong_FromLongLong(iterator.get_int64());

    case ST_uint8:
      return PyInt_FromLong(iterator.get_uint8());

    case ST_uint16:
      return PyInt_FromLong(iterator.get_uint16());

    case ST_uint32:
      return PyInt_FromLong(iterator.get_uint32());

    case ST_uint64:
      return PyLong_FromUnsignedLongLong(iterator.get_uint64());

    case ST_float64:
      return PyFloat_FromDouble(iterator.get_float64());

    case ST_string:
    case ST_blob:
      str = iterator.get_string();
      return PyString_FromStringAndSize(str.data(), str.size());
      
    case ST_blob32:
      str = iterator.get_string32();
      return PyString_FromStringAndSize(str.data(), str.size());

    default:
      return Py_BuildValue("");
    }

  } else {
    switch (type) {
    case ST_int8:
      return PyFloat_FromDouble(iterator.get_int8() / (double)_divisor);

    case ST_int16:
      return PyFloat_FromDouble(iterator.get_int16() / (double)_divisor);

    case ST_int32:
      return PyFloat_FromDouble(iterator.get_int32() / (double)_divisor);

    case ST_int64:
      return PyFloat_FromDouble(iterator.get_int64() / (double)_divisor);

    case ST_uint8:
      return PyFloat_FromDouble(iterator.get_uint8() / (double)_divisor);

    case ST_uint16:
      return PyFloat_FromDouble(iterator.get_uint16() / (double)_divisor);

    case ST_uint32:
      return PyFloat_FromDouble(iterator.get_uint32() / (double)_divisor);

    case ST_uint64:
      return PyFloat_FromDouble(iterator.get_uint64() / (double)_divisor);

    case ST_float64:
      return PyFloat_FromDouble(iterator.get_float64() / (double)_divisor);

    case ST_string:
    case ST_blob:
      str = iterator.get_string();
      return PyString_FromStringAndSize(str.data(), str.size());
      
    case ST_blob32:
      str = iterator.get_string32();
      return PyString_FromStringAndSize(str.data(), str.size());

    default:
      return Py_BuildValue("");
    }
  }
}
#endif  // HAVE_PYTHON


////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::Uint32Uint8Type::Constructor
//       Access: Public
//  Description: This special packer interface is provided just to
//               implement uint32uint8array, which is a special kind
//               of array that consists of nested pairs of (uint32,
//               uint8) values.
////////////////////////////////////////////////////////////////////
DCSimpleType::Uint32Uint8Type::
Uint32Uint8Type() {
  _uint32_type = new DCSimpleType(ST_uint32);
  _uint8_type = new DCSimpleType(ST_uint8);
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::Uint32Uint8Type::has_nested_fields
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool DCSimpleType::Uint32Uint8Type::
has_nested_fields() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::Uint32Uint8Type::get_num_nested_fields
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int DCSimpleType::Uint32Uint8Type::
get_num_nested_fields() const {
  return 2;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleType::Uint32Uint8Type::get_nested_field
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCSimpleType::Uint32Uint8Type::
get_nested_field(int n) const {
  switch (n) {
  case 0:
    return _uint32_type;

  case 1:
    return _uint8_type;

  default:
    return NULL;
  }
}
