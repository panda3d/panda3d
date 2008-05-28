// Filename: eggVertexPool.I
// Created by:  drose (16Jan99)
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
//     Function: EggVertexPool::has_vertex
//       Access: Public
//  Description: Returns true if the indicated vertex has been defined
//               in the vertex pool, false otherwise.  This does not
//               include forward references.
////////////////////////////////////////////////////////////////////
INLINE bool EggVertexPool::
has_vertex(int index) const {
  return get_vertex(index) != (EggVertex *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::indexing operator
//       Access: Public
//  Description: Returns the vertex in the pool with the indicated
//               index number, or NULL if no vertices have that index
//               number.
////////////////////////////////////////////////////////////////////
INLINE EggVertex *EggVertexPool::
operator [](int index) const {
  return get_vertex(index);
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::make_new_vertex()
//       Access: Public
//  Description: Allocates and returns a new vertex from the pool.
//               This is one of three ways to add new vertices to a
//               vertex pool.
////////////////////////////////////////////////////////////////////
INLINE EggVertex *EggVertexPool::
make_new_vertex() {
  PT(EggVertex) vertex = new EggVertex;
  return add_vertex(vertex);
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::make_new_vertex()
//       Access: Public
//  Description: Allocates and returns a new vertex from the pool.
//               This is one of three ways to add new vertices to a
//               vertex pool.
//
//               This flavor of make_new_vertex() explicitly sets the
//               vertex position as it is allocated.  It does not
//               attempt to share vertices.
////////////////////////////////////////////////////////////////////
INLINE EggVertex *EggVertexPool::
make_new_vertex(double pos) {
  EggVertex *vertex = make_new_vertex();
  vertex->set_pos(pos);
  return vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::make_new_vertex()
//       Access: Public
//  Description: Allocates and returns a new vertex from the pool.
//               This is one of three ways to add new vertices to a
//               vertex pool.
//
//               This flavor of make_new_vertex() explicitly sets the
//               vertex position as it is allocated.  It does not
//               attempt to share vertices.
////////////////////////////////////////////////////////////////////
INLINE EggVertex *EggVertexPool::
make_new_vertex(const LPoint2d &pos) {
  EggVertex *vertex = make_new_vertex();
  vertex->set_pos(pos);
  return vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::make_new_vertex()
//       Access: Public
//  Description: Allocates and returns a new vertex from the pool.
//               This is one of three ways to add new vertices to a
//               vertex pool.
//
//               This flavor of make_new_vertex() explicitly sets the
//               vertex position as it is allocated.  It does not
//               attempt to share vertices.
////////////////////////////////////////////////////////////////////
INLINE EggVertex *EggVertexPool::
make_new_vertex(const LPoint3d &pos) {
  EggVertex *vertex = make_new_vertex();
  vertex->set_pos(pos);
  return vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::make_new_vertex()
//       Access: Public
//  Description: Allocates and returns a new vertex from the pool.
//               This is one of three ways to add new vertices to a
//               vertex pool.
//
//               This flavor of make_new_vertex() explicitly sets the
//               vertex position as it is allocated.  It does not
//               attempt to share vertices.
////////////////////////////////////////////////////////////////////
INLINE EggVertex *EggVertexPool::
make_new_vertex(const LPoint4d &pos) {
  EggVertex *vertex = make_new_vertex();
  vertex->set_pos(pos);
  return vertex;
}
