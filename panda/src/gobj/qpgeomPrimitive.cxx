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
#include "preparedGraphicsObjects.h"
#include "internalName.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indent.h"

TypeHandle qpGeomPrimitive::_type_handle;

PStatCollector qpGeomPrimitive::_rotate_pcollector("Draw:Rotate");

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomPrimitive::
qpGeomPrimitive(qpGeomPrimitive::UsageHint usage_hint) {
  set_usage_hint(usage_hint);
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
  // When we destruct, we should ensure that all of our cached
  // entries, across all pipeline stages, are properly removed from
  // the cache manager.
  int num_stages = _cycler.get_num_stages();
  for (int i = 0; i < num_stages; i++) {
    if (_cycler.is_stage_unique(i)) {
      CData *cdata = _cycler.write_stage(i);
      if (cdata->_cache != (CacheEntry *)NULL) {
        cdata->_cache->erase();
        cdata->_cache = NULL;
      }
      _cycler.release_write_stage(i, cdata);
    }
  }

  release_all();
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
  unsigned short short_vertex = vertex;
  nassertv((int)short_vertex == vertex);

  clear_cache();
  CDWriter cdata(_cycler);

  int num_primitives = get_num_primitives();
  if (num_primitives > 0 &&
      (int)cdata->_vertices.size() == get_primitive_end(num_primitives - 1)) {
    // If we are beginning a new primitive, give the derived class a
    // chance to insert some degenerate vertices.
    append_unused_vertices(cdata->_vertices, vertex);
  }

  cdata->_vertices.push_back(short_vertex);
  cdata->_rotated_vertices.clear();

  if (cdata->_got_minmax) {
    cdata->_min_vertex = min(cdata->_min_vertex, short_vertex);
    cdata->_max_vertex = max(cdata->_max_vertex, short_vertex);

    if (get_num_vertices_per_primitive() == 0) {
      // Complex primitives also update their per-primitive minmax.
      size_t pi = cdata->_ends.size();
      if (pi < cdata->_mins.size()) {
        cdata->_mins[pi] = min(cdata->_mins[pi], short_vertex);
        cdata->_maxs[pi] = max(cdata->_maxs[pi], short_vertex);
      } else {
        cdata->_mins.push_back(short_vertex);
        cdata->_maxs.push_back(short_vertex);
      }
      nassertv((cdata->_mins.size() == cdata->_ends.size() + 1) &&
               (cdata->_maxs.size() == cdata->_ends.size() + 1));
    }
  }
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
  clear_cache();
  int end = (start + num_vertices) - 1;
  unsigned short short_start = start;
  unsigned short short_end = end;
  nassertv((int)short_start == start && (int)short_end == end);

  CDWriter cdata(_cycler);

  int num_primitives = get_num_primitives();
  if (num_primitives > 0 &&
      (int)cdata->_vertices.size() == get_primitive_end(num_primitives - 1)) {
    // If we are beginning a new primitive, give the derived class a
    // chance to insert some degenerate vertices.
    append_unused_vertices(cdata->_vertices, start);
  }

  for (unsigned short v = short_start; v <= short_end; ++v) {
    cdata->_vertices.push_back(v);
  }
  cdata->_rotated_vertices.clear();

  if (cdata->_got_minmax) {
    cdata->_min_vertex = min(cdata->_min_vertex, short_start);
    cdata->_max_vertex = max(cdata->_max_vertex, short_end);

    if (get_num_vertices_per_primitive() == 0) {
      // Complex primitives also update their per-primitive minmax.
      size_t pi = cdata->_ends.size();
      if (pi < cdata->_mins.size()) {
        cdata->_mins[pi] = min(cdata->_mins[pi], short_start);
        cdata->_maxs[pi] = max(cdata->_maxs[pi], short_end);
      } else {
        cdata->_mins.push_back(short_start);
        cdata->_maxs.push_back(short_end);
      }
      nassertv((cdata->_mins.size() == cdata->_ends.size() + 1) &&
               (cdata->_maxs.size() == cdata->_ends.size() + 1));
    }
  }
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
  clear_cache();
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  CDWriter cdata(_cycler);
  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each
    // primitive uses a different number of vertices.
#ifndef NDEBUG
    int num_added;
    if (cdata->_ends.empty()) {
      num_added = (int)cdata->_vertices.size();
    } else {
      num_added = (int)cdata->_vertices.size() - cdata->_ends.back();
      num_added -= get_num_unused_vertices_per_primitive();
    }
    nassertr(num_added >= get_min_num_vertices_per_primitive(), false);
#endif
    cdata->_ends.push_back((int)cdata->_vertices.size());

#ifndef NDEBUG
    if (cdata->_got_minmax) {
      nassertd((cdata->_mins.size() == cdata->_ends.size()) &&
               (cdata->_maxs.size() == cdata->_ends.size())) {
        cdata->_got_minmax = false;
      }
    }
#endif

  } else {
#ifndef NDEBUG
    // This is a simple primitive type like a triangle: each primitive
    // uses the same number of vertices.  Assert that we added the
    // correct number of vertices.
    int num_vertices_per_primitive = get_num_vertices_per_primitive();
    int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();

    int num_vertices = cdata->_vertices.size();
    nassertr((num_vertices + num_unused_vertices_per_primitive) % (num_vertices_per_primitive + num_unused_vertices_per_primitive) == 0, false)
#endif
  }

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
  clear_cache();
  CDWriter cdata(_cycler);
  cdata->_vertices.clear();
  cdata->_rotated_vertices.clear();
  cdata->_ends.clear();
  cdata->_mins.clear();
  cdata->_maxs.clear();
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
  clear_cache();
  CDWriter cdata(_cycler);

  cdata->_rotated_vertices.clear();
  cdata->_mins.clear();
  cdata->_maxs.clear();
  cdata->_got_minmax = false;

  PTA_ushort::iterator vi;
  for (vi = cdata->_vertices.begin(); vi != cdata->_vertices.end(); ++vi) {
    (*vi) += offset;
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
    return ((int)cdata->_vertices.size() / num_vertices_per_primitive);
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
  CPT(qpGeomPrimitive) result;
  {
    // Use read() and release_read() instead of CDReader, because the
    // call to record_primitive() might recursively call back into
    // this object, and require a write.
    const CData *cdata = _cycler.read();
    if (cdata->_cache != (CacheEntry *)NULL) {
      result = cdata->_cache->_decomposed;
      _cycler.release_read(cdata);
      // Record a cache hit, so this element will stay in the cache a
      // while longer.
      cdata->_cache->refresh();

      return result;
    }
    _cycler.release_read(cdata);
  }

  result = decompose_impl();
  if (result.p() == this || result.p() == NULL) {
    // decomposing this primitive has no effect or cannot be done.
    return this;
  }

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Decomposing " << get_type() << ": " << (void *)this << "\n";
  }

  // Record the result for the future.
  CacheEntry *entry;
  {
    CDWriter cdata(((qpGeomPrimitive *)this)->_cycler);
    entry = new CacheEntry;
    entry->_source = (qpGeomPrimitive *)this;
    entry->_decomposed = result;
    cdata->_cache = entry;
  }

  // And add *this* object to the cache manager.
  entry->record();

  return result;
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
  return (cdata->_vertices.size() + cdata->_mins.size() + cdata->_maxs.size()) * sizeof(short) +
    cdata->_ends.size() * sizeof(int) + sizeof(qpGeomPrimitive);
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
PTA_ushort qpGeomPrimitive::
modify_vertices() {
  clear_cache();
  CDWriter cdata(_cycler);
  cdata->_rotated_vertices.clear();
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
set_vertices(CPTA_ushort vertices) {
  clear_cache();
  CDWriter cdata(_cycler);
  cdata->_vertices = (PTA_ushort &)vertices;
  cdata->_rotated_vertices.clear();
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
  clear_cache();
  CDWriter cdata(_cycler);
  cdata->_rotated_vertices.clear();
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
  clear_cache();
  CDWriter cdata(_cycler);
  cdata->_ends = (PTA_int &)ends;
  cdata->_rotated_vertices.clear();
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
//     Function: qpGeomPrimitive::clear_cache
//       Access: Public
//  Description: Removes all of the previously-cached results of
//               decompose().
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
clear_cache() {
  // Probably we shouldn't do anything at all here unless we are
  // running in pipeline stage 0.
  CData *cdata = CDWriter(_cycler);
  if (cdata->_cache != (CacheEntry *)NULL) {
    cdata->_cache->erase();
    cdata->_cache = NULL;
  }

  // This, on the other hand, should be applied to the current
  // pipeline stage.
  cdata->_modified = qpGeom::get_next_modified();
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

  if (got_mat) {
    PTA_ushort::const_iterator ii;
    for (ii = cdata->_vertices.begin(); ii != cdata->_vertices.end(); ++ii) {
      int index = (int)(*ii);
      reader.set_vertex(index);
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
    PTA_ushort::const_iterator ii;
    for (ii = cdata->_vertices.begin(); ii != cdata->_vertices.end(); ++ii) {
      int index = (int)(*ii);
      reader.set_vertex(index);
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
//  Description: The virtual implementation of do_rotate().
////////////////////////////////////////////////////////////////////
CPTA_ushort qpGeomPrimitive::
rotate_impl() const {
  // The default implementation doesn't even try to do anything.
  nassertr(false, get_vertices());
  return get_vertices();
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
append_unused_vertices(PTA_ushort &, int) {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::do_rotate
//       Access: Private
//  Description: Fills _rotated_vertices by rotating the individual
//               primitives to bring each flat-colored vertex to the
//               opposite position.  If the current shade_model
//               indicates flat_vertex_last, this should bring the
//               last vertex to the first position; if it indicates
//               flat_vertex_first, this should bring the first vertex
//               to the last position.  This is used internally to
//               implement get_flat_first_vertices() and
//               get_flat_last_vertices().
////////////////////////////////////////////////////////////////////
CPTA_ushort qpGeomPrimitive::
do_rotate(qpGeomPrimitive::CDReader &cdata) {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Rotating " << get_type() << ": " << (void *)this << "\n";
  }

  PStatTimer timer(_rotate_pcollector);
  CPTA_ushort rotated_vertices = rotate_impl();

  CDWriter cdataw(_cycler, cdata);
  cdataw->_rotated_vertices = rotated_vertices;
  return rotated_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::recompute_minmax
//       Access: Private
//  Description: Recomputes the _min_vertex and _max_vertex values if
//               necessary.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
recompute_minmax(qpGeomPrimitive::CDWriter &cdata) {
  if (cdata->_vertices.empty()) {
    cdata->_min_vertex = 0;
    cdata->_max_vertex = 0;
    cdata->_mins.clear();
    cdata->_maxs.clear();

  } else if (get_num_vertices_per_primitive() == 0) {
    // This is a complex primitive type like a triangle strip; compute
    // the minmax of each primitive (as well as the overall minmax).
    cdata->_mins = PTA_ushort::empty_array(cdata->_ends.size());
    cdata->_maxs = PTA_ushort::empty_array(cdata->_ends.size());
    
    int pi = 0;
    int vi = 0;
    int num_vertices = (int)cdata->_vertices.size();
    
    unsigned short vertex = cdata->_vertices[vi];
    cdata->_min_vertex = vertex;
    cdata->_mins[pi] = vertex;
    cdata->_max_vertex = vertex;
    cdata->_maxs[pi] = vertex;
    
    ++vi;
    while (vi < num_vertices) {
      unsigned short vertex = cdata->_vertices[vi];
      cdata->_min_vertex = min(cdata->_min_vertex, vertex);
      cdata->_max_vertex = max(cdata->_max_vertex, vertex);
      
      if (vi == cdata->_ends[pi]) {
        ++pi;
        if (pi < (int)cdata->_ends.size()) {
          cdata->_mins[pi] = vertex;
          cdata->_maxs[pi] = vertex;
        }
      } else {
        nassertv(pi < (int)cdata->_ends.size());
        cdata->_mins[pi] = min(cdata->_mins[pi], vertex);
        cdata->_maxs[pi] = max(cdata->_maxs[pi], vertex);
      }
      
      ++vi;
    }

  } else {
    // This is a simple primitive type like a triangle; just compute
    // the overall minmax.
    PTA_ushort::const_iterator ii = cdata->_vertices.begin();
    cdata->_min_vertex = (*ii);
    cdata->_max_vertex = (*ii);
    cdata->_mins.clear();
    cdata->_maxs.clear();
    
    ++ii;
    while (ii != cdata->_vertices.end()) {
      cdata->_min_vertex = min(cdata->_min_vertex, (*ii));
      cdata->_max_vertex = max(cdata->_max_vertex, (*ii));
      
      ++ii;
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
//     Function: qpGeomPrimitive::CacheEntry::evict_callback
//       Access: Public, Virtual
//  Description: Called when the entry is evicted from the cache, this
//               should clean up the owning object appropriately.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::CacheEntry::
evict_callback() {
  // We have to operate on stage 0 of the pipeline, since that's where
  // the cache really counts.  Because of the multistage pipeline, we
  // might not actually have a cache entry there (it might have been
  // added to stage 1 instead).  No big deal if we don't.
  CData *cdata = _source->_cycler.write_stage(0);
  if (cdata->_cache.p() == this) {
    cdata->_cache = NULL;
  }
  _source->_cycler.release_write_stage(0, cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::CacheEntry::get_result_size
//       Access: Public, Virtual
//  Description: Returns the approximate number of bytes represented
//               by the computed result.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::CacheEntry::
get_result_size() const {
  return _decomposed->get_num_bytes();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::CacheEntry::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::CacheEntry::
output(ostream &out) const {
  out << "primitive " << (void *)_source;
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
  dg.add_uint8(_usage_hint);
  dg.add_uint8(_shade_model);

  WRITE_PTA(manager, dg, IPD_ushort::write_datagram, _vertices);
  WRITE_PTA(manager, dg, IPD_int::write_datagram, _ends);
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
  _usage_hint = (UsageHint)scan.get_uint8();
  _shade_model = (ShadeModel)scan.get_uint8();
  
  READ_PTA(manager, scan, IPD_ushort::read_datagram, _vertices);
  READ_PTA(manager, scan, IPD_int::read_datagram, _ends);

  _modified = qpGeom::get_next_modified();
  _got_minmax = false;
}
