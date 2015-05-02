// Filename: geomVertexData.I
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
//     Function: GeomVertexData::get_name
//       Access: Published
//  Description: Returns the name passed to the constructor, if any.
//               This name is reported on the PStats graph for vertex
//               computations.
////////////////////////////////////////////////////////////////////
INLINE const string &GeomVertexData::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_usage_hint
//       Access: Published
//  Description: Returns the usage hint that was passed to the
//               constructor, and which will be passed to each array
//               data object created initially, and arrays created as
//               the result of a convert_to() operation.  See
//               geomEnums.h.
//
//               However, each individual array may be replaced with a
//               different array object with an independent usage hint
//               specified, so there is no guarantee that the
//               individual arrays all have the same usage_hint.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData::UsageHint GeomVertexData::
get_usage_hint() const {
  CDReader cdata(_cycler);
  return cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_format
//       Access: Published
//  Description: Returns a pointer to the GeomVertexFormat structure
//               that defines this data.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexData::
get_format() const {
  CDReader cdata(_cycler);
  return cdata->_format;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::has_column
//       Access: Published
//  Description: Returns true if the data has the named column,
//               false otherwise.  This is really just a shortcut for
//               asking the same thing from the format.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexData::
has_column(const InternalName *name) const {
  CDReader cdata(_cycler);
  return cdata->_format->has_column(name);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_num_rows
//       Access: Published
//  Description: Returns the number of rows stored within all the
//               arrays.  All arrays store data for the same n
//               rows.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexData::
get_num_rows() const {
  GeomVertexDataPipelineReader reader(this, Thread::get_current_thread());
  reader.check_array_readers();
  return reader.get_num_rows();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::set_num_rows
//       Access: Published
//  Description: Sets the length of the array to n rows in all of
//               the various arrays (presumably by adding rows).
//
//               The new vertex data is initialized to 0, except for
//               the "color" column, which is initialized to (1, 1, 1,
//               1).
//
//               The return value is true if the number of rows
//               was changed, false if the object already contained n
//               rows (or if there was some error).
//
//               This can be used when you know exactly how many
//               rows you will be needing.  It is faster than
//               reserve_num_rows().  Also see unclean_set_num_rows()
//               if you are planning to fill in all the data yourself.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexData::
set_num_rows(int n) {
  GeomVertexDataPipelineWriter writer(this, true, Thread::get_current_thread());
  writer.check_array_writers();
  return writer.set_num_rows(n);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::unclean_set_num_rows
//       Access: Published
//  Description: This method behaves like set_num_rows(), except the
//               new data is not initialized.  Furthermore, after this
//               call, *any* of the data in the GeomVertexData may be
//               uninitialized, including the earlier rows.
//
//               This is intended for applications that are about to
//               completely fill the GeomVertexData with new data
//               anyway; it provides a tiny performance boost over
//               set_num_rows().
//
//               This can be used when you know exactly how many
//               rows you will be needing.  It is faster than
//               reserve_num_rows().
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexData::
unclean_set_num_rows(int n) {
  GeomVertexDataPipelineWriter writer(this, true, Thread::get_current_thread());
  writer.check_array_writers();
  return writer.unclean_set_num_rows(n);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::reserve_num_rows
//       Access: Published
//  Description: This ensures that enough memory space for n rows is
//               allocated, so that you may increase the number of
//               rows to n without causing a new memory allocation.
//               This is a performance optimization only; it is
//               especially useful when you know ahead of time that
//               you will be adding n rows to the data.
//
//               If you know exactly how many rows you will be
//               needing, it is significantly faster to use
//               set_num_rows() or unclean_set_num_rows() instead.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexData::
reserve_num_rows(int n) {
  GeomVertexDataPipelineWriter writer(this, true, Thread::get_current_thread());
  writer.check_array_writers();
  return writer.reserve_num_rows(n);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_num_arrays
//       Access: Published
//  Description: Returns the number of individual arrays stored within
//               the data.  This must match
//               get_format()->get_num_arrays().
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexData::
get_num_arrays() const {
  CDReader cdata(_cycler);
  return cdata->_arrays.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_array
//       Access: Published
//  Description: Returns a const pointer to the vertex data for the
//               indicated array, for application code to directly
//               examine (but not modify) the underlying vertex data.
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexArrayData) GeomVertexData::
get_array(int i) const {
  CDReader cdata(_cycler);
  nassertr(i >= 0 && i < (int)cdata->_arrays.size(), NULL);
  return cdata->_arrays[i].get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::modify_array
//       Access: Published
//  Description: Returns a modifiable pointer to the indicated vertex
//               array, so that application code may directly
//               manipulate the data.  You should avoid changing
//               the length of this array, since all of the arrays
//               should be kept in sync--use set_num_rows()
//               instead.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
INLINE PT(GeomVertexArrayData) GeomVertexData::
modify_array(int i) {
  GeomVertexDataPipelineWriter writer(this, true, Thread::get_current_thread());
  return writer.modify_array(i);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::set_array
//       Access: Published
//  Description: Replaces the indicated vertex data array with
//               a completely new array.  You should be careful that
//               the new array has the same length and format as the
//               old one, unless you know what you are doing.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexData::
set_array(int i, const GeomVertexArrayData *array) {
  GeomVertexDataPipelineWriter writer(this, true, Thread::get_current_thread());
  writer.set_array(i, array);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_transform_table
//       Access: Published
//  Description: Returns a const pointer to the TransformTable
//               assigned to this data.  Vertices within the table
//               will index into this table to indicate their
//               dynamic skinning information; this table is used when
//               the vertex animation is to be performed by the
//               graphics hardware (but also see
//               get_transform_blend_table()).
//
//               This will return NULL if the vertex data does not
//               have a TransformTable assigned (which implies the
//               vertices will not be animated by the graphics
//               hardware).
////////////////////////////////////////////////////////////////////
INLINE const TransformTable *GeomVertexData::
get_transform_table() const {
  CDReader cdata(_cycler);
  return cdata->_transform_table;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::clear_transform_table
//       Access: Published
//  Description: Sets the TransformTable pointer to NULL,
//               removing the table from the vertex data.  This
//               disables hardware-driven vertex animation.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexData::
clear_transform_table() {
  set_transform_table(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_transform_blend_table
//       Access: Published
//  Description: Returns a const pointer to the TransformBlendTable
//               assigned to this data.  Vertices within the table
//               will index into this table to indicate their
//               dynamic skinning information; this table is used when
//               the vertex animation is to be performed by the CPU
//               (but also see get_transform_table()).
//
//               This will return NULL if the vertex data does not
//               have a TransformBlendTable assigned (which implies
//               the vertices will not be animated by the CPU).
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformBlendTable) GeomVertexData::
get_transform_blend_table() const {
  CDReader cdata(_cycler);
  return cdata->_transform_blend_table.get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::clear_transform_blend_table
//       Access: Published
//  Description: Sets the TransformBlendTable pointer to NULL,
//               removing the table from the vertex data.  This
//               disables CPU-driven vertex animation.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexData::
clear_transform_blend_table() {
  set_transform_blend_table(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_slider_table
//       Access: Published
//  Description: Returns a const pointer to the SliderTable
//               assigned to this data.  Vertices within the vertex
//               data will look up their morph offsets, if any, within
//               this table.
//
//               This will return NULL if the vertex data does not
//               have a SliderTable assigned.
////////////////////////////////////////////////////////////////////
INLINE const SliderTable *GeomVertexData::
get_slider_table() const {
  CDReader cdata(_cycler);
  return cdata->_slider_table;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::clear_slider_table
//       Access: Published
//  Description: Sets the SliderTable pointer to NULL,
//               removing the table from the vertex data.  This
//               disables morph (blend shape) animation.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexData::
clear_slider_table() {
  set_slider_table(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_num_bytes
//       Access: Published
//  Description: Returns the total number of bytes consumed by the
//               different arrays of the vertex data.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexData::
get_num_bytes() const {
  GeomVertexDataPipelineReader reader(this, Thread::get_current_thread());
  return reader.get_num_bytes();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time the vertex data is
//               modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq GeomVertexData::
get_modified(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::pack_abcd
//       Access: Public, Static
//  Description: Packs four values in a DirectX-style NT_packed_abcd
//               value.
////////////////////////////////////////////////////////////////////
INLINE PN_uint32 GeomVertexData::
pack_abcd(unsigned int a, unsigned int b,
          unsigned int c, unsigned int d) {
  return (((a & 0xff) << 24) |
          ((b & 0xff) << 16) |
          ((c & 0xff) << 8) |
          (d & 0xff));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::unpack_abcd_a
//       Access: Public, Static
//  Description: Returns the first packed value from a DirectX-style
//               NT_packed_abcd.
////////////////////////////////////////////////////////////////////
INLINE unsigned int GeomVertexData::
unpack_abcd_a(PN_uint32 data) {
  return (data >> 24) & 0xff;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::unpack_abcd_b
//       Access: Public, Static
//  Description: Returns the second packed value from a DirectX-style
//               NT_packed_abcd.
////////////////////////////////////////////////////////////////////
INLINE unsigned int GeomVertexData::
unpack_abcd_b(PN_uint32 data) {
  return (data >> 16) & 0xff;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::unpack_abcd_c
//       Access: Public, Static
//  Description: Returns the third packed value from a DirectX-style
//               NT_packed_abcd.
////////////////////////////////////////////////////////////////////
INLINE unsigned int GeomVertexData::
unpack_abcd_c(PN_uint32 data) {
  return (data >> 8) & 0xff;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::unpack_abcd_d
//       Access: Public, Static
//  Description: Returns the fourth packed value from a DirectX-style
//               NT_packed_abcd.
////////////////////////////////////////////////////////////////////
INLINE unsigned int GeomVertexData::
unpack_abcd_d(PN_uint32 data) {
  return data & 0xff;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::pack_ufloat
//       Access: Public, Static
//  Description: Packs three float values in an unsigned 32-bit int.
////////////////////////////////////////////////////////////////////
INLINE PN_uint32 GeomVertexData::
pack_ufloat(float a, float b, float c) {
  // Since we have to clamp both low exponents and negative numbers to 0,
  // it's easier to see a float as having a 9-bit signed exponent.
  union {
    PN_int32 _packed;
    float _float;
  } f0, f1, f2;

  f0._float = a;
  f1._float = b;
  f2._float = c;

  // There are several cases here:
  // 1. exponent 0xff: NaN or infinity (negative infinity excluded)
  // 2. exponent too large: clamped to maximum value
  // 3. normalized float
  // 4. exponent 0: denormal float
  // 5. zero or anything negative, clamped to 0

  PN_uint32 packed = 0;

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

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::unpack_ufloat_a
//       Access: Public, Static
//  Description: Unpacks an unsigned float11 value from an uint32.
////////////////////////////////////////////////////////////////////
INLINE float GeomVertexData::
unpack_ufloat_a(PN_uint32 data) {
  if ((data & 0x7c0) == 0) {
    // Denormal float (includes zero).
    return ldexpf((data & 63) / 64.0f, -14);
  }

  union {
    PN_uint32 _packed;
    float _float;
  } value;
  value._packed = ((data & 0x7ff) << 17);

  if ((data & 0x7c0) == 0x7c0) {
    // Infinity / NaN
    value._packed |= 0x7f800000;
  } else {
    value._packed += 0x38000000;
  }

  return value._float;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::unpack_ufloat_b
//       Access: Public, Static
//  Description: Unpacks an unsigned float11 value from an uint32.
////////////////////////////////////////////////////////////////////
INLINE float GeomVertexData::
unpack_ufloat_b(PN_uint32 data) {
  if ((data & 0x3e0000) == 0) {
    // Denormal float (includes zero).
    return ldexpf(((data >> 11) & 63) / 64.0f, -14);
  }

  union {
    PN_uint32 _packed;
    float _float;
  } value;
  value._packed = ((data & 0x3ff800) << 6);

  if ((data & 0x3e0000) == 0x3e0000) {
    // Infinity / NaN
    value._packed |= 0x7f800000;
  } else {
    value._packed += 0x38000000;
  }

  return value._float;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::unpack_ufloat_c
//       Access: Public, Static
//  Description: Unpacks an unsigned float10 value from an uint32.
////////////////////////////////////////////////////////////////////
INLINE float GeomVertexData::
unpack_ufloat_c(PN_uint32 data) {
  if ((data & 0xf8000000u) == 0) {
    // Denormal float (includes zero).
    return ldexpf(((data >> 22) & 31) / 32.0f, -14);
  }

  union {
    PN_uint32 _packed;
    float _float;
  } value;
  value._packed = ((data & 0xffc00000u) >> 4);

  if ((data & 0xf8000000u) == 0xf8000000u) {
    // Infinity / NaN
    value._packed |= 0x7f800000;
  } else {
    value._packed += 0x38000000;
  }

  return value._float;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::add_transform
//       Access: Private, Static
//  Description: Adds the indicated transform to the table, if it is
//               not already there, and returns its index number.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexData::
add_transform(TransformTable *table, const VertexTransform *transform,
              TransformMap &already_added) {
  pair<TransformMap::iterator, bool> result = already_added.insert(TransformMap::value_type(transform, table->get_num_transforms()));

  if (result.second) {
    table->add_transform(transform);
  }

  return (*(result.first)).second;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CDataCache::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData::CDataCache::
CDataCache() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CDataCache::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData::CDataCache::
CDataCache(const GeomVertexData::CDataCache &copy) :
  _result(copy._result)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CacheKey::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData::CacheKey::
CacheKey(const GeomVertexFormat *modifier) :
  _modifier(modifier)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CacheKey::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData::CacheKey::
CacheKey(const CacheKey &copy) :
  _modifier(copy._modifier)
{
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CacheKey::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData::CacheKey::
CacheKey(CacheKey &&from) NOEXCEPT :
  _modifier(move(from._modifier))
{
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CacheKey::operator <
//       Access: Public
//  Description: Provides a unique ordering within the set.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexData::CacheKey::
operator < (const CacheKey &other) const {
  return _modifier < other._modifier;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CacheEntry::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData::CacheEntry::
CacheEntry(GeomVertexData *source, const GeomVertexFormat *modifier) :
  _source(source),
  _key(modifier)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CacheEntry::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData::CacheEntry::
CacheEntry(GeomVertexData *source, const CacheKey &key) :
  _source(source),
  _key(key)
{
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CacheEntry::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData::CacheEntry::
CacheEntry(GeomVertexData *source, CacheKey &&key) NOEXCEPT :
  _source(source),
  _key(move(key))
{
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData::CData::
CData() :
  _usage_hint(UH_unspecified)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData::CData::
CData(const GeomVertexData::CData &copy) :
  _usage_hint(copy._usage_hint),
  _format(copy._format),
  _arrays(copy._arrays),
  _transform_table(copy._transform_table),
  _transform_blend_table(copy._transform_blend_table),
  _slider_table(copy._slider_table),
  _animated_vertices(copy._animated_vertices),
  _animated_vertices_modified(copy._animated_vertices_modified),
  _modified(copy._modified)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineBase::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineBase::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexDataPipelineBase::
~GeomVertexDataPipelineBase() {
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG

#ifdef DO_PIPELINING
  unref_delete((CycleData *)_cdata);
#endif  // DO_PIPELINING

#ifdef _DEBUG
  _object = NULL;
  _cdata = NULL;
#endif  // _DEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineBase::get_current_thread
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Thread *GeomVertexDataPipelineBase::
get_current_thread() const {
  return _current_thread;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineBase::get_usage_hint
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexDataPipelineBase::UsageHint GeomVertexDataPipelineBase::
get_usage_hint() const {
  return _cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineBase::get_format
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexDataPipelineBase::
get_format() const {
  return _cdata->_format;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineBase::has_column
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexDataPipelineBase::
has_column(const InternalName *name) const {
  return _cdata->_format->has_column(name);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineBase::get_num_arrays
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexDataPipelineBase::
get_num_arrays() const {
  return _cdata->_arrays.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineBase::get_array
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexArrayData) GeomVertexDataPipelineBase::
get_array(int i) const {
  nassertr(i >= 0 && i < (int)_cdata->_arrays.size(), NULL);
  return _cdata->_arrays[i].get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineBase::get_transform_table
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const TransformTable *GeomVertexDataPipelineBase::
get_transform_table() const {
  return _cdata->_transform_table;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineBase::get_transform_blend_table
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformBlendTable) GeomVertexDataPipelineBase::
get_transform_blend_table() const {
  return _cdata->_transform_blend_table.get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineBase::get_slider_table
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const SliderTable *GeomVertexDataPipelineBase::
get_slider_table() const {
  return _cdata->_slider_table;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineBase::get_modified
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq GeomVertexDataPipelineBase::
get_modified() const {
  return _cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexDataPipelineReader::
GeomVertexDataPipelineReader(const GeomVertexData *object,
                             Thread *current_thread) :
  GeomVertexDataPipelineBase((GeomVertexData *)object, current_thread,
                             (GeomVertexData::CData *)object->_cycler.read_unlocked(current_thread)),
  _got_array_readers(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineReader::Copy Constructor
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexDataPipelineReader::
GeomVertexDataPipelineReader(const GeomVertexDataPipelineReader &copy) :
  GeomVertexDataPipelineBase(copy)
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineReader::Copy Assignment Operator
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexDataPipelineReader::
operator = (const GeomVertexDataPipelineReader &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineReader::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexDataPipelineReader::
~GeomVertexDataPipelineReader() {
  if (_got_array_readers) {
    delete_array_readers();
  }
  //  _object->_cycler.release_read(_cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineReader::get_object
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexData *GeomVertexDataPipelineReader::
get_object() const {
  return _object;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineReader::check_array_readers
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexDataPipelineReader::
check_array_readers() const {
  if (!_got_array_readers) {
    ((GeomVertexDataPipelineReader *)this)->make_array_readers();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineReader::get_array_reader
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayDataHandle *GeomVertexDataPipelineReader::
get_array_reader(int i) const {
  nassertr(_got_array_readers, NULL);
  nassertr(i >= 0 && i < (int)_array_readers.size(), NULL);
  return _array_readers[i];
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineReader::has_vertex
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexDataPipelineReader::
has_vertex() const {
  return (_cdata->_format->get_vertex_column() != (GeomVertexColumn *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineReader::is_vertex_transformed
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexDataPipelineReader::
is_vertex_transformed() const {
  const GeomVertexColumn *column = _cdata->_format->get_vertex_column();
  if (column != (GeomVertexColumn *)NULL) {
    return column->get_contents() == C_clip_point;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineReader::has_normal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexDataPipelineReader::
has_normal() const {
  return (_cdata->_format->get_normal_column() != (GeomVertexColumn *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineReader::has_color
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexDataPipelineReader::
has_color() const {
  return (_cdata->_format->get_color_column() != (GeomVertexColumn *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineWriter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexDataPipelineWriter::
GeomVertexDataPipelineWriter(GeomVertexData *object, bool force_to_0,
                             Thread *current_thread) :
  GeomVertexDataPipelineBase(object, current_thread,
                             object->_cycler.write_upstream(force_to_0, current_thread)),
  _force_to_0(force_to_0),
  _got_array_writers(false)
{
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#ifdef DO_PIPELINING
  nassertv(_cdata->test_ref_count_nonzero());
#endif  // DO_PIPELINING
#endif // _DEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineWriter::Copy Constructor
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexDataPipelineWriter::
GeomVertexDataPipelineWriter(const GeomVertexDataPipelineWriter &copy) :
  GeomVertexDataPipelineBase(copy)
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineWriter::Copy Assignment Operator
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexDataPipelineWriter::
operator = (const GeomVertexDataPipelineWriter &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineWriter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexDataPipelineWriter::
~GeomVertexDataPipelineWriter() {
  if (_got_array_writers) {
    delete_array_writers();
  }
  _object->_cycler.release_write(_cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineWriter::get_object
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData *GeomVertexDataPipelineWriter::
get_object() const {
  return _object;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineWriter::check_array_writers
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexDataPipelineWriter::
check_array_writers() const {
  if (!_got_array_writers) {
    ((GeomVertexDataPipelineWriter *)this)->make_array_writers();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexDataPipelineWriter::get_array_writer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataHandle *GeomVertexDataPipelineWriter::
get_array_writer(int i) const {
  nassertr(_got_array_writers, NULL);
  nassertr(i >= 0 && i < (int)_array_writers.size(), NULL);
  return _array_writers[i];
}

INLINE ostream &
operator << (ostream &out, const GeomVertexData &obj) {
  obj.output(out);
  return out;
}
