/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaFileStreamBuf.h
 * @author drose
 * @date 2008-09-08
 */

#ifndef PANDAFILESTREAMBUF_H
#define PANDAFILESTREAMBUF_H

#include "dtoolbase.h"

#ifdef USE_PANDAFILESTREAM

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

/**
 * The streambuf object that implements pifstream and pofstream.
 */
class EXPCL_DTOOL_DTOOLUTIL PandaFileStreamBuf : public std::streambuf {
public:
  PandaFileStreamBuf();
  virtual ~PandaFileStreamBuf();

  void open(const char *filename, std::ios::openmode mode);
#ifdef _WIN32
  void attach(const char *filename, HANDLE handle, std::ios::openmode mode);
#else
  void attach(const char *filename, int fd, std::ios::openmode mode);
#endif

  bool is_open() const;
  void close();

  enum NewlineMode {
    NM_native,
    NM_binary,
    NM_msdos,
    NM_unix,
    NM_mac,
  };
  static NewlineMode _newline_mode;

protected:
  virtual std::streampos seekoff(std::streamoff off, ios_seekdir dir, ios_openmode which);
  virtual std::streampos seekpos(std::streampos pos, ios_openmode which);

  virtual int overflow(int c);
  virtual int sync();
  virtual int underflow();

private:
  size_t read_chars(char *start, size_t length);
  size_t write_chars(const char *start, size_t length);

  size_t read_chars_raw(char *start, size_t length);
  size_t write_chars_raw(const char *start, size_t length);

  size_t decode_newlines(char *dest, size_t dest_length,
                         const char *source, size_t source_length);

  size_t encode_newlines_msdos(char *dest, size_t dest_length,
                               const char *source, size_t source_length);
  size_t encode_newlines_unix(char *dest, size_t dest_length,
                              const char *source, size_t source_length);
  size_t encode_newlines_mac(char *dest, size_t dest_length,
                             const char *source, size_t source_length);

private:
  std::string _filename;
  bool _is_open;
  std::ios::openmode _open_mode;

  char _last_read_nl;

#ifdef _WIN32
  HANDLE _handle;
#else
  int _fd;  // Posix file descriptor
#endif  // _WIN32

  char *_buffer;
  std::streampos _ppos;
  std::streampos _gpos;
};

EXPCL_DTOOL_DTOOLUTIL std::ostream &
operator << (std::ostream &out, PandaFileStreamBuf::NewlineMode newline_mode);

EXPCL_DTOOL_DTOOLUTIL std::istream &
operator >> (std::istream &in, PandaFileStreamBuf::NewlineMode &newline_mode);

#endif  // USE_PANDAFILESTREAM

#endif
