// Filename: dcField.cxx
// Created by:  drose (11Oct00)
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

#include "dcField.h"
#include "dcPacker.h"
#include "hashGenerator.h"
#include "dcmsgtypes.h"

////////////////////////////////////////////////////////////////////
//     Function: DCField::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCField::
DCField(const string &name) : DCPackerInterface(name) {
  _number = -1;
  _flags = 0;
  _has_default_value = false;
  _default_value_stale = true;

  _has_nested_fields = true;
  _num_nested_fields = 0;
  _pack_type = PT_field;

  _has_fixed_byte_size = true;
  _fixed_byte_size = 0;
  _has_fixed_structure = true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DCField::
~DCField() {
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::get_number
//       Access: Published
//  Description: Returns a unique index number associated with this
//               field.  This is defined implicitly when the .dc
//               file(s) are read.
////////////////////////////////////////////////////////////////////
int DCField::
get_number() const {
  return _number;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::as_field
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCField *DCField::
as_field() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::as_atomic_field
//       Access: Published, Virtual
//  Description: Returns the same field pointer converted to an atomic
//               field pointer, if this is in fact an atomic field;
//               otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
DCAtomicField *DCField::
as_atomic_field() {
  return (DCAtomicField *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::as_molecular_field
//       Access: Published, Virtual
//  Description: Returns the same field pointer converted to a
//               molecular field pointer, if this is in fact a
//               molecular field; otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
DCMolecularField *DCField::
as_molecular_field() {
  return (DCMolecularField *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::as_parameter
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCParameter *DCField::
as_parameter() {
  return (DCParameter *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::as_switch
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCSwitch *DCField::
as_switch() {
  return (DCSwitch *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::format_data
//       Access: Published
//  Description: Given a blob that represents the packed data for this
//               field, returns a string formatting it for human
//               consumption.  Returns empty string if there is an error.
////////////////////////////////////////////////////////////////////
string DCField::
format_data(const string &packed_data) {
  DCPacker packer;
  packer.set_unpack_data(packed_data);
  packer.begin_unpack(this);
  string result = packer.unpack_and_format();
  if (!packer.end_unpack()) {
    return string();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::parse_string
//       Access: Published
//  Description: Given a human-formatted string (for instance, as
//               returned by format_data(), above) that represents the
//               value of this field, parse the string and return the
//               corresponding packed data.  Returns empty string if
//               there is an error.
////////////////////////////////////////////////////////////////////
string DCField::
parse_string(const string &formatted_string) {
  DCPacker packer;
  packer.begin_pack(this);
  if (!packer.parse_and_pack(formatted_string)) {
    // Parse error.
    return string();
  }
  if (!packer.end_pack()) {
    // Data type mismatch.
    return string();
  }

  return packer.get_string();
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::validate_ranges
//       Access: Published
//  Description: Verifies that all of the packed values in the field
//               data are within the specified ranges and that there
//               are no extra bytes on the end of the record.  Returns
//               true if all fields are valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCField::
validate_ranges(const string &packed_data) const {
  DCPacker packer;
  packer.set_unpack_data(packed_data);
  packer.begin_unpack(this);
  packer.unpack_validate();
  if (!packer.end_unpack()) {
    return false;
  }

  return (packer.get_num_unpacked_bytes() == packed_data.length());
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::has_default_value
//       Access: Published
//  Description: Returns true if a default value has been explicitly
//               established for this field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCField::
has_default_value() const {
  return _has_default_value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::get_default_value
//       Access: Published
//  Description: Returns the default value for this field.  If a
//               default value has been explicitly set
//               (e.g. has_default_value() returns true), returns that
//               value; otherwise, returns an implicit default for the
//               field.
////////////////////////////////////////////////////////////////////
const string &DCField::
get_default_value() const {
  if (_default_value_stale) {
    ((DCField *)this)->refresh_default_value();
  }
  return _default_value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::is_required
//       Access: Published
//  Description: Returns true if the "required" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCField::
is_required() const {
  return (_flags & F_required) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::is_broadcast
//       Access: Published
//  Description: Returns true if the "broadcast" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCField::
is_broadcast() const {
  return (_flags & F_broadcast) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::is_p2p
//       Access: Published
//  Description: Returns true if the "p2p" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCField::
is_p2p() const {
  return (_flags & F_p2p) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::is_ram
//       Access: Published
//  Description: Returns true if the "ram" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCField::
is_ram() const {
  return (_flags & F_ram) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::is_db
//       Access: Published
//  Description: Returns true if the "db" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCField::
is_db() const {
  return (_flags & F_db) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::is_clsend
//       Access: Published
//  Description: Returns true if the "clsend" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCField::
is_clsend() const {
  return (_flags & F_clsend) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::is_clrecv
//       Access: Published
//  Description: Returns true if the "clrecv" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCField::
is_clrecv() const {
  return (_flags & F_clrecv) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::is_ownsend
//       Access: Published
//  Description: Returns true if the "ownsend" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCField::
is_ownsend() const {
  return (_flags & F_ownsend) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::is_airecv
//       Access: Published
//  Description: Returns true if the "airecv" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCField::
is_airecv() const {
  return (_flags & F_airecv) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::compare_flags
//       Access: Published
//  Description: Returns true if this field has the same flags
//               settings as the other field, false if some flags
//               differ.
////////////////////////////////////////////////////////////////////
bool DCField::
compare_flags(const DCField &other) const {
  return _flags == other._flags;
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCField::pack_args
//       Access: Published
//  Description: Packs the Python arguments from the indicated tuple
//               into the packer.  Returns true on success, false on
//               failure.
//
//               It is assumed that the packer is currently positioned
//               on this field.
////////////////////////////////////////////////////////////////////
bool DCField::
pack_args(DCPacker &packer, PyObject *sequence) const {
  nassertr(!packer.had_error(), false);
  nassertr(packer.get_current_field() == this, false);

  packer.pack_object(sequence);
  if (!packer.had_error()) {
    /*
    PyObject *str = PyObject_Str(sequence);
    cerr << "pack " << get_name() << PyString_AsString(str) << "\n";
    Py_DECREF(str);
    */

    return true;
  }

  if (!Notify::ptr()->has_assert_failed()) {
    PyObject *tuple = PySequence_Tuple(sequence);
    PyObject *str = PyObject_Str(tuple);
    
    ostringstream strm;
    if (packer.had_pack_error()) {
      strm << "Incorrect arguments to field: " << get_name()
           << PyString_AsString(str);
    } else {
      strm << "Value out of range on field: " << get_name()
           << PyString_AsString(str);
    }
    
    Py_DECREF(str);
    Py_DECREF(tuple);

    nassert_raise(strm.str());
  }
  return false;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCField::unpack_args
//       Access: Published
//  Description: Unpacks the values from the packer, beginning at
//               the current point in the unpack_buffer, into a Python
//               tuple and returns the tuple.
//
//               It is assumed that the packer is currently positioned
//               on this field.
////////////////////////////////////////////////////////////////////
PyObject *DCField::
unpack_args(DCPacker &packer) const {
  nassertr(!packer.had_error(), NULL);
  nassertr(packer.get_current_field() == this, NULL);

  PyObject *object = packer.unpack_object();

  if (!packer.had_error()) {
    // Successfully unpacked.
    /*
    PyObject *str = PyObject_Str(object);
    cerr << "recv " << get_name() << PyString_AsString(str) << "\n";
    Py_DECREF(str);
    */

    return object;
  }
  
  ostringstream strm;
  if (packer.had_pack_error()) {
    strm << "Error unpacking to field " << get_name();
  } else {
    PyObject *str = PyObject_Str(object);
    strm << "Found value outside specified range when unpacking field " 
         << get_name() << ": " << PyString_AsString(str);
    Py_DECREF(str);
  }

  /*
  Datagram dg(data.data(), packer.get_num_unpacked_bytes());
  dg.dump_hex(cerr);
  */
    
  nassert_raise(strm.str());
  Py_XDECREF(object);
  return NULL;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCField::receive_update
//       Access: Published
//  Description: Extracts the update message out of the datagram and
//               applies it to the indicated object by calling the
//               appropriate method.
////////////////////////////////////////////////////////////////////
void DCField::
receive_update(DCPacker &packer, PyObject *distobj) const {
  PyObject *args = unpack_args(packer);

  if (args != (PyObject *)NULL) {
    if (PyObject_HasAttrString(distobj, (char *)_name.c_str())) {
      PyObject *func = PyObject_GetAttrString(distobj, (char *)_name.c_str());
      nassertv(func != (PyObject *)NULL);
      
      PyObject *result = PyObject_CallObject(func, args);
      Py_XDECREF(result);
      Py_DECREF(func);
    }
    
    Py_DECREF(args);
  }
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCField::client_format_update
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to send an update for the indicated distributed
//               object from the client.
////////////////////////////////////////////////////////////////////
Datagram DCField::
client_format_update(int do_id, PyObject *args) const {
  DCPacker packer;

  packer.raw_pack_uint16(CLIENT_OBJECT_UPDATE_FIELD);
  packer.raw_pack_uint32(do_id);
  packer.raw_pack_uint16(_number);

  packer.begin_pack(this);
  pack_args(packer, args);
  if (!packer.end_pack()) {
    return Datagram();
  }

  return Datagram(packer.get_data(), packer.get_length());
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCField::al_format_update
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to send an update for the indicated distributed
//               object from the AI.
////////////////////////////////////////////////////////////////////
Datagram DCField::
ai_format_update(int do_id, int to_id, int from_id, PyObject *args) const {
  DCPacker packer;

  packer.raw_pack_uint32(to_id);
  packer.raw_pack_uint32(from_id);
  packer.raw_pack_uint8('A');
  packer.raw_pack_uint16(STATESERVER_OBJECT_UPDATE_FIELD);
  packer.raw_pack_uint32(do_id);
  packer.raw_pack_uint16(_number);

  packer.begin_pack(this);
  pack_args(packer, args);
  if (!packer.end_pack()) {
    return Datagram();
  }

  return Datagram(packer.get_data(), packer.get_length());
}
#endif  // HAVE_PYTHON


////////////////////////////////////////////////////////////////////
//     Function: DCField::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this field into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCField::
generate_hash(HashGenerator &hashgen) const {
  // It shouldn't be necessary to explicitly add _number to the
  // hash--this is computed based on the relative position of this
  // field with the other fields, so adding it explicitly will be
  // redundant.  However, the field name is significant.
  hashgen.add_string(_name);
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::pack_default_value
//       Access: Public, Virtual
//  Description: Packs the field's specified default value (or a
//               sensible default if no value is specified) into the
//               stream.  Returns true if the default value is packed,
//               false if the field doesn't know how to pack its
//               default value.
////////////////////////////////////////////////////////////////////
bool DCField::
pack_default_value(DCPackData &pack_data, bool &) const {
  // The default behavior is to pack the default value if we got it;
  // otherwise, to return false and let the packer visit our nested
  // elements.
  if (!_default_value_stale) {
    pack_data.append_data(_default_value.data(), _default_value.length());
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::set_number
//       Access: Public
//  Description: Assigns the unique number to this field.  This is
//               normally called only by the DCClass interface as the
//               field is added.
////////////////////////////////////////////////////////////////////
void DCField::
set_number(int number) {
  _number = number;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::set_default_value
//       Access: Public
//  Description: Establishes a default value for this field.
////////////////////////////////////////////////////////////////////
void DCField::
set_default_value(const string &default_value) {
  _default_value = default_value;
  _has_default_value = true;
  _default_value_stale = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::add_flag
//       Access: Public
//  Description: Adds a new flag to the field.
////////////////////////////////////////////////////////////////////
void DCField::
add_flag(enum Flags flag) {
  _flags |= flag;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::set_flags
//       Access: Public
//  Description: Resets the flag bitmask to the indicated value.
////////////////////////////////////////////////////////////////////
void DCField::
set_flags(int flags) {
  _flags = flags;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::get_flags
//       Access: Public
//  Description: Returns the complete flag bitmask.
////////////////////////////////////////////////////////////////////
int DCField::
get_flags() const {
  return _flags;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::refresh_default_value
//       Access: Protected
//  Description: Recomputes the default value of the field by
//               repacking it.
////////////////////////////////////////////////////////////////////
void DCField::
refresh_default_value() {
  DCPacker packer;
  packer.begin_pack(this);
  packer.pack_default_value();
  if (!packer.end_pack()) {
    cerr << "Error while packing default value for " << get_name() << "\n";
  } else {
    _default_value.assign(packer.get_data(), packer.get_length());
  }
  _default_value_stale = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::output_flags
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
void DCField::
output_flags(ostream &out) const {
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
}
