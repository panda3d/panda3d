/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaFileStreamBuf.cxx
 * @author drose
 * @date 2008-09-08
 */

#include "pandaFileStreamBuf.h"
#include "memoryHook.h"
#include "filename.h"
#include "textEncoder.h"

#ifdef USE_PANDAFILESTREAM

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#endif  // _WIN32

using std::cerr;
using std::dec;
using std::hex;
using std::ios;
using std::istream;
using std::ostream;
using std::streamoff;
using std::streampos;
using std::string;

PandaFileStreamBuf::NewlineMode PandaFileStreamBuf::_newline_mode = NM_native;

static const size_t file_buffer_size = 4096;

/**
 *
 */
PandaFileStreamBuf::
PandaFileStreamBuf() {
  _is_open = false;
  _open_mode = (ios::openmode)0;

  _last_read_nl = 0;

#ifdef _WIN32
  // Windows case.
  _handle = nullptr;
#else
  _fd = -1;
#endif  // _WIN32

#ifdef PHAVE_IOSTREAM
  _buffer = (char *)PANDA_MALLOC_ARRAY(file_buffer_size * 2);
  char *ebuf = _buffer + file_buffer_size * 2;
  char *mbuf = _buffer + file_buffer_size;
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

  _gpos = 0;
  _ppos = 0;
}

/**
 *
 */
PandaFileStreamBuf::
~PandaFileStreamBuf() {
  close();
#ifdef PHAVE_IOSTREAM
  PANDA_FREE_ARRAY(_buffer);
#endif
}

/**
 * Attempts to open the file for input and/or output.
 */
void PandaFileStreamBuf::
open(const char *filename, ios::openmode mode) {
  close();

  _filename = filename;
  _open_mode = mode;
  _is_open = false;

  if (_open_mode & ios::app) {
    // ios::app implies ios::out.
    _open_mode |= ios::out;
  }

#ifdef _WIN32
  // Windows case.
  DWORD access = 0;
  if (_open_mode & ios::in) {
    access |= GENERIC_READ;
  }
  if (_open_mode & ios::out) {
    access |= GENERIC_WRITE;
  }

  DWORD share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

  DWORD creation_disposition = 0;
  if ((_open_mode & (ios::trunc | ios::out)) == (ios::trunc | ios::out)) {
    creation_disposition = CREATE_ALWAYS;
  } else if (_open_mode & ios::out) {
    creation_disposition = OPEN_ALWAYS;
  } else {
    creation_disposition = OPEN_EXISTING;
  }

  DWORD flags = 0;

  if (!(_open_mode & ios::out)) {
    flags |= FILE_ATTRIBUTE_READONLY;
  }

#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
  // In SIMPLE_THREADS mode, we use "overlapped" IO.
  flags |= FILE_FLAG_OVERLAPPED;
#endif

  TextEncoder encoder;
  encoder.set_encoding(Filename::get_filesystem_encoding());
  encoder.set_text(_filename);
  std::wstring wfilename = encoder.get_wtext();
  _handle = CreateFileW(wfilename.c_str(), access, share_mode,
                        nullptr, creation_disposition, flags, nullptr);
  if (_handle != INVALID_HANDLE_VALUE) {
    // The file was successfully opened and locked.
    _is_open = true;
  }

#else
  // Posix case.
  int flags = 0;

  if ((_open_mode & (ios::in | ios::out)) == (ios::in | ios::out)) {
    flags |= O_RDWR | O_CREAT;
  } else if (_open_mode & ios::in) {
    flags |= O_RDONLY;
  } else if (_open_mode & ios::out) {
    flags |= O_WRONLY | O_CREAT;
  }

  if (_open_mode & ios::app) {
    flags |= O_APPEND;
  }

  if ((_open_mode & (ios::trunc | ios::out)) == (ios::trunc | ios::out)) {
    flags |= O_TRUNC;
  }

#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
  // In SIMPLE_THREADS mode, we use non-blocking IO.
  flags |= O_NONBLOCK;
#endif

  _fd = ::open(_filename.c_str(), flags, 0666);
  while (_fd == -1 && errno == EAGAIN) {
    thread_yield();
    _fd = ::open(_filename.c_str(), flags, 0666);
  }

  if (_fd != -1) {
    _is_open = true;
  }
#endif  // _WIN32

}

#ifdef _WIN32
/**
 * Connects the file stream to the existing OS-defined stream, presumably
 * opened via a low-level OS call.  The filename is for reporting only.  When
 * the file stream is closed, it will also close the underlying OS handle.
 *
 * This function is the Windows-specific variant.
 */
void PandaFileStreamBuf::
attach(const char *filename, HANDLE handle, ios::openmode mode) {
  close();

  _filename = filename;
  _open_mode = mode;
  _is_open = false;

  if (_open_mode & ios::app) {
    // ios::app implies ios::out.
    _open_mode |= ios::out;
  }

  _handle = handle;
  if (_handle != INVALID_HANDLE_VALUE) {
    // Presumably the handle is valid.
    _is_open = true;
  }
}
#endif  // _WIN32

#ifndef _WIN32
/**
 * Connects the file stream to the existing OS-defined stream, presumably
 * opened via a low-level OS call.  The filename is for reporting only.  When
 * the file stream is closed, it will also close the underlying OS handle.
 *
 * This function is the Posix-specific variant.
 */
void PandaFileStreamBuf::
attach(const char *filename, int fd, ios::openmode mode) {
  close();

  _filename = filename;
  _open_mode = mode;
  _is_open = false;

  if (_open_mode & ios::app) {
    // ios::app implies ios::out.
    _open_mode |= ios::out;
  }

  _fd = fd;
  if (_fd != -1) {
    // Presumably the file descriptor is valid.
    _is_open = true;
  }
}
#endif  // _WIN32

/**
 * Returns true if the file is open, false otherwise.
 */
bool PandaFileStreamBuf::
is_open() const {
  return _is_open;
}

/**
 * Empties the buffer and closes the file.
 */
void PandaFileStreamBuf::
close() {
  // Make sure the write buffer is flushed.
  sync();

#ifdef _WIN32
  if (_handle != nullptr) {
    CloseHandle(_handle);
  }
  _handle = nullptr;
#else
  if (_fd != -1) {
    ::close(_fd);
  }
  _fd = -1;
#endif  // _WIN32

  _is_open = false;
  _open_mode = (ios::openmode)0;

  _gpos = 0;
  _ppos = 0;

  pbump(pbase() - pptr());
  gbump(egptr() - gptr());
}

/**
 * Implements seeking within the stream.
 */
streampos PandaFileStreamBuf::
seekoff(streamoff off, ios_seekdir dir, ios_openmode which) {
  streampos result = -1;

  if (!(_open_mode & ios::binary)) {
    // Seeking on text files is only supported for seeks to the beginning of
    // the file.
    if (off != 0 || dir != ios::beg) {
      return -1;
    }

    _last_read_nl = 0;
  }

  // Sync the iostream buffer first.
  sync();

  if (which & ios::in) {
    // Determine the current file position.
    size_t n = egptr() - gptr();
    gbump(n);
    _gpos -= n;
    assert(_gpos >= 0);
    streampos cur_pos = _gpos;
    streampos new_pos = cur_pos;

    // Now adjust the data pointer appropriately.
    switch (dir) {
    case ios::beg:
      new_pos = (streampos)off;
      break;

    case ios::cur:
      new_pos = (streampos)(cur_pos + off);
      break;

    case ios::end:
#ifdef _WIN32
      // Windows case.
      {
        LARGE_INTEGER li;
        GetFileSizeEx(_handle, &li);
        new_pos = (streampos)li.QuadPart + off;
      }
#else
      // Posix case.
      {
        off_t li = lseek(_fd, off, SEEK_END);
        if (li == (off_t)-1) {
          return -1;
        }
        new_pos = (size_t)li;
      }
#endif  // _WIN32
      break;

    default:
      // Shouldn't get here.
      break;
    }

    _gpos = new_pos;
    assert(_gpos >= 0);
    result = new_pos;
  }

  if (which & ios::out) {
    // Determine the current file position.
    size_t n = pptr() - pbase();
    streampos cur_pos = _ppos + (streamoff)n;
    streampos new_pos = cur_pos;

    // Now adjust the data pointer appropriately.
    switch (dir) {
    case ios::beg:
      new_pos = (streampos)off;
      break;

    case ios::cur:
      new_pos = (streampos)(cur_pos + off);
      break;

    case ios::end:
#ifdef _WIN32
      // Windows case.
      {
        LARGE_INTEGER li;
        GetFileSizeEx(_handle, &li);
        new_pos = (streampos)li.QuadPart + off;
      }
#else
      // Posix case.
      new_pos = lseek(_fd, off, SEEK_END);
      if (new_pos == (streampos)-1) {
        return -1;
      }
#endif  // _WIN32
      break;

    default:
      // Shouldn't get here.
      break;
    }

    _ppos = new_pos;
    assert(_ppos >= 0);
    result = new_pos;
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
streampos PandaFileStreamBuf::
seekpos(streampos pos, ios_openmode which) {
  return seekoff(pos, ios::beg, which);
}

/**
 * Called by the system ostream implementation when its internal buffer is
 * filled, plus one character.
 */
int PandaFileStreamBuf::
overflow(int ch) {
  bool okflag = true;

  size_t n = pptr() - pbase();
  if (n != 0) {
    size_t wrote = write_chars(pbase(), n);
    pbump(-(streamoff)wrote);
    if (wrote != n) {
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
int PandaFileStreamBuf::
sync() {
  size_t n = pptr() - pbase();

  size_t wrote = write_chars(pbase(), n);
  pbump(-(streamoff)wrote);

  if (n != wrote) {
    return EOF;
  }
  return 0;
}

/**
 * Called by the system istream implementation when its internal buffer needs
 * more characters.
 */
int PandaFileStreamBuf::
underflow() {
  // Sometimes underflow() is called even if the buffer is not empty.
  if (gptr() >= egptr()) {
    sync();

    // Mark the buffer filled (with buffer_size bytes).
    size_t buffer_size = egptr() - eback();
    gbump(-(streamoff)buffer_size);

    size_t num_bytes = buffer_size;
    size_t read_count = read_chars(gptr(), buffer_size);

    if (read_count != num_bytes) {
      // Oops, we didn't read what we thought we would.
      if (read_count == 0) {
        gbump(num_bytes);
        return EOF;
      }

      // Slide what we did read to the top of the buffer.
      assert(read_count < num_bytes);
      size_t delta = num_bytes - read_count;
      memmove(gptr() + delta, gptr(), read_count);
      gbump(delta);
    }
  }

  return (unsigned char)*gptr();
}

/**
 * Attempts to extract the indicated number of characters from the current
 * file position.  Returns the number of characters extracted.
 */
size_t PandaFileStreamBuf::
read_chars(char *start, size_t length) {
  if (length == 0 || !_is_open) {
    // Trivial no-op.
    return 0;
  }

  // Make sure the write buffer is flushed.
  sync();

  if (_open_mode & ios::binary) {
    // If the file is opened in binary mode, just read the data in the file.
    return read_chars_raw(start, length);
  }

  // The file is opened in text mode.  We have to decode newline characters in
  // the file.
  if (_newline_mode == NM_binary) {
    // Unless we're configured to always use binary mode.
    return read_chars_raw(start, length);
  }

  char *buffer = (char *)alloca(length);

  size_t read_length;
  size_t final_length;
  do {
    read_length = length - 1;
    if (_last_read_nl != 0) {
      // If we have a newline character to grow on, we might need to expand
      // the buffer we read from the file by one character.  In that case,
      // read one character less to make room for it.  (Otherwise, we are
      // confident that the buffer will not expand when we decode the
      // newlines.)
      --read_length;
    }
    read_length = read_chars_raw(buffer, read_length);
    final_length = decode_newlines(start, length, buffer, read_length);

    // If we decoded all of the characters away, but we read nonzero
    // characters, go back and get some more.
  } while (read_length != 0 && final_length == 0);

  return final_length;
}

/**
 * Outputs the indicated stream of characters to the current file position.
 */
size_t PandaFileStreamBuf::
write_chars(const char *start, size_t length) {
  if (length == 0) {
    // Trivial no-op.
    return 0;
  }

  // Make sure the read buffer is flushed.
  size_t n = egptr() - gptr();
  gbump(n);
  _gpos -= n;
  assert(_gpos >= 0);

  // Windows case.
  if (_open_mode & ios::binary) {
    // If the file is opened in binary mode, just write the data to the file.
    return write_chars_raw(start, length);
  }

  // The file is opened in text mode.  We have to encode newline characters to
  // the file.

  NewlineMode this_newline_mode = _newline_mode;
  if (this_newline_mode == NM_native) {
#ifdef _WIN32
    this_newline_mode = NM_msdos;
#else
    // Even the Mac uses Unix-style EOL characters these days.
    this_newline_mode = NM_unix;
#endif
  }

  if (this_newline_mode == NM_binary) {
    return write_chars_raw(start, length);
  }

  size_t buffer_length = length;
  if (this_newline_mode == NM_msdos) {
    // Windows requires a larger buffer here, since we are writing two newline
    // characters for every one.
    buffer_length *= 2;
  }
  char *buffer = (char *)alloca(buffer_length);

  size_t write_length;
  switch (this_newline_mode) {
  case NM_msdos:
    write_length = encode_newlines_msdos(buffer, buffer_length, start, length);
    break;

  case NM_mac:
    write_length = encode_newlines_mac(buffer, buffer_length, start, length);
    break;

  default:
    write_length = encode_newlines_unix(buffer, buffer_length, start, length);
    break;
  }

  if (write_length == write_chars_raw(buffer, write_length)) {
    // Success.  Return the number of original characters.
    return length;
  }

  // Error.  Pretend we wrote nothing.
  return 0;
}

/**
 * Reads raw data from the file directly into the indicated buffer.  Returns
 * the number of characters read.
 */
size_t PandaFileStreamBuf::
read_chars_raw(char *start, size_t length) {
  if (length == 0) {
    return 0;
  }

#ifdef _WIN32
  // Windows case.
  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(overlapped));
  LARGE_INTEGER gpos;
  gpos.QuadPart = _gpos;
  overlapped.Offset = gpos.LowPart;
  overlapped.OffsetHigh = gpos.HighPart;

  DWORD bytes_read = 0;
  BOOL success = ReadFile(_handle, start, length, &bytes_read, &overlapped);
  int pass = 0;
  while (!success) {
    DWORD error = GetLastError();
    if (error == ERROR_IO_INCOMPLETE || error == ERROR_IO_PENDING) {
      // Wait for more later, but don't actually yield until we have made the
      // first call to GetOverlappedResult().  (Apparently, Vista and Windows
      // 7 *always* return ERROR_IO_INCOMPLETE after the first call to
      // ReadFile.)
      if (pass > 0) {
        thread_yield();
      }
    } else if (error == ERROR_HANDLE_EOF || error == ERROR_BROKEN_PIPE) {
      // End-of-file, normal result.
      break;
    } else {
      cerr
        << "Error reading " << length
        << " bytes from " << _filename << ", windows error code 0x" << hex
        << error << dec << ".\n";
      return 0;
    }
    ++pass;
    success = GetOverlappedResult(_handle, &overlapped, &bytes_read, false);
  }

  length = bytes_read;

#else
  // Posix case.
  if (lseek(_fd, _gpos, SEEK_SET) == -1) {
    cerr
      << "Error seeking to position " << _gpos << " in " << _filename << "\n";
    return 0;
  }

  int result = ::read(_fd, start, length);
  while (result < 0) {
    if (errno == EAGAIN) {
      thread_yield();
    } else {
      cerr
        << "Error reading " << length << " bytes from " << _filename << "\n";
      return 0;
    }
    result = ::read(_fd, start, length);
  }

  length = result;
#endif  // _WIN32

  _gpos += length;
  assert(_gpos >= 0);
  return length;
}

/**
 * Writes the indicated buffer directly to the file stream.  Returns the
 * number of characters written.
 */
size_t PandaFileStreamBuf::
write_chars_raw(const char *start, size_t length) {
  if (length == 0 || !_is_open) {
    return 0;
  }

#ifdef _WIN32
  // Windows case.
  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(overlapped));
  LARGE_INTEGER ppos;
  ppos.QuadPart = _ppos;
  overlapped.Offset = ppos.LowPart;
  overlapped.OffsetHigh = ppos.HighPart;

  if (_open_mode & ios::app) {
    overlapped.Offset = -1;
    overlapped.OffsetHigh = -1;
  }

  DWORD bytes_written = 0;
  BOOL success = WriteFile(_handle, start, length, &bytes_written, &overlapped);
  int pass = 0;
  while (!success) {
    DWORD error = GetLastError();
    if (error == ERROR_IO_INCOMPLETE || error == ERROR_IO_PENDING) {
      // Wait for more later, but don't actually yield until we have made the
      // first call to GetOverlappedResult().  (Apparently, Vista and Windows
      // 7 *always* return ERROR_IO_INCOMPLETE after the first call to
      // WriteFile.)
      if (pass > 0) {
        thread_yield();
      }
    } else if (error == ERROR_BROKEN_PIPE) {
      // Broken pipe, we're done.
      cerr << "Pipe closed on " << _filename << "\n";
      return bytes_written;
    } else {
      cerr
        << "Error writing " << length
        << " bytes to " << _filename << ", windows error code 0x" << hex
        << error << dec << ".  Disk full?\n";
      return bytes_written;
    }
    ++pass;
    success = GetOverlappedResult(_handle, &overlapped, &bytes_written, false);
  }
  assert(bytes_written == length);
  _ppos += bytes_written;
  assert(_ppos >= 0);

#else
  // Posix case.
  if (!(_open_mode & ios::app)) {
    if (lseek(_fd, _ppos, SEEK_SET) == -1) {
      cerr
        << "Error seeking to position " << _ppos << " in " << _filename << "\n";
      return 0;
    }
  }

  size_t remaining = length;
  while (remaining > 0) {
    int result = ::write(_fd, start, remaining);
    if (result < 0) {
      if (errno == EAGAIN) {
        thread_yield();
      } else {
        cerr
          << "Error writing " << remaining << " bytes to " << _filename << "\n";
        return length - remaining;
      }
      continue;
    }

    start += result;
    remaining -= result;
    _ppos += result;
    assert(_ppos >= 0);
  }
#endif  // _WIN32

  return length;
}

/**
 * Converts a buffer from universal newlines to \n.
 *
 * Returns the number of characters placed in dest.  This may also set (or
 * read) the value of _last_read_nl, which is preserved from call-to-call to
 * deal with newline combinations that straddle a read operation.
 */
size_t PandaFileStreamBuf::
decode_newlines(char *dest, size_t dest_length,
                const char *source, size_t source_length) {
  const char *p = source;  // Read from p
  char *q = dest;          // Write to q

  if (source_length == 0) {
    // A special case: this is at end-of-file.  Resolve the hanging newline.
    switch (_last_read_nl) {
    case '\n':
    case '\r':
      // Close the newline to grow on.
      assert(q < dest + dest_length);
      *q++ = '\n';
      _last_read_nl = 0;
      break;
    default:
      break;
    }
  }

  while (p < source + source_length) {
    assert(q < dest + dest_length);
    switch (*p) {
    case '\n':
      switch (_last_read_nl) {
      case '\r':
        // \r\n is an MS-DOS newline.
        *q++ = '\n';
        _last_read_nl = 0;
        break;
      case '\n':
        // \n\n means one Unix newline, and one more to grow on.
        *q++ = '\n';
        _last_read_nl = '\n';
        break;
      default:
        // A lone \n is a newline to grow on.
        _last_read_nl = '\n';
        break;
      }
      break;

    case '\r':
      switch (_last_read_nl) {
      case '\n':
        // \n\r is an MS-DOS newline.
        *q++ = '\n';
        _last_read_nl = 0;
        break;
      case '\r':
        // \r\r means one Apple newline, and one more to grow on.
        *q++ = '\n';
        _last_read_nl = '\r';
        break;
      default:
        // A lone \r is a newline to grow on.
        _last_read_nl = '\r';
        break;
      }
      break;

    default:
      switch (_last_read_nl) {
      case '\n':
      case '\r':
        // Close the newline to grow on.
        *q++ = '\n';
        _last_read_nl = 0;
        break;
      default:
        break;
      }
      assert(q < dest + dest_length);
      *q++ = *p;
    }
    ++p;
  }

  return q - dest;
}

/**
 * Windows case: Converts a buffer from \n to \n\r.
 *
 * To allow for full buffer expansion, dest_length should be at least
 * 2*source_length.
 *
 * Returns the number of characters placed in dest.
 */
size_t PandaFileStreamBuf::
encode_newlines_msdos(char *dest, size_t dest_length,
                      const char *source, size_t source_length) {
  const char *p = source;  // Read from p
  char *q = dest;          // Write to q

  while (p < source + source_length) {
    assert(q < dest + dest_length);
    switch (*p) {
    case '\n':
      *q++ = '\r';
      assert(q < dest + dest_length);
      *q++ = '\n';
      break;

    case '\r':
      // Huh, shouldn't have one of these.
      break;

    default:
      *q++ = *p;
      break;
    }

    ++p;
  }

  return q - dest;
}

/**
 * Unix case: Converts a buffer from \n to \n.
 *
 * This is, of course, no conversion at all; but we do strip out \r characters
 * if they appear in the buffer; this will help programmers to realize when
 * they have incorrectly tagged a binary file with text mode, even on a Posix
 * environment.
 *
 * Returns the number of characters placed in dest.
 */
size_t PandaFileStreamBuf::
encode_newlines_unix(char *dest, size_t dest_length,
                     const char *source, size_t source_length) {
  const char *p = source;  // Read from p
  char *q = dest;          // Write to q

  while (p < source + source_length) {
    assert(q < dest + dest_length);
    switch (*p) {
    case '\r':
      break;

    default:
      *q++ = *p;
      break;
    }

    ++p;
  }

  return q - dest;
}

/**
 * Classic Mac case: Converts a buffer from \n to \r.
 *
 * Returns the number of characters placed in dest.
 */
size_t PandaFileStreamBuf::
encode_newlines_mac(char *dest, size_t dest_length,
                    const char *source, size_t source_length) {
  const char *p = source;  // Read from p
  char *q = dest;          // Write to q

  while (p < source + source_length) {
    assert(q < dest + dest_length);
    switch (*p) {
    case '\n':
      *q++ = '\r';
      break;

    case '\r':
      break;

    default:
      *q++ = *p;
      break;
    }

    ++p;
  }

  return q - dest;
}

ostream &
operator << (ostream &out, PandaFileStreamBuf::NewlineMode newline_mode) {
  switch (newline_mode) {
  case PandaFileStreamBuf::NM_native:
    return out << "native";

  case PandaFileStreamBuf::NM_binary:
    return out << "binary";

  case PandaFileStreamBuf::NM_msdos:
    return out << "msdos";

  case PandaFileStreamBuf::NM_unix:
    return out << "unix";

  case PandaFileStreamBuf::NM_mac:
    return out << "mac";
  }

  cerr
    << "Invalid NewlineMode value: " << (int)newline_mode << "\n";
  return out;
}

istream &
operator >> (istream &in, PandaFileStreamBuf::NewlineMode &newline_mode) {
  string word;
  in >> word;

  if (word == "native") {
    newline_mode = PandaFileStreamBuf::NM_native;
  } else if (word == "binary") {
    newline_mode = PandaFileStreamBuf::NM_binary;
  } else if (word == "msdos") {
    newline_mode = PandaFileStreamBuf::NM_msdos;
  } else if (word == "unix") {
    newline_mode = PandaFileStreamBuf::NM_unix;
  } else if (word == "mac") {
    newline_mode = PandaFileStreamBuf::NM_mac;
  } else {
    cerr
      << "Invalid NewlineMode value: " << word << "\n";
    newline_mode = PandaFileStreamBuf::NM_native;
  }

  return in;
}

#endif  // USE_PANDAFILESTREAM
