// Filename: eggVertexPool.cxx
// Created by:  drose (16Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "eggVertexPool.h"
#include "eggPrimitive.h"
#include "eggUtilities.h"

#include <indent.h>

TypeHandle EggVertexPool::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggVertexPool::
EggVertexPool(const string &name) : EggNode(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::Copy Constructor
//       Access: Public
//  Description: Copying a vertex pool is of questionable value, since
//               it will copy all of the vertices and assign new
//               pointers to them all. There will be no polygons
//               referring to the new vertices.
////////////////////////////////////////////////////////////////////
EggVertexPool::
EggVertexPool(const EggVertexPool &copy) : EggNode(copy) {
  iterator i;
  for (i = copy.begin(); i != copy.end(); ++i) {
    add_vertex(new EggVertex(*(*i)), (*i)->get_index());
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggVertexPool::
~EggVertexPool() {
  // Remove all vertices from the pool when it destructs.

  // Sanity check.
  nassertv(_index_vertices.size() == _unique_vertices.size());

  iterator i;
  for (i = begin(); i != end(); ++i) {
    // Sanity checks on our internal data structures.
    nassertv((*i)->_pool == this);
    nassertv(get_vertex((*i)->_index) == (*i));

    (*i)->_pool = NULL;
    (*i)->_index = -1;
  }

  _index_vertices.erase(_index_vertices.begin(), _index_vertices.end());
  _unique_vertices.erase(_unique_vertices.begin(), _unique_vertices.end());
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::add_vertex
//       Access: Public
//  Description: Adds the indicated vertex to the pool.  It is an
//               error if the vertex is already a member of this or
//               any other pool.  The vertex must have been allocated
//               from the free store; its pointer will now be owned by
//               the vertex pool.  If the index number is supplied,
//               tries to assign that index number; it is an error if
//               the index number is already in use.
////////////////////////////////////////////////////////////////////
void EggVertexPool::
add_vertex(PT(EggVertex) vertex, int index) {
  // Don't try to add a vertex while it still belongs to another pool.
  nassertv(vertex->_pool == NULL);

  if (index == -1) {
    index = get_highest_index() + 1;
  }
  // Always supply an index number >= 0.
  nassertv(index >= 0);

  // Don't try to duplicate index numbers within a vertex pool.
  nassertv(_index_vertices.find(index) == _index_vertices.end());

  _unique_vertices.insert(vertex);
  _index_vertices[index] = vertex;

  vertex->_pool = this;
  vertex->_index = index;
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::create_unique_vertex
//       Access: Public
//  Description: Creates a new vertex in the pool that is a copy of
//               the indicated one and returns it.  If there is
//               already a vertex in the pool like the indicated one,
//               simply returns that one.
////////////////////////////////////////////////////////////////////
EggVertex *EggVertexPool::
create_unique_vertex(const EggVertex &copy) {
  UniqueVertices::iterator uvi;
  uvi = _unique_vertices.find((EggVertex *)&copy);

  if (uvi != _unique_vertices.end()) {
    // There was already such a vertex.  Return it.
    return (*uvi);
  }

  // Create a new vertex.
  EggVertex *vtx = new EggVertex(copy);
  add_vertex(vtx);
  return vtx;
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::remove_vertex
//       Access: Public
//  Description: Removes the vertex from the pool.  It is an error if
//               the vertex is not already a member of the pool.
////////////////////////////////////////////////////////////////////
void EggVertexPool::
remove_vertex(EggVertex *vertex) {
  // Make sure the vertex is already a member of this pool.
  nassertv(vertex->_pool == this);

  // Sanity check.  Is the vertex actually in the pool?
  nassertv(get_vertex(vertex->_index) == vertex);

  // Removing the vertex from the indexed list is simple.
  _index_vertices.erase(vertex->_index);

  // Removing the vertex from the unique list is a bit trickier--there
  // might be several other vertices that are considered identical to
  // this one, and so we have to walk through all the identical
  // vertices until we find the right one.
  UniqueVertices::iterator uvi;
  uvi = _unique_vertices.find(vertex);

  // Sanity check.  Is the vertex actually in the pool?
  nassertv(uvi != _unique_vertices.end());

  while ((*uvi) != vertex) {
    ++uvi;
    // Sanity check.  Is the vertex actually in the pool?
    nassertv(uvi != _unique_vertices.end());
  }

  _unique_vertices.erase(uvi);

  vertex->_pool = NULL;
}

// A function object for split_vertex(), used in transform(), below.
class IsLocalVertexSplitter {
public:
  int operator () (const EggPrimitive *prim) const {
    return (prim->is_local_coord() ? 1 : 0);
  }
};
  
////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::transform
//       Access: Public
//  Description: Applies the indicated transformation matrix to all
//               the vertices.  However, vertices that are attached to
//               primitives that believe their vertices are in a local
//               coordinate system are transformed only by the scale
//               and rotation component.  If a vertex happens to be
//               attached both to a local and a global primitive, and
//               the transformation includes a translation component,
//               the vertex will be split.
////////////////////////////////////////////////////////////////////
void EggVertexPool::
transform(const LMatrix4d &mat) {
  LVector3d translation = mat.get_row3(3);

  if (translation == LVector3d(0.0, 0.0, 0.0)) {
    // If the matrix does not have a translation component, we can
    // treat the local and global vertices the same.  This makes
    // things much easier.
    iterator i;
    for (i = begin(); i != end(); ++i) {
      EggVertex *vert = *i;
      vert->transform(mat);
    }

  } else {
    // The matrix does have a translation component.  That means we
    // have to treat the global and local vertices differently.
    // Yucky.

    // First, transform the global vertices.  Get a copy of the list
    // of vertices in this pool.  We must have a copy because we might
    // be modifying the list as we traverse it.
    
    typedef vector<EggVertex *> Verts;
    Verts verts;
    verts.reserve(size());
    copy(begin(), end(), back_inserter(verts));
    
    Verts::const_iterator vi;
    for (vi = verts.begin(); vi != verts.end(); ++vi) {
      EggVertex *vert = *vi;
      int num_local_coord = vert->get_num_local_coord();
      int num_global_coord = vert->get_num_global_coord();
      
      if (num_global_coord != 0) {
	// This vertex will be transformed.
	if (num_local_coord != 0) {
	  // It also needs to be split!  Yuck.
	  split_vertex(vert, IsLocalVertexSplitter());
	}
	
	vert->transform(mat);
      }
    }
    
    // Now transform the local vertices.  We can walk through the list
    // directly now, because we won't be modifying the list this time.
    LMatrix4d local_mat = mat;
    local_mat.set_row(3, LVector3d(0.0, 0.0, 0.0));
    
    iterator i;
    for (i = begin(); i != end(); ++i) {
      EggVertex *vert = *i;
      if (vert->get_num_local_coord() != 0) {

	// This should be guaranteed by the vertex-splitting logic
	// above.
	nassertv(vert->get_num_global_coord() == 0);
	vert->transform(local_mat);
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::write
//       Access: Public
//  Description: Writes the vertex pool to the indicated output stream
//               in Egg format.
////////////////////////////////////////////////////////////////////
void EggVertexPool::
write(ostream &out, int indent_level) const {
  write_header(out, indent_level, "<VertexPool>");

  iterator i;
  for (i = begin(); i != end(); ++i) {
    (*i)->write(out, indent_level+2);
  }

  indent(out, indent_level)
    << "}\n";
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::r_transform
//       Access: Protected, Virtual
//  Description: This is called from within the egg code by
//               transform().  It applies a transformation matrix
//               to the current node in some sensible way, then
//               continues down the tree.
//
//               The first matrix is the transformation to apply; the
//               second is its inverse.  The third parameter is the
//               coordinate system we are changing to, or CS_default
//               if we are not changing coordinate systems.
////////////////////////////////////////////////////////////////////
void EggVertexPool::
r_transform(const LMatrix4d &mat, const LMatrix4d &, CoordinateSystem) {
  transform(mat);
}
