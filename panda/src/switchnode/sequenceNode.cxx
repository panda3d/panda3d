// Filename: sequenceNode.cxx
// Created by:  jason (18Jul00)
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

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <clockObject.h>

#include "sequenceNode.h"
#include "config_switchnode.h"

#include <graphicsStateGuardian.h>
#include <allTransitionsWrapper.h>
#include <renderRelation.h>
#include <renderTraverser.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle SequenceNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
SequenceNode::
SequenceNode(const string &initial_name) :
  SwitchNodeOne(initial_name), TimedCycle()
{
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
SequenceNode::
SequenceNode(float switch_time, const string &initial_name) :
   SwitchNodeOne(initial_name), TimedCycle(switch_time, 0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::set_switch_time
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void SequenceNode::
set_switch_time(float switch_time)
{
  set_cycle_time(switch_time);
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.  No children will be
//               copied.
////////////////////////////////////////////////////////////////////
Node *SequenceNode::
make_copy() const {
  return new SequenceNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::compute_switch
//       Access: Public, Virtual
//  Description: Computes the node's switching properties, to decide
//               what children should be visible in the given
//               rendering situation.
//
//               In the case of a SequenceNode, this simply selects
//               the appropriate child based on the elapsed time.
////////////////////////////////////////////////////////////////////
void SequenceNode::
compute_switch(RenderTraverser *) {
  // Determine which child to traverse
  int num_children = get_num_children(RenderRelation::get_class_type());
  set_element_count(num_children);
  int index = next_element();
  select_child(index);
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::write_object
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void SequenceNode::
write_datagram(BamWriter *manager, Datagram &me) {
  SwitchNodeOne::write_datagram(manager, me);
  TimedCycle::write_datagram(me);
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_SequenceNode to
//               read in all of the relevant data from the BamFile for
//               the new SequenceNode.
////////////////////////////////////////////////////////////////////
void SequenceNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  SwitchNodeOne::fillin(scan, manager);
  TimedCycle::fillin(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::make_SequenceNode
//       Access: Protected
//  Description: This function is called by the BamReader's factory
//               when a new object of type SequenceNode is encountered in
//               the Bam file.  It should create the SequenceNode and
//               extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *SequenceNode::
make_SequenceNode(const FactoryParams &params) {
  SequenceNode *me = new SequenceNode;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               SequenceNode.
////////////////////////////////////////////////////////////////////
void SequenceNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_SequenceNode);
}
