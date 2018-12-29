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

TypeHandle BufferContext::_type_handle;

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
    if (_owning_chain != nullptr){
      --(_owning_chain->_count);
      _owning_chain->adjust_bytes(-(int)_data_size_bytes);
      remove_from_list();
    }

    _owning_chain = chain;

    if (_owning_chain != nullptr) {
      ++(_owning_chain->_count);
      _owning_chain->adjust_bytes((int)_data_size_bytes);
      insert_before(_owning_chain);
    }
  }
}
