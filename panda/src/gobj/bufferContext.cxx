/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bufferContext.cxx
 * @author drose
 * @date 2006-03-16
 */

#include "bufferContext.h"
#include "lightMutexHolder.h"

TypeHandle BufferContext::_type_handle;

/**
 * Returns an implementation-defined handle or pointer that can be used
 * to interface directly with the underlying API.
 * Returns 0 if the underlying implementation does not support this.
 */
uint64_t BufferContext::
get_native_id() const {
  return 0;
}

/**
 * Similar to get_native_id, but some implementations use a separate
 * identifier for the buffer object associated with buffer textures.
 * Returns 0 if the underlying implementation does not support this, or
 * if this is not a buffer texture.
 */
uint64_t BufferContext::
get_native_buffer_id() const {
  return 0;
}

/**
 *
 */
BufferContext::
BufferContext(BufferResidencyTracker *residency, TypedWritableReferenceCount *object) :
  _object(object),
  _residency(residency),
  _residency_state(0),
  _data_size_bytes(0),
  _owning_chain(nullptr)
{
  set_owning_chain(&residency->_chains[0]);
}

/**
 *
 */
BufferContext::
~BufferContext() {
  set_owning_chain(nullptr);
}

/**
 * Moves this object to a different BufferContextChain.
 */
void BufferContext::
set_owning_chain(BufferContextChain *chain) {
  if (chain != _owning_chain) {
    if (_owning_chain != nullptr) {
      LightMutexHolder holder(_owning_chain->_lock);
      --(_owning_chain->_count);
      _owning_chain->adjust_bytes(-(int)_data_size_bytes);
      remove_from_list();
    }

    _owning_chain = chain;

    if (_owning_chain != nullptr) {
      LightMutexHolder holder(_owning_chain->_lock);
      ++(_owning_chain->_count);
      _owning_chain->adjust_bytes((int)_data_size_bytes);
      insert_before(_owning_chain);
    }
  }
}
