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
//     Function: PandaNode::UpConnection::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::UpConnection::
UpConnection(PandaNode *parent) :
  _parent(parent)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::UpConnection::operator <
//       Access: Public
//  Description: Sorts the up connections of a node by pointer.  This
//               is different from the down connections of a node,
//               which are sorted by the specified _sort number.  This
//               makes it easy to locate a particular parent of a node
//               by pointer, or to test for a parent-child
//               relationship given two node pointers.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::UpConnection::
operator < (const UpConnection &other) const {
  return _parent < other._parent;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::UpConnection::get_parent
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::UpConnection::
get_parent() const {
  return _parent;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::CData::
CData() {
  _state = RenderState::make_empty();
  _transform = TransformState::make_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Children::
Children(const PandaNode::CDReader &cdata) :
  _cdata(cdata)
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
  CDReader cdata(_cycler);
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
  CDReader cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_up.size(), NULL);
  return cdata->_up[n].get_parent();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_parent
//       Access: Published
//  Description: Returns the index of the indicated parent node, if it
//               is a parent, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
find_parent(PandaNode *node) const {
  CDReader cdata(_cycler);
  Up::const_iterator ui = cdata->_up.find(UpConnection(node));
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
  CDReader cdata(_cycler);
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
  CDReader cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_down.size(), NULL);
  return cdata->_down[n].get_child();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_child_sort
//       Access: Published
//  Description: Returns the sort index of the nth child node of this
//               node (that is, the number that was passed to
//               add_child()).  See get_num_children().
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
get_child_sort(int n) const {
  CDReader cdata(_cycler);
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
  CDWriter cdata(_cycler);
  cdata->_state = cdata->_state->add_attrib(attrib, override);
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
  CDReader cdata(_cycler);
  int index = cdata->_state->find_attrib(type);
  if (index >= 0) {
    return cdata->_state->get_attrib(index);
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
  CDWriter cdata(_cycler);
  cdata->_state = cdata->_state->remove_attrib(type);
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
  CDWriter cdata(_cycler);
  cdata->_state = state;
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
  CDReader cdata(_cycler);
  return cdata->_state;
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
  CDWriter cdata(_cycler);
  cdata->_state = RenderState::make_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_transform
//       Access: Published
//  Description: Sets the transform that will be applied to this node
//               and below.  This defines a new coordinate space at
//               this point in the scene graph and below.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
set_transform(const TransformState *transform) {
  CDWriter cdata(_cycler);
  cdata->_transform = transform;
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_transform
//       Access: Published
//  Description: Returns the transform that has been set on this
//               particular node.  This is not the net transform from
//               the root, but simply the transform on this particular
//               node.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *PandaNode::
get_transform() const {
  CDReader cdata(_cycler);
  return cdata->_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_transform
//       Access: Published
//  Description: Resets the transform on this node to the identity
//               transform.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
clear_transform() {
  CDWriter cdata(_cycler);
  cdata->_transform = TransformState::make_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::ls
//       Access: Published
//  Description: Lists all the nodes at and below the current path
//               hierarchically.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
ls() const {
  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::ls
//       Access: Published
//  Description: Lists all the nodes at and below the current path
//               hierarchically.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
ls(ostream &out, int indent_level) const {
  r_list_descendants(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_bound
//       Access: Published
//  Description: Sets the type of the external bounding volume that is
//               placed around this node and all of its children.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
set_bound(BoundingVolumeType type) {
  BoundedObject::set_bound(type);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_bound
//       Access: Published
//  Description: Resets the internal bounding volume so that it is the
//               indicated volume.  The external bounding volume as
//               returned by get_bound() (which includes all of the
//               node's children) will be adjusted to include this
//               internal volume.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
set_bound(const BoundingVolume &volume) {
  _internal_bound.set_bound(volume);
  changed_internal_bound();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_bound
//       Access: Published
//  Description: Returns the node's external bounding volume.  This is
//               the bounding volume around the node and all of its
//               children.
////////////////////////////////////////////////////////////////////
INLINE const BoundingVolume &PandaNode::
get_bound() const {
  return BoundedObject::get_bound();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_internal_bound
//       Access: Published
//  Description: Returns the node's internal bounding volume.  This is
//               the bounding volume around the node alone, without
//               including children.
////////////////////////////////////////////////////////////////////
INLINE const BoundingVolume &PandaNode::
get_internal_bound() const {
  if (_internal_bound.is_bound_stale()) {
    ((PandaNode *)this)->recompute_internal_bound();
  }
  return _internal_bound.get_bound();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::changed_internal_bound
//       Access: Protected
//  Description: Should be called whenever you adjust the
//               _internal_bound member, to force the external
//               bounding volume to be recomputed.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
changed_internal_bound() {
  BoundedObject::mark_bound_stale();
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
  CDReader cdata(_cycler);
  return Children(cdata);
}

