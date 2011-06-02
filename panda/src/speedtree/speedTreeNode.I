// Filename: speedTreeNode.I
// Created by:  drose (30Sep10)
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
//     Function: SpeedTreeNode::is_valid
//       Access: Published
//  Description: Returns true if the node is valid and ready to
//               render, false otherwise.  Note that this might not
//               become false until after the first time the node is
//               rendered.
////////////////////////////////////////////////////////////////////
INLINE bool SpeedTreeNode::
is_valid() const {
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::get_num_trees
//       Access: Published
//  Description: Returns the number of unique tree objects that have
//               been added to the node.  This count does not include
//               multiple instances of the same tree that appear in
//               different transforms.
////////////////////////////////////////////////////////////////////
INLINE int SpeedTreeNode::
get_num_trees() const {
  return (int)_trees.size();
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::get_tree
//       Access: Published
//  Description: Returns the STTree pointer for the nth tree.
//               See get_num_trees().
////////////////////////////////////////////////////////////////////
INLINE const STTree *SpeedTreeNode::
get_tree(int n) const {
  nassertr(n >= 0 && n < (int)_trees.size(), NULL);
  InstanceList *instance_list = _trees[n];
  return instance_list->get_tree();
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::get_instance_list
//       Access: Published
//  Description: Returns a list of transforms that corresponds to the
//               instances at which the nth tree appears.
////////////////////////////////////////////////////////////////////
INLINE const SpeedTreeNode::InstanceList &SpeedTreeNode::
get_instance_list(int n) const {
  nassertr(n >= 0 && n < (int)_trees.size(), *(InstanceList *)NULL);
  InstanceList *instance_list = _trees[n];
  return *instance_list;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::modify_tree
//       Access: Published
//  Description: Returns a modifiable STTree pointer for the nth tree
//               instance.
////////////////////////////////////////////////////////////////////
INLINE STTree *SpeedTreeNode::
modify_tree(int n) {
  nassertr(n >= 0 && n < (int)_trees.size(), NULL);
  InstanceList *instance_list = _trees[n];
  _needs_repopulate = true;
  return (STTree *)instance_list->get_tree();
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::clear_terrain
//       Access: Published
//  Description: Removes the terrain associated with the node.
////////////////////////////////////////////////////////////////////
INLINE void SpeedTreeNode::
clear_terrain() {
  set_terrain(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::has_terrain
//       Access: Published
//  Description: Returns true if a valid terrain has been associated
//               with the node, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool SpeedTreeNode::
has_terrain() const {
  return _terrain != (STTerrain *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::get_terrain
//       Access: Published
//  Description: Returns the terrain associated with the node, or NULL
//               if there is no terrain.
////////////////////////////////////////////////////////////////////
INLINE STTerrain *SpeedTreeNode::
get_terrain() const {
  return _terrain;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::set_time_delta
//       Access: Published
//  Description: Specifies an offset that is to be added each frame to
//               the global clock's frame_time for the purpose of
//               animating the trees in this particular node.  Also
//               see set_global_time_delta().
////////////////////////////////////////////////////////////////////
INLINE void SpeedTreeNode::
set_time_delta(double delta) {
  _time_delta = delta;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::get_time_delta
//       Access: Published
//  Description: Returns an offset that is to be added each frame to
//               the global clock's frame_time for the purpose of
//               animating the trees in this particular node.  Also
//               see get_global_time_delta().
////////////////////////////////////////////////////////////////////
INLINE double SpeedTreeNode::
get_time_delta() const {
  return _time_delta;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::set_global_time_delta
//       Access: Published, Static
//  Description: Specifies an offset that is to be added each frame to
//               the global clock's frame_time for the purpose of
//               animating the trees in all SpeedTreeNodes.  Also
//               see set_time_delta().
////////////////////////////////////////////////////////////////////
INLINE void SpeedTreeNode::
set_global_time_delta(double delta) {
  _global_time_delta = delta;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::get_global_time_delta
//       Access: Published, Static
//  Description: Returns an offset that is to be added each frame to
//               the global clock's frame_time for the purpose of
//               animating the trees in all SpeedTreeNodes.  Also
//               see get_time_delta().
////////////////////////////////////////////////////////////////////
INLINE double SpeedTreeNode::
get_global_time_delta() {
  return _global_time_delta;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::InstanceList::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SpeedTreeNode::InstanceList::
InstanceList(const STTree *tree) : _tree((STTree *)tree) {
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::InstanceList::operator <
//       Access: Public
//  Description: Used for comparison for ov_set.
////////////////////////////////////////////////////////////////////
INLINE bool SpeedTreeNode::InstanceList::
operator < (const InstanceList &other) const {
  return _tree < other._tree;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::InstanceList::get_tree
//       Access: Published
//  Description: Returns the particular tree this list refers to.
////////////////////////////////////////////////////////////////////
INLINE const STTree *SpeedTreeNode::InstanceList::
get_tree() const {
  return _tree;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::InstanceList::get_num_instances
//       Access: Published
//  Description: Returns the number of instances of this tree.
////////////////////////////////////////////////////////////////////
INLINE int SpeedTreeNode::InstanceList::
get_num_instances() const {
  return (int)_instances.size();
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::InstanceList::get_instance
//       Access: Published
//  Description: Returns the transform of the nth instance of this
//               tree.
////////////////////////////////////////////////////////////////////
INLINE STTransform SpeedTreeNode::InstanceList::
get_instance(int n) const {
  nassertr(n >= 0 && n < (int)_instances.size(), STTransform::ident_mat());
  return _instances[n];
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::InstanceList::set_instance
//       Access: Published
//  Description: Replaces the transform of the nth instance of this
//               tree.
////////////////////////////////////////////////////////////////////
INLINE void SpeedTreeNode::InstanceList::
set_instance(int n, const STTransform &transform) {
  nassertv(n >= 0 && n < (int)_instances.size());
  _instances[n] = transform;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::InstanceList::add_instance
//       Access: Published
//  Description: Adds a new instance of this tree at the indicated
//               transform.  Returns the index number of the new
//               instance.
////////////////////////////////////////////////////////////////////
INLINE int SpeedTreeNode::InstanceList::
add_instance(const STTransform &transform) {
  _instances.push_back(transform);
  return ((int)_instances.size() - 1);
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::InstanceList::remove_instance
//       Access: Published
//  Description: Removes the nth instance of this tree.
////////////////////////////////////////////////////////////////////
INLINE void SpeedTreeNode::InstanceList::
remove_instance(int n) {
  nassertv(n >= 0 && n < (int)_instances.size());
  _instances.erase(_instances.begin() + n);
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::DrawCallback::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SpeedTreeNode::DrawCallback::
DrawCallback(SpeedTreeNode *node) : _node(node) {
}
