// Filename: dcAtomicField.cxx
// Created by:  drose (05Oct00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "dcAtomicField.h"
#include "hashGenerator.h"
#include "dcindent.h"

#include <math.h>

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCAtomicField::ElementType::
ElementType() {
  _type = ST_invalid;
  _divisor = 1;
  _has_default_value = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::set_default_value
//       Access: Public
//  Description: Stores the indicated value as the default value for
//               this element.
//
//               Returns true if the element type reasonably accepts a
//               default value of numeric type, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::ElementType::
set_default_value(double num) {
  switch (_type) {
    // Only fields of these types accept numbers.
  case ST_int8:
  case ST_int16:
  case ST_int32:
  case ST_uint8:
  case ST_uint16:
  case ST_uint32:
  case ST_float64:
    break;

  default:
    return false;
  }

  string formatted;
  if (!format_default_value(num, formatted)) {
    return false;
  }

  _default_value = formatted;
  _has_default_value = true;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::set_default_value
//       Access: Public
//  Description: Stores the indicated value as the default value for
//               this element.
//
//               Returns true if the element type reasonably accepts a
//               default value of string type, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::ElementType::
set_default_value(const string &str) {
  if (_type != ST_string && _type != ST_blob && _type != ST_blob32) {
    // Only fields of type string or blob accept quoted strings.
    return false;
  }

  _default_value = str;
  _has_default_value = true;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::set_default_value_literal
//       Access: Public
//  Description: Explicitly sets the default value to the given
//               pre-formatted string.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::ElementType::
set_default_value_literal(const string &str) {
  _default_value = str;
  _has_default_value = true;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::add_default_value
//       Access: Public
//  Description: Appends the indicated value as the next array element
//               value for the default value for this type.
//
//               Returns true if the element type reasonably accepts a
//               default value of numeric type, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::ElementType::
add_default_value(double num) {
  string formatted;
  if (!format_default_value(num, formatted)) {
    return false;
  }

  _default_value += formatted;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::add_default_value
//       Access: Public
//  Description: Appends the indicated value as the next array element
//               value for the default value for this type.
//
//               Returns true if the element type reasonably accepts a
//               default value of numeric type, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::ElementType::
add_default_value(const string &str) {
  string formatted;
  if (!format_default_value(str, formatted)) {
    return false;
  }

  _default_value += formatted;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::add_default_value_literal
//       Access: Public
//  Description: Appends the indicated value as the next array element
//               value for the default value for this type.
//
//               Returns true if the element type reasonably accepts a
//               default value of numeric type, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::ElementType::
add_default_value_literal(const string &str) {
  if (_type != ST_blob && _type != ST_blob32) {
    // Only blobs can have literal hex strings nested within arrays.
    return false;
  }

  _default_value += str;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::end_array
//       Access: Public
//  Description: Called by the parser after a number of calls to
//               add_default_value(), to indicate the array has been
//               completed.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::ElementType::
end_array() {
  switch (_type) {
  case ST_int8array:
  case ST_int16array:
  case ST_int32array:
  case ST_uint8array:
  case ST_uint16array:
  case ST_uint32array:
  case ST_blob:
  case ST_blob32:
    // These types accept arrays.
    return true;

  case ST_uint32uint8array:
    {
      // In this special case type, we collapse every other 32-bit
      // value down to an 8-bit value after formatting.
      string new_value;
      size_t p = 0;
      while (p < _default_value.size()) {
        // We should have at least 8 bytes for each two elements.  If
        // we don't, maybe the user gave us an odd number of elements.
        if (p + 8 > _default_value.size()) {
          return false;
        }
        new_value += _default_value.substr(p, 5);
        p += 8;
      }

      _default_value = new_value;
      return true;
    }

  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void DCAtomicField::ElementType::
output(ostream &out, bool brief) const {
  out << _type;
  if (_divisor != 1) {
    out << " / " << _divisor;
  }
  if (!brief) {
    if (!_name.empty()) {
      out << " " << _name;
    }
    if (_has_default_value) {
      out << " = <" << hex;
      string::const_iterator si;
      for (si = _default_value.begin(); si != _default_value.end(); ++si) {
        out << setw(2) << setfill('0') << (int)(unsigned char)(*si);
      }
      out << dec << ">";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::format_default_value
//       Access: Private
//  Description: Formats the indicated default value to a sequence of
//               bytes, according to the element type.  Returns true
//               if the element type reasonably accepts a number,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::ElementType::
format_default_value(double num, string &formatted) const {
  double real_value = num * _divisor;
  int int_value = (int)floor(real_value + 0.5);

  switch (_type) {
  case ST_int8:
  case ST_uint8:
  case ST_int8array:
  case ST_uint8array:
  case ST_blob:
  case ST_blob32:
    formatted = string(1, (char)(int_value & 0xff));
    break;

  case ST_int16:
  case ST_uint16:
  case ST_int16array:
  case ST_uint16array:
    formatted =
      string(1, (char)(int_value & 0xff)) +
      string(1, (char)((int_value >> 8) & 0xff));
    break;

  case ST_int32:
  case ST_uint32:
  case ST_int32array:
  case ST_uint32array:
  case ST_uint32uint8array:
    formatted =
      string(1, (char)(int_value & 0xff)) +
      string(1, (char)((int_value >> 8) & 0xff)) +
      string(1, (char)((int_value >> 16) & 0xff)) +
      string(1, (char)((int_value >> 24) & 0xff));
    break;

  case ST_int64:
    // We don't fully support default values for int64.  The
    // high-order 32 bits cannot be specified.
    formatted =
      string(1, (char)(int_value & 0xff)) +
      string(1, (char)((int_value >> 8) & 0xff)) +
      string(1, (char)((int_value >> 16) & 0xff)) +
      string(1, (char)((int_value >> 24) & 0xff)) +
      ((int_value & 0x80000000) != 0 ? string(4, '\xff') : string(4, '\0'));
    break;

  case ST_uint64:
    // We don't fully support default values for int64.  The
    // high-order 32 bits cannot be specified.
    formatted =
      string(1, (char)(int_value & 0xff)) +
      string(1, (char)((int_value >> 8) & 0xff)) +
      string(1, (char)((int_value >> 16) & 0xff)) +
      string(1, (char)((int_value >> 24) & 0xff)) +
      string(4, '\0');
    break;

  case ST_float64:
    // This may not be fully portable.
    formatted = string((char *)&real_value, 8);
#ifdef WORDS_BIGENDIAN
    {
      // Reverse the byte ordering for big-endian machines.
      string str;
      str.reserve(8);

      int length = str.length();
      for (size_t i = 0; i < 8; i++) {
        str += formatted[length - 1 - i];
      }
      formatted = str;
    }
#endif
    break;

  case ST_string:
    // It doesn't make sense to assign a numeric default value to a
    // string.
    return false;

  case ST_invalid:
    break;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::format_default_value
//       Access: Private
//  Description: Formats the indicated default value to a sequence of
//               bytes, according to the element type.  Returns true
//               if the element type reasonably accepts a string,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::ElementType::
format_default_value(const string &str, string &formatted) const {
  switch (_type) {
  case ST_string:
  case ST_blob:
  case ST_blob32:
    {
      int length = str.length();
      formatted =
        string(1, (char)(length & 0xff)) +
        string(1, (char)((length >> 8) & 0xff)) +
        str;
    }
    break;

  default:
    // It doesn't make sense to assign a string default to a number.
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::as_atomic_field
//       Access: Public, Virtual
//  Description: Returns the same field pointer converted to an atomic
//               field pointer, if this is in fact an atomic field;
//               otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
DCAtomicField *DCAtomicField::
as_atomic_field() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_num_elements
//       Access: Public
//  Description: Returns the number of elements of the atomic field.
////////////////////////////////////////////////////////////////////
int DCAtomicField::
get_num_elements() const {
  return _elements.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_element_type
//       Access: Public
//  Description: Returns the numeric type of the nth element of the
//               field.
////////////////////////////////////////////////////////////////////
DCSubatomicType DCAtomicField::
get_element_type(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), ST_invalid);
  return _elements[n]._type;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_element_name
//       Access: Public
//  Description: Returns the name of the nth element of the field.
//               This name is strictly for documentary purposes; it
//               does not generally affect operation.  If a name is
//               not specified, this will be the empty string.
////////////////////////////////////////////////////////////////////
string DCAtomicField::
get_element_name(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), string());
  return _elements[n]._name;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_element_divisor
//       Access: Public
//  Description: Returns the divisor associated with the nth element
//               of the field.  This implements an implicit
//               fixed-point system; floating-point values are to be
//               multiplied by this value before encoding into a
//               packet, and divided by this number after decoding.
////////////////////////////////////////////////////////////////////
int DCAtomicField::
get_element_divisor(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), 1);
  return _elements[n]._divisor;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_element_default
//       Access: Public
//  Description: Returns the pre-formatted default value associated
//               with the nth element of the field.  This is only
//               valid if has_element_default() returns true, in which
//               case this string represents the bytes that should be
//               assigned to the field as a default value.
//
//               If the element is an array-type element, the returned
//               value will include the two-byte length preceding the
//               array data.
////////////////////////////////////////////////////////////////////
string DCAtomicField::
get_element_default(int n) const {
  nassertr(has_element_default(n), string());
  nassertr(n >= 0 && n < (int)_elements.size(), string());

  string default_value = _elements[n]._default_value;

  switch (_elements[n]._type) {
  case ST_int8array:
  case ST_int16array:
  case ST_int32array:
  case ST_uint8array:
  case ST_uint16array:
  case ST_uint32array:
  case ST_uint32uint8array:
  case ST_blob:
  case ST_blob32:
  case ST_string:
    // These array types also want an implicit length.
    {
      int length = default_value.length();
      default_value =
        string(1, (char)(length & 0xff)) +
        string(1, (char)((length >> 8) & 0xff)) +
        default_value;
    }
    break;

  default:
    break;
  }

  return default_value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::has_element_default
//       Access: Public
//  Description: Returns true if the nth element of the field has a
//               default value specified, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
has_element_default(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), false);
  return _elements[n]._has_default_value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_required
//       Access: Public
//  Description: Returns true if the "required" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_required() const {
  return (_flags & F_required) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_broadcast
//       Access: Public
//  Description: Returns true if the "broadcast" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_broadcast() const {
  return (_flags & F_broadcast) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_p2p
//       Access: Public
//  Description: Returns true if the "p2p" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_p2p() const {
  return (_flags & F_p2p) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_ram
//       Access: Public
//  Description: Returns true if the "ram" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_ram() const {
  return (_flags & F_ram) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_db
//       Access: Public
//  Description: Returns true if the "db" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_db() const {
  return (_flags & F_db) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_clsend
//       Access: Public
//  Description: Returns true if the "clsend" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_clsend() const {
  return (_flags & F_clsend) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_clrecv
//       Access: Public
//  Description: Returns true if the "clrecv" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_clrecv() const {
  return (_flags & F_clrecv) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_ownsend
//       Access: Public
//  Description: Returns true if the "ownsend" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_ownsend() const {
  return (_flags & F_ownsend) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_airecv
//       Access: Public
//  Description: Returns true if the "airecv" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_airecv() const {
  return (_flags & F_airecv) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCAtomicField::
DCAtomicField() {
  _number = 0;
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::write
//       Access: Public, Virtual
//  Description: Generates a parseable description of the object to
//               the indicated output stream.
////////////////////////////////////////////////////////////////////
void DCAtomicField::
write(ostream &out, bool brief, int indent_level) const {
  indent(out, indent_level)
    << _name << "(";

  if (!_elements.empty()) {
    Elements::const_iterator ei = _elements.begin();
    (*ei).output(out, brief);
    ++ei;
    while (ei != _elements.end()) {
      out << ", ";
      (*ei).output(out, brief);
      ++ei;
    }
  }
  out << ")";

  if ((_flags & F_required) != 0) {
    out << " required";
  }
  if ((_flags & F_broadcast) != 0) {
    out << " broadcast";
  }
  if ((_flags & F_p2p) != 0) {
    out << " p2p";
  }
  if ((_flags & F_ram) != 0) {
    out << " ram";
  }
  if ((_flags & F_db) != 0) {
    out << " db";
  }
  if ((_flags & F_clsend) != 0) {
    out << " clsend";
  }
  if ((_flags & F_clrecv) != 0) {
    out << " clrecv";
  }
  if ((_flags & F_ownsend) != 0) {
    out << " ownsend";
  }
  if ((_flags & F_airecv) != 0) {
    out << " airecv";
  }

  out << ";";
  if (!brief) {
    out << "  // field " << _number;
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this field into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCAtomicField::
generate_hash(HashGenerator &hashgen) const {
  DCField::generate_hash(hashgen);

  hashgen.add_int(_elements.size());
  Elements::const_iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    const ElementType &element = (*ei);
    hashgen.add_int(element._type);
    hashgen.add_int(element._divisor);
  }
  hashgen.add_int(_flags);
}
