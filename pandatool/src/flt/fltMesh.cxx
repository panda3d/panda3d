/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltMesh.cxx
 * @author drose
 * @date 2001-02-28
 */

#include "fltMesh.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltHeader.h"
#include "fltMaterial.h"
#include "config_flt.h"

TypeHandle FltMesh::_type_handle;

/**
 *
 */
FltMesh::
FltMesh(FltHeader *header) : FltGeometry(header) {
}

/**
 * Fills in the information in this bead based on the information given in the
 * indicated datagram, whose opcode has already been read.  Returns true on
 * success, false if the datagram is invalid.
 */
bool FltMesh::
extract_record(FltRecordReader &reader) {
  if (!FltBeadID::extract_record(reader)) {
    return false;
  }

  DatagramIterator &iterator = reader.get_iterator();
  iterator.skip_bytes(4); // Undocumented padding.

  if (!FltGeometry::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_mesh, false);

  check_remaining_size(iterator);
  return true;
}

/**
 * Checks whether the given bead, which follows this bead sequentially in the
 * file, is an ancillary record of this bead.  If it is, extracts the relevant
 * information and returns true; otherwise, leaves it alone and returns false.
 */
bool FltMesh::
extract_ancillary(FltRecordReader &reader) {
  if (reader.get_opcode() == FO_local_vertex_pool) {
    _vpool = new FltLocalVertexPool(_header);
    return _vpool->extract_record(reader);
  }

  return FltBeadID::extract_ancillary(reader);
}

/**
 * Fills up the current record on the FltRecordWriter with data for this
 * record, but does not advance the writer.  Returns true on success, false if
 * there is some error.
 */
bool FltMesh::
build_record(FltRecordWriter &writer) const {
  if (!FltBeadID::build_record(writer)) {
    return false;
  }

  Datagram &datagram = writer.update_datagram();
  datagram.pad_bytes(4); // Undocumented padding.

  if (!FltGeometry::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_mesh);

  return true;
}

/**
 * Writes whatever ancillary records are required for this record.  Returns
 * FE_ok on success, or something else if there is some error.
 */
FltError FltMesh::
write_ancillary(FltRecordWriter &writer) const {
  if (_vpool != nullptr) {
    if (!_vpool->build_record(writer)) {
      assert(!flt_error_abort);
      return FE_bad_data;
    }
    FltError result = writer.advance();
    if (result != FE_ok) {
      return result;
    }
  }

  return FltBeadID::write_ancillary(writer);
}
