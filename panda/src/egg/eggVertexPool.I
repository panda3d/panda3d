// Filename: eggVertexPool.I
// Created by:  drose (16Jan99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::get_vertex
//       Access: Public
//  Description: Returns the vertex in the pool with the indicated
//               index number, or NULL if no vertices have that index
//               number.
////////////////////////////////////////////////////////////////////
INLINE EggVertex *EggVertexPool::
get_vertex(int index) const {
  IndexVertices::const_iterator ivi = _index_vertices.find(index);

  if (ivi == _index_vertices.end()) {
    return NULL;
  } else {
    return (*ivi).second;
  }
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
//     Function: EggVertexPool::get_highest_index
//       Access: Public
//  Description: Returns the highest index number used by any vertex
//               in the pool.
////////////////////////////////////////////////////////////////////
INLINE int EggVertexPool::
get_highest_index() const {
  if (_index_vertices.empty()) {
    return 0;
  }
  IndexVertices::const_reverse_iterator ivi = _index_vertices.rbegin();
  nassertr((*ivi).first == (*ivi).second->get_index(), 0);
  return (*ivi).first;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::begin()
//       Access: Public
//  Description: Returns an iterator that can be used to traverse
//               through all the vertices in the pool.
////////////////////////////////////////////////////////////////////
INLINE EggVertexPool::iterator EggVertexPool::
begin() const {
  nassertr(_index_vertices.size() == _unique_vertices.size(),
	   iterator(_index_vertices.begin()));
  return iterator(_index_vertices.begin());
}
 
////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::end()
//       Access: Public
//  Description: Returns an iterator that can be used to traverse
//               through all the vertices in the pool.
////////////////////////////////////////////////////////////////////
INLINE EggVertexPool::iterator EggVertexPool::
end() const {
  return iterator(_index_vertices.end());
}
 
////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::empty()
//       Access: Public
//  Description: Returns true if the pool is empty.
////////////////////////////////////////////////////////////////////
INLINE bool EggVertexPool::
empty() const {
  return _index_vertices.empty();
}
 
////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::size()
//       Access: Public
//  Description: Returns the number of vertices in the pool.
////////////////////////////////////////////////////////////////////
INLINE EggVertexPool::size_type EggVertexPool::
size() const {
  nassertr(_index_vertices.size() == _unique_vertices.size(), 0);
  return _index_vertices.size();
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
  add_vertex(vertex);
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
