// Filename: lensNode.cxx
// Created by:  drose (26Feb02)
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

#include "lensNode.h"
#include "geometricBoundingVolume.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle LensNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LensNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LensNode::
LensNode(const string &name) :
  PandaNode(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
LensNode::
LensNode(const LensNode &copy) :
  PandaNode(copy),
  _lens(copy._lens)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this PandaNode by the
//               indicated matrix, if it means anything to do so.  For
//               most kinds of PandaNodes, this does nothing.
////////////////////////////////////////////////////////////////////
void LensNode::
xform(const LMatrix4f &mat) {
  PandaNode::xform(mat);
  // We need to actually transform the lens here.
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *LensNode::
make_copy() const {
  return new LensNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::is_in_view
//       Access: Public
//  Description: Returns true if the given point is within the bounds
//               of the lens of the LensNode (i.e. if the camera can
//               see the point).
////////////////////////////////////////////////////////////////////
bool LensNode::
is_in_view(const LPoint3f &pos) {
  PT(BoundingVolume) bv = _lens->make_bounds();
  if (bv == (BoundingVolume *)NULL) {
    return false;
  }
  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bv);
  int ret = gbv->contains(pos);
  return (ret != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LensNode::
output(ostream &out) const {
  PandaNode::output(out);
  if (_lens != (Lens *)NULL) {
    out << " (";
    _lens->output(out);
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LensNode::
write(ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);
  if (_lens != (Lens *)NULL) {
    _lens->write(out, indent_level + 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               LensNode.
////////////////////////////////////////////////////////////////////
void LensNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void LensNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);

  // We should actually write out the lens.  Easy to do, but not
  // immediately pressing; I hope no one gets burned by the omission.
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int LensNode::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type LensNode is encountered
//               in the Bam file.  It should create the LensNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *LensNode::
make_from_bam(const FactoryParams &params) {
  LensNode *node = new LensNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new LensNode.
////////////////////////////////////////////////////////////////////
void LensNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
}
