/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wordWrapStreamBuf.h
 * @author drose
 * @date 2000-06-28
 */

#ifndef WORDWRAPSTREAMBUF_H
#define WORDWRAPSTREAMBUF_H

#include "pandatoolbase.h"

#include <string>

class ProgramBase;
class WordWrapStream;

/**
 * Used by WordWrapStream to implement an ostream that flushes its output to
 * ProgramBase::show_text().
 */
class WordWrapStreamBuf : public std::streambuf {
public:
  WordWrapStreamBuf(WordWrapStream *owner, ProgramBase *program);
  virtual ~WordWrapStreamBuf();

protected:
  virtual int overflow(int c);
  virtual int sync();

private:
  void write_chars(const char *start, int length);
  INLINE void set_literal_mode(bool mode);
  void flush_data();

  std::string _data;
  WordWrapStream *_owner;
  ProgramBase *_program;
  bool _literal_mode;
};

#include "wordWrapStreamBuf.I"

#endif
