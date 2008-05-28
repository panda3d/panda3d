// Filename: bufferContext.cxx
// Created by:  drose (16Mar06)
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

#include "bufferContext.h"

TypeHandle BufferContext::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BufferContext::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BufferContext::
BufferContext(BufferResidencyTracker *residency) :
  _residency(residency),
  _residency_state(0),
  _data_size_bytes(0),
  _owning_chain(NULL)
{
  set_owning_chain(&residency->_chains[0]);
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContext::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
BufferContext::
~BufferContext() {
  set_owning_chain(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContext::set_owning_chain
//       Access: Private
//  Description: Moves this object to a different BufferContextChain.
////////////////////////////////////////////////////////////////////
void BufferContext::
set_owning_chain(BufferContextChain *chain) {
  if (chain != _owning_chain) {
    if (_owning_chain != (BufferContextChain *)NULL){ 
      --(_owning_chain->_count);
      _owning_chain->adjust_bytes(-(int)_data_size_bytes);
      remove_from_list();
    }

    _owning_chain = chain;

    if (_owning_chain != (BufferContextChain *)NULL) {
      ++(_owning_chain->_count);
      _owning_chain->adjust_bytes((int)_data_size_bytes);
      insert_before(_owning_chain);
    }
  }
}
