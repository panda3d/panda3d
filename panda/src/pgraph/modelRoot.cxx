// Filename: modelRoot.cxx
// Created by:  drose (16Mar02)
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

#include "modelRoot.h"

TypeHandle ModelRoot::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *ModelRoot::
make_copy() const {
  return new ModelRoot(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ModelRoot.
////////////////////////////////////////////////////////////////////
void ModelRoot::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ModelRoot::
write_datagram(BamWriter *manager, Datagram &dg) {
  ModelNode::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ModelRoot is encountered
//               in the Bam file.  It should create the ModelRoot
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ModelRoot::
make_from_bam(const FactoryParams &params) {
  ModelRoot *node = new ModelRoot("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ModelRoot.
////////////////////////////////////////////////////////////////////
void ModelRoot::
fillin(DatagramIterator &scan, BamReader *manager) {
  ModelNode::fillin(scan, manager);
}
