// Filename: pandaFileStreamBuf.cxx
// Created by:  drose (08Sep08)
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

#include "pandaFileStreamBuf.h"
#include "memoryHook.h"

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#endif  // _WIN32

static const size_t file_buffer_size = 4096;

////////////////////////////////////////////////////////////////////
//     Function: PandaFileStreamBuf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaFileStreamBuf::
PandaFileStreamBuf() {
  _is_open = false;
  _open_mode = (ios::openmode)0;
  
#ifdef _WIN32
  // Windows case.
  _handle = NULL;
#else
  _fd = -1;
#endif  // _WIN32

#ifdef HAVE_IOSTREAM
  _buffer = (char *)PANDA_MALLOC_ARRAY(file_buffer_size * 2);
  char *ebuf = _buffer + file_buffer_size * 2;
  char *mbuf = _buffer + file_buffer_size;
  setg(_buffer, mbuf, mbuf);
  setp(mbuf, ebuf);

#else
  allocate();
  // Chop the buffer in half.  The bottom half goes to the get buffer;
  // the top half goes to the put buffer.
  char *b = base();
  char *t = ebuf();
  char *m = b + (t - b) / 2;
  setg(b, m, m);
  setp(b, m);
#endif

  _gpos = 0;
  _ppos = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFileStreamBuf::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PandaFileStreamBuf::
~PandaFileStreamBuf() {
  close();
#ifdef HAVE_IOSTREAM
  PANDA_FREE_ARRAY(_buffer);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFileStreamBuf::open
//       Access: Public
//  Description: Attempts to open the file for input and/or output.
////////////////////////////////////////////////////////////////////
void PandaFileStreamBuf::
open(const char *filename, ios::openmode mode) {
  close();

  _filename = filename;
  _open_mode = mode;
  _is_open = false;

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
  // In SIMPLE_THREADS mode, we use "overlapped" I/O.
  flags |= FILE_FLAG_OVERLAPPED;
#endif

  _handle = CreateFile(_filename.c_str(), access, share_mode,
                       NULL, creation_disposition, flags, NULL);
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
  // In SIMPLE_THREADS mode, we use non-blocking I/O.
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

////////////////////////////////////////////////////////////////////
//     Function: PandaFileStreamBuf::is_open
//       Access: Public
//  Description: Returns true if the file is open, false otherwise.
////////////////////////////////////////////////////////////////////
bool PandaFileStreamBuf::
is_open() const {
  return _is_open;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFileStreamBuf::close
//       Access: Public
//  Description: Empties the buffer and closes the file.
////////////////////////////////////////////////////////////////////
void PandaFileStreamBuf::
close() {
  // Make sure the write buffer is flushed.
  sync();

#ifdef _WIN32
  if (_handle != NULL) {
    CloseHandle(_handle);
  }
  _handle = NULL;
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

////////////////////////////////////////////////////////////////////
//     Function: PandaFileStreamBuf::seekoff
//       Access: Public, Virtual
//  Description: Implements seeking within the stream.
////////////////////////////////////////////////////////////////////
streampos PandaFileStreamBuf::
seekoff(streamoff off, ios_seekdir dir, ios_openmode which) {
  streampos result = -1;

  // Sync the iostream buffer first.
  sync();

  if (which & ios::in) {
    // Determine the current file position.
    size_t n = egptr() - gptr();
    gbump(n);
    _gpos -= n;
    size_t cur_pos = _gpos;
    size_t new_pos = cur_pos;
    
    // Now adjust the data pointer appropriately.
    switch (dir) {
    case ios::beg:
      new_pos = (size_t)off;
      break;
      
    case ios::cur:
      new_pos = (size_t)((int)cur_pos + off);
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
      if (new_pos == -1) {
        return -1;
      }
#endif  // _WIN32
      break;

    default:
      // Shouldn't get here.
      break;
    }

    _gpos = new_pos;
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
      new_pos = (streampos)((int)cur_pos + off);
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
    result = new_pos;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFileStreamBuf::seekpos
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
streampos PandaFileStreamBuf::
seekpos(streampos pos, ios_openmode which) {
  return seekoff(pos, ios::beg, which);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFileStreamBuf::overflow
//       Access: Protected, Virtual
//  Description: Called by the system ostream implementation when its
//               internal buffer is filled, plus one character.
////////////////////////////////////////////////////////////////////
int PandaFileStreamBuf::
overflow(int ch) {
  bool okflag = true;

  size_t n = pptr() - pbase();
  if (n != 0) {
    size_t wrote = write_chars(pbase(), n);
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

  if (!okflag) {
    return EOF;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFileStreamBuf::sync
//       Access: Protected, Virtual
//  Description: Called by the system iostream implementation to
//               implement a flush operation.
////////////////////////////////////////////////////////////////////
int PandaFileStreamBuf::
sync() {
  size_t n = pptr() - pbase();

  size_t wrote = write_chars(pbase(), n);
  pbump(-(int)wrote);

  if (n != wrote) {
    return EOF;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFileStreamBuf::underflow
//       Access: Protected, Virtual
//  Description: Called by the system istream implementation when its
//               internal buffer needs more characters.
////////////////////////////////////////////////////////////////////
int PandaFileStreamBuf::
underflow() {
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


////////////////////////////////////////////////////////////////////
//     Function: PandaFileStreamBuf::read_chars
//       Access: Private
//  Description: Attempts to extract the indicated number of
//               characters from the current file position.  Returns
//               the number of characters extracted.
////////////////////////////////////////////////////////////////////
size_t PandaFileStreamBuf::
read_chars(char *start, size_t length) {
  if (length == 0) {
    return 0;
  }

  // Make sure the write buffer is flushed.
  sync();
  
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
  while (!success) {
    DWORD error = GetLastError();
    if (error == ERROR_IO_INCOMPLETE || error == ERROR_IO_PENDING) {
      // Wait for more later.
      thread_yield();
    } else if (error == ERROR_HANDLE_EOF) {
      // End-of-file, normal result.
      break;
    } else {
      cerr
        << "Error reading " << length
        << " bytes from " << _filename << ", windows error code 0x" << hex
        << error << dec << ".\n";
      return 0;
    }
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
  
  ssize_t result = ::read(_fd, start, length);
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
  return length;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFileStreamBuf::write_chars
//       Access: Private
//  Description: Outputs the indicated stream of characters to the
//               current file position.
////////////////////////////////////////////////////////////////////
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
  
#ifdef _WIN32
  // Windows case.
  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(overlapped));
  LARGE_INTEGER ppos;
  ppos.QuadPart = _ppos;
  overlapped.Offset = ppos.LowPart;
  overlapped.OffsetHigh = ppos.HighPart;
  
  DWORD bytes_written = 0;
  BOOL success = WriteFile(_handle, start, length, &bytes_written, &overlapped);
  while (!success) {
    DWORD error = GetLastError();
    if (error == ERROR_IO_INCOMPLETE || error == ERROR_IO_PENDING) {
      // Wait for more later.
      thread_yield();
    } else {
      cerr
        << "Error writing " << length
        << " bytes to " << _filename << ", windows error code 0x" << hex
        << error << dec << ".  Disk full?\n";
      return bytes_written;
    }
    success = GetOverlappedResult(_handle, &overlapped, &bytes_written, false);
  }
  assert(bytes_written == length);
  _ppos += bytes_written;
  
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
    ssize_t result = ::write(_fd, start, remaining);
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
  }
#endif  // _WIN32

  return length;
}
