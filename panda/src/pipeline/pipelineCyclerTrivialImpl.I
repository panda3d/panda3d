/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipelineCyclerTrivialImpl.I
 * @author drose
 * @date 2006-01-31
 */

/**
 *
 */
INLINE PipelineCyclerTrivialImpl::
PipelineCyclerTrivialImpl(CycleData *initial_data, Pipeline *) {
  // In the trivial implementation, a derived class (the PipelineCycler
  // template class) stores the CycleData object directly within itself, and
  // since we have no data members or virtual functions, we get away with
  // assuming the pointer is the same as the 'this' pointer.

  // If this turns out not to be true on a particular platform, we will have
  // to store the pointer in this class, for a little bit of extra overhead.
#ifdef SIMPLE_STRUCT_POINTERS
  nassertv(initial_data == (CycleData *)this);
#else
  _data = initial_data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Grabs an overall lock on the cycler.  Release it with a call to release().
 * This lock should be held while walking the list of stages.
 */
INLINE void PipelineCyclerTrivialImpl::
acquire(Thread *) {
}

/**
 * Release the overall lock on the cycler that was grabbed via acquire().
 */
INLINE void PipelineCyclerTrivialImpl::
release() {
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
INLINE const CycleData *PipelineCyclerTrivialImpl::
read_unlocked(Thread *) const {
#ifdef SIMPLE_STRUCT_POINTERS
  return (const CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Returns a const CycleData pointer, filled with the data for the current
 * stage of the pipeline as seen by this thread.  This pointer should
 * eventually be released by calling release_read().
 */
INLINE const CycleData *PipelineCyclerTrivialImpl::
read(Thread *) const {
#ifdef SIMPLE_STRUCT_POINTERS
  return (const CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Increments the count on a pointer previously retrieved by read(); now the
 * pointer will need to be released twice.
 */
INLINE void PipelineCyclerTrivialImpl::
increment_read(const CycleData *) const {
}

/**
 * Releases a pointer previously obtained via a call to read().
 */
INLINE void PipelineCyclerTrivialImpl::
release_read(const CycleData *) const {
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
INLINE CycleData *PipelineCyclerTrivialImpl::
write(Thread *) {
#ifdef SIMPLE_STRUCT_POINTERS
  return (CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
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
 * If force_to_0 is trivial, then the CycleData pointer for the current
 * pipeline stage is propagated all the way back up to stage 0; after this
 * call, there will be only one CycleData pointer that is duplicated in all
 * stages between stage 0 and the current stage.  This may undo some recent
 * changes that were made independently at pipeline stage 0 (or any other
 * upstream stage).  However, it guarantees that the change that is to be
 * applied at this pipeline stage will stick.  This is slightly dangerous
 * because of the risk of losing upstream changes; generally, this should only
 * be done when you are confident that there are no upstream changes to be
 * lost (for instance, for an object that has been recently created).
 */
CycleData *PipelineCyclerTrivialImpl::
write_upstream(bool, Thread *) {
#ifdef SIMPLE_STRUCT_POINTERS
  return (CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Elevates a currently-held read pointer into a write pointer.  This may or
 * may not change the value of the pointer.  It is only valid to do this if
 * this is the only currently-outstanding read pointer on the current stage.
 */
INLINE CycleData *PipelineCyclerTrivialImpl::
elevate_read(const CycleData *, Thread *) {
#ifdef SIMPLE_STRUCT_POINTERS
  return (CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Elevates a currently-held read pointer into a write pointer, like
 * elevate_read(), but also propagates the pointer back to upstream stages,
 * like write_upstream().
 */
INLINE CycleData *PipelineCyclerTrivialImpl::
elevate_read_upstream(const CycleData *, bool, Thread *) {
#ifdef SIMPLE_STRUCT_POINTERS
  return (CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Increments the count on a pointer previously retrieved by write(); now the
 * pointer will need to be released twice.
 */
INLINE void PipelineCyclerTrivialImpl::
increment_write(CycleData *) const {
}

/**
 * Releases a pointer previously obtained via a call to write().
 */
INLINE void PipelineCyclerTrivialImpl::
release_write(CycleData *) {
}

/**
 * Returns the number of stages in the pipeline.
 */
INLINE int PipelineCyclerTrivialImpl::
get_num_stages() {
  return 1;
}

/**
 * Returns a const CycleData pointer, filled with the data for the indicated
 * stage of the pipeline.  As in read_unlocked(), no lock is held on the
 * returned pointer.
 */
INLINE const CycleData *PipelineCyclerTrivialImpl::
read_stage_unlocked(int) const {
#ifdef SIMPLE_STRUCT_POINTERS
  return (const CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Returns a const CycleData pointer, filled with the data for the indicated
 * pipeline stage.  This pointer should eventually be released by calling
 * release_read().
 */
INLINE const CycleData *PipelineCyclerTrivialImpl::
read_stage(int, Thread *) const {
#ifdef SIMPLE_STRUCT_POINTERS
  return (const CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Releases a pointer previously obtained via a call to read_stage().
 */
INLINE void PipelineCyclerTrivialImpl::
release_read_stage(int, const CycleData *) const {
}

/**
 * Returns a pointer suitable for writing to the nth stage of the pipeline.
 * This is for special applications that need to update the entire pipeline at
 * once (for instance, to remove an invalid pointer). This pointer should
 * later be released with release_write_stage().
 */
INLINE CycleData *PipelineCyclerTrivialImpl::
write_stage(int, Thread *) {
#ifdef SIMPLE_STRUCT_POINTERS
  return (CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Returns a pointer suitable for writing to the nth stage of the pipeline.
 * This is for special applications that need to update the entire pipeline at
 * once (for instance, to remove an invalid pointer). This pointer should
 * later be released with release_write_stage().
 */
INLINE CycleData *PipelineCyclerTrivialImpl::
write_stage_upstream(int, bool, Thread *) {
#ifdef SIMPLE_STRUCT_POINTERS
  return (CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Elevates a currently-held read pointer into a write pointer.  This may or
 * may not change the value of the pointer.  It is only valid to do this if
 * this is the only currently-outstanding read pointer on the current stage.
 */
INLINE CycleData *PipelineCyclerTrivialImpl::
elevate_read_stage(int, const CycleData *, Thread *) {
#ifdef SIMPLE_STRUCT_POINTERS
  return (CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Elevates a currently-held read pointer into a write pointer.  This may or
 * may not change the value of the pointer.  It is only valid to do this if
 * this is the only currently-outstanding read pointer on the current stage.
 */
INLINE CycleData *PipelineCyclerTrivialImpl::
elevate_read_stage_upstream(int, const CycleData *, bool, Thread *) {
#ifdef SIMPLE_STRUCT_POINTERS
  return (CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Releases a pointer previously obtained via a call to write_stage().
 */
INLINE void PipelineCyclerTrivialImpl::
release_write_stage(int, CycleData *) {
}

/**
 * Returns the type of object that owns this cycler, as reported by
 * CycleData::get_parent_type().
 */
INLINE TypeHandle PipelineCyclerTrivialImpl::
get_parent_type() const {
  return cheat()->get_parent_type();
}

/**
 * Returns a pointer without counting it.  This is only intended for use as
 * the return value for certain nassertr() functions, so the application can
 * recover after a failure to manage the read and write pointers correctly.
 * You should never call this function directly.
 */
INLINE CycleData *PipelineCyclerTrivialImpl::
cheat() const {
#ifdef SIMPLE_STRUCT_POINTERS
  return (CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

/**
 * Returns the number of handles currently outstanding to read the current
 * stage of the data.  This should only be used for debugging purposes.
 */
INLINE int PipelineCyclerTrivialImpl::
get_read_count() const {
  return 0;
}

/**
 * Returns the number of handles currently outstanding to read the current
 * stage of the data.  This will normally only be either 0 or 1.  This should
 * only be used for debugging purposes.
 */
INLINE int PipelineCyclerTrivialImpl::
get_write_count() const {
  return 0;
}
