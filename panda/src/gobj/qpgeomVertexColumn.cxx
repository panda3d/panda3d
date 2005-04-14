// Filename: qpgeomVertexColumn.cxx
// Created by:  drose (06Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "qpgeomVertexColumn.h"
#include "bamReader.h"
#include "bamWriter.h"

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::
output(ostream &out) const {
  out << *get_name() << "(" << get_num_components() << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::setup
//       Access: Private
//  Description: Called once at construction time (or at bam-reading
//               time) to initialize the internal dependent values.
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::
setup() {
  nassertv(_num_components > 0 && _start >= 0);

  _num_values = _num_components;

  switch (_numeric_type) {
  case NT_uint16:
    _component_bytes = 2;  // sizeof(PN_uint16)
    break;

  case NT_uint8:
    _component_bytes = 1;
    break;

  case NT_packed_dcba:
  case NT_packed_dabc:
    _component_bytes = 4;  // sizeof(PN_uint32)
    _num_values *= 4;
    break;

  case NT_float32:
    _component_bytes = 4;  // sizeof(PN_float32)
    break;
  }

  _total_bytes = _component_bytes * _num_components;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::write_datagram
//       Access: Public
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::
write_datagram(BamWriter *manager, Datagram &dg) {
  manager->write_pointer(dg, _name);
  dg.add_uint8(_num_components);
  dg.add_uint8(_numeric_type);
  dg.add_uint8(_contents);
  dg.add_uint16(_start);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::complete_pointers
//       Access: Public
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int qpGeomVertexColumn::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = 0;

  _name = DCAST(InternalName, p_list[pi++]);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeomVertexColumn.
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::
fillin(DatagramIterator &scan, BamReader *manager) {
  manager->read_pointer(scan);

  _num_components = scan.get_uint8();
  _numeric_type = (NumericType)scan.get_uint8();
  _contents = (Contents)scan.get_uint8();
  _start = scan.get_uint16();

  setup();
}
