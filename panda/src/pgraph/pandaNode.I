// Filename: pandaNode.I
// Created by:  drose (20Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


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
get_num_parents(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->get_up()->size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_parent
//       Access: Published
//  Description: Returns the nth parent node of this node.  See
//               get_num_parents().  Also see get_parents(), if your
//               intention is to iterate through the complete list of
//               parents; get_parents() is preferable in this case.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::
get_parent(int n, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  CPT(Up) up = cdata->get_up();
  nassertr(n >= 0 && n < (int)up->size(), NULL);
  return (*up)[n].get_parent();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_parent
//       Access: Published
//  Description: Returns the index of the indicated parent node, if it
//               is a parent, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
find_parent(PandaNode *node, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
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
get_num_children(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->get_down()->size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_child
//       Access: Published
//  Description: Returns the nth child node of this node.  See
//               get_num_children().  Also see get_children(), if your
//               intention is to iterate through the complete list of
//               children; get_children() is preferable in this case.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::
get_child(int n, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  CPT(Down) down = cdata->get_down();
  nassertr(n >= 0 && n < (int)down->size(), NULL);
  return (*down)[n].get_child();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_child_sort
//       Access: Published
//  Description: Returns the sort index of the nth child node of this
//               node (that is, the number that was passed to
//               add_child()).  See get_num_children().
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
get_child_sort(int n, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  CPT(Down) down = cdata->get_down();
  nassertr(n >= 0 && n < (int)down->size(), -1);
  return (*down)[n].get_sort();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_child
//       Access: Published
//  Description: Returns the index of the indicated child node, if it
//               is a child, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
find_child(PandaNode *node, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return do_find_child(node, cdata->get_down());
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
stash_child(PandaNode *child_node, Thread *current_thread) {
  int child_index = find_child(child_node, current_thread);
  if (child_index < 0) {
    return false;
  }
  stash_child(child_index, current_thread);
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
unstash_child(PandaNode *child_node, Thread *current_thread) {
  int stashed_index = find_stashed(child_node, current_thread);
  if (stashed_index < 0) {
    return false;
  }
  unstash_child(stashed_index, current_thread);
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
get_num_stashed(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->get_stashed()->size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_stashed
//       Access: Published
//  Description: Returns the nth stashed child of this node.  See
//               get_num_stashed().  Also see get_stashed(), if your
//               intention is to iterate through the complete list of
//               stashed children; get_stashed() is preferable in this
//               case.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::
get_stashed(int n, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  CPT(Down) stashed = cdata->get_stashed();
  nassertr(n >= 0 && n < (int)stashed->size(), NULL);
  return (*stashed)[n].get_child();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_stashed_sort
//       Access: Published
//  Description: Returns the sort index of the nth stashed node of this
//               node (that is, the number that was passed to
//               add_child()).  See get_num_stashed().
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
get_stashed_sort(int n, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  CPT(Down) stashed = cdata->get_stashed();
  nassertr(n >= 0 && n < (int)stashed->size(), -1);
  return (*stashed)[n].get_sort();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_stashed
//       Access: Published
//  Description: Returns the index of the indicated stashed node, if
//               it is a stashed child, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
find_stashed(PandaNode *node, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return do_find_child(node, cdata->get_stashed());
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
INLINE CPT(RenderAttrib) PandaNode::
get_attrib(TypeHandle type) const {
  CDReader cdata(_cycler);
  return cdata->_state->get_attrib(type);
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
INLINE CPT(RenderAttrib) PandaNode::
get_attrib(int slot) const {
  CDReader cdata(_cycler);
  return cdata->_state->get_attrib(slot);
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
  return cdata->_state->has_attrib(type);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::has_attrib
//       Access: Published
//  Description: Returns true if there is a render attribute of the
//               indicated type defined on this node, or false if
//               there is not.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
has_attrib(int slot) const {
  CDReader cdata(_cycler);
  return cdata->_state->has_attrib(slot);
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
  RenderAttribRegistry *reg = RenderAttribRegistry::quick_get_global_ptr();
  int slot = reg->get_slot(type);
  clear_attrib(slot);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_effect
//       Access: Published
//  Description: Returns the render effect of the indicated type,
//               if it is defined on the node, or NULL if it is not.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderEffect) PandaNode::
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
INLINE CPT(RenderState) PandaNode::
get_state(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_state.p();
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
clear_state(Thread *current_thread) {
  set_state(RenderState::make_empty(), current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_effects
//       Access: Published
//  Description: Returns the complete RenderEffects that will be
//               applied to this node.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderEffects) PandaNode::
get_effects(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_effects;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_effects
//       Access: Published
//  Description: Resets this node to have no render effects.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
clear_effects(Thread *current_thread) {
  set_effects(RenderEffects::make_empty(), current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_transform
//       Access: Published
//  Description: Returns the transform that has been set on this
//               particular node.  This is not the net transform from
//               the root, but simply the transform on this particular
//               node.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) PandaNode::
get_transform(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_transform.p();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_transform
//       Access: Published
//  Description: Resets the transform on this node to the identity
//               transform.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
clear_transform(Thread *current_thread) {
  set_transform(TransformState::make_identity(), current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_prev_transform
//       Access: Published
//  Description: Returns the transform that has been set as this
//               node's "previous" position.  See
//               set_prev_transform().
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) PandaNode::
get_prev_transform(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_prev_transform.p();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::has_dirty_prev_transform
//       Access: Published
//  Description: Returns true if this node has the
//               _dirty_prev_transform flag set, which indicates its
//               _prev_transform is different from its _transform
//               value (in pipeline stage 0).  In this case, the node
//               will be visited by reset_prev_transform().
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
has_dirty_prev_transform() const {
  return _dirty_prev_transform;
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
get_tag(const string &key, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
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
has_tag(const string &key, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
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
//     Function: PandaNode::get_overall_bit
//       Access: Published, Static
//  Description: Returns the special bit that, when specifically
//               cleared in the node's DrawMask, indicates that the
//               node is hidden to all cameras, regardless of the
//               remaining DrawMask bits.
////////////////////////////////////////////////////////////////////
INLINE DrawMask PandaNode::
get_overall_bit() {
  return _overall_bit;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_all_camera_mask
//       Access: Published, Static
//  Description: Returns a DrawMask that is appropriate for rendering
//               to all cameras.
////////////////////////////////////////////////////////////////////
INLINE DrawMask PandaNode::
get_all_camera_mask() {
  return ~_overall_bit;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::is_overall_hidden
//       Access: Published, Static
//  Description: Returns true if the node has been hidden to all
//               cameras by clearing its overall bit.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
is_overall_hidden() const {
  CDReader cdata(_cycler);
  return ((cdata->_draw_show_mask | ~cdata->_draw_control_mask) & _overall_bit).is_zero();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_overall_hidden
//       Access: Published
//  Description: Sets or clears the hidden flag.  When the hidden flag
//               is true, the node and all of its children are
//               invisible to all cameras, regardless of the setting
//               of any draw masks.  Setting the hidden flag to false
//               restores the previous visibility as established by
//               the draw masks.
//
//               This actually works by twiddling the reserved
//               _overall_bit in the node's draw mask, which has
//               special meaning.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
set_overall_hidden(bool hidden) {
  if (hidden) {
    adjust_draw_mask(DrawMask::all_off(), _overall_bit, DrawMask::all_off());
  } else {
    adjust_draw_mask(DrawMask::all_off(), DrawMask::all_off(), _overall_bit);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_draw_control_mask
//       Access: Published
//  Description: Returns the set of bits in draw_show_mask that are
//               considered meaningful.  See adjust_draw_mask().
////////////////////////////////////////////////////////////////////
INLINE DrawMask PandaNode::
get_draw_control_mask() const {
  CDReader cdata(_cycler);
  return cdata->_draw_control_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_draw_show_mask
//       Access: Published
//  Description: Returns the hide/show bits of this particular node.
//               See adjust_draw_mask().
////////////////////////////////////////////////////////////////////
INLINE DrawMask PandaNode::
get_draw_show_mask() const {
  CDReader cdata(_cycler);
  return cdata->_draw_show_mask;
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
//     Function: PandaNode::clear_bounds
//       Access: Published
//  Description: Reverses the effect of a previous call to
//               set_bounds(), and allows the node's bounding volume
//               to be automatically computed once more based on the
//               contents of the node.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
clear_bounds() {
  set_bounds((BoundingVolume *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_internal_bounds
//       Access: Published
//  Description: Returns the node's internal bounding volume.  This is
//               the bounding volume around the node alone, without
//               including children.  If the user has called
//               set_bounds(), it will be the specified bounding
//               volume.
////////////////////////////////////////////////////////////////////
INLINE CPT(BoundingVolume) PandaNode::
get_internal_bounds(Thread *current_thread) const {
  return get_internal_bounds(current_thread->get_pipeline_stage(),
                             current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_internal_vertices
//       Access: Published
//  Description: Returns the total number of vertices that will be
//               rendered by this particular node alone, not
//               accounting for its children.
//
//               This may not include all vertices for certain dynamic
//               effects.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
get_internal_vertices(Thread *current_thread) const {
  return get_internal_vertices(current_thread->get_pipeline_stage(),
                               current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::is_bounds_stale
//       Access: Published
//  Description: Returns true if the bounding volume of this node is
//               stale and will be implicitly recomputed at the next
//               call to get_bounds(), or false if it is fresh and
//               need not be recomputed.
////////////////////////////////////////////////////////////////////
bool PandaNode::
is_bounds_stale() const {
  CDReader cdata(_cycler);
  return (cdata->_last_bounds_update != cdata->_next_update);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_final
//       Access: Published
//  Description: Sets the "final" flag on this PandaNode.  If
//               this is true, than no bounding volume need be tested
//               below it; a positive intersection with this node's
//               bounding volume is deemed to be a positive
//               intersection with all geometry inside.
//
//               This is useful to quickly force a larger bounding
//               volume around a node when the GeomNodes themselves
//               are inaccurate for some reason, without forcing a
//               recompute of every nested bounding volume.  It's also
//               helpful when the bounding volume is tricked by some
//               special properties, like billboards, that may move
//               geometry out of its bounding volume otherwise.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
set_final(bool flag) {
  CDWriter cdata(_cycler);
  cdata->_final_bounds = flag;
  mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::is_final
//       Access: Published
//  Description: Returns the current state of the "final" flag.
//               Initially, this flag is off (false), but it may be
//               changed by an explicit call to set_final().  See
//               set_final().
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
is_final(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_final_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_fancy_bits
//       Access: Published
//  Description: Returns the union of all of the enum FancyBits values
//               corresponding to the various "fancy" attributes that
//               are set on the node.  If this returns 0, the node has
//               nothing interesting about it.  This is intended to
//               speed traversal by quickly skipping past nodes that
//               don't particularly affect the render state.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
get_fancy_bits(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_fancy_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_user_bounds
//       Access: Protected
//  Description: Returns the node's user bounding volume.  This is the
//               bounding volume specified with get_bounds().  This
//               will return NULL if the user bounding volume has
//               never been set.
////////////////////////////////////////////////////////////////////
INLINE CPT(BoundingVolume) PandaNode::
get_user_bounds(int pipeline_stage, Thread *current_thread) const {
  CDStageReader cdata(_cycler, pipeline_stage, current_thread);
  return cdata->_user_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::mark_bounds_stale
//       Access: Protected
//  Description: Indicates that the bounding volume, or something that
//               influences the bounding volume (or any of the other
//               things stored in CData, like net_collide_mask),
//               may have changed for this node, and that it must be
//               recomputed.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
mark_bounds_stale(int pipeline_stage, Thread *current_thread) const {
  // We check whether it is already marked stale.  If so, we don't have
  // to make the call to force_bounds_stale().
  bool is_stale_bounds;
  {
    CDStageReader cdata(_cycler, pipeline_stage, current_thread);
    is_stale_bounds = (cdata->_last_update != cdata->_next_update);
  }
  // It's important that we don't hold the lock during the call to
  // force_bounds_stale().
  if (!is_stale_bounds) {
    ((PandaNode *)this)->force_bounds_stale(pipeline_stage, current_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::mark_internal_bounds_stale
//       Access: Protected
//  Description: Should be called by a derived class to mark the
//               internal bounding volume stale, so that
//               recompute_internal_bounds() will be called when the
//               bounding volume is next requested.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
mark_internal_bounds_stale(int pipeline_stage, Thread *current_thread) {
  {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    ++cdata->_internal_bounds_mark;
  }
  mark_bounds_stale(pipeline_stage, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_children
//       Access: Public
//  Description: Returns an object that can be used to walk through
//               the list of children of the node.  When you intend to
//               visit multiple children, using this is slightly
//               faster than calling get_child() directly on the
//               PandaNode, since this object avoids reopening the
//               PipelineCycler each time.
//
//               This object also protects you from self-modifying
//               loops (e.g. adding or removing children during
//               traversal), since a virtual copy of the children is
//               made ahead of time.  The virtual copy is fast--it is
//               a form of copy-on-write, so the list is not actually
//               copied unless it is modified during the traversal.
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Children PandaNode::
get_children(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return Children(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_stashed
//       Access: Public
//  Description: Returns an object that can be used to walk through
//               the list of children of the node.  When you intend to
//               visit multiple children, using this is slightly
//               faster than calling get_stashed() directly on the
//               PandaNode, since this object avoids reopening the
//               PipelineCycler each time.
//
//               This object also protects you from self-modifying
//               loops (e.g. adding or removing children during
//               traversal), since a virtual copy of the children is
//               made ahead of time.  The virtual copy is fast--it is
//               a form of copy-on-write, so the list is not actually
//               copied unless it is modified during the traversal.
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Stashed PandaNode::
get_stashed(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return Stashed(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_parents
//       Access: Public
//  Description: Returns an object that can be used to walk through
//               the list of parents of the node, similar to
//               get_children() and get_stashed().
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Parents PandaNode::
get_parents(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return Parents(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::do_find_parent
//       Access: Private
//  Description: The private implementation of find_parent().
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
do_find_parent(PandaNode *node, const CData *cdata) const {
  CPT(Up) up = cdata->get_up();
  Up::const_iterator ui = up->find(UpConnection(node));
  if (ui == up->end()) {
    return -1;
  }
  return ui - up->begin();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::verify_child_no_cycles
//       Access: Private
//  Description: Ensures that attaching the indicated child node to
//               this node would not introduce a cycle in the graph.
//               Returns true if the attachment is valid, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
verify_child_no_cycles(PandaNode *child_node) {
#ifndef NDEBUG
  if (detect_graph_cycles) {
    if (!find_node_above(child_node)) {
      return true;
    }
    report_cycle(child_node);
    return false;
  }
#endif  // NDEBUG
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::do_set_dirty_prev_transform
//       Access: Private
//  Description: Sets the dirty_prev_transform flag, and adds the node
//               to the _dirty_prev_transforms chain.  Assumes
//               _dirty_prev_transforms._lock is already held.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
do_set_dirty_prev_transform() {
  nassertv(_dirty_prev_transforms._lock.debug_is_locked());
  if (!_dirty_prev_transform) {
    LinkedListNode::insert_before(&_dirty_prev_transforms);
    _dirty_prev_transform = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::do_clear_dirty_prev_transform
//       Access: Private
//  Description: Clears the dirty_prev_transform flag, and removes the node
//               from the _dirty_prev_transforms chain.  Assumes
//               _dirty_prev_transforms._lock is already held.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
do_clear_dirty_prev_transform() {
  nassertv(_dirty_prev_transforms._lock.debug_is_locked());
  if (_dirty_prev_transform) {
    LinkedListNode::remove_from_list();
    _dirty_prev_transform = false;
  }
}

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
//     Function: PandaNode::BoundsData::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::BoundsData::
BoundsData() :
  _internal_bounds(NULL),
  _internal_vertices(0)
{
  ++_internal_bounds_mark;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::BoundsData::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::BoundsData::
BoundsData(const PandaNode::BoundsData &copy) :
  _internal_bounds(copy._internal_bounds),
  _internal_vertices(copy._internal_vertices),
  _internal_bounds_mark(copy._internal_bounds_mark),
  _internal_bounds_computed(copy._internal_bounds_computed)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::BoundsData::copy_bounds
//       Access: Protected
//  Description: Copies just the BoundsData part of the structure.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::BoundsData::
copy_bounds(const PandaNode::BoundsData &copy) {
  _internal_bounds = copy._internal_bounds;
  _internal_vertices = copy._internal_vertices;
  _internal_bounds_mark = copy._internal_bounds_mark;
  _internal_bounds_computed = copy._internal_bounds_computed;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::set_fancy_bit
//       Access: Public
//  Description: Internal function to set (if value is true) or clear
//               (if value is false) the indicated bit(s) in the
//               _fancy_bits member.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::CData::
set_fancy_bit(int bits, bool value) {
  if (value) {
    _fancy_bits |= bits;
  } else {
    _fancy_bits &= ~bits;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::get_down
//       Access: Public
//  Description: Returns a read-only pointer to the _down list.
////////////////////////////////////////////////////////////////////
INLINE CPT(PandaNode::Down) PandaNode::CData::
get_down() const {
  return _down.get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::modify_down
//       Access: Public
//  Description: Returns a modifiable, unique pointer to the _down
//               list.
////////////////////////////////////////////////////////////////////
INLINE PT(PandaNode::Down) PandaNode::CData::
modify_down() {
  return _down.get_write_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::get_stashed
//       Access: Public
//  Description: Returns a read-only pointer to the _stashed list.
////////////////////////////////////////////////////////////////////
INLINE CPT(PandaNode::Down) PandaNode::CData::
get_stashed() const {
  return _stashed.get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::modify_stashed
//       Access: Public
//  Description: Returns a modifiable, unique pointer to the _stashed
//               list.
////////////////////////////////////////////////////////////////////
INLINE PT(PandaNode::Down) PandaNode::CData::
modify_stashed() {
  return _stashed.get_write_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::get_up
//       Access: Public
//  Description: Returns a read-only pointer to the _up list.
////////////////////////////////////////////////////////////////////
INLINE CPT(PandaNode::Up) PandaNode::CData::
get_up() const {
  return _up.get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::modify_up
//       Access: Public
//  Description: Returns a modifiable, unique pointer to the _up
//               list.
////////////////////////////////////////////////////////////////////
INLINE PT(PandaNode::Up) PandaNode::CData::
modify_up() {
  return _up.get_write_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Children::
Children() {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Children::
Children(const PandaNode::CData *cdata) :
  _down(cdata->get_down())
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Children::
Children(const PandaNode::Children &copy) :
  _down(copy._down)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::Children::
operator = (const PandaNode::Children &copy) {
  _down = copy._down;
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Children::
Children(PandaNode::Children &&from) NOEXCEPT :
  _down(move(from._down))
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::Move Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::Children::
operator = (PandaNode::Children &&from) NOEXCEPT {
  _down = move(from._down);
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::get_num_children
//       Access: Public
//  Description: Returns the number of children of the node.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::Children::
get_num_children() const {
  nassertr(_down != (Down *)NULL, 0);
  return _down->size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::get_child
//       Access: Public
//  Description: Returns the nth child of the node.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::Children::
get_child(int n) const {
  nassertr(_down != (Down *)NULL, NULL);
  nassertr(n >= 0 && n < (int)_down->size(), NULL);
  return (*_down)[n].get_child();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Children::get_child_sort
//       Access: Public
//  Description: Returns the sort index of the nth child node of this
//               node (that is, the number that was passed to
//               add_child()).  See get_num_children().
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::Children::
get_child_sort(int n) const {
  nassertr(_down != (Down *)NULL, -1);
  nassertr(n >= 0 && n < (int)_down->size(), -1);
  return (*_down)[n].get_sort();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Stashed::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Stashed::
Stashed() {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Stashed::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Stashed::
Stashed(const PandaNode::CData *cdata) :
  _stashed(cdata->get_stashed())
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Stashed::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Stashed::
Stashed(const PandaNode::Stashed &copy) :
  _stashed(copy._stashed)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Stashed::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::Stashed::
operator = (const PandaNode::Stashed &copy) {
  _stashed = copy._stashed;
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Stashed::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Stashed::
Stashed(PandaNode::Stashed &&from) NOEXCEPT :
  _stashed(move(from._stashed))
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Stashed::Move Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::Stashed::
operator = (PandaNode::Stashed &&from) NOEXCEPT {
  _stashed = move(from._stashed);
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Stashed::get_num_stashed
//       Access: Public
//  Description: Returns the number of stashed children of the node.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::Stashed::
get_num_stashed() const {
  nassertr(_stashed != (Down *)NULL, 0);
  return _stashed->size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Stashed::get_stashed
//       Access: Public
//  Description: Returns the nth stashed child of the node.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::Stashed::
get_stashed(int n) const {
  nassertr(_stashed != (Down *)NULL, NULL);
  nassertr(n >= 0 && n < (int)_stashed->size(), NULL);
  return (*_stashed)[n].get_child();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Stashed::get_stashed_sort
//       Access: Public
//  Description: Returns the sort index of the nth child node of this
//               node (that is, the number that was passed to
//               add_child()).  See get_num_stashed().
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::Stashed::
get_stashed_sort(int n) const {
  nassertr(_stashed != (Down *)NULL, -1);
  nassertr(n >= 0 && n < (int)_stashed->size(), -1);
  return (*_stashed)[n].get_sort();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Parents::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Parents::
Parents() {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Parents::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Parents::
Parents(const PandaNode::CData *cdata) :
  _up(cdata->get_up())
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Parents::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Parents::
Parents(const PandaNode::Parents &copy) :
  _up(copy._up)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Parents::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::Parents::
operator = (const PandaNode::Parents &copy) {
  _up = copy._up;
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Parents::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Parents::
Parents(PandaNode::Parents &&from) NOEXCEPT :
  _up(move(from._up))
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Parents::Move Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::Parents::
operator = (PandaNode::Parents &&from) NOEXCEPT {
  _up = move(from._up);
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Parents::get_num_parents
//       Access: Public
//  Description: Returns the number of parents of the node.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::Parents::
get_num_parents() const {
  nassertr(_up != (Up *)NULL, 0);
  return _up->size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Parents::get_parent
//       Access: Public
//  Description: Returns the nth parent of the node.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNode::Parents::
get_parent(int n) const {
  nassertr(_up != (Up *)NULL, NULL);
  nassertr(n >= 0 && n < (int)_up->size(), NULL);
  return (*_up)[n].get_parent();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNodePipelineReader::
PandaNodePipelineReader(const PandaNode *node, Thread *current_thread) :
  _node(node),
  _current_thread(current_thread),
  _cdata(node->_cycler.read_unlocked(current_thread))
{
#ifdef _DEBUG
  nassertv(_node->test_ref_count_nonzero());
#endif // _DEBUG

#ifdef DO_PIPELINING
  // We node_ref the CData pointer, so that if anyone makes changes to
  // the PandaNode while we hold this pointer, it will force a
  // copy--so that this object will remain unchanged (if out-of-date).
  _cdata->node_ref();
#endif  // DO_PIPELINING
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNodePipelineReader::
PandaNodePipelineReader(const PandaNodePipelineReader &copy) :
  _node(copy._node),
  _current_thread(copy._current_thread),
  _cdata(copy._cdata)
{
#ifdef DO_PIPELINING
  _cdata->node_ref();
#endif  // DO_PIPELINING

  /*
  if (_cdata != (PandaNode::CData *)NULL) {
    _node->_cycler.increment_read(_cdata);
  }
  */
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PandaNodePipelineReader::
operator = (const PandaNodePipelineReader &copy) {
  nassertv(_current_thread == copy._current_thread);

  /*
  if (_cdata != (PandaNode::CData *)NULL) {
    _node->_cycler.release_read(_cdata);
  }
  */

#ifdef DO_PIPELINING
  node_unref_delete((CycleData *)_cdata);
#endif  // DO_PIPELINING

  _node = copy._node;
  _cdata = copy._cdata;

#ifdef DO_PIPELINING
  _cdata->node_ref();
#endif  // DO_PIPELINING

  /*
  if (_cdata != (PandaNode::CData *)NULL) {
    _node->_cycler.increment_read(_cdata);
  }
  */
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNodePipelineReader::
~PandaNodePipelineReader() {
  /*
  if (_cdata != (PandaNode::CData *)NULL) {
    _node->_cycler.release_read(_cdata);
  }
  */

#ifdef DO_PIPELINING
  node_unref_delete((CycleData *)_cdata);
#endif  // DO_PIPELINING

#ifdef _DEBUG
  _node = NULL;
  _cdata = NULL;
#endif  // _DEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_node
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const PandaNode *PandaNodePipelineReader::
get_node() const {
  return _node;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_current_thread
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Thread *PandaNodePipelineReader::
get_current_thread() const {
  return _current_thread;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::release
//       Access: Public
//  Description: Releases the lock on this object.  No future calls
//               will be valid on this object.
////////////////////////////////////////////////////////////////////
INLINE void PandaNodePipelineReader::
release() {
  /*
  if (_cdata != (PandaNode::CData *)NULL) {
    _node->_cycler.release_read(_cdata);
    _cdata = NULL;
  }
  */
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::compose_draw_mask
//       Access: Public
//  Description: Computes the result of applying this node's draw
//               masks to a running draw mask, as during a traversal.
////////////////////////////////////////////////////////////////////
INLINE void PandaNodePipelineReader::
compose_draw_mask(DrawMask &running_draw_mask) const {
  nassertv(_cdata != (PandaNode::CData *)NULL);
  running_draw_mask = (running_draw_mask & ~_cdata->_draw_control_mask) |
    (_cdata->_draw_show_mask & _cdata->_draw_control_mask);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::compare_draw_mask
//       Access: Public
//  Description: Compares the running draw mask computed during a
//               traversal with this node's net draw masks.  Returns
//               true if the node should be traversed into, or false
//               if there is nothing at this level or below that will
//               be visible to the indicated camera_mask.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNodePipelineReader::
compare_draw_mask(DrawMask running_draw_mask, DrawMask camera_mask) const {
  nassertr(_cdata != (PandaNode::CData *)NULL, false);
  nassertr(_cdata->_last_update == _cdata->_next_update, false);

  // As a special case, if net_draw_show_mask is all 0, it means
  // either that all nodes under this node are hidden to all cameras,
  // or that none of them are renderable nodes (or some combination).
  // In either case, we might as well short-circuit.
  if (_cdata->_net_draw_show_mask.is_zero()) {
    return false;
  }

  DrawMask net_draw_control_mask, net_draw_show_mask;
  net_draw_control_mask = _cdata->_net_draw_control_mask;
  net_draw_show_mask = _cdata->_net_draw_show_mask;

  // Now the bits that are not in net_draw_control_mask--that is,
  // those bits that are not changed by any of the nodes at this level
  // and below--are taken from running_draw_mask, which is inherited
  // from above.  On the other hand, the bits that *are* in
  // net_draw_control_mask--those bits that are changed by any of the
  // nodes at this level and below--are taken from net_draw_show_mask,
  // which is propagated upwards from below.

  // This way, we will traverse into this node if it has any children
  // which want to be visited by the traversal, but we will avoid
  // traversing into it if all of its children are hidden to this
  // camera.
  DrawMask compare_mask = (running_draw_mask & ~net_draw_control_mask) | (net_draw_show_mask & net_draw_control_mask);

  return !((compare_mask & PandaNode::_overall_bit).is_zero()) && !((compare_mask & camera_mask).is_zero());
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_num_parents
//       Access: Public
//  Description: Returns the number of parent nodes this node has.  If
//               this number is greater than 1, the node has been
//               multiply instanced.  The order of the parent nodes is
//               not meaningful and is not related to the order in
//               which the node was instanced to them.
////////////////////////////////////////////////////////////////////
INLINE int PandaNodePipelineReader::
get_num_parents() const {
  return _cdata->get_up()->size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_parent
//       Access: Public
//  Description: Returns the nth parent node of this node.  See
//               get_num_parents().  Also see get_parents(), if your
//               intention is to iterate through the complete list of
//               parents; get_parents() is preferable in this case.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNodePipelineReader::
get_parent(int n) const {
  CPT(PandaNode::Up) up = _cdata->get_up();
  nassertr(n >= 0 && n < (int)up->size(), NULL);
  return (*up)[n].get_parent();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::find_parent
//       Access: Public
//  Description: Returns the index of the indicated parent node, if it
//               is a parent, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNodePipelineReader::
find_parent(PandaNode *node) const {
  return _node->do_find_parent(node, _cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_num_children
//       Access: Public
//  Description: Returns the number of child nodes this node has.  The
//               order of the child nodes *is* meaningful and is based
//               on the sort number that was passed to add_child(),
//               and also on the order in which the nodes were added.
////////////////////////////////////////////////////////////////////
INLINE int PandaNodePipelineReader::
get_num_children() const {
  return _cdata->get_down()->size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_child
//       Access: Public
//  Description: Returns the nth child node of this node.  See
//               get_num_children().  Also see get_children(), if your
//               intention is to iterate through the complete list of
//               children; get_children() is preferable in this case.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNodePipelineReader::
get_child(int n) const {
  CPT(PandaNode::Down) down = _cdata->get_down();
  nassertr(n >= 0 && n < (int)down->size(), NULL);
  return (*down)[n].get_child();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_child_sort
//       Access: Public
//  Description: Returns the sort index of the nth child node of this
//               node (that is, the number that was passed to
//               add_child()).  See get_num_children().
////////////////////////////////////////////////////////////////////
INLINE int PandaNodePipelineReader::
get_child_sort(int n) const {
  CPT(PandaNode::Down) down = _cdata->get_down();
  nassertr(n >= 0 && n < (int)down->size(), -1);
  return (*down)[n].get_sort();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::find_child
//       Access: Public
//  Description: Returns the index of the indicated child node, if it
//               is a child, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNodePipelineReader::
find_child(PandaNode *node) const {
  return _node->do_find_child(node, _cdata->get_down());
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_num_stashed
//       Access: Public
//  Description: Returns the number of stashed nodes this node has.
//               These are former children of the node that have been
//               moved to the special stashed list via stash_child().
////////////////////////////////////////////////////////////////////
INLINE int PandaNodePipelineReader::
get_num_stashed() const {
  return _cdata->get_stashed()->size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_stashed
//       Access: Public
//  Description: Returns the nth stashed child of this node.  See
//               get_num_stashed().  Also see get_stashed(), if your
//               intention is to iterate through the complete list of
//               stashed children; get_stashed() is preferable in this
//               case.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PandaNodePipelineReader::
get_stashed(int n) const {
  CPT(PandaNode::Down) stashed = _cdata->get_stashed();
  nassertr(n >= 0 && n < (int)stashed->size(), NULL);
  return (*stashed)[n].get_child();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_stashed_sort
//       Access: Public
//  Description: Returns the sort index of the nth stashed node of this
//               node (that is, the number that was passed to
//               add_child()).  See get_num_stashed().
////////////////////////////////////////////////////////////////////
INLINE int PandaNodePipelineReader::
get_stashed_sort(int n) const {
  CPT(PandaNode::Down) stashed = _cdata->get_stashed();
  nassertr(n >= 0 && n < (int)stashed->size(), -1);
  return (*stashed)[n].get_sort();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::find_stashed
//       Access: Public
//  Description: Returns the index of the indicated stashed node, if
//               it is a stashed child, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNodePipelineReader::
find_stashed(PandaNode *node) const {
  return _node->do_find_child(node, _cdata->get_stashed());
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_state
//       Access: Public
//  Description: Returns the complete RenderState that will be applied
//               to all nodes at this level and below, as set on this
//               node.  This returns only the RenderState set on this
//               particular node, and has nothing to do with state
//               that might be inherited from above.
////////////////////////////////////////////////////////////////////
INLINE const RenderState *PandaNodePipelineReader::
get_state() const {
  return _cdata->_state;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_effects
//       Access: Public
//  Description: Returns the complete RenderEffects that will be
//               applied to this node.
////////////////////////////////////////////////////////////////////
INLINE const RenderEffects *PandaNodePipelineReader::
get_effects() const {
  return _cdata->_effects;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_transform
//       Access: Public
//  Description: Returns the transform that has been set on this
//               particular node.  This is not the net transform from
//               the root, but simply the transform on this particular
//               node.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *PandaNodePipelineReader::
get_transform() const {
  return _cdata->_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_prev_transform
//       Access: Public
//  Description: Returns the transform that has been set as this
//               node's "previous" position.  See
//               set_prev_transform().
////////////////////////////////////////////////////////////////////
const TransformState *PandaNodePipelineReader::
get_prev_transform() const {
  return _cdata->_prev_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_tag
//       Access: Public
//  Description: Retrieves the user-defined value that was previously
//               set on this node for the particular key, if any.  If
//               no value has been previously set, returns the empty
//               string.
////////////////////////////////////////////////////////////////////
INLINE string PandaNodePipelineReader::
get_tag(const string &key) const {
  PandaNode::TagData::const_iterator ti;
  ti = _cdata->_tag_data.find(key);
  if (ti != _cdata->_tag_data.end()) {
    return (*ti).second;
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::has_tag
//       Access: Public
//  Description: Returns true if a value has been defined on this node
//               for the particular key (even if that value is the
//               empty string), or false if no value has been set.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNodePipelineReader::
has_tag(const string &key) const {
  PandaNode::TagData::const_iterator ti;
  ti = _cdata->_tag_data.find(key);
  return (ti != _cdata->_tag_data.end());
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_net_collide_mask
//       Access: Public
//  Description: Returns the union of all into_collide_mask() values
//               set at CollisionNodes at this level and below.
////////////////////////////////////////////////////////////////////
INLINE CollideMask PandaNodePipelineReader::
get_net_collide_mask() const {
  nassertr(_cdata->_last_update == _cdata->_next_update, _cdata->_net_collide_mask);
  return _cdata->_net_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_off_clip_planes
//       Access: Public
//  Description: Returns a ClipPlaneAttrib which represents the union
//               of all of the clip planes that have been turned *off*
//               at this level and below.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) PandaNodePipelineReader::
get_off_clip_planes() const {
  nassertr(_cdata->_last_update == _cdata->_next_update, _cdata->_off_clip_planes);
  return _cdata->_off_clip_planes;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_bounds
//       Access: Public
//  Description: Returns the external bounding volume of this node: a
//               bounding volume that contains the user bounding
//               volume, the internal bounding volume, and all of the
//               children's bounding volumes.
////////////////////////////////////////////////////////////////////
INLINE CPT(BoundingVolume) PandaNodePipelineReader::
get_bounds() const {
  nassertr(_cdata->_last_bounds_update == _cdata->_next_update, _cdata->_external_bounds);
  return _cdata->_external_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_nested_vertices
//       Access: Public
//  Description: Returns the total number of vertices that will be
//               rendered by this node and all of its descendents.
//
//               This is not necessarily an accurate count of vertices
//               that will actually be rendered, since this will
//               include all vertices of all LOD's, and it will also
//               include hidden nodes.  It may also omit or only
//               approximate certain kinds of dynamic geometry.
//               However, it will not include stashed nodes.
////////////////////////////////////////////////////////////////////
INLINE int PandaNodePipelineReader::
get_nested_vertices() const {
  nassertr(_cdata->_last_bounds_update == _cdata->_next_update, _cdata->_nested_vertices);
  return _cdata->_nested_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::is_final
//       Access: Public
//  Description: Returns the current state of the "final" flag.
//               Initially, this flag is off (false), but it may be
//               changed by an explicit call to set_final().  See
//               set_final().
////////////////////////////////////////////////////////////////////
INLINE bool PandaNodePipelineReader::
is_final() const {
  return _cdata->_final_bounds;
}


////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_fancy_bits
//       Access: Public
//  Description: Returns the union of all of the enum FancyBits values
//               corresponding to the various "fancy" attributes that
//               are set on the node.  If this returns 0, the node has
//               nothing interesting about it.  This is intended to
//               speed traversal by quickly skipping past nodes that
//               don't particularly affect the render state.
////////////////////////////////////////////////////////////////////
INLINE int PandaNodePipelineReader::
get_fancy_bits() const {
  return _cdata->_fancy_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_children
//       Access: Public
//  Description: Returns an object that can be used to walk through
//               the list of children of the node.  When you intend to
//               visit multiple children, using this is slightly
//               faster than calling get_child() directly on the
//               PandaNode, since this object avoids reopening the
//               PipelineCycler each time.
//
//               This object also protects you from self-modifying
//               loops (e.g. adding or removing children during
//               traversal), since a virtual copy of the children is
//               made ahead of time.  The virtual copy is fast--it is
//               a form of copy-on-write, so the list is not actually
//               copied unless it is modified during the traversal.
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Children PandaNodePipelineReader::
get_children() const {
  return PandaNode::Children(_cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_stashed
//       Access: Public
//  Description: Returns an object that can be used to walk through
//               the list of children of the node.  When you intend to
//               visit multiple children, using this is slightly
//               faster than calling get_stashed() directly on the
//               PandaNode, since this object avoids reopening the
//               PipelineCycler each time.
//
//               This object also protects you from self-modifying
//               loops (e.g. adding or removing children during
//               traversal), since a virtual copy of the children is
//               made ahead of time.  The virtual copy is fast--it is
//               a form of copy-on-write, so the list is not actually
//               copied unless it is modified during the traversal.
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Stashed PandaNodePipelineReader::
get_stashed() const {
  return PandaNode::Stashed(_cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::get_parents
//       Access: Public
//  Description: Returns an object that can be used to walk through
//               the list of parents of the node, similar to
//               get_children() and get_stashed().
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Parents PandaNodePipelineReader::
get_parents() const {
  return PandaNode::Parents(_cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::BamReaderAuxDataDown::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::BamReaderAuxDataDown::
BamReaderAuxDataDown() :
  _down_list(PandaNode::get_class_type())
{
}
