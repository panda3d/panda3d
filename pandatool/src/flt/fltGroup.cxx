// Filename: fltGroup.cxx
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

#include "fltGroup.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltHeader.h"

TypeHandle FltGroup::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltGroup::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltGroup::
FltGroup(FltHeader *header) : FltBeadID(header) {
  _relative_priority = 0;
  _flags = 0;
  _special_id1 = 0;
  _special_id2 = 0;
  _significance = 0;
  _layer_id = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FltGroup::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this bead based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltGroup::
extract_record(FltRecordReader &reader) {
  if (!FltBeadID::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_group, false);
  DatagramIterator &iterator = reader.get_iterator();

  _relative_priority = iterator.get_be_int16();
  iterator.skip_bytes(2);
  _flags = iterator.get_be_uint32();
  _special_id1 = iterator.get_be_int16();
  _special_id2 = iterator.get_be_int16();
  _significance = iterator.get_be_int16();
  _layer_id = iterator.get_int8();
  iterator.skip_bytes(1);
  if (_header->get_flt_version() >= 1420) {
    iterator.skip_bytes(4);
  }

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltGroup::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltGroup::
build_record(FltRecordWriter &writer) const {
  if (!FltBeadID::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_group);
  Datagram &datagram = writer.update_datagram();

  datagram.add_be_int16(_relative_priority);
  datagram.pad_bytes(2);
  datagram.add_be_uint32(_flags);
  datagram.add_be_int16(_special_id1);
  datagram.add_be_int16(_special_id2);
  datagram.add_be_int16(_significance);
  datagram.add_int8(_layer_id);
  datagram.pad_bytes(5);

  return true;
}
