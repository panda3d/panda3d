// Filename: qpsequenceNode.cxx
// Created by:  drose (06Mar02)
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

#include "pandabase.h"
#include "qpsequenceNode.h"

TypeHandle qpSequenceNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpSequenceNode::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *qpSequenceNode::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpSequenceNode::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
qpSequenceNode::
qpSequenceNode(const qpSequenceNode &copy) :
  SelectiveChildNode(copy)
{
  CDWriter cdata(_cycler);
  CDReader cdata_copy(copy._cycler);

  cdata->_cycle_rate = cdata_copy->_cycle_rate;
  cdata->_start_time = cdata_copy->_start_time;
  cdata->_frame_offset = cdata_copy->_frame_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: qpSequenceNode::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void qpSequenceNode::
operator = (const qpSequenceNode &copy) {
  SelectiveChildNode::operator = (copy);

  CDWriter cdata(_cycler);
  CDReader cdata_copy(copy._cycler);

  cdata->_cycle_rate = cdata_copy->_cycle_rate;
  cdata->_start_time = cdata_copy->_start_time;
  cdata->_frame_offset = cdata_copy->_frame_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: qpSequenceNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *qpSequenceNode::
make_copy() const {
  return new qpSequenceNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpSequenceNode::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool qpSequenceNode::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpSequenceNode::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool qpSequenceNode::
cull_callback(CullTraverserData &) {
  select_child(get_visible_child());
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpSequenceNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpSequenceNode.
////////////////////////////////////////////////////////////////////
void qpSequenceNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpSequenceNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpSequenceNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  SelectiveChildNode::write_datagram(manager, dg);

  CDReader cdata(_cycler);
  dg.add_float32(cdata->_cycle_rate);
  dg.add_float32(calc_frame());
}

////////////////////////////////////////////////////////////////////
//     Function: qpSequenceNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpSequenceNode is encountered
//               in the Bam file.  It should create the qpSequenceNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpSequenceNode::
make_from_bam(const FactoryParams &params) {
  qpSequenceNode *node = new qpSequenceNode(0.0f, "");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: qpSequenceNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpSequenceNode.
////////////////////////////////////////////////////////////////////
void qpSequenceNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  SelectiveChildNode::fillin(scan, manager);

  CDWriter cdata(_cycler);
  cdata->_cycle_rate = scan.get_float32();
  cdata->_frame_offset = scan.get_float32();

  float now = ClockObject::get_global_clock()->get_frame_time();
  cdata->_start_time = now;
}
