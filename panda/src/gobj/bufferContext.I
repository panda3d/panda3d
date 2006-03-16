// Filename: bufferContext.I
// Created by:  drose (16Mar06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: BufferContext::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BufferContext::
BufferContext(BufferResidencyTracker *residency) :
  _residency(residency),
  _residency_state(0),
  _data_size_bytes(0),
  _owning_chain(&residency->_chains[0])
{
#ifdef DO_PSTATS
  ++(_owning_chain->_count);
  insert_before(_owning_chain);
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContext::get_data_size_bytes
//       Access: Public
//  Description: Returns the number of bytes previously reported for
//               the data object.  This is used to track changes in
//               the data object's allocated size; if it changes from
//               this, we need to create a new buffer.  This is also
//               used to track memory utilization in PStats.
////////////////////////////////////////////////////////////////////
INLINE size_t BufferContext::
get_data_size_bytes() const {
  return _data_size_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContext::get_modified
//       Access: Public
//  Description: Returns the UpdateSeq that was recorded the last time
//               mark_loaded() was called.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq BufferContext::
get_modified() const {
  return _modified;
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContext::set_active
//       Access: Public
//  Description: Changes the active flag associated with this object.
//               An object is considered "active" if it was rendered
//               in the current frame.
////////////////////////////////////////////////////////////////////
INLINE void BufferContext::
set_active(bool flag) {
#ifdef DO_PSTATS
  if (flag) {
    _residency_state |= BufferResidencyTracker::S_active;
    // Assume that rendering the object automatically makes it
    // resident.
    _residency_state |= BufferResidencyTracker::S_resident;
  } else {
    _residency_state &= ~BufferResidencyTracker::S_active;
  }
  set_owning_chain(&_residency->_chains[_residency_state]);
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContext::set_resident
//       Access: Public
//  Description: Changes the resident flag associated with this
//               object.  An object is considered "resident" if it
//               appears to be resident in texture memory.
////////////////////////////////////////////////////////////////////
INLINE void BufferContext::
set_resident(bool flag) {
#ifdef DO_PSTATS
  if (flag) {
    _residency_state |= BufferResidencyTracker::S_resident;
  } else {
    _residency_state &= ~BufferResidencyTracker::S_resident;
  }
  set_owning_chain(&_residency->_chains[_residency_state]);
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContext::get_next
//       Access: Public
//  Description: This can be used along with
//               BufferContextChain::get_first() to walk through the
//               list of objects stored on a tracker.
////////////////////////////////////////////////////////////////////
INLINE BufferContext *BufferContext::
get_next() const {
  if ((BufferContextChain *)_next == _owning_chain) {
    return NULL;
  }
  return (BufferContext *)_next;
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContext::update_data_size_bytes
//       Access: Public
//  Description: Should be called (usually by a derived class) when
//               the on-card size of this object has changed.
////////////////////////////////////////////////////////////////////
INLINE void BufferContext::
update_data_size_bytes(size_t new_data_size_bytes) {
  _owning_chain->adjust_bytes((int)new_data_size_bytes - (int)_data_size_bytes);
  _data_size_bytes = new_data_size_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContext::update_modified
//       Access: Public
//  Description: Should be called (usually by a derived class) when
//               the modified counter for this object has changed.
////////////////////////////////////////////////////////////////////
INLINE void BufferContext::
update_modified(UpdateSeq new_modified) {
  _modified = new_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContext::set_owning_chain
//       Access: Private
//  Description: Moves this object to a different BufferContextChain.
////////////////////////////////////////////////////////////////////
INLINE void BufferContext::
set_owning_chain(BufferContextChain *chain) {
  if (chain != _owning_chain) {
    --(_owning_chain->_count);
    _owning_chain->adjust_bytes(-(int)_data_size_bytes);
    remove_from_list();
    _owning_chain = chain;
    ++(_owning_chain->_count);
    _owning_chain->adjust_bytes((int)_data_size_bytes);
    insert_before(_owning_chain);
  }
}
