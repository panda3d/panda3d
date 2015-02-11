// Filename: streamWrapper.I
// Created by:  drose (11Nov08)
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
//     Function: StreamWrapperBase::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE StreamWrapperBase::
StreamWrapperBase() {
#ifdef SIMPLE_THREADS
  _lock_flag = false;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWrapperBase::acquire
//       Access: Published
//  Description: Acquires the internal lock.  
//
//               User code should call this to take temporary
//               possession of the stream and perform direct I/O
//               operations on it, for instance to make several
//               sequential atomic reads.  You may not call any of the
//               StreamWrapper methods while the lock is held, other
//               than release().
//
//               Use with extreme caution!  This is a very low-level,
//               non-recursive lock.  You must call acquire() only
//               once, and you must later call release() exactly once.
//               Failing to do so may result in a hard deadlock with
//               no available debugging features.
////////////////////////////////////////////////////////////////////
INLINE void StreamWrapperBase::
acquire() {
  _lock.acquire();
#ifdef SIMPLE_THREADS
  while (_lock_flag) {
    thread_yield();
  }
  _lock_flag = true;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWrapperBase::release
//       Access: Published
//  Description: Releases the internal lock.  Must be called exactly
//               once following a call to acquire().  See the cautions
//               with acquire().
////////////////////////////////////////////////////////////////////
INLINE void StreamWrapperBase::
release() {
#ifdef SIMPLE_THREADS
  assert(_lock_flag);
  _lock_flag = false;
#endif
  _lock.release();
}

////////////////////////////////////////////////////////////////////
//     Function: IStreamWrapper::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE IStreamWrapper::
IStreamWrapper(istream *stream, bool owns_pointer) :
  _istream(stream),
  _owns_pointer(owns_pointer)
{
}

////////////////////////////////////////////////////////////////////
//     Function: IStreamWrapper::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE IStreamWrapper::
IStreamWrapper(istream &stream) :
  _istream(&stream),
  _owns_pointer(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: IStreamWrapper::get_istream
//       Access: Published
//  Description: Returns the istream this object is wrapping.
////////////////////////////////////////////////////////////////////
INLINE istream *IStreamWrapper::
get_istream() const {
  return _istream;
}

////////////////////////////////////////////////////////////////////
//     Function: IStreamWrapper::get
//       Access: Public
//  Description: Atomically reads a single character from the stream.
////////////////////////////////////////////////////////////////////
INLINE int IStreamWrapper::
get() {
  int result;
  acquire();
  result = _istream->get();
  release();
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: OStreamWrapper::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE OStreamWrapper::
OStreamWrapper(ostream *stream, bool owns_pointer, bool stringstream_hack) :
  _ostream(stream),
  _owns_pointer(owns_pointer)
#ifdef WIN32_VC
, _stringstream_hack(stringstream_hack)
#endif
{
}

////////////////////////////////////////////////////////////////////
//     Function: OStreamWrapper::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE OStreamWrapper::
OStreamWrapper(ostream &stream) :
  _ostream(&stream),
  _owns_pointer(false)
#ifdef WIN32_VC
, _stringstream_hack(false)
#endif
{
}

////////////////////////////////////////////////////////////////////
//     Function: OStreamWrapper::get_ostream
//       Access: Published
//  Description: Returns the ostream this object is wrapping.
////////////////////////////////////////////////////////////////////
INLINE ostream *OStreamWrapper::
get_ostream() const {
  return _ostream;
}

////////////////////////////////////////////////////////////////////
//     Function: OStreamWrapper::put
//       Access: Public
//  Description: Atomically writes a single character to the stream.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
INLINE bool OStreamWrapper::
put(char c) {
  bool success;
  acquire();
  _ostream->put(c);
  success = !_ostream->bad();
  release();
  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWrapper::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE StreamWrapper::
StreamWrapper(iostream *stream, bool owns_pointer, bool stringstream_hack) :
  IStreamWrapper(stream, false),
  OStreamWrapper(stream, false, stringstream_hack),
  _iostream(stream),
  _owns_pointer(owns_pointer)
{
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWrapper::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE StreamWrapper::
StreamWrapper(iostream &stream) :
  IStreamWrapper(&stream, false),
  OStreamWrapper(&stream, false),
  _iostream(&stream),
  _owns_pointer(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWrapper::get_iostream
//       Access: Published
//  Description: Returns the iostream this object is wrapping.
////////////////////////////////////////////////////////////////////
INLINE iostream *StreamWrapper::
get_iostream() const {
  return _iostream;
}
