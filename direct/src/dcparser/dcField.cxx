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
//     Function: DCField::get_name
//       Access: Published
//  Description: Returns the name of this field.
////////////////////////////////////////////////////////////////////
const string &DCField::
get_name() const {
  return _name;
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

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCField::pack_args
//       Access: Published
//  Description: Packs the Python arguments from the indicated tuple
//               into the datagram, appending to the end of the
//               datagram.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCField::
pack_args(Datagram &datagram, PyObject *sequence) const {
  nassertr(PySequence_Check(sequence), false);
  DCPacker packer;
  packer.begin_pack(this);
  packer.pack_object(sequence);
  if (packer.end_pack()) {
    datagram.append_data(packer.get_data(), packer.get_length());

    /*
    PyObject *str = PyObject_Str(sequence);
    cerr << "pack " << get_name() << PyString_AsString(str) << "\n";
    Py_DECREF(str);
    */

    return true;
  }

  PyObject *tuple = PySequence_Tuple(sequence);
  PyObject *str = PyObject_Str(tuple);
  
  ostringstream strm;
  strm << "Incorrect arguments to field: " << get_name()
       << PyString_AsString(str);

  Py_DECREF(str);
  Py_DECREF(tuple);
    
  nassert_raise(strm.str());
  return false;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCField::unpack_args
//       Access: Published
//  Description: Unpacks the values from the datagram, beginning at
//               the current point in the interator, into a Python
//               tuple and returns the tuple.  If there are remaining
//               bytes in the datagram, they are ignored (but the
//               iterator is left at the first unread byte).
////////////////////////////////////////////////////////////////////
PyObject *DCField::
unpack_args(DatagramIterator &iterator) const {
  DCPacker packer;
  packer.begin_unpack(iterator.get_remaining_bytes(), this);

  PyObject *object = packer.unpack_object();

  if (packer.end_unpack()) {
    // Successfully unpacked.
    iterator.skip_bytes(packer.get_num_unpacked_bytes());

    /*
    PyObject *str = PyObject_Str(object);
    cerr << "recv " << get_name() << PyString_AsString(str) << "\n";
    Py_DECREF(str);
    */

    return object;
  }
  
  ostringstream strm;
  strm << "Error unpacking field " << get_name();
    
  nassert_raise(strm.str());
  return object;
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
receive_update(PyObject *distobj, DatagramIterator &iterator) const {
  PyObject *args = unpack_args(iterator);

  if (PyObject_HasAttrString(distobj, (char *)_name.c_str())) {
    PyObject *func = PyObject_GetAttrString(distobj, (char *)_name.c_str());
    nassertv(func != (PyObject *)NULL);

    PyObject *result = PyObject_CallObject(func, args);
    Py_XDECREF(result);
    Py_DECREF(func);
  }

  Py_DECREF(args);
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
  Datagram dg;
  dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD);
  dg.add_uint32(do_id);
  dg.add_uint16(_number);
  pack_args(dg, args);
  return dg;
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
  Datagram dg;
  dg.add_uint32(to_id);
  dg.add_uint32(from_id);
  dg.add_uint8('A');
  dg.add_uint16(STATESERVER_OBJECT_UPDATE_FIELD);
  dg.add_uint32(do_id);
  dg.add_uint16(_number);
  pack_args(dg, args);
  return dg;
}
#endif  // HAVE_PYTHON


////////////////////////////////////////////////////////////////////
//     Function: DCField::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCField::
DCField(const string &name) : _name(name) {
  _number = 0;
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
//     Function: DCField::has_nested_fields
//       Access: Public, Virtual
//  Description: Returns true if this field type has any nested fields
//               (and thus expects a push() .. pop() interface to the
//               DCPacker), or false otherwise.  If this returns true,
//               get_num_nested_fields() may be called to determine
//               how many nested fields are expected.
////////////////////////////////////////////////////////////////////
bool DCField::
has_nested_fields() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::get_pack_type
//       Access: Public, Virtual
//  Description: Returns the type of value expected by this field.
////////////////////////////////////////////////////////////////////
DCPackType DCField::
get_pack_type() const {
  return PT_field;
}
