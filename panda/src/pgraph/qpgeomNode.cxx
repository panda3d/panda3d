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
#include "qpgeomTransformer.h"
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
//     Function: qpGeomNode::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeomNode::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  int num_geoms = _geoms.size();
  nassertv(num_geoms == (int)(PN_uint16)num_geoms);
  dg.add_uint16(num_geoms);
  
  Geoms::const_iterator gi;
  for (gi = _geoms.begin(); gi != _geoms.end(); ++gi) {
    const GeomEntry &entry = (*gi);
    manager->write_pointer(dg, entry._geom);
    manager->write_pointer(dg, entry._state);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int qpGeomNode::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  // Get the geom and state pointers.
  Geoms::iterator gi;
  for (gi = _geoms.begin(); gi != _geoms.end(); ++gi) {
    GeomEntry &entry = (*gi);
    entry._geom = DCAST(Geom, p_list[pi++]);
    entry._state = DCAST(RenderState, p_list[pi++]);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeomNode.
////////////////////////////////////////////////////////////////////
void qpGeomNode::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  int num_geoms = scan.get_uint16();
  // Read the list of geoms and states.  Push back a NULL for each one.
  _geoms.reserve(num_geoms);
  for (int i = 0; i < num_geoms; i++) {
    manager->read_pointer(scan);
    manager->read_pointer(scan);
    _geoms.push_back(GeomEntry(NULL, NULL));
  }
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
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
qpGeomNode::
qpGeomNode(const qpGeomNode &copy) :
  PandaNode(copy),
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
qpGeomNode::
~qpGeomNode() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated PandaNode that is a shallow
//               copy of this one.  It will be a different pointer,
//               but its internal data may or may not be shared with
//               that of the original PandaNode.  No children will be
//               copied.
////////////////////////////////////////////////////////////////////
PandaNode *qpGeomNode::
make_copy() const {
  return new qpGeomNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this node by the indicated
//               matrix, if it means anything to do so.  For most
//               kinds of nodes, this does nothing.
//
//               For a GeomNode, this does the right thing, but it is
//               better to use a GeomTransformer instead, since it
//               will share the new arrays properly between different
//               GeomNodes.
////////////////////////////////////////////////////////////////////
void qpGeomNode::
xform(const LMatrix4f &mat) {
  qpGeomTransformer transformer;
  transformer.transform_vertices(this, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::combine_with
//       Access: Public, Virtual
//  Description: Collapses this node with the other node, if possible,
//               and returns a pointer to the combined node, or NULL
//               if the two nodes cannot safely be combined.
//
//               The return value may be this, other, or a new node
//               altogether.
//
//               This function is called from GraphReducer::flatten(),
//               and need not deal with children; its job is just to
//               decide whether to collapse the two nodes and what the
//               collapsed node should look like.
////////////////////////////////////////////////////////////////////
PandaNode *qpGeomNode::
combine_with(PandaNode *other) {
  if (is_exact_type(get_class_type()) &&
      other->is_exact_type(get_class_type())) {
    // Two GeomNodes can combine by moving Geoms from one to the other.
    qpGeomNode *gother = DCAST(qpGeomNode, other);
    add_geoms_from(gother);
    return this;
  }

  return PandaNode::combine_with(other);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::add_geoms_from
//       Access: Published
//  Description: Copies the Geoms (and their associated RenderStates)
//               from the indicated GeomNode into this one.
////////////////////////////////////////////////////////////////////
void qpGeomNode::
add_geoms_from(const qpGeomNode *other) {
  CDReader cdata_other(other->_cycler);
  CDWriter cdata(_cycler);

  Geoms::const_iterator gi;
  for (gi = cdata_other->_geoms.begin(); 
       gi != cdata_other->_geoms.end(); 
       ++gi) {
    const GeomEntry &entry = (*gi);
    cdata->_geoms.push_back(entry);
  }
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomNode::write_geoms
//       Access: Published
//  Description: Writes a short description of all the Geoms in the
//               node.
////////////////////////////////////////////////////////////////////
void qpGeomNode::
write_geoms(ostream &out, int indent_level) const {
  CDReader cdata(_cycler);
  write(out, indent_level);
  Geoms::const_iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    const GeomEntry &entry = (*gi);
    indent(out, indent_level + 2) 
      << *entry._geom << " " << *entry._state << "\n";
  }
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
  write(out, indent_level);
  Geoms::const_iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    const GeomEntry &entry = (*gi);
    indent(out, indent_level + 2) 
      << *entry._geom << " " << *entry._state << "\n";
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
//     Function: qpGeomNode::recompute_internal_bound
//       Access: Protected, Virtual
//  Description: Called when needed to recompute the node's
//               _internal_bound object.  Nodes that contain anything
//               of substance should redefine this to do the right
//               thing.
////////////////////////////////////////////////////////////////////
BoundingVolume *qpGeomNode::
recompute_internal_bound() {
  // First, get ourselves a fresh, empty bounding volume.
  BoundingVolume *bound = PandaNode::recompute_internal_bound();
  nassertr(bound != (BoundingVolume *)NULL, bound);

  // Now actually compute the bounding volume by putting it around all
  // of our geoms' bounding volumes.
  pvector<const BoundingVolume *> child_volumes;

  CDReader cdata(_cycler);
  Geoms::const_iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    const GeomEntry &entry = (*gi);
    child_volumes.push_back(&entry._geom->get_bound());
  }

  const BoundingVolume **child_begin = &child_volumes[0];
  const BoundingVolume **child_end = child_begin + child_volumes.size();

  bound->around(child_begin, child_end);
  return bound;
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
  manager->write_cdata(dg, _cycler);
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
  manager->read_cdata(scan, _cycler);
}
