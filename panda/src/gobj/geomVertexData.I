/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexData.I
 * @author drose
 * @date 2005-03-06
 */

/**
 * Returns the name passed to the constructor, if any.  This name is reported
 * on the PStats graph for vertex computations.
 */
INLINE const std::string &GeomVertexData::
get_name() const {
  return _name;
}

/**
 * Returns the usage hint that was passed to the constructor, and which will
 * be passed to each array data object created initially, and arrays created
 * as the result of a convert_to() operation.  See geomEnums.h.
 *
 * However, each individual array may be replaced with a different array
 * object with an independent usage hint specified, so there is no guarantee
 * that the individual arrays all have the same usage_hint.
 */
INLINE GeomVertexData::UsageHint GeomVertexData::
get_usage_hint() const {
  CDReader cdata(_cycler);
  return cdata->_usage_hint;
}

/**
 * Returns a pointer to the GeomVertexFormat structure that defines this data.
 */
INLINE const GeomVertexFormat *GeomVertexData::
get_format() const {
  CDReader cdata(_cycler);
  return cdata->_format;
}

/**
 * Returns true if the data has the named column, false otherwise.  This is
 * really just a shortcut for asking the same thing from the format.
 */
INLINE bool GeomVertexData::
has_column(const InternalName *name) const {
  CDReader cdata(_cycler);
  return cdata->_format->has_column(name);
}

/**
 * Returns the number of rows stored within all the arrays.  All arrays store
 * data for the same n rows.
 */
INLINE int GeomVertexData::
get_num_rows() const {
  CPT(GeomVertexArrayData) array;
  {
    CDReader cdata(_cycler);
    nassertr(cdata->_format->get_num_arrays() == cdata->_arrays.size(), 0);

    if (cdata->_arrays.size() == 0) {
      // No arrays means no rows.  Weird but legal.
      return 0;
    }

    array = cdata->_arrays[0].get_read_pointer();
  }

  return array->get_num_rows();
}

/**
 * Sets the length of the array to n rows in all of the various arrays
 * (presumably by adding rows).
 *
 * The new vertex data is initialized to 0, except for the "color" column,
 * which is initialized to (1, 1, 1, 1).
 *
 * The return value is true if the number of rows was changed, false if the
 * object already contained n rows (or if there was some error).
 *
 * This can be used when you know exactly how many rows you will be needing.
 * It is faster than reserve_num_rows().  Also see unclean_set_num_rows() if
 * you are planning to fill in all the data yourself.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
INLINE bool GeomVertexData::
set_num_rows(int n) {
  GeomVertexDataPipelineWriter writer(this, true, Thread::get_current_thread());
  writer.check_array_writers();
  return writer.set_num_rows(n);
}

/**
 * This method behaves like set_num_rows(), except the new data is not
 * initialized.  Furthermore, after this call, *any* of the data in the
 * GeomVertexData may be uninitialized, including the earlier rows.
 *
 * This is intended for applications that are about to completely fill the
 * GeomVertexData with new data anyway; it provides a tiny performance boost
 * over set_num_rows().
 *
 * This can be used when you know exactly how many rows you will be needing.
 * It is faster than reserve_num_rows().
 */
INLINE bool GeomVertexData::
unclean_set_num_rows(int n) {
  GeomVertexDataPipelineWriter writer(this, true, Thread::get_current_thread());
  writer.check_array_writers();
  return writer.unclean_set_num_rows(n);
}

/**
 * This ensures that enough memory space for n rows is allocated, so that you
 * may increase the number of rows to n without causing a new memory
 * allocation.  This is a performance optimization only; it is especially
 * useful when you know ahead of time that you will be adding n rows to the
 * data.
 *
 * If you know exactly how many rows you will be needing, it is significantly
 * faster to use set_num_rows() or unclean_set_num_rows() instead.
 */
INLINE bool GeomVertexData::
reserve_num_rows(int n) {
  GeomVertexDataPipelineWriter writer(this, true, Thread::get_current_thread());
  writer.check_array_writers();
  return writer.reserve_num_rows(n);
}

/**
 * Returns the number of individual arrays stored within the data.  This must
 * match get_format()->get_num_arrays().
 */
INLINE size_t GeomVertexData::
get_num_arrays() const {
  CDReader cdata(_cycler);
  return cdata->_arrays.size();
}

/**
 * Returns a const pointer to the vertex data for the indicated array, for
 * application code to directly examine (but not modify) the underlying vertex
 * data.
 */
INLINE CPT(GeomVertexArrayData) GeomVertexData::
get_array(size_t i) const {
  CDReader cdata(_cycler);
  nassertr(i < cdata->_arrays.size(), nullptr);
  return cdata->_arrays[i].get_read_pointer();
}

/**
 * Equivalent to get_array(i).get_handle().
 */
INLINE CPT(GeomVertexArrayDataHandle) GeomVertexData::
get_array_handle(size_t i) const {
  Thread *current_thread = Thread::get_current_thread();
  CDReader cdata(_cycler, current_thread);
  nassertr(i < cdata->_arrays.size(), nullptr);
  return new GeomVertexArrayDataHandle(cdata->_arrays[i].get_read_pointer(), current_thread);
}

/**
 * Returns a modifiable pointer to the indicated vertex array, so that
 * application code may directly manipulate the data.  You should avoid
 * changing the length of this array, since all of the arrays should be kept
 * in sync--use set_num_rows() instead.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
INLINE PT(GeomVertexArrayData) GeomVertexData::
modify_array(size_t i) {
  GeomVertexDataPipelineWriter writer(this, true, Thread::get_current_thread());
  return writer.modify_array(i);
}

/**
 * Equivalent to modify_array(i).modify_handle().
 */
INLINE PT(GeomVertexArrayDataHandle) GeomVertexData::
modify_array_handle(size_t i) {
  Thread *current_thread = Thread::get_current_thread();
  GeomVertexDataPipelineWriter writer(this, true, current_thread);
  return new GeomVertexArrayDataHandle(writer.modify_array(i), current_thread);
}

/**
 * Replaces the indicated vertex data array with a completely new array.  You
 * should be careful that the new array has the same length and format as the
 * old one, unless you know what you are doing.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
INLINE void GeomVertexData::
set_array(size_t i, const GeomVertexArrayData *array) {
  GeomVertexDataPipelineWriter writer(this, true, Thread::get_current_thread());
  writer.set_array(i, array);
}

/**
 * Returns a const pointer to the TransformTable assigned to this data.
 * Vertices within the table will index into this table to indicate their
 * dynamic skinning information; this table is used when the vertex animation
 * is to be performed by the graphics hardware (but also see
 * get_transform_blend_table()).
 *
 * This will return NULL if the vertex data does not have a TransformTable
 * assigned (which implies the vertices will not be animated by the graphics
 * hardware).
 */
INLINE const TransformTable *GeomVertexData::
get_transform_table() const {
  CDReader cdata(_cycler);
  return cdata->_transform_table;
}

/**
 * Sets the TransformTable pointer to NULL, removing the table from the vertex
 * data.  This disables hardware-driven vertex animation.
 */
INLINE void GeomVertexData::
clear_transform_table() {
  set_transform_table(nullptr);
}

/**
 * Returns a const pointer to the TransformBlendTable assigned to this data.
 * Vertices within the table will index into this table to indicate their
 * dynamic skinning information; this table is used when the vertex animation
 * is to be performed by the CPU (but also see get_transform_table()).
 *
 * This will return NULL if the vertex data does not have a
 * TransformBlendTable assigned (which implies the vertices will not be
 * animated by the CPU).
 */
INLINE CPT(TransformBlendTable) GeomVertexData::
get_transform_blend_table() const {
  CDReader cdata(_cycler);
  return cdata->_transform_blend_table.get_read_pointer();
}

/**
 * Sets the TransformBlendTable pointer to NULL, removing the table from the
 * vertex data.  This disables CPU-driven vertex animation.
 */
INLINE void GeomVertexData::
clear_transform_blend_table() {
  set_transform_blend_table(nullptr);
}

/**
 * Returns a const pointer to the SliderTable assigned to this data.  Vertices
 * within the vertex data will look up their morph offsets, if any, within
 * this table.
 *
 * This will return NULL if the vertex data does not have a SliderTable
 * assigned.
 */
INLINE const SliderTable *GeomVertexData::
get_slider_table() const {
  CDReader cdata(_cycler);
  return cdata->_slider_table;
}

/**
 * Sets the SliderTable pointer to NULL, removing the table from the vertex
 * data.  This disables morph (blend shape) animation.
 */
INLINE void GeomVertexData::
clear_slider_table() {
  set_slider_table(nullptr);
}

/**
 * Returns the total number of bytes consumed by the different arrays of the
 * vertex data.
 */
INLINE int GeomVertexData::
get_num_bytes() const {
  GeomVertexDataPipelineReader reader(this, Thread::get_current_thread());
  return reader.get_num_bytes();
}

/**
 * Returns a sequence number which is guaranteed to change at least every time
 * the vertex data is modified.
 */
INLINE UpdateSeq GeomVertexData::
get_modified(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_modified;
}

/**
 * Packs four values in a DirectX-style NT_packed_abcd value.
 */
INLINE uint32_t GeomVertexData::
pack_abcd(unsigned int a, unsigned int b,
          unsigned int c, unsigned int d) {
  return (((a & 0xff) << 24) |
          ((b & 0xff) << 16) |
          ((c & 0xff) << 8) |
          (d & 0xff));
}

/**
 * Returns the first packed value from a DirectX-style NT_packed_abcd.
 */
INLINE unsigned int GeomVertexData::
unpack_abcd_a(uint32_t data) {
  return (data >> 24) & 0xff;
}

/**
 * Returns the second packed value from a DirectX-style NT_packed_abcd.
 */
INLINE unsigned int GeomVertexData::
unpack_abcd_b(uint32_t data) {
  return (data >> 16) & 0xff;
}

/**
 * Returns the third packed value from a DirectX-style NT_packed_abcd.
 */
INLINE unsigned int GeomVertexData::
unpack_abcd_c(uint32_t data) {
  return (data >> 8) & 0xff;
}

/**
 * Returns the fourth packed value from a DirectX-style NT_packed_abcd.
 */
INLINE unsigned int GeomVertexData::
unpack_abcd_d(uint32_t data) {
  return data & 0xff;
}

/**
 * Packs three float values in an unsigned 32-bit int.
 */
INLINE uint32_t GeomVertexData::
pack_ufloat(float a, float b, float c) {
  // Since we have to clamp both low exponents and negative numbers to 0, it's
  // easier to see a float as having a 9-bit signed exponent.
  union {
    int32_t _packed;
    float _float;
  } f0, f1, f2;

  f0._float = a;
  f1._float = b;
  f2._float = c;

  // There are several cases here: 1. exponent 0xff: NaN or infinity (negative
  // infinity excluded) 2. exponent too large: clamped to maximum value 3.
  // normalized float 4. exponent 0: denormal float 5. zero or anything
  // negative, clamped to 0

  uint32_t packed = 0;

  if ((f0._packed & 0x7f800000) == 0x7f800000 && (unsigned)f0._packed != 0xff800000u) {
    packed |= (f0._packed >> 17) & 0x7ffu;
  } else if (f0._packed >= 0x47800000) {
    packed |= 0x7bf;
  } else if (f0._packed >= 0x38800000) {
    packed |= (f0._packed >> 17) - 0x1c00;
  } else if (f0._packed >= 0x35000000) {
    packed |= ((f0._packed & 0x7c0000u) | 0x800000u) >> (130 - (f0._packed >> 23));
  }

  if ((f1._packed & 0x7f800000) == 0x7f800000 && (unsigned)f1._packed != 0xff800000u) {
    packed |= (f1._packed >> 6) & 0x3ff800u;
  } else if (f1._packed >= 0x47800000) {
    packed |= 0x3df800;
  } else if (f1._packed >= 0x38800000) {
    packed |= ((f1._packed >> 6) - 0xe00000) & 0x3ff800;
  } else if (f1._packed >= 0x35000000) {
    packed |= (((f1._packed & 0x7c0000u) | 0x800000u) >> (119 - (f1._packed >> 23))) & 0x1f800u;
  }

  if ((f2._packed & 0x7f800000) == 0x7f800000 && (unsigned)f2._packed != 0xff800000u) {
    packed |= (f2._packed & 0x0ffe0000u) << 4;
  } else if (f2._packed >= 0x47800000) {
    packed |= 0xf7c00000;
  } else if (f2._packed >= 0x38800000) {
    packed |= ((f2._packed - 0x38000000) << 4) & 0xffc00000;
  } else if (f2._packed >= 0x35000000) {
    packed |= ((((f2._packed << 3) & 0x03c00000u) | 0x04000000u) >> (112 - (f2._packed >> 23))) & 0x07c00000u;
  }

  return packed;
}

/**
 * Unpacks an unsigned float11 value from an uint32.
 */
INLINE float GeomVertexData::
unpack_ufloat_a(uint32_t data) {
  if ((data & 0x7c0) == 0) {
    // Denormal float (includes zero).
    return ldexpf((data & 63) / 64.0f, -14);
  }

  union {
    uint32_t _packed;
    float _float;
  } value;
  value._packed = ((data & 0x7ff) << 17);

  if ((data & 0x7c0) == 0x7c0) {
    // Infinity  NaN
    value._packed |= 0x7f800000;
  } else {
    value._packed += 0x38000000;
  }

  return value._float;
}

/**
 * Unpacks an unsigned float11 value from an uint32.
 */
INLINE float GeomVertexData::
unpack_ufloat_b(uint32_t data) {
  if ((data & 0x3e0000) == 0) {
    // Denormal float (includes zero).
    return ldexpf(((data >> 11) & 63) / 64.0f, -14);
  }

  union {
    uint32_t _packed;
    float _float;
  } value;
  value._packed = ((data & 0x3ff800) << 6);

  if ((data & 0x3e0000) == 0x3e0000) {
    // Infinity  NaN
    value._packed |= 0x7f800000;
  } else {
    value._packed += 0x38000000;
  }

  return value._float;
}

/**
 * Unpacks an unsigned float10 value from an uint32.
 */
INLINE float GeomVertexData::
unpack_ufloat_c(uint32_t data) {
  if ((data & 0xf8000000u) == 0) {
    // Denormal float (includes zero).
    return ldexpf(((data >> 22) & 31) / 32.0f, -14);
  }

  union {
    uint32_t _packed;
    float _float;
  } value;
  value._packed = ((data & 0xffc00000u) >> 4);

  if ((data & 0xf8000000u) == 0xf8000000u) {
    // Infinity  NaN
    value._packed |= 0x7f800000;
  } else {
    value._packed += 0x38000000;
  }

  return value._float;
}

/**
 * Adds the indicated transform to the table, if it is not already there, and
 * returns its index number.
 */
INLINE int GeomVertexData::
add_transform(TransformTable *table, const VertexTransform *transform,
              TransformMap &already_added) {
  std::pair<TransformMap::iterator, bool> result = already_added.insert(TransformMap::value_type(transform, table->get_num_transforms()));

  if (result.second) {
    table->add_transform(transform);
  }

  return (*(result.first)).second;
}

/**
 *
 */
INLINE GeomVertexData::CDataCache::
CDataCache() {
}

/**
 *
 */
INLINE GeomVertexData::CDataCache::
CDataCache(const GeomVertexData::CDataCache &copy) :
  _result(copy._result)
{
}

/**
 *
 */
INLINE GeomVertexData::CacheKey::
CacheKey(const GeomVertexFormat *modifier) :
  _modifier(modifier)
{
}

/**
 *
 */
INLINE GeomVertexData::CacheKey::
CacheKey(const CacheKey &copy) :
  _modifier(copy._modifier)
{
}

/**
 *
 */
INLINE GeomVertexData::CacheKey::
CacheKey(CacheKey &&from) noexcept :
  _modifier(std::move(from._modifier))
{
}

/**
 * Provides a unique ordering within the set.
 */
INLINE bool GeomVertexData::CacheKey::
operator < (const CacheKey &other) const {
  return _modifier < other._modifier;
}

/**
 *
 */
INLINE GeomVertexData::CacheEntry::
CacheEntry(GeomVertexData *source, const GeomVertexFormat *modifier) :
  _source(source),
  _key(modifier)
{
}

/**
 *
 */
INLINE GeomVertexData::CacheEntry::
CacheEntry(GeomVertexData *source, const CacheKey &key) :
  _source(source),
  _key(key)
{
}

/**
 *
 */
INLINE GeomVertexData::CacheEntry::
CacheEntry(GeomVertexData *source, CacheKey &&key) noexcept :
  _source(source),
  _key(std::move(key))
{
}

/**
 *
 */
INLINE GeomVertexData::CData::
CData() :
  _usage_hint(UH_unspecified)
{
}

/**
 *
 */
INLINE GeomVertexData::CData::
CData(const GeomVertexFormat *format, GeomVertexData::UsageHint usage_hint) :
  _format(format),
  _usage_hint(usage_hint)
{
  size_t num_arrays = format->get_num_arrays();
  for (size_t i = 0; i < num_arrays; ++i) {
    _arrays.push_back(new GeomVertexArrayData(format->get_array(i), usage_hint));
  }
}

/**
 *
 */
INLINE GeomVertexDataPipelineBase::
GeomVertexDataPipelineBase(Thread *current_thread) :
  _object(nullptr),
  _current_thread(current_thread),
  _cdata(nullptr)
{
}

/**
 *
 */
INLINE GeomVertexDataPipelineBase::
GeomVertexDataPipelineBase(GeomVertexData *object,
                           Thread *current_thread,
                           GeomVertexData::CData *cdata) :
  _object(object),
  _current_thread(current_thread),
  _cdata(cdata)
{
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG
#ifdef DO_PIPELINING
  _cdata->ref();
#endif  // DO_PIPELINING
}

/**
 *
 */
INLINE GeomVertexDataPipelineBase::
~GeomVertexDataPipelineBase() {
#ifdef _DEBUG
  if (_object != nullptr) {
    nassertv(_object->test_ref_count_nonzero());
  }
#endif // _DEBUG

#ifdef DO_PIPELINING
  if (_cdata != nullptr) {
    unref_delete((CycleData *)_cdata);
  }
#endif  // DO_PIPELINING

#ifdef _DEBUG
  _object = nullptr;
  _cdata = nullptr;
#endif  // _DEBUG
}

/**
 *
 */
INLINE Thread *GeomVertexDataPipelineBase::
get_current_thread() const {
  return _current_thread;
}

/**
 *
 */
INLINE GeomVertexDataPipelineBase::UsageHint GeomVertexDataPipelineBase::
get_usage_hint() const {
  return _cdata->_usage_hint;
}

/**
 *
 */
INLINE const GeomVertexFormat *GeomVertexDataPipelineBase::
get_format() const {
  return _cdata->_format;
}

/**
 *
 */
INLINE bool GeomVertexDataPipelineBase::
has_column(const InternalName *name) const {
  return _cdata->_format->has_column(name);
}

/**
 *
 */
INLINE size_t GeomVertexDataPipelineBase::
get_num_arrays() const {
  return _cdata->_arrays.size();
}

/**
 *
 */
INLINE CPT(GeomVertexArrayData) GeomVertexDataPipelineBase::
get_array(size_t i) const {
  nassertr(i < _cdata->_arrays.size(), nullptr);
  return _cdata->_arrays[i].get_read_pointer();
}

/**
 *
 */
INLINE const TransformTable *GeomVertexDataPipelineBase::
get_transform_table() const {
  return _cdata->_transform_table;
}

/**
 *
 */
INLINE CPT(TransformBlendTable) GeomVertexDataPipelineBase::
get_transform_blend_table() const {
  return _cdata->_transform_blend_table.get_read_pointer();
}

/**
 *
 */
INLINE const SliderTable *GeomVertexDataPipelineBase::
get_slider_table() const {
  return _cdata->_slider_table;
}

/**
 *
 */
INLINE UpdateSeq GeomVertexDataPipelineBase::
get_modified() const {
  return _cdata->_modified;
}

/**
 *
 */
INLINE GeomVertexDataPipelineReader::
GeomVertexDataPipelineReader(Thread *current_thread) :
  GeomVertexDataPipelineBase(current_thread),
  _got_array_readers(false)
{
}

/**
 *
 */
INLINE GeomVertexDataPipelineReader::
GeomVertexDataPipelineReader(const GeomVertexData *object,
                             Thread *current_thread) :
  GeomVertexDataPipelineBase((GeomVertexData *)object, current_thread,
                             (GeomVertexData::CData *)object->_cycler.read_unlocked(current_thread)),
  _got_array_readers(false)
{
}

/**
 *
 */
INLINE void GeomVertexDataPipelineReader::
set_object(const GeomVertexData *object) {
#ifdef DO_PIPELINING
  if (_cdata != nullptr) {
    unref_delete((CycleData *)_cdata);
  }
#endif  // DO_PIPELINING
  _array_readers.clear();

  _object = (GeomVertexData *)object;
  _cdata = (GeomVertexData::CData *)_object->_cycler.read_unlocked(_current_thread);
  _got_array_readers = false;

#ifdef DO_PIPELINING
  _cdata->ref();
#endif  // DO_PIPELINING
}

/**
 *
 */
INLINE const GeomVertexData *GeomVertexDataPipelineReader::
get_object() const {
  return _object;
}

/**
 *
 */
INLINE void GeomVertexDataPipelineReader::
check_array_readers() const {
  if (!_got_array_readers) {
    ((GeomVertexDataPipelineReader *)this)->make_array_readers();
  }
}

/**
 *
 */
INLINE const GeomVertexArrayDataHandle *GeomVertexDataPipelineReader::
get_array_reader(int i) const {
  nassertr(_got_array_readers, nullptr);
  nassertr(i >= 0 && i < (int)_array_readers.size(), nullptr);
  return _array_readers[i];
}

/**
 *
 */
INLINE bool GeomVertexDataPipelineReader::
has_vertex() const {
  return (_cdata->_format->get_vertex_column() != nullptr);
}

/**
 *
 */
INLINE bool GeomVertexDataPipelineReader::
is_vertex_transformed() const {
  const GeomVertexColumn *column = _cdata->_format->get_vertex_column();
  if (column != nullptr) {
    return column->get_contents() == C_clip_point;
  }

  return false;
}

/**
 *
 */
INLINE bool GeomVertexDataPipelineReader::
has_normal() const {
  return (_cdata->_format->get_normal_column() != nullptr);
}

/**
 *
 */
INLINE bool GeomVertexDataPipelineReader::
has_color() const {
  return (_cdata->_format->get_color_column() != nullptr);
}

/**
 *
 */
INLINE GeomVertexDataPipelineWriter::
GeomVertexDataPipelineWriter(GeomVertexData *object, bool force_to_0,
                             Thread *current_thread) :
  GeomVertexDataPipelineBase(object, current_thread,
                             object->_cycler.write_upstream(force_to_0, current_thread)),
  _got_array_writers(false)
{
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#ifdef DO_PIPELINING
  nassertv(_cdata->test_ref_count_nonzero());
#endif  // DO_PIPELINING
#endif // _DEBUG
}

/**
 *
 */
INLINE GeomVertexDataPipelineWriter::
~GeomVertexDataPipelineWriter() {
  if (_got_array_writers) {
    delete_array_writers();
  }
  _object->_cycler.release_write(_cdata);
}

/**
 *
 */
INLINE GeomVertexData *GeomVertexDataPipelineWriter::
get_object() const {
  return _object;
}

/**
 *
 */
INLINE void GeomVertexDataPipelineWriter::
check_array_writers() const {
  if (!_got_array_writers) {
    ((GeomVertexDataPipelineWriter *)this)->make_array_writers();
  }
}

/**
 *
 */
INLINE GeomVertexArrayDataHandle *GeomVertexDataPipelineWriter::
get_array_writer(size_t i) const {
  nassertr(_got_array_writers, nullptr);
  nassertr(i < _array_writers.size(), nullptr);
  return _array_writers[i];
}

INLINE std::ostream &
operator << (std::ostream &out, const GeomVertexData &obj) {
  obj.output(out);
  return out;
}
