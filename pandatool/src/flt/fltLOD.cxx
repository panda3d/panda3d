// Filename: fltLOD.cxx
// Created by:  drose (25Aug00)
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

#include "fltLOD.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltLOD::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltLOD::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltLOD::
FltLOD(FltHeader *header) : FltBeadID(header) {
  _switch_in = 0.0;
  _switch_out = 0.0;
  _special_id1 = 0;
  _special_id2 = 0;
  _flags = 0;
  _center_x = 0.0;
  _center_y = 0.0;
  _center_z = 0.0;
  _transition_range = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FltLOD::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this bead based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltLOD::
extract_record(FltRecordReader &reader) {
  if (!FltBeadID::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_lod, false);
  DatagramIterator &iterator = reader.get_iterator();

  iterator.skip_bytes(4);
  _switch_in = iterator.get_be_float64();
  _switch_out = iterator.get_be_float64();
  _special_id1 = iterator.get_be_int16();
  _special_id2 = iterator.get_be_int16();
  _flags = iterator.get_be_uint32();
  _center_x = iterator.get_be_float64();
  _center_y = iterator.get_be_float64();
  _center_z = iterator.get_be_float64();
  _transition_range = iterator.get_be_float64();

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltLOD::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltLOD::
build_record(FltRecordWriter &writer) const {
  if (!FltBeadID::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_lod);
  Datagram &datagram = writer.update_datagram();

  datagram.pad_bytes(4);
  datagram.add_be_float64(_switch_in);
  datagram.add_be_float64(_switch_out);
  datagram.add_be_int16(_special_id1);
  datagram.add_be_int16(_special_id2);
  datagram.add_be_uint32(_flags);
  datagram.add_be_float64(_center_x);
  datagram.add_be_float64(_center_y);
  datagram.add_be_float64(_center_z);
  datagram.add_be_float64(_transition_range);

  return true;
}
