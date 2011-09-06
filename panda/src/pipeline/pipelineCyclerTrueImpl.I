// Filename: pipelineCyclerTrueImpl.I
// Created by:  drose (31Jan06)
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
//     Function: PipelineCyclerTrueImpl::acquire
//       Access: Public
//  Description: Grabs an overall lock on the cycler.  Release it with
//               a call to release().  This lock should be held while
//               walking the list of stages.
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::
acquire() {
  TAU_PROFILE("void PipelineCyclerTrueImpl::acquire()", " ", TAU_USER);
  _lock.acquire();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::acquire
//       Access: Public
//  Description: Grabs an overall lock on the cycler.  Release it with
//               a call to release().  This lock should be held while
//               walking the list of stages.
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::
acquire(Thread *current_thread) {
  TAU_PROFILE("void PipelineCyclerTrueImpl::acquire(Thread *)", " ", TAU_USER);
  _lock.acquire(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::release
//       Access: Public
//  Description: Release the overall lock on the cycler that was
//               grabbed via acquire().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::
release() {
  TAU_PROFILE("void PipelineCyclerTrueImpl::release()", " ", TAU_USER);
  _lock.release();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::read_unlocked
//       Access: Public
//  Description: Returns a const CycleData pointer, filled with the
//               data for the current stage of the pipeline as seen by
//               this thread.  No lock is made on the contents; there
//               is no guarantee that some other thread won't modify
//               this object's data while you are working on it.
//               (However, the data within the returned CycleData
//               object itself is safe from modification; if another
//               thread modifies the data, it will perform a
//               copy-on-write, and thereby change the pointer stored
//               within the object.)
////////////////////////////////////////////////////////////////////
INLINE const CycleData *PipelineCyclerTrueImpl::
read_unlocked(Thread *current_thread) const {
  TAU_PROFILE("const CycleData *PipelineCyclerTrueImpl::read_unlocked(Thread *)", " ", TAU_USER);
  int pipeline_stage = current_thread->get_pipeline_stage();
#ifdef _DEBUG
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, NULL);
#endif
  return _data[pipeline_stage]._cdata;
}

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
read(Thread *current_thread) const {
  TAU_PROFILE("const CycleData *PipelineCyclerTrueImpl::read(Thread *)", " ", TAU_USER);
  int pipeline_stage = current_thread->get_pipeline_stage();
#ifdef _DEBUG
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, NULL);
#endif
  _lock.acquire(current_thread);
  return _data[pipeline_stage]._cdata;
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
  TAU_PROFILE("void PipelineCyclerTrueImpl::increment_read(const CycleData *)", " ", TAU_USER);
#ifdef _DEBUG
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage >= 0 && pipeline_stage < _num_stages);
  nassertv(_data[pipeline_stage]._cdata == pointer);
#endif
  _lock.elevate_lock();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::release_read
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               read().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::
release_read(const CycleData *pointer) const {
  TAU_PROFILE("void PipelineCyclerTrueImpl::release_read(const CycleData *)", " ", TAU_USER);
#ifdef _DEBUG
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage >= 0 && pipeline_stage < _num_stages);
  nassertv(_data[pipeline_stage]._cdata == pointer);
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
write(Thread *current_thread) {
  TAU_PROFILE("CycleData *PipelineCyclerTrueImpl::write(Thread *)", " ", TAU_USER);
  return write_stage(current_thread->get_pipeline_stage(), current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::write_upstream
//       Access: Public
//  Description: This special variant on write() will automatically
//               propagate changes back to upstream pipeline stages.
//               If force_to_0 is false, then it propagates back only
//               as long as the CycleData pointers are equivalent,
//               guaranteeing that it does not modify upstream data
//               (other than the modification that will be performed
//               by the code that returns this pointer).  This is
//               particularly appropriate for minor updates, where it
//               doesn't matter much if the update is lost, such as
//               storing a cached value.
//
//               If force_to_0 is true, then the CycleData pointer for
//               the current pipeline stage is propagated all the way
//               back up to stage 0; after this call, there will be
//               only one CycleData pointer that is duplicated in all
//               stages between stage 0 and the current stage.  This
//               may undo some recent changes that were made
//               independently at pipeline stage 0 (or any other
//               upstream stage).  However, it guarantees that the
//               change that is to be applied at this pipeline stage
//               will stick.  This is slightly dangerous because of
//               the risk of losing upstream changes; generally, this
//               should only be done when you are confident that there
//               are no upstream changes to be lost (for instance, for
//               an object that has been recently created).
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerTrueImpl::
write_upstream(bool force_to_0, Thread *current_thread) {
  TAU_PROFILE("CycleData *PipelineCyclerTrueImpl::write_upstream(bool, Thread *)", " ", TAU_USER);
  return write_stage_upstream(current_thread->get_pipeline_stage(), force_to_0,
                              current_thread);
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
elevate_read(const CycleData *pointer, Thread *current_thread) {
  TAU_PROFILE("CycleData *PipelineCyclerTrueImpl::elevate_read(const CycleData *)", " ", TAU_USER);
#ifdef _DEBUG
  int pipeline_stage = current_thread->get_pipeline_stage();
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, NULL);
  nassertr(_data[pipeline_stage]._cdata == pointer, NULL);
#endif
  CycleData *new_pointer = write(current_thread);
  _lock.release();
  return new_pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::elevate_read_upstream
//       Access: Public
//  Description: Elevates a currently-held read pointer into a write
//               pointer, like elevate_read(), but also propagates the
//               pointer back to upstream stages, like
//               write_upstream().
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerTrueImpl::
elevate_read_upstream(const CycleData *pointer, bool force_to_0, Thread *current_thread) {
  TAU_PROFILE("CycleData *PipelineCyclerTrueImpl::elevate_read_upstream(const CycleData *, bool)", " ", TAU_USER);
#ifdef _DEBUG
  int pipeline_stage = current_thread->get_pipeline_stage();
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, NULL);
  nassertr(_data[pipeline_stage]._cdata == pointer, NULL);
#endif
  CycleData *new_pointer = write_upstream(force_to_0, current_thread);
  _lock.release();
  return new_pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::increment_write
//       Access: Public
//  Description: Increments the count on a pointer previously
//               retrieved by write(); now the pointer will need to be
//               released twice.
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::
increment_write(CycleData *pointer) const {
  TAU_PROFILE("void PipelineCyclerTrueImpl::increment_write(CycleData *)", " ", TAU_USER);
  int pipeline_stage = Thread::get_current_pipeline_stage();
#ifdef _DEBUG
  nassertv(pipeline_stage >= 0 && pipeline_stage < _num_stages);
  nassertv(_data[pipeline_stage]._cdata == pointer);
#endif
  ++(_data[pipeline_stage]._writes_outstanding);
  _lock.elevate_lock();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::release_write
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               write().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::
release_write(CycleData *pointer) {
  TAU_PROFILE("void PipelineCyclerTrueImpl::release_write(CycleData *)", " ", TAU_USER);
  int pipeline_stage = Thread::get_current_pipeline_stage();
  return release_write_stage(pipeline_stage, pointer);
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
//     Function: PipelineCyclerTrueImpl::read_stage_unlocked
//       Access: Public
//  Description: Returns a const CycleData pointer, filled with the
//               data for the indicated stage of the pipeline.  As in
//               read_unlocked(), no lock is held on the returned
//               pointer.
////////////////////////////////////////////////////////////////////
INLINE const CycleData *PipelineCyclerTrueImpl::
read_stage_unlocked(int pipeline_stage) const {
  TAU_PROFILE("const CycleData *PipelineCyclerTrueImpl::read_stage_unlocked(int)", " ", TAU_USER);
#ifdef _DEBUG
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, NULL);
#endif
  return _data[pipeline_stage]._cdata;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::read_stage
//       Access: Public
//  Description: Returns a const CycleData pointer, filled with the
//               data for the indicated stage of the pipeline.  This
//               pointer should eventually be released by calling
//               release_read_stage().
//
//               There should be no outstanding write pointers on the
//               data when this function is called.
////////////////////////////////////////////////////////////////////
INLINE const CycleData *PipelineCyclerTrueImpl::
read_stage(int pipeline_stage, Thread *current_thread) const {
  TAU_PROFILE("const CycleData *PipelineCyclerTrueImpl::read_stage(int, Thread *)", " ", TAU_USER);
#ifdef _DEBUG
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, NULL);
#endif
  _lock.acquire(current_thread);
  return _data[pipeline_stage]._cdata;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::release_read_stage
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               read_stage().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::
release_read_stage(int pipeline_stage, const CycleData *pointer) const {
  TAU_PROFILE("void PipelineCyclerTrueImpl::release_read_stage(int, const CycleData *)", " ", TAU_USER);
#ifdef _DEBUG
  nassertv(pipeline_stage >= 0 && pipeline_stage < _num_stages);
  nassertv(_data[pipeline_stage]._cdata == pointer);
#endif
  _lock.release();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::elevate_read_stage
//       Access: Public
//  Description: Elevates a currently-held read pointer into a write
//               pointer.  This may or may not change the value of the
//               pointer.  It is only valid to do this if this is the
//               only currently-outstanding read pointer on the
//               indicated stage.
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerTrueImpl::
elevate_read_stage(int pipeline_stage, const CycleData *pointer,
                   Thread *current_thread) {
  TAU_PROFILE("CycleData *PipelineCyclerTrueImpl::elevate_read_stage(int, const CycleData *)", " ", TAU_USER);
#ifdef _DEBUG
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, NULL);
  nassertr(_data[pipeline_stage]._cdata == pointer, NULL);
#endif
  CycleData *new_pointer = write_stage(pipeline_stage, current_thread);
  _lock.release();
  return new_pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::elevate_read_stage_upstream
//       Access: Public
//  Description: Elevates a currently-held read pointer into a write
//               pointer.  This may or may not change the value of the
//               pointer.  It is only valid to do this if this is the
//               only currently-outstanding read pointer on the
//               indicated stage.
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerTrueImpl::
elevate_read_stage_upstream(int pipeline_stage, const CycleData *pointer, 
                            bool force_to_0, Thread *current_thread) {
  TAU_PROFILE("CycleData *PipelineCyclerTrueImpl::elevate_read_stage(int, const CycleData *)", " ", TAU_USER);
#ifdef _DEBUG
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, NULL);
  nassertr(_data[pipeline_stage]._cdata == pointer, NULL);
#endif
  CycleData *new_pointer = 
    write_stage_upstream(pipeline_stage, force_to_0, current_thread);
  _lock.release();
  return new_pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::release_write_stage
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               write_stage().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::
release_write_stage(int pipeline_stage, CycleData *pointer) {
  TAU_PROFILE("void PipelineCyclerTrueImpl::release_write_stage(int, const CycleData *)", " ", TAU_USER);
#ifdef _DEBUG
  nassertv(pipeline_stage >= 0 && pipeline_stage < _num_stages);
  nassertv(_data[pipeline_stage]._cdata == pointer);
  nassertv(_data[pipeline_stage]._writes_outstanding > 0);
#endif
  --(_data[pipeline_stage]._writes_outstanding);
  _lock.release();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::get_parent_type
//       Access: Public
//  Description: Returns the type of object that owns this cycler, as
//               reported by CycleData::get_parent_type().
////////////////////////////////////////////////////////////////////
INLINE TypeHandle PipelineCyclerTrueImpl::
get_parent_type() const {
  return _data[0]._cdata->get_parent_type();
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
  TAU_PROFILE("CycleData *PipelineCyclerTrueImpl::cheat()", " ", TAU_USER);
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertr(pipeline_stage >= 0 && pipeline_stage < _num_stages, NULL);
  return _data[pipeline_stage]._cdata;
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
  return 0;
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
  return 0;
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
  TAU_PROFILE("PT(CycleData) PipelineCyclerTrueImpl::cycle_2()", " ", TAU_USER);
  PT(CycleData) last_val = _data[1]._cdata.p();
  nassertr(_lock.debug_is_locked(), last_val);
  nassertr(_dirty, last_val);
  nassertr(_num_stages == 2, last_val);

  nassertr(_data[1]._writes_outstanding == 0, last_val);
  _data[1]._cdata = _data[0]._cdata;

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
  TAU_PROFILE("PT(CycleData) PipelineCyclerTrueImpl::cycle_3()", " ", TAU_USER);
  PT(CycleData) last_val = _data[2]._cdata.p();
  nassertr(_lock.debug_is_locked(), last_val);
  nassertr(_dirty, last_val);
  nassertr(_num_stages == 3, last_val);

  nassertr(_data[2]._writes_outstanding == 0, last_val);
  nassertr(_data[1]._writes_outstanding == 0, last_val);
  _data[2]._cdata = _data[1]._cdata;
  _data[1]._cdata = _data[0]._cdata;

  if (_data[2]._cdata == _data[1]._cdata) {
    // No longer dirty.
    _dirty = false;
  }

  return last_val;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::CyclerMutex::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipelineCyclerTrueImpl::CyclerMutex::
CyclerMutex(PipelineCyclerTrueImpl *cycler) {
#ifdef DEBUG_THREADS
  _cycler = cycler;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::CyclerMutex::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipelineCyclerTrueImpl::CycleDataNode::
CycleDataNode() :
  _writes_outstanding(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::CyclerMutex::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipelineCyclerTrueImpl::CycleDataNode::
CycleDataNode(const PipelineCyclerTrueImpl::CycleDataNode &copy) :
  _cdata(copy._cdata),
  _writes_outstanding(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::CyclerMutex::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipelineCyclerTrueImpl::CycleDataNode::
~CycleDataNode() {
  nassertv(_writes_outstanding == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::CyclerMutex::Copy Assignment
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrueImpl::CycleDataNode::
operator = (const PipelineCyclerTrueImpl::CycleDataNode &copy) {
  _cdata = copy._cdata;
}
