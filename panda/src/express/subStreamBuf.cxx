// Filename: subStreamBuf.cxx
// Created by:  drose (02Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "subStreamBuf.h"

#ifndef HAVE_STREAMSIZE
// Some compilers (notably SGI) don't define this for us
typedef int streamsize;
#endif /* HAVE_STREAMSIZE */

// Temporary hack to make these thread-safe.
//#define SUBSTREAM_THREAD_SAFE

#ifdef SUBSTREAM_THREAD_SAFE
// This requires NSPR for now.
#include <prlock.h>
PRLock *substream_mutex = (PRLock *)NULL;

inline void init_lock() {
  if (substream_mutex == (PRLock *)NULL) {
    substream_mutex = PR_NewLock();
  }
}
inline void grab_lock() {
  PR_Lock(substream_mutex);
}
inline void release_lock() {
  PR_Unlock(substream_mutex);
}
#else  // SUBSTREAM_THREAD_SAFE
inline void init_lock() {
}
inline void grab_lock() {
}
inline void release_lock() {
}
#endif  // SUBSTREAM_THREAD_SAFE

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
SubStreamBuf::
SubStreamBuf() {
  _source = (istream *)NULL;

  // _start is the streampos of the first byte of the SubStream within
  // its parent stream.
  _start = 0;

  // _end is the streampos of the byte following the last byte of the
  // SubStream within its parent stream.  If _end is 0, the SubStream
  // continues to the end of the parent stream, wherever that is.
  _end = 0;

  // _cur is the streampos of the end of the read buffer (that is,
  // egptr()) within the parent stream.  By comparing _cur to gpos(),
  // we can determine the actual current file position.
  _cur = 0;

  // _unused counts the number of bytes at the beginning of the buffer
  // that are unused.  Usually this is 0 after a read, but when we
  // reach the end of the file we might not need the whole buffer to
  // read the last bit, so the first part of the buffer is unused.
  // This is important to prevent us from inadvertently seeking into
  // the unused part of the buffer.
  _unused = 0;

  init_lock();

#ifdef HAVE_IOSTREAM
  // The new-style iostream library doesn't seem to support allocate().
  char *buf = new char[4096];
  char *ebuf = buf + 4096;
  setg(buf, ebuf, ebuf);

#else
  allocate();
  setg(base(), ebuf(), ebuf());
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
SubStreamBuf::
~SubStreamBuf() {
  close();
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::open
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void SubStreamBuf::
open(istream *source, streampos start, streampos end) {
  _source = source;
  _start = start;
  _end = end;
  _cur = _start;

  // Initially, the entire buffer is unused.  Duh.
  _unused = egptr() - eback();
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::close
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void SubStreamBuf::
close() {
  _source = (istream *)NULL;
  _start = 0;
  _end = 0;
  _cur = 0;
  _unused = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::seekoff
//       Access: Public, Virtual
//  Description: Implements seeking within the stream.
////////////////////////////////////////////////////////////////////
streampos SubStreamBuf::
seekoff(streamoff off, ios_seekdir dir, ios_openmode mode) {
  // Invariant: _cur points to the file location of the buffer at
  // egptr().

  // Use this to determine the actual file position right now.
  size_t n = egptr() - gptr();
  streampos cur_pos = _cur - (streampos)n;
  streampos new_pos = cur_pos;

  // Now adjust the data pointer appropriately.

  // Casting this to int to prevent GCC 3.2 compiler warnings.  Very
  // suspicious, need to investigate further.
  switch ((int)dir) {
  case ios::beg:
    new_pos = _start + off;
    break;

  case ios::cur:
    new_pos = cur_pos + off;
    break;

  case ios::end:
    if (_end == (streampos)0) {
      // If the end of the file is unspecified, we have to seek to
      // find it.
      grab_lock();
      _source->seekg(off, ios::end);
      new_pos = _source->tellg();
      release_lock();

    } else {
      new_pos = _end + off;
    }
    break;
  }

  new_pos = max(_start, new_pos);
  streamsize delta = new_pos - cur_pos;

  if (gptr() + delta >= eback() + _unused && gptr() + delta <= egptr()) {
    // If we can get away with just bumping the gptr within the
    // buffer, do so.
    gbump(delta);

  } else {
    // Otherwise, empty the buffer and force it to call underflow().
    gbump(n);
    _cur = new_pos;
  }

  return new_pos - _start;
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::seekpos
//       Access: Public, Virtual
//  Description: A variant on seekoff() to implement seeking within a
//               stream.
//
//               The MSDN Library claims that it is only necessary to
//               redefine seekoff(), and not seekpos() as well, as the
//               default implementation of seekpos() is supposed to
//               map to seekoff() exactly as I am doing here; but in
//               fact it must do something else, because seeking
//               didn't work on Windows until I redefined this
//               function as well.
////////////////////////////////////////////////////////////////////
streampos SubStreamBuf::
seekpos(streampos pos, ios_openmode mode) {
  return seekoff(pos, ios::beg, mode);
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::overflow
//       Access: Protected, Virtual
//  Description: Called by the system ostream implementation when its
//               internal buffer is filled, plus one character.
////////////////////////////////////////////////////////////////////
int SubStreamBuf::
overflow(int c) {
  // We don't support ostream.
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::sync
//       Access: Protected, Virtual
//  Description: Called by the system iostream implementation to
//               implement a flush operation.
////////////////////////////////////////////////////////////////////
int SubStreamBuf::
sync() {
  size_t n = egptr() - gptr();
  gbump(n);

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::underflow
//       Access: Protected, Virtual
//  Description: Called by the system istream implementation when its
//               internal buffer needs more characters.
////////////////////////////////////////////////////////////////////
int SubStreamBuf::
underflow() {
  // Sometimes underflow() is called even if the buffer is not empty.
  if (gptr() >= egptr()) {
    if (_end != (streampos)0 && _cur >= _end) {
      // We're done.
      return EOF;
    }
    
    size_t buffer_size = egptr() - eback();
    size_t num_bytes;
    if (_end == (streampos)0 || _end - _cur > (streampos)buffer_size) {
      // We have enough bytes in the input stream to fill our buffer.
      num_bytes = buffer_size;
    } else {
      // We won't quite fill the buffer.
      num_bytes = (size_t)(_end - _cur);
    }

    gbump(-(int)num_bytes);
    nassertr(gptr() + num_bytes <= egptr(), EOF);

    grab_lock();
    _source->seekg(_cur);
    _source->read(gptr(), num_bytes);
    size_t read_count = _source->gcount();
    release_lock();

    if (read_count != num_bytes) {
      // Oops, we didn't read what we thought we would.
      if (read_count == 0) {
        _unused = buffer_size;
        if (_end != (streampos)0) {
          _end = _cur;
        }
        gbump(num_bytes);
        return EOF;
      }

      // Slide what we did read to the top of the buffer.
      nassertr(read_count < num_bytes, EOF);
      size_t delta = num_bytes - read_count;
      memmove(gptr() + delta, gptr(), read_count);
      gbump(delta);
    }

    // Now record whatever bytes at the beginning of the buffer are
    // unused, so we won't try to seek into that area.
    _unused = buffer_size - read_count;

    // Invariant: _cur points to the file location of the buffer at
    // egptr().

    _cur += read_count;
  }

  return (unsigned char)*gptr();
}
