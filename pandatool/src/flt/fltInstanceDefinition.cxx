// Filename: fltInstanceDefinition.cxx
// Created by:  drose (30Aug00)
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

#include "fltInstanceDefinition.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltInstanceDefinition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltInstanceDefinition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltInstanceDefinition::
FltInstanceDefinition(FltHeader *header) : FltBead(header) {
  _instance_index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FltInstanceDefinition::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this bead based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltInstanceDefinition::
extract_record(FltRecordReader &reader) {
  if (!FltBead::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_instance, false);
  DatagramIterator &iterator = reader.get_iterator();

  iterator.skip_bytes(2);
  _instance_index = iterator.get_be_int16();

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltInstanceDefinition::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltInstanceDefinition::
build_record(FltRecordWriter &writer) const {
  if (!FltBead::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_instance);
  Datagram &datagram = writer.update_datagram();

  datagram.pad_bytes(2);
  datagram.add_be_int16(_instance_index);

  return true;
}
