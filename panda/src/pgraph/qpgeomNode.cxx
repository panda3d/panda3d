// Filename: qpgeomNode.cxx
// Created by:  drose (23Feb02)
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

#include "qpgeomNode.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "indent.h"

TypeHandle qpGeomNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
qpGeomNode::CData::
CData(const qpGeomNode::CData &copy) :
  _geoms(copy._geoms)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *qpGeomNode::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
qpGeomNode::
qpGeomNode(const string &name) :
  PandaNode(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
qpGeomNode::
qpGeomNode(const qpGeomNode &copy) :
  PandaNode(copy)
{
  // Copy the other node's _geoms.
  CDReader copy_cdata(copy._cycler);
  CDWriter cdata(_cycler);
  cdata->_geoms = copy_cdata->_geoms;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void qpGeomNode::
operator = (const qpGeomNode &copy) {
  PandaNode::operator = (copy);

  // Copy the other node's _geoms.
  CDReader copy_cdata(copy._cycler);
  CDWriter cdata(_cycler);
  cdata->_geoms = copy_cdata->_geoms;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
qpGeomNode::
~qpGeomNode() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::write_verbose
//       Access: Published
//  Description: Writes a detailed description of all the Geoms in the
//               node.
////////////////////////////////////////////////////////////////////
void qpGeomNode::
write_verbose(ostream &out, int indent_level) const {
  CDReader cdata(_cycler);
  PandaNode::write(out, indent_level);
  Geoms::const_iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    const GeomEntry &entry = (*gi);
    indent(out, indent_level + 2) 
      << *entry._geom << " (" << *entry._state << ")\n";
    entry._geom->write_verbose(out, indent_level + 4);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomNode::
output(ostream &out) const {
  PandaNode::output(out);
  out << " (" << get_num_geoms() << " geoms)";
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomNode::
write(ostream &out, int indent_level) const {
  CDReader cdata(_cycler);
  PandaNode::write(out, indent_level);
  Geoms::const_iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    const GeomEntry &entry = (*gi);
    indent(out, indent_level + 2) 
      << *entry._geom << " (" << *entry._state << ")\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::is_geom_node
//       Access: Public, Virtual
//  Description: A simple downcast check.  Returns true if this kind
//               of node happens to inherit from GeomNode, false
//               otherwise.
//
//               This is provided as a a faster alternative to calling
//               is_of_type(GeomNode::get_class_type()), since this
//               test is so important to rendering.
////////////////////////////////////////////////////////////////////
bool qpGeomNode::
is_geom_node() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeomNode.
////////////////////////////////////////////////////////////////////
void qpGeomNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeomNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeomNode is encountered
//               in the Bam file.  It should create the qpGeomNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeomNode::
make_from_bam(const FactoryParams &params) {
  qpGeomNode *node = new qpGeomNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeomNode.
////////////////////////////////////////////////////////////////////
void qpGeomNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
}
