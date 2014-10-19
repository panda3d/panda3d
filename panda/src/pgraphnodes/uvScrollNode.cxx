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
#include "cullTraverserData.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "luse.h"
#include "renderState.h"
#include "texMatrixAttrib.h"
#include "textureStage.h"
#include "transformState.h"

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
//     Function: UvSctrollNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               UvScrollNode.
////////////////////////////////////////////////////////////////////
void UvScrollNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: UvSctrollNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void UvScrollNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  dg.add_stdfloat(_u_speed);
  dg.add_stdfloat(_v_speed);
  dg.add_stdfloat(_w_speed);
  dg.add_stdfloat(_r_speed);
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
  UvScrollNode *node = new UvScrollNode("");
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

  _u_speed = scan.get_stdfloat();
  _v_speed = scan.get_stdfloat();
  if (manager->get_file_minor_ver() >= 33) {
    _w_speed = scan.get_stdfloat();
  }
  if (manager->get_file_minor_ver() >= 22) {
    _r_speed = scan.get_stdfloat();
  }

}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::cull_callback
//       Access: Public, Virtual
//  Description: This function will be called during the cull
//               traversal to perform any additional operations that
//               should be performed at cull time.  This may include
//               additional manipulation of render state or additional
//               visible/invisible decisions, or any other arbitrary
//               operation.
//
//               Note that this function will *not* be called unless
//               set_cull_callback() is called in the constructor of
//               the derived class.  It is necessary to call
//               set_cull_callback() to indicated that we require
//               cull_callback() to be called.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is \true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool UvScrollNode::
cull_callback(CullTraverser * trav, CullTraverserData &data) {
  double elapsed = ClockObject::get_global_clock()->get_frame_time() - _start_time; 
  CPT(TransformState) ts = TransformState::make_pos_hpr(
    LVecBase3(cmod(elapsed * _u_speed, 1.0) / 1.0,
              cmod(elapsed * _v_speed, 1.0) / 1.0,
              cmod(elapsed * _w_speed, 1.0) / 1.0),
    LVecBase3((cmod(elapsed * _r_speed, 1.0) / 1.0) * 360, 0, 0));

  CPT(RenderAttrib) tm = TexMatrixAttrib::make(TextureStage::get_default(), ts);
  CPT(RenderState) rs = RenderState::make_empty()->set_attrib(tm);
  data._state = data._state->compose(rs);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: UvScrollNode::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of PandaNode by duplicating
//               instances (by calling dupe_for_flatten()), false
//               otherwise (for instance, a Camera cannot be safely
//               flattened, because the Camera pointer itself is
//               meaningful).
////////////////////////////////////////////////////////////////////
bool UvScrollNode::
safe_to_flatten() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: UvScrollNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine this
//               with other nodes, which it isn't, so don't. Ever.
////////////////////////////////////////////////////////////////////
bool UvScrollNode::
safe_to_combine() const {
  return false;
}
