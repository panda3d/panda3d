// Filename: eggVertexPool.cxx
// Created by:  drose (16Jan99)
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

#include "eggVertexPool.h"
#include "eggPrimitive.h"
#include "eggUtilities.h"

#include "indent.h"

TypeHandle EggVertexPool::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggVertexPool::
EggVertexPool(const string &name) : EggNode(name) {
  _highest_index = -1;
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

  IndexVertices::iterator ivi;
  for (ivi = _index_vertices.begin(); ivi != _index_vertices.end(); ++ivi) {
    int index = (*ivi).first;
    EggVertex *vertex = (*ivi).second;

    // Sanity checks on our internal data structures.
    nassertv(vertex->_pool == this);
    nassertv(vertex->get_index() == index);

    vertex->_pool = NULL;
    vertex->_index = -1;
  }

  _index_vertices.clear();
  _unique_vertices.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::has_forward_vertices
//       Access: Published
//  Description: Returns true if any vertices in the pool are
//               undefined forward-reference vertices, false if all
//               vertices are defined.
////////////////////////////////////////////////////////////////////
bool EggVertexPool::
has_forward_vertices() const {
  IndexVertices::const_iterator ivi;
  for (ivi = _index_vertices.begin(); ivi != _index_vertices.end(); ++ivi) {
    EggVertex *vertex = (*ivi).second;
    if (vertex->is_forward_reference()) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::has_defined_vertices
//       Access: Published
//  Description: Returns true if any vertices in the pool are
//               fully defined vertices, false if all vertices are
//               forward references.
////////////////////////////////////////////////////////////////////
bool EggVertexPool::
has_defined_vertices() const {
  IndexVertices::const_iterator ivi;
  for (ivi = _index_vertices.begin(); ivi != _index_vertices.end(); ++ivi) {
    EggVertex *vertex = (*ivi).second;
    if (!vertex->is_forward_reference()) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::get_vertex
//       Access: Public
//  Description: Returns the vertex in the pool with the indicated
//               index number, or NULL if no vertices have that index
//               number.
////////////////////////////////////////////////////////////////////
EggVertex *EggVertexPool::
get_vertex(int index) const {
  IndexVertices::const_iterator ivi = _index_vertices.find(index);

  if (ivi == _index_vertices.end()) {
    return NULL;
  } else {
    EggVertex *vertex = (*ivi).second;
    if (vertex->is_forward_reference()) {
      return NULL;
    }
    return vertex;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::get_forward_vertex
//       Access: Public
//  Description: Returns the vertex in the pool with the indicated
//               index number.  If there is not a vertex in the pool
//               with the indicated index number, creates a special
//               forward-reference EggVertex that has no data, on the
//               assumption that the vertex pool has not yet been
//               fully read and more data will be available later.
////////////////////////////////////////////////////////////////////
EggVertex *EggVertexPool::
get_forward_vertex(int index) {
  nassertr(index >= 0, NULL);

  IndexVertices::const_iterator ivi = _index_vertices.find(index);

  if (ivi == _index_vertices.end()) {
    PT(EggVertex) forward = new EggVertex;
    forward->_forward_reference = true;
    return add_vertex(forward, index);
  } else {
    return (*ivi).second;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::get_highest_index
//       Access: Public
//  Description: Returns the highest index number used by any vertex
//               in the pool (except forward references).  Returns -1
//               if the pool is empty.
////////////////////////////////////////////////////////////////////
int EggVertexPool::
get_highest_index() const {
  return _highest_index;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::set_highest_index
//       Access: Public
//  Description: Artificially changes the "highest index number", so
//               that a newly created vertex will begin at this number
//               plus 1.  This can be used to default a vertex pool to
//               start counting at 1 (or any other index number),
//               instead of the default of 0.  Use with caution.
////////////////////////////////////////////////////////////////////
void EggVertexPool::
set_highest_index(int highest_index) {
  _highest_index = highest_index;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::get_num_dimensions
//       Access: Public
//  Description: Returns the maximum number of dimensions used by any
//               vertex in the pool.
////////////////////////////////////////////////////////////////////
int EggVertexPool::
get_num_dimensions() const {
  int num_dimensions = 0;

  IndexVertices::const_iterator ivi;
  for (ivi = _index_vertices.begin(); ivi != _index_vertices.end(); ++ivi) {
    EggVertex *vertex = (*ivi).second;
    num_dimensions = max(num_dimensions, vertex->get_num_dimensions());
  }

  return num_dimensions;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::has_normals
//       Access: Public
//  Description: Returns true if any vertex in the pool has a normal
//               defined, false if none of them do.
////////////////////////////////////////////////////////////////////
bool EggVertexPool::
has_normals() const {
  IndexVertices::const_iterator ivi;
  for (ivi = _index_vertices.begin(); ivi != _index_vertices.end(); ++ivi) {
    EggVertex *vertex = (*ivi).second;
    if (vertex->has_normal()) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::has_colors
//       Access: Public
//  Description: Returns true if any vertex in the pool has a color
//               defined, false if none of them do.
////////////////////////////////////////////////////////////////////
bool EggVertexPool::
has_colors() const {
  IndexVertices::const_iterator ivi;
  for (ivi = _index_vertices.begin(); ivi != _index_vertices.end(); ++ivi) {
    EggVertex *vertex = (*ivi).second;
    if (vertex->has_color()) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::has_uvs
//       Access: Public
//  Description: Returns true if any vertex in the pool has a uv
//               defined, false if none of them do.
////////////////////////////////////////////////////////////////////
bool EggVertexPool::
has_uvs() const {
  IndexVertices::const_iterator ivi;
  for (ivi = _index_vertices.begin(); ivi != _index_vertices.end(); ++ivi) {
    EggVertex *vertex = (*ivi).second;
    if (vertex->has_uv()) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::get_uv_names
//       Access: Public
//  Description: Returns the list of UV names that are defined by any
//               vertices in the pool.  It is the user's
//               responsibility to clear the vector before calling
//               this method.
////////////////////////////////////////////////////////////////////
void EggVertexPool::
get_uv_names(vector_string &uv_names) const {
  pset<string> names;
  IndexVertices::const_iterator ivi;
  for (ivi = _index_vertices.begin(); ivi != _index_vertices.end(); ++ivi) {
    EggVertex *vertex = (*ivi).second;
    EggVertex::const_uv_iterator uvi;
    for (uvi = vertex->uv_begin(); uvi != vertex->uv_end(); ++uvi) {
      names.insert((*uvi)->get_name());
    }
  }

  pset<string>::const_iterator si;
  for (si = names.begin(); si != names.end(); ++si) {
    uv_names.push_back(*si);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::begin()
//       Access: Public
//  Description: Returns an iterator that can be used to traverse
//               through all the vertices in the pool.
////////////////////////////////////////////////////////////////////
EggVertexPool::iterator EggVertexPool::
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
EggVertexPool::iterator EggVertexPool::
end() const {
  return iterator(_index_vertices.end());
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::empty()
//       Access: Public
//  Description: Returns true if the pool is empty.
////////////////////////////////////////////////////////////////////
bool EggVertexPool::
empty() const {
  return _index_vertices.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::size()
//       Access: Public
//  Description: Returns the number of vertices in the pool.
////////////////////////////////////////////////////////////////////
EggVertexPool::size_type EggVertexPool::
size() const {
  nassertr(_index_vertices.size() == _unique_vertices.size(), 0);
  return _index_vertices.size();
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
//
//               It is possible that a forward reference to this
//               vertex was requested in the past; if so, the data
//               from the supplied vertex is copied onto the forward
//               reference, which becomes the actual vertex.  In this
//               case, a different pointer is saved (and returned)
//               than the one actually passed in.  In the usual case,
//               however, the vertex pointer passed in is the one that
//               is saved in the vertex pool and returned from this
//               method.
////////////////////////////////////////////////////////////////////
EggVertex *EggVertexPool::
add_vertex(EggVertex *vertex, int index) {
  // Save a pointer to the vertex.
  PT(EggVertex) vertex_keep = vertex;

  // Don't try to add a vertex while it still belongs to another pool.
  nassertr(vertex->_pool == NULL, NULL);

  if (index == -1) {
    index = get_highest_index() + 1;
  }
  // Always supply an index number >= 0.
  nassertr(index >= 0, NULL);

  // Check for a forward reference.
  IndexVertices::const_iterator ivi = _index_vertices.find(index);

  if (ivi != _index_vertices.end()) {
    EggVertex *orig_vertex = (*ivi).second;
    if (orig_vertex->is_forward_reference() &&
        !vertex->is_forward_reference()) {
      (*orig_vertex) = (*vertex);
      orig_vertex->_forward_reference = false;
      _highest_index = max(_highest_index, index);
      return orig_vertex;
    }

    // Oops, you duplicated a vertex index.
    nassertr(false, NULL);
  }
  
  _unique_vertices.insert(vertex);
  _index_vertices[index] = vertex;

  if (!vertex->is_forward_reference()) {
    _highest_index = max(_highest_index, index);
  }

  vertex->_pool = this;
  vertex->_index = index;

  return vertex;
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
  return add_vertex(new EggVertex(copy));
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

  if (_highest_index == vertex->_index) {
    // Find the new highest vertex index.
    if (_index_vertices.empty()) {
      _highest_index = -1;
    } else {
      IndexVertices::reverse_iterator ivi = _index_vertices.rbegin();
      while (ivi != _index_vertices.rend() &&
             (*ivi).second->is_forward_reference()) {
        ++ivi;
      }
      if (ivi != _index_vertices.rend()) {
        _highest_index = (*ivi).first;
      } else {
        _highest_index = -1;
      }
    }
  }

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

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::remove_unused_vertices
//       Access: Public
//  Description: Removes all vertices from the pool that are not
//               referenced by at least one primitive.  Also collapses
//               together equivalent vertices, and renumbers all
//               vertices after the operation so their indices are
//               consecutive, beginning at zero.  Returns the number
//               of vertices removed.
////////////////////////////////////////////////////////////////////
int EggVertexPool::
remove_unused_vertices() {
  int num_removed = 0;

  UniqueVertices new_unique_vertices;
  IndexVertices new_index_vertices;

  IndexVertices::const_iterator ivi;
  for (ivi = _index_vertices.begin(); ivi != _index_vertices.end(); ++ivi) {
    EggVertex *vertex = (*ivi).second;
    if (vertex->pref_size() == 0) {
      // This vertex is not used.  Don't add it to the new lists.
      vertex->clear_grefs();
      vertex->_pool = NULL;
      num_removed++;

    } else {
      // The vertex *is* used somewhere.  Is it identical to an
      // existing vertex?
      UniqueVertices::iterator uvi;
      uvi = new_unique_vertices.find(vertex);
      if (uvi != new_unique_vertices.end()) {
        // Yes, there's already another vertex just like this one.
        // Redirect all the primitives currently referencing this
        // vertex to reference the other one instead.
        EggVertex *orig_vertex = (*uvi);

        EggVertex::PrimitiveRef pref = vertex->_pref;
        EggVertex::PrimitiveRef::iterator pi;
        for (pi = pref.begin(); pi != pref.end(); ++pi) {
          EggPrimitive *prim = (*pi);
          EggPrimitive::iterator pvi = prim->find(vertex);
          nassertr(pvi != prim->end(), 0);
          prim->replace(pvi, orig_vertex);
        }
        vertex->test_pref_integrity();
        orig_vertex->test_pref_integrity();
        nassertr(vertex->pref_size() == 0, 0);
        vertex->clear_grefs();
        vertex->_pool = NULL;
        num_removed++;

      } else {
        // It's a unique vertex.  Renumber it and add it to the new
        // lists.
        vertex->_index = new_index_vertices.size();
        new_index_vertices.insert(IndexVertices::value_type(vertex->_index, vertex));
        new_unique_vertices.insert(vertex);
      }
    }
  }

  // All done.  Lose the old lists.
  _unique_vertices.swap(new_unique_vertices);
  _index_vertices.swap(new_index_vertices);
  _highest_index = (int)_index_vertices.size() - 1;

  nassertr(_index_vertices.size() == _unique_vertices.size(), num_removed);

  return num_removed;
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

    typedef pvector<EggVertex *> Verts;
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
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPool::r_transform_vertices
//       Access: Protected, Virtual
//  Description: This is called from within the egg code by
//               transform_vertices_only()().  It applies a
//               transformation matrix to the current node in some
//               sensible way (if the current node is a vertex pool
//               with vertices), then continues down the tree.
////////////////////////////////////////////////////////////////////
void EggVertexPool::
r_transform_vertices(const LMatrix4d &mat) {
  transform(mat);
}
