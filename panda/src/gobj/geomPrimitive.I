// Filename: geomPrimitive.I
// Created by:  drose (06Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_shade_model
//       Access: Published
//  Description: Returns the ShadeModel hint for this primitive.
//               This is intended as a hint to the renderer to tell it
//               how the per-vertex colors and normals are applied.
////////////////////////////////////////////////////////////////////
INLINE GeomPrimitive::ShadeModel GeomPrimitive::
get_shade_model() const {
  CDReader cdata(_cycler);
  return cdata->_shade_model;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::set_shade_model
//       Access: Published
//  Description: Changes the ShadeModel hint for this primitive.
//               This is different from the ShadeModelAttrib that
//               might also be applied from the scene graph.  This
//               does not affect the shade model that is in effect
//               when rendering, but rather serves as a hint to the
//               renderer to tell it how the per-vertex colors and
//               normals on this primitive are applied.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
INLINE void GeomPrimitive::
set_shade_model(GeomPrimitive::ShadeModel shade_model) {
  CDWriter cdata(_cycler, true);
  cdata->_shade_model = shade_model;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_usage_hint
//       Access: Published
//  Description: Returns the usage hint for this primitive.  See
//               geomEnums.h.  This has nothing to do with the usage
//               hint associated with the primitive's vertices; this
//               only specifies how often the vertex indices that
//               define the primitive will be modified.
//
//               It is perfectly legal (and, in fact, common) for a
//               GeomPrimitive to have UH_static on itself, while
//               referencing vertex data with UH_dynamic.  This means
//               that the vertices themselves will be animated, but
//               the primitive will always reference the same set of
//               vertices from the pool.
////////////////////////////////////////////////////////////////////
INLINE GeomPrimitive::UsageHint GeomPrimitive::
get_usage_hint() const {
  CDReader cdata(_cycler);
  return cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_index_type
//       Access: Public
//  Description: Returns the numeric type of the index column.
//               Normally, this will be either NT_uint16 or NT_uint32.
////////////////////////////////////////////////////////////////////
INLINE GeomPrimitive::NumericType GeomPrimitive::
get_index_type() const {
  CDReader cdata(_cycler);
  return cdata->_index_type;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::is_composite
//       Access: Published
//  Description: Returns true if the primitive is a composite
//               primitive such as a tristrip or trifan, or false if
//               it is a fundamental primitive such as a collection of
//               triangles.
////////////////////////////////////////////////////////////////////
INLINE bool GeomPrimitive::
is_composite() const {
  return (get_num_vertices_per_primitive() == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::is_indexed
//       Access: Published
//  Description: Returns true if the primitive is indexed, false
//               otherwise.  An indexed primitive stores a table of
//               index numbers into its GeomVertexData, so that it can
//               reference the vertices in any order.  A nonindexed
//               primitive, on the other hand, stores only the first
//               vertex number and number of vertices used, so that it
//               can only reference the vertices consecutively.
////////////////////////////////////////////////////////////////////
INLINE bool GeomPrimitive::
is_indexed() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  return reader.is_indexed();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_first_vertex
//       Access: Published
//  Description: Returns the first vertex number referenced by the
//               primitive.  This is particularly important in the
//               case of a nonindexed primitive, in which case
//               get_first_vertex() and get_num_vertices() completely
//               define the extent of the vertex range.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_first_vertex() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  return reader.get_first_vertex();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_num_vertices
//       Access: Published
//  Description: Returns the number of indices used by all the
//               primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_num_vertices() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  return reader.get_num_vertices();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_vertex
//       Access: Published
//  Description: Returns the ith vertex index in the table.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_vertex(int i) const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  return reader.get_vertex(i);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_num_primitives
//       Access: Published
//  Description: Returns the number of individual primitives stored
//               within this object.  All primitives are the same
//               type.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_num_primitives() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  return reader.get_num_primitives();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_num_faces
//       Access: Published
//  Description: Returns the number of triangles or other fundamental
//               type (such as line segments) represented by all the
//               primitives in this object.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_primitive_num_faces
//       Access: Published
//  Description: Returns the number of triangles or other fundamental
//               type (such as line segments) represented by the nth
//               primitive in this object.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_primitive_num_faces(int n) const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    return get_primitive_num_vertices(n) - get_min_num_vertices_per_primitive() + 1;
  } else {
    return 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_min_vertex
//       Access: Published
//  Description: Returns the minimum vertex index number used by all
//               the primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_min_vertex() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  reader.check_minmax();
  return reader.get_min_vertex();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_max_vertex
//       Access: Published
//  Description: Returns the maximum vertex index number used by all
//               the primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_max_vertex() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  reader.check_minmax();
  return reader.get_max_vertex();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_data_size_bytes
//       Access: Published
//  Description: Returns the number of bytes stored in the vertices
//               array.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_data_size_bytes() const {
  CDReader cdata(_cycler);
  nassertr(!cdata->_vertices.is_null(), 0);
  return cdata->_vertices.get_read_pointer()->get_data_size_bytes();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time the vertex index array is
//               modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq GeomPrimitive::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::check_valid
//       Access: Published
//  Description: Verifies that the primitive only references vertices
//               that actually exist within the indicated
//               GeomVertexData.  Returns true if the primitive
//               appears to be valid, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GeomPrimitive::
check_valid(const GeomVertexData *vertex_data) const {
  Thread *current_thread = Thread::get_current_thread();
  GeomPrimitivePipelineReader reader(this, current_thread);
  reader.check_minmax();
  GeomVertexDataPipelineReader data_reader(vertex_data, current_thread);
  data_reader.check_array_readers();
  return reader.check_valid(&data_reader);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_vertices
//       Access: Published
//  Description: Returns a const pointer to the vertex index array so
//               application code can read it directly.  This might
//               return NULL if the primitive is nonindexed.  Do not
//               attempt to modify the returned array; use
//               modify_vertices() or set_vertices() for this.
//
//               This method is intended for low-level usage only.
//               There are higher-level methods for more common usage.
//               We recommend you do not use this method directly.  If
//               you do, be sure you know what you are doing!
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexArrayData) GeomPrimitive::
get_vertices() const {
  CDReader cdata(_cycler);
  return cdata->_vertices.get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_index_stride
//       Access: Published
//  Description: A convenience function to return the gap between
//               successive index numbers, in bytes, of the index
//               data.
//
//               This method is intended for low-level usage only.
//               There are higher-level methods for more common usage.
//               We recommend you do not use this method directly.  If
//               you do, be sure you know what you are doing!
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_index_stride() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  return reader.get_index_stride();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_strip_cut_index
//       Access: Published
//  Description: If relevant, returns the index value that may be
//               used in some cases to signify the end of a
//               primitive.  This is typically the highest value
//               that the numeric type can store.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_strip_cut_index() const {
  CDReader cdata(_cycler);
  return get_strip_cut_index(cdata->_index_type);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_ends
//       Access: Published
//  Description: Returns a const pointer to the primitive ends
//               array so application code can read it directly.  Do
//               not attempt to modify the returned array; use
//               modify_ends() or set_ends() for this.
//
//               Note that simple primitive types, like triangles, do
//               not have a ends array: since all the primitives
//               have the same number of vertices, it is not needed.
//
//               This method is intended for low-level usage only.
//               There are higher-level methods for more common usage.
//               We recommend you do not use this method directly.  If
//               you do, be sure you know what you are doing!
////////////////////////////////////////////////////////////////////
INLINE CPTA_int GeomPrimitive::
get_ends() const {
  CDReader cdata(_cycler);
  return cdata->_ends;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_mins
//       Access: Published
//  Description: Returns a const pointer to the primitive mins
//               array so application code can read it directly.  Do
//               not attempt to modify the returned array; use
//               set_minmax() for this.
//
//               Note that simple primitive types, like triangles, do
//               not have a mins array.
//
//               This method is intended for low-level usage only.
//               There are higher-level methods for more common usage.
//               We recommend you do not use this method directly.  If
//               you do, be sure you know what you are doing!
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexArrayData) GeomPrimitive::
get_mins() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  reader.check_minmax();
  return reader.get_mins();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_maxs
//       Access: Published
//  Description: Returns a const pointer to the primitive maxs
//               array so application code can read it directly.  Do
//               not attempt to modify the returned array; use
//               set_minmax().
//
//               Note that simple primitive types, like triangles, do
//               not have a maxs array.
//
//               This method is intended for low-level usage only.
//               There are higher-level methods for more common usage.
//               We recommend you do not use this method directly.  If
//               you do, be sure you know what you are doing!
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexArrayData) GeomPrimitive::
get_maxs() const {
  GeomPrimitivePipelineReader reader(this, Thread::get_current_thread());
  reader.check_minmax();
  return reader.get_maxs();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::add_vertices
//       Access: Public
//  Description: Adds several vertices in a row.
////////////////////////////////////////////////////////////////////
INLINE void GeomPrimitive::
add_vertices(int v1, int v2) {
  add_vertex(v1);
  add_vertex(v2);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::add_vertices
//       Access: Public
//  Description: Adds several vertices in a row.
////////////////////////////////////////////////////////////////////
INLINE void GeomPrimitive::
add_vertices(int v1, int v2, int v3) {
  add_vertex(v1);
  add_vertex(v2);
  add_vertex(v3);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::add_vertices
//       Access: Public
//  Description: Adds several vertices in a row.
////////////////////////////////////////////////////////////////////
INLINE void GeomPrimitive::
add_vertices(int v1, int v2, int v3, int v4) {
  add_vertex(v1);
  add_vertex(v2);
  add_vertex(v3);
  add_vertex(v4);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_index_format
//       Access: Public
//  Description: Returns a registered format appropriate for using to
//               store the index table.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayFormat *GeomPrimitive::
get_index_format() const {
  return get_index_format(get_index_type());
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::make_index_data
//       Access: Public
//  Description: Creates and returns a new, empty index table.
////////////////////////////////////////////////////////////////////
INLINE PT(GeomVertexArrayData) GeomPrimitive::
make_index_data() const {
  return new GeomVertexArrayData(get_index_format(), get_usage_hint());
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::make_index_format
//       Access: Private, Static
//  Description: Returns a registered format appropriate for using to
//               store the index table.
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexArrayFormat) GeomPrimitive::
make_index_format(NumericType index_type) {
  PT(GeomVertexArrayFormat) format = new GeomVertexArrayFormat;
  // It's important that the index format *not* respect the global
  // setting of vertex-column-alignment.  It needs to be tightly
  // packed, so we specify an explict column_alignment of 1.
  format->add_column(InternalName::get_index(), 1, index_type, C_index, 0, 1);
  return GeomVertexArrayFormat::register_format(format);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomPrimitivePipelineReader::
GeomPrimitivePipelineReader(const GeomPrimitive *object,
                            Thread *current_thread) :
  _object(object),
  _current_thread(current_thread),
  _cdata(object->_cycler.read_unlocked(current_thread)),
  _vertices_reader(NULL)
{
  nassertv(_object->test_ref_count_nonzero());
#ifdef DO_PIPELINING
  _cdata->ref();
#endif  // DO_PIPELINING
  if (!_cdata->_vertices.is_null()) {
    _vertices_reader = _cdata->_vertices.get_read_pointer()->get_handle();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::Copy Constructor
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE GeomPrimitivePipelineReader::
GeomPrimitivePipelineReader(const GeomPrimitivePipelineReader &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::Copy Assignment Operator
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE void GeomPrimitivePipelineReader::
operator = (const GeomPrimitivePipelineReader &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomPrimitivePipelineReader::
~GeomPrimitivePipelineReader() {
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG
  //  _object->_cycler.release_read(_cdata);

#ifdef DO_PIPELINING
  unref_delete((CycleData *)_cdata);
#endif  // DO_PIPELINING

#ifdef _DEBUG
  _vertices_reader = NULL;
  _object = NULL;
  _cdata = NULL;
#endif  // _DEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_object
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const GeomPrimitive *GeomPrimitivePipelineReader::
get_object() const {
  return _object;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_current_thread
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Thread *GeomPrimitivePipelineReader::
get_current_thread() const {
  return _current_thread;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_shade_model
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomPrimitivePipelineReader::ShadeModel GeomPrimitivePipelineReader::
get_shade_model() const {
  return _cdata->_shade_model;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_usage_hint
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomPrimitivePipelineReader::UsageHint GeomPrimitivePipelineReader::
get_usage_hint() const {
  return _cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_index_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomPrimitivePipelineReader::NumericType GeomPrimitivePipelineReader::
get_index_type() const {
  return _cdata->_index_type;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::is_indexed
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomPrimitivePipelineReader::
is_indexed() const {
  return (!_cdata->_vertices.is_null());
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_num_vertices
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitivePipelineReader::
get_num_vertices() const {
  if (_cdata->_num_vertices != -1) {
    return _cdata->_num_vertices;
  } else {
    nassertr(!_cdata->_vertices.is_null(), 0);
    return _vertices_reader->get_num_rows();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_min_vertex
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitivePipelineReader::
get_min_vertex() const {
  nassertr(_cdata->_got_minmax, 0);
  return _cdata->_min_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_max_vertex
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitivePipelineReader::
get_max_vertex() const {
  nassertr(_cdata->_got_minmax, 0);
  return _cdata->_max_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_data_size_bytes
//       Access: Published
//  Description: Returns the number of bytes stored in the vertices
//               array.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitivePipelineReader::
get_data_size_bytes() const {
  return _vertices_reader->get_data_size_bytes();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_modified
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq GeomPrimitivePipelineReader::
get_modified() const {
  return _cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_index_stride
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitivePipelineReader::
get_index_stride() const {
  nassertr(is_indexed(), 0);
  return _cdata->_vertices.get_read_pointer()->get_array_format()->get_stride();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_vertices_reader
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayDataHandle *GeomPrimitivePipelineReader::
get_vertices_reader() const {
  return _vertices_reader;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_read_pointer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const unsigned char *GeomPrimitivePipelineReader::
get_read_pointer(bool force) const {
  return _vertices_reader->get_read_pointer(force);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_strip_cut_index
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitivePipelineReader::
get_strip_cut_index() const {
  return GeomPrimitive::get_strip_cut_index(_cdata->_index_type);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_ends
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CPTA_int GeomPrimitivePipelineReader::
get_ends() const {
  return _cdata->_ends;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_mins
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexArrayData) GeomPrimitivePipelineReader::
get_mins() const {
  nassertr(is_indexed(), NULL);
  nassertr(_cdata->_got_minmax, NULL);
  return _cdata->_mins.get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::get_maxs
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexArrayData) GeomPrimitivePipelineReader::
get_maxs() const {
  nassertr(is_indexed(), NULL);
  nassertr(_cdata->_got_minmax, NULL);
  return _cdata->_maxs.get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitivePipelineReader::prepare_now
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IndexBufferContext *GeomPrimitivePipelineReader::
prepare_now(PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) const {
  return ((GeomPrimitive *)_object.p())->prepare_now(prepared_objects, gsg);
}

INLINE ostream &
operator << (ostream &out, const GeomPrimitive &obj) {
  obj.output(out);
  return out;
}
