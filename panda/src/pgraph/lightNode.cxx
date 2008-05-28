// Filename: lightNode.cxx
// Created by:  drose (26Mar02)
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

#include "lightNode.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle LightNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LightNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LightNode::
LightNode(const string &name) : 
  PandaNode(name) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LightNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
LightNode::
LightNode(const LightNode &copy) : 
  Light(copy),
  PandaNode(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LightNode::as_node
//       Access: Published, Virtual
//  Description: Returns the Light object upcast to a PandaNode.
////////////////////////////////////////////////////////////////////
PandaNode *LightNode::
as_node() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: LightNode::as_light
//       Access: Public, Virtual
//  Description: Cross-casts the node to a Light pointer, if it is one
//               of the four kinds of Light nodes, or returns NULL if
//               it is not.
////////////////////////////////////////////////////////////////////
Light *LightNode::
as_light() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: LightNode::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void LightNode::
output(ostream &out) const {
  PandaNode::output(out);
}

////////////////////////////////////////////////////////////////////
//     Function: LightNode::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void LightNode::
write(ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: LightNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void LightNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  Light::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: LightNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new LightNode.
////////////////////////////////////////////////////////////////////
void LightNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  Light::fillin(scan, manager);
}
