/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file subprocessWindowBuffer.cxx
 * @author drose
 * @date 2009-07-11
 */

#include "subprocessWindowBuffer.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

using std::cerr;
using std::string;

const char SubprocessWindowBuffer::
_magic_number[SubprocessWindowBuffer::magic_number_length] = "pNdaSWB";

/**
 * Placement operator.  Returns addr, a trivial pass-through.
 */
void *SubprocessWindowBuffer::
operator new(size_t, void *addr) {
  return addr;
}

/**
 * This constructor is private; it is not intended to be called directly.  It
 * is used in make_buffer() to create a temporary local object, to determine
 * the required mmap_size for a given window size.
 */
SubprocessWindowBuffer::
SubprocessWindowBuffer(int x_size, int y_size) {
  memcpy(_this_magic, _magic_number, magic_number_length);
  _x_size = x_size;
  _y_size = y_size;
  _row_size = _x_size * 4;
  _framebuffer_size = _row_size * y_size;
  _event_in = 0;
  _event_out = 0;
  _last_written = 0;
  _last_read = 0;

  _mmap_size = sizeof(*this) + _framebuffer_size;
}

/**
 *
 */
SubprocessWindowBuffer::
SubprocessWindowBuffer(const SubprocessWindowBuffer &copy) :
  _mmap_size(copy._mmap_size),
  _x_size(copy._x_size),
  _y_size(copy._y_size),
  _row_size(copy._row_size),
  _framebuffer_size(copy._framebuffer_size)
{
  memcpy(_this_magic, _magic_number, magic_number_length);
  _event_in = 0;
  _event_out = 0;
  _last_written = 0;
  _last_read = 0;
}

/**
 *
 */
SubprocessWindowBuffer::
~SubprocessWindowBuffer() {
}

/**
 * Call this method to create a new buffer in shared memory space.  Supply the
 * desired size of the window.
 *
 * This method will create the required shared-memory buffer and return a
 * SubprocessWindowBuffer allocated within that shared memory, or NULL if
 * there is a failure allocating sufficient shared memory.
 *
 * It also creates a temporary file on disk and returns fd, mmap_size, and
 * filename, which the caller must retain and eventually pass to
 * destroy_buffer().  The filename should be passed to the child process to
 * open with open_buffer().
 */
SubprocessWindowBuffer *SubprocessWindowBuffer::
new_buffer(int &fd, size_t &mmap_size, string &filename,
           int x_size, int y_size) {
  mmap_size = 0;
  fd = -1;

  filename = tmpnam(nullptr);

  fd = open(filename.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600);
  if (fd == -1) {
    perror(filename.c_str());
    return nullptr;
  }

  // Create a temporary object to determine the required size.
  SubprocessWindowBuffer temp(x_size, y_size);
  mmap_size = temp._mmap_size;

  // Ensure the disk file is large enough.
  size_t zero_size = 1024;
  char zero[zero_size];
  memset(zero, 0, zero_size);
  for (size_t bi = 0; bi < mmap_size; bi += zero_size) {
    write(fd, zero, zero_size);
  }

  void *shared_mem = mmap(nullptr, mmap_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, fd, 0);
  if (shared_mem == (void *)-1) {
    // Failure to map.
    close(fd);
    fd = -1;
    mmap_size = 0;
    return nullptr;
  }

  // Now create the actual object in the shared-memory buffer.
  return new(shared_mem) SubprocessWindowBuffer(temp);
}

/**
 * Destroys a buffer object created via a previous call to new_buffer().  This
 * destructs objects within the buffer, unmaps the shared memory, and closes
 * the file descriptor.
 */
void SubprocessWindowBuffer::
destroy_buffer(int fd, size_t mmap_size, const string &filename,
               SubprocessWindowBuffer *buffer) {
  buffer->~SubprocessWindowBuffer();
  close_buffer(fd, mmap_size, filename, buffer);

  // Now we can unlink the file.
  unlink(filename.c_str());
}

/**
 * Call this method to open a reference to an existing buffer in shared memory
 * space.  Supply the temporary filename returned by new_buffer(), above
 * (presumably from the parent process).
 *
 * This method will mmap the required shared-memory buffer and return a
 * SubprocessWindowBuffer allocated within that shared memory, or NULL if
 * there is some failure.  The caller must retain fd, mmap_size, and filename
 * and eventually pass all three to close_buffer().
 */
SubprocessWindowBuffer *SubprocessWindowBuffer::
open_buffer(int &fd, size_t &mmap_size, const string &filename) {
  mmap_size = 0;

  fd = open(filename.c_str(), O_RDWR);
  if (fd == -1) {
    perror(filename.c_str());
    return nullptr;
  }

  // Check that the disk file is large enough.
  off_t file_size = lseek(fd, 0, SEEK_END);
  if (file_size < sizeof(SubprocessWindowBuffer)) {
    cerr << filename << " not large enough.\n";
    close(fd);
    fd = -1;
    return nullptr;
  }

  // First, map enough memory to read the buffer object.
  size_t initial_size = sizeof(SubprocessWindowBuffer);
  void *shared_mem = mmap(nullptr, initial_size, PROT_READ,
                          MAP_SHARED, fd, 0);
  if (shared_mem == (void *)-1) {
    perror("mmap");
    cerr << "Couldn't map.\n";
    close(fd);
    fd = -1;
    return nullptr;
  }

  SubprocessWindowBuffer *temp = (SubprocessWindowBuffer *)shared_mem;
  if (!temp->verify_magic_number()) {
    cerr << "Not a subprocess window buffer: " << filename << "\n";
    munmap(shared_mem, initial_size);
    close(fd);
    fd = -1;
    return nullptr;
  }


  mmap_size = temp->_mmap_size;

  // Now unmap that and remap the proper-size buffer.
  munmap(shared_mem, initial_size);

  if (file_size < mmap_size) {
    cerr << filename << " not large enough.\n";
    close(fd);
    fd = -1;
    return nullptr;
  }

  shared_mem = mmap(nullptr, mmap_size, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0);
  if (shared_mem == (void *)-1) {
    perror("mmap");
    cerr << "Couldn't map 2.\n";
    return nullptr;
  }

  // Now that we've successfully opened and mapped the file, we can safely
  // delete it from the file system.

  // Actually, unlinking it now prevents us from detaching and reattaching to
  // the same file later.  Boo.  unlink(filename.c_str());

  SubprocessWindowBuffer *buffer = (SubprocessWindowBuffer *)shared_mem;
  assert(buffer->_mmap_size == mmap_size);
  return buffer;
}

/**
 * Closes a buffer object created via a previous call to open_buffer().  This
 * unmaps the shared memory and closes the file descriptor, but does not
 * molest the shared buffer itself.
 */
void SubprocessWindowBuffer::
close_buffer(int fd, size_t mmap_size, const string &filename,
             SubprocessWindowBuffer *buffer) {
  munmap((void *)buffer, mmap_size);
  close(fd);
}

/**
 * Returns true if the buffer's magic number matches, false otherwise.
 */
bool SubprocessWindowBuffer::
verify_magic_number() const {
  return (memcmp(_this_magic, _magic_number, magic_number_length) == 0);
}
