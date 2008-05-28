// Filename: fltObject.cxx
// Created by:  drose (24Aug00)
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

#include "fltObject.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltObject::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltObject::
FltObject(FltHeader *header) : FltBeadID(header) {
}

////////////////////////////////////////////////////////////////////
//     Function: FltObject::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this bead based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltObject::
extract_record(FltRecordReader &reader) {
  if (!FltBeadID::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_object, false);
  DatagramIterator &iterator = reader.get_iterator();

  _flags = iterator.get_be_uint32();
  _relative_priority = iterator.get_be_int16();
  _transparency = iterator.get_be_int16();
  _special_id1 = iterator.get_be_int16();
  _special_id2 = iterator.get_be_int16();
  _significance = iterator.get_be_int16();
  iterator.skip_bytes(2);

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltObject::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltObject::
build_record(FltRecordWriter &writer) const {
  if (!FltBeadID::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_object);
  Datagram &datagram = writer.update_datagram();

  datagram.add_be_uint32(_flags);
  datagram.add_be_int16(_relative_priority);
  datagram.add_be_int16(_transparency);
  datagram.add_be_int16(_special_id1);
  datagram.add_be_int16(_special_id2);
  datagram.add_be_int16(_significance);
  datagram.pad_bytes(2);

  return true;
}
