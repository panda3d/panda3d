// Filename: lightLensNode.cxx
// Created by:  drose (26Mar02)
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

#include "lightLensNode.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle LightLensNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LightLensNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LightLensNode::
LightLensNode(const string &name) : 
  LensNode(name) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LightLensNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
LightLensNode::
LightLensNode(const LightLensNode &copy) : 
  Light(copy),
  LensNode(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LightLensNode::as_node
//       Access: Published, Virtual
//  Description: Returns the Light object upcast to a PandaNode.
////////////////////////////////////////////////////////////////////
PandaNode *LightLensNode::
as_node() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: LightLensNode::as_light
//       Access: Public, Virtual
//  Description: Cross-casts the node to a Light pointer, if it is one
//               of the four kinds of Light nodes, or returns NULL if
//               it is not.
////////////////////////////////////////////////////////////////////
Light *LightLensNode::
as_light() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: LightLensNode::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void LightLensNode::
output(ostream &out) const {
  LensNode::output(out);
}

////////////////////////////////////////////////////////////////////
//     Function: LightLensNode::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void LightLensNode::
write(ostream &out, int indent_level) const {
  LensNode::write(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: LightLensNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void LightLensNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  LensNode::write_datagram(manager, dg);
  Light::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: LightLensNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new LightLensNode.
////////////////////////////////////////////////////////////////////
void LightLensNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  LensNode::fillin(scan, manager);
  Light::fillin(scan, manager);
}
