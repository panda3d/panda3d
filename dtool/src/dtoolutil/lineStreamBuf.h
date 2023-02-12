/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lineStreamBuf.h
 * @author drose
 * @date 2000-02-26
 */

#ifndef LINESTREAMBUF_H
#define LINESTREAMBUF_H

#include "dtoolbase.h"

#include <string>

/**
 * Used by LineStream to implement an ostream that writes to a memory buffer,
 * whose contents can be continuously extracted as a sequence of lines of
 * text.
 */
class EXPCL_DTOOL_DTOOLUTIL LineStreamBuf final : public std::streambuf {
public:
  LineStreamBuf();
  virtual ~LineStreamBuf();

  INLINE bool is_text_available() const;
  std::string get_line();
  INLINE bool has_newline() const;

protected:
  virtual int overflow(int c);
  virtual int sync();

private:
  INLINE void write_chars(const char *start, size_t length);

  std::string _data;
  bool _has_newline;
};

#include "lineStreamBuf.I"

#endif
