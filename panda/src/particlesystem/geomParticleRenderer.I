// Filename: geomParticleRenderer.I
// Created by:  charles (05Jul00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//  Function : set_geom_node
//    Access : public
////////////////////////////////////////////////////////////////////

// we're forcing a pool resize to remove every arc in the vector.
// this is because arcs are reference-counted, and we have to
// explicitly remove them from the node they were previously
// pointing at.

INLINE void GeomParticleRenderer::
set_geom_node(Node *node) {
  _geom_node = node;
  resize_pool(_pool_size);
}

////////////////////////////////////////////////////////////////////
//  Function : get_geom_node
//    Access : public
////////////////////////////////////////////////////////////////////

INLINE Node *GeomParticleRenderer::
get_geom_node(void) {
  return _geom_node;
}

////////////////////////////////////////////////////////////////////
//  Function : kill_arcs
//    Access : private
////////////////////////////////////////////////////////////////////

INLINE void GeomParticleRenderer::
kill_arcs(void) {
  vector< PT(RenderRelation) >::iterator vec_iter = _arc_vector.begin();

  for (; vec_iter != _arc_vector.end(); vec_iter++)
    remove_arc(*vec_iter);

  _arc_vector.erase(_arc_vector.begin(), _arc_vector.end());
}
