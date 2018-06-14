/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltUnsupportedRecord.cxx
 * @author drose
 * @date 2000-08-24
 */

#include "fltUnsupportedRecord.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltUnsupportedRecord::_type_handle;

/**
 *
 */
FltUnsupportedRecord::
FltUnsupportedRecord(FltHeader *header) : FltRecord(header) {
  _opcode = FO_none;
}

/**
 * Writes a quick one-line description of the bead, but not its children.
 * This is a human-readable description, primarily for debugging; to write a
 * flt file, use FltHeader::write_flt().
 */
void FltUnsupportedRecord::
output(std::ostream &out) const {
  out << "Unsupported(" << _opcode << ")";
}

/**
 * Fills in the information in this bead based on the information given in the
 * indicated datagram, whose opcode has already been read.  Returns true on
 * success, false if the datagram is invalid.
 */
bool FltUnsupportedRecord::
extract_record(FltRecordReader &reader) {
  _opcode = reader.get_opcode();
  _datagram = reader.get_datagram();

  return true;
}

/**
 * Fills up the current record on the FltRecordWriter with data for this
 * record, but does not advance the writer.  Returns true on success, false if
 * there is some error.
 */
bool FltUnsupportedRecord::
build_record(FltRecordWriter &writer) const {
  writer.set_opcode(_opcode);
  writer.set_datagram(_datagram);
  return true;
}
