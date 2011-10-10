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
//     Function: ModelNode::combine_with
//       Access: Public, Virtual
//  Description: Collapses this PandaNode with the other PandaNode, if
//               possible, and returns a pointer to the combined
//               PandaNode, or NULL if the two PandaNodes cannot
//               safely be combined.
//
//               The return value may be this, other, or a new
//               PandaNode altogether.
//
//               This function is called from GraphReducer::flatten(),
//               and need not deal with children; its job is just to
//               decide whether to collapse the two PandaNodes and
//               what the collapsed PandaNode should look like.
////////////////////////////////////////////////////////////////////
PandaNode *ModelNode::
combine_with(PandaNode *other) {
  if (_preserve_transform == PT_drop_node) {
    // If we have PT_drop_node set, we always yield to the other node.
    return other;
  }

  return PandaNode::combine_with(other);
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
  return _preserve_transform == PT_drop_node;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::safe_to_flatten_below
//       Access: Public, Virtual
//  Description: Returns true if a flatten operation may safely
//               continue past this node, or false if nodes below this
//               node may not be molested.
////////////////////////////////////////////////////////////////////
bool ModelNode::
safe_to_flatten_below() const {
  return _preserve_transform != PT_no_touch;
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
  return _preserve_transform == PT_none || _preserve_transform == PT_drop_node;
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
  return _preserve_transform != PT_local && _preserve_transform != PT_no_touch;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine this
//               particular kind of PandaNode with other kinds of
//               PandaNodes of compatible type, adding children or
//               whatever.  For instance, an LODNode should not be
//               combined with any other PandaNode, because its set of
//               children is meaningful.
////////////////////////////////////////////////////////////////////
bool ModelNode::
safe_to_combine() const {
  return _preserve_transform == PT_drop_node;
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
  return _preserve_transform != PT_drop_node && _preserve_transform != PT_no_touch;
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
//     Function : test_transform
//       Access : private
//  Description : this tests the transform to make sure it's within
//                the specified limits.  It's done so we can assert
//                to see when an invalid transform is being applied.
////////////////////////////////////////////////////////////////////
void ModelNode::
test_transform(const TransformState *ts) const {
  LPoint3 pos(ts->get_pos());
  nassertv(pos[0] < _transform_limit);
  nassertv(pos[0] > -_transform_limit);
  nassertv(pos[1] < _transform_limit);
  nassertv(pos[1] > -_transform_limit);
  nassertv(pos[2] < _transform_limit);
  nassertv(pos[2] > -_transform_limit);
}

////////////////////////////////////////////////////////////////////
//     Function : transform_changed
//       Access : private, virtual
//  Description : node hook.  This function handles outside
//                (non-physics) actions on the actor
//                and updates the internal representation of the node.
//                i.e. copy from PandaNode to PhysicsObject
////////////////////////////////////////////////////////////////////
void ModelNode::
transform_changed() {
  PandaNode::transform_changed();
  // get the transform
  CPT(TransformState) transform = get_transform();

  if (_transform_limit > 0.0) {
    test_transform(transform);
  }
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
  _preserve_attributes = scan.get_uint16();
}
