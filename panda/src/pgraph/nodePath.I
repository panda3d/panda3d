/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodePath.I
 * @author drose
 * @date 2002-02-25
 */

/**
 * This constructs an empty NodePath with no nodes.
 */
INLINE NodePath::
NodePath() :
  _error_type(ET_ok),
  _backup_key(0)
{
}

/**
 * This constructs a new NodePath with a single node.  An ordinary, unattached
 * PandaNode is created with the indicated name.
 */
INLINE NodePath::
NodePath(const std::string &top_node_name, Thread *current_thread) :
  _error_type(ET_ok)
{
  PandaNode *top_node = new PandaNode(top_node_name);
  int pipeline_stage = current_thread->get_pipeline_stage();
  _head = top_node->get_generic_component(false, pipeline_stage, current_thread);
  _backup_key = 0;
}

/**
 * This constructs a NodePath for the indicated node.  If the node does not
 * have any parents, this creates a singleton NodePath; otherwise, it
 * automatically finds the path from the node to the root.  If the node has
 * multiple paths to the root, one path is chosen arbitrarily and a warning
 * message is printed (but see also NodePath::any_path(), below).
 */
INLINE NodePath::
NodePath(PandaNode *node, Thread *current_thread) :
  _error_type(ET_ok)
{
  if (node != nullptr) {
    int pipeline_stage = current_thread->get_pipeline_stage();
    _head = node->get_generic_component(false, pipeline_stage, current_thread);
  }
  _backup_key = 0;
}

/**
 * Returns a new NodePath that represents any arbitrary path from the root to
 * the indicated node.  This is the same thing that would be returned by
 * NodePath(node), except that no warning is issued if the path is ambiguous.
 */
INLINE NodePath NodePath::
any_path(PandaNode *node, Thread *current_thread) {
  NodePath result;
  if (node != nullptr) {
    int pipeline_stage = current_thread->get_pipeline_stage();
    result._head = node->get_generic_component(true, pipeline_stage,
                                               current_thread);
  }
  return result;
}

/**
 *
 */
INLINE NodePath::
NodePath(const NodePath &copy) :
  _head(copy._head),
  _backup_key(copy._backup_key),
  _error_type(copy._error_type)
{
}

/**
 *
 */
INLINE void NodePath::
operator = (const NodePath &copy) {
  _head = copy._head;
  _backup_key = copy._backup_key;
  _error_type = copy._error_type;
}

/**
 *
 */
INLINE NodePath::
NodePath(NodePath &&from) noexcept :
  _head(std::move(from._head)),
  _backup_key(from._backup_key),
  _error_type(from._error_type)
{
}

/**
 *
 */
INLINE void NodePath::
operator = (NodePath &&from) noexcept {
  _head = std::move(from._head);
  _backup_key = from._backup_key;
  _error_type = from._error_type;
}

/**
 * Sets this NodePath to the empty NodePath.  It will no longer point to any
 * node.
 */
INLINE void NodePath::
clear() {
  _head.clear();
  _backup_key = 0;
  _error_type = ET_ok;
}

/**
 * Creates a NodePath with the ET_not_found error type set.
 */
INLINE NodePath NodePath::
not_found() {
  NodePath result;
  result._error_type = ET_not_found;
  return result;
}

/**
 * Creates a NodePath with the ET_removed error type set.
 */
INLINE NodePath NodePath::
removed() {
  NodePath result;
  result._error_type = ET_removed;
  return result;
}

/**
 * Creates a NodePath with the ET_fail error type set.
 */
INLINE NodePath NodePath::
fail() {
  NodePath result;
  result._error_type = ET_fail;
  return result;
}

/**
 * Certain operations, such as find() or find_all_matches(), require a
 * traversal of the scene graph to search for the target node or nodes.  This
 * traversal does not attempt to detect cycles, so an arbitrary cap is set on
 * the depth of the traversal as a poor man's cycle detection, in the event
 * that a cycle has inadvertently been introduced into the scene graph.
 *
 * There may be other reasons you'd want to truncate a search before the
 * bottom of the scene graph has been reached.  In any event, this function
 * sets the limit on the number of levels that a traversal will continue, and
 * hence the maximum length of a path that may be returned by a traversal.
 *
 * This is a static method, and so changing this parameter affects all of the
 * NodePaths in the universe.
 */
INLINE void NodePath::
set_max_search_depth(int max_search_depth) {
  _max_search_depth = max_search_depth;
}

/**
 * Returns the current setting of the search depth limit.  See
 * set_max_search_depth.
 */
INLINE int NodePath::
get_max_search_depth() {
  return _max_search_depth;
}

/**
 * Returns true if the NodePath contains no nodes.
 */
INLINE bool NodePath::
is_empty() const {
  return (_head == nullptr);
}

/**
 * Returns true if the NodePath contains exactly one node.
 */
INLINE bool NodePath::
is_singleton(Thread *current_thread) const {
  int pipeline_stage = current_thread->get_pipeline_stage();
  return (_head != nullptr && _head->is_top_node(pipeline_stage, current_thread));
}

/**
 * If is_empty() is true, this returns a code that represents the reason why
 * the NodePath is empty.
 */
INLINE NodePath::ErrorType NodePath::
get_error_type() const {
  return _error_type;
}

/**
 * Returns the top node of the path, or NULL if the path is empty.  This
 * requires iterating through the path.
 */
INLINE PandaNode *NodePath::
get_top_node(Thread *current_thread) const {
  if (is_empty()) {
    return nullptr;
  }

  return get_top(current_thread).node();
}

/**
 * Returns the referenced node of the path.
 */
INLINE PandaNode *NodePath::
node() const {
  nassertr_always(!is_empty(), nullptr);
  return _head->get_node();
}

/**
 * Returns an integer that is guaranteed to be the same for all NodePaths that
 * represent the same node instance, and different for all NodePaths that
 * represent a different node instance.
 *
 * The same key will be returned for a particular instance as long as at least
 * one NodePath exists that represents that instance; if all NodePaths for a
 * particular instance destruct and a new one is later created, it may have a
 * different index.  However, a given key will never be reused for a different
 * instance (unless the app has been running long enough that we overflow the
 * integer key value).
 */
INLINE int NodePath::
get_key() const {
  if (is_empty()) {
    return _backup_key;
  }
  return _head->get_key();
}

/**
 * Adds the NodePath into the running hash.  This is intended to be used by
 * lower-level code that computes a hash for each NodePath.  It modifies the
 * hash value passed in by a unique adjustment for each NodePath, and returns
 * the modified hash.
 *
 * This is similar to the unique integer returned by get_key(), but it is not
 * guaranteed to remain unique beyond the lifetime of this particular
 * NodePath.  Once this NodePath destructs, a different NodePath may be
 * created which shares the same hash value.
 */
INLINE size_t NodePath::
add_hash(size_t hash) const {
  return pointer_hash::add_hash(hash, _head);
}

/**
 * Returns true if the node represented by this NodePath is parented within
 * the same graph as that of the other NodePath.  This is essentially the same
 * thing as asking whether get_top() of both NodePaths is the same (e.g., both
 * "render").
 */
INLINE bool NodePath::
is_same_graph(const NodePath &other, Thread *current_thread) const {
  // Actually, it's possible for the top nodes to be the same, but the
  // NodePaths still to be considered in different graphs.  But even in this
  // case, get_top() will be different for each one.  (They'll be different
  // singleton NodePaths that happen to reference the same node).

  // This will happen if one of the top nodes is considered a different
  // instance--for instance, render.instance_to(NodePath()) returns a
  // different instance of render that appears to have the same top node.  But
  // this is a very rare thing to do.
  int a_count, b_count;
  return (find_common_ancestor(*this, other, a_count, b_count, current_thread) != nullptr);
}

/**
 * Returns true if the node represented by this NodePath is a parent or other
 * ancestor of the other NodePath, or false if it is not.
 */
INLINE bool NodePath::
is_ancestor_of(const NodePath &other, Thread *current_thread) const {
  int a_count, b_count;
  if (find_common_ancestor(*this, other, a_count, b_count, current_thread) == nullptr) {
    // Not related.
    return false;
  }

  // They are related; now b is descended from a only if a is the common
  // ancestor (which is to say, a_count == 0).
  return (a_count == 0);
}

/**
 * Returns the lowest NodePath that both of these two NodePaths have in
 * common: the first ancestor that both of them share.  If the two NodePaths
 * are unrelated, returns NodePath::not_found().
 */
INLINE NodePath NodePath::
get_common_ancestor(const NodePath &other, Thread *current_thread) const {
  int a_count, b_count;
  NodePathComponent *common = find_common_ancestor(*this, other, a_count, b_count, current_thread);
  if (common == nullptr) {
    return NodePath::not_found();
  }

  NodePath result;
  result._head = common;
  return result;
}

/**
 * Returns the number of children of the referenced node.
 */
INLINE int NodePath::
get_num_children(Thread *current_thread) const {
  nassertr_always(!is_empty(), 0);
  return _head->get_node()->get_num_children(current_thread);
}

/**
 * Returns a NodePath representing the nth child of the referenced node.
 */
INLINE NodePath NodePath::
get_child(int n, Thread *current_thread) const {
  nassertr_always(n >= 0 && n < get_num_children(current_thread), NodePath());
  NodePath child;
  int pipeline_stage = current_thread->get_pipeline_stage();
  child._head = PandaNode::get_component(_head, _head->get_node()->get_child(n, current_thread),
                                         pipeline_stage, current_thread);
  return child;
}

/**
 * Returns the number of nodes at and below this level.
 */
INLINE int NodePath::
count_num_descendants() const {
  if (is_empty()) {
    return 0;
  }
  return _head->get_node()->count_num_descendants();
}

/**
 * Returns true if the referenced node has a parent; i.e.  the NodePath chain
 * contains at least two nodes.
 */
INLINE bool NodePath::
has_parent(Thread *current_thread) const {
  return !is_empty() && !is_singleton(current_thread);
}

/**
 * Returns the NodePath to the parent of the referenced node: that is, this
 * NodePath, shortened by one node.  The parent of a singleton NodePath is
 * defined to be the empty NodePath.
 */
INLINE NodePath NodePath::
get_parent(Thread *current_thread) const {
  if (!has_parent(current_thread)) {
    return NodePath();
  }

  int pipeline_stage = current_thread->get_pipeline_stage();

  NodePath parent;
  parent._head = _head->get_next(pipeline_stage, current_thread);
  return parent;
}

/**
 * Creates an ordinary PandaNode and attaches it below the current NodePath,
 * returning a new NodePath that references it.
 */
INLINE NodePath NodePath::
attach_new_node(const std::string &name, int sort, Thread *current_thread) const {
  nassertr(verify_complete(current_thread), NodePath::fail());

  return attach_new_node(new PandaNode(name), sort, current_thread);
}

/**
 * Lists the hierarchy at and below the referenced node.
 */
INLINE void NodePath::
ls() const {
  ls(nout);
}

/**
 * Lists the hierarchy at and below the referenced node.
 */
INLINE void NodePath::
ls(std::ostream &out, int indent_level) const {
  if (is_empty()) {
    out << "(empty)\n";
  } else {
    node()->ls(out, indent_level);
  }
}

/**
 * Lists the hierarchy at and above the referenced node.
 */
INLINE void NodePath::
reverse_ls() const {
  reverse_ls(nout);
}

/**
 * Changes the complete state object on this node.
 */
INLINE void NodePath::
set_state(const RenderState *state, Thread *current_thread) {
  nassertv_always(!is_empty());
  node()->set_state(state, current_thread);
}

/**
 * Returns the net state on this node from the root.
 */
INLINE CPT(RenderState) NodePath::
get_net_state(Thread *current_thread) const {
  nassertr(_error_type == ET_ok, RenderState::make_empty());
  return r_get_net_state(_head, current_thread);
}

/**
 * Adds the indicated render attribute to the scene graph on this node.  This
 * attribute will now apply to this node and everything below.  If there was
 * already an attribute of the same type, it is replaced.
 */
INLINE void NodePath::
set_attrib(const RenderAttrib *attrib, int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(attrib, priority);
}

/**
 * Returns the render attribute of the indicated type, if it is defined on the
 * node, or NULL if it is not.  This checks only what is set on this
 * particular node level, and has nothing to do with what render attributes
 * may be inherited from parent nodes.
 */
INLINE const RenderAttrib *NodePath::
get_attrib(TypeHandle type) const {
  nassertr_always(!is_empty(), nullptr);
  return node()->get_attrib(type);
}

/**
 * Returns true if there is a render attribute of the indicated type defined
 * on this node, or false if there is not.
 */
INLINE bool NodePath::
has_attrib(TypeHandle type) const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(type);
}

/**
 * Removes the render attribute of the given type from this node.  This node,
 * and the subgraph below, will now inherit the indicated render attribute
 * from the nodes above this one.
 */
INLINE void NodePath::
clear_attrib(TypeHandle type) {
  nassertv_always(!is_empty());
  node()->clear_attrib(type);
}

/**
 * Adds the indicated render effect to the scene graph on this node.  If there
 * was already an effect of the same type, it is replaced.
 */
INLINE void NodePath::
set_effect(const RenderEffect *effect) {
  nassertv_always(!is_empty());
  node()->set_effect(effect);
}

/**
 * Returns the render effect of the indicated type, if it is defined on the
 * node, or NULL if it is not.
 */
INLINE const RenderEffect *NodePath::
get_effect(TypeHandle type) const {
  nassertr_always(!is_empty(), nullptr);
  return node()->get_effect(type);
}

/**
 * Returns true if there is a render effect of the indicated type defined on
 * this node, or false if there is not.
 */
INLINE bool NodePath::
has_effect(TypeHandle type) const {
  nassertr_always(!is_empty(), false);
  return node()->has_effect(type);
}

/**
 * Removes the render effect of the given type from this node.
 */
INLINE void NodePath::
clear_effect(TypeHandle type) {
  nassertv_always(!is_empty());
  node()->clear_effect(type);
}

/**
 * Sets the complete RenderEffects that will be applied this node.  This
 * completely replaces whatever has been set on this node via repeated calls
 * to set_attrib().
 */
INLINE void NodePath::
set_effects(const RenderEffects *effects) {
  nassertv_always(!is_empty());
  node()->set_effects(effects);
}

/**
 * Returns the complete RenderEffects that will be applied to this node.
 */
INLINE const RenderEffects *NodePath::
get_effects() const {
  nassertr_always(!is_empty(), RenderEffects::make_empty());
  return node()->get_effects();
}

/**
 * Resets this node to have no render effects.
 */
INLINE void NodePath::
clear_effects() {
  nassertv_always(!is_empty());
  node()->clear_effects();
}

/**
 * Sets the transform object on this node to identity.
 */
INLINE void NodePath::
clear_transform(Thread *current_thread) {
  set_transform(TransformState::make_identity(), current_thread);
}

/**
 * Changes the complete transform object on this node.
 */
INLINE void NodePath::
set_transform(const TransformState *transform, Thread *current_thread) {
  nassertv_always(!is_empty());
  node()->set_transform(transform, current_thread);
}

/**
 * Sets the transform object on this node to identity, relative to the other
 * node.  This effectively places this node at the same position as the other
 * node.
 */
INLINE void NodePath::
clear_transform(const NodePath &other, Thread *current_thread) {
  set_transform(other, TransformState::make_identity(), current_thread);
}

/**
 * Returns the net transform on this node from the root.
 */
INLINE CPT(TransformState) NodePath::
get_net_transform(Thread *current_thread) const {
  nassertr(_error_type == ET_ok, TransformState::make_identity());
  return r_get_net_transform(_head, current_thread);
}

/**
 * Sets the transform that represents this node's "previous" position, one
 * frame ago, for the purposes of detecting motion for accurate collision
 * calculations.
 */
INLINE void NodePath::
set_prev_transform(const TransformState *transform, Thread *current_thread) {
  nassertv_always(!is_empty());
  node()->set_prev_transform(transform, current_thread);
}

/**
 * Returns the net "previous" transform on this node from the root.  See
 * set_prev_transform().
 */
INLINE CPT(TransformState) NodePath::
get_net_prev_transform(Thread *current_thread) const {
  nassertr(_error_type == ET_ok, TransformState::make_identity());
  return r_get_net_prev_transform(_head, current_thread);
}

/**
 * Sets the translation component of the transform, leaving rotation and scale
 * untouched.  This also resets the node's "previous" position, so that the
 * collision system will see the node as having suddenly appeared in the new
 * position, without passing any points in between.
 */
INLINE void NodePath::
set_pos(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_pos(LPoint3(x, y, z));
}

/**
 * Sets the translation component, without changing the "previous" position,
 * so that the collision system will see the node as moving fluidly from its
 * previous position to its new position.
 */
INLINE void NodePath::
set_fluid_pos(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_fluid_pos(LPoint3(x, y, z));
}

INLINE PN_stdfloat NodePath::
get_x() const {
  return get_pos()[0];
}

INLINE PN_stdfloat NodePath::
get_y() const {
  return get_pos()[1];
}

INLINE PN_stdfloat NodePath::
get_z() const {
  return get_pos()[2];
}

/**
 * Sets the rotation component of the transform, leaving translation and scale
 * untouched.
 */
INLINE void NodePath::
set_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  set_hpr(LVecBase3(h, p, r));
}

INLINE PN_stdfloat NodePath::
get_h() const {
  return get_hpr()[0];
}

INLINE PN_stdfloat NodePath::
get_p() const {
  return get_hpr()[1];
}

INLINE PN_stdfloat NodePath::
get_r() const {
  return get_hpr()[2];
}

/**
 * Sets the scale component of the transform, leaving translation and rotation
 * untouched.
 */
INLINE void NodePath::
set_scale(PN_stdfloat scale) {
  set_scale(LVecBase3(scale, scale, scale));
}

INLINE void NodePath::
set_scale(PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz) {
  set_scale(LVecBase3(sx, sy, sz));
}

INLINE PN_stdfloat NodePath::
get_sx() const {
  return get_scale()[0];
}

INLINE PN_stdfloat NodePath::
get_sy() const {
  return get_scale()[1];
}

INLINE PN_stdfloat NodePath::
get_sz() const {
  return get_scale()[2];
}

/**
 * Sets the shear component of the transform, leaving translation, rotation,
 * and scale untouched.
 */
INLINE void NodePath::
set_shear(PN_stdfloat shxy, PN_stdfloat shxz, PN_stdfloat shyz) {
  set_shear(LVecBase3(shxy, shxz, shyz));
}

INLINE PN_stdfloat NodePath::
get_shxy() const {
  return get_shear()[0];
}

INLINE PN_stdfloat NodePath::
get_shxz() const {
  return get_shear()[1];
}

INLINE PN_stdfloat NodePath::
get_shyz() const {
  return get_shear()[2];
}

/**
 * Sets the translation and rotation component of the transform, leaving scale
 * untouched.
 */
INLINE void NodePath::
set_pos_hpr(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  set_pos_hpr(LVecBase3(x, y, z), LVecBase3(h, p, r));
}

/**
 * Sets the rotation and scale components of the transform, leaving
 * translation untouched.
 */
INLINE void NodePath::
set_hpr_scale(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r, PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz) {
  set_hpr_scale(LVecBase3(h, p, r), LVecBase3(sx, sy, sz));
}

/**
 * Completely replaces the transform with new translation, rotation, and scale
 * components.
 */
INLINE void NodePath::
set_pos_hpr_scale(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat h, PN_stdfloat p, PN_stdfloat r,
                  PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz) {
  set_pos_hpr_scale(LVecBase3(x, y, z), LVecBase3(h, p, r),
                    LVecBase3(sx, sy, sz));
}

/**
 * Completely removes any transform from the referenced node.
 */
INLINE void NodePath::
clear_mat() {
  nassertv_always(!is_empty());
  node()->clear_transform();
}

/**
 * Returns true if a non-identity transform matrix has been applied to the
 * referenced node, false otherwise.
 */
INLINE bool NodePath::
has_mat() const {
  nassertr_always(!is_empty(), false);
  return !node()->get_transform()->is_identity();
}

/**
 * Returns the transform matrix that has been applied to the referenced node,
 * or the identity matrix if no matrix has been applied.
 */
INLINE const LMatrix4 &NodePath::
get_mat() const {
  nassertr_always(!is_empty(), LMatrix4::ident_mat());

  return node()->get_transform()->get_mat();
}


/**
 * Sets the transform on this NodePath so that it rotates to face the
 * indicated point in space.  This will overwrite any previously existing
 * scale on the node, although it will preserve any translation.
 */
INLINE void NodePath::
look_at(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  look_at(LPoint3(x, y, z));
}

/**
 * Behaves like look_at(), but with a strong preference to keeping the up
 * vector oriented in the indicated "up" direction.
 */
INLINE void NodePath::
heads_up(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  heads_up(LPoint3(x, y, z));
}

/**
 * Sets the translation component of the transform, relative to the other
 * node.
 */
INLINE void NodePath::
set_pos(const NodePath &other, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_pos(other, LPoint3(x, y, z));
}

/**
 * Sets the translation component, without changing the "previous" position,
 * so that the collision system will see the node as moving fluidly from its
 * previous position to its new position.
 */
INLINE void NodePath::
set_fluid_pos(const NodePath &other, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_fluid_pos(other, LPoint3(x, y, z));
}

INLINE PN_stdfloat NodePath::
get_x(const NodePath &other) const {
  return get_pos(other)[0];
}

INLINE PN_stdfloat NodePath::
get_y(const NodePath &other) const {
  return get_pos(other)[1];
}

INLINE PN_stdfloat NodePath::
get_z(const NodePath &other) const {
  return get_pos(other)[2];
}

/**
 * Sets the rotation component of the transform, relative to the other node.
 */
INLINE void NodePath::
set_hpr(const NodePath &other, PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  set_hpr(other, LPoint3(h, p, r));
}

INLINE PN_stdfloat NodePath::
get_h(const NodePath &other) const {
  return get_hpr(other)[0];
}

INLINE PN_stdfloat NodePath::
get_p(const NodePath &other) const {
  return get_hpr(other)[1];
}

INLINE PN_stdfloat NodePath::
get_r(const NodePath &other) const {
  return get_hpr(other)[2];
}

/**
 * Sets the scale component of the transform, relative to the other node.
 */
INLINE void NodePath::
set_scale(const NodePath &other, PN_stdfloat scale) {
  set_scale(other, LPoint3(scale, scale, scale));
}

/**
 * Sets the scale component of the transform, relative to the other node.
 */
INLINE void NodePath::
set_scale(const NodePath &other, PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz) {
  set_scale(other, LPoint3(sx, sy, sz));
}

/**
 * Returns the relative scale of the referenced node as seen from the other
 * node.
 */
INLINE PN_stdfloat NodePath::
get_sx(const NodePath &other) const {
  return get_scale(other)[0];
}

INLINE PN_stdfloat NodePath::
get_sy(const NodePath &other) const {
  return get_scale(other)[1];
}

INLINE PN_stdfloat NodePath::
get_sz(const NodePath &other) const {
  return get_scale(other)[2];
}

/**
 * Sets the shear component of the transform, relative to the other node.
 */
INLINE void NodePath::
set_shear(const NodePath &other, PN_stdfloat shxy, PN_stdfloat shxz, PN_stdfloat shyz) {
  set_shear(other, LPoint3(shxy, shxz, shyz));
}

/**
 * Returns the relative shear of the referenced node as seen from the other
 * node.
 */
INLINE PN_stdfloat NodePath::
get_shxy(const NodePath &other) const {
  return get_shear(other)[0];
}

INLINE PN_stdfloat NodePath::
get_shxz(const NodePath &other) const {
  return get_shear(other)[1];
}

INLINE PN_stdfloat NodePath::
get_shyz(const NodePath &other) const {
  return get_shear(other)[2];
}

/**
 * Sets the translation and rotation component of the transform, relative to
 * the other node.
 */
INLINE void NodePath::
set_pos_hpr(const NodePath &other,
            PN_stdfloat x, PN_stdfloat y, PN_stdfloat z,
            PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  set_pos_hpr(other, LVecBase3(x, y, z), LVecBase3(h, p, r));
}

/**
 * Sets the rotation and scale components of the transform, leaving
 * translation untouched.  This, or set_pos_hpr_scale, is the preferred way to
 * update a transform when both hpr and scale are to be changed.
 */
INLINE void NodePath::
set_hpr_scale(const NodePath &other,
        PN_stdfloat h, PN_stdfloat p, PN_stdfloat r, PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz) {
  set_hpr_scale(other, LVecBase3(h, p, r), LVecBase3(sx, sy, sz));
}

/**
 * Completely replaces the transform with new translation, rotation, and scale
 * components, relative to the other node.
 */
INLINE void NodePath::
set_pos_hpr_scale(const NodePath &other,
                  PN_stdfloat x, PN_stdfloat y, PN_stdfloat z,
                  PN_stdfloat h, PN_stdfloat p, PN_stdfloat r,
                  PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz) {
  set_pos_hpr_scale(other, LVecBase3(x, y, z), LVecBase3(h, p, r),
                    LVecBase3(sx, sy, sz));
}

/**
 * Sets the hpr on this NodePath so that it rotates to face the indicated
 * point in space, which is relative to the other NodePath.
 */
INLINE void NodePath::
look_at(const NodePath &other, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  look_at(other, LPoint3(x, y, z));
}

/**
 * Behaves like look_at(), but with a strong preference to keeping the up
 * vector oriented in the indicated "up" direction.
 */
INLINE void NodePath::
heads_up(const NodePath &other, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  heads_up(other, LPoint3(x, y, z));
}

/**
 * Returns the straight-line distance between this referenced node's
 * coordinate frame's origin, and that of the other node's origin.
 */
INLINE PN_stdfloat NodePath::
get_distance(const NodePath &other) const {
  LPoint3 pos = get_pos(other);
  return length(LVector3(pos));
}

/**
 * Sets the color scale component of the transform
 */
INLINE void NodePath::
set_color_scale(PN_stdfloat sr, PN_stdfloat sg, PN_stdfloat sb, PN_stdfloat sa, int priority) {
  set_color_scale(LVecBase4(sr, sg, sb, sa), priority);
}

/**
 * Sets the color scale component of the transform
 */
INLINE void NodePath::
compose_color_scale(PN_stdfloat sr, PN_stdfloat sg, PN_stdfloat sb, PN_stdfloat sa, int priority) {
  compose_color_scale(LVecBase4(sr, sg, sb, sa), priority);
}

/**
 * Sets the red scale component of the transform
 */
INLINE void NodePath::
set_sr(PN_stdfloat sr) {
  LVecBase4 new_scale = get_color_scale();
  new_scale[0] = sr;

  set_color_scale(new_scale);
}

/**
 * Sets the alpha scale component of the transform
 */
INLINE void NodePath::
set_sg(PN_stdfloat sg) {
  LVecBase4 new_scale = get_color_scale();
  new_scale[1] = sg;

  set_color_scale(new_scale);
}

/**
 * Sets the blue scale component of the transform
 */
INLINE void NodePath::
set_sb(PN_stdfloat sb) {
  LVecBase4 new_scale = get_color_scale();
  new_scale[2] = sb;

  set_color_scale(new_scale);
}

/**
 * Sets the alpha scale component of the transform
 */
INLINE void NodePath::
set_sa(PN_stdfloat sa) {
  LVecBase4 new_scale = get_color_scale();
  new_scale[3] = sa;

  set_color_scale(new_scale);
}

/**
 * Gets the red scale component of the transform
 */
INLINE PN_stdfloat NodePath::
get_sr() const {
  return get_color_scale()[0];
}

/**
 * Gets the green scale component of the transform
 */
INLINE PN_stdfloat NodePath::
get_sg() const {
  return get_color_scale()[1];
}

/**
 * Gets the blue scale component of the transform
 */
INLINE PN_stdfloat NodePath::
get_sb() const {
  return get_color_scale()[2];
}

/**
 * Gets the alpha scale component of the transform
 */
INLINE PN_stdfloat NodePath::
get_sa() const {
  return get_color_scale()[3];
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const PTA_float &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const PTA_double &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const PTA_int &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const PTA_LVecBase4 &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const PTA_LVecBase3 &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}


/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const PTA_LVecBase2 &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const LVecBase4 &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const LVecBase3 &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const LVecBase2 &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const PTA_LVecBase4i &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const PTA_LVecBase3i &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}


/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const PTA_LVecBase2i &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const LVecBase4i &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const LVecBase3i &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const LVecBase2i &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const PTA_LMatrix4 &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const PTA_LMatrix3 &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const LMatrix4 &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const LMatrix3 &v, int priority) {
  set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, Texture *tex, int priority) {
  set_shader_input(ShaderInput(std::move(id), tex, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, Texture *tex, const SamplerState &sampler, int priority) {
  set_shader_input(ShaderInput(std::move(id), tex, sampler, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, Texture *tex, bool read, bool write, int z, int n, int priority) {
  set_shader_input(ShaderInput(std::move(id), tex, read, write, z, n, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, ShaderBuffer *buf, int priority) {
  set_shader_input(ShaderInput(std::move(id), buf, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, const NodePath &np, int priority) {
  set_shader_input(ShaderInput(std::move(id), np, priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, int n1, int n2, int n3, int n4, int priority) {
  set_shader_input(ShaderInput(std::move(id), LVecBase4i(n1, n2, n3, n4), priority));
}

/**
 *
 */
INLINE void NodePath::
set_shader_input(CPT_InternalName id, PN_stdfloat n1, PN_stdfloat n2, PN_stdfloat n3, PN_stdfloat n4, int priority) {
  set_shader_input(ShaderInput(std::move(id), LVecBase4(n1, n2, n3, n4), priority));
}

/**
 * Sets a texture matrix on the current node to apply the indicated offset to
 * UV's for the given stage.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE void NodePath::
set_tex_offset(TextureStage *stage, PN_stdfloat u, PN_stdfloat v) {
  set_tex_offset(stage, LVecBase2(u, v));
}

/**
 * Sets a texture matrix on the current node to apply the indicated offset to
 * UV's for the given stage.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE void NodePath::
set_tex_offset(TextureStage *stage, const LVecBase2 &uv) {
  nassertv_always(!is_empty());
  set_tex_transform(stage,
                    get_tex_transform(stage)->set_pos2d(uv));
}

/**
 * Sets a texture matrix on the current node to apply the indicated rotation,
 * clockwise in degrees, to UV's for the given stage.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE void NodePath::
set_tex_rotate(TextureStage *stage, PN_stdfloat r) {
  nassertv_always(!is_empty());
  set_tex_transform(stage,
                    get_tex_transform(stage)->set_rotate2d(r));
}

/**
 * Sets a texture matrix on the current node to apply the indicated scale to
 * UVW's for the given stage.
 *
 * This call is appropriate for 2-d or 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_scale(TextureStage *stage, PN_stdfloat scale) {
  nassertv_always(!is_empty());
  set_tex_transform(stage,
                    get_tex_transform(stage)->set_scale(scale));
}

/**
 * Sets a texture matrix on the current node to apply the indicated scale to
 * UV's for the given stage.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE void NodePath::
set_tex_scale(TextureStage *stage, PN_stdfloat su, PN_stdfloat sv) {
  set_tex_scale(stage, LVecBase2(su, sv));
}

/**
 * Sets a texture matrix on the current node to apply the indicated scale to
 * UV's for the given stage.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE void NodePath::
set_tex_scale(TextureStage *stage, const LVecBase2 &scale) {
  nassertv_always(!is_empty());
  set_tex_transform(stage,
                    get_tex_transform(stage)->set_scale2d(scale));
}

/**
 * Returns the offset set for the UV's for the given stage on the current
 * node.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE LVecBase2 NodePath::
get_tex_offset(TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase2::zero());
  return get_tex_transform(stage)->get_pos2d();
}

/**
 * Returns the rotation set for the UV's for the given stage on the current
 * node.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE PN_stdfloat NodePath::
get_tex_rotate(TextureStage *stage) const {
  nassertr_always(!is_empty(), 0.0f);
  return get_tex_transform(stage)->get_rotate2d();
}

/**
 * Returns the scale set for the UV's for the given stage on the current node.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE LVecBase2 NodePath::
get_tex_scale(TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase2(1.0f, 1.0f));
  return get_tex_transform(stage)->get_scale2d();
}

/**
 * Sets a texture matrix on the current node to apply the indicated offset to
 * UVW's for the given stage.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_pos(TextureStage *stage, PN_stdfloat u, PN_stdfloat v, PN_stdfloat w) {
  set_tex_pos(stage, LVecBase3(u, v, w));
}

/**
 * Sets a texture matrix on the current node to apply the indicated offset to
 * UVW's for the given stage.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_pos(TextureStage *stage, const LVecBase3 &uvw) {
  nassertv_always(!is_empty());
  set_tex_transform(stage,
                    get_tex_transform(stage)->set_pos(uvw));
}

/**
 * Sets a texture matrix on the current node to apply the indicated rotation,
 * as a 3-D HPR, to UVW's for the given stage.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_hpr(TextureStage *stage, PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  set_tex_hpr(stage, LVecBase3(h, p, r));
}

/**
 * Sets a texture matrix on the current node to apply the indicated rotation,
 * as a 3-D HPR, to UVW's for the given stage.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_hpr(TextureStage *stage, const LVecBase3 &hpr) {
  nassertv_always(!is_empty());
  set_tex_transform(stage,
                    get_tex_transform(stage)->set_hpr(hpr));
}

/**
 * Sets a texture matrix on the current node to apply the indicated scale to
 * UVW's for the given stage.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_scale(TextureStage *stage, PN_stdfloat su, PN_stdfloat sv, PN_stdfloat sw) {
  set_tex_scale(stage, LVecBase3(su, sv, sw));
}

/**
 * Sets a texture matrix on the current node to apply the indicated scale to
 * UVW's for the given stage.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_scale(TextureStage *stage, const LVecBase3 &scale) {
  nassertv_always(!is_empty());
  set_tex_transform(stage,
                    get_tex_transform(stage)->set_scale(scale));
}

/**
 * Returns the offset set for the UVW's for the given stage on the current
 * node.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE LVecBase3 NodePath::
get_tex_pos(TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase3::zero());
  return get_tex_transform(stage)->get_pos();
}

/**
 * Returns the 3-D HPR set for the UVW's for the given stage on the current
 * node.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE LVecBase3 NodePath::
get_tex_hpr(TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase3::zero());
  return get_tex_transform(stage)->get_hpr();
}

/**
 * Returns the scale set for the UVW's for the given stage on the current
 * node.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE LVecBase3 NodePath::
get_tex_scale_3d(TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase3(1.0f, 1.0f, 1.0f));
  return get_tex_transform(stage)->get_scale();
}

/**
 * Sets a texture matrix on the current node to apply the indicated offset to
 * UV's for the given stage.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE void NodePath::
set_tex_offset(const NodePath &other, TextureStage *stage, PN_stdfloat u, PN_stdfloat v) {
  set_tex_offset(other, stage, LVecBase2(u, v));
}

/**
 * Sets a texture matrix on the current node to apply the indicated offset to
 * UV's for the given stage.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE void NodePath::
set_tex_offset(const NodePath &other, TextureStage *stage, const LVecBase2 &uv) {
  nassertv_always(!is_empty());
  set_tex_transform(other, stage,
                    get_tex_transform(other, stage)->set_pos2d(uv));
}

/**
 * Sets a texture matrix on the current node to apply the indicated rotation,
 * clockwise in degrees, to UV's for the given stage.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE void NodePath::
set_tex_rotate(const NodePath &other, TextureStage *stage, PN_stdfloat r) {
  nassertv_always(!is_empty());
  set_tex_transform(other, stage,
                    get_tex_transform(other, stage)->set_rotate2d(r));
}

/**
 * Sets a texture matrix on the current node to apply the indicated scale to
 * UV's for the given stage.
 *
 * This call is appropriate for 2-d or 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_scale(const NodePath &other, TextureStage *stage, PN_stdfloat scale) {
  nassertv_always(!is_empty());
  set_tex_transform(other, stage,
                    get_tex_transform(stage)->set_scale(scale));
}

/**
 * Sets a texture matrix on the current node to apply the indicated scale to
 * UV's for the given stage.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE void NodePath::
set_tex_scale(const NodePath &other, TextureStage *stage, PN_stdfloat su, PN_stdfloat sv) {
  set_tex_scale(other, stage, LVecBase2(su, sv));
}

/**
 * Sets a texture matrix on the current node to apply the indicated scale to
 * UV's for the given stage.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE void NodePath::
set_tex_scale(const NodePath &other, TextureStage *stage, const LVecBase2 &scale) {
  nassertv_always(!is_empty());
  set_tex_transform(other, stage,
                    get_tex_transform(stage)->set_scale2d(scale));
}

/**
 * Returns the offset set for the UV's for the given stage on the current
 * node.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE LVecBase2 NodePath::
get_tex_offset(const NodePath &other, TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase2::zero());
  return get_tex_transform(other, stage)->get_pos2d();
}

/**
 * Returns the rotation set for the UV's for the given stage on the current
 * node.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE PN_stdfloat NodePath::
get_tex_rotate(const NodePath &other, TextureStage *stage) const {
  nassertr_always(!is_empty(), 0.0f);
  return get_tex_transform(other, stage)->get_rotate2d();
}

/**
 * Returns the scale set for the UV's for the given stage on the current node.
 *
 * This call is appropriate for ordinary 2-d texture coordinates.
 */
INLINE LVecBase2 NodePath::
get_tex_scale(const NodePath &other, TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase2(1.0f, 1.0f));
  return get_tex_transform(other, stage)->get_scale2d();
}

/**
 * Sets a texture matrix on the current node to apply the indicated offset to
 * UVW's for the given stage.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_pos(const NodePath &other, TextureStage *stage, PN_stdfloat u, PN_stdfloat v, PN_stdfloat w) {
  set_tex_pos(other, stage, LVecBase3(u, v, w));
}

/**
 * Sets a texture matrix on the current node to apply the indicated offset to
 * UVW's for the given stage.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_pos(const NodePath &other, TextureStage *stage, const LVecBase3 &uvw) {
  nassertv_always(!is_empty());
  set_tex_transform(other, stage,
                    get_tex_transform(stage)->set_pos(uvw));
}

/**
 * Sets a texture matrix on the current node to apply the indicated rotation,
 * as a 3-D HPR, to UVW's for the given stage.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_hpr(const NodePath &other, TextureStage *stage, PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  set_tex_hpr(other, stage, LVecBase3(h, p, r));
}

/**
 * Sets a texture matrix on the current node to apply the indicated rotation,
 * as a 3-D HPR, to UVW's for the given stage.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_hpr(const NodePath &other, TextureStage *stage, const LVecBase3 &hpr) {
  nassertv_always(!is_empty());
  set_tex_transform(other, stage,
                    get_tex_transform(stage)->set_hpr(hpr));
}

/**
 * Sets a texture matrix on the current node to apply the indicated scale to
 * UVW's for the given stage.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_scale(const NodePath &other, TextureStage *stage, PN_stdfloat su, PN_stdfloat sv, PN_stdfloat sw) {
  set_tex_scale(other, stage, LVecBase3(su, sv, sw));
}

/**
 * Sets a texture matrix on the current node to apply the indicated scale to
 * UVW's for the given stage.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE void NodePath::
set_tex_scale(const NodePath &other, TextureStage *stage, const LVecBase3 &scale) {
  nassertv_always(!is_empty());
  set_tex_transform(other, stage,
                    get_tex_transform(stage)->set_scale(scale));
}

/**
 * Returns the offset set for the UVW's for the given stage on the current
 * node.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE LVecBase3 NodePath::
get_tex_pos(const NodePath &other, TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase3::zero());
  return get_tex_transform(stage)->get_pos();
}

/**
 * Returns the 3-D HPR set for the UVW's for the given stage on the current
 * node.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE LVecBase3 NodePath::
get_tex_hpr(const NodePath &other, TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase3::zero());
  return get_tex_transform(stage)->get_hpr();
}

/**
 * Returns the scale set for the UVW's for the given stage on the current
 * node.
 *
 * This call is appropriate for 3-d texture coordinates.
 */
INLINE LVecBase3 NodePath::
get_tex_scale_3d(const NodePath &other, TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase3(1.0f, 1.0f, 1.0f));
  return get_tex_transform(stage)->get_scale();
}

/**
 * Undoes the effect of project_texture().
 */
INLINE void NodePath::
clear_project_texture(TextureStage *stage) {
  clear_texture(stage);
  clear_tex_gen(stage);
  clear_tex_projector(stage);
}

/**
 * Returns true if there are at least some vertices at this node and below
 * that use the named texture coordinate set, false otherwise.  Pass the empty
 * string for the default texture coordinate set.
 */
INLINE bool NodePath::
has_texcoord(const std::string &texcoord_name) const {
  return has_vertex_column(InternalName::get_texcoord_name(texcoord_name));
}

/**
 * Puts a billboard transition on the node such that it will rotate in two
 * dimensions around the up axis.
 */
INLINE void NodePath::
set_billboard_axis(PN_stdfloat offset) {
  set_billboard_axis(NodePath(), offset);
}

/**
 * Puts a billboard transition on the node such that it will rotate in three
 * dimensions about the origin, keeping its up vector oriented to the top of
 * the camera.
 */
INLINE void NodePath::
set_billboard_point_eye(PN_stdfloat offset, bool fixed_depth) {
  set_billboard_point_eye(NodePath(), offset, fixed_depth);
}

/**
 * Puts a billboard transition on the node such that it will rotate in three
 * dimensions about the origin, keeping its up vector oriented to the sky.
 */
INLINE void NodePath::
set_billboard_point_world(PN_stdfloat offset) {
  set_billboard_point_world(NodePath(), offset);
}

/**
 * Adds the indicated adjustment amount (which may be negative) to the
 * priority for all transitions on the referenced node, and for all nodes in
 * the subgraph below.  This can be used to force these nodes not to be
 * overridden by a high-level state change above.  If the priority would drop
 * below zero, it is set to zero.
 */
INLINE void NodePath::
adjust_all_priorities(int adjustment) {
  nassertv_always(!is_empty());
  r_adjust_all_priorities(node(), adjustment);
}

/**
 * Undoes the effect of a previous hide() on this node: makes the referenced
 * node (and the entire subgraph below this node) visible to all cameras.
 *
 * This will not reveal the node if a parent node has been hidden.
 */
INLINE void NodePath::
show() {
  nassertv_always(!is_empty());
  node()->adjust_draw_mask(DrawMask::all_off(), DrawMask::all_off(), PandaNode::get_overall_bit());
}

/**
 * Makes the referenced node visible just to the cameras whose camera_mask
 * shares the indicated bits.
 *
 * This undoes the effect of a previous hide() call.  It will not reveal the
 * node if a parent node has been hidden.  However, see show_through().
 */
INLINE void NodePath::
show(DrawMask camera_mask) {
  nassertv_always(!is_empty());
  camera_mask &= ~PandaNode::get_overall_bit();
  node()->adjust_draw_mask(DrawMask::all_off(), DrawMask::all_off(), camera_mask);
}

/**
 * Makes the referenced node visible just to the cameras whose camera_mask
 * shares the indicated bits.
 *
 * Unlike show(), this will reveal the node even if a parent node has been
 * hidden, thus "showing through" a parent's hide().
 */
INLINE void NodePath::
show_through() {
  nassertv_always(!is_empty());
  node()->adjust_draw_mask(PandaNode::get_overall_bit(), DrawMask::all_off(), DrawMask::all_off());
}

/**
 * Makes the referenced node visible just to the cameras whose camera_mask
 * shares the indicated bits.
 *
 * Unlike show(), this will reveal the node even if a parent node has been
 * hidden via the one-parameter hide() method, thus "showing through" a
 * parent's hide().  (However, it will not show through a parent's hide() call
 * if the no-parameter form of hide() was used.)
 */
INLINE void NodePath::
show_through(DrawMask camera_mask) {
  nassertv_always(!is_empty());
  camera_mask &= ~PandaNode::get_overall_bit();
  node()->adjust_draw_mask(camera_mask, DrawMask::all_off(), DrawMask::all_off());
}

/**
 * Makes the referenced node (and the entire subgraph below this node)
 * invisible to all cameras.  It remains part of the scene graph, its bounding
 * volume still contributes to its parent's bounding volume, and it will still
 * be involved in collision tests.
 */
INLINE void NodePath::
hide() {
  nassertv_always(!is_empty());
  node()->adjust_draw_mask(DrawMask::all_off(), PandaNode::get_overall_bit(), DrawMask::all_off());
}

/**
 * Makes the referenced node invisible just to the cameras whose camera_mask
 * shares the indicated bits.
 *
 * This will also hide any nodes below this node in the scene graph, including
 * those nodes for which show() has been called, but it will not hide
 * descendent nodes for which show_through() has been called.
 */
INLINE void NodePath::
hide(DrawMask camera_mask) {
  nassertv_always(!is_empty());
  camera_mask &= ~PandaNode::get_overall_bit();
  node()->adjust_draw_mask(DrawMask::all_off(), camera_mask, DrawMask::all_off());
}

/**
 * Returns true if the referenced node is hidden from the indicated camera(s)
 * either directly, or because some ancestor is hidden.
 */
INLINE bool NodePath::
is_hidden(DrawMask camera_mask) const {
  return !get_hidden_ancestor(camera_mask).is_empty();
}

/**
 * Returns true if the referenced node is stashed either directly, or because
 * some ancestor is stashed.
 */
INLINE bool NodePath::
is_stashed() const {
  return !get_stashed_ancestor().is_empty();
}

/**
 * Returns the union of all of the into_collide_masks for nodes at this level
 * and below.  This is the same thing as node()->get_net_collide_mask().
 *
 * If you want to return what the into_collide_mask of this node itself is,
 * without regard to its children, use node()->get_into_collide_mask().
 */
INLINE CollideMask NodePath::
get_collide_mask() const {
  nassertr_always(!is_empty(), CollideMask::all_off());
  return node()->get_net_collide_mask();
}

/**
 * Recursively applies the indicated CollideMask to the into_collide_masks for
 * all nodes at this level and below.  If node_type is not TypeHandle::none(),
 * then only nodes matching (or inheriting from) the indicated PandaNode
 * subclass are modified.
 *
 * The default is to change all bits, but if bits_to_change is not all bits
 * on, then only the bits that are set in bits_to_change are modified,
 * allowing this call to change only a subset of the bits in the subgraph.
 */
INLINE void NodePath::
set_collide_mask(CollideMask new_mask, CollideMask bits_to_change,
                 TypeHandle node_type) {
  nassertv_always(!is_empty());
  if (node_type == TypeHandle::none()) {
    node_type = PandaNode::get_class_type();
  }

  r_set_collide_mask(node(), ~bits_to_change, new_mask & bits_to_change,
                     node_type);
}

/**
 * Returns true if the two paths are equivalent; that is, if they contain the
 * same list of nodes in the same order.
 */
INLINE bool NodePath::
operator == (const NodePath &other) const {
  return _head == other._head;
}

/**
 * Returns true if the two paths are not equivalent.
 */
INLINE bool NodePath::
operator != (const NodePath &other) const {
  return _head != other._head;
}

/**
 * Returns true if this NodePath sorts before the other one, false otherwise.
 * The sorting order of two nonequivalent NodePaths is consistent but
 * undefined, and is useful only for storing NodePaths in a sorted container
 * like an STL set.
 */
INLINE bool NodePath::
operator < (const NodePath &other) const {
  return _head < other._head;
}

/**
 * Returns a number less than zero if this NodePath sorts before the other
 * one, greater than zero if it sorts after, or zero if they are equivalent.
 *
 * Two NodePaths are considered equivalent if they consist of exactly the same
 * list of nodes in the same order.  Otherwise, they are different; different
 * NodePaths will be ranked in a consistent but undefined ordering; the
 * ordering is useful only for placing the NodePaths in a sorted container
 * like an STL set.
 */
INLINE int NodePath::
compare_to(const NodePath &other) const {
  // Nowadays, the NodePathComponents at the head are pointerwise equivalent
  // if and only if the NodePaths are equivalent.  So we only have to compare
  // pointers.
  if (_head != other._head) {
    return _head < other._head ? -1 : 1;
  }
  return 0;
}

/**
 * Recursively walks through the scene graph at this level and below, looking
 * for ModelNodes, and calls model_node->set_preserve_transform(PT_drop_node)
 * on each one.  This allows a subsequent call to flatten_strong() to
 * eliminate all of the ModelNodes.
 *
 * Returns the number of ModelNodes found.
 */
INLINE int NodePath::
clear_model_nodes() {
  nassertr_always(!is_empty(), 0);
  return r_clear_model_nodes(node());
}

/**
 * Associates a user-defined value with a user-defined key which is stored on
 * the node.  This value has no meaning to Panda; but it is stored
 * indefinitely on the node until it is requested again.
 *
 * Each unique key stores a different string value.  There is no effective
 * limit on the number of different keys that may be stored or on the length
 * of any one key's value.
 */
INLINE void NodePath::
set_tag(const std::string &key, const std::string &value) {
  nassertv_always(!is_empty());
  node()->set_tag(key, value);
}

/**
 * Retrieves the user-defined value that was previously set on this node for
 * the particular key, if any.  If no value has been previously set, returns
 * the empty string.  See also get_net_tag().
 */
INLINE std::string NodePath::
get_tag(const std::string &key) const {
  // An empty NodePath quietly returns no tags.  This makes get_net_tag()
  // easier to implement.
  if (is_empty()) {
    return std::string();
  }
  return node()->get_tag(key);
}

/**
 * Fills the given vector up with the list of tags on this PandaNode.
 *
 * It is the user's responsibility to ensure that the keys vector is empty
 * before making this call; otherwise, the new files will be appended to it.
 */
INLINE void NodePath::
get_tag_keys(vector_string &keys) const {
  nassertv_always(!is_empty());
  node()->get_tag_keys(keys);
}

/**
 * Returns true if a value has been defined on this node for the particular
 * key (even if that value is the empty string), or false if no value has been
 * set.  See also has_net_tag().
 */
INLINE bool NodePath::
has_tag(const std::string &key) const {
  // An empty NodePath quietly has no tags.  This makes has_net_tag() easier
  // to implement.
  if (is_empty()) {
    return false;
  }
  return node()->has_tag(key);
}

/**
 * Removes the value defined for this key on this particular node.  After a
 * call to clear_tag(), has_tag() will return false for the indicated key.
 */
INLINE void NodePath::
clear_tag(const std::string &key) {
  nassertv_always(!is_empty());
  node()->clear_tag(key);
}

/**
 * Returns the tag value that has been defined on this node, or the nearest
 * ancestor node, for the indicated key.  If no value has been defined for the
 * indicated key on any ancestor node, returns the empty string.  See also
 * get_tag().
 */
INLINE std::string NodePath::
get_net_tag(const std::string &key) const {
  return find_net_tag(key).get_tag(key);
}

/**
 * Returns true if the indicated tag value has been defined on this node or on
 * any ancestor node, or false otherwise.  See also has_tag().
 */
INLINE bool NodePath::
has_net_tag(const std::string &key) const {
  return !find_net_tag(key).is_empty();
}

/**
 * Lists the tags to the nout stream, one per line.  See
 * PandaNode::list_tags() for a variant that allows you to specify the output
 * stream.
 */
INLINE void NodePath::
list_tags() const {
  nassertv_always(!is_empty());
  node()->list_tags(nout);
  nout << "\n";
}

/**
 * Changes the name of the referenced node.
 */
INLINE void NodePath::
set_name(const std::string &name) {
  nassertv_always(!is_empty());
  node()->set_name(name);
}

/**
 * Returns the name of the referenced node.
 */
INLINE std::string NodePath::
get_name() const {
  nassertr_always(!is_empty(), std::string());
  return node()->get_name();
}

/**
 * Converts the NodePath object into a single stream of data using a
 * BamWriter, and returns that data as a string string.  Returns empty string
 * on failure.  This is similar to write_bam_stream().
 *
 * This method is used by __reduce__ to handle streaming of NodePaths to a
 * pickle file.
 */
INLINE vector_uchar NodePath::
encode_to_bam_stream() const {
  vector_uchar data;
  if (!encode_to_bam_stream(data)) {
    data.clear();
  }
  return data;
}


INLINE std::ostream &operator << (std::ostream &out, const NodePath &node_path) {
  node_path.output(out);
  return out;
}
