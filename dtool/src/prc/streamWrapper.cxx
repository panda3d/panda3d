/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamWrapper.cxx
 * @author drose
 * @date 2008-11-11
 */

#include "streamWrapper.h"

using std::streamsize;

/**
 *
 */
IStreamWrapper::
~IStreamWrapper() {
  if (_owns_pointer) {
    // For some reason--compiler bug in gcc 3.2?--explicitly deleting the
    // stream pointer does not call the appropriate global delete function;
    // instead apparently calling the system delete function.  So we call the
    // delete function by hand instead.
#if !defined(WIN32_VC) && !defined(USE_MEMORY_NOWRAPPERS) && defined(REDEFINE_GLOBAL_OPERATOR_NEW)
    _istream->~istream();
    (*global_operator_delete)(_istream);
#else
    delete _istream;
#endif
  }
}

/**
 * Atomically reads a number of bytes from the stream, without error
 * detection.  If fewer bytes than requested are read, quietly fills the
 * remaining bytes with 0.
 */
void IStreamWrapper::
read(char *buffer, streamsize num_bytes) {
  acquire();
  _istream->clear();
  _istream->read(buffer, num_bytes);
  streamsize read_bytes = _istream->gcount();
  while (read_bytes < num_bytes) {
    // Fewer bytes than expected were read.  Maybe more will be coming later.
    release();
    thread_yield();
    acquire();

    _istream->read(buffer + read_bytes, num_bytes - read_bytes);
    streamsize this_read_bytes = _istream->gcount();
    assert(this_read_bytes <= num_bytes - read_bytes);
    read_bytes += this_read_bytes;

    if (this_read_bytes == 0) {
      // No, don't expect any more.
      memset(buffer + read_bytes, 0, num_bytes - read_bytes);
      break;
    }
  }
  assert(read_bytes <= num_bytes);
  release();
}

/**
 * Atomically reads a number of bytes from the stream.  Returns the number of
 * bytes actually read.
 */
void IStreamWrapper::
read(char *buffer, streamsize num_bytes, streamsize &read_bytes) {
  acquire();
  _istream->clear();
  _istream->read(buffer, num_bytes);
  read_bytes = _istream->gcount();
  assert(read_bytes <= num_bytes);
  release();
}

/**
 * Atomically reads a number of bytes from the stream.  Returns the number of
 * bytes actually read, and whether an eof condition was detected by the
 * operation.
 */
void IStreamWrapper::
read(char *buffer, streamsize num_bytes, streamsize &read_bytes, bool &eof) {
  acquire();
  _istream->clear();
  _istream->read(buffer, num_bytes);
  read_bytes = _istream->gcount();
  assert(read_bytes <= num_bytes);
  eof = _istream->eof() || _istream->fail();
  release();
}

/**
 * Atomically seeks to a particular offset from the beginning of the file, and
 * reads a number of bytes from the stream.  Returns the number of bytes
 * actually read, and whether an eof condition was detected by the operation.
 */
void IStreamWrapper::
seek_read(streamsize pos, char *buffer, streamsize num_bytes,
          streamsize &read_bytes, bool &eof) {
  acquire();
  _istream->clear();
  _istream->seekg(pos);
  _istream->read(buffer, num_bytes);
  read_bytes = _istream->gcount();
  assert(read_bytes <= num_bytes);
  eof = _istream->eof() || _istream->fail();
  release();
}

/**
 * Atomically seeks to EOF and returns the gpos there; that is, returns the
 * file size.  Note that the EOF might have been moved in another thread by
 * the time this method returns.
 */
streamsize IStreamWrapper::
seek_gpos_eof() {
  streamsize pos;
  acquire();
  _istream->seekg(0, std::ios::end);
  pos = _istream->tellg();
  release();

  return pos;
}

/**
 *
 */
OStreamWrapper::
~OStreamWrapper() {
  if (_owns_pointer) {
    // For some reason--compiler bug in gcc 3.2?--explicitly deleting the
    // stream pointer does not call the appropriate global delete function;
    // instead apparently calling the system delete function.  So we call the
    // delete function by hand instead.
#if !defined(WIN32_VC) && !defined(USE_MEMORY_NOWRAPPERS) && defined(REDEFINE_GLOBAL_OPERATOR_NEW)
    _ostream->~ostream();
    (*global_operator_delete)(_ostream);
#else
    delete _ostream;
#endif
  }
}

/**
 * Atomically writes a number of bytes to the stream, without error detection.
 */
void OStreamWrapper::
write(const char *buffer, streamsize num_bytes) {
  acquire();
  _ostream->write(buffer, num_bytes);
  release();
}

/**
 * Atomically writes a number of bytes to the stream.  Returns whether a
 * failure condition was detected by the operation.
 */
void OStreamWrapper::
write(const char *buffer, streamsize num_bytes, bool &fail) {
  acquire();
  _ostream->clear();
  _ostream->write(buffer, num_bytes);
  fail = _ostream->fail();
  release();
}

/**
 * Atomically seeks to a particular offset from the beginning of the file, and
 * writes a number of bytes to the stream.  Returns whether a failure
 * condition was detected by the operation.
 */
void OStreamWrapper::
seek_write(streamsize pos, const char *buffer, streamsize num_bytes,
           bool &fail) {
  acquire();
  _ostream->clear();
  _ostream->seekp(pos);

#ifdef WIN32_VC
  if (_ostream->fail() && _stringstream_hack && pos == 0) {
    // Ignore an unsuccessful attempt to seekp(0) if _stringstream_hack is
    // true.
    _ostream->clear();
  }
#endif // WIN32_VC

  _ostream->write(buffer, num_bytes);
  fail = _ostream->fail();
  release();
}

/**
 * Atomically seeks to the end of the file, and writes a number of bytes to
 * the stream.  Returns whether a failure condition was detected by the
 * operation.
 */
void OStreamWrapper::
seek_eof_write(const char *buffer, streamsize num_bytes, bool &fail) {
  acquire();
  _ostream->clear();
  _ostream->seekp(0, std::ios::end);

#ifdef WIN32_VC
  if (_ostream->fail() && _stringstream_hack) {
    // Ignore an unsuccessful attempt to seekp(0) if _stringstream_hack is
    // true.
    _ostream->clear();
  }
#endif // WIN32_VC

  _ostream->write(buffer, num_bytes);
  fail = _ostream->fail();
  release();
}

/**
 * Atomically seeks to EOF and returns the ppos there; that is, returns the
 * file size.  Note that the EOF might have been moved in another thread by
 * the time this method returns.
 */
streamsize OStreamWrapper::
seek_ppos_eof() {
  streamsize pos;
  acquire();
  _ostream->seekp(0, std::ios::end);

#ifdef WIN32_VC
  if (_ostream->fail() && _stringstream_hack) {
    // Ignore an unsuccessful attempt to seekp(0) if _stringstream_hack is
    // true.
    _ostream->clear();
    release();
    return 0;
  }
#endif // WIN32_VC

  pos = _ostream->tellp();
  release();

  return pos;
}

/**
 *
 */
StreamWrapper::
~StreamWrapper() {
  if (_owns_pointer) {
    // For some reason--compiler bug in gcc 3.2?--explicitly deleting the
    // stream pointer does not call the appropriate global delete function;
    // instead apparently calling the system delete function.  So we call the
    // delete function by hand instead.
#if !defined(WIN32_VC) && !defined(USE_MEMORY_NOWRAPPERS) && defined(REDEFINE_GLOBAL_OPERATOR_NEW)
    _iostream->~iostream();
    (*global_operator_delete)(_iostream);
#else
    delete _iostream;
#endif
  }
}
