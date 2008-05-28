// Filename: eggNode.I
// Created by:  drose (10Feb99)
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
//     Function: EggNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggNode::
EggNode(const string &name) : EggNamedObject(name) {
  _parent = NULL;
  _depth = 0;
  _under_flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggNode::
EggNode(const EggNode &copy) : EggNamedObject(copy) {
  _parent = NULL;
  _depth = 0;
  _under_flags = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: EggNode::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggNode &EggNode::
operator = (const EggNode &copy) {
  EggNamedObject::operator = (copy);
  update_under(0);
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_parent
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggGroupNode *EggNode::
get_parent() const {
  return _parent;
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_depth
//       Access: Public
//  Description: Returns the number of nodes above this node in the
//               egg hierarchy.
////////////////////////////////////////////////////////////////////
INLINE int EggNode::
get_depth() const {
  return _depth;
}


////////////////////////////////////////////////////////////////////
//     Function: EggNode::is_under_instance
//       Access: Public
//  Description: Returns true if there is an <Instance> node somewhere
//               in the egg tree at or above this node, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggNode::
is_under_instance() const {
  return (_under_flags & UF_under_instance) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::is_under_transform
//       Access: Public
//  Description: Returns true if there is a <Transform> entry somewhere
//               in the egg tree at or above this node, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggNode::
is_under_transform() const {
  return (_under_flags & UF_under_transform) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::is_local_coord
//       Access: Public
//  Description: Returns true if this node's vertices are not in the
//               global coordinate space.  This will be the case if
//               there was an <Instance> node under a transform at or
//               above this node.
////////////////////////////////////////////////////////////////////
INLINE bool EggNode::
is_local_coord() const {
  return (_under_flags & UF_local_coord) != 0;
}


////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_vertex_frame
//       Access: Public
//  Description: Returns the coordinate frame of the vertices
//               referenced by primitives at or under this node.  This
//               is not the same as get_node_frame().
//
//               Generally, vertices in an egg file are stored in the
//               global coordinate space, regardless of the transforms
//               defined at each node.  Thus, get_vertex_frame() will
//               usually return the identity transform (global
//               coordinate space).  However, primitives under an
//               <Instance> entry reference their vertices in the
//               coordinate system under effect at the time of the
//               <Instance>.  Thus, nodes under an <Instance> entry
//               may return this non-identity matrix.
//
//               Specifically, this may return a non-identity matrix
//               only if is_local_coord() is true.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d &EggNode::
get_vertex_frame() const {
  if (_vertex_frame == (LMatrix4d *)NULL) {
    return LMatrix4d::ident_mat();
  } else {
    return *_vertex_frame;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_node_frame
//       Access: Public
//  Description: Returns the coordinate frame of the node itself.
//               This is simply the net product of all transformations
//               up to the root.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d &EggNode::
get_node_frame() const {
  if (_node_frame == (LMatrix4d *)NULL) {
    return LMatrix4d::ident_mat();
  } else {
    return *_node_frame;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_vertex_frame_inv
//       Access: Public
//  Description: Returns the inverse of the matrix returned by
//               get_vertex_frame().  See get_vertex_frame().
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d &EggNode::
get_vertex_frame_inv() const {
  if (_vertex_frame_inv == (LMatrix4d *)NULL) {
    return LMatrix4d::ident_mat();
  } else {
    return *_vertex_frame_inv;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_node_frame_inv
//       Access: Public
//  Description: Returns the inverse of the matrix returned by
//               get_node_frame().  See get_node_frame().
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d &EggNode::
get_node_frame_inv() const {
  if (_node_frame_inv == (LMatrix4d *)NULL) {
    return LMatrix4d::ident_mat();
  } else {
    return *_node_frame_inv;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_vertex_to_node
//       Access: Public
//  Description: Returns the transformation matrix suitable for
//               converting the vertices as read from the egg file
//               into the coordinate space of the node.  This is the
//               same thing as:
//
//               get_vertex_frame() * get_node_frame_inv()
//
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d &EggNode::
get_vertex_to_node() const {
  if (_vertex_to_node == (LMatrix4d *)NULL) {
    return LMatrix4d::ident_mat();
  } else {
    return *_vertex_to_node;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_node_to_vertex
//       Access: Public
//  Description: Returns the transformation matrix suitable for
//               converting vertices in the coordinate space of the
//               node to the appropriate coordinate space for storing
//               in the egg file.  This is the same thing as:
//
//               get_node_frame() * get_vertex_frame_inv()
//
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d &EggNode::
get_node_to_vertex() const {
  if (_node_to_vertex == (LMatrix4d *)NULL) {
    return LMatrix4d::ident_mat();
  } else {
    return *_node_to_vertex;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_vertex_frame_ptr
//       Access: Public
//  Description: Returns either a NULL pointer or a unique pointer
//               shared by nodes with the same get_vertex_frame()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d *EggNode::
get_vertex_frame_ptr() const {
  return _vertex_frame;
}


////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_node_frame_ptr
//       Access: Public
//  Description: Returns either a NULL pointer or a unique pointer
//               shared by nodes with the same get_node_frame()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d *EggNode::
get_node_frame_ptr() const {
  return _node_frame;
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_vertex_frame_inv_ptr
//       Access: Public
//  Description: Returns either a NULL pointer or a unique pointer
//               shared by nodes with the same get_vertex_frame_inv()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d *EggNode::
get_vertex_frame_inv_ptr() const {
  return _vertex_frame_inv;
}


////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_node_frame_inv_ptr
//       Access: Public
//  Description: Returns either a NULL pointer or a unique pointer
//               shared by nodes with the same get_node_frame_inv()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d *EggNode::
get_node_frame_inv_ptr() const {
  return _node_frame_inv;
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_vertex_to_node_ptr
//       Access: Public
//  Description: Returns either a NULL pointer or a unique pointer
//               shared by nodes with the same get_vertex_to_node()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d *EggNode::
get_vertex_to_node_ptr() const {
  return _vertex_to_node;
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::get_node_to_vertex_ptr
//       Access: Public
//  Description: Returns either a NULL pointer or a unique pointer
//               shared by nodes with the same get_node_to_vertex()
//               matrix.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d *EggNode::
get_node_to_vertex_ptr() const {
  return _node_to_vertex;
}


////////////////////////////////////////////////////////////////////
//     Function: EggNode::transform
//       Access: Public
//  Description: Applies the indicated transformation to the node and
//               all of its descendants.
////////////////////////////////////////////////////////////////////
INLINE void EggNode::
transform(const LMatrix4d &mat) {
  LMatrix4d inv = invert(mat);

  r_transform(mat, inv, CS_default);
  r_transform_vertices(mat);

  // Now we have to recompute the under_flags to ensure that all the
  // cached relative matrices are correct.
  update_under(0);
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::transform_vertices_only
//       Access: Public
//  Description: Applies the indicated transformation only to vertices
//               that appear in global space within vertex pools at
//               this node and below.  Joints and other transforms are
//               not affected, nor are local vertices.
////////////////////////////////////////////////////////////////////
INLINE void EggNode::
transform_vertices_only(const LMatrix4d &mat) {
  r_transform_vertices(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::flatten_transforms
//       Access: Public
//  Description: Removes any transform and instance records from this
//               node in the scene graph and below.  If an instance
//               node is encountered, removes the instance and applies
//               the transform to its vertices, duplicating vertices
//               if necessary.
//
//               Since this function may result in duplicated
//               vertices, it may be a good idea to call
//               remove_unused_vertices() after calling this.
////////////////////////////////////////////////////////////////////
INLINE void EggNode::
flatten_transforms() {
  r_flatten_transforms();
  update_under(0);
}
