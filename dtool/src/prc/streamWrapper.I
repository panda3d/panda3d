/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamWrapper.I
 * @author drose
 * @date 2008-11-11
 */

/**
 *
 */
INLINE StreamWrapperBase::
StreamWrapperBase() {
#ifdef SIMPLE_THREADS
  _lock_flag = false;
#endif
}

/**
 * Acquires the internal lock.
 *
 * User code should call this to take temporary possession of the stream and
 * perform direct I/O operations on it, for instance to make several
 * sequential atomic reads.  You may not call any of the StreamWrapper methods
 * while the lock is held, other than release().
 *
 * Use with extreme caution!  This is a very low-level, non-recursive lock.
 * You must call acquire() only once, and you must later call release()
 * exactly once.  Failing to do so may result in a hard deadlock with no
 * available debugging features.
 */
INLINE void StreamWrapperBase::
acquire() {
  _lock.lock();
#ifdef SIMPLE_THREADS
  while (_lock_flag) {
    thread_yield();
  }
  _lock_flag = true;
#endif
}

/**
 * Releases the internal lock.  Must be called exactly once following a call
 * to acquire().  See the cautions with acquire().
 */
INLINE void StreamWrapperBase::
release() {
#ifdef SIMPLE_THREADS
  assert(_lock_flag);
  _lock_flag = false;
#endif
  _lock.unlock();
}

/**
 * Increments the reference count.  Only has impact if the class that manages
 * this StreamWrapper's lifetime (eg. Multifile) respects it.
 */
INLINE void StreamWrapperBase::
ref() const {
  AtomicAdjust::inc(_ref_count);
}

/**
 * Decrements the reference count.  Only has impact if the class that manages
 * this StreamWrapper's lifetime (eg. Multifile) respects it.
 */
INLINE bool StreamWrapperBase::
unref() const {
  return AtomicAdjust::dec(_ref_count);
}

/**
 *
 */
INLINE IStreamWrapper::
IStreamWrapper(std::istream *stream, bool owns_pointer) :
  _istream(stream),
  _owns_pointer(owns_pointer)
{
}

/**
 *
 */
INLINE IStreamWrapper::
IStreamWrapper(std::istream &stream) :
  _istream(&stream),
  _owns_pointer(false)
{
}

/**
 * Returns the istream this object is wrapping.
 */
INLINE std::istream *IStreamWrapper::
get_istream() const {
  return _istream;
}

/**
 * Atomically reads a single character from the stream.
 */
INLINE int IStreamWrapper::
get() {
  int result;
  acquire();
  result = _istream->get();
  release();
  return result;
}


/**
 *
 */
INLINE OStreamWrapper::
OStreamWrapper(std::ostream *stream, bool owns_pointer, bool stringstream_hack) :
  _ostream(stream),
  _owns_pointer(owns_pointer)
#ifdef WIN32_VC
, _stringstream_hack(stringstream_hack)
#endif
{
}

/**
 *
 */
INLINE OStreamWrapper::
OStreamWrapper(std::ostream &stream) :
  _ostream(&stream),
  _owns_pointer(false)
#ifdef WIN32_VC
, _stringstream_hack(false)
#endif
{
}

/**
 * Returns the ostream this object is wrapping.
 */
INLINE std::ostream *OStreamWrapper::
get_ostream() const {
  return _ostream;
}

/**
 * Atomically writes a single character to the stream.  Returns true on
 * success, false on failure.
 */
INLINE bool OStreamWrapper::
put(char c) {
  bool success;
  acquire();
  _ostream->put(c);
  success = !_ostream->bad();
  release();
  return success;
}

/**
 *
 */
INLINE StreamWrapper::
StreamWrapper(std::iostream *stream, bool owns_pointer, bool stringstream_hack) :
  IStreamWrapper(stream, false),
  OStreamWrapper(stream, false, stringstream_hack),
  _iostream(stream),
  _owns_pointer(owns_pointer)
{
}

/**
 *
 */
INLINE StreamWrapper::
StreamWrapper(std::iostream &stream) :
  IStreamWrapper(&stream, false),
  OStreamWrapper(&stream, false),
  _iostream(&stream),
  _owns_pointer(false)
{
}

/**
 * Returns the iostream this object is wrapping.
 */
INLINE std::iostream *StreamWrapper::
get_iostream() const {
  return _iostream;
}
