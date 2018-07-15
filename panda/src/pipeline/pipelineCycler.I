/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipelineCycler.I
 * @author drose
 * @date 2002-02-21
 */

#ifdef DO_PIPELINING
// The following implementations are to support the PipelineCyclerDummyImpl or
// the PipelineCyclerTrueImpl.

/**
 *
 */
template<class CycleDataType>
INLINE PipelineCycler<CycleDataType>::
PipelineCycler(Pipeline *pipeline) :
  PipelineCyclerBase(new CycleDataType, pipeline)
{
}

/**
 *
 */
template<class CycleDataType>
INLINE PipelineCycler<CycleDataType>::
PipelineCycler(CycleDataType &&initial_data, Pipeline *pipeline) :
  PipelineCyclerBase(new CycleDataType(std::move(initial_data)), pipeline)
{
}

/**
 *
 */
template<class CycleDataType>
INLINE PipelineCycler<CycleDataType>::
PipelineCycler(const PipelineCycler<CycleDataType> &copy) :
  PipelineCyclerBase(copy)
{
}

/**
 *
 */
template<class CycleDataType>
INLINE void PipelineCycler<CycleDataType>::
operator = (const PipelineCycler<CycleDataType> &copy) {
  PipelineCyclerBase::operator = (copy);
}

/**
 * See PipelineCyclerBase::read_unlocked().
 */
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read_unlocked(Thread *current_thread) const {
  return (const CycleDataType *)PipelineCyclerBase::read_unlocked(current_thread);
}

/**
 * See PipelineCyclerBase::read().
 */
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read(Thread *current_thread) const {
  return (const CycleDataType *)PipelineCyclerBase::read(current_thread);
}

/**
 * See PipelineCyclerBase::write().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write(Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::write(current_thread);
}

/**
 * See PipelineCyclerBase::write_upstream().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_upstream(bool force_to_0, Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::write_upstream(force_to_0, current_thread);
}

/**
 * See PipelineCyclerBase::elevate_read().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read(const CycleDataType *pointer, Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::elevate_read(pointer, current_thread);
}

/**
 * See PipelineCyclerBase::elevate_read_upstream().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read_upstream(const CycleDataType *pointer, bool force_to_0,
                      Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::elevate_read_upstream(pointer, force_to_0, current_thread);
}

/**
 * See PipelineCyclerBase::read_stage_unlocked().
 */
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read_stage_unlocked(int pipeline_stage) const {
  return (const CycleDataType *)PipelineCyclerBase::read_stage_unlocked(pipeline_stage);
}

/**
 * See PipelineCyclerBase::read_stage().
 */
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read_stage(int pipeline_stage, Thread *current_thread) const {
  return (const CycleDataType *)PipelineCyclerBase::read_stage(pipeline_stage, current_thread);
}

/**
 * See PipelineCyclerBase::elevate_read_stage().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read_stage(int pipeline_stage, const CycleDataType *pointer,
                   Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::elevate_read_stage(pipeline_stage, pointer, current_thread);
}

/**
 * See PipelineCyclerBase::elevate_read_stage_upstream().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read_stage_upstream(int pipeline_stage, const CycleDataType *pointer,
                            bool force_to_0, Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::elevate_read_stage_upstream(pipeline_stage, pointer, force_to_0, current_thread);
}

/**
 * See PipelineCyclerBase::write_stage().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_stage(int pipeline_stage, Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::write_stage(pipeline_stage, current_thread);
}

/**
 * See PipelineCyclerBase::write_stage_upstream().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_stage_upstream(int pipeline_stage, bool force_to_0,
                     Thread *current_thread) {
  return (CycleDataType *)PipelineCyclerBase::write_stage_upstream(pipeline_stage, force_to_0, current_thread);
}

/**
 * Returns a pointer without counting it.  This is only intended for use as
 * the return value for certain nassertr() functions, so the application can
 * recover after a failure to manage the read and write pointers correctly.
 * You should never call this function directly.
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
cheat() const {
  return (CycleDataType *)PipelineCyclerBase::cheat();
}

#else  // !DO_PIPELINING
// The following implementations are provided for when pipelining is not
// compiled in.  They are trivial functions that do as little as possible.

/**
 *
 */
template<class CycleDataType>
INLINE PipelineCycler<CycleDataType>::
PipelineCycler(Pipeline *pipeline) :
  PipelineCyclerBase(&_typed_data, pipeline)
{
}

/**
 *
 */
template<class CycleDataType>
INLINE PipelineCycler<CycleDataType>::
PipelineCycler(CycleDataType &&initial_data, Pipeline *pipeline) :
  _typed_data(std::move(initial_data)),
  PipelineCyclerBase(&_typed_data, pipeline)
{
}

/**
 *
 */
template<class CycleDataType>
INLINE PipelineCycler<CycleDataType>::
PipelineCycler(const PipelineCycler<CycleDataType> &copy) :
  PipelineCyclerBase(&_typed_data),
  _typed_data(copy._typed_data)
{
}

/**
 *
 */
template<class CycleDataType>
INLINE void PipelineCycler<CycleDataType>::
operator = (const PipelineCycler<CycleDataType> &copy) {
  _typed_data = copy._typed_data;
}

/**
 * See PipelineCyclerBase::read_unlocked().
 */
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read_unlocked(Thread *) const {
  return &_typed_data;
}

/**
 * See PipelineCyclerBase::read().
 */
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read(Thread *) const {
  return &_typed_data;
}

/**
 * See PipelineCyclerBase::write().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write(Thread *) {
  return &_typed_data;
}

/**
 * See PipelineCyclerBase::write_upstream().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_upstream(bool, Thread *) {
  return &_typed_data;
}

/**
 * See PipelineCyclerBase::elevate_read().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read(const CycleDataType *, Thread *) {
  return &_typed_data;
}

/**
 * See PipelineCyclerBase::elevate_read_upstream().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read_upstream(const CycleDataType *, bool, Thread *) {
  return &_typed_data;
}

/**
 * See PipelineCyclerBase::read_stage_unlocked().
 */
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read_stage_unlocked(int) const {
  return &_typed_data;
}

/**
 * See PipelineCyclerBase::read_stage().
 */
template<class CycleDataType>
INLINE const CycleDataType *PipelineCycler<CycleDataType>::
read_stage(int, Thread *) const {
  return &_typed_data;
}

/**
 * See PipelineCyclerBase::elevate_read_stage().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read_stage(int, const CycleDataType *, Thread *) {
  return &_typed_data;
}

/**
 * See PipelineCyclerBase::elevate_read_stage_upstream().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
elevate_read_stage_upstream(int, const CycleDataType *, bool, Thread *) {
  return &_typed_data;
}

/**
 * See PipelineCyclerBase::write_stage().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_stage(int, Thread *) {
  return &_typed_data;
}

/**
 * See PipelineCyclerBase::write_stage_upstream().
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
write_stage_upstream(int, bool, Thread *) {
  return &_typed_data;
}

/**
 * Returns a pointer without counting it.  This is only intended for use as
 * the return value for certain nassertr() functions, so the application can
 * recover after a failure to manage the read and write pointers correctly.
 * You should never call this function directly.
 */
template<class CycleDataType>
INLINE CycleDataType *PipelineCycler<CycleDataType>::
cheat() const {
  return (CycleDataType *)&_typed_data;
}


#endif   // DO_PIPELINING
