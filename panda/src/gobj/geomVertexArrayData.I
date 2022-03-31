/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexArrayData.I
 * @author drose
 * @date 2005-03-17
 */

/**
 * Returns the format object that describes this array.
 */
INLINE const GeomVertexArrayFormat *GeomVertexArrayData::
get_array_format() const {
  return _array_format;
}

/**
 * Returns the usage hint that describes to the rendering backend how often
 * the vertex data will be modified and/or rendered.  See geomEnums.h.
 */
INLINE GeomVertexArrayData::UsageHint GeomVertexArrayData::
get_usage_hint() const {
  CDReader cdata(_cycler);
  return cdata->_usage_hint;
}

/**
 * Returns true if the array has the named column, false otherwise.  This is
 * really just a shortcut for asking the same thing from the format.
 */
INLINE bool GeomVertexArrayData::
has_column(const InternalName *name) const {
  return _array_format->has_column(name);
}

/**
 * Returns the number of rows stored in the array, based on the number of
 * bytes and the stride.  This should be the same for all arrays within a
 * given GeomVertexData object.
 */
INLINE int GeomVertexArrayData::
get_num_rows() const {
  CDReader cdata(_cycler);
  nassertr(_array_format->get_stride() != 0, 0);
  return cdata->_buffer.get_size() / _array_format->get_stride();
}

/**
 * Sets the length of the array to n rows.
 *
 * Normally, you would not call this directly, since all of the arrays in a
 * particular GeomVertexData must have the same number of rows; instead, call
 * GeomVertexData::set_num_rows().
 *
 * The return value is true if the number of rows was changed, false if the
 * object already contained n rows (or if there was some error).
 *
 * The new vertex data is initialized to 0, including the "color" column (but
 * see GeomVertexData::set_num_rows()).
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
INLINE bool GeomVertexArrayData::
set_num_rows(int n) {
  return modify_handle()->set_num_rows(n);
}

/**
 * This method behaves like set_num_rows(), except the new data is not
 * initialized.  Furthermore, after this call, *any* of the data in the
 * GeomVertexArrayData may be uninitialized, including the earlier rows.
 *
 * Normally, you would not call this directly, since all of the arrays in a
 * particular GeomVertexData must have the same number of rows; instead, call
 * GeomVertexData::unclean_set_num_rows().
 */
INLINE bool GeomVertexArrayData::
unclean_set_num_rows(int n) {
  return modify_handle()->unclean_set_num_rows(n);
}

/**
 * This ensures that enough memory space for n rows is allocated, so that you
 * may increase the number of rows to n without causing a new memory
 * allocation.  This is a performance optimization only; it is especially
 * useful when you know ahead of time that you will be adding n rows to the
 * data.
 */
INLINE bool GeomVertexArrayData::
reserve_num_rows(int n) {
  return modify_handle()->reserve_num_rows(n);
}

/**
 * Removes all of the rows in the array.  Functionally equivalent to
 * set_num_rows(0).
 */
INLINE void GeomVertexArrayData::
clear_rows() {
  return modify_handle()->clear_rows();
}

/**
 * Returns the number of bytes stored in the array.
 */
INLINE size_t GeomVertexArrayData::
get_data_size_bytes() const {
  CDReader cdata(_cycler);
  return cdata->_buffer.get_size();
}

/**
 * Returns a sequence number which is guaranteed to change at least every time
 * the array vertex data is modified.
 */
INLINE UpdateSeq GeomVertexArrayData::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

/**
 * Returns true if the vertex data is currently resident in memory.  If this
 * returns true, the next call to get_handle()->get_read_pointer() will
 * probably not block.  If this returns false, the vertex data will be brought
 * back into memory shortly; try again later.
 */
INLINE bool GeomVertexArrayData::
request_resident(Thread *current_thread) const {
  const GeomVertexArrayData::CData *cdata = _cycler.read_unlocked(current_thread);

#ifdef DO_PIPELINING
  cdata->ref();
#endif

  cdata->_rw_lock.acquire();

  ((GeomVertexArrayData *)this)->mark_used();
  bool is_resident = (cdata->_buffer.get_read_pointer(false) != nullptr);

  cdata->_rw_lock.release();

#ifdef DO_PIPELINING
  unref_delete((CycleData *)cdata);
#endif

  return is_resident;
}

/**
 * Returns an object that can be used to read the actual data bytes stored in
 * the array.  Calling this method locks the data, and will block any other
 * threads attempting to read or write the data, until the returned object
 * destructs.
 */
INLINE CPT(GeomVertexArrayDataHandle) GeomVertexArrayData::
get_handle(Thread *current_thread) const {
  return new GeomVertexArrayDataHandle(this, current_thread);
}

/**
 * Returns an object that can be used to read or write the actual data bytes
 * stored in the array.  Calling this method locks the data, and will block
 * any other threads attempting to read or write the data, until the returned
 * object destructs.
 */
INLINE PT(GeomVertexArrayDataHandle) GeomVertexArrayData::
modify_handle(Thread *current_thread) {
  return new GeomVertexArrayDataHandle(PT(GeomVertexArrayData)(this), current_thread);
}

/**
 * Returns a pointer to the global LRU object that manages the
 * GeomVertexArrayData's that have not (yet) been paged out.
 */
INLINE SimpleLru *GeomVertexArrayData::
get_independent_lru() {
  return &_independent_lru;
}

/**
 * Returns a pointer to the global LRU object that manages the
 * GeomVertexArrayData's that are deemed too small to be paged out.
 */
INLINE SimpleLru *GeomVertexArrayData::
get_small_lru() {
  return &_small_lru;
}

/**
 * Returns the global VertexDataBook that will be used to allocate vertex data
 * buffers.
 */
INLINE VertexDataBook &GeomVertexArrayData::
get_book() {
  return _book;
}

/**
 * Should be called when the size of the buffer changes.
 */
INLINE void GeomVertexArrayData::
set_lru_size(size_t lru_size) {
  SimpleLruPage::set_lru_size(lru_size);

  if ((int)lru_size <= vertex_data_small_size) {
    SimpleLruPage::mark_used_lru(&_small_lru);
  } else {
    SimpleLruPage::mark_used_lru(&_independent_lru);
  }
}

/**
 */
INLINE void GeomVertexArrayData::
mark_used() {
  if ((int)get_lru_size() <= vertex_data_small_size) {
    SimpleLruPage::mark_used_lru(&_small_lru);
  } else {
    SimpleLruPage::mark_used_lru(&_independent_lru);
  }
}

/**
 *
 */
INLINE GeomVertexArrayData::CData::
CData(UsageHint usage_hint) :
  _usage_hint(usage_hint),
  _rw_lock("GeomVertexArrayData::CData::_rw_lock")
{
}

/**
 *
 */
INLINE GeomVertexArrayData::CData::
CData(GeomVertexArrayData::CData &&from) noexcept :
  _usage_hint(std::move(from._usage_hint)),
  _buffer(std::move(from._buffer)),
  _modified(std::move(from._modified)),
  _rw_lock("GeomVertexArrayData::CData::_rw_lock")
{
}

/**
 *
 */
INLINE GeomVertexArrayData::CData::
CData(const GeomVertexArrayData::CData &copy) :
  _usage_hint(copy._usage_hint),
  _buffer(copy._buffer),
  _modified(copy._modified),
  _rw_lock("GeomVertexArrayData::CData::_rw_lock")
{
}

/**
 *
 */
INLINE void GeomVertexArrayData::CData::
operator = (const GeomVertexArrayData::CData &copy) {
  _usage_hint = copy._usage_hint;
  _buffer = copy._buffer;
  _modified = copy._modified;
}

/**
 *
 */
INLINE GeomVertexArrayDataHandle::
GeomVertexArrayDataHandle(CPT(GeomVertexArrayData) object,
                          Thread *current_thread) :
  _current_thread(current_thread),
  _cdata((GeomVertexArrayData::CData *)object->_cycler.read_unlocked(current_thread)),
  _writable(false)
{
  _object.swap(object);

#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG
#ifdef DO_PIPELINING
  _cdata->ref();
#endif  // DO_PIPELINING
  // We must grab the lock *after* we have incremented the reference count,
  // above.
  _cdata->_rw_lock.acquire();
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif
}

/**
 *
 */
INLINE GeomVertexArrayDataHandle::
GeomVertexArrayDataHandle(const GeomVertexArrayData *object,
                          Thread *current_thread) :
  _object((GeomVertexArrayData *)object),
  _current_thread(current_thread),
  _cdata((GeomVertexArrayData::CData *)object->_cycler.read_unlocked(current_thread)),
  _writable(false)
{
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG
#ifdef DO_PIPELINING
  _cdata->ref();
#endif  // DO_PIPELINING
  // We must grab the lock *after* we have incremented the reference count,
  // above.
  _cdata->_rw_lock.acquire();
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif
}

/**
 *
 */
INLINE GeomVertexArrayDataHandle::
GeomVertexArrayDataHandle(PT(GeomVertexArrayData) object,
                          Thread *current_thread) :
  _current_thread(current_thread),
  _cdata(object->_cycler.write_upstream(true, current_thread)),
  _writable(true)
{
  _object.swap(object);

#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG
#ifdef DO_PIPELINING
  _cdata->ref();
#endif  // DO_PIPELINING
  // We must grab the lock *after* we have incremented the reference count,
  // above.
  _cdata->_rw_lock.acquire();
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif
}

/**
 *
 */
INLINE GeomVertexArrayDataHandle::
GeomVertexArrayDataHandle(GeomVertexArrayData *object,
                          Thread *current_thread) :
  _object(object),
  _current_thread(current_thread),
  _cdata(object->_cycler.write_upstream(true, current_thread)),
  _writable(true)
{
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG
#ifdef DO_PIPELINING
  _cdata->ref();
#endif  // DO_PIPELINING
  // We must grab the lock *after* we have incremented the reference count,
  // above.
  _cdata->_rw_lock.acquire();
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif
}

/**
 *
 */
INLINE GeomVertexArrayDataHandle::
~GeomVertexArrayDataHandle() {
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG

  if (_writable) {
    _object->_cycler.release_write(_cdata);
  }

  // We must release the lock *before* we decrement the reference count,
  // below.
  _cdata->_rw_lock.release();

#ifdef DO_PIPELINING
  unref_delete((CycleData *)_cdata);
#endif  // DO_PIPELINING

#ifdef _DEBUG
  _object = nullptr;
  _cdata = nullptr;
#endif  // _DEBUG
}

/**
 *
 */
INLINE Thread *GeomVertexArrayDataHandle::
get_current_thread() const {
  return _current_thread;
}

/**
 * Returns a readable pointer to the beginning of the actual data stream, or
 * NULL if the data is not currently resident.  If the data is not currently
 * resident, this will implicitly request it to become resident soon.
 *
 * If force is true, this method will never return NULL, but may block until
 * the data is available.
 */
INLINE const unsigned char *GeomVertexArrayDataHandle::
get_read_pointer(bool force) const {
  mark_used();
  return _cdata->_buffer.get_read_pointer(force);
}

/**
 *
 */
INLINE const GeomVertexArrayData *GeomVertexArrayDataHandle::
get_object() const {
  return _object;
}

/**
 *
 */
INLINE GeomVertexArrayData *GeomVertexArrayDataHandle::
get_object() {
  return _object;
}

/**
 *
 */
INLINE const GeomVertexArrayFormat *GeomVertexArrayDataHandle::
get_array_format() const {
  return _object->_array_format;
}

/**
 *
 */
INLINE GeomVertexArrayDataHandle::UsageHint GeomVertexArrayDataHandle::
get_usage_hint() const {
  return _cdata->_usage_hint;
}

/**
 *
 */
INLINE int GeomVertexArrayDataHandle::
get_num_rows() const {
  nassertr(_object->_array_format->get_stride() != 0, 0);
  return get_data_size_bytes() / _object->_array_format->get_stride();
}

/**
 *
 */
INLINE void GeomVertexArrayDataHandle::
clear_rows() {
  set_num_rows(0);
}

/**
 *
 */
INLINE size_t GeomVertexArrayDataHandle::
get_data_size_bytes() const {
  return _cdata->_buffer.get_size();
}

/**
 *
 */
INLINE UpdateSeq GeomVertexArrayDataHandle::
get_modified() const {
  return _cdata->_modified;
}

/**
 * Returns true if the vertex data is currently resident in memory.  If this
 * returns true, the next call to get_handle()->get_read_pointer() will
 * probably not block.  If this returns false, the vertex data will be brought
 * back into memory shortly; try again later.
 */
INLINE bool GeomVertexArrayDataHandle::
request_resident() const {
  return (get_read_pointer(false) != nullptr);
}

/**
 * Creates a context for the data on the particular GSG, if it does not
 * already exist.  Returns the new (or old) VertexBufferContext.  This assumes
 * that the GraphicsStateGuardian is the currently active rendering context
 * and that it is ready to accept new datas.  If this is not necessarily the
 * case, you should use prepare() instead.
 *
 * Normally, this is not called directly except by the GraphicsStateGuardian;
 * a data does not need to be explicitly prepared by the user before it may be
 * rendered.
 */
INLINE VertexBufferContext *GeomVertexArrayDataHandle::
prepare_now(PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) const {
  return _object->prepare_now(prepared_objects, gsg);
}

/**
 * Returns the entire raw data of the GeomVertexArrayData object, formatted as
 * a string.  This is primarily for the benefit of high-level languages such
 * as Python.
 */
INLINE vector_uchar GeomVertexArrayDataHandle::
get_data() const {
  mark_used();
  const unsigned char *ptr = _cdata->_buffer.get_read_pointer(true);
  return vector_uchar(ptr, ptr + _cdata->_buffer.get_size());
}

/**
 * Returns a subset of the raw data of the GeomVertexArrayData object,
 * formatted as a string.  This is primarily for the benefit of high-level
 * languages such as Python.
 */
INLINE vector_uchar GeomVertexArrayDataHandle::
get_subdata(size_t start, size_t size) const {
  mark_used();
  start = std::min(start, _cdata->_buffer.get_size());
  size = std::min(size, _cdata->_buffer.get_size() - start);
  const unsigned char *ptr = _cdata->_buffer.get_read_pointer(true) + start;
  return vector_uchar(ptr, ptr + size);
}

/**
 * Marks the array data recently-used.
 */
void GeomVertexArrayDataHandle::
mark_used() const {
  _object->mark_used();
}

INLINE std::ostream &
operator << (std::ostream &out, const GeomVertexArrayData &obj) {
  obj.output(out);
  return out;
}
