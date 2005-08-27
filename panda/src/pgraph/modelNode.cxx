// Filename: modelNode.cxx
// Created by:  drose (16Mar02)
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

#include "modelNode.h"

#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle ModelNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: ModelNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *ModelNode::
make_copy() const {
  return new ModelNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of Node by duplicating
//               instances, false otherwise (for instance, a Camera
//               cannot be safely flattened, because the Camera
//               pointer itself is meaningful).
////////////////////////////////////////////////////////////////////
bool ModelNode::
safe_to_flatten() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a Character.
////////////////////////////////////////////////////////////////////
bool ModelNode::
safe_to_transform() const {
  return _preserve_transform == PT_none;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::safe_to_modify_transform
//       Access: Public, Virtual
//  Description: Returns true if it is safe to automatically adjust
//               the transform on this kind of node.  Usually, this is
//               only a bad idea if the user expects to find a
//               particular transform on the node.
//
//               ModelNodes with the preserve_transform flag set are
//               presently the only kinds of nodes that should not
//               have their transform even adjusted.
////////////////////////////////////////////////////////////////////
bool ModelNode::
safe_to_modify_transform() const {
  return _preserve_transform != PT_local;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine this
//               particular kind of PandaNode with other kinds of
//               PandaNodes, adding children or whatever.  For
//               instance, an LODNode should not be combined with any
//               other PandaNode, because its set of children is
//               meaningful.
////////////////////////////////////////////////////////////////////
bool ModelNode::
safe_to_combine() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::preserve_name
//       Access: Public, Virtual
//  Description: Returns true if the node's name has extrinsic meaning
//               and must be preserved across a flatten operation,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool ModelNode::
preserve_name() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::get_unsafe_to_apply_attribs
//       Access: Public, Virtual
//  Description: Returns the union of all attributes from
//               SceneGraphReducer::AttribTypes that may not safely be
//               applied to the vertices of this node.  If this is
//               nonzero, these attributes must be dropped at this
//               node as a state change.
//
//               This is a generalization of safe_to_transform().
////////////////////////////////////////////////////////////////////
int ModelNode::
get_unsafe_to_apply_attribs() const {
  return _preserve_attributes;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ModelNode.
////////////////////////////////////////////////////////////////////
void ModelNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ModelNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  dg.add_uint8((int)_preserve_transform);
  dg.add_uint16(_preserve_attributes);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ModelNode is encountered
//               in the Bam file.  It should create the ModelNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ModelNode::
make_from_bam(const FactoryParams &params) {
  ModelNode *node = new ModelNode("");
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
void ModelNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);

  _preserve_transform = (PreserveTransform)scan.get_uint8();
  _preserve_attributes = 0;
  if (manager->get_file_minor_ver() >= 3) {
    _preserve_attributes = scan.get_uint16();
  }
}
