// Filename: eggVertexPool.I
// Created by:  drose (16Jan99)
// 
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

INLINE EggVertex *EggVertexPool::
operator [](int index) const {
  return get_vertex(index);
}

INLINE int EggVertexPool::
get_highest_index() const {
  if (_index_vertices.empty()) {
    return 0;
  }
  IndexVertices::const_reverse_iterator ivi = _index_vertices.rbegin();
  nassertr((*ivi).first == (*ivi).second->get_index(), 0);
  return (*ivi).first;
}

INLINE EggVertexPool::iterator EggVertexPool::
begin() const {
  nassertr(_index_vertices.size() == _unique_vertices.size(),
	   iterator(_index_vertices.begin()));
  return iterator(_index_vertices.begin());
}
 
INLINE EggVertexPool::iterator EggVertexPool::
end() const {
  return iterator(_index_vertices.end());
}
 
INLINE bool EggVertexPool::
empty() const {
  return _index_vertices.empty();
}
 
INLINE EggVertexPool::size_type EggVertexPool::
size() const {
  nassertr(_index_vertices.size() == _unique_vertices.size(), 0);
  return _index_vertices.size();
}
