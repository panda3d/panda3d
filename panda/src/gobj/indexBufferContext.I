/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file indexBufferContext.I
 * @author drose
 * @date 2005-03-17
 */

/**
 *
 */
INLINE IndexBufferContext::
IndexBufferContext(PreparedGraphicsObjects *pgo, GeomPrimitive *data) :
  BufferContext(&pgo->_ibuffer_residency, data),
  AdaptiveLruPage(0)
{
}

/**
 * Returns the pointer to the client-side array data object.
 */
INLINE GeomPrimitive *IndexBufferContext::
get_data() const {
  return (GeomPrimitive *)_object;
}

/**
 * Returns true if the data has changed size since the last time mark_loaded()
 * was called.
 */
INLINE bool IndexBufferContext::
changed_size(const GeomPrimitivePipelineReader *reader) const {
  nassertr(reader->get_object() == get_data(), false);
  return get_data_size_bytes() != (size_t)reader->get_data_size_bytes();
}

/**
 * Returns true if the data has changed its usage hint since the last time
 * mark_loaded() was called.
 */
INLINE bool IndexBufferContext::
changed_usage_hint(const GeomPrimitivePipelineReader *reader) const {
  nassertr(reader->get_object() == get_data(), false);
  return _usage_hint != reader->get_usage_hint();
}

/**
 * Returns true if the data has been modified since the last time
 * mark_loaded() was called.
 */
INLINE bool IndexBufferContext::
was_modified(const GeomPrimitivePipelineReader *reader) const {
  nassertr(reader->get_object() == get_data(), false);
  return get_modified() != reader->get_modified();
}

/**
 * Should be called (usually by a derived class) when the on-card size of this
 * object has changed.
 */
INLINE void IndexBufferContext::
update_data_size_bytes(size_t new_data_size_bytes) {
  BufferContext::update_data_size_bytes(new_data_size_bytes);
  AdaptiveLruPage::set_lru_size(new_data_size_bytes);
}

/**
 * Should be called after the IndexBufferContext has been loaded into graphics
 * memory, this updates the internal flags for changed_size() and modified().
 */
INLINE void IndexBufferContext::
mark_loaded(const GeomPrimitivePipelineReader *reader) {
  nassertv(reader->get_object() == get_data());
  update_data_size_bytes(reader->get_data_size_bytes());
  update_modified(reader->get_modified());
  _usage_hint = reader->get_usage_hint();

  // Assume the buffer is now resident.
  set_resident(true);
}

/**
 * Should be called after the buffer has been forced out of graphics memory.
 */
INLINE void IndexBufferContext::
mark_unloaded() {
  update_modified(UpdateSeq::old());
  set_resident(false);
}
