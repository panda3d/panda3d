// Filename: fltBeadID.cxx
// Created by:  drose (24Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "fltBeadID.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltBeadID::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltBeadID::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FltBeadID::
FltBeadID(FltHeader *header) : FltBead(header) {
}

////////////////////////////////////////////////////////////////////
//     Function: FltBeadID::output
//       Access: Public
//  Description: Writes a quick one-line description of the record, but
//               not its children.  This is a human-readable
//               description, primarily for debugging; to write a flt
//               file, use FltHeader::write_flt().
////////////////////////////////////////////////////////////////////
void FltBeadID::
output(ostream &out) const {
  out << get_type();
  if (!_id.empty()) {
    out << " " << _id;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FltBeadID::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this bead based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltBeadID::
extract_record(FltRecordReader &reader) {
  if (!FltBead::extract_record(reader)) {
    return false;
  }

  _id = reader.get_iterator().get_fixed_string(8);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltBeadID::extract_ancillary
//       Access: Protected, Virtual
//  Description: Checks whether the given bead, which follows this
//               bead sequentially in the file, is an ancillary record
//               of this bead.  If it is, extracts the relevant
//               information and returns true; otherwise, leaves it
//               alone and returns false.
////////////////////////////////////////////////////////////////////
bool FltBeadID::
extract_ancillary(FltRecordReader &reader) {
  if (reader.get_opcode() == FO_long_id) {
    _id = reader.get_iterator().get_remaining_bytes();
    return true;
  }

  return FltBead::extract_ancillary(reader);
}

////////////////////////////////////////////////////////////////////
//     Function: FltBeadID::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltBeadID::
build_record(FltRecordWriter &writer) const {
  if (!FltBead::build_record(writer)) {
    return false;
  }

  writer.update_datagram().add_fixed_string(_id.substr(0, 7), 8);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltBeadID::write_ancillary
//       Access: Protected, Virtual
//  Description: Writes whatever ancillary records are required for
//               this record.  Returns FE_ok on success, or something
//               else if there is some error.
////////////////////////////////////////////////////////////////////
FltError FltBeadID::
write_ancillary(FltRecordWriter &writer) const {
  if (_id.length() > 7) {
    Datagram dc(_id);
    FltError result = writer.write_record(FO_long_id, dc);
    if (result != FE_ok) {
      return result;
    }
  }

  return FltBead::write_ancillary(writer);
}
