// Filename: pipelineCyclerTrueImpl.I
// Created by:  drose (31Jan06)
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
//     Function: PipelineCyclerTrueImpl::read
//       Access: Public
//  Description: Returns a const CycleData pointer, filled with the
//               data for the current stage of the pipeline as seen by
//               this thread.  This pointer should eventually be
//               released by calling release_read().
//
//               There should be no outstanding write pointers on the
//               data when this function is called.
////////////////////////////////////////////////////////////////////
INLINE const CycleData *PipelineCyclerTrueImpl::
read() const {
  int pipeline_stage = Thread::get_current_thread()->get_pipeline_stage();
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, NULL);
  _lock.lock();
  return _data[pipeline_stage];
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::increment_read
//       Access: Public
//  Description: Increments the count on a pointer previously
//               retrieved by read(); now the pointer will need to be
//               released twice.
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::
increment_read(const CycleData *pointer) const {
#ifndef NDEBUG
  int pipeline_stage = Thread::get_current_thread()->get_pipeline_stage();
  nassertv(pipeline_stage >= 0 && pipeline_stage < _num_stages);
  nassertv(_data[pipeline_stage] == pointer);
#endif
  _lock.lock();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::release_read
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               read().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::
release_read(const CycleData *pointer) const {
#ifndef NDEBUG
  int pipeline_stage = Thread::get_current_thread()->get_pipeline_stage();
  nassertv(pipeline_stage >= 0 && pipeline_stage < _num_stages);
  nassertv(_data[pipeline_stage] == pointer);
#endif
  _lock.release();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::write
//       Access: Public
//  Description: Returns a non-const CycleData pointer, filled with a
//               unique copy of the data for the current stage of the
//               pipeline as seen by this thread.  This pointer may
//               now be used to write to the data, and that copy of
//               the data will be propagated to all later stages of the
//               pipeline.  This pointer should eventually be released
//               by calling release_write().
//
//               There may only be one outstanding write pointer on a
//               given stage at a time, and if there is a write
//               pointer there may be no read pointers on the same
//               stage (but see elevate_read).
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerTrueImpl::
write() {
  int pipeline_stage = Thread::get_current_thread()->get_pipeline_stage();
  return write_stage(pipeline_stage);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::elevate_read
//       Access: Public
//  Description: Elevates a currently-held read pointer into a write
//               pointer.  This may or may not change the value of the
//               pointer.  It is only valid to do this if this is the
//               only currently-outstanding read pointer on the
//               current stage.
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerTrueImpl::
elevate_read(const CycleData *pointer) {
  CycleData *new_pointer = write();
  _lock.release();
  return new_pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::release_write
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               write().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::
release_write(CycleData *pointer) {
#ifdef NDEBUG
  int pipeline_stage = Thread::get_current_thread()->get_pipeline_stage();
  return release_write_stage(pipeline_stage);
#else
  _lock.release();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::get_num_stages
//       Access: Public
//  Description: Returns the number of stages in the pipeline.
////////////////////////////////////////////////////////////////////
INLINE int PipelineCyclerTrueImpl::
get_num_stages() {
  return _num_stages;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::is_stage_unique
//       Access: Public
//  Description: Returns true if the nth stage is a different pointer
//               than the previous stage, or false if its pointer is
//               shared with the previous one.
////////////////////////////////////////////////////////////////////
INLINE bool PipelineCyclerTrueImpl::
is_stage_unique(int n) const {
  nassertr(n >= 0 && n < _num_stages, false);
  if (n == 0) {
    return true;
  } else {
    return _data[n] == _data[n - 1];
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::release_write_stage
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               write_stage().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::
release_write_stage(int n, CycleData *pointer) {
#ifndef NDEBUG
  nassertv(n >= 0 && n < _num_stages);
  nassertv(_data[n] == pointer);
#endif
  _lock.release();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::cheat
//       Access: Public
//  Description: Returns a pointer without counting it.  This is only
//               intended for use as the return value for certain
//               nassertr() functions, so the application can recover
//               after a failure to manage the read and write pointers
//               correctly.  You should never call this function
//               directly.
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerTrueImpl::
cheat() const {
  int pipeline_stage = Thread::get_current_thread()->get_pipeline_stage();
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, NULL);
  return _data[pipeline_stage];
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::get_read_count
//       Access: Public
//  Description: Returns the number of handles currently outstanding
//               to read the current stage of the data.  This should
//               only be used for debugging purposes.
////////////////////////////////////////////////////////////////////
INLINE int PipelineCyclerTrueImpl::
get_read_count() const {
  int pipeline_stage = Thread::get_current_thread()->get_pipeline_stage();
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, 0);
  _lock.lock();
  return _lock.get_lock_count();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::get_write_count
//       Access: Public
//  Description: Returns the number of handles currently outstanding
//               to read the current stage of the data.  This will
//               normally only be either 0 or 1.  This should only be
//               used for debugging purposes.
////////////////////////////////////////////////////////////////////
INLINE int PipelineCyclerTrueImpl::
get_write_count() const {
  int pipeline_stage = Thread::get_current_thread()->get_pipeline_stage();
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, 0);
  _lock.lock();
  return _lock.get_lock_count();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::cycle_2
//       Access: Private
//  Description: This is a special implementation of cycle() for the
//               special case of just two stages to the pipeline.  It
//               does the same thing as cycle(), but is a little bit
//               faster because it knows there are exactly two stages.
////////////////////////////////////////////////////////////////////
INLINE PT(CycleData) PipelineCyclerTrueImpl::
cycle_2() {
  PT(CycleData) last_val = _data[1];
  nassertr(_lock.debug_is_locked(), last_val);
  nassertr(_dirty, last_val);
  nassertr(_num_stages == 2, last_val);

  _data[1] = _data[0];

  // No longer dirty.
  _dirty = false;
  return last_val;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::cycle_3
//       Access: Private
//  Description: This is a special implementation of cycle() for the
//               special case of exactly three stages to the pipeline.
//               It does the same thing as cycle(), but is a little
//               bit faster because it knows there are exactly three
//               stages.
////////////////////////////////////////////////////////////////////
INLINE PT(CycleData) PipelineCyclerTrueImpl::
cycle_3() {
  PT(CycleData) last_val = _data[2];
  nassertr(_lock.debug_is_locked(), last_val);
  nassertr(_dirty, last_val);
  nassertr(_num_stages == 3, last_val);

  _data[2] = _data[1];
  _data[1] = _data[0];

  if (_data[2] == _data[1]) {
    // No longer dirty.
    _dirty = false;
  }

  return last_val;
}
