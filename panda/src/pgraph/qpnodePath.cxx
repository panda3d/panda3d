// Filename: qpnodePath.cxx
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

#include "qpnodePath.h"
#include "qpnodePathCollection.h"
#include "qpfindApproxPath.h"
#include "qpfindApproxLevelEntry.h"
#include "qpfindApproxLevel.h"
#include "config_pgraph.h"
#include "colorAttrib.h"
#include "cullBinAttrib.h"
#include "textureAttrib.h"
#include "materialAttrib.h"
#include "cullFaceAttrib.h"
#include "billboardAttrib.h"
#include "transparencyAttrib.h"
#include "materialPool.h"
#include "look_at.h"
#include "compose_matrix.h"
#include "plist.h"

// stack seems to overflow on Intel C++ at 7000.  If we need more than 
// 7000, need to increase stack size.
int qpNodePath::_max_search_depth = 7000; 
TypeHandle qpNodePath::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_num_nodes
//       Access: Published
//  Description: Returns the number of nodes in the path.
////////////////////////////////////////////////////////////////////
int qpNodePath::
get_num_nodes() const {
  if (is_empty()) {
    return 0;
  }
  uncollapse_head();
  return _head->get_length();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_node
//       Access: Published
//  Description: Returns the nth node of the path, where 0 is the
//               referenced (bottom) node and get_num_nodes() - 1 is
//               the top node.  This requires iterating through the
//               path.
////////////////////////////////////////////////////////////////////
PandaNode *qpNodePath::
get_node(int index) const {
  nassertr(index >= 0 && index < get_num_nodes(), NULL);

  uncollapse_head();
  qpNodePathComponent *comp = _head;
  while (index > 0) {
    // If this assertion fails, the index was out of range; the
    // component's length must have been invalid.
    nassertr(comp != (qpNodePathComponent *)NULL, NULL);
    comp = comp->get_next();
    index--;
  }

  // If this assertion fails, the index was out of range; the
  // component's length must have been invalid.
  nassertr(comp != (qpNodePathComponent *)NULL, NULL);
  return comp->get_node();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_top_node
//       Access: Published
//  Description: Returns the top node of the path, or NULL if the path
//               is empty.  This requires iterating through the path.
////////////////////////////////////////////////////////////////////
PandaNode *qpNodePath::
get_top_node() const {
  if (is_empty()) {
    return (PandaNode *)NULL;
  }

  uncollapse_head();
  qpNodePathComponent *comp = _head;
  while (!comp->is_top_node()) {
    comp = comp->get_next();
    nassertr(comp != (qpNodePathComponent *)NULL, NULL);
  }

  return comp->get_node();
}


////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_children
//       Access: Published
//  Description: Returns the set of all child nodes of the referenced
//               node.
////////////////////////////////////////////////////////////////////
qpNodePathCollection qpNodePath::
get_children() const {
  qpNodePathCollection result;
  nassertr(!is_empty(), result);

  PandaNode *bottom_node = node();

  PandaNode::Children cr = bottom_node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    qpNodePath child;
    child._head = PandaNode::get_component(_head, cr.get_child(i));
    result.add_path(child);
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: qoNodePath::find
//       Access: Published
//  Description: Searches for a node below the referenced node that
//               matches the indicated string.  Returns the shortest
//               match found, if any, or an empty NodePath if no match
//               can be found.
////////////////////////////////////////////////////////////////////
qpNodePath qpNodePath::
find(const string &path) const {
  nassertr(!is_empty(), fail());

  qpNodePathCollection col;
  find_matches(col, path, 1);

  if (col.is_empty()) {
    return qpNodePath::not_found();
  }

  return col.get_path(0);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::find_all_matches
//       Access: Published
//  Description: Returns the complete set of all NodePaths that begin
//               with this NodePath and can be extended by
//               path.  The shortest paths will be listed
//               first.
////////////////////////////////////////////////////////////////////
qpNodePathCollection qpNodePath::
find_all_matches(const string &path) const {
  qpNodePathCollection col;
  nassertr(!is_empty(), col);
  nassertr(verify_complete(), col);
  find_matches(col, path, -1);
  return col;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::find_all_paths_to
//       Access: Published
//  Description: Returns the set of all NodePaths that extend from
//               this NodePath down to the indicated node.  The
//               shortest paths will be listed first.
////////////////////////////////////////////////////////////////////
qpNodePathCollection qpNodePath::
find_all_paths_to(PandaNode *node) const {
  qpNodePathCollection col;
  nassertr(!is_empty(), col);
  nassertr(verify_complete(), col);
  nassertr(node != (PandaNode *)NULL, col);
  qpFindApproxPath approx_path;
  approx_path.add_match_many(0);
  approx_path.add_match_pointer(node, 0);
  find_matches(col, approx_path, -1);
  return col;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::reparent_to
//       Access: Published
//  Description: Removes the referenced node of the qpNodePath from its
//               current parent and attaches it to the referenced node of
//               the indicated qpNodePath.
////////////////////////////////////////////////////////////////////
void qpNodePath::
reparent_to(const qpNodePath &other, int sort) {
  nassertv(other.verify_complete());
  nassertv_always(!is_empty());
  nassertv_always(!other.is_empty());

  uncollapse_head();
  other.uncollapse_head();
  PandaNode::reparent(other._head, _head, sort);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::wrt_reparent_to
//       Access: Published
//  Description: This functions identically to reparent_to(), except
//               the transform on this node is also adjusted so that
//               the node remains in the same place in world
//               coordinates, even if it is reparented into a
//               different coordinate system.
////////////////////////////////////////////////////////////////////
void qpNodePath::
wrt_reparent_to(const qpNodePath &other, int sort) {
  nassertv(other.verify_complete());
  nassertv_always(!is_empty());
  nassertv_always(!other.is_empty());

  set_transform(get_transform(other));
  reparent_to(other, sort);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::instance_to
//       Access: Published
//  Description: Adds the referenced node of the NodePath as a child
//               of the referenced node of the indicated other
//               NodePath.  Any other parent-child relations of the
//               node are unchanged; in particular, the node is not
//               removed from its existing parent, if any.
//
//               If the node already had an existing parent, this
//               method will create a new instance of the node within
//               the scene graph.
//
//               This does not change the NodePath itself, but does
//               return a new NodePath that reflects the new instance
//               node.
////////////////////////////////////////////////////////////////////
qpNodePath qpNodePath::
instance_to(const qpNodePath &other, int sort) const {
  nassertr(verify_complete(), qpNodePath::fail());
  nassertr(!is_empty(), qpNodePath::fail());
  nassertr(!other.is_empty(), qpNodePath::fail());

  uncollapse_head();
  other.uncollapse_head();

  qpNodePath new_instance;
  new_instance._head = PandaNode::attach(other._head, node(), sort);

  return new_instance;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::copy_to
//       Access: Published
//  Description: Functions exactly like instance_to(), except a deep
//               copy is made of the referenced node and all of its
//               descendents, which is then parented to the indicated
//               node.  A qpNodePath to the newly created copy is
//               returned.
//
//               Certain kinds of nodes (like a Camera, for instance,
//               whose pointer value is important) may not be copied;
//               if one of these is encountered, the node will be
//               instanced, instead of actually copied.
////////////////////////////////////////////////////////////////////
qpNodePath qpNodePath::
copy_to(const qpNodePath &other, int sort) const {
  nassertr(verify_complete(), fail());
  nassertr(!is_empty(), fail());
  nassertr(!other.is_empty(), fail());

  PandaNode *source_node = node();
  PT(PandaNode) copy_node = source_node->copy_subgraph();
  nassertr(copy_node != (PandaNode *)NULL, fail());

  return other.attach_new_node(copy_node, sort);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::attach_new_node
//       Access: Published
//  Description: Attaches a new node, with or without existing
//               parents, to the scene graph below the referenced node
//               of this NodePath.  This is the preferred way to add
//               nodes to the graph.
//
//               This does *not* automatically extend the current
//               NodePath to reflect the attachment; however, a
//               NodePath that does reflect this extension is
//               returned.
////////////////////////////////////////////////////////////////////
qpNodePath qpNodePath::
attach_new_node(PandaNode *node, int sort) const {
  nassertr(verify_complete(), qpNodePath::fail());
  nassertr(!is_empty(), qpNodePath());
  nassertr(node != (PandaNode *)NULL, qpNodePath());

  uncollapse_head();
  qpNodePath new_path(*this);
  new_path._head = PandaNode::attach(_head, node, sort);
  return new_path;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::remove_node
//       Access: Published
//  Description: Disconnects the referenced node from the scene graph.
//               This will also delete the node if there are no other
//               pointers to it.
//
//               Normally, this should be called only when you are
//               really done with the node.  If you want to remove a
//               node from the scene graph but keep it around for
//               later, you should probably use reparent_to() and put
//               it under a holding node instead.
//
//               After the node is removed, the qpNodePath will have
//               been cleared.
////////////////////////////////////////////////////////////////////
void qpNodePath::
remove_node() {
  nassertv(_error_type != ET_not_found);
  if (is_empty() || is_singleton()) {
    // If we have no parents, remove_node() is just a do-nothing
    // operation; if we have no nodes, maybe we were already removed.
    // In either case, quietly do nothing except to ensure the
    // qpNodePath is clear.
    (*this) = qpNodePath::removed();
    return;
  }

  uncollapse_head();
  PandaNode::detach(_head);

  (*this) = qpNodePath::removed();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_state
//       Access: Published
//  Description: Returns the state changes that must be made to
//               transition from the render state of this node to the
//               render state of the other node.
////////////////////////////////////////////////////////////////////
CPT(RenderState) qpNodePath::
get_state(const qpNodePath &other) const {
  if (is_empty()) {
    return other.get_net_state();
  }
  if (other.is_empty()) {
    return get_net_state()->invert_compose(RenderState::make_empty());
  }
    
  nassertr(verify_complete(), RenderState::make_empty());
  nassertr(other.verify_complete(), RenderState::make_empty());

  int a_count, b_count;
  find_common_ancestor(*this, other, a_count, b_count);

  CPT(RenderState) a_state = r_get_partial_state(_head, a_count);
  CPT(RenderState) b_state = r_get_partial_state(other._head, b_count);
  return a_state->invert_compose(b_state);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_state
//       Access: Published
//  Description: Sets the state object on this node, relative to
//               the other node.  This computes a new state object
//               that has the indicated value when seen relative to
//               the other node.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_state(const qpNodePath &other, const RenderState *state) const {
  nassertv(_error_type == ET_ok && other._error_type == ET_ok);
  nassertv_always(!is_empty());

  // First, we perform a wrt to the parent, to get the conversion.
  qpNodePath parent = get_parent();
  CPT(RenderState) rel_state = parent.get_state(other);

  CPT(RenderState) new_state = rel_state->compose(state);
  set_state(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_transform
//       Access: Published
//  Description: Returns the relative transform to this node from the
//               other node; i.e. the transformation of this node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
CPT(TransformState) qpNodePath::
get_transform(const qpNodePath &other) const {
  if (other.is_empty()) {
    return get_net_transform();
  }
  if (is_empty()) {
    return other.get_net_transform()->invert_compose(TransformState::make_identity());
  }
    
  nassertr(verify_complete(), TransformState::make_identity());
  nassertr(other.verify_complete(), TransformState::make_identity());

  int a_count, b_count;
  find_common_ancestor(*this, other, a_count, b_count);

  CPT(TransformState) a_transform = r_get_partial_transform(_head, a_count);
  CPT(TransformState) b_transform = r_get_partial_transform(other._head, b_count);
  return b_transform->invert_compose(a_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_transform
//       Access: Published
//  Description: Sets the transform object on this node, relative to
//               the other node.  This computes a new transform object
//               that will have the indicated value when seen from the
//               other node.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_transform(const qpNodePath &other, const TransformState *transform) const {
  nassertv(_error_type == ET_ok && other._error_type == ET_ok);
  nassertv_always(!is_empty());

  // First, we perform a wrt to the parent, to get the conversion.
  qpNodePath parent = get_parent();
  CPT(TransformState) rel_trans = other.get_transform(parent);

  CPT(TransformState) new_trans = rel_trans->compose(transform);
  set_transform(new_trans);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_pos
//       Access: Published
//  Description: Sets the translation component of the transform,
//               leaving rotation and scale untouched.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_pos(const LVecBase3f &pos) {
  nassertv(!is_empty());
  set_transform(get_transform()->set_pos(pos));
}

void qpNodePath::
set_x(float x) {
  nassertv(!is_empty());
  LPoint3f pos = get_pos();
  pos[0] = x;
  set_pos(pos);
}

void qpNodePath::
set_y(float y) {
  nassertv(!is_empty());
  LPoint3f pos = get_pos();
  pos[1] = y;
  set_pos(pos);
}

void qpNodePath::
set_z(float z) {
  nassertv(!is_empty());
  LPoint3f pos = get_pos();
  pos[2] = z;
  set_pos(pos);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_pos
//       Access: Published
//  Description: Retrieves the translation component of the transform.
////////////////////////////////////////////////////////////////////
LPoint3f qpNodePath::
get_pos() const {
  nassertr(!is_empty(), LPoint3f(0.0f, 0.0f, 0.0f));
  return get_transform()->get_pos();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_hpr
//       Access: Published
//  Description: Sets the rotation component of the transform,
//               leaving translation and scale untouched.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_hpr(const LVecBase3f &hpr) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_components());
  set_transform(transform->set_hpr(hpr));
}

void qpNodePath::
set_h(float h) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_components());
  LVecBase3f hpr = transform->get_hpr();
  hpr[0] = h;
  set_transform(transform->set_hpr(hpr));
}

void qpNodePath::
set_p(float p) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_components());
  LVecBase3f hpr = transform->get_hpr();
  hpr[1] = p;
  set_transform(transform->set_hpr(hpr));
}

void qpNodePath::
set_r(float r) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_components());
  LVecBase3f hpr = transform->get_hpr();
  hpr[2] = r;
  set_transform(transform->set_hpr(hpr));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_hpr
//       Access: Published
//  Description: Retrieves the rotation component of the transform.
////////////////////////////////////////////////////////////////////
LVecBase3f qpNodePath::
get_hpr() const {
  nassertr(!is_empty(), LVecBase3f(0.0f, 0.0f, 0.0f));
  CPT(TransformState) transform = get_transform();
  nassertr(transform->has_components(), LVecBase3f(0.0f, 0.0f, 0.0f));
  return transform->get_hpr();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_hpr
//       Access: Published
//  Description: Retrieves the rotation component of the transform.
////////////////////////////////////////////////////////////////////
LVecBase3f qpNodePath::
get_hpr(float roll) const {
  // This function is deprecated.  It used to be a hack to work around
  // a problem with decomposing Euler angles, but since we no longer
  // depend on decomposing these, we shouldn't need this any more.
  return get_hpr();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_scale
//       Access: Published
//  Description: Sets the scale component of the transform,
//               leaving translation and rotation untouched.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_scale(const LVecBase3f &scale) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_components());
  set_transform(transform->set_scale(scale));
}

void qpNodePath::
set_sx(float sx) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_components());
  LVecBase3f scale = transform->get_scale();
  scale[0] = sx;
  set_transform(transform->set_scale(scale));
}

void qpNodePath::
set_sy(float sy) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_components());
  LVecBase3f scale = transform->get_scale();
  scale[1] = sy;
  set_transform(transform->set_scale(scale));
}

void qpNodePath::
set_sz(float sz) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_components());
  LVecBase3f scale = transform->get_scale();
  scale[2] = sz;
  set_transform(transform->set_scale(scale));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_scale
//       Access: Published
//  Description: Retrieves the scale component of the transform.
////////////////////////////////////////////////////////////////////
LVecBase3f qpNodePath::
get_scale() const {
  nassertr(!is_empty(), LVecBase3f(0.0f, 0.0f, 0.0f));
  CPT(TransformState) transform = get_transform();
  nassertr(transform->has_components(), LVecBase3f(0.0f, 0.0f, 0.0f));
  return transform->get_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_pos_hpr
//       Access: Published
//  Description: Sets the translation and rotation component of the
//               transform, leaving scale untouched.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_pos_hpr(const LVecBase3f &pos, const LVecBase3f &hpr) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_components());
  transform = TransformState::make_pos_hpr_scale
    (pos, hpr, transform->get_scale());
  set_transform(transform);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_hpr_scale
//       Access: Published
//  Description: Sets the rotation and scale components of the
//               transform, leaving translation untouched.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_hpr_scale(const LVecBase3f &hpr, const LVecBase3f &scale) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  transform = TransformState::make_pos_hpr_scale
    (transform->get_pos(), hpr, scale);
  set_transform(transform);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_pos_hpr_scale
//       Access: Published
//  Description: Completely replaces the transform with new
//               translation, rotation, and scale components.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_pos_hpr_scale(const LVecBase3f &pos, const LVecBase3f &hpr,
                  const LVecBase3f &scale) {
  nassertv_always(!is_empty());
  set_transform(TransformState::make_pos_hpr_scale
                (pos, hpr, scale));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_mat
//       Access: Published
//  Description: Directly sets an arbitrary 4x4 transform matrix.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_mat(const LMatrix4f &mat) {
  nassertv_always(!is_empty());
  set_transform(TransformState::make_mat(mat));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_color_scale
//       Access: Published
//  Description: Sets the color scale component of the transform,
//               leaving translation and rotation untouched.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_color_scale(const LVecBase4f &sv4) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_color_scale
//       Access: Published
//  Description: Returns the complete transform vector that has been
//               applied to the bottom node, or the all 1's if no
//               scale has been applied
////////////////////////////////////////////////////////////////////
LVecBase4f qpNodePath::
get_color_scale() const {
  nassertr(false, LVecBase4f(1.0f,1.0f,1.0f,1.0f));
  return LVecBase4f(1.0f,1.0f,1.0f,1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::look_at
//       Access: Published
//  Description: Sets the transform on this qpNodePath so that it
//               rotates to face the indicated point in space.  This
//               will overwrite any previously existing scale on the
//               node, although it will preserve any translation.  See
//               also look_at_preserve_scale().
////////////////////////////////////////////////////////////////////
void qpNodePath::
look_at(const LPoint3f &point, const LVector3f &up) {
  nassertv(!is_empty());

  LPoint3f pos = get_pos();

  LMatrix4f mat;
  ::look_at(mat, point - pos, up);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::heads_up
//       Access: Published
//  Description: Behaves like look_at(), but with a strong preference
//               to keeping the up vector oriented in the indicated
//               "up" direction.
////////////////////////////////////////////////////////////////////
void qpNodePath::
heads_up(const LPoint3f &point, const LVector3f &up) {
  nassertv(!is_empty());

  LPoint3f pos = get_pos();

  LMatrix4f mat;
  ::heads_up(mat, point - pos, up);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::look_at_preserve_scale
//       Access: Published
//  Description: Functions like look_at(), but preforms additional
//               work to preserve any scales that may already be
//               present on the node.  Normally, look_at() blows away
//               the scale because scale and rotation are represented
//               in the same part of the matrix.
////////////////////////////////////////////////////////////////////
void qpNodePath::
look_at_preserve_scale(const LPoint3f &point, const LVector3f &up) {
  nassertv(!is_empty());

  LMatrix4f mat = get_mat();

  // Extract the axes from the matrix.
  LVector3f x, y, z;

  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  // The lengths of the axes defines the scale.

  float scale_0 = length(x);
  float scale_1 = length(y);
  float scale_2 = length(z);

  LPoint3f pos;
  mat.get_row3(pos,3);
  ::look_at(mat, point - pos, up);

  // Now reapply the scale and position.

  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  x *= scale_0;
  y *= scale_1;
  z *= scale_2;

  mat.set_row(0, x);
  mat.set_row(1, y);
  mat.set_row(2, z);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::heads_up_preserve_scale
//       Access: Published
//  Description: Functions like heads_up(), but preforms additional
//               work to preserve any scales that may already be
//               present on the node.  Normally, heads_up() blows away
//               the scale because scale and rotation are represented
//               in the same part of the matrix.
////////////////////////////////////////////////////////////////////
void qpNodePath::
heads_up_preserve_scale(const LPoint3f &point, const LVector3f &up) {
  nassertv(!is_empty());

  LMatrix4f mat = get_mat();

  // Extract the axes from the matrix.
  LVector3f x, y, z;
  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  float scale_0 = length(x);
  float scale_1 = length(y);
  float scale_2 = length(z);

  // The lengths of the axes defines the scale.
  LPoint3f pos;
  mat.get_row3(pos,3);

  ::heads_up(mat, point - pos, up);

  // Now reapply the scale and position.

  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  x *= scale_0;
  y *= scale_1;
  z *= scale_2;

  mat.set_row(0, x);
  mat.set_row(1, y);
  mat.set_row(2, z);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_pos
//       Access: Published
//  Description: Sets the translation component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_pos(const qpNodePath &other, const LVecBase3f &pos) {
  nassertv(!is_empty());
  set_transform(other, get_transform(other)->set_pos(pos));
}

void qpNodePath::
set_x(const qpNodePath &other, float x) {
  nassertv(!is_empty());
  LPoint3f pos = get_pos(other);
  pos[0] = x;
  set_pos(other, pos);
}

void qpNodePath::
set_y(const qpNodePath &other, float y) {
  nassertv(!is_empty());
  LPoint3f pos = get_pos(other);
  pos[1] = y;
  set_pos(other, pos);
}

void qpNodePath::
set_z(const qpNodePath &other, float z) {
  nassertv(!is_empty());
  LPoint3f pos = get_pos(other);
  pos[2] = z;
  set_pos(other, pos);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_pos
//       Access: Published
//  Description: Returns the relative position of the referenced node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
LPoint3f qpNodePath::
get_pos(const qpNodePath &other) const {
  nassertr(!is_empty(), LPoint3f(0.0f, 0.0f, 0.0f));
  return get_transform(other)->get_pos();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_hpr
//       Access: Published
//  Description: Sets the rotation component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_hpr(const qpNodePath &other, const LVecBase3f &hpr) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform(other);
  nassertv(transform->has_components());
  set_transform(other, transform->set_hpr(hpr));
}

void qpNodePath::
set_h(const qpNodePath &other, float h) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform(other);
  nassertv(transform->has_components());
  LVecBase3f hpr = transform->get_hpr();
  hpr[0] = h;
  set_transform(other, transform->set_hpr(hpr));
}

void qpNodePath::
set_p(const qpNodePath &other, float p) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform(other);
  nassertv(transform->has_components());
  LVecBase3f hpr = transform->get_hpr();
  hpr[1] = p;
  set_transform(other, transform->set_hpr(hpr));
}

void qpNodePath::
set_r(const qpNodePath &other, float r) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform(other);
  nassertv(transform->has_components());
  LVecBase3f hpr = transform->get_hpr();
  hpr[2] = r;
  set_transform(other, transform->set_hpr(hpr));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_hpr
//       Access: Published
//  Description: Returns the relative orientation of the bottom node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
LVecBase3f qpNodePath::
get_hpr(const qpNodePath &other) const {
  nassertr(!is_empty(), LVecBase3f(0.0f, 0.0f, 0.0f));
  CPT(TransformState) transform = get_transform(other);
  nassertr(transform->has_components(), LVecBase3f(0.0f, 0.0f, 0.0f));
  return transform->get_hpr();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_hpr
//       Access: Published
//  Description: Returns the relative orientation of the bottom node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
LVecBase3f qpNodePath::
get_hpr(const qpNodePath &other, float roll) const {
  // This is still doing it the dumb way, with a decomposition.  This
  // function is deprecated anyway.
  LMatrix4f mat = get_mat(other);
  LVector3f scale, hpr, pos;
  decompose_matrix(mat, scale, hpr, pos, roll);
  return hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_scale
//       Access: Published
//  Description: Sets the scale component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_scale(const qpNodePath &other, const LVecBase3f &scale) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform(other);
  nassertv(transform->has_components());
  set_transform(other, transform->set_scale(scale));
}

void qpNodePath::
set_sx(const qpNodePath &other, float sx) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform(other);
  nassertv(transform->has_components());
  LVecBase3f scale = transform->get_scale();
  scale[0] = sx;
  set_transform(other, transform->set_scale(scale));
}

void qpNodePath::
set_sy(const qpNodePath &other, float sy) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform(other);
  nassertv(transform->has_components());
  LVecBase3f scale = transform->get_scale();
  scale[1] = sy;
  set_transform(other, transform->set_scale(scale));
}

void qpNodePath::
set_sz(const qpNodePath &other, float sz) {
  nassertv(!is_empty());
  CPT(TransformState) transform = get_transform(other);
  nassertv(transform->has_components());
  LVecBase3f scale = transform->get_scale();
  scale[2] = sz;
  set_transform(other, transform->set_scale(scale));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_scale
//       Access: Published
//  Description: Returns the relative scale of the bottom node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
LVecBase3f qpNodePath::
get_scale(const qpNodePath &other) const {
  nassertr(!is_empty(), LVecBase3f(0.0f, 0.0f, 0.0f));
  CPT(TransformState) transform = get_transform(other);
  nassertr(transform->has_components(), LVecBase3f(0.0f, 0.0f, 0.0f));
  return transform->get_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_pos_hpr
//       Access: Published
//  Description: Sets the translation and rotation component of the
//               transform, relative to the other node.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_pos_hpr(const qpNodePath &other, const LVecBase3f &pos,
            const LVecBase3f &hpr) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform(other);
  nassertv(transform->has_components());
  transform = TransformState::make_pos_hpr_scale
    (pos, hpr, transform->get_scale());
  set_transform(other, transform);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_hpr_scale
//       Access: Published
//  Description: Sets the rotation and scale components of the
//               transform, leaving translation untouched.  This, or
//               set_pos_hpr_scale, is the preferred way to update a
//               transform when both hpr and scale are to be changed.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_hpr_scale(const qpNodePath &other, const LVecBase3f &hpr, const LVecBase3f &scale) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform(other);
  transform = TransformState::make_pos_hpr_scale
    (transform->get_pos(), hpr, scale);
  set_transform(other, transform);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_pos_hpr_scale
//       Access: Published
//  Description: Completely replaces the transform with new
//               translation, rotation, and scale components, relative
//               to the other node.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_pos_hpr_scale(const qpNodePath &other,
                  const LVecBase3f &pos, const LVecBase3f &hpr,
                  const LVecBase3f &scale) {
  nassertv_always(!is_empty());
  set_transform(other, TransformState::make_pos_hpr_scale
                (pos, hpr, scale));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_mat
//       Access: Published
//  Description: Returns the matrix that describes the coordinate
//               space of the bottom node, relative to the other
//               path's bottom node's coordinate space.
////////////////////////////////////////////////////////////////////
const LMatrix4f &qpNodePath::
get_mat(const qpNodePath &other) const {
  CPT(TransformState) transform = get_transform(other);
  // We can safely assume the transform won't go away when the
  // function returns, since its reference count is also held in the
  // cache.  This assumption allows us to return a reference to the
  // matrix, instead of having to return a matrix on the stack.
  nassertr(transform->get_ref_count() > 1, LMatrix4f::ident_mat());
  return transform->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_mat
//       Access: Published
//  Description: Converts the indicated matrix from the other's
//               coordinate space to the local coordinate space, and
//               applies it to the node.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_mat(const qpNodePath &other, const LMatrix4f &mat) {
  nassertv_always(!is_empty());
  set_transform(other, TransformState::make_mat(mat));
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_relative_point
//       Access: Published
//  Description: Given that the indicated point is in the coordinate
//               system of the other node, returns the same point in
//               this node's coordinate system.
////////////////////////////////////////////////////////////////////
LPoint3f qpNodePath::
get_relative_point(const qpNodePath &other, const LVecBase3f &point) {
  LPoint3f rel_point = LPoint3f(point) * other.get_mat(*this);
  return rel_point;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::look_at
//       Access: Published
//  Description: Sets the transform on this qpNodePath so that it
//               rotates to face the indicated point in space, which
//               is relative to the other qpNodePath.  This
//               will overwrite any previously existing scale on the
//               node, although it will preserve any translation.
////////////////////////////////////////////////////////////////////
void qpNodePath::
look_at(const qpNodePath &other, const LPoint3f &point, const LVector3f &up) {
  nassertv(!is_empty());

  LPoint3f pos = get_pos();

  qpNodePath parent = get_parent();
  LPoint3f rel_point = point * other.get_mat(parent);

  LMatrix4f mat;
  ::look_at(mat, rel_point - pos, up);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::heads_up
//       Access: Published
//  Description: Behaves like look_at(), but with a strong preference
//               to keeping the up vector oriented in the indicated
//               "up" direction.
////////////////////////////////////////////////////////////////////
void qpNodePath::
heads_up(const qpNodePath &other, const LPoint3f &point, const LVector3f &up) {
  nassertv(!is_empty());

  LPoint3f pos = get_pos();

  qpNodePath parent = get_parent();
  LPoint3f rel_point = point * other.get_mat(parent);

  LMatrix4f mat;
  ::heads_up(mat, rel_point - pos, up);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::look_at_preserve_scale
//       Access: Published
//  Description: Functions like look_at(), but preforms additional
//               work to preserve any scales that may already be
//               present on the node.  Normally, look_at() blows away
//               the scale because scale and rotation are represented
//               in the same part of the matrix.
////////////////////////////////////////////////////////////////////
void qpNodePath::
look_at_preserve_scale(const qpNodePath &other, const LPoint3f &point,
                       const LVector3f &up) {
  nassertv(!is_empty());

  LMatrix4f mat = get_mat();

  // Extract the axes from the matrix.
  LVector3f x, y, z;
  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  // The lengths of the axes defines the scale.
  float scale_0 = length(x);
  float scale_1 = length(y);
  float scale_2 = length(z);

  LPoint3f pos;
  mat.get_row3(pos,3);

  qpNodePath parent = get_parent();
  LPoint3f rel_point = point * other.get_mat(parent);

  ::look_at(mat, rel_point - pos, up);

  // Now reapply the scale and position.

  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  x *= scale_0;
  y *= scale_1;
  z *= scale_2;

  mat.set_row(0, x);
  mat.set_row(1, y);
  mat.set_row(2, z);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::heads_up_preserve_scale
//       Access: Published
//  Description: Functions like heads_up(), but preforms additional
//               work to preserve any scales that may already be
//               present on the node.  Normally, heads_up() blows away
//               the scale because scale and rotation are represented
//               in the same part of the matrix.
////////////////////////////////////////////////////////////////////
void qpNodePath::
heads_up_preserve_scale(const qpNodePath &other, const LPoint3f &point,
                        const LVector3f &up) {
  nassertv(!is_empty());

  LMatrix4f mat = get_mat();

  // Extract the axes from the matrix.
  LVector3f x, y, z;
  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  // The lengths of the axes defines the scale.
  float scale_0 = length(x);
  float scale_1 = length(y);
  float scale_2 = length(z);

  LPoint3f pos;
  mat.get_row3(pos,3);

  qpNodePath parent = get_parent();
  LPoint3f rel_point = point * other.get_mat(parent);

  ::heads_up(mat, rel_point - pos, up);

  // Now reapply the scale and position.
  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  x *= scale_0;
  y *= scale_1;
  z *= scale_2;

  mat.set_row(0, x);
  mat.set_row(1, y);
  mat.set_row(2, z);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_color
//       Access: Published
//  Description: Applies a scene-graph color to the referenced node.
//               This color will apply to all geometry at this level
//               and below (that does not specify a new color or a
//               set_color_off()).
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_color(float r, float g, float b, float a,
          int priority) {
  set_color(Colorf(r, g, b, a), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_color
//       Access: Published
//  Description: Applies a scene-graph color to the referenced node.
//               This color will apply to all geometry at this level
//               and below (that does not specify a new color or a
//               set_color_off()).
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_color(const Colorf &color, int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(ColorAttrib::make_flat(color), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_color_off
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using the geometry color.  This is normally the
//               default, but it may be useful to use this to
//               contradict set_color() at a higher node level (or,
//               with a priority, to override a set_color() at a lower
//               level).
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_color_off(int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(ColorAttrib::make_vertex(), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::clear_color
//       Access: Published
//  Description: Completely removes any color adjustment from the node.
//               This allows the natural color of the geometry, or
//               whatever color transitions might be otherwise
//               affecting the geometry, to show instead.
////////////////////////////////////////////////////////////////////
void qpNodePath::
clear_color() {
  nassertv(!is_empty());
  node()->clear_attrib(ColorAttrib::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_color
//       Access: Published
//  Description: Returns true if a color has been applied to the given
//               node, false otherwise.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
has_color() const {
  nassertr(!is_empty(), false);
  return node()->has_attrib(ColorAttrib::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_color
//       Access: Published
//  Description: Returns the color that has been assigned to the node,
//               or black if no color has been assigned.
////////////////////////////////////////////////////////////////////
Colorf qpNodePath::
get_color() const {
  nassertr(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(ColorAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const ColorAttrib *ca = DCAST(ColorAttrib, attrib);
    if (ca->get_color_type() == ColorAttrib::T_flat) {
      return ca->get_color();
    }
  }

  pgraph_cat.warning()
    << "get_color() called on " << *this << " which has no color set.\n";

  return Colorf(0.0f, 0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_bin
//       Access: Published
//  Description: Assigns the geometry at this level and below to the
//               named rendering bin.  It is the user's responsibility
//               to ensure that such a bin already exists, either via
//               the cull-bin Configrc variable, or by explicitly
//               creating a GeomBin of the appropriate type at
//               runtime.
//
//               There are two default bins created when Panda is
//               started: "default" and "fixed".  Normally, all
//               geometry is assigned to "default" unless specified
//               otherwise.  This bin renders opaque geometry in
//               state-sorted order, followed by transparent geometry
//               sorted back-to-front.  If any geometry is assigned to
//               "fixed", this will be rendered following all the
//               geometry in "default", in the order specified by
//               draw_order for each piece of geometry so assigned.
//
//               The draw_order parameter is meaningful only for
//               GeomBinFixed type bins, e.g. "fixed".  Other kinds of
//               bins ignore it.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_bin(const string &bin_name, int draw_order, int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(CullBinAttrib::make(bin_name, draw_order), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::clear_bin
//       Access: Published
//  Description: Completely removes any bin adjustment that may have
//               been set via set_bin() from this particular node.
////////////////////////////////////////////////////////////////////
void qpNodePath::
clear_bin() {
  nassertv(!is_empty());
  node()->clear_attrib(CullBinAttrib::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_bin
//       Access: Published
//  Description: Returns true if the node has been assigned to the a
//               particular rendering bin via set_bin(), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
has_bin() const {
  nassertr(!is_empty(), false);
  return node()->has_attrib(CullBinAttrib::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_bin_name
//       Access: Published
//  Description: Returns the name of the bin that this particular node
//               was assigned to via set_bin(), or the empty string if
//               no bin was assigned.  See set_bin() and has_bin().
////////////////////////////////////////////////////////////////////
string qpNodePath::
get_bin_name() const {
  nassertr(!is_empty(), string());
  const RenderAttrib *attrib =
    node()->get_attrib(ColorAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const CullBinAttrib *ba = DCAST(CullBinAttrib, attrib);
    return ba->get_bin_name();
  }

  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_bin_draw_order
//       Access: Published
//  Description: Returns the drawing order associated with the bin
//               that this particular node was assigned to via
//               set_bin(), or 0 if no bin was assigned.  See
//               set_bin() and has_bin().
////////////////////////////////////////////////////////////////////
int qpNodePath::
get_bin_draw_order() const {
  nassertr(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(ColorAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const CullBinAttrib *ba = DCAST(CullBinAttrib, attrib);
    return ba->get_draw_order();
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_texture
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using the indicated texture.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_texture(Texture *tex, int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(TextureAttrib::make(tex), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_texture_off
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using no texture.  This is normally the default, but
//               it may be useful to use this to contradict
//               set_texture() at a higher node level (or, with a
//               priority, to override a set_texture() at a lower
//               level).
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_texture_off(int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(TextureAttrib::make_off(), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::clear_texture
//       Access: Published
//  Description: Completely removes any texture adjustment that may
//               have been set via set_texture() or set_texture_off()
//               from this particular node.  This allows whatever
//               textures might be otherwise affecting the geometry to
//               show instead.
////////////////////////////////////////////////////////////////////
void qpNodePath::
clear_texture() {
  nassertv(!is_empty());
  node()->clear_attrib(TextureAttrib::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_texture
//       Access: Published
//  Description: Returns true if a texture has been applied to this
//               particular node via set_texture(), false otherwise.
//               This is not the same thing as asking whether the
//               geometry at this node will be rendered with
//               texturing, as there may be a texture in effect from a
//               higher or lower level.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
has_texture() const {
  nassertr(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(TextureAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
    return !ta->is_off();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_texture_off
//       Access: Published
//  Description: Returns true if a texture has been specifically
//               disabled on this particular node via
//               set_texture_off(), false otherwise.  This is not the
//               same thing as asking whether the geometry at this
//               node will be rendered untextured, as there may be a
//               texture in effect from a higher or lower level.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
has_texture_off() const {
  nassertr(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(ColorAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
    return ta->is_off();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_texture
//       Access: Published
//  Description: Returns the texture that has been set on this
//               particular node, or NULL if no texture has been set.
//               This is not necessarily the texture that will be
//               applied to the geometry at or below this level, as
//               another texture at a higher or lower level may
//               override.
////////////////////////////////////////////////////////////////////
Texture *qpNodePath::
get_texture() const {
  nassertr(!is_empty(), NULL);
  const RenderAttrib *attrib =
    node()->get_attrib(TextureAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
    return ta->get_texture();
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_material
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using the indicated material.
//
//               This operation copies the given material pointer.  If
//               the material structure is changed later, it must be
//               reapplied via another call to set_material().
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_material(Material *mat, int priority) {
  nassertv_always(!is_empty());
  nassertv(mat != NULL);

  // We create a temporary Material pointer, a copy of the one we are
  // given, to allow the user to monkey with the material and set it
  // again later, with the desired effect.  If we stored the user's
  // pointer directly, it would be bad if the user later modified the
  // values within the Material.
  PT(Material) temp = new Material(*mat);
  const Material *mp = MaterialPool::get_material(temp);

  node()->set_attrib(MaterialAttrib::make(mp), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_material_off
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using no material.  This is normally the default, but
//               it may be useful to use this to contradict
//               set_material() at a higher node level (or, with a
//               priority, to override a set_material() at a lower
//               level).
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_material_off(int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(MaterialAttrib::make_off(), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::clear_material
//       Access: Published
//  Description: Completely removes any material adjustment that may
//               have been set via set_material() from this particular
//               node.
////////////////////////////////////////////////////////////////////
void qpNodePath::
clear_material() {
  nassertv(!is_empty());
  node()->clear_attrib(MaterialAttrib::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_material
//       Access: Published
//  Description: Returns true if a material has been applied to this
//               particular node via set_material(), false otherwise.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
has_material() const {
  nassertr(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(MaterialAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const MaterialAttrib *ma = DCAST(MaterialAttrib, attrib);
    return !ma->is_off();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_material
//       Access: Published
//  Description: Returns the material that has been set on this
//               particular node, or NULL if no material has been set.
//               This is not necessarily the material that will be
//               applied to the geometry at or below this level, as
//               another material at a higher or lower level may
//               override.
//
//               This function returns a copy of the given material,
//               to allow changes, if desired.  Once changes are made,
//               they should be reapplied via set_material().
////////////////////////////////////////////////////////////////////
PT(Material) qpNodePath::
get_material() const {
  nassertr(!is_empty(), NULL);
  const RenderAttrib *attrib =
    node()->get_attrib(MaterialAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const MaterialAttrib *ma = DCAST(MaterialAttrib, attrib);
    return new Material(*ma->get_material());
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_fog
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using the indicated fog.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_fog(Fog *fog, int priority) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_fog_off
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using no fog.  This is normally the default, but
//               it may be useful to use this to contradict
//               set_fog() at a higher node level (or, with a
//               priority, to override a set_fog() at a lower
//               level).
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_fog_off(int priority) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::clear_fog
//       Access: Published
//  Description: Completely removes any fog adjustment that may
//               have been set via set_fog() or set_fog_off()
//               from this particular node.  This allows whatever
//               fogs might be otherwise affecting the geometry to
//               show instead.
////////////////////////////////////////////////////////////////////
void qpNodePath::
clear_fog() {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_fog
//       Access: Published
//  Description: Returns true if a fog has been applied to this
//               particular node via set_fog(), false otherwise.
//               This is not the same thing as asking whether the
//               geometry at this node will be rendered with
//               fog, as there may be a fog in effect from a higher or
//               lower level.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
has_fog() const {
  nassertr(false, false);
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_fog_off
//       Access: Published
//  Description: Returns true if a fog has been specifically
//               disabled on this particular node via
//               set_fog_off(), false otherwise.  This is not the
//               same thing as asking whether the geometry at this
//               node will be rendered unfogged, as there may be a
//               fog in effect from a higher or lower level.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
has_fog_off() const {
  nassertr(false, false);
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_fog
//       Access: Published
//  Description: Returns the fog that has been set on this
//               particular node, or NULL if no fog has been set.
//               This is not necessarily the fog that will be
//               applied to the geometry at or below this level, as
//               another fog at a higher or lower level may
//               override.
////////////////////////////////////////////////////////////////////
Fog *qpNodePath::
get_fog() const {
  nassertr(false, NULL);
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_render_mode_wireframe
//       Access: Published
//  Description: Sets up the geometry at this level and below (unless
//               overridden) to render in wireframe mode.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_render_mode_wireframe(int priority) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_render_mode_filled
//       Access: Published
//  Description: Sets up the geometry at this level and below (unless
//               overridden) to render in filled (i.e. not wireframe)
//               mode.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_render_mode_filled(int priority) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::clear_render_mode
//       Access: Published
//  Description: Completely removes any render mode adjustment that
//               may have been set on this node via
//               set_render_mode_wireframe() or
//               set_render_mode_filled().
////////////////////////////////////////////////////////////////////
void qpNodePath::
clear_render_mode() {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_render_mode
//       Access: Published
//  Description: Returns true if a render mode has been explicitly set
//               on this particular node via
//               set_render_mode_wireframe() or
//               set_render_mode_filled(), false otherwise.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
has_render_mode() const {
  nassertr(false, false);
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_two_sided
//       Access: Published
//  Description: Specifically sets or disables two-sided rendering
//               mode on this particular node.  If no other nodes
//               override, this will cause backfacing polygons to be
//               drawn (in two-sided mode, true) or culled (in
//               one-sided mode, false).
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_two_sided(bool two_sided, int priority) {
  nassertv_always(!is_empty());

  CullFaceAttrib::Mode mode =
    two_sided ?
    CullFaceAttrib::M_cull_none :
    CullFaceAttrib::M_cull_clockwise;

  node()->set_attrib(CullFaceAttrib::make(mode), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::clear_two_sided
//       Access: Published
//  Description: Completely removes any two-sided adjustment that
//               may have been set on this node via set_two_sided().
//               The geometry at this level and below will
//               subsequently be rendered either two-sided or
//               one-sided, according to whatever other nodes may have
//               had set_two_sided() on it, or according to the
//               initial state otherwise.
////////////////////////////////////////////////////////////////////
void qpNodePath::
clear_two_sided() {
  nassertv(!is_empty());
  node()->clear_attrib(CullFaceAttrib::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_two_sided
//       Access: Published
//  Description: Returns true if a two-sided adjustment has been
//               explicitly set on this particular node via
//               set_two_sided().  If this returns true, then
//               get_two_sided() may be called to determine which has
//               been set.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
has_two_sided() const {
  nassertr(!is_empty(), false);
  return node()->has_attrib(CullFaceAttrib::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_two_sided
//       Access: Published
//  Description: Returns true if two-sided rendering has been
//               specifically set on this node via set_two_sided(), or
//               false if one-sided rendering has been specifically
//               set, or if nothing has been specifically set.  See
//               also has_two_sided().  This does not necessarily
//               imply that the geometry will or will not be rendered
//               two-sided, as there may be other nodes that override.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
get_two_sided() const {
  nassertr(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(CullFaceAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const CullFaceAttrib *cfa = DCAST(CullFaceAttrib, attrib);
    return (cfa->get_mode() == CullFaceAttrib::M_cull_none);
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::do_billboard_axis
//       Access: Published
//  Description: Performs a billboard-type rotate to the indicated
//               camera node, one time only, and leaves the object
//               rotated.  This is similar in principle to heads_up().
//               However, it does lose both translate and scale
//               components of the matrix.
////////////////////////////////////////////////////////////////////
void qpNodePath::
do_billboard_axis(const qpNodePath &camera, float offset) {
  nassertv(!is_empty());

  qpNodePath parent = get_parent();
  LMatrix4f rel_mat = camera.get_mat(parent);

  LVector3f up = LVector3f::up();
  LVector3f rel_pos = -rel_mat.get_row3(3);

  LMatrix4f mat;
  ::heads_up(mat, rel_pos, up);

  // Also slide the geometry towards the camera according to the
  // offset factor.
  if (offset != 0.0f) {
    LVector3f translate = rel_mat.get_row3(3);
    translate.normalize();
    translate *= offset;
    mat.set_row(3, translate);
  }

  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::do_billboard_point_eye
//       Access: Published
//  Description: Performs a billboard-type rotate to the indicated
//               camera node, one time only, and leaves the object
//               rotated.  This is similar in principle to look_at(),
//               although the point_eye billboard effect cannot be
//               achieved using the ordinary look_at() call.
////////////////////////////////////////////////////////////////////
void qpNodePath::
do_billboard_point_eye(const qpNodePath &camera, float offset) {
  nassertv(!is_empty());

  qpNodePath parent = get_parent();
  LMatrix4f rel_mat = camera.get_mat(parent);

  LVector3f up = LVector3f::up() * rel_mat;
  LVector3f rel_pos = LVector3f::forward() * rel_mat;

  LMatrix4f mat;
  ::look_at(mat, rel_pos, up);

  // Also slide the geometry towards the camera according to the
  // offset factor.
  if (offset != 0.0f) {
    LVector3f translate = rel_mat.get_row3(3);
    translate.normalize();
    translate *= offset;
    mat.set_row(3, translate);
  }

  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::do_billboard_point_world
//       Access: Published
//  Description: Performs a billboard-type rotate to the indicated
//               camera node, one time only, and leaves the object
//               rotated.  This is similar in principle to look_at().
////////////////////////////////////////////////////////////////////
void qpNodePath::
do_billboard_point_world(const qpNodePath &camera, float offset) {
  nassertv(!is_empty());

  qpNodePath parent = get_parent();
  LMatrix4f rel_mat = camera.get_mat(parent);

  LVector3f up = LVector3f::up();
  LVector3f rel_pos = -rel_mat.get_row3(3);

  LMatrix4f mat;
  ::look_at(mat, rel_pos, up);

  // Also slide the geometry towards the camera according to the
  // offset factor.
  if (offset != 0.0f) {
    LVector3f translate = rel_mat.get_row3(3);
    translate.normalize();
    translate *= offset;
    mat.set_row(3, translate);
  }

  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_billboard_axis
//       Access: Published
//  Description: Puts a billboard transition on the node such that it
//               will rotate in two dimensions around the up axis.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_billboard_axis(float offset) {
  nassertv(!is_empty());
  CPT(RenderAttrib) billboard = BillboardAttrib::make
    (LVector3f::up(), false, true, 
     offset, qpNodePath(), LPoint3f(0.0f, 0.0f, 0.0f));
  node()->set_attrib(billboard);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_billboard_point_eye
//       Access: Published
//  Description: Puts a billboard transition on the node such that it
//               will rotate in three dimensions about the origin,
//               keeping its up vector oriented to the top of the
//               camera.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_billboard_point_eye(float offset) {
  nassertv(!is_empty());
  CPT(RenderAttrib) billboard = BillboardAttrib::make
    (LVector3f::up(), true, false,
     offset, qpNodePath(), LPoint3f(0.0f, 0.0f, 0.0f));
  node()->set_attrib(billboard);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_billboard_point_world
//       Access: Published
//  Description: Puts a billboard transition on the node such that it
//               will rotate in three dimensions about the origin,
//               keeping its up vector oriented to the sky.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_billboard_point_world(float offset) {
  nassertv(!is_empty());
  CPT(RenderAttrib) billboard = BillboardAttrib::make
    (LVector3f::up(), false, false,
     offset, qpNodePath(), LPoint3f(0.0f, 0.0f, 0.0f));
  node()->set_attrib(billboard);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::clear_billboard
//       Access: Published
//  Description: Removes any billboard attributes from the node.
////////////////////////////////////////////////////////////////////
void qpNodePath::
clear_billboard() {
  nassertv(!is_empty());
  node()->clear_attrib(BillboardAttrib::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_billboard
//       Access: Published
//  Description: Returns true if there is any billboard attribute on
//               the node.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
has_billboard() const {
  nassertr(!is_empty(), false);
  return node()->has_attrib(BillboardAttrib::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::set_transparency
//       Access: Published
//  Description: Specifically sets or disables transparent rendering
//               mode on this particular node.  If no other nodes
//               override, this will cause items with a non-1 value
//               for alpha color to be rendered partially transparent.
////////////////////////////////////////////////////////////////////
void qpNodePath::
set_transparency(bool transparency, int priority) {
  nassertv_always(!is_empty());

  TransparencyAttrib::Mode mode =
    transparency ?
    TransparencyAttrib::M_alpha :
    TransparencyAttrib::M_none;

  node()->set_attrib(TransparencyAttrib::make(mode), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::clear_transparency
//       Access: Published
//  Description: Completely removes any transparency adjustment that
//               may have been set on this node via set_transparency().
//               The geometry at this level and below will
//               subsequently be rendered either transparent or not,
//               to whatever other nodes may have had
//               set_transparency() on them.
////////////////////////////////////////////////////////////////////
void qpNodePath::
clear_transparency() {
  nassertv(!is_empty());
  node()->clear_attrib(TransparencyAttrib::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::has_transparency
//       Access: Published
//  Description: Returns true if a transparent-rendering adjustment
//               has been explicitly set on this particular node via
//               set_transparency().  If this returns true, then
//               get_transparency() may be called to determine whether
//               transparency has been explicitly enabled or
//               explicitly disabled for this node.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
has_transparency() const {
  nassertr(!is_empty(), false);
  return node()->has_attrib(TransparencyAttrib::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_transparency
//       Access: Published
//  Description: Returns true if transparent rendering has been
//               specifically set on this node via set_transparency(), or
//               false if nontransparent rendering has been specifically
//               set, or if nothing has been specifically set.  See
//               also has_transparency().  This does not necessarily
//               imply that the geometry will or will not be rendered
//               transparent, as there may be other nodes that override.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
get_transparency() const {
  nassertr(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(TransparencyAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const TransparencyAttrib *cfa = DCAST(TransparencyAttrib, attrib);
    return (cfa->get_mode() != TransparencyAttrib::M_none);
  }

  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_hidden_ancestor
//       Access: Published
//  Description: Returns the NodePath at or above the referenced node
//               that is hidden to the indicated camera(s), or an
//               empty NodePath if no ancestor of the referenced node
//               is hidden (and the node should be visible).
////////////////////////////////////////////////////////////////////
qpNodePath qpNodePath::
get_hidden_ancestor(DrawMask camera_mask) const {
  qpNodePathComponent *comp;
  for (comp = _head; 
       comp != (qpNodePathComponent *)NULL; 
       comp = comp->get_next()) {
    PandaNode *node = comp->get_node();
    if ((node->get_draw_mask() & camera_mask).is_zero()) {
      qpNodePath result;
      result._head = comp;
      return result;
    }
  }

  return not_found();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::get_stashed_ancestor
//       Access: Published
//  Description: Returns the NodePath at or above the referenced node
//               that is stashed, or an empty NodePath if no ancestor
//               of the referenced node is stashed (and the node should
//               be visible).
////////////////////////////////////////////////////////////////////
qpNodePath qpNodePath::
get_stashed_ancestor() const {
  qpNodePathComponent *comp = _head;
  if (comp != (qpNodePathComponent *)NULL) {
    qpNodePathComponent *next = comp->get_next();

    while (next != (qpNodePathComponent *)NULL) {
      PandaNode *node = comp->get_node();
      PandaNode *parent_node = next->get_node();

      if (parent_node->find_stashed(node) >= 0) {
        qpNodePath result;
        result._head = comp;
        return result;
      }

      comp = next;
      next = next->get_next();
    }
  }

  return not_found();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::verify_complete
//       Access: Published
//  Description: Returns true if all of the nodes described in the
//               qpNodePath are connected and the top node is the top
//               of the graph, or false otherwise.
////////////////////////////////////////////////////////////////////
bool qpNodePath::
verify_complete() const {
  if (is_empty()) {
    return true;
  }

  uncollapse_head();
  const qpNodePathComponent *comp = _head;
  nassertr(comp != (const qpNodePathComponent *)NULL, false);

  PandaNode *node = comp->get_node();
  nassertr(node != (const PandaNode *)NULL, false);
  int length = comp->get_length();

  comp = comp->get_next();
  length--;
  while (comp != (const qpNodePathComponent *)NULL) {
    PandaNode *next_node = comp->get_node();
    nassertr(next_node != (const PandaNode *)NULL, false);

    if (node->find_parent(next_node) < 0) {
      return false;
    }

    if (comp->get_length() != length) {
      return false;
    }

    node = next_node;
    comp = comp->get_next();
    length--;
  }

  // Now that we've reached the top, we should have no parents.
  return length == 0 && node->get_num_parents() == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::uncollapse_head
//       Access: Private
//  Description: Quietly and transparently uncollapses the _head
//               pointer if it needs it.
////////////////////////////////////////////////////////////////////
void qpNodePath::
uncollapse_head() const {
  if (_head != (qpNodePathComponent *)NULL && _head->is_collapsed()) {
    ((qpNodePath *)this)->_head = _head->uncollapse();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::find_common_ancestor
//       Access: Private, Static
//  Description: Walks up from both qpNodePaths to find the first node
//               that both have in common, if any.  Fills a_count and
//               b_count with the number of nodes below the common
//               node in each path.
////////////////////////////////////////////////////////////////////
void qpNodePath::
find_common_ancestor(const qpNodePath &a, const qpNodePath &b,
                     int &a_count, int &b_count) {
  nassertv(!a.is_empty() && !b.is_empty());
  a.uncollapse_head();
  b.uncollapse_head();

  qpNodePathComponent *ac = a._head;
  qpNodePathComponent *bc = b._head;
  a_count = 0;
  b_count = 0;

  // Shorten up the longer one until they are the same length.
  while (ac->get_length() > bc->get_length()) {
    nassertv(ac != (qpNodePathComponent *)NULL);
    ac = ac->get_next();
    a_count++;
  }
  while (bc->get_length() > ac->get_length()) {
    nassertv(bc != (qpNodePathComponent *)NULL);
    bc = bc->get_next();
    b_count++;
  }

  // Now shorten them both up until we reach the same component.
  while (ac != bc) {
    // These shouldn't go to NULL unless they both go there together. 
    nassertv(ac != (qpNodePathComponent *)NULL);
    nassertv(bc != (qpNodePathComponent *)NULL);
    ac = ac->get_next();
    a_count++;
    bc = bc->get_next();
    b_count++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::r_get_net_state
//       Access: Private
//  Description: Recursively determines the net state chnages to the
//               indicated component node from the root of the graph.
////////////////////////////////////////////////////////////////////
CPT(RenderState) qpNodePath::
r_get_net_state(qpNodePathComponent *comp) const {
  if (comp == (qpNodePathComponent *)NULL) {
    return RenderState::make_empty();
  } else {
    CPT(RenderState) state = comp->get_node()->get_state();
    return r_get_net_state(comp->get_next())->compose(state);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::r_get_partial_state
//       Access: Private
//  Description: Recursively determines the net state changes to the
//               indicated component node from the nth node above it.
////////////////////////////////////////////////////////////////////
CPT(RenderState) qpNodePath::
r_get_partial_state(qpNodePathComponent *comp, int n) const {
  nassertr(comp != (qpNodePathComponent *)NULL, RenderState::make_empty());
  if (n == 0) {
    return RenderState::make_empty();
  } else {
    CPT(RenderState) state = comp->get_node()->get_state();
    return r_get_partial_state(comp->get_next(), n - 1)->compose(state);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::r_get_net_transform
//       Access: Private
//  Description: Recursively determines the net transform to the
//               indicated component node from the root of the graph.
////////////////////////////////////////////////////////////////////
CPT(TransformState) qpNodePath::
r_get_net_transform(qpNodePathComponent *comp) const {
  if (comp == (qpNodePathComponent *)NULL) {
    return TransformState::make_identity();
  } else {
    CPT(TransformState) transform = comp->get_node()->get_transform();
    return r_get_net_transform(comp->get_next())->compose(transform);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::r_get_partial_transform
//       Access: Private
//  Description: Recursively determines the net transform to the
//               indicated component node from the nth node above it.
////////////////////////////////////////////////////////////////////
CPT(TransformState) qpNodePath::
r_get_partial_transform(qpNodePathComponent *comp, int n) const {
  nassertr(comp != (qpNodePathComponent *)NULL, TransformState::make_identity());
  if (n == 0) {
    return TransformState::make_identity();
  } else {
    CPT(TransformState) transform = comp->get_node()->get_transform();
    return r_get_partial_transform(comp->get_next(), n - 1)->compose(transform);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::r_output
//       Access: Private
//  Description: The recursive implementation of output(), this writes
//               the names of each node component in order from
//               beginning to end, by first walking to the end of the
//               linked list and then outputting from there.
////////////////////////////////////////////////////////////////////
void qpNodePath::
r_output(ostream &out, qpNodePathComponent *comp) const {
  PandaNode *node = comp->get_node();
  qpNodePathComponent *next = comp->get_next();
  if (next != (qpNodePathComponent *)NULL) {
    // This is not the head of the list; keep going up.
    r_output(out, next);
    out << "/";

    PandaNode *parent_node = next->get_node();
    if (parent_node->find_stashed(node) >= 0) {
      // The node is stashed.
      out << "@@";

    } else if (node->find_parent(parent_node) < 0) {
      // Oops, there's an error.  This shouldn't happen.
      out << ".../";
    }
  }

  // Now output this component.
  if (node->has_name()) {
    out << node->get_name();
  } else {
    out << "-" << node->get_type();
  }
  //  out << "[" << comp->get_length() << "]";
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::find_matches
//       Access: Private
//  Description: Finds up to max_matches matches against the given
//               path string from this node and deeper.  The
//               max_matches count indicates the maximum number of
//               matches to return, or -1 not to limit the number
//               returned.
////////////////////////////////////////////////////////////////////
void qpNodePath::
find_matches(qpNodePathCollection &result, const string &path,
             int max_matches) const {
  if (is_empty()) {
    pgraph_cat.warning()
      << "Attempt to extend an empty qpNodePath by '" << path
      << "'.\n";
    return;
  }
  qpFindApproxPath approx_path;
  if (approx_path.add_string(path)) {
    find_matches(result, approx_path, max_matches);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::find_matches
//       Access: Private
//  Description: Finds up to max_matches matches against the given
//               approx_path from this node and deeper.  The
//               max_matches count indicates the maximum number of
//               matches to return, or -1 not to limit the number
//               returned.
////////////////////////////////////////////////////////////////////
void qpNodePath::
find_matches(qpNodePathCollection &result, qpFindApproxPath &approx_path,
             int max_matches) const {
  if (is_empty()) {
    pgraph_cat.warning()
      << "Attempt to extend an empty qpNodePath by: " << approx_path << ".\n";
    return;
  }
  qpFindApproxLevelEntry start(*this, approx_path);
  qpFindApproxLevel level;
  level.add_entry(start);
  r_find_matches(result, level, max_matches, _max_search_depth);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::r_find_matches
//       Access: Private
//  Description: The recursive implementation of find_matches.
////////////////////////////////////////////////////////////////////
void qpNodePath::
r_find_matches(qpNodePathCollection &result,
               const qpFindApproxLevel &level,
               int max_matches, int num_levels_remaining) const {
  // Go on to the next level.  If we exceeded the requested maximum
  // depth, stop.
  if (num_levels_remaining <= 0) {
    return;
  }
  num_levels_remaining--;

  qpFindApproxLevel next_level;
  bool okflag = true;

  // For each node in the current level, build up the set of possible
  // matches in the next level.
  qpFindApproxLevel::Vec::const_iterator li;
  for (li = level._v.begin(); li != level._v.end() && okflag; ++li) {
    const qpFindApproxLevelEntry &entry = (*li);

    if (entry.is_solution()) {
      // Does this entry already represent a solution?
      result.add_path(entry._node_path);
    } else {
      entry.consider_node(result, next_level, max_matches);
    }

    if (max_matches > 0 && result.get_num_paths() >= max_matches) {
      // Really, we just want to return here.  But returning from
      // within the conditional within the for loop seems to sometimes
      // cause a compiler fault in GCC.  We'll use a semaphore
      // variable instead.
      okflag = false;
    }
  }

  // Now recurse on the next level.
  if (okflag) {
    r_find_matches(result, next_level, max_matches, num_levels_remaining);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePath::r_adjust_all_priorities
//       Access: Private
//  Description: The recursive implementation of
//               adjust_all_priorities().  This walks through the
//               subgraph defined by the indicated node and below.
////////////////////////////////////////////////////////////////////
void qpNodePath::
r_adjust_all_priorities(PandaNode *node, int adjustment) {
  node->set_state(node->get_state()->adjust_all_priorities(adjustment));

  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    r_adjust_all_priorities(cr.get_child(i), adjustment);
  }
}
