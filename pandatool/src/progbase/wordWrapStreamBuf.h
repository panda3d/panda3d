// Filename: wordWrapStreamBuf.h
// Created by:  drose (28Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef WORDWRAPSTREAMBUF_H
#define WORDWRAPSTREAMBUF_H

#include "pandatoolbase.h"

#include <string>

class ProgramBase;
class WordWrapStream;

////////////////////////////////////////////////////////////////////
//       Class : WordWrapStreamBuf
// Description : Used by WordWrapStream to implement an ostream that
//               flushes its output to ProgramBase::show_text().
////////////////////////////////////////////////////////////////////
class WordWrapStreamBuf : public streambuf {
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

  string _data;
  WordWrapStream *_owner;
  ProgramBase *_program;
  bool _literal_mode;
};

#include "wordWrapStreamBuf.I"

#endif
