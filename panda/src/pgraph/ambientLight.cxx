// Filename: ambientLight.cxx
// Created by:  mike (09Jan97)
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

#include "ambientLight.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle AmbientLight::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AmbientLight::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
AmbientLight::
AmbientLight(const string &name) : 
  LightNode(name) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: AmbientLight::Copy Constructor
//       Access: Protected
//  Description: Do not call the copy constructor directly; instead,
//               use make_copy() or copy_subgraph() to make a copy of
//               a node.
////////////////////////////////////////////////////////////////////
AmbientLight::
AmbientLight(const AmbientLight &copy) :
  LightNode(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AmbientLight::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated PandaNode that is a shallow
//               copy of this one.  It will be a different pointer,
//               but its internal data may or may not be shared with
//               that of the original PandaNode.  No children will be
//               copied.
////////////////////////////////////////////////////////////////////
PandaNode *AmbientLight::
make_copy() const {
  return new AmbientLight(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: AmbientLight::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void AmbientLight::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";
  indent(out, indent_level + 2)
    << "color " << get_color() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: AmbientLight::bind
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void AmbientLight::
bind(GraphicsStateGuardianBase *, int) {
  // AmbientLights aren't bound to light id's; this function should
  // never be called.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: AmbientLight::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               AmbientLight.
////////////////////////////////////////////////////////////////////
void AmbientLight::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: AmbientLight::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void AmbientLight::
write_datagram(BamWriter *manager, Datagram &dg) {
  LightNode::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: AmbientLight::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type AmbientLight is encountered
//               in the Bam file.  It should create the AmbientLight
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *AmbientLight::
make_from_bam(const FactoryParams &params) {
  AmbientLight *node = new AmbientLight("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: AmbientLight::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new AmbientLight.
////////////////////////////////////////////////////////////////////
void AmbientLight::
fillin(DatagramIterator &scan, BamReader *manager) {
  LightNode::fillin(scan, manager);
}
