// Filename: animBundleNode.cxx
// Created by:  drose (22Feb99)
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


#include "animBundleNode.h"
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle AnimBundleNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *AnimBundleNode::
make_copy() const {
  return new AnimBundleNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void AnimBundleNode::
write_datagram(BamWriter *manager, Datagram &me)
{
  NamedNode::write_datagram(manager, me);
  manager->write_pointer(me, _bundle);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void AnimBundleNode::
fillin(DatagramIterator& scan, BamReader* manager)
{
  NamedNode::fillin(scan, manager);
  manager->read_pointer(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointes to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int AnimBundleNode::
complete_pointers(TypedWritable **p_list, BamReader* manager)
{
  int start = NamedNode::complete_pointers(p_list, manager);
  _bundle = DCAST(AnimBundle, p_list[start]);
  return start+1;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::make_AnimBundleNode
//       Access: Protected
//  Description: Factory method to generate a AnimBundleNode object
////////////////////////////////////////////////////////////////////
TypedWritable* AnimBundleNode::
make_AnimBundleNode(const FactoryParams &params)
{
  AnimBundleNode *me = new AnimBundleNode;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a AnimBundleNode object
////////////////////////////////////////////////////////////////////
void AnimBundleNode::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_AnimBundleNode);
}



