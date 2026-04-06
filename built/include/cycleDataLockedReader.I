/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cycleDataLockedReader.I
 * @author drose
 * @date 2006-04-30
 */

#ifndef CPPPARSER

#ifdef DO_PIPELINING
// This is the implementation for full support of pipelining (as well as the
// sanity-check only implementation).

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
CycleDataLockedReader(const PipelineCycler<CycleDataType> &cycler,
                      Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread)
{
  _pointer = _cycler->read(_current_thread);
  nassertv(_pointer != nullptr);
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
CycleDataLockedReader(const CycleDataLockedReader<CycleDataType> &copy) :
  _cycler(copy._cycler),
  _current_thread(copy._current_thread),
  _pointer(copy._pointer)
{
  nassertv(_pointer != nullptr);
  _cycler->increment_read(_pointer);
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
CycleDataLockedReader(CycleDataLockedReader<CycleDataType> &&from) noexcept :
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
INLINE void CycleDataLockedReader<CycleDataType>::
operator = (const CycleDataLockedReader<CycleDataType> &copy) {
  nassertv(_pointer == nullptr);
  nassertv(_current_thread == copy._current_thread);

  _cycler = copy._cycler;
  _pointer = copy._pointer;

  nassertv(_pointer != nullptr);
  _cycler->increment_read(_pointer);
}

/**
 *
 */
template<class CycleDataType>
INLINE void CycleDataLockedReader<CycleDataType>::
operator = (CycleDataLockedReader<CycleDataType> &&from) noexcept {
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
INLINE CycleDataLockedReader<CycleDataType>::
~CycleDataLockedReader() {
  if (_pointer != nullptr) {
    _cycler->release_read(_pointer);
  }
}

/**
 * This provides an indirect member access to the actual CycleData data.
 */
template<class CycleDataType>
INLINE const CycleDataType *CycleDataLockedReader<CycleDataType>::
operator -> () const {
  nassertr(_pointer != nullptr, _cycler->cheat());
  return _pointer;
}

/**
 * This allows the CycleDataLockedReader to be passed to any function that
 * expects a const CycleDataType pointer.
 */
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
operator const CycleDataType * () const {
  nassertr(_pointer != nullptr, _cycler->cheat());
  return _pointer;
}

/**
 * This is intended to be called only from CycleDataWriter when it elevates
 * the pointer from read to write status.  This function returns the reader's
 * pointer and relinquishes ownership of the pointer, rendering the reader
 * invalid for future reads.
 */
template<class CycleDataType>
INLINE const CycleDataType *CycleDataLockedReader<CycleDataType>::
take_pointer() {
  const CycleDataType *pointer = _pointer;
  _pointer = nullptr;
  nassertr(pointer != nullptr, _cycler->cheat());
  return pointer;
}

/**
 * Returns the Thread pointer of the currently-executing thread, as passed to
 * the constructor of this object.
 */
template<class CycleDataType>
INLINE Thread *CycleDataLockedReader<CycleDataType>::
get_current_thread() const {
  return _current_thread;
}

#else  // !DO_PIPELINING
// This is the trivial, do-nothing implementation.

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
CycleDataLockedReader(const PipelineCycler<CycleDataType> &cycler, Thread *) {
  _pointer = cycler.cheat();
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
CycleDataLockedReader(const CycleDataLockedReader<CycleDataType> &copy) :
  _pointer(copy._pointer)
{
}

/**
 *
 */
template<class CycleDataType>
INLINE void CycleDataLockedReader<CycleDataType>::
operator = (const CycleDataLockedReader<CycleDataType> &copy) {
  _pointer = copy._pointer;
}

/**
 *
 */
template<class CycleDataType>
INLINE void CycleDataLockedReader<CycleDataType>::
operator = (CycleDataLockedReader<CycleDataType> &&from) noexcept {
  nassertv(_pointer == nullptr);

  _pointer = from._pointer;
  from._pointer = nullptr;
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
~CycleDataLockedReader() {
}

/**
 * This provides an indirect member access to the actual CycleData data.
 */
template<class CycleDataType>
INLINE const CycleDataType *CycleDataLockedReader<CycleDataType>::
operator -> () const {
  return _pointer;
}

/**
 * This allows the CycleDataLockedReader to be passed to any function that
 * expects a const CycleDataType pointer.
 */
template<class CycleDataType>
INLINE CycleDataLockedReader<CycleDataType>::
operator const CycleDataType * () const {
  return _pointer;
}

/**
 * This is intended to be called only from CycleDataWriter when it elevates
 * the pointer from read to write status.  This function returns the reader's
 * pointer and relinquishes ownership of the pointer, rendering the reader
 * invalid for future reads.
 */
template<class CycleDataType>
INLINE const CycleDataType *CycleDataLockedReader<CycleDataType>::
take_pointer() {
  return _pointer;
}

/**
 * Returns the Thread pointer of the currently-executing thread, as passed to
 * the constructor of this object.
 */
template<class CycleDataType>
INLINE Thread *CycleDataLockedReader<CycleDataType>::
get_current_thread() const {
  return Thread::get_current_thread();
}

#endif  // DO_PIPELINING
#endif  // CPPPARSER
