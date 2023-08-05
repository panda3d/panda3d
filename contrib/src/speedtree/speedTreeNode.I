/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file speedTreeNode.I
 * @author drose
 * @date 2010-09-30
 */

/**
 * Returns true if the node is valid and ready to render, false otherwise.
 * Note that this might not become false until after the first time the node
 * is rendered.
 */
INLINE bool SpeedTreeNode::
is_valid() const {
  return _is_valid;
}

/**
 * Returns the number of unique tree objects that have been added to the node.
 * This count does not include multiple instances of the same tree that appear
 * in different transforms.
 */
INLINE int SpeedTreeNode::
get_num_trees() const {
  return (int)_trees.size();
}

/**
 * Returns the STTree pointer for the nth tree.  See get_num_trees().
 */
INLINE const STTree *SpeedTreeNode::
get_tree(int n) const {
  nassertr(n >= 0 && n < (int)_trees.size(), nullptr);
  InstanceList *instance_list = _trees[n];
  return instance_list->get_tree();
}

/**
 * Returns a list of transforms that corresponds to the instances at which the
 * nth tree appears.
 */
INLINE const SpeedTreeNode::InstanceList &SpeedTreeNode::
get_instance_list(int n) const {
  // TODO: This should be nassertr instead of assert, but there's nothing we
  // can really return when the assert fails.
  assert(n >= 0 && n < (int)_trees.size());
  InstanceList *instance_list = _trees[n];
  return *instance_list;
}

/**
 * Returns a modifiable STTree pointer for the nth tree instance.
 */
INLINE STTree *SpeedTreeNode::
modify_tree(int n) {
  nassertr(n >= 0 && n < (int)_trees.size(), nullptr);
  InstanceList *instance_list = _trees[n];
  _needs_repopulate = true;
  return (STTree *)instance_list->get_tree();
}

/**
 * Removes the terrain associated with the node.
 */
INLINE void SpeedTreeNode::
clear_terrain() {
  set_terrain(nullptr);
}

/**
 * Returns true if a valid terrain has been associated with the node, false
 * otherwise.
 */
INLINE bool SpeedTreeNode::
has_terrain() const {
  return _terrain != nullptr;
}

/**
 * Returns the terrain associated with the node, or NULL if there is no
 * terrain.
 */
INLINE STTerrain *SpeedTreeNode::
get_terrain() const {
  return _terrain;
}

/**
 * Specifies an offset that is to be added each frame to the global clock's
 * frame_time for the purpose of animating the trees in this particular node.
 * Also see set_global_time_delta().
 */
INLINE void SpeedTreeNode::
set_time_delta(double delta) {
  _time_delta = delta;
}

/**
 * Returns an offset that is to be added each frame to the global clock's
 * frame_time for the purpose of animating the trees in this particular node.
 * Also see get_global_time_delta().
 */
INLINE double SpeedTreeNode::
get_time_delta() const {
  return _time_delta;
}

/**
 * Specifies an offset that is to be added each frame to the global clock's
 * frame_time for the purpose of animating the trees in all SpeedTreeNodes.
 * Also see set_time_delta().
 */
INLINE void SpeedTreeNode::
set_global_time_delta(double delta) {
  _global_time_delta = delta;
}

/**
 * Returns an offset that is to be added each frame to the global clock's
 * frame_time for the purpose of animating the trees in all SpeedTreeNodes.
 * Also see get_time_delta().
 */
INLINE double SpeedTreeNode::
get_global_time_delta() {
  return _global_time_delta;
}

/**
 *
 */
INLINE SpeedTreeNode::InstanceList::
InstanceList(const STTree *tree) : _tree((STTree *)tree) {
}

/**
 * Used for comparison for ov_set.
 */
INLINE bool SpeedTreeNode::InstanceList::
operator < (const InstanceList &other) const {
  return _tree < other._tree;
}

/**
 * Returns the particular tree this list refers to.
 */
INLINE const STTree *SpeedTreeNode::InstanceList::
get_tree() const {
  return _tree;
}

/**
 * Returns the number of instances of this tree.
 */
INLINE int SpeedTreeNode::InstanceList::
get_num_instances() const {
  return (int)_instances.size();
}

/**
 * Returns the transform of the nth instance of this tree.
 */
INLINE STTransform SpeedTreeNode::InstanceList::
get_instance(int n) const {
  nassertr(n >= 0 && n < (int)_instances.size(), STTransform::ident_mat());
  return _instances[n];
}

/**
 * Replaces the transform of the nth instance of this tree.
 */
INLINE void SpeedTreeNode::InstanceList::
set_instance(int n, const STTransform &transform) {
  nassertv(n >= 0 && n < (int)_instances.size());
  _instances[n] = transform;
}

/**
 * Adds a new instance of this tree at the indicated transform.  Returns the
 * index number of the new instance.
 */
INLINE int SpeedTreeNode::InstanceList::
add_instance(const STTransform &transform) {
  _instances.push_back(transform);
  return ((int)_instances.size() - 1);
}

/**
 * Removes the nth instance of this tree.
 */
INLINE void SpeedTreeNode::InstanceList::
remove_instance(int n) {
  nassertv(n >= 0 && n < (int)_instances.size());
  _instances.erase(_instances.begin() + n);
}

/**
 *
 */
INLINE SpeedTreeNode::DrawCallback::
DrawCallback(SpeedTreeNode *node) : _node(node) {
}
