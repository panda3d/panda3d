// Filename: nodePath.I
// Created by:  drose (25Feb02)
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
//     Function: NodePath::Default Constructor
//       Access: Published
//  Description: This constructs an empty NodePath with no nodes.
////////////////////////////////////////////////////////////////////
INLINE NodePath::
NodePath() :
  _error_type(ET_ok)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::Constructor
//       Access: Published
//  Description: This constructs a new NodePath with a single
//               node.  An ordinary, unattached PandaNode is created
//               with the indicated name.
////////////////////////////////////////////////////////////////////
INLINE NodePath::
NodePath(const string &top_node_name) :
  _error_type(ET_ok)
{
  PandaNode *top_node = new PandaNode(top_node_name);
  _head = top_node->get_generic_component(false);
  _backup_key = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::Constructor
//       Access: Published
//  Description: This constructs a NodePath for the indicated node.
//               If the node does not have any parents, this creates a
//               singleton NodePath; otherwise, it automatically finds
//               the path from the node to the root.  If the node has
//               multiple paths to the root, one path is chosen
//               arbitrarily and a warning message is printed (but see
//               also NodePath::any_path(), below).
////////////////////////////////////////////////////////////////////
INLINE NodePath::
NodePath(PandaNode *node) :
  _error_type(ET_ok)
{
  if (node != (PandaNode *)NULL) {
    _head = node->get_generic_component(false);
  }
  _backup_key = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::any_path named constructor
//       Access: Published, Static
//  Description: Returns a new NodePath that represents any arbitrary
//               path from the root to the indicated node.  This is
//               the same thing that would be returned by
//               NodePath(node), except that no warning is issued if
//               the path is ambiguous.
////////////////////////////////////////////////////////////////////
INLINE NodePath NodePath::
any_path(PandaNode *node) {
  NodePath result;
  if (node != (PandaNode *)NULL) {
    result._head = node->get_generic_component(true);
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::Constructor
//       Access: Published
//  Description: Constructs a NodePath with the indicated parent
//               NodePath and child node; the child node must be a
//               stashed or unstashed child of the parent.
////////////////////////////////////////////////////////////////////
INLINE NodePath::
NodePath(const NodePath &parent, PandaNode *child_node) :
  _error_type(ET_fail)
{
  nassertv(!parent.is_empty());
  nassertv(child_node != (PandaNode *)NULL);
  _head = PandaNode::get_component(parent._head, child_node);
  nassertv(_head != (NodePathComponent *)NULL);

  if (_head != (NodePathComponent *)NULL) {
    _error_type = ET_ok;
  }
  _backup_key = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NodePath::
NodePath(const NodePath &copy) :
  _head(copy._head),
  _backup_key(copy._backup_key),
  _error_type(copy._error_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
operator = (const NodePath &copy) {
  _head = copy._head;
  _backup_key = copy._backup_key;
  _error_type = copy._error_type;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::not_found named constructor
//       Access: Published, Static
//  Description: Creates a NodePath with the ET_not_found error type
//               set.
////////////////////////////////////////////////////////////////////
INLINE NodePath NodePath::
not_found() {
  NodePath result;
  result._error_type = ET_not_found;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::removed named constructor
//       Access: Published, Static
//  Description: Creates a NodePath with the ET_removed error type
//               set.
////////////////////////////////////////////////////////////////////
INLINE NodePath NodePath::
removed() {
  NodePath result;
  result._error_type = ET_removed;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::fail named constructor
//       Access: Published, Static
//  Description: Creates a NodePath with the ET_fail error type
//               set.
////////////////////////////////////////////////////////////////////
INLINE NodePath NodePath::
fail() {
  NodePath result;
  result._error_type = ET_fail;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_max_search_depth
//       Access: Published, Static
//  Description: Certain operations, such as find() or
//               find_all_matches(), require a traversal of the scene
//               graph to search for the target node or nodes.  This
//               traversal does not attempt to detect cycles, so an
//               arbitrary cap is set on the depth of the traversal as
//               a poor man's cycle detection, in the event that a
//               cycle has inadvertently been introduced into the
//               scene graph.
//
//               There may be other reasons you'd want to truncate a
//               search before the bottom of the scene graph has been
//               reached.  In any event, this function sets the limit
//               on the number of levels that a traversal will
//               continue, and hence the maximum length of a path that
//               may be returned by a traversal.
//
//               This is a static method, and so changing this
//               parameter affects all of the NodePaths in the
//               universe.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_max_search_depth(int max_search_depth) {
  _max_search_depth = max_search_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_max_search_depth
//       Access: Published, Static
//  Description: Returns the current setting of the search depth
//               limit.  See set_max_search_depth.
////////////////////////////////////////////////////////////////////
INLINE int NodePath::
get_max_search_depth() {
  return _max_search_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::is_empty
//       Access: Published
//  Description: Returns true if the NodePath contains no nodes.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
is_empty() const {
  return (_head == (NodePathComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::is_singleton
//       Access: Published
//  Description: Returns true if the NodePath contains exactly one
//               node.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
is_singleton() const {
  return (_head != (NodePathComponent *)NULL && _head->is_top_node());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_error_type
//       Access: Published
//  Description: If is_empty() is true, this returns a code that
//               represents the reason why the NodePath is empty.
////////////////////////////////////////////////////////////////////
INLINE NodePath::ErrorType NodePath::
get_error_type() const {
  return _error_type;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_top_node
//       Access: Published
//  Description: Returns the top node of the path, or NULL if the path
//               is empty.  This requires iterating through the path.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *NodePath::
get_top_node() const {
  if (is_empty()) {
    return (PandaNode *)NULL;
  }

  return get_top().node();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::node
//       Access: Published
//  Description: Returns the referenced node of the path.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *NodePath::
node() const {
  nassertr_always(!is_empty(), (PandaNode *)NULL);
  return _head->get_node();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_key
//       Access: Published
//  Description: Returns an integer that is guaranteed to be the same
//               for all NodePaths that represent the same node
//               instance, and different for all NodePaths that
//               represent a different node instance.  
//
//               The same key will be returned for a particular
//               instance as long as at least one NodePath exists that
//               represents that instance; if all NodePaths for a
//               particular instance destruct and a new one is later
//               created, it may have a different index.  However, a
//               given key will never be reused for a different
//               instance (unless the app has been running long enough
//               that we overflow the integer key value).
////////////////////////////////////////////////////////////////////
INLINE int NodePath::
get_key() const {
  if (is_empty()) {
    return _backup_key;
  }
  return _head->get_key();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::is_same_graph
//       Access: Published
//  Description: Returns true if the node represented by this NodePath
//               is parented within the same graph as that of the
//               other NodePath.  This is essentially the same thing
//               as asking whether get_top() of both NodePaths is the
//               same (e.g., both "render").
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
is_same_graph(const NodePath &other) const {
  // Actually, it's possible for the top nodes to be the same, but the
  // NodePaths still to be considered in different graphs.  But even
  // in this case, get_top() will be different for each one.  (They'll
  // be different singleton NodePaths that happen to reference the
  // same node).

  // This will happen if one of the top nodes is considered a
  // different instance--for instance, render.instance_to(NodePath())
  // returns a different instance of render that appears to have the
  // same top node.  But this is a very rare thing to do.
  int a_count, b_count;
  return (find_common_ancestor(*this, other, a_count, b_count) != (NodePathComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_num_children
//       Access: Published
//  Description: Returns the number of children of the referenced node.
////////////////////////////////////////////////////////////////////
INLINE int NodePath::
get_num_children() const {
  nassertr_always(!is_empty(), 0);
  return _head->get_node()->get_num_children();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_child
//       Access: Published
//  Description: Returns a NodePath representing the nth child of the
//               referenced node.
////////////////////////////////////////////////////////////////////
INLINE NodePath NodePath::
get_child(int n) const {
  nassertr(n >= 0 && n < get_num_children(), NodePath());
  NodePath child;
  child._head = PandaNode::get_component(_head, _head->get_node()->get_child(n));
  return child;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_parent
//       Access: Published
//  Description: Returns true if the referenced node has a parent;
//               i.e. the NodePath chain contains at least two nodes.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_parent() const {
  return !is_empty() && !is_singleton();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_parent
//       Access: Published
//  Description: Returns the NodePath to the parent of the referenced
//               node: that is, this NodePath, shortened by one node.
//               The parent of a singleton NodePath is defined to be
//               the empty NodePath.
////////////////////////////////////////////////////////////////////
INLINE NodePath NodePath::
get_parent() const {
  if (!has_parent()) {
    return NodePath();
  }
  NodePath parent;
  parent._head = _head->get_next();
  return parent;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::attach_new_node
//       Access: Published
//  Description: Creates an ordinary PandaNode and attaches it below
//               the current NodePath, returning a new NodePath that
//               references it.
////////////////////////////////////////////////////////////////////
INLINE NodePath NodePath::
attach_new_node(const string &name, int sort) const {
  nassertr(verify_complete(), NodePath::fail());

  return attach_new_node(new PandaNode(name), sort);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::ls
//       Access: Published
//  Description: Lists the hierarchy at and below the referenced node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
ls() const {
  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::ls
//       Access: Published
//  Description: Lists the hierarchy at and below the referenced node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
ls(ostream &out, int indent_level) const {
  if (is_empty()) {
    out << "(empty)\n";
  } else {
    node()->ls(out, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::reverse_ls
//       Access: Published
//  Description: Lists the hierarchy at and above the referenced node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
reverse_ls() const {
  reverse_ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::reverse_ls
//       Access: Published
//  Description: Lists the hierarchy at and above the referenced node.
////////////////////////////////////////////////////////////////////
INLINE int NodePath::
reverse_ls(ostream &out, int indent_level) const {
  if (is_empty()) {
    out << "(empty)\n";
    return 0;
  } else if (has_parent()) {
    indent_level = get_parent().reverse_ls(out, indent_level);
  }
  node()->write(out, indent_level);
  return indent_level + 2;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_state
//       Access: Published
//  Description: Returns the complete state object set on this node.
////////////////////////////////////////////////////////////////////
INLINE const RenderState *NodePath::
get_state() const {
  nassertr_always(!is_empty(), RenderState::make_empty());
  return node()->get_state();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_state
//       Access: Published
//  Description: Changes the complete state object on this node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_state(const RenderState *state) {
  nassertv_always(!is_empty());
  node()->set_state(state);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_net_state
//       Access: Published
//  Description: Returns the net state on this node from the root.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderState) NodePath::
get_net_state() const {
  nassertr(_error_type == ET_ok, RenderState::make_empty());
  return r_get_net_state(_head);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_transform
//       Access: Published
//  Description: Returns the complete transform object set on this node.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *NodePath::
get_transform() const {
  nassertr_always(!is_empty(), TransformState::make_identity());
  return node()->get_transform();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_transform
//       Access: Published
//  Description: Sets the transform object on this node to identity.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_transform() {
  set_transform(TransformState::make_identity());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_transform
//       Access: Published
//  Description: Changes the complete transform object on this node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_transform(const TransformState *transform) {
  nassertv_always(!is_empty());
  node()->set_transform(transform);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_transform
//       Access: Published
//  Description: Sets the transform object on this node to identity,
//               relative to the other node.  This effectively places
//               this node at the same position as the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_transform(const NodePath &other) {
  set_transform(other, TransformState::make_identity());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_net_transform
//       Access: Published
//  Description: Returns the net transform on this node from the root.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) NodePath::
get_net_transform() const {
  nassertr(_error_type == ET_ok, TransformState::make_identity());
  return r_get_net_transform(_head);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_prev_transform
//       Access: Published
//  Description: Returns the transform that has been set as this
//               node's "previous" position.  See
//               set_prev_transform().
////////////////////////////////////////////////////////////////////
INLINE const TransformState *NodePath::
get_prev_transform() const {
  nassertr_always(!is_empty(), TransformState::make_identity());
  return node()->get_prev_transform();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_prev_transform
//       Access: Published
//  Description: Sets the transform that represents this node's
//               "previous" position, one frame ago, for the purposes
//               of detecting motion for accurate collision
//               calculations.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_prev_transform(const TransformState *transform) {
  nassertv_always(!is_empty());
  node()->set_prev_transform(transform);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_net_prev_transform
//       Access: Published
//  Description: Returns the net "previous" transform on this node
//               from the root.  See set_prev_transform().
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) NodePath::
get_net_prev_transform() const {
  nassertr(_error_type == ET_ok, TransformState::make_identity());
  return r_get_net_prev_transform(_head);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos
//       Access: Published
//  Description: Sets the translation component of the transform,
//               leaving rotation and scale untouched.  This also
//               resets the node's "previous" position, so that the
//               collision system will see the node as having suddenly
//               appeared in the new position, without passing any
//               points in between.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_pos(float x, float y, float z) {
  set_pos(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_fluid_pos
//       Access: Published
//  Description: Sets the translation component, without changing the
//               "previous" position, so that the collision system
//               will see the node as moving fluidly from its previous
//               position to its new position.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_fluid_pos(float x, float y, float z) {
  set_fluid_pos(LPoint3f(x, y, z));
}

INLINE float NodePath::
get_x() const {
  return get_pos()[0];
}

INLINE float NodePath::
get_y() const {
  return get_pos()[1];
}

INLINE float NodePath::
get_z() const {
  return get_pos()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_hpr
//       Access: Published
//  Description: Sets the rotation component of the transform,
//               leaving translation and scale untouched.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_hpr(float h, float p, float r) {
  set_hpr(LVecBase3f(h, p, r));
}

INLINE float NodePath::
get_h() const {
  return get_hpr()[0];
}

INLINE float NodePath::
get_p() const {
  return get_hpr()[1];
}

INLINE float NodePath::
get_r() const {
  return get_hpr()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_scale
//       Access: Published
//  Description: Sets the scale component of the transform,
//               leaving translation and rotation untouched.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_scale(float scale) {
  set_scale(LVecBase3f(scale, scale, scale));
}

INLINE void NodePath::
set_scale(float sx, float sy, float sz) {
  set_scale(LVecBase3f(sx, sy, sz));
}

INLINE float NodePath::
get_sx() const {
  return get_scale()[0];
}

INLINE float NodePath::
get_sy() const {
  return get_scale()[1];
}

INLINE float NodePath::
get_sz() const {
  return get_scale()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_shear
//       Access: Published
//  Description: Sets the shear component of the transform,
//               leaving translation, rotation, and scale untouched.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_shear(float shxy, float shxz, float shyz) {
  set_shear(LVecBase3f(shxy, shxz, shyz));
}

INLINE float NodePath::
get_shxy() const {
  return get_shear()[0];
}

INLINE float NodePath::
get_shxz() const {
  return get_shear()[1];
}

INLINE float NodePath::
get_shyz() const {
  return get_shear()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr
//       Access: Published
//  Description: Sets the translation and rotation component of the
//               transform, leaving scale untouched.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_pos_hpr(float x, float y, float z, float h, float p, float r) {
  set_pos_hpr(LVecBase3f(x, y, z), LVecBase3f(h, p, r));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_hpr_scale
//       Access: Published
//  Description: Sets the rotation and scale components of the
//               transform, leaving translation untouched.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_hpr_scale(float h, float p, float r, float sx, float sy, float sz) {
  set_hpr_scale(LVecBase3f(h, p, r), LVecBase3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr_scale
//       Access: Published
//  Description: Completely replaces the transform with new
//               translation, rotation, and scale components.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_pos_hpr_scale(float x, float y, float z, float h, float p, float r,
                  float sx, float sy, float sz) {
  set_pos_hpr_scale(LVecBase3f(x, y, z), LVecBase3f(h, p, r),
                    LVecBase3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_mat
//       Access: Published
//  Description: Completely removes any transform from the referenced
//               node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_mat() {
  nassertv_always(!is_empty());
  node()->clear_transform();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_mat
//       Access: Published
//  Description: Returns true if a non-identity transform matrix has
//               been applied to the referenced node, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_mat() const {
  nassertr_always(!is_empty(), false);
  return !node()->get_transform()->is_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_mat
//       Access: Published
//  Description: Returns the transform matrix that has been applied to
//               the referenced node, or the identity matrix if no
//               matrix has been applied.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &NodePath::
get_mat() const {
  nassertr_always(!is_empty(), LMatrix4f::ident_mat());

  return node()->get_transform()->get_mat();
}


////////////////////////////////////////////////////////////////////
//     Function: NodePath::look_at
//       Access: Published
//  Description: Sets the transform on this NodePath so that it
//               rotates to face the indicated point in space.  This
//               will overwrite any previously existing scale on the
//               node, although it will preserve any translation.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
look_at(float x, float y, float z) {
  look_at(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::heads_up
//       Access: Published
//  Description: Behaves like look_at(), but with a strong preference
//               to keeping the up vector oriented in the indicated
//               "up" direction.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
heads_up(float x, float y, float z) {
  heads_up(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos
//       Access: Published
//  Description: Sets the translation component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_pos(const NodePath &other, float x, float y, float z) {
  set_pos(other, LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_fluid_pos
//       Access: Published
//  Description: Sets the translation component, without changing the
//               "previous" position, so that the collision system
//               will see the node as moving fluidly from its previous
//               position to its new position.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_fluid_pos(const NodePath &other, float x, float y, float z) {
  set_fluid_pos(other, LPoint3f(x, y, z));
}

INLINE float NodePath::
get_x(const NodePath &other) const {
  return get_pos(other)[0];
}

INLINE float NodePath::
get_y(const NodePath &other) const {
  return get_pos(other)[1];
}

INLINE float NodePath::
get_z(const NodePath &other) const {
  return get_pos(other)[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_hpr
//       Access: Published
//  Description: Sets the rotation component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_hpr(const NodePath &other, float h, float p, float r) {
  set_hpr(other, LPoint3f(h, p, r));
}

INLINE float NodePath::
get_h(const NodePath &other) const {
  return get_hpr(other)[0];
}

INLINE float NodePath::
get_p(const NodePath &other) const {
  return get_hpr(other)[1];
}

INLINE float NodePath::
get_r(const NodePath &other) const {
  return get_hpr(other)[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_scale
//       Access: Published
//  Description: Sets the scale component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_scale(const NodePath &other, float scale) {
  set_scale(other, LPoint3f(scale, scale, scale));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_scale
//       Access: Published
//  Description: Sets the scale component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_scale(const NodePath &other, float sx, float sy, float sz) {
  set_scale(other, LPoint3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_scale
//       Access: Published
//  Description: Returns the relative scale of the referenced node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_sx(const NodePath &other) const {
  return get_scale(other)[0];
}

INLINE float NodePath::
get_sy(const NodePath &other) const {
  return get_scale(other)[1];
}

INLINE float NodePath::
get_sz(const NodePath &other) const {
  return get_scale(other)[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_shear
//       Access: Published
//  Description: Sets the shear component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_shear(const NodePath &other, float shxy, float shxz, float shyz) {
  set_shear(other, LPoint3f(shxy, shxz, shyz));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_shear
//       Access: Published
//  Description: Returns the relative shear of the referenced node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_shxy(const NodePath &other) const {
  return get_shear(other)[0];
}

INLINE float NodePath::
get_shxz(const NodePath &other) const {
  return get_shear(other)[1];
}

INLINE float NodePath::
get_shyz(const NodePath &other) const {
  return get_shear(other)[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr
//       Access: Published
//  Description: Sets the translation and rotation component of the
//               transform, relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_pos_hpr(const NodePath &other,
            float x, float y, float z,
            float h, float p, float r) {
  set_pos_hpr(other, LVecBase3f(x, y, z), LVecBase3f(h, p, r));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_hpr_scale
//       Access: Published
//  Description: Sets the rotation and scale components of the
//               transform, leaving translation untouched.  This, or
//               set_pos_hpr_scale, is the preferred way to update a
//               transform when both hpr and scale are to be changed.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_hpr_scale(const NodePath &other,
        float h, float p, float r, float sx, float sy, float sz) {
  set_hpr_scale(other, LVecBase3f(h, p, r), LVecBase3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr_scale
//       Access: Published
//  Description: Completely replaces the transform with new
//               translation, rotation, and scale components, relative
//               to the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_pos_hpr_scale(const NodePath &other,
                  float x, float y, float z,
                  float h, float p, float r,
                  float sx, float sy, float sz) {
  set_pos_hpr_scale(other, LVecBase3f(x, y, z), LVecBase3f(h, p, r),
                    LVecBase3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::look_at
//       Access: Published
//  Description: Sets the hpr on this NodePath so that it rotates to
//               face the indicated point in space, which is relative
//               to the other NodePath.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
look_at(const NodePath &other, float x, float y, float z) {
  look_at(other, LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::heads_up
//       Access: Published
//  Description: Behaves like look_at(), but with a strong preference
//               to keeping the up vector oriented in the indicated
//               "up" direction.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
heads_up(const NodePath &other, float x, float y, float z) {
  heads_up(other, LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_distance
//       Access: Published
//  Description: Returns the straight-line distance between this
//               referenced node's coordinate frame's origin, and that
//               of the other node's origin.
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_distance(const NodePath &other) const {
  LPoint3f pos = get_pos(other);
  return length(LVector3f(pos));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_color_scale
//       Access: Published
//  Description: Sets the color scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_color_scale(float sr, float sg, float sb, float sa, int priority) {
  set_color_scale(LVecBase4f(sr, sg, sb, sa), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_sr
//       Access: Published
//  Description: Sets the red scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_sr(float sr) {
  LVecBase4f new_scale = get_color_scale();
  new_scale[0] = sr;

  set_color_scale(new_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_sg
//       Access: Published
//  Description: Sets the alpha scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_sg(float sg) {
  LVecBase4f new_scale = get_color_scale();
  new_scale[1] = sg;

  set_color_scale(new_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_sb
//       Access: Published
//  Description: Sets the blue scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_sb(float sb) {
  LVecBase4f new_scale = get_color_scale();
  new_scale[2] = sb;

  set_color_scale(new_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_sa
//       Access: Published
//  Description: Sets the alpha scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_sa(float sa) {
  LVecBase4f new_scale = get_color_scale();
  new_scale[3] = sa;

  set_color_scale(new_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_sr
//       Access: Published
//  Description: Gets the red scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_sr() const {
  return get_color_scale()[0];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_sg
//       Access: Published
//  Description: Gets the green scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_sg() const {
  return get_color_scale()[1];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_sb
//       Access: Published
//  Description: Gets the blue scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_sb() const {
  return get_color_scale()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_sa
//       Access: Published
//  Description: Gets the alpha scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_sa() const {
  return get_color_scale()[3];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_tex_offset
//       Access: Published
//  Description: Sets a texture matrix on the current node to apply
//               the indicated offset to UV's for the given stage.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_tex_offset(TextureStage *stage, const LVecBase2f &uv) {
  set_tex_offset(stage, uv[0], uv[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_tex_offset
//       Access: Published
//  Description: Sets a texture matrix on the current node to apply
//               the indicated offset to UV's for the given stage.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_tex_offset(TextureStage *stage, float u, float v) {
  nassertv_always(!is_empty());
  set_tex_transform(stage, 
                    get_tex_transform(stage)->set_pos(LVecBase3f(u, v, 0.0f)));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_tex_rotate
//       Access: Published
//  Description: Sets a texture matrix on the current node to apply
//               the indicated rotation, clockwise in degrees, to UV's
//               for the given stage.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_tex_rotate(TextureStage *stage, float r) {
  nassertv_always(!is_empty());
  set_tex_transform(stage, 
                    get_tex_transform(stage)->set_hpr(LVecBase3f(r, 0.0f, 0.0f)));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_tex_scale
//       Access: Published
//  Description: Sets a texture matrix on the current node to apply
//               the indicated scale to UV's for the given stage.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_tex_scale(TextureStage *stage, const LVecBase2f &scale) {
  set_tex_scale(stage, scale[0], scale[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_tex_scale
//       Access: Published
//  Description: Sets a texture matrix on the current node to apply
//               the indicated offset to UV's for the given stage.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_tex_scale(TextureStage *stage, float su, float sv) {
  nassertv_always(!is_empty());
  set_tex_transform(stage, 
                    get_tex_transform(stage)->set_scale(LVecBase3f(su, sv, 1.0f)));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_tex_offset
//       Access: Published
//  Description: Returns the offset set for the UV's for the given
//               stage on the current node.
////////////////////////////////////////////////////////////////////
INLINE LVecBase2f NodePath::
get_tex_offset(TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase2f::zero());
  const LVecBase3f &pos = get_tex_transform(stage)->get_pos();
  return LVecBase2f(pos[0], pos[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_tex_rotate
//       Access: Published
//  Description: Returns the rotation set for the UV's for the given
//               stage on the current node.
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_tex_rotate(TextureStage *stage) const {
  nassertr_always(!is_empty(), 0.0f);
  return get_tex_transform(stage)->get_hpr()[0];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_tex_scale
//       Access: Published
//  Description: Returns the scale set for the UV's for the given
//               stage on the current node.
////////////////////////////////////////////////////////////////////
INLINE LVecBase2f NodePath::
get_tex_scale(TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase2f(1.0f, 1.0f));
  const LVecBase3f &scale = get_tex_transform(stage)->get_scale();
  return LVecBase2f(scale[0], scale[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_tex_offset
//       Access: Published
//  Description: Sets a texture matrix on the current node to apply
//               the indicated offset to UV's for the given stage.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_tex_offset(const NodePath &other, TextureStage *stage, const LVecBase2f &uv) {
  set_tex_offset(other, stage, uv[0], uv[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_tex_offset
//       Access: Published
//  Description: Sets a texture matrix on the current node to apply
//               the indicated offset to UV's for the given stage.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_tex_offset(const NodePath &other, TextureStage *stage, float u, float v) {
  nassertv_always(!is_empty());
  set_tex_transform(other, stage, 
                    get_tex_transform(other, stage)->set_pos(LVecBase3f(u, v, 0.0f)));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_tex_rotate
//       Access: Published
//  Description: Sets a texture matrix on the current node to apply
//               the indicated rotation, clockwise in degrees, to UV's
//               for the given stage.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_tex_rotate(const NodePath &other, TextureStage *stage, float r) {
  nassertv_always(!is_empty());
  set_tex_transform(other, stage, 
                    get_tex_transform(other, stage)->set_hpr(LVecBase3f(r, 0.0f, 0.0f)));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_tex_scale
//       Access: Published
//  Description: Sets a texture matrix on the current node to apply
//               the indicated scale to UV's for the given stage.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_tex_scale(const NodePath &other, TextureStage *stage, const LVecBase2f &scale) {
  set_tex_scale(other, stage, scale[0], scale[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_tex_scale
//       Access: Published
//  Description: Sets a texture matrix on the current node to apply
//               the indicated offset to UV's for the given stage.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_tex_scale(const NodePath &other, TextureStage *stage, float su, float sv) {
  nassertv_always(!is_empty());
  set_tex_transform(other, stage, 
                    get_tex_transform(stage)->set_scale(LVecBase3f(su, sv, 1.0f)));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_tex_offset
//       Access: Published
//  Description: Returns the offset set for the UV's for the given
//               stage on the current node.
////////////////////////////////////////////////////////////////////
INLINE LVecBase2f NodePath::
get_tex_offset(const NodePath &other, TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase2f::zero());
  const LVecBase3f &pos = get_tex_transform(other, stage)->get_pos();
  return LVecBase2f(pos[0], pos[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_tex_rotate
//       Access: Published
//  Description: Returns the rotation set for the UV's for the given
//               stage on the current node.
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_tex_rotate(const NodePath &other, TextureStage *stage) const {
  nassertr_always(!is_empty(), 0.0f);
  return get_tex_transform(other, stage)->get_hpr()[0];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_tex_scale
//       Access: Published
//  Description: Returns the scale set for the UV's for the given
//               stage on the current node.
////////////////////////////////////////////////////////////////////
INLINE LVecBase2f NodePath::
get_tex_scale(const NodePath &other, TextureStage *stage) const {
  nassertr_always(!is_empty(), LVecBase2f(1.0f, 1.0f));
  const LVecBase3f &scale = get_tex_transform(other, stage)->get_scale();
  return LVecBase2f(scale[0], scale[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_project_texture
//       Access: Published
//  Description: Undoes the effect of project_texture().
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_project_texture(TextureStage *stage) {
  clear_texture(stage);
  clear_tex_gen(stage);
  clear_tex_projector(stage);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_billboard_axis
//       Access: Published
//  Description: Puts a billboard transition on the node such that it
//               will rotate in two dimensions around the up axis.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_billboard_axis(float offset) {
  set_billboard_axis(NodePath(), offset);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_billboard_point_eye
//       Access: Published
//  Description: Puts a billboard transition on the node such that it
//               will rotate in three dimensions about the origin,
//               keeping its up vector oriented to the top of the
//               camera.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_billboard_point_eye(float offset) {
  set_billboard_point_eye(NodePath(), offset);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_billboard_point_world
//       Access: Published
//  Description: Puts a billboard transition on the node such that it
//               will rotate in three dimensions about the origin,
//               keeping its up vector oriented to the sky.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_billboard_point_world(float offset) {
  set_billboard_point_world(NodePath(), offset);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::adjust_all_priorities
//       Access: Published
//  Description: Adds the indicated adjustment amount (which may be
//               negative) to the priority for all transitions on the
//               referenced node, and for all nodes in the subgraph
//               below.  This can be used to force these nodes not to
//               be overridden by a high-level state change above.  If
//               the priority would drop below zero, it is set to
//               zero.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
adjust_all_priorities(int adjustment) {
  nassertv_always(!is_empty());
  r_adjust_all_priorities(node(), adjustment);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::show
//       Access: Published
//  Description: Undoes the effect of a previous hide() on this node:
//               makes the referenced node (and the entire subgraph
//               below this node) visible to all cameras.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
show() {
  nassertv_always(!is_empty());
  node()->set_draw_mask(DrawMask::all_on());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::show
//       Access: Published
//  Description: Makes the referenced node visible just to the
//               cameras whose camera_mask shares the indicated bits.
//               That is, this sets the indicated bits in the
//               node's draw mask.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
show(DrawMask camera_mask) {
  nassertv_always(!is_empty());
  node()->set_draw_mask(node()->get_draw_mask() | camera_mask);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::hide
//       Access: Published
//  Description: Makes the referenced node (and the entire subgraph
//               below this node) invisible to all cameras.  It
//               remains part of the scene graph, its bounding volume
//               still contributes to its parent's bounding volume,
//               and it will still be involved in collision tests.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
hide() {
  nassertv_always(!is_empty());
  node()->set_draw_mask(DrawMask::all_off());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::hide
//       Access: Published
//  Description: Makes the referenced node invisible just to the
//               cameras whose camera_mask shares the indicated bits.
//               That is, this clears the indicated bits from the
//               node's draw mask.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
hide(DrawMask camera_mask) {
  nassertv_always(!is_empty());
  node()->set_draw_mask(node()->get_draw_mask() & ~camera_mask);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::is_hidden
//       Access: Published
//  Description: Returns true if the referenced node is hidden from
//               the indicated camera(s) either directly, or because
//               some ancestor is hidden.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
is_hidden(DrawMask camera_mask) const {
  return !get_hidden_ancestor(camera_mask).is_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::is_stashed
//       Access: Published
//  Description: Returns true if the referenced node is stashed either
//               directly, or because some ancestor is stashed.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
is_stashed() const {
  return !get_stashed_ancestor().is_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_collide_mask
//       Access: Published
//  Description: Returns the union of all of the into_collide_masks
//               for nodes at this level and below.  This is the same
//               thing as node()->get_net_collide_mask().
//
//               If you want to return what the into_collide_mask of
//               this node itself is, without regard to its children,
//               use node()->get_into_collide_mask().
////////////////////////////////////////////////////////////////////
INLINE CollideMask NodePath::
get_collide_mask() const {
  nassertr_always(!is_empty(), CollideMask::all_off());
  return node()->get_net_collide_mask();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_collide_mask
//       Access: Published
//  Description: Recursively applies the indicated CollideMask to the
//               into_collide_masks for all nodes at this level and
//               below.  Only nodes 
//
//               The default is to change all bits, but if
//               bits_to_change is not all bits on, then only the bits
//               that are set in bits_to_change are modified, allowing
//               this call to change only a subset of the bits in the
//               subgraph.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: NodePath::operator ==
//       Access: Published
//  Description: Returns true if the two paths are equivalent; that
//               is, if they contain the same list of nodes in the same
//               order.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
operator == (const NodePath &other) const {
  return _head == other._head;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::operator !=
//       Access: Published
//  Description: Returns true if the two paths are not equivalent.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
operator != (const NodePath &other) const {
  return _head != other._head;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::operator <
//       Access: Published
//  Description: Returns true if this NodePath sorts before the other
//               one, false otherwise.  The sorting order of two
//               nonequivalent NodePaths is consistent but undefined,
//               and is useful only for storing NodePaths in a sorted
//               container like an STL set.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
operator < (const NodePath &other) const {
  return _head < other._head;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::compare_to
//       Access: Published
//  Description: Returns a number less than zero if this NodePath
//               sorts before the other one, greater than zero if it
//               sorts after, or zero if they are equivalent.
//
//               Two NodePaths are considered equivalent if they
//               consist of exactly the same list of nodes in the same
//               order.  Otherwise, they are different; different
//               NodePaths will be ranked in a consistent but
//               undefined ordering; the ordering is useful only for
//               placing the NodePaths in a sorted container like an
//               STL set.
////////////////////////////////////////////////////////////////////
INLINE int NodePath::
compare_to(const NodePath &other) const {
  // Nowadays, the NodePathComponents at the head are pointerwise
  // equivalent if and only if the NodePaths are equivalent.  So we
  // only have to compare pointers.
  if (_head != other._head) {
    return _head < other._head ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_tag
//       Access: Published
//  Description: Associates a user-defined value with a user-defined
//               key which is stored on the node.  This value has no
//               meaning to Panda; but it is stored indefinitely on
//               the node until it is requested again.
//
//               Each unique key stores a different string value.
//               There is no effective limit on the number of
//               different keys that may be stored or on the length of
//               any one key's value.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_tag(const string &key, const string &value) {
  nassertv_always(!is_empty());
  node()->set_tag(key, value);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_tag
//       Access: Published
//  Description: Retrieves the user-defined value that was previously
//               set on this node for the particular key, if any.  If
//               no value has been previously set, returns the empty
//               string.  See also get_net_tag().
////////////////////////////////////////////////////////////////////
INLINE string NodePath::
get_tag(const string &key) const {
  // An empty NodePath quietly returns no tags.  This makes
  // get_net_tag() easier to implement.
  if (is_empty()) {
    return string();
  }
  return node()->get_tag(key);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_tag
//       Access: Published
//  Description: Returns true if a value has been defined on this node
//               for the particular key (even if that value is the
//               empty string), or false if no value has been set.
//               See also has_net_tag().
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_tag(const string &key) const {
  // An empty NodePath quietly has no tags.  This makes has_net_tag()
  // easier to implement.
  if (is_empty()) {
    return false;
  }
  return node()->has_tag(key);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_tag
//       Access: Published
//  Description: Removes the value defined for this key on this
//               particular node.  After a call to clear_tag(),
//               has_tag() will return false for the indicated key.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_tag(const string &key) {
  nassertv_always(!is_empty());
  node()->clear_tag(key);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_net_tag
//       Access: Published
//  Description: Returns the tag value that has been defined on this
//               node, or the nearest ancestor node, for the indicated
//               key.  If no value has been defined for the indicated
//               key on any ancestor node, returns the empty string.
//               See also get_tag().
////////////////////////////////////////////////////////////////////
INLINE string NodePath::
get_net_tag(const string &key) const {
  return find_net_tag(key).get_tag(key);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_net_tag
//       Access: Published
//  Description: Returns true if the indicated tag value has been
//               defined on this node or on any ancestor node, or
//               false otherwise.  See also has_tag().
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_net_tag(const string &key) const {
  return find_net_tag(key).has_tag(key);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_name
//       Access: Published
//  Description: Changes the name of the referenced node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_name(const string &name) {
  nassertv_always(!is_empty());
  node()->set_name(name);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_name
//       Access: Published
//  Description: Returns the name of the referenced node.
////////////////////////////////////////////////////////////////////
INLINE string NodePath::
get_name() const {
  nassertr_always(!is_empty(), string());
  return node()->get_name();
}


INLINE ostream &operator << (ostream &out, const NodePath &node_path) {
  node_path.output(out);
  return out;
}

