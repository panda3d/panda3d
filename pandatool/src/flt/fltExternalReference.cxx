// Filename: fltExternalReference.cxx
// Created by:  drose (30Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "fltExternalReference.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltExternalReference::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltExternalReference::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FltExternalReference::
FltExternalReference(FltHeader *header) : FltBead(header) {
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FltExternalReference::output
//       Access: Public
//  Description: Writes a quick one-line description of the record, but
//               not its children.  This is a human-readable
//               description, primarily for debugging; to write a flt
//               file, use FltHeader::write_flt().
////////////////////////////////////////////////////////////////////
void FltExternalReference::
output(ostream &out) const {
  out << "External " << _filename;
  if (!_bead_id.empty()) {
    out << " (" << _bead_id << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FltExternalReference::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this bead based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltExternalReference::
extract_record(FltRecordReader &reader) {
  if (!FltBead::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_external_ref, false);
  DatagramIterator &iterator = reader.get_iterator();

  string name = iterator.get_fixed_string(200);
  iterator.skip_bytes(1 + 1);
  iterator.skip_bytes(2);   // Undocumented additional padding.
  _flags = iterator.get_be_uint32();
  iterator.skip_bytes(2);
  iterator.skip_bytes(2);   // Undocumented additional padding.

  _filename = name;

  if (!name.empty() && name[name.length() - 1] == '>') {
    // Extract out the bead name.
    size_t open = name.rfind('<');
    if (open != string::npos) {
      _filename = name.substr(0, open);
      _bead_id = name.substr(open + 1, name.length() - open - 2);
    }
  }

  nassertr(iterator.get_remaining_size() == 0, true);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltExternalReference::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltExternalReference::
build_record(FltRecordWriter &writer) const {
  if (!FltBead::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_external_ref);
  Datagram &datagram = writer.update_datagram();

  string name = _filename;
  if (!_bead_id.empty()) {
    name += "<" + _bead_id + ">";
  }

  datagram.add_fixed_string(name.substr(0, 199), 200);
  datagram.pad_bytes(1 + 1);
  datagram.pad_bytes(2);   // Undocumented additional padding.
  datagram.add_be_uint32(_flags);
  datagram.pad_bytes(2);
  datagram.pad_bytes(2);   // Undocumented additional padding.

  return true;
}
