/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bufferContext.I
 * @author drose
 * @date 2006-03-16
 */

/**
 * Returns the associated object.
 */
INLINE TypedWritableReferenceCount *BufferContext::
get_object() const {
  return _object;
}

/**
 * Returns the number of bytes previously reported for the data object.  This
 * is used to track changes in the data object's allocated size; if it changes
 * from this, we need to create a new buffer.  This is also used to track
 * memory utilization in PStats.
 */
INLINE size_t BufferContext::
get_data_size_bytes() const {
  return _data_size_bytes;
}

/**
 * Returns the UpdateSeq that was recorded the last time mark_loaded() was
 * called.
 */
INLINE UpdateSeq BufferContext::
get_modified() const {
  return _modified;
}

/**
 * Returns the active flag associated with this object.  An object is
 * considered "active" if it was rendered in the current frame.
 */
INLINE bool BufferContext::
get_active() const {
  return (_residency_state & BufferResidencyTracker::S_active) != 0;
}

/**
 * Returns the resident flag associated with this object.  An object is
 * considered "resident" if it appears to be resident in texture memory.
 */
INLINE bool BufferContext::
get_resident() const {
  return (_residency_state & BufferResidencyTracker::S_resident) != 0;
}

/**
 * Changes the active flag associated with this object.  An object is
 * considered "active" if it was rendered in the current frame.
 */
INLINE void BufferContext::
set_active(bool flag) {
  if (flag) {
    _residency_state |= BufferResidencyTracker::S_active;
    // Assume that rendering the object automatically makes it resident.
    _residency_state |= BufferResidencyTracker::S_resident;
  } else {
    _residency_state &= ~BufferResidencyTracker::S_active;
  }
  set_owning_chain(&_residency->_chains[_residency_state]);
}

/**
 * Changes the resident flag associated with this object.  An object is
 * considered "resident" if it appears to be resident in texture memory.
 */
INLINE void BufferContext::
set_resident(bool flag) {
  if (flag) {
    _residency_state |= BufferResidencyTracker::S_resident;
  } else {
    _residency_state &= ~BufferResidencyTracker::S_resident;
  }
  set_owning_chain(&_residency->_chains[_residency_state]);
}

/**
 * This can be used along with BufferContextChain::get_first() to walk through
 * the list of objects stored on a tracker.
 */
INLINE BufferContext *BufferContext::
get_next() const {
  nassertr(_owning_chain != nullptr, nullptr);
  if ((BufferContextChain *)_next == _owning_chain) {
    return nullptr;
  }
  return (BufferContext *)_next;
}

/**
 * Should be called (usually by a derived class) when the on-card size of this
 * object has changed.
 */
INLINE void BufferContext::
update_data_size_bytes(size_t new_data_size_bytes) {
  if (_owning_chain != nullptr) {
    _owning_chain->adjust_bytes((int)new_data_size_bytes - (int)_data_size_bytes);
  }
  _data_size_bytes = new_data_size_bytes;
}

/**
 * Should be called (usually by a derived class) when the modified counter for
 * this object has changed.
 */
INLINE void BufferContext::
update_modified(UpdateSeq new_modified) {
  _modified = new_modified;
}
