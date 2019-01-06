/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomPrimitive.I
 * @author drose
 * @date 2005-03-06
 */

/**
 * Returns the ShadeModel hint for this primitive.  This is intended as a hint
 * to the renderer to tell it how the per-vertex colors and normals are
 * applied.
 */
INLINE GeomPrimitive::ShadeModel GeomPrimitive::
get_shade_model() const {
  CDReader cdata(_cycler);
  return cdata->_shade_model;
}

/**
 * Changes the ShadeModel hint for this primitive.  This is different from the
 * ShadeModelAttrib that might also be applied from the scene graph.  This
 * does not affect the shade model that is in effect when rendering, but
 * rather serves as a hint to the renderer to tell it how the per-vertex
 * colors and normals on this primitive are applied.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
INLINE void GeomPrimitive::
set_shade_model(GeomPrimitive::ShadeModel shade_model) {
  CDWriter cdata(_cycler, true);
  cdata->_shade_model = shade_model;
}

/**
 * Returns the usage hint for this primitive.  See geomEnums.h.  This has
 * nothing to do with the usage hint associated with the primitive's vertices;
 * this only specifies how often the vertex indices that define the primitive
 * will be modified.
 *
 * It is perfectly legal (and, in fact, common) for a GeomPrimitive to have
 * UH_static on itself, while referencing vertex data with UH_dynamic.  This
 * means that the vertices themselves will be animated, but the primitive will
 * always reference the same set of vertices from the pool.
 */
INLINE GeomPrimitive::UsageHint GeomPrimitive::
get_usage_hint() const {
  CDReader cdata(_cycler);
  return cdata->_usage_hint;
}

/**
 * Returns the numeric type of the index column.  Normally, this will be
 * either NT_uint16 or NT_uint32.
 */
INLINE GeomPrimitive::NumericType GeomPrimitive::
get_index_type() const {
  CDReader cdata(_cycler);
  return cdata->_index_type;
}

/**
 * Returns true if the primitive is a composite primitive such as a tristrip
 * or trifan, or false if it is a fundamental primitive such as a collection
 * of triangles.
 */
INLINE bool GeomPrimitive::
is_composite() const {
  return (get_num_vertices_per_primitive() == 0);
}

/**
 * Returns true if the primitive is indexed, false otherwise.  An indexed
 * primitive stores a table of index numbers into its GeomVertexData, so that
 * it can reference the vertices in any order.  A nonindexed primitive, on the
 * other hand, stores only the first vertex number and number of vertices
 * used, so that it can only reference the vertices consecutively.
 */
INLINE bool GeomPrimitive::
is_indexed() const {
  CDReader cdata(_cycler);
  return (!cdata->_vertices.is_null());
}

/**
 * Returns the first vertex number referenced by the primitive.  This is
 * particularly important in the case of a nonindexed primitive, in which case
 * get_first_vertex() and get_num_vertices() completely define the extent of
 * the vertex range.
 */
INLINE int GeomPrimitive::
get_first_vertex() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  return reader.get_first_vertex();
}

/**
 * Returns the number of indices used by all the primitives in this object.
 */
INLINE int GeomPrimitive::
get_num_vertices() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  return reader.get_num_vertices();
}

/**
 * Returns the ith vertex index in the table.
 */
INLINE int GeomPrimitive::
get_vertex(int i) const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  return reader.get_vertex(i);
}

/**
 * Returns the number of individual primitives stored within this object.  All
 * primitives are the same type.
 */
INLINE int GeomPrimitive::
get_num_primitives() const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    // This is a complex primitive type like a triangle strip: each primitive
    // uses a different number of vertices.
    CDReader cdata(_cycler);
    return cdata->_ends.size();

  } else {
    // This is a simple primitive type like a triangle: each primitive uses
    // the same number of vertices.
    return (get_num_vertices() / num_vertices_per_primitive);
  }
}

/**
 * Returns the number of triangles or other fundamental type (such as line
 * segments) represented by all the primitives in this object.
 */
INLINE int GeomPrimitive::
get_num_faces() const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    int num_primitives = get_num_primitives();
    int num_vertices = get_num_vertices();
    int min_num_vertices_per_primitive = get_min_num_vertices_per_primitive();
    int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();
    return num_vertices - (num_primitives * (min_num_vertices_per_primitive - 1)) - ((num_primitives - 1) * num_unused_vertices_per_primitive);
  } else {
    return get_num_primitives();
  }
}

/**
 * Returns the number of triangles or other fundamental type (such as line
 * segments) represented by the nth primitive in this object.
 */
INLINE int GeomPrimitive::
get_primitive_num_faces(int n) const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    return get_primitive_num_vertices(n) - get_min_num_vertices_per_primitive() + 1;
  } else {
    return 1;
  }
}

/**
 * Returns the minimum vertex index number used by all the primitives in this
 * object.
 */
INLINE int GeomPrimitive::
get_min_vertex() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  reader.check_minmax();
  return reader.get_min_vertex();
}

/**
 * Returns the maximum vertex index number used by all the primitives in this
 * object.
 */
INLINE int GeomPrimitive::
get_max_vertex() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  reader.check_minmax();
  return reader.get_max_vertex();
}

/**
 * Returns the number of bytes stored in the vertices array.
 */
INLINE int GeomPrimitive::
get_data_size_bytes() const {
  CDReader cdata(_cycler);
  nassertr(!cdata->_vertices.is_null(), 0);
  return cdata->_vertices.get_read_pointer()->get_data_size_bytes();
}

/**
 * Returns a sequence number which is guaranteed to change at least every time
 * the vertex index array is modified.
 */
INLINE UpdateSeq GeomPrimitive::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

/**
 * Verifies that the primitive only references vertices that actually exist
 * within the indicated GeomVertexData.  Returns true if the primitive appears
 * to be valid, false otherwise.
 */
INLINE bool GeomPrimitive::
check_valid(const GeomVertexData *vertex_data) const {
  Thread *current_thread = Thread::get_current_thread();
  GeomVertexDataPipelineReader data_reader(vertex_data, current_thread);
  data_reader.check_array_readers();
  return check_valid(&data_reader);
}

/**
 *
 */
INLINE bool GeomPrimitive::
check_valid(const GeomVertexDataPipelineReader *data_reader) const {
  GeomPrimitivePipelineReader reader(this, data_reader->get_current_thread());
  reader.check_minmax();
  return reader.check_valid(data_reader);
}

/**
 * Returns a const pointer to the vertex index array so application code can
 * read it directly.  This might return NULL if the primitive is nonindexed.
 * Do not attempt to modify the returned array; use modify_vertices() or
 * set_vertices() for this.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
INLINE CPT(GeomVertexArrayData) GeomPrimitive::
get_vertices() const {
  CDReader cdata(_cycler);
  return cdata->_vertices.get_read_pointer();
}

/**
 * Equivalent to get_vertices().get_handle().
 */
INLINE CPT(GeomVertexArrayDataHandle) GeomPrimitive::
get_vertices_handle(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return new GeomVertexArrayDataHandle(cdata->_vertices.get_read_pointer(current_thread), current_thread);
}

/**
 * Equivalent to modify_vertices().get_handle().
 */
INLINE PT(GeomVertexArrayDataHandle) GeomPrimitive::
modify_vertices_handle(Thread *current_thread) {
  CDWriter cdata(_cycler, true, current_thread);
  return new GeomVertexArrayDataHandle(do_modify_vertices(cdata), current_thread);
}

/**
 * A convenience function to return the gap between successive index numbers,
 * in bytes, of the index data.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
INLINE int GeomPrimitive::
get_index_stride() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  return reader.get_index_stride();
}

/**
 * If relevant, returns the index value that may be used in some cases to
 * signify the end of a primitive.  This is typically the highest value that
 * the numeric type can store.
 */
INLINE int GeomPrimitive::
get_strip_cut_index() const {
  CDReader cdata(_cycler);
  return get_strip_cut_index(cdata->_index_type);
}

/**
 * Returns a const pointer to the primitive ends array so application code can
 * read it directly.  Do not attempt to modify the returned array; use
 * modify_ends() or set_ends() for this.
 *
 * Note that simple primitive types, like triangles, do not have a ends array:
 * since all the primitives have the same number of vertices, it is not
 * needed.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
INLINE CPTA_int GeomPrimitive::
get_ends() const {
  CDReader cdata(_cycler);
  return cdata->_ends;
}

/**
 * Returns a const pointer to the primitive mins array so application code can
 * read it directly.  Do not attempt to modify the returned array; use
 * set_minmax() for this.
 *
 * Note that simple primitive types, like triangles, do not have a mins array.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
INLINE CPT(GeomVertexArrayData) GeomPrimitive::
get_mins() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  reader.check_minmax();
  return reader.get_mins();
}

/**
 * Returns a const pointer to the primitive maxs array so application code can
 * read it directly.  Do not attempt to modify the returned array; use
 * set_minmax().
 *
 * Note that simple primitive types, like triangles, do not have a maxs array.
 *
 * This method is intended for low-level usage only.  There are higher-level
 * methods for more common usage.  We recommend you do not use this method
 * directly.  If you do, be sure you know what you are doing!
 */
INLINE CPT(GeomVertexArrayData) GeomPrimitive::
get_maxs() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  reader.check_minmax();
  return reader.get_maxs();
}

/**
 * Adds several vertices in a row.
 */
INLINE void GeomPrimitive::
add_vertices(int v1, int v2) {
  add_vertex(v1);
  add_vertex(v2);
}

/**
 * Adds several vertices in a row.
 */
INLINE void GeomPrimitive::
add_vertices(int v1, int v2, int v3) {
  add_vertex(v1);
  add_vertex(v2);
  add_vertex(v3);
}

/**
 * Adds several vertices in a row.
 */
INLINE void GeomPrimitive::
add_vertices(int v1, int v2, int v3, int v4) {
  add_vertex(v1);
  add_vertex(v2);
  add_vertex(v3);
  add_vertex(v4);
}

/**
 * Returns a registered format appropriate for using to store the index table.
 */
INLINE const GeomVertexArrayFormat *GeomPrimitive::
get_index_format() const {
  return get_index_format(get_index_type());
}

/**
 * Creates and returns a new, empty index table.
 */
INLINE PT(GeomVertexArrayData) GeomPrimitive::
make_index_data() const {
  return new GeomVertexArrayData(get_index_format(), get_usage_hint());
}

/**
 * Returns a registered format appropriate for using to store the index table.
 */
INLINE CPT(GeomVertexArrayFormat) GeomPrimitive::
make_index_format(NumericType index_type) {
  PT(GeomVertexArrayFormat) format = new GeomVertexArrayFormat;
  // It's important that the index format *not* respect the global setting of
  // vertex-column-alignment.  It needs to be tightly packed, so we specify an
  // explict column_alignment of 1.
  format->add_column(InternalName::get_index(), 1, index_type, C_index, 0, 1);
  return GeomVertexArrayFormat::register_format(format);
}

/**
 *
 */
INLINE GeomPrimitive::CData::
CData() :
  _shade_model(SM_smooth),
  _first_vertex(0),
  _num_vertices(0),
  _index_type(NT_uint16),
  _usage_hint(UH_unspecified),
  _got_minmax(true),
  _min_vertex(0),
  _max_vertex(0)
{
}

/**
 *
 */
INLINE GeomPrimitive::CData::
CData(const GeomPrimitive::CData &copy) :
  _shade_model(copy._shade_model),
  _first_vertex(copy._first_vertex),
  _num_vertices(copy._num_vertices),
  _index_type(copy._index_type),
  _usage_hint(copy._usage_hint),
  _vertices(copy._vertices),
  _ends(copy._ends),
  _mins(copy._mins),
  _maxs(copy._maxs),
  _modified(copy._modified),
  _got_minmax(copy._got_minmax),
  _min_vertex(copy._min_vertex),
  _max_vertex(copy._max_vertex)
{
}
/**
 *
 */
INLINE GeomPrimitivePipelineReader::
GeomPrimitivePipelineReader(CPT(GeomPrimitive) object,
                            Thread *current_thread) :
  _object(std::move(object)),
  _current_thread(current_thread),
#ifndef CPPPARSER
  _cdata(_object->_cycler.read_unlocked(current_thread)),
#endif
  _vertices_cdata(nullptr)
{
  nassertv(_object->test_ref_count_nonzero());
#ifdef DO_PIPELINING
  _cdata->ref();
#endif  // DO_PIPELINING

  if (!_cdata->_vertices.is_null()) {
    _vertices = _cdata->_vertices.get_read_pointer(current_thread);
    _vertices_cdata = _vertices->_cycler.read_unlocked(current_thread);
#ifdef DO_PIPELINING
    _vertices_cdata->ref();
#endif  // DO_PIPELINING
    // We must grab the lock *after* we have incremented the reference count,
    // above.
    _vertices_cdata->_rw_lock.acquire();
  }
}

/**
 *
 */
INLINE GeomPrimitivePipelineReader::
~GeomPrimitivePipelineReader() {
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG
  // _object->_cycler.release_read(_cdata);

#ifdef DO_PIPELINING
  unref_delete((CycleData *)_cdata);
#endif  // DO_PIPELINING

  if (_vertices_cdata != nullptr) {
    // We must release the lock *before* we decrement the reference count,
    // below.
    _vertices_cdata->_rw_lock.release();

#ifdef DO_PIPELINING
    unref_delete((CycleData *)_vertices_cdata);
#endif  // DO_PIPELINING
  }

#ifdef _DEBUG
  _object = nullptr;
  _cdata = nullptr;
#endif  // _DEBUG
}

/**
 *
 */
INLINE const GeomPrimitive *GeomPrimitivePipelineReader::
get_object() const {
  return _object;
}

/**
 *
 */
INLINE Thread *GeomPrimitivePipelineReader::
get_current_thread() const {
  return _current_thread;
}

/**
 *
 */
INLINE GeomPrimitivePipelineReader::ShadeModel GeomPrimitivePipelineReader::
get_shade_model() const {
  return _cdata->_shade_model;
}

/**
 *
 */
INLINE GeomPrimitivePipelineReader::UsageHint GeomPrimitivePipelineReader::
get_usage_hint() const {
  return _cdata->_usage_hint;
}

/**
 *
 */
INLINE GeomPrimitivePipelineReader::NumericType GeomPrimitivePipelineReader::
get_index_type() const {
  return _cdata->_index_type;
}

/**
 *
 */
INLINE bool GeomPrimitivePipelineReader::
is_indexed() const {
  return (!_vertices.is_null());
}

/**
 *
 */
INLINE int GeomPrimitivePipelineReader::
get_num_vertices() const {
  if (_cdata->_num_vertices != -1) {
    return _cdata->_num_vertices;
  } else {
    nassertr(!_vertices.is_null(), 0);
    size_t stride = _vertices->_array_format->get_stride();
    nassertr(stride != 0, 0);
    return get_data_size_bytes() / stride;
  }
}

/**
 *
 */
INLINE int GeomPrimitivePipelineReader::
get_min_vertex() const {
  nassertr(_cdata->_got_minmax, 0);
  return _cdata->_min_vertex;
}

/**
 *
 */
INLINE int GeomPrimitivePipelineReader::
get_max_vertex() const {
  nassertr(_cdata->_got_minmax, 0);
  return _cdata->_max_vertex;
}

/**
 * Returns the number of bytes stored in the vertices array.
 */
INLINE int GeomPrimitivePipelineReader::
get_data_size_bytes() const {
  return _vertices_cdata->_buffer.get_size();
}

/**
 *
 */
INLINE UpdateSeq GeomPrimitivePipelineReader::
get_modified() const {
  return _cdata->_modified;
}

/**
 *
 */
INLINE int GeomPrimitivePipelineReader::
get_index_stride() const {
  nassertr(is_indexed(), 0);
  return _vertices->_array_format->get_stride();
}

/**
 *
 */
INLINE const unsigned char *GeomPrimitivePipelineReader::
get_read_pointer(bool force) const {
  ((GeomVertexArrayData *)_vertices.p())->mark_used();
  return _vertices_cdata->_buffer.get_read_pointer(force);
}

/**
 *
 */
INLINE int GeomPrimitivePipelineReader::
get_strip_cut_index() const {
  return GeomPrimitive::get_strip_cut_index(_cdata->_index_type);
}

/**
 *
 */
INLINE CPTA_int GeomPrimitivePipelineReader::
get_ends() const {
  return _cdata->_ends;
}

/**
 *
 */
INLINE CPT(GeomVertexArrayData) GeomPrimitivePipelineReader::
get_mins() const {
  nassertr(is_indexed(), nullptr);
  nassertr(_cdata->_got_minmax, nullptr);
  return _cdata->_mins.get_read_pointer();
}

/**
 *
 */
INLINE CPT(GeomVertexArrayData) GeomPrimitivePipelineReader::
get_maxs() const {
  nassertr(is_indexed(), nullptr);
  nassertr(_cdata->_got_minmax, nullptr);
  return _cdata->_maxs.get_read_pointer();
}

/**
 *
 */
INLINE IndexBufferContext *GeomPrimitivePipelineReader::
prepare_now(PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) const {
  return ((GeomPrimitive *)_object.p())->prepare_now(prepared_objects, gsg);
}

/**
 * Calls the appropriate method on the GSG to draw the primitive.
 */
INLINE bool GeomPrimitivePipelineReader::
draw(GraphicsStateGuardianBase *gsg, bool force) const {
  return _object->draw(gsg, this, force);
}

INLINE std::ostream &
operator << (std::ostream &out, const GeomPrimitive &obj) {
  obj.output(out);
  return out;
}
