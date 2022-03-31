/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaFileStream.I
 * @author drose
 * @date 2008-09-08
 */

/**
 *
 */
INLINE IFileStream::
IFileStream() : std::istream(&_buf) {
}

/**
 *
 */
INLINE IFileStream::
IFileStream(const char *filename, std::ios::openmode mode) : std::istream(&_buf) {
  open(filename, mode);
}

/**
 *
 */
INLINE IFileStream::
~IFileStream() {
  close();
}

/**
 *
 */
INLINE void IFileStream::
open(const char *filename, std::ios::openmode mode) {
  clear((ios_iostate)0);
  _buf.open(filename, mode);
  if (!_buf.is_open()) {
    clear(std::ios::failbit);
  }
}

#ifdef _WIN32
/**
 * Connects the file stream to the existing OS-defined stream, presumably
 * opened via a low-level OS call.  The filename is for reporting only.  When
 * the file stream is closed, it will also close the underlying OS handle.
 *
 * This function is the Windows-specific variant.
 */
void IFileStream::
attach(const char *filename, HANDLE handle, std::ios::openmode mode) {
  clear((ios_iostate)0);
  _buf.attach(filename, handle, mode);
  if (!_buf.is_open()) {
    clear(std::ios::failbit);
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
void IFileStream::
attach(const char *filename, int fd, std::ios::openmode mode) {
  clear((ios_iostate)0);
  _buf.attach(filename, fd, mode);
  if (!_buf.is_open()) {
    clear(std::ios::failbit);
  }
}
#endif  // _WIN32

/**
 *
 */
INLINE void IFileStream::
close() {
  _buf.close();
}

/**
 *
 */
INLINE OFileStream::
OFileStream() : std::ostream(&_buf) {
}

/**
 *
 */
INLINE OFileStream::
OFileStream(const char *filename, std::ios::openmode mode) : std::ostream(&_buf) {
  open(filename, mode);
}

/**
 *
 */
INLINE OFileStream::
~OFileStream() {
  close();
}

/**
 *
 */
INLINE void OFileStream::
open(const char *filename, std::ios::openmode mode) {
  clear((ios_iostate)0);
  _buf.open(filename, mode);
  if (!_buf.is_open()) {
    clear(std::ios::failbit);
  }
}

#ifdef _WIN32
/**
 * Connects the file stream to the existing OS-defined stream, presumably
 * opened via a low-level OS call.  The filename is for reporting only.  When
 * the file stream is closed, it will also close the underlying OS handle.
 *
 * This function is the Windows-specific variant.
 */
void OFileStream::
attach(const char *filename, HANDLE handle, std::ios::openmode mode) {
  clear((ios_iostate)0);
  _buf.attach(filename, handle, mode);
  if (!_buf.is_open()) {
    clear(std::ios::failbit);
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
void OFileStream::
attach(const char *filename, int fd, std::ios::openmode mode) {
  clear((ios_iostate)0);
  _buf.attach(filename, fd, mode);
  if (!_buf.is_open()) {
    clear(std::ios::failbit);
  }
}
#endif  // _WIN32

/**
 *
 */
INLINE void OFileStream::
close() {
  _buf.close();
}

/**
 *
 */
INLINE FileStream::
FileStream() : std::iostream(&_buf) {
}

/**
 *
 */
INLINE FileStream::
FileStream(const char *filename, std::ios::openmode mode) : std::iostream(&_buf) {
  open(filename, mode);
}

/**
 *
 */
INLINE FileStream::
~FileStream() {
  close();
}

/**
 *
 */
INLINE void FileStream::
open(const char *filename, std::ios::openmode mode) {
  clear((ios_iostate)0);
  _buf.open(filename, mode);
  if (!_buf.is_open()) {
    clear(std::ios::failbit);
  }
}

#ifdef _WIN32
/**
 * Connects the file stream to the existing OS-defined stream, presumably
 * opened via a low-level OS call.  The filename is for reporting only.  When
 * the file stream is closed, it will also close the underlying OS handle.
 *
 * This function is the Windows-specific variant.
 */
void FileStream::
attach(const char *filename, HANDLE handle, std::ios::openmode mode) {
  clear((ios_iostate)0);
  _buf.attach(filename, handle, mode);
  if (!_buf.is_open()) {
    clear(std::ios::failbit);
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
void FileStream::
attach(const char *filename, int fd, std::ios::openmode mode) {
  clear((ios_iostate)0);
  _buf.attach(filename, fd, mode);
  if (!_buf.is_open()) {
    clear(std::ios::failbit);
  }
}
#endif  // _WIN32

/**
 *
 */
INLINE void FileStream::
close() {
  _buf.close();
}
