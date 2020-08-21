/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomPrimitive.cxx
 * @author drose
 * @date 2005-03-06
 */

#include "geomPrimitive.h"
#include "geom.h"
#include "geomPatches.h"
#include "geomVertexData.h"
#include "geomVertexArrayFormat.h"
#include "geomVertexColumn.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"
#include "geomVertexRewriter.h"
#include "geomPoints.h"
#include "geomLines.h"
#include "geomTriangles.h"
#include "preparedGraphicsObjects.h"
#include "internalName.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "ioPtaDatagramInt.h"
#include "indent.h"
#include "pStatTimer.h"

using std::max;
using std::min;

TypeHandle GeomPrimitive::_type_handle;
TypeHandle GeomPrimitive::CData::_type_handle;
TypeHandle GeomPrimitivePipelineReader::_type_handle;

PStatCollector GeomPrimitive::_decompose_pcollector("*:Munge:Decompose");
PStatCollector GeomPrimitive::_doubleside_pcollector("*:Munge:Doubleside");
PStatCollector GeomPrimitive::_reverse_pcollector("*:Munge:Reverse");
PStatCollector GeomPrimitive::_rotate_pcollector("*:Munge:Rotate");

/**
 * Constructs an invalid object.  Only used when reading from bam.
 */
GeomPrimitive::
GeomPrimitive() {
}

/**
 * Required to implement CopyOnWriteObject.
 */
PT(CopyOnWriteObject) GeomPrimitive::
make_cow_copy() {
  return make_copy();
}

/**
 *
 */
GeomPrimitive::
GeomPrimitive(GeomPrimitive::UsageHint usage_hint) {
  CDWriter cdata(_cycler, true);
  cdata->_usage_hint = usage_hint;
}

/**
 *
 */
GeomPrimitive::
GeomPrimitive(const GeomPrimitive &copy) :
  CopyOnWriteObject(copy),
  _cycler(copy._cycler)
{
}

/**
 * The copy assignment operator is not pipeline-safe.  This will completely
 * obliterate all stages of the pipeline, so don't do it for a GeomPrimitive
 * that is actively being used for rendering.
 */
void GeomPrimitive::
operator = (const GeomPrimitive &copy) {
  CopyOnWriteObject::operator = (copy);
  _cycler = copy._cycler;
}

/**
 *
 */
GeomPrimitive::
~GeomPrimitive() {
  release_all();
}

/**
 * Returns the set of GeomRendering bits that represent the rendering
 * properties required to properly render this primitive.
 */
int GeomPrimitive::
get_geom_rendering() const {
  if (is_indexed()) {
    return GR_indexed_other;
  } else {
    return 0;
  }
}

/**
 * Changes the UsageHint hint for this primitive.  See get_usage_hint().
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomPrimitive::
set_usage_hint(GeomPrimitive::UsageHint usage_hint) {
  CDWriter cdata(_cycler, true);
  cdata->_usage_hint = usage_hint;

  if (!cdata->_vertices.is_null()) {
    cdata->_modified = Geom::get_next_modified();
    cdata->_usage_hint = usage_hint;
  }
}

/**
 * Changes the numeric type of the index column.  Normally, this should be
 * either NT_uint16 or NT_uint32.
 *
 * The index type must be large enough to include all of the index values in
 * the primitive.  It may be automatically elevated, if necessary, to a larger
 * index type, by a subsequent call to add_index() that names an index value
 * that does not fit in the index type you specify.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomPrimitive::
set_index_type(GeomPrimitive::NumericType index_type) {
  nassertv(get_max_vertex() <= get_highest_index_value(index_type));

  CDWriter cdata(_cycler, true);
  if (cdata->_index_type != index_type) {
    do_set_index_type(cdata, index_type);
  }
}

/**
 * Adds the indicated vertex to the list of vertex indices used by the
 * graphics primitive type.  To define a primitive, you must call add_vertex()
 * for each vertex of the new primitive, and then call close_primitive() after
 * you have specified the last vertex of each primitive.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomPrimitive::
add_vertex(int vertex) {
  CDWriter cdata(_cycler, true);

  if (gobj_cat.is_spam()) {
    gobj_cat.spam()
      << this << ".add_vertex(" << vertex << ")\n";
  }

  consider_elevate_index_type(cdata, vertex);

  if (requires_unused_vertices()) {
    int num_primitives = get_num_primitives();
    if (num_primitives > 0 &&
        get_num_vertices() == get_primitive_end(num_primitives - 1)) {
      // If we are beginning a new primitive, give the derived class a chance to
      // insert some degenerate vertices.
      if (cdata->_vertices.is_null()) {
        do_make_indexed(cdata);
      }
      append_unused_vertices(cdata->_vertices.get_write_pointer(), vertex);
    }
  }

  if (cdata->_vertices.is_null()) {
    // The nonindexed case.  We can keep the primitive nonindexed only if the
    // vertex number happens to be the next available vertex.
    nassertv(cdata->_num_vertices != -1);
    if (cdata->_num_vertices == 0) {
      cdata->_first_vertex = vertex;
      cdata->_num_vertices = 1;
      cdata->_modified = Geom::get_next_modified();
      cdata->_got_minmax = false;
      return;

    } else if (vertex == cdata->_first_vertex + cdata->_num_vertices) {
      ++cdata->_num_vertices;
      cdata->_modified = Geom::get_next_modified();
      cdata->_got_minmax = false;
      return;
    }

    // Otherwise, we need to suddenly become an indexed primitive.
    do_make_indexed(cdata);
  }

  {
    GeomVertexArrayDataHandle handle(cdata->_vertices.get_write_pointer(),
                                     Thread::get_current_thread());
    int num_rows = handle.get_num_rows();
    handle.set_num_rows(num_rows + 1);

    unsigned char *ptr = handle.get_write_pointer();
    switch (cdata->_index_type) {
    case GeomEnums::NT_uint8:
      ((uint8_t *)ptr)[num_rows] = vertex;
      break;
    case GeomEnums::NT_uint16:
      ((uint16_t *)ptr)[num_rows] = vertex;
      break;
    case GeomEnums::NT_uint32:
      ((uint32_t *)ptr)[num_rows] = vertex;
      break;
    default:
      nassert_raise("unsupported index type");
      break;
    }
  }

  cdata->_modified = Geom::get_next_modified();
  cdata->_got_minmax = false;
}

/**
 * Adds a consecutive sequence of vertices, beginning at start, to the
 * primitive.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomPrimitive::
add_consecutive_vertices(int start, int num_vertices) {
  if (num_vertices == 0) {
    return;
  }
  int end = (start + num_vertices) - 1;

  CDWriter cdata(_cycler, true);

  consider_elevate_index_type(cdata, end);

  int num_primitives = get_num_primitives();
  if (num_primitives > 0 &&
      get_num_vertices() == get_primitive_end(num_primitives - 1)) {
    // If we are beginning a new primitive, give the derived class a chance to
    // insert some degenerate vertices.
    if (cdata->_vertices.is_null()) {
      do_make_indexed(cdata);
    }
    append_unused_vertices(cdata->_vertices.get_write_pointer(), start);
  }

  if (cdata->_vertices.is_null()) {
    // The nonindexed case.  We can keep the primitive nonindexed only if the
    // vertex number happens to be the next available vertex.
    nassertv(cdata->_num_vertices != -1);
    if (cdata->_num_vertices == 0) {
      cdata->_first_vertex = start;
      cdata->_num_vertices = num_vertices;
      cdata->_modified = Geom::get_next_modified();
      cdata->_got_minmax = false;
      return;

    } else if (start == cdata->_first_vertex + cdata->_num_vertices) {
      cdata->_num_vertices += num_vertices;
      cdata->_modified = Geom::get_next_modified();
      cdata->_got_minmax = false;
      return;
    }

    // Otherwise, we need to suddenly become an indexed primitive.
    do_make_indexed(cdata);
  }

  PT(GeomVertexArrayData) array_obj = cdata->_vertices.get_write_pointer();
  int old_num_rows = array_obj->get_num_rows();
  array_obj->set_num_rows(old_num_rows + num_vertices);

  GeomVertexWriter index(array_obj, 0);
  index.set_row_unsafe(old_num_rows);

  for (int v = start; v <= end; ++v) {
    index.set_data1i(v);
  }

  cdata->_modified = Geom::get_next_modified();
  cdata->_got_minmax = false;
}

/**
 * Adds the next n vertices in sequence, beginning from the last vertex added
 * to the primitive + 1.
 *
 * This is most useful when you are building up a primitive and a
 * GeomVertexData at the same time, and you just want the primitive to
 * reference the first n vertices from the data, then the next n, and so on.
 */
void GeomPrimitive::
add_next_vertices(int num_vertices) {
  if (get_num_vertices() == 0) {
    add_consecutive_vertices(0, num_vertices);
  } else {
    add_consecutive_vertices(get_vertex(get_num_vertices() - 1) + 1, num_vertices);
  }
}

/**
 * This ensures that enough memory space for n vertices is allocated, so that
 * you may increase the number of vertices to n without causing a new memory
 * allocation.  This is a performance optimization only; it is especially
 * useful when you know ahead of time that you will be adding n vertices to
 * the primitive.
 *
 * Note that the total you specify here should also include implicit vertices
 * which may be added at each close_primitive() call, according to
 * get_num_unused_vertices_per_primitive().
 *
 * Note also that making this call will implicitly make the primitive indexed
 * if it is not already, which could result in a performance *penalty*.  If
 * you would prefer not to lose the nonindexed nature of your existing
 * GeomPrimitives, check is_indexed() before making this call.
 */
void GeomPrimitive::
reserve_num_vertices(int num_vertices) {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << this << ".reserve_num_vertices(" << num_vertices << ")\n";
  }

  CDWriter cdata(_cycler, true);
  consider_elevate_index_type(cdata, num_vertices);
  do_make_indexed(cdata);
  PT(GeomVertexArrayData) array_obj = cdata->_vertices.get_write_pointer();
  array_obj->reserve_num_rows(num_vertices);
}

/**
 * Indicates that the previous n calls to add_vertex(), since the last call to
 * close_primitive(), have fully defined a new primitive.  Returns true if
 * successful, false otherwise.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
bool GeomPrimitive::
close_primitive() {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  CDWriter cdata(_cycler, true);
  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each primitive
    // uses a different number of vertices.
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
    if (cdata->_ends.get_ref_count() > 1) {
      PTA_int new_ends;
      new_ends.v() = cdata->_ends.v();
      cdata->_ends = new_ends;
    }
    cdata->_ends.push_back(get_num_vertices());

  } else {
#ifndef NDEBUG
    // This is a simple primitive type like a triangle: each primitive uses
    // the same number of vertices.  Assert that we added the correct number
    // of vertices.
    int num_vertices_per_primitive = get_num_vertices_per_primitive();
    int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();

    int num_vertices = get_num_vertices();
    nassertr((num_vertices + num_unused_vertices_per_primitive) % (num_vertices_per_primitive + num_unused_vertices_per_primitive) == 0, false)
#endif
  }

  cdata->_modified = Geom::get_next_modified();

  return true;
}

/**
 * Removes all of the vertices and primitives from the object, so they can be
 * re-added.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomPrimitive::
clear_vertices() {
  CDWriter cdata(_cycler, true);
  cdata->_first_vertex = 0;
  cdata->_num_vertices = 0;

  // Since we might have automatically elevated the index type by adding
  // vertices, we should automatically lower it again when we call
  // clear_vertices().
  cdata->_index_type = NT_uint16;

  cdata->_vertices.clear();
  cdata->_ends.clear();
  cdata->_mins.clear();
  cdata->_maxs.clear();
  cdata->_modified = Geom::get_next_modified();
  cdata->_got_minmax = false;
}

/**
 * Adds the indicated offset to all vertices used by the primitive.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomPrimitive::
offset_vertices(int offset) {
  if (offset == 0) {
    return;
  }

  if (is_indexed()) {
    CDWriter cdata(_cycler, true);

    if (!cdata->_got_minmax) {
      recompute_minmax(cdata);
      nassertv(cdata->_got_minmax);
    }

    consider_elevate_index_type(cdata, cdata->_max_vertex + offset);

    int strip_cut_index = get_strip_cut_index(cdata->_index_type);

    GeomVertexRewriter index(do_modify_vertices(cdata), 0);
    while (!index.is_at_end()) {
      int vertex = index.get_data1i();

      if (vertex != strip_cut_index) {
        index.set_data1i(vertex + offset);
      }
    }

  } else {
    CDWriter cdata(_cycler, true);

    cdata->_first_vertex += offset;
    cdata->_modified = Geom::get_next_modified();
    cdata->_got_minmax = false;

    consider_elevate_index_type(cdata,
                                cdata->_first_vertex + cdata->_num_vertices - 1);
  }
}

/**
 * Adds the indicated offset to the indicated segment of vertices used by the
 * primitive.  Unlike the other version of offset_vertices, this makes the
 * geometry indexed if it isn't already.
 *
 * Note that end_row indicates one past the last row that should be offset.
 * In other words, the number of vertices touched is (end_row - begin_row).
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomPrimitive::
offset_vertices(int offset, int begin_row, int end_row) {
  if (offset == 0 || end_row <= begin_row) {
    return;
  }

  nassertv(begin_row >= 0 && end_row >= 0);
  nassertv(end_row <= get_num_vertices());

  if (!is_indexed() && (begin_row > 0 || end_row < get_num_vertices())) {
    // Make it indexed unless the whole array was specified.
    make_indexed();
  }

  if (is_indexed()) {
    CDWriter cdata(_cycler, true);

    int strip_cut_index = get_strip_cut_index(cdata->_index_type);

    // Calculate the maximum vertex over our range.
    int max_vertex = 0;
    {
      GeomVertexReader index_r(cdata->_vertices.get_read_pointer(), 0);
      index_r.set_row_unsafe(begin_row);
      for (int j = begin_row; j < end_row; ++j) {
        int vertex = index_r.get_data1i();
        if (vertex != strip_cut_index) {
          max_vertex = max(max_vertex, vertex);
        }
      }
    }

    consider_elevate_index_type(cdata, max_vertex + offset);

    GeomVertexRewriter index(do_modify_vertices(cdata), 0);
    index.set_row_unsafe(begin_row);
    for (int j = begin_row; j < end_row; ++j) {
      int vertex = index.get_data1i();
      if (vertex != strip_cut_index) {
        index.set_data1i(vertex + offset);
      }
    }

  } else {
    // The supplied values cover all vertices, so we don't need to make it
    // indexed.
    CDWriter cdata(_cycler, true);

    cdata->_first_vertex += offset;
    cdata->_modified = Geom::get_next_modified();
    cdata->_got_minmax = false;

    consider_elevate_index_type(cdata,
                                cdata->_first_vertex + cdata->_num_vertices - 1);
  }
}

/**
 * Converts the primitive from indexed to nonindexed by duplicating vertices
 * as necessary into the indicated dest GeomVertexData.  Note: does not
 * support primitives with strip cut indices.
 */
void GeomPrimitive::
make_nonindexed(GeomVertexData *dest, const GeomVertexData *source) {
  Thread *current_thread = Thread::get_current_thread();

  int num_vertices, dest_start;
  {
    GeomPrimitivePipelineReader reader(this, current_thread);
    num_vertices = reader.get_num_vertices();
    int strip_cut_index = reader.get_strip_cut_index();

    GeomVertexDataPipelineWriter data_writer(dest, false, current_thread);
    data_writer.check_array_writers();
    dest_start = data_writer.get_num_rows();
    data_writer.set_num_rows(dest_start + num_vertices);

    GeomVertexDataPipelineReader data_reader(source, current_thread);
    data_reader.check_array_readers();

    for (int i = 0; i < num_vertices; ++i) {
      int v = reader.get_vertex(i);
      nassertd(v != strip_cut_index) continue;
      data_writer.copy_row_from(dest_start + i, data_reader, v);
    }
  }

  set_nonindexed_vertices(dest_start, num_vertices);
}

/**
 * Packs the vertices used by the primitive from the indicated source array
 * onto the end of the indicated destination array.
 */
void GeomPrimitive::
pack_vertices(GeomVertexData *dest, const GeomVertexData *source) {
  Thread *current_thread = Thread::get_current_thread();
  if (!is_indexed()) {
    // If the primitive is nonindexed, packing is the same as converting
    // (again) to nonindexed.
    make_nonindexed(dest, source);

  } else {
    // The indexed case: build up a new index as we go.
    CPT(GeomVertexArrayData) orig_vertices = get_vertices();
    PT(GeomVertexArrayData) new_vertices = make_index_data();
    GeomVertexWriter index(new_vertices, 0);
    typedef pmap<int, int> CopiedIndices;
    CopiedIndices copied_indices;

    int num_vertices = get_num_vertices();
    int dest_start = dest->get_num_rows();
    int strip_cut_index = get_strip_cut_index();

    for (int i = 0; i < num_vertices; ++i) {
      int v = get_vertex(i);
      if (v == strip_cut_index) {
        continue;
      }

      // Try to add the relation { v : size() }.  If that succeeds, great; if
      // it doesn't, look up whatever we previously added for v.
      std::pair<CopiedIndices::iterator, bool> result =
        copied_indices.insert(CopiedIndices::value_type(v, (int)copied_indices.size()));
      int v2 = (*result.first).second + dest_start;
      index.add_data1i(v2);

      if (result.second) {
        // This is the first time we've seen vertex v.
        dest->copy_row_from(v2, source, v, current_thread);
      }
    }

    set_vertices(new_vertices);
  }
}

/**
 * Converts the primitive from nonindexed form to indexed form.  This will
 * simply create an index table that is numbered consecutively from
 * get_first_vertex(); it does not automatically collapse together identical
 * vertices that may have been split apart by a previous call to
 * make_nonindexed().
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomPrimitive::
make_indexed() {
  CDWriter cdata(_cycler, true);
  do_make_indexed(cdata);
}

/**
 * Returns the element within the _vertices list at which the nth primitive
 * starts.
 *
 * If i is one more than the highest valid primitive vertex, the return value
 * will be one more than the last valid vertex.  Thus, it is generally true
 * that the vertices used by a particular primitive i are the set
 * get_primitive_start(n) <= vi < get_primitive_start(n + 1) (although this
 * range also includes the unused vertices between primitives).
 */
int GeomPrimitive::
get_primitive_start(int n) const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();
  int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each primitive
    // uses a different number of vertices.
    CDReader cdata(_cycler);
    nassertr(n >= 0 && n <= (int)cdata->_ends.size(), -1);
    if (n == 0) {
      return 0;
    } else {
      return cdata->_ends[n - 1] + num_unused_vertices_per_primitive;
    }

  } else {
    // This is a simple primitive type like a triangle: each primitive uses
    // the same number of vertices.
    return n * (num_vertices_per_primitive + num_unused_vertices_per_primitive);
  }
}

/**
 * Returns the element within the _vertices list at which the nth primitive
 * ends.  This is one past the last valid element for the nth primitive.
 */
int GeomPrimitive::
get_primitive_end(int n) const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each primitive
    // uses a different number of vertices.
    CDReader cdata(_cycler);
    nassertr(n >= 0 && n < (int)cdata->_ends.size(), -1);
    return cdata->_ends[n];

  } else {
    // This is a simple primitive type like a triangle: each primitive uses
    // the same number of vertices.
    int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();
    return n * (num_vertices_per_primitive + num_unused_vertices_per_primitive) + num_vertices_per_primitive;
  }
}

/**
 * Returns the number of vertices used by the nth primitive.  This is the same
 * thing as get_primitive_end(n) - get_primitive_start(n).
 */
int GeomPrimitive::
get_primitive_num_vertices(int n) const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each primitive
    // uses a different number of vertices.
    CDReader cdata(_cycler);
    nassertr(n >= 0 && n < (int)cdata->_ends.size(), 0);
    if (n == 0) {
      return cdata->_ends[0];
    } else {
      int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();
      return cdata->_ends[n] - cdata->_ends[n - 1] - num_unused_vertices_per_primitive;
    }

  } else {
    // This is a simple primitive type like a triangle: each primitive uses
    // the same number of vertices.
    return num_vertices_per_primitive;
  }
}

/**
 * Returns the number of vertices used by all of the primitives.  This is the
 * same as summing get_primitive_num_vertices(n) for n in
 * get_num_primitives().  It is like get_num_vertices except that it excludes
 * all of the degenerate vertices and strip-cut indices.
 */
int GeomPrimitive::
get_num_used_vertices() const {
  int num_primitives = get_num_primitives();

  if (num_primitives > 0) {
    return get_num_vertices() - ((num_primitives - 1) *
           get_num_unused_vertices_per_primitive());
  } else {
    return 0;
  }
}

/**
 * Returns the minimum vertex index number used by the nth primitive in this
 * object.
 */
int GeomPrimitive::
get_primitive_min_vertex(int n) const {
  if (is_indexed()) {
    CPT(GeomVertexArrayData) mins = get_mins();
    nassertr(n >= 0 && n < mins->get_num_rows(), -1);

    GeomVertexReader index(mins, 0);
    index.set_row_unsafe(n);
    return index.get_data1i();
  } else {
    return get_primitive_start(n);
  }
}

/**
 * Returns the maximum vertex index number used by the nth primitive in this
 * object.
 */
int GeomPrimitive::
get_primitive_max_vertex(int n) const {
  if (is_indexed()) {
    CPT(GeomVertexArrayData) maxs = get_maxs();
    nassertr(n >= 0 && n < maxs->get_num_rows(), -1);

    GeomVertexReader index(maxs, 0);
    index.set_row_unsafe(n);
    return index.get_data1i();
  } else {
    return get_primitive_end(n) - 1;
  }
}

/**
 * Decomposes a complex primitive type into a simpler primitive type, for
 * instance triangle strips to triangles, and returns a pointer to the new
 * primitive definition.  If the decomposition cannot be performed, this might
 * return the original object.
 *
 * This method is useful for application code that wants to iterate through
 * the set of triangles on the primitive without having to write handlers for
 * each possible kind of primitive type.
 */
CPT(GeomPrimitive) GeomPrimitive::
decompose() const {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Decomposing " << get_type() << ": " << (void *)this << "\n";
  }

  PStatTimer timer(_decompose_pcollector);
  return decompose_impl();
}

/**
 * Returns a new primitive with the shade_model reversed (if it is flat
 * shaded), if possible.  If the primitive type cannot be rotated, returns the
 * original primitive, unrotated.
 *
 * If the current shade_model indicates flat_vertex_last, this should bring
 * the last vertex to the first position; if it indicates flat_vertex_first,
 * this should bring the first vertex to the last position.
 */
CPT(GeomPrimitive) GeomPrimitive::
rotate() const {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Rotating " << get_type() << ": " << (void *)this << "\n";
  }

  PStatTimer timer(_rotate_pcollector);
  CPT(GeomVertexArrayData) rotated_vertices = rotate_impl();

  if (rotated_vertices == nullptr) {
    // This primitive type can't be rotated.
    return this;
  }

  PT(GeomPrimitive) new_prim = make_copy();
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

/**
 * Duplicates triangles in the primitive so that each triangle is back-to-back
 * with another triangle facing in the opposite direction.  Note that this
 * doesn't affect vertex normals, so this operation alone won't work in the
 * presence of lighting (but see SceneGraphReducer::doubleside()).
 *
 * Also see CullFaceAttrib, which can enable rendering of both sides of a
 * triangle without having to duplicate it (but which doesn't necessarily work
 * in the presence of lighting).
 */
CPT(GeomPrimitive) GeomPrimitive::
doubleside() const {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Doublesiding " << get_type() << ": " << (void *)this << "\n";
  }

  PStatTimer timer(_doubleside_pcollector);
  return doubleside_impl();
}

/**
 * Reverses the winding order in the primitive so that each triangle is facing
 * in the opposite direction it was originally.  Note that this doesn't affect
 * vertex normals, so this operation alone won't work in the presence of
 * lighting (but see SceneGraphReducer::reverse()).
 *
 * Also see CullFaceAttrib, which can change the visible direction of a
 * triangle without having to duplicate it (but which doesn't necessarily work
 * in the presence of lighting).
 */
CPT(GeomPrimitive) GeomPrimitive::
reverse() const {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Reversing " << get_type() << ": " << (void *)this << "\n";
  }

  PStatTimer timer(_reverse_pcollector);
  return reverse_impl();
}

/**
 * Returns a new primitive that is compatible with the indicated shade model,
 * if possible, or NULL if this is not possible.
 *
 * In most cases, this will return either NULL or the original primitive.  In
 * the case of a SM_flat_first_vertex vs.  a SM_flat_last_vertex (or vice-
 * versa), however, it will return a rotated primitive.
 */
CPT(GeomPrimitive) GeomPrimitive::
match_shade_model(GeomPrimitive::ShadeModel shade_model) const {
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
    CPT(GeomPrimitive) rotated = rotate();
    if (rotated.p() == this) {
      // Oops, can't be rotated, sorry.
      return nullptr;
    }
    return rotated;
  }

  // Not compatible, sorry.
  return nullptr;
}

/**
 * Returns a new GeomPoints primitive that represents each of the vertices in
 * the original primitive, rendered exactly once.  If the original primitive
 * is already a GeomPoints primitive, returns the original primitive
 * unchanged.
 */
CPT(GeomPrimitive) GeomPrimitive::
make_points() const {
  if (is_exact_type(GeomPoints::get_class_type())) {
    return this;
  }

  // First, get a list of all of the vertices referenced by the original
  // primitive.
  BitArray bits;
  {
    GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
    reader.get_referenced_vertices(bits);
  }

  // Now construct a new index array with just those bits.
  PT(GeomVertexArrayData) new_vertices = make_index_data();
  new_vertices->unclean_set_num_rows(bits.get_num_on_bits());

  GeomVertexWriter new_index(new_vertices, 0);
  int p = bits.get_lowest_on_bit();
  while (p != -1) {
    while (bits.get_bit(p)) {
      new_index.set_data1i(p);
      ++p;
    }
    int q = bits.get_next_higher_different_bit(p);
    if (q == p) {
      break;
    }
    p = q;
  }

  PT(GeomPrimitive) points = new GeomPoints(UH_dynamic);
  points->set_vertices(new_vertices);

  return points;
}

/**
 * Returns a new GeomLines primitive that represents each of the edges in the
 * original primitive rendered as a line.  If the original primitive is
 * already a GeomLines primitive, returns the original primitive unchanged.
 */
CPT(GeomPrimitive) GeomPrimitive::
make_lines() const {
  if (is_exact_type(GeomLines::get_class_type())) {
    return this;
  }

  PrimitiveType prim_type = get_primitive_type();
  if (prim_type == PT_lines) {
    // It's a line strip, just decompose it.
    return decompose();

  } else if (prim_type != PT_polygons && prim_type != PT_patches) {
    // Don't know how to represent this in wireframe.
    return this;
  }

  if (prim_type == PT_polygons && !is_exact_type(GeomTriangles::get_class_type())) {
    // Decompose tristrips.  We could probably make this more efficient by
    // making a specific implementation of make_lines for GeomTristrips.
    return decompose()->make_lines();
  }

  // Iterate through the primitives.
  int num_primitives = get_num_primitives();
  int verts_per_prim = get_num_vertices_per_primitive();

  PT(GeomVertexArrayData) new_vertices = make_index_data();
  new_vertices->unclean_set_num_rows(num_primitives * verts_per_prim * 2);

  GeomVertexWriter new_index(new_vertices, 0);

  for (int i = 0; i < num_primitives; ++i) {
    int begin = get_primitive_start(i);
    int end = get_primitive_end(i);
    if (begin == end) {
      continue;
    }
    for (int vi = begin; vi < end - 1; vi++) {
      new_index.set_data1i(get_vertex(vi));
      new_index.set_data1i(get_vertex(vi + 1));
    }
    new_index.set_data1i(get_vertex(end - 1));
    new_index.set_data1i(get_vertex(begin));
  }

  PT(GeomPrimitive) lines = new GeomLines(UH_dynamic);
  lines->set_vertices(new_vertices);

  return lines;
}

/**
 * Decomposes a complex primitive type into a simpler primitive type, for
 * instance triangle strips to triangles, puts these in a new GeomPatches
 * object and returns a pointer to the new primitive definition.  If the
 * decomposition cannot be performed, this might return the original object.
 *
 * This method is useful for application code that wants to use tesselation
 * shaders on arbitrary geometry.
 */
CPT(GeomPrimitive) GeomPrimitive::
make_patches() const {
  if (is_exact_type(GeomPatches::get_class_type())) {
    return this;
  }

  CPT(GeomPrimitive) prim = decompose_impl();
  int num_vertices_per_patch = prim->get_num_vertices_per_primitive();

  PT(GeomPrimitive) patches = new GeomPatches(num_vertices_per_patch, get_usage_hint());

  if (prim->is_indexed()) {
    patches->set_vertices(prim->get_vertices());
  } else {
    patches->set_nonindexed_vertices(prim->get_first_vertex(),
                                     prim->get_num_vertices());
  }

  return patches;
}

/**
 * Adds adjacency information to this primitive.  May return null if this type
 * of geometry does not support adjacency information.
 *
 * @since 1.10.0
 */
CPT(GeomPrimitive) GeomPrimitive::
make_adjacency() const {
  return nullptr;
}

/**
 * Returns the number of bytes consumed by the primitive and its index
 * table(s).
 */
int GeomPrimitive::
get_num_bytes() const {
  CDReader cdata(_cycler);
  int num_bytes = cdata->_ends.size() * sizeof(int) + sizeof(GeomPrimitive);
  if (!cdata->_vertices.is_null()) {
    num_bytes += cdata->_vertices.get_read_pointer()->get_data_size_bytes();
  }

  return num_bytes;
}

/**
 * Returns true if the primitive data is currently resident in memory.  If
 * this returns false, the primitive data will be brought back into memory
 * shortly; try again later.
 */
bool GeomPrimitive::
request_resident(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);

  bool resident = true;

  if (!cdata->_vertices.is_null() &&
      !cdata->_vertices.get_read_pointer(current_thread)->request_resident(current_thread)) {
    resident = false;
  }

  if (is_composite() && cdata->_got_minmax) {
    if (!cdata->_mins.is_null() &&
        !cdata->_mins.get_read_pointer(current_thread)->request_resident(current_thread)) {
      resident = false;
    }
    if (!cdata->_maxs.is_null() &&
        !cdata->_maxs.get_read_pointer(current_thread)->request_resident(current_thread)) {
      resident = false;
    }
  }

  return resident;
}

/**
 *
 */
void GeomPrimitive::
output(std::ostream &out) const {
  out << get_type() << ", " << get_num_primitives()
      << ", " << get_num_vertices();
}

/**
 *
 */
void GeomPrimitive::
write(std::ostream &out, int indent_level) const {
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

/**
 * Returns a modifiable pointer to the vertex index list, so application code
 * can directly fiddle with this data.  Use with caution, since there are no
 * checks that the data will be left in a stable state.
 *
 * If this is called on a nonindexed primitive, it will implicitly be
 * converted to an indexed primitive.
 *
 * If num_vertices is not -1, it specifies an artificial limit to the number
 * of vertices in the array.  Otherwise, all of the vertices in the array will
 * be used.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
PT(GeomVertexArrayData) GeomPrimitive::
modify_vertices(int num_vertices) {
  CDWriter cdata(_cycler, true);
  PT(GeomVertexArrayData) vertices = do_modify_vertices(cdata);
  cdata->_num_vertices = num_vertices;
  return vertices;
}

/**
 * Completely replaces the vertex index list with a new table.  Chances are
 * good that you should also replace the ends list with set_ends() at the same
 * time.
 *
 * If num_vertices is not -1, it specifies an artificial limit to the number
 * of vertices in the array.  Otherwise, all of the vertices in the array will
 * be used.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
void GeomPrimitive::
set_vertices(const GeomVertexArrayData *vertices, int num_vertices) {
  CDWriter cdata(_cycler, true);
  cdata->_vertices = (GeomVertexArrayData *)vertices;
  cdata->_num_vertices = num_vertices;

  // Validate the format and make sure to copy its numeric type.
  const GeomVertexArrayFormat *format = vertices->get_array_format();
  nassertv(format->get_num_columns() == 1);
  cdata->_index_type = format->get_column(0)->get_numeric_type();

  cdata->_modified = Geom::get_next_modified();
  cdata->_got_minmax = false;
}

/**
 * Sets the primitive up as a nonindexed primitive, using the indicated vertex
 * range.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
void GeomPrimitive::
set_nonindexed_vertices(int first_vertex, int num_vertices) {
  nassertv(num_vertices != -1);
  CDWriter cdata(_cycler, true);
  cdata->_vertices = nullptr;
  cdata->_first_vertex = first_vertex;
  cdata->_num_vertices = num_vertices;

  cdata->_modified = Geom::get_next_modified();
  cdata->_got_minmax = false;

  // Force the minmax to be recomputed.
  recompute_minmax(cdata);
}

/**
 * Returns a modifiable pointer to the primitive ends array, so application
 * code can directly fiddle with this data.  Use with caution, since there are
 * no checks that the data will be left in a stable state.
 *
 * Note that simple primitive types, like triangles, do not have a ends array:
 * since all the primitives have the same number of vertices, it is not
 * needed.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
PTA_int GeomPrimitive::
modify_ends() {
  CDWriter cdata(_cycler, true);

  cdata->_modified = Geom::get_next_modified();
  cdata->_got_minmax = false;

  if (cdata->_ends.get_ref_count() > 1) {
    PTA_int new_ends;
    new_ends.v() = cdata->_ends.v();
    cdata->_ends = new_ends;
  }
  return cdata->_ends;
}

/**
 * Completely replaces the primitive ends array with a new table.  Chances are
 * good that you should also replace the vertices list with set_vertices() at
 * the same time.
 *
 * Note that simple primitive types, like triangles, do not have a ends array:
 * since all the primitives have the same number of vertices, it is not
 * needed.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
void GeomPrimitive::
set_ends(PTA_int ends) {
  CDWriter cdata(_cycler, true);
  cdata->_ends = ends;

  cdata->_modified = Geom::get_next_modified();
  cdata->_got_minmax = false;
}

/**
 * Explicitly specifies the minimum and maximum vertices, as well as the lists
 * of per-component min and max.
 *
 * Use this method with extreme caution.  It's generally better to let the
 * GeomPrimitive compute these explicitly, unless for some reason you can do
 * it faster and you absolutely need the speed improvement.
 *
 * Note that any modification to the vertex array will normally cause this to
 * be recomputed, unless you set it immediately again.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
void GeomPrimitive::
set_minmax(int min_vertex, int max_vertex,
           GeomVertexArrayData *mins, GeomVertexArrayData *maxs) {
  CDWriter cdata(_cycler, true);
  cdata->_min_vertex = min_vertex;
  cdata->_max_vertex = max_vertex;
  cdata->_mins = mins;
  cdata->_maxs = maxs;

  cdata->_modified = Geom::get_next_modified();
  cdata->_got_minmax = true;
}

/**
 * Undoes a previous call to set_minmax(), and allows the minimum and maximum
 * values to be recomputed normally.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
void GeomPrimitive::
clear_minmax() {
  CDWriter cdata(_cycler, true);
  cdata->_got_minmax = false;
}

/**
 * If the primitive type is a simple type in which all primitives have the
 * same number of vertices, like triangles, returns the number of vertices per
 * primitive.  If the primitive type is a more complex type in which different
 * primitives might have different numbers of vertices, for instance a
 * triangle strip, returns 0.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
int GeomPrimitive::
get_num_vertices_per_primitive() const {
  return 0;
}

/**
 * Returns the minimum number of vertices that must be added before
 * close_primitive() may legally be called.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
int GeomPrimitive::
get_min_num_vertices_per_primitive() const {
  return 3;
}

/**
 * Returns the number of vertices that are added between primitives that
 * aren't, strictly speaking, part of the primitives themselves.  This is
 * used, for instance, to define degenerate triangles to connect otherwise
 * disconnected triangle strips.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
int GeomPrimitive::
get_num_unused_vertices_per_primitive() const {
  return 0;
}

/**
 * Indicates that the data should be enqueued to be prepared in the indicated
 * prepared_objects at the beginning of the next frame.  This will ensure the
 * data is already loaded into the GSG if it is expected to be rendered soon.
 *
 * Use this function instead of prepare_now() to preload datas from a user
 * interface standpoint.
 */
void GeomPrimitive::
prepare(PreparedGraphicsObjects *prepared_objects) {
  if (is_indexed()) {
    prepared_objects->enqueue_index_buffer(this);
  }
}

/**
 * Returns true if the data has already been prepared or enqueued for
 * preparation on the indicated GSG, false otherwise.
 */
bool GeomPrimitive::
is_prepared(PreparedGraphicsObjects *prepared_objects) const {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return true;
  }
  return prepared_objects->is_index_buffer_queued(this);
}

/**
 * Creates a context for the data on the particular GSG, if it does not
 * already exist.  Returns the new (or old) IndexBufferContext.  This assumes
 * that the GraphicsStateGuardian is the currently active rendering context
 * and that it is ready to accept new datas.  If this is not necessarily the
 * case, you should use prepare() instead.
 *
 * Normally, this is not called directly except by the GraphicsStateGuardian;
 * a data does not need to be explicitly prepared by the user before it may be
 * rendered.
 */
IndexBufferContext *GeomPrimitive::
prepare_now(PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) {
  nassertr(is_indexed(), nullptr);

  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return (*ci).second;
  }

  IndexBufferContext *ibc = prepared_objects->prepare_index_buffer_now(this, gsg);
  if (ibc != nullptr) {
    _contexts[prepared_objects] = ibc;
  }
  return ibc;
}

/**
 * Frees the data context only on the indicated object, if it exists there.
 * Returns true if it was released, false if it had not been prepared.
 */
bool GeomPrimitive::
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

/**
 * Frees the context allocated on all objects for which the data has been
 * declared.  Returns the number of contexts which have been freed.
 */
int GeomPrimitive::
release_all() {
  // We have to traverse a copy of the _contexts list, because the
  // PreparedGraphicsObjects object will call clear_prepared() in response to
  // each release_index_buffer(), and we don't want to be modifying the
  // _contexts list while we're traversing it.
  Contexts temp = _contexts;
  int num_freed = (int)_contexts.size();

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    PreparedGraphicsObjects *prepared_objects = (*ci).first;
    IndexBufferContext *ibc = (*ci).second;
    prepared_objects->release_index_buffer(ibc);
  }

  // Now that we've called release_index_buffer() on every known context, the
  // _contexts list should have completely emptied itself.
  nassertr(_contexts.empty(), num_freed);

  return num_freed;
}

/**
 * Returns a registered GeomVertexArrayFormat of the indicated unsigned
 * integer numeric type for storing index values.
 */
const GeomVertexArrayFormat *GeomPrimitive::
get_index_format(NumericType index_type) {
  switch (index_type) {
  case NT_uint8:
    {
      static CPT(GeomVertexArrayFormat) cformat = nullptr;
      if (cformat == nullptr) {
        cformat = make_index_format(NT_uint8);
      }
      return cformat;
    }
  case NT_uint16:
    {
      static CPT(GeomVertexArrayFormat) cformat = nullptr;
      if (cformat == nullptr) {
        cformat = make_index_format(NT_uint16);
      }
      return cformat;
    }
  case NT_uint32:
    {
      static CPT(GeomVertexArrayFormat) cformat = nullptr;
      if (cformat == nullptr) {
        cformat = make_index_format(NT_uint32);
      }
      return cformat;
    }

  default:
    gobj_cat.error()
      << "Not a valid index type: " << index_type << "\n";
    return nullptr;
  }

  return nullptr;
}

/**
 * Removes the indicated PreparedGraphicsObjects table from the data array's
 * table, without actually releasing the data array.  This is intended to be
 * called only from PreparedGraphicsObjects::release_index_buffer(); it should
 * never be called by user code.
 */
void GeomPrimitive::
clear_prepared(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);
  } else {
    // If this assertion fails, clear_prepared() was given a prepared_objects
    // which the data array didn't know about.
    nassert_raise("unknown PreparedGraphicsObjects");
  }
}

/**
 * Returns the largest index value that can be stored in an index of the
 * indicated type, minus one (to leave room for a potential strip cut index)
 */
int GeomPrimitive::
get_highest_index_value(NumericType index_type) {
  // Reserve the highest possible index because implementations use this as a
  // strip-cut index.
  switch (index_type) {
  case NT_uint8:
    return 0xff - 1;

  case NT_uint16:
    return 0xffff - 1;

  case NT_uint32:
    // We don't actually allow use of the sign bit, since all of our functions
    // receive an "int" instead of an "unsigned int".
    return 0x7fffffff - 1;

  default:
    return 0;
  }
}

/**
 * Returns the index of the indicated type that is reserved for use as a strip
 * cut index, if enabled for the primitive.  When the renderer encounters this
 * index, it will restart the primitive.  This is guaranteed not to point to
 * an actual vertex.
 */
int GeomPrimitive::
get_strip_cut_index(NumericType index_type) {
  // Reserve the highest possible index because implementations use this as a
  // strip-cut index.
  switch (index_type) {
  case NT_uint8:
    return 0xff;

  case NT_uint16:
    return 0xffff;

  case NT_uint32:
  default:
    return -1;
  }
}

/**
 * Expands min_point and max_point to include all of the vertices in the Geom,
 * if any (or the data of any point type, for instance, texture coordinates--
 * based on the column name).  found_any is set true if any points are found.
 * It is the caller's responsibility to initialize min_point, max_point, and
 * found_any before calling this function.  It also sets sq_center_dist, which
 * is the square of the maximum distance of the points to the center.  This
 * can be useful when deciding whether a sphere volume might be more
 * appropriate.
 */
void GeomPrimitive::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                  PN_stdfloat &sq_center_dist, bool &found_any,
                  const GeomVertexData *vertex_data,
                  bool got_mat, const LMatrix4 &mat,
                  const InternalName *column_name,
                  Thread *current_thread) const {
  GeomVertexReader reader(vertex_data, column_name, current_thread);
  if (!reader.has_column()) {
    // No vertex data.
    return;
  }

  CDReader cdata(_cycler, current_thread);
  int i = 0;

  if (cdata->_vertices.is_null()) {
    // Nonindexed case.
    nassertv(cdata->_num_vertices != -1);
    if (cdata->_num_vertices == 0) {
      return;
    }

    if (got_mat) {
      // Find the first non-NaN vertex.
      while (!found_any && i < cdata->_num_vertices) {
        reader.set_row(cdata->_first_vertex + i);
        LPoint3 first_vertex = mat.xform_point_general(reader.get_data3());
        if (!first_vertex.is_nan()) {
          min_point = first_vertex;
          max_point = first_vertex;
          sq_center_dist = first_vertex.length_squared();
          found_any = true;
        }
        ++i;
      }

      for (; i < cdata->_num_vertices; ++i) {
        reader.set_row_unsafe(cdata->_first_vertex + i);
        LPoint3 vertex = mat.xform_point_general(reader.get_data3());

        min_point.set(min(min_point[0], vertex[0]),
                      min(min_point[1], vertex[1]),
                      min(min_point[2], vertex[2]));
        max_point.set(max(max_point[0], vertex[0]),
                      max(max_point[1], vertex[1]),
                      max(max_point[2], vertex[2]));
        sq_center_dist = max(sq_center_dist, vertex.length_squared());
      }
    } else {
      // Find the first non-NaN vertex.
      while (!found_any && i < cdata->_num_vertices) {
        reader.set_row(cdata->_first_vertex + i);
        LPoint3 first_vertex = reader.get_data3();
        if (!first_vertex.is_nan()) {
          min_point = first_vertex;
          max_point = first_vertex;
          sq_center_dist = first_vertex.length_squared();
          found_any = true;
        }
        ++i;
      }

      for (; i < cdata->_num_vertices; ++i) {
        reader.set_row_unsafe(cdata->_first_vertex + i);
        const LVecBase3 &vertex = reader.get_data3();

        min_point.set(min(min_point[0], vertex[0]),
                      min(min_point[1], vertex[1]),
                      min(min_point[2], vertex[2]));
        max_point.set(max(max_point[0], vertex[0]),
                      max(max_point[1], vertex[1]),
                      max(max_point[2], vertex[2]));
        sq_center_dist = max(sq_center_dist, vertex.length_squared());
      }
    }

  } else {
    // Indexed case.
    GeomVertexReader index(cdata->_vertices.get_read_pointer(), 0, current_thread);
    if (index.is_at_end()) {
      return;
    }

    int strip_cut_index = get_strip_cut_index(cdata->_index_type);

    if (got_mat) {
      // Find the first non-NaN vertex.
      while (!found_any && !index.is_at_end()) {
        int ii = index.get_data1i();
        if (ii != strip_cut_index) {
          reader.set_row(ii);
          LPoint3 first_vertex = mat.xform_point_general(reader.get_data3());
          if (!first_vertex.is_nan()) {
            min_point = first_vertex;
            max_point = first_vertex;
            sq_center_dist = first_vertex.length_squared();
            found_any = true;
          }
        }
      }

      while (!index.is_at_end()) {
        int ii = index.get_data1i();
        if (ii == strip_cut_index) {
          continue;
        }
        reader.set_row_unsafe(ii);
        LPoint3 vertex = mat.xform_point_general(reader.get_data3());

        min_point.set(min(min_point[0], vertex[0]),
                      min(min_point[1], vertex[1]),
                      min(min_point[2], vertex[2]));
        max_point.set(max(max_point[0], vertex[0]),
                      max(max_point[1], vertex[1]),
                      max(max_point[2], vertex[2]));
        sq_center_dist = max(sq_center_dist, vertex.length_squared());
      }
    } else {
      // Find the first non-NaN vertex.
      while (!found_any && !index.is_at_end()) {
        int ii = index.get_data1i();
        if (ii != strip_cut_index) {
          reader.set_row(ii);
          LVecBase3 first_vertex = reader.get_data3();
          if (!first_vertex.is_nan()) {
            min_point = first_vertex;
            max_point = first_vertex;
            sq_center_dist = first_vertex.length_squared();
            found_any = true;
          }
        }
      }

      while (!index.is_at_end()) {
        int ii = index.get_data1i();
        if (ii == strip_cut_index) {
          continue;
        }
        reader.set_row_unsafe(ii);
        const LVecBase3 &vertex = reader.get_data3();

        min_point.set(min(min_point[0], vertex[0]),
                      min(min_point[1], vertex[1]),
                      min(min_point[2], vertex[2]));
        max_point.set(max(max_point[0], vertex[0]),
                      max(max_point[1], vertex[1]),
                      max(max_point[2], vertex[2]));
        sq_center_dist = max(sq_center_dist, vertex.length_squared());
      }
    }
  }
}

/**
 * Expands radius so that a sphere with the given center point fits all of the
 * vertices.
 *
 * The center point is assumed to already have been transformed by the matrix,
 * if one is given.
 */
void GeomPrimitive::
calc_sphere_radius(const LPoint3 &center, PN_stdfloat &sq_radius,
                   bool &found_any, const GeomVertexData *vertex_data,
                   Thread *current_thread) const {
  GeomVertexReader reader(vertex_data, InternalName::get_vertex(), current_thread);
  if (!reader.has_column()) {
    // No vertex data.
    return;
  }

  if (!found_any) {
    sq_radius = 0.0;
  }

  CDReader cdata(_cycler, current_thread);

  if (cdata->_vertices.is_null()) {
    // Nonindexed case.
    nassertv(cdata->_num_vertices != -1);
    if (cdata->_num_vertices == 0) {
      return;
    }
    found_any = true;

    for (int i = 0; i < cdata->_num_vertices; ++i) {
      reader.set_row_unsafe(cdata->_first_vertex + i);
      const LVecBase3 &vertex = reader.get_data3();

      sq_radius = max(sq_radius, (vertex - center).length_squared());
    }

  } else {
    // Indexed case.
    GeomVertexReader index(cdata->_vertices.get_read_pointer(), 0, current_thread);
    if (index.is_at_end()) {
      return;
    }
    found_any = true;

    int strip_cut_index = get_strip_cut_index(cdata->_index_type);

    while (!index.is_at_end()) {
      int ii = index.get_data1i();
      if (ii == strip_cut_index) {
        continue;
      }
      reader.set_row_unsafe(ii);
      const LVecBase3 &vertex = reader.get_data3();

      sq_radius = max(sq_radius, (vertex - center).length_squared());
    }
  }
}

/**
 * Decomposes a complex primitive type into a simpler primitive type, for
 * instance triangle strips to triangles, and returns a pointer to the new
 * primitive definition.  If the decomposition cannot be performed, this might
 * return the original object.
 *
 * This method is useful for application code that wants to iterate through
 * the set of triangles on the primitive without having to write handlers for
 * each possible kind of primitive type.
 */
CPT(GeomPrimitive) GeomPrimitive::
decompose_impl() const {
  return this;
}

/**
 * The virtual implementation of rotate().
 */
CPT(GeomVertexArrayData) GeomPrimitive::
rotate_impl() const {
  // The default implementation doesn't even try to do anything.
  nassertr(false, nullptr);
  return nullptr;
}

/**
 * The virtual implementation of doubleside().
 */
CPT(GeomPrimitive) GeomPrimitive::
doubleside_impl() const {
  return this;
}

/**
 * The virtual implementation of reverse().
 */
CPT(GeomPrimitive) GeomPrimitive::
reverse_impl() const {
  return this;
}

/**
 * Should be redefined to return true in any primitive that implements
 * append_unused_vertices().
 */
bool GeomPrimitive::
requires_unused_vertices() const {
  return false;
}

/**
 * Called when a new primitive is begun (other than the first primitive), this
 * should add some degenerate vertices between primitives, if the primitive
 * type requires that.  The second parameter is the first vertex that begins
 * the new primitive.
 *
 * This method is only called if requires_unused_vertices(), above, returns
 * true.
 */
void GeomPrimitive::
append_unused_vertices(GeomVertexArrayData *, int) {
}

/**
 * Recomputes the _min_vertex and _max_vertex values if necessary.
 */
void GeomPrimitive::
recompute_minmax(GeomPrimitive::CData *cdata) {
  if (cdata->_vertices.is_null()) {
    // In the nonindexed case, we don't need to do much (the minmax is
    // trivial).
    nassertv(cdata->_num_vertices != -1);
    cdata->_min_vertex = cdata->_first_vertex;
    cdata->_max_vertex = cdata->_first_vertex + cdata->_num_vertices - 1;
    cdata->_mins.clear();
    cdata->_maxs.clear();

  } else {
    int num_vertices = cdata->_vertices.get_read_pointer()->get_num_rows();

    if (num_vertices == 0) {
      // Or if we don't have any vertices, the minmax is also trivial.
      cdata->_min_vertex = 0;
      cdata->_max_vertex = 0;
      cdata->_mins.clear();
      cdata->_maxs.clear();

    } else if (get_num_vertices_per_primitive() == 0) {
      // This is a complex primitive type like a triangle strip; compute the
      // minmax of each primitive (as well as the overall minmax).
      GeomVertexReader index(cdata->_vertices.get_read_pointer(), 0);

      cdata->_mins = make_index_data();
      cdata->_maxs = make_index_data();

      GeomVertexArrayData *mins_data = cdata->_mins.get_write_pointer();
      GeomVertexArrayData *maxs_data = cdata->_maxs.get_write_pointer();

      mins_data->unclean_set_num_rows(cdata->_ends.size());
      maxs_data->unclean_set_num_rows(cdata->_ends.size());

      GeomVertexWriter mins(mins_data, 0);
      GeomVertexWriter maxs(maxs_data, 0);

      int pi = 0;

      unsigned int vertex = index.get_data1i();
      cdata->_min_vertex = vertex;
      cdata->_max_vertex = vertex;
      unsigned int min_prim = vertex;
      unsigned int max_prim = vertex;

      int num_unused_vertices = get_num_unused_vertices_per_primitive();

      for (int vi = 1; vi < num_vertices; ++vi) {
        nassertv(!index.is_at_end());
        nassertv(pi < (int)cdata->_ends.size());

        unsigned int vertex;

        if (vi == cdata->_ends[pi]) {
          // Skip unused vertices, since they won't be very relevant and may
          // contain a strip-cut index, which would distort the result.
          if (num_unused_vertices > 0) {
            vi += num_unused_vertices;
            index.set_row_unsafe(vi);
          }
          vertex = index.get_data1i();

          mins.set_data1i(min_prim);
          maxs.set_data1i(max_prim);
          min_prim = vertex;
          max_prim = vertex;
          ++pi;

        } else {
          vertex = index.get_data1i();
          min_prim = min(min_prim, vertex);
          max_prim = max(max_prim, vertex);
        }

        cdata->_min_vertex = min(cdata->_min_vertex, vertex);
        cdata->_max_vertex = max(cdata->_max_vertex, vertex);
      }

      mins.set_data1i(min_prim);
      maxs.set_data1i(max_prim);
      nassertv(mins.get_array_data()->get_num_rows() == (int)cdata->_ends.size());

    } else {
      // This is a simple primitive type like a triangle; just compute the
      // overall minmax.
      GeomVertexReader index(cdata->_vertices.get_read_pointer(), 0);

      cdata->_mins.clear();
      cdata->_maxs.clear();

      unsigned int vertex = index.get_data1i();
      cdata->_min_vertex = vertex;
      cdata->_max_vertex = vertex;

      for (int vi = 1; vi < num_vertices; ++vi) {
        nassertv(!index.is_at_end());
        unsigned int vertex = index.get_data1i();
        cdata->_min_vertex = min(cdata->_min_vertex, vertex);
        cdata->_max_vertex = max(cdata->_max_vertex, vertex);
      }
    }
  }

  cdata->_got_minmax = true;
}

/**
 * The private implementation of make_indexed().
 */
void GeomPrimitive::
do_make_indexed(CData *cdata) {
  if (cdata->_vertices.is_null()) {
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << this << ".make_indexed()\n";
    }

    nassertv(cdata->_num_vertices != -1);
    cdata->_vertices = make_index_data();

    GeomVertexArrayData *array_data = cdata->_vertices.get_write_pointer();
    array_data->unclean_set_num_rows(cdata->_num_vertices);
    GeomVertexWriter index(array_data, 0);

    for (int i = 0; i < cdata->_num_vertices; ++i) {
      index.set_data1i(i + cdata->_first_vertex);
    }
    cdata->_num_vertices = -1;
  }
}

/**
 * If the indicated new vertex index won't fit in the specified index type,
 * automatically elevates the index type to the next available size.
 */
void GeomPrimitive::
consider_elevate_index_type(CData *cdata, int vertex) {
  // Note that we reserve the highest possible index of a particular index
  // type (ie.  -1) because this is commonly used as a strip-cut (also known
  // as primitive restart) index.
  switch (cdata->_index_type) {
  case NT_uint8:
    if (vertex >= 0xff) {
      do_set_index_type(cdata, NT_uint16);
    }
    break;

  case NT_uint16:
    if (vertex >= 0xffff) {
      do_set_index_type(cdata, NT_uint32);
    }
    break;

  case NT_uint32:
    // Not much we can do here.
    nassertv(vertex < 0x7fffffff);
    break;

  default:
    break;
  }
}

/**
 * The private implementation of set_index_type().
 */
void GeomPrimitive::
do_set_index_type(CData *cdata, GeomPrimitive::NumericType index_type) {
  int old_strip_cut_index = get_strip_cut_index(cdata->_index_type);
  int new_strip_cut_index = get_strip_cut_index(index_type);

  cdata->_index_type = index_type;

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << this << ".set_index_type(" << index_type << ")\n";
  }

  if (!cdata->_vertices.is_null()) {
    CPT(GeomVertexArrayFormat) new_format = get_index_format();

    CPT(GeomVertexArrayData) array_obj = cdata->_vertices.get_read_pointer();
    if (array_obj->get_array_format() != new_format) {
      PT(GeomVertexArrayData) new_vertices = make_index_data();
      new_vertices->set_num_rows(array_obj->get_num_rows());

      GeomVertexReader from(array_obj, 0);
      GeomVertexWriter to(new_vertices, 0);

      while (!from.is_at_end()) {
        int index = from.get_data1i();
        if (index == old_strip_cut_index) {
          index = new_strip_cut_index;
        }
        to.set_data1i(index);
      }
      cdata->_vertices = new_vertices;
      cdata->_got_minmax = false;
    }
  }
}

/**
 * The private implementation of modify_vertices().
 */
PT(GeomVertexArrayData) GeomPrimitive::
do_modify_vertices(GeomPrimitive::CData *cdata) {
  if (cdata->_vertices.is_null()) {
    do_make_indexed(cdata);
  }

  PT(GeomVertexArrayData) vertices = cdata->_vertices.get_write_pointer();

  cdata->_modified = Geom::get_next_modified();
  cdata->_got_minmax = false;
  return vertices;
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void GeomPrimitive::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  manager->write_cdata(dg, _cycler);
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void GeomPrimitive::
finalize(BamReader *manager) {
  const GeomVertexArrayData *vertices = get_vertices();
  if (vertices != nullptr) {
    set_usage_hint(vertices->get_usage_hint());
  }
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new GeomPrimitive.
 */
void GeomPrimitive::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
  manager->register_finalize(this);
}

/**
 *
 */
CycleData *GeomPrimitive::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void GeomPrimitive::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_uint8(_shade_model);
  dg.add_int32(_first_vertex);
  dg.add_int32(_num_vertices);
  dg.add_uint8(_index_type);
  dg.add_uint8(_usage_hint);

  manager->write_pointer(dg, _vertices.get_read_pointer());
  WRITE_PTA(manager, dg, IPD_int::write_datagram, _ends);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int GeomPrimitive::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  _vertices = DCAST(GeomVertexArrayData, p_list[pi++]);

  if (manager->get_file_minor_ver() < 6 && !_vertices.is_null()) {
    // Older bam files might have a meaningless number in _num_vertices if the
    // primitive is indexed.  Nowadays, this number is always considered
    // meaningful unless it is -1.
    _num_vertices = -1;
  }

  return pi;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new GeomPrimitive.
 */
void GeomPrimitive::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _shade_model = (ShadeModel)scan.get_uint8();
  _first_vertex = scan.get_int32();
  _num_vertices = scan.get_int32();
  _index_type = (NumericType)scan.get_uint8();
  _usage_hint = (UsageHint)scan.get_uint8();

  manager->read_pointer(scan);
  READ_PTA(manager, scan, IPD_int::read_datagram, _ends);

  _modified = Geom::get_next_modified();
  _got_minmax = false;
}

/**
 * Ensures that the primitive's minmax cache has been computed.
 */
void GeomPrimitivePipelineReader::
check_minmax() const {
  if (!_cdata->_got_minmax) {
    // We'll need to get a fresh pointer, since another thread might already
    // have modified the pointer on the object since we queried it.
    {
#ifdef DO_PIPELINING
      unref_delete((CycleData *)_cdata);
#endif
      GeomPrimitive::CDWriter fresh_cdata(((GeomPrimitive *)_object.p())->_cycler,
                                          false, _current_thread);
      ((GeomPrimitivePipelineReader *)this)->_cdata = fresh_cdata;
#ifdef DO_PIPELINING
      _cdata->ref();
#endif

      if (!fresh_cdata->_got_minmax) {
        // The cache is still stale.  We have to do the work of freshening it.
        ((GeomPrimitive *)_object.p())->recompute_minmax(fresh_cdata);
        nassertv(fresh_cdata->_got_minmax);
      }

      // When fresh_cdata goes out of scope, its write lock is released, and
      // _cdata reverts to our usual convention of an unlocked copy of the
      // data.
    }
  }

  nassertv(_cdata->_got_minmax);
}

/**
 *
 */
int GeomPrimitivePipelineReader::
get_first_vertex() const {
  if (_vertices.is_null()) {
    return _cdata->_first_vertex;
  }

  size_t size = _vertices_cdata->_buffer.get_size();
  if (size == 0) {
    return 0;
  }

  GeomVertexReader index(_vertices, 0);
  return index.get_data1i();
}

/**
 * Returns the ith vertex index in the table.
 */
int GeomPrimitivePipelineReader::
get_vertex(int i) const {
  if (!_vertices.is_null()) {
    // The indexed case.
    nassertr(i >= 0 && i < get_num_vertices(), -1);

    const unsigned char *ptr = get_read_pointer(true);
    switch (_cdata->_index_type) {
    case GeomEnums::NT_uint8:
      return ((uint8_t *)ptr)[i];
      break;
    case GeomEnums::NT_uint16:
      return ((uint16_t *)ptr)[i];
      break;
    case GeomEnums::NT_uint32:
      return ((uint32_t *)ptr)[i];
      break;
    default:
      nassert_raise("unsupported index type");
      return -1;
    }

  } else {
    // The nonindexed case.
    return _cdata->_first_vertex + i;
  }
}

/**
 *
 */
int GeomPrimitivePipelineReader::
get_num_primitives() const {
  int num_vertices_per_primitive = _object->get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each primitive
    // uses a different number of vertices.
    return _cdata->_ends.size();

  } else {
    // This is a simple primitive type like a triangle: each primitive uses
    // the same number of vertices.
    return (get_num_vertices() / num_vertices_per_primitive);
  }
}

/**
 * Turns on all the bits corresponding to the vertices that are referenced
 * by this GeomPrimitive.
 */
void GeomPrimitivePipelineReader::
get_referenced_vertices(BitArray &bits) const {
  int num_vertices = get_num_vertices();

  if (is_indexed()) {
    int strip_cut_index = get_strip_cut_index();
    const unsigned char *ptr = get_read_pointer(true);
    switch (get_index_type()) {
    case GeomEnums::NT_uint8:
      for (int vi = 0; vi < num_vertices; ++vi) {
        int index = ((const uint8_t *)ptr)[vi];
        if (index != strip_cut_index) {
          bits.set_bit(index);
        }
      }
      break;
    case GeomEnums::NT_uint16:
      for (int vi = 0; vi < num_vertices; ++vi) {
        int index = ((const uint16_t *)ptr)[vi];
        if (index != strip_cut_index) {
          bits.set_bit(index);
        }
      }
      break;
    case GeomEnums::NT_uint32:
      for (int vi = 0; vi < num_vertices; ++vi) {
        int index = ((const uint32_t *)ptr)[vi];
        if (index != strip_cut_index) {
          bits.set_bit(index);
        }
      }
      break;
    default:
      nassert_raise("unsupported index type");
      break;
    }
  } else {
    // Nonindexed case.
    bits.set_range(get_first_vertex(), num_vertices);
  }
}

/**
 *
 */
bool GeomPrimitivePipelineReader::
check_valid(const GeomVertexDataPipelineReader *data_reader) const {
  if (get_num_vertices() != 0 &&
      data_reader->get_num_arrays() > 0 &&
      get_max_vertex() >= data_reader->get_num_rows()) {

#ifndef NDEBUG
    gobj_cat.error()
      << get_object()->get_type() << " references vertices up to "
      << get_max_vertex() << ", but GeomVertexData has only "
      << data_reader->get_num_rows() << " rows!\n";
#endif
    return false;
  }

  return true;
}
