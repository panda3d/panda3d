// Filename: qpmodelRoot.cxx
// Created by:  drose (16Mar02)
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

#include "qpmodelRoot.h"

TypeHandle qpModelRoot::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: qpModelRoot::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *qpModelRoot::
make_copy() const {
  return new qpModelRoot(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelRoot::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpModelRoot.
////////////////////////////////////////////////////////////////////
void qpModelRoot::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelRoot::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpModelRoot::
write_datagram(BamWriter *manager, Datagram &dg) {
  qpModelNode::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelRoot::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpModelRoot is encountered
//               in the Bam file.  It should create the qpModelRoot
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpModelRoot::
make_from_bam(const FactoryParams &params) {
  qpModelRoot *node = new qpModelRoot("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelRoot::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpModelRoot.
////////////////////////////////////////////////////////////////////
void qpModelRoot::
fillin(DatagramIterator &scan, BamReader *manager) {
  qpModelNode::fillin(scan, manager);
}
