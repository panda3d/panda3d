// Filename: qpgeomPrimitive.cxx
// Created by:  drose (06Mar05)
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

#include "qpgeomPrimitive.h"
#include "qpgeom.h"
#include "qpgeomVertexData.h"
#include "qpgeomVertexArrayFormat.h"
#include "qpgeomVertexColumn.h"
#include "qpgeomVertexReader.h"
#include "qpgeomVertexWriter.h"
#include "qpgeomVertexRewriter.h"
#include "preparedGraphicsObjects.h"
#include "internalName.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indent.h"

TypeHandle qpGeomPrimitive::_type_handle;

PStatCollector qpGeomPrimitive::_decompose_pcollector("Cull:Munge:Decompose");
PStatCollector qpGeomPrimitive::_rotate_pcollector("Cull:Munge:Rotate");

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::Default Constructor
//       Access: Protected
//  Description: Constructs an invalid object.  Only used when reading
//               from bam.
////////////////////////////////////////////////////////////////////
qpGeomPrimitive::
qpGeomPrimitive() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomPrimitive::
qpGeomPrimitive(qpGeomPrimitive::UsageHint usage_hint) {
  CDWriter cdata(_cycler);
  cdata->_usage_hint = usage_hint;
}
 
////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomPrimitive::
qpGeomPrimitive(const qpGeomPrimitive &copy) :
  TypedWritableReferenceCount(copy),
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomPrimitive::
~qpGeomPrimitive() {
  release_all();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_geom_rendering
//       Access: Published, Virtual
//  Description: Returns the set of GeomRendering bits that represent
//               the rendering properties required to properly render
//               this primitive.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_geom_rendering() const {
  // The default is nothing fancy.
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::set_usage_hint
//       Access: Published
//  Description: Changes the UsageHint hint for this primitive.  See
//               get_usage_hint().
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
set_usage_hint(qpGeomPrimitive::UsageHint usage_hint) {
  CDWriter cdata(_cycler);
  cdata->_usage_hint = usage_hint;

  if (cdata->_vertices != (qpGeomVertexArrayData *)NULL) {
    if (cdata->_vertices->get_ref_count() > 1) {
      cdata->_vertices = new qpGeomVertexArrayData(*cdata->_vertices);
    }
    
    cdata->_modified = qpGeom::get_next_modified();
    cdata->_usage_hint = usage_hint;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::set_index_type
//       Access: Published
//  Description: Changes the numeric type of the index column.
//               Normally, this should be either NT_uint16 or
//               NT_uint32.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
set_index_type(qpGeomPrimitive::NumericType index_type) {
  CDWriter cdata(_cycler);
  cdata->_index_type = index_type;

  if (cdata->_vertices != (qpGeomVertexArrayData *)NULL) {
    CPT(qpGeomVertexArrayFormat) new_format = get_index_format();
    
    if (cdata->_vertices->get_array_format() != new_format) {
      PT(qpGeomVertexArrayData) new_vertices = make_index_data();
      new_vertices->set_num_rows(cdata->_vertices->get_num_rows());

      qpGeomVertexReader from(cdata->_vertices, 0);
      qpGeomVertexWriter to(new_vertices, 0);
      
      while (!from.is_at_end()) {
        to.set_data1i(from.get_data1i());
      }
      cdata->_vertices = new_vertices;
      cdata->_got_minmax = false;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_first_vertex
//       Access: Published
//  Description: Returns the first vertex number referenced by the
//               primitive.  This is particularly important in the
//               case of a nonindexed primitive, in which case
//               get_first_vertex() and get_num_vertices() completely
//               define the extent of the vertex range.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_first_vertex() const {
  CDReader cdata(_cycler);
  if (cdata->_vertices == (qpGeomVertexArrayData *)NULL) {
    return cdata->_first_vertex;
  } else if (cdata->_vertices->get_num_rows() == 0) {
    return 0;
  } else {
    qpGeomVertexReader index(cdata->_vertices, 0);
    return index.get_data1i();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_vertex
//       Access: Published
//  Description: Returns the ith vertex index in the table.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_vertex(int i) const {
  CDReader cdata(_cycler);

  if (cdata->_vertices != (qpGeomVertexArrayData *)NULL) {
    // The indexed case.
    nassertr(i >= 0 && i < (int)cdata->_vertices->get_num_rows(), -1);

    qpGeomVertexReader index(cdata->_vertices, 0);
    index.set_row(i);
    return index.get_data1i();

  } else {
    // The nonindexed case.
    return cdata->_first_vertex + i;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::add_vertex
//       Access: Published
//  Description: Adds the indicated vertex to the list of vertex
//               indices used by the graphics primitive type.  To
//               define primitive, you must call add_vertex() for each
//               vertex of the new primitve, and then call
//               close_primitive() after you have specified the last
//               vertex of each primitive.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
add_vertex(int vertex) {
  CDWriter cdata(_cycler);

  int num_primitives = get_num_primitives();
  if (num_primitives > 0 &&
      requires_unused_vertices() && 
      get_num_vertices() == get_primitive_end(num_primitives - 1)) {
    // If we are beginning a new primitive, give the derived class a
    // chance to insert some degenerate vertices.
    if (cdata->_vertices == (qpGeomVertexArrayData *)NULL) {
      do_make_indexed(cdata);
    }
    append_unused_vertices(cdata->_vertices, vertex);
  }

  if (cdata->_vertices == (qpGeomVertexArrayData *)NULL) {
    // The nonindexed case.  We can keep the primitive nonindexed only
    // if the vertex number happens to be the next available vertex.
    if (cdata->_num_vertices == 0) {
      cdata->_first_vertex = vertex;
      cdata->_num_vertices = 1;
      cdata->_modified = qpGeom::get_next_modified();
      cdata->_got_minmax = false;
      return;

    } else if (vertex == cdata->_first_vertex + cdata->_num_vertices) {
      ++cdata->_num_vertices;
      cdata->_modified = qpGeom::get_next_modified();
      cdata->_got_minmax = false;
      return;
    }
    
    // Otherwise, we need to suddenly become an indexed primitive.
    do_make_indexed(cdata);
  }

  qpGeomVertexWriter index(cdata->_vertices, 0);
  index.set_row(cdata->_vertices->get_num_rows());

  index.add_data1i(vertex);

  cdata->_modified = qpGeom::get_next_modified();
  cdata->_got_minmax = false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::add_consecutive_vertices
//       Access: Published
//  Description: Adds a consecutive sequence of vertices, beginning at
//               start, to the primitive.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
add_consecutive_vertices(int start, int num_vertices) {
  if (num_vertices == 0) {
    return;
  }
  int end = (start + num_vertices) - 1;

  CDWriter cdata(_cycler);

  int num_primitives = get_num_primitives();
  if (num_primitives > 0 &&
      get_num_vertices() == get_primitive_end(num_primitives - 1)) {
    // If we are beginning a new primitive, give the derived class a
    // chance to insert some degenerate vertices.
    if (cdata->_vertices == (qpGeomVertexArrayData *)NULL) {
      do_make_indexed(cdata);
    }
    append_unused_vertices(cdata->_vertices, start);
  }

  if (cdata->_vertices == (qpGeomVertexArrayData *)NULL) {
    // The nonindexed case.  We can keep the primitive nonindexed only
    // if the vertex number happens to be the next available vertex.
    if (cdata->_num_vertices == 0) {
      cdata->_first_vertex = start;
      cdata->_num_vertices = num_vertices;
      return;

    } else if (start == cdata->_first_vertex + cdata->_num_vertices) {
      cdata->_num_vertices += num_vertices;
      return;
    }
    
    // Otherwise, we need to suddenly become an indexed primitive.
    do_make_indexed(cdata);
  }

  qpGeomVertexWriter index(cdata->_vertices, 0);
  index.set_row(cdata->_vertices->get_num_rows());

  for (int v = start; v <= end; ++v) {
    index.add_data1i(v);
  }

  cdata->_modified = qpGeom::get_next_modified();
  cdata->_got_minmax = false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::add_next_vertices
//       Access: Published
//  Description: Adds the next n vertices in sequence, beginning from
//               the last vertex added to the primitive + 1.
//
//               This is most useful when you are building up a
//               primitive and a GeomVertexData at the same time, and
//               you just want the primitive to reference the first n
//               vertices from the data, then the next n, and so on.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
add_next_vertices(int num_vertices) {
  if (get_num_vertices() == 0) {
    add_consecutive_vertices(0, num_vertices);
  } else {
    add_consecutive_vertices(get_vertex(get_num_vertices() - 1) + 1, num_vertices);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::close_primitive
//       Access: Published
//  Description: Indicates that the previous n calls to add_vertex(),
//               since the last call to close_primitive(), have fully
//               defined a new primitive.  Returns true if successful,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool qpGeomPrimitive::
close_primitive() {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  CDWriter cdata(_cycler);
  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each
    // primitive uses a different number of vertices.
#ifndef NDEBUG
    int num_added;
    if (cdata->_ends.empty()) {
      num_added = get_num_vertices();
    } else {
      num_added = get_num_vertices() - cdata->_ends.back();
      num_added -= get_num_unused_vertices_per_primitive();
    }
    nassertr(num_added >= get_min_num_vertices_per_primitive(), false);
#endif
    cdata->_ends.push_back(get_num_vertices());

  } else {
#ifndef NDEBUG
    // This is a simple primitive type like a triangle: each primitive
    // uses the same number of vertices.  Assert that we added the
    // correct number of vertices.
    int num_vertices_per_primitive = get_num_vertices_per_primitive();
    int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();

    int num_vertices = get_num_vertices();
    nassertr((num_vertices + num_unused_vertices_per_primitive) % (num_vertices_per_primitive + num_unused_vertices_per_primitive) == 0, false)
#endif
  }

  cdata->_modified = qpGeom::get_next_modified();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::clear_vertices
//       Access: Published
//  Description: Removes all of the vertices and primitives from the
//               object, so they can be re-added.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
clear_vertices() {
  CDWriter cdata(_cycler);
  cdata->_first_vertex = 0;
  cdata->_num_vertices = 0;
  cdata->_vertices.clear();
  cdata->_ends.clear();
  cdata->_mins.clear();
  cdata->_maxs.clear();
  cdata->_modified = qpGeom::get_next_modified();
  cdata->_got_minmax = false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::offset_vertices
//       Access: Published
//  Description: Adds the indicated offset to all vertices used by the
//               primitive.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
offset_vertices(int offset) {
  if (is_indexed()) {
    qpGeomVertexRewriter index(modify_vertices(), 0);
    while (!index.is_at_end()) {
      index.set_data1i(index.get_data1i() + offset);
    }

  } else {
    CDWriter cdata(_cycler);
    cdata->_first_vertex += offset;
    cdata->_modified = qpGeom::get_next_modified();
    cdata->_got_minmax = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::make_nonindexed
//       Access: Published
//  Description: Converts the primitive from indexed to nonindexed by
//               duplicating vertices as necessary into the indicated
//               dest GeomVertexData.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
make_nonindexed(qpGeomVertexData *dest, const qpGeomVertexData *source) {
  int num_vertices = get_num_vertices();
  int dest_start = dest->get_num_rows();

  dest->set_num_rows(dest_start + num_vertices);
  for (int i = 0; i < num_vertices; ++i) {
    int v = get_vertex(i);
    dest->copy_row_from(dest_start + i, source, v);
  }

  set_nonindexed_vertices(dest_start, num_vertices);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::pack_vertices
//       Access: Published
//  Description: Packs the vertices used by the primitive from the
//               indicated source array onto the end of the indicated
//               destination array.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
pack_vertices(qpGeomVertexData *dest, const qpGeomVertexData *source) {
  if (!is_indexed()) {
    // If the primitive is nonindexed, packing is the same as
    // converting (again) to nonindexed.
    make_nonindexed(dest, source);

  } else {
    // The indexed case: build up a new index as we go.
    CPT(qpGeomVertexArrayData) orig_vertices = get_vertices();
    PT(qpGeomVertexArrayData) new_vertices = make_index_data();
    qpGeomVertexWriter index(new_vertices, 0);
    typedef pmap<int, int> CopiedIndices;
    CopiedIndices copied_indices;

    int num_vertices = get_num_vertices();
    int dest_start = dest->get_num_rows();

    for (int i = 0; i < num_vertices; ++i) {
      int v = get_vertex(i);

      // Try to add the relation { v : size() }.  If that succeeds,
      // great; if it doesn't, look up whatever we previously added
      // for v.
      pair<CopiedIndices::iterator, bool> result = 
        copied_indices.insert(CopiedIndices::value_type(v, (int)copied_indices.size()));
      int v2 = (*result.first).second + dest_start;
      index.add_data1i(v2);

      if (result.second) {
        // This is the first time we've seen vertex v.
        dest->copy_row_from(v2, source, v);
      }
    }
    
    set_vertices(new_vertices);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::make_indexed
//       Access: Published
//  Description: Converts the primitive from nonindexed form to
//               indexed form.  This will simply create an index table
//               that is numbered consecutively from
//               get_first_vertex(); it does not automatically
//               collapse together identical vertices that may have
//               been split apart by a previous call to
//               make_nonindexed().
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
make_indexed() {
  CDWriter cdata(_cycler);
  do_make_indexed(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_num_primitives
//       Access: Published
//  Description: Returns the number of individual primitives stored
//               within this object.  All primitives are the same
//               type.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_num_primitives() const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  CDReader cdata(_cycler);
  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each
    // primitive uses a different number of vertices.
    return cdata->_ends.size();

  } else {
    // This is a simple primitive type like a triangle: each primitive
    // uses the same number of vertices.
    return (get_num_vertices() / num_vertices_per_primitive);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_primitive_start
//       Access: Published
//  Description: Returns the element within the _vertices list at which
//               the nth primitive starts.  
//
//               If i is one more than the highest valid primitive
//               vertex, the return value will be one more than the
//               last valid vertex.  Thus, it is generally true that
//               the vertices used by a particular primitive i are the
//               set get_primitive_start(n) <= vi <
//               get_primitive_start(n + 1) (although this range also
//               includes the unused vertices between primitives).
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_primitive_start(int n) const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();
  int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each
    // primitive uses a different number of vertices.
    CDReader cdata(_cycler);
    nassertr(n >= 0 && n <= (int)cdata->_ends.size(), -1);
    if (n == 0) {
      return 0;
    } else {
      return cdata->_ends[n - 1] + num_unused_vertices_per_primitive;
    }

  } else {
    // This is a simple primitive type like a triangle: each primitive
    // uses the same number of vertices.
    return n * (num_vertices_per_primitive + num_unused_vertices_per_primitive);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_primitive_end
//       Access: Published
//  Description: Returns the element within the _vertices list at which
//               the nth primitive ends.  This is one past the last
//               valid element for the nth primitive.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_primitive_end(int n) const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each
    // primitive uses a different number of vertices.
    CDReader cdata(_cycler);
    nassertr(n >= 0 && n < (int)cdata->_ends.size(), -1);
    return cdata->_ends[n];

  } else {
    // This is a simple primitive type like a triangle: each primitive
    // uses the same number of vertices.
    int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();
    return n * (num_vertices_per_primitive + num_unused_vertices_per_primitive) + num_vertices_per_primitive;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_primitive_num_vertices
//       Access: Published
//  Description: Returns the number of vertices used by the nth
//               primitive.  This is the same thing as
//               get_primitive_end(n) - get_primitive_start(n).
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_primitive_num_vertices(int n) const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each
    // primitive uses a different number of vertices.
    CDReader cdata(_cycler);
    nassertr(n >= 0 && n < (int)cdata->_ends.size(), 0);
    if (n == 0) {
      return cdata->_ends[0];
    } else {
      int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();
      return cdata->_ends[n] - cdata->_ends[n - 1] - num_unused_vertices_per_primitive;
    }      

  } else {
    // This is a simple primitive type like a triangle: each primitive
    // uses the same number of vertices.
    return num_vertices_per_primitive;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_primitive_min_vertex
//       Access: Published
//  Description: Returns the minimum vertex index number used by the
//               nth primitive in this object.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_primitive_min_vertex(int n) const {
  if (is_indexed()) {
    CPT(qpGeomVertexArrayData) mins = get_mins();
    nassertr(n >= 0 && n < mins->get_num_rows(), -1);

    qpGeomVertexReader index(mins, 0);
    index.set_row(n);
    return index.get_data1i();
  } else {
    return get_primitive_start(n);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_primitive_max_vertex
//       Access: Published
//  Description: Returns the maximum vertex index number used by the
//               nth primitive in this object.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_primitive_max_vertex(int n) const {
  if (is_indexed()) {
    CPT(qpGeomVertexArrayData) maxs = get_maxs();
    nassertr(n >= 0 && n < maxs->get_num_rows(), -1);

    qpGeomVertexReader index(maxs, 0);
    index.set_row(n);
    return index.get_data1i();
  } else {
    return get_primitive_end(n) - 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::decompose
//       Access: Published
//  Description: Decomposes a complex primitive type into a simpler
//               primitive type, for instance triangle strips to
//               triangles, and returns a pointer to the new primitive
//               definition.  If the decomposition cannot be
//               performed, this might return the original object.
//
//               This method is useful for application code that wants
//               to iterate through the set of triangles on the
//               primitive without having to write handlers for each
//               possible kind of primitive type.
////////////////////////////////////////////////////////////////////
CPT(qpGeomPrimitive) qpGeomPrimitive::
decompose() const {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Decomposing " << get_type() << ": " << (void *)this << "\n";
  }

  PStatTimer timer(_decompose_pcollector);
  return decompose_impl();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::rotate
//       Access: Published
//  Description: Returns a new primitive with the shade_model reversed
//               (if it is flat shaded), if possible.  If the
//               primitive type cannot be rotated, returns the
//               original primitive, unrotated.
//
//               If the current shade_model indicates
//               flat_vertex_last, this should bring the last vertex
//               to the first position; if it indicates
//               flat_vertex_first, this should bring the first vertex
//               to the last position.
////////////////////////////////////////////////////////////////////
CPT(qpGeomPrimitive) qpGeomPrimitive::
rotate() const {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Rotating " << get_type() << ": " << (void *)this << "\n";
  }

  PStatTimer timer(_rotate_pcollector);
  CPT(qpGeomVertexArrayData) rotated_vertices = rotate_impl();

  if (rotated_vertices == (qpGeomVertexArrayData *)NULL) {
    // This primitive type can't be rotated.
    return this;
  }

  PT(qpGeomPrimitive) new_prim = make_copy();
  new_prim->set_vertices(rotated_vertices);

  switch (get_shade_model()) {
  case SM_flat_first_vertex:
    new_prim->set_shade_model(SM_flat_last_vertex);
    break;

  case SM_flat_last_vertex:
    new_prim->set_shade_model(SM_flat_first_vertex);
    break;

  default:
    break;
  }

  return new_prim;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::match_shade_model
//       Access: Published
//  Description: Returns a new primitive that is compatible with the
//               indicated shade model, if possible, or NULL if this
//               is not possible.
//
//               In most cases, this will return either NULL or the
//               original primitive.  In the case of a
//               SM_flat_first_vertex vs. a SM_flat_last_vertex (or
//               vice-versa), however, it will return a rotated
//               primitive.
////////////////////////////////////////////////////////////////////
CPT(qpGeomPrimitive) qpGeomPrimitive::
match_shade_model(qpGeomPrimitive::ShadeModel shade_model) const {
  ShadeModel this_shade_model = get_shade_model();
  if (this_shade_model == shade_model) {
    // Trivially compatible.
    return this;
  }

  if (this_shade_model == SM_uniform || shade_model == SM_uniform) {
    // SM_uniform is compatible with anything.
    return this;
  }

  if ((this_shade_model == SM_flat_first_vertex && shade_model == SM_flat_last_vertex) ||
      (this_shade_model == SM_flat_last_vertex && shade_model == SM_flat_first_vertex)) {
    // Needs to be rotated.
    CPT(qpGeomPrimitive) rotated = rotate();
    if (rotated.p() == this) {
      // Oops, can't be rotated, sorry.
      return NULL;
    }
    return rotated;
  }

  // Not compatible, sorry.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_num_bytes
//       Access: Published
//  Description: Returns the number of bytes consumed by the primitive
//               and its index table(s).
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_num_bytes() const {
  CDReader cdata(_cycler);
  int num_bytes = cdata->_ends.size() * sizeof(int) + sizeof(qpGeomPrimitive);
  if (cdata->_vertices != (qpGeomVertexArrayData *)NULL) {
    num_bytes += cdata->_vertices->get_data_size_bytes();
  }

  return num_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::check_valid
//       Access: Published
//  Description: Verifies that the primitive only references vertices
//               that actually exist within the indicated
//               GeomVertexData.  Returns true if the primitive
//               appears to be valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool qpGeomPrimitive::
check_valid(const qpGeomVertexData *vertex_data) const {
  return get_num_vertices() == 0 ||
    get_max_vertex() < vertex_data->get_num_rows();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
output(ostream &out) const {
  out << get_type() << ", " << get_num_primitives()
      << ", " << get_num_vertices();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_type();
  if (is_indexed()) {
    out << " (indexed)";
  } else {
    out << " (nonindexed)";
  }
  out << ":\n";
  int num_primitives = get_num_primitives();
  int num_vertices = get_num_vertices();
  int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();
  for (int i = 0; i < num_primitives; ++i) {
    indent(out, indent_level + 2)
      << "[";
    int begin = get_primitive_start(i);
    int end = get_primitive_end(i);
    for (int vi = begin; vi < end; vi++) {
      out << " " << get_vertex(vi);
    }
    out << " ]";
    if (end < num_vertices) {
      for (int ui = 0; ui < num_unused_vertices_per_primitive; ++ui) {
        if (end + ui < num_vertices) {
          out << " " << get_vertex(end + ui);
        } else {
          out << " ?";
        }
      }
    }
    out << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::modify_vertices
//       Access: Public
//  Description: Returns a modifiable pointer to the vertex index
//               list, so application code can directly fiddle with
//               this data.  Use with caution, since there are no
//               checks that the data will be left in a stable state.
//
//               If this is called on a nonindexed primitive, it will
//               implicitly be converted to an indexed primitive.
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayData *qpGeomPrimitive::
modify_vertices() {
  CDWriter cdata(_cycler);

  if (cdata->_vertices == (qpGeomVertexArrayData *)NULL) {
    do_make_indexed(cdata);
  }

  if (cdata->_vertices->get_ref_count() > 1) {
    cdata->_vertices = new qpGeomVertexArrayData(*cdata->_vertices);
  }

  cdata->_modified = qpGeom::get_next_modified();
  cdata->_got_minmax = false;
  return cdata->_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::set_vertices
//       Access: Public
//  Description: Completely replaces the vertex index list with a new
//               table.  Chances are good that you should also replace
//               the ends list with set_ends() at the same time.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
set_vertices(const qpGeomVertexArrayData *vertices) {
  CDWriter cdata(_cycler);
  cdata->_vertices = (qpGeomVertexArrayData *)vertices;

  cdata->_modified = qpGeom::get_next_modified();
  cdata->_got_minmax = false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::set_nonindexed_vertices
//       Access: Public
//  Description: Sets the primitive up as a nonindexed primitive,
//               using the indicated vertex range.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
set_nonindexed_vertices(int first_vertex, int num_vertices) {
  CDWriter cdata(_cycler);
  cdata->_vertices = (qpGeomVertexArrayData *)NULL;
  cdata->_first_vertex = first_vertex;
  cdata->_num_vertices = num_vertices;

  cdata->_modified = qpGeom::get_next_modified();
  cdata->_got_minmax = false;
  recompute_minmax(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::modify_ends
//       Access: Public
//  Description: Returns a modifiable pointer to the primitive ends
//               array, so application code can directly fiddle with
//               this data.  Use with caution, since there are no
//               checks that the data will be left in a stable state.
//
//               Note that simple primitive types, like triangles, do
//               not have a ends array: since all the primitives
//               have the same number of vertices, it is not needed.
////////////////////////////////////////////////////////////////////
PTA_int qpGeomPrimitive::
modify_ends() {
  CDWriter cdata(_cycler);

  cdata->_modified = qpGeom::get_next_modified();
  cdata->_got_minmax = false;
  return cdata->_ends;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::set_ends
//       Access: Public
//  Description: Completely replaces the primitive ends array with
//               a new table.  Chances are good that you should also
//               replace the vertices list with set_vertices() at the
//               same time.
//
//               Note that simple primitive types, like triangles, do
//               not have a ends array: since all the primitives
//               have the same number of vertices, it is not needed.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
set_ends(CPTA_int ends) {
  CDWriter cdata(_cycler);
  cdata->_ends = (PTA_int &)ends;

  cdata->_modified = qpGeom::get_next_modified();
  cdata->_got_minmax = false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: If the primitive type is a simple type in which all
//               primitives have the same number of vertices, like
//               triangles, returns the number of vertices per
//               primitive.  If the primitive type is a more complex
//               type in which different primitives might have
//               different numbers of vertices, for instance a
//               triangle strip, returns 0.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_num_vertices_per_primitive() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_min_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: Returns the minimum number of vertices that must be
//               added before close_primitive() may legally be called.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_min_num_vertices_per_primitive() const {
  return 3;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_num_unused_vertices_per_primitive
//       Access: Public, Virtual
//  Description: Returns the number of vertices that are added between
//               primitives that aren't, strictly speaking, part of
//               the primitives themselves.  This is used, for
//               instance, to define degenerate triangles to connect
//               otherwise disconnected triangle strips.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_num_unused_vertices_per_primitive() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::prepare
//       Access: Public
//  Description: Indicates that the data should be enqueued to be
//               prepared in the indicated prepared_objects at the
//               beginning of the next frame.  This will ensure the
//               data is already loaded into the GSG if it is expected
//               to be rendered soon.
//
//               Use this function instead of prepare_now() to preload
//               datas from a user interface standpoint.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_index_buffer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::prepare_now
//       Access: Public
//  Description: Creates a context for the data on the particular
//               GSG, if it does not already exist.  Returns the new
//               (or old) IndexBufferContext.  This assumes that the
//               GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               datas.  If this is not necessarily the case, you
//               should use prepare() instead.
//
//               Normally, this is not called directly except by the
//               GraphicsStateGuardian; a data does not need to be
//               explicitly prepared by the user before it may be
//               rendered.
////////////////////////////////////////////////////////////////////
IndexBufferContext *qpGeomPrimitive::
prepare_now(PreparedGraphicsObjects *prepared_objects, 
            GraphicsStateGuardianBase *gsg) {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return (*ci).second;
  }

  IndexBufferContext *ibc = prepared_objects->prepare_index_buffer_now(this, gsg);
  if (ibc != (IndexBufferContext *)NULL) {
    _contexts[prepared_objects] = ibc;
  }
  return ibc;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::release
//       Access: Public
//  Description: Frees the data context only on the indicated object,
//               if it exists there.  Returns true if it was released,
//               false if it had not been prepared.
////////////////////////////////////////////////////////////////////
bool qpGeomPrimitive::
release(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    IndexBufferContext *ibc = (*ci).second;
    prepared_objects->release_index_buffer(ibc);
    return true;
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_index_buffer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::release_all
//       Access: Public
//  Description: Frees the context allocated on all objects for which
//               the data has been declared.  Returns the number of
//               contexts which have been freed.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
release_all() {
  // We have to traverse a copy of the _contexts list, because the
  // PreparedGraphicsObjects object will call clear_prepared() in response
  // to each release_index_buffer(), and we don't want to be modifying the
  // _contexts list while we're traversing it.
  Contexts temp = _contexts;
  int num_freed = (int)_contexts.size();

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    PreparedGraphicsObjects *prepared_objects = (*ci).first;
    IndexBufferContext *ibc = (*ci).second;
    prepared_objects->release_index_buffer(ibc);
  }

  // Now that we've called release_index_buffer() on every known context,
  // the _contexts list should have completely emptied itself.
  nassertr(_contexts.empty(), num_freed);

  return num_freed;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::clear_prepared
//       Access: Private
//  Description: Removes the indicated PreparedGraphicsObjects table
//               from the data array's table, without actually
//               releasing the data array.  This is intended to be
//               called only from
//               PreparedGraphicsObjects::release_index_buffer(); it should
//               never be called by user code.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
clear_prepared(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);
  } else {
    // If this assertion fails, clear_prepared() was given a
    // prepared_objects which the data array didn't know about.
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::calc_tight_bounds
//       Access: Public, Virtual
//  Description: Expands min_point and max_point to include all of the
//               vertices in the Geom, if any.  found_any is set true
//               if any points are found.  It is the caller's
//               responsibility to initialize min_point, max_point,
//               and found_any before calling this function.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                  bool &found_any, 
                  const qpGeomVertexData *vertex_data,
                  bool got_mat, const LMatrix4f &mat) const {
  qpGeomVertexReader reader(vertex_data, InternalName::get_vertex());
  if (!reader.has_column()) {
    // No vertex data.
    return;
  }

  CDReader cdata(_cycler);

  if (cdata->_vertices == (qpGeomVertexArrayData *)NULL) {
    // Nonindexed case.
    if (got_mat) {
      for (int i = 0; i < cdata->_num_vertices; i++) {
        reader.set_row(cdata->_first_vertex + i);
        const LVecBase3f &vertex = reader.get_data3f();
        
        if (found_any) {
          min_point.set(min(min_point[0], vertex[0]),
                        min(min_point[1], vertex[1]),
                        min(min_point[2], vertex[2]));
          max_point.set(max(max_point[0], vertex[0]),
                        max(max_point[1], vertex[1]),
                        max(max_point[2], vertex[2]));
        } else {
          min_point = vertex;
          max_point = vertex;
          found_any = true;
        }
      }
    } else {
      for (int i = 0; i < cdata->_num_vertices; i++) {
        reader.set_row(cdata->_first_vertex + i);
        LPoint3f vertex = mat.xform_point(reader.get_data3f());
        
        if (found_any) {
          min_point.set(min(min_point[0], vertex[0]),
                        min(min_point[1], vertex[1]),
                        min(min_point[2], vertex[2]));
          max_point.set(max(max_point[0], vertex[0]),
                        max(max_point[1], vertex[1]),
                        max(max_point[2], vertex[2]));
        } else {
          min_point = vertex;
          max_point = vertex;
          found_any = true;
        }
      }
    }

  } else {
    // Indexed case.
    qpGeomVertexReader index(cdata->_vertices, 0);

    if (got_mat) {
      while (!index.is_at_end()) {
        reader.set_row(index.get_data1i());
        const LVecBase3f &vertex = reader.get_data3f();
        
        if (found_any) {
          min_point.set(min(min_point[0], vertex[0]),
                        min(min_point[1], vertex[1]),
                        min(min_point[2], vertex[2]));
          max_point.set(max(max_point[0], vertex[0]),
                        max(max_point[1], vertex[1]),
                        max(max_point[2], vertex[2]));
        } else {
          min_point = vertex;
          max_point = vertex;
          found_any = true;
        }
      }
    } else {
      while (!index.is_at_end()) {
        reader.set_row(index.get_data1i());
        LPoint3f vertex = mat.xform_point(reader.get_data3f());
        
        if (found_any) {
          min_point.set(min(min_point[0], vertex[0]),
                        min(min_point[1], vertex[1]),
                        min(min_point[2], vertex[2]));
          max_point.set(max(max_point[0], vertex[0]),
                        max(max_point[1], vertex[1]),
                        max(max_point[2], vertex[2]));
        } else {
          min_point = vertex;
          max_point = vertex;
          found_any = true;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::decompose_impl
//       Access: Protected, Virtual
//  Description: Decomposes a complex primitive type into a simpler
//               primitive type, for instance triangle strips to
//               triangles, and returns a pointer to the new primitive
//               definition.  If the decomposition cannot be
//               performed, this might return the original object.
//
//               This method is useful for application code that wants
//               to iterate through the set of triangles on the
//               primitive without having to write handlers for each
//               possible kind of primitive type.
////////////////////////////////////////////////////////////////////
CPT(qpGeomPrimitive) qpGeomPrimitive::
decompose_impl() const {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of rotate().
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexArrayData) qpGeomPrimitive::
rotate_impl() const {
  // The default implementation doesn't even try to do anything.
  nassertr(false, NULL);
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::requires_unused_vertices
//       Access: Protected, Virtual
//  Description: Should be redefined to return true in any primitive
//               that implements append_unused_vertices().
////////////////////////////////////////////////////////////////////
bool qpGeomPrimitive::
requires_unused_vertices() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::append_unused_vertices
//       Access: Protected, Virtual
//  Description: Called when a new primitive is begun (other than the
//               first primitive), this should add some degenerate
//               vertices between primitives, if the primitive type
//               requires that.  The second parameter is the first
//               vertex that begins the new primitive.
//
//               This method is only called if
//               requires_unused_vertices(), above, returns true.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
append_unused_vertices(qpGeomVertexArrayData *, int) {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::recompute_minmax
//       Access: Private
//  Description: Recomputes the _min_vertex and _max_vertex values if
//               necessary.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
recompute_minmax(qpGeomPrimitive::CDWriter &cdata) {
  if (cdata->_vertices == (qpGeomVertexArrayData *)NULL) {
    // In the nonindexed case, we don't need to do much (the
    // minmax is trivial).
    cdata->_min_vertex = cdata->_first_vertex;
    cdata->_max_vertex = cdata->_first_vertex + cdata->_num_vertices - 1;
    cdata->_mins.clear();
    cdata->_maxs.clear();

  } else if (get_num_vertices() == 0) {
    // Or if we don't have any vertices, the minmax is also trivial.
    cdata->_min_vertex = 0;
    cdata->_max_vertex = 0;
    cdata->_mins.clear();
    cdata->_maxs.clear();

  } else if (get_num_vertices_per_primitive() == 0) {
    // This is a complex primitive type like a triangle strip; compute
    // the minmax of each primitive (as well as the overall minmax).
    qpGeomVertexReader index(cdata->_vertices, 0);

    cdata->_mins = make_index_data();
    cdata->_maxs = make_index_data();

    qpGeomVertexWriter mins(cdata->_mins, 0);
    qpGeomVertexWriter maxs(cdata->_maxs, 0);

    int pi = 0;
    int vi = 0;
    
    unsigned int vertex = index.get_data1i();
    cdata->_min_vertex = vertex;
    cdata->_max_vertex = vertex;
    unsigned int min_prim = vertex;
    unsigned int max_prim = vertex;
    
    ++vi;
    while (!index.is_at_end()) {
      unsigned int vertex = index.get_data1i();
      cdata->_min_vertex = min(cdata->_min_vertex, vertex);
      cdata->_max_vertex = max(cdata->_max_vertex, vertex);

      if (vi == cdata->_ends[pi]) {
        mins.add_data1i(min_prim);
        maxs.add_data1i(max_prim);
        min_prim = vertex;
        max_prim = vertex;
        ++pi;

      } else {
        min_prim = min(min_prim, vertex);
        max_prim = max(max_prim, vertex);
      }
      
      ++vi;
    }
    mins.add_data1i(min_prim);
    maxs.add_data1i(max_prim);
    nassertv(cdata->_mins->get_num_rows() == (int)cdata->_ends.size());

  } else {
    // This is a simple primitive type like a triangle; just compute
    // the overall minmax.
    qpGeomVertexReader index(cdata->_vertices, 0);

    cdata->_mins.clear();
    cdata->_maxs.clear();

    unsigned int vertex = index.get_data1i();
    cdata->_min_vertex = vertex;
    cdata->_max_vertex = vertex;

    while (!index.is_at_end()) {
      unsigned int vertex = index.get_data1i();
      cdata->_min_vertex = min(cdata->_min_vertex, vertex);
      cdata->_max_vertex = max(cdata->_max_vertex, vertex);
    }
  }

  cdata->_got_minmax = true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::do_make_indexed
//       Access: Private
//  Description: The private implementation of make_indexed().
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
do_make_indexed(qpGeomPrimitive::CDWriter &cdata) {
  if (cdata->_vertices == (qpGeomVertexArrayData *)NULL) {
    cdata->_vertices = make_index_data();
    qpGeomVertexWriter index(cdata->_vertices, 0);
    for (int i = 0; i < cdata->_num_vertices; ++i) {
      index.add_data1i(i + cdata->_first_vertex);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
finalize(BamReader *manager) {
  if (manager->get_file_minor_ver() < 19) {
    const qpGeomVertexArrayData *vertices = get_vertices();
    if (vertices != (qpGeomVertexArrayData *)NULL) {
      set_usage_hint(vertices->get_usage_hint());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeomPrimitive.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
  manager->register_finalize(this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *qpGeomPrimitive::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_uint8(_shade_model);
  dg.add_uint32(_first_vertex);
  dg.add_uint32(_num_vertices);
  dg.add_uint8(_index_type);
  dg.add_uint8(_usage_hint);

  manager->write_pointer(dg, _vertices);
  WRITE_PTA(manager, dg, IPD_int::write_datagram, _ends);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  _vertices = DCAST(qpGeomVertexArrayData, p_list[pi++]);    

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeomPrimitive.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _shade_model = (ShadeModel)scan.get_uint8();
  if (manager->get_file_minor_ver() >= 19) {
    _first_vertex = scan.get_uint32();
    _num_vertices = scan.get_uint32();
  }
  _index_type = (NumericType)scan.get_uint8();
  if (manager->get_file_minor_ver() >= 19) {
    _usage_hint = (UsageHint)scan.get_uint8();
  }

  manager->read_pointer(scan);
  READ_PTA(manager, scan, IPD_int::read_datagram, _ends);

  _modified = qpGeom::get_next_modified();
  _got_minmax = false;
}
