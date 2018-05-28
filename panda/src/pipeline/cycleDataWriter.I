/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cycleDataWriter.I
 * @author drose
 * @date 2002-02-21
 */

#ifndef CPPPARSER

#ifdef DO_PIPELINING
// This is the implementation for full support of pipelining (as well as the
// sanity-check only implementation).

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &cycler, Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread)
{
  _pointer = _cycler->write(_current_thread);
  nassertv(_pointer != nullptr);
}

/**
 * This two-parameter constructor, with a bool parameter for the second
 * parameter, automatically propagates the CycleData pointer upstream from the
 * current stage, either stopping at the first pointer encountered that's
 * different, going or all the way to stage 0, according to force_to_0.  See
 * PipelineCycler::write_upstream().
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &cycler, bool force_to_0,
                Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread)
{
  _pointer = _cycler->write_upstream(force_to_0, _current_thread);
  nassertv(_pointer != nullptr);
}

/**
 * This special constructor "steals" a reference count from the already-locked
 * cdata object.  It does not increment the lock count of this object, but
 * will release it when the destructor is called.
 *
 * This is designed for special functions that return an already-locked cdata
 * object and expect the caller to unlock it.
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &cycler, CycleDataType *locked_cdata,
                Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread)
{
  _pointer = locked_cdata;
  nassertv(_pointer != nullptr);
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(const CycleDataWriter<CycleDataType> &copy) :
  _cycler(copy._cycler),
  _current_thread(copy._current_thread),
  _pointer(copy._pointer)
{
  nassertv(_pointer != nullptr);
  _cycler->increment_write(_pointer);
}

/**
 *
 */
template<class CycleDataType>
INLINE void CycleDataWriter<CycleDataType>::
operator = (const CycleDataWriter<CycleDataType> &copy) {
  nassertv(_pointer == nullptr);
  nassertv(_current_thread == copy._current_thread);

  _cycler = copy._cycler;
  _pointer = copy._pointer;

  nassertv(_pointer != nullptr);
  _cycler->increment_write(_pointer);
}

/**
 * This flavor of the constructor elevates the pointer from the
 * CycleDataLockedReader from a read to a write pointer (and invalidates the
 * reader).
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &cycler,
                CycleDataLockedReader<CycleDataType> &take_from) :
  _cycler(&cycler),
  _current_thread(take_from.get_current_thread())
{
  _pointer = _cycler->elevate_read(take_from.take_pointer(), _current_thread);
}

/**
 * This flavor of the constructor elevates the pointer from the
 * CycleDataLockedReader from a read to a write pointer (and invalidates the
 * reader).  It also propagates the pointer back upstream; see
 * PipelineCycler::write_upstream().
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &cycler,
                CycleDataLockedReader<CycleDataType> &take_from,
                bool force_to_0) :
  _cycler(&cycler),
  _current_thread(take_from.get_current_thread())
{
  _pointer = _cycler->elevate_read_upstream(take_from.take_pointer(),
                                            force_to_0, _current_thread);
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(CycleDataWriter<CycleDataType> &&from) noexcept :
  _cycler(from._cycler),
  _current_thread(from._current_thread),
  _pointer(from._pointer)
{
  from._pointer = nullptr;
}

/**
 *
 */
template<class CycleDataType>
INLINE void CycleDataWriter<CycleDataType>::
operator = (CycleDataWriter<CycleDataType> &&from) noexcept {
  nassertv(_pointer == nullptr);
  nassertv(_current_thread == from._current_thread);

  _cycler = from._cycler;
  _pointer = from._pointer;

  from._pointer = nullptr;
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
~CycleDataWriter() {
  if (_pointer != nullptr) {
    _cycler->release_write(_pointer);
  }
}

/**
 * This provides an indirect member access to the actual CycleData data.
 */
template<class CycleDataType>
INLINE CycleDataType *CycleDataWriter<CycleDataType>::
operator -> () {
  nassertr(_pointer != nullptr, _cycler->cheat());
  return _pointer;
}

/**
 * This provides an indirect member access to the actual CycleData data.
 */
template<class CycleDataType>
INLINE const CycleDataType *CycleDataWriter<CycleDataType>::
operator -> () const {
  nassertr(_pointer != nullptr, _cycler->cheat());
  return _pointer;
}

/**
 * This allows the CycleDataWriter to be passed to any function that expects a
 * CycleDataType pointer.
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
operator CycleDataType * () {
  nassertr(_pointer != nullptr, _cycler->cheat());
  return _pointer;
}

/**
 * Returns the Thread pointer of the currently-executing thread, as passed to
 * the constructor of this object.
 */
template<class CycleDataType>
INLINE Thread *CycleDataWriter<CycleDataType>::
get_current_thread() const {
  return _current_thread;
}

#else  // !DO_PIPELINING
// This is the trivial, do-nothing implementation.

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &cycler, Thread *) {
  _pointer = cycler.cheat();
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &cycler, bool, Thread *) {
  _pointer = cycler.cheat();
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &, CycleDataType *locked_cdata,
                Thread *) {
  _pointer = locked_cdata;
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(const CycleDataWriter<CycleDataType> &copy) :
  _pointer(copy._pointer)
{
}

/**
 *
 */
template<class CycleDataType>
INLINE void CycleDataWriter<CycleDataType>::
operator = (const CycleDataWriter<CycleDataType> &copy) {
  _pointer = copy._pointer;
}

/**
 * This flavor of the constructor elevates the pointer from the
 * CycleDataLockedReader from a read to a write pointer (and invalidates the
 * reader).
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &,
                CycleDataLockedReader<CycleDataType> &take_from) :
  _pointer((CycleDataType *)take_from.take_pointer())
{
}

/**
 * This flavor of the constructor elevates the pointer from the
 * CycleDataLockedReader from a read to a write pointer (and invalidates the
 * reader).  It also propagates the pointer back upstream; see
 * PipelineCycler::write_upstream().
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
CycleDataWriter(PipelineCycler<CycleDataType> &,
                CycleDataLockedReader<CycleDataType> &take_from,
        bool force_to_0) :
  _pointer((CycleDataType *)take_from.take_pointer())
{
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
~CycleDataWriter() {
}

/**
 * This provides an indirect member access to the actual CycleData data.
 */
template<class CycleDataType>
INLINE CycleDataType *CycleDataWriter<CycleDataType>::
operator -> () {
  return _pointer;
}

/**
 * This provides an indirect member access to the actual CycleData data.
 */
template<class CycleDataType>
INLINE const CycleDataType *CycleDataWriter<CycleDataType>::
operator -> () const {
  return _pointer;
}

/**
 * This allows the CycleDataWriter to be passed to any function that expects a
 * CycleDataType pointer.
 */
template<class CycleDataType>
INLINE CycleDataWriter<CycleDataType>::
operator CycleDataType * () {
  return _pointer;
}

/**
 * Returns the Thread pointer of the currently-executing thread, as passed to
 * the constructor of this object.
 */
template<class CycleDataType>
INLINE Thread *CycleDataWriter<CycleDataType>::
get_current_thread() const {
  return Thread::get_current_thread();
}

#endif  // DO_PIPELINING
#endif  // CPPPARSER
