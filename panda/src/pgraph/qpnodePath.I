// Filename: qpnodePath.I
// Created by:  drose (25Feb02)
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
//     Function: qpNodePath::Default Constructor
//       Access: Published
//  Description: This constructs an empty qpNodePath with no nodes.  It
//               cannot be extended, since you cannot add nodes without
//               first specifying the top node.  Use the constructor
//               that receives a node if you ever want to do anything
//               with this path.
////////////////////////////////////////////////////////////////////
INLINE qpNodePath::
qpNodePath() :
  _error_type(ET_ok)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::Constructor
//       Access: Published
//  Description: This constructs an empty qpNodePath with a single
//               node.  An ordinary PandaNode is created with the
//               indicated name.
////////////////////////////////////////////////////////////////////
INLINE qpNodePath::
qpNodePath(const string &top_node_name) :
  _error_type(ET_ok)
{
  PandaNode *top_node = new PandaNode(top_node_name);
  _head = top_node->get_generic_component();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::Constructor
//       Access: Published
//  Description: This constructs a NodePath for the indicated node.
//               If the node does not have any parents, this creates a
//               single NodePath; otherwise, it automatically finds
//               the path from the node to the root.  If the node has
//               multiple paths to the root, one path is chosen
//               arbitrarily and a warning message is printed.
////////////////////////////////////////////////////////////////////
INLINE qpNodePath::
qpNodePath(PandaNode *top_node) :
  _error_type(ET_ok)
{
  if (top_node != (PandaNode *)NULL) {
    _head = top_node->get_generic_component();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::Constructor
//       Access: Published
//  Description: Constructs a NodePath with the indicated parent
//               NodePath and child node; the child node must be a

//               stashed or unstashed child of the parent.
////////////////////////////////////////////////////////////////////
INLINE qpNodePath::
qpNodePath(const qpNodePath &parent, PandaNode *child) :
  _error_type(ET_fail)
{
  nassertv(!parent.is_empty());
  nassertv(child != (PandaNode *)NULL);
  _head = PandaNode::get_component(parent._head, child);
  nassertv(_head != (qpNodePathComponent *)NULL);

  if (_head != (qpNodePathComponent *)NULL) {
    _error_type = ET_ok;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpNodePath::
qpNodePath(const qpNodePath &copy) :
  _head(copy._head),
  _error_type(copy._error_type)
{
  uncollapse_head();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
operator = (const qpNodePath &copy) {
  _head = copy._head;
  _error_type = copy._error_type;
  uncollapse_head();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::not_found named constructor
//       Access: Published, Static
//  Description: Creates a qpNodePath with the ET_not_found error type
//               set.
////////////////////////////////////////////////////////////////////
INLINE qpNodePath qpNodePath::
not_found() {
  qpNodePath result;
  result._error_type = ET_not_found;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::removed named constructor
//       Access: Published, Static
//  Description: Creates a qpNodePath with the ET_removed error type
//               set.
////////////////////////////////////////////////////////////////////
INLINE qpNodePath qpNodePath::
removed() {
  qpNodePath result;
  result._error_type = ET_removed;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::fail named constructor
//       Access: Published, Static
//  Description: Creates a qpNodePath with the ET_fail error type
//               set.
////////////////////////////////////////////////////////////////////
INLINE qpNodePath qpNodePath::
fail() {
  qpNodePath result;
  result._error_type = ET_fail;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_max_search_depth
//       Access: Published, Static
//  Description: Certain operations, such as extend_down_to() or
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
INLINE void qpNodePath::
set_max_search_depth(int max_search_depth) {
  _max_search_depth = max_search_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_max_search_depth
//       Access: Published, Static
//  Description: Returns the current setting of the search depth
//               limit.  See set_max_search_depth.
////////////////////////////////////////////////////////////////////
INLINE int qpNodePath::
get_max_search_depth() {
  return _max_search_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::is_empty
//       Access: Published
//  Description: Returns true if the qpNodePath contains no nodes.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePath::
is_empty() const {
  uncollapse_head();
  return (_head == (qpNodePathComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::is_singleton
//       Access: Published
//  Description: Returns true if the qpNodePath contains exactly one
//               node.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePath::
is_singleton() const {
  uncollapse_head();
  return (_head != (qpNodePathComponent *)NULL && _head->is_top_node());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_error_type
//       Access: Published
//  Description: If is_empty() is true, this returns a code that
//               represents the reason why the qpNodePath is empty.
////////////////////////////////////////////////////////////////////
INLINE qpNodePath::ErrorType qpNodePath::
get_error_type() const {
  return _error_type;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::node
//       Access: Published
//  Description: Returns the referenced node of the path, or NULL if the
//               path is empty.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *qpNodePath::
node() const {
  if (is_empty()) {
    return (PandaNode *)NULL;
  }
  return _head->get_node();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_key
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
INLINE int qpNodePath::
get_key() const {
  if (is_empty()) {
    return 0;
  }
  return _head->get_key();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_num_children
//       Access: Published
//  Description: Returns the number of children of the referenced node.
////////////////////////////////////////////////////////////////////
INLINE int qpNodePath::
get_num_children() const {
  nassertr_always(!is_empty(), 0);
  return _head->get_node()->get_num_children();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_child
//       Access: Published
//  Description: Returns a qpNodePath representing the nth child of the
//               referenced node.
////////////////////////////////////////////////////////////////////
INLINE qpNodePath qpNodePath::
get_child(int n) const {
  nassertr(n >= 0 && n < get_num_children(), qpNodePath());
  qpNodePath child;
  child._head = PandaNode::get_component(_head, _head->get_node()->get_child(n));
  return child;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_parent
//       Access: Published
//  Description: Returns true if the referenced node has a parent;
//               i.e. the NodePath chain contains at least two nodes.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePath::
has_parent() const {
  return !is_empty() && !is_singleton();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_parent
//       Access: Published
//  Description: Returns the qpNodePath to the parent of the referenced
//               node: that is, this qpNodePath, shortened by one node.
////////////////////////////////////////////////////////////////////
INLINE qpNodePath qpNodePath::
get_parent() const {
  nassertr(has_parent(), qpNodePath::fail());
  qpNodePath parent;
  parent._head = _head->get_next();
  return parent;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::attach_new_node
//       Access: Published
//  Description: Creates an ordinary PandaNode and attaches it below
//               the current qpNodePath, returning a new qpNodePath that
//               references it.
////////////////////////////////////////////////////////////////////
INLINE qpNodePath qpNodePath::
attach_new_node(const string &name, int sort) const {
  nassertr(verify_complete(), qpNodePath::fail());
  nassertr_always(!is_empty(), *this);

  return attach_new_node(new PandaNode(name), sort);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::ls
//       Access: Published
//  Description: Lists the hierarchy at and below the referenced node.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
ls() const {
  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::ls
//       Access: Published
//  Description: Lists the hierarchy at and below the referenced node.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
ls(ostream &out, int indent_level) const {
  if (is_empty()) {
    out << "(empty)\n";
  } else {
    node()->ls(out, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::ls_transitions
//       Access: Published
//  Description: Lists the hierarchy at and below the referenced node,
//               along with the state transitions at each level.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
ls_transitions() const {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::ls_transforms
//       Access: Published
//  Description: Lists the hierarchy at and below the referenced node,
//               along with the transforms at each level.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
ls_transforms() const {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_state
//       Access: Published
//  Description: Returns the complete state object set on this node.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderState) qpNodePath::
get_state() const {
  nassertr_always(!is_empty(), RenderState::make_empty());
  return node()->get_state();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_state
//       Access: Published
//  Description: Changes the complete state object on this node.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_state(const RenderState *state) const {
  nassertv_always(!is_empty());
  node()->set_state(state);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_net_state
//       Access: Published
//  Description: Returns the net state on this node from the root.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderState) qpNodePath::
get_net_state() const {
  uncollapse_head();
  return r_get_net_state(_head);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_transform
//       Access: Published
//  Description: Returns the complete transform object set on this node.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) qpNodePath::
get_transform() const {
  nassertr_always(!is_empty(), TransformState::make_identity());
  return node()->get_transform();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_transform
//       Access: Published
//  Description: Changes the complete transform object on this node.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_transform(const TransformState *transform) const {
  nassertv_always(!is_empty());
  node()->set_transform(transform);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_net_transform
//       Access: Published
//  Description: Returns the net transform on this node from the root.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) qpNodePath::
get_net_transform() const {
  uncollapse_head();
  return r_get_net_transform(_head);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_pos
//       Access: Published
//  Description: Sets the translation component of the transform,
//               leaving rotation and scale untouched.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_pos(float x, float y, float z) {
  set_pos(LPoint3f(x, y, z));
}

INLINE float qpNodePath::
get_x() const {
  return get_pos()[0];
}

INLINE float qpNodePath::
get_y() const {
  return get_pos()[1];
}

INLINE float qpNodePath::
get_z() const {
  return get_pos()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_hpr
//       Access: Published
//  Description: Sets the rotation component of the transform,
//               leaving translation and scale untouched.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_hpr(float h, float p, float r) {
  set_hpr(LVecBase3f(h, p, r));
}

INLINE float qpNodePath::
get_h() const {
  return get_hpr()[0];
}

INLINE float qpNodePath::
get_p() const {
  return get_hpr()[1];
}

INLINE float qpNodePath::
get_r() const {
  return get_hpr()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_scale
//       Access: Published
//  Description: Sets the scale component of the transform,
//               leaving translation and rotation untouched.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_scale(float scale) {
  set_scale(LVecBase3f(scale, scale, scale));
}

INLINE void qpNodePath::
set_scale(float sx, float sy, float sz) {
  set_scale(LVecBase3f(sx, sy, sz));
}

INLINE float qpNodePath::
get_sx() const {
  return get_scale()[0];
}

INLINE float qpNodePath::
get_sy() const {
  return get_scale()[1];
}

INLINE float qpNodePath::
get_sz() const {
  return get_scale()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_pos_hpr
//       Access: Published
//  Description: Sets the translation and rotation component of the
//               transform, leaving scale untouched.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_pos_hpr(float x, float y, float z, float h, float p, float r) {
  set_pos_hpr(LVecBase3f(x, y, z), LVecBase3f(h, p, r));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_hpr_scale
//       Access: Published
//  Description: Sets the rotation and scale components of the
//               transform, leaving translation untouched.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_hpr_scale(float h, float p, float r, float sx, float sy, float sz) {
  set_hpr_scale(LVecBase3f(h, p, r), LVecBase3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_pos_hpr_scale
//       Access: Published
//  Description: Completely replaces the transform with new
//               translation, rotation, and scale components.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_pos_hpr_scale(float x, float y, float z, float h, float p, float r,
                  float sx, float sy, float sz) {
  set_pos_hpr_scale(LVecBase3f(x, y, z), LVecBase3f(h, p, r),
                    LVecBase3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::clear_mat
//       Access: Published
//  Description: Completely removes any transform from the referenced
//               node.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
clear_mat() {
  nassertv_always(!is_empty());
  node()->clear_transform();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_mat
//       Access: Published
//  Description: Returns true if a non-identity transform matrix has
//               been applied to the referenced node, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePath::
has_mat() const {
  nassertr_always(!is_empty(), false);
  return !node()->get_transform()->is_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_mat
//       Access: Published
//  Description: Returns the transform matrix that has been applied to
//               the referenced node, or the identity matrix if no
//               matrix has been applied.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &qpNodePath::
get_mat() const {
  nassertr_always(!is_empty(), LMatrix4f::ident_mat());

  return node()->get_transform()->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_color_scale
//       Access: Published
//  Description: Returns true if a color scale has been applied
//               to the referenced node, false otherwise.  It is still
//               possible that color at this node might have been
//               scaled by an ancestor node.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePath::
has_color_scale() const {
  nassertr(false, false);
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::clear_color_scale
//       Access: Published
//  Description: Completely removes any color scale from the
//               referenced node.  This is preferable to simply
//               setting the color scale to identity, as it also
//               removes the overhead associated with having a color
//               scale at all.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
clear_color_scale() {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_color_scale
//       Access: Published
//  Description: Sets the color scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_color_scale(float sr, float sg, float sb, float sa) {
  set_color_scale(LVecBase4f(sr, sg, sb, sa));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_sr
//       Access: Published
//  Description: Sets the red scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_sr(float sr) {
  LVecBase4f new_scale = get_color_scale();
  new_scale[0] = sr;

  set_color_scale(new_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_sg
//       Access: Published
//  Description: Sets the alpha scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_sg(float sg) {
  LVecBase4f new_scale = get_color_scale();
  new_scale[1] = sg;

  set_color_scale(new_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_sb
//       Access: Published
//  Description: Sets the blue scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_sb(float sb) {
  LVecBase4f new_scale = get_color_scale();
  new_scale[2] = sb;

  set_color_scale(new_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_sa
//       Access: Published
//  Description: Sets the alpha scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_sa(float sa) {
  LVecBase4f new_scale = get_color_scale();
  new_scale[3] = sa;

  set_color_scale(new_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_sr
//       Access: Published
//  Description: Gets the red scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE float qpNodePath::
get_sr() const {
  return get_color_scale()[0];
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_sg
//       Access: Published
//  Description: Gets the green scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE float qpNodePath::
get_sg() const {
  return get_color_scale()[1];
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_sb
//       Access: Published
//  Description: Gets the blue scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE float qpNodePath::
get_sb() const {
  return get_color_scale()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_sa
//       Access: Published
//  Description: Gets the alpha scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE float qpNodePath::
get_sa() const {
  return get_color_scale()[3];
}


////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::look_at
//       Access: Published
//  Description: Sets the transform on this qpNodePath so that it
//               rotates to face the indicated point in space.  This
//               will overwrite any previously existing scale on the
//               node, although it will preserve any translation.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
look_at(float x, float y, float z) {
  look_at(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::heads_up
//       Access: Published
//  Description: Behaves like look_at(), but with a strong preference
//               to keeping the up vector oriented in the indicated
//               "up" direction.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
heads_up(float x, float y, float z) {
  heads_up(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_pos
//       Access: Published
//  Description: Sets the translation component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_pos(const qpNodePath &other, float x, float y, float z) {
  set_pos(other, LPoint3f(x, y, z));
}

INLINE float qpNodePath::
get_x(const qpNodePath &other) const {
  return get_pos(other)[0];
}

INLINE float qpNodePath::
get_y(const qpNodePath &other) const {
  return get_pos(other)[1];
}

INLINE float qpNodePath::
get_z(const qpNodePath &other) const {
  return get_pos(other)[2];
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_hpr
//       Access: Published
//  Description: Sets the rotation component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_hpr(const qpNodePath &other, float h, float p, float r) {
  set_hpr(other, LPoint3f(h, p, r));
}

INLINE float qpNodePath::
get_h(const qpNodePath &other) const {
  return get_hpr(other)[0];
}

INLINE float qpNodePath::
get_p(const qpNodePath &other) const {
  return get_hpr(other)[1];
}

INLINE float qpNodePath::
get_r(const qpNodePath &other) const {
  return get_hpr(other)[2];
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_scale
//       Access: Published
//  Description: Sets the scale component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_scale(const qpNodePath &other, float sx, float sy, float sz) {
  set_scale(other, LPoint3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_scale
//       Access: Published
//  Description: Returns the relative scale of the referenced node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
INLINE float qpNodePath::
get_sx(const qpNodePath &other) const {
  return get_scale(other)[0];
}

INLINE float qpNodePath::
get_sy(const qpNodePath &other) const {
  return get_scale(other)[1];
}

INLINE float qpNodePath::
get_sz(const qpNodePath &other) const {
  return get_scale(other)[2];
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_pos_hpr
//       Access: Published
//  Description: Sets the translation and rotation component of the
//               transform, relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_pos_hpr(const qpNodePath &other,
            float x, float y, float z,
            float h, float p, float r) {
  set_pos_hpr(other, LVecBase3f(x, y, z), LVecBase3f(h, p, r));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_hpr_scale
//       Access: Published
//  Description: Sets the rotation and scale components of the
//               transform, leaving translation untouched.  This, or
//               set_pos_hpr_scale, is the preferred way to update a
//               transform when both hpr and scale are to be changed.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_hpr_scale(const qpNodePath &other,
	      float h, float p, float r, float sx, float sy, float sz) {
  set_hpr_scale(other, LVecBase3f(h, p, r), LVecBase3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_pos_hpr_scale
//       Access: Published
//  Description: Completely replaces the transform with new
//               translation, rotation, and scale components, relative
//               to the other node.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
set_pos_hpr_scale(const qpNodePath &other,
                  float x, float y, float z,
                  float h, float p, float r,
                  float sx, float sy, float sz) {
  set_pos_hpr_scale(other, LVecBase3f(x, y, z), LVecBase3f(h, p, r),
                    LVecBase3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::look_at
//       Access: Published
//  Description: Sets the hpr on this NodePath so that it rotates to
//               face the indicated point in space, which is relative
//               to the other NodePath.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
look_at(const qpNodePath &other, float x, float y, float z) {
  look_at(other, LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::heads_up
//       Access: Published
//  Description: Behaves like look_at(), but with a strong preference
//               to keeping the up vector oriented in the indicated
//               "up" direction.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
heads_up(const qpNodePath &other, float x, float y, float z) {
  heads_up(other, LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_distance
//       Access: Published
//  Description: Returns the straight-line distance between this
//               referenced node's coordinate frame's origin, and that
//               of the other node's origin.
////////////////////////////////////////////////////////////////////
INLINE float qpNodePath::
get_distance(const qpNodePath &other) const {
  LPoint3f pos = get_pos(other);
  return length(LVector3f(pos));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::adjust_all_priorities
//       Access: Published
//  Description: Adds the indicated adjustment amount (which may be
//               negative) to the priority for all transitions on the
//               referenced node, and for all nodes in the subgraph
//               below.  This can be used to force these nodes not to
//               be overridden by a high-level state change above.  If
//               the priority would drop below zero, it is set to
//               zero.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
adjust_all_priorities(int adjustment) {
  nassertv_always(!is_empty());
  r_adjust_all_priorities(node(), adjustment);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::show
//       Access: Published
//  Description: Undoes the effect of a previous hide() on this node:
//               makes the referenced node (and the entire subgraph
//               below this node) visible to all cameras.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
show() {
  nassertv_always(!is_empty());
  node()->set_draw_mask(DrawMask::all_on());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::show
//       Access: Published
//  Description: Makes the referenced node visible just to the
//               cameras whose camera_mask shares the indicated bits.
//               That is, this sets the indicated bits in the
//               node's draw mask.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
show(DrawMask camera_mask) {
  nassertv_always(!is_empty());
  node()->set_draw_mask(node()->get_draw_mask() | camera_mask);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::hide
//       Access: Published
//  Description: Makes the referenced node (and the entire subgraph
//               below this node) invisible to all cameras.  It
//               remains part of the scene graph, its bounding volume
//               still contributes to its parent's bounding volume,
//               and it will still be involved in collision tests.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
hide() {
  nassertv_always(!is_empty());
  node()->set_draw_mask(DrawMask::all_off());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::hide
//       Access: Published
//  Description: Makes the referenced node invisible just to the
//               cameras whose camera_mask shares the indicated bits.
//               That is, this clears the indicated bits from the
//               node's draw mask.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePath::
hide(DrawMask camera_mask) {
  nassertv_always(!is_empty());
  node()->set_draw_mask(node()->get_draw_mask() & ~camera_mask);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::is_hidden
//       Access: Published
//  Description: Returns true if the referenced node is hidden from
//               the indicated camera(s) either directly, or because
//               some ancestor is hidden.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePath::
is_hidden(DrawMask camera_mask) const {
  return !get_hidden_ancestor(camera_mask).is_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::unstash
//       Access: Published
//  Description: Undoes the effect of a previous stash() on this
//               node: makes the referenced node (and the entire
//               subgraph below this node) once again part of the
//               scene graph.  Returns true if the node is unstashed,
//               or false if it wasn't stashed to begin with.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePath::
unstash() {
  nassertr(!is_singleton(), false);
  return get_parent().node()->unstash_child(node());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::stash
//       Access: Published
//  Description: Removes the referenced node (and the entire subgraph
//               below this node) from the scene graph in any normal
//               sense.  The node will no longer be visible and is not
//               tested for collisions; furthermore, no normal scene
//               graph traversal will visit the node.  The node's
//               bounding volume no longer contributes to its parent's
//               bounding volume.
//
//               A stashed node cannot be located by a normal find()
//               operation (although a special find string can still
//               retrieve it).
//
//               Returns true if the node is successfully stashed, or
//               false if it was already stashed.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePath::
stash() {
  nassertr(!is_singleton(), false);
  return get_parent().node()->stash_child(node());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::is_stashed
//       Access: Published
//  Description: Returns true if the referenced node is stashed either
//               directly, or because some ancestor is stashed.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePath::
is_stashed() const {
  return !get_stashed_ancestor().is_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::operator ==
//       Access: Published
//  Description: Returns true if the two paths are equivalent; that
//               is, if they contain the same list of nodes in the same
//               order.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePath::
operator == (const qpNodePath &other) const {
  return (compare_to(other) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::operator !=
//       Access: Published
//  Description: Returns true if the two paths are not equivalent.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePath::
operator != (const qpNodePath &other) const {
  return (compare_to(other) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::operator <
//       Access: Published
//  Description: Returns true if this qpNodePath sorts before the other
//               one, false otherwise.  The sorting order of two
//               nonequivalent qpNodePaths is consistent but undefined,
//               and is useful only for storing qpNodePaths in a sorted
//               container like an STL set.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePath::
operator < (const qpNodePath &other) const {
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::compare_to
//       Access: Published
//  Description: Returns a number less than zero if this qpNodePath
//               sorts before the other one, greater than zero if it
//               sorts after, or zero if they are equivalent.
//
//               Two qpNodePaths are considered equivalent if they
//               consist of exactly the same list of nodes in the same
//               order.  Otherwise, they are different; different
//               qpNodePaths will be ranked in a consistent but
//               undefined ordering; the ordering is useful only for
//               placing the qpNodePaths in a sorted container like an
//               STL set.
////////////////////////////////////////////////////////////////////
INLINE int qpNodePath::
compare_to(const qpNodePath &other) const {
  uncollapse_head();
  other.uncollapse_head();

  // Nowadays, the NodePathComponents at the head are pointerwise
  // equivalent if and only iff the NodePaths are equivalent.  So we
  // only have to compare pointers.
  return _head - other._head;
}


INLINE ostream &operator << (ostream &out, const qpNodePath &node_path) {
  node_path.output(out);
  return out;
}

