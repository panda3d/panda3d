// Filename: pandaNode.cxx
// Created by:  drose (20Feb02)
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

#include "pandaNode.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indent.h"


TypeHandle PandaNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CData::
CData(const PandaNode::CData &copy) :
  _down(copy._down),
  _up(copy._up),
  _node_bounds(copy._node_bounds),
  _subgraph_bounds(copy._subgraph_bounds),
  _state_changes(copy._state_changes)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *PandaNode::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::
PandaNode(const string &name) :
  Namable(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::
PandaNode(const PandaNode &copy) :
  TypedWritable(copy),
  Namable(copy),
  ReferenceCount(copy)
{
  // Copying a node does not copy its children.

  // Copy the other node's bounding volume.
  CDReader copy_cdata(copy._cycler);
  CDWriter cdata(_cycler);
  cdata->_node_bounds = copy_cdata->_node_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PandaNode::
operator = (const PandaNode &copy) {
  TypedWritable::operator = (copy);
  Namable::operator = (copy);
  ReferenceCount::operator = (copy);

  // Copy the other node's bounding volume.
  CDReader copy_cdata(copy._cycler);
  CDWriter cdata(_cycler);
  cdata->_node_bounds = copy_cdata->_node_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::
~PandaNode() {
  // We shouldn't have any parents left by the time we destruct, or
  // there's a refcount fault somewhere.
  CDReader cdata(_cycler);
  nassertv(cdata->_up.empty());

  remove_all_children();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_child
//       Access: Published
//  Description: Returns the index of the indicated child node, if it
//               is a child, or -1 if it is not.
////////////////////////////////////////////////////////////////////
int PandaNode::
find_child(PandaNode *node) const {
  CDReader cdata(_cycler);

  // We have to search for the child by brute force, since we don't
  // know what sort index it was added as.
  Down::const_iterator ci;
  for (ci = cdata->_down.begin(); ci != cdata->_down.end(); ++ci) {
    if ((*ci).get_child() == node) {
      return ci - cdata->_down.begin();
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::add_child
//       Access: Published
//  Description: Adds a new child to the node.  The child is added in
//               the relative position indicated by sort; if all
//               children have the same sort index, the child is added
//               at the end.
//
//               If the same child is added to a node more than once,
//               the previous instance is first removed.
//
//               The return value is the index of the new child.
////////////////////////////////////////////////////////////////////
int PandaNode::
add_child(PandaNode *child, int sort) {
  remove_child(child);
  CDWriter cdata(_cycler);
  CDWriter cdata_child(child->_cycler);

  Down::iterator ci = cdata->_down.insert(DownConnection(child, sort));
  cdata_child->_up.insert(this);

  return ci - cdata->_down.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_child
//       Access: Published
//  Description: Removes the nth child from the node.
////////////////////////////////////////////////////////////////////
void PandaNode::
remove_child(int n) {
  CDWriter cdata(_cycler);
  nassertv(n >= 0 && n < (int)cdata->_down.size());

  PandaNode *child = cdata->_down[n].get_child();
  CDWriter cdata_child(child->_cycler);

  cdata->_down.erase(cdata->_down.begin() + n);
  int num_erased = cdata_child->_up.erase(this);
  nassertv(num_erased == 1);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_child
//       Access: Published
//  Description: Removes the indicated child from the node.  Returns
//               true if the child was removed, false if it was not
//               already a child of the node.
////////////////////////////////////////////////////////////////////
bool PandaNode::
remove_child(PandaNode *child) {
  CDWriter cdata_child(child->_cycler);

  // First, look for and remove this node from the child's parent
  // list.
  int num_erased = cdata_child->_up.erase(this);
  if (num_erased == 0) {
    // No such node; it wasn't our child to begin with.
    return false;
  }

  CDWriter cdata(_cycler);

  // Now, look for and remove the child node from our down list.
  Down::iterator ci;
  for (ci = cdata->_down.begin(); ci != cdata->_down.end(); ++ci) {
    if ((*ci).get_child() == child) {
      cdata->_down.erase(ci);
      return true;
    }
  }

  // We shouldn't get here unless there was a parent-child mismatch.
  nassertr(false, false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_all_children
//       Access: Published
//  Description: Removes all the children from the node at once.
////////////////////////////////////////////////////////////////////
void PandaNode::
remove_all_children() {
  CDWriter cdata(_cycler);
  Down::iterator ci;
  for (ci = cdata->_down.begin(); ci != cdata->_down.end(); ++ci) {
    PandaNode *child = (*ci).get_child();
    CDWriter child_cdata(child->_cycler);
    child_cdata->_up.erase(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PandaNode::
output(ostream &out) const {
  out << get_type() << " " << get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PandaNode::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this;
  CDReader cdata(_cycler);
  if (!cdata->_state_changes->is_empty()) {
    out << " (" << *cdata->_state_changes << ")";
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::is_geom_node
//       Access: Public, Virtual
//  Description: A simple downcast check.  Returns true if this kind
//               of node happens to inherit from GeomNode, false
//               otherwise.
//
//               This is provided as a a faster alternative to calling
//               is_of_type(GeomNode::get_class_type()), since this
//               test is so important to rendering.
////////////////////////////////////////////////////////////////////
bool PandaNode::
is_geom_node() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               PandaNode.
////////////////////////////////////////////////////////////////////
void PandaNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PandaNode::
write_datagram(BamWriter *manager, Datagram &dg) {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type PandaNode is encountered
//               in the Bam file.  It should create the PandaNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *PandaNode::
make_from_bam(const FactoryParams &params) {
  PandaNode *node = new PandaNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new PandaNode.
////////////////////////////////////////////////////////////////////
void PandaNode::
fillin(DatagramIterator &scan, BamReader *manager) {
}
