// Filename: handleStreamBuf.cxx
// Created by:  drose (05Jun09)
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

#include "handleStreamBuf.h"

#include <assert.h>
#include <string.h>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#endif  // _WIN32

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__FreeBSD__)
#include <libio.h>
#endif // !_WIN32 && !__APPLE__ && !__FreeBSD__

static const size_t handle_buffer_size = 4096;

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
HandleStreamBuf::
HandleStreamBuf() {
  _is_open_read = false;
  _is_open_write = false;
  
  _handle = invalid_fhandle;

  INIT_LOCK(_lock);

  _buffer = new char[handle_buffer_size];
  char *ebuf = _buffer + handle_buffer_size;
  setg(_buffer, ebuf, ebuf);
  setp(_buffer, ebuf);
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
HandleStreamBuf::
~HandleStreamBuf() {
  close();

  delete[] _buffer;

  DESTROY_LOCK(_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::open_read
//       Access: Public
//  Description: Attempts to open the given handle for input.  The
//               stream may not be simultaneously open for input and
//               output.
////////////////////////////////////////////////////////////////////
void HandleStreamBuf::
open_read(FHandle handle) {
  close();

  _handle = handle;
  _is_open_read = true;
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::open_write
//       Access: Public
//  Description: Attempts to open the given handle for output.  The
//               stream may not be simultaneously open for input and
//               output.
////////////////////////////////////////////////////////////////////
void HandleStreamBuf::
open_write(FHandle handle) {
  close();

  _handle = handle;
  _is_open_write = true;
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::is_open_read
//       Access: Public
//  Description: Returns true if the file is open for input, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool HandleStreamBuf::
is_open_read() const {
  return _is_open_read;
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::is_open_write
//       Access: Public
//  Description: Returns true if the file is open for output, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool HandleStreamBuf::
is_open_write() const {
  return _is_open_write;
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::close
//       Access: Public
//  Description: Empties the buffer and closes the file.
////////////////////////////////////////////////////////////////////
void HandleStreamBuf::
close() {
  // Make sure the write buffer is flushed.
  sync();

  close_handle();

  pbump(pbase() - pptr());
  gbump(egptr() - gptr());
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::close_handle
//       Access: Public
//  Description: Closes the underlying handle, *without* attempting to
//               flush the stream.
////////////////////////////////////////////////////////////////////
void HandleStreamBuf::
close_handle() {
#ifdef _WIN32
  if (_handle != NULL) {
    CloseHandle(_handle);
  }
  _handle = NULL;
#else
  if (_handle != -1) {
    ::close(_handle);
  }
  _handle = -1;
#endif  // _WIN32

  _is_open_read = false;
  _is_open_write = false;
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::overflow
//       Access: Protected, Virtual
//  Description: Called by the system ostream implementation when its
//               internal buffer is filled, plus one character.
////////////////////////////////////////////////////////////////////
int HandleStreamBuf::
overflow(int ch) {
  ACQUIRE_LOCK(_lock);

  bool okflag = true;

  assert(pptr() >= pbase());
  size_t n = pptr() - pbase();
  if (n != 0) {
    size_t wrote = write_chars(pbase(), n);
    assert(wrote <= n);
    pbump(-(int)wrote);
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

  RELEASE_LOCK(_lock);

  if (!okflag) {
    return EOF;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::sync
//       Access: Protected, Virtual
//  Description: Called by the system iostream implementation to
//               implement a flush operation.
////////////////////////////////////////////////////////////////////
int HandleStreamBuf::
sync() {
  ACQUIRE_LOCK(_lock);
  assert(pptr() >= pbase());
  size_t n = pptr() - pbase();

  size_t wrote = write_chars(pbase(), n);
  assert(wrote <= n);
  pbump(-(int)wrote);

  RELEASE_LOCK(_lock);

  if (n != wrote) {
    return EOF;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::underflow
//       Access: Protected, Virtual
//  Description: Called by the system istream implementation when its
//               internal buffer needs more characters.
////////////////////////////////////////////////////////////////////
int HandleStreamBuf::
underflow() {
  ACQUIRE_LOCK(_lock);
  // Sometimes underflow() is called even if the buffer is not empty.
  if (gptr() >= egptr()) {
    // Mark the buffer filled (with buffer_size bytes).
    size_t buffer_size = egptr() - eback();
    gbump(-(int)buffer_size);

    size_t num_bytes = buffer_size;
    size_t read_count = read_chars(gptr(), buffer_size);

    if (read_count != num_bytes) {
      // Oops, we didn't read what we thought we would.
      if (read_count == 0) {
        gbump(num_bytes);
        RELEASE_LOCK(_lock);
        return EOF;
      }

      // Slide what we did read to the top of the buffer.
      assert(read_count < num_bytes);
      size_t delta = num_bytes - read_count;
      memmove(gptr() + delta, gptr(), read_count);
      gbump(delta);
    }
  }

  unsigned char next = *gptr();
  RELEASE_LOCK(_lock);

  return next;
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::read_chars
//       Access: Private
//  Description: Attempts to extract the indicated number of
//               characters from the current file position.  Returns
//               the number of characters extracted.
////////////////////////////////////////////////////////////////////
size_t HandleStreamBuf::
read_chars(char *start, size_t length) {
  if (length == 0 || !_is_open_read) {
    // Trivial no-op.
    return 0;
  }

  // Make sure the write buffer is flushed.
  sync();

  if (length == 0) {
    return 0;
  }
  
#ifdef _WIN32
  // Windows case.
  DWORD bytes_read = 0;
  BOOL success = ReadFile(_handle, start, length, &bytes_read, NULL);
  if (!success) {
    DWORD error = GetLastError();
    if (error != ERROR_HANDLE_EOF && error != ERROR_BROKEN_PIPE) {
      cerr << "Error reading " << length
           << " bytes, windows error code 0x" << hex
           << error << dec << ".\n";
      return 0;
    }
  }

  length = bytes_read;
  
#else
  // Posix case.
  ssize_t result = ::read(_handle, start, length);
  if (result < 0) {
    cerr << "Error reading " << length << " bytes\n";
    return 0;
  }

  length = result;
#endif  // _WIN32

  return length;
}

////////////////////////////////////////////////////////////////////
//     Function: HandleStreamBuf::write_chars
//       Access: Private
//  Description: Outputs the indicated stream of characters to the
//               current file position.
////////////////////////////////////////////////////////////////////
size_t HandleStreamBuf::
write_chars(const char *start, size_t length) {
  if (length == 0) {
    // Trivial no-op.
    return 0;
  }

  // Make sure the read buffer is flushed.
  size_t n = egptr() - gptr();
  gbump(n);

  if (length == 0 || !_is_open_write) {
    return 0;
  }
  
#ifdef _WIN32
  // Windows case.
  DWORD bytes_written = 0;
  BOOL success = WriteFile(_handle, start, length, &bytes_written, NULL);
  if (!success) {
    assert(bytes_written <= length);
    DWORD error = GetLastError();
    if (error != ERROR_NO_DATA && error != ERROR_BROKEN_PIPE) {
      cerr << "Error writing " << length
           << " bytes, windows error code 0x" << hex
           << error << dec << ".\n";
    }
    return bytes_written;
  }
  assert(bytes_written == length);
  
#else
  // Posix case.
  size_t remaining = length;
  while (remaining > 0) {
    ssize_t result = ::write(_handle, start, remaining);
    if (result < 0) {
      if (errno != EPIPE) {
        cerr << "Error writing " << remaining << " bytes\n";
      }
      return length - remaining;
    }
    
    start += result;
    remaining -= result;
  }
#endif  // _WIN32

  return length;
}
