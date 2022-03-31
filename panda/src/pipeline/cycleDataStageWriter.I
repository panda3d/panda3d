/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cycleDataStageWriter.I
 * @author drose
 * @date 2006-02-06
 */

#ifndef CPPPARSER

#ifdef DO_PIPELINING
// This is the implementation for full support of pipelining (as well as the
// sanity-check only implementation).

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                     Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread),
  _stage(stage)
{
  _pointer = _cycler->write_stage(_stage, _current_thread);
  nassertv(_pointer != nullptr);
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                     bool force_to_0, Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread),
  _stage(stage)
{
  _pointer = _cycler->write_stage_upstream(_stage, force_to_0, _current_thread);
  nassertv(_pointer != nullptr);
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(const CycleDataStageWriter<CycleDataType> &copy) :
  _cycler(copy._cycler),
  _current_thread(copy._current_thread),
  _pointer(copy._pointer),
  _stage(copy._stage)
{
  nassertv(_pointer != nullptr);
  _cycler->increment_write(_pointer);
}

/**
 * This flavor of the constructor elevates the pointer from the
 * CycleDataLockedStageReader from a read to a write pointer (and invalidates
 * the reader).
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                     CycleDataLockedStageReader<CycleDataType> &take_from) :
  _cycler(&cycler),
  _current_thread(take_from.get_current_thread()),
  _stage(stage)
{
  _pointer = _cycler->elevate_read_stage(_stage, take_from.take_pointer(),
                                         _current_thread);
}

/**
 * This flavor of the constructor elevates the pointer from the
 * CycleDataLockedStageReader from a read to a write pointer (and invalidates
 * the reader).
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int stage,
                     CycleDataLockedStageReader<CycleDataType> &take_from,
                     bool force_to_0) :
  _cycler(&cycler),
  _current_thread(take_from.get_current_thread()),
  _stage(stage)
{
  _pointer = _cycler->elevate_read_stage_upstream(_stage, take_from.take_pointer(),
                                                  force_to_0, _current_thread);
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(CycleDataStageWriter<CycleDataType> &&from) noexcept :
  _cycler(from._cycler),
  _current_thread(from._current_thread),
  _pointer(from._pointer),
  _stage(from._stage)
{
  from._pointer = nullptr;
}

/**
 *
 */
template<class CycleDataType>
INLINE void CycleDataStageWriter<CycleDataType>::
operator = (const CycleDataStageWriter<CycleDataType> &copy) {
  nassertv(_pointer == nullptr);
  nassertv(_current_thread == copy._current_thread);

  _cycler = copy._cycler;
  _pointer = copy._pointer;
  _stage = copy._stage;

  nassertv(_pointer != nullptr);
  _cycler->increment_write(_pointer);
}

/**
 *
 */
template<class CycleDataType>
INLINE void CycleDataStageWriter<CycleDataType>::
operator = (CycleDataStageWriter<CycleDataType> &&from) noexcept {
  nassertv(_pointer == nullptr);
  nassertv(_current_thread == from._current_thread);

  _cycler = from._cycler;
  _pointer = from._pointer;
  _stage = from._stage;

  from._pointer = nullptr;
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
~CycleDataStageWriter() {
  if (_pointer != nullptr) {
    _cycler->release_write_stage(_stage, _pointer);
  }
}

/**
 * This provides an indirect member access to the actual CycleData data.
 */
template<class CycleDataType>
INLINE CycleDataType *CycleDataStageWriter<CycleDataType>::
operator -> () {
  nassertr(_pointer != nullptr, _cycler->cheat());
  return _pointer;
}

/**
 * This provides an indirect member access to the actual CycleData data.
 */
template<class CycleDataType>
INLINE const CycleDataType *CycleDataStageWriter<CycleDataType>::
operator -> () const {
  nassertr(_pointer != nullptr, _cycler->cheat());
  return _pointer;
}

/**
 * This allows the CycleDataStageWriter to be passed to any function that
 * expects a CycleDataType pointer.
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
operator CycleDataType * () {
  nassertr(_pointer != nullptr, _cycler->cheat());
  return _pointer;
}

/**
 * Returns the Thread pointer of the currently-executing thread, as passed to
 * the constructor of this object.
 */
template<class CycleDataType>
INLINE Thread *CycleDataStageWriter<CycleDataType>::
get_current_thread() const {
  return _current_thread;
}

#else  // !DO_PIPELINING
// This is the trivial, do-nothing implementation.

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int, Thread *) {
  _pointer = cycler.cheat();
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &cycler, int, bool, Thread *) {
  _pointer = cycler.cheat();
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(const CycleDataStageWriter<CycleDataType> &copy) :
  _pointer(copy._pointer)
{
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(CycleDataStageWriter<CycleDataType> &&from) noexcept :
  _pointer(from._pointer)
{
  from._pointer = nullptr;
}

/**
 *
 */
template<class CycleDataType>
INLINE void CycleDataStageWriter<CycleDataType>::
operator = (const CycleDataStageWriter<CycleDataType> &copy) {
  _pointer = copy._pointer;
}

/**
 *
 */
template<class CycleDataType>
INLINE void CycleDataStageWriter<CycleDataType>::
operator = (CycleDataStageWriter<CycleDataType> &&from) noexcept {
  nassertv(_pointer == nullptr);

  _pointer = from._pointer;
  from._pointer = nullptr;
}

/**
 * This flavor of the constructor elevates the pointer from the
 * CycleDataLockedStageReader from a read to a write pointer (and invalidates
 * the reader).
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &, int,
                     CycleDataLockedStageReader<CycleDataType> &take_from) :
  _pointer((CycleDataType *)take_from.take_pointer())
{
}

/**
 * This flavor of the constructor elevates the pointer from the
 * CycleDataLockedStageReader from a read to a write pointer (and invalidates
 * the reader).
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
CycleDataStageWriter(PipelineCycler<CycleDataType> &, int,
                     CycleDataLockedStageReader<CycleDataType> &take_from,
                     bool) :
  _pointer((CycleDataType *)take_from.take_pointer())
{
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
~CycleDataStageWriter() {
}

/**
 * This provides an indirect member access to the actual CycleData data.
 */
template<class CycleDataType>
INLINE CycleDataType *CycleDataStageWriter<CycleDataType>::
operator -> () {
  return _pointer;
}

/**
 * This provides an indirect member access to the actual CycleData data.
 */
template<class CycleDataType>
INLINE const CycleDataType *CycleDataStageWriter<CycleDataType>::
operator -> () const {
  return _pointer;
}

/**
 * This allows the CycleDataStageWriter to be passed to any function that
 * expects a CycleDataType pointer.
 */
template<class CycleDataType>
INLINE CycleDataStageWriter<CycleDataType>::
operator CycleDataType * () {
  return _pointer;
}

/**
 * Returns the Thread pointer of the currently-executing thread, as passed to
 * the constructor of this object.
 */
template<class CycleDataType>
INLINE Thread *CycleDataStageWriter<CycleDataType>::
get_current_thread() const {
  return Thread::get_current_thread();
}

#endif  // DO_PIPELINING
#endif  // CPPPARSER
