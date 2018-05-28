/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipelineCyclerDummyImpl.I
 * @author drose
 * @date 2006-01-31
 */

/**
 *
 */
INLINE PipelineCyclerDummyImpl::
PipelineCyclerDummyImpl(CycleData *initial_data, Pipeline *pipeline) :
  _data(initial_data),
  _pipeline(pipeline),
  _read_count(0),
  _write_count(0),
  _locked(false)
{
  if (_pipeline == nullptr) {
    _pipeline = Pipeline::get_render_pipeline();
  }
}

/**
 *
 */
INLINE PipelineCyclerDummyImpl::
PipelineCyclerDummyImpl(const PipelineCyclerDummyImpl &copy) :
  _data(copy._data->make_copy()),
  _pipeline(copy._pipeline),
  _read_count(0),
  _write_count(0),
  _locked(false)
{
}

/**
 *
 */
INLINE void PipelineCyclerDummyImpl::
operator = (const PipelineCyclerDummyImpl &copy) {
  nassertv(_read_count == 0 && _write_count == 0);
  _data = copy._data->make_copy();
  _pipeline = copy._pipeline;
}

/**
 *
 */
INLINE PipelineCyclerDummyImpl::
~PipelineCyclerDummyImpl() {
  nassertv(_read_count == 0 && _write_count == 0 && !_locked);
}

/**
 * Grabs an overall lock on the cycler.  Release it with a call to release().
 * This lock should be held while walking the list of stages.
 */
INLINE void PipelineCyclerDummyImpl::
acquire(Thread *) {
  TAU_PROFILE("void PipelineCyclerDummyImpl::acquire(Thread *)", " ", TAU_USER);
  nassertv(!_locked);
  _locked = true;
}

/**
 * Release the overall lock on the cycler that was grabbed via acquire().
 */
INLINE void PipelineCyclerDummyImpl::
release() {
  TAU_PROFILE("void PipelineCyclerDummyImpl::release()", " ", TAU_USER);
  nassertv(_locked);
  _locked = false;
}

/**
 * Returns a const CycleData pointer, filled with the data for the current
 * stage of the pipeline as seen by this thread.  No lock is made on the
 * contents; there is no guarantee that some other thread won't modify this
 * object's data while you are working on it.  (However, the data within the
 * returned CycleData object itself is safe from modification; if another
 * thread modifies the data, it will perform a copy-on-write, and thereby
 * change the pointer stored within the object.)
 */
INLINE const CycleData *PipelineCyclerDummyImpl::
read_unlocked(Thread *current_thread) const {
  TAU_PROFILE("const CycleData *PipelineCyclerDummyImpl::read_unlocked()", " ", TAU_USER);
  return _data;
}

/**
 * Returns a const CycleData pointer, filled with the data for the current
 * stage of the pipeline as seen by this thread.  This pointer should
 * eventually be released by calling release_read().
 *
 * There should be no outstanding write pointers on the data when this
 * function is called.
 */
INLINE const CycleData *PipelineCyclerDummyImpl::
read(Thread *) const {
  TAU_PROFILE("const CycleData *PipelineCyclerDummyImpl::read()", " ", TAU_USER);
  // This function isn't truly const, but it doesn't change the data in any
  // meaningful way, so we pretend it is.
  ((PipelineCyclerDummyImpl *)this)->_read_count++;

  // It's not an error to grab a read pointer while someone else holds a read
  // or a write pointer.
  return _data;
}

/**
 * Increments the count on a pointer previously retrieved by read(); now the
 * pointer will need to be released twice.
 */
INLINE void PipelineCyclerDummyImpl::
increment_read(const CycleData *pointer) const {
  TAU_PROFILE("void PipelineCyclerDummyImpl::increment_read(const CycleData *)", " ", TAU_USER);
  // This function isn't truly const, but it doesn't change the data in any
  // meaningful way, so we pretend it is.
  nassertv(pointer == _data);
  nassertv(_read_count > 0);
  ((PipelineCyclerDummyImpl *)this)->_read_count++;
}

/**
 * Releases a pointer previously obtained via a call to read().
 */
INLINE void PipelineCyclerDummyImpl::
release_read(const CycleData *pointer) const {
  TAU_PROFILE("void PipelineCyclerDummyImpl::release_read(const CycleData *)", " ", TAU_USER);
  // This function isn't truly const, but it doesn't change the data in any
  // meaningful way, so we pretend it is.
  nassertv(pointer == _data);
  nassertv(_read_count > 0);
  ((PipelineCyclerDummyImpl *)this)->_read_count--;
}

/**
 * Returns a non-const CycleData pointer, filled with a unique copy of the
 * data for the current stage of the pipeline as seen by this thread.  This
 * pointer may now be used to write to the data, and that copy of the data
 * will be propagated to all later stages of the pipeline.  This pointer
 * should eventually be released by calling release_write().
 *
 * There may only be one outstanding write pointer on a given stage at a time,
 * and if there is a write pointer there may be no read pointers on the same
 * stage (but see elevate_read).
 */
INLINE CycleData *PipelineCyclerDummyImpl::
write(Thread *current_thread) {
  TAU_PROFILE("CycleData *PipelineCyclerDummyImpl::write()", " ", TAU_USER);
  _write_count++;

  // It's an error to grab a write pointer while someone else holds a read
  // pointer, because doing so may invalidate the read pointer.
  nassertr(_read_count == 0, _data);

  // It's not an error to do this while someone else holds a write pointer,
  // however.

  return _data;
}

/**
 * This special variant on write() will automatically propagate changes back
 * to upstream pipeline stages.  If force_to_0 is false, then it propagates
 * back only as long as the CycleData pointers are equivalent, guaranteeing
 * that it does not modify upstream data (other than the modification that
 * will be performed by the code that returns this pointer).  This is
 * particularly appropriate for minor updates, where it doesn't matter much if
 * the update is lost, such as storing a cached value.
 *
 * If force_to_0 is dummy, then the CycleData pointer for the current pipeline
 * stage is propagated all the way back up to stage 0; after this call, there
 * will be only one CycleData pointer that is duplicated in all stages between
 * stage 0 and the current stage.  This may undo some recent changes that were
 * made independently at pipeline stage 0 (or any other upstream stage).
 * However, it guarantees that the change that is to be applied at this
 * pipeline stage will stick.  This is slightly dangerous because of the risk
 * of losing upstream changes; generally, this should only be done when you
 * are confident that there are no upstream changes to be lost (for instance,
 * for an object that has been recently created).
 */
CycleData *PipelineCyclerDummyImpl::
write_upstream(bool, Thread *) {
  TAU_PROFILE("CycleData *PipelineCyclerDummyImpl::write_upstream(bool)", " ", TAU_USER);
  _write_count++;
  return _data;
}

/**
 * Elevates a currently-held read pointer into a write pointer.  This may or
 * may not change the value of the pointer.  It is only valid to do this if
 * this is the only currently-outstanding read pointer on the current stage.
 */
INLINE CycleData *PipelineCyclerDummyImpl::
elevate_read(const CycleData *pointer, Thread *current_thread) {
  TAU_PROFILE("CycleData *PipelineCyclerDummyImpl::elevate_read(const CycleData *)", " ", TAU_USER);
  release_read(pointer);
  return write(current_thread);
}

/**
 * Elevates a currently-held read pointer into a write pointer, like
 * elevate_read(), but also propagates the pointer back to upstream stages,
 * like write_upstream().
 */
INLINE CycleData *PipelineCyclerDummyImpl::
elevate_read_upstream(const CycleData *pointer, bool force_to_0, Thread *current_thread) {
  TAU_PROFILE("CycleData *PipelineCyclerDummyImpl::elevate_read_upstream(const CycleData *, bool)", " ", TAU_USER);
  release_read(pointer);
  return write_upstream(force_to_0, current_thread);
}

/**
 * Increments the count on a pointer previously retrieved by write(); now the
 * pointer will need to be released twice.
 */
INLINE void PipelineCyclerDummyImpl::
increment_write(CycleData *pointer) const {
  TAU_PROFILE("void PipelineCyclerDummyImpl::increment_write(CycleData *)", " ", TAU_USER);
  // This function isn't truly const, but it doesn't change the data in any
  // meaningful way, so we pretend it is.
  nassertv(pointer == _data);
  nassertv(_write_count > 0);
  ((PipelineCyclerDummyImpl *)this)->_write_count++;
}

/**
 * Releases a pointer previously obtained via a call to write().
 */
INLINE void PipelineCyclerDummyImpl::
release_write(CycleData *pointer) {
  TAU_PROFILE("void PipelineCyclerDummyImpl::release_write(CycleData *)", " ", TAU_USER);
  nassertv(pointer == _data);
  nassertv(_write_count > 0);
  _write_count--;
}

/**
 * Returns the number of stages in the pipeline.
 */
INLINE int PipelineCyclerDummyImpl::
get_num_stages() {
  return 1;
}

/**
 * Returns a const CycleData pointer, filled with the data for the indicated
 * stage of the pipeline.  As in read_unlocked(), no lock is held on the
 * returned pointer.
 */
INLINE const CycleData *PipelineCyclerDummyImpl::
read_stage_unlocked(int pipeline_stage) const {
  TAU_PROFILE("const CycleData *PipelineCyclerDummyImpl::read_stage_unlocked(int)", " ", TAU_USER);
  nassertr(pipeline_stage == 0, nullptr);
  return _data;
}

/**
 * Returns a const CycleData pointer, filled with the data for the indicated
 * stage of the pipeline.  This pointer should eventually be released by
 * calling release_read_stage().
 *
 * There should be no outstanding write pointers on the data when this
 * function is called.
 */
INLINE const CycleData *PipelineCyclerDummyImpl::
read_stage(int pipeline_stage, Thread *) const {
  TAU_PROFILE("const CycleData *PipelineCyclerDummyImpl::read_stage(int, Thread *)", " ", TAU_USER);
  // This function isn't truly const, but it doesn't change the data in any
  // meaningful way, so we pretend it is.
  nassertr(pipeline_stage == 0, nullptr);
  ((PipelineCyclerDummyImpl *)this)->_read_count++;

  // It's not an error to grab a read pointer while someone else holds a read
  // or a write pointer.
  return _data;
}

/**
 * Releases a pointer previously obtained via a call to read_stage().
 */
INLINE void PipelineCyclerDummyImpl::
release_read_stage(int pipeline_stage, const CycleData *pointer) const {
  TAU_PROFILE("void PipelineCyclerDummyImpl::release_read_stage(int, const CycleData *)", " ", TAU_USER);
  // This function isn't truly const, but it doesn't change the data in any
  // meaningful way, so we pretend it is.
  nassertv(pipeline_stage == 0);
  nassertv(pointer == _data);
  nassertv(_read_count > 0);
  ((PipelineCyclerDummyImpl *)this)->_read_count--;
}

/**
 * Returns a pointer suitable for writing to the nth stage of the pipeline.
 * This is for special applications that need to update the entire pipeline at
 * once (for instance, to remove an invalid pointer). This pointer should
 * later be released with release_write_stage().
 */
INLINE CycleData *PipelineCyclerDummyImpl::
write_stage(int pipeline_stage, Thread *) {
  TAU_PROFILE("CycleData *PipelineCyclerDummyImpl::write_stage(int)", " ", TAU_USER);
  nassertr(pipeline_stage == 0, nullptr);
  _write_count++;
  return _data;
}

/**
 * Returns a pointer suitable for writing to the nth stage of the pipeline.
 * This is for special applications that need to update the entire pipeline at
 * once (for instance, to remove an invalid pointer). This pointer should
 * later be released with release_write_stage().
 */
INLINE CycleData *PipelineCyclerDummyImpl::
write_stage_upstream(int pipeline_stage, bool, Thread *) {
  TAU_PROFILE("CycleData *PipelineCyclerDummyImpl::write_stage_upstream(int)", " ", TAU_USER);
  nassertr(pipeline_stage == 0, nullptr);
  _write_count++;
  return _data;
}

/**
 * Elevates a currently-held read pointer into a write pointer.  This may or
 * may not change the value of the pointer.  It is only valid to do this if
 * this is the only currently-outstanding read pointer on the current stage.
 */
INLINE CycleData *PipelineCyclerDummyImpl::
elevate_read_stage(int pipeline_stage, const CycleData *pointer, Thread *current_thread) {
  TAU_PROFILE("CycleData *PipelineCyclerDummyImpl::elevate_read_stage(int, CycleData *)", " ", TAU_USER);
  nassertr(pipeline_stage == 0, nullptr);
  release_read(pointer);
  return write(current_thread);
}

/**
 * Elevates a currently-held read pointer into a write pointer.  This may or
 * may not change the value of the pointer.  It is only valid to do this if
 * this is the only currently-outstanding read pointer on the current stage.
 */
INLINE CycleData *PipelineCyclerDummyImpl::
elevate_read_stage_upstream(int pipeline_stage, const CycleData *pointer,
                            bool, Thread *current_thread) {
  TAU_PROFILE("CycleData *PipelineCyclerDummyImpl::elevate_read_stage(int, CycleData *)", " ", TAU_USER);
  nassertr(pipeline_stage == 0, nullptr);
  release_read(pointer);
  return write(current_thread);
}

/**
 * Releases a pointer previously obtained via a call to write_stage().
 */
INLINE void PipelineCyclerDummyImpl::
release_write_stage(int pipeline_stage, CycleData *pointer) {
  TAU_PROFILE("void PipelineCyclerDummyImpl::release_write_stage(int, CycleData *)", " ", TAU_USER);
  nassertv(pipeline_stage == 0 && pointer == _data);
  nassertv(_write_count > 0);
  _write_count--;
}

/**
 * Returns the type of object that owns this cycler, as reported by
 * CycleData::get_parent_type().
 */
INLINE TypeHandle PipelineCyclerDummyImpl::
get_parent_type() const {
  return _data->get_parent_type();
}

/**
 * Returns a pointer without counting it.  This is only intended for use as
 * the return value for certain nassertr() functions, so the application can
 * recover after a failure to manage the read and write pointers correctly.
 * You should never call this function directly.
 */
INLINE CycleData *PipelineCyclerDummyImpl::
cheat() const {
  return _data;
}

/**
 * Returns the number of handles currently outstanding to read the current
 * stage of the data.  This should only be used for debugging purposes.
 */
INLINE int PipelineCyclerDummyImpl::
get_read_count() const {
  return _read_count;
}

/**
 * Returns the number of handles currently outstanding to read the current
 * stage of the data.  This will normally only be either 0 or 1.  This should
 * only be used for debugging purposes.
 */
INLINE int PipelineCyclerDummyImpl::
get_write_count() const {
  return _write_count;
}
