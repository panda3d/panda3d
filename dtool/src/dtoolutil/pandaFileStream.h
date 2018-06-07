/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaFileStream.h
 * @author drose
 * @date 2008-09-08
 */

#ifndef PANDAFILESTREAM_H
#define PANDAFILESTREAM_H

#include "dtoolbase.h"

#ifdef USE_PANDAFILESTREAM

#include "pandaFileStreamBuf.h"

/**
 * Implements a C++ stream object suitable for reading from files on disk.
 * This is similar to ifstream, but it provides low-level support for Panda's
 * simple-threading implementation (using this interface will block only the
 * current thread, rather than the entire process, on I/O waits).
 */
class EXPCL_DTOOL_DTOOLUTIL IFileStream : public std::istream {
PUBLISHED:
  INLINE IFileStream();
  INLINE explicit IFileStream(const char *filename, std::ios::openmode mode = std::ios::in);
  INLINE ~IFileStream();

  INLINE void open(const char *filename, std::ios::openmode mode = std::ios::in);

public:
#ifdef _WIN32
  INLINE void attach(const char *filename, HANDLE handle, std::ios::openmode mode = std::ios::in);
#else
  INLINE void attach(const char *filename, int fd, std::ios::openmode mode = std::ios::in);
#endif

PUBLISHED:
  INLINE void close();

private:
  PandaFileStreamBuf _buf;
};

/**
 * Implements a C++ stream object suitable for writing to files on disk.  This
 * is similar to ofstream, but it provides low-level support for Panda's
 * simple-threading implementation (using this interface will block only the
 * current thread, rather than the entire process, on I/O waits).
 */
class EXPCL_DTOOL_DTOOLUTIL OFileStream : public std::ostream {
PUBLISHED:
  INLINE OFileStream();
  INLINE explicit OFileStream(const char *filename, std::ios::openmode mode = std::ios::out);
  INLINE ~OFileStream();

  INLINE void open(const char *filename, std::ios::openmode mode = std::ios::out);

public:
#ifdef _WIN32
  INLINE void attach(const char *filename, HANDLE handle, std::ios::openmode mode = std::ios::out);
#else
  INLINE void attach(const char *filename, int fd, std::ios::openmode mode = std::ios::out);
#endif

PUBLISHED:
  INLINE void close();

private:
  PandaFileStreamBuf _buf;
};

/**
 * Implements a C++ stream object suitable for reading from and/or writing to
 * files on disk.  This is similar to fstream, but it provides low-level
 * support for Panda's simple-threading implementation (using this interface
 * will block only the current thread, rather than the entire process, on I/O
 * waits).
 */
class EXPCL_DTOOL_DTOOLUTIL FileStream : public std::iostream {
PUBLISHED:
  INLINE FileStream();
  INLINE explicit FileStream(const char *filename, std::ios::openmode mode = std::ios::in);
  INLINE ~FileStream();

  INLINE void open(const char *filename, std::ios::openmode mode = std::ios::in);

public:
#ifdef _WIN32
  INLINE void attach(const char *filename, HANDLE handle, std::ios::openmode mode);
#else
  INLINE void attach(const char *filename, int fd, std::ios::openmode mode);
#endif

PUBLISHED:
  INLINE void close();

private:
  PandaFileStreamBuf _buf;
};

#include "pandaFileStream.I"

typedef IFileStream pifstream;
typedef OFileStream pofstream;
typedef FileStream pfstream;

#else   // USE_PANDAFILESTREAM

typedef ifstream pifstream;
typedef ofstream pofstream;
typedef fstream pfstream;

#endif  // USE_PANDAFILESTREAM

#endif
