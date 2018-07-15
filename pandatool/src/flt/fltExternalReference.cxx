/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltExternalReference.cxx
 * @author drose
 * @date 2000-08-30
 */

#include "fltExternalReference.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltHeader.h"
#include "pathReplace.h"

TypeHandle FltExternalReference::_type_handle;

/**
 *
 */
FltExternalReference::
FltExternalReference(FltHeader *header) : FltBead(header) {
  _flags = 0;
}

/**
 * Walks the hierarchy at this record and below and copies the
 * _converted_filename record into the _orig_filename record, so the flt file
 * will be written out with the converted filename instead of what was
 * originally read in.
 */
void FltExternalReference::
apply_converted_filenames() {
  _orig_filename = _converted_filename.to_os_generic();
  FltBead::apply_converted_filenames();
}

/**
 * Writes a quick one-line description of the record, but not its children.
 * This is a human-readable description, primarily for debugging; to write a
 * flt file, use FltHeader::write_flt().
 */
void FltExternalReference::
output(std::ostream &out) const {
  out << "External " << get_ref_filename();
  if (!_bead_id.empty()) {
    out << " (" << _bead_id << ")";
  }
}

/**
 * Returns the name of the referenced file.
 */
Filename FltExternalReference::
get_ref_filename() const {
  return _converted_filename;
}

/**
 * Changes the name of the referenced file.
 */
void FltExternalReference::
set_ref_filename(const Filename &filename) {
  _converted_filename = filename;
  _orig_filename = _converted_filename.to_os_generic();
}

/**
 * Fills in the information in this bead based on the information given in the
 * indicated datagram, whose opcode has already been read.  Returns true on
 * success, false if the datagram is invalid.
 */
bool FltExternalReference::
extract_record(FltRecordReader &reader) {
  if (!FltBead::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_external_ref, false);
  DatagramIterator &iterator = reader.get_iterator();

  std::string name = iterator.get_fixed_string(200);
  iterator.skip_bytes(1 + 1);
  iterator.skip_bytes(2);   // Undocumented additional padding.
  _flags = iterator.get_be_uint32();
  iterator.skip_bytes(2);
  iterator.skip_bytes(2);   // Undocumented additional padding.

  _orig_filename = name;

  if (!name.empty() && name[name.length() - 1] == '>') {
    // Extract out the bead name.
    size_t open = name.rfind('<');
    if (open != std::string::npos) {
      _orig_filename = name.substr(0, open);
      _bead_id = name.substr(open + 1, name.length() - open - 2);
    }
  }
  _converted_filename = _header->convert_path(Filename::from_os_specific(_orig_filename));

  check_remaining_size(iterator);
  return true;
}

/**
 * Fills up the current record on the FltRecordWriter with data for this
 * record, but does not advance the writer.  Returns true on success, false if
 * there is some error.
 */
bool FltExternalReference::
build_record(FltRecordWriter &writer) const {
  if (!FltBead::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_external_ref);
  Datagram &datagram = writer.update_datagram();

  std::string name = _orig_filename;
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
