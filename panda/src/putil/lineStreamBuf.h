// Filename: lineStreamBuf.h
// Created by:  drose (26Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINESTREAMBUF_H
#define LINESTREAMBUF_H

#include <pandabase.h>

#include <string>

////////////////////////////////////////////////////////////////////
//       Class : LineStreamBuf
// Description : Used by LineStream to implement an ostream that
//               writes to a memory buffer, whose contents can be
//               continuously extracted as a sequence of lines of
//               text.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LineStreamBuf : public streambuf {
public:
  INLINE LineStreamBuf();
  virtual ~LineStreamBuf();

  INLINE bool is_text_available() const;
  string get_line();
  INLINE bool has_newline() const;

protected:
  virtual int overflow(int c);
  virtual int sync();

private:
  INLINE void write_chars(const char *start, int length);

  string _data;
  bool _has_newline;
};

#include "lineStreamBuf.I"

#endif
