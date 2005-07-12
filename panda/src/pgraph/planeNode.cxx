// Filename: planeNode.cxx
// Created by:  drose (11Jul02)
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

#include "planeNode.h"
#include "geometricBoundingVolume.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

UpdateSeq PlaneNode::_sort_seq;

TypeHandle PlaneNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *PlaneNode::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PlaneNode::CData::
write_datagram(BamWriter *, Datagram &dg) const {
  _plane.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Light.
////////////////////////////////////////////////////////////////////
void PlaneNode::CData::
fillin(DatagramIterator &scan, BamReader *) {
  _plane.read_datagram(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PlaneNode::
PlaneNode(const string &name) :
  PandaNode(name),
  _priority(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
PlaneNode::
PlaneNode(const PlaneNode &copy) :
  PandaNode(copy),
  _priority(copy._priority),
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *PlaneNode::
make_copy() const {
  return new PlaneNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this PandaNode by the
//               indicated matrix, if it means anything to do so.  For
//               most kinds of PandaNodes, this does nothing.
////////////////////////////////////////////////////////////////////
void PlaneNode::
xform(const LMatrix4f &mat) {
  PandaNode::xform(mat);
  CDWriter cdata(_cycler);
  cdata->_plane = cdata->_plane * mat;
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PlaneNode::
output(ostream &out) const {
  PandaNode::output(out);
  out << " " << get_plane();
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PlaneNode::
write(ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);
  get_plane().write(out, indent_level + 2);
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               PlaneNode.
////////////////////////////////////////////////////////////////////
void PlaneNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PlaneNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
  dg.add_int32(_priority);
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type PlaneNode is encountered
//               in the Bam file.  It should create the PlaneNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *PlaneNode::
make_from_bam(const FactoryParams &params) {
  PlaneNode *node = new PlaneNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new PlaneNode.
////////////////////////////////////////////////////////////////////
void PlaneNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
  _priority = scan.get_int32();
}
