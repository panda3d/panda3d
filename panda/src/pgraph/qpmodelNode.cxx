// Filename: qpmodelNode.cxx
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

#include "qpmodelNode.h"

#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle qpModelNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: qpModelNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *qpModelNode::
make_copy() const {
  return new qpModelNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelNode::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of Node by duplicating
//               instances, false otherwise (for instance, a Camera
//               cannot be safely flattened, because the Camera
//               pointer itself is meaningful).
////////////////////////////////////////////////////////////////////
bool qpModelNode::
safe_to_flatten() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelNode::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a Character.
////////////////////////////////////////////////////////////////////
bool qpModelNode::
safe_to_transform() const {
  return !_preserve_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine this
//               particular kind of PandaNode with other kinds of
//               PandaNodes, adding children or whatever.  For
//               instance, an LODNode should not be combined with any
//               other PandaNode, because its set of children is
//               meaningful.
////////////////////////////////////////////////////////////////////
bool qpModelNode::
safe_to_combine() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelNode::preserve_name
//       Access: Public, Virtual
//  Description: Returns true if the node's name has extrinsic meaning
//               and must be preserved across a flatten operation,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool qpModelNode::
preserve_name() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpModelNode.
////////////////////////////////////////////////////////////////////
void qpModelNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpModelNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  dg.add_bool(_preserve_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpModelNode is encountered
//               in the Bam file.  It should create the qpModelNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpModelNode::
make_from_bam(const FactoryParams &params) {
  qpModelNode *node = new qpModelNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpModelNode.
////////////////////////////////////////////////////////////////////
void qpModelNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);

  if (manager->get_file_minor_ver() < 2) {
    // No _preserve_transform before bams 3.2.
    _preserve_transform = false;
  } else {
    _preserve_transform = scan.get_bool();
  }
}
