// Filename: wordWrapStreamBuf.h
// Created by:  drose (28Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef WORDWRAPSTREAMBUF_H
#define WORDWRAPSTREAMBUF_H

#include <pandatoolbase.h>

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
