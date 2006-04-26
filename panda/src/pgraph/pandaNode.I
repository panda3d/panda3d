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
//     Function: PandaNode::compose_draw_mask
//       Access: Public
//  Description: Computes the result of applying this node's draw
//               masks to a running draw mask, as during a traversal.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
compose_draw_mask(DrawMask &running_draw_mask) const {
  CDHeavyReader cdata(_cycler_heavy);
  running_draw_mask = (running_draw_mask & ~cdata->_draw_control_mask) |
    (cdata->_draw_show_mask & cdata->_draw_control_mask);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::compare_draw_mask
//       Access: Public
//  Description: Compares the running draw mask computed during a
//               traversal with this node's net draw masks.  Returns
//               true if the node should be traversed into, or false
//               if there is nothing at this level or below that will
//               be visible to the indicated camera_mask.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
compare_draw_mask(DrawMask running_draw_mask, DrawMask camera_mask,
                  Thread *current_thread) const {
  DrawMask net_draw_control_mask, net_draw_show_mask;

  int pipeline_stage = current_thread->get_pipeline_stage();
  CDBoundsStageReader cdata(_cycler_bounds, pipeline_stage, current_thread);
  if (cdata->_last_update != cdata->_next_update) {
    // The cache is stale; it needs to be rebuilt.
    CDBoundsStageWriter cdataw = ((PandaNode *)this)->update_bounds(pipeline_stage, cdata); 
    net_draw_control_mask = cdataw->_net_draw_control_mask;
    net_draw_show_mask = cdataw->_net_draw_show_mask;
  } else {
    net_draw_control_mask = cdata->_net_draw_control_mask;
    net_draw_show_mask = cdata->_net_draw_show_mask;
  }

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

  return !((compare_mask & _overall_bit).is_zero()) && !((compare_mask & camera_mask).is_zero());
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
get_num_parents(Thread *current_thread) const {
  CDLinksReader cdata(_cycler_links, current_thread);
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
  CDLinksReader cdata(_cycler_links, current_thread);
  const Up &up = *cdata->get_up();
  nassertr(n >= 0 && n < (int)up.size(), NULL);
  return up[n].get_parent();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_parent
//       Access: Published
//  Description: Returns the index of the indicated parent node, if it
//               is a parent, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
find_parent(PandaNode *node, Thread *current_thread) const {
  CDLinksReader cdata(_cycler_links, current_thread);
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
  CDLinksReader cdata(_cycler_links, current_thread);
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
  CDLinksReader cdata(_cycler_links, current_thread);
  const Down &down = *cdata->get_down();
  nassertr(n >= 0 && n < (int)down.size(), NULL);
  return down[n].get_child();
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
  CDLinksReader cdata(_cycler_links, current_thread);
  const Down &down = *cdata->get_down();
  nassertr(n >= 0 && n < (int)down.size(), -1);
  return down[n].get_sort();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_child
//       Access: Published
//  Description: Returns the index of the indicated child node, if it
//               is a child, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
find_child(PandaNode *node, Thread *current_thread) const {
  CDLinksReader cdata(_cycler_links, current_thread);
  return do_find_child(node, cdata);
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
  CDLinksReader cdata(_cycler_links, current_thread);
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
  CDLinksReader cdata(_cycler_links, current_thread);
  const Down &stashed = *cdata->get_stashed();
  nassertr(n >= 0 && n < (int)stashed.size(), NULL);
  return stashed[n].get_child();
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
  CDLinksReader cdata(_cycler_links, current_thread);
  const Down &stashed = *cdata->get_stashed();
  nassertr(n >= 0 && n < (int)stashed.size(), -1);
  return stashed[n].get_sort();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_stashed
//       Access: Published
//  Description: Returns the index of the indicated stashed node, if
//               it is a stashed child, or -1 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
find_stashed(PandaNode *node, Thread *current_thread) const {
  CDLinksReader cdata(_cycler_links, current_thread);
  return do_find_stashed(node, cdata);
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
  CDLightReader cdata(_cycler_light);
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
  CDLightReader cdata(_cycler_light);
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
  CDHeavyReader cdata(_cycler_heavy);
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
  CDHeavyReader cdata(_cycler_heavy);
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
get_state(Thread *current_thread) const {
  CDLightReader cdata(_cycler_light, current_thread);
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
clear_state(Thread *current_thread) {
  set_state(RenderState::make_empty(), current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_effects
//       Access: Published
//  Description: Returns the complete RenderEffects that will be
//               applied to this node.
////////////////////////////////////////////////////////////////////
INLINE const RenderEffects *PandaNode::
get_effects() const {
  CDHeavyReader cdata(_cycler_heavy);
  return cdata->_effects;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_effects
//       Access: Published
//  Description: Resets this node to have no render effects.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
clear_effects() {
  set_effects(RenderEffects::make_empty());
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
get_transform(Thread *current_thread) const {
  CDLightReader cdata(_cycler_light, current_thread);
  return cdata->_transform;
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
const TransformState *PandaNode::
get_prev_transform(Thread *current_thread) const {
  CDLightReader cdata(_cycler_light, current_thread);
  return cdata->_prev_transform;
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
get_tag(const string &key) const {
  CDHeavyReader cdata(_cycler_heavy);
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
  CDHeavyReader cdata(_cycler_heavy);
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
  CDHeavyReader cdata(_cycler_heavy);
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
//     Function: PandaNode::is_overall_hidden
//       Access: Published, Static
//  Description: Returns true if the node has been hidden to all
//               cameras by clearing its overall bit.
////////////////////////////////////////////////////////////////////
INLINE bool PandaNode::
is_overall_hidden() const {
  CDHeavyReader cdata(_cycler_heavy);
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
  CDHeavyReader cdata(_cycler_heavy);
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
  CDHeavyReader cdata(_cycler_heavy);
  return cdata->_draw_show_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_into_collide_mask
//       Access: Published
//  Description: Returns the "into" collide mask for this node.
////////////////////////////////////////////////////////////////////
INLINE CollideMask PandaNode::
get_into_collide_mask() const {
  CDHeavyReader cdata(_cycler_heavy);
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
get_internal_bounds() const {
  Thread *current_thread = Thread::get_current_thread();
  return get_internal_bounds(current_thread->get_pipeline_stage(),
                             current_thread);
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
  CDHeavyWriter cdata(_cycler_heavy);
  cdata->_final_bounds = flag;
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
is_final() const {
  CDHeavyReader cdata(_cycler_heavy);
  return cdata->_final_bounds;
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
  CDHeavyStageReader cdata(_cycler_heavy, pipeline_stage, current_thread);
  return cdata->_user_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::mark_bounds_stale
//       Access: Protected
//  Description: Indicates that the bounding volume, or something that
//               influences the bounding volume (or any of the other
//               things stored in CDataBounds, like net_collide_mask),
//               may have changed for this node, and that it must be
//               recomputed.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
mark_bounds_stale(int pipeline_stage, Thread *current_thread) const {
  // It's important that we don't hold the lock during the call to
  // force_bounds_stale().
  bool is_stale_bounds;
  {
    CDBoundsStageReader cdata(_cycler_bounds, pipeline_stage, current_thread);
    is_stale_bounds = (cdata->_last_update != cdata->_next_update);
  }
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
    CDHeavyStageWriter cdata(_cycler_heavy, pipeline_stage, current_thread);
    cdata->_internal_bounds_stale = true;
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
  CDLinksReader cdata(_cycler_links, current_thread);
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
  CDLinksReader cdata(_cycler_links, current_thread);
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
  CDLinksReader cdata(_cycler_links, current_thread);
  return Parents(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::do_find_parent
//       Access: Private
//  Description: The private implementation of find_parent().
////////////////////////////////////////////////////////////////////
INLINE int PandaNode::
do_find_parent(PandaNode *node, const CDataLinks *cdata) const {
  const Up &up = *cdata->get_up();
  Up::const_iterator ui = up.find(UpConnection(node));
  if (ui == up.end()) {
    return -1;
  }
  return ui - up.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_dirty_prev_transform
//       Access: Private
//  Description: Sets the dirty_prev_transform flag, and adds the node
//               to the _dirty_prev_transforms chain.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
set_dirty_prev_transform() {
  if (!_dirty_prev_transform) {
    MutexHolder holder(_dirty_prev_transforms._lock);
    if (!_dirty_prev_transform) {
      LinkedListNode::insert_before(&_dirty_prev_transforms);
      _dirty_prev_transform = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_dirty_prev_transform
//       Access: Private
//  Description: Clears the dirty_prev_transform flag, and removes the node
//               from the _dirty_prev_transforms chain.
////////////////////////////////////////////////////////////////////
INLINE void PandaNode::
clear_dirty_prev_transform() {
  if (_dirty_prev_transform) {
    MutexHolder holder(_dirty_prev_transforms._lock);
    if (_dirty_prev_transform) {
      LinkedListNode::remove_from_list();
      _dirty_prev_transform = false;
    }
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
//     Function: PandaNode::CDataLight::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::CDataLight::
CDataLight() {
  _state = RenderState::make_empty();
  _transform = TransformState::make_identity();
  _prev_transform = TransformState::make_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataHeavy::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::CDataHeavy::
CDataHeavy() {
  _effects = RenderEffects::make_empty();
  _draw_control_mask = DrawMask::all_off();
  _draw_show_mask = DrawMask::all_on();
  _into_collide_mask = CollideMask::all_off();
  _user_bounds = NULL;
  _internal_bounds = NULL;
  _internal_bounds_stale = true;
  _final_bounds = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataBounds::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::CDataBounds::
CDataBounds() {
  _net_collide_mask = CollideMask::all_off();
  _net_draw_control_mask = DrawMask::all_off();
  _net_draw_show_mask = DrawMask::all_off();
  ++_next_update;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode::CDataLinks::
CDataLinks() : 
  _down(new PandaNode::Down),
  _stashed(new PandaNode::Down),
  _up(new PandaNode::Up)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::get_down
//       Access: Public
//  Description: Returns a read-only pointer to the _down list.
////////////////////////////////////////////////////////////////////
INLINE const PandaNode::Down *PandaNode::CDataLinks::
get_down() const {
  return _down;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::modify_down
//       Access: Public
//  Description: Returns a modifiable, unique pointer to the _down
//               list.
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Down *PandaNode::CDataLinks::
modify_down() {
  if (_down->get_ref_count() > 1) {
    _down = new Down(*_down);
  }
  return _down;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::get_stashed
//       Access: Public
//  Description: Returns a read-only pointer to the _stashed list.
////////////////////////////////////////////////////////////////////
INLINE const PandaNode::Down *PandaNode::CDataLinks::
get_stashed() const {
  return _stashed;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::modify_stashed
//       Access: Public
//  Description: Returns a modifiable, unique pointer to the _stashed
//               list.
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Down *PandaNode::CDataLinks::
modify_stashed() {
  if (_stashed->get_ref_count() > 1) {
    _stashed = new Down(*_stashed);
  }
  return _stashed;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::get_up
//       Access: Public
//  Description: Returns a read-only pointer to the _up list.
////////////////////////////////////////////////////////////////////
INLINE const PandaNode::Up *PandaNode::CDataLinks::
get_up() const {
  return _up;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::modify_up
//       Access: Public
//  Description: Returns a modifiable, unique pointer to the _up
//               list.
////////////////////////////////////////////////////////////////////
INLINE PandaNode::Up *PandaNode::CDataLinks::
modify_up() {
  if (_up->get_ref_count() > 1) {
    _up = new Up(*_up);
  }
  return _up;
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
Children(const PandaNode::CDataLinks *cdata) :
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
Stashed(const PandaNode::CDataLinks *cdata) :
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
Parents(const PandaNode::CDataLinks *cdata) :
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
