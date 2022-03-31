/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file subStreamBuf.cxx
 * @author drose
 * @date 2002-08-02
 */

#include "subStreamBuf.h"
#include "pnotify.h"
#include "memoryHook.h"

using std::ios;
using std::streamoff;
using std::streampos;
using std::streamsize;

static const size_t substream_buffer_size = 4096;

/**
 *
 */
SubStreamBuf::
SubStreamBuf() {
  _source = nullptr;
  _dest = nullptr;

  // _start is the streampos of the first byte of the SubStream within its
  // parent stream.
  _start = 0;

  // _end is the streampos of the byte following the last byte of the
  // SubStream within its parent stream.  If _end is 0, the SubStream
  // continues to the end of the parent stream, wherever that is.
  _end = 0;

  // _gpos is the streampos of the end of the read buffer (that is, egptr())
  // within the parent stream.  By comparing _gpos to gpos(), we can determine
  // the actual current file position.  _ppos is the similar pos, for the
  // write pointer.
  _gpos = 0;
  _ppos = 0;

#ifdef PHAVE_IOSTREAM
  _buffer = (char *)PANDA_MALLOC_ARRAY(substream_buffer_size * 2);
  char *ebuf = _buffer + substream_buffer_size * 2;
  char *mbuf = _buffer + substream_buffer_size;
  setg(_buffer, mbuf, mbuf);
  setp(mbuf, ebuf);

#else
  allocate();
  // Chop the buffer in half.  The bottom half goes to the get buffer; the top
  // half goes to the put buffer.
  char *b = base();
  char *t = ebuf();
  char *m = b + (t - b) / 2;
  setg(b, m, m);
  setp(b, m);
#endif
}

/**
 *
 */
SubStreamBuf::
~SubStreamBuf() {
  close();
#ifdef PHAVE_IOSTREAM
  PANDA_FREE_ARRAY(_buffer);
#endif
}

/**
 *
 */
void SubStreamBuf::
open(IStreamWrapper *source, OStreamWrapper *dest, streampos start, streampos end, bool append) {
  _source = source;
  _dest = dest;
  _start = start;
  _end = end;
  _append = append;
  _gpos = _start;
  _ppos = _start;

  if (source != nullptr) {
    source->ref();
  }
  if (dest != nullptr) {
    dest->ref();
  }
}

/**
 *
 */
void SubStreamBuf::
close() {
  // Make sure the write buffer is flushed.
  sync();

  if (_source != nullptr && !_source->unref()) {
    delete _source;
  }
  if (_dest != nullptr && !_dest->unref()) {
    delete _dest;
  }
  _source = nullptr;
  _dest = nullptr;
  _start = 0;
  _end = 0;

  _gpos = 0;
  _ppos = 0;

  pbump(pbase() - pptr());
  gbump(egptr() - gptr());
}

/**
 * Implements seeking within the stream.
 */
streampos SubStreamBuf::
seekoff(streamoff off, ios_seekdir dir, ios_openmode which) {
  streampos result = -1;

  // Sync the iostream buffer first.
  sync();

  if (which & ios::in) {
    // Determine the current file position.
    size_t n = egptr() - gptr();
    gbump(n);
    _gpos -= n;
    nassertr(_gpos >= 0, EOF);
    streampos cur_pos = _gpos;
    streampos new_pos = cur_pos;

    // Now adjust the data pointer appropriately.
    switch (dir) {
    case ios::beg:
      new_pos = (streampos)off + _start;
      break;

    case ios::cur:
      new_pos = (streampos)((streamoff)cur_pos + off);
      break;

    case ios::end:
      if (_end == (streampos)0) {
        // If the end of the file is unspecified, we have to seek to find it.
        new_pos = _source->seek_gpos_eof() + off;

      } else {
        new_pos = _end + off;
      }
      break;

    default:
      // Shouldn't get here.
      break;
    }

    if (new_pos < _start) {
      // Can't seek before beginning of file.
      return EOF;
    }

    if (_end != (streampos)0 && new_pos > _end) {
      // Can't seek past end of file.
      return EOF;
    }

    _gpos = new_pos;
    nassertr(_gpos >= 0, EOF);
    result = new_pos - _start;
  }

  if (which & ios::out) {
    // Determine the current file position.
    size_t n = pptr() - pbase();
    streampos cur_pos = _ppos + (streamoff)n;
    streampos new_pos = cur_pos;

    // Now adjust the data pointer appropriately.
    switch (dir) {
    case ios::beg:
      new_pos = (streampos)off + _start;
      break;

    case ios::cur:
      new_pos = (streampos)((streamoff)cur_pos + off);
      break;

    case ios::end:
      if (_end == (streampos)0) {
        // If the end of the file is unspecified, we have to seek to find it.
        new_pos = _dest->seek_ppos_eof() + off;

      } else {
        new_pos = _end + off;
      }
      break;

    default:
      // Shouldn't get here.
      break;
    }

    if (new_pos < _start) {
      // Can't seek before beginning of file.
      return EOF;
    }

    if (_end != (streampos)0 && new_pos > _end) {
      // Can't seek past end of file.
      return EOF;
    }

    _ppos = new_pos;
    nassertr(_ppos >= 0, EOF);
    result = new_pos - _start;
  }

  return result;
}

/**
 * A variant on seekoff() to implement seeking within a stream.
 *
 * The MSDN Library claims that it is only necessary to redefine seekoff(),
 * and not seekpos() as well, as the default implementation of seekpos() is
 * supposed to map to seekoff() exactly as I am doing here; but in fact it
 * must do something else, because seeking didn't work on Windows until I
 * redefined this function as well.
 */
streampos SubStreamBuf::
seekpos(streampos pos, ios_openmode which) {
  return seekoff(pos, ios::beg, which);
}

/**
 * Called by the system ostream implementation when its internal buffer is
 * filled, plus one character.
 */
int SubStreamBuf::
overflow(int ch) {
  bool okflag = true;

  size_t n = pptr() - pbase();
  if (n != 0) {
    if (_end != (streampos)0 && _ppos + (streampos)n > _end) {
      // Don't allow reading past the end of the file.
      n = (size_t)(_end - _ppos);
      if (n == 0) {
        // No room.
        return EOF;
      }
    }

    nassertr(_dest != nullptr, EOF);
    bool fail = false;
    if (_append) {
      _dest->seek_eof_write(pbase(), n, fail);
    } else {
      _dest->seek_write(_ppos, pbase(), n, fail);
    }
    _ppos += n;
    pbump(-(int)n);
    if (fail) {
      okflag = false;
    }
  }

  if (okflag && ch != EOF) {
    if (pptr() != epptr()) {
      // Store the extra character back in the buffer.
      *(pptr()) = ch;
      pbump(1);
    } else {
      // No room to store ch.
      okflag = false;
    }
  }

  if (!okflag) {
    return EOF;
  }
  return 0;
}

/**
 * Called by the system iostream implementation to implement a flush
 * operation.
 */
int SubStreamBuf::
sync() {
  size_t n = pptr() - pbase();

  if (n != 0) {
    nassertr(_dest != nullptr, EOF);
    bool fail = false;
    if (_append) {
      _dest->seek_eof_write(pbase(), n, fail);
    } else {
      _dest->seek_write(_ppos, pbase(), n, fail);
    }
    _ppos += n;
    pbump(-(int)n);

    if (fail) {
      return EOF;
    }
  }

  return 0;
}

/**
 * Called by the system istream implementation when its internal buffer needs
 * more characters.
 */
int SubStreamBuf::
underflow() {
  // Sometimes underflow() is called even if the buffer is not empty.
  if (gptr() >= egptr()) {
    sync();

    // Mark the buffer filled (with buffer_size bytes).
    size_t buffer_size = egptr() - eback();
    gbump(-(int)buffer_size);

    streamsize num_bytes = buffer_size;
    if (_end != (streampos)0 && _gpos + (streampos)num_bytes > _end) {
      // Don't allow reading past the end of the file.
      streamsize new_num_bytes = _end - _gpos;
      if (new_num_bytes == 0) {
        gbump(buffer_size);
        return EOF;
      }

      // We won't be filling the entire buffer.  Fill in only at the end of
      // the buffer.
      size_t delta = num_bytes - new_num_bytes;
      gbump(delta);
      num_bytes = new_num_bytes;
      nassertr(egptr() - gptr() == num_bytes, EOF);
    }

    nassertr(_source != nullptr, EOF);
    streamsize read_count;
    bool eof;
    _source->seek_read(_gpos, gptr(), num_bytes, read_count, eof);
    _gpos += read_count;

    if (read_count != num_bytes) {
      // Oops, we didn't read what we thought we would.
      if (read_count == 0) {
        gbump(num_bytes);
        return EOF;
      }

      // Slide what we did read to the top of the buffer.
      nassertr(read_count < num_bytes, EOF);
      size_t delta = num_bytes - read_count;
      memmove(gptr() + delta, gptr(), read_count);
      gbump(delta);
    }
  }

  return (unsigned char)*gptr();
}
