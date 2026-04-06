/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cycleDataReader.I
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
INLINE CycleDataReader<CycleDataType>::
CycleDataReader(const PipelineCycler<CycleDataType> &cycler,
                Thread *current_thread) :
  _cycler(&cycler),
  _current_thread(current_thread)
{
  _pointer = _cycler->read_unlocked(_current_thread);
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
CycleDataReader(const CycleDataReader<CycleDataType> &copy) :
  _cycler(copy._cycler),
  _current_thread(copy._current_thread),
  _pointer(copy._pointer)
{
}

/**
 *
 */
template<class CycleDataType>
INLINE void CycleDataReader<CycleDataType>::
operator = (const CycleDataReader<CycleDataType> &copy) {
  nassertv(_current_thread == copy._current_thread);

  _cycler = copy._cycler;
  _pointer = copy._pointer;
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
~CycleDataReader() {
}

/**
 * This provides an indirect member access to the actual CycleData data.
 */
template<class CycleDataType>
INLINE const CycleDataType *CycleDataReader<CycleDataType>::
operator -> () const {
  return _pointer;
}

/**
 * This allows the CycleDataReader to be passed to any function that expects a
 * const CycleDataType pointer.
 */
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
operator const CycleDataType * () const {
  return _pointer;
}

/**
 * This allows the CycleDataReader to be passed to any function that expects a
 * const CycleDataType pointer.
 */
template<class CycleDataType>
INLINE const CycleDataType *CycleDataReader<CycleDataType>::
p() const {
  return _pointer;
}

/**
 * Returns the Thread pointer of the currently-executing thread, as passed to
 * the constructor of this object.
 */
template<class CycleDataType>
INLINE Thread *CycleDataReader<CycleDataType>::
get_current_thread() const {
  return _current_thread;
}

#else  // !DO_PIPELINING
// This is the trivial, do-nothing implementation.

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
CycleDataReader(const PipelineCycler<CycleDataType> &cycler, Thread *) {
  _pointer = cycler.cheat();
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
CycleDataReader(const CycleDataReader<CycleDataType> &copy) :
  _pointer(copy._pointer)
{
}

/**
 *
 */
template<class CycleDataType>
INLINE void CycleDataReader<CycleDataType>::
operator = (const CycleDataReader<CycleDataType> &copy) {
  _pointer = copy._pointer;
}

/**
 *
 */
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
~CycleDataReader() {
}

/**
 * This provides an indirect member access to the actual CycleData data.
 */
template<class CycleDataType>
INLINE const CycleDataType *CycleDataReader<CycleDataType>::
operator -> () const {
  return _pointer;
}

/**
 * This allows the CycleDataReader to be passed to any function that expects a
 * const CycleDataType pointer.
 */
template<class CycleDataType>
INLINE CycleDataReader<CycleDataType>::
operator const CycleDataType * () const {
  return _pointer;
}

/**
 * This allows the CycleDataReader to be passed to any function that expects a
 * const CycleDataType pointer.
 */
template<class CycleDataType>
INLINE const CycleDataType *CycleDataReader<CycleDataType>::
p() const {
  return _pointer;
}

/**
 * Returns the Thread pointer of the currently-executing thread, as passed to
 * the constructor of this object.
 */
template<class CycleDataType>
INLINE Thread *CycleDataReader<CycleDataType>::
get_current_thread() const {
  return Thread::get_current_thread();
}

#endif  // DO_PIPELINING
#endif  // CPPPARSER
