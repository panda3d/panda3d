// Filename: modelNode.cxx
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

#include "uvScrollNode.h"

#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle UvScrollNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: UvScrollNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *UvScrollNode::
make_copy() const {
  return new UvScrollNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ModelNode.
////////////////////////////////////////////////////////////////////
void UvScrollNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void UvScrollNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  ModelNode::write_datagram(manager, dg);
  dg.add_float64(_u_speed);
  dg.add_float64(_v_speed);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ModelNode is encountered
//               in the Bam file.  It should create the ModelNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *UvScrollNode::
make_from_bam(const FactoryParams &params) {
  UvScrollNode *node = new UvScrollNode("", 0.0, 0.0);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ModelNode.
////////////////////////////////////////////////////////////////////
void UvScrollNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);

  _u_speed = scan.get_float64();
  _v_speed = scan.get_float64();
}
