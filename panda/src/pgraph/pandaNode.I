// Filename: pandaNode.I
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


////////////////////////////////////////////////////////////////////
//     Function: PandaNode::DownConnection::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::DownConnection::
DownConnection(PandaNode *child, int sort) :
  _child(child),
  _sort(sort)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::DownConnection::operator <
//       Access: Public
//  Description: Provides a partial ordering on the children of a node
//               so that they are ranked first in sort order, and then
//               (by virtue of the ordered_vector) in the order they
//               were added.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::DownConnection::
operator < (const DownConnection &other) const {
  return _sort < other._sort;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::DownConnection::get_child
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::DownConnection::
get_child() const {
  return _child;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::DownConnection::get_sort
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::DownConnection::
get_sort() const {
  return _sort;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::CData::
CData() {
  _state_changes = RenderState::make_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Children::
Children(const PipelineCycler<PandaNode::CData> &cycler) :
  _cdata(cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Children::
Children(const PandaNode::Children &copy) :
  _cdata(copy._cdata)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::get_num_children
//       Access: Public
//  Description: Returns the number of children of the node.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::Children::
get_num_children() const {
  return _cdata->_down.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::get_child
//       Access: Public
//  Description: Returns the nth child of the node.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::Children::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_cdata->_down.size(), NULL);
  return _cdata->_down[n].get_child();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::
PandaNode(const string &name) :
  Namable(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_num_parents
//       Access: Published
//  Description: Returns the number of parent nodes this node has.  If
//               this number is greater than 1, the node has been
//               multiply instanced.  The order of the parent nodes is
//               not meaningful and is not related to the order in
//               which the node was instanced to them.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
get_num_parents() const {
  CDR(CData) cdata(_cycler);
  return cdata->_up.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_parent
//       Access: Published
//  Description: Returns the nth parent node of this node.  See
//               get_num_parents().
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::
get_parent(int n) const {
  CDR(CData) cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_up.size(), NULL);
  return cdata->_up[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_parent
//       Access: Published
//  Description: Returns the index of the indicated parent node, if it
//               is a parent, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
find_parent(PandaNode *node) const {
  CDR(CData) cdata(_cycler);
  Up::const_iterator ui = cdata->_up.find(node);
  if (ui == cdata->_up.end()) {
    return -1;
  }
  return ui - cdata->_up.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_num_children
//       Access: Published
//  Description: Returns the number of child nodes this node has.  The
//               order of the child nodes *is* meaningful and is based
//               on the sort number that was passed to add_child(),
//               and also on the order in which the nodes were added.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
get_num_children() const {
  CDR(CData) cdata(_cycler);
  return cdata->_down.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_child
//       Access: Published
//  Description: Returns the nth child node of this node.  See
//               get_num_children().
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::
get_child(int n) const {
  CDR(CData) cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_down.size(), NULL);
  return cdata->_down[n].get_child();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_sort
//       Access: Published
//  Description: Returns the sort index of the nth child node of this
//               node (that is, the number that was passed to
//               add_child()).  See get_num_children().
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
get_sort(int n) const {
  CDR(CData) cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_down.size(), -1);
  return cdata->_down[n].get_sort();
}


////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_attrib
//       Access: Published
//  Description: Adds the indicated render attribute to the scene
//               graph on this node.  This attribute will now apply to
//               this node and everything below.  If there was already
//               an attribute of the same type, it is replaced.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
set_attrib(const RenderAttrib *attrib, int override) {
  CDW(CData) cdata(_cycler);
  cdata->_state_changes = cdata->_state_changes->add(attrib, override);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_attrib
//       Access: Published
//  Description: Returns the render attribute of the indicated type,
//               if it is defined on the node, or NULL if it is not.
//               This checks only what is set on this particular node
//               level, and has nothing to do with what render
//               attributes may be inherited from parent nodes.
////////////////////////////////////////////////////////////////////
INLINE const RenderAttrib *PandaNode::
get_attrib(TypeHandle type) const {
  CDR(CData) cdata(_cycler);
  int index = cdata->_state_changes->find_attrib(type);
  if (index >= 0) {
    return cdata->_state_changes->get_attrib(index);
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_attrib
//       Access: Published
//  Description: Removes the render attribute of the given type from
//               this node.  This node, and the subgraph below, will
//               now inherit the indicated render attribute from the
//               nodes above this one.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
clear_attrib(TypeHandle type) {
  CDW(CData) cdata(_cycler);
  cdata->_state_changes = cdata->_state_changes->remove(type);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_state
//       Access: Published
//  Description: Sets the complete RenderState that will be applied to
//               all nodes at this level and below.  (The actual state
//               that will be applied to lower nodes is based on the
//               composition of RenderStates from above this node as
//               well).  This completely replaces whatever has been
//               set on this node via repeated calls to set_attrib().
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
set_state(const RenderState *state) {
  CDW(CData) cdata(_cycler);
  cdata->_state_changes = state;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_state
//       Access: Published
//  Description: Returns the complete RenderState that will be applied
//               to all nodes at this level and below, as set on this
//               node.  This returns only the RenderState set on this
//               particular node, and has nothing to do with state
//               that might be inherited from above.
////////////////////////////////////////////////////////////////////
INLINE const RenderState *PandaNode::
get_state() const {
  CDR(CData) cdata(_cycler);
  return cdata->_state_changes;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_state
//       Access: Published
//  Description: Resets this node to leave the render state alone.
//               Nodes at this level and below will once again inherit
//               their render state unchanged from the nodes above
//               this level.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
clear_state() {
  CDW(CData) cdata(_cycler);
  cdata->_state_changes = RenderState::make_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_children
//       Access: Public
//  Description: Returns an object that can be used to walk through
//               the list of children of the node.  When you intend to
//               visit multiple children, using this is slightly
//               faster than calling get_child() directly on the
//               PandaNode, since this object keeps the PipelineCycler
//               open the whole time.
//
//               However, this object does not protect you from
//               self-modifying loops (e.g. adding or removing
//               children during traversal).
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Children PandaNode::
get_children() const {
  return Children(_cycler);
}

