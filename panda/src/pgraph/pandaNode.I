// Filename: pandaNode.I
// Created by:  drose (20Feb02)
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
//     Function: PandaNode::DownConnection::set_child
//       Access: Public
//  Description: This is only called by PandaNode::replace_child().
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::DownConnection::
set_child(PandaNode *child) {
  _child = child;
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
  _effects = RenderEffects::make_empty();
  _transform = TransformState::make_identity();
  _prev_transform = TransformState::make_identity();
  _draw_mask = DrawMask::all_on();
  _into_collide_mask = CollideMask::all_off();
  _net_collide_mask = CollideMask::all_off();
  _stale_child_cache = true;
  _fixed_internal_bound = false;
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
//     Function: PandaNode::Children::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::Children::
operator = (const PandaNode::Children &copy) {
  _cdata = copy._cdata;
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
//     Function: PandaNode::ChildrenCopy::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::ChildrenCopy::
ChildrenCopy(const PandaNode::ChildrenCopy &copy) :
  _list(copy._list)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::ChildrenCopy::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::ChildrenCopy::
operator = (const PandaNode::ChildrenCopy &copy) {
  _list = copy._list;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::ChildrenCopy::get_num_children
//       Access: Public
//  Description: Returns the number of children of the node.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::ChildrenCopy::
get_num_children() const {
  return _list.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::ChildrenCopy::get_child
//       Access: Public
//  Description: Returns the nth child of the node.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::ChildrenCopy::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_list.size(), NULL);
  return _list[n];
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
  return do_find_parent(node, cdata);
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
//     Function: PandaNode::find_child
//       Access: Published
//  Description: Returns the index of the indicated child node, if it
//               is a child, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
find_child(PandaNode *node) const {
  CDReader cdata(_cycler);
  return do_find_child(node, cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_child
//       Access: Published
//  Description: Removes the nth child from the node.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
remove_child(int n) {
  CDWriter cdata(_cycler);
  nassertv(n >= 0 && n < (int)cdata->_down.size());
  
  PT(PandaNode) child_node = cdata->_down[n].get_child();
  CDWriter cdata_child(child_node->_cycler);

  do_remove_child(n, child_node, 
                  Thread::get_current_pipeline_stage(),
                  cdata, cdata_child);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::stash_child
//       Access: Published
//  Description: Stashes the indicated child node.  This removes the
//               child from the list of active children and puts it on
//               a special list of stashed children.  This child node
//               no longer contributes to the bounding volume of the
//               PandaNode, and is not visited in normal traversals.
//               It is invisible and uncollidable.  The child may
//               later be restored by calling unstash_child().
//
//               This function returns true if the child node was
//               successfully stashed, or false if it was not a child
//               of the node in the first place (e.g. it was
//               previously stashed).
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
stash_child(PandaNode *child_node) {
  int child_index = find_child(child_node);
  if (child_index < 0) {
    return false;
  }
  stash_child(child_index);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::unstash_child
//       Access: Published
//  Description: Returns the indicated stashed node to normal child
//               status.  This removes the child from the list of
//               stashed children and puts it on the normal list of
//               active children.  This child node once again
//               contributes to the bounding volume of the PandaNode,
//               and will be visited in normal traversals.  It is
//               visible and collidable.
//
//               This function returns true if the child node was
//               successfully stashed, or false if it was not a child
//               of the node in the first place (e.g. it was
//               previously stashed).
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
unstash_child(PandaNode *child_node) {
  int stashed_index = find_stashed(child_node);
  if (stashed_index < 0) {
    return false;
  }
  unstash_child(stashed_index);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_num_stashed
//       Access: Published
//  Description: Returns the number of stashed nodes this node has.
//               These are former children of the node that have been
//               moved to the special stashed list via stash_child().
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
get_num_stashed() const {
  CDReader cdata(_cycler);
  return cdata->_stashed.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_stashed
//       Access: Published
//  Description: Returns the nth stashed node of this node.  See
//               get_num_stashed().
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::
get_stashed(int n) const {
  CDReader cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_stashed.size(), NULL);
  return cdata->_stashed[n].get_child();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_stashed_sort
//       Access: Published
//  Description: Returns the sort index of the nth stashed node of this
//               node (that is, the number that was passed to
//               add_child()).  See get_num_stashed().
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
get_stashed_sort(int n) const {
  CDReader cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_stashed.size(), -1);
  return cdata->_stashed[n].get_sort();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_stashed
//       Access: Published
//  Description: Returns the index of the indicated stashed node, if
//               it is a stashed child, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
find_stashed(PandaNode *node) const {
  CDReader cdata(_cycler);
  return do_find_stashed(node, cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_stashed
//       Access: Published
//  Description: Removes the nth stashed child from the node.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
remove_stashed(int n) {
  CDWriter cdata(_cycler);
  nassertv(n >= 0 && n < (int)cdata->_stashed.size());
  
  PT(PandaNode) child_node = cdata->_stashed[n].get_child();
  CDWriter cdata_child(child_node->_cycler);

  do_remove_stashed(n, child_node, Thread::get_current_pipeline_stage(),
                    cdata, cdata_child);
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
//     Function: PandaNode::has_attrib
//       Access: Published
//  Description: Returns true if there is a render attribute of the
//               indicated type defined on this node, or false if
//               there is not.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
has_attrib(TypeHandle type) const {
  CDReader cdata(_cycler);
  int index = cdata->_state->find_attrib(type);
  return (index >= 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_effect
//       Access: Published
//  Description: Returns the render effect of the indicated type,
//               if it is defined on the node, or NULL if it is not.
////////////////////////////////////////////////////////////////////
INLINE const RenderEffect *PandaNode::
get_effect(TypeHandle type) const {
  CDReader cdata(_cycler);
  int index = cdata->_effects->find_effect(type);
  if (index >= 0) {
    return cdata->_effects->get_effect(index);
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::has_effect
//       Access: Published
//  Description: Returns true if there is a render effect of the
//               indicated type defined on this node, or false if
//               there is not.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
has_effect(TypeHandle type) const {
  CDReader cdata(_cycler);
  int index = cdata->_effects->find_effect(type);
  return (index >= 0);
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
//     Function: PandaNode::get_effects
//       Access: Published
//  Description: Returns the complete RenderEffects that will be
//               applied to this node.
////////////////////////////////////////////////////////////////////
INLINE const RenderEffects *PandaNode::
get_effects() const {
  CDReader cdata(_cycler);
  return cdata->_effects;
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
//     Function: PandaNode::get_prev_transform
//       Access: Published
//  Description: Returns the transform that has been set as this
//               node's "previous" position.  See
//               set_prev_transform().
////////////////////////////////////////////////////////////////////
INLINE const TransformState *PandaNode::
get_prev_transform() const {
  CDReader cdata(_cycler);
  return cdata->_prev_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_tag
//       Access: Published
//  Description: Retrieves the user-defined value that was previously
//               set on this node for the particular key, if any.  If
//               no value has been previously set, returns the empty
//               string.
////////////////////////////////////////////////////////////////////
INLINE string PandaNode::
get_tag(const string &key) const {
  CDReader cdata(_cycler);
  TagData::const_iterator ti;
  ti = cdata->_tag_data.find(key);
  if (ti != cdata->_tag_data.end()) {
    return (*ti).second;
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::has_tag
//       Access: Published
//  Description: Returns true if a value has been defined on this node
//               for the particular key (even if that value is the
//               empty string), or false if no value has been set.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
has_tag(const string &key) const {
  CDReader cdata(_cycler);
  TagData::const_iterator ti;
  ti = cdata->_tag_data.find(key);
  return (ti != cdata->_tag_data.end());
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::has_tags
//       Access: Published
//  Description: Returns true if the node has any tags (or any Python
//               tags) at all, false if it has none.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
has_tags() const {
  CDReader cdata(_cycler);
  if (!cdata->_tag_data.empty()) {
    return true;
  }
#ifdef HAVE_PYTHON
  if (!cdata->_python_tag_data.empty()) {
    return true;
  }
#endif  // HAVE_PYTHON
  return false;
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
//     Function: PandaNode::get_draw_mask
//       Access: Published
//  Description: Returns the hide/show bits of this particular node.
//               See set_draw_mask().
////////////////////////////////////////////////////////////////////
INLINE DrawMask PandaNode::
get_draw_mask() const {
  CDReader cdata(_cycler);
  return cdata->_draw_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_into_collide_mask
//       Access: Published
//  Description: Returns the "into" collide mask for this node.
////////////////////////////////////////////////////////////////////
INLINE CollideMask PandaNode::
get_into_collide_mask() const {
  CDReader cdata(_cycler);
  return cdata->_into_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_bound
//       Access: Published
//  Description: Returns the node's external bounding volume.  This is
//               the bounding volume around the node and all of its
//               children.
////////////////////////////////////////////////////////////////////
INLINE const BoundingVolume *PandaNode::
get_bound() const {
  return BoundedObject::get_bound();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_bound
//       Access: Published
//  Description: Returns the node's external bounding volume.  This is
//               the bounding volume around the node and all of its
//               children.
////////////////////////////////////////////////////////////////////
INLINE const BoundingVolume *PandaNode::
get_bound(int pipeline_stage) const {
  return BoundedObject::get_bound(pipeline_stage);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_internal_bound
//       Access: Published
//  Description: Returns the node's internal bounding volume.  This is
//               the bounding volume around the node alone, without
//               including children.
////////////////////////////////////////////////////////////////////
INLINE const BoundingVolume *PandaNode::
get_internal_bound() const {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  return get_internal_bound(pipeline_stage);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_internal_bound
//       Access: Published
//  Description: Returns the node's internal bounding volume.  This is
//               the bounding volume around the node alone, without
//               including children.
////////////////////////////////////////////////////////////////////
INLINE const BoundingVolume *PandaNode::
get_internal_bound(int pipeline_stage) const {
  CDStageReader cdata(_cycler, pipeline_stage);
  if (!cdata->_fixed_internal_bound && 
      (is_bound_stale(pipeline_stage) || 
       _internal_bound.is_bound_stale(pipeline_stage))) {
    ((PandaNode *)this)->recompute_internal_bound(pipeline_stage);
  }
  return _internal_bound.get_bound(pipeline_stage);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::changed_internal_bound
//       Access: Protected
//  Description: Should be called whenever you adjust the
//               _internal_bound member, to force the external
//               bounding volume to be recomputed.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
changed_internal_bound(int pipeline_stage) {
  BoundedObject::mark_bound_stale(pipeline_stage);
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

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_children_copy
//       Access: Public
//  Description: Returns an object that can be used to walk through
//               the list of children of the node.  Unlike
//               get_children(), this function actually returns an
//               object that protects you from self-modifying loops,
//               because it makes and returns a copy of the complete
//               children list.
////////////////////////////////////////////////////////////////////
INLINE PandaNode::ChildrenCopy PandaNode::
get_children_copy() const {
  CDReader cdata(_cycler);
  return ChildrenCopy(cdata);
}


////////////////////////////////////////////////////////////////////
//     Function: PandaNode::mark_child_cache_stale
//       Access: Private
//  Description: Indicates that the _child_cache value that is
//               propagated upwards has changed for this node.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
mark_child_cache_stale(int pipeline_stage, CData *cdata) {
  if (!cdata->_stale_child_cache) {
    force_child_cache_stale(pipeline_stage, cdata);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::do_find_parent
//       Access: Private
//  Description: The private implementation of find_parent().
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
do_find_parent(PandaNode *node, const CData *cdata) const {
  Up::const_iterator ui = cdata->_up.find(UpConnection(node));
  if (ui == cdata->_up.end()) {
    return -1;
  }
  return ui - cdata->_up.begin();
}
