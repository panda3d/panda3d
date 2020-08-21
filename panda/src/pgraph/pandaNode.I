/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaNode.I
 * @author drose
 * @date 2002-02-20
 */

/**
 * Returns the number of parent nodes this node has.  If this number is
 * greater than 1, the node has been multiply instanced.  The order of the
 * parent nodes is not meaningful and is not related to the order in which the
 * node was instanced to them.
 */
INLINE int PandaNode::
get_num_parents(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->get_up()->size();
}

/**
 * Returns the nth parent node of this node.  See get_num_parents().  Also see
 * get_parents(), if your intention is to iterate through the complete list of
 * parents; get_parents() is preferable in this case.
 */
INLINE PandaNode *PandaNode::
get_parent(int n, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  CPT(Up) up = cdata->get_up();
  nassertr(n >= 0 && n < (int)up->size(), nullptr);
  return (*up)[n].get_parent();
}

/**
 * Returns the index of the indicated parent node, if it is a parent, or -1 if
 * it is not.
 */
INLINE int PandaNode::
find_parent(PandaNode *node, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return do_find_parent(node, cdata);
}

/**
 * Returns the number of child nodes this node has.  The order of the child
 * nodes *is* meaningful and is based on the sort number that was passed to
 * add_child(), and also on the order in which the nodes were added.
 */
INLINE int PandaNode::
get_num_children(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->get_down()->size();
}

/**
 * Returns the nth child node of this node.  See get_num_children().  Also see
 * get_children(), if your intention is to iterate through the complete list
 * of children; get_children() is preferable in this case.
 */
INLINE PandaNode *PandaNode::
get_child(int n, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  CPT(Down) down = cdata->get_down();
  nassertr(n >= 0 && n < (int)down->size(), nullptr);
  return (*down)[n].get_child();
}

/**
 * Returns the sort index of the nth child node of this node (that is, the
 * number that was passed to add_child()).  See get_num_children().
 */
INLINE int PandaNode::
get_child_sort(int n, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  CPT(Down) down = cdata->get_down();
  nassertr(n >= 0 && n < (int)down->size(), -1);
  return (*down)[n].get_sort();
}

/**
 * Returns the index of the indicated child node, if it is a child, or -1 if
 * it is not.
 */
INLINE int PandaNode::
find_child(PandaNode *node, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return do_find_child(node, cdata->get_down());
}

/**
 * Stashes the indicated child node.  This removes the child from the list of
 * active children and puts it on a special list of stashed children.  This
 * child node no longer contributes to the bounding volume of the PandaNode,
 * and is not visited in normal traversals.  It is invisible and uncollidable.
 * The child may later be restored by calling unstash_child().
 *
 * This function returns true if the child node was successfully stashed, or
 * false if it was not a child of the node in the first place (e.g.  it was
 * previously stashed).
 */
INLINE bool PandaNode::
stash_child(PandaNode *child_node, Thread *current_thread) {
  int child_index = find_child(child_node, current_thread);
  if (child_index < 0) {
    return false;
  }
  stash_child(child_index, current_thread);
  return true;
}

/**
 * Returns the indicated stashed node to normal child status.  This removes
 * the child from the list of stashed children and puts it on the normal list
 * of active children.  This child node once again contributes to the bounding
 * volume of the PandaNode, and will be visited in normal traversals.  It is
 * visible and collidable.
 *
 * This function returns true if the child node was successfully stashed, or
 * false if it was not a child of the node in the first place (e.g.  it was
 * previously stashed).
 */
INLINE bool PandaNode::
unstash_child(PandaNode *child_node, Thread *current_thread) {
  int stashed_index = find_stashed(child_node, current_thread);
  if (stashed_index < 0) {
    return false;
  }
  unstash_child(stashed_index, current_thread);
  return true;
}

/**
 * Returns the number of stashed nodes this node has.  These are former
 * children of the node that have been moved to the special stashed list via
 * stash_child().
 */
INLINE int PandaNode::
get_num_stashed(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->get_stashed()->size();
}

/**
 * Returns the nth stashed child of this node.  See get_num_stashed().  Also
 * see get_stashed(), if your intention is to iterate through the complete
 * list of stashed children; get_stashed() is preferable in this case.
 */
INLINE PandaNode *PandaNode::
get_stashed(int n, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  CPT(Down) stashed = cdata->get_stashed();
  nassertr(n >= 0 && n < (int)stashed->size(), nullptr);
  return (*stashed)[n].get_child();
}

/**
 * Returns the sort index of the nth stashed node of this node (that is, the
 * number that was passed to add_child()).  See get_num_stashed().
 */
INLINE int PandaNode::
get_stashed_sort(int n, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  CPT(Down) stashed = cdata->get_stashed();
  nassertr(n >= 0 && n < (int)stashed->size(), -1);
  return (*stashed)[n].get_sort();
}

/**
 * Returns the index of the indicated stashed node, if it is a stashed child,
 * or -1 if it is not.
 */
INLINE int PandaNode::
find_stashed(PandaNode *node, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return do_find_child(node, cdata->get_stashed());
}

/**
 * Returns the render attribute of the indicated type, if it is defined on the
 * node, or NULL if it is not.  This checks only what is set on this
 * particular node level, and has nothing to do with what render attributes
 * may be inherited from parent nodes.
 */
INLINE CPT(RenderAttrib) PandaNode::
get_attrib(TypeHandle type) const {
  CDReader cdata(_cycler);
  return cdata->_state->get_attrib(type);
}

/**
 * Returns the render attribute of the indicated type, if it is defined on the
 * node, or NULL if it is not.  This checks only what is set on this
 * particular node level, and has nothing to do with what render attributes
 * may be inherited from parent nodes.
 */
INLINE CPT(RenderAttrib) PandaNode::
get_attrib(int slot) const {
  CDReader cdata(_cycler);
  return cdata->_state->get_attrib(slot);
}

/**
 * Returns true if there is a render attribute of the indicated type defined
 * on this node, or false if there is not.
 */
INLINE bool PandaNode::
has_attrib(TypeHandle type) const {
  CDReader cdata(_cycler);
  return cdata->_state->has_attrib(type);
}

/**
 * Returns true if there is a render attribute of the indicated type defined
 * on this node, or false if there is not.
 */
INLINE bool PandaNode::
has_attrib(int slot) const {
  CDReader cdata(_cycler);
  return cdata->_state->has_attrib(slot);
}

/**
 * Removes the render attribute of the given type from this node.  This node,
 * and the subgraph below, will now inherit the indicated render attribute
 * from the nodes above this one.
 */
INLINE void PandaNode::
clear_attrib(TypeHandle type) {
  RenderAttribRegistry *reg = RenderAttribRegistry::quick_get_global_ptr();
  int slot = reg->get_slot(type);
  clear_attrib(slot);
}

/**
 * Returns the render effect of the indicated type, if it is defined on the
 * node, or NULL if it is not.
 */
INLINE CPT(RenderEffect) PandaNode::
get_effect(TypeHandle type) const {
  CDReader cdata(_cycler);
  int index = cdata->_effects->find_effect(type);
  if (index >= 0) {
    return cdata->_effects->get_effect(index);
  }
  return nullptr;
}

/**
 * Returns true if there is a render effect of the indicated type defined on
 * this node, or false if there is not.
 */
INLINE bool PandaNode::
has_effect(TypeHandle type) const {
  CDReader cdata(_cycler);
  int index = cdata->_effects->find_effect(type);
  return (index >= 0);
}

/**
 * Returns the complete RenderState that will be applied to all nodes at this
 * level and below, as set on this node.  This returns only the RenderState
 * set on this particular node, and has nothing to do with state that might be
 * inherited from above.
 */
INLINE CPT(RenderState) PandaNode::
get_state(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_state.p();
}

/**
 * Resets this node to leave the render state alone.  Nodes at this level and
 * below will once again inherit their render state unchanged from the nodes
 * above this level.
 */
INLINE void PandaNode::
clear_state(Thread *current_thread) {
  set_state(RenderState::make_empty(), current_thread);
}

/**
 * Returns the complete RenderEffects that will be applied to this node.
 */
INLINE CPT(RenderEffects) PandaNode::
get_effects(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_effects;
}

/**
 * Resets this node to have no render effects.
 */
INLINE void PandaNode::
clear_effects(Thread *current_thread) {
  set_effects(RenderEffects::make_empty(), current_thread);
}

/**
 * Returns the transform that has been set on this particular node.  This is
 * not the net transform from the root, but simply the transform on this
 * particular node.
 */
INLINE CPT(TransformState) PandaNode::
get_transform(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_transform.p();
}

/**
 * Resets the transform on this node to the identity transform.
 */
INLINE void PandaNode::
clear_transform(Thread *current_thread) {
  set_transform(TransformState::make_identity(), current_thread);
}

/**
 * Returns the transform that has been set as this node's "previous" position.
 * See set_prev_transform().
 */
INLINE CPT(TransformState) PandaNode::
get_prev_transform(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_prev_transform.p();
}

/**
 * Returns true if this node has the _dirty_prev_transform flag set, which
 * indicates its _prev_transform is different from its _transform value (in
 * pipeline stage 0).  In this case, the node will be visited by
 * reset_prev_transform().
 */
INLINE bool PandaNode::
has_dirty_prev_transform() const {
  return _dirty_prev_transform;
}

/**
 * Retrieves the user-defined value that was previously set on this node for
 * the particular key, if any.  If no value has been previously set, returns
 * the empty string.
 */
INLINE std::string PandaNode::
get_tag(const std::string &key, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  int index = cdata->_tag_data.find(key);
  if (index >= 0) {
    return cdata->_tag_data.get_data((size_t)index);
  } else {
    return std::string();
  }
}

/**
 * Returns true if a value has been defined on this node for the particular
 * key (even if that value is the empty string), or false if no value has been
 * set.
 */
INLINE bool PandaNode::
has_tag(const std::string &key, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_tag_data.find(key) >= 0;
}

/**
 * Returns the number of tags applied to this node.
 */
INLINE size_t PandaNode::
get_num_tags() const {
  CDReader cdata(_cycler);
  return cdata->_tag_data.size();
}

/**
 * Returns the key of the nth tag applied to this node.
 */
INLINE std::string PandaNode::
get_tag_key(size_t i) const {
  CDReader cdata(_cycler);
  return cdata->_tag_data.get_key(i);
}

/**
 * Returns true if the node has any tags (or any Python tags) at all, false if
 * it has none.
 */
INLINE bool PandaNode::
has_tags() const {
  CDReader cdata(_cycler);
  if (!cdata->_tag_data.is_empty()) {
    return true;
  }
#ifdef HAVE_PYTHON
  // This is a bit awkward, since it doesn't catch cases where the Python tags
  // are cleared.  Maybe we should make this behavior deprecated, so that
  // has_tags will not consider the Python tags.
  if (!_python_tag_data.is_null()) {
    return true;
  }
#endif  // HAVE_PYTHON
  return false;
}

/**
 * Lists all the nodes at and below the current path hierarchically.
 */
INLINE void PandaNode::
ls(std::ostream &out, int indent_level) const {
  r_list_descendants(out, indent_level);
}

/**
 * Returns the special bit that, when specifically cleared in the node's
 * DrawMask, indicates that the node is hidden to all cameras, regardless of
 * the remaining DrawMask bits.
 */
INLINE DrawMask PandaNode::
get_overall_bit() {
  return _overall_bit;
}

/**
 * Returns a DrawMask that is appropriate for rendering to all cameras.
 */
INLINE DrawMask PandaNode::
get_all_camera_mask() {
  return ~_overall_bit;
}

/**
 * Returns true if the node has been hidden to all cameras by clearing its
 * overall bit.
 */
INLINE bool PandaNode::
is_overall_hidden() const {
  CDReader cdata(_cycler);
  return ((cdata->_draw_show_mask | ~cdata->_draw_control_mask) & _overall_bit).is_zero();
}

/**
 * Sets or clears the hidden flag.  When the hidden flag is true, the node and
 * all of its children are invisible to all cameras, regardless of the setting
 * of any draw masks.  Setting the hidden flag to false restores the previous
 * visibility as established by the draw masks.
 *
 * This actually works by twiddling the reserved _overall_bit in the node's
 * draw mask, which has special meaning.
 */
INLINE void PandaNode::
set_overall_hidden(bool hidden) {
  if (hidden) {
    adjust_draw_mask(DrawMask::all_off(), _overall_bit, DrawMask::all_off());
  } else {
    adjust_draw_mask(DrawMask::all_off(), DrawMask::all_off(), _overall_bit);
  }
}

/**
 * Returns the set of bits in draw_show_mask that are considered meaningful.
 * See adjust_draw_mask().
 */
INLINE DrawMask PandaNode::
get_draw_control_mask() const {
  CDReader cdata(_cycler);
  return cdata->_draw_control_mask;
}

/**
 * Returns the hide/show bits of this particular node.  See
 * adjust_draw_mask().
 */
INLINE DrawMask PandaNode::
get_draw_show_mask() const {
  CDReader cdata(_cycler);
  return cdata->_draw_show_mask;
}

/**
 * Returns the "into" collide mask for this node.
 */
INLINE CollideMask PandaNode::
get_into_collide_mask() const {
  CDReader cdata(_cycler);
  return cdata->_into_collide_mask;
}

/**
 * Reverses the effect of a previous call to set_bounds(), and allows the
 * node's bounding volume to be automatically computed once more based on the
 * contents of the node.
 */
INLINE void PandaNode::
clear_bounds() {
  set_bounds(nullptr);
}

/**
 * Returns the node's internal bounding volume.  This is the bounding volume
 * around the node alone, without including children.  If the user has called
 * set_bounds(), it will be the specified bounding volume.
 */
INLINE CPT(BoundingVolume) PandaNode::
get_internal_bounds(Thread *current_thread) const {
  return get_internal_bounds(current_thread->get_pipeline_stage(),
                             current_thread);
}

/**
 * Returns the total number of vertices that will be rendered by this
 * particular node alone, not accounting for its children.
 *
 * This may not include all vertices for certain dynamic effects.
 */
INLINE int PandaNode::
get_internal_vertices(Thread *current_thread) const {
  return get_internal_vertices(current_thread->get_pipeline_stage(),
                               current_thread);
}

/**
 * Returns true if the bounding volume of this node is stale and will be
 * implicitly recomputed at the next call to get_bounds(), or false if it is
 * fresh and need not be recomputed.
 */
bool PandaNode::
is_bounds_stale() const {
  CDReader cdata(_cycler);
  return (cdata->_last_bounds_update != cdata->_next_update);
}

/**
 * Sets the "final" flag on this PandaNode.  If this is true, than no bounding
 * volume need be tested below it; a positive intersection with this node's
 * bounding volume is deemed to be a positive intersection with all geometry
 * inside.
 *
 * This is useful to quickly force a larger bounding volume around a node when
 * the GeomNodes themselves are inaccurate for some reason, without forcing a
 * recompute of every nested bounding volume.  It's also helpful when the
 * bounding volume is tricked by some special properties, like billboards,
 * that may move geometry out of its bounding volume otherwise.
 */
INLINE void PandaNode::
set_final(bool flag) {
  CDWriter cdata(_cycler);
  cdata->_final_bounds = flag;
  mark_bam_modified();
}

/**
 * Returns the current state of the "final" flag.  Initially, this flag is off
 * (false), but it may be changed by an explicit call to set_final().  See
 * set_final().
 */
INLINE bool PandaNode::
is_final(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_final_bounds;
}

/**
 * Returns the union of all of the enum FancyBits values corresponding to the
 * various "fancy" attributes that are set on the node.  If this returns 0,
 * the node has nothing interesting about it.  This is intended to speed
 * traversal by quickly skipping past nodes that don't particularly affect the
 * render state.
 */
INLINE int PandaNode::
get_fancy_bits(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_fancy_bits;
}

/**
 * Returns the node's user bounding volume.  This is the bounding volume
 * specified with get_bounds().  This will return NULL if the user bounding
 * volume has never been set.
 */
INLINE CPT(BoundingVolume) PandaNode::
get_user_bounds(int pipeline_stage, Thread *current_thread) const {
  CDStageReader cdata(_cycler, pipeline_stage, current_thread);
  return cdata->_user_bounds;
}

/**
 * Indicates that the bounding volume, or something that influences the
 * bounding volume (or any of the other things stored in CData, like
 * net_collide_mask), may have changed for this node, and that it must be
 * recomputed.
 */
INLINE void PandaNode::
mark_bounds_stale(int pipeline_stage, Thread *current_thread) const {
  // We check whether it is already marked stale.  If so, we don't have to
  // make the call to force_bounds_stale().
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

/**
 * Should be called by a derived class to mark the internal bounding volume
 * stale, so that recompute_internal_bounds() will be called when the bounding
 * volume is next requested.
 */
INLINE void PandaNode::
mark_internal_bounds_stale(int pipeline_stage, Thread *current_thread) {
  {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    ++cdata->_internal_bounds_mark;
  }
  mark_bounds_stale(pipeline_stage, current_thread);
}

/**
 * Returns an object that can be used to walk through the list of children of
 * the node.  When you intend to visit multiple children, using this is
 * slightly faster than calling get_child() directly on the PandaNode, since
 * this object avoids reopening the PipelineCycler each time.
 *
 * This object also protects you from self-modifying loops (e.g.  adding or
 * removing children during traversal), since a virtual copy of the children
 * is made ahead of time.  The virtual copy is fast--it is a form of copy-on-
 * write, so the list is not actually copied unless it is modified during the
 * traversal.
 */
INLINE PandaNode::Children PandaNode::
get_children(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return Children(cdata);
}

/**
 * Returns an object that can be used to walk through the list of children of
 * the node.  When you intend to visit multiple children, using this is
 * slightly faster than calling get_stashed() directly on the PandaNode, since
 * this object avoids reopening the PipelineCycler each time.
 *
 * This object also protects you from self-modifying loops (e.g.  adding or
 * removing children during traversal), since a virtual copy of the children
 * is made ahead of time.  The virtual copy is fast--it is a form of copy-on-
 * write, so the list is not actually copied unless it is modified during the
 * traversal.
 */
INLINE PandaNode::Stashed PandaNode::
get_stashed(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return Stashed(cdata);
}

/**
 * Returns an object that can be used to walk through the list of parents of
 * the node, similar to get_children() and get_stashed().
 */
INLINE PandaNode::Parents PandaNode::
get_parents(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return Parents(cdata);
}

/**
 * The private implementation of find_parent().
 */
INLINE int PandaNode::
do_find_parent(PandaNode *node, const CData *cdata) const {
  CPT(Up) up = cdata->get_up();
  Up::const_iterator ui = up->find(UpConnection(node));
  if (ui == up->end()) {
    return -1;
  }
  return ui - up->begin();
}

/**
 * Ensures that attaching the indicated child node to this node would not
 * introduce a cycle in the graph.  Returns true if the attachment is valid,
 * false otherwise.
 */
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

/**
 * Sets the dirty_prev_transform flag, and adds the node to the
 * _dirty_prev_transforms chain.  Assumes _dirty_prev_transforms._lock is
 * already held.
 */
INLINE void PandaNode::
do_set_dirty_prev_transform() {
  nassertv(_dirty_prev_transforms._lock.debug_is_locked());
  if (!_dirty_prev_transform) {
    LinkedListNode::insert_before(&_dirty_prev_transforms);
    _dirty_prev_transform = true;
  }
}

/**
 * Clears the dirty_prev_transform flag, and removes the node from the
 * _dirty_prev_transforms chain.  Assumes _dirty_prev_transforms._lock is
 * already held.
 */
INLINE void PandaNode::
do_clear_dirty_prev_transform() {
  nassertv(_dirty_prev_transforms._lock.debug_is_locked());
  if (_dirty_prev_transform) {
    LinkedListNode::remove_from_list();
    _dirty_prev_transform = false;
  }
}

/**
 *
 */
INLINE PandaNode::DownConnection::
DownConnection(PandaNode *child, int sort) :
  _child(child),
  _sort(sort)
{
}

/**
 * Provides a partial ordering on the children of a node so that they are
 * ranked first in sort order, and then (by virtue of the ordered_vector) in
 * the order they were added.
 */
INLINE bool PandaNode::DownConnection::
operator < (const DownConnection &other) const {
  return _sort < other._sort;
}

/**
 *
 */
INLINE PandaNode *PandaNode::DownConnection::
get_child() const {
  return _child;
}

/**
 * This is only called by PandaNode::replace_child().
 */
INLINE void PandaNode::DownConnection::
set_child(PandaNode *child) {
  _child = child;
}

/**
 *
 */
INLINE int PandaNode::DownConnection::
get_sort() const {
  return _sort;
}

/**
 *
 */
INLINE PandaNode::UpConnection::
UpConnection(PandaNode *parent) :
  _parent(parent)
{
}

/**
 * Sorts the up connections of a node by pointer.  This is different from the
 * down connections of a node, which are sorted by the specified _sort number.
 * This makes it easy to locate a particular parent of a node by pointer, or
 * to test for a parent-child relationship given two node pointers.
 */
INLINE bool PandaNode::UpConnection::
operator < (const UpConnection &other) const {
  return _parent < other._parent;
}

/**
 *
 */
INLINE PandaNode *PandaNode::UpConnection::
get_parent() const {
  return _parent;
}

/**
 *
 */
INLINE PandaNode::BoundsData::
BoundsData() :
  _internal_bounds(nullptr),
  _internal_vertices(0)
{
  ++_internal_bounds_mark;
}

/**
 *
 */
INLINE PandaNode::BoundsData::
BoundsData(const PandaNode::BoundsData &copy) :
  _internal_bounds(copy._internal_bounds),
  _internal_vertices(copy._internal_vertices),
  _internal_bounds_mark(copy._internal_bounds_mark),
  _internal_bounds_computed(copy._internal_bounds_computed)
{
}

/**
 * Copies just the BoundsData part of the structure.
 */
INLINE void PandaNode::BoundsData::
copy_bounds(const PandaNode::BoundsData &copy) {
  _internal_bounds = copy._internal_bounds;
  _internal_vertices = copy._internal_vertices;
  _internal_bounds_mark = copy._internal_bounds_mark;
  _internal_bounds_computed = copy._internal_bounds_computed;
}

/**
 * Internal function to set (if value is true) or clear (if value is false)
 * the indicated bit(s) in the _fancy_bits member.
 */
INLINE void PandaNode::CData::
set_fancy_bit(int bits, bool value) {
  if (value) {
    _fancy_bits |= bits;
  } else {
    _fancy_bits &= ~bits;
  }
}

/**
 * Returns a read-only pointer to the _down list.
 */
INLINE CPT(PandaNode::Down) PandaNode::CData::
get_down() const {
  return _down.get_read_pointer();
}

/**
 * Returns a modifiable, unique pointer to the _down list.
 */
INLINE PT(PandaNode::Down) PandaNode::CData::
modify_down() {
  return _down.get_write_pointer();
}

/**
 * Returns a read-only pointer to the _stashed list.
 */
INLINE CPT(PandaNode::Down) PandaNode::CData::
get_stashed() const {
  return _stashed.get_read_pointer();
}

/**
 * Returns a modifiable, unique pointer to the _stashed list.
 */
INLINE PT(PandaNode::Down) PandaNode::CData::
modify_stashed() {
  return _stashed.get_write_pointer();
}

/**
 * Returns a read-only pointer to the _up list.
 */
INLINE CPT(PandaNode::Up) PandaNode::CData::
get_up() const {
  return _up.get_read_pointer();
}

/**
 * Returns a modifiable, unique pointer to the _up list.
 */
INLINE PT(PandaNode::Up) PandaNode::CData::
modify_up() {
  return _up.get_write_pointer();
}

/**
 *
 */
INLINE PandaNode::Children::
Children() {
}

/**
 *
 */
INLINE PandaNode::Children::
Children(const PandaNode::CData *cdata) :
  _down(cdata->get_down())
{
}

/**
 *
 */
INLINE PandaNode::Children::
Children(const PandaNode::Children &copy) :
  _down(copy._down)
{
}

/**
 *
 */
INLINE void PandaNode::Children::
operator = (const PandaNode::Children &copy) {
  _down = copy._down;
}

/**
 *
 */
INLINE PandaNode::Children::
Children(PandaNode::Children &&from) noexcept :
  _down(std::move(from._down))
{
}

/**
 *
 */
INLINE void PandaNode::Children::
operator = (PandaNode::Children &&from) noexcept {
  _down = std::move(from._down);
}

/**
 * Returns the number of children of the node.
 */
INLINE size_t PandaNode::Children::
get_num_children() const {
  nassertr(_down != nullptr, 0);
  return _down->size();
}

/**
 * Returns the nth child of the node.
 */
INLINE PandaNode *PandaNode::Children::
get_child(size_t n) const {
  nassertr(_down != nullptr, nullptr);
  nassertr(n < (size_t)_down->size(), nullptr);
  return (*_down)[n].get_child();
}

/**
 * Returns the sort index of the nth child node of this node (that is, the
 * number that was passed to add_child()).  See get_num_children().
 */
INLINE int PandaNode::Children::
get_child_sort(size_t n) const {
  nassertr(_down != nullptr, -1);
  nassertr(n < _down->size(), -1);
  return (*_down)[n].get_sort();
}

/**
 *
 */
INLINE PandaNode::Stashed::
Stashed() {
}

/**
 *
 */
INLINE PandaNode::Stashed::
Stashed(const PandaNode::CData *cdata) :
  _stashed(cdata->get_stashed())
{
}

/**
 *
 */
INLINE PandaNode::Stashed::
Stashed(const PandaNode::Stashed &copy) :
  _stashed(copy._stashed)
{
}

/**
 *
 */
INLINE void PandaNode::Stashed::
operator = (const PandaNode::Stashed &copy) {
  _stashed = copy._stashed;
}

/**
 *
 */
INLINE PandaNode::Stashed::
Stashed(PandaNode::Stashed &&from) noexcept :
  _stashed(std::move(from._stashed))
{
}

/**
 *
 */
INLINE void PandaNode::Stashed::
operator = (PandaNode::Stashed &&from) noexcept {
  _stashed = std::move(from._stashed);
}

/**
 * Returns the number of stashed children of the node.
 */
INLINE size_t PandaNode::Stashed::
get_num_stashed() const {
  nassertr(_stashed != nullptr, 0);
  return _stashed->size();
}

/**
 * Returns the nth stashed child of the node.
 */
INLINE PandaNode *PandaNode::Stashed::
get_stashed(size_t n) const {
  nassertr(_stashed != nullptr, nullptr);
  nassertr(n < _stashed->size(), nullptr);
  return (*_stashed)[n].get_child();
}

/**
 * Returns the sort index of the nth child node of this node (that is, the
 * number that was passed to add_child()).  See get_num_stashed().
 */
INLINE int PandaNode::Stashed::
get_stashed_sort(size_t n) const {
  nassertr(_stashed != nullptr, -1);
  nassertr(n < _stashed->size(), -1);
  return (*_stashed)[n].get_sort();
}

/**
 *
 */
INLINE PandaNode::Parents::
Parents() {
}

/**
 *
 */
INLINE PandaNode::Parents::
Parents(const PandaNode::CData *cdata) :
  _up(cdata->get_up())
{
}

/**
 *
 */
INLINE PandaNode::Parents::
Parents(const PandaNode::Parents &copy) :
  _up(copy._up)
{
}

/**
 *
 */
INLINE void PandaNode::Parents::
operator = (const PandaNode::Parents &copy) {
  _up = copy._up;
}

/**
 *
 */
INLINE PandaNode::Parents::
Parents(PandaNode::Parents &&from) noexcept :
  _up(std::move(from._up))
{
}

/**
 *
 */
INLINE void PandaNode::Parents::
operator = (PandaNode::Parents &&from) noexcept {
  _up = std::move(from._up);
}

/**
 * Returns the number of parents of the node.
 */
INLINE size_t PandaNode::Parents::
get_num_parents() const {
  nassertr(_up != nullptr, 0);
  return _up->size();
}

/**
 * Returns the nth parent of the node.
 */
INLINE PandaNode *PandaNode::Parents::
get_parent(size_t n) const {
  nassertr(_up != nullptr, nullptr);
  nassertr(n < _up->size(), nullptr);
  return (*_up)[n].get_parent();
}

/**
 *
 */
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
  // We node_ref the CData pointer, so that if anyone makes changes to the
  // PandaNode while we hold this pointer, it will force a copy--so that this
  // object will remain unchanged (if out-of-date).
  _cdata->node_ref();
#endif  // DO_PIPELINING
}

/**
 *
 */
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

/**
 *
 */
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

/**
 *
 */
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
  _node = nullptr;
  _cdata = nullptr;
#endif  // _DEBUG
}

/**
 *
 */
INLINE const PandaNode *PandaNodePipelineReader::
get_node() const {
  return _node;
}

/**
 *
 */
INLINE Thread *PandaNodePipelineReader::
get_current_thread() const {
  return _current_thread;
}

/**
 * Releases the lock on this object.  No future calls will be valid on this
 * object.
 */
INLINE void PandaNodePipelineReader::
release() {
  /*
  if (_cdata != (PandaNode::CData *)NULL) {
    _node->_cycler.release_read(_cdata);
    _cdata = NULL;
  }
  */
}

/**
 * Computes the result of applying this node's draw masks to a running draw
 * mask, as during a traversal.
 */
INLINE void PandaNodePipelineReader::
compose_draw_mask(DrawMask &running_draw_mask) const {
  nassertv(_cdata != nullptr);
  running_draw_mask = (running_draw_mask & ~_cdata->_draw_control_mask) |
    (_cdata->_draw_show_mask & _cdata->_draw_control_mask);
}

/**
 * Compares the running draw mask computed during a traversal with this node's
 * net draw masks.  Returns true if the node should be traversed into, or
 * false if there is nothing at this level or below that will be visible to
 * the indicated camera_mask.
 */
INLINE bool PandaNodePipelineReader::
compare_draw_mask(DrawMask running_draw_mask, DrawMask camera_mask) const {
  nassertr(_cdata != nullptr, false);
  nassertr(_cdata->_last_update == _cdata->_next_update, false);

  // As a special case, if net_draw_show_mask is all 0, it means either that
  // all nodes under this node are hidden to all cameras, or that none of them
  // are renderable nodes (or some combination). In either case, we might as
  // well short-circuit.
  if (_cdata->_net_draw_show_mask.is_zero()) {
    return false;
  }

  DrawMask net_draw_control_mask, net_draw_show_mask;
  net_draw_control_mask = _cdata->_net_draw_control_mask;
  net_draw_show_mask = _cdata->_net_draw_show_mask;

  // Now the bits that are not in net_draw_control_mask--that is, those bits
  // that are not changed by any of the nodes at this level and below--are
  // taken from running_draw_mask, which is inherited from above.  On the
  // other hand, the bits that *are* in net_draw_control_mask--those bits that
  // are changed by any of the nodes at this level and below--are taken from
  // net_draw_show_mask, which is propagated upwards from below.

  // This way, we will traverse into this node if it has any children which
  // want to be visited by the traversal, but we will avoid traversing into it
  // if all of its children are hidden to this camera.
  DrawMask compare_mask = (running_draw_mask & ~net_draw_control_mask) | (net_draw_show_mask & net_draw_control_mask);

  return !((compare_mask & PandaNode::_overall_bit).is_zero()) && !((compare_mask & camera_mask).is_zero());
}

/**
 * Returns the number of parent nodes this node has.  If this number is
 * greater than 1, the node has been multiply instanced.  The order of the
 * parent nodes is not meaningful and is not related to the order in which the
 * node was instanced to them.
 */
INLINE int PandaNodePipelineReader::
get_num_parents() const {
  return _cdata->get_up()->size();
}

/**
 * Returns the nth parent node of this node.  See get_num_parents().  Also see
 * get_parents(), if your intention is to iterate through the complete list of
 * parents; get_parents() is preferable in this case.
 */
INLINE PandaNode *PandaNodePipelineReader::
get_parent(int n) const {
  CPT(PandaNode::Up) up = _cdata->get_up();
  nassertr(n >= 0 && n < (int)up->size(), nullptr);
  return (*up)[n].get_parent();
}

/**
 * Returns the index of the indicated parent node, if it is a parent, or -1 if
 * it is not.
 */
INLINE int PandaNodePipelineReader::
find_parent(PandaNode *node) const {
  return _node->do_find_parent(node, _cdata);
}

/**
 * Returns the number of child nodes this node has.  The order of the child
 * nodes *is* meaningful and is based on the sort number that was passed to
 * add_child(), and also on the order in which the nodes were added.
 */
INLINE int PandaNodePipelineReader::
get_num_children() const {
  return _cdata->get_down()->size();
}

/**
 * Returns the nth child node of this node.  See get_num_children().  Also see
 * get_children(), if your intention is to iterate through the complete list
 * of children; get_children() is preferable in this case.
 */
INLINE PandaNode *PandaNodePipelineReader::
get_child(int n) const {
  CPT(PandaNode::Down) down = _cdata->get_down();
  nassertr(n >= 0 && n < (int)down->size(), nullptr);
  return (*down)[n].get_child();
}

/**
 * Returns the sort index of the nth child node of this node (that is, the
 * number that was passed to add_child()).  See get_num_children().
 */
INLINE int PandaNodePipelineReader::
get_child_sort(int n) const {
  CPT(PandaNode::Down) down = _cdata->get_down();
  nassertr(n >= 0 && n < (int)down->size(), -1);
  return (*down)[n].get_sort();
}

/**
 * Returns the index of the indicated child node, if it is a child, or -1 if
 * it is not.
 */
INLINE int PandaNodePipelineReader::
find_child(PandaNode *node) const {
  return _node->do_find_child(node, _cdata->get_down());
}

/**
 * Returns the number of stashed nodes this node has.  These are former
 * children of the node that have been moved to the special stashed list via
 * stash_child().
 */
INLINE int PandaNodePipelineReader::
get_num_stashed() const {
  return _cdata->get_stashed()->size();
}

/**
 * Returns the nth stashed child of this node.  See get_num_stashed().  Also
 * see get_stashed(), if your intention is to iterate through the complete
 * list of stashed children; get_stashed() is preferable in this case.
 */
INLINE PandaNode *PandaNodePipelineReader::
get_stashed(int n) const {
  CPT(PandaNode::Down) stashed = _cdata->get_stashed();
  nassertr(n >= 0 && n < (int)stashed->size(), nullptr);
  return (*stashed)[n].get_child();
}

/**
 * Returns the sort index of the nth stashed node of this node (that is, the
 * number that was passed to add_child()).  See get_num_stashed().
 */
INLINE int PandaNodePipelineReader::
get_stashed_sort(int n) const {
  CPT(PandaNode::Down) stashed = _cdata->get_stashed();
  nassertr(n >= 0 && n < (int)stashed->size(), -1);
  return (*stashed)[n].get_sort();
}

/**
 * Returns the index of the indicated stashed node, if it is a stashed child,
 * or -1 if it is not.
 */
INLINE int PandaNodePipelineReader::
find_stashed(PandaNode *node) const {
  return _node->do_find_child(node, _cdata->get_stashed());
}

/**
 * Returns the complete RenderState that will be applied to all nodes at this
 * level and below, as set on this node.  This returns only the RenderState
 * set on this particular node, and has nothing to do with state that might be
 * inherited from above.
 */
INLINE const RenderState *PandaNodePipelineReader::
get_state() const {
  return _cdata->_state;
}

/**
 * Returns the complete RenderEffects that will be applied to this node.
 */
INLINE const RenderEffects *PandaNodePipelineReader::
get_effects() const {
  return _cdata->_effects;
}

/**
 * Returns the transform that has been set on this particular node.  This is
 * not the net transform from the root, but simply the transform on this
 * particular node.
 */
INLINE const TransformState *PandaNodePipelineReader::
get_transform() const {
  return _cdata->_transform;
}

/**
 * Returns the transform that has been set as this node's "previous" position.
 * See set_prev_transform().
 */
const TransformState *PandaNodePipelineReader::
get_prev_transform() const {
  return _cdata->_prev_transform;
}

/**
 * Retrieves the user-defined value that was previously set on this node for
 * the particular key, if any.  If no value has been previously set, returns
 * the empty string.
 */
INLINE std::string PandaNodePipelineReader::
get_tag(const std::string &key) const {
  int index = _cdata->_tag_data.find(key);
  if (index >= 0) {
    return _cdata->_tag_data.get_data((size_t)index);
  } else {
    return std::string();
  }
}

/**
 * Returns true if a value has been defined on this node for the particular
 * key (even if that value is the empty string), or false if no value has been
 * set.
 */
INLINE bool PandaNodePipelineReader::
has_tag(const std::string &key) const {
  return _cdata->_tag_data.find(key) >= 0;
}

/**
 * Returns the union of all into_collide_mask() values set at CollisionNodes
 * at this level and below.
 */
INLINE CollideMask PandaNodePipelineReader::
get_net_collide_mask() const {
  nassertr(_cdata->_last_update == _cdata->_next_update, _cdata->_net_collide_mask);
  return _cdata->_net_collide_mask;
}

/**
 * Returns a ClipPlaneAttrib which represents the union of all of the clip
 * planes that have been turned *off* at this level and below.
 */
INLINE const RenderAttrib *PandaNodePipelineReader::
get_off_clip_planes() const {
  nassertr(_cdata->_last_update == _cdata->_next_update, _cdata->_off_clip_planes);
  return _cdata->_off_clip_planes;
}

/**
 * Returns the external bounding volume of this node: a bounding volume that
 * contains the user bounding volume, the internal bounding volume, and all of
 * the children's bounding volumes.
 */
INLINE const BoundingVolume *PandaNodePipelineReader::
get_bounds() const {
  nassertr(_cdata->_last_bounds_update == _cdata->_next_update, _cdata->_external_bounds);
  return _cdata->_external_bounds;
}

/**
 * Returns the total number of vertices that will be rendered by this node and
 * all of its descendents.
 *
 * This is not necessarily an accurate count of vertices that will actually be
 * rendered, since this will include all vertices of all LOD's, and it will
 * also include hidden nodes.  It may also omit or only approximate certain
 * kinds of dynamic geometry.  However, it will not include stashed nodes.
 */
INLINE int PandaNodePipelineReader::
get_nested_vertices() const {
  nassertr(_cdata->_last_bounds_update == _cdata->_next_update, _cdata->_nested_vertices);
  return _cdata->_nested_vertices;
}

/**
 * Returns the current state of the "final" flag.  Initially, this flag is off
 * (false), but it may be changed by an explicit call to set_final().  See
 * set_final().
 */
INLINE bool PandaNodePipelineReader::
is_final() const {
  return _cdata->_final_bounds;
}


/**
 * Returns the union of all of the enum FancyBits values corresponding to the
 * various "fancy" attributes that are set on the node.  If this returns 0,
 * the node has nothing interesting about it.  This is intended to speed
 * traversal by quickly skipping past nodes that don't particularly affect the
 * render state.
 */
INLINE int PandaNodePipelineReader::
get_fancy_bits() const {
  return _cdata->_fancy_bits;
}

/**
 * Returns an object that can be used to walk through the list of children of
 * the node.  When you intend to visit multiple children, using this is
 * slightly faster than calling get_child() directly on the PandaNode, since
 * this object avoids reopening the PipelineCycler each time.
 *
 * This object also protects you from self-modifying loops (e.g.  adding or
 * removing children during traversal), since a virtual copy of the children
 * is made ahead of time.  The virtual copy is fast--it is a form of copy-on-
 * write, so the list is not actually copied unless it is modified during the
 * traversal.
 */
INLINE PandaNode::Children PandaNodePipelineReader::
get_children() const {
  return PandaNode::Children(_cdata);
}

/**
 * Returns an object that can be used to walk through the list of children of
 * the node.  When you intend to visit multiple children, using this is
 * slightly faster than calling get_stashed() directly on the PandaNode, since
 * this object avoids reopening the PipelineCycler each time.
 *
 * This object also protects you from self-modifying loops (e.g.  adding or
 * removing children during traversal), since a virtual copy of the children
 * is made ahead of time.  The virtual copy is fast--it is a form of copy-on-
 * write, so the list is not actually copied unless it is modified during the
 * traversal.
 */
INLINE PandaNode::Stashed PandaNodePipelineReader::
get_stashed() const {
  return PandaNode::Stashed(_cdata);
}

/**
 * Returns an object that can be used to walk through the list of parents of
 * the node, similar to get_children() and get_stashed().
 */
INLINE PandaNode::Parents PandaNodePipelineReader::
get_parents() const {
  return PandaNode::Parents(_cdata);
}

/**
 *
 */
INLINE PandaNode::BamReaderAuxDataDown::
BamReaderAuxDataDown() :
  _down_list(PandaNode::get_class_type())
{
}
