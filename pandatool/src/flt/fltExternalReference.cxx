// Filename: fltExternalReference.cxx
// Created by:  drose (30Aug00)
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

#include "fltExternalReference.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltHeader.h"

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
//     Function: FltTexture::get_ref_filename
//       Access: Public
//  Description: Returns the name of the referenced file.  If it
//               appears to be a relative filename, it will be
//               converted to the correct full pathname according to
//               the model_path specified in the header.
////////////////////////////////////////////////////////////////////
Filename FltExternalReference::
get_ref_filename() const {
  Filename file = Filename::from_os_specific(_filename);
  file.resolve_filename(_header->get_model_path());
  return file;
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

  check_remaining_size(iterator);
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
