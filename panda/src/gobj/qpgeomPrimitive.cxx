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
#include "qpgeomVertexData.h"
#include "qpgeomVertexArrayFormat.h"
#include "qpgeomVertexDataType.h"
#include "qpgeomVertexCacheManager.h"
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
qpGeomPrimitive() {
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
  qpGeomVertexCacheManager *cache_mgr = 
    qpGeomVertexCacheManager::get_global_ptr();

  int num_stages = _cycler.get_num_stages();
  for (int i = 0; i < num_stages; i++) {
    if (_cycler.is_stage_unique(i)) {
      CData *cdata = _cycler.write_stage(i);
      if (cdata->_decomposed != (qpGeomPrimitive *)NULL) {
        cache_mgr->remove_primitive(this);
        cdata->_decomposed = NULL;
      }
      _cycler.release_write_stage(i, cdata);
    }
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
  unsigned short short_vertex = vertex;
  nassertv((int)short_vertex == vertex);

  clear_cache();
  CDWriter cdata(_cycler);
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
  clear_cache();
  int end = (start + num_vertices) - 1;
  unsigned short short_start = start;
  unsigned short short_end = end;
  nassertv((int)short_start == start && (int)short_end == end);

  CDWriter cdata(_cycler);
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
//     Function: qpGeomPrimitive::close_primitive
//       Access: Published
//  Description: Indicates that the previous n calls to add_vertex(),
//               since the last call to close_primitive(), have fully
//               defined a new primitive.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
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
    }
    nassertv(num_added >= get_min_num_vertices_per_primitive());
#endif
    cdata->_ends.push_back((int)cdata->_vertices.size());

    if (cdata->_got_minmax) {
      nassertv((cdata->_mins.size() == cdata->_ends.size()) &&
               (cdata->_maxs.size() == cdata->_ends.size()));
    }

  } else {
    // This is a simple primitive type like a triangle: each primitive
    // uses the same number of vertices.  Assert that we added the
    // correct number of vertices.
    nassertv((int)cdata->_vertices.size() % num_vertices_per_primitive == 0);
  }
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
//     Function: qpGeomPrimitive::modify_vertices
//       Access: Published
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
//       Access: Published
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
//       Access: Published
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
//       Access: Published
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
//     Function: qpGeomPrimitive::get_num_vertices_per_primitive
//       Access: Published, Virtual
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
//       Access: Published, Virtual
//  Description: Returns the minimum number of vertices that must be
//               added before close_primitive() may legally be called.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_min_num_vertices_per_primitive() const {
  return 3;
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
//               the ith primitive starts.  
//
//               If i is one more than the highest valid primitive
//               vertex, the return value will be one more than the
//               last valid vertex.  Thus, it is always true that the
//               vertices used by a particular primitive i are the set
//               get_primitive_start(i) <= vi < get_primitive_start(i
//               + 1).
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_primitive_start(int i) const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each
    // primitive uses a different number of vertices.
    CDReader cdata(_cycler);
    nassertr(i >= 0 && i <= (int)cdata->_ends.size(), -1);
    if (i == 0) {
      return 0;
    } else {
      return cdata->_ends[i - 1];
    }

  } else {
    // This is a simple primitive type like a triangle: each primitive
    // uses the same number of vertices.
    return i * num_vertices_per_primitive;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_primitive_num_vertices
//       Access: Published
//  Description: Returns the number of vertices used by the ith
//               primitive.
////////////////////////////////////////////////////////////////////
int qpGeomPrimitive::
get_primitive_num_vertices(int i) const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each
    // primitive uses a different number of vertices.
    CDReader cdata(_cycler);
    nassertr(i >= 0 && i < (int)cdata->_ends.size(), 0);
    if (i == 0) {
      return cdata->_ends[0];
    } else {
      return cdata->_ends[i] - cdata->_ends[i - 1];
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
    if (cdata->_decomposed != (qpGeomPrimitive *)NULL) {
      result = cdata->_decomposed;
      _cycler.release_read(cdata);
      // Record a cache hit, so this element will stay in the cache a
      // while longer.
      qpGeomVertexCacheManager *cache_mgr = 
        qpGeomVertexCacheManager::get_global_ptr();
      cache_mgr->record_primitive(this, result->get_num_bytes());

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
  CDWriter cdata(((qpGeomPrimitive *)this)->_cycler);
  cdata->_decomposed = result;

  // And add *this* object to the cache manager.
  qpGeomVertexCacheManager *cache_mgr = 
    qpGeomVertexCacheManager::get_global_ptr();
  cache_mgr->record_primitive(this, result->get_num_bytes());

  return result;
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
  for (int i = 0; i < num_primitives; i++) {
    indent(out, indent_level + 2)
      << "[";
    int begin = get_primitive_start(i);
    int end = get_primitive_start(i + 1);
    for (int vi = begin; vi < end; vi++) {
      out << " " << get_vertex(vi);
    }
    out << " ]\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::clear_cache
//       Access: Published
//  Description: Removes all of the previously-cached results of
//               decompose().
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
clear_cache() {
  // Probably we shouldn't do anything at all here unless we are
  // running in pipeline stage 0.
  qpGeomVertexCacheManager *cache_mgr = 
    qpGeomVertexCacheManager::get_global_ptr();

  CData *cdata = CDWriter(_cycler);
  if (cdata->_decomposed != (qpGeomPrimitive *)NULL) {
    cache_mgr->remove_primitive(this);
    cdata->_decomposed = NULL;
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
                  const qpGeomVertexData *vertex_data) const {
  CDReader cdata(_cycler);

  const qpGeomVertexFormat *format = vertex_data->get_format();

  int array_index = format->get_array_with(InternalName::get_vertex());
  if (array_index < 0) {
    // No vertex data.
    return;
  }

  const qpGeomVertexArrayFormat *array_format = format->get_array(array_index);
  const qpGeomVertexDataType *data_type = 
    array_format->get_data_type(InternalName::get_vertex());

  int stride = array_format->get_stride();
  int start = data_type->get_start();
  int num_components = data_type->get_num_components();

  CPTA_uchar array_data = vertex_data->get_array(array_index)->get_data();

  PTA_ushort::const_iterator ii;
  for (ii = cdata->_vertices.begin(); ii != cdata->_vertices.end(); ++ii) {
    int index = (int)(*ii);
    const PN_float32 *v = (const PN_float32 *)&array_data[start + index * stride];

    LPoint3f vertex;
    qpGeomVertexData::to_vec3(vertex, v, num_components);

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
void qpGeomPrimitive::
do_rotate() {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Rotating " << get_type() << ": " << (void *)this << "\n";
  }

  PStatTimer timer(_rotate_pcollector);
  CDReader cdata(_cycler);
  CPTA_ushort rotated_vertices = rotate_impl();

  CDWriter cdataw(_cycler, cdata);
  cdataw->_rotated_vertices = rotated_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::remove_cache_entry
//       Access: Private
//  Description: Removes a particular entry from the local cache; it
//               has already been removed from the cache manager.
//               This is only called from GeomVertexCacheManager.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
remove_cache_entry() const {
  // We have to operate on stage 0 of the pipeline, since that's where
  // the cache really counts.  Because of the multistage pipeline, we
  // might not actually have a cache entry there (it might have been
  // added to stage 1 instead).  No big deal if we don't.
  CData *cdata = ((qpGeomPrimitive *)this)->_cycler.write_stage(0);
  cdata->_decomposed = NULL;
  ((qpGeomPrimitive *)this)->_cycler.release_write_stage(0, cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::recompute_minmax
//       Access: Private
//  Description: Recomputes the _min_vertex and _max_vertex values if
//               necessary.
////////////////////////////////////////////////////////////////////
void qpGeomPrimitive::
recompute_minmax() {
  CDWriter cdata(_cycler);
  
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
}
