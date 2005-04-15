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

  CPT(qpGeomVertexArrayFormat) new_format =
    qpGeomVertexArrayFormat::register_format
    (new qpGeomVertexArrayFormat(InternalName::get_index(), 1, 
                                 cdata->_index_type, C_index));

  cdata->_vertices = new qpGeomVertexArrayData(new_format, usage_hint);
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
  
  CPT(qpGeomVertexArrayFormat) new_format =
    qpGeomVertexArrayFormat::register_format
    (new qpGeomVertexArrayFormat(InternalName::get_index(), 1, index_type, 
                                 C_index));

  if (cdata->_vertices->get_array_format() != new_format) {
    PT(qpGeomVertexArrayData) new_vertices = 
      new qpGeomVertexArrayData(new_format, cdata->_vertices->get_usage_hint());
    qpGeomVertexReader from(cdata->_vertices, 0);
    qpGeomVertexWriter to(new_vertices, 0);

    while (!from.is_at_end()) {
      to.add_data1i(from.get_data1i());
    }
    cdata->_vertices = new_vertices;
    cdata->_got_minmax = false;
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
  nassertr(i >= 0 && i < (int)cdata->_vertices->get_num_vertices(), -1);

  qpGeomVertexReader index(cdata->_vertices, 0);
  index.set_vertex(i);
  return index.get_data1i();
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
      (int)cdata->_vertices->get_num_vertices() == get_primitive_end(num_primitives - 1)) {
    // If we are beginning a new primitive, give the derived class a
    // chance to insert some degenerate vertices.
    append_unused_vertices(cdata->_vertices, vertex);
  }

  qpGeomVertexWriter index(cdata->_vertices, 0);
  index.set_vertex(cdata->_vertices->get_num_vertices());

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
      (int)cdata->_vertices->get_num_vertices() == get_primitive_end(num_primitives - 1)) {
    // If we are beginning a new primitive, give the derived class a
    // chance to insert some degenerate vertices.
    append_unused_vertices(cdata->_vertices, start);
  }

  qpGeomVertexWriter index(cdata->_vertices, 0);
  index.set_vertex(cdata->_vertices->get_num_vertices());

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
      num_added = (int)cdata->_vertices->get_num_vertices();
    } else {
      num_added = (int)cdata->_vertices->get_num_vertices() - cdata->_ends.back();
      num_added -= get_num_unused_vertices_per_primitive();
    }
    nassertr(num_added >= get_min_num_vertices_per_primitive(), false);
#endif
    cdata->_ends.push_back((int)cdata->_vertices->get_num_vertices());

  } else {
#ifndef NDEBUG
    // This is a simple primitive type like a triangle: each primitive
    // uses the same number of vertices.  Assert that we added the
    // correct number of vertices.
    int num_vertices_per_primitive = get_num_vertices_per_primitive();
    int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();

    int num_vertices = cdata->_vertices->get_num_vertices();
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
  cdata->_vertices = new qpGeomVertexArrayData
    (cdata->_vertices->get_array_format(), cdata->_vertices->get_usage_hint());
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
  CDWriter cdata(_cycler);

  cdata->_got_minmax = false;

  qpGeomVertexRewriter index(cdata->_vertices, 0);

  while (!index.is_at_end()) {
    index.set_data1i(index.get_data1i() + offset);
  }
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
    return ((int)cdata->_vertices->get_num_vertices() / num_vertices_per_primitive);
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
  CDReader cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_mins->get_num_vertices(), -1);

  qpGeomVertexReader index(cdata->_mins, 0);
  index.set_vertex(n);
  return index.get_data1i();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_primitive_max_vertex
//       Access: Published
//  Description: Returns the maximum vertex index number used by the
//               nth primitive in this object.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_primitive_max_vertex(int n) const {
  CDReader cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_maxs->get_num_vertices(), -1);

  qpGeomVertexReader index(cdata->_maxs, 0);
  index.set_vertex(n);
  return index.get_data1i();
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
//               (if it is flat shaded).
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
    return this;
  }

  PT(qpGeomPrimitive) new_prim = make_copy();
  new_prim->set_vertices(rotated_vertices);
  return new_prim;
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
  return (cdata->_vertices->get_data_size_bytes() +
    cdata->_ends.size() * sizeof(int) + sizeof(qpGeomPrimitive));
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
    get_max_vertex() < vertex_data->get_num_vertices();
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
    << get_type() << ":\n";
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
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayData *qpGeomPrimitive::
modify_vertices() {
  CDWriter cdata(_cycler);
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

  qpGeomVertexReader index(cdata->_vertices, 0);

  if (got_mat) {
    while (!index.is_at_end()) {
      reader.set_vertex(index.get_data1i());
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
      reader.set_vertex(index.get_data1i());
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
//     Function: qpGeomPrimitive::append_unused_vertices
//       Access: Protected, Virtual
//  Description: Called when a new primitive is begun (other than the
//               first primitive), this should add some degenerate
//               vertices between primitives, if the primitive type
//               requires that.  The second parameter is the first
//               vertex that begins the new primitive.
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
  if (cdata->_vertices->get_num_vertices() == 0) {
    cdata->_min_vertex = 0;
    cdata->_max_vertex = 0;
    cdata->_mins.clear();
    cdata->_maxs.clear();

  } else if (get_num_vertices_per_primitive() == 0) {
    // This is a complex primitive type like a triangle strip; compute
    // the minmax of each primitive (as well as the overall minmax).
    qpGeomVertexReader index(cdata->_vertices, 0);

    cdata->_mins = new qpGeomVertexArrayData
      (cdata->_vertices->get_array_format(), UH_unspecified);
    cdata->_maxs = new qpGeomVertexArrayData
      (cdata->_vertices->get_array_format(), UH_unspecified);

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
    nassertv(cdata->_mins->get_num_vertices() == (int)cdata->_ends.size());

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
  dg.add_uint8(_index_type);

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
  _index_type = (NumericType)scan.get_uint8();

  manager->read_pointer(scan);
  READ_PTA(manager, scan, IPD_int::read_datagram, _ends);

  _modified = qpGeom::get_next_modified();
  _got_minmax = false;
}
